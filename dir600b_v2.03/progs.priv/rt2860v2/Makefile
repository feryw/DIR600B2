#
# Makefile for rt2860v2 driver in elbox
#
# by david_hsieh@alphanetworks.com
#
# Modify to use new driver for RT3052
# by leon_yen@alphanetworks.com

-include ../../path.mk
-include ../arch.mk
-include ../../.config
####################################################################################

ifeq ($(strip $(ELBOX_PROGS_PRIV_RT2860V2)), y)
SRCTARBALL  := rt2860v2_2200_dpa_1109.tar.bz2
EEPROMFILE  := RT2880_eeprom_v1_0.bin
endif
ifeq ($(strip $(ELBOX_PROGS_PRIV_RT2860V2_V1900)), y)
#SRCTARBALL	:= rt2860v2_SDK3300_v2200.tar.bz2
SRCTARBALL	:= rt2860v2_SDK3100_v1900.tar.bz2
EEPROMFILE  := RT3052_AP_2T2R_V1_1.bin
endif
ifeq ($(strip $(ELBOX_PROGS_PRIV_RT2860V2_V3200)), y)
SRCTARBALL      := rt2860v2_SDK3200_v2000.tar.bz2
EEPROMFILE  :=  RT2880_eeprom_v1_0.bin
endif
ifeq ($(strip $(ELBOX_PROGS_PRIV_RT2860V2_V2200)), y)
SRCTARBALL	:= rt2860v2_SDK3300_v2200.tar.bz2
ifeq ($(strip $(ELBOX_BOARD_WRGN22)), y)
EEPROMFILE  := RT3052_AP_2T2R_V1_1.bin
else
EEPROMFILE  := RT3050_AP_1T1R_V1_0.bin
endif
ifeq ($(strip $(ELBOX_PROGS_PRIV_RT2860V2_WAPI)), y)
RALINK_WAPI_SUPPORT=y
export RALINK_WAPI_SUPPORT
RALINK_WAPI_OBJ=wapi/wapi_lib.wa
RALINK_WAPI_DAEMON=wapi/wapid
#RALINK_WAPI_LIBRARY=wapi/libcrypto.so.0.9.8
endif
endif
ifeq ($(strip $(ELBOX_PROGS_PRIV_RT2860V2_V2300)), y)
SRCTARBALL	:= rt2860v2_sdk3330_all_dpd_20090924.tar.bz2
ifeq ($(strip $(ELBOX_BOARD_WRGN22)), y)
EEPROMFILE  := RT3052_AP_2T2R_V1_1.bin
else
EEPROMFILE  := RT3050_AP_1T1R_V1_0.bin
endif
endif
ifeq ($(strip $(ELBOX_PROGS_PRIV_RT2860V2_WDS)), y)
RALINK_WDS_SUPPORT=y
export RALINK_WDS_SUPPORT
endif

MYNAME		:= RT2860v2
CURRDIR		:= $(shell pwd)
BUILD		:= build
BUILDDIR	:= $(CURRDIR)/$(BUILD)
SRCDIR		:= $(CURRDIR)/sources
DRVPATH		:= $(BUILD)/rt2860v2_ap
ifeq ($(strip $(ELBOX_PROGS_PRIV_RT2860V2_STA)),y)
STAPATH     := $(BUILD)/rt2860v2_sta
endif
UNTAR		:= tar jxf
DIFF		:= $(SRCTARBALL).alpha.diff

####################################################################################

ifeq ($(BUILD), $(wildcard $(BUILD)))
all: all_builds
else
all: prepare_source all_builds
endif

all_builds: driver_build

clean:
	@echo -e "\033[32mCleaning $(MYNAME) ...\033[0m"
	$(Q)rm -rf $(BUILDDIR)

prepare_source:
	@echo -e "\033[32mPreparing the build directory for $(MYNAME) ...\033[0m"
	$(Q)mkdir -p $(BUILDDIR)
	$(Q)cd $(BUILDDIR); $(UNTAR) $(SRCDIR)/$(SRCTARBALL)
	$(Q)chmod +w -R $(BUILDDIR)
	$(Q)patch -p0 < $(SRCDIR)/$(DIFF)
	$(Q)cp $(SRCDIR)/$(EEPROMFILE) $(DRVPATH)
	if [ ! -f $(DRVPATH)/Makefile.rt2860v2_ap ]; then   \
		ln -s ./Makefile $(DRVPATH)/Makefile.rt2860v2_ap;   \
	fi
ifeq ($(strip $(RALINK_WAPI_SUPPORT)), y)
	$(Q)cp $(SRCDIR)/$(RALINK_WAPI_OBJ) $(DRVPATH)
	$(Q)cp $(SRCDIR)/$(RALINK_WAPI_DAEMON) $(DRVPATH)
#	$(Q)cp $(SRCDIR)/$(RALINK_WAPI_LIBRARY) $(DRVPATH)
endif

install:
	@echo -e "\033[32mInstalling $(MYNAME) ...\033[0m"
###	#$(Q)cp build/rt2860v2/rt2860v2_ap.o $(TARGET)/lib/modules/.
	$(Q)make -C $(DRVPATH) install -f Makefile.rt2860v2_ap
ifeq ($(strip $(ELBOX_PROGS_PRIV_RT2860V2_STA)),y)
	$(Q)make -C $(STAPATH) install -f Makefile.rt2860v2_sta
endif
.PHONY: all clean prepare_source

##################################################################################

mkdiff: driver_clean
	@echo -e "\033[32mGenerating diff file ...\033[0m"
	$(Q)rm -rf $(BUILDDIR)/bin
	$(Q)mv $(BUILDDIR) current
	$(Q)mkdir -p $(BUILD)
	$(Q)cd $(BUILD); $(UNTAR) $(SRCDIR)/$(SRCTARBALL)
	$(Q)mv $(BUILD) $(BUILD).original
	$(Q)mv current $(BUILD)
	$(Q)$(SRCDIR)/mkdiff $(BUILD).original $(BUILD) $(SRCDIR)/$(DIFF)
	$(Q)rm -rf $(BUILD).original

.PHONY: mkdiff

##################################################################################

driver_build:
	@echo -e "\033[32mBuilding $(MYNAME) driver ...\033[0m"
	$(Q)make -C $(DRVPATH) -f Makefile.rt2860v2_ap
ifeq ($(strip $(ELBOX_PROGS_PRIV_RT2860V2_STA)),y)
	$(Q)make -C $(STAPATH) -f Makefile.rt2860v2_sta
endif

driver_clean:
	@echo -e "\033[32mCleaning $(MYNAME) driver ...\033[0m"
	$(Q)make -C $(DRVPATH) clean -f Makefile.rt2860v2_ap
ifeq ($(strip $(ELBOX_PROGS_PRIV_RT2860V2_STA)),y)
	$(Q)make -C $(STAPATH) clean -f Makefile.rt2860v2_sta
endif

.PHONY: driver_build driver_clean

##################################################################################
