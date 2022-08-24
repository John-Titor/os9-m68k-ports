#############################################################################
# RBF CompactFlash driver
#############################################################################

# disable object file rules as we call the linker directly
-bo

SDIR		= $(CFIDE)
ODIR		= ../CMDS/BOOTOBJS
RDIR		= ./RELS
MAKER		= $(CFIDE)/cfide_driver.make	# this file
FLAGFILE	= $(ODIR)/.updated
SYSDEFS		= ../systype.d
RFLAGS		= -qb -u=. -u=$(OSDEFS) -u=$(MACDIR)
SLIB		= $(SYSRELS)/sys.l \
		  $(SYSRELS)/drvs1.l		# one drive table
LFLAGS		= -l=$(SLIB) -gu=0.0

DRVR		= cfide

build: $(ODIR) $(RDIR) $(DRVR)

$(DRVR): $(SLIB) $(RDIR)/$(DRVR).r
	$(LC) $(LFLAGS) $(RDIR)/$(DRVR).r -O=$(ODIR)/$*
	$(TOUCH) $(FLAGFILE)

$(DRVR).r: $(SYSDEFS) $(MAKER)

$(ODIR) $(RDIR):
	@$(MD) $@

clean:
	$(RM) $(RDIR)/$(DRVR).r $(ODIR)/$(DRVR)
