#!/usr/bin/env python3
#
# OS-9 memory image comprehension
#

from ctypes import *


class BadImageError(Exception):
    pass


def zstr(buffer):
    """read a nul-terminated string from buffer"""
    ret = ""
    index = 0
    while buffer[index] != 0:
        ret += chr(buffer[index])
        index += 1
    return ret


class FastDispatch(BigEndianStructure):
    _fields_ = [
        ("FstP_next",       c_uint32),          # next element in queue
        ("FstP_routine",    c_uint32),          # pointer to dispatched routine
        ("FstP_data",       c_uint32),          # pointer to static data
        ("FstP_status",     c_uint32),          # status information
    ]


class FreeBlock(BigEndianStructure):
    _fields_ = [
        ("M_NxtPtr",        c_uint32),          # ptr to next higher free block
        ("M_PrvPtr",        c_uint32),          # ptr to next lower free block
        ("M_BlkSiz",        c_uint32),          # size of this block
    ]


class ModuleHeader(BigEndianStructure):
    _fields_ = [
        ("M_ID",            c_uint16),          # ID code
        ("M_SysRev",        c_uint16),          # system revision check value
        ("M_Size",          c_uint32),          # module size
        ("M_Owner",         c_uint32),          # owner ID
        ("M_Name",          c_uint32),          # name offset
        ("M_Accs",          c_uint16),          # access permissions
        ("M_Type",          c_uint8),           # type
        ("M_Lang",          c_uint8),           # language
        ("M_Attr",          c_uint8),           # attributes
        ("M_Revs",          c_uint8),           # revision level
        ("M_Edit",          c_uint16),          # edition number
        ("M_Usage",         c_uint32),          # comment string offset
        ("M_Symbol",        c_uint32),          # symbol table offset
        ("M_Ident",         c_uint16),          # ident code
        ("",                c_uint16 * 3),      # reserved
        ("M_HdExt",         c_uint32),          # module header extension offset
        ("M_HdExtSz",       c_uint16),          # module header extension size
        ("M_Parity",        c_uint16),          # header parity
    ]
    id_code = 0x4afc
    sys_rev = 1

    module_types = {
        "MT_ANY":       0,
        "MT_PROGRAM":   1,
        "MT_SUBROUT":   2,
        "MT_MULTI":     3,
        "MT_DATA":      4,
        "MT_CSDDATA":   5,
        "MT_TRAPLIB":   11,
        "MT_SYSTEM":    12,
        "MT_FILEMAN":   13,
        "MT_DEVDRVR":   14,
        "MT_DEVDESC":   15,
        0:              "MT_ANY",
        1:              "MT_PROGRAM",
        2:              "MT_SUBROUT",
        3:              "MT_MULTI",
        4:              "MT_DATA",
        5:              "MT_CSDDATA",
        11:             "MT_TRAPLIB",
        12:             "MT_SYSTEM",
        13:             "MT_FILEMAN",
        14:             "MT_DEVDRVR",
        15:             "MT_DEVDESC",
    }

    @property
    def valid(self):
        return self.M_ID == self.id_code

    @property
    def type(self, code=None):
        if code is None:
            code = self.M_Type
        try:
            return self.module_types[code]
        except KeyError:
            return f"<unknown:{code}>"


class ExecModule(ModuleHeader):
    _fields_ = [
        ("M_Exec",          c_uint32),          # execution entry offset
        ("M_Excpt",         c_uint32),          # exception entry offset
        ("M_Mem",           c_uint32),          # data area requirement
        ("M_Stack",         c_uint32),          # stack area requirement
        ("M_IData",         c_uint32),          # initialized data ptr
        ("M_IRefs",         c_uint32),          # initialized reference lists ptr
        ("M_Init",          c_uint32),          # initialization execution offset
        ("M_Term",          c_uint32),          # termination execution offset
    ]


class DescriptorModule(ModuleHeader):
    _fields_ = [
        ("M_Port",          c_uint32),          # port address
        ("M_Vector",        c_uint8),           # hardware vector number
        ("M_IRQLvl",        c_uint8),           # Interrupt hardware priority level
        ("M_Prior",         c_uint8),           # interrupt (polling table) priority
        ("M_Mode",          c_uint8),           # device mode capabilities
        ("M_FMgr",          c_uint16),          # file manager name offset
        ("M_PDev",          c_uint16),          # device driver name offset
        ("M_DevCon",        c_uint16),          # offset of device dependent constants
        ("",                c_uint16),          # reserved
        ("M_DevFlags",      c_uint32),          # reserved for future usage
        ("",                c_uint16),          # reserved
        ("M_Opt",           c_uint16),          # device default option count
        ("M_DTyp",          c_uint8),           # device type
    ]

    descriptor_types = {
        "DT_SCF":   0,                          # sequential character file type
        "DT_RBF":   1,                          # random block file type
        "DT_Pipe":  2,                          # pipe file type
        "DT_SBF":   3,                          # sequential block file type
        "DT_NFM":   4,                          # network file type
        "DT_CDFM":  5,                          # compact disc file type
        "DT_UCM":   6,                          # user communications manager
        "DT_SOCK":  7,                          # socket communication manager
        "DT_PTTY":  8,                          # pseudo-keyboard manager
        "DT_INET":  9,                          # internet interface manager
        "DT_NRF":   10,                         # non-volatile ram file manager (CD-I variety)
        "DT_GFM":   11,                         # graphics file manager
        "DT_ISDN":  12,                         # ISDN file manager
        "DT_MPFM":  13,                         # MPEG file manager
        "DT_RTNFM": 14,                         # real time network file manager
        "DT_SPF":   15,                         # serial protocol file manager
        "DT_MFM":   16,                         # multimedia file manager (graphics & audio)
        "DT_DVM":   17,                         # device manager
        "DT_DVDFM": 18,                         # DVD file manager
        "DT_MODFM": 19,                         # Module I/O file manager
        "DT_NULL":  20,                         # DPIO Null File Manager
        "DT_PCF":   21,                         # (Reserved FUTURE: OS-9/68K PCF returns DT_RBF)

        0:          "DT_SCF",
        1:          "DT_RBF",
        2:          "DT_Pipe",
        3:          "DT_SBF",
        4:          "DT_NFM",
        5:          "DT_CDFM",
        6:          "DT_UCM",
        7:          "DT_SOCK",
        8:          "DT_PTTY",
        9:          "DT_INET",
        10:         "DT_NRF",
        11:         "DT_GFM",
        12:         "DT_ISDN",
        13:         "DT_MPFM",
        14:         "DT_RTNFM",
        15:         "DT_SPF",
        16:         "DT_MFM",
        17:         "DT_DVM",
        18:         "DT_DVDFM",
        19:         "DT_MODFM",
        20:         "DT_NULL",
        21:         "DT_PCF",
    }


class ConfigModule(ModuleHeader):
    _fields_ = [
        ("",                c_uint32),          # reserved
        ("M_PollSz",        c_uint16),          # number of entries in interrupt polling table
        ("M_DevCnt",        c_uint16),          # number of entries in device table
        ("M_Procs",         c_uint16),          # initial process table count
        ("M_Paths",         c_uint16),          # initial path table count
        ("M_SParam",        c_uint16),          # initial startup module parameter string
        ("M_Sysgo",         c_uint16),          # initial startup module name offset
        ("M_SysDev",        c_uint16),          # system device name
        ("M_Consol",        c_uint16),          # standard I/O pathlist name offset
        ("M_Extens",        c_uint16),          # customization module name offset
        ("M_Clock",         c_uint16),          # clock module name offset
        ("M_Slice",         c_uint16),          # clock ticks per time slice
        ("M_IPID",          c_uint16),          # interprocessor ID
        ("M_Site",          c_uint32),          # installation site code
        ("M_Instal",        c_uint16),          # installation name offset
        ("M_CPUTyp",        c_uint32),          # expected cpu type: 68000/68010/68020/etc.
        ("M_OS9Lvl",        c_uint8 * 4),       # operating system level/version/edition
        ("M_OS9Rev",        c_uint16),          # OS-9 level/revision string offset
        ("M_SysPri",        c_uint16),          # initial system priority
        ("M_MinPty",        c_uint16),          # initial system minimum executable priority
        ("M_MaxAge",        c_uint16),          # initial system maximum natural age
        ("M_MDirSz",        c_uint16),          # initial module directory count
        ("",                c_uint16),          # reserved
        ("M_Events",        c_uint16),          # initial system event table count
        ("M_Compat",        c_uint8),           # compatibility byte #1
        ("M_Compat2",       c_uint8),           # compatibility byte #2
        ("M_MemList",       c_uint16),          # offset to memory definitions (if any)
        ("M_IRQStk",        c_uint16),          # size of IRQ stack (in longwords)
        ("M_ColdTrys",      c_uint16),          # number of retries to attempt if initial chd fails
        ("",                c_uint16 * 2),      # reserved
        ("M_CacheList",     c_uint16),          # offset to SSM(MMU) cache modes (if any)
        ("M_IOMan",         c_uint16),          # offset to IOMan module name
        ("M_PreIO",         c_uint16),          # offset to "pre-IO" module list (called prior to M_Extens and M_IOMan entries)
        ("M_SysConf",       c_uint16),          # system configuration control flags
        ("M_NumSigs",       c_uint16),          # max number of queued signals (currently unimplemented)
        ("M_PrcDescStack",  c_uint16),          # process descriptor stack size
        ("",                c_uint16 * 32),     # reserved
    ]


class ModuleDirEntry(BigEndianStructure):
    _fields_ = [
        ("MD_MPtr",         c_uint32),          # module ptr
        ("MD_Group",        c_uint32),          # module group ptr
        ("MD_Static",       c_uint32),          # module group memory size
        ("MD_Link",         c_uint16),          # module link count
        ("MD_MChk",         c_uint16),          # module header checksum
    ]


class Globals(BigEndianStructure):
    _fields_ = [
        ("D_ID",            c_uint16),          # set to modsync code after coldstart has finished
        ("D_NoSleep",       c_uint16),          # set to non-zero to prevent sysproc from sleeping.
        ("",                c_uint16 * 14),     # reserved
        ("D_Init",          c_uint32),          # initialization module ptr ("init")
        ("D_Clock",         c_uint32),          # address of system tick routine
        ("D_TckSec",        c_uint16),          # number of ticks per second
        ("D_Year",          c_uint16),          # year
        ("D_Month",         c_uint8),           # month
        ("D_Day",           c_uint8),           # day
        ("D_Compat",        c_uint8),           # reserved for version compatability problems (#1)
        ("D_68881",         c_uint8),           # FPU Type (68020/030/040/060 systems)
        ("D_Julian",        c_uint32),          # julian day number
        ("D_Second",        c_uint32),          # system time: seconds left until midnight
        ("D_SysConf",       c_uint16),          # system configuration control
        ("D_IRQFlag",       c_uint8),           # IRQ flag
        ("D_UnkIRQ",        c_uint8),           # number of times unknown IRQ occurred in a row
        ("D_ModDir",        c_uint32 * 2),      # module directory (start & end ptrs)
        ("D_PrcDBT",        c_uint32),          # Process Descriptor Block Table ptr
        ("D_PthDBT",        c_uint32),          # Path Descriptor Block Table ptr
        ("D_Proc",          c_uint32),          # Current Process descriptor ptr
        ("D_SysPrc",        c_uint32),          # System Process Descriptor ptr
        ("D_Ticks",         c_uint32),          # ever-increment system tick
        ("D_FProc",         c_uint32),          # Process whose context is in FPU registers
        ("D_AbtStk",        c_uint32),          # System state bus trap panic abort sp, return pc
        ("D_SysStk",        c_uint32),          # System IRQ stack ptr
        ("D_SysROM",        c_uint32),          # Bootstrap ROM execution entry point
        ("D_ExcJmp",        c_uint32),          # Exception Jump Table ptr
        ("D_TotRAM",        c_uint32),          # total RAM found by BootROM at startup
        ("D_MinBlk",        c_uint32),          # process minimum allocatable block size
        ("D_FreMem",        c_uint32 * 2),      # system free memory list head
        ("D_BlkSiz",        c_uint32),          # system minimum allocatable block size
        ("D_DevTbl",        c_uint32),          # I/O Device Table ptr
        ("D_SpurIRQ",       c_uint32),          # spurious IRQ counter
        ("D_AutIRQ2",       c_uint32 * 7),      # 68070 on-chip I/O AutoVector Polling table heads
        ("D_VctIRQ",        c_uint32 * 192),    # Vectored Interrupt device tbl ptrs
        ("D_SysDis",        c_uint32),          # System Service dispatch table ptr
        ("D_UsrDis",        c_uint32),          # User Service dispatch table ptr
        ("D_ActivQ",        c_uint32 * 2),      # Active process queue head
        ("D_SleepQ",        c_uint32 * 2),      # Sleeping process queue head
        ("D_WaitQ",         c_uint32 * 2),      # Sleeping process queue head
        ("D_ActAge",        c_uint32),          # Active queue age
        ("D_MPUTyp",        c_uint32),          # MPU type (68000/68010/68020/68030/68070/68300/68040/68060)
        ("D_EvTbl",         c_uint32 * 2),      # ptr to system Event table start, end
        ("D_EvID",          c_uint32),          # next (incrementing) event ID number
        ("D_SPUMem",        c_uint32),          # ptr to SPU global variables (null == not enabled)
        ("D_AddrLim",       c_uint32),          # highest address found during startup
        ("D_Compat2",       c_uint8),           # cache compatibility/configuration flags
        ("D_SnoopD",        c_uint8),           # all data caches are coherent/snoopy (if non-zero)
        ("D_ProcSz",        c_uint16),          # size of a process descriptor
        ("D_PolTbl",        c_uint32 * 8),      # I/O AutoVector Polling table heads
        ("D_FreeMem",       c_uint32 * 2),      # head of system free memory color node list
        ("D_IPID",          c_uint16),          # interprocessor identification number
        ("",                c_uint16),          # reserved (interprocessor)
        ("D_CPUs",          c_uint32),          # ptr to array of cpu descriptor list heads
        ("D_IPCmd",         c_uint32 * 2),      # head of inter-processor command queue
        ("D_SProc",         c_uint32 * 210),    # reserved (old system process descriptor stub)
        ("D_CachMode",      c_uint32),          # 68020/68030/68040/68060 CACR mode
        ("D_DisInst",       c_uint32),          # instruction cache disable depth
        ("D_DisData",       c_uint32),          # data cache disable depth
        ("D_ClkMem",        c_uint32),          # ptr to clock tick thief's static storage
        ("D_Tick",          c_uint16),          # current tick
        ("D_TSlice",        c_uint16),          # ticks per slice
        ("D_Slice",         c_uint16),          # current time slice remaining (ticks)
        ("",                c_uint16),          # reserved
        ("D_Elapse",        c_uint32),          # time (ticks) to elapse before sys proc is awakened
        ("D_Thread",        c_uint32 * 2),      # system Thread queue head (immediate, or at absolute time)
        ("D_AlarTh",        c_uint32 * 2),      # system timed alarm threads (relative times)
        ("D_SStkLm",        c_uint32),          # System IRQ stack low bound
        ("D_Forks",         c_uint32),          # Number of actively forked processes.
        ("D_BootRAM",       c_uint32),          # Ram found during bootrom search (for integrity check)
        ("D_FPUSize",       c_uint32),          # FPU (max) state frame size
        ("D_FPUMem",        c_uint32),          # FPU Emulator global data
        ("D_IOGlob",        c_uint8 * 256),     # System Hardware dependent I/O flags
        ("D_DevSiz",        c_uint16),          # device table entry size (IOMan)
        ("D_MinPty",        c_uint16),          # system minimum process priority
        ("D_MaxAge",        c_uint16),          # system priority maximum age limit
        ("D_Sieze",         c_uint16),          # process ID of process that has siezed cpu
        ("D_Cigar",         c_uint32),          # gross estimate of sysglob, process, and module defs
        ("D_MMinLim",       c_uint32),          # former minimum memory address allocatable
        ("D_MMaxLim",       c_uint32),          # former maximum memory address allocatable (+1)
        ("D_MoveMin",       c_uint32),          # min count for DMA/move16 mem operations (move, clr)
        ("D_Preempt",       c_uint32),          # system-state pre-emption flag (0=allowed)
        ("D_FDispQ",        c_uint32),          # fast dispatch queue ptr
        ("D_FDisp",         c_uint32),          # address of insert "fast dispatch routine" routine
        ("D_ProfMem",       c_uint32),          # profiler (FasTrak) memory ptr
        ("",                c_uint32 * 6),      # reserved
        ("D_FIRQVct",       c_uint32),          # fast IRQ system routine/data ptr table
        ("D_VctJmp",        c_uint32),          # fast IRQ system vector/jump-target save table
        ("D_SysDbg",        c_uint32),          # system debugger entry pt address
        ("D_DbgMem",        c_uint32),          # system debugger memory ptr
        ("D_DbgFlg",        c_uint8),           # system debugger active flag
        ("D_AllocType",     c_uint8),           # memory allocator type
        ("D_DevCnt",        c_uint16),          # system device count
        ("D_Cache",         c_uint32),          # disk cache buffer head
        ("D_NumSigs",       c_uint16),          # default max signal depth
        ("D_PrcDescStack",  c_uint16),          # default process descriptor stack size
        ("D_FDispSys",      FastDispatch),      # system's fast dispatch block
        ("D_InHouse",       c_uint32 * 8),      # internal debugging
        ("D_KerTyp",        c_uint8),           # kernel type (development or atomic)
        ("D_InIRQ",         c_uint8),           # irq context flag
        ("D_FIRQOff",       c_uint8),           # FIRQ system - offset to stack frame
        ("D_IRQOff",        c_uint8),           # IRQ system - offset to stack frame
        ("D_IRQSPOff",      c_uint8),           # IRQ system - offset to (isp) stack ptr
        ("",                c_uint8 * 3),       # reserved
        ("D_MBAR",          c_uint32),          # CPU32 family Module Base Address
        ("D_Crystal",       c_uint32),          # system boot flags
        ("D_Idle",          c_uint32),          # idle loop call out routine
        ("D_IdleData",      c_uint32),          # idle loop call out routine data ptr
        ("D_Switches",      c_uint32),          # context switch counter for idle checks
        ("",                c_uint32 * 5),      # reserved
        ("D_IRQHeads",      c_uint32 * 8),      # irq head regions (for non-MSP kernels)
    ]


class OS9Module:
    def __init__(self, address, data):
        self.address = address
        self._data = data
        self._hdr = ModuleHeader.from_buffer_copy(data)
        if self._hdr.type == "MT_SYSTEM" and self.name == "init":
            self._hdr = ConfigModule.from_buffer_copy(data)
        elif self._hdr.type in ["MT_SYSTEM", "MT_PROGRAM", "MT_TRAPLIB", "MT_FILEMAN", "MT_DEVDRVR"]:
            self._hdr = ExecModule.from_buffer_copy(data)
        elif self._hdr.type in ["MT_DEVDESC"]:
            self._hdr = DescriptorModule.from_buffer_copy(data)

    @property
    def name(self):
        return zstr(self._data[self._hdr.M_Name:])

    @property
    def size(self):
        return self._hdr.M_Size

    @property
    def type(self):
        return self._hdr.type

    def search(self, str):
        addr = 0
        while True:
            addr = self._data.find(str, addr)
            if addr == -1:
                return
            yield addr
            addr += 1


class OS9Image:
    def __init__(self, image, base_address, vbr):
        self._data = image
        self._base = base_address
        self._vbr = vbr
        self._validate()

    def _contains(self, address, object_length=4):
        return (address >= self._base) and ((address + object_length) < (self._base + len(self._data)))

    def _validate(self):
        if len(self._data) <= 1024:
            raise BadImageError("memory image too small for vector table")
        if not self._contains(self._vbr, 1024):
            raise BadImageError("vector table outside memory image")

    def vector(self, index):
        """fetch an exception vector"""
        if index >= 256:
            raise KeyError("vector index must be < 256")

        class VectorTable(BigEndianStructure):
            _fields_ = [("vectors", c_uint32 * 256)]

        vt = VectorTable.from_buffer_copy(self._data, self._vbr - self._base)
        return vt.vectors[index]

    @property
    def globals(self):
        return self.struct_at(self.vector(0), Globals)

    def span(self, address, length):
        if not self._contains(address, length):
            raise KeyError(f"address/length {address:#x}/{length:#x} outside memory image")
        return self._data[address:address+length]

    def struct_at(self, address, struct_type):
        if not self._contains(address, sizeof(struct_type)):
            raise KeyError("structure outside memory image")
        return struct_type.from_buffer_copy(self._data[address - self._base:])

    def module_at(self, address):
        hdr = self.struct_at(address, ModuleHeader)
        if not self._contains(address, hdr.M_Size):
            raise KeyError("module outside memory image")
        return OS9Module(address, self._data[address:address+hdr.M_Size])

    def search(self, str):
        addr = 0
        while True:
            addr = self._data.find(str, addr)
            if addr == -1:
                return
            yield addr
            addr += 1

    @property
    def modules(self):
        addr = self.globals.D_ModDir[0]
        while addr < self.globals.D_ModDir[1]:
            entry = self.struct_at(addr, ModuleDirEntry)
            if entry.MD_MPtr != 0:
                yield self.module_at(entry.MD_MPtr)
            addr += sizeof(entry)


if __name__ == "__main__":
    import argparse
    from pathlib import Path

    def auto_int(x):
        return int(x, 0)

    parser = argparse.ArgumentParser(description="OS-9 memory image analyzer")
    parser.add_argument("image",
                        type=Path,
                        metavar="IMAGE-PATH",
                        help="path to memory image to analyze")
    parser.add_argument("--vbr",
                        type=auto_int,
                        metavar="VBR-VALUE",
                        default=0,
                        help="VBR register value")
    parser.add_argument("--base-address",
                        type=auto_int,
                        metavar="IMAGE-BASE",
                        default=0,
                        help="base address of the image")

    args = parser.parse_args()
    image = OS9Image(args.image.read_bytes(), args.base_address, args.vbr)
    g = image.globals
    print(f"Tick rate               {g.D_TckSec}Hz")
    print(f"Tick count              {g.D_Ticks}")
    print(f"IRQ nesting level       {g.D_IRQFlag}")
    print(f"Unknown IRQ count       {g.D_UnkIRQ}")
    print(f"Abort stack             {g.D_AbtStk:#010x}")
    print(f"Total RAM               {g.D_TotRAM:#010x}")
    print(f"Cache mode              {g.D_CachMode:#010x}")
    print(f"Current tick            {g.D_Tick}")
    print(f"Slice ticks             {g.D_TSlice}")
    print(f"Slice ticks remain      {g.D_Slice}")
    print(f"Context switches        {g.D_Switches}")
    print(f"System proc deadline    {g.D_Elapse}")
    print(f"Preemption disabled     {g.D_Preempt}")
    print(f"Idle callout            {g.D_Idle:#010x}({g.D_IdleData:#010x})")

    # these don't seem to match the actual stack in use
    # print(f"IRQ stack               {g.D_SysStk:#010x}")
    # print(f"IRQ stack limit         {g.D_SStkLm:#010x}")

    print("Modules:")
    for m in image.modules:
        print(f"    {m.address:#010x} {m.type:12} {m.name}")
    print(f"    {m.address+m.size:#010x}  <<end>>")

    for a in image.search(b"\x4A\x84\x67\x0A"):
        print(f"found at {a:#010x}")
