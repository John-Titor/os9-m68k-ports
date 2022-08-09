#############################################################################
# Build a ROM image for TYPE [and BOOTFILE]
#############################################################################
#
# ROM = image [+ bootfile]
#   image = rom_common.l + rom_port.l + rom_serial.l [+ rombug.l + romio.l] + LIBS
#     LIBS = LSYSBOOT + LCACHEFL + ROMDESC + CLIB + SYS_CLIB + MLIB + SYSL
# rom.bf = <files from bootlist, assembled by BOOTS/makefile>
#
# Factored heavily because os9make can only search one SDIR. Could probably do better
# with more explicit paths.
#
# rom_common.make: (vectors.a + boot.a) -> rom_common.l
# rom_port.make: sysinit.a + syscon.c -> rom_port.l
# rom_serial.make: (io68681.a) -> rom_serial.l
# rom_booter.make: rom_common.l + rom_port.l + rom_serial.l + (rombug.l + romio.l + syslibs) -> rom_booter
# rombug.make: rom_booter [+ <bootfile>.bf] -> romimage.[<bootfile>]
# 
# Pass TYPE= ROMBUG or NOBUG
# Pass BOOTFILE=<bootfilename> to add a bootfile
# 

# disable built-in rules
-b

RDIR		= ./RELS/$(TYPE)
ODIR		= ../CMDS/BOOTOBJS/$(TYPE)	# final product output path
BFDIR		= ../CMDS/BOOTOBJS/BOOTFILES 	# where to find bootfiles
MAKER		= ./rom_image.make		# this file
MAKERS		= rom_common.make \
		  rom_serial.make \
		  rom_port.make \
		  rom_booter.make		# sub-makefiles - order is important

FILES		= $(ODIR)/romboot		# ROM booter

if defined(BOOTFILE)
FILES		+= $(BFDIR)/$(BOOTFILE).bf	# optional bootfile
PROM		= $(ODIR)/romimage.$(BOOTFILE)	# final output file name
else
PROM		= $(ODIR)/romimage.no_bootfile	# final output file name
endif

build: $(ODIR) $(MAKER) $(PROM)

# build the PROM by merging $(FILES)
$(PROM): $(MAKER) $(FILES)
	$(MERGE) -O=$@ $(FILES)

# build the booter by invoking sub-makefiles
$(FILES): $(MAKERS) .

# invoke individual sub-makefiles
$(MAKERS): .
	@echo ~~~ $@
	$(MAKE) -f=$@ TYPE=$(TYPE) $(GOAL)

$(ODIR):
	@$(MD) $@

clean: $(MAKERS) .
	$(RM) $(PROM) $(FILES)
