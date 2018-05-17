/*
 *	Forwarding database
 *	Linux ethernet bridge
 *
 *	Authors:
 *	Lennert Buytenhek		<buytenh@gnu.org>
 *
 *	$Id: br_fdb.c,v 1.1.1.1 2007-05-25 06:50:00 bruce Exp $
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/times.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/jhash.h>
#include <asm/atomic.h>
#include "br_private.h"

static struct kmem_cache *br_fdb_cache __read_mostly;

/*+++, MAC Address Translator, Builder, 2008/12/10, start*/
#ifdef CONFIG_BRIDGE_MAT
static int fdb_insert(struct net_bridge *br, struct net_bridge_port *source,
		      const unsigned char *addr, unsigned int ipaddr);
#else
static int fdb_insert(struct net_bridge *br, struct net_bridge_port *source,
		      const unsigned char *addr);
#endif /*CONFIG_BRIDGE_MAT*/
/*+++, MAC Address Translator, Builder, 2008/12/10, end*/

void __init br_fdb_init(void)
{
	br_fdb_cache = kmem_cache_create("bridge_fdb_cache",
					 sizeof(struct net_bridge_fdb_entry),
					 0,
					 SLAB_HWCACHE_ALIGN, NULL, NULL);
}

void __exit br_fdb_fini(void)
{
	kmem_cache_destroy(br_fdb_cache);
}


/* if topology_changing then use forward_delay (default 15 sec)
 * otherwise keep longer (default 5 minutes)
 */
static __inline__ unsigned long hold_time(const struct net_bridge *br)
{
	return br->topology_change ? br->forward_delay : br->ageing_time;
}

static __inline__ int has_expired(const struct net_bridge *br,
				  const struct net_bridge_fdb_entry *fdb)
{
	return !fdb->is_static
		&& time_before_eq(fdb->ageing_timer + hold_time(br), jiffies);
}

static __inline__ int br_mac_hash(const unsigned char *mac)
{
	return jhash(mac, ETH_ALEN, 0) & (BR_HASH_SIZE - 1);
}

static __inline__ void fdb_delete(struct net_bridge_fdb_entry *f)
{
	hlist_del_rcu(&f->hlist);
	br_fdb_put(f);
}

void br_fdb_changeaddr(struct net_bridge_port *p, const unsigned char *newaddr)
{
	struct net_bridge *br = p->br;
	int i;

	spin_lock_bh(&br->hash_lock);

	/* Search all chains since old address/hash is unknown */
	for (i = 0; i < BR_HASH_SIZE; i++) {
		struct hlist_node *h;
		hlist_for_each(h, &br->hash[i]) {
			struct net_bridge_fdb_entry *f;

			f = hlist_entry(h, struct net_bridge_fdb_entry, hlist);
			if (f->dst == p && f->is_local) {
				/* maybe another port has same hw addr? */
				struct net_bridge_port *op;
				list_for_each_entry(op, &br->port_list, list) {
					if (op != p &&
					    !compare_ether_addr(op->dev->dev_addr,
								f->addr.addr)) {
						f->dst = op;
						goto insert;
					}
				}

				/* delete old one */
				fdb_delete(f);
				goto insert;
			}
		}
	}
 insert:
	/* insert new address,  may fail if invalid address or dup. */
/*+++, MAC Address Translator, Builder, 2008/12/10, start*/
#ifdef CONFIG_BRIDGE_MAT
	fdb_insert(br, p, newaddr, 0);
#else
	fdb_insert(br, p, newaddr);
#endif /*CONFIG_BRIDGE_MAT*/
/*+++, MAC Address Translator, Builder, 2008/12/10, end*/

	spin_unlock_bh(&br->hash_lock);
}

void br_fdb_cleanup(unsigned long _data)
{
	struct net_bridge *br = (struct net_bridge *)_data;
	unsigned long delay = hold_time(br);
	int i;

	spin_lock_bh(&br->hash_lock);
	for (i = 0; i < BR_HASH_SIZE; i++) {
		struct net_bridge_fdb_entry *f;
		struct hlist_node *h, *n;

		hlist_for_each_entry_safe(f, h, n, &br->hash[i], hlist) {
			if (!f->is_static &&
			    time_before_eq(f->ageing_timer + delay, jiffies))
				fdb_delete(f);
		}
	}
	spin_unlock_bh(&br->hash_lock);

	mod_timer(&br->gc_timer, jiffies + HZ/10);
}


void br_fdb_delete_by_port(struct net_bridge *br,
			   const struct net_bridge_port *p,
			   int do_all)
{
	int i;

	spin_lock_bh(&br->hash_lock);
	for (i = 0; i < BR_HASH_SIZE; i++) {
		struct hlist_node *h, *g;

		hlist_for_each_safe(h, g, &br->hash[i]) {
			struct net_bridge_fdb_entry *f
				= hlist_entry(h, struct net_bridge_fdb_entry, hlist);
			if (f->dst != p)
				continue;

			if (f->is_static && !do_all)
				continue;
			/*
			 * if multiple ports all have the same device address
			 * then when one port is deleted, assign
			 * the local entry to other port
			 */
			if (f->is_local) {
				struct net_bridge_port *op;
				list_for_each_entry(op, &br->port_list, list) {
					if (op != p &&
					    !compare_ether_addr(op->dev->dev_addr,
								f->addr.addr)) {
						f->dst = op;
						goto skip_delete;
					}
				}
			}

			fdb_delete(f);
		skip_delete: ;
		}
	}
	spin_unlock_bh(&br->hash_lock);
}

/* No locking or refcounting, assumes caller has no preempt (rcu_read_lock) */
struct net_bridge_fdb_entry *__br_fdb_get(struct net_bridge *br,
					  const unsigned char *addr)
{
	struct hlist_node *h;
	struct net_bridge_fdb_entry *fdb;

	hlist_for_each_entry_rcu(fdb, h, &br->hash[br_mac_hash(addr)], hlist) {
		if (!compare_ether_addr(fdb->addr.addr, addr)) {
			if (unlikely(has_expired(br, fdb)))
				break;
			return fdb;
		}
	}

	return NULL;
}

/*+++, MAC Address Translator, Builder, 2008/12/10, start*/
#ifdef CONFIG_BRIDGE_MAT
struct net_bridge_fdb_entry *br_fdb_ip_get(struct net_bridge *br, unsigned int ipaddr)
{
	struct hlist_node *h;
	struct net_bridge_fdb_entry *fdb;
	struct hlist_head *head;
	unsigned int i;

	rcu_read_lock();
	for(i=0;i<BR_HASH_SIZE;i++){
		head = &br->hash[i];
		if (head!=NULL){
			hlist_for_each_entry_rcu(fdb, h, head, hlist) {
				if (has_expired(br, fdb)){
					continue;
				}
				if (fdb->ipaddr == ipaddr){
					if((br->mat_ena)&&(br->mat_mode!=0)&&(memcmp(fdb->addr.addr, br->wl_curr_mac, ETH_ALEN)==0)){
						rcu_read_unlock();
						return NULL;
					}else{
						rcu_read_unlock();
						return fdb;
					}
				}else{
					continue;
				}
			}
		}
	}
	rcu_read_unlock();
	return NULL;
}

struct net_bridge_fdb_entry *br_fdb_ip_get6(struct net_bridge *br, struct in6_addr *ipv6addr)
{
    struct hlist_node *h;
	struct net_bridge_fdb_entry *fdb;
	struct hlist_head *head;
	unsigned int i;
	
	rcu_read_lock();
	for(i=0;i<BR_HASH_SIZE;i++){
		head = &br->hash[i];
		if (head!=NULL){
			hlist_for_each_entry_rcu(fdb, h, head, hlist) {
				if (has_expired(br, fdb)){
					continue;
				}
				if ((!memcmp(&fdb->global_ipv6addr[0], ipv6addr, sizeof(struct in6_addr)))||
					(!memcmp(&fdb->global_ipv6addr[1], ipv6addr, sizeof(struct in6_addr)))||
				    (!memcmp(&fdb->local_ipv6addr, ipv6addr, sizeof(struct in6_addr)))){
					if((br->mat_ena)&&(br->mat_mode!=0)&&(memcmp(fdb->addr.addr, br->wl_curr_mac, ETH_ALEN)==0)){
						rcu_read_unlock();
						return NULL;
					}else{
					rcu_read_unlock();
					return fdb;
					}
				}else{
					continue;
				}
			}
		}
	}
	rcu_read_unlock();
	return NULL;
}
#endif /*CONFIG_BRIDGE_MAT*/
/*+++, MAC Address Translator, Builder, 2008/12/10, end*/

/* Interface used by ATM hook that keeps a ref count */
struct net_bridge_fdb_entry *br_fdb_get(struct net_bridge *br,
					unsigned char *addr)
{
	struct net_bridge_fdb_entry *fdb;

	rcu_read_lock();
	fdb = __br_fdb_get(br, addr);
	if (fdb && !atomic_inc_not_zero(&fdb->use_count))
		fdb = NULL;
	rcu_read_unlock();
	return fdb;
}

static void fdb_rcu_free(struct rcu_head *head)
{
	struct net_bridge_fdb_entry *ent
		= container_of(head, struct net_bridge_fdb_entry, rcu);
	kmem_cache_free(br_fdb_cache, ent);
}

/* Set entry up for deletion with RCU  */
void br_fdb_put(struct net_bridge_fdb_entry *ent)
{
	if (atomic_dec_and_test(&ent->use_count))
		call_rcu(&ent->rcu, fdb_rcu_free);
}

/*
 * Fill buffer with forwarding table records in
 * the API format.
 */
int br_fdb_fillbuf(struct net_bridge *br, void *buf,
		   unsigned long maxnum, unsigned long skip)
{
	struct __fdb_entry *fe = buf;
	int i, num = 0;
	struct hlist_node *h;
	struct net_bridge_fdb_entry *f;

	memset(buf, 0, maxnum*sizeof(struct __fdb_entry));

	rcu_read_lock();
	for (i = 0; i < BR_HASH_SIZE; i++) {
		hlist_for_each_entry_rcu(f, h, &br->hash[i], hlist) {
			if (num >= maxnum)
				goto out;

			if (has_expired(br, f))
				continue;

			if (skip) {
				--skip;
				continue;
			}

			/* convert from internal format to API */
			memcpy(fe->mac_addr, f->addr.addr, ETH_ALEN);
			fe->port_no = f->dst->port_no;
			fe->is_local = f->is_local;
			if (!f->is_static)
				fe->ageing_timer_value = jiffies_to_clock_t(jiffies - f->ageing_timer);
			++fe;
			++num;
		}
	}

 out:
	rcu_read_unlock();

	return num;
}

static inline struct net_bridge_fdb_entry *fdb_find(struct hlist_head *head,
						    const unsigned char *addr)
{
	struct hlist_node *h;
	struct net_bridge_fdb_entry *fdb;

	hlist_for_each_entry_rcu(fdb, h, head, hlist) {
		if (!compare_ether_addr(fdb->addr.addr, addr))
			return fdb;
	}
	return NULL;
}

/*+++, MAC Address Translator, Builder, 2008/12/10, start*/
#ifdef CONFIG_BRIDGE_MAT
static struct net_bridge_fdb_entry *fdb_create(struct hlist_head *head,
					       struct net_bridge_port *source,
					       const unsigned char *addr,
					       unsigned int ipaddr,
					       int is_local)
{
	struct net_bridge_fdb_entry *fdb;

	fdb = kmem_cache_alloc(br_fdb_cache, GFP_ATOMIC);
	if (fdb) {
		memcpy(fdb->addr.addr, addr, ETH_ALEN);
		atomic_set(&fdb->use_count, 1);
		hlist_add_head_rcu(&fdb->hlist, head);
		fdb->dst = source;
		if(ipaddr!=0){
		    fdb->ipaddr = ipaddr;
		}
		fdb->dst = source;
		fdb->is_local = is_local;
		fdb->is_static = is_local;
		fdb->ageing_timer = jiffies;
	}
	return fdb;
}

static struct net_bridge_fdb_entry *fdb_create6(struct hlist_head *head,
					       struct net_bridge_port *source,
					       const unsigned char *addr,
					       struct in6_addr *ipv6addr,
					       int is_local)
{
	struct net_bridge_fdb_entry *fdb;
	struct in6_addr empty_ipv6addr;
	
	fdb = kmem_cache_alloc(br_fdb_cache, GFP_ATOMIC);
	if (fdb) {
		memcpy(fdb->addr.addr, addr, ETH_ALEN);
		atomic_set(&fdb->use_count, 1);
		hlist_add_head_rcu(&fdb->hlist, head);
		fdb->dst = source;
		memset((char *)&empty_ipv6addr, 0x0, sizeof(struct in6_addr));
		if (memcmp((char *)ipv6addr, (char *)&empty_ipv6addr, sizeof(struct in6_addr))){
			if (ipv6addr->in6_u.u6_addr16[0]==htons(0xfe80)){
			    	memcpy((char *)&fdb->local_ipv6addr, (char *)ipv6addr, sizeof(struct in6_addr));
			    }else{
			    	if((ipv6addr->in6_u.u6_addr8[13]==addr[3])&&
			    	   (ipv6addr->in6_u.u6_addr8[14]==addr[4])&&
			    	   (ipv6addr->in6_u.u6_addr8[15]==addr[5])){
			    	   	/*prefix + mac addr*/
			    		memcpy((char *)&fdb->global_ipv6addr[0], (char *)ipv6addr, sizeof(struct in6_addr));
			    	}else{
			    		/*assigned by radvd*/
			    		memcpy((char *)&fdb->global_ipv6addr[1], (char *)ipv6addr, sizeof(struct in6_addr));
			    	}
/*
			    	printk("fdb_create6: global ipv6 addr\n");
			    	printk("ipv6addr->in6_u.u6_addr8[13~15]: %.2x:%.2x:%.2x\n",
			    	       ipv6addr->in6_u.u6_addr8[13],
			    	       ipv6addr->in6_u.u6_addr8[14],
			    	       ipv6addr->in6_u.u6_addr8[15]);
			    	printk("addr[3~5]: %.2x:%.2x:%.2x\n", addr[3], addr[4], addr[5]);
*/
			    }
		}
		fdb->dst = source;
		fdb->is_local = is_local;
		fdb->is_static = is_local;
		fdb->ageing_timer = jiffies;
	}
	return fdb;
}
#else
static struct net_bridge_fdb_entry *fdb_create(struct hlist_head *head,
					       struct net_bridge_port *source,
					       const unsigned char *addr,
					       int is_local)
{
	struct net_bridge_fdb_entry *fdb;

	fdb = kmem_cache_alloc(br_fdb_cache, GFP_ATOMIC);
	if (fdb) {
		memcpy(fdb->addr.addr, addr, ETH_ALEN);
		atomic_set(&fdb->use_count, 1);
		hlist_add_head_rcu(&fdb->hlist, head);

		fdb->dst = source;
		fdb->is_local = is_local;
		fdb->is_static = is_local;
		fdb->ageing_timer = jiffies;
	}
	return fdb;
}
#endif /*CONFIG_BRIDGE_MAT*/
/*+++, MAC Address Translator, Builder, 2008/12/10, start*/

/*+++, MAC Address Translator, Builder, 2008/12/10, start*/
#ifdef CONFIG_BRIDGE_MAT
static int fdb_insert(struct net_bridge *br, struct net_bridge_port *source,
		  const unsigned char *addr, unsigned int ipaddr)
{
	struct hlist_head *head = &br->hash[br_mac_hash(addr)];
	struct net_bridge_fdb_entry *fdb;

	if (!is_valid_ether_addr(addr))
		return -EINVAL;

	fdb = fdb_find(head, addr);
	if (fdb) {
		/* it is okay to have multiple ports with same
		 * address, just use the first one.
		 */
		if (fdb->is_local)
			return 0;

		printk(KERN_WARNING "%s adding interface with same address "
		       "as a received packet\n",
		       source->dev->name);
		fdb_delete(fdb);
 	}

	if (!fdb_create(head, source, addr, ipaddr, 1))
		return -ENOMEM;

	return 0;
}
#else
static int fdb_insert(struct net_bridge *br, struct net_bridge_port *source,
		  const unsigned char *addr)
{
	struct hlist_head *head = &br->hash[br_mac_hash(addr)];
	struct net_bridge_fdb_entry *fdb;

	if (!is_valid_ether_addr(addr))
		return -EINVAL;

	fdb = fdb_find(head, addr);
	if (fdb) {
		/* it is okay to have multiple ports with same
		 * address, just use the first one.
		 */
		if (fdb->is_local)
			return 0;

		printk(KERN_WARNING "%s adding interface with same address "
		       "as a received packet\n",
		       source->dev->name);
		fdb_delete(fdb);
	}

	if (!fdb_create(head, source, addr, 1))
		return -ENOMEM;

	return 0;
}
#endif /*CONFIG_BRIDGE_MAT*/
/*+++, MAC Address Translator, Builder, 2008/12/10, end*/

/*+++, MAC Address Translator, Builder, 2008/12/10, start*/
#ifdef CONFIG_BRIDGE_MAT
int br_fdb_insert(struct net_bridge *br, struct net_bridge_port *source,
		  const unsigned char *addr, unsigned int ipaddr)
{
	int ret;

	if((br->mat_ena)&&(br->mat_mode!=0)&&(memcmp(addr, br->wl_curr_mac, ETH_ALEN)==0)){
		printk("br_fdb_insert() return 0.\n");
		return 0;
	}

	spin_lock_bh(&br->hash_lock);
	ret = fdb_insert(br, source, addr, ipaddr);
	spin_unlock_bh(&br->hash_lock);
	return ret;
}
#else
int br_fdb_insert(struct net_bridge *br, struct net_bridge_port *source,
		  const unsigned char *addr)
{
	int ret;

	spin_lock_bh(&br->hash_lock);
	ret = fdb_insert(br, source, addr);
	spin_unlock_bh(&br->hash_lock);
	return ret;
}
#endif /*CONFIG_BRIDGE_MAT*/
/*+++, MAC Address Translator, Builder, 2008/12/10, end*/

/*+++, MAC Address Translator, Builder, 2008/12/10, start*/
#ifdef CONFIG_BRIDGE_MAT
void br_fdb_update(struct net_bridge *br, struct net_bridge_port *source,
		   const unsigned char *addr, unsigned int ipaddr)
{
	struct hlist_head *head = &br->hash[br_mac_hash(addr)];
	struct net_bridge_fdb_entry *fdb;

	/* some users want to always flood. */
	if (hold_time(br) == 0)
		return;

	rcu_read_lock();
	fdb = fdb_find(head, addr);
	if (likely(fdb)) {
		/* attempt to update an entry for a local interface */
		if (unlikely(fdb->is_local)) {
			if (net_ratelimit())
				printk(KERN_WARNING "%s: received packet with "
				       " own address as source address\n",
				       source->dev->name);
		} else {
			/* fastpath: update of existing entry */
			fdb->dst = source;
			fdb->ageing_timer = jiffies;
			if (ipaddr!=0){
			    fdb->ipaddr = ipaddr;
			}
		}
	} else {
		spin_lock_bh(&br->hash_lock);
		if (!fdb_find(head, addr)){
			fdb_create(head, source, addr, ipaddr, 0);
		}
		/* else  we lose race and someone else inserts
		 * it first, don't bother updating
		 */
		spin_unlock_bh(&br->hash_lock);
	}
	rcu_read_unlock();
}

void br_fdb_update6(struct net_bridge *br, struct net_bridge_port *source,
		   const unsigned char *addr, struct in6_addr *ipv6addr)
{
	struct hlist_head *head = &br->hash[br_mac_hash(addr)];
	struct net_bridge_fdb_entry *fdb;
	struct in6_addr empty_ipv6addr;
	
	/* some users want to always flood. */
	if (hold_time(br) == 0)
		return;

	rcu_read_lock();
	fdb = fdb_find(head, addr);
	if (likely(fdb)) {
		/* attempt to update an entry for a local interface */
		if (unlikely(fdb->is_local)) {
			if (net_ratelimit())
				printk(KERN_WARNING "%s: received packet with "
				       " own address as source address\n",
				       source->dev->name);
		} else {
			/* fastpath: update of existing entry */
			fdb->dst = source;
			fdb->ageing_timer = jiffies;
			memset((char *)&empty_ipv6addr, 0x0, sizeof(struct in6_addr));
			if (memcmp((char *)ipv6addr, (char *)&empty_ipv6addr, sizeof(struct in6_addr))){
				if (ipv6addr->in6_u.u6_addr16[0]==htons(0xfe80)){
			    	memcpy((char *)&fdb->local_ipv6addr, (char *)ipv6addr, sizeof(struct in6_addr));
			    }else{
			    	if((ipv6addr->in6_u.u6_addr8[13]==addr[3])&&
			    	   (ipv6addr->in6_u.u6_addr8[14]==addr[4])&&
			    	   (ipv6addr->in6_u.u6_addr8[15]==addr[5])){
			    		/*prefix + mac addr*/
			    		memcpy((char *)&fdb->global_ipv6addr[0], (char *)ipv6addr, sizeof(struct in6_addr));
			    	}else{
			    		/*assigned by radvd*/
			    		memcpy((char *)&fdb->global_ipv6addr[1], (char *)ipv6addr, sizeof(struct in6_addr));
			    	}
/*
			    	printk("br_fdb_update6: global ipv6 addr\n");
			    	printk("ipv6addr->in6_u.u6_addr8[13~15]: %.2x:%.2x:%.2x\n",
			    	       ipv6addr->in6_u.u6_addr8[13],
			    	       ipv6addr->in6_u.u6_addr8[14],
			    	       ipv6addr->in6_u.u6_addr8[15]);
			    	printk("addr[3~5]: %.2x:%.2x:%.2x\n", addr[3], addr[4], addr[5]);
*/
			    }
			}
		}
	} else {
		spin_lock_bh(&br->hash_lock);
		if (!fdb_find(head, addr)){
			fdb_create6(head, source, addr, ipv6addr, 0);
		}
		/* else  we lose race and someone else inserts
		 * it first, don't bother updating
		 */
		spin_unlock_bh(&br->hash_lock);
	}
	rcu_read_unlock();
}
#else
void br_fdb_update(struct net_bridge *br, struct net_bridge_port *source,
		   const unsigned char *addr)
{
	struct hlist_head *head = &br->hash[br_mac_hash(addr)];
	struct net_bridge_fdb_entry *fdb;

	/* some users want to always flood. */
	if (hold_time(br) == 0)
		return;

	fdb = fdb_find(head, addr);
	if (likely(fdb)) {
		/* attempt to update an entry for a local interface */
		if (unlikely(fdb->is_local)) {
			if (net_ratelimit())
				printk(KERN_WARNING "%s: received packet with "
				       " own address as source address\n",
				       source->dev->name);
		} else {
			/* fastpath: update of existing entry */
			fdb->dst = source;
			fdb->ageing_timer = jiffies;
		}
	} else {
		spin_lock(&br->hash_lock);
		if (!fdb_find(head, addr))
			fdb_create(head, source, addr, 0);
		/* else  we lose race and someone else inserts
		 * it first, don't bother updating
		 */
		spin_unlock(&br->hash_lock);
	}
}
#endif /*CONFIG_BRIDGE_MAT*/
/*+++, MAC Address Translator, Builder, 2008/12/10, end*/