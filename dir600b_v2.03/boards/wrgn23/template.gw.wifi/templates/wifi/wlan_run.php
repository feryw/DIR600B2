#!/bin/sh
echo [$0] ... > /dev/console
<? /* vi: set sw=4 ts=4: */
$TROOT="/etc/templates";
$wireless_conf = "/var/run/RT2860.dat";

$timeset	= query("/runtime/timeset");
$lanif		= query("/runtime/layout/lanif");
$wlanif  	= query("/runtime/layout/wlanif");
$wlanmac 	= query("/runtime/layout/wlanmac");
$authtype	= query("/wireless/authtype");
$wlan_en	= query("/wireless/enable");
$wlan2lan	= query("/wireless/bridge/wlan2lan");	if ($wlan2lan!="0")	{$wlan2lan="1";}
$wlan_schedule	= query("/wireless/schedule/id");
/*generate_start==1:only create start shell,generate_start==0:only create stop shell,gererate_start==2:only create wlan_schedule.sh*/
if ($generate_start==1)
{
	echo "echo Start WLAN interface ".$wlanif." ... > /dev/console\n";
	if ($wlan_en!=1)
	{
		echo "echo WLAN is disabled ! > /dev/console\n";
		/* Turn off the WLAN LED when wireless is off */
		echo "usockc /var/run/fresetd_unixsock RADIO_OFF\n";	
		exit;
	}

	/* Generate Ralink RT2860 wireless driver configuration file*/
	echo "xmldbc -A ".$TROOT."/wifi/RT2860.php > ".$wireless_conf."\n";

	/* re-insert wireless module. */
	require($TROOT."/wifi/restart_wlan_driver.php");

	/* bridge to lan */
	echo "echo ".$wlan2lan." > /proc/net/br_forward_br0\n";
	//echo "brctl setfdctrl br0 ".$wlanif." off\n";

	/* Enable WPS ? */
	$HAPD_wps = 0;
	if (query("/runtime/func/wps")==1)
	{
		if (query("/wireless/wps/enable")==1)
		{
			$HAPD_wps = 1;
			$HAPD_eapuserfile = "/var/run/hostapd.wps.eap_user";
		}
	}

	/* Generate config file for hostapd */
	$HAPD_interface	= $wlanif;
	$HAPD_bridge	= $lanif;
	$HAPD_conf		= "/var/run/hostapd.".$HAPD_interface.".conf";
	anchor("/wireless");
	require($TROOT."/wifi/hostapd_used.php");
	$hostapd_conf	= $HAPD_conf;
	echo "hostapd ".$hostapd_conf." &\n";
	echo "wlxmlpatch -S ra0 /runtime/stats/wireless RADIO_ON RADIO_BLINK RT2800 > /dev/console &\n";
	echo "usockc /var/run/fresetd_unixsock RADIO_ON\n";
	if (query("/wireless/wps/enable")=="1") { echo "/etc/templates/wps.sh setie\n"; }
	echo "echo Start WLAN interface ".$wlanif." Done !!! > /dev/console\n";
}
else if($generate_start==0)
{
	echo "echo Stop WLAN interface ".$wlanif." ... > /dev/console\n";
	if (query("/wireless/wps/enable")=="1") { echo "killall wps > /dev/console\n"; }
	echo "killall wlxmlpatch > /dev/null 2>&1\n";
	echo "killall hostapd > /dev/null 2>&1\n";
	echo "ifconfig ".$wlanif." down\n";
}
else if($generate_start==2)
{
    if ($wlan_schedule !="" && $wlan_schedule !=0)
    {
        /* WLAN start with schedule setting. Start WLAN when time is not sync yet. */
        $sch_sock_path = "/var/run/schedule_usock";
        $UNIQUEID = $wlan_schedule;
        require("/etc/templates/rg/__schedule.php");

        $sch_cmd = "usockc ".$sch_sock_path." \"act=add";
        /*if ($timeset != 1)
        {
            $sch_cmd = $sch_cmd." et=1";
        }*/
        $sch_cmd = $sch_cmd." start=".$START." end=".$END." days=".$DAYS." cmd=[sh /etc/templates/wlan_schedule.sh]\" ";
        echo "usockc ".$sch_sock_path." \"act=del cmd=[sh /etc/templates/wlan_schedule.sh]\" >/dev/console \n";
        echo "sleep 1\n";
        echo $sch_cmd." > /dev/console\n";
        echo "echo Start Wlan with schedule! > /dev/console \n";
    }

}
?>
