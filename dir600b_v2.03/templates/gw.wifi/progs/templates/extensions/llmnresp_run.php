#!/bin/sh
echo [$0] ... > /dev/console
<? /* vi: set sw=4 ts=4: */
require("/etc/templates/troot.php");

$lanif = query("/runtime/layout/lanif");
$hostname = query("/sys/hostname");

if ($generate_start==1)
{
	echo "echo Start LLMNRESP daemon ... > /dev/console\n";
	echo "llmnresp -i ".$lanif." -r ".$hostname." & > /dev/console\n";
}
else
{
	echo "echo Stop LLMNRESP daemon ... > /dev/console\n";
	echo "killall llmnresp\n";
}
?>
