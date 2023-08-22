#############################################################################
# Build the ROM serial driver
#############################################################################

# disable object-file rules
-bo

SDIR		= .				# source search directory
RDIR		= ./RELS/$(TYPE)
MAKER		= ./rom_serial.make		# this file
OLIB		= $(RDIR)/rom_serial.l		# output library
OBJECTS		= $(RDIR)/io_mfp_uart.r		# objects to build
SYSDEFS		= ../systype.d
RFLAGS		= -qb -u=. -u=.. -u=$(OSDEFS) -u=$(MACDIR) -a$(TYPE)

build: $(RDIR) $(OLIB)

$(OLIB): $(OBJECTS)
	$(MERGE) -o=$@ $(OBJECTS)

$(OBJECTS): $(SYSDEFS) $(MAKER)

$(RDIR):
	@$(MD) $@

clean:
	$(RM) $(OBJECTS) $(OLIB)
