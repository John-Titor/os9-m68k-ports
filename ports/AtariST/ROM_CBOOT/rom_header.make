#############################################################################
# Build the Atari ROM header
#############################################################################

# disable object-file rules
-bo

SDIR		= .				# source search directory
RDIR		= ./RELS/$(TYPE)
MAKER		= rom_header.make		# this file
OLIB		= $(RDIR)/rom_header.l		# output library
OBJECTS		= $(RDIR)/header.r
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
