#
# Makefile for WRG-N23
#

-include arch.mk
-include path.mk
-include .config

#####################################################################
# targets for main Makefile
MYNAME	:= WRGN23
MFILE	:= $(shell echo Makefile.$(ELBOX_TEMPLATE_NAME))

ifeq ($(MFILE), $(wildcard $(MFILE)))
all: apps

-include $(MFILE)
else
all:
	@echo -e "\033[31mERROR: $(MYNAME) does not support TEMPLATE - $(ELBOX_TEMPLATE_NAME) !!\033[0m"
endif

.PHONY: all
#####################################################################
