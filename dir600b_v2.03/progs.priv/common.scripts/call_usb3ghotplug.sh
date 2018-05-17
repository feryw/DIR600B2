#!/bin/sh
#This script is designed to be called by udev/hotplug
#We need this script to call in usb3ghotplug.sh in background

#0 is this script name 
#1 is start/stop/start_dial/stop_dial
#2 is vid 
#3 is pid
#4 is kernel name
#5 is model name

if [ "`rgdb -i -g /runtime/func/wwan_3g`" = "1" ]; then 
echo "[$0] [$1] [$2] [$3] [$4] [$5] [$6].." > /dev/console
/etc/scripts/misc/usb3ghotplug.sh $1 $2 $3 $4 $5 &

exit 0
fi
