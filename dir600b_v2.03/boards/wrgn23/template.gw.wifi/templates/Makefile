#
# Makefile for the template of board dependent part.
#
-include ../../arch.mk
-include ../../path.mk
-include ../../.config

TMPS=$(TARGET)/etc/templates

all:

install:
	@echo -e "\033[32mInstalling templates for WRGN22 board dependent part...\033[0m"
	$(Q)[ -d $(TMPS) ] || mkdir -p $(TMPS)
	$(Q)cp *.sh			$(TMPS)/.
	$(Q)chmod +x		$(TMPS)/*.sh
	$(Q)[ -d $(TMPS)/wifi ] || mkdir -p $(TMPS)/wifi
	$(Q)cp wifi/*.php	$(TMPS)/wifi/.

clean:

.PHONY: all install clean
