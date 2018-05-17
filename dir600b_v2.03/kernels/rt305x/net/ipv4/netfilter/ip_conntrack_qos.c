
#include <linux/netfilter_ipv4/ip_conntrack_qos.h>

/*ips_qos_max_len=ip_conntrack_max-IPS_QOS_HI_MAX_LEN*/
/*static*/ /*int		ips_qos_len;*/
/*static*/ /*int		ips_qos_hi_len;*/
/*static*/ int		ips_qos_max_len;
atomic_t ips_qos_len = ATOMIC_INIT(0);
atomic_t ips_qos_hi_len = ATOMIC_INIT(0);
static struct timer_list	ips_qos_timer;
static struct list_head	ips_qos_filter_list[IPS_QOS_FILTER_LIST_MAX];
static  rwlock_t			ips_qos_lock=RW_LOCK_UNLOCKED;
extern int 					ip_conntrack_max;

/* TCP filter */
static IPS_QOS_FILTER	ips_qos_tcp_ftpdata= {
	.protonum		=	IPPROTO_TCP, 
	.name			=	"ftp-data", 
	.u.tcp.port	=	20, 
};
static IPS_QOS_FILTER	ips_qos_tcp_ftpctrl= {
	.protonum		=	IPPROTO_TCP, 
	.name			=	"ftp-ctrl", 
	.u.tcp.port	=	21, 
};
static IPS_QOS_FILTER	ips_qos_tcp_ssh= {
	.protonum		=	IPPROTO_TCP, 
	.name			=	"ssh", 
	.u.tcp.port	=	22, 
};
static IPS_QOS_FILTER	ips_qos_tcp_telnet= {
	.protonum		=	IPPROTO_TCP, 
	.name			=	"telnet", 
	.u.tcp.port	=	23, 
};
static IPS_QOS_FILTER	ips_qos_tcp_smtp= {
	.protonum		=	IPPROTO_TCP, 
	.name			=	"smtp", 
	.u.tcp.port	=	25, 
};
static IPS_QOS_FILTER	ips_qos_tcp_pop3= {
	.protonum		=	IPPROTO_TCP, 
	.name			=	"pop3", 
	.u.tcp.port	=	110, 
};
static IPS_QOS_FILTER	ips_qos_tcp_http1= {
	.protonum		=	IPPROTO_TCP, 
	.name			=	"http", 
	.u.tcp.port	=	80, 
};
static IPS_QOS_FILTER	ips_qos_tcp_https= {
	.protonum		=	IPPROTO_TCP, 
	.name			=	"https", 
	.u.tcp.port	=	443, 
};
static IPS_QOS_FILTER	ips_qos_tcp_msn= {
	.protonum		=	IPPROTO_TCP, 
	.name			=	"msn", 
	.u.tcp.port	=	1863, 
};
static IPS_QOS_FILTER	ips_qos_tcp_http2= {
	.protonum		=	IPPROTO_TCP, 
	.name			=	"http", 
	.u.tcp.port	=	8080, 
};
/* UDP filter */
static IPS_QOS_FILTER	ips_qos_udp_dns= {
	.protonum		=	IPPROTO_UDP, 
	.name			=	"dns", 
	.u.udp.port	=	53, 
};
static IPS_QOS_FILTER	ips_qos_udp_dhcp= {
	.protonum		=	IPPROTO_UDP, 
	.name			=	"dhcp", 
	.u.udp.port	=	67, 
};
static IPS_QOS_FILTER	ips_qos_udp_dhcpr= {
	.protonum		=	IPPROTO_UDP, 
	.name			=	"dhcp-reply", 
	.u.udp.port	=	68, 
};
/* ICMP filter */
static IPS_QOS_FILTER	ips_qos_icmp_echo= {
	.protonum		=	IPPROTO_ICMP, 
	.name			=	"echo", 
	.u.icmp.id	=	8, //??
};
static IPS_QOS_FILTER	ips_qos_icmp_reply= {
	.protonum		=	IPPROTO_ICMP, 
	.name			=	"reply", 
	.u.icmp.id	=	0, //??
};

static void	ips_qos_tune_maxlen(unsigned long data)
{
	if(ips_qos_max_len+IPS_QOS_HI_MAX_LEN!=ip_conntrack_max)
		ips_qos_max_len=(ip_conntrack_max-IPS_QOS_HI_MAX_LEN);
	IPS_QOSDBG("*************************************\n");
	IPS_QOSDBG("ips_qos_max_len:%d VS IPS_QOS_HI_MAX_LEN:%d\n"
		, ips_qos_max_len, IPS_QOS_HI_MAX_LEN);
	IPS_QOSDBG("*************************************\n");
}

void	ips_qos_initial(void)
{
	int		i;
	
	//ips_qos_len=0;		//ips_qos_len=ips_qos_low_len+ips_qos_mid_len
	//ips_qos_hi_len=0;
	ips_qos_max_len=(ip_conntrack_max-IPS_QOS_HI_MAX_LEN);
	IPS_QOSDBG("*************************************\n");
	IPS_QOSDBG("ips_qos_max_len:%d VS IPS_QOS_HI_MAX_LEN:%d\n"
		, ips_qos_max_len, IPS_QOS_HI_MAX_LEN);
	IPS_QOSDBG("*************************************\n");
	
	/* Re-check ips_qos_max_len in case of re-assignment in /proc */
	init_timer(&ips_qos_timer);
	ips_qos_timer.expires=IPS_QOS_DELAY+jiffies;
	ips_qos_timer.data=0;
	ips_qos_timer.function=ips_qos_tune_maxlen;
	add_timer(&ips_qos_timer);
	
	for(i=0; i<IPS_QOS_FILTER_LIST_MAX; i++) {
		INIT_LIST_HEAD(&ips_qos_filter_list[i]);
	}
	/* Register TCP filter */
	list_add(&ips_qos_tcp_ftpdata.list, &ips_qos_filter_list[IPS_QOS_FILTER_TCP]);
	list_add(&ips_qos_tcp_ftpctrl.list, &ips_qos_filter_list[IPS_QOS_FILTER_TCP]);
	list_add(&ips_qos_tcp_ssh.list, &ips_qos_filter_list[IPS_QOS_FILTER_TCP]);
	list_add(&ips_qos_tcp_telnet.list, &ips_qos_filter_list[IPS_QOS_FILTER_TCP]);
	list_add(&ips_qos_tcp_smtp.list, &ips_qos_filter_list[IPS_QOS_FILTER_TCP]);
	list_add(&ips_qos_tcp_pop3.list, &ips_qos_filter_list[IPS_QOS_FILTER_TCP]);
	list_add(&ips_qos_tcp_http1.list, &ips_qos_filter_list[IPS_QOS_FILTER_TCP]);
	list_add(&ips_qos_tcp_https.list, &ips_qos_filter_list[IPS_QOS_FILTER_TCP]);
	list_add(&ips_qos_tcp_msn.list, &ips_qos_filter_list[IPS_QOS_FILTER_TCP]);
	list_add(&ips_qos_tcp_http2.list, &ips_qos_filter_list[IPS_QOS_FILTER_TCP]);
	/* Register UDP filter */
	list_add(&ips_qos_udp_dns.list, &ips_qos_filter_list[IPS_QOS_FILTER_UDP]);
	list_add(&ips_qos_udp_dhcp.list, &ips_qos_filter_list[IPS_QOS_FILTER_UDP]);
	list_add(&ips_qos_udp_dhcpr.list, &ips_qos_filter_list[IPS_QOS_FILTER_UDP]);
	/* Register ICMP filter */
	list_add(&ips_qos_icmp_echo.list, &ips_qos_filter_list[IPS_QOS_FILTER_ICMP]);
	list_add(&ips_qos_icmp_reply.list, &ips_qos_filter_list[IPS_QOS_FILTER_ICMP]);
	
#if		IPS_QOS_DEBUGMOD
	{
		IPS_QOS_FILTER	*temp;
		int					i=0;
		for(; i<IPS_QOS_FILTER_LIST_MAX; i++) {
			list_for_each_entry(temp, &ips_qos_filter_list[i], list) {
				switch(i) {
				case	IPS_QOS_FILTER_ICMP:
					IPS_QOSDBG("protonum: %d(%s)\n", temp->u.icmp.id, temp->name);
					break;
				case	IPS_QOS_FILTER_TCP:
					IPS_QOSDBG("protonum: %d(%s)\n", temp->u.tcp.port, temp->name);
					break;
				case	IPS_QOS_FILTER_UDP:
					IPS_QOSDBG("protonum: %d(%s)\n", temp->u.udp.port, temp->name);
					break;
				}
			}
		}
	}
#endif
}

#if		IPS_QOS_DEBUGMOD
static void show_ip_2_char(uint32_t	x)
{
	uint8_t	*y;
	char		z[16]={0};
	
	u32to8(&x, &y);
	printk("%d.%d.%d.%d", *(y), *(y+1), *(y+2), *(y+3));
	return;
}

static void show_ip_ct(uint32_t	sip, 
							uint16_t sp, 
							uint32_t	mip, 
							uint16_t mp, 
							uint32_t	dip, 
							uint16_t dp, 
							int type)
{
	if(type) {
		show_ip_2_char(sip);
		printk(":%d->", sp);
		show_ip_2_char(mip);
		printk(":%d->", mp);
		show_ip_2_char(dip);
		printk(":%d\n", dp);
	} else {
		show_ip_2_char(sip);
		printk("->");
		show_ip_2_char(mip);
		printk("->");
		show_ip_2_char(dip);
		printk("\n");
		
	}
}

extern struct list_head	*ip_conntrack_hash;
extern rwlock_t				ip_conntrack_lock;

/*void	show_ips_qos_hi(void)
{
	unsigned int				i=0, count=0;
	extern unsigned int		ip_conntrack_htable_size;
	
	for(; i<ip_conntrack_htable_size; i++) {
		if(list_empty(&ip_conntrack_hash[i]))
			continue;
		READ_LOCK(&ip_conntrack_lock);
		struct list_head	*pos;
		list_for_each(pos, &ip_conntrack_hash[i]) {
			struct		ip_conntrack	*ipctrack;
			int			prio, keep;
			u_int16_t	sp, mp, dp, protonum;
			uint32_t	sip, mip, dip;
			
			ipctrack=((struct ip_conntrack_tuple_hash *)pos)->ctrack;
			prio=ips_qos_get_priority(ipctrack);
			keep=ips_qos_get_keeptimeout(ipctrack);
			
			/ * Find out the reply tuple and don't print it * /
			if(DIRECTION((struct ip_conntrack_tuple_hash *)pos)==IP_CT_DIR_REPLY) {
				continue;
			}
			if(prio==IPS_QOS_HI_BIT || keep==1) {
				sip=ipctrack->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip;
				mip=ipctrack->tuplehash[IP_CT_DIR_REPLY].tuple.dst.ip;
				dip=ipctrack->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip;
				protonum=ipctrack->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum;
				
				if(protonum==IPPROTO_TCP) {
					sp=ipctrack->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u.tcp.port;
					dp=ipctrack->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.tcp.port;
					mp=ipctrack->tuplehash[IP_CT_DIR_REPLY].tuple.dst.u.tcp.port;
					printk("%d-T ", ++count);
					show_ip_ct(sip, sp, mip, mp, dip, dp, 1);
				} else if(protonum==IPPROTO_UDP) {
					sp=ipctrack->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u.udp.port;
					dp=ipctrack->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.udp.port;
					mp=ipctrack->tuplehash[IP_CT_DIR_REPLY].tuple.dst.u.udp.port;
					printk("%d-U ", ++count);
					show_ip_ct(sip, sp, mip, mp, dip, dp, 1);
				} else if(protonum==IPPROTO_ICMP) {
					printk("%d-I ", ++count);
					show_ip_ct(sip, 0, mip, 0, dip, 0, 0);
				} else {
					printk("%d-X ", ++count);
					show_ip_ct(sip, 0, mip, 0, dip, 0, 0);
				}
			}
		}
		READ_UNLOCK(&ip_conntrack_lock);
	}		
}*/
#endif

/* flag: 1/IPS_QOS_INC/IPS_QOS_HI_INC
 *       0/IPS_QOS_DEC/IPS_QOS_HI_DEC
 */
void	ips_qos_alter_len(const int prio, 
							const int keep, 
							const int flag)
{
	if(keep!=0 && keep!=1)
		IPS_QOS_ERROR;
	if(flag) {		/* ADD */
		if(keep) {
			IPS_QOS_HI_INC;
			goto	END_ALTER_LEN;
		}
		if(prio==IPS_QOS_HI_BIT) {
			IPS_QOS_HI_INC;
		} else {
			IPS_QOS_INC;
		}
	} else {			/* DEL */
		if(keep) {
			IPS_QOS_HI_DEC;
			goto	END_ALTER_LEN;
		}
		if(prio==IPS_QOS_HI_BIT) {
			IPS_QOS_HI_DEC;
		} else {
			IPS_QOS_DEC;
		}
	}
END_ALTER_LEN:
	IPS_QOSDBG("All(%d) hi:%d vs nr:%d\n"
		, atomic_read(&ip_conntrack_amount_atomic), atomic_read(&ips_qos_hi_len), atomic_read(&ips_qos_len));
	return;
}

void	ips_qos_tune_len(const int old_prio, 
							const int new_prio, 
							const int keep)
{
	if(keep==1) {
		return;
	}/* else if(keep!=0 && keep!=1)
		IPS_QOS_ERROR;*/
	/*if((old_prio>IPS_QOS_LOW_BIT || old_prio<IPS_QOS_HI_BIT) 
		|| (new_prio>IPS_QOS_LOW_BIT || new_prio<IPS_QOS_HI_BIT))
		IPS_QOS_ERROR;
	else */if((old_prio==IPS_QOS_LOW_BIT || old_prio==IPS_QOS_MID_BIT) 
		&& (new_prio==IPS_QOS_LOW_BIT || new_prio==IPS_QOS_MID_BIT))
		return;
	else if(old_prio!=new_prio) {
		if(old_prio==IPS_QOS_HI_BIT) {
			IPS_QOS_HI_DEC;
			IPS_QOS_INC;
		} else {
			IPS_QOS_DEC;
			IPS_QOS_HI_INC;
		}
	}
}

int		ips_qos_get_priority(struct ip_conntrack *ct)
{
	int		retval=0;
	IPS_QOS_TEST_BIT_BGN;
	if(	IPS_QOS_TEST_BIT(IPS_QOS_LOW_BIT
		, &ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.ips_qos_type))
		retval=IPS_QOS_LOW_BIT;
	else if(IPS_QOS_TEST_BIT(IPS_QOS_MID_BIT
		, &ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.ips_qos_type))
		retval=IPS_QOS_MID_BIT;
	else if(IPS_QOS_TEST_BIT(IPS_QOS_HI_BIT
		, &ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.ips_qos_type))
		retval=IPS_QOS_HI_BIT;
#if 1 //kwest modified to fix warning bug
	else if(IPS_QOS_TEST_BIT(IPS_QOS_KEEP_BIT
		, &ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.ips_qos_type))
		retval=IPS_QOS_KEEP_BIT;
#endif
	else
		IPS_QOS_ERROR;
	IPS_QOS_TEST_BIT_FIN;
	return		retval;
}

int		ips_qos_get_keeptimeout(struct ip_conntrack *ct)
{
	int		retval=-1;
	
	IPS_QOS_TEST_BIT_BGN;
	retval=IPS_QOS_TEST_BIT(IPS_QOS_KEEP_BIT
		, &ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.ips_qos_type)?1:0;
	IPS_QOS_TEST_BIT_FIN;
	
	return		retval;
}

void	u32to8(uint32_t *a, 
				uint8_t **b)
{
	*b=(uint8_t *)a;
	return;
}

void	ips_qos_modify_qostype_2_ct(const int prio, 
										const int keep, 
										struct ip_conntrack *ct)
{
	IPS_QOS_TEST_BIT_BGN;
	if(	IPS_QOS_TEST_BIT(prio, &ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.ips_qos_type)) {
		IPS_QOS_TEST_BIT_FIN;
		return;
	}
	IPS_QOS_TEST_BIT_FIN;
	
	IPS_QOS_WRT_BIT_BGN;
	/*clear_bit(IPS_QOS_LOW_BIT, &ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.ips_qos_type);
	clear_bit(IPS_QOS_MID_BIT, &ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.ips_qos_type);
	clear_bit(IPS_QOS_HI_BIT, &ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.ips_qos_type);
	clear_bit(IPS_QOS_KEEP_BIT, &ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.ips_qos_type);*/
	ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.ips_qos_type=0;
	IPS_QOS_WRT_BIT_FIN;
	
	if(prio<=IPS_QOS_LOW_BIT && prio>=IPS_QOS_HI_BIT) {
		IPS_QOS_SET_BIT(prio, &ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.ips_qos_type);
	} else {
		IPS_QOS_ERROR;
	}
	if(keep==1) {
		IPS_QOS_SET_BIT(IPS_QOS_KEEP_BIT
			, &ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.ips_qos_type);
	} else if(keep!=0 && keep!=1) {
		IPS_QOS_ERROR;
	}
#if		IPS_QOS_DEBUGMOD
	if(keep==1 || prio==IPS_QOS_HI_BIT) {
		uint32_t	sip, dip;
		uint8_t	*psip_8, *pdip_8;
		int			nprio;
		sip=ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip;
		dip=ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip;
		
		IPS_QOS_TEST_BIT_BGN;
		if(IPS_QOS_TEST_BIT(IPS_QOS_LOW_BIT
			, &ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.ips_qos_type)) {
			nprio=IPS_QOS_LOW_BIT;
		} else if(IPS_QOS_TEST_BIT(IPS_QOS_MID_BIT
			, &ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.ips_qos_type)) {
			nprio=IPS_QOS_MID_BIT;
		} else if(IPS_QOS_TEST_BIT(IPS_QOS_HI_BIT
			, &ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.ips_qos_type)) {
			nprio=IPS_QOS_HI_BIT;
		} else {
			IPS_QOS_ERROR;
		}
		IPS_QOS_TEST_BIT_FIN;
		
		u32to8(&sip, &psip_8);
		u32to8(&dip, &pdip_8);
		IPS_QOSDBG("(+%d)%d.%d.%d.%d -> %d.%d.%d.%d\n"
			, nprio
			, *(psip_8+0), *(psip_8+1), *(psip_8+2), *(psip_8+3)
			, *(pdip_8+0), *(pdip_8+1), *(pdip_8+2), *(pdip_8+3));
	}
#endif
	return;
}

int		ips_qos_check_entry(struct ip_conntrack_tuple *tp, 
								int *keep)
{
	uint32_t	sip=tp->src.ip, 
				dip=tp->dst.ip;
	int			retval=0;
	int			prio=0;
	
	/*clear_bit(IPS_QOS_LOW_BIT, (void *)&tp->ips_qos_type);
	clear_bit(IPS_QOS_MID_BIT, (void *)&tp->ips_qos_type);
	clear_bit(IPS_QOS_HI_BIT, (void *)&tp->ips_qos_type);
	clear_bit(IPS_QOS_KEEP_BIT, (void *)&tp->ips_qos_type);*/
	IPS_QOS_WRT_BIT_BGN;
	tp->ips_qos_type=0;
	IPS_QOS_WRT_BIT_FIN;
	
	if(inet_addr_type(sip)==RTN_LOCAL && inet_addr_type(dip)==RTN_LOCAL) {
		IPS_QOS_SET_TYPE(IPS_DEV_DEV, (void *)&tp->ips_qos_type);
	} else if(inet_addr_type(sip)==RTN_LOCAL) {
		IPS_QOS_SET_TYPE(IPS_DEV_WAN, (void *)&tp->ips_qos_type);
	} else if(inet_addr_type(dip)==RTN_LOCAL) {
		IPS_QOS_SET_TYPE(IPS_LAN_DEV, (void *)&tp->ips_qos_type);
	} else {
		IPS_QOS_SET_TYPE(IPS_UNKNOWN_TYPE, (void *)&tp->ips_qos_type);
	}
#if		IPS_QOS_DEBUGMOD
	IPS_QOSDBG("PROTO:%d\n", tp->dst.protonum);
	if(tp->dst.protonum==IPPROTO_TCP)
		IPS_QOSDBG("PORT:%d\n", ntohs(tp->dst.u.tcp.port));
#endif
	
	/* Checking packet type by ips_qos filter */
	int					i=-1;
	IPS_QOS_FILTER	*temp;
	uint16_t			protonum_4_layer=tp->dst.protonum;
	uint32_t			protonum_7_layer=0;
	
	switch(protonum_4_layer) {
	case	IPPROTO_ICMP:
		/*i=IPS_QOS_FILTER_ICMP;
		protonum_7_layer;
		ntohs(tp->dst.u.icmp.type);
		ntohs(tp->dst.u.icmp.code);
		break;*/
		goto	KEEP;
	case	IPPROTO_TCP:
		i=IPS_QOS_FILTER_TCP;
		protonum_7_layer=ntohs(tp->dst.u.tcp.port);
		break;
	case	IPPROTO_UDP:
		i=IPS_QOS_FILTER_UDP;
		protonum_7_layer=ntohs(tp->dst.u.udp.port);
		break;
	default:
		goto	END;
	}
	list_for_each_entry(temp, &ips_qos_filter_list[i], list) {
		switch(i) {
		case	IPS_QOS_FILTER_ICMP:
			if(protonum_7_layer!=temp->u.icmp.id)
				continue;
			else
				goto	KEEP;
		case	IPS_QOS_FILTER_TCP:
			if(protonum_7_layer!=temp->u.tcp.port)
				continue;
			else
				goto	KEEP;
		case	IPS_QOS_FILTER_UDP:
			if(protonum_7_layer!=temp->u.udp.port)
				continue;
			else
				goto	KEEP;
		}
	}
	goto	END;
KEEP:
	IPS_QOS_SET_BIT(IPS_QOS_KEEP_BIT, (void *)&tp->ips_qos_type);
	*keep=1;
	/* Old, easy-style */
	/*{
		if(tp->dst.protonum==IPPROTO_TCP 
			&& (ntohs(tp->dst.u.tcp.port)>1024 	
			&& ntohs(tp->dst.u.tcp.port)!=1863 
			&& ntohs(tp->dst.u.tcp.port)!=8080 
			)) {
			goto	END;
		} else if(tp->dst.protonum==IPPROTO_TCP 
			&& (ntohs(tp->dst.u.tcp.port)==20 
			|| ntohs(tp->dst.u.tcp.port)==21 
			|| ntohs(tp->dst.u.tcp.port)==22 
			|| ntohs(tp->dst.u.tcp.port)==23 
			|| ntohs(tp->dst.u.tcp.port)==25  
			|| ntohs(tp->dst.u.tcp.port)==64 
			|| ntohs(tp->dst.u.tcp.port)==80 
			|| ntohs(tp->dst.u.tcp.port)==443 
			|| ntohs(tp->dst.u.tcp.port)==1863 
			|| ntohs(tp->dst.u.tcp.port)==8080)) {
			IPS_QOS_SET_BIT(IPS_QOS_KEEP_BIT, (void *)&tp->ips_qos_type);
			*keep=1;
		} else if(tp->dst.protonum==IPPROTO_ICMP) {
			IPS_QOS_SET_BIT(IPS_QOS_KEEP_BIT, (void *)&tp->ips_qos_type);
			*keep=1;
		}
	}*/
END:
	retval=prio;
#if		IPS_QOS_DEBUGMOD
	uint8_t *psip_8, *pdip_8;
	if(*keep==1 || retval==IPS_QOS_HI_BIT) {
		u32to8(&sip, &psip_8);
		u32to8(&dip, &pdip_8);
		IPS_QOS_TEST_BIT_BGN;
		IPS_QOSDBG("(%d-%d)"
			, IPS_QOS_TEST_BIT(IPS_QOS_KEEP_BIT	, &tp->ips_qos_type), prio);
		IPS_QOS_TEST_BIT_FIN;
	}
#endif
	return		retval;
}
