/*
 * SCF driver tests.
 *
 * Requires hardware loopback (tx<->rx, cts<->rts>) installed.
 *
 * - open descriptor
 * 
 * GetOpt
 * - _os_gs_devnm
 *   verify name
 * - _os_gs_popt
 *   print / sanity check
 * - _os_gs_ready
 *   verify not ready
 *
 * SetOpt
 * - _os_ss_popt
 *   verify legal speeds can be set
 *   verify legal line options can be set
 * - _os_ss_sendsig, _os_ss_dcoff, _os_ss_dcon
 *   verify no error on first set
 *   verify error on duplicate set
 * - _os_ss_relea
 *   verify no error
 *   verify can first-set signals again
 * - _os_ss_dsrts, _os_ss_enrts
 *   verify no error
 *
 * Loopback data test
 * - verify no data available
 * - register data-available signal handler
 * - disable all flow control
 * - send byte
 * - verify signal received
 * - verify data available
 * - read data
 *
 * Overflow error test
 * - verify no data available
 * - register data-available signal handler
 * - disable all flow control
 * - send too much data
 * - verify overflow error (how?)
 *
 * todo: 
 *  - how to test soft flow control?
 *  - how to test hard flow control?
 *  - fork helper process?
 */

#include <assert.h>
#include <const.h>
#include <errno.h>
#include <modes.h>
#include <scf.h>
#include <signal.h>
#include <sg_codes.h>
#include <stdio.h>
#include <stdlib.h>

#include "unitest.h"

static const char *PORT_NAME = "/t2";

/* custom / testing sg codes */
#define SS_LoopbackOn   256
#define SS_LoopbackOff  257

/* custom signal values */
#define SIG_DATAREADY   100
#define SIG_DCDON       101
#define SIG_DCDOFF      102

static const char *speed_names[] = {
    "50",
    "75",
    "110",
    "134.5",
    "150",
    "300",
    "600",
    "1200",
    "1800",
    "2000",
    "2400",
    "3600",
    "4800",
    "7200",
    "9600",
    "19200",
    "38400",
    "56000",
    "64000",
    "31250",
    "31250",
    "57600",
    "115200",
    "230400",
    "460800",
    "921600",
    "76800",
    "153600",
    "307200",
    "614400",
    "1228800"
};

static const unsigned speed_max = sizeof(speed_names) / sizeof(speed_names[0]);
volatile unsigned sig_ready_count = 0;
volatile unsigned sig_dcdon_count = 0;
volatile unsigned sig_dcdoff_count = 0;

void
_setup(void **args)
{
    _os_open(PORT_NAME, FAM_READ|FAM_WRITE, (path_id *)&args[0]);
    args[1] = malloc(sizeof(struct scf_opt));
}
#define PATH ((path_id)args[0])
#define POPT ((struct scf_opt *)args[1])

void
_teardown(void **args)
{
    _os_close((path_id)args[0]);
    free(args[1]);
}

u_int16
_os_SetStt(path_id path, u_int16 code, u_int32 s_arg, void *p_arg)
{
    _asm(0,
         "  os9     I$SetStt"
         "  bcs     %0"
         "  moveq.l #0,d1"
         "%0",
         __label(),
         __reg_d0(__obj_copy(path)),
         __reg_d1(__obj_modify(code)),
         __reg_d2(s_arg),
         __reg_a0(p_arg));
    return code;
}

void
sig_ready(int signum)
{
    sig_ready_count++;
}

void
sig_dcd(int signum)
{
    if (signum == SIG_DCDON) {
        sig_dcdon_count++;
    }
    if (signum == SIG_DCDOFF) {
        sig_dcdoff_count++;
    }
}

static void
test_os_gs_devnm(void **args)
{
    char namebuf[32];

    T_ASSERT(_os_gs_devnm(PATH, namebuf) == SUCCESS);
    T_ASSERT_STRING(namebuf, PORT_NAME+1);
}

static void
test_os_gs_popt(void **args)
{
    u_int32 size = sizeof(*POPT);

    T_ASSERT(_os_gs_popt(PATH, &size, POPT) == SUCCESS);
    T_ASSERT(POPT->pd_dtp == DT_SCF);
    T_ASSERT(POPT->pd_bau < speed_max);
/*
    printf("device type                                     %u\n", (unsigned)POPT->pd_dtp);
    printf("case (0=both, any other value=upper only)       %u\n", (unsigned)POPT->pd_upc);
    printf("backsp (0=bse, any other value=bse,sp,bse)      %u\n", (unsigned)POPT->pd_bso);
    printf("delete (0=bse over line, any other value=crlf)  %u\n", (unsigned)POPT->pd_dlo);
    printf("echo (0=no echo)                                %u\n", (unsigned)POPT->pd_eko);
    printf("autolf (0=no auto lf)                           %u\n", (unsigned)POPT->pd_alf);
    printf("end of line null count                          %u\n", (unsigned)POPT->pd_nul);
    printf("pause (0=no end of page pause)                  %u\n", (unsigned)POPT->pd_pau);
    printf("lines per page                                  %u\n", (unsigned)POPT->pd_pag);
    printf("backspace character                             %u\n", (unsigned)POPT->pd_bsp);
    printf("delete line character                           %u\n", (unsigned)POPT->pd_del);
    printf("end of record char (read only)                  %u\n", (unsigned)POPT->pd_eor);
    printf("end of file char                                %u\n", (unsigned)POPT->pd_eof);
    printf("reprint line char                               %u\n", (unsigned)POPT->pd_rpr);
    printf("dup last line char                              %u\n", (unsigned)POPT->pd_dup);
    printf("pause char                                      %u\n", (unsigned)POPT->pd_psc);
    printf("kbd intr char (ctl c)                           %u\n", (unsigned)POPT->pd_int);
    printf("kbd quit char (ctl q)                           %u\n", (unsigned)POPT->pd_qut);
    printf("backspace echo character                        %u\n", (unsigned)POPT->pd_bse);
    printf("line overflow char (bell)                       %u\n", (unsigned)POPT->pd_ovf);
    printf("parity code                                     %u\n", (unsigned)POPT->pd_par);
    printf("acia baud rate (color computer)                 %u\n", (unsigned)POPT->pd_bau);
    printf("x-on char                                       %u\n", (unsigned)POPT->pd_xon);
    printf("x-off char                                      %u\n", (unsigned)POPT->pd_xoff);
    printf("Tab character (0=none)                          %u\n", (unsigned)POPT->pd_Tab);
    printf("Tab field size                                  %u\n", (unsigned)POPT->pd_Tabs);
    printf("current column number                           %u\n", (unsigned)POPT->pd_Col);
    printf("most recent I/O error status                    %u\n", (unsigned)POPT->pd_err);
*/
}

static void
test_os_gs_ready(void **args)
{
    u_int32 count = 0;

    T_ASSERT(_os_gs_ready(PATH, &count) == E_NOTRDY);
    T_ASSERT(count == 0);
}

static void
test_os_ss_popt(void **args)
{
    u_int32 size = sizeof(*POPT);

    /* test all valid speed settings */
    T_ASSERT(_os_gs_popt(PATH, &size, POPT) == SUCCESS);
    for (POPT->pd_bau = 0; POPT->pd_bau < speed_max; POPT->pd_bau++) {
        ASSERT((_os_ss_popt(PATH, sizeof(*POPT), POPT) == SUCCESS),
               POPT->pd_bau,
               speed_names[POPT->pd_bau],
               "set speed %u/%s (possibly unsupported)");
    }

    /* test an invalid speed setting */
    POPT->pd_bau = speed_max;
    ASSERT((_os_ss_popt(PATH, sizeof(*POPT), POPT) != SUCCESS),
           POPT->pd_bau,
           speed_names[POPT->pd_bau],
           "set speed %u/%s (should fail)");

    /* test sensible line settings */
    POPT->pd_bau = BAUD9600;
    POPT->pd_par = NOPARITY | WORDSIZE8 | ONESTOP;
    ASSERT((_os_ss_popt(PATH, sizeof(*POPT), POPT) == SUCCESS),
           POPT->pd_par,
           "NOPARITY | WORDSIZE8 | ONESTOP",
           "set line 0x%x = %s");

    POPT->pd_par = ODDPARITY | WORDSIZE8 | ONESTOP;
    ASSERT((_os_ss_popt(PATH, sizeof(*POPT), POPT) == SUCCESS),
           POPT->pd_par,
           "ODDPARITY | WORDSIZE8 | ONESTOP",
           "set line 0x%x = %s");

    POPT->pd_par = EVENPARITY | WORDSIZE8 | ONESTOP;
    ASSERT((_os_ss_popt(PATH, sizeof(*POPT), POPT) == SUCCESS),
           POPT->pd_par,
           "EVENPARITY | WORDSIZE8 | ONESTOP",
           "set line 0x%x = %s");
}

static void
test_os_ss_signals(void **args)
{
    T_ASSERT(_os_ss_sendsig(PATH, SIG_DATAREADY) == SUCCESS);
    T_ASSERT(_os_ss_sendsig(PATH, SIG_DATAREADY) != SUCCESS);
    T_ASSERT(_os_ss_dcoff(PATH, SIG_DCDOFF) == SUCCESS);
    T_ASSERT(_os_ss_dcoff(PATH, SIG_DCDOFF) != SUCCESS);
    T_ASSERT(_os_ss_dcon(PATH, SIG_DCDON) == SUCCESS);
    T_ASSERT(_os_ss_dcon(PATH, SIG_DCDON) != SUCCESS);
    T_ASSERT(_os_ss_relea(PATH) == SUCCESS);
    T_ASSERT(_os_ss_sendsig(PATH, SIG_DATAREADY) == SUCCESS);
    T_ASSERT(_os_ss_dcoff(PATH, SIG_DCDOFF) == SUCCESS);
    T_ASSERT(_os_ss_dcon(PATH, SIG_DCDON) == SUCCESS);
}

static void
test_os_ss_rts(void **args)
{
    T_ASSERT(_os_ss_enrts(PATH) == SUCCESS);
    T_ASSERT(_os_ss_dsrts(PATH) == SUCCESS);
}

static void
test_loopback_noflow(void **args)
{
    u_int32 size = sizeof(*POPT);
    u_int32 count = 0;
    u_int32 ticks = 0;
    u_int8 sbuf = 'X';
    u_int8 rbuf = '\0';

    /* configure port 9600n81 */
    T_ASSERT(_os_gs_popt(PATH, &size, POPT) == SUCCESS);
    POPT->pd_bau = BAUD9600;
    POPT->pd_par = NOPARITY | WORDSIZE8 | ONESTOP;
    POPT->pd_xon = 0;
    POPT->pd_xoff = 0;
    T_ASSERT(_os_ss_popt(PATH, size, POPT) == SUCCESS);
    T_ASSERT(_os_ss_dsrts(PATH) == SUCCESS);

    /* enable loopback */
    /*T_ASSERT(_os_SetStt(PATH, SS_LoopbackOn, 0, 0) == SUCCESS);*/

    /* verify no data waiting */
    T_ASSERT(_os_gs_ready(PATH, &count) == E_NOTRDY);
    T_ASSERT(count == 0);

    /* write a byte */
    count = 1;
    T_ASSERT(_os_write(PATH, &sbuf, &count) == SUCCESS);
    T_ASSERT(count == 1);

    /* give the byte time to arrive */
    ticks = 100;
    _os9_sleep(&ticks);

    /* assert that it has arrived */
    count = 0;
    T_ASSERT(_os_gs_ready(PATH, &count) == SUCCESS);
    T_ASSERT(count == 1);

    /* verify that it's correct */
    T_ASSERT(_os_read(PATH, &rbuf, &count) == SUCCESS);
    T_ASSERT(count == 1);
    T_ASSERT(sbuf == rbuf);
}

int main(int argc, const char *argv[])
{
    printf("COM driver testing %s\n", PORT_NAME);
    T_SETUP(_setup);
    T_TEARDOWN(_teardown);

    signal(SIG_DATAREADY, sig_ready);
    signal(SIG_DCDON, sig_dcd);
    signal(SIG_DCDOFF, sig_dcd);

    TEST(_os_gs_devnm,      test_os_gs_devnm(T_SETUP_RESULT));
    TEST(_os_gs_popt,       test_os_gs_popt(T_SETUP_RESULT));
    TEST(_os_gs_ready,      test_os_gs_ready(T_SETUP_RESULT));
    TEST(_os_ss_popt,       test_os_ss_popt(T_SETUP_RESULT));
    TEST(_os_ss_signals,    test_os_ss_signals(T_SETUP_RESULT));
    TEST(_os_ss_rts,        test_os_ss_rts(T_SETUP_RESULT));
    TEST(loopback_noflow,   test_loopback_noflow(T_SETUP_RESULT));


    T_CONCLUDE();
    exit(0);

#if 0
    path_id path;
    struct scf_opt opt;
    u_int32 size = sizeof(opt);

    /* verify descriptor can be opened */
    assert(_os_open("/t2", FAM_READ+FAM_WRITE, &path) == SUCCESS);

    {
        char namebuf[32];
        assert(_os_gs_devnm(path, namebuf) == SUCCESS);
        printf("name:                                           %.31s\n", namebuf);
    }


    opt.pd_bau = BAUD9600;
    assert(_os_ss_popt(path, sizeof(opt), &opt) == SUCCESS);

    {
        int j;

        for (j = 0; j < 1000; j++) {
            char buf = 'a';
            u_int32 count = 1;
            assert(_os_write(path, &buf, &count) == SUCCESS);
            assert(count == 1);
            fprintf(stderr, "%d\n", j);
        }
    }

    opt.pd_par |= SS_LoopbackOn;
    assert(_os_ss_popt(path, size, &opt) == SUCCESS);

    {
        u_char i;

        for (i = BAUD9600; i <= BAUD1228800; i++) {
            printf("Set speed=%s\n", speed_names[i]);
            opt.pd_bau = i;
            assert(_os_ss_popt(path, sizeof(opt), &opt) == SUCCESS);

            {
                char buf = 'x';
                u_int32 count = 1;
                int j;

                assert(_os_write(path, &buf, &count) == SUCCESS);
                assert(count == 1);

                count = 0;
                for (j = 0; j < 1000; j++) {
                    if (_os_gs_ready(path, &count) == SUCCESS) {
                        break;
                    }
                }
                printf("%d cycles\n", j);
                assert(count == 1);
                buf = '\0';
                assert(_os_read(path, &buf, &count) == SUCCESS);
                assert(count == 1);
            }
        }
    }
#endif
}
