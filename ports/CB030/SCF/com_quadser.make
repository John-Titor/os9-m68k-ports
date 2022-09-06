#############################################################################
# OX16C954 on plasmo's "quadser" RC2014 board
#############################################################################

# disable implicit rules
-b

COM		= ../../common/SCF/com_c

SUBMAKES	= $(COM)/com_driver.make

$(GOAL): $(SUBMAKES) .

$(SUBMAKES): .
	@echo ~~~~ $@
	$(MAKE) -f=$@ $(GOAL) COM=$(COM) COM_TYPE=95x
