/*
 * IP Personality
 *   ip_personality_seq.c - tcp isn rewritting
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
 * $Id: ip_personality_seq.c,v 1.18 2001/07/19 23:41:50 g_roualland Exp $
 *
 */

#include <net/tcp.h>
#include <net/checksum.h>
#include <linux/random.h>
#include <linux/netfilter_ipv4/ip_conntrack.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netfilter_ipv4/ip_personality.h>
#include <linux/sched.h>

#ifdef IP_PERS_DEBUG
#define DEBUGP if (ip_pers_debug & IP_PERS_DEBUG_SEQ) printk
#else
#define DEBUGP(format, args...)
#endif

/* rewrite packet */
unsigned int pers_tcp_mangle_seq(struct iphdr *iph, struct tcphdr *th,
				 struct ip_pers_conn_info *pci,
				 struct ip_pers *pers,
				 struct sk_buff **pskb,
				 enum ip_conntrack_info ctinfo)
{
  u_int32_t diffs[2];

  if(pers->isn_type == IP_PERS_ASIS){
      DEBUGP("PERS: isn type asic, donot change SEQ on pkt from %u.%u.%u.%u:%u (seq=%u)\n",
	        NIPQUAD(iph->saddr), ntohs(th->source), ntohl(th->seq));
      return IP_PERS_OK;
  }

  if (pers->tweak_type == IP_PERS_TWEAK_SRC) {
	if( (ctinfo) < IP_CT_IS_REPLY){
            diffs[0] = ~th->seq;
            th->seq = htonl(ntohl(th->seq) + pci->seq_offset);
            diffs[1] = th->seq;
            th->check = csum_fold(csum_partial((char *)diffs,
				       sizeof(diffs),
				       th->check^0xFFFF));
            DEBUGP("PERS:   Rewrite SEQ on pkt from %u.%u.%u.%u:%u (new seq=%u)\n",
	        NIPQUAD(iph->saddr), ntohs(th->source), ntohl(th->seq));
	}else{
            diffs[0] = ~th->ack_seq;
            th->ack_seq = htonl(ntohl(th->ack_seq) - pci->seq_offset);
            diffs[1] = th->ack_seq;
            th->check = csum_fold(csum_partial((char *)diffs,
				       sizeof(diffs),
				       th->check^0xFFFF));
            DEBUGP("PERS:   Rewrite ACK on pkt from %u.%u.%u.%u:%u (new ack=%u)\n",
	        NIPQUAD(iph->saddr), ntohs(th->source), ntohl(th->ack_seq));
	}    
  } else if (pers->tweak_type == IP_PERS_TWEAK_DST) {
    diffs[0] = ~th->ack_seq;
    th->ack_seq = htonl(ntohl(th->ack_seq) - pci->seq_offset);
    diffs[1] = th->ack_seq;
    th->check = csum_fold(csum_partial((char *)diffs,
				       sizeof(diffs),
				       th->check^0xFFFF));
    DEBUGP("PERS:   Rewrite ACK on pkt from %u.%u.%u.%u:%u (new ack=%u)\n",
	   NIPQUAD(iph->saddr), ntohs(th->source), ntohl(th->ack_seq));
  }

  return IP_PERS_OK;
}

inline void pers_tcp_next_seq(struct iphdr *iph, struct tcphdr *th,
			      struct ip_pers *pers) {
  u_int32_t tmp;

  switch (pers->isn_type) {
  case IP_PERS_FIXED_INC:
  case IP_PERS_BROKEN_INC:
    pers->current_isn += pers->isn_param;
    break;
  case IP_PERS_RAND_INC:
    get_random_bytes(&tmp, sizeof(tmp));
    pers->current_isn += (pers->isn_param ? tmp % pers->isn_param : 0);
    break;
  case IP_PERS_RANDOM:
    get_random_bytes(&tmp, sizeof(tmp));
    pers->current_isn = tmp;
    break;
  case IP_PERS_BUILTIN:
    pers->current_isn = secure_tcp_sequence_number(iph->daddr, iph->saddr,
						   th->dest, th->source);
    break;
  case IP_PERS_TIME_INC:
    if(pers->isn_param > 0) {
      pers->current_isn += ((jiffies - pers->isn_jiffies) 
			   * pers->isn_param) / HZ;
      pers->isn_jiffies = jiffies;
    }
    break;    
  case IP_PERS_ASIS:
  default:
    pers->current_isn = ntohl(th->seq);
    break;
  }
}

/* initialize a new mangled connexion */
void pers_tcp_mangle_seq_new(struct iphdr *iph, struct tcphdr *th,
			     struct ip_pers_conn_info *pci, struct ip_pers *pers) {
  pers_tcp_next_seq(iph, th, pers);
  pci->seq_offset = pers->current_isn - ntohl(th->seq);
  DEBUGP("PERS:   NEW: Using ISN %u [offset %u] for"
	 " %u.%u.%u.%u:%u => %u.%u.%u.%u:%u\n",
	 pers->current_isn, pci->seq_offset, 
	 NIPQUAD(iph->saddr), ntohs(th->source),
	 NIPQUAD(iph->daddr), ntohs(th->dest));
}

