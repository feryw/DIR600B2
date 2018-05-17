/*
 * IP Personality
 *   ip_personality_opt.c - TCP options handling
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
 * $Id: ip_personality_opts.c,v 1.26 2001/07/22 14:17:30 g_roualland Exp $
 *
 */

#include <linux/skbuff.h>
#include <net/ip.h>
#include <net/tcp.h>
#include <net/route.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netfilter_ipv4/ip_personality.h>

#ifdef IP_PERS_DEBUG
#define DEBUGP if (ip_pers_debug & IP_PERS_DEBUG_OPTS) printk

/* known tcp options names (usable in prog) */
static char * tcpopts[] = {
  "eol",
  "nop",
  "mss",
  "wscale",
  "sackOK",
  "sack",
  "echo",
  "echoreply",
  "timestamp",
  "pocOK",
  "pocSP",
  "CC",
  "CC.NEW",
  "CC.ECHO",
  "acreq",
  "acdata"
};

#else
#define DEBUGP(format, args...)
#endif

/* add nops for padding */
inline void pers_tcp_add_padding(unsigned char *optbuf, unsigned int *optlen)
{
  unsigned int align = (4 - (*optlen % 4)) % 4 ;
  unsigned char *optend = optbuf + *optlen;

  if(*optlen + align >= IP_PERS_MAX_OPT_LEN)
    return;
  (*optlen) += align;
  while(align--) {
    DEBUGP(" %s", tcpopts[TCPOPT_EOL]);
    *(optend++) = TCPOPT_EOL;
  }
}

/* parse options in TCP header */
inline void pers_tcp_parse_opts(struct tcphdr *th,
				struct ip_pers_tcp_info * odata)
{
  /* stolen from tcp_parse_options() in tcp_input.c */
  unsigned char *ptr;
  int length=(th->doff*4)-sizeof(struct tcphdr);

  /* initialize data */
  memset(odata, 0, sizeof(struct ip_pers_tcp_info));
  odata->initialized = 1;
  
  odata->flags = ntohs(*(((u_int16_t*)th) +6)) & IP_PERS_EXTRACT_FLAGS;
  
  ptr = (unsigned char *)(th + 1);

  DEBUGP("PERS:   Parsing options:");

  while(length>0) {
    unsigned char opcode=*ptr++;
    unsigned char opsize;

    switch (opcode) {
    case TCPOPT_EOL:
      DEBUGP(" %s\n", tcpopts[opcode]);
      odata->seen_opt[TCPOPT_EOL] = (unsigned char*)1;
      return;
    case TCPOPT_NOP:	/* Ref: RFC 793 section 3.1 */
      DEBUGP(" %s", tcpopts[opcode]);
      odata->seen_opt[TCPOPT_NOP] = (unsigned char*)1;
      length--;
      continue;
    default:
      opsize=*ptr++;
      if (opsize < 2) { /* "silly options" */
	DEBUGP("\n");
	return;
      }
      if (opsize > length)
	return; /* don't parse partial options */
      if (opcode < IP_PERS_MAX_OPTS) {
	DEBUGP(" %s", tcpopts[opcode]);
	odata->seen_opt[opcode] = ptr-2;
	if (opcode == TCPOPT_WINDOW)
	  odata->wscale = (u_int8_t) *ptr;
	else {
	  if (opcode == TCPOPT_MSS) {
	    u_int16_t mss;
	    memcpy(&mss, ptr, sizeof(u_int16_t));
	    odata->mss = ntohs(mss);
	  }
	}
      } else {
	DEBUGP(" opt(%d)", opcode);
	odata->has_unknown_opts = 1;
      }
      ptr+=opsize-2;
      length-=opsize;
    }
  }
  DEBUGP("\n");
}

/* parse options in TCP header, inserting unknown ones */
inline void pers_tcp_add_unknown_opts(struct tcphdr *th,
				      unsigned char *optbuf,
				      unsigned int *optlen)
{
  unsigned char *ptr = (unsigned char *)(th + 1);
  int length=(th->doff*4)-sizeof(struct tcphdr);

  while(length>0) {
    unsigned char opcode=*ptr++;
    unsigned char opsize;

    switch (opcode) {
    case TCPOPT_EOL:
      return;
    case TCPOPT_NOP:
      length--;
      continue;
    default:
      opsize=*ptr++;
      if (opsize < 2) /* "silly options" */
	return;
      if (opsize > length)
	return;	/* don't parse partial options */
      if (opcode > IP_PERS_MAX_OPTS) {
	DEBUGP(" opt(%d)", opcode);
	if (pers_tcp_add_opt(opcode, ptr-2, optbuf, optlen)) {
	  DEBUGP("*** error : not enough room to add option %i\n", opcode);
	  return;
	}
      }
      ptr+=opsize-2;
      length-=opsize;
    }
  }
}

/* replace old TCP options with new ones
   completely /stolen/ from mangle_packets() in ip_nat_ftp.c
   blame Rusty's hamster if this is buggy or ours for the fragmentation stuff */
static inline int pers_tcp_replace_opts(struct tcphdr *tcph,
					const struct net_device *in,
					const struct net_device *out,
					struct sk_buff **pskb,
					unsigned char *newopts,
					unsigned int newoptlen)
{
  struct iphdr *iph = (*pskb)->nh.iph;
  unsigned char *data;
  int tcplen, oldiplen, newiplen, newtcplen;
  int optlen;

  /* the following doesn't handle well fragmented packets yet, so ignore them */
  if (iph->frag_off & __constant_htons(IP_MF)) {
    DEBUGP("PERS:  Not replacing options of first fragmented IP packet\n");
    return IP_PERS_ERROR;
  }
  
  optlen = tcph->doff * 4 - sizeof(struct tcphdr);
  tcplen = (*pskb)->len - iph->ihl * 4;
  newtcplen = tcplen - optlen + newoptlen;
  oldiplen = (*pskb)->len;
  newiplen = iph->ihl * 4 + newtcplen;

  if (newiplen > 65535) {
    if (net_ratelimit())
      printk(KERN_WARNING "PERS: OPT: cheat: %u.%u.%u.%u:%u->%u.%u.%u.%u:%u ignored\n",
	     NIPQUAD(iph->saddr), tcph->source,
	     NIPQUAD(iph->daddr), tcph->dest);
    return IP_PERS_ERROR;
  }

  if (newiplen > (*pskb)->len + skb_tailroom(*pskb)) {
    struct sk_buff *newskb;
    DEBUGP("PERS:   OPT: Need to enlarge skb\n");
    newskb = skb_copy_expand(*pskb, skb_headroom(*pskb), newiplen, GFP_ATOMIC);
    if (!newskb) {
      if (net_ratelimit())
	printk(KERN_WARNING "PERS: Out of memory for options buffer\n");
      return IP_PERS_ERROR;
    } else {
      newskb->sk = (*pskb)->sk;
      newskb->dev = (*pskb)->dev;
      (*pskb)->sk = NULL;
      (*pskb)->dev = NULL;
      kfree_skb(*pskb);
      *pskb = newskb;
      iph = (*pskb)->nh.iph;
      tcph = (void *)iph + iph->ihl*4;
    }
  }
  
  data = (void *)tcph + tcph->doff*4;
  
  /* do the real thing */
  if (newoptlen != optlen)
    memmove(data + newoptlen - optlen, data, tcplen - optlen - sizeof(struct tcphdr));
  memcpy((unsigned char *)(tcph + 1), newopts, newoptlen);
  
  /* Resize packet. */
  if (newiplen > (*pskb)->len) {
    DEBUGP("PERS:   OPT: Extending packet from %u to %u bytes\n",
	   oldiplen, newiplen);
    skb_put(*pskb, newiplen - oldiplen);
  } else if (newiplen < (*pskb)->len){
    DEBUGP("PERS:   OPT: Shrinking packet from %u to %u bytes\n",
	   oldiplen, newiplen);
    skb_trim(*pskb, newiplen);
  }

  /* Fix IP and TCP headers */
  iph->tot_len = htons(newiplen);
  tcph->doff = (sizeof(struct tcphdr) + newoptlen) / 4;
  (*pskb)->csum = csum_partial((char *)tcph + tcph->doff*4,
			       newtcplen - tcph->doff * 4, 0);
  tcph->check = 0;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
  tcph->check = tcp_v4_check(newtcplen, iph->saddr, iph->daddr,
			     csum_partial((char *)tcph, tcph->doff*4,
					  (*pskb)->csum));
#else
  tcph->check = tcp_v4_check(tcph, newtcplen, iph->saddr, iph->daddr,
			     csum_partial((char *)tcph, tcph->doff*4,
					  (*pskb)->csum));
#endif

  /* Black magic here:
     if we enlarge the packet beyond the MTU of the outgoing interface,
     we need to have the local IP stack fragemnt it as the originating
     TCP stack will not understand why it needs to resend an already
     well sized packet. */
  if (newiplen > oldiplen) {
    /* If packet has been already routed, it is probably local,
       so just have the kernel fragment it right away otherwise
       it will reject it. */
    if (out) {
      if ((newiplen > out->mtu) && (oldiplen <= out->mtu)) {
	/* allow more fragmentation and set IP id if needed */
	iph->frag_off &= htons(~IP_DF);
	if (iph->id == 0)
	  ip_select_ident(iph, (*pskb)->dst, NULL);
	DEBUGP("PERS:   OPT: Fragmenting IP packet exceeding MTU (%u).\n", out->mtu);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
	ip_fragment((*pskb), dst_output);
#else
	ip_fragment((*pskb), ip_finish_output);
#endif
	return IP_PERS_KEEP;
      }
    } else {
      /* if the packet would have been rejected */
      if (iph->frag_off & __constant_htons(IP_DF)) {
	/* compute the outgoing interface */
	if ((*pskb)->dst == NULL) {
	  if (!ip_route_input(*pskb, iph->daddr, iph->saddr, iph->tos, in))
	    out = (*pskb)->dst->dev;
	} else
	  out = (*pskb)->dst->dev;
	if (out && (newiplen > out->mtu) && (oldiplen <= out->mtu)) {
	  DEBUGP("PERS:   OPT: Unsetting DF on IP packet exceeding MTU (%u).\n", out->mtu);
	  iph->frag_off &= __constant_htons(~IP_DF);
	}
      }
    }
  }

  /* fix IP checksum */
  ip_send_check(iph);

  return IP_PERS_OK;
}

/* reorder TCP options */
unsigned int pers_tcp_reorder_opts(struct ip_pers * pers,
				   struct tcphdr * tcph,
				   const struct net_device *in,
				   const struct net_device *out,
				   struct sk_buff **pskb,
				   struct ip_pers_tcp_info * odata)
{
  struct ip_pers_vm_state state;
  unsigned int ret;
  
  if (pers->opt_prog.instr == NULL || pers->opt_prog.prog_len == 0) 
    return IP_PERS_OK; /* nothing to do */

  if (!odata->initialized)
    pers_tcp_parse_opts(tcph, odata);

  memset(&state, 0, sizeof(state));

  /* run program and fill optout_buf */
  DEBUGP("PERS:   Adding options:");
  ret = pers_vm_run_prog(&(pers->opt_prog), in, out, (*pskb)->nh.iph, tcph,
			 odata, pers, &state);

  if (ret != IP_PERS_ACCEPT) {
    DEBUGP("\n");
    return IP_PERS_ERROR;
  }

  if (pers->opt_keep_unused) {
    /* insert known options not handled in prog */
    u_int8_t i;
    
    for (i=0; i < IP_PERS_MAX_OPTS; i++) {
      if (odata->seen_opt[i] && !odata->used_opt[i]) {
	if(pers_tcp_add_opt(i, odata->seen_opt[i],
			    state.optout_buf, &state.optout_len)) {
	  DEBUGP("*** error : not enough room to add option %i\n", i);
	  return IP_PERS_ERROR;
	}
	DEBUGP(" %s", tcpopts[i]);
      }
    }
  }

  if (pers->opt_keep_unknown && odata->has_unknown_opts) {
    /* insert unknown options that can't be handled in prog */
    pers_tcp_add_unknown_opts(tcph, state.optout_buf, &state.optout_len);
  }

  pers_tcp_add_padding(state.optout_buf, &state.optout_len);
  DEBUGP("\n");

  /* reinitialize odata for potential later call since options changed */
  odata->initialized = 0;
  
  /* replace old options with new ones and update sk_buff */
  return pers_tcp_replace_opts(tcph, in, out, pskb, state.optout_buf, state.optout_len);
}

/* Update timestamp for local packet */
unsigned int pers_tcp_timestamp_scale(struct iphdr *iph, struct tcphdr *th,
				      struct ip_pers_conn_info *pci,
				      struct ip_pers * pers,
				      struct ip_pers_tcp_info * odata)
{
  u_int32_t diffs[2];
  
  if (!odata->initialized)
    pers_tcp_parse_opts(th, odata);

  if (!odata->seen_opt[TCPOPT_TIMESTAMP] || /* nothing to scale :) */
      pci->timestamp_scale == 0) /* shouldn't happen.. */
    return IP_PERS_OK;

  /* in the following we assume timestamp was set by the local host
     to its jiffies value, at HZ frequency, and that time between
     writing the timestamp and going through us is negligible */
  if (pers->tweak_type == IP_PERS_TWEAK_SRC) {
    /* save the old timestamp */
    memcpy(&(diffs[0]), odata->seen_opt[TCPOPT_TIMESTAMP] + 2, 4);

    /* compute new one */
    diffs[1] = htonl((jiffies * pci->timestamp_scale) / HZ);

    DEBUGP("PERS:   Scaled timestamp on pkt from %u.%u.%u.%u:%u (%u -> %u)\n",
	   NIPQUAD(iph->saddr), ntohs(th->source),
	   ntohl(diffs[0]), ntohl(diffs[1]));

    /* replace and update checksum */
    memcpy(odata->seen_opt[TCPOPT_TIMESTAMP] + 2, &(diffs[1]), 4);
    diffs[0] = ~diffs[0];
    th->check = csum_fold(csum_partial((char *)diffs,
				       sizeof(diffs),
				       th->check^0xFFFF));
  } else if (pers->tweak_type == IP_PERS_TWEAK_DST) {
    /* save the old timestamp */
    memcpy(&diffs, odata->seen_opt[TCPOPT_TIMESTAMP] + 6, 4);

    /* compute the new one */
    diffs[1] = htonl(jiffies +
		     ((((jiffies * pci->timestamp_scale) / HZ) -
		       ntohl(diffs[0])) * HZ) / pci->timestamp_scale);

    DEBUGP("PERS:   Unscaled timestamp on pkt from %u.%u.%u.%u:%u (%u -> %u)\n",
	   NIPQUAD(iph->saddr), ntohs(th->source),
	   ntohl(diffs[0]), ntohl(diffs[1]));

    /* replace and update checksum */
    memcpy(odata->seen_opt[TCPOPT_TIMESTAMP] + 6, &(diffs[1]), 4);
    diffs[0] = ~diffs[0];
    th->check = csum_fold(csum_partial((char *)diffs,
				       sizeof(diffs),
				       th->check^0xFFFF));
  }

  return IP_PERS_OK;
}
