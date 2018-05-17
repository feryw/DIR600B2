<?
if($action=="makeshell")
{
	echo "#!/bin/sh\n";
	echo "echo [\$0] ... > /dev/console\n";
	$TROOT="/etc/templates";

	//config variable
	$enable = query("/flowmeter/tc/email_notify/enable");

	$email_file_path="/var/run/flowmeter_msg.txt";
    $mail_server=query("/sys/log/mailserver");
    $email_addr=query("/sys/log/email");
    $mail_subject=query("/sys/log/subject");
    $username=query("/sys/log/username");
    $password=query("/sys/log/pass1");
    $from=query("/sys/log/sender");
	echo "xmldbc -A ".$TROOT."/flowmeter/mail_notify.php -V action=makemsg > ".$email_file_path."\n";
	if($enable==1)
	{
    if($mail_server != "" || $email_addr != "")
    {
		if($username != "")
		{
            echo "/usr/sbin/sendmail -s \"".$mail_subject."\" -S ".$mail_server." -a ".$from."  -t ".$email_addr." -u ".$username." -p ".$password." -f ".$email_file_path."\n";
        }
		else
		{
            echo "/usr/sbin/sendmail -s \"".$mail_subject."\" -S ".$mail_server." -a ".$from."  -t ".$email_addr." -f ".$email_file_path."\n ";
        }
	}
	}
}
else if($action=="makemsg")
{
	$fm_path="/runtime/flowmeter";
	$starttime=query($fm_path."/mouth")."/".query($fm_path."/day")."/".query($fm_path."/year")." , ".query($fm_path."/hour").":00:00";/*No second and minits in xmldb*/
	$currenttime= query("/runtime/time/date")." , ".query("/runtime/time/time");
	echo "INTERNET USAGE METER:\n";
	echo "Start Date/Time			: ".$starttime."\n";
	echo "Current(s) Date/Time		: ".$currenttime."\n";
	echo "Current(s) Traffic Status	: ";
		if(query($fm_path."/tc/disconn") == "1")
		{
			echo "Disconnected\n";
		}
		else
		{
			echo "Connected\n";
		}
	echo "\n";
	echo "Today\t\t=>\tConnection Time:".query($fm_path."/time/today").
		 "	Upload:".query($fm_path."/up/today").
		 "	Download:".query($fm_path."/down/today").
		 "	Total:".query($fm_path."/total/today")."\n";
	echo "Yesterday\t=>\tConnection Time:".query($fm_path."/time/yesterday").
		 "	Upload:".query($fm_path."/up/yesterday").
		 "	Download:".query($fm_path."/down/yesterday").
		 "	Total:".query($fm_path."/total/yesterday")."\n";
	echo "This week\t=>\tConnection Time:".query($fm_path."/time/week").
		 "	Upload:".query($fm_path."/up/week").
		 "	Download:".query($fm_path."/down/week").
		 "	Total:".query($fm_path."/total/week")."\n";
	echo "This Month\t=>\tConnection Time:".query($fm_path."/time/thismonth").
		 "	Upload:".query($fm_path."/up/thismonth").
		 "	Download:".query($fm_path."/down/thismonth").
		 "	Total:".query($fm_path."/total/thismonth")."\n";
	echo "Last Month\t=>\tConnection Time:".query($fm_path."/time/lastmonth").
		 "	Upload:".query($fm_path."/up/lastmonth").
		 "	Download:".query($fm_path."/down/lastmonth").
		 "	Total:".query($fm_path."/total/lastmonth")."\n\n";
	echo "Download Volume Limit\t:";
		if(query($fm_path."/tc/enable") != 0)
		{
			echo query($fm_path."/tc/downlimit")."\n";
		}
		else
		{
			echo "No Limit\n";
		}
	echo "Upload Volume Limit\t:";
		if(query($fm_path."/tc/enable") != 0)
		{
			echo query($fm_path."/tc/uplimit")."\n";
		}
		else
		{
			echo "No Limit\n";
		}
	echo "Web Notify\t\t:";
		if(query($fm_path."/web_notify/enable") == "1")
		{
			echo "\n		Download Volume Threshold:".query($fm_path."/web_notity/down_threshold")."\n";
			echo "			Upload Volume Threshold:".query($fm_path."/web_notify/up_threshold")."\n";
		}
		else
		{
			echo "Disabled\n";
		}
}
?>
