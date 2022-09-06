#############################################################################
# SCF sc68681 driver
#############################################################################

# disable object file rules as we call the linker directly
-bo

SDIR		= $(SRCROOT)/IO/SCF/DRVR	# SCF driver sources
ODIR		= ../CMDS/BOOTOBJS
RDIR		= ./RELS
MAKER		= ./scf_sc68681.make		# this file
FLAGFILE	= $(ODIR)/.updated
SPEC_RFLAGS	= #-aHWSHAKE			# enable hardware handshaking
SYSDEFS		= ../systype.d
RFLAGS		= -qb -u=. -u=$(OSDEFS) -u=$(MACDIR) $(SPEC_RFLAGS)
SLIB		= $(SYSRELS)/sys.l \
		  $(SYSRELS)/scfstat.l
LFLAGS		= -l=$(SLIB) -gu=0.0

DRVR		= sc68681

build: $(ODIR) $(RDIR) $(DRVR)

$(DRVR): $(SLIB) $(RDIR)/$(DRVR).r
	$(LC) $(LFLAGS) $(RDIR)/$(DRVR).r -O=$(ODIR)/$*
	$(TOUCH) $(FLAGFILE)

$(DRVR).r: $(SYSDEFS) $(MAKER)

$(ODIR) $(RDIR):
	@$(MD) $@

clean:
	$(RM) $(RDIR)/$(DRVR).r $(ODIR)/$(DRVR)
