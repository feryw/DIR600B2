#ifndef RA_MAC
#define RA_MAC

#include <linux/config.h>
#include <linux/module.h>
#include <linux/version.h>

#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/interrupt.h>
#include <linux/ptrace.h>
#include <linux/ioport.h>
#include <linux/in.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/signal.h>
#include <linux/irq.h>

#include <asm/system.h>
#include <asm/io.h>
#include <asm/bitops.h>
#include <asm/io.h>
#include <asm/dma.h>

#include <asm-mips/mipsregs.h>  /* for cp0 reg definition */
#include <asm/rt2880/surfboardint.h>	/* for cp0 reg access, added by bobtseng */

#include <linux/errno.h>
#include <linux/init.h>
#include <linux/mca.h>

#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>

#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>

#include "ra2882ethreg.h"
#include "raether.h"
#include "ra_mac.h"
#include "ra_ethtool.h"

extern END_DEVICE *ra_ei_local;

#if defined (CONFIG_RALINK_RT3052) && defined (CONFIG_RT_3052_ESW_PORT_STATUS_INT)
/* Pid of process we want send signal to, should not be zero*/
unsigned long int linkUpProcPid = 0;
#endif

int phyregread(unsigned int phy_register, unsigned int* value)
{
	unsigned long mdio_reg = sysRegRead(MDIO_ACCESS);
	unsigned long timer = jiffies;

	for (;;)
	{
		if ( !(mdio_reg & RT2880_BIT(31) ) )
			break;
		else if ((jiffies - timer) > (5*HZ))
		{
			printk(KERN_ERR "rt2880 MDIO read Failed...\n");
			return -1;
		}
	}
	sysRegWrite(MDIO_ACCESS, 0);

	mdio_reg = (0x1f << 24) | (phy_register << 16);
	sysRegWrite(MDIO_ACCESS, mdio_reg);
	mdio_reg |= RT2880_BIT(31);
	sysRegWrite(MDIO_ACCESS, mdio_reg);

	timer = jiffies;

        for (;;)
        {
		mdio_reg = sysRegRead(MDIO_ACCESS);

                if ( !(mdio_reg & RT2880_BIT(31) )) {
			mdio_reg = sysRegRead(MDIO_ACCESS);
			*value = (unsigned long)(mdio_reg & 0x0000ffff);
                        break;
		} else if ((jiffies - timer) > (5*HZ))
                {
                        printk(KERN_ERR "rt2880 MDIO read Failed!\n");
                        return -2;
                }
        }

	return 1;
}

int phyregwrite(unsigned int phy_register, unsigned int value)
{
        unsigned long mdio_reg = sysRegRead(MDIO_ACCESS);
        unsigned long timer = jiffies;

        for (;;)
        {
                if ( !(mdio_reg & RT2880_BIT(31) ) )
                        break;
                else if ((jiffies - timer) > (5*HZ))
                {
                        printk(KERN_ERR "rt2880 MDIO Write Failed...\n");
                        return -1;
                }
        }

        sysRegWrite(MDIO_ACCESS, 0);
	mdio_reg = (0x1f << 24) | (phy_register << 16) | RT2880_BIT(30) | value;
        sysRegWrite(MDIO_ACCESS, mdio_reg);

	mdio_reg |= RT2880_BIT(31);
	sysRegWrite(MDIO_ACCESS, mdio_reg);

	timer = jiffies;

        mdio_reg = sysRegRead(MDIO_ACCESS);
        for (;;)
        {
                if ( !(mdio_reg & RT2880_BIT(31) )) {
                        break;
                } else if ((jiffies - timer) > (5*HZ))
                {
                        printk(KERN_ERR "rt2880 MDIO Write Failed!\n");
                        return -2;
                }
        	mdio_reg = sysRegRead(MDIO_ACCESS);
        }
	return 1;

}

void rt2880_mdio_cfg(unsigned int addr, unsigned int offset, unsigned int value)
{
	unsigned int phy_register;
	int ret;
	if (offset > 32) {
		printk("offset input failed");
		return;
	}

	ret = phyregread(addr, &phy_register);
	if (ret < 0) {
		printk("PHY config read failed!");
		return;
	} else
		printk("mdio.wb addr[%08X] value[0x%08X]\n", addr, phy_register);

	if(phy_register)
	{
		phy_register |= RT2880_BIT(offset);
	} else {
		phy_register &= ~(RT2880_BIT(offset));
	}

	ret = phyregwrite(addr, phy_register);

	if ( ret < 0)
		printk("PHY config write failed!");
	else
		printk("mdio.wb addr[0x%X] value[0x%X]\n", addr, phy_register);
}

#ifdef CONFIG_GIGAPHY
inline void enable_auto_negotiate(void)
{

	u32 regValue = sysRegRead(MDIO_CFG);
	u32 addr = CONFIG_MAC_TO_GIGAPHY_MODE_ADDR;	// define in linux/autoconf.h

	regValue &= 0xe0ff7fff;			// clear auto polling related field:
 						// (MD_PHY1ADDR & GP1_FRC_EN).
	regValue |= 0x20000000;			// force to enable MDC/MDIO auto polling.
	regValue |= (addr << 24);		// setup PHY address for auto polling.

	sysRegWrite(MDIO_CFG, regValue);
	return;
}

#define LINKUP_RETRY_MAX 10
inline void wait_linkup(void)
{
	u32 regValue;
	int retry=0;
	while(retry < LINKUP_RETRY_MAX){
		regValue = sysRegRead(MDIO_CFG);
		if(! (regValue & RT2880_GP1_LNK_DWN)){
			printk("raeth: link up.\n");
            return;
		}
		msleep(50);
		retry++;
	}
	printk("raeth: give up to wait for linkup\n");
	return;
}

#define AN_RETRY_MAX 30
inline void wait_an_completed(void)
{
	u32 regValue;
	int retry=0;
	while(retry < AN_RETRY_MAX){
		regValue = sysRegRead(MDIO_CFG);
		if((regValue & RT2880_GP1_AN_FAIL)){
			printk("raeth: AN completed.\n");
			break;
		}
		msleep(100);
		retry++;
	}
	printk("raeth: timeout for AN\n");
	return;
}
#endif


void phySetup()
{
#ifdef CONFIG_MAC_TO_MAC_MODE
	*(unsigned long *)MDIO_CFG = 0x1F01DC01;
	printk("MDIO_CFG = %x\n", *(unsigned long *)MDIO_CFG);
	return;
#endif

#ifdef CONFIG_GIGAPHY
	unsigned long phy_id0, phy_id1;
	unsigned long regValue = sysRegRead(MDIO_CFG);
	int ret = 0;

	regValue &= ~(0x1F << 24);
	regValue |= (0x1F << 24);
	sysRegWrite(MDIO_CFG, regValue);

	ret =  phyregread(2, &phy_id0);
	if (!ret) {
		printk("PhyID 0 read failed!\n");
		phy_id0 = 0;
	}
	
	ret = phyregread(3, &phy_id1);
	if (!ret) {
		printk("PhyID 1 read failed!\n");
		phy_id1 = 0;
	}

	if ( (phy_id0 == EV_MARVELL_PHY_ID0) && (phy_id1 == EV_MARVELL_PHY_ID1) ) {
		printk("MARVELL PHY Init ok...\n");
                rt2880_mdio_cfg(20, 7, 1);
                rt2880_mdio_cfg(0, 15, 1);
		regValue = sysRegRead(FE_GLO_CFG); 
		regValue |= 0x0000ff00;
		sysRegWrite(FE_GLO_CFG, regValue);
	} else {
		enable_auto_negotiate();
		wait_linkup();
		wait_an_completed();
	}
	return;
#endif

#if defined (CONFIG_RAETH_ROUTER ) || defined (CONFIG_ICPLUS_PHY)
	printk("IC-Plus PHY Init...");
	sysRegWrite(MDIO_CFG, INIT_VALUE_OF_ICPLUS_PHY_INIT_VALUE);
	printk("ok\n");
#endif

}

void ra2880stop(END_DEVICE *ei_local)
{
	unsigned int regValue;
	printk("ra2880stop()...");

	regValue = sysRegRead(PDMA_GLO_CFG);
	regValue &= ~(RT2880_TX_WB_DDONE | RT2880_RX_DMA_EN | RT2880_TX_DMA_EN);
	sysRegWrite(PDMA_GLO_CFG, regValue);
    	
	printk("Done\n");	
	// printk("Done0x%x...\n", readreg(PDMA_GLO_CFG));
}

void ei_irq_clear(void)
{
        sysRegWrite(FE_INT_STATUS, 0xFFFFFFFF);
}

void rt2880_gmac_hard_reset(void)
{
	unsigned int regValue = sysRegRead(FE_RESET);

	regValue |= FE_RESET_BIT;
	sysRegWrite(FE_RESET, regValue);
}

void ra2880EnableInterrupt()
{
	unsigned int regValue = sysRegRead(FE_INT_ENABLE);
	RAETH_PRINT("FE_INT_ENABLE -- : 0x%08x\n", regValue);
//	regValue |= (RX_DONE_INT0 | TX_DONE_INT0);
		
	sysRegWrite(FE_INT_ENABLE, regValue);
}

/* add by david for reading the current MAC address.
 * The MAC address should be setup in bootcode, it can be used as default. */
void ra2880_read_mac(unsigned char p[6])
{
	unsigned long val;

	val = sysRegRead(GDMA1_MAC_ADRH);
	p[0] = 0xff & (val>>8);
	p[1] = 0xff & val;

	val = sysRegRead(GDMA1_MAC_ADRL);
	p[2] = 0xff & (val>>24);
	p[3] = 0xff & (val>>16);
	p[4] = 0xff & (val>>8);
	p[5] = 0xff & val;

	printk("%s: %02x:%02x:%02x:%02x:%02x:%02x\n",__func__,p[0],p[1],p[2],p[3],p[4],p[5]);
}

void ra2880MacAddressSet(MAC_INFO *MACInfo, unsigned char p[6])
{
	unsigned long regValue;
	unsigned char mac[6];

	regValue = (p[0] << 8) + (p[1]);
	sysRegWrite(GDMA1_MAC_ADRH, regValue);

	regValue = (p[2] << 8) + (p[3]);
	regValue = (regValue << 16);
	regValue |= (p[4] << 8) + (p[5]);
	sysRegWrite(GDMA1_MAC_ADRL, regValue);

	ra2880_read_mac(mac);
	return;
}

void ra2880Mac2AddressSet(MAC_INFO *MACInfo, unsigned char p[6])
{
	unsigned long regValue;
	unsigned char mac[6];

	regValue = (p[0] << 8) + (p[1]);
	sysRegWrite(GDMA2_MAC_ADRH, regValue);

	regValue = (p[2] << 8) + (p[3]);
	regValue = (regValue << 16);
	regValue |= (p[4] << 8) + (p[5]);
	sysRegWrite(GDMA2_MAC_ADRL, regValue);

	ra2880_read_mac(mac);
	return;
}

/*
 *	mii_mgr_read wrapper for mii.o ethtool
 */
static int mdio_read(struct net_device *dev, int phy_id, int location)
{
	unsigned int result;
	END_DEVICE *ei_local = dev->priv; 
	mii_mgr_read( (unsigned int) ei_local->mii_info.phy_id, (unsigned int)location, &result);
	//printk("mii.o query= phy_id:%d, address:%d retval:%d\n", phy_id, location, result);
	return (int)result;
}

/*
 *	mii_mgr_write wrapper for mii.o ethtool
 */
static void mdio_write(struct net_device *dev, int phy_id, int location, int value)
{
	END_DEVICE *ei_local = dev->priv;
	//printk("mii.o write= phy_id:%d, address:%d value:%d\n", phy_id, location, value);
	mii_mgr_write( (unsigned int) ei_local->mii_info.phy_id, (unsigned int)location, (unsigned int)value);
	return;
}

/**
 * hard_init - Called by raeth_probe to inititialize network device
 * @dev: device pointer
 *
 * ethdev_init initilize dev->priv and set to END_DEVICE structure
 *
 */
void hard_init(struct net_device *dev)
{
	END_DEVICE *ei_local = kmalloc(sizeof(END_DEVICE), GFP_KERNEL);
	MAC_INFO *macinfo = kmalloc(sizeof(MAC_INFO), GFP_KERNEL);

	memset(ei_local, 0 , sizeof(END_DEVICE));
	memset(macinfo, 0 , sizeof(MAC_INFO));

	macinfo->ivec = dev->irq;
	
	RAETH_PRINT("debug: dev_raether irq is %d(%s)\n", dev->irq, dev->name);
	ei_local->MACInfo = macinfo;
	dev->priv = (void *)ei_local;

	if ( dev->dev_addr != NULL)
		ra2880MacAddressSet(macinfo, (void *)(dev->dev_addr));
	else
		printk("HWnetInit() failed!!!\n");

	// init mii structure
	ei_local->mii_info.dev = dev;
	ei_local->mii_info.mdio_read = mdio_read;
	ei_local->mii_info.mdio_write = mdio_write;
	ei_local->mii_info.phy_id_mask = 0x1f;
	ei_local->mii_info.reg_num_mask = 0x1f;
	// TODO:   phy_id: 0~4
	ei_local->mii_info.phy_id = 1;

	return;
}

/*
 *	Routine Name : get_idx(mode, index)
 *	Description: calculate ring usage for tx/rx rings
 *	Mode 1 : Tx Ring 
 *	Mode 2 : Rx Ring
 */
int get_ring_usage(int mode, int i)
{
	unsigned long tx_ctx_idx, tx_dtx_idx, rx_usage, tx_usage;
	struct PDMA_rxdesc* rxring;
	struct PDMA_txdesc* txring;

	if (mode == 2 ) {
		/* cpu point to the next descriptor of rx dma ring */
		rx_usage =  (*(unsigned long*)RX_DRX_IDX0 - *(unsigned long*)RX_CALC_IDX0 - 1 + NUM_RX_DESC) % NUM_RX_DESC;

		if ( rx_usage == 0 )
		{
			rxring = (struct PDMA_rxdesc*)RX_BASE_PTR0;
			if(rxring == NULL)
				return 0;

			if( rxring->rxd_info2.DDONE_bit == 1)
				rx_usage = NUM_RX_DESC;
			else
				rx_usage = 0;
		}
		return rx_usage;
	}

	
	switch (i) {
		case 0:
				tx_ctx_idx = *(unsigned long*)TX_CTX_IDX0;
				tx_dtx_idx = *(unsigned long*)TX_DTX_IDX0;
				txring = (struct PDMA_txdesc*)(TX_BASE_PTR0 + (sizeof(struct PDMA_txdesc)*((tx_dtx_idx + NUM_TX_DESC-1)%NUM_TX_DESC)));
				break;
		case 1:
				tx_ctx_idx = *(unsigned long*)TX_CTX_IDX1;
				tx_dtx_idx = *(unsigned long*)TX_DTX_IDX1;
				txring = (struct PDMA_txdesc*)(TX_BASE_PTR1 + (sizeof(struct PDMA_txdesc)*((tx_dtx_idx + NUM_TX_DESC-1)%NUM_TX_DESC)));
				break;
		case 2:
				tx_ctx_idx = *(unsigned long*)TX_CTX_IDX2;
				tx_dtx_idx = *(unsigned long*)TX_DTX_IDX2;
				txring = (struct PDMA_txdesc*)(TX_BASE_PTR2 + (sizeof(struct PDMA_txdesc)*((tx_dtx_idx + NUM_TX_DESC-1)%NUM_TX_DESC)));
				break;
		case 3:
				tx_ctx_idx = *(unsigned long*)TX_CTX_IDX3;
				tx_dtx_idx = *(unsigned long*)TX_DTX_IDX3;
				txring = (struct PDMA_txdesc*)(TX_BASE_PTR3 + (sizeof(struct PDMA_txdesc)*((tx_dtx_idx + NUM_TX_DESC-1)%NUM_TX_DESC)));
				break;
		default:
			printk("get_tx_idx failed %d %d\n", mode, i);
			return 0;
	};

	tx_usage = (tx_ctx_idx - tx_dtx_idx + NUM_TX_DESC) % NUM_TX_DESC;
	if ( tx_ctx_idx == tx_dtx_idx ) {
		if ( txring == NULL )
			return -1;
		if ( txring->txd_info2.DDONE_bit == 1)
			tx_usage = NUM_TX_DESC;
		else
			tx_usage = 0;
	}
	return tx_usage;

}

#if defined(CONFIG_RAETH_QOS)
void dump_qos()
{
	int usage;
	int i;

	printk("\n-----Raeth QOS -----\n\n");

	for ( i = 0; i < 4; i++)  {
		usage = get_ring_usage(1,i);
		printk("Tx Ring%d Usage : %d/%d\n", i, usage, NUM_TX_DESC);
	}

	usage = get_ring_usage(2,0);
	printk("RX Usage : %d/%d\n\n", usage, NUM_RX_DESC);
	printk("GDMA1_FC_CFG(0x%08x)  : 0x%08x\n", GDMA1_FC_CFG, sysRegRead(GDMA1_FC_CFG));
	printk("GDMA2_FC_CFG(0x%08x)  : 0x%08x\n", GDMA2_FC_CFG, sysRegRead(GDMA2_FC_CFG));
	printk("PDMA_FC_CFG(0x%08x)  : 0x%08x\n", PDMA_FC_CFG, sysRegRead(PDMA_FC_CFG));
	printk("PSE_FQ_CFG(0x%08x)  : 0x%08x\n", PSE_FQ_CFG, sysRegRead(PSE_FQ_CFG));

	printk("\n\nTX_CTX_IDX0    : 0x%08x\n", sysRegRead(TX_CTX_IDX0));	
	printk("TX_DTX_IDX0    : 0x%08x\n", sysRegRead(TX_DTX_IDX0));
	printk("TX_CTX_IDX1    : 0x%08x\n", sysRegRead(TX_CTX_IDX1));	
	printk("TX_DTX_IDX1    : 0x%08x\n", sysRegRead(TX_DTX_IDX1));
	printk("TX_CTX_IDX2    : 0x%08x\n", sysRegRead(TX_CTX_IDX2));	
	printk("TX_DTX_IDX2    : 0x%08x\n", sysRegRead(TX_DTX_IDX2));
	printk("TX_CTX_IDX3    : 0x%08x\n", sysRegRead(TX_CTX_IDX3));
	printk("TX_DTX_IDX3    : 0x%08x\n", sysRegRead(TX_DTX_IDX3));
	printk("RX_CALC_IDX0   : 0x%08x\n", sysRegRead(RX_CALC_IDX0));
	printk("RX_DRX_IDX0    : 0x%08x\n", sysRegRead(RX_DRX_IDX0));
#if 0
	for(i=0;i<4;i++){
		printk("free_idx[%d] = %d\n", i, ra_ei_local->free_idx[i]);
	}
	{
		int j, x, y;
		struct PDMA_txdesc *tx_desc;
	for(i=0;i<4;i++){

        switch ( i) {
                case 0:
                        tx_desc = ra_ei_local->tx_ring0;
                        break;
                case 1:
                        tx_desc = ra_ei_local->tx_ring1;
                        break;
                case 2:
                        tx_desc = ra_ei_local->tx_ring2;
                        break;
                case 3:
                        tx_desc = ra_ei_local->tx_ring3;
                        break;
                default:
                        printk("ring_no input error... %d\n", i);
                        return -1;
        };
 


		j = ra_ei_local->free_idx[i];
		x= ((j-1)+NUM_TX_DESC)%NUM_TX_DESC;
		y= (j+1)%NUM_TX_DESC;
		printk("skb_free[%d][%d] = 0x%x\n", i, x, ra_ei_local->skb_free[i][x]);
		printk("skb_free[%d][%d] = 0x%x\n", i, j, ra_ei_local->skb_free[i][j]);
		printk("skb_free[%d][%d] = 0x%x\n", i, y, ra_ei_local->skb_free[i][y]);
		printk("tx_desc[%d][%d].txd_info2=0x%x\n",i, x, tx_desc[x].txd_info2);
		printk("tx_desc[%d][%d].txd_info2=0x%x\n",i, j, tx_desc[j].txd_info2);
		printk("tx_desc[%d][%d].txd_info2=0x%x\n",i, y, tx_desc[y].txd_info2);
	}
	}
#endif

	printk("\n------------------------------\n\n");
}
#endif

void dump_reg()
{
	printk("\n\nFE_INT_ENABLE  : 0x%08x\n", sysRegRead(FE_INT_ENABLE));
	printk("DLY_INT_CFG	: 0x%08x\n", sysRegRead(DLY_INT_CFG));
	printk("TX_BASE_PTR0   : 0x%08x\n", sysRegRead(TX_BASE_PTR0));	
	printk("TX_CTX_IDX0    : 0x%08x\n", sysRegRead(TX_CTX_IDX0));	
	printk("TX_DTX_IDX0    : 0x%08x\n", sysRegRead(TX_DTX_IDX0));
	printk("TX_BASE_PTR1(0x%08x)   : 0x%08x\n", TX_BASE_PTR1, sysRegRead(TX_BASE_PTR1));	
	printk("TX_CTX_IDX1(0x%08x)    : 0x%08x\n", TX_CTX_IDX1, sysRegRead(TX_CTX_IDX1));
	printk("TX_DTX_IDX1(0x%08x)    : 0x%08x\n", TX_DTX_IDX1, sysRegRead(TX_DTX_IDX1));
	printk("TX_BASE_PTR2(0x%08x)   : 0x%08x\n", TX_BASE_PTR2, sysRegRead(TX_BASE_PTR2));	
	printk("TX_CTX_IDX2(0x%08x)    : 0x%08x\n", TX_CTX_IDX2, sysRegRead(TX_CTX_IDX2));
	printk("TX_DTX_IDX2(0x%08x)    : 0x%08x\n", TX_DTX_IDX2, sysRegRead(TX_DTX_IDX2));
	printk("TX_BASE_PTR3(0x%08x)   : 0x%08x\n", TX_BASE_PTR3, sysRegRead(TX_BASE_PTR3));	
	printk("TX_CTX_IDX3(0x%08x)    : 0x%08x\n", TX_CTX_IDX3, sysRegRead(TX_CTX_IDX3));
	printk("TX_DTX_IDX3(0x%08x)    : 0x%08x\n", TX_DTX_IDX3, sysRegRead(TX_DTX_IDX3));

	printk("RX_BASE_PTR0   : 0x%08x\n", sysRegRead(RX_BASE_PTR0));	
	printk("RX_MAX_CNT0    : 0x%08x\n", sysRegRead(RX_MAX_CNT0));	
	printk("RX_CALC_IDX0   : 0x%08x\n", sysRegRead(RX_CALC_IDX0));
	printk("RX_DRX_IDX0    : 0x%08x\n", sysRegRead(RX_DRX_IDX0));

	printk("GDMA1_FWD_CFG  : 0x%08x\n", sysRegRead(GDMA1_FWD_CFG));	
	printk("PDMA_GLO_CFG   : 0x%08x\n", sysRegRead(PDMA_GLO_CFG));
	printk("PDMA_RST_CFG   : 0x%08x\n", sysRegRead(PDMA_RST_CFG));	
	printk("FE_RST_GL      : 0x%08x\n\n", sysRegRead(FE_RST_GL));

	printk("MDIO_CFG(0x%08x)     : 0x%08x\n", MDIO_CFG, sysRegRead(MDIO_CFG));
	printk("CDMA_OQ_STA(0x%08x)     : 0x%08x\n", CDMA_OQ_STA, sysRegRead(CDMA_OQ_STA));
	printk("GDMA1_OQ_STA(0x%08x)     : 0x%08x\n",GDMA1_OQ_STA, sysRegRead(GDMA1_OQ_STA));
	printk("PPE_OQ_STA(0x%08x)     : 0x%08x\n", PPE_OQ_STA, sysRegRead(PPE_OQ_STA));
	printk("PSE_IQ_STA(0x%08x)     : 0x%08x\n", PSE_IQ_STA, sysRegRead(PSE_IQ_STA));
	printk("GDMA1_SCH_CFG(0x%08x)  : 0x%08x\n", GDMA1_SCH_CFG, sysRegRead(GDMA1_SCH_CFG));
	printk("GDMA2_SCH_CFG(0x%08x)  : 0x%08x\n", GDMA2_SCH_CFG, sysRegRead(GDMA2_SCH_CFG));
	printk("CDMA_FC_CFG(0x%08x)  : 0x%08x\n", CDMA_FC_CFG, sysRegRead(CDMA_FC_CFG));
	printk("GDMA1_FC_CFG(0x%08x)  : 0x%08x\n", GDMA1_FC_CFG, sysRegRead(GDMA1_FC_CFG));
	printk("FE_GLO_CFG(0x%08x)  : 0x%08x\n", FE_GLO_CFG, sysRegRead(FE_GLO_CFG));
	printk("GDMA1_SHPR_CFG(0x%08x)  : 0x%08x\n", GDMA1_SHPR_CFG, sysRegRead(GDMA1_SHPR_CFG));
	printk("GDMA2_SHPR_CFG(0x%08x)  : 0x%08x\n", GDMA2_SHPR_CFG, sysRegRead(GDMA2_SHPR_CFG));
	printk("PDMA_FC_CFG(0x%08x)  : 0x%08x\n", PDMA_FC_CFG, sysRegRead(PDMA_FC_CFG));
	printk("PDMA_SCH_CFG(0x%08x)  : 0x%08x\n", PDMA_SCH_CFG, sysRegRead(PDMA_SCH_CFG));
	
	printk("\n-----\nRX Counters:\n");
	printk("GDMA_RX_GBCNT0(0x%08x)     : 0x%08x\n", GDMA_RX_GBCNT0, sysRegRead(GDMA_RX_GBCNT0));
	printk("GDMA_RX_GPCNT0(0x%08x)     : 0x%08x\n", GDMA_RX_GPCNT0, sysRegRead(GDMA_RX_GPCNT0));
	printk("GDMA_RX_OERCNT0(0x%08x)    : 0x%08x\n", GDMA_RX_OERCNT0, sysRegRead(GDMA_RX_OERCNT0));
	printk("GDMA_RX_FERCNT0(0x%08x)     : 0x%08x\n", GDMA_RX_FERCNT0, sysRegRead(GDMA_RX_FERCNT0));
	printk("GDMA_RX_SERCNT0(0x%08x)     : 0x%08x\n", GDMA_RX_SERCNT0, sysRegRead(GDMA_RX_SERCNT0));
	printk("GDMA_RX_LERCNT0(0x%08x)     : 0x%08x\n", GDMA_RX_LERCNT0, sysRegRead(GDMA_RX_LERCNT0));
	printk("GDMA_RX_CERCNT0(0x%08x)     : 0x%08x\n\n", GDMA_RX_CERCNT0, sysRegRead(GDMA_RX_CERCNT0));	

#ifdef CONFIG_RAETH_ROUTER
	// just for debug
	printk("The current PHY address selected by ethtool is %d\n", get_current_phy_address());
#endif
}

void dump_cp0(void)
{
	printk("CP0 Register dump --\n");
	printk("CP0_INDEX\t: 0x%08x\n", read_32bit_cp0_register(CP0_INDEX));
	printk("CP0_RANDOM\t: 0x%08x\n", read_32bit_cp0_register(CP0_RANDOM));
	printk("CP0_ENTRYLO0\t: 0x%08x\n", read_32bit_cp0_register(CP0_ENTRYLO0));
	printk("CP0_ENTRYLO1\t: 0x%08x\n", read_32bit_cp0_register(CP0_ENTRYLO1));
	printk("CP0_CONF\t: 0x%08x\n", read_32bit_cp0_register(CP0_CONF));
	printk("CP0_CONTEXT\t: 0x%08x\n", read_32bit_cp0_register(CP0_CONTEXT));
	printk("CP0_PAGEMASK\t: 0x%08x\n", read_32bit_cp0_register(CP0_PAGEMASK));
	printk("CP0_WIRED\t: 0x%08x\n", read_32bit_cp0_register(CP0_WIRED));
	printk("CP0_INFO\t: 0x%08x\n", read_32bit_cp0_register(CP0_INFO));
	printk("CP0_BADVADDR\t: 0x%08x\n", read_32bit_cp0_register(CP0_BADVADDR));
	printk("CP0_COUNT\t: 0x%08x\n", read_32bit_cp0_register(CP0_COUNT));
	printk("CP0_ENTRYHI\t: 0x%08x\n", read_32bit_cp0_register(CP0_ENTRYHI));
	printk("CP0_COMPARE\t: 0x%08x\n", read_32bit_cp0_register(CP0_COMPARE));
	printk("CP0_STATUS\t: 0x%08x\n", read_32bit_cp0_register(CP0_STATUS));
	printk("CP0_CAUSE\t: 0x%08x\n", read_32bit_cp0_register(CP0_CAUSE));
	printk("CP0_EPC\t: 0x%08x\n", read_32bit_cp0_register(CP0_EPC));
	printk("CP0_PRID\t: 0x%08x\n", read_32bit_cp0_register(CP0_PRID));
	printk("CP0_CONFIG\t: 0x%08x\n", read_32bit_cp0_register(CP0_CONFIG));
	printk("CP0_LLADDR\t: 0x%08x\n", read_32bit_cp0_register(CP0_LLADDR));
	printk("CP0_WATCHLO\t: 0x%08x\n", read_32bit_cp0_register(CP0_WATCHLO));
	printk("CP0_WATCHHI\t: 0x%08x\n", read_32bit_cp0_register(CP0_WATCHHI));
	printk("CP0_XCONTEXT\t: 0x%08x\n", read_32bit_cp0_register(CP0_XCONTEXT));
	printk("CP0_FRAMEMASK\t: 0x%08x\n", read_32bit_cp0_register(CP0_FRAMEMASK));
	printk("CP0_DIAGNOSTIC\t: 0x%08x\n", read_32bit_cp0_register(CP0_DIAGNOSTIC));
	printk("CP0_DEBUG\t: 0x%08x\n", read_32bit_cp0_register(CP0_DEBUG));
	printk("CP0_DEPC\t: 0x%08x\n", read_32bit_cp0_register(CP0_DEPC));
	printk("CP0_PERFORMANCE\t: 0x%08x\n", read_32bit_cp0_register(CP0_PERFORMANCE));
	printk("CP0_ECC\t: 0x%08x\n", read_32bit_cp0_register(CP0_ECC));
	printk("CP0_CACHEERR\t: 0x%08x\n", read_32bit_cp0_register(CP0_CACHEERR));
	printk("CP0_TAGLO\t: 0x%08x\n", read_32bit_cp0_register(CP0_TAGLO));
	printk("CP0_TAGHI\t: 0x%08x\n", read_32bit_cp0_register(CP0_TAGHI));
	printk("CP0_ERROREPC\t: 0x%08x\n", read_32bit_cp0_register(CP0_ERROREPC));
	printk("CP0_DESAVE\t: 0x%08x\n\n", read_32bit_cp0_register(CP0_DESAVE));
}

static struct proc_dir_entry *procRegDir, *procGmac, *procSysCP0;

int RegReadMain(void)
{
	dump_reg();
	return 0;
}

int CP0RegRead(void)
{
	dump_cp0();
	return 0;
}

#if defined(CONFIG_RAETH_QOS)
static struct proc_dir_entry *procRaQOS;
int RaQOSRegRead(void)
{
	dump_qos();
	return 0;
}
#endif

#if defined (CONFIG_RALINK_RT3052) && defined (CONFIG_RT_3052_ESW_PORT_STATUS_INT)
static struct proc_dir_entry *procLinkProcPid;

int LinkProcPidRead (char * buf, char **start, off_t offset, int len, int * eof, void * data)
{
	char *p = buf;

	if (linkUpProcPid > 0) {
		p += sprintf(p, "%lu\n", linkUpProcPid);
	}
	else p += sprintf(p, "n/a\n");
	*eof = 1;
	return p - buf;
}

int LinkProcPidWrite(struct file * file, const char * buf, unsigned long count, void * data)
{
	
	linkUpProcPid = simple_strtoul(buf, NULL, 10);
	printk("Link Up Process PID:%lu\n",linkUpProcPid);

	return count;
}
#endif
/*
 * proc write procedure
 */
static int change_phyid(struct file *file, const char *buffer, unsigned long count, void *data)
{
	char buf[32];
	struct net_device *cur_dev_p;
	END_DEVICE *ei_local;

	for(cur_dev_p=dev_base; cur_dev_p!=NULL; cur_dev_p=cur_dev_p->next){
		if (strncmp(cur_dev_p->name, DEV_NAME /* "eth2" usually */, 4) == 0)
			break;
	}
	if (cur_dev_p == NULL)
		return -EFAULT;
	ei_local = cur_dev_p->priv;

	if (count > 32)
		count = 32;
	memset(buf, 0, 32);
	if (copy_from_user(buf, buffer, count))
		return -EFAULT;
	ei_local->mii_info.phy_id = (unsigned char)(simple_strtol(buf, 0, 10));
	return count;
}

int debug_proc_init(void)
{
    procRegDir = proc_mkdir(PROCREG_DIR, NULL);
	
    if ((procGmac = create_proc_entry(PROCREG_GMAC, 0, procRegDir))){
	 procGmac->read_proc = (read_proc_t*)&RegReadMain;
	 procGmac->write_proc = (write_proc_t*)&change_phyid;
	}

    if ((procSysCP0 = create_proc_entry(PROCREG_CP0, 0, procRegDir)))
	 procSysCP0->read_proc = (read_proc_t*)&CP0RegRead;
     
#if defined(CONFIG_RAETH_QOS)
    if ((procRaQOS = create_proc_entry(PROCREG_RAQOS, 0, procRegDir)))
	 procRaQOS->read_proc = (read_proc_t*)&RaQOSRegRead;
#endif
#if defined (CONFIG_RALINK_RT3052) && defined (CONFIG_RT_3052_ESW_PORT_STATUS_INT)
	if ((procLinkProcPid = create_proc_entry(PROCREG_LINKPROCPID, 0, procRegDir))) {
	procLinkProcPid->read_proc = (read_proc_t*)&LinkProcPidRead;
	procLinkProcPid->write_proc = (write_proc_t*)&LinkProcPidWrite;
	}
#endif

    return 0;
}

void debug_proc_exit(void)
{

    if (procSysCP0)
    	remove_proc_entry(PROCREG_CP0, procRegDir);

    if (procGmac)
    	remove_proc_entry(PROCREG_GMAC, procRegDir);
    
#if defined(CONFIG_RAETH_QOS)
    if (procRaQOS)
    	remove_proc_entry(PROCREG_RAQOS, procRegDir);
#endif
#if defined (CONFIG_RALINK_RT3052) && defined (CONFIG_RT_3052_ESW_PORT_STATUS_INT)
	if (procLinkProcPid)
		remove_proc_entry(PROCREG_LINKPROCPID, procRegDir);
#endif
    if (procRegDir)
	remove_proc_entry(PROCREG_DIR, 0);
	

    printk(KERN_ALERT "proc exit\n");
}

#endif
