/*
 * IP Personality
 *   ip_personality_ipid.c - IP ID rewritting
 *
 * Copyright (C) 2001, Gaël Roualland <gael.roualland@iname.com>
 * Copyright (C) 2001, Jean-Marc Saffroy <saffroy@mail.com>   
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
 * $Id: ip_personality_ipid.c,v 1.3 2001/07/22 22:56:28 g_roualland Exp $
 *
 */

#include <net/tcp.h>
#include <net/checksum.h>
#include <linux/random.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netfilter_ipv4/ip_personality.h>
#include <linux/sched.h>
#include <asm/byteorder.h>

#ifdef IP_PERS_DEBUG
#define DEBUGP if (ip_pers_debug & IP_PERS_DEBUG_IPID) printk
#else
#define DEBUGP(format, args...)
#endif

unsigned int pers_ip_mangle_id(struct ip_pers *pers,
			       struct iphdr *iph,
			       struct sk_buff ** pskb) {
  u_int16_t newid, diffs[2];

  switch (pers->ipid_type) {
  case IP_PERS_FIXED_INC:
    pers->current_ipid += pers->ipid_param;
    newid = htons(pers->current_ipid);
    break;
  case IP_PERS_BROKEN_INC:
    pers->current_ipid += pers->ipid_param;
    newid = cpu_to_le16(pers->current_ipid);
    break;
  case IP_PERS_RAND_INC:
    get_random_bytes(&newid, sizeof(newid));
    pers->current_ipid += (pers->ipid_param ? newid % pers->ipid_param : 0);
    newid = htons(pers->current_ipid);
    break;
  case IP_PERS_RANDOM:
    get_random_bytes(&newid, sizeof(newid));
    break;
  case IP_PERS_BUILTIN:
    newid = 0;
    break;
  case IP_PERS_TIME_INC:
    if(pers->ipid_param > 0) {
      pers->current_ipid += ((jiffies - pers->ipid_jiffies) 
			   * pers->ipid_param) / HZ;
      pers->ipid_jiffies = jiffies;
    }
    newid = htons(pers->current_ipid);
    break;    
  case IP_PERS_ASIS:
  default:
    newid = iph->id;
    break;
  }

  if (newid == iph->id)
    return IP_PERS_OK; /* no changes */
    
  DEBUGP("PERS:  Rewrite IPID on pkt from %u.%u.%u.%u: %u -> %u\n",
	 NIPQUAD(iph->saddr), ntohs(iph->id), ntohs(newid));

  diffs[0] = ~iph->id;
  iph->id = newid;
  diffs[1] = newid;
  iph->check = csum_fold(csum_partial((char *)diffs,
				      sizeof(diffs),
				      iph->check^0xFFFF));

  return IP_PERS_OK;
}


unsigned int pers_ip_mangle_ttl(struct ip_pers *pers,
			       struct iphdr *iph,
			       struct sk_buff ** pskb) {

  u_int8_t newttl, diffs[2];

  if(pers->current_ipttl <=0)
	newttl = iph->ttl;
  else
	newttl = pers->current_ipttl;

  if (newttl == iph->ttl)
    return IP_PERS_OK; /* no changes */

  DEBUGP("PERS:  Rewrite IPTTL on pkt from %u.%u.%u.%u: %u -> %u\n",
	 NIPQUAD(iph->saddr), iph->ttl, newttl);

  iph->check = 0;
  iph->ttl = newttl;
  iph->check  = ip_fast_csum((unsigned char *)iph, iph->ihl);

  return IP_PERS_OK;
}
