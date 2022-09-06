#############################################################################
# SCF modules toplevel
#############################################################################

# disable implicit rules
-b

SUBMAKES	 = scf_descriptors.make		# descriptors
SUBMAKES	+= sc68681_driver.make		# driver for onboard serial
SUBMAKES	+= com_quadser.make		# driver for "quadser" card

$(GOAL): $(SUBMAKES) .

$(SUBMAKES): .					# build sub-makefiles
	@echo ~~~~ $@
	$(MAKE) -f=$@ $(GOAL)
