/* vi: set sw=4 ts=4: */
/*
 *	Forwarding decision
 *	Linux ethernet bridge
 *
 *	Authors:
 *	Lennert Buytenhek		<buytenh@gnu.org>
 *
 *	$Id: br_forward.c,v 1.1.1.1 2007-05-25 06:50:00 bruce Exp $
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 */

#include <linux/ip.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/if_vlan.h>
#include <linux/netfilter_bridge.h>
#ifdef CONFIG_SOFTWARE_TURBO 
#include <linux/hw_tcpip.h>
#endif
#include "br_private.h"
#ifdef CONFIG_BRIDGE_IGMPP_PROCFS
#include <linux/in.h>
unsigned char bcast_mac_addr[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
#endif

/*+++, MAC Address Translator, Builder, 2008/12/10, start*/
#ifdef CONFIG_BRIDGE_MAT
#include <linux/jhash.h>
/*DHCP_BOOTP_FLAG_BC  start{*/
#include <net/checksum.h>
#include <linux/udp.h>
struct bootp_pkt {      /* BOOTP packet format */
 //  struct iphdr iph;   /* IP header */
 //  struct udphdr udph; /* UDP header */
	u8 op;          /* 1=request, 2=reply */
	u8 htype;       /* HW address type */
	u8 hlen;        /* HW address length */
	u8 hops;        /* Used only by gateways */
	u32 xid;        /* Transaction ID */
	u16 secs;       /* Seconds since we started */
	u16 flags;      /* Just what it says */
	u32 client_ip;      /* Client's IP address if known */
	u32 your_ip;        /* Assigned IP address */
	u32 server_ip;      /* (Next, e.g. NFS) Server's IP address */
	u32 relay_ip;       /* IP address of BOOTP relay */
	u8 hw_addr[16];     /* Client's HW address */
	u8 serv_name[64];   /* Server host name */
	u8 boot_file[128];  /* Name of boot file */
//  u8 exten[312];      /* DHCP options / BOOTP vendor extensions */
};

void dhcp_bcflags_update(struct sk_buff *skb)
{
    struct iphdr _iph, *ih;
 
    if(eth_hdr(skb)->h_proto==htons(ETH_P_IP)) //IP packets
    {
        ih = skb_header_pointer(skb, 0, sizeof(_iph), &_iph);
        if(ih->protocol==IPPROTO_UDP){
            struct udphdr _udph, *pudph;
            pudph = skb_header_pointer(skb, ih->ihl*4,sizeof(_udph), &_udph);
            if((pudph->dest==htons(0x0043))&&(ntohs(pudph->len)>=sizeof(struct bootp_pkt))) //DHCP request
            {
                struct bootp_pkt _bootppkt, *pbootph;

                pbootph = skb_header_pointer(skb, ih->ihl*4+sizeof(_udph),sizeof(_bootppkt), &_bootppkt);
                if(pbootph->op == 0x01/*DHCP Discover*/)
                {
                    if(pbootph->flags == htons(0x0000))//(Broadcast flag = 0) means unicast.
                    {
                        unsigned int  udphoff;
                      
                        udphoff = (skb)->nh.iph->ihl * 4;
                        pbootph->flags = htons(0x8000);
                        pudph->check = 0;
                        (skb)->csum = skb_checksum(skb, udphoff, (skb)->len - udphoff, 0);
                        pudph->check = csum_tcpudp_magic(ih->saddr, ih->daddr, (skb)->len - udphoff, ih->protocol, (skb)->csum);
                        skb->ip_summed = CHECKSUM_NONE;
                    }
                }
            }
        }
    }
}
 /*DHCP_BOOTP_FLAG_BC end }*/
#endif /*CONFIG_BRIDGE_MAT*/
/*+++, MAC Address Translator, Builder, 2008/12/10, end*/

static void Wlan_Tx(struct sk_buff *skb)
{
/*+++, MAC Address Translator, Builder, 2008/12/10, start*/
#ifdef CONFIG_BRIDGE_MAT
	struct net_bridge				*br=NULL;
	struct net_bridge_port	*p;
	struct ethhdr			*ethdr;
	struct iphdr						iphd, *iph;
	struct arphdr						*arphd, arphdh;
	struct postarphdr				*postarphd, postarphdh;

	p = skb->dev->br_port;
	br = p->br;
	if (br->mat_ena == MAT_ENA)
	{
		if (br->mat_ena == MAT_ENA){
			ethdr = (struct ethhdr*)skb->mac.raw;
			if(ethdr->h_proto==htons(ETH_P_IP)){
				/*IP based packets*/
				iph = skb_header_pointer(skb, 0, sizeof(iphd), &iphd);
				memcpy(ethdr->h_source, br->wl_curr_mac, ETH_ALEN);
			}else if(ethdr->h_proto==htons(ETH_P_ARP)){
				/*ARP packets*/
				arphd = skb_header_pointer(skb, 0, sizeof(arphdh), &arphdh);
				postarphd = skb_header_pointer(skb, sizeof(arphdh), sizeof(postarphdh), &postarphdh);
				memcpy(ethdr->h_source, br->wl_curr_mac, ETH_ALEN);
				memcpy(postarphd->ar_sha, br->wl_curr_mac, ETH_ALEN);
			}
		}
	}/*if (br->mat_ena == MAT_ENA)*/
#endif /*CONFIG_BRIDGE_MAT*/
/*+++, MAC Address Translator, Builder, 2008/12/10, end*/

	return;
}

static void Eth_Tx(struct sk_buff *skb)
{
/*+++, MAC Address Translator, Builder, 2008/12/10, start*/
#ifdef CONFIG_BRIDGE_MAT
	struct net_bridge				*br=NULL;
	struct net_bridge_port	*p;
	struct ethhdr			*ethdr;
	struct arphdr			*arphd, arphdh;
	struct postarphdr		*postarphd, postarphdh;

	p = skb->dev->br_port;
	br = p->br;
	if (br->mat_ena == MAT_ENA)
	{
			ethdr = (struct ethhdr*)skb->mac.raw;

			if(ethdr->h_proto==htons(ETH_P_IP)){
			if((br->mat_mode!=0)&&(memcmp(ethdr->h_source, br->wl_curr_mac, ETH_ALEN)==0)){
						memcpy(ethdr->h_source, br->dev->dev_addr, ETH_ALEN);
					}
			}else if(ethdr->h_proto==htons(ETH_P_ARP)){
				arphd = skb_header_pointer(skb, 0, sizeof(arphdh), &arphdh);
				postarphd = skb_header_pointer(skb, sizeof(arphdh), sizeof(postarphdh), &postarphdh);

			if((br->mat_mode!=0)&&(memcmp(ethdr->h_source, br->wl_curr_mac, ETH_ALEN)==0)){
						memcpy(ethdr->h_source, br->dev->dev_addr, ETH_ALEN);
						memcpy(postarphd->ar_sha, br->dev->dev_addr, ETH_ALEN);
					}
				}
	}/*if (br->mat_ena == MAT_ENA)*/
#endif /*CONFIG_BRIDGE_MAT*/
/*+++, MAC Address Translator, Builder, 2008/12/10, end*/

	return;
}

static void frame_Tx(struct sk_buff *skb)
{
	if(!strcmp(skb->dev->name, WLAN)){
		Wlan_Tx(skb);
	}else if(!strcmp(skb->dev->name, ETHERNET)){
		Eth_Tx(skb);
	}

/*+++, MAC Address Translator, Builder, 2008/12/10, start*/
#ifdef CONFIG_BRIDGE_MAT
	dhcp_bcflags_update(skb);
#endif /*CONFIG_BRIDGE_MAT*/
/*+++, MAC Address Translator, Builder, 2008/12/10, end*/

	return;
}

/* Don't forward packets to originating port or forwarding diasabled */
static inline int should_deliver(struct net_bridge_port *p, 
				 const struct sk_buff *skb)
{
#ifdef CONFIG_BRIDGE_MULTICAST_BWCTRL
	unsigned char * dest;
#endif
 	if (skb->dev == p->dev ||
 	    p->state != BR_STATE_FORWARDING)
 		return 0;

#ifdef CONFIG_BRIDGE_MULTICAST_BWCTRL
    dest = skb->mac.raw;
    if ((dest[0] & 1) && p->bandwidth !=0) {
            if ((p->accumulation + skb->len) > p->bandwidth) return 0;
            p->accumulation += skb->len;
    }
#endif

	return 1;
}

static inline unsigned packet_length(const struct sk_buff *skb)
{
	return skb->len - (skb->protocol == htons(ETH_P_8021Q) ? VLAN_HLEN : 0);
}

int br_dev_queue_push_xmit(struct sk_buff *skb)
{
	/* drop mtu oversized packets except gso */
	if (packet_length(skb) > skb->dev->mtu && !skb_is_gso(skb))
		kfree_skb(skb);
	else {
		/* ip_refrag calls ip_fragment, doesn't copy the MAC header. */
		if (nf_bridge_maybe_copy_header(skb))
			kfree_skb(skb);
		else {
			skb_push(skb, ETH_HLEN);

			dev_queue_xmit(skb);
		}
	}

	return 0;
}

int br_forward_finish(struct sk_buff *skb)
{

/*+++, MAC Address Translator, Builder, 2008/12/10, start*/
	frame_Tx(skb);
/*+++, MAC Address Translator, Builder, 2008/12/10, end*/

	return NF_HOOK(PF_BRIDGE, NF_BR_POST_ROUTING, skb, NULL, skb->dev,
		       br_dev_queue_push_xmit);

}

static void __br_deliver(const struct net_bridge_port *to, struct sk_buff *skb)
{
#ifdef CONFIG_SOFTWARE_TURBO 
		struct net_device *indev;
		indev = skb->dev;
#endif
	skb->dev = to->dev;

#ifdef CONFIG_SOFTWARE_TURBO 
///////////////////////////////////////////////////////////////////////////////////////////
///// mark this part for fastnat bridge enabled issue which force turbonat to use br0 again
/*
		if(skb->nfct)
		{
				struct ip_conntrack *ct=(struct ip_conntrack *)skb->nfct;
				if(ct->sw!=NULL && hw_tcpip && hw_tcpip->sw_refresh_out_dev)
						hw_tcpip->sw_refresh_out_dev(ct->sw, indev, to->dev);
		}
*/
///////////////////////////////////////////////////////////////////////////////////////////
#endif
	NF_HOOK(PF_BRIDGE, NF_BR_LOCAL_OUT, skb, NULL, skb->dev,
			br_forward_finish);
}

static void __br_forward(const struct net_bridge_port *to, struct sk_buff *skb)
{
	struct net_device *indev;

	indev = skb->dev;
	skb->dev = to->dev;
	skb->ip_summed = CHECKSUM_NONE;

	NF_HOOK(PF_BRIDGE, NF_BR_FORWARD, skb, indev, skb->dev,
			br_forward_finish);
}

/* called with rcu_read_lock */
void br_deliver(struct net_bridge_port *to, struct sk_buff *skb)
{
	if (should_deliver(to, skb)) {
		__br_deliver(to, skb);
		return;
	}

	kfree_skb(skb);
}

/* called with rcu_read_lock */
void br_forward(struct net_bridge_port *to, struct sk_buff *skb)
{
	if (should_deliver(to, skb)) {
		__br_forward(to, skb);
		return;
	}

	kfree_skb(skb);
}
#ifdef CONFIG_BRIDGE_IGMPP_PROCFS
static void copy_mac(unsigned char* to, unsigned char * from)
{
	#if 0
	int i;
	for(i=0; i<6; i++)
		*(to+i)=*(from+i);
	#endif
	memcpy(to, from, sizeof(uint8_t)*6 );
	return;
}
#endif

/* called under bridge lock */
static void br_flood(struct net_bridge *br, struct sk_buff *skb, int clone,
	void (*__packet_hook)(const struct net_bridge_port *p,
			      struct sk_buff *skb))
{
	struct net_bridge_port *p;
	struct net_bridge_port *prev;

	if (clone) {
		struct sk_buff *skb2;

		if ((skb2 = skb_clone(skb, GFP_ATOMIC)) == NULL) {
			br->statistics.tx_dropped++;
			return;
		}

		skb = skb2;
	}
#ifdef CONFIG_BRIDGE_IGMPP_PROCFS
	if( (atomic_read(&br->br_igmpp_table_enable) == 1)  && 				// wireless enhance enable ?!
		(memcmp(eth_hdr(skb)->h_dest, bcast_mac_addr, 6) != 0) &&		// non-broadcast packet ?!
		( (eth_hdr(skb)->h_proto == htons(ETH_P_IP)) || (eth_hdr(skb)->h_proto == htons(ETH_P_IPV6)))	) // either IPv4 or IPv6 
	{
		if (eth_hdr(skb)->h_proto == htons(ETH_P_IP) ) { // IPv4 --------------------------------------------------------------------
			list_for_each_entry_rcu(p, &br->port_list, list) {
				struct sk_buff *skb2;
				if ( (atomic_read(&p->wireless_interface) == 1)  // wireless interface
					&& (skb->nh.iph->protocol == IPPROTO_UDP) ) //	only allow UDP packets ( IPPROTO_UDP: 17)
				{ 
					/*  does group address stored in table ? */
					int groupIdx;
					groupIdx = br_igmpp_search_group_IP( &p->port_igmpp_table, skb->nh.iph->daddr);
					if (groupIdx >=0){
						/* skb_copy for each host*/
						int i;
						for(i=0; i<HOSTLIST_NUMBER; i++){	
							if (p->port_igmpp_table.group_list[groupIdx].host_list[i].used ==1){
								if ((skb2 = skb_copy(skb, GFP_ATOMIC)) == NULL) {
									br->statistics.tx_dropped++;
									kfree_skb(skb);
									return;
								}
								struct ethhdr * dest = eth_hdr(skb2);
								copy_mac( dest->h_dest, p->port_igmpp_table.group_list[groupIdx].host_list[i].mac_addr);
								if (should_deliver(p, skb2))
									__packet_hook(p, skb2);
								else
									kfree_skb(skb2);
							}// if used - END
						}// for loop - END
					}else { /* skb's destination IP address does't match in port_igmpp_table */
						/* check specific address, if skb's dest IP address match specific address, we flooding it */
						if (br_igmpp_block_grp(&skb->nh.iph->daddr))
						{
							if ((skb2 = skb_clone(skb, GFP_ATOMIC)) == NULL) {
								br->statistics.tx_dropped++;
								kfree_skb(skb);
								return;
							}
							if (should_deliver(p,skb2))
								__packet_hook(p, skb2);
							else
								kfree_skb(skb2);
						}else{ /* else, do nothing, drop!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
						}
					}// groupIdx >=0 - END
				}else{ 
					/* it's wired interface or non-UDP packets*/
					/* skb_clone.....(flooding) */
					if ((skb2 = skb_clone(skb, GFP_ATOMIC)) == NULL) {
						br->statistics.tx_dropped++;
						kfree_skb(skb);
						return;
					}
					if (should_deliver(p,skb2))
						__packet_hook(p, skb2);
					else
						kfree_skb(skb2);

				}// interface & UDP check - END
			} //list_for_each_entry_rcu() - END
		} // end of IPv4 ------------------------------------------------------------------------------------------------------------
		else { // IPv6 --------------------------------------------------------------------------------------------------------------
			list_for_each_entry_rcu(p, &br->port_list, list) {
				struct sk_buff *skb2;
				if ( (atomic_read(&p->wireless_interface) == 1)  // wireless interface 
					&& (skb->nh.ipv6h->nexthdr == NEXTHDR_UDP) ) // only allow UDP packets ( NEXTHDR_UDP: 17)
				{ 
					/*  does group address stored in table ? */
					int groupIdx;
					groupIdx = br_igmpp_search_group_IP6( &p->port_igmpp_table, skb->nh.ipv6h->daddr);
					if (groupIdx >=0){
						/* skb_copy for each host*/
						int i;
						for(i=0; i<HOSTLIST_NUMBER; i++){	
							if (p->port_igmpp_table.group_list6[groupIdx].host_list[i].used ==1){
								if ((skb2 = skb_copy(skb, GFP_ATOMIC)) == NULL) {
									br->statistics.tx_dropped++;
									kfree_skb(skb);
									return;
								}
								struct ethhdr * dest = eth_hdr(skb2);
								copy_mac( dest->h_dest, p->port_igmpp_table.group_list6[groupIdx].host_list[i].mac_addr);
								if (should_deliver(p, skb2))
									__packet_hook(p, skb2);
								else
									kfree_skb(skb2);
							}// if used - END
						}// for loop - END
					}else { /* skb's destination IP address does't match in port_igmpp_table */
						/* check specific address, if skb's dest IP address match specific address, we flooding it */
						if (br_igmpp_block_grp6(&skb->nh.ipv6h->daddr))
						{
							if ((skb2 = skb_clone(skb, GFP_ATOMIC)) == NULL) {
								br->statistics.tx_dropped++;
								kfree_skb(skb);
								return;
							}
							if (should_deliver(p,skb2))
								__packet_hook(p, skb2);
							else
								kfree_skb(skb2);
						}else{ /* else, do nothing, drop!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
						}
					}// groupIdx >=0 - END
				}else{ 
					/* it's wired interface or non-UDP packets*/
					/* skb_clone..... */
					if ((skb2 = skb_clone(skb, GFP_ATOMIC)) == NULL) {
						br->statistics.tx_dropped++;
						kfree_skb(skb);
						return;
					}
					if (should_deliver(p,skb2))
						__packet_hook(p, skb2);
					else
						kfree_skb(skb2);

				}// interface & UDP check - END
			} //list_for_each_entry_rcu() - END

		} // end of IPv6 ------------------------------------------------------------------------------------------------------------

		kfree_skb(skb);

	}else{ // (wireless enhance mode disabled) or (broadcast packet) or (neither IPv4 nor IPv6)
		prev = NULL;

		list_for_each_entry_rcu(p, &br->port_list, list) {
			if (should_deliver(p, skb)) {
				if (prev != NULL) {
					struct sk_buff *skb2;

					if ((skb2 = skb_clone(skb, GFP_ATOMIC)) == NULL) {
						br->statistics.tx_dropped++;
						kfree_skb(skb);
						return;
					}

					__packet_hook(prev, skb2);
				}

				prev = p;
			}
		}

		if (prev != NULL) {
			__packet_hook(prev, skb);
			return;
		}

		kfree_skb(skb);
	}
#else
	prev = NULL;

	list_for_each_entry_rcu(p, &br->port_list, list) {
		if (should_deliver(p, skb)) {
			if (prev != NULL) {
				struct sk_buff *skb2;

				if ((skb2 = skb_clone(skb, GFP_ATOMIC)) == NULL) {
					br->statistics.tx_dropped++;
					kfree_skb(skb);
					return;
				}

				__packet_hook(prev, skb2);
			}

			prev = p;
		}
	}

	if (prev != NULL) {
		__packet_hook(prev, skb);
		return;
	}

	kfree_skb(skb);
#endif
}


/* called with rcu_read_lock */
void br_flood_deliver(struct net_bridge *br, struct sk_buff *skb, int clone)
{
	br_flood(br, skb, clone, __br_deliver);
}

/* called under bridge lock */
void br_flood_forward(struct net_bridge *br, struct sk_buff *skb, int clone)
{
	br_flood(br, skb, clone, __br_forward);
}
