#############################################################################
# CB030 ticker driver
#############################################################################

# disable builtin rules as we call the assembler and linker directly
-b

SDIR		= $(SRCROOT)/SYSMODS/GCLOCK	# standard clock sources
RDIR		= RELS
ODIR		= ../CMDS/BOOTOBJS
MAKER		= ./tkcb030.make		# this file
SYSDEFS		= ../systype.d
RFLAGS		= -qb -u=. -u=$(OSDEFS) -u=$(MACDIR)
SLIB		= $(SYSRELS)/sys.l
LFLAGS		= -l=$(SLIB) -gu=0.0
TKGEN		= tickgeneric
TKGENSRC	= $(SDIR)/$(TKGEN).a
TKGENREL	= $(RDIR)/$(TKGEN).r
TKDRV		= tkcb030
TKDRVSRC	= ./$(TKDRV).a
TKDRVREL	= $(RDIR)/$(TKDRV).r
TKMOD		= $(ODIR)/$(TKDRV)

build: $(ODIR) $(RDIR) $(TKMOD)

$(TKMOD): $(TKGENREL) $(TKDRVREL) $(SLIB)
	$(LC) $(LFLAGS) $(TKGENREL) $(TKDRVREL) -O=$@

$(TKDRVREL): $(TKDRVSRC) $(SYSDEFS) $(MAKER)
	$(RC) $(RFLAGS) $(TKDRVSRC) -O=$@

$(TKGENREL): $(TKGENSRC) $(SYSDEFS) $(MAKER)
	$(RC) $(RFLAGS) $(TKGENSRC) -O=$@

$(ODIR) $(RDIR): .
	@$(MD) $@

clean:
	$(RM) $(ODIR)/$(TKDRV)
	$(RM) $(RDIR)/$(TKDRVREL)
	$(RM) $(RDIR)/$(TKGENREL)
