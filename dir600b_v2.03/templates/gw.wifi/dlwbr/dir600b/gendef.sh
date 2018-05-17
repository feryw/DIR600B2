#!/bin/sh
RGDB="./tools/alpha/rgdb/rgdb -S $1"

WANIF="/wan/rg/inf:1"
WSTAT="$WANIF/static"
WDHCP="$WANIF/dhcp"
WPPOE="$WANIF/pppoe"
WPPTP="$WANIF/pptp"
WL2TP="$WANIF/l2tp"
$RGDB -s $WANIF/mode			2
$RGDB -s $WANIF/etherlinktype	0
$RGDB -s $WSTAT/ip				""
$RGDB -s $WSTAT/netmask			""
$RGDB -s $WSTAT/gateway			""
$RGDB -s $WSTAT/clonemac		""
$RGDB -s $WSTAT/mtu				1500

$RGDB -s $WDHCP/clonemac		""
$RGDB -s $WDHCP/autodns			1
$RGDB -s $WDHCP/unicast			1
$RGDB -s $WDHCP/mtu				1500

$RGDB -s $WPPOE/mode			2
$RGDB -s $WANIF/netsniper/enable	"0"
$RGDB -s $WPPOE/staticip		""
$RGDB -s $WPPOE/user			""
$RGDB -s $WPPOE/password		""
$RGDB -s $WPPOE/acname			""
$RGDB -s $WPPOE/acservice		""
$RGDB -s $WPPOE/autoreconnect	1
$RGDB -s $WPPOE/ondemand		1
$RGDB -s $WPPOE/idletimeout		300
$RGDB -s $WPPOE/autodns			1
$RGDB -s $WPPOE/mtu				1492
$RGDB -s $WPPOE/clonemac		""

$RGDB -s $WPPTP/mode			2
$RGDB -s $WPPTP/ip				""
$RGDB -s $WPPTP/netmask			""
$RGDB -s $WPPTP/gateway			""
$RGDB -s $WPPTP/dns				""
$RGDB -s $WPPTP/serverip		""
$RGDB -s $WPPTP/user			""
$RGDB -s $WPPTP/password		""
$RGDB -s $WPPTP/autoreconnect	1
$RGDB -s $WPPTP/ondemand		1
$RGDB -s $WPPTP/idletimeout		300
$RGDB -s $WPPTP/autodns			1
$RGDB -s $WPPTP/mtu				1400

$RGDB -s $WL2TP/mode			2
$RGDB -s $WL2TP/ip				""
$RGDB -s $WL2TP/netmask			""
$RGDB -s $WL2TP/gateway			""
$RGDB -s $WL2TP/dns				""
$RGDB -s $WL2TP/serverip		""
$RGDB -s $WL2TP/user			""
$RGDB -s $WL2TP/password		""
$RGDB -s $WL2TP/autoReconnect	1
$RGDB -s $WL2TP/onDemand		1
$RGDB -s $WL2TP/idleTimeout		300
$RGDB -s $WL2TP/autodns			1
$RGDB -s $WL2TP/mtu				1400

LANIF="/lan"
LETH="$LANIF/ethernet"
LDYN="$LANIF/dhcp"
LDYNS="$LDYN/server"
LDYNS1="$LDYNS/pool:1"
LDYNR="$LDYN/relay"

$RGDB -s $LETH/ip				192.168.0.1
$RGDB -s $LETH/netmask			255.255.255.0

$RGDB -s $LDYN/mode				1
$RGDB -s $LDYNS/enable			1
$RGDB -s $LDYNS1/startip		192.168.0.100
$RGDB -s $LDYNS1/endip			192.168.0.199
$RGDB -s $LDYNS1/netmask		255.255.255.0
$RGDB -s $LDYNS1/domain			""
$RGDB -s $LDYNS1/leasetime		86400
$RGDB -s $LDYNS1/staticdhcp/enable	1

DNSR="/dnsRelay"
$RGDB -s $DNSR/mode					2
$RGDB -s $DNSR/server/primarydns	""
$RGDB -s $DNSR/server/secondarydns	""

NAT="/nat"
NATGEN="$NAT/general"
NATVSR="$NAT/vrtsrv"
NATPTR="$NAT/porttrigger"
NATDMZ="$NAT/dmzsrv"
NATPST="$NAT/passthrough"
$RGDB -s $NAT/enable					1
$RGDB -s $NATGEN/gamingmode				1
$RGDB -s $NATGEN/igmpproxy/enable		0
$RGDB -s $NATGEN/igmpproxy/enhancement	0
$RGDB -s $NATPST/pptp					1
$RGDB -s $NATPST/ipsec					1
$RGDB -s $NATPST/l2tp					1

# Virtual Server
$RGDB -s $NATVSR/max_rules			20

# Application (Port trigger)
$RGDB -s $NATPTR/max_rules			20

# DMZ
$RGDB -s $NATDMZ/enable		0
$RGDB -s $NATDMZ/ip			""

$RGDB -s $NATPST/pppoe		1
$RGDB -s $NATPST/ipsec		1
$RGDB -s $NATPST/pptp		1

STUN="/stun"
$RGDB -s $STUN/type/tcp		1
$RGDB -s $STUN/type/udp     1
$RGDB -s $STUN/type/icmp    1

SEC="/security"
SDOS="$SEC/dos"
SARP="$SEC/arp"
SLOG="$SEC/log"
SGLB="$SEC/global"
SFWL="$SEC/firewall"
SIPF="$SEC/ipfilter"
SMAC="$SEC/macfilter"
SDBK="$SEC/domainblocking"
SUBK="$SEC/urlblocking"
SPAC="$SEC/policy"
$RGDB -s $SDOS/enable			1
$RGDB -s $SARP/enable			0

$RGDB -s $SLOG/systeminfo		1
$RGDB -s $SLOG/debuginfo		0
$RGDB -s $SLOG/attackinfo		1
$RGDB -s $SLOG/droppacketinfo	1
$RGDB -s $SLOG/noticeinfo		1

$RGDB -s $SFWL/dosaction		0
$RGDB -s $SFWL/pingallow		0
$RGDB -s $SFWL/telnetallow		0
$RGDB -s $SFWL/ftpallow			0
$RGDB -s $SFWL/httpallow		0
$RGDB -s $SFWL/snmpallow		0
$RGDB -s $SFWL/httpremoteip		"0.0.0.0"
$RGDB -s $SFWL/httpremoteport	8080

# MAC filtering
$RGDB -s $SMAC/max_rules		20
$RGDB -s $SMAC/enable			0
$RGDB -s $SMAC/action			0

# Domain Blocking
$RGDB -s $SDBK/max_rules		20
$RGDB -s $SDBK/enable			0
$RGDB -s $SDBK/action			0

# URL Blocking
$RGDB -s $SUBK/max_rules		10
$RGDB -s $SUBK/enable			0
$RGDB -s $SUBK/action			0

# Firewall/IP filter
$RGDB -s $SFWL/max_rules		20
$RGDB -s $SIPF/max_rules		20

# Policy Access Control
$RGDB -s $SPAC/enable			1

UPNP="/upnp"
$RGDB -s $UPNP/enable			1
$RGDB -s $UPNP/wanctrl			1

TIME="/time"
NTPS="$TIME/ntpServer"
$RGDB -s $TIME/syncwith			0
$RGDB -s $TIME/timezone			5
$RGDB -s $TIME/daylightsaving	0
$RGDB -s $NTPS/ip				"ntp1.dlink.com"
$RGDB -s $NTPS/interval			604800

SYS="/sys"
$RGDB -s $SYS/hostname		"DIR-600"
$RGDB -s $SYS/captcha		0
$RGDB -s $SYS/easysetup/enable	1
ACC="$SYS/user"
$RGDB -s $ACC:1/name		admin
$RGDB -s $ACC:1/password	""
$RGDB -s $ACC:1/group		0

# Wireless settings
WLAN="/wireless"
$RGDB -s $WLAN/enable			1
$RGDB -s $WLAN/ssid				"dlink"
$RGDB -s $WLAN/autochannel		1
$RGDB -s $WLAN/channel			6
$RGDB -s $WLAN/beaconinterval	100
$RGDB -s $WLAN/fraglength		2346
$RGDB -s $WLAN/rtslength		2346
$RGDB -s $WLAN/preamble			1
$RGDB -s $WLAN/txrate			0
$RGDB -s $WLAN/txpower			1
$RGDB -s $WLAN/dtim				1
$RGDB -s $WLAN/wmm				1
$RGDB -s $WLAN/ssidhidden		0
$RGDB -s $WLAN/bridge/wlan2wlan	1
$RGDB -s $WLAN/wlanmode			7
$RGDB -s $WLAN/ctsmode			2
$RGDB -s $WLAN/authtype			0
$RGDB -s $WLAN/encrypttype		0
$RGDB -s $WLAN/antenna/tx       1
$RGDB -s $WLAN/antenna/rx       1
# WEP
$RGDB -s $WLAN/wep/length		64
$RGDB -s $WLAN/wep/format		2
$RGDB -s $WLAN/wep/defkey		1
$RGDB -s $WLAN/wep/key:1		""
$RGDB -s $WLAN/wep/key:2		""
$RGDB -s $WLAN/wep/key:3		""
$RGDB -s $WLAN/wep/key:4		""
# WPA
$RGDB -s $WLAN/wpa/format		1
$RGDB -s $WLAN/wpa/key			""
# WPS
$RGDB -s $WLAN/wps/enable		1
# 802.11n
$RGDB -s $WLAN/mcs/auto			1
$RGDB -s $WLAN/mcs/index		""
$RGDB -s $WLAN/bandwidth		2
$RGDB -s $WLAN/shortguardinterval	1

# QoS
QOS="/qos"
$RGDB -s $QOS/mode				0
$RGDB -s $QOS/ubicom			1
$RGDB -s $QOS/conntype			0
$RGDB -s $QOS/bandwidth/upstream	102400
$RGDB -s $QOS/bandwidth/downstream	102400
