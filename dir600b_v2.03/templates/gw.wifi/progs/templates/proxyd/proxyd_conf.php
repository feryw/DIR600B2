<?
$lanif=query("/runtime/layout/lanif");
if($config_file=="") { $config_file="/var/etc/proxyd.conf"; }
if($lanif=="") { $lanif="br0"; }
fwrite( $config_file, "CONTROL\n{\n");
fwrite2($config_file, "\tTIMEOUT_CONNECT\t30\n");
fwrite2($config_file, "\tTIMEOUT_READ\t30\n");
fwrite2($config_file, "\tTIMEOUT_WRITE\t30\n");
fwrite2($config_file, "\tMAX_CLIENT\t32\n");
fwrite2($config_file, "}\n\n");

fwrite2($config_file, "HTTP\n{\n");
fwrite2($config_file, "\tINTERFACE\t".$lanif."\n");
fwrite2($config_file, "\tPORT\t5449\n");
fwrite2($config_file, "\tALLOW_TYPE\t{ gif jpg css png }\n");
fwrite2($config_file, "\tERROR_PAGE\n\t{\n");
fwrite2($config_file, "\t\tdefault\t/var/run/blocked.html\n");
fwrite2($config_file, "\t\t403\t/var/run/blocked.html\n");
fwrite2($config_file, "\t\t404\t/none_exist_file\n");
fwrite2($config_file, "\t}\n}\n\n");
?>
