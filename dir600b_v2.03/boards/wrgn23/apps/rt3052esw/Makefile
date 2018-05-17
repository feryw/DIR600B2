
-include ../../arch.mk
-include ../../path.mk

APPLET:=rt3052esw
CFLAGS += -I $(TOPDIR)/include -I$(KERNELDIR)/drivers/net/raeth

all: $(APPLET)

$(APPLET): rt3052esw.c 
	@echo -e "\033[32mBuilding tools for Ralink rt3052 embedded switch ...\033[0m"
	$(Q)$(CC) $(CFLAGS) -c rt3052esw.c 
	$(Q)$(CC) $(CFLAGS) -o $(APPLET) rt3052esw.o 
	$(Q)$(STRIP) $(APPLET)

install:
	@echo -e "\033[32mInstalling tools for Ralink rt3052 embedded switch ...\033[0m"
	$(Q)cp $(APPLET) $(TARGET)/usr/sbin/.
	$(Q)[ ! -f $(TARGET)/usr/sbin/psts ] || rm -f $(TARGET)/usr/sbin/psts
	$(Q)ln -s $(APPLET) $(TARGET)/usr/sbin/psts
	$(Q)[ ! -f $(TARGET)/usr/sbin/slinktype ] || rm -f $(TARGET)/usr/sbin/slinktype
	$(Q)ln -s $(APPLET) $(TARGET)/usr/sbin/slinktype
	$(Q)[ ! -f $(TARGET)/usr/sbin/ctest ] || rm -f $(TARGET)/usr/sbin/ctest
	$(Q)ln -s $(APPLET) $(TARGET)/usr/sbin/ctest

clean:
	@echo -e "\033[32mCleaning tools for Ralink rt3052 embedded switch ...\033[0m"
	$(Q)rm -f $(APPLET) *.o

.PHONY: all install clean
