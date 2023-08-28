# QEMU-virt

`qemu-system-m68k -m virt`

## Status

WIP, but stalled due to a mystery crash in the 040 kernel.

 - 000/010 won't work with the virt peripherals as they are out of addressable range
 - 020/030 kernels use the CAAR register which is not emulated by QEMU
 - 060 throws a fatal illegal instruction exception in ROMBUG, for what seems to be an innocuous instruction:

```
<Called>
Exception Error, vector offset $0010 addr $0000181C
Fatal System Error; rebooting system
```
     181c:       2d48 2c80       movel %a0,%fp@(11392)
```

## Getting Started

TBD

### booting QEMU

Use the loader device to dump the ROM image into memory, and set the initial PC. Note that the initial PC is "magic" since the head of the ROM is actually a vector table, and QEMU doesn't seem to emulate the normal reset flow.

#### new config with ROM at zero
`qemu-system-m68k -M virt,memory-backend=foo.ram -m 8M -nographic -serial mon:stdio -object memory-backend-file,size=8M,id=foo.ram,mem-path=/tmp/foo.ram,share=on,prealloc=on -device loader,file=CMDS/BOOTOBJS/ROMBUG/romboot,addr=0x00000000,force-raw=on -device loader,addr=0x494,cpu-num=0`
