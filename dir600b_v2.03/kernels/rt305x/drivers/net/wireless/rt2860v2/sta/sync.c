/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
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
	sync.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	John Chang	2004-09-01      modified for rt2561/2661
	Jan Lee		2006-08-01      modified for rt2860 for 802.11n
*/
#include "rt_config.h"
#ifdef MESH_SUPPORT
#include "mesh_sanity.h"
#endif // MESH_SUPPORT //

#if 0 // this portion already move to cmm_sync.c
// 2.4 Ghz channel plan index in the TxPower arrays.
#define	BG_BAND_REGION_0_START	0			// 1,2,3,4,5,6,7,8,9,10,11	
#define	BG_BAND_REGION_0_SIZE	11
#define	BG_BAND_REGION_1_START	0			// 1,2,3,4,5,6,7,8,9,10,11,12,13
#define	BG_BAND_REGION_1_SIZE	13
#define	BG_BAND_REGION_2_START	9			// 10,11
#define	BG_BAND_REGION_2_SIZE	2
#define	BG_BAND_REGION_3_START	9			// 10,11,12,13
#define	BG_BAND_REGION_3_SIZE	4
#define	BG_BAND_REGION_4_START	13			// 14
#define	BG_BAND_REGION_4_SIZE	1
#define	BG_BAND_REGION_5_START	0			// 1,2,3,4,5,6,7,8,9,10,11,12,13,14 
#define	BG_BAND_REGION_5_SIZE	14
#define	BG_BAND_REGION_6_START	2			// 3,4,5,6,7,8,9
#define	BG_BAND_REGION_6_SIZE	7
#define	BG_BAND_REGION_7_START	4			// 5,6,7,8,9,10,11,12,13
#define	BG_BAND_REGION_7_SIZE	9

// 5 Ghz channel plan index in the TxPower arrays.
UCHAR A_BAND_REGION_0_CHANNEL_LIST[]={36, 40, 44, 48, 52, 56, 60, 64, 149, 153, 157, 161, 165};
UCHAR A_BAND_REGION_1_CHANNEL_LIST[]={36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140};
UCHAR A_BAND_REGION_2_CHANNEL_LIST[]={36, 40, 44, 48, 52, 56, 60, 64};
UCHAR A_BAND_REGION_3_CHANNEL_LIST[]={52, 56, 60, 64, 149, 153, 157, 161};
UCHAR A_BAND_REGION_4_CHANNEL_LIST[]={149, 153, 157, 161, 165};
UCHAR A_BAND_REGION_5_CHANNEL_LIST[]={149, 153, 157, 161};
UCHAR A_BAND_REGION_6_CHANNEL_LIST[]={36, 40, 44, 48};
UCHAR A_BAND_REGION_7_CHANNEL_LIST[]={36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 149, 153, 157, 161, 165};
UCHAR A_BAND_REGION_8_CHANNEL_LIST[]={52, 56, 60, 64};
UCHAR A_BAND_REGION_9_CHANNEL_LIST[]={100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140};

//BaSizeArray follows the 802.11n definition as MaxRxFactor.  2^(13+factor) bytes. When factor =0, it's about Ba buffer size =8.
UCHAR	BaSizeArray[4] = {8,16,32,64};
#endif

/*
	==========================================================================
	Description:
		The sync state machine, 
	Parameters:
		Sm - pointer to the state machine
	Note:
		the state machine looks like the following

	==========================================================================
 */
VOID SyncStateMachineInit(
	IN PRTMP_ADAPTER pAd, 
	IN STATE_MACHINE *Sm, 
	OUT STATE_MACHINE_FUNC Trans[]) 
{
	StateMachineInit(Sm, Trans, MAX_SYNC_STATE, MAX_SYNC_MSG, (STATE_MACHINE_FUNC)Drop, SYNC_IDLE, SYNC_MACHINE_BASE);

	// column 1
	StateMachineSetAction(Sm, SYNC_IDLE, MT2_MLME_SCAN_REQ, (STATE_MACHINE_FUNC)MlmeScanReqAction);
	StateMachineSetAction(Sm, SYNC_IDLE, MT2_MLME_JOIN_REQ, (STATE_MACHINE_FUNC)MlmeJoinReqAction);
	StateMachineSetAction(Sm, SYNC_IDLE, MT2_MLME_START_REQ, (STATE_MACHINE_FUNC)MlmeStartReqAction);
	StateMachineSetAction(Sm, SYNC_IDLE, MT2_PEER_BEACON, (STATE_MACHINE_FUNC)PeerBeacon);
	StateMachineSetAction(Sm, SYNC_IDLE, MT2_PEER_PROBE_REQ, (STATE_MACHINE_FUNC)PeerProbeReqAction); 

	//column 2
	StateMachineSetAction(Sm, JOIN_WAIT_BEACON, MT2_MLME_SCAN_REQ, (STATE_MACHINE_FUNC)InvalidStateWhenScan);
	StateMachineSetAction(Sm, JOIN_WAIT_BEACON, MT2_MLME_JOIN_REQ, (STATE_MACHINE_FUNC)InvalidStateWhenJoin);
	StateMachineSetAction(Sm, JOIN_WAIT_BEACON, MT2_MLME_START_REQ, (STATE_MACHINE_FUNC)InvalidStateWhenStart);
	StateMachineSetAction(Sm, JOIN_WAIT_BEACON, MT2_PEER_BEACON, (STATE_MACHINE_FUNC)PeerBeaconAtJoinAction);
	StateMachineSetAction(Sm, JOIN_WAIT_BEACON, MT2_BEACON_TIMEOUT, (STATE_MACHINE_FUNC)BeaconTimeoutAtJoinAction);

	// column 3
	StateMachineSetAction(Sm, SCAN_LISTEN, MT2_MLME_SCAN_REQ, (STATE_MACHINE_FUNC)InvalidStateWhenScan);
	StateMachineSetAction(Sm, SCAN_LISTEN, MT2_MLME_JOIN_REQ, (STATE_MACHINE_FUNC)InvalidStateWhenJoin);
	StateMachineSetAction(Sm, SCAN_LISTEN, MT2_MLME_START_REQ, (STATE_MACHINE_FUNC)InvalidStateWhenStart);
	StateMachineSetAction(Sm, SCAN_LISTEN, MT2_PEER_BEACON, (STATE_MACHINE_FUNC)PeerBeaconAtScanAction);
	StateMachineSetAction(Sm, SCAN_LISTEN, MT2_PEER_PROBE_RSP, (STATE_MACHINE_FUNC)PeerBeaconAtScanAction);
	StateMachineSetAction(Sm, SCAN_LISTEN, MT2_SCAN_TIMEOUT, (STATE_MACHINE_FUNC)ScanTimeoutAction);

	// timer init
	RTMPInitTimer(pAd, &pAd->MlmeAux.BeaconTimer, GET_TIMER_FUNCTION(BeaconTimeout), pAd, FALSE);
	RTMPInitTimer(pAd, &pAd->MlmeAux.ScanTimer, GET_TIMER_FUNCTION(ScanTimeout), pAd, FALSE);
}

/* 
	==========================================================================
	Description:
		Beacon timeout handler, executed in timer thread

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
VOID BeaconTimeout(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3) 
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)FunctionContext;

	DBGPRINT(RT_DEBUG_TRACE,("SYNC - BeaconTimeout\n"));
	
	// Do nothing if the driver is starting halt state.
	// This might happen when timer already been fired before cancel timer with mlmehalt
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS))
		return;
	
	MlmeEnqueue(pAd, SYNC_STATE_MACHINE, MT2_BEACON_TIMEOUT, 0, NULL);
	RT28XX_MLME_HANDLER(pAd);
}

/* 
	==========================================================================
	Description:
		Scan timeout handler, executed in timer thread

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
VOID ScanTimeout(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3) 
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)FunctionContext;

	DBGPRINT(RT_DEBUG_INFO,("SYNC - Scan Timeout \n"));
	
	// Do nothing if the driver is starting halt state.
	// This might happen when timer already been fired before cancel timer with mlmehalt
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS))
		return;
	
	MlmeEnqueue(pAd, SYNC_STATE_MACHINE, MT2_SCAN_TIMEOUT, 0, NULL);
	RT28XX_MLME_HANDLER(pAd);
}

/* 
	==========================================================================
	Description:
		MLME SCAN req state machine procedure
	==========================================================================
 */
VOID MlmeScanReqAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 
{
	UCHAR          Ssid[MAX_LEN_OF_SSID], SsidLen, ScanType, BssType, BBPValue = 0;
	BOOLEAN        TimerCancelled;
	ULONG		   Now;
	USHORT         Status;
	PHEADER_802_11 pHdr80211;
	PUCHAR         pOutBuffer = NULL;	
	NDIS_STATUS    NStatus;

	// Check the total scan tries for one single OID command
	// If this is the CCX 2.0 Case, skip that!
	if ( !RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("SYNC - MlmeScanReqAction before Startup\n"));
		return;
	}
#if 0	
	if (pAd->StaCfg.ScanCnt >= 3)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("SYNC - MlmeScanReqAction: Too many tries already\n"));
		pAd->Mlme.SyncMachine.CurrState = SYNC_IDLE;
		Status = MLME_INVALID_FORMAT;
		MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_SCAN_CONF, 2, &Status);
		return;
	}
#endif

	// Increase the scan retry counters.
	pAd->StaCfg.ScanCnt++;
	
#ifdef RT2860
    if ((OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_ADVANCE_POWER_SAVE_PCIE_DEVICE)) && 
        (IDLE_ON(pAd)) &&
		(pAd->StaCfg.bRadio == TRUE) &&
		(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF)))
	{
		PCIeAsicRadioOn(pAd, GUI_IDLE_POWER_SAVE);
	}
#endif // RT2860 //

	// first check the parameter sanity
	if (MlmeScanReqSanity(pAd, 
						  Elem->Msg, 
						  Elem->MsgLen, 
						  &BssType, 
						  Ssid, 
						  &SsidLen, 
						  &ScanType)) 
	{

		// Check for channel load and noise hist request
		// Suspend MSDU only at scan request, not the last two mentioned
		if ((ScanType == SCAN_CISCO_NOISE) || (ScanType == SCAN_CISCO_CHANNEL_LOAD))
		{
			if (pAd->StaCfg.CCXScanChannel != pAd->CommonCfg.Channel)
				RTMPSuspendMsduTransmission(pAd);			// Suspend MSDU transmission here
		}
		else
		{
			// Suspend MSDU transmission here
			RTMPSuspendMsduTransmission(pAd);
		}	
		
		//
		// To prevent data lost.	
		// Send an NULL data with turned PSM bit on to current associated AP before SCAN progress.
		// And should send an NULL data with turned PSM bit off to AP, when scan progress done 
		//
		if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED) && (INFRA_ON(pAd)))
		{
			NStatus = MlmeAllocateMemory(pAd, (PVOID)&pOutBuffer);
			if (NStatus	== NDIS_STATUS_SUCCESS)
			{
				pHdr80211 = (PHEADER_802_11) pOutBuffer;
				MgtMacHeaderInit(pAd, pHdr80211, SUBTYPE_NULL_FUNC, 1, pAd->CommonCfg.Bssid, pAd->CommonCfg.Bssid);
				pHdr80211->Duration = 0;
				pHdr80211->FC.Type = BTYPE_DATA;
				pHdr80211->FC.PwrMgmt = PWR_SAVE;

				// Send using priority queue
				MiniportMMRequest(pAd, 0, pOutBuffer, sizeof(HEADER_802_11));
				DBGPRINT(RT_DEBUG_TRACE, ("MlmeScanReqAction -- Send PSM Data frame for off channel RM\n"));
				MlmeFreeMemory(pAd, pOutBuffer);
				RTMPusecDelay(5000);
			}
		}
		
		NdisGetSystemUpTime(&Now);
		pAd->StaCfg.LastScanTime = Now;
		// reset all the timers
		RTMPCancelTimer(&pAd->MlmeAux.BeaconTimer, &TimerCancelled);
		RTMPCancelTimer(&pAd->MlmeAux.ScanTimer, &TimerCancelled);

		// record desired BSS parameters
		pAd->MlmeAux.BssType = BssType;
		pAd->MlmeAux.ScanType = ScanType;
		pAd->MlmeAux.SsidLen = SsidLen;
        NdisZeroMemory(pAd->MlmeAux.Ssid, MAX_LEN_OF_SSID);
		NdisMoveMemory(pAd->MlmeAux.Ssid, Ssid, SsidLen);

		// start from the first channel
		pAd->MlmeAux.Channel = FirstChannel(pAd);

		// Change the scan channel when dealing with CCX beacon report
		if ((ScanType == SCAN_CISCO_PASSIVE) || (ScanType == SCAN_CISCO_ACTIVE) ||
			(ScanType == SCAN_CISCO_CHANNEL_LOAD) || (ScanType == SCAN_CISCO_NOISE))
			pAd->MlmeAux.Channel = pAd->StaCfg.CCXScanChannel;
		
		// Let BBP register at 20MHz to do scan		
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &BBPValue);
		BBPValue &= (~0x18);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, BBPValue);
		DBGPRINT(RT_DEBUG_ERROR, ("SYNC - BBP R4 to 20MHz.l\n"));
		ScanNextChannel(pAd);
	} 
	else 
	{
		DBGPRINT_ERR(("SYNC - MlmeScanReqAction() sanity check fail\n"));
		pAd->Mlme.SyncMachine.CurrState = SYNC_IDLE;
		Status = MLME_INVALID_FORMAT;
		MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_SCAN_CONF, 2, &Status);
	}
}

/* 
	==========================================================================
	Description:
		MLME JOIN req state machine procedure
	==========================================================================
 */
VOID MlmeJoinReqAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 
{
	BSS_ENTRY    *pBss;
	BOOLEAN       TimerCancelled;
	HEADER_802_11 Hdr80211;	
	NDIS_STATUS   NStatus;
	ULONG         FrameLen = 0;	
	PUCHAR        pOutBuffer = NULL;
	PUCHAR        pSupRate = NULL;
	UCHAR         SupRateLen;
	PUCHAR        pExtRate = NULL;
	UCHAR         ExtRateLen;	
	UCHAR         ASupRate[] = {0x8C, 0x12, 0x98, 0x24, 0xb0, 0x48, 0x60, 0x6C};
	UCHAR         ASupRateLen = sizeof(ASupRate)/sizeof(UCHAR);	
	MLME_JOIN_REQ_STRUCT *pInfo = (MLME_JOIN_REQ_STRUCT *)(Elem->Msg);

	DBGPRINT(RT_DEBUG_TRACE, ("SYNC - MlmeJoinReqAction(BSS #%ld)\n", pInfo->BssIdx));

#ifdef RT2860
    if ((OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_ADVANCE_POWER_SAVE_PCIE_DEVICE)) && 
        (IDLE_ON(pAd)) &&
		(pAd->StaCfg.bRadio == TRUE) &&
		(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF)))
	{
		PCIeAsicRadioOn(pAd, GUI_IDLE_POWER_SAVE);
	}
#endif // RT2860 //

	// reset all the timers
	RTMPCancelTimer(&pAd->MlmeAux.ScanTimer, &TimerCancelled);
	RTMPCancelTimer(&pAd->MlmeAux.BeaconTimer, &TimerCancelled);

	pBss = &pAd->MlmeAux.SsidBssTab.BssEntry[pInfo->BssIdx];

	// record the desired SSID & BSSID we're waiting for
	COPY_MAC_ADDR(pAd->MlmeAux.Bssid, pBss->Bssid);
	// If AP's SSID is not hidden, it is OK for updating ssid to MlmeAux again.
	if (pBss->Hidden == 0)
	{
		NdisMoveMemory(pAd->MlmeAux.Ssid, pBss->Ssid, pBss->SsidLen);
		pAd->MlmeAux.SsidLen = pBss->SsidLen;
	}
	pAd->MlmeAux.BssType = pBss->BssType;
	pAd->MlmeAux.Channel = pBss->Channel;
	pAd->MlmeAux.CentralChannel = pBss->CentralChannel;
	
	// switch channel and waiting for beacon timer
	AsicSwitchChannel(pAd, pAd->MlmeAux.Channel, FALSE);
	AsicLockChannel(pAd, pAd->MlmeAux.Channel);
	RTMPSetTimer(&pAd->MlmeAux.BeaconTimer, JOIN_TIMEOUT);

    do
	{
		if (((pAd->CommonCfg.bIEEE80211H == 1) && 
            (pAd->MlmeAux.Channel > 14) && 
             RadarChannelCheck(pAd, pAd->MlmeAux.Channel))
#ifdef CARRIER_DETECTION_SUPPORT // Roger sync Carrier
             || (pAd->CommonCfg.CarrierDetect.Enable == TRUE)
#endif
             )
		{
			//
			// We can't send any Probe request frame to meet 802.11h.
			//
			if (pBss->Hidden == 0)
				break;
		}
            
	//n
	// send probe request
	//
	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
	if (NStatus == NDIS_STATUS_SUCCESS)
	{
		if (pAd->MlmeAux.Channel <= 14)
		{
			pSupRate = pAd->CommonCfg.SupRate;
			SupRateLen = pAd->CommonCfg.SupRateLen;
			pExtRate = pAd->CommonCfg.ExtRate;
			ExtRateLen = pAd->CommonCfg.ExtRateLen;
		}
		else
		{
			//
			// Overwrite Support Rate, CCK rate are not allowed
			//
			pSupRate = ASupRate;
			SupRateLen = ASupRateLen;
			ExtRateLen = 0;
		}

		if (pAd->MlmeAux.BssType == BSS_INFRA)
			MgtMacHeaderInit(pAd, &Hdr80211, SUBTYPE_PROBE_REQ, 0, pAd->MlmeAux.Bssid, pAd->MlmeAux.Bssid);
		else
			MgtMacHeaderInit(pAd, &Hdr80211, SUBTYPE_PROBE_REQ, 0, BROADCAST_ADDR, BROADCAST_ADDR);

		MakeOutgoingFrame(pOutBuffer,               &FrameLen,
						  sizeof(HEADER_802_11),    &Hdr80211,
						  1,                        &SsidIe,
						  1,                        &pAd->MlmeAux.SsidLen,
						  pAd->MlmeAux.SsidLen,	    pAd->MlmeAux.Ssid,
						  1,                        &SupRateIe,
						  1,                        &SupRateLen,
						  SupRateLen,               pSupRate,
						  END_OF_ARGS);

		if (ExtRateLen)
		{
			ULONG Tmp;
			MakeOutgoingFrame(pOutBuffer + FrameLen,            &Tmp,
							  1,                                &ExtRateIe,
							  1,                                &ExtRateLen,
							  ExtRateLen,                       pExtRate, 
							  END_OF_ARGS);
			FrameLen += Tmp;
		}
		
#ifdef WSC_STA_SUPPORT
		// Append WSC information in probe request if WSC state is running
		if ((pAd->StaCfg.WscControl.WscEnProbeReqIE) && (pAd->StaCfg.WscControl.WscConfMode != WSC_DISABLE))
		{
			UCHAR		WscBuf[256], WscIeLen = 0;
			ULONG 		WscTmpLen = 0;

			NdisZeroMemory(WscBuf, 256);
			WscMakeProbeReqIE(pAd, &WscBuf[0], &WscIeLen);

			MakeOutgoingFrame(pOutBuffer + FrameLen,              &WscTmpLen,
							WscIeLen,                             &WscBuf,
							END_OF_ARGS);

			FrameLen += WscTmpLen;
		}
#endif	// WSC_STA_SUPPORT //

		MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
		MlmeFreeMemory(pAd, pOutBuffer);
		DBGPRINT(RT_DEBUG_INFO, ("SYNC - send ProbeReq @ channel=%d, Len=%ld\n", pAd->MlmeAux.Channel, FrameLen));
	}
    } while (FALSE);

	DBGPRINT(RT_DEBUG_TRACE, ("SYNC - Switch to ch %d, Wait BEACON from %02x:%02x:%02x:%02x:%02x:%02x\n", 
		pBss->Channel, pBss->Bssid[0], pBss->Bssid[1], pBss->Bssid[2], pBss->Bssid[3], pBss->Bssid[4], pBss->Bssid[5]));

	pAd->Mlme.SyncMachine.CurrState = JOIN_WAIT_BEACON;
}

/* 
	==========================================================================
	Description:
		MLME START Request state machine procedure, starting an IBSS
	==========================================================================
 */
VOID MlmeStartReqAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 
{
	UCHAR         Ssid[MAX_LEN_OF_SSID], SsidLen; 
	BOOLEAN       TimerCancelled;

	// New for WPA security suites
	UCHAR						VarIE[MAX_VIE_LEN]; 	// Total VIE length = MAX_VIE_LEN - -5
	NDIS_802_11_VARIABLE_IEs	*pVIE = NULL;
	LARGE_INTEGER				TimeStamp;
	BOOLEAN Privacy;
	USHORT Status;
#ifdef	SINGLE_ADHOC_LINKUP
	ULONG	Bssidx;
	CF_PARM CfParm;
	CfParm.bValid = FALSE;
#endif

	// Init Variable IE structure
	pVIE = (PNDIS_802_11_VARIABLE_IEs) VarIE;
	pVIE->Length = 0;
	TimeStamp.u.LowPart  = 0;
	TimeStamp.u.HighPart = 0;
	
	if (MlmeStartReqSanity(pAd, Elem->Msg, Elem->MsgLen, Ssid, &SsidLen)) 
	{
		// reset all the timers
		RTMPCancelTimer(&pAd->MlmeAux.ScanTimer, &TimerCancelled);
		RTMPCancelTimer(&pAd->MlmeAux.BeaconTimer, &TimerCancelled);

		//
		// Start a new IBSS. All IBSS parameters are decided now....
		//
		DBGPRINT(RT_DEBUG_TRACE, ("MlmeStartReqAction - Start a new IBSS. All IBSS parameters are decided now.... \n"));
		pAd->MlmeAux.BssType           = BSS_ADHOC;
		NdisMoveMemory(pAd->MlmeAux.Ssid, Ssid, SsidLen); 
		pAd->MlmeAux.SsidLen           = SsidLen;

		// generate a radom number as BSSID
		MacAddrRandomBssid(pAd, pAd->MlmeAux.Bssid);
		DBGPRINT(RT_DEBUG_TRACE, ("MlmeStartReqAction - generate a radom number as BSSID \n"));

		Privacy = (pAd->StaCfg.WepStatus == Ndis802_11Encryption1Enabled) || 
				  (pAd->StaCfg.WepStatus == Ndis802_11Encryption2Enabled) || 
				  (pAd->StaCfg.WepStatus == Ndis802_11Encryption3Enabled);
		pAd->MlmeAux.CapabilityInfo    = CAP_GENERATE(0,1,Privacy, (pAd->CommonCfg.TxPreamble == Rt802_11PreambleShort), 1, 0);
		pAd->MlmeAux.BeaconPeriod      = pAd->CommonCfg.BeaconPeriod;
		pAd->MlmeAux.AtimWin           = pAd->StaCfg.AtimWin;
		pAd->MlmeAux.Channel           = pAd->CommonCfg.Channel;

		// TODO: if consider 40MHz BW in Adhoc
		pAd->CommonCfg.CentralChannel  = pAd->CommonCfg.Channel;
		pAd->MlmeAux.CentralChannel    = pAd->CommonCfg.CentralChannel;

		pAd->MlmeAux.SupRateLen= pAd->CommonCfg.SupRateLen;
		NdisMoveMemory(pAd->MlmeAux.SupRate, pAd->CommonCfg.SupRate, MAX_LEN_OF_SUPPORTED_RATES);
		RTMPCheckRates(pAd, pAd->MlmeAux.SupRate, &pAd->MlmeAux.SupRateLen);
		pAd->MlmeAux.ExtRateLen = pAd->CommonCfg.ExtRateLen;
		NdisMoveMemory(pAd->MlmeAux.ExtRate, pAd->CommonCfg.ExtRate, MAX_LEN_OF_SUPPORTED_RATES);
		RTMPCheckRates(pAd, pAd->MlmeAux.ExtRate, &pAd->MlmeAux.ExtRateLen);
		if (pAd->CommonCfg.PhyMode >= PHY_11ABGN_MIXED)
		{
			RTMPUpdateHTIE(&pAd->CommonCfg.DesiredHtPhy, &pAd->StaCfg.DesiredHtPhyInfo.MCSSet[0], &pAd->MlmeAux.HtCapability, &pAd->MlmeAux.AddHtInfo);
			pAd->MlmeAux.HtCapabilityLen = sizeof(HT_CAPABILITY_IE);
			// Not turn pAd->StaActive.SupportedHtPhy.bHtEnable = TRUE here.
			DBGPRINT(RT_DEBUG_TRACE, ("SYNC -pAd->StaActive.SupportedHtPhy.bHtEnable = TRUE\n"));
		}
		else
		{
			pAd->MlmeAux.HtCapabilityLen = 0;
			pAd->StaActive.SupportedPhyInfo.bHtEnable = FALSE;
		}
		// temporarily not support QOS in IBSS
		NdisZeroMemory(&pAd->MlmeAux.APEdcaParm, sizeof(EDCA_PARM));
		NdisZeroMemory(&pAd->MlmeAux.APQbssLoad, sizeof(QBSS_LOAD_PARM));
		NdisZeroMemory(&pAd->MlmeAux.APQosCapability, sizeof(QOS_CAPABILITY_PARM));

		AsicSwitchChannel(pAd, pAd->MlmeAux.Channel, FALSE);
		AsicLockChannel(pAd, pAd->MlmeAux.Channel);

		DBGPRINT(RT_DEBUG_TRACE, ("SYNC - MlmeStartReqAction(ch= %d,sup rates= %d, ext rates=%d)\n",
			pAd->MlmeAux.Channel, pAd->MlmeAux.SupRateLen, pAd->MlmeAux.ExtRateLen));

#ifdef	SINGLE_ADHOC_LINKUP
		// Add itself as the entry within BSS table
		Bssidx = BssTableSearch(&pAd->ScanTab, pAd->MlmeAux.Bssid, pAd->MlmeAux.Channel);
		if (Bssidx == BSS_NOT_FOUND)
		{
			Bssidx = BssTableSetEntry(pAd, &pAd->ScanTab, pAd->MlmeAux.Bssid,Ssid, SsidLen, pAd->MlmeAux.BssType, 
				pAd->MlmeAux.BeaconPeriod, &CfParm, pAd->MlmeAux.AtimWin, pAd->MlmeAux.CapabilityInfo, 
				&pAd->MlmeAux.HtCapability, pAd->MlmeAux.HtCapabilityLen, &pAd->CommonCfg.AddHTInfo, SIZE_ADD_HT_INFO_IE, pAd->MlmeAux.NewExtChannelOffset,
				pAd->MlmeAux.SupRate, pAd->MlmeAux.SupRateLen, pAd->MlmeAux.ExtRate, pAd->MlmeAux.ExtRateLen,
				&pAd->MlmeAux.SupportedHtPhy, pAd->MlmeAux.Channel, pAd->BbpRssiToDbmDelta - 30, TimeStamp, 0, NULL, NULL, 0, pVIE);
		}
#endif

		pAd->Mlme.SyncMachine.CurrState = SYNC_IDLE;
		Status = MLME_SUCCESS;
		MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_START_CONF, 2, &Status);
	} 
	else 
	{
		DBGPRINT_ERR(("SYNC - MlmeStartReqAction() sanity check fail.\n"));
		pAd->Mlme.SyncMachine.CurrState = SYNC_IDLE;
		Status = MLME_INVALID_FORMAT;
		MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_START_CONF, 2, &Status);
	}
}

/* 
	==========================================================================
	Description:
		peer sends beacon back when scanning
	==========================================================================
 */
VOID PeerBeaconAtScanAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 
{
	UCHAR           Bssid[MAC_ADDR_LEN], Addr2[MAC_ADDR_LEN];
	UCHAR           Ssid[MAX_LEN_OF_SSID], BssType, Channel, NewChannel,
					SsidLen, DtimCount, DtimPeriod, BcastFlag, MessageToMe;
	CF_PARM         CfParm;
	USHORT          BeaconPeriod, AtimWin, CapabilityInfo;
	PFRAME_802_11   pFrame;
	LARGE_INTEGER   TimeStamp;
	UCHAR           Erp;
	UCHAR         	SupRate[MAX_LEN_OF_SUPPORTED_RATES], ExtRate[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR		  	SupRateLen, ExtRateLen;
	USHORT 			LenVIE;
	UCHAR			CkipFlag;
	UCHAR			AironetCellPowerLimit;
	EDCA_PARM       EdcaParm;
	QBSS_LOAD_PARM  QbssLoad;
	QOS_CAPABILITY_PARM QosCapability;
	ULONG						RalinkIe;
	UCHAR						VarIE[MAX_VIE_LEN];		// Total VIE length = MAX_VIE_LEN - -5
	NDIS_802_11_VARIABLE_IEs	*pVIE = NULL;
	HT_CAPABILITY_IE		HtCapability;
	ADD_HT_INFO_IE		AddHtInfo;	// AP might use this additional ht info IE 
	UCHAR			HtCapabilityLen, PreNHtCapabilityLen;
	UCHAR			AddHtInfoLen;
	UCHAR			NewExtChannelOffset = 0xff;

#ifdef MESH_SUPPORT
	UCHAR HostName[MAX_HOST_NAME_LEN] = {0};
	UCHAR HostNameLen = 0;
	UCHAR MeshId[MAX_MESH_ID_LEN] = {0};
	UCHAR MeshIdLen = 0;
	MESH_CONFIGURAION_IE MeshConfig = {0};
#endif // MESH_SUPPORT //

	// NdisFillMemory(Ssid, MAX_LEN_OF_SSID, 0x00);
	pFrame = (PFRAME_802_11) Elem->Msg;
	// Init Variable IE structure
	pVIE = (PNDIS_802_11_VARIABLE_IEs) VarIE;
	pVIE->Length = 0;
    RTMPZeroMemory(&HtCapability, sizeof(HtCapability));
	RTMPZeroMemory(&AddHtInfo, sizeof(ADD_HT_INFO_IE));
    
	if (PeerBeaconAndProbeRspSanity(pAd, 
								Elem->Msg, 
								Elem->MsgLen, 
								Addr2, 
								Bssid, 
								Ssid, 
								&SsidLen, 
								&BssType, 
								&BeaconPeriod, 
								&Channel, 
								&NewChannel,
								&TimeStamp, 
								&CfParm, 
								&AtimWin, 
								&CapabilityInfo, 
								&Erp,
								&DtimCount, 
								&DtimPeriod, 
								&BcastFlag, 
								&MessageToMe, 
								SupRate,
								&SupRateLen,
								ExtRate,
								&ExtRateLen,
								&CkipFlag,
								&AironetCellPowerLimit,
								&EdcaParm,
								&QbssLoad,
								&QosCapability,
								&RalinkIe,
								&HtCapabilityLen,
								&PreNHtCapabilityLen,
								&HtCapability,
								&AddHtInfoLen,
								&AddHtInfo,
								&NewExtChannelOffset,
								&LenVIE,
								pVIE)) 
	{
		ULONG Idx;
		CHAR Rssi = 0;
//        BOOLEAN	bUpdateRssi = TRUE;

		Idx = BssTableSearch(&pAd->ScanTab, Bssid, Channel);
		if (Idx != BSS_NOT_FOUND) 
			Rssi = pAd->ScanTab.BssEntry[Idx].Rssi;

		Rssi = RTMPMaxRssi(pAd, ConvertToRssi(pAd, Elem->Rssi0, RSSI_0), ConvertToRssi(pAd, Elem->Rssi1, RSSI_1), ConvertToRssi(pAd, Elem->Rssi2, RSSI_2));

#ifdef MESH_SUPPORT
		MeshPeerBeaconAndProbeSanity(pAd, Elem->Msg, Elem->MsgLen, HostName, &HostNameLen, MeshId, &MeshIdLen, &MeshConfig);
		if(MESH_ON(pAd) && (MeshIdLen != 0))
		{
			//PMAC_TABLE_ENTRY pEntry = MacTableLookup(pAd, Addr2);
			ULONG LinkIdx;
			UCHAR MeshEncrypType = ENCRYPT_OPEN_NONE;

			MeshEncrypType = MeshCheckPeerMpCipher(CapabilityInfo, (PUCHAR)pVIE, LenVIE);

			InsertNeighborMP(pAd, Rssi, Bssid, Channel, (UINT8)HtCapability.HtCapInfo.ChannelWidth,
				(UINT8)AddHtInfo.AddHtInfo.ExtChanOffset, HostName, MeshId, MeshIdLen, MeshEncrypType, &MeshConfig);

			LinkIdx = GetMeshLinkId(pAd, Addr2);
			if(VALID_MESH_LINK_ID(LinkIdx))
			{
				ULONG Now;
				NdisGetSystemUpTime(&Now);
				pAd->MeshTab.MeshLink[LinkIdx].Entry.LastBeaconTime = Now;
			}
		}
#endif // MESH_SUPPORT //

//		DBGPRINT(RT_DEBUG_TRACE, ("SYNC - PeerBeaconAtScanAction (SubType=%d, SsidLen=%d, Ssid=%s)\n", pFrame->Hdr.FC.SubType, SsidLen,Ssid));
		if ((HtCapabilityLen > 0) || (PreNHtCapabilityLen > 0))
			HtCapabilityLen = SIZE_HT_CAP_IE;

		if ((pAd->StaCfg.CCXReqType != MSRN_TYPE_UNUSED) && (Channel == pAd->StaCfg.CCXScanChannel))
		{
			Idx = BssTableSetEntry(pAd, &pAd->StaCfg.CCXBssTab, Bssid, Ssid, SsidLen, BssType, BeaconPeriod, 
						 &CfParm, AtimWin, CapabilityInfo, SupRate, SupRateLen,ExtRate, ExtRateLen, &HtCapability,
						 &AddHtInfo, HtCapabilityLen, AddHtInfoLen, NewExtChannelOffset, Channel, Rssi, TimeStamp, CkipFlag, 
						 &EdcaParm, &QosCapability, &QbssLoad, LenVIE, pVIE);
			if (Idx != BSS_NOT_FOUND)
			{
				NdisMoveMemory(pAd->StaCfg.CCXBssTab.BssEntry[Idx].PTSF, &Elem->Msg[24], 4);
				NdisMoveMemory(&pAd->StaCfg.CCXBssTab.BssEntry[Idx].TTSF[0], &Elem->TimeStamp.u.LowPart, 4);
				NdisMoveMemory(&pAd->StaCfg.CCXBssTab.BssEntry[Idx].TTSF[4], &Elem->TimeStamp.u.LowPart, 4);
				if (pAd->StaCfg.CCXReqType == MSRN_TYPE_BEACON_REQ)
					AironetAddBeaconReport(pAd, Idx, Elem);
			}
		}
		else
		{
			Idx = BssTableSetEntry(pAd, &pAd->ScanTab, Bssid, Ssid, SsidLen, BssType, BeaconPeriod,
						  &CfParm, AtimWin, CapabilityInfo, SupRate, SupRateLen, ExtRate, ExtRateLen,  &HtCapability,
						 &AddHtInfo, HtCapabilityLen, AddHtInfoLen, NewExtChannelOffset, Channel, Rssi, TimeStamp, CkipFlag, 
						 &EdcaParm, &QosCapability, &QbssLoad, LenVIE, pVIE);
			if (Idx != BSS_NOT_FOUND)
			{
				NdisMoveMemory(pAd->ScanTab.BssEntry[Idx].PTSF, &Elem->Msg[24], 4);
				NdisMoveMemory(&pAd->ScanTab.BssEntry[Idx].TTSF[0], &Elem->TimeStamp.u.LowPart, 4);
				NdisMoveMemory(&pAd->ScanTab.BssEntry[Idx].TTSF[4], &Elem->TimeStamp.u.LowPart, 4);
			}
		}
	}
	// sanity check fail, ignored
}

/* 
	==========================================================================
	Description:
		When waiting joining the (I)BSS, beacon received from external
	==========================================================================
 */
VOID PeerBeaconAtJoinAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 
{
	UCHAR         Bssid[MAC_ADDR_LEN], Addr2[MAC_ADDR_LEN];
	UCHAR         Ssid[MAX_LEN_OF_SSID], SsidLen, BssType, Channel, MessageToMe, 
				  DtimCount, DtimPeriod, BcastFlag, NewChannel; 
	LARGE_INTEGER TimeStamp;
	USHORT        BeaconPeriod, AtimWin, CapabilityInfo;
//	UINT          FrameLen = 0;
	CF_PARM       Cf;
	BOOLEAN       TimerCancelled;
	UCHAR         Erp;
	UCHAR         SupRate[MAX_LEN_OF_SUPPORTED_RATES], ExtRate[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR		  SupRateLen, ExtRateLen;
	UCHAR         CkipFlag;
	USHORT 		  LenVIE;
	UCHAR		  AironetCellPowerLimit;
	EDCA_PARM       EdcaParm;
	QBSS_LOAD_PARM  QbssLoad;
	QOS_CAPABILITY_PARM QosCapability;
	USHORT        Status;
	UCHAR						VarIE[MAX_VIE_LEN];		// Total VIE length = MAX_VIE_LEN - -5
	NDIS_802_11_VARIABLE_IEs	*pVIE = NULL;
	ULONG           RalinkIe;
	ULONG         Idx;
	HT_CAPABILITY_IE		HtCapability;
	ADD_HT_INFO_IE		AddHtInfo;	// AP might use this additional ht info IE 
	UCHAR			HtCapabilityLen, PreNHtCapabilityLen;
	UCHAR			AddHtInfoLen;
	UCHAR			NewExtChannelOffset = 0xff, CentralChannel;

	// Init Variable IE structure
	pVIE = (PNDIS_802_11_VARIABLE_IEs) VarIE;
	pVIE->Length = 0;
    RTMPZeroMemory(&HtCapability, sizeof(HtCapability));
	RTMPZeroMemory(&AddHtInfo, sizeof(ADD_HT_INFO_IE));

	if (PeerBeaconAndProbeRspSanity(pAd, 
								Elem->Msg, 
								Elem->MsgLen, 
								Addr2, 
								Bssid, 
								Ssid, 
								&SsidLen, 
								&BssType, 
								&BeaconPeriod, 
								&Channel, 
								&NewChannel,
								&TimeStamp, 
								&Cf, 
								&AtimWin, 
								&CapabilityInfo, 
								&Erp,
								&DtimCount, 
								&DtimPeriod, 
								&BcastFlag, 
								&MessageToMe, 
								SupRate,
								&SupRateLen,
								ExtRate,
								&ExtRateLen,
								&CkipFlag,
								&AironetCellPowerLimit,
								&EdcaParm,
								&QbssLoad,
								&QosCapability,
								&RalinkIe,
								&HtCapabilityLen,
								&PreNHtCapabilityLen,
								&HtCapability,
								&AddHtInfoLen,
								&AddHtInfo,
								&NewExtChannelOffset,
								&LenVIE,
								pVIE)) 
	{
		// Disqualify 11b only adhoc when we are in 11g only adhoc mode
		if ((BssType == BSS_ADHOC) && (pAd->StaCfg.AdhocMode == 2) && ((SupRateLen+ExtRateLen)< 12))
			return;

		// BEACON from desired BSS/IBSS found. We should be able to decide most
		// BSS parameters here.
		// Q. But what happen if this JOIN doesn't conclude a successful ASSOCIATEION?
		//    Do we need to receover back all parameters belonging to previous BSS?
		// A. Should be not. There's no back-door recover to previous AP. It still need
		//    a new JOIN-AUTH-ASSOC sequence.
		if (MAC_ADDR_EQUAL(pAd->MlmeAux.Bssid, Bssid)) 
		{
			DBGPRINT(RT_DEBUG_TRACE, ("SYNC - receive desired BEACON at JoinWaitBeacon... Channel = %d\n", Channel));
			RTMPCancelTimer(&pAd->MlmeAux.BeaconTimer, &TimerCancelled);

			// Update RSSI to prevent No signal display when cards first initialized
			pAd->StaCfg.RssiSample.LastRssi0	= ConvertToRssi(pAd, Elem->Rssi0, RSSI_0);
			pAd->StaCfg.RssiSample.LastRssi1	= ConvertToRssi(pAd, Elem->Rssi1, RSSI_1);
			pAd->StaCfg.RssiSample.LastRssi2	= ConvertToRssi(pAd, Elem->Rssi2, RSSI_2);
			pAd->StaCfg.RssiSample.AvgRssi0	= pAd->StaCfg.RssiSample.LastRssi0;
			pAd->StaCfg.RssiSample.AvgRssi0X8	= pAd->StaCfg.RssiSample.AvgRssi0 << 3;
			pAd->StaCfg.RssiSample.AvgRssi1	= pAd->StaCfg.RssiSample.LastRssi1;
			pAd->StaCfg.RssiSample.AvgRssi1X8	= pAd->StaCfg.RssiSample.AvgRssi1 << 3;
			pAd->StaCfg.RssiSample.AvgRssi2	= pAd->StaCfg.RssiSample.LastRssi2;
			pAd->StaCfg.RssiSample.AvgRssi2X8	= pAd->StaCfg.RssiSample.AvgRssi2 << 3;

			//
			// We need to check if SSID only set to any, then we can record the current SSID.
			// Otherwise will cause hidden SSID association failed. 
			//
			if (pAd->MlmeAux.SsidLen == 0)
			{
				NdisMoveMemory(pAd->MlmeAux.Ssid, Ssid, SsidLen);
				pAd->MlmeAux.SsidLen = SsidLen;
			}
			else
			{
				Idx = BssSsidTableSearch(&pAd->ScanTab, Bssid, pAd->MlmeAux.Ssid, pAd->MlmeAux.SsidLen, Channel);

				if (Idx != BSS_NOT_FOUND)
				{
					//
					// Multiple SSID case, used correct CapabilityInfo
					//
					CapabilityInfo = pAd->ScanTab.BssEntry[Idx].CapabilityInfo;
				}
			}
			NdisMoveMemory(pAd->MlmeAux.Bssid, Bssid, MAC_ADDR_LEN);
			pAd->MlmeAux.CapabilityInfo = CapabilityInfo & SUPPORTED_CAPABILITY_INFO;
			pAd->MlmeAux.BssType = BssType;
			pAd->MlmeAux.BeaconPeriod = BeaconPeriod;
			pAd->MlmeAux.Channel = Channel;
			pAd->MlmeAux.AtimWin = AtimWin;
			pAd->MlmeAux.CfpPeriod = Cf.CfpPeriod;
			pAd->MlmeAux.CfpMaxDuration = Cf.CfpMaxDuration;
			pAd->MlmeAux.APRalinkIe = RalinkIe;

			// Copy AP's supported rate to MlmeAux for creating assoication request
			// Also filter out not supported rate
			pAd->MlmeAux.SupRateLen = SupRateLen;
			NdisMoveMemory(pAd->MlmeAux.SupRate, SupRate, SupRateLen);
			RTMPCheckRates(pAd, pAd->MlmeAux.SupRate, &pAd->MlmeAux.SupRateLen);
			pAd->MlmeAux.ExtRateLen = ExtRateLen;
			NdisMoveMemory(pAd->MlmeAux.ExtRate, ExtRate, ExtRateLen);
			RTMPCheckRates(pAd, pAd->MlmeAux.ExtRate, &pAd->MlmeAux.ExtRateLen);
			pAd->MlmeAux.NewExtChannelOffset = NewExtChannelOffset;
			pAd->MlmeAux.HtCapabilityLen = HtCapabilityLen;
            NdisZeroMemory(pAd->StaActive.SupportedPhyInfo.MCSSet, 16);
			// filter out un-supported ht rates
			if (((HtCapabilityLen > 0) || (PreNHtCapabilityLen > 0)) && (pAd->CommonCfg.PhyMode >= PHY_11ABGN_MIXED))
			{
				RTMPZeroMemory(&pAd->MlmeAux.HtCapability, SIZE_HT_CAP_IE);
   				RTMPMoveMemory(&pAd->MlmeAux.AddHtInfo, &AddHtInfo, SIZE_ADD_HT_INFO_IE);
                // StaActive.SupportedHtPhy.MCSSet stores Peer AP's 11n Rx capability
				NdisMoveMemory(pAd->StaActive.SupportedPhyInfo.MCSSet, HtCapability.MCSSet, 16);
				pAd->MlmeAux.NewExtChannelOffset = NewExtChannelOffset;
				pAd->MlmeAux.HtCapabilityLen = SIZE_HT_CAP_IE;
				pAd->StaActive.SupportedPhyInfo.bHtEnable = TRUE;
				if (PreNHtCapabilityLen > 0)
					pAd->StaActive.SupportedPhyInfo.bPreNHt = TRUE;
				RTMPCheckHt(pAd, BSSID_WCID, &HtCapability, &AddHtInfo);
				// Copy AP Parameter to StaActive.  This is also in LinkUp.
				DBGPRINT(RT_DEBUG_TRACE, ("PeerBeaconAtJoinAction! (MpduDensity=%d, MaxRAmpduFactor=%d, BW=%d)\n", 
					pAd->StaActive.SupportedHtPhy.MpduDensity, pAd->StaActive.SupportedHtPhy.MaxRAmpduFactor, HtCapability.HtCapInfo.ChannelWidth));
				
				if (AddHtInfoLen > 0)
				{
					CentralChannel = AddHtInfo.ControlChan;
		 			// Check again the Bandwidth capability of this AP.
		 			if ((AddHtInfo.ControlChan > 2)&& (AddHtInfo.AddHtInfo.ExtChanOffset == EXTCHA_BELOW) && (HtCapability.HtCapInfo.ChannelWidth == BW_40))
		 			{
		 				CentralChannel = AddHtInfo.ControlChan - 2;
		 			}
		 			else if ((AddHtInfo.AddHtInfo.ExtChanOffset == EXTCHA_ABOVE) && (HtCapability.HtCapInfo.ChannelWidth == BW_40))
		 			{
		 				CentralChannel = AddHtInfo.ControlChan + 2;
		 			}
                    
                    // Check Error .
					if (pAd->MlmeAux.CentralChannel != CentralChannel)
		 				DBGPRINT(RT_DEBUG_ERROR, ("PeerBeaconAtJoinAction HT===>Beacon Central Channel = %d, Control Channel = %d. Mlmeaux CentralChannel = %d\n", CentralChannel, AddHtInfo.ControlChan, pAd->MlmeAux.CentralChannel));

		 			DBGPRINT(RT_DEBUG_TRACE, ("PeerBeaconAtJoinAction HT===>Central Channel = %d, Control Channel = %d,  .\n", CentralChannel, AddHtInfo.ControlChan));

				}
				
			}
			else
			{
   				// To prevent error, let legacy AP must have same CentralChannel and Channel.
				if ((HtCapabilityLen == 0) && (PreNHtCapabilityLen == 0))
					pAd->MlmeAux.CentralChannel = pAd->MlmeAux.Channel;

				pAd->StaActive.SupportedPhyInfo.bHtEnable = FALSE;
				RTMPZeroMemory(&pAd->MlmeAux.HtCapability, SIZE_HT_CAP_IE);
				RTMPZeroMemory(&pAd->MlmeAux.AddHtInfo, SIZE_ADD_HT_INFO_IE);
			}

			/*if (AddHtInfoLen > 0)
			{
				RTMP_IO_READ32(pAd, TX_BAND_CFG, 	&Value);
				if (AddHtInfo.AddHtInfo.ExtChanOffset == EXTCHA_BELOW )
				{
					Value &= (0xfffffffe);
					RTMP_IO_WRITE32(pAd, TX_BAND_CFG, 	Value);
					pAd->StaActive.SupportedHtPhy.ExtChanOffset = ex
				}
				else if (AddHtInfo.AddHtInfo.ExtChanOffset == EXTCHA_ABOVE)
				{
					Value |= (0x1);
					RTMP_IO_WRITE32(pAd, TX_BAND_CFG, 	Value);
				}
			}*/
			RTMPUpdateMlmeRate(pAd);
	
			// copy QOS related information
			if ((pAd->CommonCfg.PhyMode >= PHY_11ABGN_MIXED)|| (pAd->CommonCfg.bWmmCapable))
			{
				NdisMoveMemory(&pAd->MlmeAux.APEdcaParm, &EdcaParm, sizeof(EDCA_PARM));
				NdisMoveMemory(&pAd->MlmeAux.APQbssLoad, &QbssLoad, sizeof(QBSS_LOAD_PARM));
				NdisMoveMemory(&pAd->MlmeAux.APQosCapability, &QosCapability, sizeof(QOS_CAPABILITY_PARM));
			}
			else
			{
				NdisZeroMemory(&pAd->MlmeAux.APEdcaParm, sizeof(EDCA_PARM));
				NdisZeroMemory(&pAd->MlmeAux.APQbssLoad, sizeof(QBSS_LOAD_PARM));
				NdisZeroMemory(&pAd->MlmeAux.APQosCapability, sizeof(QOS_CAPABILITY_PARM));
			}
			
			DBGPRINT(RT_DEBUG_TRACE, ("SYNC - after JOIN, SupRateLen=%d, ExtRateLen=%d\n", 
				pAd->MlmeAux.SupRateLen, pAd->MlmeAux.ExtRateLen));

#ifdef LEAP_SUPPORT
			// Update CkipFlag
			pAd->StaCfg.CkipFlag = CkipFlag;
			
			// Keep TimeStamp for Re-Association used.
			if (LEAP_CCKM_ON(pAd) && (pAd->StaCfg.CCKMLinkUpFlag == TRUE))
				pAd->StaCfg.CCKMBeaconAtJoinTimeStamp = TimeStamp;
#endif // LEAP_SUPPORT //
				
			if (AironetCellPowerLimit != 0xFF)
			{
				//We need to change our TxPower for CCX 2.0 AP Control of Client Transmit Power
				ChangeToCellPowerLimit(pAd, AironetCellPowerLimit);
			}
			else  //Used the default TX Power Percentage.
				pAd->CommonCfg.TxPowerPercentage = pAd->CommonCfg.TxPowerDefault;

			pAd->Mlme.SyncMachine.CurrState = SYNC_IDLE;
			Status = MLME_SUCCESS;
			MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_JOIN_CONF, 2, &Status);
		}
		// not to me BEACON, ignored
	} 
	// sanity check fail, ignore this frame
}

/* 
	==========================================================================
	Description:
		receive BEACON from peer

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
VOID PeerBeacon(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 
{
	UCHAR         Bssid[MAC_ADDR_LEN], Addr2[MAC_ADDR_LEN];
	CHAR          Ssid[MAX_LEN_OF_SSID];
	CF_PARM       CfParm;
	UCHAR         SsidLen, MessageToMe=0, BssType, Channel, NewChannel, index=0;
	UCHAR         DtimCount=0, DtimPeriod=0, BcastFlag=0;
	USHORT        CapabilityInfo, AtimWin, BeaconPeriod;
	LARGE_INTEGER TimeStamp;
	USHORT        TbttNumToNextWakeUp;
	UCHAR         Erp;
	UCHAR         SupRate[MAX_LEN_OF_SUPPORTED_RATES], ExtRate[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR		  SupRateLen, ExtRateLen;
	UCHAR		  CkipFlag;
	USHORT        LenVIE;
	UCHAR		  AironetCellPowerLimit;
	EDCA_PARM       EdcaParm;
	QBSS_LOAD_PARM  QbssLoad;
	QOS_CAPABILITY_PARM QosCapability;
	ULONG           RalinkIe;
	// New for WPA security suites
	UCHAR						VarIE[MAX_VIE_LEN];		// Total VIE length = MAX_VIE_LEN - -5
	NDIS_802_11_VARIABLE_IEs	*pVIE = NULL;
	HT_CAPABILITY_IE		HtCapability;
	ADD_HT_INFO_IE		AddHtInfo;	// AP might use this additional ht info IE 
	UCHAR			HtCapabilityLen, PreNHtCapabilityLen;
	UCHAR			AddHtInfoLen;
	UCHAR			NewExtChannelOffset = 0xff;

#ifdef MESH_SUPPORT
	UCHAR HostName[MAX_HOST_NAME_LEN] = {0};
	UCHAR HostNameLen = 0;
	UCHAR MeshId[MAX_MESH_ID_LEN] = {0};
	UCHAR MeshIdLen = 0;
	MESH_CONFIGURAION_IE MeshConfig = {0};
#endif // MESH_SUPPORT //

#ifdef RALINK_ATE
    if(pAd->ate.Mode != ATE_STOP)
    {
		return;
    }
#endif // RALINK_ATE //

	if (!(INFRA_ON(pAd) || ADHOC_ON(pAd)
#ifdef MESH_SUPPORT
			|| MESH_ON(pAd)
#endif // MESH_SUPPORT //
		))
		return;

	// Init Variable IE structure
	pVIE = (PNDIS_802_11_VARIABLE_IEs) VarIE;
	pVIE->Length = 0;
    RTMPZeroMemory(&HtCapability, sizeof(HtCapability));
	RTMPZeroMemory(&AddHtInfo, sizeof(ADD_HT_INFO_IE));

	if (PeerBeaconAndProbeRspSanity(pAd, 
								Elem->Msg, 
								Elem->MsgLen, 
								Addr2, 
								Bssid, 
								Ssid, 
								&SsidLen, 
								&BssType, 
								&BeaconPeriod, 
								&Channel, 
								&NewChannel,
								&TimeStamp, 
								&CfParm, 
								&AtimWin, 
								&CapabilityInfo, 
								&Erp,
								&DtimCount, 
								&DtimPeriod, 
								&BcastFlag, 
								&MessageToMe, 
								SupRate,
								&SupRateLen,
								ExtRate,
								&ExtRateLen,
								&CkipFlag,
								&AironetCellPowerLimit,
								&EdcaParm,
								&QbssLoad,
								&QosCapability,
								&RalinkIe,
								&HtCapabilityLen,
								&PreNHtCapabilityLen,
								&HtCapability,
								&AddHtInfoLen,
								&AddHtInfo,
								&NewExtChannelOffset,
								&LenVIE,
								pVIE)) 
	{
		BOOLEAN is_my_bssid, is_my_ssid;
		ULONG   Bssidx, Now;
		BSS_ENTRY *pBss;
		CHAR		RealRssi = RTMPMaxRssi(pAd, ConvertToRssi(pAd, Elem->Rssi0, RSSI_0), ConvertToRssi(pAd, Elem->Rssi1, RSSI_1), ConvertToRssi(pAd, Elem->Rssi2, RSSI_2));

		is_my_bssid = MAC_ADDR_EQUAL(Bssid, pAd->CommonCfg.Bssid)? TRUE : FALSE;
		is_my_ssid = SSID_EQUAL(Ssid, SsidLen, pAd->CommonCfg.Ssid, pAd->CommonCfg.SsidLen)? TRUE:FALSE;

#ifdef MESH_SUPPORT
		MeshPeerBeaconAndProbeSanity(pAd, Elem->Msg, Elem->MsgLen, HostName, &HostNameLen, MeshId, &MeshIdLen, &MeshConfig);
		if(MESH_ON(pAd) && (MeshIdLen != 0))
		{
			//PMAC_TABLE_ENTRY pEntry = MacTableLookup(pAd, Addr2);
			ULONG LinkIdx;
			UCHAR MeshEncrypType = ENCRYPT_OPEN_NONE;

			MeshEncrypType = MeshCheckPeerMpCipher(CapabilityInfo, (PUCHAR)pVIE, LenVIE);

			InsertNeighborMP(pAd, RealRssi, Bssid, Channel, (UINT8)HtCapability.HtCapInfo.ChannelWidth,
				(UINT8)AddHtInfo.AddHtInfo.ExtChanOffset, HostName, MeshId, MeshIdLen, MeshEncrypType, &MeshConfig);

			LinkIdx = GetMeshLinkId(pAd, Addr2);
			if(VALID_MESH_LINK_ID(LinkIdx))
			{
				ULONG Now;
				NdisGetSystemUpTime(&Now);
				pAd->MeshTab.MeshLink[LinkIdx].Entry.LastBeaconTime = Now;
			}
		}
#endif // MESH_SUPPORT //

		// ignore BEACON not for my SSID
		if ((! is_my_ssid) && (! is_my_bssid))
			return;
		// Copy Control channel for this BSSID.		
		if (AddHtInfoLen != 0)
			Channel = AddHtInfo.ControlChan;

		if ((HtCapabilityLen > 0) || (PreNHtCapabilityLen > 0))
			HtCapabilityLen = SIZE_HT_CAP_IE;

		//
		// Housekeeping "SsidBssTab" table for later-on ROAMing usage. 
		//
		Bssidx = BssTableSearch(&pAd->MlmeAux.SsidBssTab, Bssid, Channel);
		if (Bssidx == BSS_NOT_FOUND)
		{
			// discover new AP of this network, create BSS entry
			Bssidx = BssTableSetEntry(pAd, &pAd->MlmeAux.SsidBssTab, Bssid, Ssid, SsidLen, BssType, BeaconPeriod,
						 &CfParm, AtimWin, CapabilityInfo, SupRate, SupRateLen, ExtRate, ExtRateLen, 
						&HtCapability, &AddHtInfo,HtCapabilityLen,AddHtInfoLen,NewExtChannelOffset, Channel, 
						RealRssi, TimeStamp, CkipFlag, &EdcaParm, &QosCapability, 
						&QbssLoad, LenVIE, pVIE);
			if (Bssidx == BSS_NOT_FOUND) // return if BSS table full
				return;  
			
			NdisMoveMemory(pAd->MlmeAux.SsidBssTab.BssEntry[Bssidx].PTSF, &Elem->Msg[24], 4);
			NdisMoveMemory(&pAd->MlmeAux.SsidBssTab.BssEntry[Bssidx].TTSF[0], &Elem->TimeStamp.u.LowPart, 4);
			NdisMoveMemory(&pAd->MlmeAux.SsidBssTab.BssEntry[Bssidx].TTSF[4], &Elem->TimeStamp.u.LowPart, 4);
			
			DBGPRINT(RT_DEBUG_INFO, ("SYNC - New AP added to SsidBssTab[%ld], RSSI=%d, MAC=%02x:%02x:%02x:%02x:%02x:%02x\n", 
				Bssidx, RealRssi, Bssid[0], Bssid[1], Bssid[2], Bssid[3], Bssid[4], Bssid[5]));
		}

		if ((pAd->CommonCfg.bIEEE80211H == 1) && (NewChannel != 0) && (Channel != NewChannel))
		{
		    LinkDown(pAd, FALSE);
		    RTMPusecDelay(1000000);		// use delay to prevent STA do reassoc
					
			// channel sanity check
			for (index = 0 ; index < pAd->ChannelListNum; index++)
			{
				if (pAd->ChannelList[index].Channel == NewChannel)
				{
					pAd->MlmeAux.SsidBssTab.BssEntry[Bssidx].Channel = NewChannel;
					pAd->CommonCfg.Channel = NewChannel;
					AsicSwitchChannel(pAd, pAd->CommonCfg.Channel, FALSE);
					AsicLockChannel(pAd, pAd->CommonCfg.Channel);
					DBGPRINT(RT_DEBUG_TRACE, ("PeerBeacon - STA receive channel switch announcement IE (New Channel =%d)\n", NewChannel));
					break;
				}
			}

			if (index >= pAd->ChannelListNum)
			{
				DBGPRINT_ERR(("PeerBeacon(can not find New Channel=%d in ChannelList[%d]\n", pAd->CommonCfg.Channel, pAd->ChannelListNum));
			}
		}

		// if the ssid matched & bssid unmatched, we should select the bssid with large value.
		// This might happened when two STA start at the same time
		if ((! is_my_bssid) && ADHOC_ON(pAd))
		{
			INT	i;

			// Add the safeguard against the mismatch of adhoc wep status
			if (pAd->StaCfg.WepStatus != pAd->MlmeAux.SsidBssTab.BssEntry[Bssidx].WepStatus)
			{
				DBGPRINT(RT_DEBUG_TRACE, ("SYNC - Not matched wep status %d %d\n", pAd->StaCfg.WepStatus, pAd->MlmeAux.SsidBssTab.BssEntry[Bssidx].WepStatus));
				DBGPRINT(RT_DEBUG_TRACE, ("bssid=%s\n", pAd->MlmeAux.SsidBssTab.BssEntry[Bssidx].Bssid));
				return;
			}
			
			// collapse into the ADHOC network which has bigger BSSID value.
			for (i = 0; i < 6; i++)
			{
				if (Bssid[i] > pAd->CommonCfg.Bssid[i])
				{
					DBGPRINT(RT_DEBUG_TRACE, ("SYNC - merge to the IBSS with bigger BSSID=%02x:%02x:%02x:%02x:%02x:%02x\n", 
						Bssid[0], Bssid[1], Bssid[2], Bssid[3], Bssid[4], Bssid[5]));
					AsicDisableSync(pAd);
					COPY_MAC_ADDR(pAd->CommonCfg.Bssid, Bssid);
					AsicSetBssid(pAd, pAd->CommonCfg.Bssid); 
					MakeIbssBeacon(pAd);        // re-build BEACON frame
					AsicEnableIbssSync(pAd);    // copy BEACON frame to on-chip memory
					is_my_bssid = TRUE;
					break;
				}
				else if (Bssid[i] < pAd->CommonCfg.Bssid[i])
					break;
			}
		}

		DBGPRINT(RT_DEBUG_INFO, ("SYNC - PeerBeacon from %02x:%02x:%02x:%02x:%02x:%02x - Dtim=%d/%d, Rssi=%02x\n", 
			Bssid[0], Bssid[1], Bssid[2], Bssid[3], Bssid[4], Bssid[5], 
			DtimCount, DtimPeriod, RealRssi));

		NdisGetSystemUpTime(&Now);
		pBss = &pAd->MlmeAux.SsidBssTab.BssEntry[Bssidx];
		pBss->Rssi = RealRssi;       // lastest RSSI
		pBss->LastBeaconRxTime = Now;   // last RX timestamp

		//
		// BEACON from my BSSID - either IBSS or INFRA network
		// 
		if (is_my_bssid)
		{
			RXWI_STRUC	RxWI;

			pAd->StaCfg.DtimCount = DtimCount;
			pAd->StaCfg.DtimPeriod = DtimPeriod;
			pAd->StaCfg.LastBeaconRxTime = Now;


			RxWI.RSSI0 = Elem->Rssi0;
			RxWI.RSSI1 = Elem->Rssi1;
			RxWI.RSSI2 = Elem->Rssi2;

			Update_Rssi_Sample(pAd, &pAd->StaCfg.RssiSample, &RxWI);

			DBGPRINT(RT_DEBUG_INFO, ("SYNC - my beacon from %02x:%02x:%02x:%02x:%02x:%02x - Dtim=%d/%d, Rssi=%d, Rssi2=%d, Rssi3=%d\n", 
				Bssid[0], Bssid[1], Bssid[2], Bssid[3], Bssid[4], Bssid[5], DtimCount, DtimPeriod, pAd->StaCfg.RssiSample.LastRssi0, pAd->StaCfg.RssiSample.LastRssi1, pAd->StaCfg.RssiSample.LastRssi2));

			if (AironetCellPowerLimit != 0xFF)
			{
				//
				// We get the Cisco (ccx) "TxPower Limit" required
				// Changed to appropriate TxPower Limit for Ciso Compatible Extensions
				//
				ChangeToCellPowerLimit(pAd, AironetCellPowerLimit);
			}
			else
			{
				//
				// AironetCellPowerLimit equal to 0xFF means the Cisco (ccx) "TxPower Limit" not exist.
				// Used the default TX Power Percentage, that set from UI.	
				//
				pAd->CommonCfg.TxPowerPercentage = pAd->CommonCfg.TxPowerDefault;	
			}

			// at least one 11b peer joined. downgrade the MaxTxRate to 11Mbps
			// after last 11b peer left for several seconds, we'll auto switch back to 11G rate
			// in MlmePeriodicExec()
			if (ADHOC_ON(pAd) && (CAP_IS_IBSS_ON(CapabilityInfo)))   
			{
				 BOOLEAN	bRestart;
                BOOLEAN	bnRestart;
                
				// this timestamp is for MlmePeriodicExec() to check if all 11B peers have left
				if (SupRateLen+ExtRateLen <= 4)				
					pAd->StaCfg.Last11bBeaconRxTime = Now;
                
				bRestart = FALSE;
                bnRestart = FALSE;
				if ((SupRateLen+ExtRateLen <= 4) && (pAd->CommonCfg.MaxTxRate > RATE_11))
				{
					DBGPRINT(RT_DEBUG_TRACE, ("SYNC - 11b peer joined. down-grade to 11b TX rates \n"));
					NdisMoveMemory(pAd->StaActive.SupRate, SupRate, MAX_LEN_OF_SUPPORTED_RATES);
					pAd->StaActive.SupRateLen = SupRateLen;
					NdisMoveMemory(pAd->StaActive.ExtRate, ExtRate, MAX_LEN_OF_SUPPORTED_RATES);
					pAd->StaActive.ExtRateLen = ExtRateLen;
                    pAd->StaCfg.AdhocBOnlyJoined = TRUE;
					bRestart = TRUE;
				}
				
				// Update Ht Phy. 
				if ((pAd->CommonCfg.PhyMode >= PHY_11ABGN_MIXED))
				{
#if 0
					// turn on Ht rate
					if ((HtCapabilityLen != 0) || (PreNHtCapabilityLen != 0))
					{
						NdisMoveMemory(&pAd->MlmeAux.HtCapability, &HtCapability, SIZE_HT_CAP_IE);
						if (AddHtInfoLen != 0)
							NdisMoveMemory(&pAd->MlmeAux.AddHtInfo, &AddHtInfo, AddHtInfoLen);
						RTMPCheckHt(pAd, Elem->Wcid, &pAd->MlmeAux.HtCapability, &pAd->MlmeAux.AddHtInfo);
						COPY_HTSETTINGS_FROM_MLME_AUX_TO_ACTIVE_CFG(pAd);
						pAd->StaActive.SupportedHtPhy.bHtEnable = TRUE;
                                        
						// Turn on QOs if use HT rate.
						if (pAd->CommonCfg.APEdcaParm.bValid == FALSE)
						{
							pAd->CommonCfg.APEdcaParm.bValid = TRUE;
							pAd->CommonCfg.APEdcaParm.Aifsn[0] = 3;
							pAd->CommonCfg.APEdcaParm.Aifsn[1] = 7;
							pAd->CommonCfg.APEdcaParm.Aifsn[2] = 1;
							pAd->CommonCfg.APEdcaParm.Aifsn[3] = 1;

							pAd->CommonCfg.APEdcaParm.Cwmin[0] = 4;
							pAd->CommonCfg.APEdcaParm.Cwmin[1] = 4;
							pAd->CommonCfg.APEdcaParm.Cwmin[2] = 3;
							pAd->CommonCfg.APEdcaParm.Cwmin[3] = 2;

							pAd->CommonCfg.APEdcaParm.Cwmax[0] = 10;
							pAd->CommonCfg.APEdcaParm.Cwmax[1] = 6;
							pAd->CommonCfg.APEdcaParm.Cwmax[2] = 4;
							pAd->CommonCfg.APEdcaParm.Cwmax[3] = 3;

							pAd->CommonCfg.APEdcaParm.Txop[0]  = 0;
							pAd->CommonCfg.APEdcaParm.Txop[1]  = 0;
							pAd->CommonCfg.APEdcaParm.Txop[2]  = 96;
							pAd->CommonCfg.APEdcaParm.Txop[3]  = 48;
						}
						AsicSetEdcaParm(pAd, &pAd->CommonCfg.APEdcaParm);

						bRestart = TRUE;
					}
					// turn off Ht rate
					if (((pAd->StaActive.SupportedHtPhy.bHtEnable == TRUE) && (HtCapabilityLen == 0)))
					{
						pAd->StaActive.SupportedHtPhy.bHtEnable = FALSE;
						bRestart = TRUE;
					}
#endif
				}
				
				// If peer Adhoc is legacy mode, I don't need to call MlmeUpdateHtTxRates no matter I support HT or not
				if ((bRestart == TRUE) && (bnRestart == FALSE))
				{
					MlmeUpdateTxRates(pAd, FALSE, 0);
					MakeIbssBeacon(pAd);        // re-build BEACON frame
					AsicEnableIbssSync(pAd);    // copy to on-chip memory
				}
				else if ((bRestart == TRUE) && (bnRestart == TRUE))
				{
					MlmeUpdateTxRates(pAd, FALSE, BSS0);
					MlmeUpdateHtTxRates(pAd, BSS0);
					MakeIbssBeacon(pAd);        // re-build BEACON frame
					AsicEnableIbssSync(pAd);    // copy to on-chip memory
				}
			}

#if 0
			if ((pAd->StaCfg.RssiTriggerMode == RSSI_TRIGGERED_UPON_BELOW_THRESHOLD) &&
				(pAd->StaCfg.LastRssi0 < pAd->StaCfg.RssiTrigger))
			{
				NDIS_802_11_RSSI Dbm = pAd->StaCfg.LastRssi0;
				NdisMIndicateStatus(pAd->AdapterHandle, NDIS_STATUS_MEDIA_SPECIFIC_INDICATION, &Dbm, sizeof(NDIS_802_11_RSSI));
				DBGPRINT(RT_DEBUG_INFO, ("SYNC - NdisMIndicateStatus *** RSSI %d dBm, less than threshold %d dBm\n", 
					Dbm, pAd->StaCfg.RssiTrigger - pAd->BbpRssiToDbmDelta));
			}
			else if ((pAd->StaCfg.RssiTriggerMode == RSSI_TRIGGERED_UPON_EXCCEED_THRESHOLD) &&
				(pAd->StaCfg.LastRssi0 > pAd->StaCfg.RssiTrigger))
			{
				NDIS_802_11_RSSI Dbm = pAd->StaCfg.LastRssi0;
				NdisMIndicateStatus(pAd->AdapterHandle, NDIS_STATUS_MEDIA_SPECIFIC_INDICATION, &Dbm, sizeof(NDIS_802_11_RSSI));
				DBGPRINT(RT_DEBUG_INFO, ("SYNC - NdisMIndicateStatus *** RSSI %d dBm, greater than threshold %d dBm\n", 
					Dbm, pAd->StaCfg.RssiTrigger - pAd->BbpRssiToDbmDelta));
			}
#endif

			if (INFRA_ON(pAd)) // && (pAd->CommonCfg.PhyMode == PHY_11BG_MIXED))
			{
				BOOLEAN bUseShortSlot, bUseBGProtection;

				// decide to use/change to - 
				//      1. long slot (20 us) or short slot (9 us) time
				//      2. turn on/off RTS/CTS and/or CTS-to-self protection
				//      3. short preamble
					
				bUseShortSlot = pAd->CommonCfg.bUseShortSlotTime && CAP_IS_SHORT_SLOT(CapabilityInfo);
				if (bUseShortSlot != OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SHORT_SLOT_INUSED))
					AsicSetSlotTime(pAd, bUseShortSlot);

				bUseBGProtection = (pAd->CommonCfg.UseBGProtection == 1) ||    // always use
								   ((pAd->CommonCfg.UseBGProtection == 0) && ERP_IS_USE_PROTECTION(Erp));

				if (pAd->CommonCfg.Channel > 14) // always no BG protection in A-band. falsely happened when switching A/G band to a dual-band AP
					bUseBGProtection = FALSE;

				if (bUseBGProtection != OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED))
				{
					if (bUseBGProtection)
					{
						OPSTATUS_SET_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED);
						AsicUpdateProtect(pAd, pAd->MlmeAux.AddHtInfo.AddHtInfo2.OperaionMode, (OFDMSETPROTECT|CCKSETPROTECT|ALLN_SETPROTECT),FALSE,(pAd->MlmeAux.AddHtInfo.AddHtInfo2.NonGfPresent == 1));
					}
					else
					{
						OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED);
						AsicUpdateProtect(pAd, pAd->MlmeAux.AddHtInfo.AddHtInfo2.OperaionMode, (OFDMSETPROTECT|CCKSETPROTECT|ALLN_SETPROTECT),TRUE,(pAd->MlmeAux.AddHtInfo.AddHtInfo2.NonGfPresent == 1));
					}
					
					DBGPRINT(RT_DEBUG_WARN, ("SYNC - AP changed B/G protection to %d\n", bUseBGProtection));
				}
				
				// check Ht protection mode. and adhere to the Non-GF device indication by AP.
				if ((AddHtInfoLen != 0) && 
					((AddHtInfo.AddHtInfo2.OperaionMode != pAd->MlmeAux.AddHtInfo.AddHtInfo2.OperaionMode) ||
					(AddHtInfo.AddHtInfo2.NonGfPresent != pAd->MlmeAux.AddHtInfo.AddHtInfo2.NonGfPresent)))
				{
					pAd->MlmeAux.AddHtInfo.AddHtInfo2.NonGfPresent = AddHtInfo.AddHtInfo2.NonGfPresent;
					pAd->MlmeAux.AddHtInfo.AddHtInfo2.OperaionMode = AddHtInfo.AddHtInfo2.OperaionMode;
					if (pAd->MlmeAux.AddHtInfo.AddHtInfo2.NonGfPresent == 1)
				{
						AsicUpdateProtect(pAd, pAd->MlmeAux.AddHtInfo.AddHtInfo2.OperaionMode, ALLN_SETPROTECT, FALSE, TRUE);
					}
					else
						AsicUpdateProtect(pAd, pAd->MlmeAux.AddHtInfo.AddHtInfo2.OperaionMode, ALLN_SETPROTECT, FALSE, FALSE);

					DBGPRINT(RT_DEBUG_TRACE, ("SYNC - AP changed N OperaionMode to %d\n", pAd->MlmeAux.AddHtInfo.AddHtInfo2.OperaionMode));
				}
				
				if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED) && 
					ERP_IS_USE_BARKER_PREAMBLE(Erp))
				{
					MlmeSetTxPreamble(pAd, Rt802_11PreambleLong);
					DBGPRINT(RT_DEBUG_TRACE, ("SYNC - AP forced to use LONG preamble\n"));
				}

				if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_WMM_INUSED)    &&
					(EdcaParm.bValid == TRUE)                          &&
					(EdcaParm.EdcaUpdateCount != pAd->CommonCfg.APEdcaParm.EdcaUpdateCount))
				{
					DBGPRINT(RT_DEBUG_TRACE, ("SYNC - AP change EDCA parameters(from %d to %d)\n", 
						pAd->CommonCfg.APEdcaParm.EdcaUpdateCount,
						EdcaParm.EdcaUpdateCount));
					AsicSetEdcaParm(pAd, &EdcaParm);
				}

				// copy QOS related information
				NdisMoveMemory(&pAd->CommonCfg.APQbssLoad, &QbssLoad, sizeof(QBSS_LOAD_PARM));
				NdisMoveMemory(&pAd->CommonCfg.APQosCapability, &QosCapability, sizeof(QOS_CAPABILITY_PARM));
			}

			// only INFRASTRUCTURE mode support power-saving feature
			if ((INFRA_ON(pAd) && (pAd->StaCfg.Psm == PWR_SAVE)) || (pAd->CommonCfg.bAPSDForcePowerSave))
			{
				UCHAR FreeNumber;
				//  1. AP has backlogged unicast-to-me frame, stay AWAKE, send PSPOLL
				//  2. AP has backlogged broadcast/multicast frame and we want those frames, stay AWAKE
				//  3. we have outgoing frames in TxRing or MgmtRing, better stay AWAKE
				//  4. Psm change to PWR_SAVE, but AP not been informed yet, we better stay AWAKE
				//  5. otherwise, put PHY back to sleep to save battery.
				if (MessageToMe)
				{
					DBGPRINT(RT_DEBUG_TRACE, ("SYNC - AP backlog unicast-to-me, stay AWAKE, send PSPOLL\n"));
#ifdef RT2860
					if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_ADVANCE_POWER_SAVE_PCIE_DEVICE))
					{
						RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, pAd->StaActive.BBPR3);
						// Turn clk to 80Mhz.
    					if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_CLKSELECT_40MHZ))
						{               
							RTMP_IO_WRITE32(pAd, PBF_SYS_CTRL, 0xe80);
							OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_CLKSELECT_40MHZ);
						}
					}
#endif // RT2860 //
					if (pAd->CommonCfg.bAPSDCapable && pAd->CommonCfg.APEdcaParm.bAPSDCapable &&
						pAd->CommonCfg.bAPSDAC_BE && pAd->CommonCfg.bAPSDAC_BK && pAd->CommonCfg.bAPSDAC_VI && pAd->CommonCfg.bAPSDAC_VO)
					{
						pAd->CommonCfg.bNeedSendTriggerFrame = TRUE;
					}
					else
						RT28XX_PS_POLL_ENQUEUE(pAd);
				}
				else if (BcastFlag && (DtimCount == 0) && OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_RECEIVE_DTIM))
				{
#ifdef RT2860
					if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_ADVANCE_POWER_SAVE_PCIE_DEVICE))
					{
						RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, pAd->StaActive.BBPR3);
						if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_CLKSELECT_40MHZ))
						{ 
							// Turn clk to 80Mhz.  
							RTMP_IO_WRITE32(pAd, PBF_SYS_CTRL, 0xe80);
							OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_CLKSELECT_40MHZ);
						}
					}
#endif // RT2860 //
					DBGPRINT(RT_DEBUG_TRACE, ("SYNC - AP backlog broadcast/multicast, stay AWAKE\n"));
				} 
#if 1
				else if ((pAd->TxSwQueue[QID_AC_BK].Number != 0)													||
						(pAd->TxSwQueue[QID_AC_BE].Number != 0)														||
						(pAd->TxSwQueue[QID_AC_VI].Number != 0)														||
						(pAd->TxSwQueue[QID_AC_VO].Number != 0)														||
						(RTMPFreeTXDRequest(pAd, QID_AC_BK, TX_RING_SIZE - 1, &FreeNumber) != NDIS_STATUS_SUCCESS)	||
						(RTMPFreeTXDRequest(pAd, QID_AC_BE, TX_RING_SIZE - 1, &FreeNumber) != NDIS_STATUS_SUCCESS)	||
						(RTMPFreeTXDRequest(pAd, QID_AC_VI, TX_RING_SIZE - 1, &FreeNumber) != NDIS_STATUS_SUCCESS)	||
						(RTMPFreeTXDRequest(pAd, QID_AC_VO, TX_RING_SIZE - 1, &FreeNumber) != NDIS_STATUS_SUCCESS)	||
						(RTMPFreeTXDRequest(pAd, QID_MGMT, MGMT_RING_SIZE - 1, &FreeNumber) != NDIS_STATUS_SUCCESS)) 
#else
				else if ((RTMPFreeTXDRequest(pAd, QID_AC_BE, TX_RING_SIZE, &FreeNumber) != NDIS_STATUS_SUCCESS)  ||
//						(RTMPFreeTXDRequest(pAd, QID_AC_BK, TX_RING_SIZE, &FreeNumber) != NDIS_STATUS_SUCCESS)  ||
//						(RTMPFreeTXDRequest(pAd, QID_AC_VI, TX_RING_SIZE, &FreeNumber) != NDIS_STATUS_SUCCESS)       ||
//						(RTMPFreeTXDRequest(pAd, QID_AC_VO, TX_RING_SIZE, &FreeNumber) != NDIS_STATUS_SUCCESS)       ||
						(RTMPFreeTXDRequest(pAd, QID_MGMT, MGMT_RING_SIZE, &FreeNumber) != NDIS_STATUS_SUCCESS))
#endif
				{
					// TODO: consider scheduled HCCA. might not be proper to use traditional DTIM-based power-saving scheme
					// can we cheat here (i.e. just check MGMT & AC_BE) for better performance?
					DBGPRINT(RT_DEBUG_TRACE, ("SYNC - outgoing frame in TxRing/MgmtRing, stay AWAKE\n"));
#ifdef RT2860
					if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_ADVANCE_POWER_SAVE_PCIE_DEVICE))
					{
						RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, pAd->StaActive.BBPR3);
						if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_CLKSELECT_40MHZ))
						{ 
							// Turn clk to 80Mhz.  
							RTMP_IO_WRITE32(pAd, PBF_SYS_CTRL, 0xe80);
							OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_CLKSELECT_40MHZ);
						}
					}
#endif // RT2860 //                    
				}
				else 
				{
					USHORT NextDtim = DtimCount;

					if (NextDtim == 0) 
						NextDtim = DtimPeriod;

					TbttNumToNextWakeUp = pAd->StaCfg.DefaultListenCount;
					if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_RECEIVE_DTIM) && (TbttNumToNextWakeUp > NextDtim))
						TbttNumToNextWakeUp = NextDtim;

					DBGPRINT(RT_DEBUG_INFO, ("SYNC - PHY sleeps for %d TBTT, Dtim=%d/%d\n", TbttNumToNextWakeUp, DtimCount, DtimPeriod));
					if (!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE))
					{
						AsicSleepThenAutoWakeup(pAd, TbttNumToNextWakeUp);
					}
				}
			}
#ifndef	SINGLE_ADHOC_LINKUP
			// At least another peer in this IBSS, declare MediaState as CONNECTED
			if (ADHOC_ON(pAd) && 
				!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED) && ((CapabilityInfo&0x1)==0))
			{
				OPSTATUS_SET_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED); 

				pAd->IndicateMediaState = NdisMediaStateConnected;
#ifdef WIN_NDIS
				NdisMIndicateStatus(pAd->AdapterHandle, NDIS_STATUS_MEDIA_CONNECT, (PVOID)NULL, 0);
				NdisMIndicateStatusComplete(pAd->AdapterHandle);
#else
				RTMP_IndicateMediaState();
#endif
                pAd->ExtraInfo = GENERAL_LINK_UP;
				AsicSetBssid(pAd, pAd->CommonCfg.Bssid);
				// 2003/03/12 - john
				// Make sure this entry in "ScanTab" table, thus complies to Microsoft's policy that
				// "site survey" result should always include the current connected network. 
				//
				Bssidx = BssTableSearch(&pAd->ScanTab, Bssid, Channel);
				if (Bssidx == BSS_NOT_FOUND)
				{
					Bssidx = BssTableSetEntry(pAd, &pAd->ScanTab, Bssid, Ssid, SsidLen, BssType, BeaconPeriod, 
								&CfParm, AtimWin, CapabilityInfo, SupRate, SupRateLen, ExtRate, ExtRateLen, &HtCapability,
								&AddHtInfo, HtCapabilityLen, AddHtInfoLen, NewExtChannelOffset, Channel, RealRssi, TimeStamp, 0, 
								&EdcaParm, &QosCapability, &QbssLoad, LenVIE, pVIE);
				}
				DBGPRINT(RT_DEBUG_TRACE, ("ADHOC  fOP_STATUS_MEDIA_STATE_CONNECTED.\n"));
			}	
			// Ad-hoc mode is using MAC address as BA session. So we need to continuously find newly joined adhoc station by receiving beacon.
			// To prevent always check this, we use wcid == RESERVED_WCID to recognize it as newly joined adhoc station.
			if (ADHOC_ON(pAd) && (Elem->Wcid == RESERVED_WCID))
			{
				UCHAR	idx;
				MAC_TABLE_ENTRY *pEntry;
				
				// look up the existing table
				pEntry = MacTableLookup(pAd, Addr2);
				if (pEntry == NULL)
				{
					// Another adhoc joining, add to our MAC table. 
					pEntry = MacTableInsertEntry(pAd, Addr2, BSS0, FALSE);	 			
					if (pEntry)
					{
						pEntry->Sst = SST_ASSOC;
						idx = pAd->StaCfg.DefaultKeyId;	
						// After InsertEntry, Write to ASIC on-chip table.
#if 0						
						AsicAddWcidCipherEntry(pAd, 
											(UCHAR)pEntry->Aid, 
											BSS0, 
											SHAREDKEYTABLE,
											pAd->SharedKey[BSS0][0].CipherAlg,
											Addr2, 
											&pAd->SharedKey[BSS0][0]);
#else
						RT28XX_STA_SECURITY_INFO_ADD(pAd, BSS0, idx, pEntry);
#endif
						DBGPRINT(RT_DEBUG_TRACE, ("ADHOC %x:%x:%x:%x:%x:%x  join in.Entry=%d\n", Addr2[0],Addr2[1],Addr2[2],Addr2[3],Addr2[4],Addr2[5], pEntry->Aid));

                        if ((pAd->CommonCfg.BACapability.field.AutoBA == TRUE) &&
                            (HtCapabilityLen > 0))
						    BAOriSessionSetUp(pAd, pEntry, 0, 0, 2000, FALSE);

                        pEntry->HTPhyMode.word = pAd->StaCfg.HTPhyMode.word;
                        if (HtCapabilityLen <= 0)
                        {
                            pEntry->HTPhyMode.field.STBC = 0;
                            pEntry->HTPhyMode.field.BW = 0;
                            pEntry->HTPhyMode.field.ShortGI = 0;
                            if ((SupRateLen+ExtRateLen <= 4) && (pAd->CommonCfg.Channel <= 14))
        					{
        						pEntry->HTPhyMode.field.MODE = MODE_CCK;
        					}
        					else
        					{
        						pEntry->HTPhyMode.field.MODE = MODE_OFDM;
        					}
                        }


#ifdef WPA_SUPPLICANT_SUPPORT
#ifndef NATIVE_WPA_SUPPLICANT_SUPPORT
                        if (pAd->StaCfg.WpaSupplicantUP)
                        {
                            union iwreq_data    wrqu;

                            SendAssocIEsToWpaSupplicant(pAd);
                            memset(&wrqu, 0, sizeof(wrqu));
                            wrqu.data.flags = RT_ASSOC_EVENT_FLAG;
                            wireless_send_event(pAd->net_dev, IWEVCUSTOM, &wrqu, NULL);
                        }
#endif // NATIVE_WPA_SUPPLICANT_SUPPORT //
#endif // WPA_SUPPLICANT_SUPPORT //                    

#ifdef NATIVE_WPA_SUPPLICANT_SUPPORT
                        {
                            union iwreq_data    wrqu;
                            wext_notify_event_assoc(pAd);
                            
                            memset(wrqu.ap_addr.sa_data, 0, MAC_ADDR_LEN);
                            memcpy(wrqu.ap_addr.sa_data, pAd->MlmeAux.Bssid, MAC_ADDR_LEN);
                            wireless_send_event(pAd->net_dev, SIOCGIWAP, &wrqu, NULL);
                            
                        }
#endif // NATIVE_WPA_SUPPLICANT_SUPPORT //
					}
				}
			}
#endif
		}
		// not my BSSID, ignore it
	}
	// sanity check fail, ignore this frame
}

/* 
	==========================================================================
	Description:
		Receive PROBE REQ from remote peer when operating in IBSS mode
	==========================================================================
 */
VOID PeerProbeReqAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 
{
	UCHAR         Addr2[MAC_ADDR_LEN];
	CHAR          Ssid[MAX_LEN_OF_SSID];
	UCHAR           SsidLen, HtLen, AddHtLen, NewExtLen;
	HEADER_802_11 ProbeRspHdr;
	NDIS_STATUS   NStatus;
	PUCHAR        pOutBuffer = NULL;
	ULONG         FrameLen = 0;
	LARGE_INTEGER FakeTimestamp;
	UCHAR         DsLen = 1, IbssLen = 2;
	UCHAR         LocalErpIe[3] = {IE_ERP, 1, 0};
	BOOLEAN       Privacy;
	USHORT        CapabilityInfo;
	UCHAR		  RSNIe = IE_WPA;

	if (! ADHOC_ON(pAd))
		return;

	if (PeerProbeReqSanity(pAd, Elem->Msg, Elem->MsgLen, Addr2, Ssid, &SsidLen))
	{
		if ((SsidLen == 0) || SSID_EQUAL(Ssid, SsidLen, pAd->CommonCfg.Ssid, pAd->CommonCfg.SsidLen))
		{
#if 0
			CSR15_STRUC Csr15;

			// we should respond a ProbeRsp only when we're the last BEACON transmitter 
			// in this ADHOC network.
			RTMP_IO_READ32(pAd, CSR15, &Csr15.word);
			if (Csr15.field.BeaconSent == 0)
			{
				DBGPRINT(RT_DEBUG_INFO, ("SYNC - NOT last BEACON sender, no PROBE_RSP to %02x:%02x:%02x:%02x:%02x:%02x...\n",
					Addr2[0],Addr2[1],Addr2[2],Addr2[3],Addr2[4],Addr2[5] ));
				return;
			}
#endif
			
			// allocate and send out ProbeRsp frame
			NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  //Get an unused nonpaged memory
			if (NStatus != NDIS_STATUS_SUCCESS)
				return;

			//pAd->StaCfg.AtimWin = 0;  // ??????
			DBGPRINT(RT_DEBUG_INFO, ("SYNC - Send PROBE_RSP to %02x:%02x:%02x:%02x:%02x:%02x...\n", 
				Addr2[0],Addr2[1],Addr2[2],Addr2[3],Addr2[4],Addr2[5] ));
			MgtMacHeaderInit(pAd, &ProbeRspHdr, SUBTYPE_PROBE_RSP, 0, Addr2, pAd->CommonCfg.Bssid);

			Privacy = (pAd->StaCfg.WepStatus == Ndis802_11Encryption1Enabled) || 
					  (pAd->StaCfg.WepStatus == Ndis802_11Encryption2Enabled) || 
					  (pAd->StaCfg.WepStatus == Ndis802_11Encryption3Enabled);
			CapabilityInfo = CAP_GENERATE(0, 1, Privacy, (pAd->CommonCfg.TxPreamble == Rt802_11PreambleShort), 0, 0);

			MakeOutgoingFrame(pOutBuffer,                   &FrameLen, 
							  sizeof(HEADER_802_11),        &ProbeRspHdr, 
							  TIMESTAMP_LEN,                &FakeTimestamp,
							  2,                            &pAd->CommonCfg.BeaconPeriod,
							  2,                            &CapabilityInfo,
							  1,                            &SsidIe, 
							  1,                            &pAd->CommonCfg.SsidLen, 
							  pAd->CommonCfg.SsidLen,       pAd->CommonCfg.Ssid,
							  1,                            &SupRateIe, 
							  1,                            &pAd->StaActive.SupRateLen,
							  pAd->StaActive.SupRateLen,    pAd->StaActive.SupRate, 
							  1,                            &DsIe, 
							  1,                            &DsLen, 
							  1,                            &pAd->CommonCfg.Channel,
							  1,                            &IbssIe, 
							  1,                            &IbssLen, 
							  2,                            &pAd->StaActive.AtimWin,
							  END_OF_ARGS);

			if (pAd->StaActive.ExtRateLen)
			{
				ULONG tmp;
				MakeOutgoingFrame(pOutBuffer + FrameLen,        &tmp, 
								  3,                            LocalErpIe,
								  1,                            &ExtRateIe,
								  1,                            &pAd->StaActive.ExtRateLen,
								  pAd->StaActive.ExtRateLen,    &pAd->StaActive.ExtRate,
								  END_OF_ARGS);
				FrameLen += tmp;
			}

			// If adhoc secruity is set for WPA-None, append the cipher suite IE
			if (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPANone)
			{
#if 0
				ULONG	tmp;
				
				if (pAd->StaCfg.WepStatus == Ndis802_11Encryption2Enabled) 	// Tkip
				{
					MakeOutgoingFrame(pOutBuffer + FrameLen, 		&tmp,
									  1,						    &WpaIe,
									  1,							&CipherSuiteWpaNoneTkipLen,
									  CipherSuiteWpaNoneTkipLen,	&CipherSuiteWpaNoneTkip[0],
									  END_OF_ARGS);
					FrameLen += tmp;
				}
				else if (pAd->StaCfg.WepStatus == Ndis802_11Encryption3Enabled)	// Aes
				{
					MakeOutgoingFrame(pOutBuffer + FrameLen, 		&tmp,
									  1,						    &WpaIe,
									  1,							&CipherSuiteWpaNoneAesLen,
									  CipherSuiteWpaNoneAesLen,	    &CipherSuiteWpaNoneAes[0],
									  END_OF_ARGS);
					FrameLen += tmp;
				}
#else
				ULONG tmp;
				MakeOutgoingFrame(pOutBuffer + FrameLen,        	&tmp,
						  			1,                              &RSNIe,
						  			1,                            	&pAd->StaCfg.RSNIE_Len,
						  			pAd->StaCfg.RSNIE_Len,      	pAd->StaCfg.RSN_IE,
						  			END_OF_ARGS);
				FrameLen += tmp;
#endif
			}
			if (pAd->CommonCfg.PhyMode >= PHY_11ABGN_MIXED)
			{
				ULONG TmpLen;
				UCHAR	BROADCOM[4] = {0x0, 0x90, 0x4c, 0x33};
				HtLen = sizeof(pAd->CommonCfg.HtCapability);
				AddHtLen = sizeof(pAd->CommonCfg.AddHTInfo);
				NewExtLen = 1;
				//New extension channel offset IE is included in Beacon, Probe Rsp or channel Switch Announcement Frame
				if (pAd->bBroadComHT == TRUE)
				{
					MakeOutgoingFrame(pOutBuffer + FrameLen,            &TmpLen,
								  1,                                &WpaIe,
								  4,                                &BROADCOM[0],
								 pAd->MlmeAux.HtCapabilityLen,          &pAd->MlmeAux.HtCapability, 
								  END_OF_ARGS);
				}
				else				
				{
				MakeOutgoingFrame(pOutBuffer + FrameLen,            &TmpLen,
								  1,                                &HtCapIe,
								  1,                                &HtLen,
								 sizeof(HT_CAPABILITY_IE),          &pAd->CommonCfg.HtCapability, 
								  1,                                &AddHtInfoIe,
								  1,                                &AddHtLen,
								 sizeof(ADD_HT_INFO_IE),          &pAd->CommonCfg.AddHTInfo, 
								  1,                                &NewExtChanIe,
								  1,                                &NewExtLen,
								 sizeof(NEW_EXT_CHAN_IE),          &pAd->CommonCfg.NewExtChanOffset, 
								  END_OF_ARGS);
				}
				FrameLen += TmpLen;
			}

			MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
			MlmeFreeMemory(pAd, pOutBuffer);
		}
	}
}

VOID BeaconTimeoutAtJoinAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 
{
	USHORT Status;
	DBGPRINT(RT_DEBUG_TRACE, ("SYNC - BeaconTimeoutAtJoinAction\n"));
	pAd->Mlme.SyncMachine.CurrState = SYNC_IDLE;
	Status = MLME_REJ_TIMEOUT;
	MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_JOIN_CONF, 2, &Status);
}

/* 
	==========================================================================
	Description:
		Scan timeout procedure. basically add channel index by 1 and rescan
	==========================================================================
 */
VOID ScanTimeoutAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 
{
//	DBGPRINT(RT_DEBUG_TRACE,("ScanTimeoutAction\n"));
	pAd->MlmeAux.Channel = NextChannel(pAd, pAd->MlmeAux.Channel);

	// Only one channel scanned for CISCO beacon request
	if ((pAd->MlmeAux.ScanType == SCAN_CISCO_ACTIVE) || 
		(pAd->MlmeAux.ScanType == SCAN_CISCO_PASSIVE) ||
		(pAd->MlmeAux.ScanType == SCAN_CISCO_NOISE) ||
		(pAd->MlmeAux.ScanType == SCAN_CISCO_CHANNEL_LOAD))
		pAd->MlmeAux.Channel = 0;

#ifdef MESH_SUPPORT
	// Only one channel scanned to prevent Mesh dis-connecting
	if (MESH_ON(pAd))
		pAd->MlmeAux.Channel = 0;
#endif // MESH_SUPPORT //

	ScanNextChannel(pAd); // this routine will stop if pAd->MlmeAux.Channel == 0
}

#if 0 // move to cmm_sync.c
/* 
	==========================================================================
	Description:
		Scan next channel
	==========================================================================
 */
VOID ScanNextChannel(
	IN PRTMP_ADAPTER pAd) 
{
	HEADER_802_11   Hdr80211;
	PUCHAR          pOutBuffer = NULL;
	NDIS_STATUS     NStatus;
	ULONG           FrameLen = 0;
	UCHAR           SsidLen = 0, ScanType = pAd->MlmeAux.ScanType;
	USHORT          Status;
	PHEADER_802_11  pHdr80211;
	UCHAR		HtLen;
	if (pAd->MlmeAux.Channel == 0) 
	{
		if (pAd->CommonCfg.CentralChannel != pAd->CommonCfg.Channel)
		{
			AsicSwitchChannel(pAd, pAd->CommonCfg.CentralChannel, FALSE);
			AsicLockChannel(pAd, pAd->CommonCfg.CentralChannel);
			DBGPRINT(RT_DEBUG_TRACE, ("SYNC - End of SCAN, restore to channel %d, Total BSS[%02d]\n",pAd->CommonCfg.CentralChannel, pAd->ScanTab.BssNr));
		}
		else
		{
			AsicSwitchChannel(pAd, pAd->CommonCfg.Channel, FALSE);
		AsicLockChannel(pAd, pAd->CommonCfg.Channel);
			DBGPRINT(RT_DEBUG_TRACE, ("SYNC - End of SCAN, restore to channel %d, Total BSS[%02d]\n",pAd->CommonCfg.Channel, pAd->ScanTab.BssNr));
		}
		
		//
		// To prevent data lost.
		// Send an NULL data with turned PSM bit on to current associated AP before SCAN progress.
		// Now, we need to send an NULL data with turned PSM bit off to AP, when scan progress done 
		//
		if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED) && (INFRA_ON(pAd)))
		{
			NStatus = MlmeAllocateMemory(pAd, (PVOID)&pOutBuffer);
			if (NStatus	== NDIS_STATUS_SUCCESS)
			{
				pHdr80211 = (PHEADER_802_11) pOutBuffer;
				MgtMacHeaderInit(pAd, pHdr80211, SUBTYPE_NULL_FUNC, 1, pAd->CommonCfg.Bssid, pAd->CommonCfg.Bssid);
				pHdr80211->Duration = 0;
				pHdr80211->FC.Type = BTYPE_DATA;
				pHdr80211->FC.PwrMgmt = (pAd->StaCfg.Psm == PWR_SAVE);

				// Send using priority queue
				MiniportMMRequest(pAd, 0, pOutBuffer, sizeof(HEADER_802_11));
				DBGPRINT(RT_DEBUG_TRACE, ("MlmeScanReqAction -- Send PSM Data frame\n"));
				MlmeFreeMemory(pAd, pOutBuffer);
				RTMPusecDelay(5000);
			}
		}

		pAd->Mlme.SyncMachine.CurrState = SYNC_IDLE;
		Status = MLME_SUCCESS;
		MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_SCAN_CONF, 2, &Status);
	} 
	else 
	{
		// BBP and RF are not accessible in PS mode, we has to wake them up first
		if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE))
			AsicForceWakeup(pAd);

		// leave PSM during scanning. otherwise we may lost ProbeRsp & BEACON
		if (pAd->StaCfg.Psm == PWR_SAVE)
			MlmeSetPsmBit(pAd, PWR_ACTIVE);

		AsicSwitchChannel(pAd, pAd->MlmeAux.Channel, TRUE);
		AsicLockChannel(pAd, pAd->MlmeAux.Channel);

		// We need to shorten active scan time in order for WZC connect issue
		// Chnage the channel scan time for CISCO stuff based on its IAPP announcement
		if (ScanType == FAST_SCAN_ACTIVE)
			RTMPSetTimer(&pAd->MlmeAux.ScanTimer, FAST_ACTIVE_SCAN_TIME); 
		else if ((ScanType == SCAN_CISCO_ACTIVE) ||
				(ScanType == SCAN_CISCO_PASSIVE) ||
				(ScanType == SCAN_CISCO_CHANNEL_LOAD) ||
				(ScanType == SCAN_CISCO_NOISE))
		{
			if (pAd->StaCfg.CCXScanTime < 25)
				RTMPSetTimer(&pAd->MlmeAux.ScanTimer, pAd->StaCfg.CCXScanTime * 2);
			else	
				RTMPSetTimer(&pAd->MlmeAux.ScanTimer, pAd->StaCfg.CCXScanTime);
		}
		else // must be SCAN_PASSIVE or SCAN_ACTIVE
		{
			if (pAd->CommonCfg.PhyMode == PHY_11ABG_MIXED)
				RTMPSetTimer(&pAd->MlmeAux.ScanTimer, MIN_CHANNEL_TIME);
			else
				RTMPSetTimer(&pAd->MlmeAux.ScanTimer, MAX_CHANNEL_TIME);
		}

		if ((ScanType == SCAN_ACTIVE) || (ScanType == FAST_SCAN_ACTIVE) ||
			(ScanType == SCAN_CISCO_ACTIVE))
		{
			NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  //Get an unused nonpaged memory
			if (NStatus != NDIS_STATUS_SUCCESS)
			{
				DBGPRINT(RT_DEBUG_TRACE, ("SYNC - ScanNextChannel() allocate memory fail\n"));
				pAd->Mlme.SyncMachine.CurrState = SYNC_IDLE;
				Status = MLME_FAIL_NO_RESOURCE;
				MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_SCAN_CONF, 2, &Status);
				return;
			}

			// There is no need to send broadcast probe request if active scan is in effect.
			if ((ScanType == SCAN_ACTIVE) || (ScanType == FAST_SCAN_ACTIVE))
				SsidLen = pAd->MlmeAux.SsidLen;
			else
				SsidLen = 0;

			MgtMacHeaderInit(pAd, &Hdr80211, SUBTYPE_PROBE_REQ, 0, BROADCAST_ADDR, BROADCAST_ADDR);
			MakeOutgoingFrame(pOutBuffer,               &FrameLen,
							  sizeof(HEADER_802_11),    &Hdr80211,
							  1,                        &SsidIe,
							  1,                        &SsidLen,
							  SsidLen,			        pAd->MlmeAux.Ssid,
							  1,                        &SupRateIe,
							  1,                        &pAd->CommonCfg.SupRateLen,
							  pAd->CommonCfg.SupRateLen,  pAd->CommonCfg.SupRate, 
							  END_OF_ARGS);

			if (pAd->CommonCfg.ExtRateLen)
			{
				ULONG Tmp;
				MakeOutgoingFrame(pOutBuffer + FrameLen,            &Tmp,
								  1,                                &ExtRateIe,
								  1,                                &pAd->CommonCfg.ExtRateLen,
								  pAd->CommonCfg.ExtRateLen,          pAd->CommonCfg.ExtRate, 
								  END_OF_ARGS);
				FrameLen += Tmp;
			}
			if ((pAd->CommonCfg.PhyMode == PHY_11N) || (pAd->CommonCfg.PhyMode >= PHY_11ABGN_MIXED))
			{
				ULONG Tmp;
				UCHAR	BROADCOM[4] = {0x0, 0x90, 0x4c, 0x33};
				if (pAd->bBroadComHT == TRUE)
				{
					HtLen = SIZE_HT_CAP_IE + 4;
					MakeOutgoingFrame(pOutBuffer + FrameLen,            &Tmp,
								  1,                                &WpaIe,
									  1,                                &HtLen,
								  4,                                &BROADCOM[0],
								 pAd->MlmeAux.HtCapabilityLen,          &pAd->MlmeAux.HtCapability, 
								  END_OF_ARGS);
				}
				else				
				{
				HtLen = SIZE_HT_CAP_IE;
				MakeOutgoingFrame(pOutBuffer + FrameLen,            &Tmp,
								  1,                                &HtCapIe,
								  1,                                &HtLen,
								 HtLen,          &pAd->CommonCfg.HtCapability, 
								  END_OF_ARGS);
				}
				FrameLen += Tmp;
			}
			MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
			MlmeFreeMemory(pAd, pOutBuffer);
		}

		// For SCAN_CISCO_PASSIVE, do nothing and silently wait for beacon or other probe reponse
		
		pAd->Mlme.SyncMachine.CurrState = SCAN_LISTEN;
	}
}
#endif // if 0

/* 
	==========================================================================
	Description:
	==========================================================================
 */
VOID InvalidStateWhenScan(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 
{
	USHORT Status;
	DBGPRINT(RT_DEBUG_TRACE, ("AYNC - InvalidStateWhenScan(state=%ld). Reset SYNC machine\n", pAd->Mlme.SyncMachine.CurrState));
	pAd->Mlme.SyncMachine.CurrState = SYNC_IDLE;
	Status = MLME_STATE_MACHINE_REJECT;
	MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_SCAN_CONF, 2, &Status);
}

/* 
	==========================================================================
	Description:
	==========================================================================
 */
VOID InvalidStateWhenJoin(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 
{
	USHORT Status;
	DBGPRINT(RT_DEBUG_TRACE, ("InvalidStateWhenJoin(state=%ld). Reset SYNC machine\n", pAd->Mlme.SyncMachine.CurrState));
	pAd->Mlme.SyncMachine.CurrState = SYNC_IDLE;
	Status = MLME_STATE_MACHINE_REJECT;
	MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_JOIN_CONF, 2, &Status);
}

/* 
	==========================================================================
	Description:
	==========================================================================
 */
VOID InvalidStateWhenStart(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 
{
	USHORT Status;
	DBGPRINT(RT_DEBUG_TRACE, ("InvalidStateWhenStart(state=%ld). Reset SYNC machine\n", pAd->Mlme.SyncMachine.CurrState));
	pAd->Mlme.SyncMachine.CurrState = SYNC_IDLE;
	Status = MLME_STATE_MACHINE_REJECT;
	MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_START_CONF, 2, &Status);
}

/* 
	==========================================================================
	Description:

	IRQL = DISPATCH_LEVEL
	
	==========================================================================
 */
VOID EnqueuePsPoll(
	IN PRTMP_ADAPTER pAd) 
{
#ifdef RALINK_ATE
    if(pAd->ate.Mode != ATE_STOP)
    {
		return;
    }
#endif // RALINK_ATE //

	DBGPRINT(RT_DEBUG_TRACE, ("SYNC - send PsPoll ...\n"));
	MiniportMMRequest(pAd, 0, (PUCHAR)&pAd->PsPollFrame, sizeof(PSPOLL_FRAME));
}

// 2003-04-17 john
// driver force send out a BEACON frame to cover ADHOC mode BEACON starving issue
// that is, in ADHOC mode, driver guarantee itself can send out at least a BEACON
// per a specified duration, even the peer's clock is faster than us and win all the
// hardware-based BEACON TX oppertunity. 
// we may remove this software feature once 2560 IC fix this problem in ASIC.
// IRQL = DISPATCH_LEVEL
VOID EnqueueBeaconFrame(
	IN PRTMP_ADAPTER pAd) 
{
#if 0
	PTXWI_STRUC	  pTxWI = &pAd->BeaconTxWI;
	PCHAR         pBeacon = pAd->BeaconBuf;


	DBGPRINT(RT_DEBUG_TRACE, ("SYNC - driver sent BEACON (len=%d)...\n",pTxWI->MPDUtotalByteCount));
	RTMP_IO_READ32(pAd, TSF_TIMER_DW1, &Tsf.u.HighPart);
	RTMP_IO_READ32(pAd, TSF_TIMER_DW0, &Tsf.u.LowPart);
	// TODO: not good if porting to big endian platform - TSF byte order ???
	NdisMoveMemory(pBeacon + sizeof(HEADER_802_11), &Tsf, TIMESTAMP_LEN);
	MiniportMMRequest(pAd, 0, pBeacon, pTxWI->MPDUtotalByteCount);
#endif
}

/* 
	==========================================================================
	Description:
	==========================================================================
 */
VOID EnqueueProbeRequest(
	IN PRTMP_ADAPTER pAd) 
{
	NDIS_STATUS     NState;
	PUCHAR          pOutBuffer;
	ULONG           FrameLen = 0;
	HEADER_802_11   Hdr80211;

	DBGPRINT(RT_DEBUG_TRACE, ("force out a ProbeRequest ...\n"));

	NState = MlmeAllocateMemory(pAd, &pOutBuffer);  //Get an unused nonpaged memory
	if (NState == NDIS_STATUS_SUCCESS) 
	{
		MgtMacHeaderInit(pAd, &Hdr80211, SUBTYPE_PROBE_REQ, 0, BROADCAST_ADDR, BROADCAST_ADDR);

		// this ProbeRequest explicitly specify SSID to reduce unwanted ProbeResponse
		MakeOutgoingFrame(pOutBuffer,                     &FrameLen,
						  sizeof(HEADER_802_11),          &Hdr80211,
						  1,                              &SsidIe,
						  1,                              &pAd->CommonCfg.SsidLen,
						  pAd->CommonCfg.SsidLen,		  pAd->CommonCfg.Ssid,
						  1,                              &SupRateIe,
						  1,                              &pAd->StaActive.SupRateLen,
						  pAd->StaActive.SupRateLen,      pAd->StaActive.SupRate, 
						  END_OF_ARGS);
		MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
		MlmeFreeMemory(pAd, pOutBuffer);
	}

}

#if 0 //this portion already move to cmm_sync.c
/* 
	==========================================================================
	Description:
		Update StaCfg->ChannelList[] according to 1) Country Region 2) RF IC type,
		and 3) PHY-mode user selected.
		The outcome is used by driver when doing site survey.

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL
	
	==========================================================================
 */
VOID BuildChannelList(
	IN PRTMP_ADAPTER pAd)
{
	UCHAR i, j, index=0, num=0;
	PUCHAR	pChannelList;

	NdisZeroMemory(pAd->ChannelList, MAX_NUM_OF_CHANNELS * sizeof(CHANNEL_TX_POWER));

	// if not 11a-only mode, channel list starts from 2.4Ghz band
	if ((pAd->CommonCfg.PhyMode != PHY_11A) && (pAd->CommonCfg.PhyMode != PHY_11AN_MIXED))
	{
		switch (pAd->CommonCfg.CountryRegion  & 0x7f)
		{
			case REGION_0_BG_BAND:	// 1 -11
				NdisMoveMemory(&pAd->ChannelList[index], &pAd->TxPower[BG_BAND_REGION_0_START], sizeof(CHANNEL_TX_POWER) * BG_BAND_REGION_0_SIZE);
				index += BG_BAND_REGION_0_SIZE;
				break;
			case REGION_1_BG_BAND:	// 1 - 13
				NdisMoveMemory(&pAd->ChannelList[index], &pAd->TxPower[BG_BAND_REGION_1_START], sizeof(CHANNEL_TX_POWER) * BG_BAND_REGION_1_SIZE);
				index += BG_BAND_REGION_1_SIZE;
				break;
			case REGION_2_BG_BAND:	// 10 - 11
				NdisMoveMemory(&pAd->ChannelList[index], &pAd->TxPower[BG_BAND_REGION_2_START], sizeof(CHANNEL_TX_POWER) * BG_BAND_REGION_2_SIZE);
				index += BG_BAND_REGION_2_SIZE;
				break;
			case REGION_3_BG_BAND:	// 10 - 13
				NdisMoveMemory(&pAd->ChannelList[index], &pAd->TxPower[BG_BAND_REGION_3_START], sizeof(CHANNEL_TX_POWER) * BG_BAND_REGION_3_SIZE);
				index += BG_BAND_REGION_3_SIZE;
				break;
			case REGION_4_BG_BAND:	// 14
				NdisMoveMemory(&pAd->ChannelList[index], &pAd->TxPower[BG_BAND_REGION_4_START], sizeof(CHANNEL_TX_POWER) * BG_BAND_REGION_4_SIZE);
				index += BG_BAND_REGION_4_SIZE;
				break;
			case REGION_5_BG_BAND:	// 1 - 14
				NdisMoveMemory(&pAd->ChannelList[index], &pAd->TxPower[BG_BAND_REGION_5_START], sizeof(CHANNEL_TX_POWER) * BG_BAND_REGION_5_SIZE);
				index += BG_BAND_REGION_5_SIZE;
				break;
			case REGION_6_BG_BAND:	// 3 - 9
				NdisMoveMemory(&pAd->ChannelList[index], &pAd->TxPower[BG_BAND_REGION_6_START], sizeof(CHANNEL_TX_POWER) * BG_BAND_REGION_6_SIZE);
				index += BG_BAND_REGION_6_SIZE;
				break;
			case REGION_7_BG_BAND:  // 5 - 13
				NdisMoveMemory(&pAd->ChannelList[index], &pAd->TxPower[BG_BAND_REGION_7_START], sizeof(CHANNEL_TX_POWER) * BG_BAND_REGION_7_SIZE);
				index += BG_BAND_REGION_7_SIZE;
				break;
			default:            // Error. should never happen
				break;
		}   
	}

	if ((pAd->CommonCfg.PhyMode == PHY_11A) || (pAd->CommonCfg.PhyMode == PHY_11ABG_MIXED) || (pAd->CommonCfg.PhyMode == PHY_11ABGN_MIXED) ||
		(pAd->CommonCfg.PhyMode == PHY_11AN_MIXED) || (pAd->CommonCfg.PhyMode == PHY_11AGN_MIXED))
	{
		switch (pAd->CommonCfg.CountryRegionForABand & 0x7f)
		{
			case REGION_0_A_BAND:
				num = sizeof(A_BAND_REGION_0_CHANNEL_LIST)/sizeof(UCHAR);
				pChannelList = A_BAND_REGION_0_CHANNEL_LIST;
				break;
			case REGION_1_A_BAND:
				num = sizeof(A_BAND_REGION_1_CHANNEL_LIST)/sizeof(UCHAR);
				pChannelList = A_BAND_REGION_1_CHANNEL_LIST;
				break;
			case REGION_2_A_BAND:
				num = sizeof(A_BAND_REGION_2_CHANNEL_LIST)/sizeof(UCHAR);
				pChannelList = A_BAND_REGION_2_CHANNEL_LIST;
				break;
			case REGION_3_A_BAND:
				num = sizeof(A_BAND_REGION_3_CHANNEL_LIST)/sizeof(UCHAR);
				pChannelList = A_BAND_REGION_3_CHANNEL_LIST;
				break;
			case REGION_4_A_BAND:
				num = sizeof(A_BAND_REGION_4_CHANNEL_LIST)/sizeof(UCHAR);
				pChannelList = A_BAND_REGION_4_CHANNEL_LIST;
				break;
			case REGION_5_A_BAND:
				num = sizeof(A_BAND_REGION_5_CHANNEL_LIST)/sizeof(UCHAR);
				pChannelList = A_BAND_REGION_5_CHANNEL_LIST;
				break;
			case REGION_6_A_BAND:
				num = sizeof(A_BAND_REGION_6_CHANNEL_LIST)/sizeof(UCHAR);
				pChannelList = A_BAND_REGION_6_CHANNEL_LIST;
				break;
			case REGION_7_A_BAND:
				num = sizeof(A_BAND_REGION_7_CHANNEL_LIST)/sizeof(UCHAR);
				pChannelList = A_BAND_REGION_7_CHANNEL_LIST;
				break;
			case REGION_8_A_BAND:
				num = sizeof(A_BAND_REGION_8_CHANNEL_LIST)/sizeof(UCHAR);
				pChannelList = A_BAND_REGION_8_CHANNEL_LIST;
				break;
			case REGION_9_A_BAND:
				num = sizeof(A_BAND_REGION_9_CHANNEL_LIST)/sizeof(UCHAR);
				pChannelList = A_BAND_REGION_9_CHANNEL_LIST;
				break;
			default:            // Error. should never happen
				DBGPRINT(RT_DEBUG_WARN,("countryregion=%d not support", pAd->CommonCfg.CountryRegionForABand));
				break;
		}

		if (num != 0)
		{
			for (i=0; i<num; i++)
			{
				for (j=0; j<MAX_NUM_OF_CHANNELS; j++)
				{
					if (pChannelList[i] == pAd->TxPower[j].Channel)
					{
						NdisMoveMemory(&pAd->ChannelList[index+i], &pAd->TxPower[j], sizeof(CHANNEL_TX_POWER));
					}
				}
			}
			index += num;
		}
	}

	pAd->ChannelListNum = index;
	DBGPRINT(RT_DEBUG_TRACE,("country code=%d/%d, RFIC=%d, PHY mode=%d, support %d channels\n", 
		pAd->CommonCfg.CountryRegion, pAd->CommonCfg.CountryRegionForABand, pAd->RfIcType, pAd->CommonCfg.PhyMode, pAd->ChannelListNum));
#if DBG	
	for (i=0;i<pAd->ChannelListNum;i++)
	{
		DBGPRINT_RAW(RT_DEBUG_TRACE,("BuildChannel # %d :: Pwr0 = %d, Pwr1 =%d, \n ", pAd->ChannelList[i].Channel, pAd->ChannelList[i].Power, pAd->ChannelList[i].Power2));
	}
#endif
}

/* 
	==========================================================================
	Description:
		This routine return the first channel number according to the country 
		code selection and RF IC selection (signal band or dual band). It is called
		whenever driver need to start a site survey of all supported channels.
	Return:
		ch - the first channel number of current country code setting

	IRQL = PASSIVE_LEVEL

	==========================================================================
 */
UCHAR FirstChannel(
	IN PRTMP_ADAPTER pAd)
{
	return pAd->ChannelList[0].Channel;
}

/* 
	==========================================================================
	Description:
		This routine returns the next channel number. This routine is called
		during driver need to start a site survey of all supported channels.
	Return:
		next_channel - the next channel number valid in current country code setting.
	Note:
		return 0 if no more next channel
	==========================================================================
 */
UCHAR NextChannel(
	IN PRTMP_ADAPTER pAd, 
	IN UCHAR channel)
{
	int i;
	UCHAR next_channel = 0;
			
	for (i = 0; i < (pAd->ChannelListNum - 1); i++)
		if (channel == pAd->ChannelList[i].Channel)
		{
			next_channel = pAd->ChannelList[i+1].Channel;
			break;
	}
	return next_channel;
}

/* 
	==========================================================================
	Description:
		This routine is for Cisco Compatible Extensions 2.X 
		Spec31. AP Control of Client Transmit Power
	Return:
		None
	Note:
	   Required by Aironet dBm(mW)
		   0dBm(1mW),   1dBm(5mW), 13dBm(20mW), 15dBm(30mW),
		  17dBm(50mw), 20dBm(100mW)

	   We supported 
		   3dBm(Lowest), 6dBm(10%), 9dBm(25%), 12dBm(50%),
		  14dBm(75%),   15dBm(100%)

		The client station's actual transmit power shall be within +/- 5dB of
		the minimum value or next lower value.
	==========================================================================
 */
VOID ChangeToCellPowerLimit(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR         AironetCellPowerLimit)
{
	//valud 0xFF means that hasn't found power limit information 
	//from the AP's Beacon/Probe response.
	if (AironetCellPowerLimit == 0xFF)
		return;  
	
	if (AironetCellPowerLimit < 6) //Used Lowest Power Percentage.
		pAd->CommonCfg.TxPowerPercentage = 6; 
	else if (AironetCellPowerLimit < 9)
		pAd->CommonCfg.TxPowerPercentage = 10;
	else if (AironetCellPowerLimit < 12)
		pAd->CommonCfg.TxPowerPercentage = 25;
	else if (AironetCellPowerLimit < 14)
		pAd->CommonCfg.TxPowerPercentage = 50;
	else if (AironetCellPowerLimit < 15)
		pAd->CommonCfg.TxPowerPercentage = 75;
	else
		pAd->CommonCfg.TxPowerPercentage = 100; //else used maximum

	if (pAd->CommonCfg.TxPowerPercentage > pAd->CommonCfg.TxPowerDefault)
		pAd->CommonCfg.TxPowerPercentage = pAd->CommonCfg.TxPowerDefault;
	
}

CHAR	ConvertToRssi(
	IN PRTMP_ADAPTER pAd,
	IN	CHAR			Rssi,
	IN  UCHAR   RssiNumber)
{
	UCHAR	RssiOffset, LNAGain;

	// Rssi equals to zero should be an invalid value
	if (Rssi == 0)
		return -99;

    if (pAd->CommonCfg.Channel > 14)
    {
        LNAGain = pAd->ALNAGain;
        if (RssiNumber == 0)
			RssiOffset = pAd->ARssiOffset0;
		else if (RssiNumber == 1)
			RssiOffset = pAd->ARssiOffset1;
		else
			RssiOffset = pAd->ARssiOffset2;
    }
    else
    {
        LNAGain = pAd->BLNAGain;
        if (RssiNumber == 0)
			RssiOffset = pAd->BGRssiOffset0;
		else if (RssiNumber == 1)
			RssiOffset = pAd->BGRssiOffset1;
		else
			RssiOffset = pAd->BGRssiOffset2;
    }
	
    return (-32 - RssiOffset + LNAGain - Rssi);
}
#endif 

