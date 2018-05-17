#
# Board dependent Makefile for WRG-N23
#

ifeq ($(strip $(ELBOX_TEMPLATE_NAME)),"aries")
-include $(TOPDIR)/progs.board/template.aries/config.mk
endif

ifeq ($(strip $(ELBOX_TEMPLATE_NAME)),"gw.wifi")
-include $(TOPDIR)/progs.board/template.gw.wifi/config.mk
endif

