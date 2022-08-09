/*
 * System boot configuration for CB030.
 */

#include "systype.h"
#include <sysboot.h>

/* define these if not depending on the defs in rombug.l */
#ifndef ROMBUG
int     errno;
u_char  trapflag;
#endif  /* ROMBUG */

/* base of non-volatile ramdisk (required by diskboot.c) */
Sect_zero   rdiskbase;

/* required by clib.l to avoid getting the default */
#ifdef _UCC
_stkhandler() {}
#endif

extern error_code bootcf(void);
extern error_code sysreset(void);

int
getbootmethod(void)
{
    /* list boot drivers in order that they should be tried */
    iniz_boot_driver(bootcf, "", "boot from CompactFlash", "");
    iniz_boot_driver(loadrom, "", "download from ROM", "");
    iniz_boot_driver(sysreset, "", "reset the system", "");
    vflag = TRUE;
    return AUTOSELECT;
}

Bdrivdef
getboottype(void)
{
    return NULL;
}
