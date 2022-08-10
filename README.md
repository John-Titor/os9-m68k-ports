# OS-9 for 68K ports

A small collection of OS-9 ports to various 68K systems.

## Ports

### CB030

A simple 68030 board with lots of RAM. 

https://www.retrobrewcomputers.org/doku.php?id=builderpages:plasmo:cb030

#### Status

Mostly functional. Boots from CompactFlash or ROM image.

    /term    DUART port A
    /t1      DUART port B
    /dd      aliased to /c0
    /c0      CompactFlash
    /c0_fmt  CompactFlash (formattable)

#### TODO

 - [ ] Realtime clock driver (work in progress)

## Building

The ports are developed on macOS, but should be easily built on Intel Linux or
Windows systems.

### macOS / Linux

Building requires Wine (to run the compiler and tools). Any recent version
should do. Apple Silicon Macs require Crossover 21.0.0, available from
Homebrew as `wine-crossover`; see https://github.com/Gcenx/homebrew-wine/.

### Windows

No additional tooling is required.

### Installing the SDK

Ports are developed using the OS-9 for 68K SDK v1.2.

Unpack the OS-9 for 68K SDK in a convenient location. It should be on a path
with no spaces; by default the build system expects it to be placed in
`M:\MWOS`.

Configure a drive alias if necessary. Wine users should add a symlink in $
(WINEPREFIX)/dosdevices to map drive `M:` to the path where the SDK is
installed; this can be achieved using `winecfg` under the Drives tab.

### Build / Clean

At the top of each port tree (`ports/<portname>`) are two scripts. `make.bat`
invokes the `os9make` utility from the SDK and can be called directly on a
Windows system. macOS and Linux users should use `make.sh`, which wraps
`make.bat` with Wine.

Build with `./make.sh build` or `.\make.bat build` respectively. The `MWOS`
environment variable must be set to the Windows path where the SDK was
installed if it is not in `M:\MWOS`. Note that the makefiles are not tolerant
of paths with spaces in them, and some of the OS-9 tools have odd
restrictions on legal path characters, file and directory name lengths, etc,
so using any other path should be considered experimental.

Clean with `./make.sh clean` or `.\make.bat clean`.

### Build products

Ports will generally produce a ROM image in `CMDS/BOOTOBJS/ROMBUG` with the
ROM debugger, and `CMDS/BOOTOBJS/NOBUG` without. These images contain the ROM
bootloader, and may also contain a bootfile for direct booting from ROM.

Additional bootfiles may be produced in `CMDS/BOOTOBJS/BOOTFILES`. See the
per-port documentation for details regarding bootfiles and descriptors that
may be produced.

### Building disk images

Work in progress under `/dist`.

### Notes and FAQs

Port layouts generally follow the layout of the examples in the SDK, with
various simplifications and changes where appropriate or necessary.

    BOOTFILE        scripts to build bootfiles
    CMDS            system-specific utilities
      /BOOTOBJS     system, driver and descriptor modules
        /BOOTFILES  bootfiles (*.bf)
        /INITS      init modules
        /ROMBUG     ROM bootloader and ROM image(s) with rombug
        /NOBUG      ROM bootloader and RON image(s) without rombug
    INIT            scripts to build init modules
    RBF             block storage driver and descriptor sources
    ROM_CBOOT       ROM bootloader and build scripts
    SCF             serial driver and descriptor sources
    SYSMOD          system modules (ticker, RTC, etc.) and build scripts

#### Disk support

OS-9 RBF is limited to 24-bit LSNs (LBAs) which limits disks to 8GiB
(though some comments suggest a 23-bit limit). Additionally, the format is
limited to 524,280 allocation units (clusters), which limits the total number
of files.

#### `os9make`

The tool can be picky and challenging at times. Some specific notes:

 - Timestamp precision is not good (actually a DOS/Windows issue). This means
   that sometimes products get re-built unnecessarily.

 - Calls the wrong compiler or linker sometimes. Occasionally you may find
   that it's trying to run `ucc` instead of `xcc`, or `xcc` when it should be
   running `l68`.

 - Implicit rules can only have one search directory (`SDIR`).

## Planned ports

### P90MB

Philips P90CE201 with RC2014 slots.

https://www.retrobrewcomputers.org/doku.php?id=builderpages:plasmo:p90mb

### Mini RoboMind

MC68332 in a very compact form-factor.

https://robominds.com



## 