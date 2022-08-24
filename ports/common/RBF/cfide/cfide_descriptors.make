#############################################################################
# RBF descriptor modules
#############################################################################

# disable implicit rules as we call the assembler and linker directly
-bo

SDIR		= $(SRCROOT)/IO/RBF/DESC	# RBF descriptor sources
ODIR		= ../CMDS/BOOTOBJS
RDIR		= ./RELS
MAKER		= $(CFIDE)/cfide_descriptors.make	# this file
FLAGFILE	= $(ODIR)/.updated
SYSDEFS		= ../systype.d
RFLAGS		= -q -u=. -u=$(OSDEFS) -u=$(SDIR)
SLIB		= $(SYSRELS)/sys.l
LFLAGS		= -l=$(SLIB) -gu=0.0

DESC		= c0
DESCSRC		= $(CFIDE)/$(DESC).a
DESCMOD		= $(ODIR)/$(DESC)
DESCREL		= $(RDIR)/$(DESC).r
DESCMOD_FMT	= $(ODIR)/$(DESC)_fmt
DESCREL_FMT	= $(RDIR)/$(DESC)_fmt.r
DDMOD		= $(ODIR)/dd

build: $(RDIR) $(ODIR) $(DESCMOD) $(DESCMOD_FMT) $(DDMOD)

$(DESCMOD): $(DESCREL) $(SLIB)
	$(LC) $(LFLAGS) $(DESCREL) -O=$@
	$(TOUCH) $(FLAGFILE)

$(DESCREL): $(DESCSRC) $(SYSDEFS) $(MAKER)
	$(RC) $(RFLAGS) $(DESCSRC) -O=$@

$(DESCMOD_FMT): $(DESCREL_FMT) $(SLIB)
	$(LC) $(LFLAGS) $(DESCREL_FMT) -O=$@
	$(TOUCH) $(FLAGFILE)

$(DESCREL_FMT): $(DESCSRC) $(SYSDEFS) $(MAKER)
	$(RC) $(RFLAGS) -aFMT_ENABLE $(DESCSRC) -O=$@

$(DDMOD): $(DESCREL) $(SLIB)
	$(LC) $(LFLAGS) $(DESCREL) -O=$@ -n=dd
	$(TOUCH) $(FLAGFILE)

$(ODIR) $(RDIR):
	@$(MD) $@

clean:
	$(RM) $(DESCMOD) $(DESCREL) $(DESCMOD_FMT) $(DESCREL_FMT) $(DDMOD)
