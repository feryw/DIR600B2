#
# Makefile for starspeed
#
-include ../arch.mk
-include ../../path.mk
-include ../../.config

##########################################################################
# source for starspeed

MYNAME      := Starspeed (Mainland China)
APPLET      := starspeed
LOCAL_SRCS  := main.c
CMLIB_SRCS  := md5.c

##########################################################################

CMDS=
CFLAGS+=-I$(TOPDIR)/include

##########################################################################

ifeq ($(strip $(CONFIG_STARSPEED_HUBEI)),y)
LOCAL_SRCS+=hubei.c
CMDS+=hubei
endif

ifeq ($(strip $(CONFIG_STARSPEED_XIAN)),y)
LOCAL_SRCS+=pap_crack.c
CMDS+=pap_crack
endif

ifeq ($(strip $(CONFIG_STARSPEED_HENAN)),y)
LOCAL_SRCS+=henan.c
CMDS+=henan
endif

ifeq ($(strip $(CONFIG_STARSPEED_NANCHANG_CAMPUS)),y)
LOCAL_SRCS+=nanchang_campus.c
CMDS+=nanchang_campus
endif
##########################################################################

-include $(TOPDIR)/Rules.mk

showconfig:
	@echo "APPLET     = "$(APPLET)
	@echo "LOCAL_SRCS = "$(LOCAL_SRCS)
	@echo "CMLIB_SRCS = "$(CMLIB_SRCS)
	@echo "CMDS       = "$(CMDS)
	@echo "CFLAGS     = "$(CFLAGS)

install:
	@echo -e "\033[32mInstalling $(MYNAME) ...\033[0m"
	$(Q)$(STRIP) $(APPLET)
	$(Q)[ -d $(TARGET)/usr/sbin ] || mkdir -p $(TARGET)/usr/sbin
	$(Q)cp $(APPLET) $(TARGET)/usr/sbin
	$(Q)for i in $(CMDS) ; do \
		[ ! -f $(TARGET)/usr/sbin/$$i ] || rm -f $(TARGET)/usr/sbin/$$i ; \
		ln -s ./$(APPLET) $(TARGET)/usr/sbin/$$i ; \
	done

clean: clean_objs
	@echo -e "\033[32mCleaning $(MYNAME) ...\033[0m"
	$(Q)rm -f $(APPLET)

.PHONY: showconfig install clean
