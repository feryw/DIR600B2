<?

echo "var day_sch =  new Array();\n";
echo "var time_sch = new Array();\n";
$count_sch = 1;
for("/sys/schedule/entry")
{
	/* Days */
	$day_sch = 0;
	if (query("sun")==1)	{ $day_sch = $day_sch + 1;}
	if (query("mon")==1)	{ $day_sch = $day_sch + 2;}
	if (query("tue")==1)	{ $day_sch = $day_sch + 4;}
	if (query("wed")==1)	{ $day_sch = $day_sch + 8; }
	if (query("thu")==1)	{ $day_sch = $day_sch + 16; }
	if (query("fri")==1)	{ $day_sch = $day_sch + 32;}
	if (query("sat")==1)	{ $day_sch = $day_sch + 64;}
	/* TIME */
	$stime	= query("starttime");
	$etime	= query("endtime");
	
	echo "time_sch[".$count_sch."] = new Array();\n";
	echo "time_sch[".$count_sch."][0] = \"".$stime."\";\n";
	echo "time_sch[".$count_sch."][1] = \"".$etime."\";\n";
	echo "day_sch[".$count_sch."] = ".$day_sch.";\n";
	$count_sch++;
}
$count_sch--;
echo "count_sch = ".$count_sch.";\n";

$cfg_date	= query("/runtime/time/date");
$cfg_time	= query("/runtime/time/time");
echo "cfg_time = \"".$cfg_time."\";\n";
echo "cfg_date = \"".$cfg_date."\";\n";

?>
