#############################################################################
# Bootfiles
#############################################################################

# disable implicit rules
-b

FLAVORS		= dev # diskboot

$(GOAL): $(FLAVORS) .

$(FLAVORS): .
	@echo ~~~~ $@
	$(MAKE) -f=bootfile.make BOOTFILE=$@ $(GOAL)
