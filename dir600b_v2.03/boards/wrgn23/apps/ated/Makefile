
-include ../../arch.mk
-include ../../path.mk

APPLET:=ated

all: $(APPLET)

$(APPLET): ated.c ated.h
	@echo -e "\033[32mBuilding ATE tools for RT2880 ... \033[0m"
	$(Q)$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $(LDFLAGS) -o $@ $@.c $(LDLIBS)
	$(Q)$(STRIP) --remove-section=.note --remove-section=.comment $@

install: 
	@echo -e "\033[32mInstalling ATE tools for RT2880 ... \033[0m"
	$(Q)cp $(APPLET) $(TARGET)/usr/sbin/.

clean:
	@echo -e "\033[32mCleaning ATE tools for RT2880 ... \033[0m"
	$(Q)rm -f $(APPLET) *.o

.PHONY: all install clean
