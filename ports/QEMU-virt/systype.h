/* 
 * System Definitions
 */

#ifndef _types
#include <types.h>
#endif

#include <gendefs.h>

#define CBOOT           1
#define QEMU_virt       0x76697274

#define GF_PIC_1_BASE   0xff000000  /* Goldfish PIC for IPL1 - irq 32 = GF_TTY */
#define GF_PIC_2_BASE   0xff001000  /* Goldfish PIC for IPL2 - irq 1-32 VirtIO */
#define GF_PIC_3_BASE   0xff002000  /* Goldfish PIC for IPL3 - irq 1-32 VirtIO */
#define GF_PIC_4_BASE   0xff003000  /* Goldfish PIC for IPL4 - irq 1-32 VirtIO */
#define GF_PIC_5_BASE   0xff004000  /* Goldfish PIC for IPL5 - irq 1-32 VirtIO */
#define GF_PIC_6_BASE   0xff005000  /* Goldfish PIC for IPL6 - irq 1 = GF_RTC */

#define GF_RTC_BASE     0xff006000  /* Goldfish RTC */
#define GF_RTC_LEVEL    6
#define GF_RTC_VECTOR   30          /* level 6 autovector */

#define GF_TTY_BASE     0xff008000  /* Goldfish TTY / console */
#define GF_TTY_LEVEL    1
#define GF_TTY_VECTOR   25          /* level 1 autovector */

#define VIRT_CTRL_BASE  0xff009000  /* QEMU-virt control port */

#define VIRTIO_BASE(_x) (0xff010000 + (_x) * 0x200) /* VirtIO MMIO base per device */
