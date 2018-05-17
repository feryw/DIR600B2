/*
 * IP Personality
 *   ip_personality_udp - UDP to closed port handling
 *
 * Copyright (C) 2000, Gaël Roualland <gael.roualland@iname.com>
 * Copyright (C) 2000, Jean-Marc Saffroy <saffroy@mail.com>   
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * $Id: ip_personality_udp.c,v 1.16 2001/07/22 22:00:09 g_roualland Exp $
 *
 */

#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/icmp.h>
#include <net/tcp.h>
#include <net/sock.h>
#include <net/route.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netfilter_ipv4/ip_personality.h>

#ifdef IP_PERS_DEBUG
#define DEBUGP if (ip_pers_debug & IP_PERS_DEBUG_UDP) printk
#else
#define DEBUGP(format, arg...)
#endif

#define MIN3(a,b,c) ( (a) < (b) ?                  \
		      ( (a) < (c) ? (a) : (c) ) :  \
		      ( (b) < (c) ? (b) : (c) ) )  \

struct sock *udp_v4_lookup(u32 saddr, u16 sport, u32 daddr, u16 dport, int dif);

static inline int pers_udp_dport_isopen(const struct net_device *in,
					struct iphdr * iph,
					struct udphdr * uh)
{
  struct sock *sk;

  if(in == NULL)
    return 0;

  sk = udp_v4_lookup(iph->saddr, uh->source, iph->daddr, uh->dest, in->ifindex);
  if(sk != NULL)
    __sock_put(sk); /* because sock_hold() called in udp_v4_lookup() */

  return sk != NULL;
}

#define UDP_MOD(var, mod)               \
  if( (mod) != (IP_PERS_UDP_SAME) ) {   \
    if( (mod) == (IP_PERS_UDP_ZERO) ) { \
      var = 0;                          \
    } else {                            \
      var++; } }

static inline unsigned int pers_send_punreach(const struct net_device *in,
					      struct sk_buff **pskb,
					      struct ip_pers *pers)
{
  struct iphdr *iph = (*pskb)->nh.iph;
  struct iphdr *riph;
  struct udphdr *uh;
  struct icmphdr *icmph;
  struct sk_buff * rskb;
  struct rtable *rt;
  unsigned int oldlen, len;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
  struct flowi fl = { .nl_u = { .ip4_u = { 
		.daddr = iph->daddr,
		.saddr = iph->saddr, 
		  .tos = RT_TOS(pers->udp_info[IP_PERS_UDP_TOS])
		} } };
#endif

  /* do we really need to do it ? :-) */
  if(!pers->udp_info[IP_PERS_UDP_REPLY])
    return IP_PERS_REJ; /* drop pkt */

  /* route pkt */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
  if (ip_route_output_key(&rt, &fl)) {
#else
  if (ip_route_output(&rt, iph->saddr, iph->daddr,
		      RT_TOS(pers->udp_info[IP_PERS_UDP_TOS]), 0)) {
#endif
    if(net_ratelimit())
      printk(KERN_WARNING "PERS: Can't route icmp reply to %u.%u.%u.%u\n",
	     NIPQUAD(iph->saddr));
    return IP_PERS_ERROR;
  }

  /* compute new packet length and adjust sk_buff if necessary */
  oldlen = ntohs(iph->tot_len);
  len = MIN3(pers->udp_info[IP_PERS_UDP_MAXLEN],
	     sizeof(struct iphdr) + sizeof(struct icmphdr) + oldlen,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
	     dst_mtu(&rt->u.dst)); /* make sure we have no fragmentation */
#else
	     rt->u.dst.pmtu); /* make sure we have no fragmentation */
#endif

  /* make sure we have enough room for required info */
  if (len < 2 * sizeof(struct iphdr) + sizeof(struct icmphdr) + sizeof(struct udphdr))
    len = 2 * sizeof(struct iphdr) + sizeof(struct icmphdr) + sizeof(struct udphdr);
  
  rskb = alloc_skb(len + in->hard_header_len + 15, GFP_ATOMIC);
  if (!rskb) {
    if (net_ratelimit())
      printk(KERN_WARNING "PERS: Can't allocate new icmp skb!\n");
    return IP_PERS_ERROR;
  }
  rskb->pkt_type = PACKET_OUTGOING;
  rskb->priority = (*pskb)->priority;
  skb_reserve(rskb, (in->hard_header_len + 15) & ~15);
  skb_put(rskb, len);
  rskb->dst = &rt->u.dst;
  rskb->dev = rskb->dst->dev;
  rskb->sk = NULL;
 
  /* mangle original UDP packet as specified by pers */
  uh = (void *)iph + iph->ihl * 4;
  if (!pers->udp_info[IP_PERS_UDP_RIPLEN_SAME])
    iph->tot_len = htons(pers->udp_info[IP_PERS_UDP_RIPLEN]);
  UDP_MOD(iph->id, pers->udp_info[IP_PERS_UDP_RIPID]);
  UDP_MOD(iph->check, pers->udp_info[IP_PERS_UDP_RIPCSUM]);
  UDP_MOD(uh->check, pers->udp_info[IP_PERS_UDP_RCSUM]);
  if (!pers->udp_info[IP_PERS_UDP_RLEN_SAME])
    uh->len = htons(pers->udp_info[IP_PERS_UDP_RLEN]);
  {
    char c = *((char*)(uh+1));
    UDP_MOD(c, pers->udp_info[IP_PERS_UDP_RDATA]);
    *((char*)(uh+1)) = c;
  }

  /* copy old packet in new skbuff */
  riph = rskb->nh.iph = (struct iphdr *) rskb->data;
  memcpy((void *)riph + sizeof(struct iphdr) + sizeof(struct icmphdr),
	 iph, len - (sizeof(struct iphdr) + sizeof(struct icmphdr)));

  /* now fill in the new IP and ICMP headers as specified by pers */
  riph->version = 4;
  riph->ihl = 5; /* plain IP header, no fancy option */
  riph->tos = pers->udp_info[IP_PERS_UDP_TOS];
  riph->tot_len = htons(len);
  riph->frag_off = (pers->udp_info[IP_PERS_UDP_DF] ? __constant_htons(IP_DF) : 0);
  riph->ttl = MAXTTL;
  riph->protocol = IPPROTO_ICMP;
  riph->check = 0;
  riph->saddr = iph->daddr;
  riph->daddr = iph->saddr;
  ip_select_ident(riph, &rt->u.dst, NULL);
  rskb->h.icmph = icmph = (void*) riph + sizeof(struct iphdr);
  icmph->type = ICMP_DEST_UNREACH;
  icmph->code = ICMP_PORT_UNREACH;
  icmph->checksum = 0;
  icmph->un.gateway = 0; /* zero this unused field */

  /* compute all checksums */
  ip_send_check(riph);
  icmph->checksum = csum_fold(csum_partial((char*)icmph,
					   len - sizeof(struct iphdr), 0));

  DEBUGP("PERS:   Sending ICMP port unreachable message\n");
  pers_send_local(rskb, *pskb);

  return IP_PERS_REJ;
}

unsigned int pers_udp_check(struct ip_pers * pers,
			    struct iphdr * iph,
			    struct udphdr * uh,
			    const struct net_device *in,
			    const struct net_device *out,
			    struct sk_buff **pskb) {
  if(!pers_udp_dport_isopen(in, iph, uh))
    return pers_send_punreach(in, pskb, pers);
  return IP_PERS_OK;
}
