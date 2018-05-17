/*
 * IP Personality
 *   ip_personality_vm.c - Virtual machine (used for decoy/opt)
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
 * $Id: ip_personality_vm.c,v 1.13 2001/07/22 14:17:30 g_roualland Exp $
 *
 */

#include <linux/skbuff.h>
#include <net/ip.h>
#include <net/tcp.h>
#include <net/sock.h>
#include <net/route.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netfilter_ipv4/ip_personality.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
#include <net/inet_hashtables.h>
#endif

#ifdef IP_PERS_DEBUG
#define DEBUGP if (ip_pers_debug & IP_PERS_DEBUG_VM) printk

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

/* determine if we are listening for TCP connections on daddr:dport */
static inline int pers_tcp_dport_isopen(const struct net_device *in,
				 struct iphdr * iph,
				 struct tcphdr * th)
{
  struct sock *sk;

  if(in == NULL)
    return 0;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
  sk = inet_lookup_listener(&tcp_hashinfo, iph->daddr, ntohs(th->dest), in->ifindex);
#else
  sk = tcp_v4_lookup_listener(iph->daddr, ntohs(th->dest), in->ifindex);
#endif
  if(sk != NULL)
    __sock_put(sk); /* because sock_hold() called in tcp_v4_lookup_listener() */

  return (sk != NULL);
}

/* add a TCP option */
inline int pers_tcp_add_opt(unsigned char opt,
			    u_int8_t * data,
			    unsigned char *optbuf,
			    unsigned int *optlen)
{
  unsigned char *optend = optbuf + *optlen;
  unsigned char len;

  if(*optlen + 1 >= IP_PERS_MAX_OPT_LEN)
    return -1;
  if(opt == TCPOPT_NOP || opt == TCPOPT_EOL) {
    *(optend++) = opt;
    (*optlen)++;
    return 0;
  }

  if(!data)
    return 0;

  len = data[1];
  if(*optlen + len >= IP_PERS_MAX_OPT_LEN)
    return -1;
  (*optlen) += len;
  while(len--)
    *(optend++) = *(data++);

  return 0;
}

/*
 * Syntax of instructions :
 * 32 bits bitfield : 8 bits instruction, 24 bits data
 * instructions : TEST, JMP, INS
 * data : type of option, offset
 */

/* 'opcodes' for options are TCPOPT_* in net/tcp.h */
/* codes for TCP flags are TCPCB_FLAG_* in net/tcp.h */
unsigned int pers_vm_run_prog(struct ip_pers_prog * prog,
			      const struct net_device *in,
			      const struct net_device *out,
			      struct iphdr * iph,
			      struct tcphdr * th,
			      struct ip_pers_tcp_info * odata,
			      struct ip_pers *pers,
			      struct ip_pers_vm_state * state)
{
  u_int32_t *ip;  /* instruction pointer */
  u_int32_t *eoi; /* end of instructions */
  u_int32_t cur;  /* current instruction, pointed to by ip */

  /* run program */
  ip = prog->instr;
  eoi = ip + prog->prog_len;

  while(ip < eoi) {
    cur = *ip;
    switch(cur & IP_PERS_INST) {
    case IP_PERS_PUT:
      switch(cur & IP_PERS_IOPT) {
      case IP_PERS_PUT_COPY: {
	unsigned char opcode = (unsigned char) (cur & IP_PERS_DATA);
	
	if(opcode >= IP_PERS_MAX_OPTS) {
	  DEBUGP("*** bogus option %i\n", opcode);
	  return IP_PERS_FAILED;
	}
	/* insert option (data) */
	if(pers_tcp_add_opt(opcode, odata->seen_opt[opcode],
			    state->optout_buf, &(state->optout_len))) {
	  DEBUGP("*** error : not enough room to add option %i\n", opcode);
	  return IP_PERS_FAILED;
	}
	odata->used_opt[opcode] = 1;
	DEBUGP(" %s", tcpopts[opcode]);
	break;
      }
      case IP_PERS_PUT_INS: {
	unsigned char opcode = (unsigned char) (cur & IP_PERS_DATA);
	unsigned char opbuf[20];
	u_int16_t mss;
	u_int32_t tstamp;
	
	switch(opcode) {
	case TCPOPT_MSS:
	  opbuf[0] = TCPOPT_MSS;
	  opbuf[1] = TCPOLEN_MSS;
	  mss = htons(state->mss);
	  memcpy(opbuf+2, &mss, sizeof(mss));
	  break;
	case TCPOPT_WINDOW:
	  opbuf[0] = TCPOPT_WINDOW;
	  opbuf[1] = TCPOLEN_WINDOW;
	  opbuf[3] = (unsigned char) state->wscale;
	  break;
	case TCPOPT_TIMESTAMP:
	  opbuf[0] = TCPOPT_TIMESTAMP;
	  opbuf[1] = TCPOLEN_TIMESTAMP;
	  tstamp = htonl(state->timestamp);
	  memcpy(opbuf + 2, &tstamp, 4);
	  if (odata->seen_opt[TCPOPT_TIMESTAMP])
	    memcpy(opbuf + 2 + 4,
		   odata->seen_opt[TCPOPT_TIMESTAMP] + 2, 4);
	  else {
	    tstamp = 0;
	    memcpy(opbuf + 2 + 4, &tstamp, 4);
	  }
	  break;
	default:
	  DEBUGP("*** found illegal option\n");
	  opcode = TCPOPT_EOL;
	  opbuf[0] = TCPOPT_EOL;
	  break;
	}
	if(pers_tcp_add_opt(opcode, opbuf,
			    state->optout_buf, &(state->optout_len))) {
	  DEBUGP("*** error : not enough room to add option %i\n", opcode);
	  return IP_PERS_FAILED;
	}
	DEBUGP(" %s", tcpopts[opcode]);
        break;
      }
      default:
	DEBUGP("*** found illegal instruction option\n");
	break;
      }
      ip++;
      break;
    case IP_PERS_JMP:
      /* JMP contains offset to next instruction */
      ip = prog->instr + (cur & IP_PERS_DATA);
      break;
    case IP_PERS_TEST: {
      /* TEST tests presence of option or TCP flag (data field) */
      /* TEST is followed by JMP to 'else' part */
      u_int32_t dfield = (u_int32_t) (cur & IP_PERS_DATA);

      switch(cur & IP_PERS_IOPT) {
      case IP_PERS_TEST_OPTION:
	if(dfield >= IP_PERS_MAX_OPTS) {
	  DEBUGP("*** bogus option %i\n", dfield);
	  return IP_PERS_FAILED;
	}
	odata->used_opt[dfield] = 1;
	if(odata->seen_opt[dfield])
	  ip++;
	break;
      case IP_PERS_TEST_FLAG_ANY:
	if( (odata->flags & dfield)
	    || ((dfield == 0) && (odata->flags == 0)) )
	  ip++;
	break;
      case IP_PERS_TEST_FLAG_ALL:
	if( ( (odata->flags & dfield) == dfield)
	    || ((dfield == 0) && (odata->flags == 0)) )
	  ip++;
	break;
      case IP_PERS_TEST_LISTEN:
	if (pers_tcp_dport_isopen(in, iph, th))
	  ip++;
	break;
      case IP_PERS_TEST_ACK:
	if (ntohl(th->ack_seq) == dfield)
	  ip++;
	break;
      default:
	DEBUGP("*** found illegal instruction option\n");
	break;
      }
      ip++;
      break;
    }
    case IP_PERS_SET: {
      u_int32_t dfield = (u_int32_t) (cur & IP_PERS_DATA);
      
      switch ((cur & IP_PERS_IOPT) & ~IP_PERS_SET_FROM_THIS) {
      case IP_PERS_SET_FLAGS:
	state->flags = dfield;
	break;
      case IP_PERS_SET_ACK:
	if (cur & IP_PERS_SET_FROM_THIS)
	  state->ack = ntohl(th->seq) + dfield;
	else
	  state->ack = dfield;
	break;
      case IP_PERS_SET_DF:
	if (cur & IP_PERS_SET_FROM_THIS)
	  state->df = (((iph->frag_off & __constant_htons(IP_DF)) != 0) + dfield) % 2;
	else
	  state->df = (dfield != 0);
	break;
      case IP_PERS_SET_WIN:
	if (cur & IP_PERS_SET_FROM_THIS)
	  state->win = ntohs(th->window) + dfield;
	else
	  state->win = dfield;
	break;
      case IP_PERS_SET_MSS:
	if (cur & IP_PERS_SET_FROM_THIS)
	  state->mss = odata->mss + dfield;
	else
	  state->mss = dfield;
	break;
      case IP_PERS_SET_WSCALE:
	if (cur & IP_PERS_SET_FROM_THIS)
	  state->wscale = odata->wscale + dfield;
	else
	  state->wscale = dfield;
	break;
      case IP_PERS_SET_TIMESTAMP:
	if (cur & IP_PERS_SET_FROM_THIS) {
	  if (pers->timestamp_scale && pers->timestamp_scale != HZ)
	    state->timestamp = (jiffies * pers->timestamp_scale) / HZ + dfield;
	  else
	    state->timestamp = jiffies + dfield;
	} else
	  state->timestamp = dfield;
	break;
      default:
	DEBUGP("*** found illegal instruction option\n");
	break;
      }
      ip++;
      break;
    }
    case IP_PERS_RET:
      switch(cur & IP_PERS_DATA) {
      case IP_PERS_ACCEPT:
      case IP_PERS_DROP:
      case IP_PERS_REPLY:
	return (cur & IP_PERS_DATA);
	break;
      default:
	DEBUGP("*** found illegal instruction value\n");
	break;
      }
      ip++;
      break;
    default:
      /* illegal instruction */
      DEBUGP("*** found illegal instruction\n");
      return IP_PERS_FAILED;
    }
  }

  if (ip != eoi) {
    /* program did not end gracefully ! */
    DEBUGP("PERS:   VM: program execution failed\n");
    return IP_PERS_FAILED;
  }
  
  return IP_PERS_ACCEPT;
}

