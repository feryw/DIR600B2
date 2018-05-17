<?
/* vi: set sw=4 ts=4: ---------------------------------------------------------*/
$MY_NAME		="easy_setup_current";
$MY_MSG_FILE	=$MY_NAME.".php";
$MY_ACTION	= "current";
$WIZ_PREV		="";
$WIZ_NEXT	= "welcome";
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
$WAN_MODE = query("/wan/rg/inf:1/mode");
if($WAN_MODE == "2")
{
	$wan_type = "dhcp";
}
else if($WAN_MODE == "1")
{
	$wan_type = "static";
	$entry="/wan/rg/inf:1/static/";
	$ipaddr = query($entry."ip");
	$netmask = query($entry."netmask");
	$gateway = query($entry."gateway");
	$entry="/dnsrelay/server/";
	$dns1 = query($entry."primarydns");
	$dns2 = query($entry."secondarydns");
}
else if($WAN_MODE == "3")
{
	$wan_type = "pppoe";
	$entry="/wan/rg/inf:1/pppoe/";
	$username = query($entry."user");
	$password = $G_DEF_PASSWORD ;
	
}
else if($WAN_MODE == "4")
{
	$wan_type = "pptp";
}
else if($WAN_MODE == "5")
{
	$wan_type = "l2tp";
}

$ssid = query("/wireless/ssid");
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
}
function go_next()
{
}
function wiz_easy(fn)
{
	if(fn=="")
		document.write("<input type='submit' name='next' value=\"<?=$m_easy_setup?>\">&nbsp;");
	else
		document.write("<input type='button' name='next' value=\"<?=$m_easy_setup?>\" onClick='return "+fn+"'>&nbsp;");
}
function wiz_manual()
{
	document.write("<input type='button' name='exit' value=\"<?=$m_manual_setup?>\" onClick=\"exit_confirm('bsc_internet.php')\">&nbsp;");
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
					<td class="l_tb"><? if($wan_type == "pppoe"){echo $m_pppoe;}else if($wan_type == "static"){echo $m_static_ip;} else if($wan_type == "dhcp"){echo $m_dhcp;} else if($wan_type == "pptp"){echo $m_pptp;} else if($wan_type == "l2tp"){echo $m_l2tp;} ?>
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
					<td class="l_tb"><? if($ssid == ""){echo $m_disabled;}else{echo $ssid;}?>
					</td>
				</tr>
				<tr>
					<td class="r_tb"><?=$m_secuity?>&nbsp;:&nbsp;</td>
					<td class="l_tb">
					<?
					$authtype = query("/wireless/authtype");
					$enctype = query("/wireless/encrypttype");
					if($authtype == "0" && $enctype == "1") //open
					{
						$security_type = $m_open;
						$defkey = query("/wireless/wep/defkey");
						$network_key = query("/wireless/wep/key:".$defkey);
					}
					else if($authtype == "1") //shared key
					{
						$security_type = $m_share;
						$defkey = query("/wireless/wep/defkey");
						$network_key = query("/wireless/wep/key:".$defkey);
					}
					else if($authtype == "6") //wpa/wpa2 auto
					{
						$security_type = $m_wpa_wpa2;
						$network_key = query("/wireless/wpa/radius:1/secret");
					}
					else if($authtype == "7") //wpa-psk/wpa2-psk auto
					{
						$security_type = $m_wpa_wpa2_psk;
						$network_key = query("/wireless/wpa/key");
					}
					else
					{
						$security_type = $m_disabled;
						$network_key = "";
					}
					echo $security_type;
					?>
					</td>
				</tr>
				<?
				if($network_key != "")
				{
					echo "<tr>\n";
					echo "<td class=\"r_tb\">".$m_network_key."&nbsp;:&nbsp;</td>\n";
					echo "<td class=\"l_tb\">".$network_key."</td>\n";
					echo "</tr>";
				}
				?>
			</table>
			</fieldset>

<!-- ________________________________ Main Content Start ______________________________ -->
		<br>
		<center><script>wiz_easy("");wiz_manual();</script></center>
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

