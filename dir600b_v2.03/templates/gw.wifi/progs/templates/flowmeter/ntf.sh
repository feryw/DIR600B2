#!/bin/sh
echo [$0] ... > /dev/console
TROOT=/etc/templates
xmldbc -A $TROOT/flowmeter/mail_notify.php -V action=makeshell > /var/run/flowmeter_sendmail.sh
sh /var/run/flowmeter_sendmail.sh
