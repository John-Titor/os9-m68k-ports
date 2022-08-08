/*
 * System boot configuration for CB030.
 */

#include "systype.h"
#include <sysboot.h>

extern error_code bootcf(void);
extern error_code sysreset(void);

Sect_zero   rdiskbase;          /* base of non-volatile ramdisk (required by diskboot.c) */

#ifdef _UCC
/*
 * Dummy _stkhandler routine for clib.l calls
 */
_stkhandler()
{
}
#endif


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
