#############################################################################
# Build the ROM board-specific library
#############################################################################

# disable object-file rules
-bo

SDIR		= .				# source search directory
RDIR		= ./RELS/$(TYPE)
CBOOTDEFS	= $(SRCROOT)/ROM/CBOOT/DEFS	# CBOOT defs
SCSIDEFS	= $(CDEFS)/IO/SCSI		# SCSI defines
MAKER		= rom_port.make			# this file
OLIB		= $(RDIR)/rom_port.l		# output library
OBJECTS		= $(RDIR)/syscon.r \
		  $(RDIR)/sysinit.r		# objects to build
SYSDEFS		= ../systype.d
RFLAGS		= -qb -u=. -u=.. -u=$(OSDEFS) -u=$(MACDIR) -a$(TYPE)

CFLAGS		+= -r				# disable stack checking
#CFLAGS		+= -t=$(TMP			# use system temporary directory
#CFLAGS		+= -O=0				# disable optimisations
CFLAGS		+= -v=.
CFLAGS		+= -v=..
CFLAGS		+= -v=$(CBOOTDEFS)
CFLAGS		+= -v=$(OSDEFS)
CFLAGS		+= -v=$(CDEFS)
CFLAGS		+= -v=$(SCSIDEFS)
CFLAGS		+= -d=$(TYPE)

build: $(RDIR) $(OLIB)

$(OLIB): $(OBJECTS)
	$(MERGE) -o=$@ $(OBJECTS)

$(OBJECTS): $(SYSDEFS) $(MAKER)

$(RDIR):
	@$(MD) $@

clean:
	$(RM) $(OBJECTS) $(OLIB)
