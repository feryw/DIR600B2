#include <linux/init.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <asm/rt2880/surfboardint.h>
#include <linux/pci.h>
#include <linux/delay.h>
#include "pcm_ctrl.h"
//#include "./codec_lib/lib_g711/g711.h"
#include "../ralink_gpio.h"
#include "../spi_drv.h"
#include "./si3220_rev1.5/dual_io.h"
#include "./si3220_rev1.5/dual.h"

#include "../ralink_gdma.h"

pcm_config_type* ppcm_config;
pcm_status_type* ppcm_status;
static short drop_buf[PCM_PAGE_SAMPLES];
static short proc_buf[MAX_PCM_CH][PCM_PAGE_SAMPLES];
static short zero_buf[PCM_PAGE_SAMPLES];

int pcm_reg_setup(pcm_config_type* ptrpcm_config); 
int pcm_enable(unsigned int chid, pcm_config_type* ptrpcm_config);
int pcm_disable(unsigned int chid, pcm_config_type* ptrpcm_config);

void pcm_dma_tx_isr(u32 chid);
void pcm_dma_rx_isr(u32 chid);
void pcm_unmask_isr(u32 dma_ch);
void pcm_reset_slic ();

static irqreturn_t pcm_irq_isr(int irq, void *irqaction);

struct file_operations pcmdrv_fops = {
    ioctl:      pcm_ioctl,
};
static int pcmdrv_major =  233;
static dma_addr_t TxPage0, TxPage1;
static dma_addr_t RxPage0, RxPage1;

#ifdef PCM_TASKLET
struct tasklet_struct pcm_rx_tasklet;
struct tasklet_struct pcm_tx_tasklet;
#endif

#ifdef D2_TECH_MODIFICATION
void (*pcm_data_callback)(short *, int *) = NULL;
void (*pcm_isrSem_callback)(void) = NULL;
void pcm_data_register_callback(
        void (*pcm_callback)( short *, int *));
void pcm_isrSem_register_callback(
        void (*pcm_callback)(void));

/*
 * ======== dsp_register_isrX_callback() ========
 */
void pcm_data_register_callback(void (*pcm_callback)(short *, int *))
{
	pcm_data_callback=pcm_callback;
}
EXPORT_SYMBOL(pcm_data_register_callback);

/*
 * ======== dsp_register_isrSem_callback() ========
 */
void pcm_isrSem_register_callback(void (*pcm_callback)(void))
{
	pcm_isrSem_callback = pcm_callback;
}
EXPORT_SYMBOL(pcm_isrSem_register_callback);

static void pcm_start(void);
#endif

int slic_type = 3220;
//struct tasklet_struct phone_tasklet;
//extern int phone_task(unsigned long pData);

codec_data_type codec_obj[MAX_CODEC_CH];

int __init pcm_init(void)
{
	int i;
	int result=0;
	result = register_chrdev(pcmdrv_major, "PCM", &pcmdrv_fops);
	if (result < 0) {
		printk(KERN_WARNING "pcm_drv: can't get major %d\n",pcmdrv_major);
	return result;
	}
	MSG("pcm_init()\n");
#ifndef D2_TECH_MODIFICATION
	pcm_open();
#endif
	
	return 0;
}

void pcm_exit(void)
{
	pcm_close();
	unregister_chrdev(pcmdrv_major, "PCM");
	return 0;
}

int pcm_open()
{
	int i, j, result, data, cnt0, cnt1, flags;
	
	/* set pcm_config */
	ppcm_config = (pcm_config_type*)kmalloc(sizeof(pcm_config_type), GFP_KERNEL);
	if(ppcm_config==NULL)
		return PCM_OUTOFMEM;
	memset(ppcm_config, 0, sizeof(pcm_config_type));

#ifdef PCM_STATISTIC
	ppcm_status = (pcm_status_type*)kmalloc(sizeof(pcm_status_type), GFP_KERNEL);
	if (ppcm_status==NULL) {
		return PCM_OUTOFMEM;
    }
	memset(ppcm_status, 0, sizeof(pcm_status_type));
#endif
	
	ppcm_config->pcm_ch_num = CONFIG_PCM_CH;
	ppcm_config->codec_ch_num = MAX_CODEC_CH;
	ppcm_config->nch_active = 0;
	ppcm_config->extclk_en = CONFIG_PCM_EXT_CLK_EN;
	ppcm_config->clkout_en = CONFIG_PCM_CLKOUT_EN;
	ppcm_config->ext_fsync = CONFIG_PCM_EXT_FSYNC;
	ppcm_config->long_fynsc = CONFIG_PCM_LONG_FSYNC;
	ppcm_config->fsync_pol = CONFIG_PCM_FSYNC_POL;
	ppcm_config->drx_tri = CONFIG_PCM_DRX_TRI;
	ppcm_config->slot_mode = CONFIG_PCM_SLOTMODE;
	ppcm_config->tff_thres = CONFIG_PCM_TFF_THRES;
	ppcm_config->rff_thres = CONFIG_PCM_RFF_THRES;
		
	for ( i = 0 ; i < ppcm_config->pcm_ch_num; i ++ )
	{
		ppcm_config->lbk[i] = CONFIG_PCM_LBK;
		ppcm_config->ext_lbk[i] = CONFIG_PCM_EXT_LBK;
		ppcm_config->cmp_mode[i] = CONFIG_PCM_CMP_MODE;
		ppcm_config->ts_start[i] = CONFIG_PCM_TS_START + i*16;	
		ppcm_config->txfifo_rd_idx[i] = 0;
		ppcm_config->txfifo_wt_idx[i] = 0;
		ppcm_config->rxfifo_rd_idx[i] = 0;
		ppcm_config->rxfifo_wt_idx[i] = 0;
		ppcm_config->bsfifo_rd_idx[i] = 0;
		ppcm_config->bsfifo_wt_idx[i] = 0;

	}

#ifdef D2_TECH_MODIFICATION
	ppcm_config->ts_start[0] = 1;
	ppcm_config->ts_start[1] = 17;
#endif

	MSG("allocate fifo buffer\n");
	/* allocate fifo buffer */
	for ( i = 0 ; i < ppcm_config->pcm_ch_num; i ++ ) {
		ppcm_config->TxFIFOBuf16Ptr[i] = kmalloc(PCM_FIFO_SIZE, GFP_KERNEL);
		if (ppcm_config->TxFIFOBuf16Ptr[i]==NULL) {
			pcm_close();
			return PCM_OUTOFMEM;
		}

		ppcm_config->RxFIFOBuf16Ptr[i] = kmalloc(PCM_FIFO_SIZE, GFP_KERNEL);
		if (ppcm_config->RxFIFOBuf16Ptr[i]==NULL) {
			pcm_close();
			return PCM_OUTOFMEM;
		}
		
#ifdef PCM_SW_G729AB		
		ppcm_config->BSFIFOBuf16Ptr[i] = kmalloc(PCM_BSFIFO_SIZE, GFP_KERNEL);
		if (ppcm_config->BSFIFOBuf16Ptr[i]==NULL) {
			pcm_close();
			return PCM_OUTOFMEM;
		}
#endif		
	}
	MSG("allocate page buffer\n");
	/* allocate page buffer */
	for ( i = 0 ; i < ppcm_config->pcm_ch_num; i ++ ) {
		ppcm_config->TxPage0Buf16Ptr[i] = pci_alloc_consistent(NULL, PCM_PAGE_SIZE , &TxPage0);
		if(ppcm_config->TxPage0Buf16Ptr[i]==NULL)
		{
			pcm_close();
			return PCM_OUTOFMEM;
		}
		ppcm_config->TxPage1Buf16Ptr[i] = pci_alloc_consistent(NULL, PCM_PAGE_SIZE , &TxPage1);
		if(ppcm_config->TxPage1Buf16Ptr[i]==NULL)
		{
			pcm_close();
			return PCM_OUTOFMEM;
		}
		ppcm_config->RxPage0Buf16Ptr[i] = pci_alloc_consistent(NULL, PCM_PAGE_SIZE , &RxPage0);
		if(ppcm_config->RxPage0Buf16Ptr[i]==NULL)
		{
			pcm_close();
			return PCM_OUTOFMEM;
		}
		ppcm_config->RxPage1Buf16Ptr[i] = pci_alloc_consistent(NULL, PCM_PAGE_SIZE , &RxPage1);
		if(ppcm_config->RxPage1Buf16Ptr[i]==NULL)
		{
			pcm_close();
			return PCM_OUTOFMEM;
		}
	}
	

	/* PCM controller reset */

PCM_RESET:	
	
	data = pcm_inw(RALINK_SYSCTL_BASE+0x34);
	data |= 0x00000800;
	pcm_outw(RALINK_SYSCTL_BASE+0x34,data);
	data = pcm_inw(RALINK_SYSCTL_BASE+0x34);
	data &= 0xFFFFF7FF;
	pcm_outw(RALINK_SYSCTL_BASE+0x34,data);
	for(i=0;i<100000;i++);
	
	data = pcm_inw(RALINK_SYSCTL_BASE+0x34);
	data |= 0x00040000;
	pcm_outw(RALINK_SYSCTL_BASE+0x34, data);
	data = pcm_inw(RALINK_SYSCTL_BASE+0x34);
	data &= 0xFFFBFFFF;
	pcm_outw(RALINK_SYSCTL_BASE+0x34,data);
	/* PCM controller CLK_DIV set */
	data = pcm_inw(RALINK_SYSCTL_BASE+0x30);

#ifdef PCM_IN_CLK
	data &= ~REGBIT(1, PCM_CLK_SEL);
	data |= REGBIT(1, PCM_CLK_EN);	
	data &= 0xFFFFFFC0;
#if	defined(CONFIG_RT3052_FPGA)
	data |= REGBIT(48, PCM_CLK_DIV);	/* Assume internal clock = 12.5Mhz */
#else
	data |= REGBIT(60, PCM_CLK_DIV);    /* Assume internal clock = 15.625Mhz */
#endif
	data |= 0x00000080;
#else	
	data |= REGBIT(1, PCM_CLK_SEL);
	data |= REGBIT(1, PCM_CLK_EN);
	data &= 0xFFFFFFC0;
	data |= REGBIT(46, PCM_CLK_DIV);	/* Assume REF_CLK = 12Mhz */
#endif	
	
	pcm_outw(RALINK_SYSCTL_BASE+0x30, data);
	MSG("RALINK_SYSCTL_BASE+0x30=0x%08X\n",data);
	
	data = pcm_inw(RALINK_REG_GPIOMODE); 
	data &= 0xFFFFFFE1;
	data |= 0x00000010;
	pcm_outw(RALINK_REG_GPIOMODE, data);
	MSG("RALINK_REG_GPIOMODE=0x%08X\n",data);

	if(pcm_reg_setup(ppcm_config)!=PCM_OK)
		MSG("PCM:pcm_reg_setup() failed\n");


	spin_lock_irqsave(&ppcm_config->lock, flags);

    data = pcm_inw(PCM_GLBCFG);
    data |= REGBIT(0x1, PCM_EN);
    pcm_outw(PCM_GLBCFG, data);
		
	MSG("SLIC %d reset....\n",slic_type);
	
	pcm_reset_slic();
#ifdef D2_TECH_MODIFICATION
	spin_unlock_irqrestore(&ppcm_config->lock, flags);
	pcm_start();
#else
	if(slic_type==3220)
	{
		if(si3220_init()!=0)
		{
			for ( i = 0 ; i < ppcm_config->pcm_ch_num ; i ++ )
				pcm_disable(i, ppcm_config);
				
			spin_unlock_irqrestore(&ppcm_config->lock, flags);	
			goto PCM_RESET;
		}
	}
	else if(slic_type==3210)
	{
		if(si3210_init()==0)
		{
			for ( i = 0 ; i < ppcm_config->pcm_ch_num ; i ++ )
				pcm_disable(i, ppcm_config);
				
			spin_unlock_irqrestore(&ppcm_config->lock, flags);	
			goto PCM_RESET;
		}
	}
	else
	{
		MSG("slic type not supported\n");
		for ( i = 0 ; i < ppcm_config->pcm_ch_num ; i ++ )
				pcm_disable(i, ppcm_config);
				
		spin_unlock_irqrestore(&ppcm_config->lock, flags);	
		goto PCM_RESET;
	}
	spin_unlock_irqrestore(&ppcm_config->lock, flags);
	
#ifdef PCM_TASKLET
	tasklet_init(&pcm_rx_tasklet, pcm_rx_task, ppcm_config);
	tasklet_init(&pcm_tx_tasklet, pcm_tx_task, ppcm_config);
	MSG("pcm tasklet initialization\n");
#endif	
#endif	// D2_TECH_MODIFICATION
	//data = pcm_inw(PCM_GLBCFG);
	//data |= REGBIT(0x1, DMA_EN);
	//pcm_outw(PCM_GLBCFG, data);

	MSG("pcm_open done...\n");
	
	//tasklet_init(&phone_tasklet, phone_task, NULL);
	//MSG("phone tasklet initialization\n");
	//tasklet_hi_schedule(&phone_tasklet);



	return PCM_OK;
}

int pcm_reg_setup(pcm_config_type* ptrpcm_config)
{
	unsigned int data = 0;
	
	/* set PCMCFG */
	MSG("pcm_reg_setup\n");
	data |= REGBIT(ptrpcm_config->extclk_en, PCM_EXT_CLK_EN);
	data |= REGBIT(ptrpcm_config->clkout_en,  PCM_CLKOUT);
	
	data |= REGBIT(ptrpcm_config->ext_fsync, PCM_EXT_FSYNC);
	data |= REGBIT(ptrpcm_config->long_fynsc, PCM_LONG_FSYNC);
	data |= REGBIT(ptrpcm_config->fsync_pol, PCM_FSYNC_POL);
	data |= REGBIT(ptrpcm_config->drx_tri, PCM_DRX_TRI);
	data |= REGBIT(ptrpcm_config->slot_mode, PCM_SLOTMODE);
	MSG("pcm_reg_setup:PCM_PCMCFG=%X\n",data);
	pcm_outw(PCM_PCMCFG, data);

	/* set GLBCFG's threshold fields */

	data = 0;
	data |= REGBIT(ptrpcm_config->tff_thres, TFF_THRES);
	data |= REGBIT(ptrpcm_config->rff_thres, RFF_THRES);
	MSG("PCM_GLBCFG=%X\n",data);
	pcm_outw(PCM_GLBCFG, data);

	/* set CH0/1_CFG */
	
	data = 0;
	data |= REGBIT(ptrpcm_config->lbk[0], PCM_LBK);
	data |= REGBIT(ptrpcm_config->ext_lbk[0], PCM_EXT_LBK);
	data |= REGBIT(ptrpcm_config->cmp_mode[0], PCM_CMP_MODE);
	data |= REGBIT(ptrpcm_config->ts_start[0], PCM_TS_START);
	MSG("PCM_CH0_CFG=%X\n",data);
	pcm_outw(PCM_CH0_CFG, data);

	data = 0;
	data |= REGBIT(ptrpcm_config->lbk[1], PCM_LBK);
	data |= REGBIT(ptrpcm_config->ext_lbk[1], PCM_EXT_LBK);
	data |= REGBIT(ptrpcm_config->cmp_mode[1], PCM_CMP_MODE);
	data |= REGBIT(ptrpcm_config->ts_start[1], PCM_TS_START);
	MSG("PCM_CH1_CFG=%X\n",data);
	pcm_outw(PCM_CH1_CFG, data);


	return PCM_OK;
}
int pcm_close()
{
	int i;
		
	MSG("pcm_close\n");	
#ifndef D2_TECH_MODIFICATION
#ifdef PCM_TASKLET
	tasklet_kill(&pcm_rx_tasklet);
	tasklet_kill(&pcm_tx_tasklet);
#endif	
#endif

#ifdef D2_TECH_MODIFICATION
	pcm_disable(0, ppcm_config);
#else
	for( i = 0 ; i < ppcm_config->pcm_ch_num ; i ++ )
		pcm_disable(i, ppcm_config);
#endif
	
#ifdef PCM_STATISTIC
	kfree(ppcm_status);
#endif
	
	/* free buffer */
	for( i = 0 ; i < ppcm_config->pcm_ch_num ; i ++ )
	{
		if(ppcm_config->TxPage0Buf16Ptr[i])
			pci_free_consistent(NULL, PCM_PAGE_SIZE, TxPage0, ppcm_config->TxPage0Buf16Ptr[i]);
		if(ppcm_config->TxPage1Buf16Ptr[i])
			pci_free_consistent(NULL, PCM_PAGE_SIZE, TxPage1, ppcm_config->TxPage1Buf16Ptr[i]);	
		if(ppcm_config->RxPage0Buf16Ptr[i])
			pci_free_consistent(NULL, PCM_PAGE_SIZE, RxPage0, ppcm_config->RxPage0Buf16Ptr[i]);
		if(ppcm_config->RxPage1Buf16Ptr[i])
			pci_free_consistent(NULL, PCM_PAGE_SIZE, RxPage1, ppcm_config->RxPage1Buf16Ptr[i]);					
		if(ppcm_config->TxFIFOBuf16Ptr[i])
			kfree(ppcm_config->TxFIFOBuf16Ptr[i]);	
		if(ppcm_config->RxFIFOBuf16Ptr[i])
			kfree(ppcm_config->RxFIFOBuf16Ptr[i]);
#ifdef PCM_SW_G729AB			
		if(ppcm_config->BSFIFOBuf16Ptr[i])
			kfree(ppcm_config->BSFIFOBuf16Ptr[i]);
#endif						
	}

	kfree(ppcm_config);
	ppcm_config = NULL;
	
	return PCM_OK;
}

int pcm_enable(unsigned int chid, pcm_config_type* ptrpcm_config)
{
	unsigned int GLBCFG_Data=0, int_en, Ret,i;
	char* p8Data;
	
	if(ptrpcm_config->nch_active>=ptrpcm_config->pcm_ch_num)
	{
		MSG("There are %d channels already enabled\n",ptrpcm_config->nch_active);
		return PCM_OK;
	}
	int_en = pcm_inw(PCM_INT_EN);
	GLBCFG_Data = pcm_inw(PCM_GLBCFG);

	pcm_outw(PCM_INT_STATUS, 0x0);
	
	switch(chid)
	{
		case 0:
			MSG("PCM:enable CH0\n");
			GLBCFG_Data |= REGBIT(0x1, CH0_TX_EN);
			GLBCFG_Data |= REGBIT(0x1, CH0_RX_EN);
			
			int_en |= REGBIT(0x1, CH0T_DMA_FAULT);
			int_en |= REGBIT(0x1, CH0R_DMA_FAULT);
			
			//int_en |= REGBIT(0x1, CH0T_OVRUN);
			//int_en |= REGBIT(0x1, CH0T_UNRUN);
			//int_en |= REGBIT(0x1, CH0R_OVRUN);
			//int_en |= REGBIT(0x1, CH0R_UNRUN);
			
			//int_en |= REGBIT(0x1, CH0T_THRES);
			//int_en |= REGBIT(0x1, CH0R_THRES);
			ptrpcm_config->nch_active++;
			break;
		case 1:
			MSG("PCM:enable CH1\n");

			GLBCFG_Data |= REGBIT(0x1, CH1_TX_EN);
			GLBCFG_Data |= REGBIT(0x1, CH1_RX_EN);
			
			int_en |= REGBIT(0x1, CH1T_DMA_FAULT);
			int_en |= REGBIT(0x1, CH1R_DMA_FAULT);
			
			//int_en |= REGBIT(0x1, CH1T_OVRUN);
			//int_en |= REGBIT(0x1, CH1T_UNRUN);
			//int_en |= REGBIT(0x1, CH1R_OVRUN);
			//int_en |= REGBIT(0x1, CH1R_UNRUN);

			//int_en |= REGBIT(0x1, CH1T_THRES);
			//int_en |= REGBIT(0x1, CH1R_THRES);
			ptrpcm_config->nch_active++;
			break;
		default:
			break;
	}

	GLBCFG_Data |= REGBIT(0x1, PCM_EN);
	pcm_outw(PCM_INT_EN, int_en);
	pcm_outw(PCM_GLBCFG, GLBCFG_Data);
	
	return PCM_OK;
}

int pcm_disable(unsigned int chid, pcm_config_type* ptrpcm_config)
{
	unsigned int data, int_en;

	if(ptrpcm_config->nch_active<=0)
	{ 
		MSG("No channels needed to disable\n");
		return PCM_OK;
	}
	ppcm_config->txfifo_rd_idx[chid] = 0;
	ppcm_config->txfifo_wt_idx[chid] = 0;
	ppcm_config->rxfifo_rd_idx[chid] = 0;
	ppcm_config->rxfifo_wt_idx[chid] = 0;
	ppcm_config->bsfifo_rd_idx[chid] = 0;
	ppcm_config->bsfifo_wt_idx[chid] = 0;
	
	int_en = pcm_inw(PCM_INT_EN);
	data = pcm_inw(PCM_PCMCFG);
	
	switch(chid)
	{
		case 0:
			MSG("PCM:disable CH0\n");
			data &= ~REGBIT(0x1, CH0_TX_EN);
			data &= ~REGBIT(0x1, CH0_RX_EN);
			int_en &= ~REGBIT(0x1, CH0T_DMA_FAULT);
			int_en &= ~REGBIT(0x1, CH0R_DMA_FAULT);
			pcm_outw(PCM_INT_EN, int_en);
			ptrpcm_config->nch_active--;

			break;
		case 1:
			MSG("PCM:disable CH1\n");
			data &= ~REGBIT(0x1, CH1_TX_EN);
			data &= ~REGBIT(0x1, CH1_RX_EN);
			int_en &= ~REGBIT(0x1, CH1T_DMA_FAULT);
			int_en &= ~REGBIT(0x1, CH1R_DMA_FAULT);
			pcm_outw(PCM_INT_EN, int_en);
			ptrpcm_config->nch_active--;

			break;
		default:
			break;
	}
	if(ptrpcm_config->nch_active<=0)
	{
		data &= ~REGBIT(0x1, PCM_EN);
		data &= ~REGBIT(0x1, DMA_EN);
	}
	pcm_outw(PCM_PCMCFG, data);
	return PCM_OK;
}

void pcm_dma_tx_isr(u32 dma_ch)
{
	int i,j;
	int chid=0;
	int page=0;
	int value;
	char* p8PageBuf=NULL, *p8FIFOBuf=NULL, *p8Data;
	u32 pcm_status,dma_status=0;
	u32* pPCM_FIFO=NULL;

#ifndef D2_TECH_MODIFICATION
	if((ppcm_config->tx_isr_cnt%170==11)||(ppcm_config->tx_isr_cnt%170==90))
	{
		dma_status=pcm_inw(RALINK_GDMAISTS);
		i= pcm_inw(PCM_GLBCFG);
		pcm_status=pcm_inw(PCM_INT_STATUS);
		//printk("ti c=%d %X %X tc=%d\n",dma_ch,pcm_status,i, ppcm_config->tx_isr_cnt);	
	}
#endif

	ppcm_config->tx_isr_cnt++;
	if((dma_ch==GDMA_PCM0_TX0)||(dma_ch==GDMA_PCM0_TX1))
	{
		chid = 0;
		pPCM_FIFO = PCM_CH0_FIFO;
		
	}
	else if((dma_ch==GDMA_PCM1_TX0)||(dma_ch==GDMA_PCM1_TX1))
	{
		chid = 1;
		pPCM_FIFO = PCM_CH1_FIFO;
	}
	else
	{
		printk("PCM ERR : tx dma channel number is illeagle\n");
	}
	
	if((dma_ch==GDMA_PCM0_TX0)||(dma_ch==GDMA_PCM1_TX0))
	{
		page = 0;
		p8PageBuf = ppcm_config->TxPage0Buf8Ptr[chid];
	}
	if((dma_ch==GDMA_PCM0_TX1)||(dma_ch==GDMA_PCM1_TX1))
	{
		page = 1;
		p8PageBuf = ppcm_config->TxPage1Buf8Ptr[chid];
	}
	
	p8FIFOBuf = ppcm_config->TxFIFOBuf8Ptr[chid];
	ppcm_config->pos = 0;

#ifdef D2_TECH_MODIFICATION
	if (NULL != pcm_data_callback) {
		pcm_data_callback(p8PageBuf, 1);
	}
	GdmaPcmTx((char *) p8PageBuf, pPCM_FIFO, chid, page, PCM_PAGE_SIZE, pcm_dma_tx_isr, pcm_unmask_isr);
	ppcm_config->txcurchid = chid;
#else
	if(ppcm_config->tx_isr_cnt>4)
	{
	
		for (i = 0 ; i < MAX_PCM_PROC_UNIT ; i ++ )
		{
			if((ppcm_config->txfifo_rd_idx[chid]%MAX_PCM_FIFO)==ppcm_config->txfifo_wt_idx[chid])
			{
				/* tx fifo empty */
				printk("TFE[%d](%d) \n",chid,dma_ch);
				break;
			}
			
			p8Data = p8FIFOBuf + (ppcm_config->txfifo_rd_idx[chid]*PCM_8KHZ_SAMPLES*PCM_SAMPLE_SIZE);
			memcpy((void*)(p8PageBuf+ppcm_config->pos), p8Data, PCM_8KHZ_SAMPLES*PCM_SAMPLE_SIZE);	
			
			ppcm_config->pos+=PCM_8KHZ_SAMPLES*PCM_SAMPLE_SIZE;
			ppcm_config->txfifo_rd_idx[chid] = (ppcm_config->txfifo_rd_idx[chid]+1)%MAX_PCM_FIFO;
		}
	}
	
	GdmaPcmTx((char*)p8PageBuf, pPCM_FIFO, chid, page, PCM_PAGE_SIZE, pcm_dma_tx_isr, pcm_unmask_isr);		

	ppcm_config->txcurchid = chid;
	
	if(ppcm_config->tx_isr_cnt%(CONFIG_PCM_CH*2)==0)
		tasklet_hi_schedule(&pcm_tx_tasklet);	
#endif	// D2_TECH_MODIFICATION	
	return;
	
}

void pcm_dma_rx_isr(u32 dma_ch)
{
	int i,j;
	int chid=0; 
	int page=0;

	char* p8PageBuf=NULL, *p8FIFOBuf=NULL, *p8Data;

	u32 pcm_status=0,dma_status=0;
	u32* pPCM_FIFO=NULL;
	
	if ((dma_ch==GDMA_PCM0_RX0)||(dma_ch==GDMA_PCM0_RX1)) {
		chid = 0;
		pPCM_FIFO = PCM_CH0_FIFO;	
	}
	else if ((dma_ch==GDMA_PCM1_RX0)||(dma_ch==GDMA_PCM1_RX1)) {
		chid = 1;
		pPCM_FIFO = PCM_CH1_FIFO;
		printk("OL %s %d \n", __FILE__, __LINE__);
	}
	else
	{
		MSG("PCM ERR : rx dma channel number (CH=%d) is illeagle\n", dma_ch);
	}
	
	
	if((dma_ch==GDMA_PCM0_RX0)||(dma_ch==GDMA_PCM1_RX0))
	{
		page = 0;
		p8PageBuf = (char*)(ppcm_config->RxPage0Buf8Ptr[chid]);
	}
	else if((dma_ch==GDMA_PCM0_RX1)||(dma_ch==GDMA_PCM1_RX1))
	{
		page = 1;
		p8PageBuf = (char*)(ppcm_config->RxPage1Buf8Ptr[chid]);
	}
	else
	{
		MSG("PCM ERR : rx dma channel number (CH=%d) is illeagle\n", dma_ch);
	}
#ifndef D2_TECH_MODIFICATION
	if((ppcm_config->rx_isr_cnt%170==50)||(ppcm_config->rx_isr_cnt%170==129))
	{
		dma_status=pcm_inw(RALINK_GDMAISTS);
		pcm_status=pcm_inw(PCM_INT_STATUS);
		i=pcm_inw(PCM_GLBCFG);
		//printk("ri c=%d %X %X rc=%d\n",dma_ch,pcm_status,i,ppcm_config->rx_isr_cnt);
	}
#endif

	ppcm_config->rx_isr_cnt++;
	p8FIFOBuf = ppcm_config->RxFIFOBuf8Ptr[chid];
	ppcm_config->pos = 0;
	
#ifdef D2_TECH_MODIFICATION
	/*
	 * D2_MOD Begin
	 * VHW ISR function is called here for giving the semaphore
	 */
	if (NULL != pcm_data_callback) {
		pcm_data_callback(p8PageBuf, 0);
	}
	if (NULL != pcm_isrSem_callback) {
		pcm_isrSem_callback();	/* _VHW_pcmIsrSemGive(); */
	}
	/* D2_MOD End */
	GdmaPcmRx(pPCM_FIFO, (char *) p8PageBuf, chid, page, PCM_PAGE_SIZE, pcm_dma_rx_isr, pcm_unmask_isr);
	ppcm_config->curchid = chid;
#else	
	for (i = 0 ; i < MAX_PCM_PROC_UNIT ; i ++ )
	{
		if(((ppcm_config->rxfifo_wt_idx[chid]+1)%MAX_PCM_FIFO)==ppcm_config->rxfifo_rd_idx[chid])
		{
			/* rx fifo full */
			printk("RFF[%d](%d) ",chid,dma_ch);
			break;
		}

		p8Data = p8FIFOBuf + (ppcm_config->rxfifo_wt_idx[chid]*PCM_8KHZ_SAMPLES*PCM_SAMPLE_SIZE);
		memcpy((void*)p8Data, (void*)(p8PageBuf+ppcm_config->pos), PCM_8KHZ_SAMPLES*PCM_SAMPLE_SIZE);
		ppcm_config->pos+=PCM_8KHZ_SAMPLES*PCM_SAMPLE_SIZE;
		ppcm_config->rxfifo_wt_idx[chid] = (ppcm_config->rxfifo_wt_idx[chid]+1)%MAX_PCM_FIFO;
		
	}
	
	GdmaPcmRx(pPCM_FIFO, (char*)p8PageBuf, chid, page, PCM_PAGE_SIZE, pcm_dma_rx_isr, pcm_unmask_isr);

	ppcm_config->curchid = chid;
	if(ppcm_config->rx_isr_cnt%(CONFIG_PCM_CH*2)==1)
		tasklet_hi_schedule(&pcm_rx_tasklet);
#endif		
	return;
}

int pcm_tx_task(unsigned long pData)
{
	pcm_config_type* ptrpcm_config = ppcm_config;
	int txch,rxch,ch;
	short* pBuf; 
	char PCMBuf[PCM_8KHZ_SAMPLES*PCM_SAMPLE_SIZE];
	short* uCompressBuf = PCMBuf;
	short serial[88];
	int i,j, value;
	short* PCM16bitBuf = PCMBuf;
	short* pTx16Data;
	short* pRx16Data;
	char* pTx8Data;
	char* pRx8Data;
	char* p8Data;
	unsigned int flags, data;
	
	/* handle rx->tx fifo buffer */

	for( ch = 0 ; ch < CONFIG_PCM_CH ; ch ++ )
	{
		rxch = ch;
		txch = (CONFIG_PCM_CH-1)-ch;
			
		for (i = 0 ; i < MAX_PCM_PROC_UNIT ; i ++ )
		{
 
			spin_lock_irqsave(&ptrpcm_config->lock, flags);

			if(((ptrpcm_config->txfifo_wt_idx[txch]+1)%MAX_PCM_FIFO)==ptrpcm_config->txfifo_rd_idx[txch])
			{
				/* tx fifo full */
				printk("TTFF(%d) ",txch);
				pTx16Data = NULL;

			}
			else
			{	
				pTx8Data = ptrpcm_config->TxFIFOBuf8Ptr[txch] + (ptrpcm_config->txfifo_wt_idx[txch]*PCM_8KHZ_SAMPLES*PCM_SAMPLE_SIZE);
				ptrpcm_config->txfifo_wt_idx[txch] = (ptrpcm_config->txfifo_wt_idx[txch]+1)%MAX_PCM_FIFO;
			}

			if((ptrpcm_config->rxfifo_rd_idx[rxch]%MAX_PCM_FIFO)==ptrpcm_config->rxfifo_wt_idx[rxch])
			{
				/* rx fifo empty */
				printk("TRFE(%d) ",rxch);
				pRx8Data = NULL;
			}
			else
			{		
				pRx8Data = ptrpcm_config->RxFIFOBuf8Ptr[rxch] + (ptrpcm_config->rxfifo_rd_idx[rxch]*PCM_8KHZ_SAMPLES*PCM_SAMPLE_SIZE);
				ptrpcm_config->rxfifo_rd_idx[rxch] = (ptrpcm_config->rxfifo_rd_idx[rxch]+1)%MAX_PCM_FIFO;
			}	
			pRx16Data = pRx8Data;
			pTx16Data = pTx8Data;
	
			spin_unlock_irqrestore(&ptrpcm_config->lock, flags);

			if((pRx8Data==NULL)||(pTx8Data==NULL))
				continue;

#ifdef PCM_RECORD 
			if(ptrpcm_config->iRecordCH==rxch)
			{
				if(ptrpcm_config->bStartRecord)
				{
					if(ptrpcm_config->mmappos==0)
					{
						if((ptrpcm_config->mmap_wt_idx+1)%MAX_PCM_PAGE!=ptrpcm_config->mmap_rd_idx)
						{
							ptrpcm_config->mmap_wt_idx = (ptrpcm_config->mmap_wt_idx+1)%MAX_PCM_PAGE;
						}
						else
						{
							printk("DR\n");
						}		
						p8Data = ptrpcm_config->mmapbuf+ptrpcm_config->mmap_wt_idx*PCM_PAGE_SIZE;
					}
					memcpy((void*)p8Data, (void*)pRx16Data, PCM_8KHZ_SAMPLES*sizeof(short));
					ptrpcm_config->mmappos+=PCM_8KHZ_SAMPLES*sizeof(short);
					p8Data+=PCM_8KHZ_SAMPLES*sizeof(short);
				}
				
			}
#endif			
			if(codec_obj[rxch].type)
			{
				spin_lock_irqsave(&ptrpcm_config->lock, flags);	
				memcpy((short*)(codec_obj[rxch].pPCMBuf16), pRx16Data, PCM_8KHZ_SAMPLES*sizeof(short));
				voice_encode_frame(&codec_obj[rxch]);
				spin_unlock_irqrestore(&ptrpcm_config->lock, flags);		
				memcpy(codec_obj[txch].pBitBuf, codec_obj[rxch].pBitBuf, codec_obj[rxch].BitBufByteLen);
				spin_lock_irqsave(&ptrpcm_config->lock, flags);	
				voice_decode_frame(&codec_obj[txch]);
				memcpy(pTx16Data, (short*)(codec_obj[txch].pPCMBuf16), PCM_8KHZ_SAMPLES*sizeof(short));
				spin_unlock_irqrestore(&ptrpcm_config->lock, flags);
			}
#ifdef PCM_PLAYBACK 
			if(ptrpcm_config->iPlaybackCH==txch)
			{
				if(ptrpcm_config->bStartPlayback)
				{
					if(ptrpcm_config->mmappos==0)
					{
						if((ptrpcm_config->mmap_rd_idx+1)%MAX_PCM_PAGE!=ptrpcm_config->mmap_wt_idx)
						{
							ptrpcm_config->mmap_rd_idx = (ptrpcm_config->mmap_rd_idx+1)%MAX_PCM_PAGE;
						}
						else
						{
							printk("DR\n");
						}
						p8Data = ptrpcm_config->mmapbuf+ptrpcm_config->mmap_rd_idx*PCM_PAGE_SIZE;
					}
					memcpy((void*)pTx8Data, (void*)p8Data, PCM_8KHZ_SAMPLES*PCM_SAMPLE_SIZE);
					ptrpcm_config->mmappos+=PCM_8KHZ_SAMPLES*PCM_SAMPLE_SIZE;
					p8Data+=PCM_8KHZ_SAMPLES*PCM_SAMPLE_SIZE;
					continue;
				}
				
			}
#endif		
			if(codec_obj[rxch].type==0)
			{
				memcpy((void*)pTx8Data, (void*)pRx8Data, PCM_8KHZ_SAMPLES*PCM_SAMPLE_SIZE);
			}
		}
	
	}

	if((ptrpcm_config->bStartRecord)||(ptrpcm_config->bStartPlayback))
	{
		ptrpcm_config->mmappos = 0;
		wake_up_interruptible(&(ptrpcm_config->pcm_qh));
	}
	return 0;
}

int pcm_rx_task(unsigned long pData)
{
	pcm_config_type* ptrpcm_config = ppcm_config;
	int txch,rxch,ch;
	short* pBuf; 
	char PCMBuf[PCM_8KHZ_SAMPLES*PCM_SAMPLE_SIZE];
	short* uCompressBuf = PCMBuf;
	short serial[88];
	int i,j, value;
	
	short* pTx16Data;
	short* pRx16Data;
	char* pTx8Data;
	char* pRx8Data;
	char* p8Data;
	unsigned int flags, data;
	
	/* handle rx->tx fifo buffer */
	for( ch = 0 ; ch < MAX_PCM_CH ; ch ++ )
	{
		rxch = ch;
		txch = (MAX_PCM_CH-1)-ch;
	
		for (i = 0 ; i < MAX_PCM_PROC_UNIT ; i ++ )
		{	  
			spin_lock_irqsave(&ptrpcm_config->lock, flags);
				
			if(((ptrpcm_config->txfifo_wt_idx[txch]+1)%MAX_PCM_FIFO)==ptrpcm_config->txfifo_rd_idx[txch])
			{
				/* tx fifo full */
				printk("RTFF(%d) ",txch);
				pTx8Data = NULL;
			}
			else
			{	
				pTx8Data = ptrpcm_config->TxFIFOBuf8Ptr[txch] + (ptrpcm_config->txfifo_wt_idx[txch]*PCM_8KHZ_SAMPLES*PCM_SAMPLE_SIZE);
				ptrpcm_config->txfifo_wt_idx[txch] = (ptrpcm_config->txfifo_wt_idx[txch]+1)%MAX_PCM_FIFO;
			}

			if((ptrpcm_config->rxfifo_rd_idx[rxch]%MAX_PCM_FIFO)==ptrpcm_config->rxfifo_wt_idx[rxch])
			{
				/* rx fifo empty */
				printk("RRFE(%d) ",rxch);
				pRx8Data = NULL;
			}
			else
			{		
				pRx8Data = ptrpcm_config->RxFIFOBuf8Ptr[rxch] + (ptrpcm_config->rxfifo_rd_idx[rxch]*PCM_8KHZ_SAMPLES*PCM_SAMPLE_SIZE);
				ptrpcm_config->rxfifo_rd_idx[rxch] = (ptrpcm_config->rxfifo_rd_idx[rxch]+1)%MAX_PCM_FIFO;
			}	
			pTx16Data = pTx8Data;
			pRx16Data = pRx8Data;

			spin_unlock_irqrestore(&ptrpcm_config->lock, flags);

			if((pRx16Data==NULL)||(pTx16Data==NULL))
				continue;
			
#ifdef PCM_RECORD 
			if(ptrpcm_config->iRecordCH==rxch)
			{
				if(ptrpcm_config->bStartRecord)
				{
					if(ptrpcm_config->mmappos==0)
					{
						if((ptrpcm_config->mmap_wt_idx+1)%MAX_PCM_PAGE!=ptrpcm_config->mmap_rd_idx)
						{
							ptrpcm_config->mmap_wt_idx = (ptrpcm_config->mmap_wt_idx+1)%MAX_PCM_PAGE;
						}
						else
						{
							printk("DR\n");
						}
						p8Data = ptrpcm_config->mmapbuf+ptrpcm_config->mmap_wt_idx*PCM_PAGE_SIZE;
					}
					memcpy((void*)p8Data, (void*)pRx8Data, PCM_8KHZ_SAMPLES*PCM_SAMPLE_SIZE);
					ptrpcm_config->mmappos+=PCM_8KHZ_SAMPLES*PCM_SAMPLE_SIZE;
					p8Data+=PCM_8KHZ_SAMPLES*PCM_SAMPLE_SIZE;
				}
				
			}
#endif
			if(codec_obj[rxch].type)
			{
				spin_lock_irqsave(&ptrpcm_config->lock, flags);	
				memcpy((short*)(codec_obj[rxch].pPCMBuf16), pRx16Data, PCM_8KHZ_SAMPLES*sizeof(short));
				voice_encode_frame(&codec_obj[rxch]);
				spin_unlock_irqrestore(&ptrpcm_config->lock, flags);		
				memcpy(codec_obj[txch].pBitBuf, codec_obj[rxch].pBitBuf, codec_obj[rxch].BitBufByteLen);
				spin_lock_irqsave(&ptrpcm_config->lock, flags);	
				voice_decode_frame(&codec_obj[txch]);
				memcpy(pTx16Data, (short*)(codec_obj[txch].pPCMBuf16), PCM_8KHZ_SAMPLES*sizeof(short));
				spin_unlock_irqrestore(&ptrpcm_config->lock, flags);
			}
#ifdef PCM_PLAYBACK 
			if(ptrpcm_config->iPlaybackCH==txch)
			{
				if(ptrpcm_config->bStartPlayback)
				{
					if(ptrpcm_config->mmappos==0)
					{
						if((ptrpcm_config->mmap_rd_idx+1)%MAX_PCM_PAGE!=ptrpcm_config->mmap_wt_idx)
						{
							ptrpcm_config->mmap_rd_idx = (ptrpcm_config->mmap_rd_idx+1)%MAX_PCM_PAGE;
						}
						else
						{
							printk("DR\n");
						}
						p8Data = ptrpcm_config->mmapbuf+ptrpcm_config->mmap_rd_idx*PCM_PAGE_SIZE;
					}
					memcpy((void*)pTx8Data, (void*)p8Data, PCM_8KHZ_SAMPLES*PCM_SAMPLE_SIZE);
					ptrpcm_config->mmappos+=PCM_8KHZ_SAMPLES*PCM_SAMPLE_SIZE;
					p8Data+=PCM_8KHZ_SAMPLES*PCM_SAMPLE_SIZE;
					continue;
				}
				
			}
#endif
			if(codec_obj[rxch].type==0)
			{
				memcpy((void*)pTx8Data, (void*)pRx8Data, PCM_8KHZ_SAMPLES*PCM_SAMPLE_SIZE);
			}
		}
	}
	
	if((ptrpcm_config->bStartRecord)||(ptrpcm_config->bStartPlayback))
	{
		ptrpcm_config->mmappos = 0;
		wake_up_interruptible(&(ptrpcm_config->pcm_qh));
	}
	return 0;
}

void pcm_unmask_isr(u32 dma_ch)
{
	int i, Ret;
	unsigned long data;
	char* p8Data;
	MSG("umisr c=%d\n",dma_ch);

	/* disable system interrupt for PCM */
	data = pcm_inw(RALINK_REG_INTENA);
	data &= 0xFFFFFFEF;
	pcm_outw(RALINK_REG_INTENA, data);
    		
#ifdef D2_TECH_MODIFICATION
	pcm_disable(0, ppcm_config);
	GdmaUnMaskChannel(GDMA_PCM0_RX0);
	GdmaUnMaskChannel(GDMA_PCM0_TX0);
#else
	for( i = 0 ; i < ppcm_config->pcm_ch_num ; i ++ )
		pcm_disable(i, ppcm_config);
	p8Data = (char*)(ppcm_config->RxPage0Buf16Ptr[0]);
	GdmaPcmRx(PCM_CH0_FIFO, p8Data, 0, 0, PCM_PAGE_SIZE, pcm_dma_rx_isr, pcm_unmask_isr);
	
	p8Data = (char*)(ppcm_config->RxPage1Buf16Ptr[0]);
	GdmaPcmRx(PCM_CH0_FIFO, p8Data	, 0, 1, PCM_PAGE_SIZE, pcm_dma_rx_isr, pcm_unmask_isr);	
	
	GdmaUnMaskChannel(GDMA_PCM0_RX0);
	
	p8Data = (char*)(ppcm_config->TxPage0Buf16Ptr[0]);
	GdmaPcmTx(p8Data, PCM_CH0_FIFO, 0, 0, PCM_PAGE_SIZE, pcm_dma_tx_isr, pcm_unmask_isr);
	
	p8Data = (char*)(ppcm_config->TxPage1Buf16Ptr[0]);
	GdmaPcmTx(p8Data, PCM_CH0_FIFO, 0, 1, PCM_PAGE_SIZE, pcm_dma_tx_isr, pcm_unmask_isr);

	GdmaUnMaskChannel(GDMA_PCM0_TX0);
	
	p8Data = (char*)(ppcm_config->TxPage0Buf16Ptr[1]);
	GdmaPcmTx(p8Data, PCM_CH1_FIFO, 1, 0, PCM_PAGE_SIZE, pcm_dma_tx_isr, pcm_unmask_isr);
	
	p8Data = (char*)(ppcm_config->TxPage1Buf16Ptr[1]);
	GdmaPcmTx(p8Data, PCM_CH1_FIFO, 1, 1, PCM_PAGE_SIZE, pcm_dma_tx_isr, pcm_unmask_isr);

	GdmaUnMaskChannel(GDMA_PCM1_TX0);
	
	p8Data = (char*)(ppcm_config->RxPage0Buf16Ptr[1]);
	GdmaPcmRx(PCM_CH1_FIFO, p8Data, 1, 0, PCM_PAGE_SIZE, pcm_dma_rx_isr, pcm_unmask_isr);
		
	p8Data = (char*)(ppcm_config->RxPage1Buf16Ptr[1]);					
	GdmaPcmRx(PCM_CH1_FIFO, p8Data, 1, 1, PCM_PAGE_SIZE, pcm_dma_rx_isr, pcm_unmask_isr);
	
	GdmaUnMaskChannel(GDMA_PCM1_RX0);
#endif	
	data = pcm_inw(PCM_GLBCFG);
	data |= REGBIT(0x1, DMA_EN);
	pcm_outw(PCM_GLBCFG, data);
	
	/* enable system interrupt for PCM */
	data = pcm_inw(RALINK_REG_INTENA);
	data |=0x010;
	pcm_outw(RALINK_REG_INTENA, data);

	for( i = 0; i < ppcm_config->nch_active; i++)
	{
		ppcm_config->txfifo_rd_idx[i] = 0;
		ppcm_config->txfifo_wt_idx[i] = 0;
		ppcm_config->rxfifo_rd_idx[i] = 0;
		ppcm_config->rxfifo_wt_idx[i] = 0;
		ppcm_config->bsfifo_rd_idx[i] = 0;
		ppcm_config->bsfifo_wt_idx[i] = 0;
	}
	ppcm_config->rx_isr_cnt = 0;
	ppcm_config->tx_isr_cnt = 0;
	
#ifdef D2_TECH_MODIFICATION
	pcm_enable(0, ppcm_config);
#else	
	for ( i = 0 ; i < ppcm_config->pcm_ch_num ; i ++ )
		pcm_enable(i, ppcm_config);
#endif

	return;
}

/**
 * @brief PCM interrupt handler 
 *
 * When PCM interrupt happened , call related handler 
 * to do the remain job.
 *
 */
irqreturn_t pcm_irq_isr(int irq, void *irqaction)
{
	u32 pcm_status;
	
	
	pcm_status=pcm_inw(PCM_INT_STATUS);
	MSG("SR=%08X\n",pcm_status);

	/* check CH0 status */
	if(pcm_status&REGBIT(1, CH0T_DMA_FAULT))
	{
#ifdef PCM_STATISTIC
		ppcm_status->ch0txdmafault++;
#endif
	}
	if(pcm_status&REGBIT(1, CH0T_OVRUN))
	{
#ifdef PCM_STATISTIC
		ppcm_status->ch0txovrun++;
#endif
	}
	if(pcm_status&REGBIT(1, CH0T_UNRUN))
	{
#ifdef PCM_STATISTIC
		ppcm_status->ch0txunrun++;
#endif	
	}
	if(pcm_status&REGBIT(1, CH0T_THRES))
	{
#ifdef PCM_STATISTIC
		ppcm_status->ch0txunrun++;
#endif		
	}
	if(pcm_status&REGBIT(1, CH0R_DMA_FAULT))
	{
#ifdef PCM_STATISTIC
		ppcm_status->ch0rxdmafault++;
#endif		
	}
	if(pcm_status&REGBIT(1, CH0R_UNRUN))
	{
#ifdef PCM_STATISTIC
		ppcm_status->ch0rxovrun++;
#endif		
	}
	if(pcm_status&REGBIT(1, CH0R_OVRUN))
	{
#ifdef PCM_STATISTIC
		ppcm_status->ch0rxunrun++;
#endif		
	}
	if(pcm_status&REGBIT(1, CH0R_THRES))
	{
#ifdef PCM_STATISTIC
		ppcm_status->ch0rxthres++;
#endif		
	}

	/* check CH1 status */
	if(pcm_status&REGBIT(1, CH1T_DMA_FAULT))
	{
#ifdef PCM_STATISTIC
		ppcm_status->ch1txdmafault++;
#endif		
	}
	if(pcm_status&REGBIT(1, CH1T_OVRUN))
	{
#ifdef PCM_STATISTIC
		ppcm_status->ch1txovrun++;
#endif		
	}
	if(pcm_status&REGBIT(1, CH1T_UNRUN))
	{
#ifdef PCM_STATISTIC
		ppcm_status->ch1txunrun++;
#endif		
	}
	if(pcm_status&REGBIT(1, CH1T_THRES))
	{
#ifdef PCM_STATISTIC
		ppcm_status->ch1txthres++;
#endif		
	}
	if(pcm_status&REGBIT(1, CH1R_DMA_FAULT))
	{
#ifdef PCM_STATISTIC
		ppcm_status->ch1rxdmafault++;
#endif	
	}
	if(pcm_status&REGBIT(1, CH1R_OVRUN))
	{
#ifdef PCM_STATISTIC
		ppcm_status->ch1rxovrun++;
#endif		
	}
	if(pcm_status&REGBIT(1, CH1R_UNRUN))
	{
#ifdef PCM_STATISTIC
		ppcm_status->ch1rxunrun++;
#endif		
	}
	if(pcm_status&REGBIT(1, CH1R_THRES))
	{
#ifdef PCM_STATISTIC
		ppcm_status->ch1rxthres++;
#endif		
	}
	pcm_outw(PCM_INT_STATUS, 0xFFFF);
	
	return IRQ_HANDLED;

}

#ifdef D2_TECH_MODIFICATION
static void pcm_start(void)
{
	int data, i, Ret;
	char *p8Data;

	pcm_config_type *ptrpcm_config = ppcm_config;

	MSG("iocmd=PCM_START\n");

	p8Data = (char *)(ptrpcm_config->RxPage0Buf16Ptr[0]);
	GdmaPcmRx(PCM_CH0_FIFO, p8Data, 0, 0, PCM_PAGE_SIZE, pcm_dma_rx_isr, pcm_unmask_isr);

	p8Data = (char *)(ptrpcm_config->RxPage1Buf16Ptr[0]);
	GdmaPcmRx(PCM_CH0_FIFO, p8Data, 0, 1, PCM_PAGE_SIZE, pcm_dma_rx_isr, pcm_unmask_isr);

	GdmaUnMaskChannel(GDMA_PCM0_RX0);

	p8Data = (char *)(ptrpcm_config->TxPage0Buf16Ptr[0]);
	GdmaPcmTx(p8Data, PCM_CH0_FIFO, 0, 0, PCM_PAGE_SIZE, pcm_dma_tx_isr, pcm_unmask_isr);

	p8Data = (char *)(ptrpcm_config->TxPage1Buf16Ptr[0]);
	GdmaPcmTx(p8Data, PCM_CH0_FIFO, 0, 1, PCM_PAGE_SIZE, pcm_dma_tx_isr, pcm_unmask_isr);

	GdmaUnMaskChannel(GDMA_PCM0_TX0);

	data = pcm_inw(PCM_GLBCFG);
	data |= REGBIT(0x1, DMA_EN);
	pcm_outw(PCM_GLBCFG, data);

	/* enable system interrupt for PCM */
	data = pcm_inw(RALINK_REG_INTENA);
	data |= 0x010;
	pcm_outw(RALINK_REG_INTENA, data);

	Ret = request_irq(SURFBOARDINT_PCM, pcm_irq_isr, SA_INTERRUPT, "Ralink_PCM", NULL);
	if (Ret) {
		MSG("PCM: IRQ %d is not free.\n", SURFBOARDINT_PCM);
		return PCM_REQUEST_IRQ_FAILED;
	}

	for (i = 0; i < ptrpcm_config->nch_active; i++) {
		ptrpcm_config->txfifo_rd_idx[i] = 0;
		ptrpcm_config->txfifo_wt_idx[i] = 0;
		ptrpcm_config->rxfifo_rd_idx[i] = 0;
		ptrpcm_config->rxfifo_wt_idx[i] = 0;
		ptrpcm_config->bsfifo_rd_idx[i] = 0;
		ptrpcm_config->bsfifo_wt_idx[i] = 0;
	}

	ptrpcm_config->rx_isr_cnt = 0;
	ptrpcm_config->tx_isr_cnt = 0;

	pcm_enable(0, ptrpcm_config);
}
#endif

int pcm_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	int data, i, Ret;
	extern unsigned char currentChannel;
	char* p8Data;
	unsigned long flags;
	
	pcm_config_type* ptrpcm_config = ppcm_config;
	pcm_record_type* ptrpcm_record;
	pcm_playback_type* ptrpcm_playback;
	
	switch(cmd)
	{
		case PCM_SET_RECORD:
			MSG("iocmd=PCM_SET_RECORD\n");
			ptrpcm_config->mmapbuf = kmalloc(PCM_PAGE_SIZE*MAX_PCM_PAGE, GFP_KERNEL);
			if(ptrpcm_config->mmapbuf==NULL)
				return -1;
			ptrpcm_config->mmap_rd_idx = ptrpcm_config->mmap_wt_idx = 0;		
			ptrpcm_config->mmappos = 0;
			ptrpcm_config->bStartRecord = 1;
			ptrpcm_config->iRecordCH = arg;
			init_waitqueue_head(&(ptrpcm_config->pcm_qh));
			break;
		case PCM_SET_UNRECORD:
			MSG("iocmd=PCM_SET_UNRECORD\n");
			ptrpcm_config->bStartRecord = 0;
			ptrpcm_config->mmappos = 0;
			ptrpcm_config->iRecordCH = -1;
			kfree(ptrpcm_config->mmapbuf);
			break;
		case PCM_SET_PLAYBACK:
			MSG("iocmd=PCM_SET_RECORD\n");
			ptrpcm_config->mmapbuf = kmalloc(PCM_PAGE_SIZE*MAX_PCM_PAGE, GFP_KERNEL);
			if(ptrpcm_config->mmapbuf==NULL)
				return -1;
			ptrpcm_config->mmap_rd_idx = ptrpcm_config->mmap_wt_idx = 0;	
			ptrpcm_config->mmappos = 0;
			ptrpcm_config->bStartPlayback = 1;
			ptrpcm_config->iPlaybackCH = arg;
			init_waitqueue_head(&(ptrpcm_config->pcm_qh));
			break;
		case PCM_SET_UNPLAYBACK:
			MSG("iocmd=PCM_SET_UNRECORD\n");
			ptrpcm_config->bStartPlayback = 0;
			ptrpcm_config->mmappos = 0;
			ptrpcm_config->iPlaybackCH = -1;
			kfree(ptrpcm_config->mmapbuf);
			break;			
		case PCM_READ_PCM:
			ptrpcm_record = arg;
			if(ptrpcm_config->nch_active <= 0)
				return -1;
			do{
				spin_lock_irqsave(&ptrpcm_config->lock, flags);	
				if((ptrpcm_config->mmap_rd_idx+1)%MAX_PCM_PAGE!=ptrpcm_config->mmap_wt_idx)
				{
					ptrpcm_config->mmap_rd_idx = (ptrpcm_config->mmap_rd_idx+1)%MAX_PCM_PAGE;
					p8Data = ptrpcm_config->mmapbuf+ptrpcm_config->mmap_rd_idx*PCM_PAGE_SIZE;	
					spin_unlock_irqrestore(&ptrpcm_config->lock, flags);
					break;
				}
				else
				{
					spin_unlock_irqrestore(&ptrpcm_config->lock, flags);
					interruptible_sleep_on(&(ptrpcm_config->pcm_qh));
				}
			}while(1);
			
			copy_to_user(ptrpcm_record->pcmbuf, p8Data, PCM_PAGE_SIZE);
			ptrpcm_record->size = PCM_PAGE_SIZE;
			ptrpcm_config->mmappos = 0;
			break;
		case PCM_WRITE_PCM:
			ptrpcm_playback = arg;
			if(ptrpcm_config->nch_active <= 0)
				return -1;
			do{
				spin_lock_irqsave(&ptrpcm_config->lock, flags);	
				if((ptrpcm_config->mmap_wt_idx+1)%MAX_PCM_PAGE!=ptrpcm_config->mmap_rd_idx)
				{
					ptrpcm_config->mmap_wt_idx = (ptrpcm_config->mmap_wt_idx+1)%MAX_PCM_PAGE;
					p8Data = ptrpcm_config->mmapbuf+ptrpcm_config->mmap_wt_idx*PCM_PAGE_SIZE;	
					spin_unlock_irqrestore(&ptrpcm_config->lock, flags);
					break;
				}
				else
				{
					spin_unlock_irqrestore(&ptrpcm_config->lock, flags);
					interruptible_sleep_on(&(ptrpcm_config->pcm_qh));
				}
			}while(1);
			
			copy_from_user(p8Data, ptrpcm_playback->pcmbuf, PCM_PAGE_SIZE);
			ptrpcm_playback->size = PCM_PAGE_SIZE;
			ptrpcm_config->mmappos = 0;
			break;	
		case PCM_SET_CODEC_TYPE:
			{
				long param[2];
				
				copy_from_user(param, (long*)arg, sizeof(long)*2);
				MSG("iocmd=PCM_SET_CODEC_TYPE CH=%d, Type=%d\n",param[0],param[1]);
				switch(param[1])
				{
					case G711ULAW_CODEC:
					case G711ALAW_CODEC:
					case G729AB_CODEC: 				
					case G723A_CODEC: 		
						ptrpcm_config->codec_type[param[0]] = param[1];
						break;
					default:
						ptrpcm_config->codec_type[param[0]] = 0;	
						break;
				}
				break;
			}	
		case PCM_START:
			MSG("iocmd=PCM_START\n");
			
			for( i = 0 ; i < ptrpcm_config->codec_ch_num ; i++ )
			{		
				codec_obj[i].type = ptrpcm_config->codec_type[i];
				codec_obj[i].ch = i ;
				voice_init_codec(&codec_obj[i]);
				MSG("CH[%d]=%d\n",i,codec_obj[i].type);
			}
				
			p8Data = (char*)(ptrpcm_config->RxPage0Buf16Ptr[0]);
			GdmaPcmRx(PCM_CH0_FIFO, p8Data, 0, 0, PCM_PAGE_SIZE, pcm_dma_rx_isr, pcm_unmask_isr);
			
			p8Data = (char*)(ptrpcm_config->RxPage1Buf16Ptr[0]);
			GdmaPcmRx(PCM_CH0_FIFO, p8Data, 0, 1, PCM_PAGE_SIZE, pcm_dma_rx_isr, pcm_unmask_isr);	
			
			GdmaUnMaskChannel(GDMA_PCM0_RX0);
			
			p8Data = (char*)(ptrpcm_config->TxPage0Buf16Ptr[0]);
			GdmaPcmTx(p8Data, PCM_CH0_FIFO, 0, 0, PCM_PAGE_SIZE, pcm_dma_tx_isr, pcm_unmask_isr);
			
			p8Data = (char*)(ptrpcm_config->TxPage1Buf16Ptr[0]);
			GdmaPcmTx(p8Data, PCM_CH0_FIFO, 0, 1, PCM_PAGE_SIZE, pcm_dma_tx_isr, pcm_unmask_isr);
		
			GdmaUnMaskChannel(GDMA_PCM0_TX0);
			
			if(ptrpcm_config->pcm_ch_num>=2)
			{
				p8Data = (char*)(ptrpcm_config->TxPage0Buf16Ptr[1]);
				GdmaPcmTx(p8Data, PCM_CH1_FIFO, 1, 0, PCM_PAGE_SIZE, pcm_dma_tx_isr, pcm_unmask_isr);
				
				p8Data = (char*)(ptrpcm_config->TxPage1Buf16Ptr[1]);
				GdmaPcmTx(p8Data, PCM_CH1_FIFO, 1, 1, PCM_PAGE_SIZE, pcm_dma_tx_isr, pcm_unmask_isr);
	
				GdmaUnMaskChannel(GDMA_PCM1_TX0);
				
				p8Data = (char*)(ptrpcm_config->RxPage0Buf16Ptr[1]);
				GdmaPcmRx(PCM_CH1_FIFO, p8Data, 1, 0, PCM_PAGE_SIZE, pcm_dma_rx_isr, pcm_unmask_isr);
					
				p8Data = (char*)(ptrpcm_config->RxPage1Buf16Ptr[1]);					
				GdmaPcmRx(PCM_CH1_FIFO, p8Data, 1, 1, PCM_PAGE_SIZE, pcm_dma_rx_isr, pcm_unmask_isr);
				
				GdmaUnMaskChannel(GDMA_PCM1_RX0);
			}
			
			Ret = request_irq(SURFBOARDINT_PCM, pcm_irq_isr, SA_INTERRUPT, "Ralink_PCM", NULL);
			if(Ret){
				MSG("PCM: IRQ %d is not free.\n", SURFBOARDINT_PCM);
				return PCM_REQUEST_IRQ_FAILED;
			}
			for( i = 0; i < ptrpcm_config->pcm_ch_num; i++)
			{
				ptrpcm_config->txfifo_rd_idx[i] = 0;
				ptrpcm_config->txfifo_wt_idx[i] = 0;
				ptrpcm_config->rxfifo_rd_idx[i] = 0;
				ptrpcm_config->rxfifo_wt_idx[i] = 0;
				ptrpcm_config->bsfifo_rd_idx[i] = 0;
				ptrpcm_config->bsfifo_wt_idx[i] = 0;
			}
			ptrpcm_config->rx_isr_cnt = 0;
			ptrpcm_config->tx_isr_cnt = 0;
			
			ptrpcm_config->bStartRecord = 0;
			
			for ( i = 0 ; i < ptrpcm_config->pcm_ch_num ; i ++ )
				pcm_enable(i, ptrpcm_config);
				
			data = pcm_inw(PCM_GLBCFG);
			data |= REGBIT(0x1, DMA_EN);
			pcm_outw(PCM_GLBCFG, data);
			
			/* enable system interrupt for PCM */
			data = pcm_inw(RALINK_REG_INTENA);
			data |=0x010;
    		pcm_outw(RALINK_REG_INTENA, data);
    	
			break;
		case PCM_STOP:
			MSG("iocmd=PCM_STOP\n");
			spin_lock_irqsave(&ptrpcm_config->lock, flags);	
			data = pcm_inw(PCM_GLBCFG);
			data &= ~REGBIT(0x1, DMA_EN);
			pcm_outw(PCM_GLBCFG, data);
				
			/* disable system interrupt for PCM */
			data = pcm_inw(RALINK_REG_INTENA);
			data &=~0x010;
    		pcm_outw(RALINK_REG_INTENA, data);
		
			synchronize_irq(SURFBOARDINT_PCM);
			free_irq(SURFBOARDINT_PCM, NULL);
			
			for( i = 0 ; i < ptrpcm_config->codec_ch_num ; i++ )
			{
				codec_obj[i].type = 0;
				codec_obj[i].ch = 0 ;
				voice_release_codec(&codec_obj[i]);
				ptrpcm_config->codec_type[i] = 0;
			}
			
			data = read_c0_status();
			data&=0xFEFFFFFF;			/* clean mips ASE mode bit */
			write_c0_status(data|0x00007000);
			spin_unlock_irqrestore(&ptrpcm_config->lock, flags);
			break;
		default:
			break;
	}
	
	return 0;
}

void pcm_reset_slic ()
{
	//press reset
 	int data;
 	int i;

	/* Set SPI to GPIO mode */
	data = pcm_inw(RALINK_REG_GPIOMODE); 
	data |= RALINK_GPIO(1);
	pcm_outw(RALINK_REG_GPIOMODE, data);
	
	/* RESET set to low */
	data = pcm_inw(RALINK_REG_PIODIR);
	data |= RALINK_GPIO(0);
	pcm_outw(RALINK_REG_PIODIR, data);
	data = pcm_inw(RALINK_REG_PIODATA);
	data &= 0xFFFFFFFE;
	pcm_outw(RALINK_REG_PIODATA, data);
	
	/* CS set to high */
	data = pcm_inw(RALINK_REG_PIODIR);
	data |= RALINK_GPIO(3);
	pcm_outw(RALINK_REG_PIODIR, data);
	data = pcm_inw(RALINK_REG_PIODATA);
	data |= RALINK_GPIO(3); 
	pcm_outw(RALINK_REG_PIODATA, data);
	
	mdelay(1000);
	
	/* RESET set to high */
	data = pcm_inw(RALINK_REG_PIODIR);
	data |= RALINK_GPIO(0);
	pcm_outw(RALINK_REG_PIODIR, data);
	data = pcm_inw(RALINK_REG_PIODATA);
	data |= RALINK_GPIO(0);
	pcm_outw(RALINK_REG_PIODATA, data);

	mdelay(1000);
	
	/* Set GPIO to SPI mode */
	data = pcm_inw(RALINK_REG_GPIOMODE); 
	data &= 0xFFFFFFFD;
	pcm_outw(RALINK_REG_GPIOMODE, data);
	
	return;
}

module_init(pcm_init);
module_exit(pcm_exit);
module_param_named(slic, slic_type, int, S_IRUGO);

#ifdef D2_TECH_MODIFICATION
EXPORT_SYMBOL(pcm_open);
#endif

MODULE_DESCRIPTION("Ralink SoC PCM Controller Module");
MODULE_AUTHOR("Qwert Chin <qwert.chin@ralinktech.com.tw>");
MODULE_LICENSE("GPL");
MODULE_VERSION(MOD_VERSION);

