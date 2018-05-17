<? /* vi: set sw=4 ts=4: */
include "/htdocs/phplib/trace.php";
include "/htdocs/phplib/xnode.php";
include "/htdocs/phplib/phyinf.php";

function startcmd($cmd)			{fwrite(a,$_GLOBALS["START"], $cmd."\n");}
function stopcmd($cmd)			{fwrite(a,$_GLOBALS["STOP"], $cmd."\n");}
function pifsetup_error($errno)	{startcmd("exit ".$errno); stopcmd("exit ".$errno);}

function phyinf_setmedia($layout, $ifname, $media)
{
	/* Only support for WAN port now. */
	if		($layout=="1W1L" && $ifname=="ETH-2") $port = 4;
	else if	($layout=="1W2L" && $ifname=="ETH-3") $port = 4;
	else return;

	if ($media=="") $media="AUTO";
	startcmd("slinktype -i ".$port." -d ".$media);
	/* DO NOTHING at stop. */
}

function phyinf_setipv6($layout, $ifname)
{
	if ($layout=="1W2L")
	{
		if		($ifname=="ETH-1") $phy="br0";
		else if ($ifname=="ETH-2") $phy="br1";
		else if ($ifname=="ETH-3") $phy="eth2.2";
	}
	else
	{
		if		($ifname=="ETH-1") $phy="br0";
		else if	($ifname=="ETH-2") $phy="eth2.2";
	}

	if ($phy!="")
	{
		startcmd("echo 0 > /proc/sys/net/ipv6/conf/".$phy."/disable_ipv6");
		stopcmd( "echo 1 > /proc/sys/net/ipv6/conf/".$phy."/disable_ipv6");
	}
}

function phyinf_setup($ifname)
{
	$phyinf	= XNODE_getpathbytarget("", "phyinf", "uid", $ifname, 0);
	if ($phyinf=="") { pifsetup_error("9"); return; }
	if (query($phyinf."/active")!="1") { pifsetup_error("8"); return; }

	/* Get layout mode */
	$layout = query("/runtime/device/layout");
	if		($layout=="bridge") $mode = "1BRIDGE";
	else if	($layout=="router") $mode = query("/runtime/device/router/mode");
	else { pifsetup_error("9"); return; }
	if ($mode=="") $mode = "1W2L";

	/* Set media */
	$media = query($phyinf."/media/linktype");
	phyinf_setmedia($mode, $ifname, $media);

	/* Set IPv6 */
	if (isfile("/proc/net/if_inet6")==1)
	{
		/**************************************************************************
		 * For IPv6 Ready Logo, we force switch power-up at this moment.
		 * NOTE: don't use "service ENLAN start" replace this code because
		 *      we don't know what time servd really run it, we force power-up
		 *      line by line.
		 **************************************************************************/
		if ($ifname=="ETH-1")
		{
			$i=0;
			while ($i<4) {startcmd('echo "write '.$i.' 0 0x3300" > /proc/rt3052/mii/ctrl'); $i++;}
			/* need waiting until power-up take effect */
			startcmd('sleep 4');
		}
		/**********************************************************************************
		 * only enable ipv6 function at br0(LAN) and eth2.2(WAN), other disable by default
		 *********************************************************************************/
		phyinf_setipv6($mode, $ifname);
	}

	/* Set the MAC address */
	$stsp = XNODE_getpathbytarget("/runtime", "phyinf", "uid", $ifname, 0);
	if ($stsp=="")
	{
		/* The interface should be started already. */
		$reboot = "REQUIRED";
	}
	else
	{
		$mac = PHYINF_gettargetmacaddr($mode, $ifname);
		if ($mac!="")
		{
			$curr= query($stsp."/macaddr");
			$mac = tolower($mac);
			$curr= tolower($curr);
			if ($mac != $curr) $reboot = "REQUIRED";
		}
	}

	/* Mark if we need to reboot the device */
	if ($reboot!="") set("/runtime/device/reboot", $reboot);
}
?>
