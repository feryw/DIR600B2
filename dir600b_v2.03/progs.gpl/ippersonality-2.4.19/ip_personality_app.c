/*
 * IP Personality
 *   ip_personality_app.c - Application(http) rewritting, by Hoo Hu
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
 * $Id: ip_personality_win.c,v 1.2 2007/10/31 06:07:08 hoo Exp $
 *
 */

#include <linux/skbuff.h>
#include <net/ip.h>
#include <net/tcp.h>
#include <net/route.h>
#include <net/checksum.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netfilter_ipv4/ip_personality.h>

#define IPSKB_PERS  8

#ifdef IP_PERS_DEBUG
#define DEBUGP if (ip_pers_debug & IP_PERS_DEBUG_HTTP) printk
#else
#define DEBUGP(format, args...)
#endif

unsigned int pers_http_hold(struct iphdr *iph, struct tcphdr *th,
				 struct ip_pers_conn_info *pci,
				 struct ip_pers *pers,
				 struct sk_buff **pskb)
{
    struct sk_buff *skb = NULL;
    int datalen, find = 0, split=0, cookie_find=0, ii;
    char *urldata = NULL, *url_point;
    struct sk_buff *pack1 = NULL;
    struct sk_buff *pack2 = NULL;
	struct iphdr *iph1 = NULL;
	struct iphdr *iph2 = NULL;
	struct tcphdr *th1 = NULL;
	struct tcphdr *th2 = NULL;
	int ret;

    skb = *pskb;

    if( IPCB(skb)->flags & IPSKB_PERS) {
        DEBUGP("PERS: packet has been done by %s()\n", __FUNCTION__);
        return IP_PERS_OK;	
    }
    	
    datalen = ntohs(iph->tot_len) - iph->ihl*4 - th->doff*4;
    if(datalen < 6){
        DEBUGP("PERS: %s() packet is too short!\n", __FUNCTION__);
        return IP_PERS_OK;
    }

    urldata = (char *)th + th->doff*4 ;

    switch(urldata[0]){
        case 'G':
            if(memcmp(urldata, "GET ", 4) == 0){
                //printk("its a GET\n");
                find = 1;
        }
            break;
        case 'P':
            if(memcmp(urldata, "POST ", 5) == 0){
                //printk("its a POST\n");
                find = 1;
            }
            break;
        case 'H':
            if(memcmp(urldata, "HEAD ", 5) == 0){
                //printk("its a HEAD\n");
                find = 1;
            }
            break;
    }

    if(find != 1){
        DEBUGP("PERS: %s() NOT http packet, do nothing!\n", __FUNCTION__);
        return IP_PERS_OK;
    }

    //hoo hold a monment
    for(ii=5; ii < datalen; ii++){
        url_point = urldata +ii;
        if(datalen - ii > 7){
            if(memcmp(url_point, "Cookie:",7) == 0){
                split = ii + 14;	
                //printk("cookie %d \n", split);
                cookie_find = 1;
                break;
             }
		}else{
            break;
        }
    }
    if(cookie_find == 0){
        split = datalen/2;
    }
    
    DEBUGP("PERS: find http packet, %s Cookie, data len %d, ip head len %d, tcp head len %d, split from %d!\n", 
             cookie_find? "with" : "without", datalen, iph->ihl*4, th->doff*4, split);

    if( (pack2 = skb_copy(skb, GFP_ATOMIC)) == NULL){
		DEBUGP("PERS: %s() skb of pack2 allocate failed!\n", __FUNCTION__);
		return IP_PERS_OK;
    }

    //deal with pack2
    iph2 = pack2->nh.iph;
    th2 = (void *)iph2 + iph2->ihl*4;
    
    skb_trim(pack2, (datalen + iph2->ihl*4 + th2->doff*4 - split));
    skb_copy_bits(skb, iph2->ihl*4+th2->doff*4+split, pack2->data+iph2->ihl*4+th2->doff*4, datalen - split);
    
    //fix ip and tcp header
    iph2->tot_len = htons((datalen - split + iph2->ihl*4 + th2->doff*4));
    th2->seq = htonl(ntohl(th2->seq) + split);
    
    pack2->csum = csum_partial((char *)th2 + th2->doff*4, datalen - split, 0);
    th2->check = 0;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
    th2->check = tcp_v4_check((datalen- split + th2->doff*4), iph2->saddr, iph2->daddr, 
			        csum_partial((char *)th2, th2->doff*4, pack2->csum));
#else
    th2->check = tcp_v4_check(th2, (datalen- split + th2->doff*4), iph2->saddr, iph2->daddr, 
			        csum_partial((char *)th2, th2->doff*4, pack2->csum));
#endif
    ip_send_check(iph2);

    IPCB(pack2)->flags |= IPSKB_PERS;

    DEBUGP("PERS: pack2 %p, total len of IP is %d, checksum of IP is %04x, seq of TCP is %08x, checksum of TCP is 0x%04x\n", 
                  pack2, ntohs(iph2->tot_len), ntohs(iph2->check), ntohl(th2->seq), ntohs(th2->check));

    //deal with pack1
    pack1 = skb;

    iph1 = pack1->nh.iph;
    th1 = (void *)iph1 + iph1->ihl*4;
   
    skb_trim(pack1, (split+iph1->ihl*4 + th1->doff*4));

    //fix ip and tcp header
    iph1->tot_len = htons((split + iph1->ihl*4 + th1->doff*4));

    pack1->csum = csum_partial((char *)th1 + th1->doff*4, split, 0);
    th1->check = 0;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
    th1->check = tcp_v4_check((split+th1->doff*4), iph1->saddr,iph1->daddr,
			        csum_partial((char *)th1, th1->doff*4, pack1->csum));
#else
    th1->check = tcp_v4_check(th1, (split+th1->doff*4), iph1->saddr,iph1->daddr,
			        csum_partial((char *)th1, th1->doff*4, pack1->csum));
#endif
    ip_send_check(iph1);

    
    DEBUGP("PERS: pack1 %p, total len of IP is %d, checksum of IP is %04x, seq of TCP is %08x, checksum of TCP is 0x%04x\n", 
                  pack1, ntohs(iph1->tot_len), ntohs(iph1->check), ntohl(th1->seq), ntohs(th1->check));

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
    ret = ip_output(pack2);
#else
    ret = ip_finish_output(pack2);
#endif
    DEBUGP("PERS: first send out pack2, ret = %d\n", ret);
    
    return IP_PERS_OK;

}

