#############################################################################
# RBF modules toplevel
#############################################################################

# disable implicit rules
-b

CFIDE		= ../../common/RBF/cfide
SUBMAKES	= $(CFIDE)/cfide_descriptors.make \
		  $(CFIDE)/cfide_driver.make

$(GOAL): $(SUBMAKES) .

$(SUBMAKES): .					# build sub-makefiles
	@echo ~~~~ $@
	$(MAKE) -f=$@ $(GOAL) CFIDE=$(CFIDE)
