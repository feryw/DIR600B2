<?
set("/sys/modelname",	"DIR-600");
set("/netbios/name",  "DIR-600");
anchor("/sys");
set("devicename",		"D-Link Wireless Router");
set("modeldescription",	"Wireless N Router");
set("vendor",			"D-Link");
set("hwversion",		"B1");
set("url",				"http:\/\/www.dlink.com.tw");
set("supporturl",		"http:\/\/support.dlink.com.tw");
set("ipaddr",			"192.168.0.1");
set("netmask",			"255.255.255.0");
set("startip",			"192.168.0.100");
set("endip",			"192.168.0.199");
set("ssid",				"dlink");
set("authtype",			"s");
set("fwinfosrv",		"wrpd.dlink.com.tw");
set("fwinfopath",		"/router/firmware/query.asp");
set("wlandriverver",		"rt2860v2 SDK3100 v1900");
set("kernel_version",		"Linux version 2.6.21");
/* Fix HNAP security issue */
if (query("/sys/user:2/name")!="")
{
	del("/sys/user:2");
}

anchor("/runtime/sys/info");
set("hardwareversion",	"Bx");	/* D-Link require for DIR-600B */

set("/lan/dhcp/server/pool:1/staticdhcp/max_client", "24");

set("/nat/vrtsrv/max_rules",			24);
set("/nat/porttrigger/max_rules",		24);
set("/security/macfilter/max_rules",	24);
set("/security/urlblocking/max_rules",	24);
set("/security/firewall/max_rules",		50);
set("/routing/route/max_rules",			32);

/* function availability */
set("/runtime/func/superg",			"0");
set("/runtime/func/wps",			"1");
set("/runtime/func/static_dhcp",	"1");
set("/runtime/func/firewall",		"1");
set("/runtime/func/log_setting",	"1");
set("/runtime/func/schedule",			"1");
set("/runtime/func/schedule/vrtsrv",	"0");
set("/runtime/func/schedule/portt", 	"0");
set("/runtime/func/schedule/macfilter", "1");
set("/runtime/func/schedule/firewall",	"1");
set("/runtime/func/neaps",			"1");
set("/runtime/func/wfadev",			"1");
set("/runtime/func/ieee80211n", 	"1");
set("/runtime/func/dis_qos",				"1");
set("/runtime/func/dis_wlan_b",				"1");
set("/runtime/func/dis_wlan_g",				"1");
set("/runtime/func/netsniper",				"1");
set("/runtime/func/widget/yahoo",		"1");
set("/runtime/func/peanut",			"1");
set("/runtime/func/qos_engine",         "1");
set("/wan/rg/inf:1/pppoe/starspeed/enable",     "1");
set("/runtime/func/panic_reboot",				"1");
set("/runtime/func/easysetup",          "1");
set("/runtime/func/stun/enabled",       "0");
?>
