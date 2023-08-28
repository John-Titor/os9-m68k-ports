# Atari ST Mega/4

As emulated by Hatari.

## Status

Very raw work in progress.

 - the 'dev' romimage is working
 - the CFIDE driver can be used to format an emulated IDE drive
 - currently no way to get a system image onto an emulated drive

 See TODO for more details.

## Getting Started

Clean:
`env -i HOME=${HOME} WINEDEBUG=-all `which wine` cmd /c ..\\make.bat clean`

Build:
`env -i HOME=${HOME} WINEDEBUG=-all `which wine` cmd /c ..\\make.bat`

Console (persistent, start first and leave running):
`socat  PTY,link=/tmp/pseudo-serial open:/dev/tty,rawer`

Emulator:
`hatari --configfile hatari.cfg --confirm-quit FALSE --tos CMDS/BOOTOBJS/ROMBUG/romimage.dev`

With IDE disk:
`dd if=/dev/zero of=/tmp/hd.bin bs=1m count=64`
`hatari --configfile hatari.cfg --confirm-quit FALSE --tos CMDS/BOOTOBJS/ROMBUG/romimage.dev --ide-master /tmp/hd.bin`

Instruction tracing:
`hatari --configfile hatari.cfg --confirm-quit FALSE --tos CMDS/BOOTOBJS/ROMBUG/romboot --trace cpu_all,cpu_regs --trace-file /tmp/trace.txt`
