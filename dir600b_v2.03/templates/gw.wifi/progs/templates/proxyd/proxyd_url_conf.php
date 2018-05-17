<?
	if($config_url_file=="") { $config_url_file="/var/etc/proxyd_url.conf"; }

	if (query("/security/urlblocking/action")=="1")
	{
		fwrite($config_url_file, "1\n");

		for ("/security/urlblocking/entry")
		{
			$enable = query("enable");
			$target_url=query("url");
			if ($target_url!="" && $enable!=0)
			{
				$UNIQUEID = query("schedule/id");
				$TIMESTRING = "";
				if ($UNIQUEID!="") { require("/etc/templates/rg/__schedule_proxyd.php"); }

				fwrite2($config_url_file, $target_url."\t".$TIMESTRING."\n");
				$urlcount++;
			}
		}
	}
	else
	{
		fwrite($config_url_file, "0\n");

		for ("/security/urlblocking/entry")
		{
			$enable = query("enable");
			$target_url=query("url");
			if ($target_url!="" && $enable!=0)
			{
				$UNIQUEID = query("schedule/id");
				$TIMESTRING = "";
				if ($UNIQUEID!="") { require("/etc/templates/rg/__schedule_proxyd.php"); }

				fwrite2($config_url_file, $target_url."\t".$TIMESTRING."\n");
				$urlcount++;
			}
		}
	}

?>
