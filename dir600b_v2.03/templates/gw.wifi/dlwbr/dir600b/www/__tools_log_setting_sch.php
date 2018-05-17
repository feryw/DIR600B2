		<div id="log_and_schedule" class="box" style="display: yes">
			<h2><?=$m_email_title?></h2>
			<table borderColor=#ffffff cellSpacing=1 cellPadding=2 width=525 bgColor=#dfdfdf border=1>
			<tr>
				<td width=35% class=l_tb><?=$m_on_log_full?></td>
				<td width=65%><input type=checkbox name=mail_full_log id=mail_full_log maxlength="60" value="1"></td>
			</tr>
			<tr>
				<td width=35% class=l_tb><?=$m_enable_sch?></td>
				<td width=65%><input type=checkbox name=mail_schedule_enable id=mail_schedule_enable maxlength="60" value="1" onclick="click_enable_sch();"></td>
			</tr>
			<tr>
				<td width=35% class=l_tb><?=$m_sch?></td>
				<td>
					<?
					
						if(query("/runtime/func/logmail") == "1")
						{
						$PREFIX		= "\t\t\t\t\t";
						$OBJID		= "log_mail_sch";
						$OBJNAME	= "log_mail_sch";
						$UNIQUEID	= $cfg_log_sch;
						require("/www/__schedule_combobox.php"); 
						}
					?>				
				
				</td>
			</tr>
		</table>
		</div>
