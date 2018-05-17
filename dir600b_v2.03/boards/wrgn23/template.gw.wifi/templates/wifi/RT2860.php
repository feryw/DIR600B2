<?
/******************************************************************************************************************** 
 *	NOTE: 
 *		This configuration generator is based on Ralink RT2800 Linux SoftAP Drv1.5.0.0 Release Note and User's Guide.	 
 *		Package Name : 2007_0801_RT2860_Linux_SoftAP_Drv1.5.0.0.tgz
 ********************************************************************************************************************/

$templates_path = "/etc/templates/wifi/";
anchor("/wireless");

/* -------------------------------------------- get config ----------------------------------------*/
/* country code */
$ccode = query("/runtime/nvram/countrycode");
if( $ccode == "" )
{
	$ccode = query("/runtime/layout/countrycode");
}
if ($ccode=="")	{$ccode="GB";}
/* The country code "DI" and "AU" are Hight Power version of "US" and "GB" respectively. */
/* They are defined by customer and not defined in driver */
if ($ccode=="DI") {$ccode="US";}
if ($ccode=="AU") {$ccode="GB";}
if		($ccode == "392" || $ccode == "JP" )	{$c_region="1";	$c_string="JP";}	/* MKK */
else if ($ccode == "826" || $ccode == "GB" )	{$c_region="1";	$c_string="GB";}	/* UK */
else if ($ccode == "152" || $ccode == "CL")		{$c_region="1";	$c_string="CL";}	/* CHILE, for South America */
else						{$c_region="0";	$c_string="US";}	/* FCC */
/* wlanmode */
/* wlanmode (xmldb) : 1:b only, 2:g only, 3:b+g, 4:n only, 5:b+n, 6:g/n mix , 7:b+n+g , 8:a only, 12:a/n mix */
/* wlanmode (ralink): 0:b+g, 1: b only, 2:a only, 3: a+b+g, 4:g only, 5:a+b+g+n, 6:n only, 7:g+n, 8:a+n, 9:b+g+n, 10:a+g+n */
$wlanmode	= query("wlanmode");
$wmm		= query("wmm");
$model		= query("/sys/modelname");
if 		($wlanmode=="1")	{$wlan_mode="1"; $basic_rate="3";				}	/* 11b 		*/
else if ($wlanmode=="2")	{$wlan_mode="4"; $basic_rate="351";				}	/* 11g		*/
else if ($wlanmode=="3")	{$wlan_mode="0"; $basic_rate="15";				}	/* 11g/b	*/
else if ($wlanmode=="4")	{$wlan_mode="6"; $basic_rate="15";	if($model!="DIR-605") { $wmm = "1";	} }	/* 11n		*/
else if ($wlanmode=="6")	{$wlan_mode="7"; $basic_rate="15";	if($model!="DIR-605") { $wmm = "1";	} }	/* 11n/g	*/
else if ($wlanmode=="7")	{$wlan_mode="9"; $basic_rate="15";	if($model!="DIR-605") { $wmm = "1";	} } /* 11n/g/b	*/
else 						{$wlan_mode="9"; $basic_rate="15";	if($model!="DIR-605") { $wmm = "1"; } }	/* Default: 11n/g/b */
/* 1. When 11n is enabled, wmm must be enabled. 2. When wmm is enabled, the txburst must be disabled. */
if ($wmm == "1")	{$txburst = "0";}	else	{$wmm="0"; $txburst ="1";}

/* -------- RT2860AP.dat -------*/
echo "Default"."\n";	/* The word of "Default" must not be removed. */
echo "CountryRegion="			.$c_region	."\n";	
echo "CountryCode="				.$c_string	."\n";
echo "WirelessMode="			.$wlan_mode	."\n";
echo "BasicRate="				.$basic_rate."\n";	
echo "WmmCapable="				.$wmm		."\n";
/* If wmm is enabled, wmm power saving should be enabled too. */
echo "APSDCapable="				.$wmm		."\n";
echo "TxBurst="					.$txburst	."\n";
echo "HT_TxStream=1"						."\n";
echo "HT_RxStream=1"						."\n"; 
/*************************************************************************************
 * Ralink:	1. no matter wmm is diabled or enabled, the WMM parameters should be set.
 *			2. Both of parameter for STA and AP have to be set. (from Ralink)
 *************************************************************************************/
if ($wlanmode=="1") {$WMM_PAR="11b";}
else				{$WMM_PAR="11ag";}
require($templates_path."/wmm_paramters.php");

/* -------------------------------------------- get config ----------------------------------------*/
$ssid		= query("ssid");
$h_ssid		= query("ssidhidden");			if ($h_ssid!="1")	{$h_ssid	="0";}
$auto_ch	= query("autochannel");			if ($auto_ch=="")	{$auto_ch	="0";}
$channel	= query("channel");				if ($channel=="")	{$channel	="6";}
if ($auto_ch=="1")	{$channel="";}  /* If auto channel is enabled, channel value should not to be set */
$b_intval	= query("beaconinterval");		if ($b_intval=="")	{$b_intval	="100";}
$dtim		= query("dtim");				if ($dtim=="")		{$dtim		="1";}
$rtslength	= query("rtslength");			if ($rtslength=="")	{$rtslength	="2346";}
$fraglength	= query("fraglength");			if ($fraglength==""){$fraglength="2346";}
$txrate		= query("txrate");
$preamble	= query("preamble");			if ($preamble=="")	{$preamble	="2";	}/* default:long preamble */
if ($wlanmode == "4")
{
	if (query("mcs/auto")=="1")	{$mcs_index="33";}
	else						{$mcs_index = query("mcs/index");	if ($mcs_index=="") {$mcs_index=33;}	}
}
else if ($wlanmode=="1" || $wlanmode=="2") /* 11b, or 11g only.*/
{
	/* Ralink is no more support txtrae parameter,
	* instead, tx rate is set by  "WirelessMode" and "HT_MCS" combination.		*/
	/*				11B					11G 									*/
	/*--------------------------------------------------------------------------*/
	if		($txrate == "1"		||	$txrate == "6")		{$mcs_index = 0; }
	else if ($txrate == "2"		||	$txrate == "9")		{$mcs_index = 1; }
	else if ($txrate == "5.5"	||	$txrate == "12")	{$mcs_index = 2; }
	else if ($txrate == "11"	||	$txrate == "18")	{$mcs_index = 3; }
	else if (						$txrate == "24")	{$mcs_index = 4; }
	else if (						$txrate == "36")	{$mcs_index = 5; }
	else if (						$txrate == "48")	{$mcs_index = 6; }
	else if (						$txrate == "54")	{$mcs_index = 7; }
	else												{$mcs_index = 33;}/* set default as "auto" */
}
else
{
	$mcs_index = 33;
}
/* -------- RT2860AP.dat -------*/
echo "BssidNum=1"						."\n";
echo "SSID="				.$ssid		."\n";
echo "HideSSID="			.$h_ssid	."\n";
echo "AutoChannelSelect="	.$auto_ch	."\n";
if ($channel != "" )	{	echo "Channel="				.$channel	."\n";	}
echo "BeaconPeriod="		.$b_intval	."\n";
echo "DtimPeriod="			.$dtim		."\n";
echo "RTSThreshold="		.$rtslength	."\n";
echo "FragThreshold="		.$fraglength."\n";
echo "HT_MCS="				.$mcs_index	."\n";

/* -------------------------------------------- get config ----------------------------------------*/
/* tx power */
$txpower = query("txpower");
if		($txpower=="2")	{$txpower="45";}
else if ($txpower=="3")	{$txpower="25";}
else if ($txpower=="4")	{$txpower="10";}
else if ($txpower=="5")	{$txpower="5";}
else					{$txpower="95";}
/* bridge */
$wlan2wlan = query("bridge/wlan2wlan");		if ($wlan2wlan=="0") {$no_forwarding="1";}	else {$no_forwarding="0";}

/* -------- RT2860AP.dat -------*/
/**********************************************************************************************
 * for passing the WiFi test in secter 4.2.3.2.2. 
 * The default value of preamble in Ralink driver is auto (long preamble).
 * When the preamble is long preamble, the result of 4.2.3.2 MA8 is 
 * 3.543/3.430 (Broadcom-G, Intel-B(RTS-256), but the throughput must be more than 4.140/2.253.
 * Ralink said to set the preamble as short to pass this item.
 * (set the TxPreabmle=1.)
 * Per D-Link's rqeust, preamble still need to be changeable in order to work with legacy client,
 * so set default value to short preamble.
 **********************************************************************************************/
if ($preamble=="2")	{$preamble="0";}
else 				{$preamble="1";}
echo "TxPreamble="		.$preamble	."\n";
echo "TxPower="			.$txpower	."\n";
/* bridge */
echo "NoForwarding="	.$no_forwarding."\n";
echo "NoForwardingBTNBSSID=1\n";
echo "WiFiTest=1\n";
echo "ShortSlot=1\n";
echo "CSPeriod=6\n";
echo "PktAggregate=1\n";
/* -------------------------------------------- get config ----------------------------------------*/
/* auth and encrytption */
/* auth_type:
	0: open system, 1:share key,
	2: WPA,			3: WPA-PSK,
	4: WPA2,		5: WPA2-PSK,
	6: WPA + WPA2,	7: WPA-PSK + WPA2-PSK,
	8:802.1X
*/
$authtype		= query("authtype");
$encrypttype	= query("encrypttype");
if		($authtype == "0")
{
	$STR_AUTH="OPEN";
	if ($encrypttype=="1")	{ $STR_ENC = "WEP";				$NEED_WEP	= "1";}
	else					{ $STR_ENC = "NONE";}
}
else if ($authtype == "1")	{ $STR_AUTH = "SHARED";			$NEED_WEP	= "1";	$STR_ENC = "WEP";}
else if ($authtype == "2")	{ $STR_AUTH = "WPA";			$NEED_RADIUS= "1";}
else if ($authtype == "3")	{ $STR_AUTH = "WPAPSK";			$NEED_PSK	= "1";}
else if ($authtype == "4")	{ $STR_AUTH = "WPA2";			$NEED_RADIUS= "1";}
else if ($authtype == "5")	{ $STR_AUTH = "WPA2PSK";		$NEED_PSK	= "1";}
else if ($authtype == "6")	{ $STR_AUTH = "WPA1WPA2";		$NEED_RADIUS= "1";}
else if ($authtype == "7")	{ $STR_AUTH = "WPAPSKWPA2PSK";	$NEED_PSK	= "1";}

if ($authtype >= 2)
{
	if 		($encrypttype=="2")	{$STR_ENC = "TKIP";}
	else if	($encrypttype=="3") {$STR_ENC = "AES";}
	else						{$STR_ENC = "TKIPAES";}
}

/* -------- RT2860AP.dat -------*/
/* Authentication */
echo "AuthMode="	.$STR_AUTH. "\n";
echo "EncrypType="	.$STR_ENC.	"\n";

if ($NEED_WEP=="1")
{
	$defkey_index	= query("wep/defkey");
	$weptype		= query("wep/format");		if ($weptype != "1"){$weptype="0";}
	$key			= query("wep/key:".$defkey_index);
	echo "DefaultKeyID="				.$defkey_index	."\n";
	echo "Key".$defkey_index."Type="	.$weptype		."\n";
	echo "Key".$defkey_index."Str="		.$key			."\n";
}

/*
if ($NEED_RADIUS=="1" || $NEED_PSK =="1")
{
	$p_wpa="/wireless/wpa/";
	if ($NEED_RADIUS=="1")
	{
		echo "RADIUS_Server="	.query($p_wpa."radius:1/host")		."\n";
		echo "RADIUS_Port="		.query($p_wpa."radius:1/port")		."\n";
		echo "RADIUS_Key="		.query($p_wpa."radius:1/secret")	."\n";
	}
	else
	{
		echo "WPAPSK="			.query($p_wpa."key")				."\n";
	}

	$rekey_interval	= query($p_wpa."grp_rekey_interval");
	$pmk_period		= query($p_wpa."pmkperiod");
	$rekey_method	= query($p_wpa."rekeymethod");
	echo "IEEE8021X=0"."\n";
	echo "RekeyInterval="		.$rekey_interval			."\n";
	echo "RekeyMethod="			.$pmk_period				."\n";
	echo "PMKCachePeriod="		.$rekey_method				."\n";
	echo "own_ip_addr="			.query("/lan/ethernet/ip")	."\n";
}
*/

/* -------------------------------------------- get config ----------------------------------------*/
/* 11n short guard interval */
$s_gi_11n = query("shortguardinterval");	if ($s_gi_11n=="")	{$s_gi_11n	="0";}
/* 11n bandwidth and extension channel */
/* bandwidth: 1:20MHz, 2: 40MHz, 3:20 MHz + 40 MHz*/
$bw_11n	 = query("bandwidth");
if ($bw_11n=="2" || $bw_11n=="3")
{
	$ht_bw="1";
	if ($channel <= 4)	{$ht_extcha=1;}
	else				{$ht_extcha=0;}
}
else
{
	$ht_bw="0";
}
/* -------- RT2860AP.dat -------*/
/* HT (11n) */
echo "HT_HTC=1"			."\n";
echo "HT_RDG=1"			."\n";
echo "HT_LinkAdapt=0"	."\n";
echo "HT_OpMode=0"		."\n";
echo "HT_MpduDensity=5"	."\n";
echo "HT_AutoBA=1"		."\n";
echo "HT_AMSDU=0"		."\n";
echo "HT_BAWinSize=64"	."\n";
echo "HT_STBC=1"		."\n";
echo "HT_BADecline=0"	."\n";
echo "HT_PROTECT=1"		."\n";
echo "HT_GI="			.$s_gi_11n	."\n";
echo "HT_BW="			.$ht_bw		."\n";
if ($ht_bw=="1")
{
	echo "HT_EXTCHA="	.$ht_extcha	."\n";
}
/* -------------------------------------------- get config ----------------------------------------*/
/* Access Control List */
$acl_policy=query("acl/mode");
if ($acl_policy==""){$acl_policy="0";}
$acl_str="";
$acl_div="";
$acl_index=0;
for("/wireless/acl/mac")
{
	$acl_index++;
	$acl_path="/wireless/acl/mac:".$acl_index;
	$en	=query($acl_path."/enable");
	if($en=="1")
	{
		$acl_str=$acl_str.$acl_div.query($acl_path);
		$acl_div=";";
	}
}
/* -------- RT2860AP.dat -------*/
echo "AccessPolicy0="			.$acl_policy	."\n";
if($acl_str!="")
{
	echo "AccessControlList0="	.$acl_str		."\n";
}
/* -------- RT2860AP.dat end -------*/
?>
