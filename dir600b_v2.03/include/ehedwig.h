
#ifndef __EHEDWIG_HEADER__
#define __EHEDWIG_HEADER__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

/**********************************************************************/
#define SIGNATURE_FILE		"/etc/config/image_sign"
#define XML_TEMP_FILE		"/var/tmp/fatladytemp.xml"
#define WAND_SCRIPT			"/htdocs/webinc/ewand.php"
#define BUFF_MAX_LEN		1024
#define NODES_LEN			5
#define DO_SETCFG           1
#define DO_ACTIVATE         1
#define DO_SAVE             1
#define DONT_SETCFG         0
#define DONT_ACTIVATE       0
#define DONT_SAVE           0
/* Form data, which have the name/value pair */
const char * reatwhite(char * ptr);
int hedwig_pigwidgeon(char setcfg,char activate,char save, char * reason,const char * service,const char * node,const char * value);
#endif
