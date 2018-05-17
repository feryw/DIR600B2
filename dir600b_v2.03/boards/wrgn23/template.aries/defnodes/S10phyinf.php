<?
include "/htdocs/phplib/phyinf.php";

/* LAN */
//PHYINF_setup("ETH-1", "eth", "br0");
//PHYINF_setup("ETH-2", "eth", "br1");
/* WAN */
//PHYINF_setup("ETH-3", "eth", "eth2.2");
/* WLAN */
$wlan1 = PHYINF_setup("WLAN-1", "wifi", "ra0");
set($wlan1."/media/band", "11GN");
$wlan2 = PHYINF_setup("WLAN-2", "wifi", "ra1");
set($wlan2."/media/parent", "WLAN-1");
?>
