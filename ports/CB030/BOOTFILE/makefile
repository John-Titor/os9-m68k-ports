#############################################################################
# Bootfiles
#############################################################################

# disable implicit rules
-b

FLAVORS		= rom

$(GOAL): $(FLAVORS) .

$(FLAVORS): .
	@echo ~~~~ $@
	$(MAKE) -f=bootfile.make BOOTFILE=$@ $(GOAL)
