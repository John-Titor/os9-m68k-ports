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

SCFDSC		= term
SCFDSCR		= term.r

term: $(RDIR)/term.r $(SLIB)
	$(LC) $(LFLAGS) $(RDIR)/$*.r -O=$(ODIR)/$*
	$(TOUCH) $(FLAGFILE)

for TX in 1 2 3 4 5
SCFDSC		+= t$(TX)
SCFDSCR		+= r$(TX).r
CLEAN		+= $(ODIR)/t$(TX) $(RDIR)/t$(TX).r

t$(TX): $(RDIR)/t$(TX).r $(SLIB)
	$(LC) $(LFLAGS) $(RDIR)/$*.r -O=$(ODIR)/$*
	$(TOUCH) $(FLAGFILE)

t$(TX).r: $(SYSDEFS) $(MAKER)
endfor

build: $(RDIR) $(ODIR) $(SCFDSC)


$(SCFDSCR): $(SYSDEFS) $(MAKER)

$(ODIR) $(RDIR):
	@$(MD) $@

clean:
	$(RM) $(RDIR)/term.r $(ODIR)/term
	$(RM) $(CLEAN)
