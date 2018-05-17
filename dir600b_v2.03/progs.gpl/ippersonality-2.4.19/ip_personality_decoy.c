/*
 * IP Personality
 *   ip_personality_decoy.c - tcp decoy handling module
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
 * $Id: ip_personality_decoy.c,v 1.19 2001/07/22 22:00:09 g_roualland Exp $
 *
 */

#include <linux/skbuff.h>
#include <net/ip.h>
#include <net/tcp.h>
#include <linux/random.h>
#include <net/sock.h>
#include <net/route.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netfilter_ipv4/ip_personality.h>

#ifdef IP_PERS_DEBUG
#define DEBUGP if (ip_pers_debug & IP_PERS_DEBUG_DECOY) printk
#else
#define DEBUGP(format, args...)
#endif

unsigned int pers_tcp_decoy(struct ip_pers * pers,
			    struct iphdr * iph,
			    struct tcphdr * th,
			    const struct net_device *in,
			    const struct net_device *out,
			    struct sk_buff **pskb,
			    struct ip_pers_tcp_info * odata)
{
  struct ip_pers_vm_state state;
  unsigned int ret;
  struct iphdr * riph;
  struct tcphdr * rth;
  struct sk_buff * rskb;
  unsigned int len;
  struct rtable *rt;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
  struct flowi fl = { .nl_u = { .ip4_u = { 
		.daddr = iph->daddr,
		.saddr = iph->saddr, 
		 .tos = RT_TOS(iph->tos) | RTO_CONN,
		} } };
#endif
  
  if (pers->decoy_prog.instr == NULL || pers->decoy_prog.prog_len == 0 || !in) 
    return IP_PERS_OK; /* nothing to do */

  if (!odata->initialized)
    pers_tcp_parse_opts(th, odata);

  /* initalize vm state */
  memset(&state, 0, sizeof(state));
  state.win = ntohs(th->window);
  state.df = ((*pskb)->nh.iph->frag_off & __constant_htons(IP_DF));
  state.mss = odata->mss;
  state.wscale = odata->wscale;
  
  /* run program and fill optout_buf */
  DEBUGP("PERS:   Decoying:");
  ret = pers_vm_run_prog(&(pers->decoy_prog), in, out, (*pskb)->nh.iph, th,
			 odata, pers, &state);

  switch(ret) {
  case IP_PERS_DROP:
    DEBUGP(" DROP\n");
    return IP_PERS_REJ;
  case IP_PERS_ACCEPT:
    DEBUGP(" ACCEPT\n");
    return IP_PERS_OK;
  case IP_PERS_REPLY:
    pers_tcp_add_padding(state.optout_buf, &state.optout_len);
    DEBUGP(" REPLY\n");
    break;
  default:
    DEBUGP("** failed\n");
    return IP_PERS_ERROR;
  }

  /* route pkt */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
  if (ip_route_output_key(&rt, &fl)) { 
#else
  if (ip_route_output(&rt, iph->saddr, iph->daddr,
		      RT_TOS(iph->tos) | RTO_CONN,
		      0)) {
#endif
    if (net_ratelimit())
      printk(KERN_WARNING "PERS: Can't route decoy reply to %u.%u.%u.%u\n",
	     NIPQUAD(iph->saddr));
    return IP_PERS_ERROR;
  }

  len = sizeof(struct iphdr) + sizeof(struct tcphdr) + state.optout_len;

  rskb = alloc_skb(len + in->hard_header_len + 15, GFP_ATOMIC);
  if (!rskb) {
    if (net_ratelimit())
      printk(KERN_WARNING "PERS: Can't allocate new decoy skb!\n");
    return IP_PERS_ERROR;
  }

  rskb->pkt_type = PACKET_OUTGOING;
  rskb->priority = (*pskb)->priority;
  skb_reserve(rskb, (in->hard_header_len + 15) & ~15);
  skb_put(rskb, len);
  rskb->dst = &rt->u.dst;
  rskb->dev = rskb->dst->dev;
  rskb->sk = NULL;

  /* build ip header */
  riph = rskb->nh.iph = (struct iphdr *) rskb->data;
  riph->version = 4;
  riph->ihl = sizeof(struct iphdr) / 4;
  riph->tos = iph->tos;
  riph->frag_off = (state.df ? __constant_htons(IP_DF) : 0);
  riph->ttl = MAXTTL;
  riph->daddr = iph->saddr;
  riph->saddr = iph->daddr;
  riph->protocol = IPPROTO_TCP;
  riph->tot_len = ntohs(len);
  riph->check = 0;
  ip_select_ident(riph, &rt->u.dst, NULL);
  ip_send_check(riph);

  /* build tcp header */
  rth = rskb->h.th = (struct tcphdr *) (rskb->data + sizeof(struct iphdr));
  rth->source = th->dest;
  rth->dest = th->source;
  rth->ack_seq = htonl(state.ack);
  *(((u_int16_t *)rth)+6) = htons(state.flags & IP_PERS_EXTRACT_FLAGS);
  /* sequence numbers */
  if (rth->rst) {
    if (th->ack)
      rth->seq = th->ack_seq;
    else
      rth->seq = 0;
  } else {
    if (pers->isn_type && pers->isn_type != IP_PERS_ASIS) {
      pers_tcp_next_seq(riph, rth, pers);
      rth->seq = htonl(pers->current_isn);
    } else
      rth->seq = secure_tcp_sequence_number(riph->daddr, riph->saddr,
					    rth->dest, rth->source);
  }
  rth->window = htons(state.win);
  rth->check = 0;
  rth->urg_ptr = 0;
  if (state.optout_len) /* add options */
    memcpy((void *) (rth + 1), state.optout_buf, state.optout_len);
  rth->doff = (sizeof(struct tcphdr) + state.optout_len) / 4;
  rskb->csum = csum_partial((char *)rth + rth->doff*4, 0, 0);
  rth->check = 0;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
  rth->check = tcp_v4_check(rth->doff*4, riph->saddr, riph->daddr,
			    csum_partial((char *)rth, rth->doff*4, rskb->csum));
#else
  rth->check = tcp_v4_check(rth, rth->doff*4, riph->saddr, riph->daddr,
			    csum_partial((char *)rth, rth->doff*4, rskb->csum));
#endif
  
  pers_send_local(rskb, *pskb);

  return IP_PERS_REJ;
}
