# Makefile for syslogd

-include ../arch.mk
-include ../../path.mk
-include ../../.config
UBCOM := ubcom
SRCS := ubcom.c
OBJS:=$(patsubst %c,%o,$(SRCS))
DEPENDS = Makefile
CFLAGS += -I$(TOPDIR)/include
CFLAGS_NOOS = $(subst -Os,,$(CFLAGS))
###CFLAGS_NOOS = $(patsubst -O%,,$(CFLAGS))


all:	$(UBCOM) 

install:
	@echo -e "\033[32mInstalling ubcom ...\033[0m"
	$(Q)[ -d $(TARGET)/sbin ] || mkdir -p $(TARGET)/sbin
	$(Q)install $(UBCOM)  $(TARGET)/sbin/.

$(UBCOM): $(OBJS)
	@echo -e "\033[32mBuilding ubcom ...\033[0m"
###	@$(CC) $(CFLAGS) -o $@ $(OBJS)
	$(CC) $(CFLAGS_NOOS) -o $@ $(OBJS)
	$(Q)$(STRIP) $(UBCOM)

$(OBJS): $(DEPENDS)
.c.o:
###	$(CC) -c $(CFLAGS) $< -o $@
	$(CC) -c $(CFLAGS_NOOS) $< -o $@
ubcom.o: Makefile ubcom.c 
###	$(Q)$(CC) -c $(CFLAGS) ubcom.c
	$(CC) -c $(CFLAGS_NOOS) ubcom.c

clean:
	@echo -e "\033[32mCleaning ubcom ...\033[0m"
	$(Q)rm -f *.o *~ *.gdb *.elf $(UBCOM)
.PHONY:	all install clean
