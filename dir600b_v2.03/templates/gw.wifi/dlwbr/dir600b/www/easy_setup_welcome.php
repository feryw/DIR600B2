<?
/* vi: set sw=4 ts=4: ---------------------------------------------------------*/
$MY_NAME		="easy_setup_welcome";
$MY_MSG_FILE	=$MY_NAME.".php";
$MY_ACTION	= "welcome";
$WIZ_NEXT	= "internet";
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

$enable_easy = query("/sys/easysetup/enable");

/* --------------------------------------------------------------------------- */
?>
<script>
/* page init functoin */
function init()
{
	var enable_easy = get_obj("enable_easy");
	if("<?=$enable_easy?>" == "1")
	{
		enable_easy.checked = true;
	}else
	{
		enable_easy_checked = false;
	}
	on_change_enable();
}
function check()
{
	if(get_obj("easy_setup").checked == false)
	{
		self.location.href= "bsc_internet.php";
		return false;
	}
	on_change_enable();
	return true;
//	get_obj("frm").submit();	
}
function go_next()
{
}
function logout()
{
	document.write("<input type='button' name='exit' value=\"<?=$m_cancel?>\" onClick=\"exit_confirm('logout.php')\">&nbsp;");
}
function on_change_enable()
{
	var enable_easy = get_obj("enable_easy");
	if(enable_easy.checked == true)
	{enable_easy.value = "1";}
	else
	{enable_easy.value = "0";}
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
			<h1><?=$m_wel_set_wiz?></h1>
			<p><?=$m_connect_internet?></p>
			<table align="center">
				<tr>
				<td>&nbsp;</td>
	
				<td>
				<input name="easy_setup" id="easy_setup" type="radio" value="1" checked> <?=$m_easy_setup?>
				</td>
				</tr>
				<tr><td>&nbsp;</td></tr>
				<tr>
				<td></td>
				<td>
				<input name="easy_setup" id="easy_setup" type="radio" value="0"> <?=$m_manual_setup?>
				</td>
				</tr>
			</table>
			<p><input name="enable_easy" id="enable_easy" type="checkbox" onClick="on_change_enable()" value="0"><?=$m_always_easy?></p>
<!-- ________________________________ Main Content Start ______________________________ -->
		<br>
		<center><script>next("");logout();</script></center>
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

