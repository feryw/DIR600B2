<?
/* vi: set sw=4 ts=4: ---------------------------------------------------------*/
$MY_NAME		="st_wlan";
$MY_MSG_FILE	=$MY_NAME.".php";
$CATEGORY		="st";
/* --------------------------------------------------------------------------- */

/* --------------------------------------------------------------------------- */
$OTHER_META="<meta http-equiv=Refresh content='10;url=st_wlan.php'>";
require("/www/model/__html_head.php");
?>

<script>
/* page init functoin */
function show_conn_time(conn_time)
{
	var t=second_to_daytime(conn_time);
	var str;
	str=(t[0]>0 ? t[0]+" <?=$m_days?>, ":"0"+" <?=$m_days?>, ")+(t[1]>0 ? t[1]+":":"00:")+(t[2]>0 ? t[2]+":":"00:")+(t[3]>0 ? t[3]:"00");
	document.write(str);
}

</script>
<body <?=$G_BODY_ATTR?>>
<form name="frm" id="frm">
<input type="hidden" name="ACTION_POST" value="SOMETHING">
<?require("/www/model/__banner.php");?>
<?require("/www/model/__menu_top.php");?>
<table <?=$G_MAIN_TABLE_ATTR?> height="100%">
<tr valign=top>
	<td <?=$G_MENU_TABLE_ATTR?>>
	<?require("/www/model/__menu_left.php");?>
	</td>
	<td id="maincontent">
		<div id="box_header">
		<?require($LOCALE_PATH."/dsc/dsc_".$MY_NAME.".php");?>
		</div>
<!-- ________________________________ Main Content Start ______________________________ -->
		<div class="box">
			<h2><?=$m_context_title_wlan?><?query("/runtime/stats/wireless/client#");?></h2>
			<table borderColor=#ffffff cellSpacing=1 cellPadding=2 width=525 bgColor=#dfdfdf border=1>
			<tr id="box_header">
				<td class=bc_tb><?=$m_conn_time?></td>
				<td class=bc_tb><?=$m_macaddr?></td>
				<td class=bc_tb><?=$m_ipaddr?></td>
				<td class=bc_tb><?=$m_mode?></td>
				<td class=bc_tb><?=$m_rate?></td>
				<td class=bc_tb><?=$m_signal?></td>
			</tr>
			<?
			for("/runtime/stats/wireless/client")
			{
				echo "<td class=c_tb width=28%><script>show_conn_time('".query("time")."');</script></td>\n";
				$client_mac	= query("mac");
				$client_ip	= "N/A";
				$hit = 0;
				for ("/runtime/dhcpserver/lease" && !$hit)
				{
					if (query("mac") == $client_mac )
					{
						$client_ip = query("ip");
						$nohit = 1;
					}
				}
				echo "<td class=c_tb width=22%>".$client_mac."</td>\n";
				echo "<td class=c_tb width=20%>".$client_ip."</td>\n";
				echo "<td class=c_tb width=8%>".query("mode")."</td>\n";
				echo "<td class=c_tb width=8%>".query("rate")."</td>\n";
				echo "<td class=c_tb width=14%>".query("rssi")."</td></tr>\n";
			}
			?>
			</table>
		</div>

<!-- ________________________________  Main Content End _______________________________ -->
	</td>
	<td <?=$G_HELP_TABLE_ATTR?>><?require($LOCALE_PATH."/help/h_".$MY_NAME.".php");?></td>
</tr>
</table>
<?require("/www/model/__tailer.php");?>
</form>
</body>
</html>
