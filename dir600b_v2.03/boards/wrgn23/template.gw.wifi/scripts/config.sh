#!/bin/sh
image_sign=`cat /etc/config/image_sign`

case "$1" in
start)
	echo "Mounting proc and var ..."
	mount -t proc none /proc
	mount -t sysfs sysfs /sys
	mount -t ramfs ramfs /var
	mkdir -p /var/etc /var/log /var/run /var/state /var/tmp /var/etc/ppp /var/etc/config /var/dnrd /var/etc/iproute2
	echo -n > /var/etc/resolv.conf
	echo -n > /var/TZ
	echo "127.0.0.1 hgw" > /var/hosts

	# Tune ip_conntrack hash table size
	echo 4096 > /sys/module/ip_conntrack/parameters/hashsize

	# if no PIN, generate one
	#pin=`devdata get -e pin`
	#[ "$pin" = "" ] && devdata set -e pin=`wps -g`

	# prepare db...
	echo "Start xmldb ..." > /dev/console
	xmldb -n $image_sign -t > /dev/console &
	sleep 1
	/etc/scripts/misc/profile.sh get
	/etc/templates/timezone.sh set
	/etc/templates/logs.sh
	sleep 1
	logger -p 192.1 "SYS:001"

	# bring up network devices
	ifconfig lo up

	env_wan=`devdata get -e wanmac`
	[ "$env_wan" = "" ] && env_wan="00:28:80:de:fa:01"
	ifconfig eth2 hw ether $env_wan up
	rgdb -i -s /runtime/wan/inf:1/mac "$env_wan"

	PANIC=`rgdb -i -g /runtime/func/panic_reboot`
	[ "$PANIC" != "" ] && echo "$PANIC" > /proc/sys/kernel/panic

	TIMEOUT=`rgdb -g /nat/general/tcpidletimeout`
	[ "$TIMEOUT" = "" ] && TIMEOUT=7200 && rgdb -s /nat/general/tcpidletimeout $TIMEOUT
	echo "$TIMEOUT" > /proc/sys/net/ipv4/netfilter/ip_conntrack_tcp_timeout_established

	# Setup VLAN
	vconfig set_name_type DEV_PLUS_VID_NO_PAD > /dev/console
	vconfig add eth2 0	> /dev/null 2>&1
	vconfig add eth2 2	> /dev/null 2>&1
	# Setup bridge
	brctl addbr br0 	> /dev/console
	brctl stp br0 off	> /dev/console
	brctl setfd br0 0	> /dev/console
	# Start up LAN interface & httpd
	ifconfig br0 0.0.0.0 up			> /dev/console
	/etc/templates/webs.sh start	> /dev/console
	;;
stop)
	umount /tmp
	umount /proc
	umount /var
	;;
esac
