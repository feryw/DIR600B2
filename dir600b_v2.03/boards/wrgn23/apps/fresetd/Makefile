# vi: set sw=4 ts=4:
#
# Makefile for WRG-G19 rev C board dependent programs
#

-include ../../arch.mk
-include ../../path.mk
-include $(TOPDIR)/.config

#####################################################################
# source files

APPLET      := fresetd
ifeq ($(strip $(ELBOX_BRAND_DLWBR)),y)
LOCAL_SRCS  := fresetd.c rt3052_gpio.c
CFLAGS += -D__DLINK__
else
LOCAL_SRCS	:= fresetd_other.c rt3052_gpio.c
endif
CMLIB_SRCS  := sloop.c dtrace.c unixsock.c libxmldbc.c
CPLUS_SRCS  :=

OTHER_TARGETS:=

CFLAGS += -DSLOOP_USE_GETTIMEOFDAY -DDEBUG_SLOOP=0
CFLAGS += -I$(KERNELDIR)/drivers/char -I$(KERNELDIR)/include

#WAN status detection is triggered by ethernet interrupt.Note, kernel option CONFIG_RT_3052_ESW_PORT_STATUS_INT should be enabled.
ifeq ($(strip $(ELBOX_WRGN23_WAN_UP_DETECT_BY_INTERRUPT)),y)
CFLAGS += -DWAN_UP_DETECT_BY_INTERRUPT
endif

#####################################################################
# flags to turn on debug mode

ifeq ($(strip $(DEBUG)),y)
DDEBUG:=y
DDEBUG_LEVEL:=0
endif

ifeq ($(strip $(DDEBUG)),y)
CFLAGS += -DDDEBUG
#CFLAGS += -DDEBUG_SLOOP=1
ifdef DDEBUG_LEVEL
CFLAGS += -DDBG_DEFAULT=$(DDEBUG_LEVEL)
endif
endif

#####################################################################

-include $(TOPDIR)/Rules.mk
install:
	@echo -e "\033[32mInstall fresetd for $(ELBOX_BRAND_NAME) $(ELBOX_MODEL_NAME)...\033[0m"
	cp fresetd $(TARGET)/usr/sbin/.

clean: 
	rm -f fresetd *.o .depend

.PHONY: all install clean
#####################################################################

