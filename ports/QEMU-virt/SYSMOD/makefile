#############################################################################
# System modules toplevel
#############################################################################

# disable implicit rules
-b

SUBMAKES	= gftick.make \
		  gfrtc.make

$(GOAL): $(SUBMAKES) .

$(SUBMAKES): .
	@echo ~~~~ $@
	$(MAKE) -f=$@ $(GOAL)
