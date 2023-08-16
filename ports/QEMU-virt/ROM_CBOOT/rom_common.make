#############################################################################
# Build the common ROM sources
#############################################################################

# disable object-file rules
-bo

SDIR		= $(SRCROOT)/ROM/COMMON		# source search directory
RDIR		= ./RELS/$(TYPE)
MAKER		= rom_common.make		# this file
OLIB		= $(RDIR)/rom_common.l		# output library
OBJECTS		= $(RDIR)/vectors.r \
		  $(RDIR)/boot.r		# objects to build
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
