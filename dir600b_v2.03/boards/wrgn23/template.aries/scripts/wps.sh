#!/bin/sh
echo [$0] $1 ... > /dev/console
CFGSCRIPT="/etc/scripts/wifi/wpscfg.php"
WPSUPSTATE="/etc/scripts/wifi/wpsupstate.php"
WPSCONF="/var/run/wps.config"
case "$1" in
init)
	PIN=`wps -g`
	devdata set -e pin=$PIN
	;;
setie)
	xmldbc -P $CFGSCRIPT -V PHY_UID=WLAN-1 > $WPSCONF
	wps -c $WPSCONF -e setup > /dev/console
	;;
eap:registrar|upnp:gdi)
	xmldbc -P $CFGSCRIPT -V PHY_UID=WLAN-1 -V PARAM=enrollee > $WPSCONF
	wps -c $WPSCONF > /dev/console &
	;;
eap:enrollee|upnp:ssr)
	xmldbc -P $CFGSCRIPT -V PHY_UID=WLAN-1 > $WPSCONF
	wps -c $WPSCONF > /dev/console &
	;;
pin)
	xmldbc -P $WPSUPSTATE -V PHY_UID=WLAN-1 -V STATE=WPS_IN_PROGRESS > /dev/console &
	event WPS.INPROGRESS
	xmldbc -P $CFGSCRIPT -V PHY_UID=WLAN-1 > $WPSCONF
	wps -c $WPSCONF -e int:pin > /dev/console &
	;;
pbc)
	xmldbc -P $WPSUPSTATE -V PHY_UID=WLAN-1 -V STATE=WPS_IN_PROGRESS > /dev/console &
	event WPS.INPROGRESS
	xmldbc -P $CFGSCRIPT -V PHY_UID=WLAN-1 -V PBC=1 > $WPSCONF
	wps -c $WPSCONF -e int:pbc > /dev/console &
	;;
restartap)
	shell="/var/run/wpsrestartap.sh"
	echo "#!/bin/sh"					>  $shell
	echo "xmldbc -P /etc/scripts/wifi/wpsset.php -V PHY_UID=WLAN-1" >> $shell
	echo "/etc/scripts/dbsave.sh"		>> $shell
	echo "service WIFI.WLAN-1 restart"	>> $shell
	echo "rm -f $shell"					>> $shell
	xmldbc -t "WPS:1:sh $shell > /dev/console"
	;;
WPS_NONE)
	xmldbc -P $WPSUPSTATE -V PHY_UID=WLAN-1 -V STATE=WPS_NONE > /dev/console &
	event WPS.NONE
	;;
WPS_IN_PROGRESS)
	xmldbc -P $WPSUPSTATE -V PHY_UID=WLAN-1 -V STATE=WPS_IN_PROGRESS > /dev/console &
	event WPS.INPROGRESS
	;;
WPS_ERROR)
	xmldbc -P $WPSUPSTATE -V PHY_UID=WLAN-1 -V STATE=WPS_ERROR > /dev/console &
	event WPS.ERROR
	;;
WPS_OVERLAP)
	xmldbc -P $WPSUPSTATE -V PHY_UID=WLAN-1 -V STATE=WPS_OVERLAP > /dev/console &
	event WPS.OVERLAP
	;;
WPS_SUCCESS)
	xmldbc -P $WPSUPSTATE -V PHY_UID=WLAN-1 -V STATE=WPS_SUCCESS > /dev/console &
	event WPS.SUCCESS
	;;
*)
	echo "usage: $0 [init|setie|eap:enrollee|eap:registrar|upnp:gdi|upnp:ssr|pin|pbc|" > /dev/console
	echo "           WPS_NONE|WPS_IN_PROGRESS|WPS_ERROR|WPS_OVERLAP|WPS_SUCCESS]" > /dev/console
	exit 9
	;;
esac
exit 0;
