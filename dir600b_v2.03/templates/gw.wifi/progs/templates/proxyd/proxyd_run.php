<?
require("/etc/templates/troot.php");

$proxyd_pid="/var/run/proxyd.pid";

// create start script file
$filename="/var/run/proxyd_start.sh";
if($config_file=="") { $config_file="/var/etc/proxyd.conf"; }
if($config_url_file=="") { $config_url_file="/var/etc/proxyd_url.conf"; }

fwrite( $filename, "#!/bin/sh\n");
fwrite2($filename, "echo Starting PROXYD ... > /dev/console\n");
fwrite2($filename, "rgdb -A ".$template_root."/proxyd/blocked.tmp > /var/run/blocked.html\n");
require($template_root."/proxyd/proxyd_conf.php\n");
require($template_root."/proxyd/proxyd_url_conf.php\n");
fwrite2($filename, "proxyd -f ".$config_file." -u ".$config_url_file." & > /dev/console\n");

// create stop script file
$filename="/var/run/proxyd_stop.sh";
fwrite( $filename, "#!/bin/sh\n");
fwrite2($filename, "echo Stoping PROXYD ... > /dev/console\n");
fwrite2($filename, "if [ -f ".$proxyd_pid." ]; then\n");
fwrite2($filename, "	pid=`cat ".$proxyd_pid."`\n");
fwrite2($filename, "	if [ $pid != 0 ]; then\n");
fwrite2($filename, "		kill $pid > /dev/null 2>&1\n");
fwrite2($filename, "	fi\n");
fwrite2($filename, "	rm -f ".$proxyd_pid."\n");
fwrite2($filename, "fi\n");

// create reload script file
$filename="/var/run/proxyd_reload.sh";
fwrite( $filename, "#!/bin/sh\n");
fwrite2($filename, "echo Reloading PROXYD ... > /dev/console\n");
fwrite2($filename, "if [ -f ".$proxyd_pid." ]; then\n");
fwrite2($filename, "	pid=`cat ".$proxyd_pid."`\n");
fwrite2($filename, "	if [ $pid != 0 ]; then\n");
fwrite2($filename, "		kill -SIGUSR1 $pid > /dev/null 2>&1\n");
fwrite2($filename, "	fi\n");
fwrite2($filename, "fi\n");
?>
