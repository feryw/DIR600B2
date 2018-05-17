/*
 * IP Personality
 *   ip_personality_core.c - Main module
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
 * $Id: ip_personality_core.c,v 1.48 2001/07/22 22:00:09 g_roualland Exp $
 *
 */

#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <net/icmp.h>
#include <net/ip.h>
#include <net/tcp.h>
#include <net/route.h>
#include <linux/netfilter_ipv4/ip_conntrack.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netfilter_ipv4/ip_personality.h>
#include <linux/sched.h>
#include <linux/proc_fs.h>
#include <linux/sysctl.h>

/* proc/sys/net/ipv4/ip_personality_debug */
#ifdef IP_PERS_DEBUG
unsigned int ip_pers_debug = IP_PERS_DEBUG;

#define DEBUGP if (ip_pers_debug & IP_PERS_DEBUG_CORE) printk

/* log a TCP packet - verbose, but not as much as the LOG target */
inline static void pers_tcp_log_packet(const struct net_device *in,
				       struct iphdr * iph,
				       struct tcphdr * th,
				       unsigned int hooknum)
{
  if (ip_pers_debug & IP_PERS_DEBUG_CORE)
    printk("PERS: TCP: %u.%u.%u.%u:%u => %u.%u.%u.%u:%u, " 
	   "flags=%s%s%s%s%s%s, seq=%u, ack=%u, win=%u, ipid=%u [hook=%u]\n",
	   NIPQUAD(iph->saddr), ntohs(th->source),
	   NIPQUAD(iph->daddr), ntohs(th->dest),
	   (th->syn ? "S" : ""),
	   (th->psh ? "P" : ""),
	   (th->ack ? "A" : ""),
	   (th->fin ? "F" : ""),
	   (th->rst ? "R" : ""),
	   (th->urg ? "U" : ""),
	   ntohl(th->seq), ntohl(th->ack_seq), ntohs(th->window),
	   ntohs(iph->id),
	   hooknum);
}

inline static void pers_udp_log_packet(const struct net_device *in,
				       struct iphdr * iph,
				       struct udphdr * uh,
				       unsigned int hooknum)
{
  if (ip_pers_debug & IP_PERS_DEBUG_CORE)
    printk("PERS: UDP: %u.%u.%u.%u:%u => %u.%u.%u.%u:%u, " 
	   "ipid=%u [hook=%u]\n",
	   NIPQUAD(iph->saddr), ntohs(uh->source),
	   NIPQUAD(iph->daddr), ntohs(uh->dest),
	   ntohs(iph->id),
	   hooknum);
}

#else
#define DEBUGP(format, args...)
#define pers_tcp_log_packet(a, b, c, d)
#define pers_udp_log_packet(a, b, c, d)
#endif

/* proc/sys/net/ipv4/ip_personality_sport, add by Ella */

//unsigned int ip_pers_sport;

#ifdef CONFIG_SYSCTL

#define NET_IPV4_PERS_SPORT 1789
#define NET_IPV4_PERS_ENABLE 1790

static struct ctl_table_header *ip_pers_table_header;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
static ctl_table net_ipv4_pers_table[] = {
  {
	.ctl_name	= NET_IPV4_PERS_SPORT,
	.procname	= "ip_personality_sport",
	.data		= &ip_pers_sport,
	.maxlen		= sizeof(ip_pers_sport),
	.mode		= 0644,
	.proc_handler	= proc_dointvec,
  },
  {
	.ctl_name	= NET_IPV4_PERS_ENABLE,
	.procname	= "ip_personality_enable",
	.data		= &ip_pers_enable,
	.maxlen		= sizeof(ip_pers_enable),
	.mode		= 0644,
	.proc_handler	= proc_dointvec,
  },
#ifdef IP_PERS_DEBUG
#define NET_IPV4_PERS_DEBUG 1790
  {
	.ctl_name	= NET_IPV4_PERS_DEBUG,
	.procname	= "ip_personality_debug",
	.data		= &ip_pers_debug,
	.maxlen		= sizeof(ip_pers_debug),
	.mode		= 0644,
	.proc_handler	= proc_dointvec,
  },
#endif
  { 	.ctl_name	= 0 }

};

static ctl_table net_ipv4_table[] = {
  {
	.ctl_name	= NET_IPV4,
	.procname	= "ipv4",
	.mode		= 0555,
	.child		= net_ipv4_pers_table, 
  },
  { 	.ctl_name	= 0 }
};

static ctl_table net_table[] = {
  {
	.ctl_name	= CTL_NET,
	.procname	= "net",
	.mode		= 0555,
	.child		= net_ipv4_table, 
  },
  { 	.ctl_name	= 0 }
};
#else
static ctl_table net_ipv4_pers_table[] = {
  { NET_IPV4_PERS_SPORT, "ip_personality_sport", &ip_pers_sport,
    sizeof(ip_pers_sport), 0644, NULL, proc_dointvec },
  { NET_IPV4_PERS_ENABLE, "ip_personality_enable", &ip_pers_enable,
    sizeof(ip_pers_enable), 0644, NULL, proc_dointvec },
#ifdef IP_PERS_DEBUG
#define NET_IPV4_PERS_DEBUG 1790
  { NET_IPV4_PERS_DEBUG, "ip_personality_debug", &ip_pers_debug,
    sizeof(ip_pers_debug), 0644, NULL, proc_dointvec },
#endif
  { 0 }
};

static ctl_table net_ipv4_table[] = {
  { NET_IPV4, "ipv4", NULL, 0, 0555, net_ipv4_pers_table },
  { 0 }
};

static ctl_table net_table[] = {
  { CTL_NET, "net", NULL, 0, 0555, net_ipv4_table },
  { 0 }
};
#endif

#endif

#define PERS_RET(result) { if((ret = (result)) != IP_PERS_OK) return ret; }

/* Initialize connection data to rewrite its packets */
static inline void pers_tcp_mangle_new(struct iphdr * iph,
                                       struct tcphdr * th,
				       struct ip_pers_conn_info *pci,
				       struct ip_pers * pers,
				       const struct net_device *in,
				       const struct net_device *out,
				       struct sk_buff **pskb,
				       struct ip_pers_tcp_info * odata) {
  /* initialize ISN rewriting */
  pers_tcp_mangle_seq_new(iph, th, pci, pers);

  /* initialize WIN rewriting */
  pers_tcp_mangle_win_new(iph, th, pci, pers, odata);
  
  /* inialize timestamp scaling if needed */
  if (pers->local && pers->timestamp_scale != HZ) 
    pci->timestamp_scale = pers->timestamp_scale;
  else
    pci->timestamp_scale = 0;

  pci->status = IP_PERS_CONN_ESTABLISHED;
}

/* Rewrite packets of an extablished connections */
static inline unsigned int pers_tcp_mangle_est(struct iphdr * iph,
					       struct tcphdr * th,
					       struct ip_pers_conn_info *pci,
					       struct ip_pers * pers,
					       const struct net_device *in,
					       const struct net_device *out,
					       struct sk_buff **pskb,
					       struct ip_pers_tcp_info * odata,
					       enum ip_conntrack_info ctinfo) {
  unsigned int ret;
  
  /* http request(cookie) frag */
  if (pers->http_cookiefrag && (th->syn == 0) && th->dest == htons(80))
    PERS_RET(pers_http_hold(iph, th, pci, pers, pskb));
  
  /* ISN rewriting works in both ways (seq/ack). */
  if (pci->seq_offset)
    PERS_RET(pers_tcp_mangle_seq(iph, th, pci, pers, pskb, ctinfo));

  /* Timestamp scaling */
  if (pci->timestamp_scale)
    PERS_RET(pers_tcp_timestamp_scale(iph, th, pci, pers, odata));
  
  if (pers->tweak_type == IP_PERS_TWEAK_SRC) {
    /* Window rewriting */
    if (pci->win_offset && th->syn)
      PERS_RET(pers_tcp_mangle_win(iph, th, pci, pers, pskb));
    
    /* Options rewriting */
    if (pers->opt_prog.prog_len)
      PERS_RET(pers_tcp_reorder_opts(pers, th, in, out, pskb, odata));
  }
  
  return IP_PERS_OK;
}

/* Rewrite packets of unknown connections -- isolated packets */
static inline unsigned int pers_tcp_mangle_other(struct iphdr * iph,
						 struct tcphdr * th,
						 struct ip_pers_conn_info *pci,
                                                 struct ip_pers * pers,
						 const struct net_device *in,
						 const struct net_device *out,
						 struct sk_buff **pskb,
						 struct ip_pers_tcp_info * odata) {
  unsigned int ret;
  
  /* right now, we only do options rewrting here */
  if ((pers->tweak_type == IP_PERS_TWEAK_SRC) && pers->opt_isolated
      && pers->opt_prog.prog_len)
    PERS_RET(pers_tcp_reorder_opts(pers, th, in, out, pskb, odata));

  return IP_PERS_OK;
}


static inline unsigned int pers_tcp_mangle(struct sk_buff **pskb,
					   unsigned int hooknum,
					   const struct net_device *in,
					   const struct net_device *out,
					   const void *targinfo)

{
  struct ip_conntrack *ct;
  enum ip_conntrack_info ctinfo;
  struct ip_pers * pers = (struct ip_pers *)targinfo;
  struct tcphdr * th;
  struct iphdr * iph;
  struct ip_pers_tcp_info odata;
  unsigned int ret;

  odata.initialized = 0;
  
  iph = (*pskb)->nh.iph;
  th = (struct tcphdr *) ((void *)iph + 4*iph->ihl);

  pers_tcp_log_packet(in, iph, th, hooknum);
  
  if (!pers->initialized) { /* it is the first time that target matches */
    DEBUGP("PERS:   First pass in rule, initializing ISN.\n");
    if (pers->isn_type == IP_PERS_TIME_INC)
      pers->isn_jiffies = jiffies;
    if (pers->ipid_type == IP_PERS_TIME_INC)
      pers->ipid_jiffies = jiffies;
    pers->initialized = 1;
  }

  ct = ip_conntrack_get(*pskb, &ctinfo);

  if ((ct && !(ctinfo == IP_CT_ESTABLISHED || ctinfo == IP_CT_IS_REPLY)) || !ct)
    if (pers->tweak_type == IP_PERS_TWEAK_DST && pers->local)
      PERS_RET(pers_tcp_decoy(pers, iph, th, in, out, pskb, &odata));

  if(ct) {
    DEBUGP("PERS:   Conntrack status %d, %s%s%s%s\n",
	   ctinfo,
	   (ctinfo == IP_CT_ESTABLISHED ? "EST " : ""),
	   (ctinfo == IP_CT_RELATED ? "REL " : ""),
	   (ctinfo == IP_CT_NEW ? "NEW " : ""),
	   (ctinfo >= IP_CT_IS_REPLY ? "REP " : ""));
    
    if (ct->pers.status &&
	(ctinfo == IP_CT_ESTABLISHED || ctinfo == IP_CT_IS_REPLY)) {
      /* this is a known connection */
      if ((ct->pers.status == IP_PERS_CONN_WAITFORACK) &&
	  th->syn && th->ack && (pers->tweak_type == IP_PERS_TWEAK_SRC))
	/* this is a SYN+ACK we were waiting for,
	 * now we can consider the connection established */
	pers_tcp_mangle_new(iph, th, &(ct->pers), pers, in, out, pskb, &odata);
      
      if (ct->pers.status == IP_PERS_CONN_ESTABLISHED)
	PERS_RET(pers_tcp_mangle_est(iph, th, &(ct->pers), pers, in, out, pskb, &odata, ctinfo));
    }
    
    if (ctinfo == IP_CT_NEW) {
      /* set up a new connection */
      DEBUGP("PERS:   New connection! (%u.%u.%u.%u:%u => %u.%u.%u.%u:%u)\n",
	     NIPQUAD(iph->saddr), ntohs(th->source),
	     NIPQUAD(iph->daddr), ntohs(th->dest));
      
      if (pers->tweak_type == IP_PERS_TWEAK_SRC) {
	if (!ct->pers.status && (pers->tcp_way & IP_PERS_TCP_OUT))
	  /* we have not seen that connection yet */
	  pers_tcp_mangle_new(iph, th, &(ct->pers), pers, in, out, pskb, &odata);
	if (ct->pers.status)
	  PERS_RET(pers_tcp_mangle_est(iph, th, &(ct->pers), pers, in, out, pskb, &odata, ctinfo));
      } else if (pers->tweak_type == IP_PERS_TWEAK_DST) {
	/* need SYN+ACK to complete setup of connection */
	if (!ct->pers.status && (pers->tcp_way & IP_PERS_TCP_IN)) {
	  DEBUGP("PERS:   External connection, waiting for SYN ACK.\n");
	  ct->pers.status = IP_PERS_CONN_WAITFORACK;
	}
      }
    }

    if (!ct->pers.status)
      PERS_RET(pers_tcp_mangle_other(iph, th, (&ct->pers), pers, in, out, pskb, &odata));
  } else
    PERS_RET(pers_tcp_mangle_other(iph, th, (&ct->pers), pers, in, out, pskb, &odata));
  
  return IP_PERS_OK;
}

static inline unsigned int pers_udp_mangle(struct sk_buff **pskb,
					   unsigned int hooknum,
					   const struct net_device *in,
					   const struct net_device *out,
					   const void *targinfo)
{
  struct udphdr * uh;
  struct iphdr * iph;
  struct ip_pers *pers = (struct ip_pers *)targinfo;
  int ret;
  
  iph = (*pskb)->nh.iph;
  uh = (struct udphdr *) ((void *)iph + 4*iph->ihl);

  pers_udp_log_packet(in, iph, uh, hooknum);

  if (pers->tweak_type == IP_PERS_TWEAK_DST && pers->local)
    PERS_RET(pers_udp_check(pers, iph, uh, in, out, pskb));

  return IP_PERS_OK;
}

static inline unsigned int pers_ip_mangle(struct sk_buff **pskb,
					   unsigned int hooknum,
					   const struct net_device *in,
					   const struct net_device *out,
					   const void *targinfo)
{
  struct iphdr * iph;
  struct ip_pers *pers = (struct ip_pers *)targinfo;
  int ret;
  
  iph = (*pskb)->nh.iph;

  if (pers->tweak_type == IP_PERS_TWEAK_SRC &&
      !(ntohs(iph->frag_off) & IP_MF)){ /* skip fragmented packets */
    pers_ip_mangle_ttl(pers, iph, pskb);
    PERS_RET(pers_ip_mangle_id(pers, iph, pskb));
  }
  
  return IP_PERS_OK;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
static unsigned int pers_target(struct sk_buff **pskb,
				const struct net_device *in,
				const struct net_device *out,
				unsigned int hooknum,
				const struct xt_target *target,
				const void *targinfo,
				void *userinfo)
#else
static unsigned int pers_target(struct sk_buff **pskb,
				unsigned int hooknum,
				const struct net_device *in,
				const struct net_device *out,
				const void *targinfo,
				void *userinfo)
#endif
{
  unsigned int ret;
  
  /* not the first fragment */
  if(ntohs((*pskb)->nh.iph->frag_off) & IP_OFFSET)
    return IP_PERS_OK;

  /* TCP stuff first */
  if ((*pskb)->nh.iph->protocol == IPPROTO_TCP)
    PERS_RET(pers_tcp_mangle(pskb, hooknum, in, out, targinfo));

  /* handle UDP */
  if ((*pskb)->nh.iph->protocol == IPPROTO_UDP)
    PERS_RET(pers_udp_mangle(pskb, hooknum, in, out, targinfo));

  /* finally handle IP ID */
  return pers_ip_mangle(pskb, hooknum, in, out, targinfo);
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
static int pers_check_rule(const char *tablename,
		 const void *e,
		 const struct xt_target *target,
		 void *targinfo,
		 unsigned int hook_mask)
#else
static int pers_check_rule(const char *tablename,
		 const struct ipt_entry *e,
		 void *targinfo,
		 unsigned int targinfosize,
		 unsigned int hook_mask)
#endif
{
  struct ip_pers *pers = (struct ip_pers *)targinfo;

  if (pers->tweak_type != IP_PERS_TWEAK_SRC && pers->tweak_type != IP_PERS_TWEAK_DST) {
    printk(KERN_WARNING "PERS: No valid tweak type specified!\n");
    return 0;
  }

  if (!(pers->tcp_way & (IP_PERS_TCP_IN + IP_PERS_TCP_OUT))) {
    printk(KERN_WARNING "PERS: No TCP type specified!\n");
    return 0;
  }

  if (strcmp(tablename, "mangle") != 0) {
    printk(KERN_WARNING "PERS: Can only be called from \"mangle\" table, not \"%s\"\n",
	   tablename);
    return 0;
  }

  return 1;
}


#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
static struct ipt_target ip_pers_target = { 
	.list		= { NULL, NULL }, 
	.name		= "PERS", 
	.target 	= &pers_target, 
	.checkentry	= &pers_check_rule, 
	.destroy	= NULL, 
	.hooks		= 1<<NF_IP_POST_ROUTING,
	.targetsize	= sizeof(struct ip_pers),
	.table		= "mangle",
	.me		THIS_MODULE 
};
#else
static struct ipt_target ip_pers_target
= { { NULL, NULL }, "PERS", pers_target, pers_check_rule, NULL, THIS_MODULE };
#endif

static int __init init(void)
{
#ifdef CONFIG_SYSCTL
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
  if ((ip_pers_table_header =
       register_sysctl_table(net_table)) == NULL) {
#else
  if ((ip_pers_table_header =
       register_sysctl_table(net_table, 0)) == NULL) {
#endif
    printk("PERS: Cannot register sysctl.\n");
    return -EINVAL;
  }
#endif  

  if (ipt_register_target(&ip_pers_target))
    return -EINVAL;
  
  printk("PERS: Successfully registered IPTable target.\n");
  
  return 0;
}

static void __exit fini(void)
{
  ip_pers_enable=0;
  ipt_unregister_target(&ip_pers_target);
  printk("PERS: Unregistered IPTable target.\n");
#ifdef CONFIG_SYSCTL
  unregister_sysctl_table(ip_pers_table_header);
#endif  
}

module_init(init);
module_exit(fini);
MODULE_LICENSE("GPL");
