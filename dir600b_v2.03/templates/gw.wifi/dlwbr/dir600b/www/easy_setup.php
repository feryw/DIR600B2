<?
require("/www/model/__g_var.php");
if ($TARGET_PAGE=="")
{
	del($G_EZY_PREFIX_WAN);
/*	set($G_EZY_PREFIX_WAN."/password", $G_DEF_PASSWORD);
	set($G_EZY_PREFIX_WAN."/pppoe/password", $G_DEF_PASSWORD);
	set($G_EZY_PREFIX_WAN."/pptp/password", $G_DEF_PASSWORD);
	set($G_EZY_PREFIX_WAN."/l2tp/password", $G_DEF_PASSWORD);
*/
	if(query("/sys/configdirty")=="1")
	{
		$TARGET_PAGE="current";
	}else
	{
		$TARGET_PAGE="welcome";
	}
}
//$POST_ACTION="easy_setup_welcome.php";
require("/www/easy_setup_".$TARGET_PAGE.".php");
?>
