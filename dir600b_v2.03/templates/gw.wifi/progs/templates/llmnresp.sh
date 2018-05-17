#!/bin/sh
echo [$0] ... > /dev/console
TROOT="/etc/templates"
[ ! -f $TROOT/extensions/llmnresp_run.php ] && exit 0
case "$1" in
start|restart)
	[ -f /var/run/llmnresp_stop.sh ] && sh /var/run/llmnresp_stop.sh > /dev/console
	rgdb -A $TROOT/extensions/llmnresp_run.php -V generate_start=1 > /var/run/llmnresp_start.sh
	rgdb -A $TROOT/extensions/llmnresp_run.php -V generate_start=0 > /var/run/llmnresp_stop.sh
	sh /var/run/llmnresp_start.sh > /dev/console
	;;
stop)
	if [ -f /var/run/llmnresp_stop.sh ]; then
		sh /var/run/llmnresp_stop.sh > /dev/console
		rm -f /var/run/llmnresp_stop.sh
	fi
	;;
*)
	echo "usage: lan.sh {start|stop|restart}"
	;;
esac
