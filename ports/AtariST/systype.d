*****************************************************************************
*****************************************************************************
*
* System Definitions for Atari Mega ST/4
*
* Names beginning with underscores are local to this file.
*

ATARIST         equ $1
CPUTyp          set 68010                   * sets the CPU type
CPUType         equ ATARIST                 * sets the board type

*****************************************************************************
*
* Memory constants
* 
* We have to avoid the 8B ROM remap at 8
*
_RAMBase        equ 0
_RAMSize        equ (4*1024*1024)           * 4MiB
_ROMBase        equ $00e00000               * ROM at natural location
_ROMSize        equ (512*1024)              * 512kiB

VTblSize        equ (256*4)                 * size of vector table
VBRBase         equ $8                      * base address of vectors

*
* Memory for memory lists; round up to avoid confusing things.
* 
* vectors.a consumes Mem.Beg, so need to keep this format.
*
Mem.Beg         equ $1000                   * exclude vector space
Mem.End         equ (_RAMBase+_RAMSize)
_SpcBeg         equ _ROMBase                * search ROM for modules
_SpcEnd         equ _ROMBase+_ROMSize


*
* Peripheral constants
*
_MFPBase        equ $00fffa01               * base address of the MFP
_MFPVect        equ $40                     * base vector for MFP
_MFPLevel       equ $6                      * IPL 6
_PortABase      equ _MFPBase

_TckBase        equ _MFPBase                * base address of MFP for Timer C
_TckVect        equ $45                     * timer C interrupt vector

*****************************************************************************
*
* ROM bootloader configuration
*

MemDefs macro
    dc.l        Mem.Beg,Mem.End             * regular memory list
    dc.l        0                           * ... terminator
    dc.l        _SpcBeg,_SpcEnd             * special / module search memory list
    dc.l        0                           * ... terminator
    endm

Cons_Adr        equ _MFPBase                * ROM console on MFP
Comm_Adr        equ 0                       * no aux port

*
* Options
*
MANUAL_RAM      set 1                       * RAM must be set up in SysInit
CBOOT           set 1                       * build with CBOOT
FIXED_CPUTYP    set 1                       * trust CPUTyp, don't probe
RAMVects        set 1                       * vectors in RAM
SysDisk         set 0                       * no boot disk
FDsk_Vct        set 0                       * no floppy drive

*****************************************************************************
*
* Init module configuration
*
    ifdef _INITMOD

SnoopExt        set 1                       * no external caches
Compat          set 0

CONFIG macro

* system / board name
MainFram:
    dc.b        "Atari ST",0

* startup module
SysStart:
    dc.b        "sysgo",0

* no statup module parameters
SysParam:
    dc.b        "",0

 ifdef ROMBOOT
* no root device for ROM boot
SysDev          equ 0
 else
* try to iniz a default drive
SysDev:
    dc.b        "/dd",0
 endc

* console terminal
ConsolNm:
    dc.b        "/term",0

* clock module name
ClockNm:
    dc.b        "tk_mfp_timerc",0

* ordered list of extensions
Extens:
    dc.b        "OS9P2 fpu",0

* configured memory (search) list
    align
MemList:
*               type,priority,attributes,blksize,start,end,name,DMAstart
    MemType     SYSRAM,250,B_USER,$1000,Mem.Beg,Mem.End,DRAMName,Mem.Beg
    dc.l        0                           * end MemList

DRAMName:
    dc.b        "STRAM",0

    endm                                    * CONFIG
    endc                                    * _INITMOD

*****************************************************************************
*
* System tick timer configuration
*

TicksSec        equ 100
ClkVect         equ _TckVect
ClkPort         equ _TckBase
ClkPrior        equ 0

*****************************************************************************
*
* SCF device descriptor configuration (MFP).
*
* Port always runs at 19200bps.
*

* disable end-of-page pause
pagpause        equ OFF

TERM macro
* console: port,vector,level,priority,parity,baudcode,drivername
    SCFDesc _MFPBase,_MFPVect,_MFPLevel,5,$00,$0f,sc_mfp_uart

DevCon dc.w 0

    endm                                * TERM

*****************************************************************************
*
* RBF device descriptor configuration (CompactFlash)
*

CF_Base     equ $00f00000

*****************************************************************************
*
* CFIDE driver build options
*
CFIDE_REG_ATARI     equ 1
CFIDE_DATA_WIDTH    equ 16
CFIDE_SWAP_BYTES    equ 0
