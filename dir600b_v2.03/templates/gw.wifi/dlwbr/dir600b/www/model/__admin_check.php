<?
require("/www/model/__auth_check.php");
if ($NO_NEED_NEXT_LINK!="1") {$NEXT_LINK=$MY_NAME.".php";}
if ($AUTH_GROUP!="0") {require("/www/permission_deny.php");exit;}
?>
