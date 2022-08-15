# EHBasic
#
-e

ifndef ODIR
ODIR	= ../../bin
endif
PROG	= $(ODIR)/basic68k
ASRC	= ./basic68k.a
AREL	= ./basic68k.r
ADEF	= basic68k.d

$(PROG): $(AREL)
	l68 -g -O=$@ $(AREL) -l=$(MWOS)/OS9/68000/LIB/sys.l

$(AREL): $(ASRC) $(ADEF)
	r68 -q -g -O=$@ $(ASRC)
