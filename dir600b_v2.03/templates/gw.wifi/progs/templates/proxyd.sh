#!/bin/sh
echo [$0] $1... > /dev/console
TROOT=`rgdb -i -g /runtime/template_root`
config_file="/var/etc/proxyd.conf"
[ "$TROOT" = "" ] && TROOT="/etc/templates"
case "$1" in
start|restart)
	[ -f /var/run/proxyd_stop.sh ] && sh /var/run/proxyd_stop.sh > /dev/console
	rgdb -A $TROOT/proxyd/proxyd_conf.php
	rgdb -A $TROOT/proxyd/proxyd_url_conf.php
	rgdb -A $TROOT/proxyd/proxyd_run.php -V config_file=$config_file
	sh /var/run/proxyd_start.sh > /dev/console
	;;
reload)
	if [ -f /var/run/proxyd_reload.sh ]; then
		sh /var/run/proxyd_reload.sh > /dev/console
	fi
	;;
stop)
	if [ -f /var/run/proxyd_stop.sh ]; then
		sh /var/run/proxyd_stop.sh > /dev/console
		rm -f /var/run/proxyd_stop.sh
	fi
	if [ -f /var/run/proxyd_reload.sh ]; then
		rm -f /var/run/proxyd_reload.sh
	fi
	;;
*)
	echo "usage: $0 {start|stop|restart|reload}"
	;;
esac
