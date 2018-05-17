<?
	$enable_easy = query($G_WIZ_PREFIX_ENABLE);
	$wan_type = query($G_WIZ_PREFIX_WAN."/wan_type");
	$ipaddr = query($G_WIZ_PREFIX_WAN."/ipaddr");
	$netmask = query($G_WIZ_PREFIX_WAN."/netmask");
	$gateway = query($G_WIZ_PREFIX_WAN."/gateway");
	$dns1 = query($G_WIZ_PREFIX_WAN."/dns1");
	$dns2 = query($G_WIZ_PREFIX_WAN."/dns2");

	$username = query($G_WIZ_PREFIX_WAN."/username");
	$password = query($G_WIZ_PREFIX_WAN."/password");
	
	
	$ssid = query($G_WIZ_PREFIX_WLAN."/ssid");
	$security_type = query($G_WIZ_PREFIX_WLAN."/security_type");
	$wpapsk1 = query($G_WIZ_PREFIX_WLAN."/wpapsk1");
?>
