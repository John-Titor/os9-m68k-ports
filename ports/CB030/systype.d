*****************************************************************************
*****************************************************************************
*
* System Definitions for Plasmo's CB030 board.
*
* Names beginning with underscores are local to this file.
*

CB030       equ $cb030
CPUTyp      set 68030                   * sets the CPU type
CPUType     equ CB030                   * sets the board type

*****************************************************************************
*
* Memory constants
*
_RAMBase    equ 0                       * RAM starts at zero
_RAMMax     equ (16*1024*1024)          * 16MiB (XXX 64 or 128MiB also possible)
_ROMBase    equ $fe000000               * ROM at natural location
_ROMSize    equ (512*1024)              * 512kiB

VTblSize    equ (256*4)                 * size of vector table
VBRBase     equ 0                       * base address of vectors

*
* Peripheral constants
*
_DUARTBase  equ $fffff000               * base address of the 68681
_DUARTLevel equ 3                       * level 3
_DUARTVect  equ 80                      * ... vectored
_PortABase  equ _DUARTBase              * port A registers
_PortBBase  equ _DUARTBase + $10        * port B registers

_TckBase    equ $ffff9000               * turn-on address
_TckVect    equ 30                      * new CPLD, level 6 autovector
*_TckVect    equ 26                      * old CPLD, level 2 autovector

*****************************************************************************
*
* ROM bootloader configuration
*

*
* Memory
*
Mem.Beg     equ _RAMBase+$400           * exclude vector space
Mem.End     equ _RAMMax
Spc.Beg     equ _ROMBase                * search ROM for modules
Spc.End     equ _ROMBase+_ROMSize

MemDefs macro
    dc.l    Mem.Beg,Mem.End             * regular memory list
    dc.l    0                           * ... terminator
    dc.l    Spc.Beg,Spc.End             * special / module search memory list
    dc.l    0                           * ... terminator
    endm

Cons_Adr    equ _PortABase              * ROM console on port A
Comm_Adr    equ _PortBBase              * ROM aux on port B

*
* Options
*
MANUAL_RAM  set 1                       * RAM must be set up in SysInit
CBOOT       set 1                       * build with CBOOT
FIXED_CPUTYP set 1                      * trust CPUTyp, don't probe
RAMVects    set 1                       * vectors in RAM
SysDisk     set 0                       * no boot disk
FDsk_Vct    set 0                       * no floppy drive
FASTCONS    set 1                       * 19.2kbps console

*****************************************************************************
*
* Init module configuration
*
    ifdef _INITMOD

* no external cache, so caches are coherent
SnoopExt set 1

* no DMA devices
NoDataDis set 1

* lots of RAM, so have lots of interrupt stack
StackSz set $1000

CONFIG macro

* system / board name
MainFram:
    dc.b    "CB030",0

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
    dc.b    "tkcb030",0

* ordered list of extensions
Extens:
    dc.b    "OS9P2 syscache ssm fpu OS9P3",0

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

*****************************************************************************
*
* System tick timer configuration
*

TicksSec    equ 100
ClkVect     equ _TckVect
ClkPort     equ _TckBase
ClkPrior    equ 0

*****************************************************************************
*
* SCF device descriptor configuration (DUART).
* 
* Both ports default to 19.2kbps.
*

* disable end-of-page pause
pagpause    equ OFF

* this really should be a per-port option
HWSHAKE     equ ON

TERM macro
* console: port,vector,irq,priority,parity,baudcode,drivername
    SCFDesc _PortABase,_DUARTVect,_DUARTLevel,5,$00,$0f,sc68681

* port A/B share two bytes at this offset in OEM Global Data
DevCon dc.w 0

    endm                                * TERM

T1 macro
* aux serial: port,vector,irq,priority,parity,baudcode,drivername
    SCFDesc _PortBBase,_DUARTVect,_DUARTLevel,5,$00,$0f,sc68681

* port A/B share two bytes at this offset in OEM Global Data
DevCon dc.w 0

    endm                                * T1

*****************************************************************************
*
* RBF device descriptor configuration (CompactFlash)
*

CF_Base     equ $ffffe000

*****************************************************************************
*
* CFIDE driver build options
*
CFIDE_REG_COMPACT   equ 1
CFIDE_DATA_WIDTH    equ 8
CFIDE_SWAP_BYTES    equ 1

*****************************************************************************
*
* DS1302 RTC configuration
*

RTCPort     equ _DUARTBase

*****************************************************************************
*
* Board debug LEDs (wired to DAURT GPIO if RTC not present)
*
*  ##       b1
* #  #   b6    b2
*  ##       b7
*    #   b5    b3
*  ##       b4
*
DBGCtrl     equ _DUARTBase+$1a          * control register for debug LEDs
DBGIn       equ _DUARTBase+$1a          * debug input bits
DBGInBug    equ 4                       * do not enter debugger if 1
DBGSet      equ _DUARTBase+$1c          * set debug LED bits
DBGClr      equ _DUARTBase+$1e          * clear debug LED bits
DBG_9       equ $de                     * digit encodings
DBG_8       equ $fe
DBG_7       equ $0e
DBG_6       equ $fa
DBG_5       equ $da
DBG_4       equ $cc
DBG_3       equ $9e
DBG_2       equ $b6
DBG_1       equ $0c
DBG_0       equ $7e
