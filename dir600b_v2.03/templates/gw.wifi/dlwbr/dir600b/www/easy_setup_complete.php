<?
/* vi: set sw=4 ts=4: ---------------------------------------------------------*/
$MY_NAME		="easy_setup_complete";
$MY_MSG_FILE	=$MY_NAME.".php";
$MY_ACTION	= "complete";
$WIZ_PREV		="wireless";
$WIZ_NEXT	= "";
$NO_NEED_NEXT_LINK="1";
/* --------------------------------------------------------------------------- */

$enable_easy = query($G_EZY_PREFIX_ENABLE);
$wan_type = query($G_EZY_PREFIX_WAN."/wan_type");
$ipaddr   = query($G_EZY_PREFIX_WAN."/ipaddr");
$netmask  = query($G_EZY_PREFIX_WAN."/netmask");
$gateway  = query($G_EZY_PREFIX_WAN."/gateway");
$dns1     = query($G_EZY_PREFIX_WAN."/dns1");
$dns2     = query($G_EZY_PREFIX_WAN."/dns2");
$username = query($G_EZY_PREFIX_WAN."/username");
$password = query($G_EZY_PREFIX_WAN."/password");


$ssid          = query($G_EZY_PREFIX_WLAN."/ssid");
$security_type = query($G_EZY_PREFIX_WLAN."/security_type");
$wpapsk1       = query($G_EZY_PREFIX_WLAN."/wpapsk1");

if ($ACTION_POST!="")
{
	echo "<!--\n";
	require("/www/model/__admin_check.php");
	$dirty_enable = "0";
	if(query("/sys/easysetup/enable") != $enable_easy){$dirty_enable++; set("/sys/easysetup/enable",$enable_easy);}
	$dirty_wan = 0;
	
	
	
	//<-------------------------- WAN -------------------------------------------->
	$rb_dirty="0";
	$enable_ap_mode="";
	$pppoe_phy="";
	$pptp_phy="";
	
	
	$bridge_setting = query("/bridge");
	if ($enable_ap_mode!=1) {$enable_ap_mode=0;}
	if ($bridge_setting!=1) {$bridge_setting=0;}
	if ($bridge_setting!=$enable_ap_mode) {$rb_dirty++; set("/bridge", $enable_ap_mode);}
	
	if ($pppoe_phy!="1" && query("/wan/rg/inf:2/mode")!="")
	{
		$dirty_wan++;
		//del("/wan/rg/inf:2");
		set("/wan/rg/inf:1/pppoe/mppe/enable", "0");
		$pppoe_mppe=0;
	}
	if ($pptp_phy!="1" && query("/wan/rg/inf:1/pptp/physical")==1)
	{
		$dirty_wan++;
		set("/wan/rg/inf:1/pptp/physical","0");
		set("/wan/rg/inf:1/pptp/mppe/enable", "0");
		//$pptp_mppe=0;
	}
	
	/* disable netsniper */
	if(query("/runtime/func/netsniper")=="1")
	{
		if(query("/wan/rg/inf:1/netsniper_enable") != 0) {$dirty_wan++; set("/wan/rg/inf:1/netsniper_enable", 0);}
	}
	
	
	//static
	if ($wan_type == "static")
	{
		$clonemac="";
		$mtu="1500";
		
	//w8021x	
		$st_eap_type="1";
		$st_authentication="0"
		$enable_st8021x="";
		$st_username="";
		$st_password="";
		
		
		
		
		echo "ipaddr=".$ipaddr."\n";
		echo "netmask=".$netmask."\n";
		echo "gateway=".$gateway."\n";
		echo "clonemac=".$clonemac."\n";
		echo "dns1=".$dns1."\n";
		echo "dns2=".$dns2."\n";
		echo "mtu=".$mtu."\n";
			
	
		$entry="/wan/rg/inf:1/static/";
		if (query("/wan/rg/inf:1/mode")	!="1")	{$dirty_wan++; set("/wan/rg/inf:1/mode", "1");}
		if (query($entry."ip")		!=$ipaddr)	{$dirty_wan++; set($entry."ip", $ipaddr);}
		if (query($entry."netmask")	!=$netmask)	{$dirty_wan++; set($entry."netmask", $netmask);}
		if (query($entry."gateway")	!=$gateway)	{$dirty_wan++; set($entry."gateway", $gateway);}
		if (query($entry."clonemac")!=$clonemac){$dirty_wan++; set($entry."clonemac", $clonemac);}
		if (query($entry."mtu")		!=$mtu)		{$dirty_wan++; set($entry."mtu", $mtu);}
		$entry="/dnsrelay/server/";
		if (query($entry."primarydns")	!=$dns1){$dirty_wan++; set($entry."primarydns", $dns1);}
		if (query($entry."secondarydns")!=$dns2){$dirty_wan++; set($entry."secondarydns", $dns2);}
		$entry="/w8021x/";
		if (query($entry."sttype")!=$st_eap_type){$dirty_wan++; set($entry."sttype", $st_eap_type);}
		if (query($entry."stauth")!=$st_authentication){$dirty_wan++; set($entry."stauth", $st_authentication);}
		if (query($entry."stenable")!=$enable_st8021x){$dirty_wan++; set($entry."stenable", $enable_st8021x);}
		if (query($entry."stuser")!=$st_username){$dirty_wan++;	set($entry."stuser", $st_username);}
		if (query($entry."stpassword")!=$st_password){$dirty_wan++; set($entry."stpassword", $st_password);}
	
	//dhcp
	}else if($wan_type == "dhcp")
	{
		$hostname="DIR-600";
		$unicast="";
		$clonemac="";
		$dns1="";
		$dns2="";
		$mtu="";
		$mtu="1500";
		$dh_eap_type="1";
		$dh_authentication="0";
		$enable_dh8021x="";
		$dh_username="";
		$dh_password="";
		$dh_password_v="";
		if ($unicast != "1") {$unicast = "0";}
		echo "hostname=".$hostname."\n";
		echo "unicast=".$unicast."\n";
		echo "clonemac=".$clonemac."\n";
		echo "dns1=".$dns1."\n";
		echo "dns2=".$dns2."\n";
		echo "mtu=".$mtu."\n";
	
		$entry="/wan/rg/inf:1/dhcp/";
		if (query("/wan/rg/inf:1/mode") != "2")	{$dirty_wan++; set("/wan/rg/inf:1/mode", "2");}
		if (query("/sys/hostname")!= $hostname)	{$dirty_wan++; set("/sys/hostname", $hostname);}
		if (query($entry."unicast")		!= $unicast)		{$dirty_wan++; set($entry."unicast", $unicast);}
		if (query($entry."clonemac")!= $clonemac)	{$dirty_wan++; set($entry."clonemac", $clonemac);}
		if (query($entry."mtu")		!= $mtu)		{$dirty_wan++; set($entry."mtu", $mtu);}
		$entry="/dnsrelay/server/";
		if (query($entry."primarydns")	!=$dns1){$dirty_wan++; set($entry."primarydns", $dns1);}
		if (query($entry."secondarydns")!=$dns2){$dirty_wan++; set($entry."secondarydns", $dns2);}
		$entry="/w8021x/";
		if (query($entry."dhtype")!=$dh_eap_type){$dirty_wan++; set($entry."dhtype", $dh_eap_type);}
		if (query($entry."dhauth")!=$dh_authentication){$dirty_wan++; set($entry."dhauth", $dh_authentication);}
		if (query($entry."dhenable")!=$enable_dh8021x){$dirty_wan++; set($entry."dhenable", $enable_dh8021x);}
		if (query($entry."dhuser")!=$dh_username){$dirty_wan++; set($entry."dhuser", $dh_username);}
		if (query($entry."dhpassword")!=$dh_password){$dirty_wan++; set($entry."dhpassword", $dh_password);}
	}else if($wan_type == "pppoe")
	{
	
		$mode="2";
		$svc_name="";
		$clonemac="";
		$dnsmode="1";
		$autodns="1";
		$idletime="5";
		$mtu="1492";
		$ppp_conn_mode="2";
		$ppp_auto="1";
		$ppp_ondemand="1";
		$pppoe_eap_type="1";
		$pppoe_authentication="0";
		$pppoe_username="";
		$pppoe_password="";
		$pppoe_password_v="";
		$phy_mode="2";
		$wan_sch="";
	
		echo "mode=".$mode."\n";
		echo "username=".$username."\n";
		echo "pppoe_mppe=".$pppoe_mppe."\n";
		echo "password=".$password."\n";
		echo "clonemac=".$clonemac."\n";
		echo "autodns=".$autodns."\n";
		echo "dns1=".$dns1."\n";
		echo "dns2=".$dns2."\n";
		echo "idletime=".$idletime."\n";
		echo "mtu=".$mtu."\n";
		echo "ppp_auto=".$ppp_auto."\n";
		echo "ppp_ondemand=".$ppp_ondemand."\n";
		echo "wan_sch=".$wan_sch."\n";
		echo "pppoe_phy=".$pppoe_phy."\n";
		echo "phy_mode=".$phy_mode."\n";
		echo "phy_ip=".$phy_ip."\n";
		echo "phy_mask=".$phy_mask."\n";
		echo "phy_gw=".$phy_gw."\n";
		echo "phy_dns1=".$phy_dns1."\n";
		echo "phy_dns2=".$phy_dns2."\n";
	
		if ($pppoe_mppe != "1") {$pppoe_mppe = "0";}
		$entry="/wan/rg/inf:1/pppoe/";
		if (query("/wan/rg/inf:1/mode") != "3")		{$dirty_wan++; set("/wan/rg/inf:1/mode", "3");}
		if (query($entry."mode") != $mode)			{$dirty_wan++; set($entry."mode", $mode);}
		if (query($entry."user") != $username)		{$dirty_wan++; set($entry."user", $username);}
		if (query($entry."mppe/enable") != $pppoe_mppe) {$dirty_wan++; set($entry."mppe/enable", $pppoe_mppe);}
		if ($G_DEF_PASSWORD != $password &&
			query($entry."password") != $password)	{$dirty_wan++; set($entry."password", $password);}
		if (query($entry."acservice") != $svc_name)	{$dirty_wan++; set($entry."acservice", $svc_name);}
		//mode == 2
		if (query($entry."autodns") != $autodns)	{$dirty_wan++; set($entry."autodns", $autodns);}
		//ondemand == 1
		
		$idletime = $idletime * 60;
		if (query($entry."idletimeout")!=$idletime)	{$dirty_wan++; set($entry."idletimeout", $idletime);}
		
		if (query($entry."mtu")!=$mtu)					{$dirty_wan++; set($entry."mtu", $mtu);}
		if (query($entry."autoreconnect")!=$ppp_auto)	{$dirty_wan++; set($entry."autoreconnect", $ppp_auto);}
		if (query($entry."ondemand") != $ppp_ondemand)	{$dirty_wan++; set($entry."ondemand", $ppp_ondemand);}
		if (query($entry."schedule/id") != $wan_sch)	{$dirty_wan++; set($entry."schedule/id", $wan_sch);}
		if (query($entry."clonemac") != $clonemac)		{$dirty_wan++; set($entry."clonemac", $clonemac);}
		/* Russia PPPoE */
		del("/wan/rg/inf:2");
	
		$entry="/w8021x/";
		if (query($entry."pppoetype")!=$pppoe_eap_type){$dirty_wan++; set($entry."pppoetype", $pppoe_eap_type);}
		if (query($entry."pppoeauth")!=$pppoe_authentication){$dirty_wan++; set($entry."pppoeauth", $pppoe_authentication);}
		if (query($entry."pppoeenable")!=$enable_pppoe8021x){$dirty_wan++; set($entry."pppoeenable", $enable_pppoe8021x);}
		if (query($entry."pppoeuser")!=$pppoe_username){$dirty_wan++; set($entry."pppoeuser", $pppoe_username);}
		if (query($entry."pppoepassword")!=$pppoe_password){$dirty_wan++; set($entry."pppoepassword", $pppoe_password);}
	
		/* netsniper */
	
	}

//<-------------------------- WAN -------------------------------------------->

	// wlan related settings
	$dirty_wlan = "0";
	$wps_cfg    = "0";
	anchor("/wireless");
	if(query("enable")!="1")  { set("enable","1"); $dirty_wlan++; }
	if(query("ssid")!= $ssid) { set("ssid",$ssid); $dirty_wlan++; $wps_cfg++; }
	if($security_type == "0")
	{
		if(query("authtype")    != "0") { set("authtype", "0");    $dirty_wlan++; $wps_cfg++; }
		if(query("encrypttype") != "0") { set("encrypttype", "0"); $dirty_wlan++; $wps_cfg++; } 
	}
	else   //  Otherwise, set authtype to 7(Enable WPA/WPA2),  and chek encrypttype, if not valid for wpa, set it to 4(AUTO(TKIP/AES)).
	{
		if(query("authtype") != "7") { set("authtype", "7");    $dirty_wlan++; $wps_cfg++; }
		if(query("encrypttype") == "0" || query("encrypttype") == "1") { set("encrypttype", "4"); $dirty_wlan++; }
		if(query("wpa/key")  != $wpapsk1)
		{
			set("wpa/key", $wpapsk1); 
			$dirty_wlan++;
			
			if($wpapsk1_len == "64" )
			{	set("wpa/format", "2");}
			else
			{	set("wpa/format", "1");}
		}
	}
	
	if ( query("/runtime/func/wps")=="1" && $wps_cfg > 0)  {set("/wireless/wps/configured", "1");}

	
	$NEXT_PAGE = "logout";
	//submit system or submit wan and wlan
	if ($rb_dirty > 0)		{$SUBMIT_STR=";submit SYSTEM";$XGISET_STR="set/runtime/stats/resetCounter=1"; $NEXT_PAGE="bsc_chg_rg_mode";}
	else
	{
		if($dirty_enable > 0) {$SUBMIT_STR=";";}
		if ($dirty_wan > 0)	{$SUBMIT_STR="submit WAN;";set("/sys/configdirty", "1");}
		if ($dirty_wlan > 0) {$SUBMIT_STR = "submit WLAN;".$SUBMIT_STR;set("/sys/configdirty", "1");}
	}

	echo "SUBMIT_STR=".$SUBMIT_STR."\n";
	echo "-->\n";

	if($SUBMIT_STR!="")	{require($G_SAVING_URL);}
	else				{require($G_NO_CHANGED_URL);}
}
/* --------------------------------------------------------------------------- */
require("/www/model/__html_head.php");
/* --------------------------------------------------------------------------- */
// get the variable value from rgdb.


/* --------------------------------------------------------------------------- */


?>
<script>
/* page init functoin */
function init()
{

}
function check()
{
	var wpapsk1_len = get_obj("wpapsk1_len");
	var wpapsk1 = "<?=$wpapsk1?>";
	wpapsk1_len.value = wpapsk1.length;
	return true;
}
function go_next()
{
}
function go_prev()
{
	self.location.href="<?=$POST_ACTION?>?TARGET_PAGE=<?=$WIZ_PREV?>";
}
function logout()
{
	document.write("<input type='button' name='exit' value=\"<?=$m_cancel?>\" onClick=\"exit_confirm('logout.php')\">&nbsp;");
}

</script>
<style type="text/css">
legend{
		color:#000000;
		font-size: 12px;
		font-weight: bold;
}

</style>
<body onLoad="init();" <?=$G_BODY_ATTR?>>
<form name="frm" id="frm" method="post" action="<?=$POST_ACTION?>" onSubmit="return check();">
<input type="hidden" name="ACTION_POST" value="<?=$MY_ACTION?>">
<input type="hidden" name="TARGET_PAGE" value="<?=$MY_ACTION?>">
<input type="hidden" name="wpapsk1_len" id="wpapsk1_len" value="0">



<? require("/www/model/__banner.php");?>
<table <?=$G_MAIN_TABLE_ATTR?>>
<tr valign=top>
	<td width=10%></td>
	<td id="maincontent" width=80%>
		<br>
		<div id="box_header">
			<h1><?=$m_easy_setup_complete?></h1>
			<p><?=$m_click_button_to_save?></p>
			<fieldset>
			<legend><?=$m_internet_setting?></legend> 
			<table>
				<tr>
					<td class="r_tb" width=300><?=$m_internet_connection?>&nbsp;:&nbsp;</td>
					<td class="l_tb"><? if($wan_type == "pppoe"){echo $m_pppoe;}else if($wan_type == "static"){echo $m_static_ip;} else if($wan_type == "dhcp"){echo $m_dhcp;}?>
					</td>
				</tr>
			</table>
			</fieldset>
	<br />
			<fieldset>
			<legend><?=$m_wireless_setting?></legend> 
			<table>
				<tr>
					<td class="r_tb" width=300><?=$m_wireless_name?>&nbsp;:&nbsp;</td>
					<td class="l_tb"><?=$ssid?>
					</td>
				</tr>
				<tr>
					<td class="r_tb"><?=$m_secuity?>&nbsp;:&nbsp;</td>
					<td class="l_tb"><? if($security_type == "0"){echo $m_disabled;} else if($security_type == "4"){echo $m_wpa_wpa2;}?>
					</td>
				</tr>
				<? if($security_type != "0")
				{
					echo "<tr>\n";
					echo "<td class=\"r_tb\">".$m_network_key."&nbsp;:&nbsp;</td>\n";
					echo "<td class=\"l_tb\">".$wpapsk1."</td>\n";
					echo "</tr>\n";
				}
				?>
			</table>
			</fieldset>
			<div class=bl_tb>
			<? require($LOCALE_PATH."/dsc/dsc_".$MY_NAME.".php"); ?>
			</div>

<!-- ________________________________ Main Content Start ______________________________ -->
		<br>
		<center><script>prev("");wiz_save("check()");logout();</script></center>
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

