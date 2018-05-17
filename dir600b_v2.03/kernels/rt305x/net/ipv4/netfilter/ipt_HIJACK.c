/*
 * This is a module which is used for hijack HTTP request packets.
 *
 * Copyright (C) 2009 by Alpha Networks, Inc. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/kernel.h>
#include <linux/ip.h>
#include <linux/if_ether.h>
#include <net/ip.h>
#include <net/tcp.h>
#include <net/route.h>
#include <net/dst.h>
#include <linux/netfilter/x_tables.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netfilter_ipv4/ipt_HIJACK.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kwest Wan <kwest_wan@cn.alphanetworks.com>");
MODULE_DESCRIPTION("iptables HTTP hijack target module");

#if 0
#define DEBUGP printk
#else
#define DEBUGP(format, args...)
#endif

#define MAX_PAYLOAD_LEN	512

#define MAX_URI_LEN		256	/* RFC 2616:  ... URI lengths above 255 bytes ...*/
#define MAX_LINE_LEN		MAX_URI_LEN*2

static char  *fmt = \
"HTTP/1.1 302 Found\r\n"
"Content-Type: text/html\r\n"
"Location: http://%s?url=%s\r\n\r\n";

static char * get_one_line(char *string, int len, char * buffer, int bufflen)
{
	int i;

	for (i=0; i<len && i<bufflen; i++)
	{
		if (string[i] == '\n' || string[i] == '\r')	break;
		else		buffer[i] = string[i];
	}
	if(i >= bufflen)
		buffer[i-1] = '\0';
	else
		buffer[i] = '\0';
	while (i < len && (string[i]=='\r' || string[i]=='\n')) i++;
	return &string[i];
}
static int match_http_method(char *haystack)
{
	/* RFC 2616 - 5.1.1 Method */
	if(strstr(haystack, "GET")!=0 || strstr(haystack, "POST")!=0 || strstr(haystack, "HEAD")!=0
	    || strstr(haystack,"PUT")!=0 || strstr(haystack,"DELETE")!=0 || strstr(haystack, "OPTIONS")!=0
	    || strstr(haystack,"TRACE")!=0 ||strstr(haystack,"CONNECT")!=0)
	{
		return 1;
	}
	return 0;
}
static int match_http_version(char *haystack)
{
	char *version;
	version = strstr(haystack, "HTTP/");
	if(version!=NULL)
	{
		if ((version[5] > '0' && version[5] < '9') && version[6] == '.'
			&& (version[7] > '0' && version[7] < '9') )
			return 1;
	}
	return 0;
}

/*
 * skb_get_url - Get the URI from the HTTP packet
 * @ih: the pointer to IP header
 * @th: the pointer to TCP header 
 * @url_path: the pointer to store the returned URL string
 * @size: the max length of url_path buffer 
 *
 * return value - 1: get success  0: get failed
 *
 * NOTE: Kwest add to support Internet Usage Meter, also you can use it for other purpose :)
 */
static int skb_get_url(struct iphdr *ih, struct tcphdr *th, char *url_path, size_t size)
{
		int datalen;
		char *data, *start, *end;
		char line[MAX_LINE_LEN];  
		char relative[MAX_URI_LEN];
		int line_len;
		char *p_uri_start;
		char *p_uri_end;
		char *absoluteURI;
		char *abs_path;
		char *host;
		
		datalen = ntohs(ih->tot_len) - ih->ihl * 4 - th->doff * 4;
		if (datalen > 0 && (ntohs(th->dest) == 80 || ntohs(th->dest) == 8080))
		{
			data = (char *)th + th->doff * 4;
			start = data;
			/* 16 characters: "GET / HTTP/1.1\r\n" */
			if (datalen > 16) 
			{ 
				memset(line, 0, MAX_LINE_LEN);
				end = get_one_line(start, datalen, line, MAX_LINE_LEN);	
				line_len = end - start;
				datalen -= line_len;
				start = end;
				if (match_http_method(line) && match_http_version(line))
				{
					abs_path = strstr(line, " /");
					if (abs_path == NULL) /*proxy http*/
					{
						absoluteURI = strstr(line, "http://");
						if (absoluteURI != NULL)
						{
							absoluteURI += 7;
							p_uri_end = strchr(absoluteURI, ' ');
							if (p_uri_end != NULL) 
							{
								*p_uri_end = '\0';
								printk("HTTP=%s\n", absoluteURI);
								strncpy(url_path, absoluteURI, size);
								return 1;
							}
						}
						else  /* don't have "http://" prefix */
						{
							p_uri_start = strchr(line, ' ');
							if (p_uri_start != NULL)
							{
								p_uri_start++;
								p_uri_end = strchr(p_uri_start, ' ');
								if (p_uri_end != NULL)
								{
									*p_uri_end = '\0';
									printk("HTTP=%s\n", p_uri_start);
									strncpy(url_path, p_uri_start, size);
									return 1;
								}
							}
						}
					}
					else
					{
						abs_path++; //ignore first blank
						p_uri_end = strchr(abs_path, ' ');
						if (p_uri_end != NULL) *p_uri_end = '\0';
						memset(relative, 0, MAX_URI_LEN);
						strncpy(relative, abs_path, MAX_URI_LEN);

						while (datalen > 6) /* 6 characters: "Host: " */
						{
							memset(line, 0, MAX_LINE_LEN);
							end = get_one_line(start, datalen, line, MAX_LINE_LEN);
							line_len = end - start;
							datalen -= line_len;
							start = end;
							
							host = strstr(line, "Host: ");
							if (host != NULL)
							{
								host += 6;
								printk("HTTP=%s%s\n", host, relative);
								snprintf(url_path, size, "%s%s", host, relative);
								return 1;
							}
						}
					}
				}

			}
		}

		return 0;
}

/* Send FIN reply */
static void send_fin(struct sk_buff *oldskb, int hook, struct ipt_hijack_info *info)
{
	struct sk_buff *nskb;
	struct iphdr *iph = oldskb->nh.iph;
	struct tcphdr _otcph, *oth, *tcph;
	__be16 tmp_port;
	__be32 tmp_addr;
	unsigned int addr_type, data_len, dlen, odlen;
	char data[MAX_PAYLOAD_LEN];
	char req_url[MAX_URI_LEN];

	/* IP header checks: fragment. */
	if (oldskb->nh.iph->frag_off & htons(IP_OFFSET))
		return;

	oth = skb_header_pointer(oldskb, oldskb->nh.iph->ihl * 4,
				 sizeof(_otcph), &_otcph);
	if (oth == NULL)
		return;

	/* Check checksum */
	if (nf_ip_checksum(oldskb, hook, iph->ihl * 4, IPPROTO_TCP))
		return;

	/* We need a linear, writeable skb.  We also need to expand
	   headroom in case hh_len of incoming interface < hh_len of
	   outgoing interface */
	if(skb_get_url(iph, oth, req_url, sizeof(req_url)))
		snprintf(data, sizeof(data), fmt, info->url, req_url);
	else
		snprintf(data, sizeof(data), fmt, info->url, "notfound");
	data_len = strlen(data);
	nskb = skb_copy_expand(oldskb, LL_MAX_HEADER, data_len,
			       			   GFP_ATOMIC);
	if (!nskb) {
		if (net_ratelimit())
			printk(KERN_ERR "ipt_HIJACK: unable to allocate larger skb!\n");
		return;
	}

	/* This packet will not be the same as the other: clear nf fields */
	nf_reset(nskb);
	nskb->mark = 0;
	skb_init_secmark(nskb);

	skb_shinfo(nskb)->gso_size = 0;
	skb_shinfo(nskb)->gso_segs = 0;
	skb_shinfo(nskb)->gso_type = 0;

	tcph = (struct tcphdr *)((u_int32_t*)nskb->nh.iph + nskb->nh.iph->ihl);

	/* Swap source and dest */
	tmp_addr = nskb->nh.iph->saddr;
	nskb->nh.iph->saddr = nskb->nh.iph->daddr;
	nskb->nh.iph->daddr = tmp_addr;
	tmp_port = tcph->source;
	tcph->source = tcph->dest;
	tcph->dest = tmp_port;

	/* Truncate to length (no data) */
	tcph->doff = sizeof(struct tcphdr)/4;
	skb_trim(nskb, nskb->nh.iph->ihl*4 + sizeof(struct tcphdr));

	/* Add payload length */
	memcpy(skb_put(nskb, data_len), data, data_len);
	nskb->nh.iph->tot_len = htons(nskb->len);
	
	if (tcph->ack) {
		odlen = oldskb->len - oldskb->nh.iph->ihl*4 - (oth->doff<<2); //http request data len
		tcph->seq = oth->ack_seq;
		tcph->ack_seq = htonl(ntohl(oth->seq) + odlen);
	}
	else
		goto free_nskb;

	/* Reset flags */
	((u_int8_t *)tcph)[13] = 0;
	tcph->ack = 1;
	tcph->psh = 1;
	tcph->fin = 1;   

	tcph->window = htons(0xFFFF - odlen);
	tcph->urg_ptr = 0;

	/* Adjust TCP checksum */
	dlen = nskb->len - nskb->nh.iph->ihl*4;
	tcph->check = 0;
	tcph->check = tcp_v4_check(dlen,
				   nskb->nh.iph->saddr,
				   nskb->nh.iph->daddr,
				   csum_partial((char *)tcph,
								dlen, 0));

	/* Set DF, id = 0 */
	nskb->nh.iph->frag_off = htons(IP_DF);
	nskb->nh.iph->id = (unsigned short)net_random();

	addr_type = RTN_UNSPEC;

	if (ip_route_me_harder(&nskb, addr_type))
		goto free_nskb;

	nskb->ip_summed = CHECKSUM_NONE;

	/* Adjust IP TTL */
	nskb->nh.iph->ttl = dst_metric(nskb->dst, RTAX_HOPLIMIT);

	/* Adjust IP checksum */
	nskb->nh.iph->check = 0;
	nskb->nh.iph->check = ip_fast_csum((unsigned char *)nskb->nh.iph,
					   nskb->nh.iph->ihl);

	/* "Never happens" */
	if (nskb->len > dst_mtu(nskb->dst))
		goto free_nskb;

	//Don't do attach, it should be done by OUTPUT chain of netfilter.
	//nf_ct_attach(nskb, oldskb);

	NF_HOOK(PF_INET, NF_IP_LOCAL_OUT, nskb, NULL, nskb->dst->dev,
		dst_output);
	return;

 free_nskb:
	kfree_skb(nskb);
}

/* Send RST reply to server */
static void send_reset(struct sk_buff *oldskb, int hook, struct ipt_hijack_info *info)
{
	struct sk_buff *nskb;
	struct iphdr *iph = oldskb->nh.iph;
	struct tcphdr _otcph, *oth, *tcph;
	unsigned int addr_type;

	/* IP header checks: fragment. */
	if (oldskb->nh.iph->frag_off & htons(IP_OFFSET))
		return;

	oth = skb_header_pointer(oldskb, oldskb->nh.iph->ihl * 4,
				 sizeof(_otcph), &_otcph);
	if (oth == NULL)
		return;

	/* Check checksum */
	if (nf_ip_checksum(oldskb, hook, iph->ihl * 4, IPPROTO_TCP))
		return;

	/* We need a linear, writeable skb.  We also need to expand
	   headroom in case hh_len of incoming interface < hh_len of
	   outgoing interface */
	nskb = skb_copy_expand(oldskb, LL_MAX_HEADER, skb_tailroom(oldskb),
			       GFP_ATOMIC);
	if (!nskb)
		return;

	/* This packet will not be the same as the other: clear nf fields */
	nf_reset(nskb);
	nskb->mark = 0;
	skb_init_secmark(nskb);

	skb_shinfo(nskb)->gso_size = 0;
	skb_shinfo(nskb)->gso_segs = 0;
	skb_shinfo(nskb)->gso_type = 0;

	tcph = (struct tcphdr *)((u_int32_t*)nskb->nh.iph + nskb->nh.iph->ihl);

	/* Truncate to length (no data) */
	tcph->doff = sizeof(struct tcphdr)/4;
	skb_trim(nskb, nskb->nh.iph->ihl*4 + sizeof(struct tcphdr));
	nskb->nh.iph->tot_len = htons(nskb->len);

	/* Fill seq and ack number */
	tcph->seq = oth->seq;
	tcph->ack_seq = 0;
	
	/* Reset flags */
	((u_int8_t *)tcph)[13] = 0;
	tcph->rst = 1;

	tcph->window = 0;
	tcph->urg_ptr = 0;

	/* Adjust TCP checksum */
	tcph->check = 0;
	tcph->check = tcp_v4_check(sizeof(struct tcphdr),
				   nskb->nh.iph->saddr,
				   nskb->nh.iph->daddr,
				   csum_partial((char *)tcph,
						sizeof(struct tcphdr), 0));

	/* Set DF, id = 0 */
	nskb->nh.iph->frag_off = htons(IP_DF);
	nskb->nh.iph->id = (unsigned short)net_random();;

	addr_type = RTN_UNSPEC;

	if (ip_route_me_harder(&nskb, addr_type))
		goto free_nskb;

	nskb->ip_summed = CHECKSUM_NONE;

	/* Adjust IP TTL */
	nskb->nh.iph->ttl = dst_metric(nskb->dst, RTAX_HOPLIMIT);

	/* Adjust IP checksum */
	nskb->nh.iph->check = 0;
	nskb->nh.iph->check = ip_fast_csum((unsigned char *)nskb->nh.iph,
					   nskb->nh.iph->ihl);

	/* "Never happens" */
	if (nskb->len > dst_mtu(nskb->dst))
		goto free_nskb;

	//Don't do attach, it should be done by OUTPUT chain of netfilter.
	//nf_ct_attach(nskb, oldskb);

	NF_HOOK(PF_INET, NF_IP_LOCAL_OUT, nskb, NULL, nskb->dst->dev,
		dst_output);
	return;

 free_nskb:
	kfree_skb(nskb);
}
static unsigned int target(struct sk_buff **pskb,
			   const struct net_device *in,
			   const struct net_device *out,
			   unsigned int hooknum,
			   const struct xt_target *target,
			   const void *targinfo)
{
	struct ipt_hijack_info *hijack = (struct ipt_hijack_info *)targinfo;

	/* Send HTTP 302 packet to client */
	send_fin(*pskb, hooknum, hijack);

	/* Send RST packet to server */
	send_reset(*pskb, hooknum, hijack);

	return NF_DROP;
}

static int check(const char *tablename,
		 const void *e_void,
		 const struct xt_target *target,
		 void *targinfo,
		 unsigned int hook_mask)
{
	const struct ipt_entry *e = e_void;
	
	if (e->ip.proto != IPPROTO_TCP || (e->ip.invflags & XT_INV_PROTO)) {
		printk(KERN_WARNING "ipt_HIJACK: cannot use this target for non-tcp session\n");
		return 0;
	}
	return 1;
}

static struct xt_target ipt_hijack_reg = {
	.name		= "HIJACK",
	.family		= AF_INET,
	.target		= target,
	.targetsize	= sizeof(struct ipt_hijack_info),
	.table		= "filter",
	.hooks		= (1 << NF_IP_FORWARD),
	.checkentry	= check,
	.me		= THIS_MODULE,
};

static int __init ipt_hijack_init(void)
{
	return xt_register_target(&ipt_hijack_reg);
}

static void __exit ipt_hijack_fini(void)
{
	xt_unregister_target(&ipt_hijack_reg);
}

module_init(ipt_hijack_init);
module_exit(ipt_hijack_fini);
