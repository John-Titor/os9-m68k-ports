#############################################################################
# ROM toplevel
#############################################################################

# disable implicit rules
-b

FLAVORS		= rombug

$(GOAL): $(FLAVORS) .

$(FLAVORS): .
	@echo ~~~~ $@
	$(MAKE) -f=$@.make GOAL=$(GOAL) $(GOAL)
