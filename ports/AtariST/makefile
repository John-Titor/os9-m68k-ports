#############################################################################
# Top-level port makefile
#############################################################################

# disable built-in rules
-b

# configuration for hardware
ROM_BASE	@= 00e00000			# bootrom link address

# configure common paths; use -e in sub-makefiles to access
SDKROOT		@= $(MWOS)/OS9			# OS9 SDK root
SRCROOT		@= $(MWOS)/OS9/SRC		# OS9 SDK sources
OSDEFS		@= $(MWOS)/OS9/SRC/DEFS		# std OS defs (XXX should be SYSDEFS ?)
MACDIR		@= $(MWOS)/OS9/SRC/MACROS	# std OS macros
ARCHROOT	@= $(MWOS)/OS9/68000		# 68k arch root
SYSRELS		@= $(MWOS)/OS9/68000/LIB	# 68k arch libraries
CDEFS		@= $(MWOS)/SRC/DEFS		# standard C defs

# tools
RC		@= r68
LC		@= l68
LD		@= l68
CC		@= xcc
MAKE		@= os9make -e
MD		@= os9makdir -pq
RM		@= os9del -qf
CD		@= cd
MERGE		@= os9merge
TOUCH		@= os9touch

# subdirectories to build, order is important
DIRS		+= INIT
DIRS		+= RBF
DIRS		+= SCF
DIRS		+= SYSMOD
DIRS		+= BOOTFILE
DIRS		+= ROM_CBOOT

# decide what to build in subdirectories
if make(clean)
GOAL		= clean
else
GOAL		= build
endif
GOAL		@= $(GOAL)

build: $(DIRS) .

clean: $(DIRS) .
	@echo remove CMDS

$(DIRS): .
	@echo ~~~ $@
	cd $@; $(MAKE) -e -f=makefile $(GOAL)
