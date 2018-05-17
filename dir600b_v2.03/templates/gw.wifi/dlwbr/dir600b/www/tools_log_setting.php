<?
/* vi: set sw=4 ts=4: ---------------------------------------------------------*/
$MY_NAME		="tools_log_setting";
$MY_MSG_FILE	=$MY_NAME.".php";
$CATEGORY		="tools";
/* --------------------------------------------------------------------------- */
if ($ACTION_POST!="")
{
	require("/www/model/__admin_check.php");
	
	echo "<!--\n";
	echo "srv=".$srv."\n";
	echo "email_addr=".$email_addr."\n";
	echo "log_sys=".$log_sys."\n";
	echo "log_attdrp=".$log_attdrp."\n";
	echo "log_ntc=".$log_ntc."\n";
	echo "send_mail=".$send_mail."\n";
	echo "log_enable=".$log_enable."\n";
	echo "log_srvip=".$log_srvip."\n";
	echo "-->\n";

	$mail_dirty=0;
	$log_dirty=0;
	if($log_sys!="1"){$log_sys="0";}
	if($log_attdrp!="1"){$log_attdrp="0";}
	if($log_ntc!="1"){$log_ntc="0";}
	
	$pre="/sys/log/";
	if(query($pre."email")		!=$email_addr)	{set($pre."email",		$email_addr);	$log_dirty++;}
	if(query($pre."subject")	!=$subject)		{set($pre."subject",	$subject);		$log_dirty++;}
	if(query($pre."sender")		!=$sender)		{set($pre."sender",		$sender);		$log_dirty++;}
	if(query($pre."mailserver")	!=$srv)			{set($pre."mailserver",	$srv);	 		$log_dirty++;}
	if(query("/runtime/func/mailport") == "1")  
	{
	if(query($pre."mailport")   !=$srv_port)    {set($pre."mailport",   $srv_port);     $log_dirty++;}
	if($auth_enable!="1"){$auth_enable="0";}
	if(query($pre."emailauthenable")	!=$auth_enable)	{set($pre."emailauthenable",	$auth_enable);	$log_dirty++;}
	}
	if(query($pre."username")	!=$name)		{set($pre."username",	$name);			$log_dirty++;}
	if(query($pre."pass1")		!=$pass1)		{set($pre."pass1",		$pass1);		$log_dirty++;}
	if(query($pre."pass2")		!=$pass2)		{set($pre."pass2",		$pass2);		$log_dirty++;}
	if(query("/runtime/func/logmail") == "1")
	{
		if(query($pre."mailonfull")	!=$mail_full_log)		{set($pre."mailonfull",	$mail_full_log);		$log_dirty++;}
		if(query($pre."enablemailschedule")	!=$mail_schedule_enable)	{set($pre."enablemailschedule",	$mail_schedule_enable);		$log_dirty++;}	
		if(query($pre."schedule/id")	!=$log_mail_sch)	{set($pre."schedule/id",	$log_mail_sch);	$log_dirty++;}
	}
	
	if(query($pre."logserver")			!=$log_srvip)	{set($pre."logserver",			$log_srvip);	$log_dirty++;}
	if(query($pre."logserverenable")	!=$log_enable)	{set($pre."logserverenable",	$log_enable);	$log_dirty++;}
	
	anchor("/security/log");
	if(query("systemInfo")		!=$log_sys)		{set("systeminfo",		$log_sys);		$log_dirty++;}
	if(query("attackInfo")		!=$log_attdrp)	{set("attackinfo",		$log_attdrp);	$log_dirty++;}
	if(query("dropPacketInfo")	!=$log_attdrp)	{set("dropPacketInfo",	$log_attdrp);	$log_dirty++;}
	if(query("noticeInfo")		!=$log_ntc)		{set("noticeinfo",		$log_ntc);		$log_dirty++;}
	
	$SUBMIT_STR="";
	if($log_dirty>0)	{$SUBMIT_STR=";submit SYSLOG;submit RG";}
	if($send_mail=="1")	{$XGISET_AFTER_COMMIT_STR="set/runtime/syslog/auth_sendmail=1";}

	$NEXT_PAGE=$MY_NAME;
	if($SUBMIT_STR!="" || $XGISET_AFTER_COMMIT_STR!="")	{require($G_SAVING_URL);}
	else												{require($G_NO_CHANGED_URL);}
}

/* --------------------------------------------------------------------------- */
require("/www/model/__html_head.php");
require("/www/comm/__js_ip.php");
/* --------------------------------------------------------------------------- */
// get the variable value from rgdb.
$cfg_ipaddr		= query("/lan/ethernet/ip");
$cfg_netmask	= query("/lan/ethernet/netmask");

$mail_auth_enable = query("/sys/log/emailauthenable");
/* --------------------------------------------------------------------------- */
?>

<script>
// Lily
Lists=[<?inclog("[\"%0\",\"%1\"],","/var/log/messages");?>["",""]];

sData=[<?
anchor("/sys/log");
echo "'".get("j","email")."','".get("j","subject")."','".get("j","sender")."','".get("j","mailserver")."','".get("j","username")."','".get("j","pass1")."','".get("j","pass2")."','".get("j","mailonfull")."','".get("j","enablemailschedule")."','".get("j","mailport")."'";
?>];
//system, debug, att, drop, notice
syslog=['',<?
anchor("/security/log");
echo	"'".	query("systemInfo").		"','".query("debugInfo").	"','".query("attackInfo").
		"','".	query("dropPacketInfo").	"','".query("noticeInfo").	"'";
?>];
/* page init functoin */
function init()
{
	var f=get_obj("frm");
	f.email_addr.value=sData[0];
	f.subject.value=sData[1];
	f.sender.value=sData[2];
	f.srv.value=sData[3];
	f.name.value=sData[4];
	f.pass1.value=sData[5];
	f.pass2.value=sData[6];
	f.log_sys.checked=(syslog[1]=="1" ?true:false);
	f.log_attdrp.checked=(syslog[3]=="1" && syslog[4]=="1" ?true:false);
	f.log_ntc.checked=(syslog[5]=="1" ?true:false);
	/*f.mail_full_log.checked=true;*/
	<?
	if(query("/runtime/func/logmail") == "1")
	{
		echo "if(sData[7]==1){f.mail_full_log.checked=true;}\n";
		echo "\t
		if(sData[8]==1){
			f.mail_schedule_enable.checked=true;
		}else{
			f.log_mail_sch.disabled=true;
			f.log_mail_sch_btn.disabled=true;
		}\n";
		$cfg_log_sch=query("/sys/log/schedule/id");
		
		/*echo "f.schedule_detial.value=\"aaaa\";";*/
	}
	?>
	<?
	if(query("/runtime/func/mailport") == "1")
	{
		echo "\t
		if(sData[9]==\"\")
			f.srv_port.value=25;
		else
			f.srv_port.value=sData[9];\n";
	
		if($mail_auth_enable=="1")	{echo "f.auth_enable.checked=true;\n";}

		echo "\tcheck_auth_ebox();\n";
	}
	?>
}
<?
if(query("/runtime/func/logmail") == "1")
{		echo "\t
function click_enable_sch(){
	var f=get_obj(\"frm\");
	if(f.mail_schedule_enable.checked ==true)
	{
		f.log_mail_sch.disabled=false;
		f.log_mail_sch_btn.disabled=false;
	}
	else
	{
		f.log_mail_sch.disabled=true;
		f.log_mail_sch_btn.disabled=true;
	}
}";
}
?>
/* parameter checking */
function check(bmail)
{
	var f=get_obj("frm");

	if (f.log_enable.checked)
	{
		if (is_valid_ip(f.log_srvip.value, 0)==false)
		{
			alert("<?=$a_invalid_srvip?>");
			field_focus(f.log_srvip, "**");
			return false;
		}
		net1 = get_network_id(f.log_srvip.value, "<?=$cfg_netmask?>");
		net2 = get_network_id("<?=$cfg_ipaddr?>", "<?=$cfg_netmask?>");
		if (net1[0] != net2[0])
		{
			alert("<?=$a_ip_in_different_subnet?>");
			field_focus(f.log_srvip, "**");
			return false;
		}
	}
	if(is_blank(f.email_addr.value) && is_blank(f.subject.value) && is_blank(f.sender.value) && 
	   is_blank(f.srv.value) && is_blank(f.name.value) && is_blank(f.pass1.value) && is_blank(f.pass2.value))
	{
		if(bmail!="1") return true;
	}
	if(is_blank(f.email_addr.value))
	{
		alert("<?=$a_invalid_email?>");
		f.email_addr.focus();
		return false;
	}
	if(is_blank(f.sender.value))
	{
		alert("<?=$a_invalid_email?>");
		f.sender.focus();
		return false;
	}
	if(is_blank(f.srv.value))
	{
		alert("<?=$a_invalid_smtp_srv?>");
		f.srv.focus();
		return false;
	}
	if(!is_blank(f.email_addr.value))
	{
		if(strchk_email(f.email_addr.value)==false)
		{
			alert("<?=$a_invalid_email?>");
			f.email_addr.select();
			return false;
		}
		var str=f.email_addr.value;
		var valid_email=false;
		for(var i=0; i<str.length; i++)
		{
			if( (str.charAt(i) != '@') )	continue;
			else	valid_email=true;
		}
		if(!valid_email)
		{
				alert("<?=$a_invalid_email?>");
				f.email_addr.select();
				return false;
		}
	}
	if(!is_blank(f.sender.value))
	{
		if(strchk_email(f.sender.value)==false)
		{
			alert("<?=$a_invalid_email?>");
			f.sender.select();
			return false;
		}
		var str=f.sender.value;
		var valid_email=false;
		for(var i=0; i<str.length; i++)
		{
			if( (str.charAt(i) != '@') )	continue;
			else	valid_email=true;
		}
		if(!valid_email)
		{
				alert("<?=$a_invalid_email?>");
				f.sender.select();
				return false;
		}
	}
	if(!is_blank(f.srv.value))
	{
		if(strchk_hostname(f.srv.value)==false)
		{
			alert("<?=$a_invalid_smtp_srv?>");
			f.srv.select();
			return false;
		}
	}
<?
if(query("/runtime/func/mailport") == "1")
{
	echo "\t
	if(is_blank(f.srv_port.value))
 	{
		alert('".$a_blank_port."');
		f.srv_port.focus();
 		return false;
 	}
	if (!is_valid_port_str(f.srv_port.value))
 	{
		alert('".$a_invalid_port."');
		f.srv_port.select();
 		return false;
	}
	if(f.auth_enable.checked)
	{\n";
}
?>
		if(is_blank(f.name.value))
		{
			alert("<?=$a_invalid_name?>");
			f.name.select();
			return false;
		}
		if(is_blank(f.pass1.value))
		{
			alert("<?=$a_invalid_password?>");
			f.pass1.select();
			return false;
		}
		if(is_blank(f.pass2.value))
		{
			alert("<?=$a_invalid_password?>");
			f.pass2.select();
			return false;
		}
		if(f.pass1.value!=f.pass2.value)
		{
			alert("<?=$a_invalid_pass?>");
			f.pass1.select();
			return false;
		}
<?
if(query("/runtime/func/mailport") == "1")
{
	echo "\t}\n";
}
?>

	return true;
}
/* cancel function */
function do_cancel()
{
	self.location.href="<?=$MY_NAME?>.php?random_str="+generate_random_str();
}

function send_mail_now()
{
	var f=get_obj("frm");
	if(check(1)==false) return false;

	f.send_mail.value="1";
	f.submit();
}

function copy_ip()
{
	var pc = get_obj("sel_computer");
	var srvip = get_obj("log_srvip");

	if (pc.options.selectedIndex == 0)
	{
		alert("<?=$a_no_pc_selected?>");
		return false;
	}
	srvip.value = pc.value;
}

function check_auth_ebox()
{
	var f=get_obj("frm");
	if(f.auth_enable.checked)
	{		
		f.name.disabled=false;
		f.pass1.disabled=false;
		f.pass2.disabled=false;
	}
	else
	{
		f.name.disabled=true;
		f.pass1.disabled=true;
		f.pass2.disabled=true;
	}
}
</script>
<body onload="init();" <?=$G_BODY_ATTR?>>
<form name="frm" id="frm" method="post" action="<?=$MY_NAME?>.php" onsubmit="return check(0)">
<input type="hidden" name="ACTION_POST" value="SOMETHING">
<input type="hidden" name="send_mail" value="0">
<?require("/www/model/__banner.php");?>
<?require("/www/model/__menu_top.php");?>
<table <?=$G_MAIN_TABLE_ATTR?> height="100%">
<tr valign=top>
	<td <?=$G_MENU_TABLE_ATTR?>>
	<?require("/www/model/__menu_left.php");?>
	</td>
	<td id="maincontent">
		<div id="box_header">
		<?
		require($LOCALE_PATH."/dsc/dsc_".$MY_NAME.".php");
		echo $G_APPLY_CANEL_BUTTON;
		?>
		</div>
<!-- ________________________________ Main Content Start ______________________________ -->
		<div class="box">
			<h2><?=$m_savelog_title?></h2>
			<table cellSpacing=1 cellPadding=2 width=525>
			<tr>
				<td class=l_tb colspan=2><?=$m_save_log_cfg?>&nbsp;<input type=button name=save_log value="<?=$m_save?>" onClick="window.location.href='/tsyslog.rg';"></td>
			</tr>
		</table>
		</div>
		<div class="box">
			<h2><?=$m_logsrv_title?></h2>
			<table cellSpacing=1 cellPadding=2 width=525>
			<tr>
				<td class=l_tb>
					<table border=0>
					<tr>
					<td class=r_tb><?=$m_log_en_logsrv?>: </td>
					<td><input type=checkbox name=log_enable value="1" <?if(query("/sys/log/logserverenable")=="1") { echo "checked"; }?>>
					</td>
					</tr>
					<tr>
					<td class=r_tb><?=$m_log_logsrv_ip?>: </td>
					<td><input type=text name=log_srvip id=log_srvip size=18 maxlength=15 value="<?query("/sys/log/logserver");?>"></td>
					<td><input type=button id=copy_ipbtn value="<<" onclick="copy_ip()"></td>
					<td>
						<select id=sel_computer>
						<option value=0><?=$m_computer_name?></option selected>
<?
for ("/runtime/dhcpserver/lease")
{
	echo	"						<option value=\"".query("ip")."\">".get(h,"hostname")."</option>\n";
}
?>						</select>
					</td>
					</tr>
					</table>
				</td>
			</tr>
			<tr>
				<td class=l_tb>
					<table border=0>
					</table>
				</td>
			</tr>
		</table>
		</div>
		<div class="box">
			<h2><?=$m_leveltype_title?></h2>
			<table cellSpacing=1 cellPadding=2 width=525>
			<tr>
				<td class=r_tb><?=$m_log_type?>:</td>
				<td class=l_tb><input type=checkbox name=log_sys value="1"><?=$m_logtype_system?></td>
				<td class=l_tb><input type=checkbox name=log_attdrp value="1"><?=$m_logtype_firewall?></td>
				<td class=l_tb><input type=checkbox name=log_ntc value="1"><?=$m_logtype_notice?></td>
			</tr>
			<tr>
				<td colspan=2></td>
			</tr>
			<tr>
				<td class=r_tb disabled><?=$m_log_level?>:</td>
				<td class=l_tb disabled><input type=checkbox name=log_level_1 value="1" disabled><?=$m_loglevel_1?></td>
				<td class=l_tb disabled><input type=checkbox name=log_level_2 value="1" disabled><?=$m_loglevel_2?></td>
				<td class=l_tb disabled><input type=checkbox name=log_level_3 value="1" disabled><?=$m_loglevel_3?></td>
			</tr>
			</table>
		</div>
		<div class="box">
			<h2><?=$m_sendmail_title?></h2>
			<table cellSpacing=1 cellPadding=2 width=525>
			<tr>
				<td width=35% class=r_tb><?=$m_email_addr?>:</td>
				<td width=65%><input type=text name=email_addr maxlength="60"></td>
			</tr>
			<tr>
				<td width=35% class=r_tb><?=$m_email_sub?>:</td>
				<td width=65%><input type=text name=subject maxlength="60"></td>
			</tr>
			<tr>
				<td width=35% class=r_tb><?=$m_send_email_addr?>:</td>
				<td width=65%><input type=text name=sender maxlength="60"></td>
			</tr>
			<tr>
				<td width=35% class=r_tb><?=$m_smtp_srv_ip?>:</td>
				<td width=65%><input type=text name=srv maxlength="60"></td>
			</tr>
			<?if(query("/runtime/func/mailport") != "1") {echo "<!--\n";}?>
 			<tr>
				<td width=35% class=r_tb><?=$m_smtp_srv_port?>:</td>
				<td width=65%><input type=text name=srv_port size="5" maxlength="5"></td>
			</tr>			
			<tr>
				<td width=35% class=r_tb><?=$m_send_auth?>:</td>
				<td width=65%><input type=checkbox name=auth_enable  value=1 onclick="check_auth_ebox();"></td>
			</tr>
			<?if(query("/runtime/func/mailport") != "1") {echo "-->\n";}?>
			<tr>
				<td width=35% class=r_tb><?=$m_send_name?>:</td>
				<td width=65%><input type=text name=name maxlength="60"></td>
			</tr>
			<tr>
				<td class=r_tb><?=$m_send_pass1?>:</td>
				<td><input type=password name=pass1 maxlength="60"></td>
			</tr>
			<tr>
				<td class=r_tb><?=$m_send_pass2?>:</td>
				<td><input type=password name=pass2 maxlength="60">&nbsp;<input type=button name=sendmail onclick="send_mail_now()" value="<?=$m_send_mail_now?>"></td>
			</tr>
		</table>
		</div>
		<?
			if(query("/runtime/func/logmail") == "1")
			{
				require("/www/__tools_log_setting_sch.php");
			}
		?>

<!-- ________________________________  Main Content End _______________________________ -->
	</td>
	<td <?=$G_HELP_TABLE_ATTR?>><?require($LOCALE_PATH."/help/h_".$MY_NAME.".php");?></td>
</tr>
</table>
<?require("/www/model/__tailer.php");?>
</form>
</body>
</html>
