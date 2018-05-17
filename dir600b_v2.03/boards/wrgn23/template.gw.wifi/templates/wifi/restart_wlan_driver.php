echo "restart WLAN driver ..." > /dev/console
# remove interface from bridge
brctl delif br0 ra0
# remove wireless driver
rmmod rt2860v2_ap
# install wireless driver
insmod /lib/modules/rt2860v2_ap.ko
# bring up the interfaces
ifconfig ra0 txqueuelen 1000
ifconfig ra0 up
# add interfaces to bridge
brctl addif br0 ra0
brctl setbwctrl br0 ra0 900
