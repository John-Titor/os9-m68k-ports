#############################################################################
# System modules toplevel
#############################################################################

# disable implicit rules
-b

SUBMAKES	= tk_mfp_timerc.make \
		  rtc5c15.make

$(GOAL): $(SUBMAKES) .

$(SUBMAKES): .
	@echo ~~~~ $@
	$(MAKE) -f=$@ $(GOAL)
