#!/bin/sh
echo [$0] $1 ... > /dev/console
TROOT="/etc/templates"
case "$1" in
"email_notify")
	$TROOT/flowmeter/ntf.sh
	;;
"flush_rg")
	$TROOT/rg.sh flowmeter
	;;
*)
	echo "usage: $0 {flush_rg|email_notify}"
	;;
esac
