<?
/* vi: set sw=4 ts=4: ---------------------------------------------------------*/
$MY_NAME		="adv_qos";
$MY_MSG_FILE	=$MY_NAME.".php";
$CATEGORY		="adv";
$SUB_CATEGORY   ="adv_qos";
$TEMP_NODES     = "/runtime/post/session_".$sid;
/* --------------------------------------------------------------------------- */
$STEP_1			="wiz_policy.php?TARGET_PAGE=1_name";
/* --------------------------------------------------------------------------- */
$router=query("/runtime/router/enable");

/*
if($del_id!="")
{
		require("/www/model/__admin_check.php");
		del("/security/policy/entry:".$del_id);
		$SUBMIT_STR = "submit RG_POLICY";
		$NEXT_PAGE=$MY_NAME;
		require($G_SAVING_URL);
}
else if ($ACTION_POST!="" && $router=="1")
*/
if ($ACTION_POST!="")
{
	require("/www/model/__admin_check.php");

	$dirty=0;

	anchor("/qos");
	if(query("mode")!=$mode) 			{ $dirty++; set("mode", $mode); }
	if(query("ubicom")!=$ubicom) 		{ $dirty++; set("ubicom", $ubicom); }
	if(query("conntype")!=$conntype) 	{ $dirty++; set("conntype", $conntype); }

	anchor("/qos/bandwidth");
	set("downstream","102400");
	if($ubicom==1){ set("upstream","102400"); }
	else
	{
	if(query("upstream")!=$bwup) 		{ $dirty++; set("upstream", $bwup); }
	}

	if($dirty  > 0)	{$SUBMIT_STR="submit QOS";}
	else			{$SUBMIT_STR="";}

	del($TEMP_NODES);
	$NEXT_PAGE=$MY_NAME;
	if($SUBMIT_STR!="")	{require($G_SAVING_URL);}
	else				{require($G_NO_CHANGED_URL);}
}

/* --------------------------------------------------------------------------- */
require("/www/model/__html_head.php");
/* --------------------------------------------------------------------------- */
// get the variable value from rgdb.
/* --------------------------------------------------------------------------- */

anchor("/qos");
$mode	    =query("mode");
if($mode=="") {$mode="0";}
$ubicom		=query("ubicom");
if($ubicom=="") {$ubicom="0";}
$bwup		=query("bandwidth/upstream");
if($bwup=="") {$bwup="128";}
$bwdown		=query("bandwidth/downstream");
$conntype	=query("conntype");
if($conntype=="") {$conntype="0";}

?>

<script>

/*
//for enable/disable rules
<?
	$total = 0;
	for ("/security/policy/entry")
	{
		$total++;
	}
?>
var rules=<?=$total?>+1;
var max_rules=<?=$total?>;
var data = new Array(rules);
var AjaxReq = null;
var rules_cnt = 0;
*/

function send_callback()
{
	if (AjaxReq != null && AjaxReq.readyState == 4)
	{
		delete AjaxReq;
		AjaxReq = null;

		if (rules_cnt < max_rules)  send_rules(20);
		else                        get_obj("frm").submit();
	}
	return true;
}

function send_rules(count)
{
	var str = "TEMP_NODES="+escape("<?=$TEMP_NODES?>")+"&data=1";

	str += "&start="+(rules_cnt+1);
	for (var i=0; i<count && rules_cnt < max_rules; i++)
	{
		rules_cnt++;
		str += "&d_"+rules_cnt+"_0="+escape(data[rules_cnt][0]);
	}
	str += "&end="+rules_cnt;

	AjaxReq = createRequest();
	if (AjaxReq != null)
	{
		AjaxReq.open("POST", "/set_temp_nodes.php", true);
		AjaxReq.onreadystatechange = send_callback;
		AjaxReq.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
		AjaxReq.send(str);
	}
}
///////////////////////////
/* page init functoin */
function init()
{
	var f=get_obj("frm");
	
	f.autoQOS_enable.checked = <?if($mode=="1"){echo "true";}else{echo "false";}?>;
	f.autoBW_enable.checked = <?if($ubicom=="1"){echo "true";}else{echo "false";}?>;

	f.qos_upstream.value = <?=$bwup?>;
	f.qos_conntype.value = <?=$conntype?>;

	qos_change_check();

	<?if($router!=1){echo "fields_disabled(f, true);\n";}?>
}
function check()
{
	var f=get_obj("frm");


/*
	var count=<?=$total?>;

	f.enable.value = f.policy_enable.checked? "1" : "0";
	/// save to global array 
	if (count > 0)
	{
		count = 0;
		for (i=1; i <= <?=$total?>; i++)
		{
			count++;
			data[count] = new Array(1);
			data[count][0] = (eval("f.entry_enable_"+i+".checked") == true) ? "1" : "0";
		}
	}

	///  fill rest array to empty value
	count++;
	for (i=count; i<rules; i++)
	{
		data[i] = new Array(1);
		data[i][0] = "";
	}
	
	var objs = document.getElementsByName("apply");
	for (var i=0; i<objs.length; i++) objs[i].disabled = true;

	rules_cnt = 0;
	send_rules(20);
*/
	        if(!is_digit(f.qos_upstream.value))
	        {
            alert("<?=$a_invalid_bw?>");
       		field_focus(f.qos_upstream, "**");
			return false;
	        }

	if( f.autoQOS_enable.checked )f.mode.value=1;else f.mode.value=0;
	if( f.autoBW_enable.checked )f.ubicom.value=1;else f.ubicom.value=0;
	f.bwup.value=f.qos_upstream.value;
	f.conntype.value=f.qos_conntype.value;
	
	f.submit();
}
/* cancel function */
function do_cancel()
{
	self.location.href="<?=$MY_NAME?>.php?random_str="+generate_random_str();
}
function qos_change_check()
{
	var f=get_obj("frm");

	if(f.autoQOS_enable.checked)
	{
		f.autoBW_enable.disabled = false;
		f.qos_conntype.disabled = false;

		if(f.autoBW_enable.checked)
		{
		f.qos_upstream.disabled = true;
		f.select_up.disabled = true;
		}
		else
		{
		f.qos_upstream.disabled = false;
		f.select_up.disabled = false;
		}
	}
	else
	{
		f.autoBW_enable.disabled = true;
		f.qos_conntype.disabled = true;

		f.qos_upstream.disabled = true;
		f.select_up.disabled = true;
	}
}
function qos_change_upstream(bw)
{
	var f=get_obj("frm");

	f.qos_upstream.value=bw;
	f.select_up.value=0;
}
function del_confirm(id)
{
	if(confirm("<?=$a_del_confirm?>")==false) return;
	self.location.href="<?=$MY_NAME?>.php?del_id="+id;
}
</script>

<body onload="init();" <?=$G_BODY_ATTR?>>
<form name="frm" id="frm" method="post" action="<?=$MY_NAME?>.php">
<input type="hidden" name="ACTION_POST"	value="SOMETHING">

<input type="hidden" name="mode">
<input type="hidden" name="ubicom">
<input type="hidden" name="bwup">
<input type="hidden" name="conntype">

<?require("/www/model/__banner.php");?>
<?require("/www/model/__menu_top.php");?>
<table <?=$G_MAIN_TABLE_ATTR?> height="100%">
<tr valign=top>
	<td <?=$G_MENU_TABLE_ATTR?>>
	<?require("/www/model/__menu_left.php");?>
	</td>
	<td id="maincontent">
		<div id="box_header">
		<? require($LOCALE_PATH."/dsc/dsc_".$MY_NAME.".php"); ?>
		<script>apply('check()'); echo("&nbsp;"); cancel('');</script>
		</div>
<!-- ________________________________ Main Content Start ______________________________ -->
<?
$td_width0="width=\"180\" align=\"right\"";
$td_width1="width=\"160\" align=\"right\"";
$td_width2="width=\"150\"";
$td_width3="width=\"60\"";
?>
		<div class="box">
			<h2><?=$m_title_qos?></h2>
			<table border=0>
			<tr>
				<td <?=$td_width0?>><strong><?=$m_enable_autoQOS?></strong>&nbsp;:</td>
				<td <?=$td_width2?>>&nbsp;<input type="checkbox" name="autoQOS_enable" id=autoQOS_enable  onclick="qos_change_check()"></td>
			</tr>
			<tr>
				<td <?=$td_width0?>><strong><?=$m_enable_autoBW?></strong>&nbsp;:</td>
				<td <?=$td_width2?>>&nbsp;<input type="checkbox" name="autoBW_enable" id=autoBW_enable onclick="qos_change_check()"></td>
			</tr>
			<tr>
				<td <?=$td_width0?>><strong><?=$m_uplinkspeed_auto?></strong>&nbsp;:</td>
				
				<td <?=$td_width2?>>&nbsp;
				<?
				if($ubicom==0){ echo $m_nodetect; }
				else
				{
				if($bwup==102400){echo $m_nodetect;} else {echo $bwup." kbps";}
				}
				?>
				&nbsp;</td>

			</tr>
			<tr>
				<td <?=$td_width0?>><strong><?=$m_uplinkspeed_manual?></strong>&nbsp;:</td>
                <td>
				<input type="text" name="qos_upstream" id=qos_upstream size=6 maxlength=6>
			    kbps <span>&nbsp;&lt;&lt;&nbsp;</span>
                <select name="select_up" id=select_up onchange=qos_change_upstream(this.value)>
                <option value="0" selected><?=$m_seltransrate?></option>
                <option value="128">128k</option>
                <option value="256">256k</option>
                <option value="384">384k</option>
                <option value="512">512k</option>
                <option value="1024">1M</option>
                <option value="2048">2M</option>
				</select>
				</td>
			</tr>
			<tr>
				<td <?=$td_width0?>><strong><?=$m_conntype?></strong>&nbsp;:</td>
                <td>
                <select name="qos_conntype" id=qos_conntype>
                <option value="0" selected><?=$m_type_autodetect?></option>
                <option value="1"><?=$m_type_adsl?></option>
                <option value="2"><?=$m_type_cable?></option>
			</tr>
			<tr>
				<td <?=$td_width0?>><strong><?=$m_adslorcable?></strong>&nbsp;:</td>
				<td <?=$td_width2?>>&nbsp;<?if(query("/wan/rg/inf:1/mode")=="3"){echo "Yes";}else{echo "No";}?>&nbsp;</td>
			</tr>
			</table>
		</div>
<!-- ________________________________  Main Content End _______________________________ -->
		<div id="box_bottom">
		<script>apply('check()'); echo("&nbsp;"); cancel('');</script>
		</div>
	</td>
	<td <?=$G_HELP_TABLE_ATTR?>><?require($LOCALE_PATH."/help/h_".$MY_NAME.".php");?></td>
</tr>
</table>
<?require("/www/model/__tailer.php");?>
</form>
</body>
</html>
