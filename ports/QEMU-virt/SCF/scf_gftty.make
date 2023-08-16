#############################################################################
# SCF Goldfish TTY driver
#############################################################################

# disable object file rules as we call the linker directly
-bo

SDIR		= .				# driver sources
ODIR		= ../CMDS/BOOTOBJS
RDIR		= ./RELS
MAKER		= ./scf_gftty.make		# this file
FLAGFILE	= $(ODIR)/.updated
SPEC_RFLAGS	= #-aHWSHAKE			# enable hardware handshaking
SYSDEFS		= ../systype.d
RFLAGS		= -qb -u=. -u=$(OSDEFS) -u=$(MACDIR) $(SPEC_RFLAGS) -g
SLIB		= $(SYSRELS)/sys.l \
		  $(SYSRELS)/scfstat.l
LFLAGS		= -l=$(SLIB) -gu=0.0 -g

DRVR		= gftty

build: $(ODIR) $(RDIR) $(DRVR)

$(DRVR): $(SLIB) $(RDIR)/$(DRVR).r
	$(LC) $(LFLAGS) $(RDIR)/$(DRVR).r -O=$(ODIR)/$*
	$(TOUCH) $(FLAGFILE)

$(DRVR).r: $(SYSDEFS) $(MAKER)

$(ODIR) $(RDIR):
	@$(MD) $@

clean:
	$(RM) $(RDIR)/$(DRVR).r $(ODIR)/$(DRVR)
