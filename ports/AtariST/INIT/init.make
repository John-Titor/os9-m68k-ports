#############################################################################
# Common init module builder
#############################################################################

# disable implicit rules as we call the assembler and linker directly
-b

# get per-flavor config
include ./$(INIT).com

SDIR		= $(SRCROOT)/SYSMODS/INIT	# INIT descriptor sources
RDIR		= ./RELS
ODIR		= ../CMDS/BOOTOBJS
MAKER		= ./init.make ./$(INIT).com
FLAGFILE	= ../CMDS/BOOTOBJS/.updated
SYSDEFS		= ../systype.d
RFLAGS		= -qb -u=. -u=$(OSDEFS) $(INIT_OPTS)
SLIB		= $(SYSRELS)/sys.l
LFLAGS		= -l=$(SLIB) -n=init -gu=0.0
INITSRC		= init.a
INITREL		= $(RDIR)/$(INIT).r
INITMOD		= $(ODIR)/$(INIT)

build: $(ODIR) $(RDIR) $(INITMOD)

$(INITMOD): $(INITREL) $(SLIB)
	$(LC) $(LFLAGS) $(RDIR)/$*.r -O=$(ODIR)/$*
	$(TOUCH) $(FLAGFILE)

$(INITREL): $(SDIR)/$(INITSRC) $(SYSDEFS) $(MAKER)
	$(RC) $(RFLAGS) $(SDIR)/$(INITSRC) -O=$(RDIR)/$*.r

$(ODIR) $(RDIR):
	$(MD) $@

clean:
	$(RM) $(INITMOD) $(INITREL)
