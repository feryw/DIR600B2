/* String matching match for iptables
 *
 * (C) 2005 Pablo Neira Ayuso <pablo@eurodev.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/skbuff.h>
#include <linux/netfilter/x_tables.h>
#include <linux/netfilter/xt_string.h>
#include <linux/textsearch.h>
#ifdef CONFIG_NETFILTER_XT_MATCH_STRING_ENHANCE
#include <linux/tcp.h>
#include <linux/udp.h>
#include <net/ip.h>
#include <linux/netfilter_ipv4/ipt_string.h>
#endif
MODULE_AUTHOR("Pablo Neira Ayuso <pablo@eurodev.net>");
MODULE_DESCRIPTION("IP tables string match module");
MODULE_LICENSE("GPL");
MODULE_ALIAS("ipt_string");
MODULE_ALIAS("ip6t_string");
#ifdef CONFIG_NETFILTER_XT_MATCH_STRING_DEBUG
#define dprintk(x, args...)     printk(x, ##args)
#else
#define dprintk(x, args...)     do{}while(0)
#endif

#ifdef CONFIG_NETFILTER_XT_MATCH_STRING_ENHANCE

struct dnshdr
{
	__u16   id;
#if defined(__BIG_ENDIAN_BITFIELD)
	__u16   qr:1,
			opcode:4,
			aa:1,
			tc:1,
			rd:1,
			ra:1,
			reserced:3,
			rcode:4;
#elif defined(__LITTLE_ENDIAN_BITFIELD)
	__u16	rd:1,
			tc:1,
			aa:1,
			opcode:4,
			qr:1,
			rcode:4,
			reserced:3,
			ra:1;
#endif
	__u16   qdcount;
	__u16   ancount;
	__u16   nscount;
	__u16   arcount;
};
static int match_http_method(char *haystack, int hlen)
{
	if(strstr(haystack, "get")!=0 || strstr(haystack, "post")!=0 || strstr(haystack, "head")!=0
			|| strstr(haystack,"put")!=0 || strstr(haystack,"delete")!=0 || strstr(haystack, "options")!=0
			|| strstr(haystack,"trace")!=0 ||strstr(haystack,"connect")!=0)
	{
		dprintk("match_http_method match!\n");
		return 1;
	}
	return 0;
}
static char * get_one_query(char *string, int len, char * buffer, int bufflen)
{
	int i;
	
	for (i=0; i<len && i<bufflen; i++)
	{
		if (string[i] == '\0')	break;
		else					buffer[i] = string[i];
	}
	buffer[i]='\0';
	dprintk("get_one_query = %s\n", string);
	
	while (i < len && (string[i]=='\0')) i++;
	return &string[i];
}
static char * get_one_line(char *string, int len, char * buffer, int bufflen)
{
	int i;

	for (i=0; i<len && i<bufflen; i++)
	{
		if (string[i] == '\n' || string[i] == '\r')     break;
		else                                            buffer[i] = string[i];
	}
#if 0 /* Modified by Kwest Wan 20080909 */
	buffer[i]='\0';		/* When i=bufflen, this line will cause overflow. */
#else
	if(i >= bufflen){
		buffer[i-1] = '\0';
	}
	else
		buffer[i] = '\0';
#endif
	while (i < len && (string[i]=='\r' || string[i]=='\n')) i++;
	return &string[i];
}
static int match_HTTP_REQ(char **haystack, int hlen)
{
	char *pend=(*haystack);
	char *pstart=(*haystack);//ponit to the http head
	char line[256];
	int line_len            =256;
	int data_len            = hlen;
	int data_line_len;
	int i;

	dprintk("match HTTP REQ -------->\n");
	dprintk("data_len=%d\n\n", data_len);
	if(data_len>16) //16 characters: "GET / HTTP/1.1\r\n"
	{
		memset(line,0,line_len);
		pend = get_one_line(pend, data_len, line, line_len);
		data_line_len = pend - pstart;
		data_len -= data_line_len;
		pstart = pend;
		dprintk("data_line_len=%d; data_len=%d\n", data_line_len,data_len);

		if(data_line_len > 12)  //12 characters: "GET HTTP/1.1"
		{
			for(i=0;i<line_len;i++) { if(line[i]>='A' && line[i]<='Z')line[i]+=32;  }

			return match_http_method(line, line_len);
			/*if(match_http_method(line, line_len))
			  {
				dprintk("found!\n");
				dprintk("match HTTP REQ <--------\n");
				return 1;
			  }
			*/
		}
	}
	dprintk("match HTTP REQ <--------\n");
	return 0;
}
static int match_DNS(char **queries, int qlen, int qcount, char *needle, int nlen)
{
	char	*pend		=(*queries);
	char	*pstart		=(*queries);
	char	buff[256];
	int		buff_len	= 256;
	int		cmp_qlen	= 0;
	int		i;
	
	unsigned int j;
	unsigned char * data;
	
	dprintk("match DNS -------->\n");
	for(i=0; i<qcount; i++)
	{
		qlen -= cmp_qlen;
		pend = get_one_query(pend, qlen, buff, buff_len);
		cmp_qlen = pend - pstart;
		
		dprintk("qlen=%d, pstart=%s, cmp_qlen=%d, \nneedle=%s, nlen=%d,\n pend-cmp_qlen=%s\n",
		qlen, pstart, cmp_qlen, needle, nlen, (char*)(pend-nlen-2));
		dprintk("cmp_qlen >= nlen %s \n", (cmp_qlen >= nlen?"TRUE":"FALSE"));
		dprintk("stncmp=%d\n", strncmp((char*)(pend-nlen-2), needle, nlen));
		data=(unsigned char *)(pend-nlen-2);
		dprintk("query in packet=");
		for(j=0;j<nlen;j++) dprintk("%02x ", data[j]);
		dprintk("\n");
		data=(unsigned char *)(needle);
		dprintk("needle         =");
		for(j=0;j<nlen;j++) dprintk("%02x ", data[j]);
		dprintk("\n");
		
		if (cmp_qlen >= nlen && (strncmp((char*)(pend-nlen-2), needle, nlen)==0))
		{
			dprintk("found!\n");
			dprintk("match DNS <--------\n");
			return 1;
		}
		pstart = pend;
	}
	dprintk("match DNS <--------\n");
	return 0;
}
static int match_URL(char **haystack, int hlen, char *needle, int nlen)
{
	char *pend=(*haystack);
	char *pstart=(*haystack);//ponit to the http head
	char *p_url_start;
	char url[512];
	char buff[256];
	char line[256];
	int url_len             =512;
	int buff_len            =256;
	int line_len            = 256;
	int data_len            = hlen;
	int data_line_len;
	int data_url_len        =0;
	int data_buff_len       =0;
	int i;

	dprintk("match URL -------->\n");
	if(data_len>16) //16 characters: "GET / HTTP/1.1\r\n"
	{
		dprintk("data_len=%d\n\n", data_len);
		memset(line,0,line_len);
		pend = get_one_line(pend, data_len, line, line_len);
		data_line_len = pend - pstart;
		data_len -= data_line_len;
		pstart = pend;
		if(data_line_len > 12)  //12 characters: "GET HTTP/1.1"
		{
			//dprintk("line before=%s\n", line);
			for(i=0;i<line_len;i++) { if(line[i]>='A' && line[i]<='Z')line[i]+=32;  }
			dprintk("line after =%s\n", line);
			if(match_http_method(line, line_len))
			{
				p_url_start=strstr(line," /");
				if(p_url_start!=0) p_url_start++;
				else return 0;

				memset(buff,0,buff_len);
				memset(url,0,url_len);
				i=0;
				while((p_url_start[i]!=' ') && (p_url_start[i]!='\0') && i<(buff_len-1))
				{
					buff[i]=p_url_start[i];
					i++;
				}
				buff[i]='\0';
				data_buff_len=i;
				
				dprintk("uri buff=%s, len=%d\n", buff, data_buff_len);
				if(strstr(buff, "http://")!=0)  // proxy case
				{
					for(i=0;i<data_buff_len;i++)
					{
						url[i]=buff[7+i];
					}
					url[i]='\0';
					data_url_len=i;
					dprintk("proxy case url=%s\n", url);
				}
				else
				{
					dprintk("data_len=%d\n", data_len);
					while(data_len>6)
					{
						memset(line,0,line_len);
						pend = get_one_line(pend, data_len, line, line_len);
						data_line_len = pend - pstart;
						data_len -= data_line_len;
						pstart = pend;

						for(i=0;i<line_len;i++) { if(line[i]>='A' && line[i]<='Z')line[i]+=32;  }
					
						if(strstr(line, "host: ")!=0)
						{
							dprintk("line =%s\n", line);
							p_url_start=strstr(line, "host: ");
							if(p_url_start!=0)      p_url_start+=6;
							else return 0;

							i=0;
							while((p_url_start[i]!=' ') && (p_url_start[i]!='\0'))
							{
								url[i]=p_url_start[i];
								i++;
							}
							data_url_len=i;
							for(i=0;i<data_buff_len;i++)
							{
								url[data_url_len+i]=buff[i];
							}
							url[data_url_len+i]='\0';
							data_url_len+=i-1;
							break;
						}
					}
				}
				dprintk("normal case url=%s, len=%d\n", url, data_url_len);
				dprintk("normal case needle=%s, len=%d\n", needle, nlen);
				if(data_url_len >= nlen)
				{
					for(i=0; i< nlen; i++){if(needle[i]>='A' && needle[i]<='Z') needle[i]+=32;}
					if(strstr(url, needle)!=0)
					{
						dprintk("found!\n");
						dprintk("match URL <--------\n");
						return 1;
					}
				}
			}
		}
	}
	dprintk("match URL <--------\n");
	return 0;
}

#endif

static int match(const struct sk_buff *skb,
		 const struct net_device *in,
		 const struct net_device *out,
		 const struct xt_match *match,
		 const void *matchinfo,
		 int offset,
		 unsigned int protoff,
		 int *hotdrop)
{
	const struct xt_string_info *conf = matchinfo;
	struct ts_state state;

#ifdef CONFIG_NETFILTER_XT_MATCH_STRING_ENHANCE
	/* CONFIG_IP_NF_MATCH_STRING_ENHANCE is added by joanw */
	struct iphdr *ip	= skb->nh.iph;
	struct tcphdr *tcp	= (struct tcphdr *)((char *)ip+(ip->ihl*4));
	struct udphdr *udp	= (struct udphdr *)((char *)ip+(ip->ihl*4));
	struct dnshdr *dns;
	int hlen, nlen;
	int dnshdr_size=sizeof(struct dnshdr);
	char *needle, *haystack;
	unsigned int j;
	unsigned char * data;

	if(!ip) return 0;

	/* get lengths, and validate them */
	nlen = conf->patlen;
	hlen = ntohs(ip->tot_len)-(ip->ihl*4);
	if(nlen>hlen) return 0;

	needle = (char *)&conf->pattern;
	haystack = (char *)ip+(ip->ihl*4);
	if(conf->flags == IPT_URL_STRING)
	{
		hlen = ntohs(ip->tot_len)-(ip->ihl*4)-(tcp->doff*4);
		if(hlen>0)
		{
			dprintk("URL: tot_len=%d, ip_ihl=%d, doff=%d; hlen=%d\n",
					ntohs(ip->tot_len),(ip->ihl*4), (tcp->doff*4),hlen);
			haystack=(char *)tcp+(tcp->doff*4);
			return match_URL(&haystack,hlen, needle, nlen);
		}
	}
	else if(conf->flags == IPT_HTTP_REQ)
	{
		hlen=ntohs(ip->tot_len)-(ip->ihl*4)-(tcp->doff*4);
		if(hlen>0)
		{
			dprintk("HTTP_REQ: tot_len=%d, ip_ihl=%d, doff=%d; hlen=%d\n",
					ntohs(ip->tot_len), (ip->ihl*4),(tcp->doff*4),hlen);
			haystack=(char *)tcp+(tcp->doff*4);
			return match_HTTP_REQ(&haystack,hlen);
		}
	}
	else if(conf->flags == IPT_DNS_STRING)
	{
		dprintk("IPT_DNS_STRING\n");
		if (ntohs(udp->dest)==53) /* Destination port is "domain" */
		{
			hlen=ntohs(udp->len)-sizeof(udp);
			dprintk("needle=%s, nlen=%d\n", needle, nlen);
			dprintk("hlen=%d, dnshdr+nlen=%d\n", hlen, dnshdr_size+nlen);
			if (hlen < dnshdr_size+nlen)    return 0;
			
			dns=(struct dnshdr *)((char*)ip+(ip->ihl*4)+8);
			if(ntohs(dns->opcode)==0) /* DNS op code is query. */
			{
				dprintk("DNS: tot_len=%d, ip_ihl=%d, hlen=%d, dns dport=%d, dns len=%d, qdcount=%d\n",
						ntohs(ip->tot_len), (ip->ihl*4), hlen, ntohs(udp->dest), ntohs(udp->len),ntohs(dns->qdcount));
				haystack=(char *)dns + dnshdr_size;
#if 0
				data=(unsigned char *)udp;
				dprintk("udp packet=\n\t");
				for(j=0;j<8;j++) dprintk("%02x ", data[j]);     dprintk("\n\t");
				for(;j<16+8;j++) dprintk("%02x ", data[j]);     dprintk("\n\t");
				for(;j<hlen;j++) dprintk("%02x ", data[j]);     dprintk("\n\n");
#endif
				data=(unsigned char *)haystack;
				dprintk("dns header=\n\t");
				for(j=0;j<16;j++) dprintk("%02x ", data[j]);    dprintk("\n\t");
				for(;j<hlen;j++)  dprintk("%02x ", data[j]);    dprintk("\n");
				
				return match_DNS(&haystack,hlen, ntohs(dns->qdcount), needle, nlen);
			}
		}
	}
	return 0;
#else

	memset(&state, 0, sizeof(struct ts_state));

	return (skb_find_text((struct sk_buff *)skb, conf->from_offset,
			     conf->to_offset, conf->config, &state)
			     != UINT_MAX) ^ conf->invert;
#endif
}

#define STRING_TEXT_PRIV(m) ((struct xt_string_info *) m)

static int checkentry(const char *tablename,
		      const void *ip,
		      const struct xt_match *match,
		      void *matchinfo,
		      unsigned int hook_mask)
{
	struct xt_string_info *conf = matchinfo;
	struct ts_config *ts_conf;

#ifdef CONFIG_NETFILTER_XT_MATCH_STRING_ENHANCE
	if (conf->flags == IPT_URL_STRING || conf->flags == IPT_HTTP_REQ ||
			conf->flags == IPT_DNS_STRING)
		return 1;
#endif

	/* Damn, can't handle this case properly with iptables... */
	if (conf->from_offset > conf->to_offset)
		return 0;
	if (conf->algo[XT_STRING_MAX_ALGO_NAME_SIZE - 1] != '\0')
		return 0;
	if (conf->patlen > XT_STRING_MAX_PATTERN_SIZE)
		return 0;
	ts_conf = textsearch_prepare(conf->algo, conf->pattern, conf->patlen,
				     GFP_KERNEL, TS_AUTOLOAD);
	if (IS_ERR(ts_conf))
		return 0;

	conf->config = ts_conf;

	return 1;
}

static void destroy(const struct xt_match *match, void *matchinfo)
{
#ifdef CONFIG_NETFILTER_XT_MATCH_STRING_ENHANCE
	struct ipt_string_info *conf = (struct ipt_string_info *) matchinfo;
	if (conf->flags != IPT_URL_STRING && conf->flags != IPT_HTTP_REQ &&
			conf->flags != IPT_DNS_STRING)
		textsearch_destroy(STRING_TEXT_PRIV(matchinfo)->config);
#else
	textsearch_destroy(STRING_TEXT_PRIV(matchinfo)->config);
#endif
}

static struct xt_match xt_string_match[] = {
	{
		.name 		= "string",
		.family		= AF_INET,
		.checkentry	= checkentry,
		.match 		= match,
		.destroy 	= destroy,
		.matchsize	= sizeof(struct xt_string_info),
		.me 		= THIS_MODULE
	},
	{
		.name 		= "string",
		.family		= AF_INET6,
		.checkentry	= checkentry,
		.match 		= match,
		.destroy 	= destroy,
		.matchsize	= sizeof(struct xt_string_info),
		.me 		= THIS_MODULE
	},
};

static int __init xt_string_init(void)
{
	return xt_register_matches(xt_string_match, ARRAY_SIZE(xt_string_match));
}

static void __exit xt_string_fini(void)
{
	xt_unregister_matches(xt_string_match, ARRAY_SIZE(xt_string_match));
}

module_init(xt_string_init);
module_exit(xt_string_fini);
