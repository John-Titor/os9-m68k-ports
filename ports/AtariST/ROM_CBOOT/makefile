#############################################################################
# ROM toplevel
#############################################################################

# disable implicit rules
-b

$(GOAL): .
#	$(MAKE) -f=rom_image.make TYPE=ROMBUG $(GOAL)
	$(MAKE) -f=rom_image.make TYPE=ROMBUG BOOTFILE=dev $(GOAL)
#	$(MAKE) -f=rom_image.make TYPE=NOBUG BOOTFILE=dev $(GOAL)
#	$(MAKE) -f=rom_image.make TYPE=NOBUG $(GOAL)
#	$(MAKE) -f=rom_image.make TYPE=NOBUG BOOTFILE=diskboot $(GOAL)
