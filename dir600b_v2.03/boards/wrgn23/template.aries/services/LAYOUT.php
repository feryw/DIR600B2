<?
/* vi: set sw=4 ts=4:

	PORT	Switch Port	VID
	====	===========	===
	CPU		PORT6		0,2
	WAN		PORT4		2
	LAN1	PORT3		0
	LAN2	PORT2		0
	LAN3	PORT1		0
	LAN4	PORT0		0

NOTE:	We use VLAN 2 for WAN port, VLAN 0 for LAN ports.
		by David Hsieh <david_hsieh@alphanetworks.com>
*/
include "/htdocs/phplib/xnode.php";
include "/htdocs/phplib/trace.php";
include "/htdocs/phplib/phyinf.php";

function startcmd($cmd)	{fwrite(a,$_GLOBALS["START"], $cmd."\n");}
function stopcmd($cmd)	{fwrite(a,$_GLOBALS["STOP"], $cmd."\n");}
function error($errno)	{startcmd("exit ".$errno); stopcmd("exit ".$errno);}
function vlancmd($var, $val) {fwrite(a,$_GLOBALS["START"], "echo ".$val." > /proc/rt3052/vlan/".$var."\n");}

function layout_bridge()
{
	SHELL_info($START, "LAYOUT: Start bridge layout ...");

	/* Start .......................................................................... */
	/* COnfig VLAN as bridge layout. */
	/* port 6 tag, port 4,3,2,1,0 untag */
	$i=0; while ($i<5) {vlancmd("untag_port_".$i, "1"); $i++;}
	vlancmd("untag_port_6", "0");
	/* We use 2 VLAN, 0 (for LAN) & 2 (for WAN) */
	vlancmd("vid_vlan_0", "0");
	vlancmd("vid_vlan_1", "2");
	/* VLAN member port setting */
	vlancmd("mem_vlan_0", "0x5f"); /* port 6,4,3,2,1,0 */
	$i=1; while ($i<16) {vlancmd("mem_vlan_".$i, "0x00"); $i++;}
	/* Port VLAN setting */
	$i=0; while ($i<5) {vlancmd("vid_port_".$i, "0"); $i++;} /* LAN/WAN port */
	vlancmd("vid_port_6", "0");	/* CPU port */
	/* Port VLAN enable */
	$i=0; while ($i<5) {vlancmd("vlan_en_port_".$i, "1"); $i++;}
	vlancmd("vlan_en_port_6", "1");

	/* Using WAN MAC address during bridge mode. */
	$mac = PHYINF_gettargetmacaddr("1BRIDGE", "ETH-1");
	if ($mac=="") $mac="00:de:fa:30:50:10";
	startcmd("vconfig add eth2 0; ip link set eth2.0 addr ".$mac."; ip link set eth2.0 up");

	/* Create bridge interface. */
	startcmd("brctl addbr br0; brctl stp br0 off; brctl setfd br0 0");
	startcmd('brctl addif br0 eth2.0');
	startcmd('brctl addif br0 ra0');
	startcmd('brctl addif br0 ra1');
	startcmd('ip link set br0 up');

	/* Setup the runtime nodes. */
	PHYINF_setup("ETH-1", "eth", "br0");

	/* Done */
	startcmd('xmldbc -s /runtime/device/layout bridge');
	startcmd('usockc /var/gpio_ctrl BRIDGE');
	startcmd('service ENLAN start');
	startcmd('service PHYINF.ETH-1 alias PHYINF.BRIDGE-1');
	startcmd('service PHYINF.ETH-1 start');

	/* Stop ........................................................................... */
	SHELL_info($STOP, "LAYOUT: Stop bridge layout ...");
	stopcmd("service PHYINF.ETH-1 stop");
	stopcmd('service PHYINF.BRIDGE-1 delete');
	stopcmd('xmldbc -s /runtime/device/layout ""');
	stopcmd('/etc/scripts/delpathbytarget.sh /runtime phyinf uid ETH-1');
	stopcmd('brctl delif br0 ra1');
	stopcmd('brctl delif br0 ra0');
	stopcmd('brctl delif br0 eth2.0');
	stopcmd('ip link set eth2.0 down');
	stopcmd('brctl delbr br0');
	stopcmd('vconfig rem eth2.0');
	return 0;
}

function layout_router($mode)
{
	SHELL_info($START, "LAYOUT: Start router layout ...");

	/* Start .......................................................................... */
	/* Config VLAN as router mode layout. (1 WAN + 4 LAN) */
	/* port 6 tag, port 4,3,2,1,0 untag */
	$i=0; while ($i<5) {vlancmd("untag_port_".$i, "1"); $i++;}
	vlancmd("untag_port_6", "0");
	/* We use 2 VLAN, 0 (for LAN) & 2 (for WAN) */
	vlancmd("vid_vlan_0", "0");
	vlancmd("vid_vlan_1", "2");
	/* VLAN member port setting */
	vlancmd("mem_vlan_0", "0x4f");	/* port 6,3,2,1,0 */
	vlancmd("mem_vlan_1", "0x50");	/* port 6,4 */
	$i=2; while ($i<16) {vlancmd("mem_vlan_".$i, "0x00"); $i++;}
	/* Port VLAN setting */
	$i=0; while ($i<4) {vlancmd("vid_port_".$i, "0"); $i++;} /* LAN port */
	vlancmd("vid_port_4", "2");	/* WAN port */
	vlancmd("vid_port_6", "0");	/* CPU port */
	/* Port VLAN enable */
	$i=0; while ($i<5) {vlancmd("vlan_en_port_".$i, "1"); $i++;}
	vlancmd("vlan_en_port_6", "1");

	/* Setup MAC address */
	/* Check User configuration for WAN port. */
	$lanmac = PHYINF_gettargetmacaddr($mode, "ETH-1");
	if		($mode=="1W1L") $wanmac = PHYINF_gettargetmacaddr("1W1L", "ETH-2");
	else if	($mode=="1W2L") $wanmac = PHYINF_gettargetmacaddr("1W2L", "ETH-3");
	if ($wanmac=="") $wanmac = "00:de:fa:30:50:10";
	if ($lanmac=="") $lanmac = "00:de:fa:30:50:00";
	startcmd("vconfig add eth2 0; ip link set eth2.0 addr ".$lanmac."; ip link set eth2.0 up");
	startcmd("vconfig add eth2 2; ip link set eth2.2 addr ".$wanmac."; ip link set eth2.2 up");

	/* Create bridge interface. */
	startcmd("brctl addbr br0; brctl stp br0 off; brctl setfd br0 0;");
	startcmd("brctl addif br0 eth2.0");
	startcmd("brctl addif br0 ra0");
	startcmd("brctl addif br1 ra1");
	startcmd("ip link set br0 up");
	if ($mode=="1W2L")
	{
		startcmd("brctl addbr br1; brctl stp br1 off; brctl setfd br1 0;");
		startcmd("ip link set br1 up");
	}

	/* Setup the runtime nodes. */
	if ($mode=="1W1L")
	{
		PHYINF_setup("ETH-1", "eth", "br0");
		PHYINF_setup("ETH-2", "eth", "eth2.2");
		/* set Service Alias */
		startcmd('service PHYINF.ETH-1 alias PHYINF.LAN-1');
		startcmd('service PHYINF.ETH-2 alias PHYINF.WAN-1');
		/* WAN: set extension nodes for linkstatus */
		$path = XNODE_getpathbytarget("/runtime", "phyinf", "uid", "ETH-2", 0);
		startcmd('xmldbc -x '.$path.'/linkstatus "get:psts -i 4"');
	}
	else if ($mode=="1W2L")
	{
		PHYINF_setup("ETH-1", "eth", "br0");
		PHYINF_setup("ETH-2", "eth", "br1");
		PHYINF_setup("ETH-3", "eth", "eth2.2");
		/* set Service Alias */
		startcmd('service PHYINF.ETH-1 alias PHYINF.LAN-1');
		startcmd('service PHYINF.ETH-2 alias PHYINF.LAN-2');
		startcmd('service PHYINF.ETH-3 alias PHYINF.WAN-1');
		/* WAN: set extension nodes for linkstatus */
		$path = XNODE_getpathbytarget("/runtime", "phyinf", "uid", "ETH-3", 0);
		startcmd('xmldbc -x '.$path.'/linkstatus "get:psts -i 4"');
	}

	/* LAN: set extension nodes for linkstatus */
	$path = XNODE_getpathbytarget("/runtime", "phyinf", "uid", "ETH-1", 0);
	startcmd('xmldbc -x '.$path.'/linkstatus:1 "get:psts -i 3"');
	startcmd('xmldbc -x '.$path.'/linkstatus:2 "get:psts -i 2"');
	startcmd('xmldbc -x '.$path.'/linkstatus:3 "get:psts -i 1"');
	startcmd('xmldbc -x '.$path.'/linkstatus:4 "get:psts -i 0"');

	/* Done */
	startcmd("xmldbc -s /runtime/device/layout router");
	startcmd("xmldbc -s /runtime/device/router/mode ".$mode);
	startcmd("usockc /var/gpio_ctrl ROUTER");
	startcmd("service PHYINF.ETH-1 start");
	startcmd("service PHYINF.ETH-2 start");
	if ($mode=="1W2L") startcmd("service PHYINF.ETH-3 start");

	/* Stop ........................................................................... */
	SHELL_info($STOP, "LAYOUT: Stop router layout ...");
	if ($mode=="1W2L")
	{
		stopcmd("service PHYINF.ETH-3 stop");
		stopcmd("service PHYINF.LAN-2 delete");
	}
	stopcmd('service PHYINF.ETH-2 stop');
	stopcmd('service PHYINF.ETH-1 stop');
	stopcmd('service PHYINF.WAN-1 delete');
	stopcmd('service PHYINF.LAN-1 delete');
	stopcmd('xmldbc -s /runtime/device/layout ""');
	stopcmd('/etc/scripts/delpathbytarget.sh /runtime phyinf uid ETH-1');
	stopcmd('/etc/scripts/delpathbytarget.sh /runtime phyinf uid ETH-2');
	stopcmd('/etc/scripts/delpathbytarget.sh /runtime phyinf uid ETH-3');
	stopcmd('brctl delif br0 ra0');
	stopcmd('brctl delif br0 eth2.0');
	stopcmd('brctl delif br1 ra1');
	stopcmd('ip link set eth2.2 down');
	stopcmd('ip link set eth2.0 down');
	stopcmd('brctl delbr br0; brctl delbr br1');
	stopcmd('vconfig rem eth2.1; vconfig rem eth2.2');
	return 0;
}

/* everything starts from here !! */
fwrite("w",$START, "#!/bin/sh\n");
fwrite("w", $STOP, "#!/bin/sh\n");

$ret = 9;
$layout = query("/device/layout");
if ($layout=="router")
{
	/* only 1W1L & 1W2L supported for router mode. */
	$mode = query("/device/router/mode"); if ($mode!="1W1L") $mode = "1W2L";
	$ret = layout_router($mode);
}
else if ($layout=="bridge")
{
	$ret = layout_bridge();
}
error($ret);

?>
