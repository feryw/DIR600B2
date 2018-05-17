<?
include "/etc/services/WIFI/wifi.php";
fwrite("w",$START, "#!/bin/sh\n");
fwrite("w", $STOP, "#!/bin/sh\n");

/* Get the phyinf */
$phy1	= get_path_by_phy_uid(0, "WLAN-1");	if ($phy1 == "")	return;
$phy2	= get_path_by_phy_uid(0, "WLAN-2");	if ($phy2 == "")	return;
$phyrp1	= get_path_by_phy_uid(1, "WLAN-1");	if ($phyrp1 == "")	return;
$phyrp2	= get_path_by_phy_uid(1, "WLAN-2");	if ($phyrp2 == "")	return;
$wifi	= XNODE_getpathbytarget("/wifi", "entry", "uid", query($phy1."/wifi"), 0);
$en_wps = query($wifi."/wps/enable");
/* prepare needed config files */
$winf1	= query($phyrp1."/name");
$winf2	= query($phyrp2."/name");
$rtcfg	= "/var/run/RT2860.dat";
$hostapdcfg1		= "/var/servd/hostapd-".$winf1.".conf";
$hapd_eapuserfile1	= "/var/servd/hostapd-".$winf1.".wps.eap_user";
$hostapdcfg2		= "/var/servd/hostapd-".$winf2.".conf";
$upwifistatshlper	= "/etc/scripts/upwifistatshlper.sh";

/* config file of ralink driver. */
fwrite("a", $START, "xmldbc -P /etc/services/WIFI/rtcfg.php > ".$rtcfg."\n");

/* config file of hostapd. */
fwrite("a", $START,
	'xmldbc -P /etc/services/WIFI/hostapdcfg.php -V PHY_UID=WLAN-1 -V EAPUSERFILE='.$hapd_eapuserfile1.' > '.$hostapdcfg1.'\n'.
	'xmldbc -P /etc/services/WIFI/hostapdcfg.php -V PHY_UID=WLAN-2 > '.$hostapdcfg2.'\n'
	);

/************************* restart driver **************************/
/* reinstall wireless driver */
fwrite("a", $START,
	'rmmod rt2860v2_ap\n'.
	'insmod /lib/modules/rt2860v2_ap.ko\n'
	);
/* bringup the interface */
fwrite("a", $START,
	'ifconfig '.$winf1.' txqueuelen 1000\n'.
	'ifconfig '.$winf1.' up\n'.
	'ifconfig '.$winf2.' up\n');

	$idx = 0;
	foreach("/phyinf")
	{
		if(cut(query("uid"), 0, "-") == "WDS")
		{
			if(query("active")== 1 && query("media/peermac") != "")
			{
				$stsp = XNODE_getpathbytarget("/runtime", "phyinf", "uid", "WLAN-1", 0);
				$brinf      = query($stsp."/brinf");
				$brphyinf   = PHYINF_getphyinf($brinf);

				$addWDS = $addWDS."brctl addif ".$brphyinf." wds".$idx."\n";
				$delWDS = $delWDS."brctl delif ".$brphyinf." wds".$idx."\n";
				
				fwrite("a", $START,
					'ifconfig '.'wds'.$idx.' up\n'
					);
				fwrite("a", $STOP,
					'ifconfig '.'wds'.$idx.' down\n'
					);
				
				$idx++;	
			}
		}
	}

fwrite("a", $START,	
	'event WLAN-1.UP\n'.
	'event WLAN-2.UP\n'.
	$addWDS
	);
if (query($phy2."/active")==1)
{
	fwrite("a", $START, 'event "QUICKCONFIG.BLINK" add "event STATUS.AMBERBLINK"\n');
	fwrite("a", $STOP,  'event "QUICKCONFIG.BLINK" add "true"\n');
}
else
	fwrite("a", $START, 'ifconfig '.$winf2.' down\n');

fwrite("a", $START,
	'brctl setbwctrl br0 '.$winf1.' 900\n'.
	'echo 1 > /proc/net/br_forward_br0\n'
	);
fwrite("a", $STOP,
		$delWDS
		);
	if ($addWDS != "" && $delWDS != "")
	{
		fwrite("a", $START,
		'brctl stp '.$brphyinf.' on\n'
		);
		fwrite("a", $STOP,
		'brctl stp '.$brphyinf.' off\n'
		);
	}
/*******************************************************************/

/* start hostapd */
//fwrite("a", $_GLOBALS["START"], "hostapd ".$hostapdcfg1." ".$hostapdcfg2." &\n");
fwrite("a", $START, 'hostapd '.$hostapdcfg1.' &\n');
if ($en_wps == 1)
{
	fwrite("a", $START,
		'event WPSPBC.PUSH add "/etc/scripts/wps.sh pbc"\n'.
		'event WPSPIN add "/etc/scripts/wps.sh pin"\n'.
		'/etc/scripts/wps.sh setie\n'
		);
	fwrite("a", $STOP,
		'event WPSPBC.NONE\n'.
		'event WPSPBC.PUSH add true\n'
		);
}
/* start updatewifistats */
if (isfile($upwifistatshlper)==1) $upwifistatshlper=" -s ".$upwifistatshlper;
fwrite("a",$START,
	'updatewifistats -i '.$winf1.' -x '.$phy1.' '.$phy2.' -r '.$phyrp1.' '.$phyrp2.' -m RT2800'.$upwifistatshlper.' &\n'.
	'exit 0\n'
	);

/* stop updatewifistats & hostapd */
fwrite("a", $STOP,
	'killall updatewifistats\n'.
	'killall hostapd\n'
	);
/* remove interface from birdge */
fwrite("a", $STOP,
	'ifconfig '.$winf1.' down\n'.
	'ifconfig '.$winf2.' down\n'.
	'exit 0\n'
	);
?>
