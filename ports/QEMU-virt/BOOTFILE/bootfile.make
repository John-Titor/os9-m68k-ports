#############################################################################
# ROM bootfile
#############################################################################

# disable implicit rules, we're doing everything ourselves
-b

ODIR		= ../CMDS/BOOTOBJS/BOOTFILES
MAKER		= ./bootfile.make
PATCHER		= ./patch.bat
FLAGFILE	= ../CMDS/BOOTOBJS/.updated
OFILE		= $(ODIR)/$(BOOTFILE).bf
INFILE		= $(BOOTFILE).bl
TMPFILE		= $(ODIR)/$(BOOTFILE).bl

# path for "local" bootfiles
LOCAL		@= ..

build: $(ODIR) $(OFILE)

$(OFILE): $(MAKER) $(TMPFILE) $(FLAGFILE)
	$(MERGE) -o=$@ -z=$(TMPFILE)

$(TMPFILE): $(MAKER) $(PATCHER) $(INFILE)
	$(PATCHER) $(INFILE) >$@

$(FLAGFILE):
	$(TOUCH) $(FLAGFILE)

$(ODIR):
	@$(MD) $@

clean:
	$(RM) $(OFILE) $(TMPFILE) $(FLAGFILE)
