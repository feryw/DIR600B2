#!/bin/sh
echo [$0] ... > /dev/console
if [ -f /etc/templates/ledctrl.sh ]; then
/etc/templates/ledctrl.sh STATUS ORG > /dev/console
fi
