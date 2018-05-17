<?
include "/etc/services/WIFI/wifi.php";
/* check the path of /runtime/phyinf, phyinf and wifi are existence. */
$phy1	= get_path_by_phy_uid(0, "WLAN-1");	if ($phy1 == "")	return;
/* Is the phyinf of main bssid active? */
$active = query($phy1."/active");
if ($active!=1) { wifisetup_error("8"); return; }

foreach("/phyinf")
{
	$u = cut(query("uid"), 0, "-");
	if ($u == "WLAN")
	{
		$stsp = XNODE_getpathbytarget("/runtime", "phyinf", "uid", query("uid"), 0);
		$brinf		= query($stsp."/brinf");
		$wlanphy	= query($stsp."/name");
		$brphyinf	= PHYINF_getphyinf($brinf);

		fwrite(a,$_GLOBALS["START"], "# stsp=".$stsp.", brinf=".$brinf.", wlanphy=".$wlanphy.", brphyinf=".$brphyinf."\n");
		if ($brphyinf!="" && $wlanphy!="")
		{
			fwrite(a, $_GLOBALS["STOP"], "brctl delif ".$brphyinf." ".$wlanphy."\n");
			fwrite(a, $_GLOBALS["START"],
				'event '.$name.'.UP add "brctl addif '.$brphyinf.' '.$wlanphy.'"\n');
		}
	}
}
fwrite("a", $_GLOBALS["START"], "service WIFI start\n");
fwrite("a", $_GLOBALS["STOP"],  "service WIFI stop\n");

/* restart IGMP proxy, WIFI enhancement */
fwrite("a",$START, "service MULTICAST restart\n");
fwrite("a",$STOP,  "service MULTICAST restart\n");

fwrite("a",$_GLOBALS["START"], "exit 0\n");
fwrite("a",$_GLOBALS["STOP"],  "exit 0\n");
?>
