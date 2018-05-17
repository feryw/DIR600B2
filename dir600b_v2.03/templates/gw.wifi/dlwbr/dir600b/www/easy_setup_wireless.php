<?
/* vi: set sw=4 ts=4: ---------------------------------------------------------*/
$MY_NAME		="easy_setup_wireless";
$MY_MSG_FILE	= "bsc_wlan.php";
$MY_ACTION	= "wireless";
$WIZ_PREV 	= "internet";
$WIZ_NEXT	= "complete";
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
/* --------------------------------------------------------------------------- */
require("/www/model/__html_head.php");
/* --------------------------------------------------------------------------- */
// get the variable value from rgdb.
anchor("/wireless");
$cfg_ssid		= get("j", "ssid");
$cfg_auth		= query("authtype");
$cfg_cipher		= query("encrypttype");
if ($cfg_cipher=="0") {$security_type="0";}
else {$security_type="4";}
$cfg_wpapsk		= get("j", "wpa/key");
/* --------------------------------------------------------------------------- */
?>
<script>
function on_change_security_type()
{
	var sec_type = get_obj("security_type");

	get_obj("psk_setting").style.display = "none";

	if (sec_type.value != 0)
	{
		get_obj("psk_setting").style.display = "";
	}
}
/* page init functoin */
function init()
{
	var f=get_obj("frm");
	
	f.ssid.value = "<?=$cfg_ssid?>";
	
	select_index(f.security_type, "<?=$security_type?>");
	on_change_security_type()

	f.wpapsk1.value		="<?=$cfg_wpapsk?>";
}
function check()
{
	var f=get_obj("frm");
	
	if(is_blank(f.ssid.value))
	{
		alert("<?=$a_empty_ssid?>");
		f.ssid.focus();
		return false;
	}
	if(strchk_unicode(f.ssid.value))
	{
		alert("<?=$a_invalid_ssid?>");
		f.ssid.select();
		return false;
	}
	if(f.security_type.value=="4")
	{
		if(f.wpapsk1.value.length==64)
		{
			var test_char,j;
			for(j=0; j<f.wpapsk1.value.length; j++)
			{
				test_char=f.wpapsk1.value.charAt(j);
				if( (test_char >= '0' && test_char <= '9') ||
						(test_char >= 'a' && test_char <= 'f') ||
						(test_char >= 'A' && test_char <= 'F'))
					continue;

				alert("<?=$a_invalid_psk?>");
				f.wpapsk1.select();
				return false;
			}
		}
		else
		{
			if(f.wpapsk1.value.length <8 || f.wpapsk1.value.length > 63)
			{
				alert("<?=$a_invalid_passphrase_len?>");
				f.wpapsk1.select();
				return false;
			}
			if(strchk_unicode(f.wpapsk1.value))
			{
				alert("<?=$a_invalid_passphrase?>");
				f.wpapsk1.select();
				return false;
			}
		}
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
		<div id="box_header">		
		<h1><?=$m_title_config?></h1>
		<br>
<!-- ________________________________ Main Content Start ______________________________ -->
		<table align="center">
		<tr>
			<td class="r_tb"><?=$m_net_name_ssid?>&nbsp;:</td>
			<td class="l_tb">
				<input name="ssid" id="ssid" type="text" size="20" maxlength="32" value="">
			</td>
		</tr>
		<tr>
			<td class="r_tb"><?=$m_wlan_security_mode?>&nbsp;:</td>
			<td class="l_tb">
			<select id="security_type" name="security_type" onChange="on_change_security_type()">
				<option value="0" selected><?=$m_disable_security?></option>
				<option value="4"><?=$m_wpa2_auto_security?></option>
			</select>
			</td>
		</tr>
		<tr id="psk_setting" style="display:none">
			<td class="r_tb"><?=$m_wlan_passphrase?>&nbsp;:</td>
			<td class="l_tb" >
				<input type="text" id="wpapsk1" name="wpapsk1" size="40" maxlength="64" value="">
			</td>
		</tr>
		</table>
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

