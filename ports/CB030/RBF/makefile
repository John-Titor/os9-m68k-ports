#############################################################################
# RBF modules toplevel
#############################################################################

# disable implicit rules
-b

SUBMAKES	= rbf_descriptors.make \
		  rbf_cfide.make		# sub-makefiles to build

$(GOAL): $(SUBMAKES) .

$(SUBMAKES): .					# build sub-makefiles
	@echo ~~~~ $@
	$(MAKE) -f=$@ $(GOAL)
