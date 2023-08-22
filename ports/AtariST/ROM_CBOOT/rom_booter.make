#############################################################################
# Link the ROM booter.
#############################################################################

# disable implicit rules
-b

RDIR		= ./RELS/$(TYPE)
ODIR		= ../CMDS/BOOTOBJS/$(TYPE)
ROMRELS		= $(SRCROOT)/ROM/LIB		# ROM libraries
TARGET		= romboot			# base name of the thing we are making
TGT_BIN		= $(ODIR)/$(TARGET)		# binary we are making
TGT_MAP		= $(ODIR)/$(TARGET).map		# link map
MAKER		= rom_booter.make		# this file
SYSLIBS		= $(ROMRELS)/sysboot.l \
		  $(ROMRELS)/flushcache.l \
		  $(SYSRELS)/clib.l \
		  $(SYSRELS)/sys_clib.l \
		  $(SYSRELS)/os_lib.l \
		  $(SYSRELS)/sys.l		# libraries - order is important
OBJECTS		= $(RDIR)/rom_header.l \
		  $(RDIR)/rom_common.l \
		  $(RDIR)/rom_port.l \
		  $(RDIR)/rom_serial.l \
		  $(ROMRELS)/romio.l		# (merged) objects - order is important
if $(TYPE) == "ROMBUG"
OBJECTS		+= $(ROMRELS)/rombug.l
endif
for lib in $(SYSLIBS)				# generate -l=$(lib) for linker
LNKLIBS		+= -l=$(lib)			# ... because $(LIBS:%=-l=%) doesn't work
endfor
DEPLIBS		= $(SYSLIBS) \
		  $(OBJECTS)			# library files depended on
LFLAGS		= -r=$(ROM_BASE)		# link at base of ROM
LFLAGS		+= -s				# print symbol relative addresses
LFLAGS		+= -w				# sort symbols alphabetically
LFLAGS		+= -m				# print link map
LFLAGS		+= -a				# enable GOT/PLT-like table
LFLAGS		+= -M=3k			# add 3k to stack allocation
#LFLAGS		+= -g				# generate debug symbol file
LFLAGS		+= -b=4				# 4-align segments
#LFLAGS		+= -gu=0.0			# module owned by superuser

build: $(ODIR) $(TGT_BIN)

# Use $(LD) here as os9make stubbornly insists on calling xcc even though
# $(LC) is set to l68.
$(TGT_BIN): $(DEPLIBS) $(MAKER)
	$(LD) $(LFLAGS) $(LNKLIBS) $(OBJECTS) -O=$@ >$(TGT_MAP)

$(ODIR):
	@$(MD) $@

clean:
	$(RM) $(TGT_BIN) $(TGT_MAP)
