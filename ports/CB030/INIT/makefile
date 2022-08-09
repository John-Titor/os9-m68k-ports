#############################################################################
# Build the init modules
#############################################################################

# disable implicit rules
-b

FLAVORS		= init_rom init_cf

$(GOAL): $(FLAVORS) .

$(FLAVORS): .
	@echo ~~~~ $@
	$(MAKE) -f=init.make INIT=$@ $(GOAL)
