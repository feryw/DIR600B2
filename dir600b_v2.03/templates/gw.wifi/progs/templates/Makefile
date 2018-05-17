# vi: set sw=4 ts=4:
#
# Makefile for the template of gw.wifi
#

-include ../arch.mk
-include ../path.mk
-include ../.config

TMPS		:=	$(TARGET)/etc/templates
TMPS_SHELL	:=	dhcpd.sh dnrd.sh dyndns.sh dyndns_test.sh lan.sh logs.sh \
				rg.sh timezone.sh upnpd.sh wandown.sh wan.sh route.sh \
				wan_ppp.sh wanup.sh webs.sh ntp.sh onlanchange.sh qos.sh \
				lpd.sh wakeup.sh dnsmasq.sh firmware.sh

TMPS_SUBDIR	:=	ppp misc rg httpd upnpd 

all:

install:
	@echo -e "\033[32mInstall templates for gw.wifi ...\033[0m"
	$(Q)[ -d $(TMPS) ] || mkdir -p $(TMPS)
	$(Q)cp *.php $(TMPS)/.
	$(Q)for i in $(TMPS_SHELL); do cp $$i $(TMPS)/.; done
	$(Q)for i in $(TMPS_SUBDIR); do \
			[ -d $(TMPS)/$$i ] || mkdir -p $(TMPS)/$$i; \
			cp $$i/* $(TMPS)/$$i/.; \
	done
	$(Q)cp upnpd/.shell_action $(TMPS)/upnpd/.
	$(Q)make install_runtime
	@echo -e "\033[32mInstalling extensions ...\033[0m"
	$(Q)make install_hnap
	$(Q)make install_neaps
	$(Q)make install_module_script
	$(Q)make install_lld2d
	$(Q)make install_igmpproxy
	$(Q)make install_qos
	$(Q)make install_gzone
	$(Q)make install_scheduled
	$(Q)make install_dhcp
	$(Q)make install_dns
	$(Q)make install_appex
	$(Q)make install_proxyd
	$(Q)make install_flowmeter
	$(Q)make install_llmnresp
#	$(Q)make install_arp_spoof
	$(Q)chmod +x $(TMPS)/*.sh
	$(Q)find $(TMPS) -name .svn -type d | xargs rm -rf

clean:

.PHONY: all install clean

#########################################################################

install_runtime:
	$(Q)cp ./runtime/diagnostic		$(TARGET)/usr/sbin/.
	$(Q)cp ./runtime/stats			$(TARGET)/usr/sbin/.
ifeq ($(strip $(ELBOX_PROGS_GPL_SYSLOGD_V2)),y)
	$(Q)cp ./runtime/syslog_v2		$(TARGET)/usr/sbin/syslog
else
	$(Q)cp ./runtime/syslog			$(TARGET)/usr/sbin/.
endif
	$(Q)cp ./runtime/time			$(TARGET)/usr/sbin/.
	$(Q)cp ./runtime/wan			$(TARGET)/usr/sbin/.
ifeq ($(strip $(CONFIG_RGBIN_DEVCONF)),y)
	$(Q)cp ./runtime/sys.devconf	$(TARGET)/usr/sbin/sys
else
	$(Q)cp ./runtime/sys			$(TARGET)/usr/sbin/sys
endif

install_hnap:
ifeq ($(strip $(CONFIG_UPNPKITS_HNAP)),y)
	@echo -e "\033[32mInstalling HNAP support ...\033[0m"
	$(Q)[ -d $(TMPS)/hnap ] || mkdir -p $(TMPS)/hnap
	$(Q)cp hnap/*	$(TMPS)/hnap/.
	$(Q)cp hnap/.shell_action $(TMPS)/hnap/.
else
	@echo -e "\033[32mHNAP is not installing ...\033[0m"
endif

install_neaps:
ifeq ($(strip $(ELBOX_PROGS_PRIV_NEAPS)),y)
	@echo -e "\033[32mInstalling NEAPS support ...\033[0m"
	$(Q)cp neaps.sh	$(TMPS)/.
	$(Q)[ -d $(TMPS)/neaps ] || mkdir -p $(TMPS)/neaps
	$(Q)cp neaps/*	$(TMPS)/neaps/.
else
	@echo -e "\033[32mNEAPS is not installing ...\033[0m"
endif

install_module_script:
ifeq ($(strip $(ELBOX_LINUX_MAJOR)),2)
ifeq ($(strip $(ELBOX_LINUX_MINOR)),4)
	@echo -e "\033[32mInstalling modules.sh for Linux kernel 2.4 ...\033[0m"
	$(Q)cp modules_24.sh	$(TMPS)/modules.sh
endif
ifeq ($(strip $(ELBOX_LINUX_MINOR)),6)
	@echo -e "\033[32mInstalling modules.sh for Linux kernel 2.6 ...\033[0m"
	$(Q)cp modules_26.sh	$(TMPS)/modules.sh
endif
endif

.PHONY: install_hnap install_neaps install_module_script

#########################################################################
# Extensions
mkextensions:
	$(Q)[ -d $(TMPS)/extensions ] || mkdir -p $(TMPS)/extensions

# Installing LLD2 scripts.
install_lld2d:
	@echo -e "\033[32mInstalling LLD2 support ...\033[0m"
	$(Q)cp lld2d.sh							$(TMPS)/.
ifeq ($(strip $(ELBOX_PROGS_PRIV_LLD2D)),y)
	$(Q)make mkextensions
	$(Q)cp extensions/lld2d_run.php			$(TMPS)/extensions/.
endif

# Installing IGMP proxy scripts.
install_igmpproxy:
	@echo -e "\033[32mInstalling IGMP proxy support ...\033[0m"
	$(Q)cp igmpproxy.sh						$(TMPS)/.
	$(Q)cp igmpproxy_helper.sh				$(TMPS)/.
ifeq ($(strip $(ELBOX_PROGS_PRIV_IGMPPROXY)),y)
	$(Q)make mkextensions
	$(Q)cp extensions/igmpproxy_run.php		$(TMPS)/extensions/.
	$(Q)cp extensions/igmpproxy_helper.php	$(TMPS)/extensions/.
endif

# Installing QOS scripts.
install_qos:
	@echo -e "\033[32mInstalling QOS support ...\033[0m"
	$(Q)cp qos.sh							$(TMPS)/.
ifeq ($(strip $(ELBOX_PROGS_PRIV_TURBONAT_QOS)),y)
	$(Q)make mkextensions
	$(Q)cp extensions/qos_run.php			$(TMPS)/extensions/.
endif
ifeq ($(strip $(ELBOX_PROGS_PRIV_TURBONAT_UBCOM)),y)
	$(Q)cp extensions/pp.sh $(TMPS)/extensions/pp.sh
	$(Q)cp extensions/qosubcom_run.php $(TMPS)/extensions/qos_run.php
endif


# Installing Guest Zone scripts.
install_gzone:
	@echo -e "\033[32mInstalling Guest Zone support ...\033[0m"
	$(Q)cp gzone.sh							$(TMPS)/.
	$(Q)cp enable_gzone.sh					$(TMPS)/.
ifeq ($(strip $(ELBOX_TEMPLATE_GW_WIFI_GZONE)),y)
	$(Q)make mkextensions
	$(Q)cp extensions/gzone_run.php			$(TMPS)/extensions/.
	$(Q)cp extensions/enable_gzone_run.php	$(TMPS)/extensions/.
	$(Q)cp extensions/trafficmanager_run.php $(TMPS)/extensions/qos_run.php
endif

# Installing scheduled scripts.
install_scheduled:
	@echo -e "\033[32mInstalling scheduled support ...\033[0m"
	$(Q)cp scheduled.sh							$(TMPS)/.
ifeq ($(strip $(ELBOX_PROGS_PRIV_SCHEDULED)),y)
	$(Q)make mkextensions
	$(Q)cp extensions/scheduled_run.php			$(TMPS)/extensions/.
endif

# Installing dhcp scripts.
install_dhcp:
	$(Q)[ -d $(TMPS)/dhcp ] || mkdir -p $(TMPS)/dhcp
	$(Q)cp ./dhcp/*.php $(TMPS)/dhcp/
ifeq ($(strip $(ELBOX_MODEL_DIR300_ORANGE)),y)
	$(Q)cp -f dhcp/dhcpd.php_Orange $(TMPS)/dhcp/dhcpd.php
endif

# Installing dns scripts. For compatibility, just create a symbolic link insted of original dnrd.sh
install_dns:
ifeq ($(strip $(ELBOX_PROGS_GPL_DNSMASQ)),y)
	$(Q)rm -f $(TMPS)/dnrd.sh
	$(Q)ln -s ./dnsmasq.sh  $(TMPS)/dnrd.sh
endif

# Installing Appex
install_appex:
ifeq ($(strip $(ELBOX_PROGS_PRIV_APX)),y)
	$(Q)cp ./appex.sh  $(TMPS)/appex.sh
endif

# Installing arp-spoof scripts.
install_arp_spoof:
	@echo -e "\033[32mInstalling arp-spoof support ...\033[0m"
ifeq ($(strip $(ELBOX_ARP_SPOOF)),y)
	$(Q)make mkextensions
endif

# Install proxyd scripts.
install_proxyd:
	@echo -e "\033[32mInstalling proxyd support ...\033[0m"
ifeq ($(strip $(ELBOX_PROGS_PRIV_PROXYD)),y)
	[ -d $(TMPS)/proxyd ] || mkdir -p $(TMPS)/proxyd; \
	cp proxyd.sh $(TMPS)/.; \
	cp proxyd/proxyd_run.php $(TMPS)/proxyd/.; \
	cp proxyd/proxyd_conf.php $(TMPS)/proxyd/.; \
	cp proxyd/blocked.tmp $(TMPS)/proxyd/.; \
	cp proxyd/proxyd_url_conf.php $(TMPS)/proxyd/.; \
	cp -f proxyd/flush_blocking.php $(TMPS)/rg/.; \
	cp -f proxyd/__schedule_proxyd.php $(TMPS)/rg/.; \
	cp -f proxyd/rg_start.php $(TMPS)/rg/.; \
	cp -f proxyd/flush_main.php $(TMPS)/rg/.;
endif

# Installing Internet Usage Meter scripts.
install_flowmeter:
	@echo -e "\033[32mInstalling Internet Usage Meter support ...\033[0m"
ifeq ($(strip $(ELBOX_PROGS_PRIV_FLOWMETER)),y)
	[ -d $(TMPS)/flowmeter ] || mkdir -p $(TMPS)/flowmeter; \
	cp flowmeter.sh $(TMPS)/.; \
	cp flowmeter/flowmeter_run.php $(TMPS)/flowmeter/.; \
	cp flowmeter/flowmeter_helper.sh $(TMPS)/flowmeter/.; \
	cp flowmeter/mail_notify.php $(TMPS)/flowmeter/.; \
	cp flowmeter/ntf.sh $(TMPS)/flowmeter/.;
endif


# Installing llmnresp scripts.
install_llmnresp:
	@echo -e "\033[32mInstalling LLMNRESP support ...\033[0m"
ifeq ($(strip $(ELBOX_PROGS_PRIV_LLMNRESP)),y)
	$(Q)cp llmnresp.sh							$(TMPS)/.
	$(Q)make mkextensions
	$(Q)cp extensions/llmnresp_run.php			$(TMPS)/extensions/.
endif

.PHONY: mkextensions install_lld2d install_igmpproxy install_qos install_scheduled install_dhcp install_proxyd install_llmnresp
