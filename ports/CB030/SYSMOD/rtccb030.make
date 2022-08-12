#############################################################################
# CB030 RTC driver
#############################################################################

# disable builtin rules as we call the assembler and linker directly
-b

SDIR		= .
RDIR		= RELS
ODIR		= ../CMDS/BOOTOBJS
MAKER		= ./rtccb030.make		# this file
SYSDEFS		= ../systype.d
RFLAGS		= -qb -u=. -u=$(OSDEFS) -u=$(MACDIR)
SLIB		= $(SYSRELS)/sys.l
LFLAGS		= -l=$(SLIB) -gu=0.0 -n=rtclock
RTCDRV		= rtccb030
RTCDRVSRC	= ./$(RTCDRV).a
RTCDRVREL	= $(RDIR)/$(RTCDRV).r
RTCMOD		= $(ODIR)/$(RTCDRV)

build: $(ODIR) $(RDIR) $(RTCMOD)

$(RTCMOD): $(RTCDRVREL) $(SLIB)
	$(LC) $(LFLAGS) $(RTCDRVREL) -O=$@

$(RTCDRVREL): $(RTCDRVSRC) $(SYSDEFS) $(MAKER)
	$(RC) $(RFLAGS) $(RTCDRVSRC) -O=$@

$(ODIR) $(RDIR): .
	@$(MD) $@

clean:
	$(RM) $(ODIR)/$(RTCDRV)
	$(RM) $(RDIR)/$(RTCDRVREL)
