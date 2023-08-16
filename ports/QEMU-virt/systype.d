*****************************************************************************
*****************************************************************************
*
* System Definitions for `qemu-system-m68k -m virt -cpu m68040`
*
* Names beginning with underscores are local to this file.
*

QEMU_virt   equ $76697274
CPUTyp      set 68040                   * sets the CPU type
CPUType     equ QEMU_virt               * sets the board type

*****************************************************************************
*
* Memory constants
*
VTblSize    equ (256*4)                 * size of vector table

_ROMBase    equ 0                       * ROM starts at zero
_ROMSize    equ (1*1024*1024)           * 1MiB
_RAMBase    equ (_ROMBase+_ROMSize)     * RAM starts at end of ROM
_RAMMax     equ (8*1024*1024)           * 7MiB

VBRBase     equ 0                       * vectors in ROM

*
* Peripheral constants
*
_GFTTYBase  equ $ff008000               * base address of the Goldfish TTY
_GFTTYLevel equ 1                       * level 1 (isn't this really polling priority?)
_GFTTYVect  equ 25                      * ... autovector

_TckBase    equ $ff006000               * base address of the Goldfish RTC
_TckLevel   equ 6                       * level 6 (isn't this really polling priority?)
_TckVect    equ 30                      * ... autovector

*****************************************************************************
*
* ROM bootloader configuration
*

*
* Memory
*
Mem.Beg     equ _RAMBase                * exclude vector space
Mem.End     equ _RAMMax
Spc.Beg     equ _ROMBase                * search ROM for modules
Spc.End     equ _ROMBase+_ROMSize

MemDefs macro
    dc.l    Mem.Beg,Mem.End             * regular memory list
    dc.l    0                           * ... terminator
    dc.l    Spc.Beg,Spc.End             * special / module search memory list
    dc.l    0                           * ... terminator
    endm

Cons_Adr    equ _GFTTYBase              * ROM console
Comm_Adr    equ _GFTTYBase              * ROM console (not used)

*
* Options
*
MANUAL_RAM  set 1                       * RAM must be set up in SysInit
CBOOT       set 1                       * build with CBOOT
FIXED_CPUTYP set 1                      * trust CPUTyp, don't probe
RAMVects    set 0                       * vectors in RAM
SysDisk     set 0                       * no boot disk
FDsk_Vct    set 0                       * no floppy drive

*****************************************************************************
*
* Init module configuration
*
    ifdef _INITMOD

* no external cache, so caches are coherent
SnoopExt set 1

* no DMA devices, so leave caches on when doing I/O
NoDataDis set 1

CONFIG macro

* system / board name
MainFram:
    dc.b    "QEMU-virt",0

* startup module
SysStart:
    dc.b    "sysgo",0

* no statup module parameters
SysParam:
    dc.b    0

 ifdef ROMBOOT
* no root device for ROM boot
SysDev equ 0
 else
* try to iniz a default drive
SysDev:
    dc.b    "/dd",0
 endc

* console terminal
ConsolNm:
    dc.b    "/term",0

* clock module name
ClockNm:
    dc.b    "gftick",0

* ordered list of extensions
Extens:
    dc.b    "OS9P2 syscache ssm fpsp fpu OS9P3",0

* configured memory (search) list
    align
MemList:
*           type,priority,attributes,blksize,start,end,name,DMAstart
    MemType SYSRAM,250,B_USER,$1000,_RAMBase,_RAMMax,DRAMName,_RAMBase
    dc.l    0                           * end MemList

DRAMName:
    dc.b    "DRAM",0

    endm                                * CONFIG
    endc                                * _INITMOD

********************
* System State Cacheing Mode
*  Set system state cacheing mode to copyback (no Page tables)
*
 ifdef Config
Config set SSM_CMode
 endc

*****************************************************************************
*
* System tick timer configuration
*

TicksSec    equ 100
ClkVect     equ _TckVect
ClkPort     equ _TckBase
ClkPrior    equ 0

RTCPort     equ 0

*****************************************************************************
*
* SCF device descriptor configuration (GF TTY)
* 

* disable end-of-page pause
pagpause    equ OFF

* this is entirely pretend, as there is no flow control
HWSHAKE     equ ON

TERM macro
* console: port,vector,irq,priority,parity,baudcode,drivername
    SCFDesc _GFTTYBase,_GFTTYVect,_GFTTYLevel,$00,$00,$00,gftty
DevCon      set 0
    endm                                * TERM
