/* vi: set sw=4 ts=4: */
/*
 *	Handle incoming frames
 *	Linux ethernet bridge
 *
 *	Authors:
 *	Lennert Buytenhek		<buytenh@gnu.org>
 *
 *	$Id: br_input.c,v 1.1.1.1 2007-05-25 06:50:00 bruce Exp $
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 */

#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/netfilter_bridge.h>
#include "br_private.h"

#ifdef CONFIG_BRIDGE_IGMPP_PROCFS
	#include <linux/ip.h>
	#include <linux/in.h>
	#include <linux/igmp.h>
#endif

#include <linux/ipv6.h>
#include <linux/in6.h>
#include <linux/inetdevice.h>
#include <linux/types.h>
#include <linux/icmpv6.h>
#include <net/ndisc.h>
#include <net/ip6_checksum.h>

/**************************************************************/
/* static int frame_Rx(struct sk_buff *skb)                   */
/* input: struct sk_buff *skb                                 */
/* return value:                                              */
/*      0: The skb is processed well and let it pass through. */
/*      1: Drop the skb.                                      */
/**************************************************************/

/*+++, MAC Address Translator, Builder, 2008/12/10, start*/
#ifdef CONFIG_BRIDGE_MAT
static void br_pass_frame_up(struct net_bridge *br, struct sk_buff *skb);
#endif /*CONFIG_BRIDGE_MAT*/
/*+++, MAC Address Translator, Builder, 2008/12/10, end*/

/*+++, MAC Address Translator, Builder, 2008/12/10, start*/
#ifdef CONFIG_BRIDGE_MAT
#define MULTICAST_V6	0xFF00
#endif /*CONFIG_BRIDGE_MAT*/
/*+++, MAC Address Translator, Builder, 2008/12/10, end*/

/*Parse ICMPv6 options and replace link layer address with wlan link layer address.*/
struct opt_hdr {
	unsigned char       opt_type;
	unsigned char       opt_len;
	unsigned char       opt_value[0];
};

static int icmpv6_update_lladdr_opt(unsigned char *opt, unsigned int optlen, unsigned int lladdr_type, unsigned char *lladdr)
{
    struct opt_hdr *icmpv6_opt;
    unsigned        l=0;
    
    while(optlen){
        icmpv6_opt = (struct opt_hdr *)(opt+(l<<3));
        l = icmpv6_opt->opt_len;
        if (icmpv6_opt->opt_type==lladdr_type){
            memcpy(icmpv6_opt->opt_value, lladdr, ETH_ALEN);
            return 1;
        }
        optlen-=l;
    }
    
    return 0;
}

static int Wlan_Rx(struct sk_buff *skb)
{

/*+++, MAC Address Translator, Builder, 2008/12/10, start*/
#ifdef CONFIG_BRIDGE_MAT
	struct net_bridge       *br;
	struct net_bridge_port  *p;
	struct sk_buff          *skb2;
	struct ethhdr           *ethdr;
	struct iphdr            iphd, *iphdp;
	struct ipv6hdr          ipv6hd, *ipv6hdp;
	struct icmp6hdr         icmpv6hd, *icmpv6hdp;
	struct arphdr           arphd, *arphdp;
	struct postarphdr       parphd, *parphdp;
	struct net_bridge_fdb_entry*    dst=NULL;
	unsigned char           multicastMAC[6] = {0x01, 0x00, 0x5E, 0x00, 0x00, 0x00};
	unsigned char           multicastMAC_v6[6] = {0x33, 0x33, 0x00, 0x00, 0x00, 0x00};
	unsigned int            dip=0;
	struct in6_addr         dipv6;
	/*--- Add your filter below ---*/

	p = skb->dev->br_port;
	br = p->br;
	if (br->mat_ena == MAT_ENA)
	{
		ethdr = (struct ethhdr*)skb->mac.raw;

		if(ethdr->h_proto == htons(ETH_P_IP)){
			/*IP based packets*/
			iphdp = skb_header_pointer(skb, 0, sizeof(struct iphdr), &iphd);
			memcpy(&dip, &(iphdp->daddr), sizeof(unsigned int));

			if (MULTICAST(dip)){
				/*Multicast case*/
				multicastMAC[3] = ((htonl(dip)>>16) & 0x000000ff) & 0x7f;
				multicastMAC[4] =  (htonl(dip)>> 8) & 0x000000ff;
				multicastMAC[5] = htonl(dip)&0x000000ff;
				memcpy(ethdr->h_dest, multicastMAC, ETH_ALEN);
			}else{
				/*Not Multicast case*/
				dst = br_fdb_ip_get(br, dip);
				if(dst!=NULL){
					memcpy(ethdr->h_dest, dst->addr.addr, ETH_ALEN);
				}
			}
		}else if(ethdr->h_proto == htons(ETH_P_ARP)){
			/*ARP packets*/
			arphdp = skb_header_pointer(skb, 0, sizeof(struct arphdr), &arphd);
			parphdp = skb_header_pointer(skb, sizeof(struct arphdr), sizeof(struct postarphdr), &parphd);
			memcpy(&dip, parphdp->ar_tip, sizeof(unsigned int));
			if(arphdp->ar_op!=htons(ARPOP_REQUEST)){
				dst = br_fdb_ip_get(br, dip);
				if(dst!=NULL){
					memcpy(ethdr->h_dest, dst->addr.addr, ETH_ALEN);
					memcpy(parphdp->ar_tha, dst->addr.addr, ETH_ALEN);
				}else{
					//dst not found.
				}
			}else{
				//Do Nothing
			}
		}else if(ethdr->h_proto == htons(ETH_P_IPV6)){
			/*IPv6 based packets*/
			/*multicast with unicast look*/
			ipv6hdp = skb_header_pointer(skb, 0, sizeof(struct ipv6hdr), &ipv6hd);
			if (ipv6hdp->daddr.in6_u.u6_addr16[0]&htonl(MULTICAST_V6) == htonl(MULTICAST_V6)){
				multicastMAC_v6[2] = (unsigned char)((ntohs(ipv6hdp->daddr.in6_u.u6_addr16[6])>>8) & (0x00ff));
				multicastMAC_v6[3] = (unsigned char)(ntohs(ipv6hdp->daddr.in6_u.u6_addr16[6]) & (0x00ff));
				multicastMAC_v6[4] = (unsigned char)((ntohs(ipv6hdp->daddr.in6_u.u6_addr16[7])>>8) & (0x00ff));
				multicastMAC_v6[5] = (unsigned char)(ntohs(ipv6hdp->daddr.in6_u.u6_addr16[7]) & (0x00ff));
				memcpy(ethdr->h_dest, multicastMAC_v6, ETH_ALEN);
			}else{
				/*Not Multicast case*/
				memcpy(&dipv6, &(ipv6hdp->daddr), sizeof(struct in6_addr));
				dst = br_fdb_ip_get6(br, &dipv6);
				if(dst!=NULL){
					memcpy(ethdr->h_dest, dst->addr.addr, ETH_ALEN);
					switch(ipv6hdp->nexthdr)
					{
						case IPPROTO_HOPOPTS:
						case  IPPROTO_ROUTING:
						case IPPROTO_FRAGMENT:
						case IPPROTO_ICMPV6:
							icmpv6hdp = skb_header_pointer(skb, sizeof(struct ipv6hdr), sizeof(struct icmp6hdr), &icmpv6hd);
							switch(icmpv6hdp->icmp6_type)
							{
								case NDISC_ROUTER_SOLICITATION:
								case NDISC_ROUTER_ADVERTISEMENT:
								case NDISC_NEIGHBOUR_SOLICITATION:
								case NDISC_NEIGHBOUR_ADVERTISEMENT:
								case NDISC_REDIRECT:
								default:
									/* printk("unkown ICMPv6 type: %x\n", icmpv6hdp->icmp6_type); */
									break;
							}
							break;
						case IPPROTO_NONE:
						case IPPROTO_DSTOPTS:
						case IPPROTO_MH:
						default:
							/* printk("unknown IPv6 packet: Next Header #: %x\n", ipv6hdp->nexthdr); */
							break;
					}
				}
			}
		}else if(ethdr->h_proto == htons(0x888e)){
			/*EAPOL packets (for WPS)*/
			/*Do nothing...*/
		}

		if((br->mat_mode!=0)&&(memcmp(ethdr->h_dest, br->wl_curr_mac, ETH_ALEN)==0)){
#if 0
			switch(br->mat_mode){
				case 0:
					printk("MAT works with disabled MAC Clone\n");
					break;
				case 1:
					printk("MAT works with Auto MAC Clone\n");
					break;
				case 2:
					printk("MAT works with Manual MAC Clone\n");
					break;
				default:
					printk("MAT works with Manual MAC Clone\n");
					break;
			}
#endif /*0*/
			if(ntohs(ethdr->h_proto) == ETH_P_IP){
				iphdp = skb_header_pointer(skb, 0, sizeof(struct iphdr), &iphd);
				memcpy(&dip, &(iphdp->daddr), sizeof(unsigned int));
				if(inet_confirm_addr((struct net_device *)&br->dev, (__be32)dip, (__be32)dip, (int)0)) {
					return 0;
				}
			}
			if((skb2 = skb_copy(skb, GFP_ATOMIC)) == NULL) {
				return 1;
			}

			ethdr = (struct ethhdr*)skb2->mac.raw;
			memcpy(ethdr->h_dest, br->dev->dev_addr, ETH_ALEN);
			if(ntohs(ethdr->h_proto) == ETH_P_ARP){
				arphdp = skb_header_pointer(skb2, 0, sizeof(struct arphdr), &arphd);
				parphdp = skb_header_pointer(skb2, sizeof(struct arphdr), sizeof(struct postarphdr), &parphd);
				memcpy(parphdp->ar_tha, br->dev->dev_addr, ETH_ALEN);
			}
			br_pass_frame_up(br, skb2);
		}
	}/*if (br->mat_ena == MAT_ENA)*/
#endif /*CONFIG_BRIDGE_MAT*/
/*+++, MAC Address Translator, Builder, 2008/12/10, end*/

	return 0;
}

static int Eth_Rx(struct sk_buff *skb)
{
/*+++, MAC Address Translator, Builder, 2008/12/10, start*/
#ifdef CONFIG_BRIDGE_MAT
	struct net_bridge       *br;
	struct net_bridge_port  *p;
	struct ethhdr           *ethdr;
	struct iphdr            iphd, *iphdp;
	struct postarphdr       parphd, *parphdp;
	struct ipv6hdr          ipv6hd, *ipv6hdp;
	struct icmp6hdr         icmpv6hd, *icmpv6hdp;
	unsigned char           *icmpv6opt;
	unsigned int            icmpv6optl;
	unsigned int            sip;
	struct in6_addr         sipv6;
	/*--- Add your filter below ---*/

	p = skb->dev->br_port;
	br = p->br;
	if (br->mat_ena == MAT_ENA)
	{
		ethdr = (struct ethhdr*)skb->mac.raw;

		if ((br->mat_mode==MACCLONE_AUTO)&&(memcmp(br->auto_clone_mac, "\0\0\0\0\0\0", ETH_ALEN)==0)) {
			memcpy(br->auto_clone_mac, ethdr->h_source, ETH_ALEN);
			memcpy(br->wl_curr_mac, br->auto_clone_mac, ETH_ALEN);
		}

		if (p->state == BR_STATE_LEARNING || p->state == BR_STATE_FORWARDING){
			if(ethdr->h_proto==htons(ETH_P_IP)){
				/*IP based packets*/
				iphdp = skb_header_pointer(skb, 0, sizeof(struct iphdr), &iphd);
				memcpy(&sip, &(iphdp->saddr), sizeof(unsigned int));
				br_fdb_update(p->br, p, ethdr->h_source, sip);

				if((br->mat_mode!=0)&&(!memcmp(ethdr->h_dest, br->wl_curr_mac, ETH_ALEN))){
					memcpy(ethdr->h_dest, br->dev->dev_addr, ETH_ALEN);
				}
			}else if(ethdr->h_proto==htons(ETH_P_ARP)){
				/*ARP packets*/
				parphdp = skb_header_pointer(skb, sizeof(struct arphdr), sizeof(struct postarphdr), &parphd);
				memcpy(&sip, parphdp->ar_sip, sizeof(unsigned int));
				br_fdb_update(p->br, p, ethdr->h_source, sip);

				if((br->mat_mode!=0)&&(!memcmp(ethdr->h_dest, br->wl_curr_mac, ETH_ALEN))){
					memcpy(ethdr->h_dest, br->dev->dev_addr, ETH_ALEN);
					memcpy(parphdp->ar_tha, br->dev->dev_addr, ETH_ALEN);
				}
			}else if(ethdr->h_proto==htons(ETH_P_IPV6)){
				/*IPv6 packets*/
				ipv6hdp = skb_header_pointer(skb, 0, sizeof(struct ipv6hdr), &ipv6hd);
				memcpy(&sipv6, &(ipv6hdp->saddr), sizeof(struct in6_addr));
				br_fdb_update6(p->br, p, ethdr->h_source, &sipv6);

				switch(ipv6hdp->nexthdr)
				{
					case IPPROTO_HOPOPTS:
					case IPPROTO_ROUTING:
					case IPPROTO_FRAGMENT:
						break;
					case IPPROTO_ICMPV6:
						icmpv6hdp = skb_header_pointer(skb, sizeof(struct ipv6hdr), sizeof(struct icmp6hdr), &icmpv6hd);
						switch(icmpv6hdp->icmp6_type)
						{
							case NDISC_ROUTER_SOLICITATION:
							case NDISC_ROUTER_ADVERTISEMENT:
								break;
							case NDISC_NEIGHBOUR_SOLICITATION:
								/* calculate the point of ICMPv6 option(s) */
								icmpv6opt = (unsigned char*)icmpv6hdp+sizeof(struct icmp6hdr)+sizeof(struct in6_addr);
								icmpv6optl = (unsigned int) (skb->tail-icmpv6opt);
                                
								/* if option(s) is found, search for SOURCE_LL_ADDR and replace it with dst->addr.addr */
								if (icmpv6optl>0){
									icmpv6_update_lladdr_opt(icmpv6opt, icmpv6optl, ND_OPT_SOURCE_LL_ADDR, br->wl_curr_mac);
								}

								/* recalculate checksum */
								icmpv6hdp->icmp6_cksum = 0;
								icmpv6hdp->icmp6_cksum = csum_ipv6_magic(&ipv6hdp->saddr,
								                                         &ipv6hdp->daddr,
								                                         (icmpv6optl+sizeof(struct icmp6hdr)+sizeof(struct in6_addr)),
								                                         IPPROTO_ICMPV6,
								                                         csum_partial((unsigned char*) (icmpv6opt-sizeof(struct icmp6hdr)-sizeof(struct in6_addr)),
								                                                      (icmpv6optl+sizeof(struct icmp6hdr)+sizeof(struct in6_addr)),
								                                                      0));
								break;
							case NDISC_NEIGHBOUR_ADVERTISEMENT:
								/* calculate the point of ICMPv6 option(s) */
								icmpv6opt = (unsigned char*)icmpv6hdp+sizeof(struct icmp6hdr)+sizeof(struct in6_addr);
								icmpv6optl = (unsigned int) (skb->tail-icmpv6opt);

								/* if option(s) is found, search for SOURCE_LL_ADDR and replace it with dst->addr.addr */
								if (icmpv6optl>0){
									icmpv6_update_lladdr_opt(icmpv6opt, icmpv6optl, ND_OPT_TARGET_LL_ADDR, br->wl_curr_mac);
								}

								/* recalculate checksum */
								icmpv6hdp->icmp6_cksum = 0;
								icmpv6hdp->icmp6_cksum = csum_ipv6_magic(&ipv6hdp->saddr,
								                                         &ipv6hdp->daddr,
								                                         (icmpv6optl+sizeof(struct icmp6hdr)+sizeof(struct in6_addr)),
								                                         IPPROTO_ICMPV6,
								                                         csum_partial((unsigned char*) (icmpv6opt-sizeof(struct icmp6hdr)-sizeof(struct in6_addr)),
								                                                      (icmpv6optl+sizeof(struct icmp6hdr)+sizeof(struct in6_addr)),
								                                                      0));
								break;
							case NDISC_REDIRECT:
							default:
								/* printk("unkown ICMPv6 type: %x\n", icmpv6hdp->icmp6_type); */
								break;
						}
						break;
					case IPPROTO_NONE:
					case IPPROTO_DSTOPTS:
					case IPPROTO_MH:
						break;
					default:
						/* printk("unknown IPv6 packet: Next Header #: %x\n", ipv6hdp->nexthdr); */
						break;
				}
			    
				if((br->mat_mode!=0)&&(!memcmp(ethdr->h_dest, br->wl_curr_mac, ETH_ALEN))){
					memcpy(ethdr->h_dest, br->dev->dev_addr, ETH_ALEN);
				}
			}else if(ethdr->h_proto==htons(0x888e)){
				/*EAPOL packets (for WPS)*/
				/*Do nothing...*/
			}else{
				return 1;            
			}
		}
	}/*if (br->mat_ena == MAT_ENA)*/
#endif /*CONFIG_BRIDGE_MAT*/
/*+++, MAC Address Translator, Builder, 2008/12/10, end*/

	return 0;
}

static int frame_Rx(struct sk_buff *skb)
{
#ifdef CONFIG_BRIDGE_IOAPNL
    struct net_bridge   *br_p;
    struct iphdr        iphd, *iphd_p;
    int                 i;

    br_p = skb->dev->br_port->br;
    if (!strcmp(skb->dev->name, br_p->ionlif)){
        //printk("skb->dev->name: %s\n", skb->dev->name);
        if (br_p->ionl_sw == IONL_ENA){
            //printk("auto bridge enabled!\n");
            if (skb->protocol == htons(ETH_P_IP)/*ETH_P_IP*/){
                //printk("IP protocol!\n");
                iphd_p = skb_header_pointer(skb, 0, sizeof(iphd), &iphd);
                for(i=0;i<ALLOWED_TBSIZE;i++){
                    /*Allowed list(GW,DNS...)*/
                    if(htonl(iphd_p->daddr) == htonl(br_p->ionl_allowed[i])){
                        //printk("Allowed list: %x\n", iphd_p->daddr);
                        return 0;
                    }
                }
                if ((htonl(iphd_p->daddr)&htonl(0x000000ff)) == htonl(0x000000ff)){
                    /*Broadcast*/
                    //printk("Broadcast: %x\n", iphd_p->daddr);
                    return 0;
                }else if((htonl(iphd_p->daddr)&htonl(0xf0000000)) == htonl(0xE0000000)){
                    /*Multicast*/
                    //printk("Multicast: %x\n", iphd_p->daddr);
                    return 0;
                }else if((htonl(iphd_p->daddr)&htonl(br_p->ipnl_mask)) != (htonl(br_p->ipnl_ipaddr)&htonl(br_p->ipnl_mask))){
                    /*Internet access*/
                    //printk("Internet access: %x(%x) -> %x(%x)\n", htonl(iphd_p->daddr), htonl(br_p->ipnl_mask), htonl(br_p->ipnl_ipaddr), htonl(br_p->ipnl_mask));
                    return 0;
                }else{
                    /*Drop the access to LAN*/
                    //printk("The access to LAN: %x\n", htonl(iphd_p->daddr));
                    return 1;
                }
            }else{
                //printk("Not IP protocol!\n");
                return 0;
            }
        }else{
            //printk("auto bridge disabled!\n");
            return 0;
        }
    }
#endif /*CONFIG_BRIDGE_IOAPNL*/

	if(!strcmp(skb->dev->name, WLAN)){
		if(Wlan_Rx(skb) == 1)
			return 1;
	}else if(!strcmp(skb->dev->name, ETHERNET)){
		if(Eth_Rx(skb) == 1)
			return 1;
	}

    return 0;
}

/* Bridge group multicast address 802.1d (pg 51). */
const u8 br_group_address[ETH_ALEN] = { 0x01, 0x80, 0xc2, 0x00, 0x00, 0x00 };

static void br_pass_frame_up(struct net_bridge *br, struct sk_buff *skb)
{
	struct net_device *indev;

	br->statistics.rx_packets++;
	br->statistics.rx_bytes += skb->len;

	indev = skb->dev;
	skb->dev = br->dev;

	NF_HOOK(PF_BRIDGE, NF_BR_LOCAL_IN, skb, indev, NULL,
		netif_receive_skb);
}

#ifdef CONFIG_BRIDGE_IGMPP_PROCFS

#ifdef CONFIG_BRIDGE_IGMPP_PROCFS_DEBUG
static void trans_32to8(uint32_t *ip, uint8_t **a)
{
    *a = (uint8_t *)ip;
    return;
}
#endif

/* br_igmpp_blcok_grp(): block specific group address for IGMP(v4)
	return 0: this addrs is ok
	retrun 1: this addrs is not allowed */
int br_igmpp_block_grp(uint32_t *grp)
{
	uint32_t baddrs[] = { htonl(0xEFFFFFFA), htonl(0xE00000FC) };  //239.255.255.250, 224.0.0.252(LLMNR)
	uint8_t baddrs_cnt = sizeof(baddrs) / sizeof(uint32_t);

	uint8_t i = 0;	
	while ( i < baddrs_cnt)
	{
		if ( *grp == baddrs[i] )
		{
			#ifdef CONFIG_BRIDGE_IGMPP_PROCFS_DEBUG
			uint8_t *ip8_addr;
			trans_32to8(grp, &ip8_addr);
			printk(KERN_INFO "[BR_MAC_PROC]-> block IPv4 group address, group addr (dst addr): %u.%u.%u.%u\n",
				*ip8_addr, *(ip8_addr+1), *(ip8_addr+2), *(ip8_addr+3)	
			);
			#endif
			return 1;
		}
		i++;
	}

	return 0;
}


/* blcok_grp6(): block specific address 
	return 0: this addrs is ok
	retrun 1: this addrs is not allowed */
int br_igmpp_block_grp6( struct in6_addr * ip6_addr)
{

	/*********************************************************************************************/
	/* check "solicited-node multicast address" (prefix: FF020000 00000000 00000001 FFXXXXXX)	*/
	if ( 	ip6_addr->s6_addr32[0] == htonl(0xFF020000) &&
			ip6_addr->s6_addr32[1] == htonl(0x00000000) &&
			ip6_addr->s6_addr32[2] == htonl(0x00000001) &&
			ip6_addr->s6_addr [12] == 0xFF					)					

	{
		#ifdef CONFIG_BRIDGE_IGMPP_PROCFS_DEBUG
		printk(KERN_INFO "[BR_MAC_PROC]-> block IPv6 group address, group addr (dst addr): %08X %08X %08X %08X\n",
				ntohl(ip6_addr->s6_addr32[0]), ntohl(ip6_addr->s6_addr32[1]),
				ntohl(ip6_addr->s6_addr32[2]), ntohl(ip6_addr->s6_addr32[3])
		);
		#endif
		/* this addrs is not allowed */
		return 1; 
	}

	/*********************************************************************************************/
	/* check LLMNR address: FF02::1:3 ==> FF020000 00000000 00000000 00010003 */
	if ( 	ip6_addr->s6_addr32[0] == htonl(0xFF020000) &&
			ip6_addr->s6_addr32[1] == htonl(0x00000000) &&
			ip6_addr->s6_addr32[2] == htonl(0x00000000) &&
			ip6_addr->s6_addr32[3] == htonl(0x00010003)					)					

	{
		#ifdef CONFIG_BRIDGE_IGMPP_PROCFS_DEBUG
		printk(KERN_INFO "[BR_MAC_PROC]-> block IPv6 group address, group addr (dst addr): %08X %08X %08X %08X\n",
				ntohl(ip6_addr->s6_addr32[0]), ntohl(ip6_addr->s6_addr32[1]),
				ntohl(ip6_addr->s6_addr32[2]), ntohl(ip6_addr->s6_addr32[3])
		);
		#endif
		/* this addrs is not allowed */
		return 1; 
	}


	/* check done, it's ok */
	return 0;
}

/* snoop_MAC() => If IP address that existed in br_mac_table ,replace it,
 * else create a new list entry and add it to list.
 * called under bridge lock */
static void snoop_MAC(struct net_bridge *br ,struct sk_buff *skb2)
{
	uint32_t ip32 =  (uint32_t) skb2->nh.iph->saddr;

	struct br_mac_table_t *tlist;
	int find = 0;;
	list_for_each_entry(tlist,&(br->br_mac_table.list), list){
		if ( tlist->ip_addr == ip32){
			find =1;
			int i;
			struct ethhdr * src = eth_hdr(skb2);
			for (i =0; i<6; i++)
				tlist->mac_addr[i] = src->h_source[i];
			break;
		}
	}
	if (find == 0 ){
		struct br_mac_table_t * new_entry;
		new_entry = (struct br_mac_table_t *)kmalloc(sizeof(struct br_mac_table_t), GFP_ATOMIC);
		if (new_entry != NULL){
			int i;
			struct ethhdr * src = eth_hdr(skb2);
			for (i =0; i<6; i++)
				new_entry->mac_addr[i] = src->h_source[i];
			new_entry->ip_addr = ip32;
			list_add(&(new_entry->list), &(br->br_mac_table.list));
		}else{
			#ifdef CONFIG_BRIDGE_IGMPP_PROCFS_DEBUG
			printk(KERN_INFO "[BR_MAC_PROC]-> alloc new br_mac_table_t fail !!\n");
			#endif
		}
	}
}

static void igmp_join(struct net_bridge *br, struct net_bridge_fdb_entry * fdb,
					uint32_t mca, unsigned char * src_mac, uint32_t *src_ip)
{
	if(atomic_read(&fdb->dst->wireless_interface) == 1){

		#ifdef CONFIG_BRIDGE_IGMPP_PROCFS_DEBUG
		uint8_t *ip8_addr,*mca8_addr;
		trans_32to8(src_ip, &ip8_addr);
		trans_32to8(&mca, &mca8_addr); 
		printk(KERN_INFO "[BR_MAC_PROC]-> snooping ADD [IP: %u.%u.%u.%u, MAC: %X:%X:%X:%X:%X:%X, Multicast: %u.%u.%u.%u] =====\n",
			*ip8_addr, *(ip8_addr+1), *(ip8_addr+2), *(ip8_addr+3),
			src_mac[0], src_mac[1], src_mac[2], src_mac[3], src_mac[4], src_mac[5],
			*mca8_addr, *(mca8_addr+1), *(mca8_addr+2), *(mca8_addr+3)
		);
		#endif

		br_igmpp_igmp_table_add(fdb->dst, &fdb->dst->port_igmpp_table, mca, src_mac);

		#ifdef CONFIG_BRIDGE_IGMPP_PROCFS_DEBUG
		printk(KERN_INFO "[BR_MAC_PROC]-> snooping ADD DONE !! ==============================================================\n");
		#endif


		#ifdef CONFIG_BRIDGE_IGMPP_PROCFS_DEBUG
		/* make sure there's only one record existed */
		printk(KERN_INFO "[BR_MAC_PROC]-> checking and clean all other port_igmpp_table ... \n");
		#endif
		struct net_bridge_port *p;
		list_for_each_entry(p, &br->port_list, list) {
			if(fdb->dst->port_no != p->port_no){
				br_igmpp_igmp_table_remove(&p->port_igmpp_table, mca, src_mac);
			}
		}
		#ifdef CONFIG_BRIDGE_IGMPP_PROCFS_DEBUG
		printk(KERN_INFO "[BR_MAC_PROC]-> clean other port_igmpp_table DONE !!\n");
		#endif
	}else{ /* report from wired interface(port), we won't transforming mulitcast to unicast, so we don't add to table here*/

		#ifdef CONFIG_BRIDGE_IGMPP_PROCFS_DEBUG
		uint8_t *ip8_addr,*mca8_addr;
		trans_32to8(src_ip, &ip8_addr);
		trans_32to8(&mca, &mca8_addr); 
		printk("[BR_MAC_PROC]->[IP: %u.%u.%u.%u, MAC: %X:%X:%X:%X:%X:%X, Multicast: %u.%u.%u.%u] is belong to wired interface(port) !!\n",
			*ip8_addr, *(ip8_addr+1), *(ip8_addr+2), *(ip8_addr+3),
			src_mac[0], src_mac[1], src_mac[2], src_mac[3], src_mac[4], src_mac[5],
			*mca8_addr, *(mca8_addr+1), *(mca8_addr+2), *(mca8_addr+3)
		);
		printk(KERN_INFO "[BR_MAC_PROC]-> fdb.addr =  %X:%X:%X:%X:%X:%X \n",
			fdb->addr.addr[0], fdb->addr.addr[1],
			fdb->addr.addr[2], fdb->addr.addr[3],
			fdb->addr.addr[4], fdb->addr.addr[5] );
		printk(KERN_INFO "[BR_MAC_PROC]-> fdb->dst->dev.name : %s \n", fdb->dst->dev->name);
		#endif
	}
}

static void igmp_leave(struct net_bridge_fdb_entry * fdb,
					uint32_t mca, unsigned char * src_mac, uint32_t *src_ip)
{
	if(atomic_read(&fdb->dst->wireless_interface) == 1){
		#ifdef CONFIG_BRIDGE_IGMPP_PROCFS_DEBUG
		uint8_t *ip8_addr,*mca8_addr;
		trans_32to8(src_ip, &ip8_addr);
		trans_32to8(&mca, &mca8_addr); 
		printk(KERN_INFO "[BR_MAC_PROC]-> snooping REMOVE [IP: %u.%u.%u.%u, MAC: %X:%X:%X:%X:%X:%X, Multicast: %u.%u.%u.%u] ===== \n",
			*ip8_addr, *(ip8_addr+1), *(ip8_addr+2), *(ip8_addr+3),
			src_mac[0], src_mac[1], src_mac[2], src_mac[3], src_mac[4], src_mac[5],
			*mca8_addr, *(mca8_addr+1), *(mca8_addr+2), *(mca8_addr+3)
		);
		#endif

		br_igmpp_igmp_table_remove(&fdb->dst->port_igmpp_table, mca, src_mac);

		#ifdef CONFIG_BRIDGE_IGMPP_PROCFS_DEBUG
		printk(KERN_INFO "[BR_MAC_PROC]-> snooping REMOVE DONE !! ===============================================================\n");
		#endif
	}else{

		#ifdef CONFIG_BRIDGE_IGMPP_PROCFS_DEBUG
		uint8_t *ip8_addr,*mca8_addr;
		trans_32to8(src_ip, &ip8_addr);
		trans_32to8(&mca, &mca8_addr); 
		printk("[BR_MAC_PROC]->[IP: %u.%u.%u.%u, MAC: %X:%X:%X:%X:%X:%X, Multicast: %u.%u.%u.%u] is belong to wired interface(port) !!\n",
			*ip8_addr, *(ip8_addr+1), *(ip8_addr+2), *(ip8_addr+3),
			src_mac[0], src_mac[1], src_mac[2], src_mac[3], src_mac[4], src_mac[5],
			*mca8_addr, *(mca8_addr+1), *(mca8_addr+2), *(mca8_addr+3)
		);
		printk(KERN_INFO "[BR_MAC_PROC]-> fdb.addr =  %X:%X:%X:%X:%X:%X \n",
			fdb->addr.addr[0], fdb->addr.addr[1],
			fdb->addr.addr[2], fdb->addr.addr[3],
			fdb->addr.addr[4], fdb->addr.addr[5] );
		printk(KERN_INFO "[BR_MAC_PROC]-> fdb->dst->dev.name : %s \n", fdb->dst->dev->name);
		#endif
	}
}

/*********************************************************************************************
snoop_IGMPV2_rep : snooping IGMPv2 report packet, add multicast address to table (port_igmpp_table), 
				this function calling br_fdb_get() to get fdb entry then calling igmp_join() 
				do detailed check.
	NOTE: caller must under bridge locked 
	@br:		bridge
	@igmph:	pointer to igmp header
	@src_mac:	source MAC address
	@src_ip:	source IPv4 address	(for debug only)										
********************************************************************************************/
static void snoop_IGMPV2_rep(struct net_bridge *br, struct igmphdr * igmph,
								unsigned char *src_mac, uint32_t *src_ip)
{
	
	/* searching bridge_fdb_entry */
	struct net_bridge_fdb_entry *hit_fdb_entry;
	hit_fdb_entry = br_fdb_get(br, src_mac);
	/* NOTE: The effect of successful called br_fdb_get() also takes lock bridge and reference counts. */

	if (hit_fdb_entry != NULL){
		/* check multicast address agign. For IGMPv2 rep, it's should already checked via dst addrs at IP(v4) layer */
		if(!br_igmpp_block_grp(&igmph->group)) {
			igmp_join( br, hit_fdb_entry, igmph->group, src_mac, src_ip);
		}
		br_fdb_put(hit_fdb_entry); // release br_fdb_get() locks
	}else{
		#ifdef CONFIG_BRIDGE_IGMPP_PROCFS_DEBUG
		uint8_t *ip8_addr,*mca8_addr;
		trans_32to8(src_ip, &ip8_addr);
		trans_32to8(&igmph->group, &mca8_addr); 
		printk(KERN_INFO "The return value of br_fdb_get() is NULL->[IP:%u.%u.%u.%u, MAC:%X:%X:%X:%X:%X:%X, Multicast:%u.%u.%u.%u ]\n",
				*ip8_addr, *(ip8_addr+1), *(ip8_addr+2), *(ip8_addr+3),
				src_mac[0], src_mac[1], src_mac[2], src_mac[3], src_mac[4], src_mac[5],
				*mca8_addr, *(mca8_addr+1), *(mca8_addr+2), *(mca8_addr+3)
		);
		#endif
	}
}

static void snoop_IGMPV3_rep(struct net_bridge *br, struct igmphdr * igmph,
								unsigned char *src_mac, uint32_t *src_ip)
{

	struct igmpv3_report * igmpv3rep = (struct igmpv3_report *)igmph;

	/* searching bridge_fdb_entry */
	struct net_bridge_fdb_entry *hit_fdb_entry;
	hit_fdb_entry = br_fdb_get(br, src_mac);
	/* NOTE: The effect of successful called br_fdb_get() also takes lock bridge and reference counts. */
	
	if (hit_fdb_entry != NULL){

		uint16_t cnt_ngrec = ntohs(igmpv3rep->ngrec);
		uint16_t i = 0;
		while( i < cnt_ngrec )
		{
			/* for igmpv3 rep, we should check multicast address at each Mcast Address Record */
			if( !br_igmpp_block_grp(&igmpv3rep->grec[i].grec_mca))
			{
				switch (igmpv3rep->grec[i].grec_type)
				{
					case IGMPV3_MODE_IS_EXCLUDE:    /***************************************************    2: MODE_IS_EXCLUDE          */
					case IGMPV3_CHANGE_TO_EXCLUDE:  /***************************************************    4: CHANGE_TO_EXCLUDE_MODE   */
					case IGMPV3_ALLOW_NEW_SOURCES:  /*******************l*******************************    5: ALLOW_NEW_SOURCES        */
						igmp_join( br, hit_fdb_entry, igmpv3rep->grec[i].grec_mca, src_mac, src_ip);
						break;

					case IGMPV3_MODE_IS_INCLUDE:    /***************************************************    1: MODE_IS_INCLUDE          */
					case IGMPV3_CHANGE_TO_INCLUDE:  /***************************************************    3: CHANGE_TO_INCLUDE_MODE   */
					case IGMPV3_BLOCK_OLD_SOURCES:  /***************************************************    6: BLOCK_OLD_SOURCES        */
						igmp_leave(hit_fdb_entry, igmpv3rep->grec[i].grec_mca, src_mac, src_ip);
						break;

					default:
						printk(KERN_INFO "unknown IGMPv3 Group Record Type - 0x%x, ignoring!\n", igmpv3rep->grec[i].grec_type);
						break;
				}
			}//br_igmpp_block_grp() end
			i++;
		}
		br_fdb_put(hit_fdb_entry); // release br_fdb_get() locks
	}else{
		#ifdef CONFIG_BRIDGE_IGMPP_PROCFS_DEBUG
		uint8_t *ip8_addr;
		trans_32to8(src_ip, &ip8_addr);
		printk(KERN_INFO "snoop_IGMPV3_rep() - The return value of br_fdb_get() is NULL->[IP:%u.%u.%u.%u, MAC:%X:%X:%X:%X:%X:%X \n",
				*ip8_addr, *(ip8_addr+1), *(ip8_addr+2), *(ip8_addr+3),
				src_mac[0], src_mac[1], src_mac[2], src_mac[3], src_mac[4], src_mac[5]
		);
		#endif
	}
}

/*********************************************************************************************
snoop_IGMP_leave : snooping IGMPv2 leave packet, remove multicast address from table (port_igmpp_table), 
				this function calling br_fdb_get() to get fdb entry then calling igmp_leave() 
				do detailed check.
	NOTE: caller must under bridge locked 
	@br:		bridge
	@igmph:	pointer to igmp header
	@src_mac:	source MAC address
	@src_ip:	source IPv4 address	(for debug only)										
********************************************************************************************/
static void snoop_IGMP_leave(struct net_bridge *br, struct igmphdr * igmph,
								unsigned char *src_mac, uint32_t *src_ip)
{

	/* searching bridge_fdb_entry */
	struct net_bridge_fdb_entry *hit_fdb_entry;
	hit_fdb_entry = br_fdb_get(br, src_mac);
	/* NOTE: The effect of successful called br_fdb_get() also takes lock bridge and reference counts. */

	if (hit_fdb_entry != NULL){
		igmp_leave(hit_fdb_entry, igmph->group, src_mac, src_ip);
		br_fdb_put(hit_fdb_entry); // release br_fdb_get() locks
	}else{
		#ifdef CONFIG_BRIDGE_IGMPP_PROCFS_DEBUG
		uint8_t *ip8_addr,*mca8_addr;
		trans_32to8(src_ip, &ip8_addr);
		trans_32to8(&igmph->group, &mca8_addr); 
		printk(KERN_INFO "The return value of br_fdb_get() is NULL->[IP:%u.%u.%u.%u, MAC:%X:%X:%X:%X:%X:%X, Multicast:%u.%u.%u.%u ]\n",
				*ip8_addr, *(ip8_addr+1), *(ip8_addr+2), *(ip8_addr+3),
				src_mac[0], src_mac[1], src_mac[2], src_mac[3], src_mac[4], src_mac[5],
				*mca8_addr, *(mca8_addr+1), *(mca8_addr+2), *(mca8_addr+3)
		);
		#endif
	}
}

/* snoop_MAC6() => If IP address that existed in br_mac_table6 ,replace it,
 * else create a new list entry and add it to list.
 * called under bridge lock */
static void snoop_MAC6(struct net_bridge *br ,struct sk_buff *skb2)
{

	struct br_mac_table6_t *tlist;
	int find = 0;;
	list_for_each_entry(tlist,&(br->br_mac_table6.list), list){
		if ( ipv6_addr_equal(	&tlist->ip6_addr,  &skb2->nh.ipv6h->saddr )	){
			find =1;
			int i;
			struct ethhdr * src = eth_hdr(skb2);
			for (i =0; i<6; i++)
				tlist->mac_addr[i] = src->h_source[i];
			break;
		}
	}
	if (find == 0 ){
		struct br_mac_table6_t * new_entry;
		new_entry = (struct br_mac_table6_t *)kmalloc(sizeof(struct br_mac_table6_t), GFP_ATOMIC);
		if (new_entry != NULL){
			int i;
			struct ethhdr * src = eth_hdr(skb2);
			for (i =0; i<6; i++)
				new_entry->mac_addr[i] = src->h_source[i];
			ipv6_addr_copy( &new_entry->ip6_addr, &skb2->nh.ipv6h->saddr);
			list_add( &(new_entry->list), &(br->br_mac_table6.list) );
		}else{
			#ifdef CONFIG_BRIDGE_IGMPP_PROCFS_DEBUG
			printk(KERN_INFO "[BR_MAC_PROC]-> alloc new br_mac_table6_t fail !!\n");
			#endif
		}
	}
}

static void mld_join(struct net_bridge *br, struct net_bridge_fdb_entry * fdb,
					struct in6_addr mca, unsigned char * src_mac, struct in6_addr *src_ip6)
{
	if(atomic_read(&fdb->dst->wireless_interface) == 1){

		#ifdef CONFIG_BRIDGE_IGMPP_PROCFS_DEBUG
		printk(KERN_INFO "[BR_MAC_PROC]-> snooping ADD [IP: %08X %08X %08X %08X, MAC: %X:%X:%X:%X:%X:%X, Multicast: %08X %08X %08X %08X] ===\n",
			ntohl(src_ip6->s6_addr32[0]), ntohl(src_ip6->s6_addr32[1]),
			ntohl(src_ip6->s6_addr32[2]), ntohl(src_ip6->s6_addr32[3]),
			src_mac[0], src_mac[1], src_mac[2], src_mac[3], src_mac[4], src_mac[5],
			ntohl(mca.s6_addr32[0]), ntohl(mca.s6_addr32[1]),
			ntohl(mca.s6_addr32[2]), ntohl(mca.s6_addr32[3])
		);
		#endif

		br_igmpp_mld_table_add(fdb->dst, &fdb->dst->port_igmpp_table, mca, src_mac);

		#ifdef CONFIG_BRIDGE_IGMPP_PROCFS_DEBUG
		printk(KERN_INFO "[BR_MAC_PROC]-> snooping ADD DONE !! =============================================================================\n");
		#endif


		#ifdef CONFIG_BRIDGE_IGMPP_PROCFS_DEBUG
		/* make sure there's only one record existed */
		printk(KERN_INFO "[BR_MAC_PROC]-> checking and clean all other port_igmpp_table ... \n");
		#endif
		struct net_bridge_port *p;
		list_for_each_entry(p, &br->port_list, list) {
			if(fdb->dst->port_no != p->port_no){
				br_igmpp_mld_table_remove(&p->port_igmpp_table, mca, src_mac);
			}
		}
		#ifdef CONFIG_BRIDGE_IGMPP_PROCFS_DEBUG
		printk(KERN_INFO "[BR_MAC_PROC]-> clean other port_igmpp_table DONE !!\n");
		#endif
	}else{ /* report from wired interface(port), we won't transforming mulitcast to unicast, so we don't add to table here*/

		#ifdef CONFIG_BRIDGE_IGMPP_PROCFS_DEBUG
		printk("[BR_MAC_PROC]->[IP: %08X %08X %08X %08X, MAC: %X:%X:%X:%X:%X:%X, Multicast: %08X %08X %08X %08X] is belong to wired interface(port) !!\n",
			ntohl(src_ip6->s6_addr32[0]), ntohl(src_ip6->s6_addr32[1]),
			ntohl(src_ip6->s6_addr32[2]), ntohl(src_ip6->s6_addr32[3]),
			src_mac[0], src_mac[1], src_mac[2], src_mac[3], src_mac[4], src_mac[5],
			ntohl(mca.s6_addr32[0]), ntohl(mca.s6_addr32[1]),
			ntohl(mca.s6_addr32[2]), ntohl(mca.s6_addr32[3])
		);
		printk(KERN_INFO "[BR_MAC_PROC]-> fdb.addr =  %X:%X:%X:%X:%X:%X \n",
			fdb->addr.addr[0], fdb->addr.addr[1],
			fdb->addr.addr[2], fdb->addr.addr[3],
			fdb->addr.addr[4], fdb->addr.addr[5] );
		printk(KERN_INFO "[BR_MAC_PROC]-> fdb->dst->dev.name : %s \n", fdb->dst->dev->name);
		#endif
	}
}

static void mld_leave(struct net_bridge_fdb_entry * fdb,
					struct in6_addr mca, unsigned char * src_mac, struct in6_addr *src_ip6)
{
	if(atomic_read(&fdb->dst->wireless_interface) == 1){
		#ifdef CONFIG_BRIDGE_IGMPP_PROCFS_DEBUG
		printk(KERN_INFO "[BR_MAC_PROC]-> snooping REMOVE [IP: %08X %08X %08X %08X, MAC: %X:%X:%X:%X:%X:%X, Multicast: %08X %08X %08X %08X] === \n",
			ntohl(src_ip6->s6_addr32[0]), ntohl(src_ip6->s6_addr32[1]),
			ntohl(src_ip6->s6_addr32[2]), ntohl(src_ip6->s6_addr32[3]),
			src_mac[0], src_mac[1], src_mac[2], src_mac[3], src_mac[4], src_mac[5],
			ntohl(mca.s6_addr32[0]), ntohl(mca.s6_addr32[1]),
			ntohl(mca.s6_addr32[2]), ntohl(mca.s6_addr32[3])
		);
		#endif

		br_igmpp_mld_table_remove(&fdb->dst->port_igmpp_table, mca, src_mac);

		#ifdef CONFIG_BRIDGE_IGMPP_PROCFS_DEBUG
		printk(KERN_INFO "[BR_MAC_PROC]-> snooping REMOVE DONE !! =============================================================================\n");
		#endif
	}else{

		#ifdef CONFIG_BRIDGE_IGMPP_PROCFS_DEBUG
		printk("[BR_MAC_PROC]->[IP: %08X %08X %08X %08X, MAC: %X:%X:%X:%X:%X:%X, Multicast: %08X %08X %08X %08X] is belong to wired interface(port) !!\n",
			ntohl(src_ip6->s6_addr32[0]), ntohl(src_ip6->s6_addr32[1]),
			ntohl(src_ip6->s6_addr32[2]), ntohl(src_ip6->s6_addr32[3]),
			src_mac[0], src_mac[1], src_mac[2], src_mac[3], src_mac[4], src_mac[5],
			ntohl(mca.s6_addr32[0]), ntohl(mca.s6_addr32[1]),
			ntohl(mca.s6_addr32[2]), ntohl(mca.s6_addr32[3])
		);
		printk(KERN_INFO "[BR_MAC_PROC]-> fdb.addr =  %X:%X:%X:%X:%X:%X \n",
			fdb->addr.addr[0], fdb->addr.addr[1],
			fdb->addr.addr[2], fdb->addr.addr[3],
			fdb->addr.addr[4], fdb->addr.addr[5] );
		printk(KERN_INFO "[BR_MAC_PROC]-> fdb->dst->dev.name : %s \n", fdb->dst->dev->name);
		#endif
	}
}

/*********************************************************************************************
snoop_MLD_rep : snooping MLDv1 report packet, add multicast address to table (port_igmpp_table), 
				this function calling "br_igmpp_mld_table_add()" and "br_igmpp_mld_table_remove"
				do detailed check.
	NOTE: caller must under bridge locked 
	@br:		bridge
	@ip6icmp6:	pointer to icmpv6 header
	@src_mac:	source MAC address
	@src_ip6:	source IPv6 address	(for debug only)										
********************************************************************************************/
static void snoop_MLD_rep(struct net_bridge *br, struct icmp6hdr * ip6icmp6,
								unsigned char *src_mac, struct in6_addr *src_ip6)
{
	
	struct igmpp_mldhdr * mldrep = (struct igmpp_mldhdr *)ip6icmp6;
	
	/* searching bridge_fdb_entry */
	struct net_bridge_fdb_entry *hit_fdb_entry;
	hit_fdb_entry = br_fdb_get(br, src_mac);
	/* NOTE: The effect of successful called br_fdb_get() also takes lock bridge and reference counts. */

	if (hit_fdb_entry != NULL){
		/* check multicast address agign. For MLDv1 rep, it's should already checked via dst addrs at IP(v6) layer */
		if(!br_igmpp_block_grp6(&mldrep->mca)) {
			mld_join( br, hit_fdb_entry, mldrep->mca, src_mac, src_ip6);
		}
		br_fdb_put(hit_fdb_entry); // release br_fdb_get() locks
	}else{
		#ifdef CONFIG_BRIDGE_IGMPP_PROCFS_DEBUG
		printk(KERN_INFO "The return value of br_fdb_get() is NULL->[IP:%08X %08X %08X %08X, MAC:%X:%X:%X:%X:%X:%X, Multicast:%08X %08X %08X %08X]\n",
				ntohl(src_ip6->s6_addr32[0]), ntohl(src_ip6->s6_addr32[1]),
				ntohl(src_ip6->s6_addr32[2]), ntohl(src_ip6->s6_addr32[3]),
				src_mac[0], src_mac[1], src_mac[2], src_mac[3], src_mac[4], src_mac[5],
				ntohl(mldrep->mca.s6_addr32[0]), ntohl(mldrep->mca.s6_addr32[1]),
				ntohl(mldrep->mca.s6_addr32[2]), ntohl(mldrep->mca.s6_addr32[3])
		);
		#endif
	}
}

static void snoop_MLD2_rep(struct net_bridge *br, struct icmp6hdr * ip6icmp6,
								unsigned char *src_mac, struct in6_addr *src_ip6)
{

	struct igmpp_mld2_report * mld2rep = (struct igmpp_mld2_report *)ip6icmp6;

	/* searching bridge_fdb_entry */
	struct net_bridge_fdb_entry *hit_fdb_entry;
	hit_fdb_entry = br_fdb_get(br, src_mac);
	/* NOTE: The effect of successful called br_fdb_get() also takes lock bridge and reference counts. */
	
	if (hit_fdb_entry != NULL){

		uint16_t cnt_ngrec = ntohs(mld2rep->ngrec);
		uint16_t i = 0;
		while( i < cnt_ngrec )
		{
			/* for mldv2 rep, we should check multicast address at each Mcast Address Record */
			if( !br_igmpp_block_grp6(&mld2rep->grec[i].grec_mca))
			{
				switch (mld2rep->grec[i].grec_type)
				{
					case MLD2_MODE_IS_INCLUDE:    /***************************************************    1: MODE_IS_INCLUDE          */
					case MLD2_CHANGE_TO_EXCLUDE:  /***************************************************    4: CHANGE_TO_EXCLUDE_MODE   */
					case MLD2_ALLOW_NEW_SOURCES:  /*******************l*******************************    5: ALLOW_NEW_SOURCES        */
						mld_join( br, hit_fdb_entry, mld2rep->grec[i].grec_mca, src_mac, src_ip6);
						break;

					case MLD2_MODE_IS_EXCLUDE:    /***************************************************    2: MODE_IS_EXCLUDE          */
					case MLD2_CHANGE_TO_INCLUDE:  /***************************************************    3: CHANGE_TO_INCLUDE_MODE   */
					case MLD2_BLOCK_OLD_SOURCES:  /***************************************************    6: BLOCK_OLD_SOURCES        */
						mld_leave(hit_fdb_entry, mld2rep->grec[i].grec_mca, src_mac, src_ip6);
						break;

					default:
						printk(KERN_INFO "unknown MLDv2 Group Record Type - 0x%x, ignoring!\n", mld2rep->grec[i].grec_type);
						break;
				}
			}//br_igmpp_block_grp6() end
			i++;
		}
		br_fdb_put(hit_fdb_entry); // release br_fdb_get() locks
	}else{
		#ifdef CONFIG_BRIDGE_IGMPP_PROCFS_DEBUG
		printk(KERN_INFO "snoop_MLD2_rep() - The return value of br_fdb_get() is NULL->[IP:%08X %08X %08X %08X, MAC:%X:%X:%X:%X:%X:%X \n",
				ntohl(src_ip6->s6_addr32[0]), ntohl(src_ip6->s6_addr32[1]),
				ntohl(src_ip6->s6_addr32[2]), ntohl(src_ip6->s6_addr32[3]),
				src_mac[0], src_mac[1], src_mac[2], src_mac[3], src_mac[4], src_mac[5]
		);
		#endif
	}
}


/*********************************************************************************************
snoop_MLD_done : snooping MLD done packet, remove multicast address from table (port_igmpp_table), 
				this function calling "br_igmpp_mld_table_remove" do detailed check.
	NOTE: caller must under bridge locked 
	@br:		bridge
	@ip6icmp6:	pointer to icmpv6 header
	@src_mac:	source MAC address
	@src_ip6:	source IPv6 address	(for debug only)										
********************************************************************************************/
static void snoop_MLD_done(struct net_bridge *br, struct icmp6hdr * ip6icmp6,
								unsigned char *src_mac, struct in6_addr *src_ip6)
{

	struct igmpp_mldhdr * mldrep = (struct igmpp_mldhdr *)ip6icmp6;

	/* searching bridge_fdb_entry */
	struct net_bridge_fdb_entry *hit_fdb_entry;
	hit_fdb_entry = br_fdb_get(br, src_mac);
	/* NOTE: The effect of successful called br_fdb_get() also takes lock bridge and reference counts. */

	if (hit_fdb_entry != NULL){
		mld_leave(hit_fdb_entry, mldrep->mca, src_mac, src_ip6);
		br_fdb_put(hit_fdb_entry); // release br_fdb_get() locks
	}else{
		#ifdef CONFIG_BRIDGE_IGMPP_PROCFS_DEBUG
		printk(KERN_INFO "The return value of br_fdb_get() is NULL->[IP:%08X %08X %08X %08X, MAC:%X:%X:%X:%X:%X:%X, Multicast:%08X %08X %08X %08X]\n",
				ntohl(src_ip6->s6_addr32[0]), ntohl(src_ip6->s6_addr32[1]),
				ntohl(src_ip6->s6_addr32[2]), ntohl(src_ip6->s6_addr32[3]),
				src_mac[0], src_mac[1], src_mac[2], src_mac[3], src_mac[4], src_mac[5],
				ntohl(mldrep->mca.s6_addr32[0]), ntohl(mldrep->mca.s6_addr32[1]),
				ntohl(mldrep->mca.s6_addr32[2]), ntohl(mldrep->mca.s6_addr32[3])
		);
		#endif
	}
}
#endif


/* note: already called with rcu_read_lock (preempt_disabled) */
int br_handle_frame_finish(struct sk_buff *skb)
{
	const unsigned char *dest = eth_hdr(skb)->h_dest;
	struct net_bridge_port *p = rcu_dereference(skb->dev->br_port);
	struct net_bridge *br;
	struct net_bridge_fdb_entry *dst;

	if (!p || p->state == BR_STATE_DISABLED)
		goto drop;

	/* insert into forwarding database after filtering to avoid spoofing */
	br = p->br;
/*+++, MAC Address Translator, Builder, 2008/12/10, start*/
#ifdef CONFIG_BRIDGE_MAT
	if (br->mat_ena == MAT_ENA){
	}else{
		br_fdb_update(br, p, eth_hdr(skb)->h_source, 0);
	}
#else
	br_fdb_update(br, p, eth_hdr(skb)->h_source);
#endif /*CONFIG_BRIDGE_MAT*/
/*+++, MAC Address Translator, Builder, 2008/12/10, end*/

	if (p->state == BR_STATE_LEARNING)
		goto drop;

	if (is_multicast_ether_addr(dest)) { 
#ifdef CONFIG_BRIDGE_IGMPP_PROCFS

		spin_lock_bh(&br->lock); // bridge lock

		if (atomic_read(&br->br_mac_table_enable) == 1 )
		{
			/* check IP version */
			/* IPv4 ***********************************************************************************/
			if ( eth_hdr(skb)->h_proto == htons(ETH_P_IP) ) { 
				/* IPv4 IGMP snooping MAC */
				if(skb->nh.iph->protocol == IPPROTO_IGMP){ // IGMP protocol number: 0x02
					struct sk_buff *skb2;
					if ((skb2 = skb_clone(skb, GFP_ATOMIC)) != NULL) {
						skb_pull(skb2, skb2->nh.iph->ihl<<2);
						struct igmphdr *ih = (struct igmphdr *) skb2->data;
						if (ih->type == IGMP_HOST_MEMBERSHIP_REPORT		||		// IGMPv1 REPORT
							ih->type == IGMPV2_HOST_MEMBERSHIP_REPORT	||		// IGMPv2 REPORT
							ih->type == IGMPV3_HOST_MEMBERSHIP_REPORT	||		// IGMPv3 REPORT
							ih->type == IGMP_HOST_LEAVE_MESSAGE				)	// IGMP LEAVE
						{

							snoop_MAC(br, skb2);

							if ( !br_igmpp_block_grp(&skb2->nh.iph->daddr) )
							{
								switch (ih->type)
								{
									case IGMPV2_HOST_MEMBERSHIP_REPORT:
										snoop_IGMPV2_rep(br, ih, eth_hdr(skb2)->h_source, &skb2->nh.iph->saddr);
										break;
									case IGMPV3_HOST_MEMBERSHIP_REPORT:
										snoop_IGMPV3_rep(br, ih, eth_hdr(skb2)->h_source, &skb2->nh.iph->saddr);						
										break;
									case IGMP_HOST_LEAVE_MESSAGE:
										snoop_IGMP_leave(br, ih, eth_hdr(skb2)->h_source, &skb2->nh.iph->saddr);
										break;
									default:
										break;	
								}
							}
						}// end - IGMP v1/v2/v3 report and leave check
						kfree_skb(skb2);
					}else{
						#ifdef CONFIG_BRIDGE_IGMPP_PROCFS_DEBUG
						printk(KERN_INFO "[BR_MAC_PROC]-> alloc new sk_buff fail !!\n");
						#endif
					}// end - skb clone fail check
				}// end of IPv4 IGMP check
			}
			/* end of IPv4 ****************************************************************************/ 


			/* IPv6 ***********************************************************************************/
			/* check next header is IPv6 at MAC layer first */
			if ( eth_hdr(skb)->h_proto == htons(ETH_P_IPV6) ) {
				/* check next header is IPv6 hop-by-hop at IP layer and not wildcard address*/
				if( (skb->nh.ipv6h->nexthdr == IPPROTO_HOPOPTS) &&		// check IPv6 hop-by-hop and
					!ipv6_addr_any(&skb->nh.ipv6h->saddr)			)	// src addr not a wildcard address (::)
				{
					struct sk_buff *skb2;
					if ((skb2 = skb_clone(skb, GFP_ATOMIC)) != NULL) {
						/* strip ipv6 header, ipv6hdr len :40, meaning point skb2->data to ipv6 hop-by-hop */
						skb_pull(skb2, sizeof(struct ipv6hdr));

						struct ipv6_hopopt_hdr *ip6hopopt = (struct ipv6_hopopt_hdr *) skb2->data;

						/* check next header is icmpv6 at hop-by-hop header (still at IP layer)*/
						if (ip6hopopt->nexthdr == IPPROTO_ICMPV6 ){
							/* strip ipv6 hop-by-hop header,  point skb2->data to icmpv6 */	
							skb_pull(skb2,ipv6_optlen(ip6hopopt));
							struct icmp6hdr *ip6icmp6 = (struct icmp6hdr *) skb2->data;

							/* check icmpv6 is MLD  */
							if (ip6icmp6->icmp6_type == ICMPV6_MGM_REPORT		||		// MLDv1 report: 131
								ip6icmp6->icmp6_type == ICMPV6_MLD2_REPORT		||		// or MLDv2 report: 143
								ip6icmp6->icmp6_type == ICMPV6_MGM_REDUCTION 		)	// or MLD Done: 132
							{

								snoop_MAC6(br, skb2);

								if ( !br_igmpp_block_grp6(&skb2->nh.ipv6h->daddr) )
								{
									switch (ip6icmp6->icmp6_type)
									{
										case ICMPV6_MGM_REPORT:
											snoop_MLD_rep(br, ip6icmp6, eth_hdr(skb2)->h_source, &skb2->nh.ipv6h->saddr);
											break;
										case ICMPV6_MLD2_REPORT:
											snoop_MLD2_rep(br, ip6icmp6, eth_hdr(skb2)->h_source, &skb2->nh.ipv6h->saddr);
											break;
										case ICMPV6_MGM_REDUCTION:
											snoop_MLD_done(br, ip6icmp6, eth_hdr(skb2)->h_source, &skb2->nh.ipv6h->saddr);
											break;
										default:
											break;	
									}
								}
							} // end - MLDv1/MLDv2 check
						} // end - icmpv6 check

						kfree_skb(skb2);

					}else{
						#ifdef CONFIG_BRIDGE_IGMPP_PROCFS_DEBUG
						printk(KERN_INFO "[BR_MAC_PROC]-> alloc new sk_buff fail !!\n");
						#endif
					} // end - skb clone fail check
				}// end - IPv6 hop-by-hop check 
			}
			/* end of IPv6 ****************************************************************************/ 

		} // END 

		spin_unlock_bh(&br->lock); // bridge unlock

#endif

/*+++, MAC Address Translator, Builder, 20081022, start*/
#ifdef CONFIG_BRIDGE_MAT
	if (br->mat_ena == MAT_ENA){
		struct sk_buff *skb2;
		if ((skb2 = skb_copy(skb, GFP_ATOMIC)) == NULL) {
			br->statistics.tx_dropped++;
			kfree_skb(skb); /*Root cause of memery leak.*/
			goto out;
		}
		br_flood_forward(br, skb, 1);

		kfree_skb(skb); /*Root cause of memery leak.*/
		br_pass_frame_up(br, skb2);
	}else{
		br->statistics.multicast++;
		br_flood_forward(br, skb, 1);
			br_pass_frame_up(br, skb);
	}
#else
	br->statistics.multicast++;
	br_flood_forward(br, skb, 1);
	br_pass_frame_up(br, skb);
#endif /*CONFIG_BRIDGE_MAT*/
/*+++, MAC Address Translator, Builder, 2008/12/10, end*/

		goto out;
	}

	dst = __br_fdb_get(br, dest); 
	if (dst != NULL && dst->is_local) {
		br_pass_frame_up(br, skb);
		goto out;
	}

	if (br->dev->flags & IFF_PROMISC) {
		struct sk_buff *skb2;

		skb2 = skb_clone(skb, GFP_ATOMIC);
		if (skb2 != NULL) {
			br_pass_frame_up(br, skb2);
		}
	}

#ifdef CONFIG_BRIDGE_FORWARD_CTRL 
	if (dst != NULL && !atomic_read(&br->br_forward)) {
		kfree_skb(skb);
		br_fdb_put(dst);
		goto out;
	}
#endif
#ifdef CONFIG_BRIDGE_PORT_FORWARD 
	if (dst != NULL && !p->port_forwarding) {
		kfree_skb(skb);
		br_fdb_put(dst);
		goto out;
	}
#endif
	if (dst != NULL) {
		br_forward(dst->dst, skb);
		goto out;
	}

	br_flood_forward(br, skb, 0);

out:
	return 0;
drop:
	kfree_skb(skb);
	goto out;
}

/* note: already called with rcu_read_lock (preempt_disabled) */
static int br_handle_local_finish(struct sk_buff *skb)
{
	struct net_bridge_port *p = rcu_dereference(skb->dev->br_port);

/*+++, MAC Address Translator, Builder, 2008/12/10, start*/
#ifdef CONFIG_BRIDGE_MAT
	if (p && p->state != BR_STATE_DISABLED)
		br_fdb_update(p->br, p, eth_hdr(skb)->h_source, 0);
#else
	if (p && p->state != BR_STATE_DISABLED)
		br_fdb_update(p->br, p, eth_hdr(skb)->h_source);
#endif /*CONFIG_BRIDGE_MAT*/
/*+++, MAC Address Translator, Builder, 2008/12/10, end*/

	return 0;	 /* process further */
}

/* Does address match the link local multicast address.
 * 01:80:c2:00:00:0X
 */
static inline int is_link_local(const unsigned char *dest)
{
	return memcmp(dest, br_group_address, 5) == 0 && (dest[5] & 0xf0) == 0;
}

/*
 * Called via br_handle_frame_hook.
 * Return 0 if *pskb should be processed furthur
 *	  1 if *pskb is handled
 * note: already called with rcu_read_lock (preempt_disabled)
 */
int br_handle_frame(struct net_bridge_port *p, struct sk_buff **pskb)
{
	struct sk_buff *skb = *pskb;
	const unsigned char *dest = eth_hdr(skb)->h_dest;

	if (!is_valid_ether_addr(eth_hdr(skb)->h_source))
		goto err;

/*+++, MAC Address Translator, Builder, 2008/12/10, start*/
		if (frame_Rx(skb)==1)
			goto err;
/*+++, MAC Address Translator, Builder, 2008/12/10, start*/

	if (unlikely(is_link_local(dest))) {
		skb->pkt_type = PACKET_HOST;
		return NF_HOOK(PF_BRIDGE, NF_BR_LOCAL_IN, skb, skb->dev,
			       NULL, br_handle_local_finish) != 0;
	}

	if (p->state == BR_STATE_FORWARDING || p->state == BR_STATE_LEARNING) {
		if (br_should_route_hook) {
			if (br_should_route_hook(pskb))
				return 0;
			skb = *pskb;
			dest = eth_hdr(skb)->h_dest;
		}

		if (!compare_ether_addr(p->br->dev->dev_addr, dest))
			skb->pkt_type = PACKET_HOST;

		NF_HOOK(PF_BRIDGE, NF_BR_PRE_ROUTING, skb, skb->dev, NULL,
			br_handle_frame_finish);
		return 1;
	}

err:
	kfree_skb(skb);
	return 1;
}
