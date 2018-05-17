#!/bin/sh
echo [$0] ... > /dev/console
TROOT="/etc/templates"
[ ! -f $TROOT/flowmeter/flowmeter_run.php ] && exit 0
xmldbc -A $TROOT/flowmeter/flowmeter_run.php > /var/run/flowmeter_start.sh
sh /var/run/flowmeter_start.sh
rm -f /var/run/flowmeter_start.sh
