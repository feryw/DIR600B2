/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    rt_main_dev.c

    Abstract:
    Create and register network interface.

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
	Sample		Mar/21/07		Merge RT2870 and RT2860 drivers.
*/

#include "rt_config.h"

#define FORTY_MHZ_INTOLERANT_INTERVAL	(60*1000) // 1 min

#ifdef MULTIPLE_CARD_SUPPORT
// record whether the card in the card list is used in the card file
UINT8  MC_CardUsed[MAX_NUM_OF_MULTIPLE_CARD];
// record used card mac address in the card list
static UINT8  MC_CardMac[MAX_NUM_OF_MULTIPLE_CARD][6];
#endif // MULTIPLE_CARD_SUPPORT //

#ifdef CONFIG_APSTA_MIXED_SUPPORT
UINT32 CW_MAX_IN_BITS;
#endif // CONFIG_APSTA_MIXED_SUPPORT //

/*---------------------------------------------------------------------*/
/* Private Variables Used                                              */
/*---------------------------------------------------------------------*/
//static RALINK_TIMER_STRUCT     PeriodicTimer;

char *mac = "";		   // default 00:00:00:00:00:00
char *hostname = "";		   // default CMPC
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,12)
MODULE_PARM (mac, "s");
#else
module_param (mac, charp, 0);
#endif
MODULE_PARM_DESC (mac, "rt28xx: wireless mac addr");


/*---------------------------------------------------------------------*/
/* Prototypes of Functions Used                                        */
/*---------------------------------------------------------------------*/
extern BOOLEAN ba_reordering_resource_init(PRTMP_ADAPTER pAd, int num);
extern void ba_reordering_resource_release(PRTMP_ADAPTER pAd);
extern NDIS_STATUS NICLoadRateSwitchingParams(IN PRTMP_ADAPTER pAd);

#ifdef RT2860
extern void init_thread_task(PRTMP_ADAPTER pAd);
#endif // RT2860 //

#if 0
#ifdef RT2870
extern struct net_device_stats *rtusb_get_ether_stats(IN struct net_device *net_dev);
#if ((WIRELESS_EXT >= 12) && (WIRELESS_EXT <= 20 ))
extern struct iw_statistics *rtusb_get_wireless_stats(IN struct net_device *net_dev);
#endif // WIRELESS_EXT //

#endif // RT2870 //
#endif

#if defined(CONFIG_RT2860V2_AP_RBUS_PCI) || defined(CONFIG_RT2860V2_STA_RBUS_PCI)
// public function prototype
INT __devinit rt28xx_probe(IN void *_dev_p, IN void *_dev_id_p,
							IN UINT argc, OUT PRTMP_ADAPTER *ppAd);
#endif // #if defined(CONFIG_RT2860V2_AP_RBUS_PCI) || defined(CONFIG_RT2860V2_STA_RBUS_PCI) //

// private function prototype
static int rt28xx_init(IN struct net_device *net_dev);
int rt28xx_close(IN struct net_device *net_dev);
int rt28xx_open(struct net_device *net_dev);
INT rt28xx_send_packets(IN struct sk_buff *skb_p, IN struct net_device *net_dev);

#if LINUX_VERSION_CODE <= 0x20402	// Red Hat 7.1
struct net_device *alloc_netdev(
	int sizeof_priv,
	const char *mask,
	void (*setup)(struct net_device *));
#endif // LINUX_VERSION_CODE //

static void CfgInitHook(PRTMP_ADAPTER pAd);
//static BOOLEAN RT28XXAvailRANameAssign(IN CHAR *name_p);

#ifdef CONFIG_STA_SUPPORT
extern	const struct iw_handler_def rt28xx_iw_handler_def;
#endif // CONFIG_STA_SUPPORT //

#ifdef CONFIG_APSTA_MIXED_SUPPORT
extern	const struct iw_handler_def rt28xx_ap_iw_handler_def;
#endif // CONFIG_APSTA_MIXED_SUPPORT //

#if WIRELESS_EXT >= 12
// This function will be called when query /proc
struct iw_statistics *rt28xx_get_wireless_stats(
    IN struct net_device *net_dev);
#endif
#if !defined(CONFIG_RA_NAT_NONE)
extern int (*ra_sw_nat_hook_tx)(struct sk_buff *skb);
#endif

struct net_device_stats *RT28xx_get_ether_stats(
    IN  struct net_device *net_dev);
/*
========================================================================
Routine Description:
    Close raxx interface.

Arguments:
	*net_dev			the raxx interface pointer

Return Value:
    0					Open OK
	otherwise			Open Fail

Note:
	1. if open fail, kernel will not call the close function.
	2. Free memory for
		(1) Mlme Memory Handler:		MlmeHalt()
		(2) TX & RX:					RTMPFreeTxRxRingMemory()
		(3) BA Reordering: 				ba_reordering_resource_release()
========================================================================
*/
int rt28xx_close(IN struct net_device *net_dev)
{
    RTMP_ADAPTER	*pAd = net_dev->priv;
	BOOLEAN			Cancelled = FALSE;
	UINT32			i = 0;
#ifdef RT2870
	DECLARE_WAIT_QUEUE_HEAD(unlink_wakeup); 
	DECLARE_WAITQUEUE(wait, current);

	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_REMOVE_IN_PROGRESS);
#endif // RT2870 //


    DBGPRINT(RT_DEBUG_TRACE, ("===> rt28xx_close\n"));

	// Sanity check for pAd
	if (pAd == NULL)
		return 0; // close ok

#ifdef CONFIG_STA_SUPPORT
#ifdef RT2860
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
        RTMPPCIeLinkCtrlValueRestore(pAd, RESTORE_CLOSE);
#endif // RT2860 //
#endif // CONFIG_STA_SUPPORT //

	netif_carrier_off(pAd->net_dev);
	netif_stop_queue(pAd->net_dev);

#ifdef MESH_SUPPORT
	// close all mesh link before the interface go down.
	if (MESH_ON(pAd))
		MeshDown(pAd, TRUE);
#endif // MESH_SUPPORT //

	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);


	for (i = 0 ; i < NUM_OF_TX_RING; i++)
	{
		while (pAd->DeQueueRunning[i] == TRUE)
		{
			printk("Waiting for TxQueue[%d] done..........\n", i);
			RTMPusecDelay(1000);
		}
	}
	
#ifdef RT2870
	// ensure there are no more active urbs.
	add_wait_queue (&unlink_wakeup, &wait);
	pAd->wait = &unlink_wakeup;

	// maybe wait for deletions to finish.
	i = 0;
	//while((i < 25) && atomic_read(&pAd->PendingRx) > 0) 
	while(i < 25)
	{
		unsigned long IrqFlags;

		RTMP_IRQ_LOCK(&pAd->BulkInLock, IrqFlags);
		if (pAd->PendingRx == 0)
		{
			RTMP_IRQ_UNLOCK(&pAd->BulkInLock, IrqFlags);
			break;
		}
		RTMP_IRQ_UNLOCK(&pAd->BulkInLock, IrqFlags);
		
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,9)
		msleep(UNLINK_TIMEOUT_MS);	//Time in millisecond
#else
		RTMPusecDelay(UNLINK_TIMEOUT_MS*1000);	//Time in microsecond
#endif
		i++;
	}
	pAd->wait = NULL;
	remove_wait_queue (&unlink_wakeup, &wait); 
#endif // RT2870 //


#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		if (INFRA_ON(pAd)
#ifdef RT2870
		&& (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
#endif // RT2870 //
		) 
		{
			MLME_DISASSOC_REQ_STRUCT	DisReq;
			MLME_QUEUE_ELEM				MsgElem;
    
			COPY_MAC_ADDR(DisReq.Addr, pAd->CommonCfg.Bssid);
			DisReq.Reason =  REASON_DEAUTH_STA_LEAVING;

			MsgElem.Machine = ASSOC_STATE_MACHINE;
			MsgElem.MsgType = MT2_MLME_DISASSOC_REQ;
			MsgElem.MsgLen = sizeof(MLME_DISASSOC_REQ_STRUCT);
			NdisMoveMemory(MsgElem.Msg, &DisReq, sizeof(MLME_DISASSOC_REQ_STRUCT));

			// Prevent to connect AP again in STAMlmePeriodicExec
			pAd->MlmeAux.AutoReconnectSsidLen= 32;
			NdisZeroMemory(pAd->MlmeAux.AutoReconnectSsid, pAd->MlmeAux.AutoReconnectSsidLen);

			pAd->Mlme.CntlMachine.CurrState = CNTL_WAIT_OID_DISASSOC;
			MlmeDisassocReqAction(pAd, &MsgElem);
			RTMPusecDelay(1000);
		}

#ifdef CCX_SUPPORT
		RTMPCancelTimer(&pAd->StaCfg.LeapAuthTimer, &Cancelled);
#endif

		RTMPCancelTimer(&pAd->StaCfg.StaQuickResponeForRateUpTimer, &Cancelled);
		RTMPCancelTimer(&pAd->StaCfg.WpaDisassocAndBlockAssocTimer, &Cancelled);
#ifdef WPA_SUPPLICANT_SUPPORT
#ifndef NATIVE_WPA_SUPPLICANT_SUPPORT
//		if (pAd->StaCfg.WpaSupplicantUP) 
		{
			union iwreq_data    wrqu;
			// send wireless event to wpa_supplicant for infroming interface down.
			memset(&wrqu, 0, sizeof(wrqu));
			wrqu.data.flags = RT_INTERFACE_DOWN;
			wireless_send_event(pAd->net_dev, IWEVCUSTOM, &wrqu, NULL);
		}
#endif // NATIVE_WPA_SUPPLICANT_SUPPORT //
#endif // WPA_SUPPLICANT_SUPPORT //

		// If dirver doesn't wake up firmware here,
		// NICLoadFirmware will hang forever when interface is up again.
		if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE))
        {      
		    AsicForceWakeup(pAd, TRUE);
        }

#ifdef RT2860
		pAd->bPCIclkOff = FALSE;    
#endif // RT2860 //
	}
#endif // CONFIG_STA_SUPPORT //

	//RTUSBCleanUpMLMEWaitQueue(pAd);	/*not used in RT28xx*/

#ifdef CONFIG_AP_SUPPORT

	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
#ifdef RT2870
		RTMPCancelTimer(&pAd->ApCfg.BeaconUpdateTimer, &Cancelled);
#endif // RT2870 //

		RTMPCancelTimer(&pAd->PeriodicTimer,	&Cancelled);
	}
#endif // CONFIG_AP_SUPPORT //

#ifdef RT2870
	// We need clear timerQ related structure before exits of the timer thread.
	RT2870_TimerQ_Exit(pAd);
	// Close kernel threads or tasklets
	RT28xxThreadTerminate(pAd);
#endif // RT2870 //

	// Stop Mlme state machine
	MlmeHalt(pAd);

	// Close kernel threads or tasklets
	kill_thread_task(pAd);


#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		STAMacTableReset(pAd);
	}
#endif // CONFIG_STA_SUPPORT //

#ifdef MESH_SUPPORT
	RTMP_Mesh_Close(pAd);
#endif // MESH_SUPPORT //

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
#ifdef APCLI_SUPPORT
		// remove all AP-client virtual interfaces.
		RT28xx_ApCli_Close(pAd);
#endif // APCLI_SUPPORT //

#ifdef MAT_SUPPORT
		MATEngineExit(pAd);
#endif // MAT_SUPPORT //

#ifdef WDS_SUPPORT
		// remove all WDS virtual interfaces.
		RT28xx_WDS_Close(pAd);
#endif // WDS_SUPPORT //

#ifdef MBSS_SUPPORT
		RT28xx_MBSS_Close(pAd);
#endif // MBSS_SUPPORT //

		// Shutdown Access Point function, release all related resources 
		APShutdown(pAd);

		// Free BssTab & ChannelInfo tabbles.
		AutoChBssTableDestroy(pAd);
		ChannelInfoDestroy(pAd);
	}
#endif // CONFIG_AP_SUPPORT //

#ifdef WSC_INCLUDED
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		WscStop(pAd, FALSE, &pAd->ApCfg.MBSSID[MAIN_MBSSID].WscControl);
#endif // CONFIG_AP_SUPPORT //
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		WscStop(pAd,
#ifdef CONFIG_AP_SUPPORT
				FALSE,
#endif // CONFIG_AP_SUPPORT //
				&pAd->StaCfg.WscControl);
#endif // CONFIG_STA_SUPPORT //

#ifdef OLD_DH_KEY
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	    	WSC_VFREE_KEY_MEM(pAd->ApCfg.MBSSID[0].WscControl.pPubKeyMem, pAd->ApCfg.MBSSID[0].WscControl.pSecKeyMem);
#endif // CONFIG_AP_SUPPORT //
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		WSC_VFREE_KEY_MEM(pAd->StaCfg.WscControl.pPubKeyMem, pAd->StaCfg.WscControl.pSecKeyMem);
#endif // CONFIG_STA_SUPPORT //
#endif // OLD_DH_KEY //

#ifndef OLD_DH_KEY
	DH_freeall();
#endif // OLD_DH_KEY //

#endif // WSC_INCLUDED //

#ifdef RT2860
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_ACTIVE))
	{
		NICDisableInterrupt(pAd);
	}

	// Disable Rx, register value supposed will remain after reset
	NICIssueReset(pAd);

	// Free IRQ
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
	{
		// Deregister interrupt function
		if (pAd->CSRBaseAddress == RT2860_CSR_ADDR)
			free_irq(net_dev->irq, net_dev);
		else
			RT28XX_IRQ_RELEASE(net_dev)
		RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE);
	}
#endif // RT2860 //


	// Free Ring or USB buffers
	RTMPFreeTxRxRingMemory(pAd);

	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);


	// Free BA reorder resource
	ba_reordering_resource_release(pAd);
	
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0)
	module_put(THIS_MODULE);
#else
	MOD_DEC_USE_COUNT;
#endif

	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_START_UP);

	return 0; // close ok
} /* End of rt28xx_close */

static int rt28xx_init(IN struct net_device *net_dev)
{
	PRTMP_ADAPTER 			pAd = (PRTMP_ADAPTER)net_dev->priv;
	UINT					index;
	UCHAR					TmpPhy;
//	ULONG					Value=0;
	NDIS_STATUS				Status;
//    OID_SET_HT_PHYMODE		SetHT;
//	WPDMA_GLO_CFG_STRUC     GloCfg;
	UINT32 		MacCsr0 = 0;

	// Allocate BA Reordering memory
	ba_reordering_resource_init(pAd, MAX_REORDERING_MPDU_NUM);
	

	// Make sure MAC gets ready.
	index = 0;
	do
	{
		RTMP_IO_READ32(pAd, MAC_CSR0, &MacCsr0);
		pAd->MACVersion = MacCsr0;

		if ((pAd->MACVersion != 0x00) && (pAd->MACVersion != 0xFFFFFFFF))
			break;

		RTMPusecDelay(10);
	} while (index++ < 100);

	DBGPRINT(RT_DEBUG_TRACE, ("MAC_CSR0  [ Ver:Rev=0x%08x]\n", pAd->MACVersion));

	// Disable DMA
	RT28XXDMADisable(pAd);


	// Load 8051 firmware
	if (pAd->CSRBaseAddress != RT2860_CSR_ADDR)
	{
	Status = NICLoadFirmware(pAd);
	if (Status != NDIS_STATUS_SUCCESS)
	{
		DBGPRINT_ERR(("NICLoadFirmware failed, Status[=0x%08x]\n", Status));
		goto err1;
	}
	}
	else
		Status = NDIS_STATUS_SUCCESS;

	NICLoadRateSwitchingParams(pAd);

	// Disable interrupts here which is as soon as possible
	// This statement should never be true. We might consider to remove it later
#ifdef RT2860
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_ACTIVE))
	{
		NICDisableInterrupt(pAd);
	}
#endif // RT2860 //

	Status = RTMPAllocTxRxRingMemory(pAd);
	if (Status != NDIS_STATUS_SUCCESS)
	{
		DBGPRINT_ERR(("RTMPAllocDMAMemory failed, Status[=0x%08x]\n", Status));
		goto err1;
	}

	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE);

	// initialize MLME
	//

	Status = MlmeInit(pAd);
	if (Status != NDIS_STATUS_SUCCESS)
	{
		DBGPRINT_ERR(("MlmeInit failed, Status[=0x%08x]\n", Status));
		goto err2;
	}

	// Initialize pAd->StaCfg, pAd->ApCfg, pAd->CommonCfg to manufacture default
	//
	UserCfgInit(pAd);

#ifdef RT2870
	// We need init timerQ related structure before create the timer thread.
	RT2870_TimerQ_Init(pAd);
#endif // RT2870 //

	RT28XX_TASK_THREAD_INIT(pAd, Status);
	if (Status != NDIS_STATUS_SUCCESS)
		goto err1;

//	COPY_MAC_ADDR(pAd->ApCfg.MBSSID[apidx].Bssid, netif->hwaddr);
//	pAd->bForcePrintTX = TRUE;

	CfgInitHook(pAd);

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		APInitialize(pAd);
#endif // CONFIG_AP_SUPPORT //	

#ifdef BLOCK_NET_IF
	initblockQueueTab(pAd);
#endif // BLOCK_NET_IF //

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		NdisAllocateSpinLock(&pAd->MacTabLock);
#endif // CONFIG_STA_SUPPORT //

	//
	// Init the hardware, we need to init asic before read registry, otherwise mac register will be reset
	//
	Status = NICInitializeAdapter(pAd, TRUE);
	if (Status != NDIS_STATUS_SUCCESS)
	{
		DBGPRINT_ERR(("NICInitializeAdapter failed, Status[=0x%08x]\n", Status));
		if (Status != NDIS_STATUS_SUCCESS)
		goto err3;
	}	

	// Read parameters from Config File 
	Status = RTMPReadParametersHook(pAd);

	printk("1. Phy Mode = %d\n", pAd->CommonCfg.PhyMode);
	if (Status != NDIS_STATUS_SUCCESS)
	{
		DBGPRINT_ERR(("NICReadRegParameters failed, Status[=0x%08x]\n",Status));
		goto err4;
	}

#ifdef RT2870
	pAd->CommonCfg.bMultipleIRP = FALSE;

	if (pAd->CommonCfg.bMultipleIRP)
		pAd->CommonCfg.NumOfBulkInIRP = RX_RING_SIZE;
	else
		pAd->CommonCfg.NumOfBulkInIRP = 1;
#endif // RT2870 //


   	//Init Ba Capability parameters.
//	RT28XX_BA_INIT(pAd);
	pAd->CommonCfg.DesiredHtPhy.MpduDensity = (UCHAR)pAd->CommonCfg.BACapability.field.MpduDensity;
	pAd->CommonCfg.DesiredHtPhy.AmsduEnable = (USHORT)pAd->CommonCfg.BACapability.field.AmsduEnable;
	pAd->CommonCfg.DesiredHtPhy.AmsduSize = (USHORT)pAd->CommonCfg.BACapability.field.AmsduSize;
	pAd->CommonCfg.DesiredHtPhy.MimoPs = (USHORT)pAd->CommonCfg.BACapability.field.MMPSmode;
	// UPdata to HT IE
	pAd->CommonCfg.HtCapability.HtCapInfo.MimoPs = (USHORT)pAd->CommonCfg.BACapability.field.MMPSmode;
	pAd->CommonCfg.HtCapability.HtCapInfo.AMsduSize = (USHORT)pAd->CommonCfg.BACapability.field.AmsduSize;
	pAd->CommonCfg.HtCapability.HtCapParm.MpduDensity = (UCHAR)pAd->CommonCfg.BACapability.field.MpduDensity;


	// after reading Registry, we now know if in AP mode or STA mode

	// Load 8051 firmware; crash when FW image not existent
	// Status = NICLoadFirmware(pAd);
	// if (Status != NDIS_STATUS_SUCCESS)
	//    break;

	printk("2. Phy Mode = %d\n", pAd->CommonCfg.PhyMode);

	// We should read EEPROM for all cases.  rt2860b
	NICReadEEPROMParameters(pAd, mac);	
#ifdef CONFIG_STA_SUPPORT
#endif // CONFIG_STA_SUPPORT //

	printk("3. Phy Mode = %d\n", pAd->CommonCfg.PhyMode);

	// Set PHY to appropriate mode
	TmpPhy = pAd->CommonCfg.PhyMode;
	pAd->CommonCfg.PhyMode = 0xff;
	RTMPSetPhyMode(pAd, TmpPhy);
	SetCommonHT(pAd);
	
	printk("MCS Set = %02x %02x %02x %02x %02x\n", pAd->CommonCfg.HtCapability.MCSSet[0],
           pAd->CommonCfg.HtCapability.MCSSet[1], pAd->CommonCfg.HtCapability.MCSSet[2],
           pAd->CommonCfg.HtCapability.MCSSet[3], pAd->CommonCfg.HtCapability.MCSSet[4]);
	NICInitAsicFromEEPROM(pAd); //rt2860b

#if 0
	// Patch cardbus controller if EEPROM said so.
	if (pAd->bTest1 == FALSE)
		RTMPPatchCardBus(pAd);
#endif

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		/* init QBSS Element */
		QBSS_LoadInit(pAd);
	}
#endif // CONFIG_AP_SUPPORT //

//		APInitialize(pAd);

#ifdef IKANOS_VX_1X0
	VR_IKANOS_FP_Init(pAd->ApCfg.BssidNum, pAd->PermanentAddress);
#endif // IKANOS_VX_1X0 //

		//
	// Initialize RF register to default value
	//
	AsicSwitchChannel(pAd, pAd->CommonCfg.Channel, FALSE);
	AsicLockChannel(pAd, pAd->CommonCfg.Channel);		

	// 8051 firmware require the signal during booting time.
	AsicSendCommandToMcu(pAd, 0x72, 0xFF, 0x00, 0x00);

	if (pAd && (Status != NDIS_STATUS_SUCCESS))
	{
		//
		// Undo everything if it failed
		//
#ifdef WIN_NDIS			
		if (RTMP_GET_REF(pAd) > 0)
		{
			RTMP_DEC_REF(pAd);
		}
#endif // WIN_NDIS //		

		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
		{
//			NdisMDeregisterInterrupt(&pAd->Interrupt);
			RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE);
		}
//		RTMPFreeAdapter(pAd); // we will free it in disconnect()
	}
	else if (pAd)
	{
		// Microsoft HCT require driver send a disconnect event after driver initialization.
		OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED);
//		pAd->IndicateMediaState = NdisMediaStateDisconnected;
		RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_MEDIA_STATE_CHANGE);

		DBGPRINT(RT_DEBUG_TRACE, ("NDIS_STATUS_MEDIA_DISCONNECT Event B!\n"));

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		if (pAd->ApCfg.bAutoChannelAtBootup || (pAd->CommonCfg.Channel == 0))
		{
			UINT8 BBPValue = 0;
#ifdef RT2860
			// Enable Interrupt
			pAd->int_enable_reg = ((DELAYINTMASK)  | (RxINT|TxDataInt|TxMgmtInt)) & ~(0x03);
			pAd->int_disable_mask = 0;
			pAd->int_pending = 0;
			
			RTMP_IO_WRITE32(pAd, INT_SOURCE_CSR, 0xffffffff);  // clear garbage interrupts
			NICEnableInterrupt(pAd);

			// Now Enable RxTx
			RTMPEnableRxTx(pAd);
			// APRxDoneInterruptHandle API will check this flag to decide accept incoming packet or not.
			// Set the flag be ready to receive Beacon frame for autochannel select.
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_START_UP);
#endif // RT2860 //

#ifdef RT2870
			RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS);
			RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_REMOVE_IN_PROGRESS);

			//
			// Support multiple BulkIn IRP,
			// the value on pAd->CommonCfg.NumOfBulkInIRP may be large than 1.
			//
			for(index=0; index<pAd->CommonCfg.NumOfBulkInIRP; index++)
			{
				RTUSBBulkReceive(pAd);
				DBGPRINT(RT_DEBUG_TRACE, ("RTUSBBulkReceive!\n" ));
			}
			// Now Enable RxTx
			RTMPEnableRxTx(pAd);
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_START_UP);
#endif // RT2870 //
			// Let BBP register at 20MHz to do scan		
			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &BBPValue);
			BBPValue &= (~0x18);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, BBPValue);
			DBGPRINT(RT_DEBUG_ERROR, ("SYNC - BBP R4 to 20MHz.l\n"));

			// Now we can receive the beacon and do the listen beacon
			pAd->CommonCfg.Channel = APAutoSelectChannel(pAd, TRUE);
		}
		RTMPInitTimer(pAd, &pAd->PeriodicTimer, GET_TIMER_FUNCTION(APDetectOverlappingExec), pAd, TRUE);
		RTMPSetTimer(&pAd->PeriodicTimer, FORTY_MHZ_INTOLERANT_INTERVAL);
		// If phymode > PHY_11ABGN_MIXED and BW=40 check extension channel, after select channel  
		N_ChannelCheck(pAd);

		APStartUp(pAd);
		printk("Main bssid = %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(pAd->ApCfg.MBSSID[BSS0].Bssid));
	}
#endif // CONFIG_AP_SUPPORT //

#ifdef RT2870
		RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS);
		RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_REMOVE_IN_PROGRESS);

		//
		// Support multiple BulkIn IRP,
		// the value on pAd->CommonCfg.NumOfBulkInIRP may be large than 1.
		//
		for(index=0; index<pAd->CommonCfg.NumOfBulkInIRP; index++)
		{
			RTUSBBulkReceive(pAd);
			DBGPRINT(RT_DEBUG_TRACE, ("RTUSBBulkReceive!\n" ));
		}
#endif // RT2870 //
	}// end of else

#ifdef WSC_INCLUDED
		NdisZeroMemory(&pAd->Wsc_Uuid_Str[0], sizeof(pAd->Wsc_Uuid_Str));
		NdisZeroMemory(&pAd->Wsc_Uuid_E[0], sizeof(pAd->Wsc_Uuid_E));
		WscGenerateUUID(pAd, &pAd->Wsc_Uuid_E[0], &pAd->Wsc_Uuid_Str[0], 0);

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		WscInit(pAd, FALSE, &pAd->ApCfg.MBSSID[BSS0].WscControl);
		if (pAd->ApCfg.MBSSID[BSS0].WscControl.WscEnrolleePinCode == 0)
			pAd->ApCfg.MBSSID[BSS0].WscControl.WscEnrolleePinCode = WscGeneratePinCode(pAd, FALSE, 0);
	}
#endif // CONFIG_AP_SUPPORT //	

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		WscInit(pAd,
#ifdef CONFIG_AP_SUPPORT
				FALSE,
#endif // CONFIG_AP_SUPPORT //
				&pAd->StaCfg.WscControl);
		if (pAd->StaCfg.WscControl.WscEnrolleePinCode == 0)
			pAd->StaCfg.WscControl.WscEnrolleePinCode = WscGeneratePinCode(pAd,
#ifdef CONFIG_AP_SUPPORT
																		FALSE,
#endif // CONFIG_AP_SUPPORT //
																		0);
	}
#endif // CONFIG_STA_SUPPORT //

#endif // WSC_INCLUDED //

	DBGPRINT_S(Status, ("<==== RTMPInitialize, Status=%x\n", Status));

	return TRUE;


err4:
err3:
	MlmeHalt(pAd);
err2:
	RTMPFreeTxRxRingMemory(pAd);
//	RTMPFreeAdapter(pAd);
err1:

	os_free_mem(pAd, pAd->mpdu_blk_pool.mem); // free BA pool

	if (pAd->CSRBaseAddress == RT2860_CSR_ADDR)
		free_irq(net_dev->irq, net_dev);
	else
		RT28XX_IRQ_RELEASE(net_dev);
	net_dev->priv = 0;
	printk("!!! %s Initialized fail !!!\n", RT28xx_CHIP_NAME);
	return FALSE;
} /* End of rt28xx_init */


/*
========================================================================
Routine Description:
    Open raxx interface.

Arguments:
	*net_dev			the raxx interface pointer

Return Value:
    0					Open OK
	otherwise			Open Fail

Note:
========================================================================
*/
int rt28xx_open(struct net_device *net_dev)
{				 
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)net_dev->priv;
	int retval = 0;
 	POS_COOKIE pObj;


	// Sanity check for pAd
	if (pAd == NULL)
	{
		/* if 1st open fail, pAd will be free;
		   So the net_dev->priv will be NULL in 2rd open */
		return -1;
	}

#ifdef CONFIG_APSTA_MIXED_SUPPORT
	if (pAd->OpMode == OPMODE_AP)
	{
		CW_MAX_IN_BITS = 6;
	}
	else if (pAd->OpMode == OPMODE_STA)
	{
		CW_MAX_IN_BITS = 10;
	}

#if WIRELESS_EXT >= 12
	if (net_dev->priv_flags == INT_MAIN) 
	{
		if (pAd->OpMode == OPMODE_AP)
			net_dev->wireless_handlers = (struct iw_handler_def *) &rt28xx_ap_iw_handler_def;
		else if (pAd->OpMode == OPMODE_STA)
			net_dev->wireless_handlers = (struct iw_handler_def *) &rt28xx_iw_handler_def;
	}
#endif // WIRELESS_EXT >= 12 //
#endif // CONFIG_APSTA_MIXED_SUPPORT //

#ifdef CONFIG_STA_SUPPORT
#ifdef RT2860
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
    	// If dirver doesn't wake up firmware here,
    	// NICLoadFirmware will hang forever when interface is up again.
    	// RT2860 PCI
    	if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE) &&
        	OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_ADVANCE_POWER_SAVE_PCIE_DEVICE))
    	{
        	AUTO_WAKEUP_STRUC AutoWakeupCfg;
			AsicForceWakeup(pAd, TRUE);
        	AutoWakeupCfg.word = 0;
	    	RTMP_IO_WRITE32(pAd, AUTO_WAKEUP_CFG, AutoWakeupCfg.word);
        	OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_DOZE);
    	}
	}
#endif // RT2860 //
#endif // CONFIG_STA_SUPPORT //

	// Init
 	pObj = (POS_COOKIE)pAd->OS_Cookie;

	// reset Adapter flags
	RTMP_CLEAR_FLAGS(pAd);

	// increase MODULE use count
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0)
    if (!try_module_get(THIS_MODULE))
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s: cannot reserve module\n", __FUNCTION__));
        return -1;
    }
#else
    MOD_INC_USE_COUNT;
#endif


	// Request interrupt service routine for PCI device
	// register the interrupt routine with the os
	if (pAd->CSRBaseAddress != RT2860_CSR_ADDR)
	{
	RT28XX_IRQ_REQUEST(net_dev);
	}
	else
	{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,22)
    	if ((retval = request_irq(net_dev->irq, rt2860_interrupt, IRQF_SHARED, net_dev->name ,net_dev))) 
#else
	    if ((retval = request_irq(net_dev->irq,rt2860_interrupt, SA_INTERRUPT, net_dev->name ,net_dev))) 
#endif
		{
			printk("RT2860: request_irq  ERROR(%d)\n", retval);
			return retval;
		}
	}


	// Init BssTab & ChannelInfo tabbles for auto channel select.
#ifdef CONFIG_AP_SUPPORT	
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		AutoChBssTableInit(pAd);
		ChannelInfoInit(pAd);
	}
#endif // CONFIG_AP_SUPPORT //


	// Chip & other init
	if (rt28xx_init(net_dev) == FALSE)
		goto err;

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		NdisZeroMemory(pAd->StaCfg.dev_name, 16);
		NdisMoveMemory(pAd->StaCfg.dev_name, net_dev->name, strlen(net_dev->name));
	}
#endif // CONFIG_STA_SUPPORT //	

	// Set up the Mac address
	NdisMoveMemory(net_dev->dev_addr, (void *) pAd->CurrentAddress, 6);

	// Init IRQ parameters
	RT28XX_IRQ_INIT(pAd);


	// Various AP function init
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
#ifdef MBSS_SUPPORT
		/* the function can not be moved to RT2860_probe() even register_netdev()
		   is changed as register_netdevice().
		   Or in some PC, kernel will panic (Fedora 4) */
		RT28xx_MBSS_Init(pAd, net_dev);
#endif // MBSS_SUPPORT //

#ifdef WDS_SUPPORT
		RT28xx_WDS_Init(pAd, net_dev);
#endif // WDS_SUPPORT //

#ifdef MAT_SUPPORT
		MATEngineInit(pAd);
#endif // MAT_SUPPORT //

#ifdef APCLI_SUPPORT
		RT28xx_ApCli_Init(pAd, net_dev);
#endif // APCLI_SUPPORT //
	}
#endif // CONFIG_AP_SUPPORT //

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
#ifdef WPA_SUPPLICANT_SUPPORT
#ifndef NATIVE_WPA_SUPPLICANT_SUPPORT
		{
			union iwreq_data    wrqu;
			// send wireless event to wpa_supplicant for infroming interface down.
			memset(&wrqu, 0, sizeof(wrqu));
			wrqu.data.flags = RT_INTERFACE_UP;
			wireless_send_event(pAd->net_dev, IWEVCUSTOM, &wrqu, NULL);
		}
#endif // NATIVE_WPA_SUPPLICANT_SUPPORT //
#endif // WPA_SUPPLICANT_SUPPORT //

	}
#endif // CONFIG_STA_SUPPORT //

#ifdef MESH_SUPPORT
	RTMP_Mesh_Init(pAd, net_dev, hostname);
#endif // MESH_SUPPORT //	

	// Enable Interrupt
	RT28XX_IRQ_ENABLE(pAd);

	// Now Enable RxTx
	RTMPEnableRxTx(pAd);
	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_START_UP);

	{
	UINT32 reg = 0;
	RTMP_IO_READ32(pAd, 0x1300, &reg);  // clear garbage interrupts
	printk("0x1300 = %08x\n", reg);
	}
	netif_start_queue(net_dev);
	netif_carrier_on(net_dev);
	netif_wake_queue(net_dev);

	{
//	u32 reg;
//	u8  byte;
//	u16 tmp;

//	RTMP_IO_READ32(pAd, XIFS_TIME_CFG, &reg);

//	tmp = 0x0805;
//	reg  = (reg & 0xffff0000) | tmp;
//	RTMP_IO_WRITE32(pAd, XIFS_TIME_CFG, reg);

	}

#if 0
	/* 
	 * debugging helper
	 * 		show the size of main table in Adapter structure
	 *		MacTab  -- 185K
	 *		BATable -- 137K
	 * 		Total 	-- 385K  !!!!! (5/26/2006)
	 */
	printk("sizeof(pAd->MacTab) = %ld\n", sizeof(pAd->MacTab));
	printk("sizeof(pAd->AccessControlList) = %ld\n", sizeof(pAd->AccessControlList));
	printk("sizeof(pAd->ApCfg) = %ld\n", sizeof(pAd->ApCfg));
	printk("sizeof(pAd->BATable) = %ld\n", sizeof(pAd->BATable));	
	BUG();
#endif 

#ifdef CONFIG_STA_SUPPORT
#ifdef RT2860
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
        RTMPInitPCIeLinkCtrlValue(pAd);
#endif // RT2860 //
#endif // CONFIG_STA_SUPPORT //

	return (retval);

err:
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0)
    module_put(THIS_MODULE);
#else
    MOD_DEC_USE_COUNT;
#endif
	return (-1);
} /* End of rt28xx_open */


/* Must not be called for mdev and apdev */
NDIS_STATUS rt_ieee80211_if_setup(struct net_device *dev, PRTMP_ADAPTER pAd)
{
	NDIS_STATUS Status;
	INT     i=0;
	CHAR    slot_name[IFNAMSIZ];
	struct net_device   *device;


	//ether_setup(dev);
	dev->hard_start_xmit = rt28xx_send_packets;

#ifdef IKANOS_VX_1X0
	dev->hard_start_xmit = IKANOS_DataFramesTx;
#endif // IKANOS_VX_1X0 //

//	dev->set_multicast_list = ieee80211_set_multicast_list;
//	dev->change_mtu = ieee80211_change_mtu;
#ifdef CONFIG_STA_SUPPORT
#if WIRELESS_EXT >= 12
	if (pAd->OpMode == OPMODE_STA)
	{
		dev->wireless_handlers = &rt28xx_iw_handler_def;
	}
#endif //WIRELESS_EXT >= 12
#endif // CONFIG_STA_SUPPORT //

#ifdef CONFIG_APSTA_MIXED_SUPPORT
#if WIRELESS_EXT >= 12
	if (pAd->OpMode == OPMODE_AP)
	{
		dev->wireless_handlers = &rt28xx_ap_iw_handler_def;
	}
#endif //WIRELESS_EXT >= 12
#endif // CONFIG_APSTA_MIXED_SUPPORT //

#if WIRELESS_EXT < 21
		dev->get_wireless_stats = rt28xx_get_wireless_stats;
#endif
	dev->get_stats = RT28xx_get_ether_stats;
	dev->open = rt28xx_open;
	dev->stop = rt28xx_close;
//	dev->uninit = ieee80211_if_reinit;
//	dev->destructor = ieee80211_if_free;
	dev->priv_flags = INT_MAIN;
	dev->do_ioctl = rt28xx_ioctl;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24)    
    dev->validate_addr = NULL;
#endif
	// find available device name
	for (i = 0; i < 8; i++)
	{
#ifdef MULTIPLE_CARD_SUPPORT
		if (pAd->MC_RowID >= 0)
			sprintf(slot_name, "ra%02d_%d", pAd->MC_RowID, i);
		else
#endif // MULTIPLE_CARD_SUPPORT //
		sprintf(slot_name, "ra%d", i);
	    
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24)
        device = dev_get_by_name(dev->nd_net, slot_name);
#else
		device = dev_get_by_name(slot_name);
#endif
		if (device != NULL) dev_put(device);
#else
		for (device = dev_base; device != NULL; device = device->next)
		{
			if (strncmp(device->name, slot_name, 4) == 0)
				break;
		}
#endif
		if(device == NULL)  
			break;
	}
	
	if(i == 8)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("No available slot name\n"));
		Status = NDIS_STATUS_FAILURE;
	} 
	else 
	{
#ifdef MULTIPLE_CARD_SUPPORT
		if (pAd->MC_RowID >= 0)
	        sprintf(dev->name, "ra%02d_%d", pAd->MC_RowID, i);
		else
#endif // MULTIPLE_CARD_SUPPORT //
		sprintf(dev->name, "ra%d", i);
		DBGPRINT(RT_DEBUG_INFO, ("Assign the net device name as %s\n", dev->name));
		Status = NDIS_STATUS_SUCCESS;
	}

	return Status;
	
}


#ifdef MULTIPLE_CARD_SUPPORT
/*
========================================================================
Routine Description:
    Get card profile path.

Arguments:
    pAd

Return Value:
    TRUE		- Find a card profile
	FALSE		- use default profile

Note:
========================================================================
*/
extern INT RTMPGetKeyParameter(
    IN  PCHAR   key,
    OUT PCHAR   dest,   
    IN  INT     destsize,
    IN  PCHAR   buffer);

BOOLEAN RTMP_CardInfoRead(
	IN	PRTMP_ADAPTER pAd)
{
#define MC_SELECT_CARDID		0	/* use CARD ID (0 ~ 31) to identify different cards */
#define MC_SELECT_MAC			1	/* use CARD MAC to identify different cards */
#define MC_SELECT_CARDTYPE		2	/* use CARD type (abgn or bgn) to identify different cards */

#define LETTER_CASE_TRANSLATE(txt_p, card_id)			\
	{	UINT32 _len; char _char;						\
		for(_len=0; _len<strlen(card_id); _len++) {		\
			_char = *(txt_p + _len);					\
			if (('A' <= _char) && (_char <= 'Z'))		\
				*(txt_p+_len) = 'a'+(_char-'A');		\
		} }

	struct file *srcf;
	INT retval, orgfsuid, orgfsgid;
   	mm_segment_t orgfs;
	CHAR *buffer, *tmpbuf, card_id_buf[30], RFIC_word[30];
	BOOLEAN flg_match_ok = FALSE;
	INT32 card_select_method;
	INT32 card_free_id, card_nouse_id, card_same_mac_id, card_match_id;
	EEPROM_ANTENNA_STRUC antenna;
	USHORT addr01, addr23, addr45;
	UINT8 mac[6];
	UINT32 data, card_index;
	UCHAR *start_ptr;


	// init
	buffer = kmalloc(MAX_INI_BUFFER_SIZE, MEM_ALLOC_FLAG);
	if (buffer == NULL)
        return FALSE;

	tmpbuf = kmalloc(MAX_PARAM_BUFFER_SIZE, MEM_ALLOC_FLAG);
	if(tmpbuf == NULL)
	{
		kfree(buffer);
        return NDIS_STATUS_FAILURE;
	}

	orgfsuid = current->fsuid;
	orgfsgid = current->fsgid;
	current->fsuid = current->fsgid = 0;
    orgfs = get_fs();
    set_fs(KERNEL_DS);

	// get RF IC type
	RTMP_IO_READ32(pAd, E2PROM_CSR, &data);

	if ((data & 0x30) == 0)
		pAd->EEPROMAddressNum = 6;	// 93C46
	else if ((data & 0x30) == 0x10)
		pAd->EEPROMAddressNum = 8;	// 93C66
	else
		pAd->EEPROMAddressNum = 8;	// 93C86

	antenna.word = RTMP_EEPROM_READ16(pAd, EEPROM_NIC1_OFFSET);

	if ((antenna.field.RfIcType == RFIC_2850) ||
		(antenna.field.RfIcType == RFIC_2750))
	{
		/* ABGN card */
		strcpy(RFIC_word, "abgn");
	}
	else
	{
		/* BGN card */
		strcpy(RFIC_word, "bgn");
	}

	// get MAC address
	addr01 = RTMP_EEPROM_READ16(pAd, 0x04);
	addr23 = RTMP_EEPROM_READ16(pAd, 0x06);
	addr45 = RTMP_EEPROM_READ16(pAd, 0x08);

	mac[0] = (UCHAR)(addr01 & 0xff);
	mac[1] = (UCHAR)(addr01 >> 8);
	mac[2] = (UCHAR)(addr23 & 0xff);
	mac[3] = (UCHAR)(addr23 >> 8);
	mac[4] = (UCHAR)(addr45 & 0xff);
	mac[5] = (UCHAR)(addr45 >> 8);

	// open card information file
	srcf = filp_open(CARD_INFO_PATH, O_RDONLY, 0);
	if (IS_ERR(srcf)) 
	{
		/* card information file does not exist */
			DBGPRINT(RT_DEBUG_TRACE,
				("--> Error %ld opening %s\n", -PTR_ERR(srcf), CARD_INFO_PATH));
		return FALSE;
	}

	if (srcf->f_op && srcf->f_op->read)
	{
		/* card information file exists so reading the card information */
		memset(buffer, 0x00, MAX_INI_BUFFER_SIZE);
		retval = srcf->f_op->read(srcf, buffer, MAX_INI_BUFFER_SIZE, &srcf->f_pos);
		if (retval < 0)
		{
			/* read fail */
				DBGPRINT(RT_DEBUG_TRACE,
					("--> Read %s error %d\n", CARD_INFO_PATH, -retval));
		}
		else
		{
			/* get card selection method */
			memset(tmpbuf, 0x00, MAX_PARAM_BUFFER_SIZE);
			card_select_method = MC_SELECT_CARDTYPE; // default

			if (RTMPGetKeyParameter("SELECT", tmpbuf, 256, buffer))
			{
				if (strcmp(tmpbuf, "CARDID") == 0)
					card_select_method = MC_SELECT_CARDID;
				else if (strcmp(tmpbuf, "MAC") == 0)
					card_select_method = MC_SELECT_MAC;
				else if (strcmp(tmpbuf, "CARDTYPE") == 0)
					card_select_method = MC_SELECT_CARDTYPE;
			}

			DBGPRINT(RT_DEBUG_TRACE,
					("MC> Card Selection = %d\n", card_select_method));

			// init
			card_free_id = -1;
			card_nouse_id = -1;
			card_same_mac_id = -1;
			card_match_id = -1;

			// search current card information records
			for(card_index=0;
				card_index<MAX_NUM_OF_MULTIPLE_CARD;
				card_index++)
			{
				if ((*(UINT32 *)&MC_CardMac[card_index][0] == 0) &&
					(*(UINT16 *)&MC_CardMac[card_index][4] == 0))
				{
					// MAC is all-0 so the entry is available
					MC_CardUsed[card_index] = 0;

					if (card_free_id < 0)
						card_free_id = card_index; // 1st free entry
				}
				else
				{
					if (memcmp(MC_CardMac[card_index], mac, 6) == 0)
					{
						// we find the entry with same MAC
						if (card_same_mac_id < 0)
							card_same_mac_id = card_index; // 1st same entry
					}
					else
					{
						// MAC is not all-0 but used flag == 0
						if ((MC_CardUsed[card_index] == 0) &&
							(card_nouse_id < 0))
						{
							card_nouse_id = card_index; // 1st available entry
						}
					}
				}
			}

			DBGPRINT(RT_DEBUG_TRACE,
					("MC> Free = %d, Same = %d, NOUSE = %d\n",
					card_free_id, card_same_mac_id, card_nouse_id));

			if ((card_same_mac_id >= 0) &&
				((card_select_method == MC_SELECT_CARDID) ||
				(card_select_method == MC_SELECT_CARDTYPE)))
			{
				// same MAC entry is found
				card_match_id = card_same_mac_id;

				if (card_select_method == MC_SELECT_CARDTYPE)
				{
					// for CARDTYPE
					sprintf(card_id_buf, "%02dCARDTYPE%s",
							card_match_id, RFIC_word);

					if ((start_ptr=rtstrstruncasecmp(buffer, card_id_buf)) != NULL)
					{
						// we found the card ID
						LETTER_CASE_TRANSLATE(start_ptr, card_id_buf);
					}
				}
			}
			else
			{
				// the card is 1st plug-in, try to find the match card profile
				switch(card_select_method)
				{
					case MC_SELECT_CARDID: // CARDID
					default:
						if (card_free_id >= 0)
							card_match_id = card_free_id;
						else
							card_match_id = card_nouse_id;
						break;

					case MC_SELECT_MAC: // MAC
						sprintf(card_id_buf, "MAC%02x:%02x:%02x:%02x:%02x:%02x",
								mac[0], mac[1], mac[2],
								mac[3], mac[4], mac[5]);

						/* try to find the key word in the card file */
						if ((start_ptr=rtstrstruncasecmp(buffer, card_id_buf)) != NULL)
						{
							LETTER_CASE_TRANSLATE(start_ptr, card_id_buf);

							/* get the row ID (2 ASCII characters) */
							start_ptr -= 2;
							card_id_buf[0] = *(start_ptr);
							card_id_buf[1] = *(start_ptr+1);
							card_id_buf[2] = 0x00;

							card_match_id = simple_strtol(card_id_buf, 0, 10);
						}
						break;

					case MC_SELECT_CARDTYPE: // CARDTYPE
						card_nouse_id = -1;

						for(card_index=0;
							card_index<MAX_NUM_OF_MULTIPLE_CARD;
							card_index++)
						{
							sprintf(card_id_buf, "%02dCARDTYPE%s",
									card_index, RFIC_word);

							if ((start_ptr=rtstrstruncasecmp(buffer,
														card_id_buf)) != NULL)
							{
								LETTER_CASE_TRANSLATE(start_ptr, card_id_buf);

								if (MC_CardUsed[card_index] == 0)
								{
									/* current the card profile is not used */
									if ((*(UINT32 *)&MC_CardMac[card_index][0] == 0) &&
										(*(UINT16 *)&MC_CardMac[card_index][4] == 0))
									{
										// find it and no previous card use it
										card_match_id = card_index;
										break;
									}
									else
									{
										// ever a card use it
										if (card_nouse_id < 0)
											card_nouse_id = card_index;
									}
								}
							}
						}

						// if not find a free one, use the available one
						if (card_match_id < 0)
							card_match_id = card_nouse_id;
						break;
				}
			}

			if (card_match_id >= 0)
			{
				// make up search keyword
				switch(card_select_method)
				{
					case MC_SELECT_CARDID: // CARDID
						sprintf(card_id_buf, "%02dCARDID", card_match_id);
						break;

					case MC_SELECT_MAC: // MAC
						sprintf(card_id_buf,
								"%02dmac%02x:%02x:%02x:%02x:%02x:%02x",
								card_match_id,
								mac[0], mac[1], mac[2],
								mac[3], mac[4], mac[5]);
						break;

					case MC_SELECT_CARDTYPE: // CARDTYPE
					default:
						sprintf(card_id_buf, "%02dcardtype%s",
								card_match_id, RFIC_word);
						break;
				}

				DBGPRINT(RT_DEBUG_TRACE, ("Search Keyword = %s\n", card_id_buf));

				// read card file path
				if (RTMPGetKeyParameter(card_id_buf, tmpbuf, 256, buffer))
				{
					if (strlen(tmpbuf) < sizeof(pAd->MC_FileName))
					{
						// backup card information
						pAd->MC_RowID = card_match_id; /* base 0 */
						MC_CardUsed[card_match_id] = 1;
						memcpy(MC_CardMac[card_match_id], mac, sizeof(mac));

						// backup card file path
						NdisMoveMemory(pAd->MC_FileName, tmpbuf , strlen(tmpbuf));
						pAd->MC_FileName[strlen(tmpbuf)] = '\0';
						flg_match_ok = TRUE;
	
						DBGPRINT(RT_DEBUG_TRACE,
								("Card Profile Name = %s\n", pAd->MC_FileName));
					}
					else
					{
						DBGPRINT(RT_DEBUG_ERROR,
								("Card Profile Name length too large!\n"));
					}
				}
				else
				{
					DBGPRINT(RT_DEBUG_ERROR,
							("Can not find search key word in card.dat!\n"));
				}

				if ((flg_match_ok != TRUE) &&
					(card_match_id < MAX_NUM_OF_MULTIPLE_CARD))
				{
					MC_CardUsed[card_match_id] = 0;
					memset(MC_CardMac[card_match_id], 0, sizeof(mac));
				}
			} // if (card_match_id >= 0)
		}
	}

	// close file
	retval = filp_close(srcf, NULL);
	set_fs(orgfs);
	current->fsuid = orgfsuid;
	current->fsgid = orgfsgid;
	kfree(buffer);
	kfree(tmpbuf);
	return flg_match_ok;
}
#endif // MULTIPLE_CARD_SUPPORT //

#if defined(CONFIG_RT2860V2_AP_RBUS_PCI) || defined(CONFIG_RT2860V2_STA_RBUS_PCI)
/*
========================================================================
Routine Description:
    Probe RT28XX chipset.

Arguments:
    _dev_p				Point to the PCI or USB device
	_dev_id_p			Point to the PCI or USB device ID

Return Value:
    0					Probe OK
	-ENODEV				Probe Fail

Note:
========================================================================
*/
INT __devinit   rt28xx_probe(
    IN  void *_dev_p,
    IN  void *_dev_id_p,
	IN  UINT argc,
	OUT PRTMP_ADAPTER *ppAd)
{
    struct  net_device	*net_dev;
    PRTMP_ADAPTER       pAd = (PRTMP_ADAPTER) NULL;
    INT                 status;
	PVOID				handle;
#ifdef RT2860
	struct pci_dev *dev_p = (struct pci_dev *)_dev_p;
#endif // RT2860 //
#ifdef RT2870
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)	/* kernel 2.4 series */
	struct usb_device *dev_p = (struct usb_device *)_dev_p;
#else
	struct usb_interface *intf = (struct usb_interface *)_dev_p;
	struct usb_device *dev_p = interface_to_usbdev(intf);

	dev_p = usb_get_dev(dev_p);
#endif // LINUX_VERSION_CODE //
#endif // RT2870 //

#ifdef CONFIG_AP_SUPPORT
    DBGPRINT(RT_DEBUG_TRACE, ("AP Driver version-%s\n", AP_DRIVER_VERSION));
#endif // CONFIG_AP_SUPPORT //

#ifdef CONFIG_STA_SUPPORT
    DBGPRINT(RT_DEBUG_TRACE, ("STA Driver version-%s\n", STA_DRIVER_VERSION));
#endif // CONFIG_STA_SUPPORT //

	// Check chipset vendor/product ID
//	if (RT28XXChipsetCheck(_dev_p) == FALSE)
//		goto err_out;

#if LINUX_VERSION_CODE <= 0x20402       // Red Hat 7.1
    net_dev = alloc_netdev(sizeof(PRTMP_ADAPTER), "eth%d", ether_setup);
#else
    net_dev = alloc_etherdev(sizeof(PRTMP_ADAPTER));
#endif
    if (net_dev == NULL)
    {
        printk("alloc_netdev failed\n");

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0)
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,15)
		module_put(THIS_MODULE);
#endif //LINUX_VERSION_CODE < KERNEL_VERSION(2,6,15)
#else
		MOD_DEC_USE_COUNT;
#endif
        goto err_out;
    }

// sample
//	if (rt_ieee80211_if_setup(net_dev) != NDIS_STATUS_SUCCESS)
//		goto err_out;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24)
    SET_MODULE_OWNER(net_dev);
#endif 

	netif_stop_queue(net_dev);
#ifdef NATIVE_WPA_SUPPLICANT_SUPPORT
/* for supporting Network Manager */
/* Set the sysfs physical device reference for the network logical device
 * if set prior to registration will cause a symlink during initialization.
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
    SET_NETDEV_DEV(net_dev, &(dev_p->dev));
#endif
#endif // NATIVE_WPA_SUPPLICANT_SUPPORT //

	// Allocate RTMP_ADAPTER miniport adapter structure
	handle = kmalloc(sizeof(struct os_cookie), GFP_KERNEL);
	RT28XX_HANDLE_DEV_ASSIGN(handle, dev_p);

	status = RTMPAllocAdapterBlock(handle, &pAd);
	if (status != NDIS_STATUS_SUCCESS) 
		goto err_out_free_netdev;

	net_dev->priv = (PVOID)pAd;
    pAd->net_dev = net_dev; // must be before RT28XXNetDevInit()

	RT28XXNetDevInit(_dev_p, net_dev, pAd);

#ifdef CONFIG_STA_SUPPORT
    pAd->StaCfg.OriDevType = net_dev->type;
#endif // CONFIG_STA_SUPPORT //

	// Find and assign a free interface name, raxx
//	RT28XXAvailRANameAssign(net_dev->name);

	// Post config
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
	if (RT28XXProbePostConfig(_dev_p, pAd, argc) == FALSE)
		goto err_out_unmap;
#else
	if (RT28XXProbePostConfig(_dev_p, pAd, 0) == FALSE)
		goto err_out_unmap;
#endif // LINUX_VERSION_CODE //

#ifdef CONFIG_STA_SUPPORT
	pAd->OpMode = OPMODE_STA;
#endif // CONFIG_STA_SUPPORT //

#ifdef CONFIG_AP_SUPPORT
	pAd->OpMode = OPMODE_AP;
#endif // CONFIG_AP_SUPPORT //

#ifdef MULTIPLE_CARD_SUPPORT
	// find its profile path
	pAd->MC_RowID = -1; // use default profile path
	RTMP_CardInfoRead(pAd);

	if (pAd->MC_RowID == -1)
#ifdef CONFIG_AP_SUPPORT
		strcpy(pAd->MC_FileName, AP_PROFILE_PATH);
#endif // CONFIG_AP_SUPPORT //
#ifdef CONFIG_STA_SUPPORT
		strcpy(pAd->MC_FileName, STA_PROFILE_PATH);
#endif // CONFIG_STA_SUPPORT //

	DBGPRINT(RT_DEBUG_TRACE,
			("MC> ROW = %d, PATH = %s\n", pAd->MC_RowID, pAd->MC_FileName));
#endif // MULTIPLE_CARD_SUPPORT //

	// sample move
	if (rt_ieee80211_if_setup(net_dev, pAd) != NDIS_STATUS_SUCCESS)
		goto err_out_unmap;

    // Register this device
    status = register_netdev(net_dev);
    if (status)
        goto err_out_unmap;

    // Set driver data
	RT28XX_DRVDATA_SET(_dev_p);

#ifdef WSC_INCLUDED
	InitializeWSCTLV();
    init_MUTEX_LOCKED(&(pAd->write_dat_file_semaphore));
    init_completion(&pAd->write_dat_file_notify);
    start_write_dat_file_thread(pAd);
    
    if (pAd->pHmacData)
        kfree(pAd->pHmacData);
    pAd->pHmacData = kmalloc(sizeof(CHAR)*(2048), MEM_ALLOC_FLAG);
    if (pAd->pHmacData == NULL)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("Wsc HmacData memory alloc failed!\n"));
    }
#endif // WSC_INCLUDED //

#ifdef CONFIG_AP_SUPPORT
	pAd->ApCfg.MBSSID[MAIN_MBSSID].MSSIDDev = net_dev;
#endif // CONFIG_AP_SUPPORT //

	*ppAd = pAd;
    return 0; // probe ok


	/* --------------------------- ERROR HANDLE --------------------------- */
err_out_unmap:
	RTMPFreeAdapter(pAd);
	RT28XX_UNMAP();
	
err_out_free_netdev:
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0)
    free_netdev(net_dev);
#else
	kfree(net_dev);
#endif

err_out:
	RT28XX_PUT_DEVICE(dev_p);

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
	return (LONG)NULL;
#else
    return -ENODEV; /* probe fail */
#endif // LINUX_VERSION_CODE //
} /* End of rt28xx_probe */
#endif // #if defined(CONFIG_RT2860V2_AP_RBUS_PCI) || defined(CONFIG_RT2860V2_STA_RBUS_PCI) //


/*
========================================================================
Routine Description:
    The entry point for Linux kernel sent packet to our driver.

Arguments:
    sk_buff *skb		the pointer refer to a sk_buffer.

Return Value:
    0					

Note:
	This function is the entry point of Tx Path for Os delivery packet to 
	our driver. You only can put OS-depened & STA/AP common handle procedures 
	in here.
========================================================================
*/
int rt28xx_packet_xmit(struct sk_buff *skb)
{
	struct net_device *net_dev = skb->dev;
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) net_dev->priv;
	int status = 0;
	PNDIS_PACKET pPacket = (PNDIS_PACKET) skb;

	/* RT2870STA does this in RTMPSendPackets() */
#ifdef RALINK_ATE
	if (pAd->ate.Mode != ATE_STOP)
	{
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_RESOURCES);
		return 0;
	}
#endif // RALINK_ATE //

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		// Drop send request since we are in monitor mode
		if (MONITOR_ON(pAd))
		{
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
			goto done;
		}
	}
#endif // CONFIG_STA_SUPPORT //

        // EapolStart size is 18
	if (skb->len < 14)
	{
		//printk("bad packet size: %d\n", pkt->len);
		hex_dump("bad packet", skb->data, skb->len);
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
		goto done;
	}
	
#if !defined(CONFIG_RA_NAT_NONE)
/* bruce+
 */
	unsigned int flags;
         if(ra_sw_nat_hook_tx!= NULL)
         {
           spin_lock_irqsave(pAd->page_lock, flags);
           ra_sw_nat_hook_tx(pPacket);
           spin_unlock_irqrestore(pAd->page_lock, flags);
         }
#endif

#if 0
//	if ((pkt->data[0] & 0x1) == 0)
	{
		//hex_dump(__FUNCTION__, pkt->data, pkt->len);
		printk("pPacket = %x\n", pPacket);
	}
#endif

	RTMP_SET_PACKET_5VT(pPacket, 0);
//	MiniportMMRequest(pAd, pkt->data, pkt->len);
#ifdef CONFIG_5VT_ENHANCE
    if (*(int*)(skb->cb) == BRIDGE_TAG) {
		RTMP_SET_PACKET_5VT(pPacket, 1);
    }
#endif


#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		APSendPackets((NDIS_HANDLE)pAd, (PPNDIS_PACKET) &pPacket, 1);
#endif // CONFIG_AP_SUPPORT //

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{

		STASendPackets((NDIS_HANDLE)pAd, (PPNDIS_PACKET) &pPacket, 1);
	}

#endif // CONFIG_STA_SUPPORT //

	status = 0;
done:
			   
	return status;
}


/*
========================================================================
Routine Description:
    Send a packet to WLAN.

Arguments:
    skb_p           points to our adapter
    dev_p           which WLAN network interface

Return Value:
    0: transmit successfully
    otherwise: transmit fail

Note:
========================================================================
*/
INT rt28xx_send_packets(
	IN struct sk_buff 		*skb_p, 
	IN struct net_device 	*net_dev)
{
	NdisZeroMemory((PUCHAR)&skb_p->cb[CB_OFF], 13);
	RTMP_SET_PACKET_NET_DEVICE_MBSSID(skb_p, MAIN_MBSSID);

		return rt28xx_packet_xmit(skb_p);

} /* End of MBSS_VirtualIF_PacketSend */




#if LINUX_VERSION_CODE <= 0x20402	// Red Hat 7.1
//static struct net_device *alloc_netdev(int sizeof_priv, const char *mask, void (*setup)(struct net_device *)) //sample
struct net_device *alloc_netdev(
	int sizeof_priv,
	const char *mask,
	void (*setup)(struct net_device *))
{
    struct net_device	*dev;
    INT					alloc_size;


    /* ensure 32-byte alignment of the private area */
    alloc_size = sizeof (*dev) + sizeof_priv + 31;

    dev = (struct net_device *) kmalloc(alloc_size, GFP_KERNEL);
    if (dev == NULL)
    {
        DBGPRINT(RT_DEBUG_ERROR,
				("alloc_netdev: Unable to allocate device memory.\n"));
        return NULL;
    }

    memset(dev, 0, alloc_size);

    if (sizeof_priv)
        dev->priv = (void *) (((long)(dev + 1) + 31) & ~31);

    setup(dev);
    strcpy(dev->name, mask);

    return dev;
}
#endif // LINUX_VERSION_CODE //


void CfgInitHook(PRTMP_ADAPTER pAd)
{
	//OID_SET_HT_PHYMODE HTPhyMode;
	//int i, KeyIdx;
	//UCHAR CipherAlg;
	//PUCHAR	Key;


	pAd->bBroadComHT = TRUE;

	// WEP mode
#if 0
	for (i=0; i<MAX_MBSSID_NUM; i++)
	{	
		int j, keyid;

		pAd->ApCfg.MBSSID[i].AuthMode = Ndis802_11AuthModeOpen;
		pAd->ApCfg.MBSSID[i].WepStatus = Ndis802_11WEPEnabled;
		pAd->ApCfg.MBSSID[i].DefaultKeyId = keyid = i%4;

		CipherAlg = pAd->SharedKey[i][keyid].CipherAlg = CIPHER_WEP64;

		for (j=0; j<5; j++)
		{ 
			pAd->SharedKey[i][keyid].Key[j] = '0'+i;
		}

		Key = pAd->SharedKey[i][keyid].Key;

		AsicAddSharedKeyEntry(pAd, i, keyid, CipherAlg, Key, NULL, NULL);
		
	}
#endif


//	pAd->CommonCfg.BACapability.field.MpduDensity = 4;
//	pAd->CommonCfg.BACapability.field.AutoBA = TRUE;
//	pAd->CommonCfg.BACapability.field.RxBAWinLimit = 16;
//	pAd->CommonCfg.PhyMode = PHY_11A;

#if 0
	pAd->CommonCfg.PhyMode = PHY_11N;
	pAd->CommonCfg.BACapability.field.RxBAWinLimit = 16;

	HTPhyMode.BW = BW_40;
	HTPhyMode.PhyMode = PHY_11N;
	HTPhyMode.ExtOffset = EXTCHA_NONE;
	HTPhyMode.TransmitNo = 1;
	HTPhyMode.HtMode = HTMODE_GF;
	HTPhyMode.SHORTGI = GI_400; 
	HTPhyMode.STBC = STBC_NONE;
	HTPhyMode.MCS = MCS_AUTO;

	if ((pAd->CommonCfg.PhyMode >= PHY_11ABGN_MIXED))
		RTMPSetHT(pAd, &HTPhyMode);
#endif // 0


	//NdisMoveMemory(pAd->ApCfg.MBSSID[apidx].Ssid, "Sam_AP", 6);
	//pAd->ApCfg.MBSSID[apidx].SsidLen = 6;

#if 0
	{
		UCHAR *key = "12345678";
		UCHAR keyMaterial[40];
		int i;
		
		// set WPAPSK 
		pAd->ApCfg.MBSSID[apidx].AuthMode = Ndis802_11AuthModeWPAPSK;

		// set TKIP
		pAd->ApCfg.MBSSID[apidx].WepStatus = Ndis802_11Encryption2Enabled;
	
		// Init some variable
		for (i=0; i<MAX_LEN_OF_MAC_TABLE; i++)
		{
			if (pAd->MacTab.Content[i].ValidAsCLI)
			{
				pAd->MacTab.Content[i].PortSecured  = WPA_802_1X_PORT_NOT_SECURED;
			}
		}
		pAd->ApCfg.PortSecured = WPA_802_1X_PORT_NOT_SECURED;
    
		ApCfg.MBSSID[apidx].DefaultKeyId  = 0;
		if(pAd->ApCfg.MBSSID[apidx].AuthMode >= Ndis802_11AuthModeWPA)
		{   
			pAd->ApCfg.WpaGTKState = SETKEYS;
			pAd->ApCfg.GKeyDoneStations = pAd->MacTab.Size;
			ApCfg.MBSSID[apidx].DefaultKeyId = 1;		
		}

#ifdef CONFIG_STA_SUPPORT
		//pAd->StaCfg.PskKey.KeyLen = 32;
		NdisMoveMemory(pAd->StaCfg.PMK, keyMaterial, 32);
		// Use RaConfig as PSK agent.
		// Start STA supplicant state machine
		pAd->StaCfg.WpaState = SS_START;
		hex_dump("PMK", pAd->StaCfg.PMK, 32);
#endif // CONFIG_STA_SUPPORT //
#ifdef CONFIG_AP_SUPPORT
		// set WPAPSK key
		PasswordHash((CHAR *)key, pAd->ApCfg.MBSSID[apidx].Ssid, pAd->ApCfg.MBSSID[apidx].SsidLen, keyMaterial);
    	NdisMoveMemory(pAd->ApCfg.PMK, keyMaterial, 32);
#endif // CONFIG_AP_SUPPORT //
	
		RTMPMakeRSNIE(pAd, pAd->ApCfg.MBSSID[apidx].AuthMode, pAd->ApCfg.MBSSID[apidx].WepStatus, apidx);
	}
#endif
} /* End of CfgInitHook */


#if 0	// Not used now, should keep it in our source tree??
/*
========================================================================
Routine Description:
    Find and assign a free interface name (raxx).

Arguments:
    *name_p				the interface name pointer

Return Value:
	TRUE				OK
	FALSE				FAIL

Note:
========================================================================
*/
static BOOLEAN RT28XXAvailRANameAssign(
	IN CHAR			*name_p)
{
    CHAR				slot_name[IFNAMSIZ];
    struct net_device	*device;
	UINT32				if_id;


    for(if_id=0; if_id<8; if_id++)
    {
        sprintf(slot_name, "ra%d", if_id);

        for(device=dev_base; device!=NULL; device=device->next)
        {
            if (strncmp(device->name, slot_name, 4) == 0)
                break;
        }

        if (device == NULL)
			break;
    }

    if (if_id == 8)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("No available slot name\n"));
        return FALSE;
    }

    sprintf(name_p, "ra%d", if_id);
	return TRUE;
} /* End of RT28XXAvailRANameAssign */
#endif

#if WIRELESS_EXT >= 12
// This function will be called when query /proc
struct iw_statistics *rt28xx_get_wireless_stats(
    IN struct net_device *net_dev)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) net_dev->priv;


	DBGPRINT(RT_DEBUG_TRACE, ("rt28xx_get_wireless_stats --->\n"));
	
	// TODO: All elements are zero before be implemented

	pAd->iw_stats.status = 0; // Status - device dependent for now

	// link quality (%retries, SNR, %missed beacons or better...)
	pAd->iw_stats.qual.qual = pAd->Mlme.ChannelQuality;

	// signal level (dBm)
#if 0
	pAd->iw_stats.qual.level = abs(pAd->StaCfg.LastRssi);
	pAd->iw_stats.qual.level += 256 - pAd->BbpRssiToDbmDelta;
#else
#ifdef CONFIG_STA_SUPPORT
	if (pAd->OpMode == OPMODE_STA)
		pAd->iw_stats.qual.level = pAd->StaCfg.RssiSample.LastRssi0 - (CHAR)pAd->BbpRssiToDbmDelta;
#endif // CONFIG_STA_SUPPORT //
#endif

	pAd->iw_stats.qual.noise = pAd->BbpWriteLatch[66]; // noise level (dBm)
	
	pAd->iw_stats.qual.noise += 256 - 143;
	pAd->iw_stats.qual.updated = 1;     // Flags to know if updated

	pAd->iw_stats.discard.nwid = 0;     // Rx : Wrong nwid/essid
	pAd->iw_stats.miss.beacon = 0;      // Missed beacons/superframe
	
	// pAd->iw_stats.discard.code, discard.fragment, discard.retries,
	// discard.misc has counted in other place
	DBGPRINT(RT_DEBUG_TRACE, ("<--- rt28xx_get_wireless_stats\n"));
	return &pAd->iw_stats;
} /* End of rt28xx_get_wireless_stats */
#endif // WIRELESS_EXT //

#ifdef CONFIG_AP_SUPPORT
#ifdef DFS_SUPPORT
void pulse_radar_detect_tasklet(unsigned long data)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) data;
	POS_COOKIE pObj;
	
    pObj = (POS_COOKIE) pAd->OS_Cookie;

	RadarSMDetect(pAd, RADAR_PULSE);
}

void width_radar_detect_tasklet(unsigned long data)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) data;
	POS_COOKIE pObj;
	
    pObj = (POS_COOKIE) pAd->OS_Cookie;

	RadarSMDetect(pAd, RADAR_WIDTH);
}
#endif // DFS_SUPPORT //

#ifdef CARRIER_DETECTION_SUPPORT
void carrier_sense_tasklet(unsigned long data)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) data;
	POS_COOKIE pObj;
	
    pObj = (POS_COOKIE) pAd->OS_Cookie;

	CarrierDetectionCheck(pAd);
}
#endif // CARRIER_DETECTION_SUPPORT //
#endif // CONFIG_AP_SUPPORT //


void tbtt_tasklet(unsigned long data)
{
#define MAX_TX_IN_TBTT		(16)

#ifdef CONFIG_AP_SUPPORT
		PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) data;

#ifdef RT2860
	if (pAd->OpMode == OPMODE_AP)
	{
		/* update channel utilization */
		QBSS_LoadUpdate(pAd);
	}
#endif // RT2860 //

	if (pAd->OpMode == OPMODE_AP)
	{
		//
		// step 7 - if DTIM, then move backlogged bcast/mcast frames from PSQ to TXQ whenever DtimCount==0
#if 0    
		// NOTE: This updated BEACON frame will be sent at "next" TBTT instead of at cureent TBTT. The reason is
		//       because ASIC already fetch the BEACON content down to TX FIFO before driver can make any
		//       modification. To compenstate this effect, the actual time to deilver PSQ frames will be
		//       at the time that we wrapping around DtimCount from 0 to DtimPeriod-1
		if ((pAd->ApCfg.DtimCount + 1) == pAd->ApCfg.DtimPeriod)
#else
		if (pAd->ApCfg.DtimCount == 0)
#endif
		{
			PQUEUE_ENTRY    pEntry;
			BOOLEAN			bPS = FALSE;
			UINT 			count = 0;
			unsigned long 		IrqFlags;

//			NdisAcquireSpinLock(&pAd->MacTabLock);
//			NdisAcquireSpinLock(&pAd->TxSwQueueLock);
			
			RTMP_IRQ_LOCK(&pAd->irq_lock, IrqFlags);
			while (pAd->MacTab.McastPsQueue.Head)
			{
				bPS = TRUE;
				if (pAd->TxSwQueue[QID_AC_BE].Number <= (MAX_PACKETS_IN_QUEUE + MAX_PACKETS_IN_MCAST_PS_QUEUE))
				{
					pEntry = RemoveHeadQueue(&pAd->MacTab.McastPsQueue);
					//if(pAd->MacTab.McastPsQueue.Number)
					if (count)
					{
						RTMP_SET_PACKET_MOREDATA(pEntry, TRUE);
					}
					InsertHeadQueue(&pAd->TxSwQueue[QID_AC_BE], pEntry);
					count++;
				}
				else
				{
					break;
				}
			}
			RTMP_IRQ_UNLOCK(&pAd->irq_lock, IrqFlags);
			
			DBGPRINT(RT_DEBUG_INFO, ("DTIM=%d/%d, tx mcast/bcast out...\n",pAd->ApCfg.DtimCount,pAd->ApCfg.DtimPeriod));
//			NdisReleaseSpinLock(&pAd->TxSwQueueLock);
//			NdisReleaseSpinLock(&pAd->MacTabLock);
			if (pAd->MacTab.McastPsQueue.Number == 0)
			{			
                UINT bss_index;

                /* clear MCAST/BCAST backlog bit for all BSS */
                for(bss_index=BSS0; bss_index<pAd->ApCfg.BssidNum; bss_index++)
					WLAN_MR_TIM_BCMC_CLEAR(bss_index);
                /* End of for */
			}
			pAd->MacTab.PsQIdleCount = 0;

			// Dequeue outgoing framea from TxSwQueue0..3 queue and process it
            if (bPS == TRUE) 
			{
				RTMPDeQueuePacket(pAd, TRUE, NUM_OF_TX_RING, /*MAX_TX_IN_TBTT*/MAX_PACKETS_IN_MCAST_PS_QUEUE);
			}
		}
	}
#endif // CONFIG_AP_SUPPORT //
}

INT rt28xx_ioctl(
	IN	struct net_device	*net_dev, 
	IN	OUT	struct ifreq	*rq, 
	IN	INT					cmd)
{
#ifdef CONFIG_AP_SUPPORT
	VIRTUAL_ADAPTER	*pVirtualAd = NULL;
#endif // CONFIG_AP_SUPPORT //
	RTMP_ADAPTER	*pAd = NULL;
	INT				ret = 0;

	if (net_dev->priv_flags == INT_MAIN)
	{
		pAd = net_dev->priv;
	}
#ifdef CONFIG_AP_SUPPORT
	else
	{
		pVirtualAd = net_dev->priv;
		pAd = pVirtualAd->RtmpDev->priv;
	}
#endif // CONFIG_AP_SUPPORT //

	if (pAd == NULL)
	{
		/* if 1st open fail, pAd will be free;
		   So the net_dev->priv will be NULL in 2rd open */
		return -ENETDOWN;
	}

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		ret = rt28xx_ap_ioctl(net_dev, rq, cmd);
	}
#endif // CONFIG_AP_SUPPORT //

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		ret = rt28xx_sta_ioctl(net_dev, rq, cmd);
	}
#endif // CONFIG_STA_SUPPORT //

	return ret;
}

/*
    ========================================================================

    Routine Description:
        return ethernet statistics counter

    Arguments:
        net_dev                     Pointer to net_device

    Return Value:
        net_device_stats*

    Note:

    ========================================================================
*/
struct net_device_stats *RT28xx_get_ether_stats(
    IN  struct net_device *net_dev)
{
    RTMP_ADAPTER *pAd = net_dev->priv;

    DBGPRINT(RT_DEBUG_TRACE, ("RT28xx_get_ether_stats --->\n"));

	pAd->stats.rx_packets = pAd->WlanCounters.ReceivedFragmentCount.QuadPart;
	pAd->stats.tx_packets = pAd->WlanCounters.TransmittedFragmentCount.QuadPart;

	pAd->stats.rx_bytes = pAd->RalinkCounters.ReceivedByteCount;
	pAd->stats.tx_bytes = pAd->RalinkCounters.TransmittedByteCount;

	pAd->stats.rx_errors = pAd->Counters8023.RxErrors;
	pAd->stats.tx_errors = pAd->Counters8023.TxErrors;

	pAd->stats.rx_dropped = 0;
	pAd->stats.tx_dropped = 0;

    pAd->stats.multicast = pAd->WlanCounters.MulticastReceivedFrameCount.QuadPart;   // multicast packets received
    pAd->stats.collisions = pAd->Counters8023.OneCollision + pAd->Counters8023.MoreCollisions;  // Collision packets

    pAd->stats.rx_length_errors = 0;
    pAd->stats.rx_over_errors = pAd->Counters8023.RxNoBuffer;                   // receiver ring buff overflow
    pAd->stats.rx_crc_errors = 0;//pAd->WlanCounters.FCSErrorCount;     // recved pkt with crc error
    pAd->stats.rx_frame_errors = pAd->Counters8023.RcvAlignmentErrors;          // recv'd frame alignment error
    pAd->stats.rx_fifo_errors = pAd->Counters8023.RxNoBuffer;                   // recv'r fifo overrun
    pAd->stats.rx_missed_errors = 0;                                            // receiver missed packet

    // detailed tx_errors
    pAd->stats.tx_aborted_errors = 0;
    pAd->stats.tx_carrier_errors = 0;
    pAd->stats.tx_fifo_errors = 0;
    pAd->stats.tx_heartbeat_errors = 0;
    pAd->stats.tx_window_errors = 0;

    // for cslip etc
    pAd->stats.rx_compressed = 0;
    pAd->stats.tx_compressed = 0;

    return &pAd->stats;
}

