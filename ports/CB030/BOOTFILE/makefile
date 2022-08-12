#############################################################################
# Bootfiles
#############################################################################

# disable implicit rules
-b

FLAVORS		= diskboot dev

$(GOAL): $(FLAVORS) .

$(FLAVORS): .
	@echo ~~~~ $@
	$(MAKE) -f=bootfile.make BOOTFILE=$@ $(GOAL)
