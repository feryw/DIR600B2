<?
/* vi: set sw=4 ts=4: */
/*
 * The result of WPS registration protocol will be saved under /runtime/wps/setting/Xxx.
 * This file is used to move the setting to the proper place for the device.
 */
$dbg="/dev/console";

anchor("/runtime/wps/setting");
$scfg	= query("selfconfig");	fwrite($dbg, "selfconf = ".$scfg."\n");
$ssid	= query("ssid");		fwrite($dbg, "ssid     = ".$ssid."\n");
$atype	= query("authtype");	fwrite($dbg, "authtype = ".$atype."\n");
$etype	= query("encrtype");	fwrite($dbg, "encrtype = ".$etype."\n");
$defkey	= query("defkey");		fwrite($dbg, "defkey   = ".$defkey."\n");
$maddr	= query("macaddr");		fwrite($dbg, "macaddr  = ".$maddr."\n");
$newpwd	= query("newpassword");	fwrite($dbg, "newpwd   = ".$newpwd."\n");
$devpid	= query("devpwdid");	fwrite($dbg, "devpwdid = ".$devpid."\n");

/* If we started from Unconfigured AP (self configured),
 * change the setting to auto. */
if ($scfg == 1) { $atype = 7; $etype = 4; /* WPA/WPA2 PSK & TKIP+AES */ }

set("/wireless/ssid",		$ssid);
set("/wireless/authtype",	$atype);
set("/wireless/encrypttype",$etype);

if ($etype == 1)
{
	/* WEP keys */
	for ("key")
	{
		$idx = query("index");	fwrite("/dev/console", "key index = ".$idx."\n");
		$key = query("key");	fwrite("/dev/console", "key       = ".$key."\n");
		$fmt = query("format");	fwrite("/dev/console", "format    = ".$fmt."\n");
		$len = query("len");	fwrite("/dev/console", "len       = ".$len."\n");

		if ($idx < 5 && $idx > 0) { set("/wireless/wep/key:".$idx, $key); }
	}
	set("/wireless/wep/format", $fmt);
	set("/wireless/wep/length", $len);
	set("/wireless/wep/defkey", $defkey);
}
else
{
	/* We only need the first key */
	$idx = query("key:1/index");	fwrite("/dev/console", "key index = ".$idx."\n");
	$key = query("key:1/key");		fwrite("/dev/console", "key       = ".$key."\n");
	$fmt = query("key:1/format");	fwrite("/dev/console", "format    = ".$fmt."\n");
	$len = query("key:1/len");		fwrite("/dev/console", "len       = ".$len."\n");
	set("/wireless/wpa/format", $fmt);
	set("/wireless/wpa/key", $key);
}

set("/wireless/wps/configured", "1");
?>
