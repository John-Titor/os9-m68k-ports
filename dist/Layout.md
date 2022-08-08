# categories

## installable
	cmds - list of base commands
	libs - list of shared libraries
	cmds_net - list of networking commands
	cmds_obscure - list of obscure, not useful commands

# install search paths

68000/68010/68070 		: `OS9/68000/CMDS`
68020/68030/68040/68060 : `OS9/68020/CMDS`, `OS9/68000/CMDS`
CPU32 					: `OS9/CPU32/CMDS`, `OS9/68000/CMDS`

# BOOTOBJS
Continue to build bootfiles out of ports/, so ignore everything in bootobjs. Keep bootfiles
lean and get most stuff off the filesystem.

## SYS

### startup
Try for a generic startup

### autoload
Load 'common' non-shell-builtin commands


