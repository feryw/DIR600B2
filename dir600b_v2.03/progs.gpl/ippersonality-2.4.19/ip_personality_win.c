/*
 * IP Personality
 *   ip_personality_win.c - TCP window size rewritting
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
 * $Id: ip_personality_win.c,v 1.8 2001/07/14 13:23:33 g_roualland Exp $
 *
 */

#include <net/tcp.h>
#include <net/checksum.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netfilter_ipv4/ip_personality.h>

#ifdef IP_PERS_DEBUG
#define DEBUGP if (ip_pers_debug & IP_PERS_DEBUG_WIN) printk
#else
#define DEBUGP(format, args...)
#endif

/* rewrite packet */
unsigned int pers_tcp_mangle_win(struct iphdr *iph, struct tcphdr *th,
				 struct ip_pers_conn_info *pci,
				 struct ip_pers *pers,
				 struct sk_buff **pskb)
{
  u_int16_t diffs[2];
  u_int16_t win = ntohs(th->window);
  
  diffs[0] = ~th->window;
  if (win < pci->win_offset)
    th->window = __constant_htons(0);
  else
    th->window = htons(win - pci->win_offset);
  diffs[1] = th->window;
  th->check = csum_fold(csum_partial((char *)diffs,
				     sizeof(diffs),
				     th->check^0xFFFF));
  DEBUGP("PERS:   Rewrite WIN on pkt from %u.%u.%u.%u:%u (new win=%u)\n",
	 NIPQUAD(iph->saddr), ntohs(th->source), ntohs(th->window));

  return IP_PERS_OK;
}

/* initialize a new mangled connexion */
void pers_tcp_mangle_win_new(struct iphdr *iph, struct tcphdr *th,
			     struct ip_pers_conn_info *pci, struct ip_pers *pers,
			     struct ip_pers_tcp_info * odata) {
  u_int16_t win = ntohs(th->window);

  if (!odata->initialized)
    pers_tcp_parse_opts(th, odata);

  if ((pers->tcp_maxwin > 0) && (win > pers->tcp_maxwin) && !odata->wscale) {
    pci->win_offset = win - pers->tcp_maxwin;
    DEBUGP("PERS:   NEW: Using WIN %u [offset %u] for"
	   " %u.%u.%u.%u:%u => %u.%u.%u.%u:%u\n",
	   pers->tcp_maxwin, pci->win_offset, 
	   NIPQUAD(iph->saddr), ntohs(th->source),
	   NIPQUAD(iph->daddr), ntohs(th->dest));
  }
}

