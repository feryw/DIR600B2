<?
include "/etc/services/WIFI/wifi.php";
fwrite("w",$START, "#!/bin/sh\n");
fwrite("w", $STOP, "#!/bin/sh\n");
wifisetup("WLAN-1");

/* restart IGMP proxy, WIFI enhancement */
fwrite("a",$START, "service MULTICAST restart\n");
fwrite("a",$STOP,  "service MULTICAST restart\n");

fwrite("a",$_GLOBALS["START"], "exit 0\n");
fwrite("a",$_GLOBALS["STOP"],  "exit 0\n");
?>
