#!/bin/sh
echo [$0] $1 ... > /dev/console
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

*/
require("/etc/templates/troot.php");
$mii_dev = "/proc/rt3052/vlan";
$bridge = query("/bridge");
if ($bridge!=1)	{ $router_on = 1; }
else			{ $router_on = 0; }

$lanmac = query("/runtime/layout/lanmac");
$wanmac = query("/runtime/layout/wanmac");

if ($router_on==1)
{
	if ($generate_start==1)
	{
		echo "echo Start router layout ...\n";
		if (query("/runtime/router/enable")==1)
		{
			echo "echo Already in router mode!\n";
			exit;
		}

		/* port 6 tag, port 4,3,2,1,0 untag */
		echo "echo 1 > ".$mii_dev."/untag_port_0\n";
		echo "echo 1 > ".$mii_dev."/untag_port_1\n";
		echo "echo 1 > ".$mii_dev."/untag_port_2\n";
		echo "echo 1 > ".$mii_dev."/untag_port_3\n";
		echo "echo 1 > ".$mii_dev."/untag_port_4\n";
		echo "echo 0 > ".$mii_dev."/untag_port_6\n";


		/* We use 2 VLAN, 0 (for LAN) & 2 (for WAN) */
		echo "echo 0 > ".$mii_dev."/vid_vlan_0\n";
		echo "echo 2 > ".$mii_dev."/vid_vlan_1\n";

		/* VLAN member port setting */
		echo "echo 0x4f > ".$mii_dev."/mem_vlan_0\n";	/* port 6,3,2,1,0 */
		echo "echo 0x50 > ".$mii_dev."/mem_vlan_1\n";	/* port 6,4 */
		echo "echo 0x00 > ".$mii_dev."/mem_vlan_2\n";
		echo "echo 0x00 > ".$mii_dev."/mem_vlan_3\n";
		echo "echo 0x00 > ".$mii_dev."/mem_vlan_4\n";
		echo "echo 0x00 > ".$mii_dev."/mem_vlan_5\n";
		echo "echo 0x00 > ".$mii_dev."/mem_vlan_6\n";
		echo "echo 0x00 > ".$mii_dev."/mem_vlan_7\n";
		echo "echo 0x00 > ".$mii_dev."/mem_vlan_8\n";
		echo "echo 0x00 > ".$mii_dev."/mem_vlan_9\n";
		echo "echo 0x00 > ".$mii_dev."/mem_vlan_10\n";
		echo "echo 0x00 > ".$mii_dev."/mem_vlan_11\n";
		echo "echo 0x00 > ".$mii_dev."/mem_vlan_12\n";
		echo "echo 0x00 > ".$mii_dev."/mem_vlan_13\n";
		echo "echo 0x00 > ".$mii_dev."/mem_vlan_14\n";
		echo "echo 0x00 > ".$mii_dev."/mem_vlan_15\n";

		/* Port VLAN setting */
		echo "echo 0 > ".$mii_dev."/vid_port_0\n";	/* LAN port */
		echo "echo 0 > ".$mii_dev."/vid_port_1\n";	/* LAN port */
		echo "echo 0 > ".$mii_dev."/vid_port_2\n";	/* LAN port */
		echo "echo 0 > ".$mii_dev."/vid_port_3\n";	/* LAN port */
		echo "echo 2 > ".$mii_dev."/vid_port_4\n";	/* WAN port */
		echo "echo 0 > ".$mii_dev."/vid_port_6\n";	/* CPU port */

		/* Port VLAN enable */
		echo "echo 1 > ".$mii_dev."/vlan_en_port_0\n";
		echo "echo 1 > ".$mii_dev."/vlan_en_port_1\n";
		echo "echo 1 > ".$mii_dev."/vlan_en_port_2\n";
		echo "echo 1 > ".$mii_dev."/vlan_en_port_3\n";
		echo "echo 1 > ".$mii_dev."/vlan_en_port_4\n";
		echo "echo 1 > ".$mii_dev."/vlan_en_port_6\n";


		echo "ifconfig eth2.0 hw ether ".$lanmac."\n";
		echo "ifconfig eth2.2 hw ether ".$wanmac."\n";
		echo "ifconfig eth2.0 up\n";
		echo "ifconfig eth2.2 up\n";
		echo "brctl addif br0 eth2.0\n";
		echo "ifconfig br0 up\n";
		echo "rgdb -i -s /runtime/router/enable 1\n";
	}
	else
	{
		echo "brctl delif br0 ra0\n";
		echo "brctl delif br0 eth2.0\n";
		echo "ifconfig eth2.2 down\n";
		echo "ifconfig eth2.0 down\n";
		echo "rgdb -i -s /runtime/router/enable \"\"\n";
	}
}
else
{
	if ($generate_start==1)
	{
		echo "echo Start bridge layout ...\n";
		if (query("/runtime/router/enable")==0)
		{
			echo "echo Already in bridge mode!\n";
			exit;
		}

		/* port 6 tag, port 4,3,2,1,0 untag */
		echo "echo 1 > ".$mii_dev."/untag_port_0\n";
		echo "echo 1 > ".$mii_dev."/untag_port_1\n";
		echo "echo 1 > ".$mii_dev."/untag_port_2\n";
		echo "echo 1 > ".$mii_dev."/untag_port_3\n";
		echo "echo 1 > ".$mii_dev."/untag_port_4\n";
		echo "echo 0 > ".$mii_dev."/untag_port_6\n";

		/* We use 2 VLAN, 0 (for LAN) & 2 (for WAN) */
		echo "echo 0 > ".$mii_dev."/vid_vlan_0\n";
		echo "echo 2 > ".$mii_dev."/vid_vlan_1\n";

		/* VLAN member port setting */
		echo "echo 0x5f > ".$mii_dev."/mem_vlan_0\n";	/* port 6,4,3,2,1,0 */
		echo "echo 0x00 > ".$mii_dev."/mem_vlan_1\n";
		echo "echo 0x00 > ".$mii_dev."/mem_vlan_2\n";
		echo "echo 0x00 > ".$mii_dev."/mem_vlan_3\n";
		echo "echo 0x00 > ".$mii_dev."/mem_vlan_4\n";
		echo "echo 0x00 > ".$mii_dev."/mem_vlan_5\n";
		echo "echo 0x00 > ".$mii_dev."/mem_vlan_6\n";
		echo "echo 0x00 > ".$mii_dev."/mem_vlan_7\n";
		echo "echo 0x00 > ".$mii_dev."/mem_vlan_8\n";
		echo "echo 0x00 > ".$mii_dev."/mem_vlan_9\n";
		echo "echo 0x00 > ".$mii_dev."/mem_vlan_10\n";
		echo "echo 0x00 > ".$mii_dev."/mem_vlan_11\n";
		echo "echo 0x00 > ".$mii_dev."/mem_vlan_12\n";
		echo "echo 0x00 > ".$mii_dev."/mem_vlan_13\n";
		echo "echo 0x00 > ".$mii_dev."/mem_vlan_14\n";
		echo "echo 0x00 > ".$mii_dev."/mem_vlan_15\n";

		/* Port VLAN setting */
		echo "echo 0 > ".$mii_dev."/vid_port_0\n";	/* LAN port */
		echo "echo 0 > ".$mii_dev."/vid_port_1\n";	/* LAN port */
		echo "echo 0 > ".$mii_dev."/vid_port_2\n";	/* LAN port */
		echo "echo 0 > ".$mii_dev."/vid_port_3\n";	/* LAN port */
		echo "echo 0 > ".$mii_dev."/vid_port_4\n";	/* WAN port */
		echo "echo 0 > ".$mii_dev."/vid_port_6\n";	/* CPU port */

		/* Port VLAN enable */
		echo "echo 1 > ".$mii_dev."/vlan_en_port_0\n";
		echo "echo 1 > ".$mii_dev."/vlan_en_port_1\n";
		echo "echo 1 > ".$mii_dev."/vlan_en_port_2\n";
		echo "echo 1 > ".$mii_dev."/vlan_en_port_3\n";
		echo "echo 1 > ".$mii_dev."/vlan_en_port_4\n";
		echo "echo 1 > ".$mii_dev."/vlan_en_port_6\n";

		echo "ifconfig eth2.0 hw ether ".$wanmac."\n";
		echo "ifconfig eth2.0 up\n";
		echo "brctl addif br0 eth2.0\n";
		echo "ifconfig br0 up\n";
		echo "rgdb -i -s /runtime/router/enable 0\n";
	}
	else
	{
		echo "echo Stop bridge layout ...\n";
		echo "brctl delif br0 ra0\n";
		echo "brctl delif br0 eth2.0\n";
		echo "ifconfig eth2.0 down\n";
		echo "rgdb -i -s /runtime/router/enable \"\"\n";
	}
}
?>
