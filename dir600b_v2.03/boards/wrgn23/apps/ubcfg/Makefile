# Makefile for uboot environment variables management tool.
#
# Leon Yen.
-include ../../arch.mk
-include ../../path.mk

APPLET:=ubcfg

SOURCE:=ubcfg.c crc32.c
# Following two defines could be get in kernel/drivers/mtd/maps/elbox_physmap.c
EXTRA_CFLAGS:= -DCFG_LOCATION=\"/dev/mtdblock/4\"
#EXTRA_CFLAGS:= -DCFG_LOCATION=\"ubootcfg.hex\"
EXTRA_CFLAGS+= -DCFG_ENV_SIZE=0x04000
# Here we follow Ralink RT3052 uboot's implementation, their CRC only calculate 4K-4 bytes in length. 
EXTRA_CFLAGS+= -DCFG_CRC_CALCULATED_SIZE=0x1000-4

all: $(APPLET)

$(APPLET): $(SOURCE)
	@echo -e "\033[32mBuilding uboot config tool for RT3052 ... \033[0m"
	$(Q)$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $(LDFLAGS) -o $@ $(SOURCE)
	$(Q)$(STRIP) --remove-section=.note --remove-section=.comment $@

install: 
	@echo -e "\033[32mInstalling uboot config tool for RT3052 ... \033[0m"
	$(Q)cp $(APPLET) $(TARGET)/usr/sbin/.

clean:
	@echo -e "\033[32mCleaning uboot config for RT3052 ... \033[0m"
	$(Q)rm -f $(APPLET) *.o

.PHONY: all install clean
