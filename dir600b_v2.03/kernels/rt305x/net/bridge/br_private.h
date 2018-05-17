/*
 *	Linux ethernet bridge
 *
 *	Authors:
 *	Lennert Buytenhek		<buytenh@gnu.org>
 *
 *	$Id: br_private.h,v 1.1.1.1 2007-05-25 06:50:00 bruce Exp $
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 */

#ifndef _BR_PRIVATE_H
#define _BR_PRIVATE_H

#include <linux/netdevice.h>
#include <linux/if_bridge.h>

#ifdef CONFIG_BRIDGE_IGMPP_PROCFS
#include <linux/list.h>
#include <net/ipv6.h>
#include <asm/byteorder.h>
#endif

/*+++, MAC Address Translator, Builder, 2008/12/10, start*/
#ifdef CONFIG_BRIDGE_MAT
#include <linux/ip.h>
#include <linux/if_arp.h>
#include <linux/if_ether.h>
#include <net/ipv6.h>
#endif /*CONFIG_BRIDGE_MAT*/
/*+++, MAC Address Translator, Builder, 2008/12/10, end*/

#define BR_HASH_BITS 8
#define BR_HASH_SIZE (1 << BR_HASH_BITS)

#define BR_HOLD_TIME (1*HZ)

#define BR_PORT_BITS	10
#define BR_MAX_PORTS	(1<<BR_PORT_BITS)

#define WLAN        "ra0"
#define ETHERNET    "eth2"

/*+++, MAC Address Translator, Builder, 2008/12/10, start*/
#ifdef CONFIG_BRIDGE_MAT
#define BRIDGE_MAT_DBG 	0

#define MAT_DIS					0
#define MAT_ENA					1

#define MACCLONE_DIS			0
#define MACCLONE_AUTO			1
#define MAC_CLONE_MANUAL	2

struct postarphdr {
	unsigned char		ar_sha[ETH_ALEN];	/* sender hardware address	*/
	unsigned char		ar_sip[4];		/* sender IP address		*/
	unsigned char		ar_tha[ETH_ALEN];	/* target hardware address	*/
	unsigned char		ar_tip[4];		/* target IP address		*/
};
#endif /*CONFIG_BRIDGE_MAT*/
/*+++, MAC Address Translator, Builder, 2008/12/10, end*/

#define BR_VERSION	"2.2"

typedef struct bridge_id bridge_id;
typedef struct mac_addr mac_addr;
typedef __u16 port_id;

#ifdef CONFIG_BRIDGE_IGMPP_PROCFS
#define BR_IGMPP_MSG_ERROR		0
#define BR_IGMPP_MSG_OK			1
#define BR_IGMPP_MSG_INFO		2	
#define BR_IGMPP_MSG_WARNING		3	

#define MESSAGE_LENGTH			80
#define MESSAGE_DELIM			" \t"
#define MESSAGE_ARGC			3
#define IP_DELIM				"."
#define IP_ACCEPT_CHAR			".0123456789"
#define MAC_DELIM				":"
#define MAC_ACCEPT_CHAR			":0123456789ABCDEFabcdef"
#define ACTION_ADD				"add"
#define ACTION_REMOVE			"remove"
#define ACTION_SET_WL			"setwl"
#define ACTION_UNSET_WL			"unsetwl"
#define ACTION_ENABLE_TABLE		"enable"
#define ACTION_DISABLE_TABLE	"disable"

#define HOSTLIST_NUMBER			32		
#define GROUPLIST_NUMBER		32						

struct port_igmpp_mac_t {
	int				used;
	uint8_t				mac_addr[6];			
};

struct port_igmpp_group_t {
	int		used;
	uint32_t	ip_addr;
	struct		port_igmpp_mac_t host_list[HOSTLIST_NUMBER];
};

struct port_igmpp_group6_t {
	int				used;
	struct	in6_addr 		ip6_addr;
	struct	port_igmpp_mac_t	host_list[HOSTLIST_NUMBER];
};

struct port_igmpp_table_t {
	int				enable;	
	struct port_igmpp_group_t	group_list[GROUPLIST_NUMBER];
	struct port_igmpp_group6_t	group_list6[GROUPLIST_NUMBER];
};

struct br_mac_table_t {
	struct list_head list;
	uint32_t	ip_addr;
	uint8_t		mac_addr[6];
};

struct br_mac_table6_t {
	struct list_head list;
	struct in6_addr	ip6_addr;
	uint8_t	mac_addr[6];
};


/******** MLD header define **************************/
struct igmpp_mldhdr {
	__u8	type;
	__u8	code;
	__sum16	csum;
	__be16	maxrsp;
	__be16	resv;
	struct in6_addr mca;
};


/* Multicast Listener Discovery version 2 headers */
/* steal from kernel/net/ipv6/mcast.c */
struct igmpp_mld2_grec {
	__u8		grec_type;
	__u8		grec_auxwords;
	__be16		grec_nsrcs;
	struct in6_addr grec_mca;
	struct in6_addr grec_src[0];
};

struct igmpp_mld2_report {
	__u8	type;
	__u8	resv1;
	__sum16	csum;
	__be16	resv2;
	__be16	ngrec;
	struct	igmpp_mld2_grec grec[0];
};

struct igmpp_mld2_query {
	__u8 type;
	__u8 code;
	__sum16 csum;
	__be16 mrc;
	__be16 resv1;
	struct in6_addr mca;
#if defined(__LITTLE_ENDIAN_BITFIELD)
	__u8	qrv:3,
		suppress:1,
		resv2:4;
#elif defined(__BIG_ENDIAN_BITFIELD)
	__u8 resv2:4,
		suppress:1,
		qrv:3;
#else
#error "Please fix <asm/byteorder.h>"
#endif
	__u8 qqic;
	__be16 nsrcs;
	struct in6_addr srcs[0];
};
/****************************************************/
#endif

struct bridge_id
{
	unsigned char	prio[2];
	unsigned char	addr[6];
};

struct mac_addr
{
	unsigned char	addr[6];
};

struct net_bridge_fdb_entry
{
	struct hlist_node		hlist;
	struct net_bridge_port		*dst;

	struct rcu_head			rcu;
	atomic_t			use_count;
	unsigned long			ageing_timer;
	mac_addr			addr;
	unsigned char			is_local;
	unsigned char			is_static;

/*+++, MAC Address Translator, Builder, 2008/12/10, start*/
#ifdef CONFIG_BRIDGE_MAT
	unsigned int			ipaddr;
	struct in6_addr			global_ipv6addr[2];
	/*global_ipv6addr[0]: prefix+mac addr*/
	/*global_ipv6addr[1]: assigned by RADVD*/
	struct in6_addr			local_ipv6addr;
#endif /*CONFIG_BRIDGE_MAT*/
/*+++, MAC Address Translator, Builder, 2008/12/10, end*/

};

struct net_bridge_port
{
	struct net_bridge		*br;
	struct net_device		*dev;
	struct list_head		list;

	/* STP */
	u8				priority;
	u8				state;
	u16				port_no;
	unsigned char			topology_change_ack;
	unsigned char			config_pending;
	port_id				port_id;
	port_id				designated_port;
	bridge_id			designated_root;
	bridge_id			designated_bridge;
	u32				path_cost;
	u32				designated_cost;

	struct timer_list		forward_delay_timer;
	struct timer_list		hold_timer;
	struct timer_list		message_age_timer;
	struct kobject			kobj;
	struct rcu_head			rcu;
#ifdef CONFIG_BRIDGE_MULTICAST_BWCTRL
    unsigned int                    bandwidth;
    unsigned int                    accumulation;
    struct timer_list               bwctrl_timer;
#endif
#ifdef CONFIG_BRIDGE_PORT_FORWARD
	u8			port_forwarding;
#endif
#ifdef CONFIG_BRIDGE_IGMPP_PROCFS
	struct port_igmpp_table_t port_igmpp_table;	// two-way array

	/* wireless_interface = 1, corresponding device of net_bridge_port is wireless device. 
	 * wireless_interface = 0, corresponding device of net_bridge_port is wired device.
	 * NOTE & TODO: This variable only change by IGMPProxy, should we modify br_add_if() 
	 * 				for initial this variable ?!
	 */
	atomic_t				wireless_interface;
#endif
};

#ifdef CONFIG_BRIDGE_IOAPNL /*IO-Data AP Network Limitation*/
#define IONL_ENA        1
#define IONL_DIS        0
#define ALLOWED_TBSIZE  8
#endif /*CONFIG_BRIDGE_IOAPNL*/

struct net_bridge
{
	spinlock_t			lock;
	struct list_head		port_list;
	struct net_device		*dev;
	struct net_device_stats		statistics;
	spinlock_t			hash_lock;
	struct hlist_head		hash[BR_HASH_SIZE];
	struct list_head		age_list;
	unsigned long			feature_mask;

	/* STP */
	bridge_id			designated_root;
	bridge_id			bridge_id;
	u32				root_path_cost;
	unsigned long			max_age;
	unsigned long			hello_time;
	unsigned long			forward_delay;
	unsigned long			bridge_max_age;
	unsigned long			ageing_time;
	unsigned long			bridge_hello_time;
	unsigned long			bridge_forward_delay;

	u8				group_addr[ETH_ALEN];
	u16				root_port;
	unsigned char			stp_enabled;
	unsigned char			topology_change;
	unsigned char			topology_change_detected;

	struct timer_list		hello_timer;
	struct timer_list		tcn_timer;
	struct timer_list		topology_change_timer;
	struct timer_list		gc_timer;
	struct kobject			ifobj;
#ifdef CONFIG_BRIDGE_FORWARD_CTRL
    atomic_t                        br_forward;
    struct proc_dir_entry           * br_proc;
#endif

#ifdef CONFIG_BRIDGE_IGMPP_PROCFS
	atomic_t			br_igmpp_table_enable; // for check each port_igmpp_table conveniently
	struct proc_dir_entry		*br_igmpp_proc; // port_igmpp_table I/O with user or processes
 
	struct br_mac_table_t		br_mac_table; // IP-MAC table (IPv4), linking list structure	
	struct br_mac_table6_t		br_mac_table6; // IP-MAC table (IPv6),linking list structure	
	atomic_t					br_mac_table_enable; // for check br_mac_table and br_mac_table6 conveniently
	struct proc_dir_entry		*br_mac_proc; // br_mac_table I/O with user or processes
#endif

#ifdef CONFIG_BRIDGE_IOAPNL /*IO-Data AP Network Limitation*/
    char                        ionlif[IFNAMSIZ];
    unsigned int                ionl_sw;
    unsigned int                ipnl_ipaddr;
    unsigned int                ipnl_mask;
    unsigned int                ionl_allowed[ALLOWED_TBSIZE];
#endif /*CONFIG_BRIDGE_IOAPNL*/

/*+++, MAC Address Translator, Builder, 2008/12/10, start*/
#ifdef CONFIG_BRIDGE_MAT
	unsigned int	mat_ena;
	unsigned int  mat_mode; //0:disable, 1:auto, 2:manual
	unsigned char	wl_curr_mac[6];
	unsigned char	auto_clone_mac[6];
	unsigned char	manual_clone_mac[6];
#endif /*CONFIG_BRIDGE_MAT*/
/*+++, MAC address translator, Builder, 2008/12/10, end*/
};

extern struct notifier_block br_device_notifier;
extern const u8 br_group_address[ETH_ALEN];

/* called under bridge lock */
static inline int br_is_root_bridge(const struct net_bridge *br)
{
	return !memcmp(&br->bridge_id, &br->designated_root, 8);
}


/* br_device.c */
extern void br_dev_setup(struct net_device *dev);
extern int br_dev_xmit(struct sk_buff *skb, struct net_device *dev);

/* br_fdb.c */
extern void br_fdb_init(void);
extern void br_fdb_fini(void);
extern void br_fdb_changeaddr(struct net_bridge_port *p,
			      const unsigned char *newaddr);
extern void br_fdb_cleanup(unsigned long arg);
extern void br_fdb_delete_by_port(struct net_bridge *br,
				  const struct net_bridge_port *p, int do_all);
extern struct net_bridge_fdb_entry *__br_fdb_get(struct net_bridge *br,
						 const unsigned char *addr);
extern struct net_bridge_fdb_entry *br_fdb_get(struct net_bridge *br,
					       unsigned char *addr);
/*+++, MAC Address Translator, Builder, 2008/12/10, start*/
#ifdef CONFIG_BRIDGE_MAT
extern struct net_bridge_fdb_entry *br_fdb_ip_get(struct net_bridge *br,
					       unsigned int ipaddr);
extern struct net_bridge_fdb_entry *br_fdb_ip_get6(struct net_bridge *br,
					       struct in6_addr *ipv6addr);
#endif /*CONFIG_BRIDGE_MAT*/
/*+++, MAC Address Translator, Builder, 2008/12/10, end*/
extern void br_fdb_put(struct net_bridge_fdb_entry *ent);
extern int br_fdb_fillbuf(struct net_bridge *br, void *buf,
			  unsigned long count, unsigned long off);

/*+++, MAC Address Translator, Builder, 2008/12/10, start*/
#ifdef CONFIG_BRIDGE_MAT
extern int br_fdb_insert(struct net_bridge *br,
			 struct net_bridge_port *source,
			 const unsigned char *addr, unsigned int ipaddr);
#else
extern int br_fdb_insert(struct net_bridge *br,
			 struct net_bridge_port *source,
			 const unsigned char *addr);
#endif /*CONFIG_BRIDGE_MAT*/
/*+++, MAC Address Translator, Builder, 2008/12/10, end*/

/*+++, MAC Address Translator, Builder, 2008/12/10, start*/
#ifdef CONFIG_BRIDGE_MAT
extern void br_fdb_update(struct net_bridge *br,
			  struct net_bridge_port *source,
			  const unsigned char *addr,
			  unsigned int ip);
extern void br_fdb_update6(struct net_bridge *br,
              struct net_bridge_port *source,
              const unsigned char *addr,
              struct in6_addr *ipv6addr);
#else
extern void br_fdb_update(struct net_bridge *br,
			  struct net_bridge_port *source,
			  const unsigned char *addr);
#endif /*CONFIG_BRIDGE_MAT*/
/*+++, MAC Address Translator, Builder, 2008/12/10, end*/

/* br_forward.c */
extern void br_deliver(struct net_bridge_port *to,
		struct sk_buff *skb);
extern int br_dev_queue_push_xmit(struct sk_buff *skb);
extern void br_forward(struct net_bridge_port *to,
		struct sk_buff *skb);
extern int br_forward_finish(struct sk_buff *skb);
extern void br_flood_deliver(struct net_bridge *br,
		      struct sk_buff *skb,
		      int clone);
extern void br_flood_forward(struct net_bridge *br,
		      struct sk_buff *skb,
		      int clone);

/* br_if.c */
extern void br_port_carrier_check(struct net_bridge_port *p);
extern int br_add_bridge(const char *name);
extern int br_del_bridge(const char *name);
extern void br_cleanup_bridges(void);
extern int br_add_if(struct net_bridge *br,
	      struct net_device *dev);
extern int br_del_if(struct net_bridge *br,
	      struct net_device *dev);
extern int br_min_mtu(const struct net_bridge *br);
extern void br_features_recompute(struct net_bridge *br);
#ifdef CONFIG_BRIDGE_IGMPP_PROCFS
extern int br_igmpp_search_group_IP(struct port_igmpp_table_t *pt, uint32_t ip_addr);
extern int br_igmpp_search_group_IP6(struct port_igmpp_table_t *pt, struct in6_addr ip6_addr);

extern void br_igmpp_igmp_table_add(struct net_bridge_port *p, struct port_igmpp_table_t *pt, uint32_t mca, unsigned char * mac_addr);
extern void br_igmpp_igmp_table_remove(struct port_igmpp_table_t *pt, uint32_t mca, unsigned char * mac_addr);

extern void br_igmpp_mld_table_add(struct net_bridge_port *p, struct port_igmpp_table_t *pt, struct in6_addr mca, unsigned char * mac_addr);
extern void br_igmpp_mld_table_remove(struct port_igmpp_table_t *pt, struct in6_addr mca, unsigned char * mac_addr);
#endif

/* br_input.c */
extern int br_handle_frame_finish(struct sk_buff *skb);
extern int br_handle_frame(struct net_bridge_port *p, struct sk_buff **pskb);
#ifdef CONFIG_BRIDGE_IGMPP_PROCFS
extern int br_igmpp_block_grp(uint32_t *grp);
extern int br_igmpp_block_grp6( struct in6_addr * ip6_addr);
#endif

/* br_ioctl.c */
extern int br_dev_ioctl(struct net_device *dev, struct ifreq *rq, int cmd);
extern int br_ioctl_deviceless_stub(unsigned int cmd, void __user *arg);

/* br_netfilter.c */
#ifdef CONFIG_BRIDGE_NETFILTER
extern int br_netfilter_init(void);
extern void br_netfilter_fini(void);
#else
#define br_netfilter_init()	(0)
#define br_netfilter_fini()	do { } while(0)
#endif

/* br_stp.c */
extern void br_log_state(const struct net_bridge_port *p);
extern struct net_bridge_port *br_get_port(struct net_bridge *br,
					   u16 port_no);
extern void br_init_port(struct net_bridge_port *p);
extern void br_become_designated_port(struct net_bridge_port *p);

/* br_stp_if.c */
extern void br_stp_enable_bridge(struct net_bridge *br);
extern void br_stp_disable_bridge(struct net_bridge *br);
extern void br_stp_enable_port(struct net_bridge_port *p);
extern void br_stp_disable_port(struct net_bridge_port *p);
extern void br_stp_recalculate_bridge_id(struct net_bridge *br);
extern void br_stp_change_bridge_id(struct net_bridge *br, const unsigned char *a);
extern void br_stp_set_bridge_priority(struct net_bridge *br,
				       u16 newprio);
extern void br_stp_set_port_priority(struct net_bridge_port *p,
				     u8 newprio);
extern void br_stp_set_path_cost(struct net_bridge_port *p,
				 u32 path_cost);
extern ssize_t br_show_bridge_id(char *buf, const struct bridge_id *id);

/* br_stp_bpdu.c */
extern int br_stp_rcv(struct sk_buff *skb, struct net_device *dev,
		      struct packet_type *pt, struct net_device *orig_dev);

/* br_stp_timer.c */
extern void br_stp_timer_init(struct net_bridge *br);
extern void br_stp_port_timer_init(struct net_bridge_port *p);
extern unsigned long br_timer_value(const struct timer_list *timer);

/* br.c */
extern struct net_bridge_fdb_entry *(*br_fdb_get_hook)(struct net_bridge *br,
						       unsigned char *addr);
extern void (*br_fdb_put_hook)(struct net_bridge_fdb_entry *ent);


/* br_netlink.c */
extern void br_netlink_init(void);
extern void br_netlink_fini(void);
extern void br_ifinfo_notify(int event, struct net_bridge_port *port);

#ifdef CONFIG_SYSFS
/* br_sysfs_if.c */
extern struct sysfs_ops brport_sysfs_ops;
extern int br_sysfs_addif(struct net_bridge_port *p);

/* br_sysfs_br.c */
extern int br_sysfs_addbr(struct net_device *dev);
extern void br_sysfs_delbr(struct net_device *dev);

#else

#define br_sysfs_addif(p)	(0)
#define br_sysfs_addbr(dev)	(0)
#define br_sysfs_delbr(dev)	do { } while(0)
#endif /* CONFIG_SYSFS */

#endif
