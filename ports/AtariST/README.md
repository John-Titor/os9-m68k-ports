# Atari Mega ST4 with 68010

As emulated by Hatari.

## Status

Very raw work in progress.

 - the 'dev' romimage is working
 - the CFIDE driver can be used to format an emulated IDE drive
 - currently no way to get a system image onto an emulated drive

See TODO for details.

## Port Notes

### Realtime clock

Non-Mega STs do not have a realtime clock.

### CPU type

The ST ROM's low 2 words (initial stack pointer and reset vector respectively)
are mapped to 0x0000_0000 by hardware. TOS ROMs contain version information in
the second word (stack pointer), but for the 68000 OS-9 uses this value to
locate system variables. The Hatari ROM loader checks the TOS version in the
ROM and has strong opinions about what values it may have, which preclude using
it in the way that OS-9 requires.

ROM_CBOOT prepends a minimal TOS ROM header to the image which satisfies
Hatari. The 68010 and up kernels use VBR to locate the initial stack pointer
and system variables, which allows the vector table to be placed after the
header, thus avoiding the conflict.

### Horizontal / Vertical blanking interrupts

The ST generates an interrupt at the start of each of the horizontal and
vertical blanking intervals; the HBL interrupts arrive every ~60µs depending
on the video mode. This is far too fast for the OS-9 kernel to cope, and there
is no convenient way to mask these. Normally you would run the ST at a higher
IPL if you wanted to ignore these interrupts, but the OS-9 kernel does not
support this.

To work around the issue, the bootloader does not unmask interrupts while it
is running, and patches vectors 26 and 28 (HBL and VBL respectively) to point
to a stub handler before handing off to the kernel.

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
