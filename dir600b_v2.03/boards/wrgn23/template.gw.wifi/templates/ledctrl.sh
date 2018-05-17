#!/bin/sh
usock_file="/var/run/fresetd_unixsock"

if [ "$#" != "2" ]; then
	echo "$0 ledname action"
	echo "$0 [STATUS|INET] [GREEN|GREEN_BF|GREEN_BN|GREEN_BS|ORG|ORG_BF|ORG_BN|ORG_BS|OFF]"
	echo "example : $0 STATUS GREEN"
fi
usockc $usock_file $1_$2

