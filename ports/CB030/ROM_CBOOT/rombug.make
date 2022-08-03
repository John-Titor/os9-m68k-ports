#############################################################################
# Build a ROM with ROMBUG
#############################################################################
#
# ROM = image + bootfile
#   image = rom_common.l + rom_port.l + rom_serial.l + rombug.l + romio.l + LIBS
#     LIBS = LSYSBOOT + LCACHEFL + ROMDESC + CLIB + SYS_CLIB + MLIB + SYSL
# rom.bf = <files from bootlist, assembled by BOOTS/makefile>
#
# Factored heavily because os9make can only search one SDIR. Could probably do better
# with more explicit paths.
#
# rom_common.make: (vectors.a + boot.a) -> rom_common.l
# rom_port.make: sysinit.a + syscon.c -> rom_port.l
# rom_serial.make: (io68681.a) -> rom_serial.l
# rom_image.make: rom_common.l + rom_port.l + rom_serial.l + (rombug.l + romio.l + syslibs) -> rom_booter
# rombug.make: rom_booter + rom.bf -> romimage.rombug
# 

# disable built-in rules
-b

RDIR		= ./RELS/ROMBUG
ODIR		= ../CMDS/BOOTOBJS/ROMBUG	# final product output path
BFDIR		= ../CMDS/BOOTOBJS/BOOTFILES 	# where to find bootfiles
MAKER		= ./rombug.make			# this file
MAKERS		= rom_common.make \
		  rom_serial.make \
		  rom_port.make \
		  rom_image.make		# sub-makefiles - order is important

PROM		= $(ODIR)/romimage.rombug	# final output file name
FILES		= $(ODIR)/romboot \
		  $(BFDIR)/rom.bf		# additional inputs

build: $(ODIR) $(MAKER) $(PROM)

# build the PROM by merging $(FILES)
$(PROM): $(MAKER) $(FILES)
	$(MERGE) -O=$@ $(FILES)

# build input files by invoking sub-makefiles
$(FILES): $(MAKERS) .

# invoke individual sub-makefiles
$(MAKERS): .
	@echo ~~~ $@
	$(MAKE) -f=$@ TYPE=ROMBUG $(GOAL)

$(ODIR):
	@$(MD) $@

clean: $(MAKERS) .
	$(RM) $(PROM) $(FILES)
