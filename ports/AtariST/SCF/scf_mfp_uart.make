#############################################################################
# Atari ST MFP UART driver
#############################################################################

# disable object file rules as we call the linker directly
-bo

SDIR		= .				# SCF driver sources
ODIR		= ../CMDS/BOOTOBJS
RDIR		= ./RELS
MAKER		= ./scf_mfp_uart.make		# this file
FLAGFILE	= $(ODIR)/.updated
SYSDEFS		= ../systype.d
RFLAGS		= -qb -u=. -u=$(OSDEFS) -u=$(MACDIR)
SLIB		= $(SYSRELS)/sys.l \
		  $(SYSRELS)/scfstat.l
LFLAGS		= -l=$(SLIB) -gu=0.0

DRVR		= sc_mfp_uart

build: $(ODIR) $(RDIR) $(DRVR)

$(DRVR): $(SLIB) $(RDIR)/$(DRVR).r
	$(LC) $(LFLAGS) $(RDIR)/$(DRVR).r -O=$(ODIR)/$*
	$(TOUCH) $(FLAGFILE)

$(DRVR).r: $(SYSDEFS) $(MAKER)

$(ODIR) $(RDIR):
	@$(MD) $@

clean:
	$(RM) $(RDIR)/$(DRVR).r $(ODIR)/$(DRVR)
