/*
 * System boot configuration for CB030.
 */

#include "systype.h"
#include <sysboot.h>

extern error_code       sysreset();

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
    /*iniz_boot_driver(romboot, "", "boot from ROM", "");*/
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
