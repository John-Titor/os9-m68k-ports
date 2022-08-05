#############################################################################
# SCF descriptor modules
#############################################################################

# disable object file rules as we call the linker directly
-bo

SDIR		= $(SRCROOT)/IO/SCF/DESC	# SCF descriptor sources
ODIR		= ../CMDS/BOOTOBJS
RDIR		= ./RELS
MAKER		= ./scf_descriptors.make	# this file
FLAGFILE	= $(ODIR)/.updated
SYSDEFS		= ../systype.d
RFLAGS		= -q -u=. -u=$(OSDEFS)
SLIB		= $(SYSRELS)/sys.l
PERM		= -p=577			# W:er, GO:ewr module permissions
LFLAGS		= -l=$(SLIB) -gu=0.0 $(PERM)

SCFDSC		= term   t1
SCFDSCR		= term.r t1.r

build: $(RDIR) $(ODIR) $(SCFDSC)

term: $(RDIR)/term.r $(SLIB)
	$(LC) $(LFLAGS) $(RDIR)/$*.r -O=$(ODIR)/$*
	$(TOUCH) $(FLAGFILE)

t1: $(RDIR)/t1.r $(SLIB)
	$(LC) $(LFLAGS) $(RDIR)/$*.r -O=$(ODIR)/$*
	$(TOUCH) $(FLAGFILE)

$(SCFDSCR): $(SYSDEFS) $(MAKER)

$(ODIR) $(RDIR):
	@$(MD) $@

clean:
	$(RM) $(RDIR)/term.r $(ODIR)/term
	$(RM) $(RDIR)/t1.r $(ODIR)/t1
