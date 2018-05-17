/*
 * IP Personality
 *   ip_personality.h - Defines/Prototypes.
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
 * $Id: ip_personality.h,v 1.50 2001/07/22 21:59:35 g_roualland Exp $
 *
 */

#ifndef _IP_PERSONALITY_H
#define _IP_PERSONALITY_H

/* Debug categories */
#define IP_PERS_DEBUG_CORE    1
#define IP_PERS_DEBUG_SEQ     2
#define IP_PERS_DEBUG_OPTS    4
#define IP_PERS_DEBUG_WIN     8
#define IP_PERS_DEBUG_DECOY  16 
#define IP_PERS_DEBUG_VM     32
#define IP_PERS_DEBUG_UDP    64
#define IP_PERS_DEBUG_IPID  128
#define IP_PERS_DEBUG_HTTP	256

/* The following defines the default debug level for the module,
   as a bitfield of the previous values for submodules.
   If you have systcl enabled, this can be set at runtime
   by writing to /proc/sys/net/ipv4/ip_personality_debug.
   Undef this if you want to get rid of all debugging code. */
#define IP_PERS_DEBUG 0

#ifdef IP_PERS_DEBUG
extern unsigned int ip_pers_debug;
#endif

extern unsigned int ip_pers_sport;
extern unsigned int ip_pers_enable;

/* Generator types */
#define IP_PERS_ASIS          1
#define IP_PERS_FIXED_INC     2
#define IP_PERS_RAND_INC      3
#define IP_PERS_RANDOM        4
#define IP_PERS_BUILTIN       5
#define IP_PERS_TIME_INC      6
#define IP_PERS_BROKEN_INC    7

/* Rules way */
#define IP_PERS_TWEAK_SRC     1
#define IP_PERS_TWEAK_DST     2

/* Connection settings */
#define IP_PERS_TCP_IN        1
#define IP_PERS_TCP_OUT       2

/* Max code length */
#define IP_PERS_MAX_CODE      256

struct ip_pers_prog {
  u_int32_t prog_len;
  u_int32_t instr[IP_PERS_MAX_CODE];
};

#define IP_PERS_MAX_UDP       12

struct ip_pers {
  u_int32_t initialized;
  u_int32_t local;
  u_int32_t current_isn;
  u_int32_t isn_type;
  u_int32_t isn_param;
  u_int32_t ipid_type;
  u_int16_t ipid_param;
  u_int16_t current_ipid;
  u_int8_t current_ipttl;
  u_int32_t http_cookiefrag;
  u_int32_t tweak_type;
  u_int32_t tcp_way;
  u_int32_t opt_keep_unknown;
  u_int32_t opt_keep_unused;
  u_int32_t opt_isolated;
  u_int32_t tcp_maxwin;
  u_int32_t udp_unreach;
  u_int16_t udp_info[IP_PERS_MAX_UDP];
  char id[21];
  u_int32_t isn_jiffies;
  u_int32_t ipid_jiffies;
  u_int32_t timestamp_scale;
  struct ip_pers_prog opt_prog;
  struct ip_pers_prog decoy_prog;
}/* __attribute__ ((packed))*/;

/* Connexion status */
#define IP_PERS_CONN_UNKNOWN 0
#define IP_PERS_CONN_ESTABLISHED 1
#define IP_PERS_CONN_WAITFORACK 2

struct ip_pers_conn_info {
  u_int32_t status;
  u_int32_t seq_offset;
  u_int32_t win_offset;
  u_int32_t timestamp_scale;
};

/* defines and struct for option replacement programs */

/* Options 16-18 are not recognized */
#define IP_PERS_MAX_OPTS      16

struct ip_pers_tcp_info {
  /* presence of options */
  u_int8_t initialized, has_unknown_opts;
  u_int8_t * seen_opt[IP_PERS_MAX_OPTS];
  u_int8_t used_opt[IP_PERS_MAX_OPTS];
  u_int16_t flags;
  u_int16_t mss;
  u_int8_t wscale;
} /*__attribute__ ((packed))*/;

/* Output buffer size: a TCP packet options
   section can not be more than 44 bytes (4 * 16 - 4). */
#define IP_PERS_MAX_OPT_LEN   44

struct ip_pers_vm_state {
  u_int16_t flags;
  u_int16_t win;
  u_int32_t ack;
  u_int32_t df;
  u_int32_t mss;
  u_int32_t wscale;
  u_int32_t timestamp;
  unsigned char optout_buf[IP_PERS_MAX_OPT_LEN];
  unsigned int optout_len;
} /*__attribute__ ((packed))*/;

/* mask to extract TCP flags in TCP header */
#define IP_PERS_EXTRACT_FLAGS 0x0FFF

/* masks to extract instructions and data */
#define IP_PERS_INST          0xFF000000
#define IP_PERS_IOPT          0x00F00000
#define IP_PERS_DATA          0x000FFFFF

/* 'opcodes' for instructions */
#define IP_PERS_TEST          0x01000000
#define IP_PERS_JMP           0x02000000
#define IP_PERS_PUT           0x03000000
#define IP_PERS_SET           0x04000000
#define IP_PERS_RET           0x05000000

/* 'opcodes' for instruction options */
#define IP_PERS_TEST_OPTION   0x00000000
#define IP_PERS_TEST_FLAG_ANY 0x00100000
#define IP_PERS_TEST_FLAG_ALL 0x00200000
#define IP_PERS_TEST_ACK      0x00300000
#define IP_PERS_TEST_LISTEN   0x00400000

#define IP_PERS_PUT_COPY      0x00000000
#define IP_PERS_PUT_INS       0x00100000

#define IP_PERS_SET_FLAGS     0x00000000
#define IP_PERS_SET_ACK       0x00100000
#define IP_PERS_SET_DF        0x00200000
#define IP_PERS_SET_WIN       0x00300000
#define IP_PERS_SET_MSS       0x00400000
#define IP_PERS_SET_WSCALE    0x00500000
#define IP_PERS_SET_TIMESTAMP 0x00600000
#define IP_PERS_SET_FROM_THIS 0x00800000

/* return values for vm */
#define IP_PERS_FAILED        0
#define IP_PERS_ACCEPT        1
#define IP_PERS_DROP          2
#define IP_PERS_REPLY         3

/* return values for 'hooks' */
#define IP_PERS_ERROR         NF_ACCEPT
#define IP_PERS_OK            NF_ACCEPT
#define IP_PERS_KEEP          NF_STOLEN
#define IP_PERS_REJ           NF_DROP

/* UDP settings */
#define IP_PERS_UDP_REPLY     0
#define IP_PERS_UDP_DF        1
#define IP_PERS_UDP_TOS       2
#define IP_PERS_UDP_MAXLEN    3
#define IP_PERS_UDP_RIPLEN    4
#define IP_PERS_UDP_RIPID     5
#define IP_PERS_UDP_RIPCSUM   6
#define IP_PERS_UDP_RCSUM     7
#define IP_PERS_UDP_RLEN      8
#define IP_PERS_UDP_RDATA     9
#define IP_PERS_UDP_RIPLEN_SAME 10
#define IP_PERS_UDP_RLEN_SAME 11

/* UDP actions */
#define IP_PERS_UDP_SAME      0
#define IP_PERS_UDP_ZERO      1
#define IP_PERS_UDP_MANGLE    2

#ifdef __KERNEL__
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/skbuff.h>
#include <net/ip.h>
#include <net/tcp.h>
#include <net/sock.h>
#include <net/route.h>
#include <linux/netfilter_ipv4/ip_conntrack.h>

/* ip_personality_seq routines */
unsigned int pers_tcp_mangle_seq(struct iphdr *iph, struct tcphdr *th,
				 struct ip_pers_conn_info *pci,
				 struct ip_pers *pers,
				 struct sk_buff **pskb,
				 enum ip_conntrack_info ctinfo);
void pers_tcp_next_seq(struct iphdr *iph, struct tcphdr *th,
		       struct ip_pers *pers);
void pers_tcp_mangle_seq_new(struct iphdr *iph, struct tcphdr *th,
			     struct ip_pers_conn_info *pci, struct ip_pers *pers);

/* ip_personality_app routines */
unsigned int pers_http_hold(struct iphdr *iph, struct tcphdr *th,
				struct ip_pers_conn_info *pci,
				struct ip_pers *pers,
				struct sk_buff **pskb);

/* ip_personality_opts routines */
unsigned int pers_tcp_reorder_opts(struct ip_pers *pers,
				  struct tcphdr *tcph,
				  const struct net_device *in,
				  const struct net_device *out,
				  struct sk_buff **pskb,
				  struct ip_pers_tcp_info * odata);
void pers_tcp_parse_opts(struct tcphdr *th, struct ip_pers_tcp_info *odata);
void pers_tcp_add_padding(unsigned char *optbuf, unsigned int *optlen);
unsigned int pers_tcp_timestamp_scale(struct iphdr *iph, struct tcphdr *th,
				      struct ip_pers_conn_info *pci,
				      struct ip_pers * pers,
				      struct ip_pers_tcp_info * odata);

/* ip_personality_win routines */
unsigned int pers_tcp_mangle_win(struct iphdr *iph, struct tcphdr *th,
				 struct ip_pers_conn_info *pci,
				 struct ip_pers *pers,
				 struct sk_buff **pskb);
void pers_tcp_mangle_win_new(struct iphdr *iph, struct tcphdr *th,
			     struct ip_pers_conn_info *pci, struct ip_pers *pers,
			     struct ip_pers_tcp_info * odata);
/* ip_personality_vm routines */
inline int pers_tcp_add_opt(unsigned char opt,
			    u_int8_t * data,
			    unsigned char *optbuf,
			    unsigned int *optlen);
unsigned int pers_vm_run_prog(struct ip_pers_prog * prog,
			      const struct net_device *in,
			      const struct net_device *out,
			      struct iphdr * iph,
			      struct tcphdr * th,
			      struct ip_pers_tcp_info * odata,
			      struct ip_pers *pers,
			      struct ip_pers_vm_state * state);

/* ip_personality_decoy routines */
unsigned int pers_tcp_decoy(struct ip_pers * pers,
			    struct iphdr * iph,
			    struct tcphdr * tcph,
			    const struct net_device *in,
			    const struct net_device *out,
			    struct sk_buff **pskb,
			    struct ip_pers_tcp_info * odata);

/* ip_personality_udp routines */
unsigned int pers_udp_check(struct ip_pers * pers,
			    struct iphdr * iph,
			    struct udphdr * uh,
			    const struct net_device *in,
			    const struct net_device *out,
			    struct sk_buff **pskb);

/* ip_personality_ipid routines */
unsigned int pers_ip_mangle_id(struct ip_pers * pers,
			       struct iphdr * iph,
			       struct sk_buff **pskb);

/* ip_personality_ipttl routines */
unsigned int pers_ip_mangle_ttl(struct ip_pers * pers,
			       struct iphdr * iph,
			       struct sk_buff **pskb);

/* common pkt sending code */
static inline unsigned int
pers_send_local(struct sk_buff * skb, struct sk_buff * oskb) {
  void (*attach)(struct sk_buff *, struct sk_buff *);

  /* attach this packet to the initial packet connection if any,
     so conntrack can follow it */
  if (oskb && oskb->nfct && (attach = ip_ct_attach) != NULL)
    attach(skb, oskb);

  return NF_HOOK(PF_INET, NF_IP_LOCAL_OUT, skb, NULL, skb->dst->dev,
		 dst_output);
}
#endif

#endif
