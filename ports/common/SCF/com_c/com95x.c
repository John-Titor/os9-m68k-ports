/*
 * SCF driver for the OX16C95x UART.
 * 
 * TODO:
 *  - do we need to handle _os_gs_eof ?
 *  - respect xon/xoff popt changes (should update V_XON/V_XOFF, too?)
 *  - send break
 */

#include <errno.h>
#include <modes.h>          /* for _os_write* / debug only */
#include <procid.h>
#include <scf.h>            /* must be included before <path.h> */
#include <path.h>
#include <sg_codes.h>
#include <signal.h>
#include <stdarg.h>         /* for va_* / debug only */

/** track a registered signal handler */
typedef struct {
    short       pid;
    short       sig;
    short       path;
} sig_info;

/** driver private state */
typedef struct {
    status_code irq_level;      /* IRQ mask level */

    u_int8      cur_IER;        /* IER shadow */
    u_int8      cur_LCR;        /* LCR shadow */
    u_int8      cur_MCR;        /* MCR shadow */
    u_int8      cur_PAR;        /* current PD_PAR */
    u_int8      cur_BAU;        /* current PD_BAU */

    sig_info    sig_rxrdy;      /* proc/signal for Rx ready */
    sig_info    sig_dcdon;      /* proc/signal for DCD asserted */
    sig_info    sig_dcdoff;     /* proc/signal for DCD de-asserted */

    u_int8      tx_space;       /* estimated TX FIFO space */
    u_int8      rx_in;          /* rx_buf in ptr */
    u_int8      rx_out;         /* rx_buf out ptr */
    char        rx_buf[256];    /* receive buffer */
} com_softc;

#define TXFIFO_SIZE         128
#define RXBUF_HYSTERESIS    32

/* custom / testing sg codes */
#define SS_LoopbackOn   256
#define SS_LoopbackOff  257

/** dummy softc to pad out static data size - do not reference directly */
com_softc _dummy_softc;

/** layout of static data space */
typedef struct {
    /* system per-instance data */
    struct scfstatic    _scf;
    /* driver per-instance data */
    com_softc           _sc;
} driver_static;

/* accessor macros */
#define SCF(_ds)        ((_ds)->_scf)
#define SOFTC(_ds)      ((_ds)->_sc)
#define IOBASE(_ds)     ((u_int8 *)(SCF(_ds).v_sysio.v_port))

/* read/write register */
#define REG_RW(_name, _ofs)                                                                 \
static u_int8 get ## _name (driver_static *ds) { return *(IOBASE(ds) + (_ofs)); }           \
static void set ## _name (driver_static *ds, u_int8 val) { *(IOBASE(ds) + (_ofs)) = val; }  \
struct hack

/* read-only register */
#define REG_RO(_name, _ofs)                                                                 \
static u_int8 get ## _name (driver_static *ds) { return *(IOBASE(ds) + (_ofs)); }           \
struct hack

/* write-only register */
#define REG_WO(_name, _ofs)                                                                 \
static void set ## _name (driver_static *ds, u_int8 val) { *(IOBASE(ds) + (_ofs)) = val; }  \
struct hack

/* write-only register with shadow copy */
#define REG_SHADOWED(_name, _ofs)                                                           \
static u_int8 get ## _name (driver_static *ds) { return SOFTC(ds).cur_ ## _name; }          \
static void set ## _name (driver_static *ds, u_int8 val)                                    \
{                                                                                           \
    SOFTC(ds).cur_ ## _name = val;                                                          \
    *(IOBASE(ds) + (_ofs)) = val;                                                           \
}                                                                                           \
struct hack

static u_int16
_mask_sr(u_int16 level)
{
    _asm(0,
         "
         move.w  sr,%1
         andi.w  #$f8ff,%1
         lsl.w   #8,%0
         or.w    %0,%1
         move.w  sr,%0
         move.w  %1,sr
         ",
         __reg_data(__obj_modify(level)),
         __reg_data());
    return(level);
}

static void
_restore_sr(u_int16 value)
{
    _asm(0,
         "
         move.w %0,sr
         ",
         value);
}

#define ENTER_CRITICAL(_ds) { u_int16 _saved_sr = _mask_sr(SOFTC(ds).irq_level);
#define EXIT_CRITICAL(_ds) _restore_sr(_saved_sr); } do {} while(0)

/*
 * Core register definitions.
 *
 * Read in conjunction with the OX16C95x datasheet.
 */
REG_RO(RBR, 0);
REG_WO(THR, 0);

REG_SHADOWED(IER, 1);                   /* masked by ASR */
#define IER_ERBFI           0x01        /* enable receive interrupt */
#define IER_ETBEI           0x02        /* enable transmit interrupt */
#define IER_ELSI            0x04        /* enable line status interrupt */
#define IER_EDSSI           0x08        /* enable modem status interrupt */
#define IER_Sleep           0x10        /* sleep mode */
#define IER_SpecChar        0x20        /* enable special character interrupt */
#define IER_RTSI            0x40        /* enable RTS interrupt */
#define IER_CTSI            0x80        /* enable CTS interrupt */

REG_RO(ISR, 2);
#define ISR_NoInt           0x01        /* no interrupt */
#define ISR_IdMask          0x3f        /* mask for interrupt ID */
#define ISR_IdLStatus       0x06        /* line status change */
#define ISR_IdRxRdy         0x04        /* data available / FIFO highwater met */
#define ISR_IdRxTmo         0x0c        /* Rx FIFO idle timeout */
#define ISR_IdTxRdy         0x02        /* Tx holding register empty */
#define ISR_IdMStatus       0x00        /* modem status change */
#define ISR_IdSpecChar      0x10        /* special character or XOFF */
#define ISR_IdFlowChg       0x20        /* CTS or RTS change of state */

REG_WO(FCR, 2);
#define FCR_Enable          0x01        /* FIFO enable */
#define FCR_RxReset         0x02        /* Rx FIFO reset */
#define FCR_TxReset         0x04        /* Tx FIFO reset */
#define FCR_DMA             0x08        /* DMA mode select (see datasheet) */

REG_SHADOWED(LCR, 3);                   /* masked by RFL */
#define LCR_WLS5            0x00        /* 5-bit mode */
#define LCR_WLS6            0x01        /* 6-bit mode */
#define LCR_WLS7            0x02        /* 7-bit mode */
#define LCR_WLS8            0x03        /* 8-bit mode */
#define LCR_STB             0x04        /* 2 stop bits */
#define LCR_PAR             0x08        /* parity enable */
#define LCR_EPS             0x10        /* even parity select */
#define LCR_FP              0x20        /* force parity */
#define LCR_SBRK            0x40        /* send break */
#define LCR_DLAB            0x80        /* divisor latch (_DLx) enable */
#define LCR_EnExtReg        0xbf        /* enable extended 650 register access */

REG_SHADOWED(MCR, 4);                   /* masked by TFL */
#define MCR_DTR             0x01        /* DTR control */
#define MCR_RTS             0x02        /* RTS control */
#define MCR_IntEn           0x08        /* interrupt enable (see datasheet) */
#define MCR_Loopback        0x10        /* loopback enable */
#define MCR_Prescale        0x80        /* prescaler enable */

REG_RO(LSR, 5);
#define LSR_RxRdy           0x01        /* data ready */
#define LSR_OR              0x02        /* overrun error */
#define LSR_PE              0x04        /* parity error */
#define LSR_FE              0x08        /* framing error */
#define LSR_ErrMask         0x0e        /* error bit mask */
#define LSR_BI              0x10        /* break indicator */
#define LSR_THRE            0x20        /* transmit holding register empty */
#define LSR_TSRE            0x40        /* transmitter empty */
#define LSR_FIE             0x80        /* error in FIFO */

REG_RO(MSR, 6);
#define MSR_DCTS            0x01        /* CTS changed */
#define MSR_DDSR            0x02        /* DSR changed */
#define MSR_TERI            0x04        /* RI changed */
#define MSR_DCDD            0x08        /* DCD changed */
#define MSR_CTS             0x10        /* CTS status */
#define MSR_DSR             0x20        /* DSR status */
#define MSR_RI              0x40        /* RI status */
#define MSR_DCD             0x80        /* DCD status */

REG_WO(SPR, 7);                         /* indexed control register select */

/* divisor latch (_LCR_DLAB set) registers */
REG_WO(DLL, 0);                         /* divisor low */
REG_WO(DLH, 1);                         /* divisor high */

/* extended 650 registers, normally disabled, write _LCR_EnExtReg to LCR
 * to enable */
REG_RW(EFR, 2);                           /* enhanced flow control register */
#define EFR_RxFlowNone      0x00        /* no in-band Rx flow control */
#define EFR_RxFlow2         0x01        /* single-character in-band using XON2/XOFF2 */
#define EFR_RxFlow1         0x02        /* single-character in-band using XON1/XOFF1 */
#define EFR_RxFlowTx        0x03        /* copy mode from _EFR_TxFlow */
#define EFR_TxFlowNone      0x00        /* no in-band Tx flow control */
#define EFR_TxFlow2         0x04        /* single-character in-band using XON2/XOFF2 */
#define EFR_TxFlow1         0x08        /* single-character in-band using XON1/XOFF1 */
#define EFR_FlowMask        0x0f        /* mask for in-band flow control bits */
#define EFR_Enhanced        0x10        /* enable enhanced mode */
#define EFR_SpecDetect      0x20        /* enable special character detection */
#define EFR_AutoRTS         0x40        /* enable automatic RTS flow control */
#define EFR_AutoCTS         0x80        /* enable automatic CTS flow control */
#define EFR_AutoFlow        0xc0        /* auto RTS+CTS */

REG_WO(XON1, 4);                        /* XON1 character */
//REG_WO(XON2, 5);                        /* XON2 character */
REG_WO(XOFF1, 6);                       /* XOFF1 character */
//REG_WO(XOFF2, 7);                       /* XOFF2 character */

/* extended 950 registers */
//REG_RW(ASR, 1);
#define ASR_TxDis           0x01        /* tx disabled due to XOFF received */
#define ASR_RTxDis          0x02        /* remote tx disabled by sending XOFF */
#define ASR_RTS             0x04        /* RTS output status */
#define ASR_DTR             0x08        /* DTR output status */
#define ASR_SpecDetect      0x10        /* special character in RHR */
#define ASR_FIFOSel         0x20        /* FIFOSEL pin state */
#define ASR_FIFOSize        0x40        /* 16/128 FIFO size indicator */
#define ASR_TxIdle          0x80        /* transmitter is idle */

REG_RO(RFL, 3);                         /* Rx FIFO level */
REG_RO(TFL, 4);                         /* Tx FIFO level */
REG_RW(ICR, 5);                         /* indexed control register, normally WO (LSR) */

/* indexed 950 registers */
#define ACR             0
#define ACR_RxDis           0x01        /* receiver disabled */
#define ACR_TxDis           0x02        /* transmitter disabled */
#define ACR_AutoDSR         0x04        /* enable automatic DSR flow control */
#define ACR_AutoDTR         0x08        /* enable automatic DTR flow control */
#define ACR_485DTRLo        0x10        /* enable RS485 active-low Tx control */
#define ACR_485DTRHi        0x30        /* enable RS485 active-high Tx control */
#define ACR_TrigLevel       0x20        /* enable 950 mode FIFO level controls */
#define ACR_ICRRead         0x40        /* map ICR over LSR for reads */
#define ACR_ExtStatus       0x80        /* enable ASR, TFL, RFL registers */
#define ACR_Default         (ACR_TrigLevel | ACR_ExtStatus)

#define CPR             1               /* clock prescaler */
#define TCR             2               /* oversampling control */
#define CKS             3               /* clock select */
#define TTL             4               /* transmit FIFO trigger level */
#define RTL             5               /* receive FIFO trigger level */
#define FCL             6               /* automatic flow control low-water mark */
#define FCH             7               /* automatic floc control high-water mark */
#define ID1             8               /* $16 */
#define ID2             9               /* $c9 */
#define ID3             10              /* $54 */
#define REV             11              /* $04 */
#define CSR             12              /* write 0 to reset device */
#define NMR             13              /* 9-bit mode register */
#define MDM             14              /* wakeup signal enables */
#define RFC             15              /* copy of FCR */
#define GDS             16              /* good data status */
#define DMS             17              /* DMA status */
#define PIDX            18              /* port index (0-3) */
#define CKA             19              /* clock alteration */

/* line speed table - 16MHz clock */
static const struct {
    u_int8      tcr;
    u_int8      dlh;
    u_int8      dll;
    u_int8      cpr;
} speedtab[] = {
    {0x00, 0x02, 0x80, 0xfa},   /* 00: 50 error = 0.00% */
    {0x0d, 0x06, 0x41, 0x52},   /* 01: 75 error = 0.00% */
    {0x05, 0x52, 0xa5, 0x0b},   /* 02: 110 error = 0.00% */
    {0x0c, 0x08, 0x27, 0x26},   /* 03: 134.5 error = 0.00% */
    {0x0d, 0x06, 0x41, 0x29},   /* 04: 150 error = 0.00% */
    {0x06, 0x03, 0x1f, 0x59},   /* 05: 300 error = 0.00% */
    {0x0e, 0x01, 0x91, 0x26},   /* 06: 600 error = 0.00% */
    {0x0e, 0x01, 0x91, 0x13},   /* 07: 1200 error = 0.00% */
    {0x0d, 0x02, 0x23, 0x0a},   /* 08: 1800 error = 0.00% */
    {0x00, 0x00, 0x10, 0xfa},   /* 09: 2000 error = 0.00% */
    {0x07, 0x01, 0x91, 0x13},   /* 0a: 2400 error = 0.00% */
    {0x05, 0x02, 0x23, 0x0d},   /* 0b: 3600 error = 0.00% */
    {0x0c, 0x00, 0x0b, 0xca},   /* 0c: 4800 error = 0.01% */
    {0x00, 0x00, 0x0b, 0x65},   /* 0d: 7200 error = 0.01% */
    {0x0c, 0x00, 0x0b, 0x65},   /* 0e: 9600 error = 0.01% */
    {0x0b, 0x00, 0x03, 0xca},   /* 0f: 19200 error = 0.01% */
    {0x0b, 0x00, 0x03, 0x65},   /* 10: 38400 error = 0.01% */
    {0x0e, 0x00, 0x01, 0xa3},   /* 11: 56000 error = 0.16% */
    {0x00, 0x00, 0x01, 0x7d},   /* 12: 64000 error = 0.00% */
    {0x00, 0x00, 0x02, 0x80},   /* 13: 31250 error = 0.00% */
    {0x00, 0x00, 0x02, 0x80},   /* 14: 31250 error = 0.00% */
    {0x0b, 0x00, 0x01, 0xca},   /* 15: 57600 error = 0.01% */
    {0x0b, 0x00, 0x01, 0x65},   /* 16: 115200 error = 0.01% */
    {0x0f, 0x00, 0x01, 0x25},   /* 17: 230400 error = 0.10% */
    {0x0c, 0x00, 0x01, 0x17},   /* 18: 460800 error = 0.64% */
    {0x06, 0x00, 0x01, 0x17},   /* 19: 921600 error = 0.64% */
    {0x0e, 0x00, 0x01, 0x77},   /* 1a: 76800 error = 0.04% */
    {0x07, 0x00, 0x01, 0x77},   /* 1b: 153600 error = 0.04% */
    {0x00, 0x00, 0x01, 0x1a},   /* 1c: 307200 error = 0.16% */
    {0x00, 0x00, 0x01, 0x0d},   /* 1d: 614400 error = 0.16% */
    {0x08, 0x00, 0x01, 0x0d}    /* 1e: 1228800 error = 0.16% */
};

#define speedtab_max (sizeof(speedtab) / sizeof(speedtab[0]))

/**
 * @brief             Write an indexed register.
 *
 * @param      ds     driver static pointer
 * @param[in]  index  index value
 * @param[in]  val    new value
 */
static void
setIreg(driver_static *ds, u_int8 index, u_int8 val)
{
    /* not reentrant */
    setSPR(ds, index);
    setICR(ds, val);
}

/**
 * @brief      Read an indexed register.
 *
 * @param      ds     driver static pointer
 * @param[in]  index  index value
 *
 * @return     The indexed register.
 */
static u_int8
getIreg(driver_static *ds, u_int8 index)
{
    u_int8  val;

    /* not reentrant */
    setIreg(ds, ACR, ACR_Default | ACR_ICRRead);
    setSPR(ds, index);
    val = getICR(ds);
    setIreg(ds, ACR, ACR_Default);
    return val;
}

/**
 * @brief      Emit a debug message.
 *
 * @param[in]  s          Format string
 * @param[in]  <unnamed>  optional unsigned arguments
 * 
 * Each '@' in the format string is replaced with the corresponding argument.
 * All values are formatted as 8 hex digits with leading zeros.
 */
static void msg(const char *s, ...)
{
    va_list ap;
    u_int32 l;

    va_start(ap, s);

    l = 8;
    _os_write(1, "com95x: ", &l);
    l = 1;
    while (*s) {
        if (*s != '@') {
            _os_write(1, s, &l);
        } else {
            unsigned v = va_arg(ap, unsigned);
            int shift = 28;
            do {
                unsigned n = (v >> shift) & 0xf;
                char c = (n < 10) ? ('0' + n) : ('a' + n - 10);
                _os_write(1, &c, &l);
                shift -= 4;
            } while (shift >= 0);
        }
        s++;
    }
    _os_writeln(1, "\n", &l);
}

/**
 * @brief      Configure a port.
 *
 * @param      ds    Driver static data pointer.
 * @param[in]  opt   Options.
 *
 * @return     0 on success, negative error code on error.
 */
static int
config(driver_static *ds, const struct scf_opt *opt)
{
    /* check speed arg */
    if ((opt->pd_bau >= speedtab_max) ||
        (speedtab[opt->pd_bau].tcr == 0xff)) {
        msg("bad speed @", (unsigned)opt->pd_bau);
        return -E_BMODE;
    }

//    msg("speed @  line @", (unsigned)opt->pd_bau, (unsigned)opt->pd_par);

    /* soft-reset */
    setIreg(ds, CSR, 0);

    /* enable enhanced mode */
    setLCR(ds, LCR_EnExtReg);
    setEFR(ds, EFR_Enhanced);
    setLCR(ds, getLCR(ds) & ~LCR_DLAB);

    /* enable prescaler, interrupts, maybe loopback */
    setMCR(ds, MCR_Prescale | MCR_IntEn | ((opt->pd_par & 0x40) ? MCR_Loopback : 0));

    /* enable extended trigger control and status registers */
    setIreg(ds, ACR, ACR_Default);

    /* enable / clear FIFOs */
    setFCR(ds, FCR_Enable | FCR_RxReset | FCR_TxReset);
    setFCR(ds, FCR_Enable);

    /* set incoming flow control thresholds */
    setIreg(ds, FCL, 16);
    setIreg(ds, FCH, 100);
    setIreg(ds, TTL, (TXFIFO_SIZE-32));
    setIreg(ds, RTL, 1);

    /* update state to match soft-reset */
    SOFTC(ds).cur_IER = 0;
    SOFTC(ds).cur_LCR = 0;
    SOFTC(ds).rx_in = 0;
    SOFTC(ds).rx_out = 0;

    /* configure line speed */
    setIreg(ds, TCR, speedtab[opt->pd_bau].tcr);
    setLCR(ds, getLCR(ds) | LCR_DLAB);
    setDLH(ds, speedtab[opt->pd_bau].dlh);
    setDLL(ds, speedtab[opt->pd_bau].dll);
    setLCR(ds, getLCR(ds) & ~LCR_DLAB);
    setIreg(ds, CPR, speedtab[opt->pd_bau].cpr);

    /* configure in-band flow control */
    setXON1(ds, opt->pd_xon);
    setXOFF1(ds, opt->pd_xoff);
    SCF(ds).v_xon = opt->pd_xon;
    SCF(ds).v_xoff = opt->pd_xoff;
    if (opt->pd_xon && opt->pd_xoff) {
        setLCR(ds, LCR_EnExtReg);
        setEFR(ds, (getEFR(ds) & ~EFR_FlowMask) | EFR_RxFlowTx | EFR_TxFlow1);
        setLCR(ds, getLCR(ds) & ~LCR_DLAB);
    }

    /* compute new line-control byte */
    {
        u_int8 lcr = 0;

        switch (opt->pd_par & 0x3) {
        case EVENPARITY:
            lcr |= LCR_PAR | LCR_EPS;
            break;
        case ODDPARITY:
            lcr |= LCR_PAR;
            break;
        }
        switch (opt->pd_par & (0xc)) {
        case WORDSIZE8:
            lcr |= LCR_WLS8;
            break;
        case WORDSIZE7:
            lcr |= LCR_WLS7;
            break;
        case WORDSIZE6:
            lcr |= LCR_WLS6;
            break;
        case WORDSIZE5:
            lcr |= LCR_WLS5;
            break;
        }
        switch (opt->pd_par & 0x30) {
        case TWOSTOP:
            lcr |= LCR_STB;
        }
        setLCR(ds, lcr);
    }

    /* save current settings */
    SOFTC(ds).cur_PAR = opt->pd_par;
    SOFTC(ds).cur_BAU = opt->pd_bau;

    /* enable receive interrupts & modem status interrupts */
    setIER(ds, getIER(ds) | IER_ERBFI | IER_EDSSI);

#if 0
    msg("config done\n");
    msg("IER @", getIER(ds));
    msg("LCR @", getLCR(ds));
    msg("MCR @", getMCR(ds));
    msg("ISR @", getISR(ds));
    msg("LSR @", getLSR(ds));
    msg("MSR @", getMSR(ds));
    msg("RFL @", getRFL(ds));
    msg("TFL @", getTFL(ds));
    msg("ACR @", getIreg(ds, ACR));
    msg("CPR @", getIreg(ds, CPR));
    msg("TCR @", getIreg(ds, TCR));
    msg("CKS @", getIreg(ds, CKS));
    msg("TTL @", getIreg(ds, TTL));
    msg("RTL @", getIreg(ds, RTL));
    msg("FCL @", getIreg(ds, FCL));
    msg("FCH @", getIreg(ds, FCH));
    msg("ID1 @", getIreg(ds, ID1));
    msg("ID2 @", getIreg(ds, ID2));
    msg("ID3 @", getIreg(ds, ID3));
    msg("REV @", getIreg(ds, REV));
    msg("CSR @", getIreg(ds, CSR));
    msg("NMR @", getIreg(ds, NMR));
    msg("MDM @", getIreg(ds, MDM));
    msg("RFC @", getIreg(ds, RFC));
    msg("GDS @", getIreg(ds, GDS));
    msg("DMS @", getIreg(ds, DMS));
    msg("PIDX @", getIreg(ds, PIDX));
    msg("CKA @", getIreg(ds, CKA));
#endif

    return 0;
}

/**
 * @brief      Reconfigure a port if the configuration has changed.
 *
 * @param      ds    Driver static data pointer.
 * @param[in]  opt   Options.
 *
 * @return     0 on success, negative error code on error.
 */
static int
config_maybe(driver_static *ds, const struct scf_opt *opt)
{
    if ((opt->pd_bau != SOFTC(ds).cur_BAU) ||
        (opt->pd_par != SOFTC(ds).cur_PAR) ||
        (opt->pd_xon != SCF(ds).v_xon) ||
        (opt->pd_xoff != SCF(ds).v_xoff)) {

        return config(ds, opt);
    }
    return 0;
}

/**
 * @brief      Wait to be woken by a signal or some other event.
 *
 * @param      proc_desc  Process descriptor.
 *
 * @return     0 if woken by a benign signal, negative error code otherwise.
 */
static int
wait(procid *proc_desc)
{
    _os9_sleep(0);
    if (fatalsignal(proc_desc->_signal)) {
        return -E_SIGNAL;
    }
    if (proc_desc->_state & PS_CONDEMN) {
        return -E_PRCABT;
    }
    return 0;
}

/**
 * @brief      Initialise a port.
 *
 * @param      dev   Pointer to device descriptor.
 * @param      ds    Driver static data pointer.
 *
 * @return     0 on success, negative error code on error.
 */
int
Init(const mod_dev *dev, driver_static *ds)
{
//    msg("init");

    /* mask port interrupts */
    setIER(ds, 0);

    /* verify chip ID */
    if ((getIreg(ds, ID1) != 0x16) ||
        (getIreg(ds, ID2) != 0xc9) ||
        (getIreg(ds, ID3) != 0x54)) {
        msg("bad 16c95x ID");
        return -ENODEV;
    }

    /* configure IRQ masking level */
    SOFTC(ds).irq_level = dev->_mirqlvl;

    /* do initial configuration */
    config(ds, (struct scf_opt *)&(dev->_mdtype));

//    msg("ready");

    /* shim handles interrupt registration */
    return 0;
}

int
Read(Pathdesc path, driver_static *ds, procid *proc_desc)
{
    return 0;
}

/**
 * @brief      Write a byte to the port Tx FIFO
 *
 * @param[in]  c     The byte to send.
 * @param      path  The path descriptor writing.
 * @param      ds    Driver static data pointer.
 *
 * @return     0 on success, negative error code on error.
 */
int
Write(char c, Pathdesc path, driver_static *ds, procid *proc_desc)
{
    u_int8  space = SOFTC(ds).tx_space;
    int     err = 0;

    /* note: Not currently masking interrupts, as ETBEI should only
     *       be set when we are waiting for space in the FIFO and thus
     *       there should be no opportunity for a race.
     * note: not reentrant (tx_space race)
     */
    for (;;) {

        /* if we have run out of credits, try to refill by checking the TX FIFO level */
        if (space == 0) {
            u_int8  tfl;

            /* read twice per spec */
            do {
                tfl = getTFL(ds);
            } while (tfl != getTFL(ds));
            space = TXFIFO_SIZE - tfl;
            msg("tx: refill @ credits", (unsigned)space);
        }

        /* if we have a TX credit, put the byte in the FIFO and consume the credit */
        if (space > 0) {
            msg("tx: @", (unsigned)c);
            setTHR(ds, c);
            SOFTC(ds).tx_space = space - 1;
            break;
        }

        /* FIFO full - need to sleep until it drains */
        ENTER_CRITICAL(ds) {
            msg("tx: wait for FIFO");
            /* arrange for wakeup on Tx interrupt */
            SCF(ds).v_sysio.v_wake = SCF(ds).v_sysio.v_busy;

            /* enable Tx interrupt */
            setIER(ds, getIER(ds) | IER_ETBEI);

            /* and wait to be woken */
            err = wait(proc_desc);

        } EXIT_CRITICAL(ds);

        if (err) {
            break;
        }
    }
    return err;
}

/**
 * @brief      Handle GetStat system call.
 *
 * @param[in]  code       Operation code.
 * @param      path       Path descriptor.
 * @param      ds         Driver static data pointer.
 * @param      proc_desc  Current process descriptor.
 * @param      proc_regs  Current process registers (for args).
 *
 * @return     The stat.
 */
int
GetStat(short code, Pathdesc path, driver_static *ds, procid *proc_desc, REGISTERS *proc_regs)
{
    switch (code) {
        /* check for data available */
    case SS_Ready:
        msg("rdy: RFL @", getRFL(ds));
//        msg("rdy: ISR @", getISR(ds));
        return (SOFTC(ds).rx_in == SOFTC(ds).rx_out) ? -E_NOTRDY : 0;

        /* update path with current options */
    case SS_Opt:
        path->scfopt.pd_bau = SOFTC(ds).cur_BAU;
        path->scfopt.pd_par = SOFTC(ds).cur_PAR;
//        msg("SG_Opt  bau @  par @", path->scfopt.pd_bau, path->scfopt.pd_par);
        return 0;
    }
    return -E_UNKSVC;
}

/**
 * @brief      Register a process/signal/path for an internal event.
 *
 * @param      ds         Driver static data pointer.
 * @param      sig      The internal event to register for.
 * @param[in]  pid      The pid to signal.
 * @param[in]  signal   The signal to send.
 * @param[in]  path_id  The path identifier.
 *
 * @return     0 on success, -E_NOTRDY if the event is already registered for.
 */
static int
sig_register(driver_static *ds,
            sig_info *sig,
            unsigned short pid,
            unsigned short signal,
            unsigned short path_id)
{
    if (sig->pid) {
        return -E_NOTRDY;
    }
    ENTER_CRITICAL(ds) {
        sig->pid = pid;
        sig->sig = signal;
        sig->path = path_id;
    } EXIT_CRITICAL(ds);
    return 0;
}

/**
 * @brief      Unregister a handler for an internal event.
 *
 * @param      ds         Driver static data pointer.
 * @param      sig   The internal event to unregister for.
 * @param[in]  pid   The pid to be unregistered.
 */
static void
sig_unregister(driver_static *ds, sig_info *sig, unsigned short pid)
{
    ENTER_CRITICAL(ds) {
        if (sig->pid == pid) {
            sig->pid = 0;
            sig->sig = 0;
            sig->path = 0;
        }
    } EXIT_CRITICAL(ds);
}

/**
 * @brief      Handle SetStat system call.
 *
 * @param[in]  code       Operation code.
 * @param      path       Path descriptor.
 * @param      ds         Driver static data pointer.
 * @param      proc_desc  Current process descriptor.
 * @param      proc_regs  Current process registers (for args).
 *
 * @return     { description_of_the_return_value }
 */
int
SetStat(short code, Pathdesc path, driver_static *ds, procid *proc_desc, REGISTERS *proc_regs)
{
    switch (code) {
        /* register data-ready signal */
    case SS_SSig:
        return sig_register(ds,
                            &(SOFTC(ds).sig_rxrdy),
                            path->path.pd_cpr,
                            proc_regs->d[2],
                            path->path.pd_pd);

        /* register DCD-turned-on signal */
    case SS_DCOn:
        return sig_register(ds, &(SOFTC(ds).sig_dcdon),
                            path->path.pd_cpr,
                            proc_regs->d[2],
                            path->path.pd_pd);

        /* register DCD-turned-off signal */
    case SS_DCOff:
        return sig_register(ds,
                            &(SOFTC(ds).sig_dcdoff),
                            path->path.pd_cpr,
                            proc_regs->d[2],
                            path->path.pd_pd);

        /* release signal registrations for calling process */
    case SS_Relea:
        sig_unregister(ds, &(SOFTC(ds).sig_rxrdy), path->path.pd_cpr);
        sig_unregister(ds, &(SOFTC(ds).sig_dcdon), path->path.pd_cpr);
        sig_unregister(ds, &(SOFTC(ds).sig_dcdoff), path->path.pd_cpr);
        return 0;

        /* en/disable RTS flow control */
    case SS_EnRTS:
    case SS_DsRTS:
        ENTER_CRITICAL(ds) {
            u_int8 lcr = getLCR(ds);

            setLCR(ds, lcr | LCR_EnExtReg);
            if (code == SS_EnRTS) {
//                msg("SS_EnRTS");
                setEFR(ds, getEFR(ds) | EFR_AutoFlow);
            } else {
//                msg("SS_DsRTS");
                setEFR(ds, getEFR(ds) & ~EFR_AutoFlow);
            }
            setLCR(ds, lcr);
        } EXIT_CRITICAL(ds);
        return 0;

        /* en/disable loopback */
    case SS_LoopbackOn:
        setMCR(ds, getMCR(ds) | MCR_Loopback);
        return 0;
    case SS_LoopbackOff:
        setMCR(ds, getMCR(ds) & ~MCR_Loopback);
        return 0;

        /* reconfigure port */
    case SS_Opt:
//        msg("SS_Opt");
        return config_maybe(ds, &path->scfopt);

    case SS_Open:
//        msg("SS_Open");
        return config_maybe(ds, &path->scfopt);

    case SS_Close:
//        msg("SS_Close");
        /* XXX drain? */
        return 0;

        /* send break */
    case SS_Break:
        return 0;   /* XXX */
    }
    msg("SetStat: unexpected %d\n", code);
    return -E_UNKSVC;
}

static void
irq_rx(driver_static *ds)
{
    u_int8      stop_slot = SOFTC(ds).rx_out - 1;
    u_int8      in_slot = SOFTC(ds).rx_in;

    for (;;) {
        /* out of FIFO space? */
        if (in_slot == stop_slot) {
            msg("rx: fifo_full");
            /* mask RX interrupts */
            setIER(ds, getIER(ds) & ~IER_ERBFI);
            break;
        }
        {
            /* get byte */
            u_int8  c = getRBR(ds);

            msg("rx: @", (unsigned)c);

            /* if non-zero, might be control character */
            if (c != 0) {

                /* interrupt? */
                if (c == SCF(ds).v_intr) {
                    _os_send(SCF(ds).v_sysio.v_lprc, SIGINT);
                } 

                /* quit? */
                else if (c == SCF(ds).v_quit) {
                    _os_send(SCF(ds).v_sysio.v_lprc, SIGABRT);
                }

                /* pause? */
                else if (c == SCF(ds).v_pchr) {
                    if (SCF(ds).v_dev2) {
                        SCF(ds).v_dev2->v_pause = c;
                    }
                }
            }

            /* stash in FIFO */
            SOFTC(ds).rx_buf[in_slot++] = c;
        }
    }

    /* update driver state */
    SOFTC(ds).rx_in = in_slot;

    /* Rx-ready signal registered? */
    if (SOFTC(ds).sig_rxrdy.pid) {
        _os_send(SOFTC(ds).sig_rxrdy.pid, SOFTC(ds).sig_rxrdy.sig);
        SOFTC(ds).sig_rxrdy.pid = 0;
    }

    /* blocked reader? */
    else if (SCF(ds).v_sysio.v_wake) {
        _os_send(SCF(ds).v_sysio.v_wake, SIGWAKE);
        SCF(ds).v_sysio.v_wake =0;
    }
}

static void
irq_tx(driver_static *ds)
{
    /* mask interrupt */
    setIER(ds, getIER(ds) & ~IER_ETBEI);

    /* anyone waiting? */
    if (SCF(ds).v_sysio.v_wake) {
        /* yes, send wake signal */
        unsigned short pid = SCF(ds).v_sysio.v_wake;
        SCF(ds).v_sysio.v_wake = 0;
        _os_send(pid, SIGWAKE);
    }
}

static void
irq_modem(driver_static *ds)
{
    u_int8  msr = getMSR(ds);
    sig_info *si = NULL;

    /* DCD changed? */
    if (msr & MSR_DCDD) {
        if (msr & MSR_DCD) {
            si = &(SOFTC(ds).sig_dcdon);
        } else {
            si = &(SOFTC(ds).sig_dcdoff);
        }
    }

    /* handler registered? */
    if (si) {
        _os_send(si->pid, si->sig);
        si->pid = 0;
    }
}

int
IRQ(driver_static *ds, void *iobase)
{
    u_int8 reason = getISR(ds) & ISR_IdMask;

    switch (reason) {
    case ISR_IdRxRdy:
    case ISR_IdRxTmo:
        msg("irq: rx");
        irq_rx(ds);
        return 0;

    case ISR_IdTxRdy:
        msg("irq: tx");
        irq_tx(ds);
        return 0;

    case ISR_IdMStatus:
        msg("irq: modem");
        irq_modem(ds);
        return 0;
    }
    msg("irq: unk @", (unsigned)reason);
    return -1;
}

void
Terminate(mod_dev *dev, driver_static *ds)
{
    /* soft-reset the port */
    setIreg(ds, CSR, 0);

//    msg("terminate\n");
    /* shim handles interrupt de-registration */
}

