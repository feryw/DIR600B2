<? /* vi: set sw=4 ts=4: */
include "/htdocs/phplib/xnode.php";
include "/htdocs/phplib/phyinf.php";
include "/htdocs/phplib/trace.php";

function wifisetup_error($errno)
{
	fwrite("a", $_GLOBALS["START"], "exit ".$errno."\n");
	fwrite("a", $_GLOBALS["STOP"],  "exit ".$errno."\n");
}

function get_path_by_phy_uid($is_runtime, $uid)
{
	if ($is_runtime == 1) $base = "/runtime";
	$p = XNODE_getpathbytarget($base, "phyinf", "uid", $uid, 0);
	if ($p == "")
	{
		wifisetup_error("9");
		return "";
	}
	return $p;
}
function get_path_by_wifi_uid($uid)
{
	$p = XNODE_getpathbytarget("/wifi", "entry", "uid", $uid, 0);
	if ($p == "")
	{
		wifisetup_error("9");
		return "";
	}
	return $p;
}
function wifisetup($name)
{
	/* check the path of /runtime/phyinf, phyinf and wifi are existence. */
	$phy1	= get_path_by_phy_uid(0, "WLAN-1");	if ($phy1 == "")	return;
	$phy2	= get_path_by_phy_uid(0, "WLAN-2");	if ($phy2 == "")	return;
	$phyrp1	= get_path_by_phy_uid(1, "WLAN-1");	if ($phyrp1 == "")	return;
	$phyrp2	= get_path_by_phy_uid(1, "WLAN-2");	if ($phyrp2 == "")	return;
	$wifi1	= get_path_by_wifi_uid(query($phy1."/wifi")); if ($wifi1 == "") return;
	$wifi2	= get_path_by_wifi_uid(query($phy2."/wifi")); if ($wifi2 == "") return;

	/* Is the phyinf of main bssid active? */
	$active = query($phy1."/active");
	if ($active!=1) { wifisetup_error("8"); return; }

	$stsp = XNODE_getpathbytarget("/runtime", "phyinf", "uid", $name, 0);
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

	fwrite("a", $_GLOBALS["START"], "service WIFI start\n");
	fwrite("a", $_GLOBALS["STOP"],  "service WIFI stop\n");
}
?>
