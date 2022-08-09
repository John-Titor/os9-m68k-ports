#############################################################################
# Bootfiles
#############################################################################

# disable implicit rules
-b

FLAVORS		= lite dev

$(GOAL): $(FLAVORS) .

$(FLAVORS): .
	@echo ~~~~ $@
	$(MAKE) -f=bootfile.make BOOTFILE=$@ $(GOAL)
