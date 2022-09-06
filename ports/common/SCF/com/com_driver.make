#############################################################################
# Makefile for the COM driver
#############################################################################

# disable object file rules as we call the linker directly
-bo

# get COM_TYPE from the commandline
-e

SDIR		= $(COM)
ODIR		= ../CMDS/BOOTOBJS
RDIR		= ./RELS
MAKER		= $(COM)/com_driver.make	# this file
FLAGFILE	= $(ODIR)/.updated
SYSDEFS		= ../systype.d
RFLAGS		= -qb -u=. -u=$(OSDEFS) -u=$(MACDIR) -g
SLIB		= $(SYSRELS)/sys.l \
		  $(SYSRELS)/scfstat.l
LFLAGS		= -l=$(SLIB) -gu=0.0 -g

DRVR		= com$(COM_TYPE)

build: $(ODIR) $(RDIR) $(DRVR)

$(DRVR): $(RDIR)/$(DRVR).r $(MAKER)
	$(LC) $(LFLAGS) $(RDIR)/$(DRVR).r -O=$(ODIR)/$*
	$(TOUCH) $(FLAGFILE)

$(DRVR).r: $(SYSDEFS) $(MAKER)

$(ODIR) $(RDIR):
	@$(MD) $@

clean:
	$(RM) $(RDIR)/$(DRVR).r $(ODIR)/$(DRVR)
