#
# Makefile for WRG-N18
#

-include ../arch.mk
-include ../path.mk
-include ../.config

#####################################################################
# targets for main Makefile
MYNAME:=WRG-N23

# build board dependent programs
all:
#	@echo -e "\033[32mBuilding board related programs for $(MYNAME) ...\033[0m"
#	$(Q)make -C apps V=$(V)

# install board dependent programs
install:
	@echo -e "\033[32mInstalling board related programs for $(MYNAME) ...\033[0m"
	$(Q)[ -d $(TARGET)/etc/scripts ] || mkdir -p $(TARGET)/etc/scripts
	$(Q)[ -d $(TARGET)/etc/scripts/misc ] || mkdir -p $(TARGET)/etc/scripts/misc
	$(Q)[ -d $(TARGET)/etc/init.d ] || mkdir -p $(TARGET)/etc/init.d
	$(Q)cp scripts/config.sh		$(TARGET)/etc/scripts/.
	$(Q)cp scripts/layout_run.php	$(TARGET)/etc/scripts/.
	$(Q)cp scripts/layout.sh		$(TARGET)/etc/scripts/.
	$(Q)cp scripts/enlan.sh			$(TARGET)/etc/scripts/.
	$(Q)cp scripts/dislan.sh		$(TARGET)/etc/scripts/.
	$(Q)chmod +x $(TARGET)/etc/scripts/*.sh
	$(Q)ln -sf /etc/scripts/config.sh $(TARGET)/etc/init.d/S03config.sh
	$(Q)cp scripts/cabletest:?		$(TARGET)/usr/sbin/.
	$(Q)cp scripts/switch			$(TARGET)/usr/sbin/.
	$(Q)cp scripts/setwantype.sh	$(TARGET)/etc/scripts/misc/.
	$(Q)cp scripts/startburning.sh	$(TARGET)/etc/scripts/.
#	$(Q)make -C apps install V=$(V)
	$(Q)make -C templates install V=$(V)

# clean board dependent programs
clean:
#	@echo -e "\033[32mCleaning board related programs for $(MYNAME) ...\033[0m"
#	make -C apps clean V=$(V)

# setup board dependent program configuration
prepare:
	@echo -e "\033[32mPreparing configuration for board - $(MYNAME) ...\033[0m"

# build board dependent rootfs
rootfs:
	@echo -e "\033[32mCreate rootfs for board - $(MYNAME) ...\033[0m"
	$(Q)echo "/dev/mtdblock/3" >> $(TARGET)/etc/config/devconf
	$(Q)echo "/dev/mtdblock/4" >> $(TARGET)/etc/config/devdata
ifeq ($(strip $(ELBOX_CREATE_LANGUAGE_PACK)),y)
	$(Q)echo "/dev/mtdblock/5" >> $(TARGET)/etc/config/langpack
endif
	$(Q)cp -rf defnodes $(TARGET)/etc/.
	$(Q)echo "#!/bin/sh" >> $(TARGET)/bin/reboot
	$(Q)echo "echo 1 > /proc/system_reset" >> $(TARGET)/bin/reboot
	$(Q)chmod +x $(TARGET)/bin/reboot

.PHONY: all install clean prepare rootfs

#####################################################################
