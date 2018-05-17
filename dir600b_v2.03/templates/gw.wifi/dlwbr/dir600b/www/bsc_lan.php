<?
/* vi: set sw=4 ts=4: ---------------------------------------------------------*/
$MY_NAME		="bsc_lan";
$MY_MSG_FILE	=$MY_NAME.".php";
$CATEGORY		="bsc";
$TEMP_NODES		= "/runtime/post/session_".$sid;
/* --------------------------------------------------------------------------- */
$router=query("/runtime/router/enable");
$MAX_RULES=query("/lan/dhcp/server/pool:1/staticdhcp/max_client");
if ($MAX_RULES==""){$MAX_RULES=10;}
if($router=="1")
{
	if ($ACTION_POST!="" || $del_id!="")
	{
		require("/www/model/__admin_check.php");
		
		if($del_id!="")
		{
			del("/lan/dhcp/server/pool:1/staticdhcp/entry:".$del_id);
			$SUBMIT_STR=";submit DHCPD";
		}
		else
		{
			if ($dnsr == "enable")	{ $dnsr = "2"; } else { $dnsr="1"; }	/* 1:disable, 2:auto */
			if ($dhcpsvr != "1")	{ $dhcpsvr = "0"; }
			if ($netbios_enable != "1") {$netbios_enable = "0";}
			if ($netbios_learn != "1") {$netbios_learn = "0";}
			$lan_ip_dirty=0; $lan_dirty = 0; $dnsr_dirty = 0; $dhcp_dirty = 0;

			/* Check LAN config */
			anchor("/lan/ethernet");
			if (query("ip")!= $ipaddr)			{$lan_dirty++; $lan_ip_dirty++; set("/runtime/dhcpd/disable_lan", "1"); set("ip", $ipaddr);}
			if (query("netmask")!=$netmask)		{$lan_dirty++; set("netmask", $netmask);}

			/* Check DNS relay config */
			if (query("/dnsrelay/mode")!=$dnsr)	{$dnsr_dirty++; set("/dnsrelay/mode", $dnsr);}

			/* Check DHCP server config */
			anchor("/lan/dhcp/server");
			if (query("enable")!=$dhcpsvr)		{$dhcp_dirty++; set("enable", $dhcpsvr);}
			anchor("/lan/dhcp/server/pool:1");
			if (query("domain")!=$domain)		{$dhcp_dirty++; set("domain", $domain);}
			if ($dhcpsvr == "1")
			{
				if (query("startip")!=$startipaddr)		{$dhcp_dirty++; set("startip",	$startipaddr);}
				if (query("endip")!=$endipaddr)			{$dhcp_dirty++; set("endip",	$endipaddr);}
				if (query("leasetime")!=$lease_seconds)	{$dhcp_dirty++; set("leasetime",$lease_seconds);}
				/* Check NetBIOS config*/
				if (query("netbios_enable")!=$netbios_enable) {$dhcp_dirty++; set("netbios_enable",$netbios_enable);}
				if($netbios_enable == "1")
				{
					if (query("netbios_learn")!=$netbios_learn) {$dhcp_dirty++; set("netbios_learn",$netbios_learn);}
					if($netbios_learn != "1")
					{
						if (query("netbios_scope")!=$netbios_learn) {$dhcp_dirty++; set("netbios_scope",$netbios_scope);}
						if (query("winstype")!=$winstype) {$dhcp_dirty++; set("winstype",$winstype);}
						if (query("primarywins")!=$primarywins) {$dhcp_dirty++; set("primarywins",$primarywins);}
						if (query("secondarywins")!=$secondarywins) {$dhcp_dirty++; set("secondarywins",$secondarywins);}
					}
				}
			}
			
			/* ----------------------------- static dhcp --------------------------------------- */
			anchor("/lan/dhcp/server/pool:1/staticdhcp");
			if(query("/runtime/func/static_dhcp")=="1")
			{
				$i=0;

				echo "<!--\n";

				while($i<$MAX_RULES)
				{
					$index=$i+1;

					$en		= query($TEMP_NODES."/entry:".$index."/data_0"); if ($en!="1") {$en="0";}
					$host	= query($TEMP_NODES."/entry:".$index."/data_1");
					$ip		= query($TEMP_NODES."/entry:".$index."/data_2");
					$mac	= query($TEMP_NODES."/entry:".$index."/data_3");
					
					echo "en	= ".$en		."\t";
					echo "host	= ".$host	."\t";
					echo "ip	= ".$ip		."\t";
					echo "mac	= ".$mac	."\n";

					$entry	= "entry:".$index;
					if(query($entry."/enable")		!=$en)		{$dhcp_dirty++; set($entry."/enable",	$en);}
					if(query($entry."/hostname")	!=$host)	{$dhcp_dirty++; set($entry."/hostname",	$host);}
					if(query($entry."/ip")			!=$ip)		{$dhcp_dirty++; set($entry."/ip",		$ip);}
					if(query($entry."/mac")			!=$mac)		{$dhcp_dirty++; set($entry."/mac",		$mac);}

					$i++;
				}
				echo "-->\n\n";
			}

			/* ------------------------------------------------------------------------------- */
			
			/* Check dirty */
			$SUBMIT_STR="";
			if	($on_lan_change=="1")	{$SUBMIT_STR="submit LAN_CHANGE"; $lan_ip_dirty++;}
			if		($lan_dirty > 0)	{$SUBMIT_STR=$SUBMIT_STR.";submit LAN";}
			else if	($dhcp_dirty > 0)	{$SUBMIT_STR=$SUBMIT_STR.";submit DHCPD";}
			if ($dnsr_dirty > 0) //kwest: to prevent restart dhcpd twice which would cause running two dhcpd process sometimes.
			{
				if($dhcp_dirty > 0)		{$SUBMIT_STR=$SUBMIT_STR.";submit DNSR";}
				else					{$SUBMIT_STR=$SUBMIT_STR.";submit DNSR;submit DHCPD";}
			}
		}
		if($lan_ip_dirty > 0)
		{
			$NEXT_PAGE="bsc_lan_ipchanged";
			$ONLY_DO_SUBMIT_STR="submit COMMIT;".$SUBMIT_STR;
			$SUBMIT_STR="";
		}
		else
		{
			$NEXT_PAGE=$MY_NAME;
		}
		if($SUBMIT_STR!="" || $ONLY_DO_SUBMIT_STR!="")
		{require($G_SAVING_URL);}
		else
		{require($G_NO_CHANGED_URL);}
	}
}
/* --------------------------------------------------------------------------- */
require("/www/model/__html_head.php");
require("/www/comm/__js_ip.php");
/* --------------------------------------------------------------------------- */
// get the variable value from rgdb.
$cfg_ipaddr		= query("/lan/ethernet/ip");
$cfg_netmask	= query("/lan/ethernet/netmask");
$cfg_domain		= get(h,"/lan/dhcp/server/pool:1/domain");
$cfg_dnsr		= query("/dnsrelay/mode");
$cfg_dhcpsvr	= query("/lan/dhcp/server/enable");
$cfg_startip	= query("/lan/dhcp/server/pool:1/startip");
$cfg_endip		= query("/lan/dhcp/server/pool:1/endip");
$cfg_leasetime	= query("/lan/dhcp/server/pool:1/leasetime");
//NetBIOS add
$cfg_netbios_enable	= query("/lan/dhcp/server/pool:1/netbios_enable");
$cfg_netbios_learn	= query("/lan/dhcp/server/pool:1/netbios_learn");
$cfg_netbios_scope	= query("/lan/dhcp/server/pool:1/netbios_scope");
$cfg_winstype	= query("/lan/dhcp/server/pool:1/winstype");
$cfg_primarywins	= query("/lan/dhcp/server/pool:1/primarywins");
$cfg_secondarywins	= query("/lan/dhcp/server/pool:1/secondarywins");
/* --------------------------------------------------------------------------- */
?>

<script>
var rules=<?=$MAX_RULES?>+1;
var max_rules=<?=$MAX_RULES?>;
var data = new Array(rules);
var rules_per_send = 25;
var AjaxReq = null;
var rules_cnt = 0;

function send_callback()
{
	if (AjaxReq != null && AjaxReq.readyState == 4)
	{
		delete AjaxReq;
		AjaxReq = null;

		if (rules_cnt < max_rules)	send_rules(rules_per_send);
		else						get_obj("frm").submit();
	}
	return true;
}

function send_rules(count)
{
	var str = "TEMP_NODES="+escape("<?=$TEMP_NODES?>")+"&data=4";

	str += "&start="+(rules_cnt+1);
	for (var i=0; i<count && rules_cnt < max_rules; i++)
	{
		rules_cnt++;
		str += "&d_"+rules_cnt+"_0="+escape(data[rules_cnt][0]);
		str += "&d_"+rules_cnt+"_1="+escape(data[rules_cnt][1]);
		str += "&d_"+rules_cnt+"_2="+escape(data[rules_cnt][2]);
		str += "&d_"+rules_cnt+"_3="+escape(data[rules_cnt][3]);
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
//---------------------------------------------DHCP NetBIOS------add------------------------------------------/
function on_check_dhcpsvr()
{
	var f = get_obj("frm");
	f.startip.disabled = f.endip.disabled = f.leasetime.disabled = !f.dhcpsvr.checked;
<?	
	if(query("/runtime/func/router_netbios")==1)
	{
		echo "f.netbios_enable.disabled = !f.dhcpsvr.checked;\n";
		echo "on_check_netbios();\n";
	}
?>
}

function on_check_netbios()
{
	var f = get_obj("frm");
	f.netbios_learn.disabled = !f.netbios_enable.checked || !f.dhcpsvr.checked;
	on_check_learn();
}

function on_check_learn()
{
	var f = get_obj("frm");
	f.winstype_0.disabled = f.winstype_1.disabled = f.winstype_2.disabled = f.winstype_3.disabled = f.netbios_learn.checked || !f.netbios_enable.checked || !f.dhcpsvr.checked;
	f.netbios_scope.disabled = f.primarywins.disabled = f.secondarywins.disabled = f.netbios_learn.checked || !f.netbios_enable.checked || !f.dhcpsvr.checked;
}

//----------------------------------------------DHCP NetBIOS--------add-End------------------------------------/

function print_daytime(str_second)
{
	if( str_second == "-1" )
		var str = "<?=$m_never?>";
	else if( str_second <= "0" )
		var str = "<?=$m_expired?>";
	else
	{
		var time = second_to_daytime(str_second);
		var str =	(time[0]>0 ? time[0]+" <?=$m_days?> " : "") +
					(time[1]>0 ? time[1]+" <?=$m_hrs?> " : "") +
					(time[2]>0 ? time[2]+" <?=$m_mins?> " : "") +
					(time[3]>0 ? time[3]+" <?=$m_secs?> " : "");
	}
	document.write(str);
}
/* -------------------------------- static dhcp start -------------------------------------- */
var d_list=[['index','hostname','ip','mac']<?
for("/runtime/dhcpserver/lease")
{
	echo ",\n['".$@."','".get("j","hostname")."','".query("ip")."','".query("mac")."']";
}
?>];
var s_list=[['index','enable','hostname','ip','mac']<?
$static_dhcp_num=0;
$max_static_dhcp=$MAX_RULES;
for("/lan/dhcp/server/pool:1/staticdhcp/entry")
{
	if(query("hostname")!=""){$static_dhcp_num++;}
	echo ",\n['".$@."','".query("enable")."','".get("j","hostname")."','".query("ip")."','".query("mac")."']";
}
$remain_static_num=$max_static_dhcp-$static_dhcp_num;
?>];

function cp_computer_info(i)
{
	if(get_obj("sel_"+i).value=='-1')
	{
		get_obj("s_en_"+i).checked	=false;
		get_obj("s_host_"+i).value	="";
		get_obj("s_ip_"+i).value	="";
		get_obj("s_mac_"+i).value	="";
	}
	else
	{
		var index = get_obj("sel_"+i).value;
		get_obj("s_host_"+i).value	= d_list[index][1];
		get_obj("s_ip_"+i).value	= d_list[index][2];
		get_obj("s_mac_"+i).value	= d_list[index][3];
	}
}
function sync_pc_mac()
{
	var f		= get_obj("frm");
	var tmp_mac	= get_mac("<?=$macaddr?>");
	for(i=1;i<7;i++)	eval("f.mac"+i+".value=tmp_mac["+i+"]");
}
function print_dhcp_sel(n)
{
	var str="<select id="+n+" size=1>";
	str+="<option value='-1'><?=$m_computer_name?></option>";
	for(i=1;i<d_list.length;i++)
	{
		str+="<option value='"+d_list[i][0]+"'>"+d_list[i][1]+"</option>";
	}
	str+="</select>";
	document.write(str);
}
function print_pre_ip(n)
{
	var str="";
	var lan_ip=get_ip("<?=$cfg_ipaddr?>");
	for(i=1;i<4;i++)
		str+="<input type=text readonly name="+n+i+" size=3 maxlength=3 value="+lan_ip[i]+" style='border:0;width=25px;text-align:center'>.";
	document.write(str);
}
function print_mac(n)
{
	var str="";
	for(i=1;i<7;i++)
	{
		str+="<input type=text name="+n+i+" size=2 maxlength=2 value=''>";
		if(i!=6)        str+=":";
	}
	document.write(str);
}
function print_edit_del(id)
{
	var str="";
	//edit
	str="<a href='<?=$MY_NAME?>.php?edit_id="+id+"'><img src='/pic/edit.jpg' border=0></a>";
	str+="&nbsp;&nbsp;";

	//del
	str+="<a href='javascript:del_confirm(\""+id+"\")'><img src='/pic/delete.jpg' border=0></a>";

	document.write(str);
}
function del_confirm(id)
{
	if(confirm("<?=$a_del_confirm?>")==false) return;
	self.location.href="<?=$MY_NAME?>.php?del_id="+id;
}

function strchk_pcname(str)
{
	if (__is_str_in_allow_chars(str, 1, "-")) return true;
	return false;
}
/* -------------------------------- static dhcp end   -------------------------------------- */

/* page init functoin */
function init()
{
	var f=get_obj("frm");
	var ipaddr;
	// init here ...
	f.ipaddr.value	= "<?=$cfg_ipaddr?>";
	f.netmask.value	= "<?=$cfg_netmask?>";
	f.domain.value	= "<?=$cfg_domain?>";
	f.dnsr.checked	= <? if ($cfg_dnsr=="1") {echo "false";} else {echo "true";} ?>;

	f.dhcpsvr.checked	= <? if ($cfg_dhcpsvr == "1") {echo "true";} else {echo "false";} ?>;
	ipaddr				= get_ip("<?=$cfg_startip?>");
	f.startip.value		= ipaddr[4];
	ipaddr				= get_ip("<?=$cfg_endip?>");
	f.endip.value		= ipaddr[4];
	f.leasetime.value	= "<?$min_leasetime=$cfg_leasetime/60; echo $min_leasetime;?>";
<?
	//NetBIOS init ...
	if(query("/runtime/func/router_netbios")==1)
	{
		if ($cfg_netbios_enable == "1") {echo "f.netbios_enable.checked = true;\n";} 
		else 							{echo "f.netbios_enable.checked = false;\n";}
		
		if ($cfg_netbios_learn == "1") 	{echo "f.netbios_learn.checked = true;\n";} 
		else 							{echo "f.netbios_learn.checked = false;\n";}
		
		echo "f.netbios_scope.value = \"".$cfg_netbios_scope."\";\n";

		if($cfg_winstype =="1"){echo "f.winstype_0.checked=true;\n";}
		else if($cfg_winstype == "2"){echo "f.winstype_1.checked=true;\n";}
		else if($cfg_winstype == "4"){echo "f.winstype_2.checked=true;\n";}
		else if($cfg_winstype == "8"){echo "f.winstype_3.checked=true;\n";}
		else {echo "f.winstype_2.checked=true;\n";}
		
		if($cfg_primarywins == ""){$cfg_primarywins="0.0.0.0";}
		if($cfg_secondarywins == ""){$cfg_secondarywins="0.0.0.0";}
		echo "f.primarywins.value = \"".$cfg_primarywins."\";\n";
		echo "f.secondarywins.value = \"".$cfg_secondarywins."\";\n";
	}
?>
	on_check_dhcpsvr();
/* -------------------------------- static dhcp start -------------------------------------- */
<?
if(query("/runtime/func/static_dhcp")=="1")
{
	echo "get_obj('static_dhcp').style.display = '';\n";
}
?>
	var i;
	for(i=1; i<s_list.length; i++)
	{
		get_obj("s_en_"+i).checked=(s_list[i][1]=="1")?true:false;
		get_obj("s_host_"+i).value=s_list[i][2];
		get_obj("s_ip_"+i).value=s_list[i][3];
		get_obj("s_mac_"+i).value=s_list[i][4];
	}
/* -------------------------------- static dhcp end   -------------------------------------- */
	<?if($router!="1"){echo "fields_disabled(f, true);\n";}?>
}
/* cancel function */
function do_cancel()
{
	self.location.href="<?=$MY_NAME?>.php?random_str="+generate_random_str();
}

/* parameter checking */
function check()
{
	// do check here ....
	var f = get_obj("frm");
	var val, min, max;
	var ip, tmp_ip, tmp_mac;
	var gzone, hzone;
	var dhcp_range_ip = new Array();


	if (is_valid_ip(f.ipaddr.value, 0)==false)
	{
		alert("<?=$a_invalid_ip?>");
		field_focus(f.ipaddr, "**");
		return false;
	}
	if (is_valid_mask(f.netmask.value)==false)
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
	if ("<?query("/gzone/enable");?>" == "1")
	{
		hzone = get_network_id(f.ipaddr.value, f.netmask.value);
		gzone = get_network_id("<?query("/gzone/ethernet/ip");?>", f.netmask.value);
		if (hzone[0] == gzone[0])
		{
			alert("<?=$a_network_conflict?>");
			field_focus(f.ipaddr, "**");
			return false;
		}
	}
	if (!is_blank(f.domain.value) && strchk_hostname(f.domain.value)==false)
	{
		alert("<?=$a_invalid_domain_name?>");
		field_focus(f.domain, "**");
		return false;
	}
	if (f.dhcpsvr.checked)
	{
		ip = get_ip(f.ipaddr.value);
		dhcp_range_ip = get_host_range_ip(f.ipaddr.value, f.netmask.value);
		min = parseInt(f.startip.value, [10]);
		max = parseInt(f.endip.value, [10]);

		if (!is_digit(f.startip.value))
		{
			alert("<?=$a_invalid_ip_range?>");
			field_focus(f.startip, "**");
			return false;
		}
		if(!is_digit(f.endip.value))
		{
			alert("<?=$a_invalid_ip_range?>");
			field_focus(f.endip, "**");
			return false;
		}
		if(!is_digit(f.leasetime.value))
		{
			alert("<?=$a_invalid_lease_time?>");
			field_focus(f.leasetime, "**");
			return false;
		}
		/*if (!is_in_range(min, 1, 254) || !is_in_range(max, 1, 254) || min > max || is_in_range(ip[4], min, max))
		{
			alert("<?=$a_invalid_ip_range?>");
			field_focus(f.startip, "**");
			return false;
		}*/
		/*
		  Use is_in_range function to determine the range is right or error
		*/
		if( !is_in_range(min, 1, 254) || !is_in_range(max, 1, 254) || min > max ||
		    min < dhcp_range_ip[0] || max > dhcp_range_ip[1] || is_in_range(ip[4], min, max))
		{
			alert("<?=$a_invalid_ip_range?>");
			field_focus(f.startip, "**");
			return false;
		}
		if(is_blank(f.leasetime.value)||f.leasetime.value==0)
		{
			alert("<?=$a_invalid_lease_time?>");
			field_focus(f.leasetime, "**");
			return false;
		}
		//----------------NetBIOS--check-add---------------------------------------/
		<?
			$netbios_func=query(/runtime/func/router_netbios);
			if($netbios_func!=1){$netbios_func=0;}
		?>
		var netbios_func=<?=$netbios_func?>;
	 	if(netbios_func==1)
	 	{	
			if(f.netbios_enable.checked && !f.netbios_learn.checked)
			{
				if(strchk_unicode(f.netbios_scope.value))
				{
					alert("<?=$a_invalid_form?>");
					f.netbios_scope.select();
					return false;
				}		
				if(f.primarywins.value != "0.0.0.0")
				{
					if (is_valid_ip(f.primarywins.value, 0)==false)
					{
						alert("<?=$a_invalid_primaywins?>");
						field_focus(f.primarywins, "**");
						return false;
					}
				}
				if(f.primarywins.value == "0.0.0.0")
				{
					if(is_valid_ip(f.secondarywins.value, 0)==true)
					{
						alert("<?=$a_primary_null?>");
						field_focus(f.primarywins, "**");
						return false;
					}
				}
				if(f.secondarywins.value != "0.0.0.0")
				{
					if (is_valid_ip(f.secondarywins.value, 0)==false)
					{
						alert("<?=$a_invalid_secondaywins?>");
						field_focus(f.secondarywins, "**");
						return false;
					}
				}
			}
	 	}
		//-------------------NetBIOS--check-add-End--------------------------------/	
	}

	ip = get_ip(f.ipaddr.value);
	f.startipaddr.value = ip[1]+"."+ip[2]+"."+ip[3]+"."+f.startip.value;
	f.endipaddr.value = ip[1]+"."+ip[2]+"."+ip[3]+"."+f.endip.value;
	f.lease_seconds.value = f.leasetime.value * 60;

	var net1, net2;
	net1 = get_network_id(f.ipaddr.value, f.netmask.value);
	net2 = get_network_id("<?=$cfg_ipaddr?>", f.netmask.value);
	if(net1[0]!=net2[0])	f.on_lan_change.value="1";
	else					f.on_lan_change.value="";
/* -------------------------------- static dhcp start -------------------------------------- */
	var s_dhcp_count=0;
	var i,j;

	for(i=1;i<=<?=$max_static_dhcp?>;i++)
	{
		var en=(get_obj("s_en_"+i).checked?1:0);
		var host=get_obj("s_host_"+i).value;
		var ip=get_obj("s_ip_"+i).value;
		var mac=get_obj("s_mac_"+i).value;
		if(en || !is_blank(host) ||!is_blank(ip) || !is_blank(mac))
		{
			s_dhcp_count++;
			//host
			if(is_blank(host) || strchk_pcname(host)==false)
			{
				alert("<?=$a_invalid_computer_name?>");
				get_obj("s_host_"+i).select();
				return false;
			}
			//ip
			if(is_blank(ip) || is_valid_ip(ip,0)==false || is_valid_ip2(ip, f.netmask.value)==false )
			{
				alert("<?=$a_invalid_ip?>");
				get_obj("s_ip_"+i).select();
				return false;
			}
			if(ip=="<?=$cfg_ipaddr?>")
			{
				alert("<?=$a_same_with_lan_ip?>");
				get_obj("s_ip_"+i).select();
				return false;
			}
			var net1, net2;
			net1 = get_network_id(ip, f.netmask.value);
			net2 = get_network_id("<?=$cfg_ipaddr?>", f.netmask.value);
			if(net1[0]!=net2[0])
			{
				alert("<?=$a_invalid_ip?>");
				get_obj("s_ip_"+i).select();
				return false;
			}
			//mac
			if(is_blank(mac) || !is_valid_mac_str(mac))
			{
				alert("<?=$a_invalid_mac?>");
				get_obj("s_mac_"+i).select();
				return false;
			}
			var tmp_mac=get_mac(mac);
			mac=tmp_mac[1]+":"+tmp_mac[2]+":"+tmp_mac[3]+":"+tmp_mac[4]+":"+tmp_mac[5]+":"+tmp_mac[6];

			for(j=i+1;j<=<?=$max_static_dhcp?>;j++)
			{
				if(host==get_obj("s_host_"+j).value)
				{
					alert("<?=$a_same_static_hostname?>");
					get_obj("s_host_"+j).select();
					return false;
				}
				if(ip==get_obj("s_ip_"+j).value)
				{
					alert("<?=$a_same_static_ip?>");
					get_obj("s_ip_"+j).select();
					return false;
				}
				tmp_mac=get_mac(get_obj("s_mac_"+j).value);
				var cmp_mac=tmp_mac[1]+":"+tmp_mac[2]+":"+tmp_mac[3]+":"+tmp_mac[4]+":"+tmp_mac[5]+":"+tmp_mac[6];
				if(mac==cmp_mac)
				{
					alert("<?=$a_same_static_mac?>");
					get_obj("s_mac_"+j).select();
					return false;
				}
			}

			data[s_dhcp_count] = new Array(4);
			data[s_dhcp_count][0] = en;
			data[s_dhcp_count][1] = host;
			data[s_dhcp_count][2] = ip;
			data[s_dhcp_count][3] = mac;
		}
	}
	s_dhcp_count++;
	for(j=s_dhcp_count; j<rules; j++)
	{
		data[j] = new Array(4);
		data[j][0] = data[j][1] = data[j][2] = data[j][3] = "";
	}
	
	var objs = document.getElementsByName("apply");
	for (var i=0; i<objs.length; i++) objs[i].disabled = true;

	rules_cnt = 0;
	send_rules(rules_per_send);
/* -------------------------------- static dhcp end   -------------------------------------- */
}
</script>


<body onload="init();" <?=$G_BODY_ATTR?>>
<form name="frm" id="frm" method="post" action="<?=$MY_NAME?>.php">
<input type="hidden" name="ACTION_POST" value="SOMETHING">
<input type="hidden" name="lease_seconds" value="">
<?require("/www/model/__banner.php");?>
<?require("/www/model/__menu_top.php");?>
<table <?=$G_MAIN_TABLE_ATTR?> height="100%">
<tr valign=top>
	<td <?=$G_MENU_TABLE_ATTR?>>
	<?require("/www/model/__menu_left.php");?>
	</td>
	<td id="maincontent">
		<div id="box_header">
		<?require($LOCALE_PATH."/dsc/dsc_".$MY_NAME.".php");?>
		<script>apply('check()'); echo("&nbsp;"); cancel('');</script>
		</div>
<!-- ________________________________ Main Content Start ______________________________ -->
		<div class="box">
			<h2><?=$m_title_router_setting?></h2>
			<?=$m_desc_router_setting?><br><br>
			<input type=hidden name=on_lan_change>
			<table cellpadding="1" cellspacing="1" border="0" width="525">
			<tr>
				<td class="r_tb" width="200"><?=$m_router_ipaddr?> :</td>
				<td class="l_tb">&nbsp;&nbsp;
					<input name="ipaddr" type="text" id="ipaddr" size="20" maxlength="15" value="">
				</td>
			</tr>
			<tr>
				<td class="r_tb"><?=$m_subnet_mask?> :</td>
				<td class="l_tb">&nbsp;&nbsp;
					<input name="netmask" type="text" id="netmask" size="20" maxlength="15" value="">
				</td>
			</tr>
			<tr>
				<td class="r_tb"><?=$m_domain_name?> :</td>
				<td class="l_tb">&nbsp;&nbsp;
					<input name="domain" type="text" id="domain" size="40" maxlength="30" value="">
				</td>
			</tr>
			<tr>
				<td class="r_tb"><?=$m_enable_dnsr?> :</td>
				<td class="l_tb">&nbsp;&nbsp;
					<input name="dnsr" type=checkbox id="dnsr" value="enable">
				</td>
			</tr>
			</table>
		</div>
		<div class="box">
			<h2><?=$m_title_dhcp_svr?></h2>
			<?=$m_desc_dhcp_svr?><br><br>
			<table width="525" border=0 cellPadding=1 cellSpacing=1>
			<tr>
				<td class="r_tb" width="200"><?=$m_enable_dhcp?> :</td>
				<td class="l_tb">&nbsp;&nbsp;
					<input name="dhcpsvr" type=checkbox id="dhcpsvr" onClick="on_check_dhcpsvr()" value="1">
				</td>
			</tr>
			<tr>
				<td class="r_tb"><?=$m_dhcp_range?> :</td>
				<td class="l_tb">&nbsp;&nbsp;
					<input name="startip" type="text" id="startip" size="3" maxlength="3" value="">
					<input name="startipaddr" type="hidden" id="startipaddr" value="">
					&nbsp;<?=$m_to?>&nbsp;
					<input name="endip" type="text" id="endip" size="3" maxlength="3" value="">
					<input name="endipaddr" type="hidden" id="endipaddr" value="">
					<?=$m_range_comment?>
				</td>
			</tr>
			<tr>
				<td class="r_tb"><?=$m_lease_time?> :</td>
				<td class="l_tb">&nbsp;&nbsp;
					<input type="text" id="leasetime" name="leasetime" size="6" maxlength="6" value="">
					<?=$m_minutes?>
				</td>
			</tr>
			<!-----------------------------NetBIOS------------------------------------------------------------>
			<?if(query("/runtime/func/router_netbios")!=1){echo"<!--";}?>
			<tr>
				<td class="r_tb"><?=$m_netbios?> :</td>
				<td class="l_tb">&nbsp;&nbsp;
					<input name="netbios_enable" type="checkbox" id="netbios_enable" onclick="on_check_netbios();" value="1">
				</td>
			</tr>
			<tr>
				<td class="r_tb"><?=$m_netbios_learn?> :</td>
				<td class="l_tb">&nbsp;&nbsp;
					<input name="netbios_learn" type="checkbox" id="netbios_learn" onclick="on_check_learn();" value="1">
				</td>
			</tr>							
			<tr>
				<td class="r_tb"><?=$m_netbios_scope?> :</td>
				<td class="l_tb">&nbsp;&nbsp;
					<input type="text" id="netbios_scope" name="netbios_scope" size="20" maxlength="30" value=""> <?=$m_optional?>
				</td>
			</tr>
			<tr>
				<td class="r_tb"><?=$m_netbios_type?> :</td>
				<td class="l_tb">
					&nbsp;&nbsp;
					<input type="radio" id="winstype_0" name="winstype" value="1"><?=$m_broadcast?><br>							
					&nbsp;&nbsp;
					<input type="radio" id="winstype_1" name="winstype" value="2"><?=$m_ptop?><br>
					&nbsp;&nbsp;
					<input type="radio" id="winstype_2" name="winstype" value="4" checked><?=$m_mix_mode?><br>
					&nbsp;&nbsp;
					<input type="radio" id="winstype_3" name="winstype" value="8"><?=$m_hybrid?>
				</td>
			</tr>
			<tr>
				 <td class="r_tb"><?=$m_primary_wins?> :</td>
				 <td class="l_tb">&nbsp;&nbsp;
					<input type="text" id="primarywins" name="primarywins" size="20" maxlength="15" value="">
				 </td>
			</tr>
			<tr>
				 <td class="r_tb"><?=$m_secondary_wins?> :</td>
				 <td class="l_tb">&nbsp;&nbsp;
				 	<input type="text" id="secondarywins" name="secondarywins" size="20" maxlength="15" value="">
				 </td>
			</tr>
			<?if(query("/runtime/func/router_netbios")!=1){echo"-->";}?>
			<!--------------------------------END NetBIOS-------------------------------------------------------->
			</table>
		</div>
		<div class="box">
			<h2><?=$m_title_client_list?></h2>
			<table width="525" border=0 cellPadding=1 cellSpacing=1>
			<tr>
				<td class="l_tb"><?=$m_host_name?></td>
				<td class="l_tb"><?=$m_ipaddr?></td>
				<td class="l_tb"><?=$m_macaddr?></td>
				<td class="l_tb"><?=$m_expired_time?></td>
				<td></td>
			</tr>
			<tr><td></td><td></td><td></td></tr>
<?
for ("/runtime/dhcpserver/lease")
{
echo
"			<tr>\n".
"				<td class=\"l_tb\">".get(h,"hostname")."</td>\n".
"				<td class=\"l_tb\">".query("ip")."</td>\n".
"				<td class=\"l_tb\">".query("mac")."</td>\n".
"				<td class=\"l_tb\"><script>print_daytime(\"".query("expire")."\");</script></td>\n".
"			</tr>\n";
}
/*
for ("/lan/dhcp/server/pool:1/staticdhcp/entry")
{
echo
"			<tr>\n".
"				<td class=\"l_tb\">".get(h,"hostname")."</td>\n".
"				<td class=\"l_tb\">".query("ip")."</td>\n".
"				<td class=\"l_tb\">".query("mac")."</td>\n".
"				<td class=\"l_tb\">".$m_never."</td>\n".
"			</tr>\n";
}
*/
?>
			</table>
		</div>

	<!-- ________________________________ add static DHCP client start ______________________________ -->
		<div class="box" name="static_dhcp" id="static_dhcp" style="display:none">
			<h2><?echo $max_static_dhcp." - ";?><?=$m_title_add_client?></h2>
			<br><?echo $remaining_wording." : <font color=red>".$remain_static_num."</font>\n";?><br><br>
			<table borderColor=#ffffff cellSpacing=1 cellPadding=2 width=525 bgColor=#dfdfdf border=1>
			<tr>
				<td class=c_tb>&nbsp;</td>
				<td class=c_tb><?=$m_computer_name?></td>
				<td class=c_tb><?=$m_ipaddr?></td>
				<td class=c_tb><?=$m_macaddr?></td>
				<td class=c_tb>&nbsp;</td>
			</tr>
<?
$s_index=0;
while($s_index<$max_static_dhcp)
{
	$s_index++;
	echo "<tr>\n";
	echo "\t<td>\n";
	echo "\t\t<input type=checkbox id=s_en_".$s_index.">\n";
	echo "\t</td>\n";
	echo "\t<td>\n";
	echo "\t\t<input type=text id='s_host_".$s_index."' size=15 maxlength=20 value=''>\n";
	echo "\t</td>\n";
	echo "\t<td>\n";
	echo "\t\t<input type=text id='s_ip_".$s_index."' size=16 maxlength=15 value=''>\n";
	echo "\t</td>\n";
	echo "\t<td>\n";
	echo "\t\t<input type=text id='s_mac_".$s_index."' size=18 maxlength=17 value=''>\n";
	echo "\t</td>\n";
	echo "\t<td>\n";
	echo "\t\t<input type=button  value='<<' style='width:24;height:24' onClick=\"cp_computer_info('".$s_index."')\">\n";
	echo "\t\t<script>print_dhcp_sel('sel_".$s_index."');</script>\n";
	echo "\t</td>\n";
	echo "</tr>\n";
}
?>
			</table>
		</div>
	<!-- ________________________________ add static DHCP client end   ______________________________ -->
		<div id="box_bottom">
			<script>apply('check()'); echo("&nbsp;"); cancel('');</script>
		</div>
	<!-- ________________________________ add static DHCP client end ___________________________________ -->

<!-- ________________________________  Main Content End _______________________________ -->
	</td>
	<td <?=$G_HELP_TABLE_ATTR?>><?require($LOCALE_PATH."/help/h_".$MY_NAME.".php");?></td>
</tr>
</table>
<?require("/www/model/__tailer.php");?>
</form>
</body>
</html>
