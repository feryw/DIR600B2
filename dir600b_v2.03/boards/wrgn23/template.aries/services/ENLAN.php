<?
fwrite("w",$START, "#!/bin/sh\n");
fwrite("a",$START, "echo \"write 0 0 0x3300\" > /proc/rt3052/mii/ctrl\n");
fwrite("a",$START, "echo \"write 1 0 0x3300\" > /proc/rt3052/mii/ctrl\n");
fwrite("a",$START, "echo \"write 2 0 0x3300\" > /proc/rt3052/mii/ctrl\n");
fwrite("a",$START, "echo \"write 3 0 0x3300\" > /proc/rt3052/mii/ctrl\n");

fwrite("w",$STOP, "#!/bin/sh\n");
fwrite("a",$STOP, "echo \"write 0 0 0x800\" > /proc/rt3052/mii/ctrl\n");
fwrite("a",$STOP, "echo \"write 1 0 0x800\" > /proc/rt3052/mii/ctrl\n");
fwrite("a",$STOP, "echo \"write 2 0 0x800\" > /proc/rt3052/mii/ctrl\n");
fwrite("a",$STOP, "echo \"write 3 0 0x800\" > /proc/rt3052/mii/ctrl\n");
?>
