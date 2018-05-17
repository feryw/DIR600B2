<?/* vi: set sw=4 ts=4: */
include "/htdocs/phplib/xnode.php";
include "/htdocs/phplib/trace.php";

$phy	= XNODE_getpathbytarget("", "phyinf", "uid", $PHY_UID, 0);
$phyrp	= XNODE_getpathbytarget("/runtime", "phyinf", "uid", $PHY_UID, 0);
$wifi	= XNODE_getpathbytarget("/wifi", "entry", "uid", query($phy."/wifi"), 0);
$br		= XNODE_getpathbytarget("", "inf", "uid", query($phyrp."/brinf"),0);
$brrp	= XNODE_getpathbytarget("/runtime", "phyinf", "uid", query($br."/phyinf"), 0);

anchor($phyrp);
$HAPD_inf	= query("name");
$HADP_br	= query($brrp."/name");
$HAPD_eapuserfile = $EAPUSERFILE;

anchor($wifi);
$authtype	= query("authtype");
$encrtype	= query("encrtype");
$Rport		= query("nwkey/eap/port");
$Rhost		= query("nwkey/eap/radius");
$Rsecret	= query("nwkey/eap/secret");
$wpapsk		= query("nwkey/psk/key");
//$rkeyint	= query("wpa/grp_rekey_interval");
$ssid		= query("ssid");
$HAPD_wps	= query("wps/enable");

if		($authtype=="OPEN")		{ $HAPD_wpa=0; $HAPD_ieee8021x=0; }	/* Open					*/
else if	($authtype=="SHARED")	{ $HAPD_wpa=0; $HAPD_ieee8021x=0; }	/* Shared-Key			*/
else if	($authtype=="WPA")		{ $HAPD_wpa=1; $HAPD_ieee8021x=1; }	/* WPA					*/
else if	($authtype=="WPAPSK")	{ $HAPD_wpa=1; $HAPD_ieee8021x=0; }	/* WPA-PSK				*/
else if	($authtype=="WPA2")		{ $HAPD_wpa=2; $HAPD_ieee8021x=1; }	/* WPA2					*/
else if	($authtype=="WPA2PSK")	{ $HAPD_wpa=2; $HAPD_ieee8021x=0; }	/* WPA2-PSK				*/
else if	($authtype=="WPA+2")	{ $HAPD_wpa=3; $HAPD_ieee8021x=1; }	/* WPA+WPA2				*/
else if	($authtype=="WPA+2PSK")	{ $HAPD_wpa=3; $HAPD_ieee8021x=0; }	/* WPA-PSK + WPA2-PSK	*/

/* Create config file for hostapd */
echo "driver=ralink\n";
echo "eapol_key_index_workaround=0\n";
echo "logger_syslog=0\nlogger_syslog_level=0\nlogger_stdout=0\nlogger_stdout_level=0\ndebug=0\n";

echo "interface="	.$HAPD_inf		."\n";
echo "bridge="		.$HADP_br		."\n";
echo "ssid="		.$ssid			."\n";
echo "wpa="			.$HAPD_wpa		."\n";
echo "ieee8021x="	.$HAPD_ieee8021x."\n";
echo "wps="			.$HAPD_wps		."\n";

/* Generate WPS config */
if ($HAPD_wps==1)
{
	echo "start_enrollee_cmd=/etc/scripts/wps.sh eap:registrar &\n";
	echo "start_registrar_cmd=/etc/scripts/wps.sh eap:enrollee &\n";
	echo "eap_user_file=".$HAPD_eapuserfile."\n";

	fwrite("w", $HAPD_eapuserfile, "\"WFA-SimpleConfig-Registrar-1-0\" WPS\n");
	fwrite("a", $HAPD_eapuserfile, "\"WFA-SimpleConfig-Enrollee-1-0\" WPS\n");
}

if ($HAPD_wpa > 0)
{
	if		($rkeyint!="")			{ echo "wpa_group_rekey=".$rkeyint."\n";}
	if		($encrtype=="TKIP")		{ echo "wpa_pairwise=TKIP\n";		}
	else if	($encrtype=="AES")		{ echo "wpa_pairwise=CCMP\n";		}
	else if	($encrtype=="TKIP+AES")	{ echo "wpa_pairwise=TKIP CCMP\n";	}

	if ($HAPD_ieee8021x == 1)
	{
		echo "wpa_key_mgmt=WPA-EAP\n";
		echo "auth_server_addr=".$Rhost."\n";
		echo "auth_server_port=".$Rport."\n";
		echo "auth_server_shared_secret=".$Rsecret."\n";
	}
	else
	{
		echo "wpa_key_mgmt=WPA-PSK\n";
		if (query("nwkey/psk/passphrase")=="1")	{echo "wpa_passphrase=".$wpapsk."\n";}
		else									{echo "wpa_psk=".$wpapsk."\n";}
	}
}
?>

