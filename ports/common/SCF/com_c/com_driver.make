#############################################################################
# Makefile for the COM C driver
#############################################################################

# disable object file rules as we call the linker directly
-bo

SDIR		= $(COM)
ODIR		= ../CMDS/BOOTOBJS
RDIR		= ./RELS
MAKER		= $(COM)/com_driver.make	# this file
FLAGFILE	= $(ODIR)/.updated
SYSDEFS		= ../systype.d
RFLAGS		= -qb -u=. -u=$(OSDEFS) -u=$(MACDIR) -g
CFLAGS		= -r -cw -cq -O=7 -iom
#CFLAGS		+= -as=l					# assembly listing
CFLAGS		+= -g						# debug symbols
SLIB		= -l=$(SYSRELS)/os_lib.l \
			  -l=$(SYSRELS)/sys.l \
			  -l=$(SYSRELS)/cpu.l \
			  -l=$(SYSRELS)/scfstat.l
LFLAGS		= $(SLIB) -gu=0.0 -g -n=com95x

DRVR		= com95x
SHIM		= scf_shims
OBJECTS		= $(RDIR)/$(SHIM).r \
			  $(RDIR)/$(DRVR).r

build: $(DRVR)

$(DRVR): $(OBJECTS) $(MAKER) $(ODIR)
	$(LC) $(LFLAGS) $(OBJECTS) -O=$(ODIR)/$*
	$(TOUCH) $(FLAGFILE)

$(OBJECTS): $(MAKER) $(RDIR)

$(ODIR) $(RDIR):
	@$(MD) $@

clean:
	$(RM) $(OBJECTS) $(ODIR)/$(DRVR)
