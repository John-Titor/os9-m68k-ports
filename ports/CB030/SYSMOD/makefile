#############################################################################
# System modules toplevel
#############################################################################

# disable implicit rules
-b

SUBMAKES	= tkcb030.make

$(GOAL): $(SUBMAKES) .

$(SUBMAKES): .
	@echo ~~~~ $@
	$(MAKE) -f=$@ $(GOAL)
