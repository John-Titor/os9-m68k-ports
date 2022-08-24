# CB030

A simple 68030 board with lots of RAM. 

https://www.retrobrewcomputers.org/doku.php?id=builderpages:plasmo:cb030

## Status

Mostly functional. Boots from CompactFlash or ROM image.

    /term    DUART port A
    /t1      DUART port B
    /dd      aliased to /c0
    /c0      CompactFlash
    /c0_fmt  CompactFlash (formattable)

## Getting Started

Flash `ports/CB030/CMDS/BOOTOBJS/ROMBUG/romimage.dev` to the EEPROM, and
connect to serial A at 19200bps. Ensure that the hardware flow control
lines (RTS/CTS) are connected and that hardware flow control is enabled
in your terminal program.

Power the board on, and after a few seconds OS-9 should load the bootfile
from ROM and present the mshell prompt, with output similar to:

    OS-9/68K System Bootstrap
    Now trying to boot from CompactFlash.
    CF: RDY/BSY timeout
    Can't initialize the drive.
    This error occurred during the  boot driver!
    The OS-9 error code is: #000:246.
    
    Now trying to download from ROM.
    Now searching memory ($FE000000 - $FE07FFFF) for an OS-9 Kernel...
    
    An OS-9 kernel module was found at $FE015890
    A valid OS-9 bootfile was found.
    pd: can't open current directory. $

The booter will prefer a bootfile from CompactFlash, so this flash image can
be used with a CF without reflashing (though it will not run SYS/startup).

Other images include:

 - `ports/CB030/CMDS/BOOTOBJS/NOBUG/romimage.diskboot`: has no RomBug, but does
have a bootfile with tools for repairing and installing to disk. This is a the
image to use for installing to CompactFlash, and a good general-purpose ROM.

 - `ports/CB030/CMDS/BOOTOBJS/ROMBUG/romimage.no_bootfile`: has RomBug but
no bootfile; can only boot from CompactFlash.

 - `ports/CB030/CMDS/BOOTOBJS/NOBUG/romimage.no_bootfile`: has no RomBug and
no bootfile; can only boot from CompactFlash. This is the smallest ROM.
