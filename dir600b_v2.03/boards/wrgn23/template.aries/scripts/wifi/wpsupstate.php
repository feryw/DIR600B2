<?/* vi: set sw=4 ts=4: */
/* update WPS status */
include "/htdocs/phplib/xnode.php";
include "/htdocs/phplib/trace.php";

$p = XNODE_getpathbytarget("/runtime", "phyinf", "uid", $PHY_UID);
set($p."/media/wps/enrollee/state", $STATE);
?>
