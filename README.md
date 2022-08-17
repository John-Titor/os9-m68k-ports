# OS-9 for 68K ports

A small collection of OS-9 ports to various 68K systems.

    /apps           OS-9 applications
        /bin        binaries
        /src        sources
    /dist           OS media builder
        /filesets   media metadata
    /ports          port source trees for various targets
    /tools          prebuilt host tools

----

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

#### Getting Started

Flash `ports/CB030/CMDS/BOOTOBJS/ROMBUG/romimage.dev` to the EEPROM, and
connect to serial A at 9600bps. Power the board on, and after a few seconds
OS-9 should load the bootfile from ROM and present the mshell prompt:

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
be used with a CF without reflashing.

Other images include:
`ports/CB030/CMDS/BOOTOBJS/ROMBUG/romimage.no_bootfile`: has RomBug but 
no bootfile; can only boot from CompactFlash.

`ports/CB030/CMDS/BOOTOBJS/NOBUG/romimage.diskboot`: has no RomBug and 
a minimal bootfile for repairing disks.

`ports/CB030/CMDS/BOOTOBJS/NOBUG/romimage.no_bootfile`: has no RomBug and
no bootfile; can only boot from CompactFlash.

----

## Building

The ports are developed on macOS, but should be easily built on Intel Linux or
Windows systems.

### macOS / Linux

Building requires Wine (to run the compiler and tools). Any recent version
should do. Apple Silicon Macs require Crossover 21.0.0, available from
Homebrew as `wine-crossover`. See https://github.com/Gcenx/homebrew-wine/
for more details.

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

----

## Installing to disk

During the installation process you will prepare a new disk (or CompactFlash
card), upload the OS-9 commands and datafiles, and prepare the disk for 
booting.

### Preparation

Create the distribution archive by running the `makedist.bat` or `makedist.sh`
script in the `dist` directory.

Windows:

    > cd dist
    > .\makedist.bat

Linux/macOS:

    $ cd dist
    $ ./makedist.sh

This will prepare the distribution archives in `dist/archives/`.

Build the port that you will be targeting, and ensure that the files

    ports/<port>/CMDS/BOOTOBJS/BOOTFILES/diskboot.bf
    ports/<port>/CMDS/BOOTOBJS/NOBUG/romimage.diskboot

have been created. Flash `romimage.diskboot` to your board (or ensure that
the modules listed for it are present in your current ROM).

### Installing

Connect the drive and boot your board. In the examples below we will be 
installing to CompactFlash on a CB030 board. Here, the format-enabled
descriptor for the CompactFlash is `c0_fmt`. Check the section above for the
correct descriptor for your target board.

If your terminal program does not support the ZModem protocol, replace `z` in
the examples below with `xy` and use XModem or YModem instead.

First, disable screen paging:

    $ tmode nopause

Use the `format` command to format the disk. This will typically require use of
the format-enabled descriptor; e.g. for CB030:

    $ format c0_fmt
    ...
    Formatting device:  c0_fmt
    proceed?  y
    this is a HARD disk - are you sure? y
    physical format desired?  n
    physical verify desired?  n
    volume name:  boot
    building media bitmap...
    ...
    writing root directory structure
    $

Now change data directory to the formatted device:

    $ chd /c0_fmt

Note that using `/dd` here will cause problems later when you attempt to install
the bootfiles.

First, prepare to upload the SYS files:

    $ makdir SYS
    $ chd SYS
    $ z -b
    Receiving...
    **B0100000027fed4

and send `dist/archives/osk_sys.zip` with your terminal program. At 9600bps
this will take about a minute. When the upload completes, unpack the archive.
Note the use of the `-a` option to convert text file line endings to OS-9
format:

    $ unzip -a osk_sys.zip
    ...

Next prepare to upload the CMDS files:

    $ chd ..
    $ makdir CMDS
    $ chd CMDS
    $ z -b
    Receiving...
    **B0100000027fed4

and send `dist/archives/osk_cmds.zip`. This will take about 13 minutes. When
the upload completes, unpack the archive, fix permissions, set the execution
directory so the commands are now available, and clean up:

    $ unzip osk_cmds.zip
    ...
    $ attr -nw -npw -e -pe -r -pr *
    $ attr -w osk_cmds.zip
    $ chx /dd/CMDS
    $ del osk_cmds.zip

Next, prepare to upload the bootfile:

    $ chd /c0_fmt
    $ z -b
    Receiving...
    **B0100000027fed4

and send `ports/<port>/CMDS/BOOTOBJS/BOOTFILES/diskboot.bf`.

When the upload completes, install the bootfile:

    $ os9gen -e /c0_fmt -q=diskboot.bf

Now we can create a startup file and clean up in `/SYS`:

    $ chd /dd/SYS
    $ build startup
    ? chd /dd
    ? chx /dd/CMDS
    ?
    $

At this point installation is complete and the system is now ready to boot from
the disk.

----

## Ported applications and libraries

Work in progress under `/apps` and `/libs`.

Non-trivial apps and libraries are tracked as submodules. Use 
`git submodule update --init` the first time you clone this repository in
order to prepare it for submodules. Then use `git submodule update --remote`
to fetch the most recent versions of the submodules.

----

## Notes and FAQs

Port layouts generally follow the layout of the examples in the SDK, with
various simplifications and changes where appropriate or necessary.

    BOOTFILE        scripts to build bootfiles
    CMDS            build system output
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

### Disk support

OS-9 RBF is limited to 24-bit LSNs (LBAs) which limits disks to 8GiB
(though some comments suggest a 23-bit limit). Additionally, the format is
limited to 524,280 allocation units (clusters), which limits the total number
of files.

### os9make

The tool can be picky and challenging at times. Some specific notes:

 - Timestamp precision is not good (actually a DOS/Windows issue). This means
   that often products get re-built unnecessarily.

 - Calls the wrong compiler or linker sometimes. Occasionally you may find
   that it's trying to run `ucc` instead of `xcc`, or `xcc` when it should be
   running `l68`.

 - Sources can only be searched for implicitly in one directory (`SDIR`).

 - Targets like 'clean:' are treated as binaries rather than phony utility 
   targets. It's common to use `-b` or `-bo` and explicit rules rather than
   the implicit rules.

----

## Planned ports

### P90MB

Philips P90CE201 with RC2014 slots.

https://www.retrobrewcomputers.org/doku.php?id=builderpages:plasmo:p90mb

### Mini RoboMind

MC68332 in a very compact form-factor.

https://robominds.com

----

## Reporting issues

Please file issues for any problems found. Pull requests are also welcome.
