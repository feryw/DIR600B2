#!/bin/sh
case "$1" in
start)
	echo "start wlan....."
	if [ -f /var/run/wlan_start.sh ];then
		sh /var/run/wlan_start.sh
	else 
		echo "/var/run/wlan_start.sh is not exist...."
	fi
	;;
stop)
	echo "shut down wlan....."
	if [ -f /var/run/wlan_stop.sh ];then
		sh /var/run/wlan_stop.sh
	else 
		echo "/var/run/wlan_stop.sh is not exist...."
	fi
	;;
*)
	echo "usage: wan_schedule.sh [start|stop]" >/dev/console
	;;
esac
