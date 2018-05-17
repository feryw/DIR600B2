<?
echo "<!--__easy_setup_action.php\n";
echo "ACTION_POST = ".$ACTION_POST."\n";
if ($ACTION_POST=="welcome")
{	
	echo "enable_easy = ".$enable_easy."\n";
    set($G_EZY_PREFIX_ENABLE, $enable_easy); 

}else if ($ACTION_POST=="internet")
{
	
	echo "wan_type = ".$wan_type."\n";
	set($G_EZY_PREFIX_WAN."/wan_type", $wan_type);
	if($wan_type == "static")
	{
		echo "ipaddr = ".$ipaddr."\n";
		echo "netmask = ".$netmask."\n";
		echo "gateway = ".$gateway."\n";
		echo "dns1 = ".$dns1."\n";
		echo "dns2 = ".$dns2."\n";
		set($G_EZY_PREFIX_WAN."/ipaddr", $ipaddr);
		set($G_EZY_PREFIX_WAN."/netmask", $netmask);
		set($G_EZY_PREFIX_WAN."/gateway", $gateway);
		set($G_EZY_PREFIX_WAN."/dns1", $dns1);
		set($G_EZY_PREFIX_WAN."/dns2", $dns2);
	}else if($wan_type == "pppoe")
	{
		echo "username = ".$username."\n";
		echo "password = ".$password."\n";
		set($G_EZY_PREFIX_WAN."/username", $username);
		set($G_EZY_PREFIX_WAN."/password", $password);
	}
}else  if ($ACTION_POST=="wireless")
{
	echo "ssid = ".$ssid."\n";
	echo "security_type = ".$security_type."\n";
	echo "wpapsk1 = ".$wpapsk1."\n";
	set($G_EZY_PREFIX_WLAN."/ssid", $ssid);
	set($G_EZY_PREFIX_WLAN."/security_type", $security_type);
	set($G_EZY_PREFIX_WLAN."/wpapsk1", $wpapsk1);
}
echo "-->\n";
?>
