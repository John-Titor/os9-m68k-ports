#############################################################################
# Build the init modules
#############################################################################

# disable implicit rules
-b

FLAVORS		= init_rom

$(GOAL): $(FLAVORS) .

$(FLAVORS): .
	@echo ~~~~ $@
	$(MAKE) -f=init.make INIT=$@ $(GOAL)
