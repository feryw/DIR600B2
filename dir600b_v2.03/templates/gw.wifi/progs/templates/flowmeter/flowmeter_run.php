#!/bin/sh
echo [$0] ... > /dev/console
<? /* vi: set sw=4 ts=4: */
require("/etc/templates/troot.php");
$flowmeter_pidf = "/var/run/flowmeter.pid";
$flowmeter_root = "/flowmeter";
$flowmeter_script = $template_root."/flowmeter/flowmeter_helper.sh";

$enable = query("/flowmeter/enable");
if ($enable==1)
{
	echo "echo Start Internet Usage Meter ... > /dev/console\n";

	if (query("/runtime/router/enable")!=1)
	{
		echo "echo Bridge mode selected, Internet Usage Meter is disabled ! > /dev/console\n";
	}
	else 
	{
		echo "if [ -f ".$flowmeter_pidf." ]; then\n";
		echo "	pid=`cat ".$flowmeter_pidf."`\n";
		echo "	kill -SIGUSR1 $pid\n";
		echo "else\n";
		echo "	flowmeter -p ".$flowmeter_pidf." -c ".$flowmeter_root." -s ".$flowmeter_script." &\n";
		echo "fi\n";
	}	
}
else
{
	echo "echo Stop Internet Usage Meter ... > /dev/console\n";

	if (query("/runtime/router/enable")!=1)
	{
		echo "echo Bridge mode selected, Internet Usage Meter is disabled ! > /dev/console\n";
	}
	else
	{
		echo "if [ -f ".$flowmeter_pidf." ]; then\n";
		echo "	pid=`cat ".$flowmeter_pidf."`\n";
		echo "	if [ $pid != 0 ]; then\n";
		echo "		kill $pid > /dev/console 2>&1\n";
		echo "	fi\n";
		echo "	rm -f ".$flowmeter_pidf."\n";
		echo "fi\n";
	}
}
?>
