#!/bin/sh
echo [$0] $1 ... > /dev/console

while [ 1 ];do

echo "<===pp : wait 100 sec"
sleep 100
echo "<===pp : try to run ubcom"
killall ubcom
ubcom &
echo "<===pp : wait 500 sec"
sleep 500
BWUP="`rgdb -g /qos/bandwidth/upstream`"
if [ "$BWUP" = "102400" ];then 
echo "<===pp : BWUP still 102400 ubcom retry will be after 3 sec"
sleep 3;
else 
echo "<===pp : BWUP is $BWUP ubcom retry will be after 3600 sec"
###############################
date_time=`date | scut -f2 -n3`
echo "$date_time  |  auto uplink system found bandwidth $BWUP ." >> /var/log/messages
rgdb -i -s /runtime/qosoldbw $BWUP
###############################
sleep 3600;
rgdb -s /qos/bandwidth/upstream 102400
rgdb -s /qos/bandwidth/downstream 102400
sleep 3;submit QOS &
exit
fi

done
