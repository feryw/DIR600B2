<?
/* vi: set sw=4 ts=4: ---------------------------------------------------------*/
$MY_NAME		="easy_setup_internet";
$MY_MSG_FILE	=$MY_NAME.".php";
$MY_ACTION	= "internet";
$WIZ_PREV		="welcome";
$WIZ_NEXT	= "wireless";
$NO_NEED_NEXT_LINK="1";

/* --------------------------------------------------------------------------- */
if ($ACTION_POST!="")
{
	require("/www/model/__admin_check.php");
	require("/www/__easy_setup_action.php");
	$ACTION_POST="";
	$TARGET_PAGE = $WIZ_NEXT;
	require("/www/easy_setup.php");
	exit;
}

$cfg_lan_ip		= query("/lan/ethernet/ip");
$cfg_lan_mask	= query("/lan/ethernet/netmask");


/* --------------------------------------------------------------------------- */
require("/www/model/__html_head.php");
require("/www/comm/__js_ip.php");
/* --------------------------------------------------------------------------- */
// get the variable value from rgdb.


$WAN_MODE = query("/wan/rg/inf:1/mode");
if($WAN_MODE == "2")
{
	$wan_type = "dhcp";
}
else if($WAN_MODE == "1")
{
	$wan_type = "static";
}
else if($WAN_MODE == "3")
{
	$wan_type = "pppoe";
}
anchor("/wan/rg/inf:1/static");
$ipaddr = query("ip");
$netmask = query("netmask");
$gateway = query("gateway");
$entry="/dnsrelay/server/";
$dns1 = query($entry."primarydns");
$dns2 = query($entry."secondarydns");
	
anchor("/wan/rg/inf:1/pppoe");
$username = get("h","user");
$password = $G_DEF_PASSWORD ;

if($ipaddr == "")
{$ipaddr = "0.0.0.0";}
if($netmask == "")
{$netmask = "255.255.255.0";}
if($gateway == "")
{$gateway = "0.0.0.0";}
if($dns1 == "")
{$dns1 = "0.0.0.0";}
if($dns2 == "")
{$dns2 = "0.0.0.0";}

/* --------------------------------------------------------------------------- */
?>
<script>
/* page init functoin */
function init()
{
	var wan_type = get_obj("wan_type");
	if("<?=$wan_type?>" == "dhcp")
		select_index(wan_type,"0");
	else if("<?=$wan_type?>" == "pppoe")
		select_index(wan_type,"1");
	else if("<?=$wan_type?>" == "static")
		select_index(wan_type,"2");
	wan_type.value = "<?=$wan_type?>" ;
	on_change_wan_type();
}
function check()
{
	var f = get_obj("frm");
	if(f.wan_type.value == "static")
	{
		if(check_static() == false)
			return false;
/*		f.username.disabled = true;
		f.password.disabled = true;
		f.password_v.disabled = true;
*/	}else if(f.wan_type.value == "pppoe")
	{
		if(check_pppoe() == false)
			return false;
	}
	return true;
}
function go_prev()
{
	self.location.href="<?=$POST_ACTION?>?TARGET_PAGE=<?=$WIZ_PREV?>";
}
function go_next()
{
}
function check_pppoe()
{

	var f = get_obj("frm");
	var net1, net2;
	if (is_blank(f.username.value))
	{
		alert("<?=$a_invalid_username?>");
		field_focus(f.username, "**");
		return false;
	}
	if (f.password.value != f.password_v.value)
	{
		alert("<?=$a_password_mismatch?>");
		field_focus(f.password, "**");
		return false;
	}
	return true;
}
function check_static()
{
	var f = get_obj("frm");
	var net1, net2;
	if (!is_valid_ip(f.ipaddr.value, 0))
	{
		alert("<?=$a_invalid_ip?>");
		field_focus(f.ipaddr, "**");
		return false;
	}
	if (!is_valid_mask(f.netmask.value))
	{
		alert("<?=$a_invalid_netmask?>");
		field_focus(f.netmask, "**");
		return false;
	}
	if (!is_valid_ip2(f.ipaddr.value, f.netmask.value))
	{
		alert("<?=$a_invalid_ip?>");
		field_focus(f.ipaddr, "**");
		return false;
	}
	//check if it is at the same subnet with LAN
	var lannet, wannet;
	lannet = get_network_id("<?=$cfg_lan_ip?>", "<?=$cfg_lan_mask?>");
	wannet = get_network_id(f.ipaddr.value, f.netmask.value);
	if (lannet[0] == wannet[0])
	{
		alert("<?=$a_invalid_ip?>");
		field_focus(f.ipaddr, "**");
		return false;
	}
	if (!is_valid_gateway(f.ipaddr.value, f.netmask.value, f.gateway.value, 0))
	{
		alert("<?=$a_invalid_ip?>");
		field_focus(f.gateway, "**");
		return false;
	}
	if(f.ipaddr.value == f.gateway.value)
	{
		alert("<?=$a_ip_equal_gateway?>");
		field_focus(f.ipaddr, "**");
		return false;		
	}
	net1 = get_network_id(f.ipaddr.value, f.netmask.value);
	net2 = get_network_id(f.gateway.value, f.netmask.value);
	if (net1[0] != net2[0])
	{
		alert("<?=$a_gw_in_different_subnet?>");
		field_focus(f.gateway, "**");
		return false;
	}

	if (!is_valid_ip(f.dns1.value, 0))
	{
		alert("<?=$a_invalid_ip?>");
		field_focus(f.dns1, "**");
		return false;
	}
	if (!is_valid_ip(f.dns2.value, 1))
	{
		alert("<?=$a_invalid_ip?>");
		field_focus(f.dns2, "**");
		return false;
	}
	return true;

}
function on_change_wan_type()
{
	var wan_type = get_obj("wan_type");
	var div_static = get_obj("div_static");
	var div_pppoe = get_obj("div_pppoe");
	div_pppoe.style.display = "none";
	div_static.style.display = "none";
	if(wan_type.value == "static")
	{
		div_static.style.display = "";
	}else if(wan_type.value == "pppoe")
	{
		div_pppoe.style.display = "";
	}
}
function out()
{
	document.write("<input type='button' name='exit' value=\"<?=$m_cancel?>\" onClick=\"exit_confirm('logout.php')\">&nbsp;");
}

</script>
<body onLoad="init();" <?=$G_BODY_ATTR?>>
<form name="frm" id="frm" method="post" action="<?=$POST_ACTION?>" onSubmit="return check();">
<input type="hidden" name="ACTION_POST" value="<?=$MY_ACTION?>">
<input type="hidden" name="TARGET_PAGE" value="<?=$MY_ACTION?>">
<?require("/www/model/__banner.php");?>
<table <?=$G_MAIN_TABLE_ATTR?>>
<tr valign=top>
	<td width=10%></td>
	<td id="maincontent" width=80%>
		<br>
		<div id="box_header">
		<h1><?=$m_config_connect?></h1>
		<br>
		<div id="div_wan_type" style="display:block">
		<table>
			<tr>
				<td class="r_tb" width=300><?=$m_internet_connection?>:&nbsp;</td>
				<td class="l_tb">
					<select name="wan_type" id="wan_type" onChange="on_change_wan_type()">
						<option value="static"><?=$m_static_ip?></option>
						<option value="dhcp" ><?=$m_dhcp?></option>
						<option value="pppoe"><?=$m_pppoe?></option>
					</select>
				</td>
			</tr>
		</table>
		</div>
		<div id="div_static" style="display:none">
		<table>
			<tr>
				<td class="r_tb" width=300><?=$m_ip_addr?>:&nbsp;</td>
				<td class="l_tb">
				<input type=text id="ipaddr" name="ipaddr" size=16 maxlength=15 value="<?=$ipaddr?>">
				</td>
			</tr>
			<tr>
				<td class="r_tb"><?=$m_sub_mask?>:&nbsp;</td>
				<td class="l_tb">				
				<input type=text id="netmask"  name="netmask" size=16 maxlength=15 value="<?=$netmask?>">
				</td>
			</tr>
			<tr>
				<td class="r_tb"><?=$m_gateway?>:&nbsp;</td>
				<td class="l_tb">				
				<input type=text id="gateway" name="gateway"  size=16 maxlength=15 value="<?=$gateway?>">
				</td>
			</tr>
			<tr>
				<td class="r_tb"><?=$m_primary_dns?>:&nbsp;</td>
				<td class="l_tb">				
				<input type=text id="dns1" name="dns1" size=16 maxlength=15 value="<?=$dns1?>">
				</td>
			</tr>
			<tr>
				<td class="r_tb"><?=$m_second_dns?>:&nbsp;</td>
				<td class="l_tb">				
				<input type=text id="dns2" name="dns2" size=16 maxlength=15 value="<?=$dns2?>">
				</td>
			</tr>
		</table>
		</div>
		<div id="div_pppoe" style="display:none">
		<table>
			<tr>
				<td class="r_tb" width=300><?=$m_user_name?>:&nbsp;</td>
				<td class="l_tb">
				<input type=text id="username" name="username" size=16 maxlength=255 value="<?=$username?>">
				</td>
			</tr>
			<tr>
				<td class="r_tb"><?=$m_passwd?>:&nbsp;</td>
				<td class="l_tb">				
				<input type="password" id="password"  name="password" size=16 maxlength=255 value="<?=$G_DEF_PASSWORD?>">
				</td>
			</tr>
			<tr>
				<td class="r_tb"><?=$m_confirm_passwd?>:&nbsp;</td>
				<td class="l_tb">				
				<input type="password" id="password_v" size=16 maxlength=255 value="<?=$G_DEF_PASSWORD?>">
				</td>
			</tr>		
		</table>
		</div>
		<div class=bl_tb>
		<? require($LOCALE_PATH."/dsc/dsc_".$MY_NAME.".php"); ?>
		</div>

<!-- ________________________________ Main Content Start ______________________________ -->
		<br>
		<center><script>prev("");next("");out();</script></center>
		<br>
<!-- ________________________________  Main Content End _______________________________ -->
		</div>
		<br>
	</td>
	<td width=10%></td>
</tr>
</table>
<?require("/www/model/__tailer.php");?>
</form>
</body>
</html>

