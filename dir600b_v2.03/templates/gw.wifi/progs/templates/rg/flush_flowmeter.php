# flush_flowmeter >>>
<? /* vi: set sw=4 ts=4: */ ?>
iptables -F FOR_FLOWMETER
<?
/* $action indicate the trigger action */
$action		= query("/runtime/flowmeter/action");

$LAN_IP     = query("/lan/ethernet/ip");
$WANIF		= query("/runtime/wan/inf:1/interface");

$en_downlimit	= query("/flowmeter/tc/downlimit/enable");
$en_uplimit		= query("/flowmeter/tc/uplimit/enable");
$downlimit		= query("/flowmeter/tc/downlimit/threshold");
$uplimit		= query("/flowmeter/tc/uplimit/threshold");
$discon_wan		= query("/flowmeter/tc/discon_wan");

$webnotify	= query("/flowmeter/tc/web_notify/enable");
$downthres	= query("/flowmeter/tc/web_notify/down_threshold");
$upthres	= query("/flowmeter/tc/web_notify/up_threshold");
$download	= query("/runtime/flowmeter/download");
$upload		= query("/runtime/flowmeter/upload");

set("/runtime/flowmeter/status", "");

$blockcount	= 0;
if (query("/flowmeter/enable")==1 && query("/flowmeter/tc/enable")==1)
{	
	if($action=="downlimit")
	{
		if($discon_wan==1)
		{
			if($en_downlimit==1 && $download >= $downlimit)
			{
				set("/runtime/flowmeter/status", "exceeded"); //turn INET LED to Orange
				$webpage = "down_limit_info.php";
				$blockcount++;
			}
		}
	}
	else if($action=="uplimit")
	{
		if($discon_wan==1)
		{
			if($en_uplimit==1 && $upload >= $uplimit)
			{
				set("/runtime/flowmeter/status", "exceeded"); //turn INET LED to Orange
				$webpage = "up_limit_info.php";
				$blockcount++;
			}
		}
	}
	else if($action=="downthreshold")
	{
		if($webnotify==1)
		{
			if($download >= $downthres)
			{
				$webpage = "down_threshold_info.php";
			}
		}
	}
	else if($action=="upthreshold")
	{
		if($webnotify==1)
		{
			if($upload >= $upthres)
			{
				$webpage = "up_threshold_info.php";
			}
		}
	}

	if($WANIF!="" && $webpage!="")
	{
		echo "iptables -A FOR_FLOWMETER -o ".$WANIF." -p udp --dport 53 -j ACCEPT\n";
		echo "iptables -A FOR_FLOWMETER -o ".$WANIF." -p tcp --dport 80 -m string --http_req -j HIJACK --to-url ".$LAN_IP."/".$webpage."\n";
		echo "iptables -A FOR_FLOWMETER -o ".$WANIF." -p tcp --dport 80 -j ACCEPT\n";
		if($blockcount > 0)
		{
			echo "iptables -A FOR_FLOWMETER -o ".$WANIF." -p all -j DROP\n";
		}
		$blockcount++;
	}
}
set("/runtime/rgfunc/flowmeter",$blockcount);
?>
# flush_flowmeter <<< 
