/*
 * CompactFlash boot driver
 */

#include <errno.h>
#include <types.h>
#include <sysboot.h>
#include <systype.h>

/*
 * Make a path descriptor
 */
#define OPTSNAME    cf_hbd
#define SECTSIZE    512

#include <bootdesc.h>


#ifndef CF_BASE
#error Must define CF_BASE in <systype.h>
#endif
#ifndef CF_WIDTH
#error Must define CF_WIDTH in <systype.h>
#endif

#if CF_WIDTH == 8
#define REG8(_x)  (*(volatile unsigned char *)(CF_BASE+_x))
#define CF_DATA             REG8(0x00)

#define CF_FEAT             REG8(0x01)
#define CF_FEAT_8BIT            0x01
#define CF_SECTOR_COUNT     REG8(0x02)
#define CF_LBA_0            REG8(0x03)
#define CF_LBA_1            REG8(0x04)
#define CF_LBA_2            REG8(0x05)
#define CF_LBA_3            REG8(0x06)
#define CF_LBA_3_LBA_EN         0xe0

#define CF_STATUS           REG8(0x07)
#define CF_STATUS_ERR           (1<<0)
#define CF_STATUS_DRQ           (1<<3)
#define CF_STATUS_DF            (1<<5)
#define CF_STATUS_RDY           (1<<6)
#define CF_STATUS_BSY           (1<<7)

#define CF_CMD              REG8(0x07)
#define CF_CMD_READ             0x20
#define CF_CMD_IDENTIFY         0xec
#define CF_IDENTIFY_ATA             (1U<<15)    /* if bit is zero, ATA device */
#define CF_IDENTIFY_MAGIC           0x848a      /* if equal, CF device */
#define CF_CMD_SET_FEAT         0xef
#else
#error CF_WIDTH != 8 not supported yet
#endif

/*
 * Wait for the drive to become not-busy, and ready for a command, or time out.
 */
static int
cf_drive_ready(void)
{
    unsigned long timeout = 0x123456ul;

    while(--timeout) {
        /* wait for BSY to be clear and RDY to be set */
        if ((CF_STATUS & (CF_STATUS_BSY|CF_STATUS_RDY)) == CF_STATUS_RDY) {
            return 1;
        }
    }
    outstr("CF: RDY/BSY timeout\n");
    return 0;
}

/*
 * Wait for the drive to become not-busy and have data ready.
 */
static int
cf_drive_data_ready(void)
{
    unsigned long timeout = 0x123456ul;

    while(--timeout) {
        /* wait for BSY to be clear and DRQ to be set */
        if ((CF_STATUS & (CF_STATUS_BSY|CF_STATUS_DRQ)) == CF_STATUS_DRQ) {
            return 1;
        }
    }
    outstr("CF: DRQ/BSY timeout\n");
    return 0;
}

static u_int16
cf_read16(void)
{
#if CF_WIDTH == 8
    union {
        u_int8  b[2];
        u_int16 w;
    } x;
    x.b[1] = CF_DATA;
    x.b[0] = CF_DATA;
    return x.w;
#endif
}

#if CF_WIDTH == 8
static void
cf_read_sector(u_int8 *buf)
{
    unsigned count = SECTSIZE;

    while (count--) {
        *buf++ = CF_DATA;
    }
}
#endif

static error_code
cf_iniz(void)
{
    /* we want to see a drive that's ready */
    if (!cf_drive_ready()) {
        return E_NOTRDY;
    }
    /* enable LBA mode */
    CF_LBA_3 = CF_LBA_3_LBA_EN;

#if CF_WIDTH == 8
    /* enable 8-bit mode */
    CF_FEAT = CF_FEAT_8BIT;
    CF_CMD = CF_CMD_SET_FEAT;
    if (!cf_drive_ready()) {
        return E_NOTRDY;
    }
    if (CF_STATUS & CF_STATUS_ERR) {
        outstr("CF: error setting 8-bit mode\n");
        return E_BTYP;
    }
#endif

    /* identify device & verify compatibility */
    CF_CMD = CF_CMD_IDENTIFY;
    if (cf_drive_ready()) {
        u_int16 conf = cf_read16();
        if (!(conf & CF_IDENTIFY_ATA) ||    /* regular ATA drive */
            (conf == CF_IDENTIFY_MAGIC)) {  /* CF */
            return SUCCESS;
        }
    }
    outstr("CF: not a supported drive type\n");
    return E_BTYP;
}

static error_code
cf_read(u_int32 numsects, u_int32 blkaddr)
{
    u_int8      *buf = pathbuf;

    /* loop reading groups of sectors */
    while (numsects) {
        /* how many this time? */
        u_int8 count = (numsects < 256) ? numsects : 255;

        /* drive must be ready */
        if (!cf_drive_ready()) {
            return E_NOTRDY;
        }

        /* issue read command */
        CF_SECTOR_COUNT = count;
        CF_LBA_0 = blkaddr & 0xff;
        CF_LBA_1 = (blkaddr >> 8) & 0xff;
        CF_LBA_2 = (blkaddr >> 16) & 0xff;
        CF_CMD = CF_CMD_READ;

        numsects -= count;
        blkaddr += count;

        /* extract sectors from drive buffer */
        while (count--) {
            /* need DRQ... */
            if (!cf_drive_data_ready()) {
                return E_READ;
            }
            /* get a sector */
            cf_read_sector(buf);
            buf += SECTSIZE;
        }
    }
    return SUCCESS;
}

error_code
bootcf(void)
{
    defopts = &cf_hbd;
	inizdriver = cf_iniz;
    readdriver = cf_read;
    termdriver = (error_code (*)())NULL;
    return diskboot();
}