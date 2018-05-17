# Auto generated config file<?
include "/htdocs/phplib/xnode.php";
include "/htdocs/phplib/upnp.php";
include "/htdocs/phplib/trace.php";

$phy	= XNODE_getpathbytarget("", "phyinf", "uid", $PHY_UID, 0);
$phyrp	= XNODE_getpathbytarget("/runtime", "phyinf", "uid", $PHY_UID, 0);
$wifi	= XNODE_getpathbytarget("/wifi", "entry", "uid", query($phy."/wifi"), 0);
$br		= XNODE_getpathbytarget("", "inf", "uid", query($phyrp."/brinf"),0);
$brrp	= XNODE_getpathbytarget("/runtime", "phyinf", "uid", query($br."/phyinf"), 0);

$upnp_devtype = "urn:schemas-wifialliance-org:device:WFADevice:1";
$upnprp = UPNP_getdevpathbytype(query($phyrp."/brinf"), $upnp_devtype);
if ($upnprp == "")
{
	$USE_UPNP	= 0;
	$UUID		= "";
}
else
{
	$USE_UPNP	= 1;
	$UUID		= query($upnprp."/guid");
}
anchor($wifi);
/* WPS state */
if (query("wps/enable")=="1")
{
	if (query("wps/configured")==1)	$WPS_STATE = 2;
	else							$WPS_STATE = 1;
}
else
{
	$WPS_STATE = 0;
}

$SSID = query("ssid");
$auth = query("authtype");
$wepmode = query("encrtype");

if ($PARAM=="enrollee")
{
	/* use our own PIN */
	$DEVPWDID = "0x0000";
	/* Get configured PIN. */
	$DEVPWD = query("wps/pin");
	/* Factory default PIN. (label) */
	if ($DEVPWD == "") $DEVPWD = query("/runtime/devdata/pin");
}
else
{
	if ($PBC==1) $PIN = "00000000";
	else $PIN = query($phyrp."/media/wps/enrollee/pin");
	/* Get all the config setting for WPS */
	if ($PIN=="00000000")	$DEVPWDID = "0x0004";
	else					$DEVPWDID = "0x0000";

	$DEVPWD=$PIN;

	set($phyrp."/media/wps/enrollee/pin", "");

	/* Config ourself, if we are unconfigured. */
	if ($WPS_STATE == 1)
	{
		$SSID = query($wifi."/ssid");
		$auth = "WPAPSK";
		$wepmode = "TKIP";
	}
}

if		($auth == "OPEN")		$AUTH_TYPE="0x0001";	/* Open */
else if	($auth == "SHARED")		$AUTH_TYPE="0x0004";	/* Shared-Key */
else if	($auth == "WPA")		$AUTH_TYPE="0x0008";	/* WPA */
else if	($auth == "WPAPSK")		$AUTH_TYPE="0x0002";	/* WPA PSK */
else if	($auth == "WPA2")		$AUTH_TYPE="0x0010";	/* WPA2 */
else if	($auth == "WPA2PSK")	$AUTH_TYPE="0x0020";	/* WPA2 PSK */
else if	($auth == "WPA+2")		$AUTH_TYPE="0x0008";	/* WPA/WPA2 */
else if	($auth == "WPA+2PSK")	$AUTH_TYPE="0x0002";	/* WPA/WPA2 PSK */
//else if	($auth == "8021X")	$AUTH_TYPE="0x0001";	/* 802.1x */

if		($wepmode == "NONE")	$ENCR_TYPE="0x0001";	/* None */
else if	($wepmode == "WEP")		$ENCR_TYPE="0x0002";	/* WEP */
else if	($wepmode == "TKIP")	$ENCR_TYPE="0x0004";	/* TKIP */
else if	($wepmode == "AES")		$ENCR_TYPE="0x0008";	/* AES */
else if	($wepmode == "TKIP+AES")						/* TKIP+AES */
{
	if ($AUTH_TYPE=="0x0008" || $AUTH_TYPE=="0x0002")	$ENCR_TYPE="0x0004";
	else												$ENCR_TYPE="0x0008";
}
if ($wepmode == "WEP")
{
	$keyid = query("nwkey/wep/defkey");
	$NWKEY = query("nwkey/wep/key:".$keyid);
}
else if ($wepmode == "NONE")
{
	$NWKEY = "";
}
else
{
	$NWKEY = query("nwkey/psk/key");
}

$AUTH_TYPE_FLAGS="0x003f";
$ENCR_TYPE_FLAGS="0x000f";

?>
WPS_STATE=<?=$WPS_STATE?>
CONFIG_METHODS=0x0086
DEV_PASSWORD_ID=<?=$DEVPWDID?>
DEV_PASSWORD=<?=$DEVPWD?>
PRI_DEV_CATEGORY=6
PRI_DEV_OUI=0x0050F204
PRI_DEV_SUB_CATEGORY=1
CONN_TYPE_FLAGS=0x01
UUID=<?=$UUID?> 
VERSION=0x10
DEVICE_NAME=<?	echo query("/runtime/device/modelname"); ?>
MAC_ADDRESS=<?	echo query($phyrp."/macaddr");?>
MANUFACTURER=<?	echo query("/runtime/device/vendor"); ?>
MODEL_NAME=<?	echo query("/runtime/device/modelname"); ?>
MODEL_NUMBER=<?	echo query("/runtime/device/modelname"); ?>
SERIAL_NUMBER=00000000
RF_BAND=1
OS_VER=0x80000000
SSID=<?=$SSID?>
AUTH_TYPE_FLAGS=<?=$AUTH_TYPE_FLAGS?>
ENCR_TYPE_FLAGS=<?=$ENCR_TYPE_FLAGS?>
AUTH_TYPE=<?=$AUTH_TYPE?>
ENCR_TYPE=<?=$ENCR_TYPE?>
NW_KEY=<?=$NWKEY?>
USE_UPNP=<?=$USE_UPNP?> 
RESTART_AP_CMD=/etc/scripts/wps.sh restartap
WPS_HELPER=/etc/scripts/wps.sh
