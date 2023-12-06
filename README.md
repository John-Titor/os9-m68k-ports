# OS-9 for 68K ports

A small collection of OS-9 ports to various 68K systems.

    /apps           OS-9 applications
        /bin        binaries
        /src        sources
    /dist           OS media builder
        /archives   destination for distribution archives
        /filesets   media metadata
        /SYS        files to go in /SYS on the target
    /ports          port source trees for various targets
        /AtariST    port to the Hatari ST emulator
        /CB030      https://www.retrobrewcomputers.org/doku.php?id=builderpages:plasmo:cb030
    /tools          prebuilt host tools

----

## Building

The ports are developed on macOS, but should be easily built on x86 Linux or
Windows systems.

### macOS / Linux

Building requires Wine (to run the compiler and tools). Any recent version
should do. Apple Silicon Macs require Crossover 21.0.0 or later, available 
from Homebrew as `wine-crossover`. See https://github.com/Gcenx/homebrew-wine/
for more details. Wine on Linux requires an x86 system.

### Windows

No additional tooling is required.

### Installing the SDK

Ports are developed using the OS-9 for 68K SDK v1.2. v1.3 should work, but
has not been tested due to not being generally available.

Unpack the OS-9 for 68K SDK in a convenient location. It should be on a path
with no spaces; by default the build system expects it to be placed in
`M:\MWOS`.

Configure a drive alias if necessary. Wine users should add a symlink in 
`$(WINEPREFIX)/dosdevices` to map drive `M:` to the path where the SDK is
installed; this can be achieved using `winecfg` under the Drives tab.

### Build / Clean

At the top of the port tree (`/ports`) are two scripts. `make.bat` invokes the
`os9make` utility from the SDK and can be called directly on a Windows system.
macOS and Linux users should use `make.sh`, which wraps `make.bat` with Wine.

Change directory to the port you intend to build, and invoke `../make.sh build`
or `..\make.bat build` as appropriate. The `MWOS` environment variable must be
set to the Windows path where the SDK was installed if it is not in `M:\MWOS`.
Note that the makefiles are not tolerant of paths with spaces in them, and some
of the OS-9 tools have odd restrictions on legal path characters, file and
directory name lengths, etc, so using any other path should be considered
experimental.

Clean the port workspace with `../make.sh clean` or `..\make.bat clean`. If you
suspect that the build system is not picking up a change you've made, cleaning
will force a complete rebuild next time around.

### Build products

Ports will generally produce ROM images in `CMDS/BOOTOBJS/ROMBUG` with the
ROM debugger, and `CMDS/BOOTOBJS/NOBUG` without. These images contain the ROM
bootloader, and may also contain the debugger (ROMBUG) and a bootfile for
direct booting from ROM. 

Additional bootfiles may be produced in `CMDS/BOOTOBJS/BOOTFILES`. See the
per-port documentation for details regarding bootfiles and descriptors that
may be produced.

----

## Installing to disk

During the installation process you will prepare a new disk (or CompactFlash
card), upload the OS-9 commands and datafiles, and prepare the disk for 
booting.

### Preparation

You will need a terminal emulation program that supports the Kermit protocol.
There are many; Mincom for Linux or macOS systems, TeraTerm for Windows for
example.

Create the distribution archive by running the `makedist.bat` or `makedist.sh`
script in the `/dist` directory.

Windows:

    > cd dist
    > .\makedist.bat

Linux/macOS:

    $ cd dist
    $ ./makedist.sh

This will prepare the distribution archives in `dist/archives/`.

Build the port that you will be installing, and check `CMDS/BOOTOBJS/BOOTFILES`
to see whether a `diskboot.bf` file has been generated. If it does, your port
can boot from disk; if not, it boots from ROM. Follow the respective directions
below.

#### Boot-from-disk ports

To boot from disk, you will flash `/CMDS/BOOTOBJS/NOBUG/romimage.diskboot` and
install `/CMDS/BOOTOBJS/BOOTFILES/diskboot.bf` as a bootfile.

To boot from ROM, follow the instructions below.

#### Boot-from-ROM ports

To boot from ROM, you will flash `/CMDS/BOOTOBJS/NOBUG/romimage.diskboot`. This
image includes the bootfile, so you won't install one to the disk and should
ignore the bootfile installation step below.

### Installing

Connect the drive and boot your board. In the examples below we will be
installing to CompactFlash on a CB030 board. Here, the format-enabled
descriptor for the CompactFlash is `c0_fmt`. Check the documentation for the
port you are installing for the correct descriptor for your target board.

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

Prepare to upload the CMDS files:

    $ makdir CMDS
    $ chd CMDS
    $ kermit -ri

and send `dist/archives/osk_cmds.zip` using the Kermit protocol with your
terminal program. At 19200bps this will take a little over ten minutes.
When the upload completes, unpack the archive, fix permissions, set the
execution directory so the commands are now available, and clean up:

    $ unzip osk_cmds.zip
    ...
    $ attr -nw -npw -e -pe -r -pr *
    $ attr -w osk_cmds.zip
    $ chx /dd/CMDS
    $ del osk_cmds.zip

Prepare to upload the SYS files:

    $ chd ..
    $ makdir SYS
    $ chd SYS
    $ kermit -ri

and send `dist/archives/osk_sys.zip`. This will take about a minute. When the
upload completes, unpack the archive. Note the use of the `-a` option to convert
text file line endings to OS-9 format:

    $ unzip -a osk_sys.zip
    ...
    $ del osk_sys.zip

If you are following the boot-from-disk flow, prepare to upload the bootfile:

    $ chd /c0_fmt
    $ kermit -ri

and send `ports/<port>/CMDS/BOOTOBJS/BOOTFILES/diskboot.bf`. This should take
about five minutes. When the upload completes, install the bootfile:

    $ os9gen -e /c0_fmt -q=diskboot.bf

At this point installation is complete and the system is now ready to boot from
the disk.

----

## Ported applications and libraries

Work in progress under `/apps` and `/libs`.

Non-trivial apps and libraries are tracked as submodules. Use `git submodule
update --init` the first time you clone this repository in order to prepare it
for submodules. Then use `git submodule update --remote` to fetch the most
recent versions of the submodules.

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

The OS-9 RBF API uses 24-bit LSNs (LBAs), meaning that disks are limited
to 4GiB (larger disks can be connected, but the extra capacity cannot be used).
Additionally, the on-disk format can only support 524,280 allocation clusters
which limits the total number of files that can be created.

### os9make

The tool can be picky and challenging at times. Some specific notes:

 - Timestamp precision is not good (actually a DOS/Windows issue). This means
   that often products get re-built unnecessarily.

 - Calls the wrong compiler or linker sometimes. Occasionally you may find
   that it's trying to run `ucc` instead of `xcc`, or `xcc` when it should be
   running `l68`.

 - Sources can only be searched for implicitly in one directory `$(SDIR)`.
   Likewise object files can only be searched for in `$(RDIR)`.

 - Targets like 'clean:' are treated as binaries rather than phony utility
   targets. It's common to use `-b` or `-bo` and explicit rules rather than
   the implicit rules.

----

## Possible ports

### QEMU-virt

Possibly the fastest m68k emulator, with a very simple virtual machine model.
Some issues with the CPU emulation may make this difficult:

 - Virtual peripherals are all above 0xff000000, meaning that the 68000/010
   cannot access them.
 - The CAAR is not emulated making the 020/030 kernels un-usable.
 - There is an issue with the 040 kernel that causes it to crash shortly after
   the first timer tick, but the exact issue is not yet understood.
 - 060 emulation has not yet been tested, but as it's very similar to the 040
   it's likely to have the same issue.

### Mini RoboMind

MC68332 in a very compact form-factor.

https://robominds.com

----

## Reporting issues

Please file issues for any problems found. Pull requests are also welcome.
