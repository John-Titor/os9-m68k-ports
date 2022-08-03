#############################################################################
# SCF modules toplevel
#############################################################################

# disable implicit rules
-b

SUBMAKES	= scf_descriptors.make \
		  scf_sc68681.make		# sub-makefiles to build

$(GOAL): $(SUBMAKES) .

$(SUBMAKES): .					# build sub-makefiles
	@echo ~~~~ $@
	$(MAKE) -f=$@ $(GOAL)
