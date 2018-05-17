/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	sanity.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	John Chang  2004-09-01      add WMM support
*/
#include "rt_config.h"

#if 0 // move to mlme.c
UCHAR	WPA_OUI[] = {0x00, 0x50, 0xf2, 0x01};
UCHAR	RSN_OUI[] = {0x00, 0x0f, 0xac};
UCHAR   WME_INFO_ELEM[]  = {0x00, 0x50, 0xf2, 0x02, 0x00, 0x01};
UCHAR   WME_PARM_ELEM[] = {0x00, 0x50, 0xf2, 0x02, 0x01, 0x01};
UCHAR	Ccx2QosInfo[] = {0x00, 0x40, 0x96, 0x04};
UCHAR   RALINK_OUI[]  = {0x00, 0x0c, 0x43};
#else
extern UCHAR	CISCO_OUI[];

extern UCHAR	WPA_OUI[];
extern UCHAR	RSN_OUI[];
extern UCHAR	WME_INFO_ELEM[];
extern UCHAR	WME_PARM_ELEM[];
extern UCHAR	Ccx2QosInfo[];
extern UCHAR	RALINK_OUI[];
extern UCHAR	BROADCOM_OUI[];
#endif

#if 0 // move to cmm_sync.c
/* 
    ==========================================================================
    Description:
        MLME message sanity check
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
 */
BOOLEAN MlmeScanReqSanity(
    IN PRTMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen, 
    OUT UCHAR *pBssType, 
    OUT CHAR Ssid[], 
    OUT UCHAR *pSsidLen, 
    OUT UCHAR *pScanType) 
{
    MLME_SCAN_REQ_STRUCT *Info;

    Info = (MLME_SCAN_REQ_STRUCT *)(Msg);
    *pBssType = Info->BssType;
    *pSsidLen = Info->SsidLen;	
    NdisMoveMemory(Ssid, Info->Ssid, *pSsidLen);
    *pScanType = Info->ScanType;
    
    if ((*pBssType == BSS_INFRA || *pBssType == BSS_ADHOC || *pBssType == BSS_ANY) &&
       (*pScanType == SCAN_ACTIVE || *pScanType == SCAN_PASSIVE ||
       *pScanType == SCAN_CISCO_PASSIVE || *pScanType == SCAN_CISCO_ACTIVE ||
       *pScanType == SCAN_CISCO_CHANNEL_LOAD || *pScanType == SCAN_CISCO_NOISE)) 
        return TRUE;
    else 
    {
        DBGPRINT(RT_DEBUG_TRACE, ("MlmeScanReqSanity fail - wrong BssType or ScanType\n"));
        return FALSE;
    }
}
#endif

/* 
    ==========================================================================
    Description:
        MLME message sanity check
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
 */
BOOLEAN MlmeStartReqSanity(
    IN PRTMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen, 
    OUT CHAR Ssid[], 
    OUT UCHAR *pSsidLen) 
{
    MLME_START_REQ_STRUCT *Info;

    Info = (MLME_START_REQ_STRUCT *)(Msg);
    
    if (Info->SsidLen > MAX_LEN_OF_SSID)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("MlmeStartReqSanity fail - wrong SSID length\n"));
        return FALSE;
    }

    *pSsidLen = Info->SsidLen;
    NdisMoveMemory(Ssid, Info->Ssid, *pSsidLen);

    return TRUE;
}

#if 0 // move to cmm_sync.c
/* 
    ==========================================================================
    Description:
        MLME message sanity check
    Return:
        TRUE if all parameters are OK, FALSE otherwise
        
	IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
BOOLEAN MlmeAssocReqSanity(
    IN PRTMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen, 
    OUT PUCHAR pApAddr, 
    OUT USHORT *pCapabilityInfo, 
    OUT ULONG *pTimeout, 
    OUT USHORT *pListenIntv) 
{
    MLME_ASSOC_REQ_STRUCT *pInfo;

    pInfo = (MLME_ASSOC_REQ_STRUCT *)Msg;
    *pTimeout = pInfo->Timeout;                             // timeout
    COPY_MAC_ADDR(pApAddr, pInfo->Addr);                   // AP address
    *pCapabilityInfo = pInfo->CapabilityInfo;               // capability info
    *pListenIntv = pInfo->ListenIntv;
    
    return TRUE;
}

/* 
    ==========================================================================
    Description:
        MLME message sanity check
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
 */
BOOLEAN MlmeAuthReqSanity(
    IN PRTMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen, 
    OUT PUCHAR pAddr, 
    OUT ULONG *pTimeout, 
    OUT USHORT *pAlg) 
{
    MLME_AUTH_REQ_STRUCT *pInfo;

    pInfo  = (MLME_AUTH_REQ_STRUCT *)Msg;
    COPY_MAC_ADDR(pAddr, pInfo->Addr);
    *pTimeout = pInfo->Timeout;
    *pAlg = pInfo->Alg;
    
    if (((*pAlg == Ndis802_11AuthModeShared) ||(*pAlg == Ndis802_11AuthModeOpen) || (*pAlg == CISCO_AuthModeLEAP)) && 
        ((*pAddr & 0x01) == 0)) 
    {
        return TRUE;
    } 
    else 
    {
        DBGPRINT(RT_DEBUG_TRACE, ("MlmeAuthReqSanity fail - wrong algorithm\n"));
        return FALSE;
    }
}
#endif // 0 //
/* 
    ==========================================================================
    Description:
        MLME message sanity check
    Return:
        TRUE if all parameters are OK, FALSE otherwise
        
    IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
BOOLEAN PeerAssocRspSanity(
    IN PRTMP_ADAPTER pAd, 
    IN VOID *pMsg, 
    IN ULONG MsgLen, 
    OUT PUCHAR pAddr2, 
    OUT USHORT *pCapabilityInfo, 
    OUT USHORT *pStatus, 
    OUT USHORT *pAid, 
    OUT UCHAR SupRate[], 
    OUT UCHAR *pSupRateLen,
    OUT UCHAR ExtRate[], 
    OUT UCHAR *pExtRateLen,
    OUT HT_CAPABILITY_IE		*pHtCapability,
    OUT ADD_HT_INFO_IE		*pAddHtInfo,	// AP might use this additional ht info IE 
    OUT UCHAR			*pHtCapabilityLen,
    OUT UCHAR			*pAddHtInfoLen,
    OUT UCHAR			*pNewExtChannelOffset,
    OUT PEDCA_PARM pEdcaParm,
    OUT UCHAR *pCkipFlag) 
{
    CHAR          IeType, *Ptr;
    PFRAME_802_11 pFrame = (PFRAME_802_11)pMsg;
    PEID_STRUCT   pEid;
    ULONG         Length = 0;
    
	*pNewExtChannelOffset = 0xff;
	*pHtCapabilityLen = 0;
	*pAddHtInfoLen = 0;
    COPY_MAC_ADDR(pAddr2, pFrame->Hdr.Addr2);
    Ptr = pFrame->Octet;
    Length += LENGTH_802_11;
        
    NdisMoveMemory(pCapabilityInfo, &pFrame->Octet[0], 2);
    Length += 2;
    NdisMoveMemory(pStatus,         &pFrame->Octet[2], 2);
    Length += 2;
    *pCkipFlag = 0;
    *pExtRateLen = 0;
    pEdcaParm->bValid = FALSE;
    
    if (*pStatus != MLME_SUCCESS) 
        return TRUE;
    
    NdisMoveMemory(pAid, &pFrame->Octet[4], 2);
    Length += 2;
        
    // Aid already swaped byte order in RTMPFrameEndianChange() for big endian platform
    *pAid = (*pAid) & 0x3fff; // AID is low 14-bit
        
    // -- get supported rates from payload and advance the pointer
    IeType = pFrame->Octet[6];
    *pSupRateLen = pFrame->Octet[7];
    if ((IeType != IE_SUPP_RATES) || (*pSupRateLen > MAX_LEN_OF_SUPPORTED_RATES))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("PeerAssocRspSanity fail - wrong SupportedRates IE\n"));
        return FALSE;
    } 
    else 
        NdisMoveMemory(SupRate, &pFrame->Octet[8], *pSupRateLen);

    Length = Length + 2 + *pSupRateLen;

    // many AP implement proprietary IEs in non-standard order, we'd better
    // tolerate mis-ordered IEs to get best compatibility
    pEid = (PEID_STRUCT) &pFrame->Octet[8 + (*pSupRateLen)];
            
    // get variable fields from payload and advance the pointer
    while ((Length + 2 + pEid->Len) <= MsgLen)
    {
        switch (pEid->Eid)
        {
            case IE_EXT_SUPP_RATES:
                if (pEid->Len <= MAX_LEN_OF_SUPPORTED_RATES)
                {
                    NdisMoveMemory(ExtRate, pEid->Octet, pEid->Len);
                    *pExtRateLen = pEid->Len;
                }
                break;

             case IE_HT_CAP:
            case IE_HT_CAP2:
			if (pEid->Len >= SIZE_HT_CAP_IE)  //Note: allow extension.!!
			{
				NdisMoveMemory(pHtCapability, pEid->Octet, SIZE_HT_CAP_IE);

				*(USHORT *)(&pHtCapability->HtCapInfo) = cpu2le16(*(USHORT *)(&pHtCapability->HtCapInfo));
				*(USHORT *)(&pHtCapability->ExtHtCapInfo) = cpu2le16(*(USHORT *)(&pHtCapability->ExtHtCapInfo));

				*pHtCapabilityLen = SIZE_HT_CAP_IE;
			}
			else
			{
				DBGPRINT(RT_DEBUG_WARN, ("PeerAssocRspSanity - wrong IE_HT_CAP. \n"));
			}
				
		break;
            case IE_ADD_HT:
            case IE_ADD_HT2:
			if (pEid->Len >= sizeof(ADD_HT_INFO_IE))				
			{
				// This IE allows extension, but we can ignore extra bytes beyond our knowledge , so only
				// copy first sizeof(ADD_HT_INFO_IE)
				NdisMoveMemory(pAddHtInfo, pEid->Octet, sizeof(ADD_HT_INFO_IE));
				
				*(USHORT *)(&pAddHtInfo->AddHtInfo2) = cpu2le16(*(USHORT *)(&pAddHtInfo->AddHtInfo2));
				*(USHORT *)(&pAddHtInfo->AddHtInfo3) = cpu2le16(*(USHORT *)(&pAddHtInfo->AddHtInfo3));

				*pAddHtInfoLen = SIZE_ADD_HT_INFO_IE;
			}
			else
			{
				DBGPRINT(RT_DEBUG_WARN, ("PeerAssocRspSanity - wrong IE_ADD_HT. \n"));
			}
				
		break;
            case IE_NEW_EXT_CHA_OFFSET:
			if (pEid->Len == 1)
			{
				*pNewExtChannelOffset = pEid->Octet[0];
			}
			else
			{
				DBGPRINT(RT_DEBUG_WARN, ("PeerAssocRspSanity - wrong IE_NEW_EXT_CHA_OFFSET. \n"));
			}
				
		break;
            case IE_AIRONET_CKIP:
                // 0. Check Aironet IE length, it must be larger or equal to 28
                //    Cisco's AP VxWork version(will not be supported) used this IE length as 28
                //    Cisco's AP IOS version used this IE length as 30 
                if (pEid->Len < (CKIP_NEGOTIATION_LENGTH - 2))
                break;

                // 1. Copy CKIP flag byte to buffer for process
                *pCkipFlag = *(pEid->Octet + 8);				
                break;

            case IE_AIRONET_IPADDRESS:
                if (pEid->Len != 0x0A)
                break;

                // Get Cisco Aironet IP information
                if (NdisEqualMemory(pEid->Octet, CISCO_OUI, 3) == 1)
                    NdisMoveMemory(pAd->StaCfg.AironetIPAddress, pEid->Octet + 4, 4);
                break;

            // CCX2, WMM use the same IE value
            // case IE_CCX_V2:
            case IE_VENDOR_SPECIFIC:
                // handle WME PARAMTER ELEMENT
                if (NdisEqualMemory(pEid->Octet, WME_PARM_ELEM, 6) && (pEid->Len == 24))
                {
                    PUCHAR ptr;
                    int i;
        
                    // parsing EDCA parameters
                    pEdcaParm->bValid          = TRUE;
                    pEdcaParm->bQAck           = FALSE; // pEid->Octet[0] & 0x10;
                    pEdcaParm->bQueueRequest   = FALSE; // pEid->Octet[0] & 0x20;
                    pEdcaParm->bTxopRequest    = FALSE; // pEid->Octet[0] & 0x40;
                    //pEdcaParm->bMoreDataAck    = FALSE; // pEid->Octet[0] & 0x80;
                    pEdcaParm->EdcaUpdateCount = pEid->Octet[6] & 0x0f;
                    pEdcaParm->bAPSDCapable    = (pEid->Octet[6] & 0x80) ? 1 : 0;
                    ptr = &pEid->Octet[8];
                    for (i=0; i<4; i++)
                    {
                        UCHAR aci = (*ptr & 0x60) >> 5; // b5~6 is AC INDEX
                        pEdcaParm->bACM[aci]  = (((*ptr) & 0x10) == 0x10);   // b5 is ACM
                        pEdcaParm->Aifsn[aci] = (*ptr) & 0x0f;               // b0~3 is AIFSN
                        pEdcaParm->Cwmin[aci] = *(ptr+1) & 0x0f;             // b0~4 is Cwmin
                        pEdcaParm->Cwmax[aci] = *(ptr+1) >> 4;               // b5~8 is Cwmax
                        pEdcaParm->Txop[aci]  = *(ptr+2) + 256 * (*(ptr+3)); // in unit of 32-us
                        ptr += 4; // point to next AC
                    }
                }

                // handle CCX IE
                else
                {
                    // 0. Check the size and CCX admin control
                    if (pAd->StaCfg.CCXControl.field.Enable == 0)
                        break;
                    if (pEid->Len != 5)
                        break;

                    // Turn CCX2 if matched
                    if (NdisEqualMemory(pEid->Octet, Ccx2IeInfo, 5) == 1)
                        pAd->StaCfg.CCXEnable = TRUE;
                    break;
                }
                break;

#if 0
            case IE_EDCA_PARAMETER:
                if (pEid->Len == 18)
                {
                    PUCHAR ptr;
                    int i;
                    pEdcaParm->bValid          = TRUE;
                    pEdcaParm->bQAck           = pEid->Octet[0] & 0x10;
                    pEdcaParm->bQueueRequest   = pEid->Octet[0] & 0x20;
                    pEdcaParm->bTxopRequest    = pEid->Octet[0] & 0x40;
//                  pEdcaParm->bMoreDataAck    = pEid->Octet[0] & 0x80;
                    pEdcaParm->EdcaUpdateCount = pEid->Octet[0] & 0x0f;
                    ptr = &pEid->Octet[2];
                    for (i=0; i<4; i++)
                    {
                        UCHAR aci = (*ptr & 0x60) >> 5; // b5~6 is AC INDEX
                        pEdcaParm->bACM[aci]  = (((*ptr) & 0x10) == 0x10);   // b5 is ACM
                        pEdcaParm->Aifsn[aci] = (*ptr) & 0x0f;               // b0~3 is AIFSN
                        pEdcaParm->Cwmin[aci] = *(ptr+1) & 0x0f;             // b0~4 is Cwmin
                        pEdcaParm->Cwmax[aci] = *(ptr+1) >> 4;               // b5~8 is Cwmax
                        pEdcaParm->Txop[aci]  = *(ptr+2) + 256 * (*(ptr+3)); // in unit of 32-us
                        ptr += 4; // point to next AC
                    }
                }
                break;
#endif
            default:
                DBGPRINT(RT_DEBUG_TRACE, ("PeerAssocRspSanity - ignore unrecognized EID = %d\n", pEid->Eid));
                break;
        }
                   
        Length = Length + 2 + pEid->Len; 
        pEid = (PEID_STRUCT)((UCHAR*)pEid + 2 + pEid->Len);        
    }

    // Force CCX2 enable to TRUE for those AP didn't replay CCX v2 IE, we still force it to be on
    if (pAd->StaCfg.CCXControl.field.Enable == 1)
        pAd->StaCfg.CCXEnable = TRUE;

    return TRUE;
}

#if 0 // move to cmm_sanity.c
/* 
    ==========================================================================
    Description:
        MLME message sanity check
    Return:
        TRUE if all parameters are OK, FALSE otherwise
        
	IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
BOOLEAN PeerDisassocSanity(
    IN PRTMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen, 
    OUT PUCHAR pAddr2, 
    OUT USHORT *pReason) 
{
    PFRAME_802_11 pFrame = (PFRAME_802_11)Msg;

    COPY_MAC_ADDR(pAddr2, pFrame->Hdr.Addr2);
    NdisMoveMemory(pReason, &pFrame->Octet[0], 2);

    return TRUE;
}

/* 
    ==========================================================================
    Description:
        MLME message sanity check
    Return:
        TRUE if all parameters are OK, FALSE otherwise
        
	IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
BOOLEAN PeerDeauthSanity(
    IN PRTMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen, 
    OUT PUCHAR pAddr2, 
    OUT USHORT *pReason) 
{
    PFRAME_802_11 pFrame = (PFRAME_802_11)Msg;

    COPY_MAC_ADDR(pAddr2, pFrame->Hdr.Addr2);
    NdisMoveMemory(pReason, &pFrame->Octet[0], 2);

    return TRUE;
}

/* 
    ==========================================================================
    Description:
        MLME message sanity check
    Return:
        TRUE if all parameters are OK, FALSE otherwise
        
	IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
BOOLEAN PeerAuthSanity(
    IN PRTMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen, 
    OUT PUCHAR pAddr, 
    OUT USHORT *pAlg, 
    OUT USHORT *pSeq, 
    OUT USHORT *pStatus, 
    CHAR *pChlgText) 
{
    PFRAME_802_11 pFrame = (PFRAME_802_11)Msg;

    COPY_MAC_ADDR(pAddr,   pFrame->Hdr.Addr2);
    NdisMoveMemory(pAlg,    &pFrame->Octet[0], 2);
    NdisMoveMemory(pSeq,    &pFrame->Octet[2], 2);
    NdisMoveMemory(pStatus, &pFrame->Octet[4], 2);

    if ((*pAlg == Ndis802_11AuthModeOpen)  || (*pAlg == CISCO_AuthModeLEAP))
    {
        if (*pSeq == 1 || *pSeq == 2) 
        {
            return TRUE;
        } 
        else 
        {
            DBGPRINT(RT_DEBUG_TRACE, ("PeerAuthSanity fail - wrong Seg#\n"));
            return FALSE;
        }
    } 
    else if (*pAlg == Ndis802_11AuthModeShared) 
    {
        if (*pSeq == 1 || *pSeq == 4) 
        {
            return TRUE;
        } 
        else if (*pSeq == 2 || *pSeq == 3) 
        {
            NdisMoveMemory(pChlgText, &pFrame->Octet[8], CIPHER_TEXT_LEN);
            return TRUE;
        } 
        else 
        {
            DBGPRINT(RT_DEBUG_TRACE, ("PeerAuthSanity fail - wrong Seg#\n"));
            return FALSE;
        }
    } 
    else 
    {
        DBGPRINT(RT_DEBUG_TRACE, ("PeerAuthSanity fail - wrong algorithm\n"));
        return FALSE;
    }
}
#endif // 0 //

/* 
    ==========================================================================
    Description:
        MLME message sanity check
    Return:
        TRUE if all parameters are OK, FALSE otherwise
        
	IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
BOOLEAN PeerProbeReqSanity(
    IN PRTMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen, 
    OUT PUCHAR pAddr2,
    OUT CHAR Ssid[], 
    OUT UCHAR *pSsidLen) 
{
    UCHAR         Idx;
    UCHAR	      RateLen;
    CHAR          IeType;
    PFRAME_802_11 pFrame = (PFRAME_802_11)Msg;

    COPY_MAC_ADDR(pAddr2, pFrame->Hdr.Addr2);

    if ((pFrame->Octet[0] != IE_SSID) || (pFrame->Octet[1] > MAX_LEN_OF_SSID)) 
    {
        DBGPRINT(RT_DEBUG_TRACE, ("PeerProbeReqSanity fail - wrong SSID IE(Type=%d,Len=%d)\n",pFrame->Octet[0],pFrame->Octet[1]));
        return FALSE;
    } 
    
    *pSsidLen = pFrame->Octet[1];
    NdisMoveMemory(Ssid, &pFrame->Octet[2], *pSsidLen);

    Idx = *pSsidLen + 2;

    // -- get supported rates from payload and advance the pointer
    IeType = pFrame->Octet[Idx];
    RateLen = pFrame->Octet[Idx + 1];
    if (IeType != IE_SUPP_RATES) 
    {
        DBGPRINT(RT_DEBUG_TRACE, ("PeerProbeReqSanity fail - wrong SupportRates IE(Type=%d,Len=%d)\n",pFrame->Octet[Idx],pFrame->Octet[Idx+1]));
        return FALSE;
    }
    else
    {
        if ((pAd->StaCfg.AdhocMode == 2) && (RateLen < 8))
            return (FALSE);
    }

    return TRUE;
}

#if 0 // move to cmm_sanity.c
/* 
    ==========================================================================
    Description:
        MLME message sanity check
    Return:
        TRUE if all parameters are OK, FALSE otherwise
        
	IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
BOOLEAN PeerBeaconAndProbeRspSanity(
    IN PRTMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen, 
    OUT PUCHAR pAddr2, 
    OUT PUCHAR pBssid, 
    OUT CHAR Ssid[], 
    OUT UCHAR *pSsidLen, 
    OUT UCHAR *pBssType, 
    OUT USHORT *pBeaconPeriod, 
    OUT UCHAR *pChannel, 
    OUT UCHAR *pNewChannel, 
    OUT LARGE_INTEGER *pTimestamp, 
    OUT CF_PARM *pCfParm, 
    OUT USHORT *pAtimWin, 
    OUT USHORT *pCapabilityInfo, 
    OUT UCHAR *pErp,
    OUT UCHAR *pDtimCount, 
    OUT UCHAR *pDtimPeriod, 
    OUT UCHAR *pBcastFlag, 
    OUT UCHAR *pMessageToMe, 
    OUT UCHAR SupRate[],
    OUT UCHAR *pSupRateLen,
    OUT UCHAR ExtRate[],
    OUT UCHAR *pExtRateLen,
    OUT	UCHAR *pCkipFlag,
    OUT	UCHAR *pAironetCellPowerLimit,
    OUT PEDCA_PARM       pEdcaParm,
    OUT PQBSS_LOAD_PARM  pQbssLoad,
    OUT PQOS_CAPABILITY_PARM pQosCapability,
    OUT ULONG *pRalinkIe,
    OUT UCHAR		 *pHtCapabilityLen,
    OUT UCHAR		 *pPreNHtCapabilityLen,
    OUT HT_CAPABILITY_IE *pHtCapability,
	OUT UCHAR		 *AddHtInfoLen,
	OUT ADD_HT_INFO_IE *AddHtInfo,
	OUT UCHAR *NewExtChannelOffset,		// Ht extension channel offset(above or below)
    OUT UCHAR *LengthVIE,	
    OUT	PNDIS_802_11_VARIABLE_IEs pVIE) 
{
    CHAR				*Ptr, TimLen;
    PFRAME_802_11		pFrame;
    PEID_STRUCT         pEid;
    UCHAR				SubType;
    UCHAR				Sanity;
    ULONG				Length = 0;

    // Add for 3 necessary EID field check
    Sanity = 0;

    *pAtimWin = 0;
    *pErp = 0;	
    *pDtimCount = 0;
    *pDtimPeriod = 0;
    *pBcastFlag = 0;
    *pMessageToMe = 0;
    *pExtRateLen = 0;
    *pCkipFlag = 0;			        // Default of CkipFlag is 0
    *pAironetCellPowerLimit = 0xFF;  // Default of AironetCellPowerLimit is 0xFF
    *LengthVIE = 0;					// Set the length of VIE to init value 0
    *pHtCapabilityLen = 0;					// Set the length of VIE to init value 0
    *pPreNHtCapabilityLen = 0;					// Set the length of VIE to init value 0
    *AddHtInfoLen = 0;					// Set the length of VIE to init value 0
    *pRalinkIe = 0;
    *pNewChannel = 0;
    *NewExtChannelOffset = 0xff;	//Default 0xff means no such IE
    pCfParm->bValid = FALSE;        // default: no IE_CF found
    pQbssLoad->bValid = FALSE;      // default: no IE_QBSS_LOAD found
    pEdcaParm->bValid = FALSE;      // default: no IE_EDCA_PARAMETER found
    pQosCapability->bValid = FALSE; // default: no IE_QOS_CAPABILITY found
    
    pFrame = (PFRAME_802_11)Msg;
    
    // get subtype from header
    SubType = (UCHAR)pFrame->Hdr.FC.SubType;

    // get Addr2 and BSSID from header
    COPY_MAC_ADDR(pAddr2, pFrame->Hdr.Addr2);
    COPY_MAC_ADDR(pBssid, pFrame->Hdr.Addr3);
    
//	hex_dump("Beacon", Msg, MsgLen);

    Ptr = pFrame->Octet;
    Length += LENGTH_802_11;
    
    // get timestamp from payload and advance the pointer
    NdisMoveMemory(pTimestamp, Ptr, TIMESTAMP_LEN);
    Ptr += TIMESTAMP_LEN;
    Length += TIMESTAMP_LEN;

    // get beacon interval from payload and advance the pointer
    NdisMoveMemory(pBeaconPeriod, Ptr, 2);
    Ptr += 2;
    Length += 2;

    // get capability info from payload and advance the pointer
    NdisMoveMemory(pCapabilityInfo, Ptr, 2);
    Ptr += 2;
    Length += 2;

    if (CAP_IS_ESS_ON(*pCapabilityInfo)) 
        *pBssType = BSS_INFRA;
    else 
        *pBssType = BSS_ADHOC;

    pEid = (PEID_STRUCT) Ptr;

    // get variable fields from payload and advance the pointer
    while ((Length + 2 + pEid->Len) <= MsgLen)    
    {
        //
        // Secure copy VIE to VarIE[MAX_VIE_LEN] didn't overflow.
        //
        if ((*LengthVIE + pEid->Len + 2) >= MAX_VIE_LEN)
        {
            DBGPRINT(RT_DEBUG_WARN, ("PeerBeaconAndProbeRspSanity - Variable IEs out of resource [len(=%d) > MAX_VIE_LEN(=%d)]\n",
                    (*LengthVIE + pEid->Len + 2), MAX_VIE_LEN));
            break;
        }

        switch(pEid->Eid)
        {
            case IE_SSID:
                // Already has one SSID EID in this beacon, ignore the second one
                if (Sanity & 0x1)
                    break;
                if(pEid->Len <= MAX_LEN_OF_SSID)
                {
                    NdisMoveMemory(Ssid, pEid->Octet, pEid->Len);
                    *pSsidLen = pEid->Len;
                    Sanity |= 0x1;
                }
                else
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("PeerBeaconAndProbeRspSanity - wrong IE_SSID (len=%d)\n",pEid->Len));
                    return FALSE;
                }
                break;

            case IE_SUPP_RATES:
                if(pEid->Len <= MAX_LEN_OF_SUPPORTED_RATES)
                {
                    Sanity |= 0x2;
                    NdisMoveMemory(SupRate, pEid->Octet, pEid->Len);
                    *pSupRateLen = pEid->Len;

                    // TODO: 2004-09-14 not a good design here, cause it exclude extra rates 
                    // from ScanTab. We should report as is. And filter out unsupported
                    // rates in MlmeAux.
                    // Check against the supported rates
                    // RTMPCheckRates(pAd, SupRate, pSupRateLen);
                }
                else
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("PeerBeaconAndProbeRspSanity - wrong IE_SUPP_RATES (len=%d)\n",pEid->Len));
                    return FALSE;
                }
                break;

            case IE_HT_CAP:
			if (pEid->Len >= SIZE_HT_CAP_IE)  //Note: allow extension.!!
			{
				NdisMoveMemory(pHtCapability, pEid->Octet, sizeof(HT_CAPABILITY_IE));
				*pHtCapabilityLen = SIZE_HT_CAP_IE;	// Nnow we only support 26 bytes.
				*pPreNHtCapabilityLen = 0;	// Nnow we only support 26 bytes.

	                Ptr = (PUCHAR) pVIE;
	                NdisMoveMemory(Ptr + *LengthVIE, &pEid->Eid, pEid->Len + 2);
	                *LengthVIE += (pEid->Len + 2);
			}
			else
			{
				DBGPRINT(RT_DEBUG_WARN, ("PeerBeaconAndProbeRspSanity - wrong IE_HT_CAP. pEid->Len = %d\n", pEid->Len));
			}
				
		break;
            case IE_ADD_HT:
			if (pEid->Len >= sizeof(ADD_HT_INFO_IE))				
			{
				// This IE allows extension, but we can ignore extra bytes beyond our knowledge , so only
				// copy first sizeof(ADD_HT_INFO_IE)
				NdisMoveMemory(AddHtInfo, pEid->Octet, sizeof(ADD_HT_INFO_IE));
				*AddHtInfoLen = SIZE_ADD_HT_INFO_IE;

	                Ptr = (PUCHAR) pVIE;
	                NdisMoveMemory(Ptr + *LengthVIE, &pEid->Eid, pEid->Len + 2);
	                *LengthVIE += (pEid->Len + 2);
			}
			else
			{
				DBGPRINT(RT_DEBUG_WARN, ("PeerBeaconAndProbeRspSanity - wrong IE_ADD_HT. \n"));
			}
				
		break;
            case IE_NEW_EXT_CHA_OFFSET:
			if (pEid->Len == 1)
			{
				*NewExtChannelOffset = pEid->Octet[0];
			}
			else
			{
				DBGPRINT(RT_DEBUG_WARN, ("PeerBeaconAndProbeRspSanity - wrong IE_NEW_EXT_CHA_OFFSET. \n"));
			}
				
		break;
            case IE_FH_PARM:
                DBGPRINT(RT_DEBUG_TRACE, ("PeerBeaconAndProbeRspSanity(IE_FH_PARM) \n"));
                break;

            case IE_DS_PARM:
                if(pEid->Len == 1)
                {
                    *pChannel = *pEid->Octet;                    
                    if (ChannelSanity(pAd, *pChannel) == 0)
                    {
                        DBGPRINT(RT_DEBUG_INFO, ("PeerBeaconAndProbeRspSanity - wrong IE_DS_PARM (ch=%d)\n",*pChannel));
                        return FALSE;
                    }
                    Sanity |= 0x4;
                }
                else
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("PeerBeaconAndProbeRspSanity - wrong IE_DS_PARM (len=%d)\n",pEid->Len));
                    return FALSE;
                }
                break;

            case IE_CF_PARM:
                if(pEid->Len == 6)
                {
                    pCfParm->bValid = TRUE;
                    pCfParm->CfpCount = pEid->Octet[0];
                    pCfParm->CfpPeriod = pEid->Octet[1];
                    pCfParm->CfpMaxDuration = pEid->Octet[2] + 256 * pEid->Octet[3];
                    pCfParm->CfpDurRemaining = pEid->Octet[4] + 256 * pEid->Octet[5];
                }
                else
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("PeerBeaconAndProbeRspSanity - wrong IE_CF_PARM\n"));
                    return FALSE;
                }
                break;

            case IE_IBSS_PARM:
                if(pEid->Len == 2)
                {
                    NdisMoveMemory(pAtimWin, pEid->Octet, pEid->Len);
                }
                else
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("PeerBeaconAndProbeRspSanity - wrong IE_IBSS_PARM\n"));
                    return FALSE;
                }
                break;

            case IE_TIM:
                if(INFRA_ON(pAd) && SubType == SUBTYPE_BEACON)
                {
                    GetTimBit((PUCHAR)pEid, pAd->StaActive.Aid, &TimLen, pBcastFlag, pDtimCount, pDtimPeriod, pMessageToMe);
                }
                break;

            case IE_CHANNEL_SWITCH_ANNOUNCEMENT:
                if(pEid->Len == 3)
                {
                	*pNewChannel = pEid->Octet[1];	//extract new channel number
                }
                break;

            // New for WPA
            // CCX v2 has the same IE, we need to parse that too
            // Wifi WMM use the same IE vale, need to parse that too
            // case IE_WPA:
            case IE_VENDOR_SPECIFIC:
                // Check the OUI version, filter out non-standard usage
                
                // Ralink proprietary IE for proprietary functionality
                if (NdisEqualMemory(pEid->Octet, RALINK_OUI, 3) && (pEid->Len == 7))
                {
                    *pRalinkIe = pEid->Octet[3];
                }
                // Check pre-N OUI for HT related IE, including HT Capatilities Info and HT extended Capabilities
                else if ((*pHtCapabilityLen == 0) && NdisEqualMemory(pEid->Octet, PRE_N_HT_OUI, 3) && (pEid->Len >= 4))
                {
                	// HT Capatilities Info
                    if ((pEid->Octet[3] == OUI_PREN_HT_CAP) && (pEid->Len >= 30))
                    {
                        NdisMoveMemory(pHtCapability, &pEid->Octet[4], sizeof(HT_CAPABILITY_IE));
                        *pPreNHtCapabilityLen = SIZE_HT_CAP_IE;
                    }

					// HT extended Capabilities
                    if ((pEid->Octet[3] == OUI_PREN_ADD_HT) && (pEid->Len >= 26))
                    {
                        NdisMoveMemory(AddHtInfo, &pEid->Octet[4], sizeof(ADD_HT_INFO_IE));
                        *AddHtInfoLen = SIZE_ADD_HT_INFO_IE;
                    }
                }
                else if (NdisEqualMemory(pEid->Octet, WPA_OUI, 4))
                {
                    // Copy to pVIE which will report to microsoft bssid list.
                    Ptr = (PUCHAR) pVIE;
                    NdisMoveMemory(Ptr + *LengthVIE, &pEid->Eid, pEid->Len + 2);
                    *LengthVIE += (pEid->Len + 2);
                }
                else if (NdisEqualMemory(pEid->Octet, WME_PARM_ELEM, 6) && (pEid->Len == 24))
                {
                    PUCHAR ptr;
                    int i;

                    // parsing EDCA parameters
                    pEdcaParm->bValid          = TRUE;
                    pEdcaParm->bQAck           = FALSE; // pEid->Octet[0] & 0x10;
                    pEdcaParm->bQueueRequest   = FALSE; // pEid->Octet[0] & 0x20;
                    pEdcaParm->bTxopRequest    = FALSE; // pEid->Octet[0] & 0x40;
                    pEdcaParm->EdcaUpdateCount = pEid->Octet[6] & 0x0f;
                    pEdcaParm->bAPSDCapable    = (pEid->Octet[6] & 0x80) ? 1 : 0;
                    ptr = &pEid->Octet[8];
                    for (i=0; i<4; i++)
                    {
                        UCHAR aci = (*ptr & 0x60) >> 5; // b5~6 is AC INDEX
                        pEdcaParm->bACM[aci]  = (((*ptr) & 0x10) == 0x10);   // b5 is ACM
                        pEdcaParm->Aifsn[aci] = (*ptr) & 0x0f;               // b0~3 is AIFSN
                        pEdcaParm->Cwmin[aci] = *(ptr+1) & 0x0f;             // b0~4 is Cwmin
                        pEdcaParm->Cwmax[aci] = *(ptr+1) >> 4;               // b5~8 is Cwmax
                        pEdcaParm->Txop[aci]  = *(ptr+2) + 256 * (*(ptr+3)); // in unit of 32-us
                        ptr += 4; // point to next AC
                    }
                }
                else if (NdisEqualMemory(pEid->Octet, WME_INFO_ELEM, 6) && (pEid->Len == 7))
                {
                    // parsing EDCA parameters
                    pEdcaParm->bValid          = TRUE;
                    pEdcaParm->bQAck           = FALSE; // pEid->Octet[0] & 0x10;
                    pEdcaParm->bQueueRequest   = FALSE; // pEid->Octet[0] & 0x20;
                    pEdcaParm->bTxopRequest    = FALSE; // pEid->Octet[0] & 0x40;
                    pEdcaParm->EdcaUpdateCount = pEid->Octet[6] & 0x0f;
                    pEdcaParm->bAPSDCapable    = (pEid->Octet[6] & 0x80) ? 1 : 0;

                    // use default EDCA parameter
                    pEdcaParm->bACM[QID_AC_BE]  = 0;
                    pEdcaParm->Aifsn[QID_AC_BE] = 3;
                    pEdcaParm->Cwmin[QID_AC_BE] = CW_MIN_IN_BITS;
                    pEdcaParm->Cwmax[QID_AC_BE] = CW_MAX_IN_BITS;
                    pEdcaParm->Txop[QID_AC_BE]  = 0;

                    pEdcaParm->bACM[QID_AC_BK]  = 0;
                    pEdcaParm->Aifsn[QID_AC_BK] = 7;
                    pEdcaParm->Cwmin[QID_AC_BK] = CW_MIN_IN_BITS;
                    pEdcaParm->Cwmax[QID_AC_BK] = CW_MAX_IN_BITS;
                    pEdcaParm->Txop[QID_AC_BK]  = 0;

                    pEdcaParm->bACM[QID_AC_VI]  = 0;
                    pEdcaParm->Aifsn[QID_AC_VI] = 2;
                    pEdcaParm->Cwmin[QID_AC_VI] = CW_MIN_IN_BITS-1;
                    pEdcaParm->Cwmax[QID_AC_VI] = CW_MAX_IN_BITS;
                    pEdcaParm->Txop[QID_AC_VI]  = 96;   // AC_VI: 96*32us ~= 3ms

                    pEdcaParm->bACM[QID_AC_VO]  = 0;
                    pEdcaParm->Aifsn[QID_AC_VO] = 2;
                    pEdcaParm->Cwmin[QID_AC_VO] = CW_MIN_IN_BITS-2;
                    pEdcaParm->Cwmax[QID_AC_VO] = CW_MAX_IN_BITS-1;
                    pEdcaParm->Txop[QID_AC_VO]  = 48;   // AC_VO: 48*32us ~= 1.5ms
                }
                else	
                {
                    // Gemtek ask us to pass other vendor's IE for their applications
                    Ptr = (PUCHAR) pVIE;
                    NdisMoveMemory(Ptr + *LengthVIE, &pEid->Eid, pEid->Len + 2);
                    *LengthVIE += (pEid->Len + 2);
                }

#if 0
// QOS obselete in CCX2 TestPlan ver3.54
                if ((pAd->StaCfg.CCXControl.field.QOSEnable) && (NdisEqualMemory(pEid->Octet, Ccx2QosInfo, 4)))
                {
                    // Filter out other BSSID QOS info
                    if (! MAC_ADDR_EQUAL(pBssid, pAd->CommonCfg.Bssid))
                        break;

                    // Parse CCX 2 QOS information
                    ECWMax = (pEid->Octet[7] & 0xf0) >> 4;
                    ECWMin = (pEid->Octet[7] & 0x0f);
                    // Only enabled when CCX2 admin state is on,
                    if (pAd->StaCfg.CCXControl.field.Enable)
                    {
                        if ((pAd->StaCfg.CCXQosECWMin != ECWMin) || (pAd->StaCfg.CCXQosECWMax != ECWMax))
                        {
                            // Change the QOS ECW setting
                            pAd->StaCfg.CCXQosECWMin = ECWMin;
                            pAd->StaCfg.CCXQosECWMax = ECWMax;

                            // Read the current value of MAC_CSR9
                            RTMP_IO_READ32(pAd, MAC_CSR9, &Csr9.word);
                            DBGPRINT(RT_DEBUG_TRACE, ("QOS setting before QOS IE 0x%08x\n", Csr9.word));

                            // Update CWMin, CwMax, and change the CW control from Tx descriptor to register
                            Csr9.field.CWMin = ECWMin;
                            Csr9.field.CWMax = ECWMax;
                            Csr9.field.CWSelect = 1;
                            DBGPRINT(RT_DEBUG_TRACE, ("QOS setting after QOS IE 0x%08x\n", Csr9.word));

                            // Write the data
                            RTMP_IO_WRITE32(pAd, MAC_CSR9, Csr9.word);
                        }
                    }
                }
#endif
                DBGPRINT(RT_DEBUG_INFO, ("PeerBeaconAndProbeRspSanity - Receive IE_WPA\n"));
                break;

            case IE_EXT_SUPP_RATES:
                if (pEid->Len <= MAX_LEN_OF_SUPPORTED_RATES)
                {
                    NdisMoveMemory(ExtRate, pEid->Octet, pEid->Len);
                    *pExtRateLen = pEid->Len;

                    // TODO: 2004-09-14 not a good design here, cause it exclude extra rates 
                    // from ScanTab. We should report as is. And filter out unsupported
                    // rates in MlmeAux.
                    // Check against the supported rates
                    // RTMPCheckRates(pAd, ExtRate, pExtRateLen);
                }
                break;

            case IE_ERP:
                if (pEid->Len == 1)
                {
                    *pErp = (UCHAR)pEid->Octet[0];
                }
                break;

            case IE_AIRONET_CKIP:
                // 0. Check Aironet IE length, it must be larger or equal to 28
                // Cisco AP350 used length as 28
                // Cisco AP12XX used length as 30
                if (pEid->Len < (CKIP_NEGOTIATION_LENGTH - 2))
                    break;

                // 1. Copy CKIP flag byte to buffer for process
                *pCkipFlag = *(pEid->Octet + 8);				
                break;

            case IE_AP_TX_POWER:
                // AP Control of Client Transmit Power
                //0. Check Aironet IE length, it must be 6
                if (pEid->Len != 0x06)
                    break;

                // Get cell power limit in dBm
                if (NdisEqualMemory(pEid->Octet, CISCO_OUI, 3) == 1)
                    *pAironetCellPowerLimit = *(pEid->Octet + 4);	
                break;

            // WPA2 & 802.11i RSN
            case IE_RSN:
                // There is no OUI for version anymore, check the group cipher OUI before copying
                if (RTMPEqualMemory(pEid->Octet + 2, RSN_OUI, 3))
                {
                    // Copy to pVIE which will report to microsoft bssid list.
                    Ptr = (PUCHAR) pVIE;
                    NdisMoveMemory(Ptr + *LengthVIE, &pEid->Eid, pEid->Len + 2);
                    *LengthVIE += (pEid->Len + 2);
                }
                DBGPRINT(RT_DEBUG_INFO, ("IE_RSN length = %d\n", pEid->Len));
                break;
#if 0
            case IE_QBSS_LOAD:
                if (pEid->Len == 5)
                {
                    pQbssLoad->bValid = TRUE;
                    pQbssLoad->StaNum = pEid->Octet[0] + pEid->Octet[1] * 256;
                    pQbssLoad->ChannelUtilization = pEid->Octet[2];
                    pQbssLoad->RemainingAdmissionControl = pEid->Octet[3] + pEid->Octet[4] * 256;
                }
                break;
                
            case IE_EDCA_PARAMETER:
                if (pEid->Len == 18)
                {
                    PUCHAR ptr;
                    int i;
                    pEdcaParm->bValid          = TRUE;
                    pEdcaParm->bQAck           = pEid->Octet[0] & 0x10;
                    pEdcaParm->bQueueRequest   = pEid->Octet[0] & 0x20;
                    pEdcaParm->bTxopRequest    = pEid->Octet[0] & 0x40;
//                  pEdcaParm->bMoreDataAck    = pEid->Octet[0] & 0x80;
                    pEdcaParm->EdcaUpdateCount = pEid->Octet[0] & 0x0f;
                    ptr = &pEid->Octet[2];
                    for (i=0; i<4; i++)
                    {
                        UCHAR aci = (*ptr & 0x60) >> 5; // b5~6 is AC INDEX
                        pEdcaParm->bACM[aci]  = (((*ptr) & 0x10) == 0x10);   // b5 is ACM
                        pEdcaParm->Aifsn[aci] = (*ptr) & 0x0f;               // b0~3 is AIFSN
                        pEdcaParm->Cwmin[aci] = *(ptr+1) & 0x0f;             // b0~4 is Cwmin
                        pEdcaParm->Cwmax[aci] = *(ptr+1) >> 4;               // b5~8 is Cwmax
                        pEdcaParm->Txop[aci]  = *(ptr+2) + 256 * (*(ptr+3)); // in unit of 32-us
                        ptr += 4; // point to next AC
                    }
                }
                break;

            case IE_QOS_CAPABILITY:
                // this IE contains redundant information as stated in EDCA_IE
                if (pEid->Len == 1)
                {
                    pQosCapability->bValid          = TRUE;
                    pQosCapability->bQAck           = pEid->Octet[0] & 0x01;
                    pQosCapability->bQueueRequest   = pEid->Octet[0] & 0x02;
                    pQosCapability->bTxopRequest    = pEid->Octet[0] & 0x04;
//                  pQosCapability->bMoreDataAck    = pEid->Octet[0] & 0x08;
                    pQosCapability->EdcaUpdateCount = pEid->Octet[0] >> 4;
                }
                break;
#endif                
            default:
                // Unknown IE, we have to pass it as variable IEs
                Ptr = (PUCHAR) pVIE;
                NdisMoveMemory(Ptr + *LengthVIE, &pEid->Eid, pEid->Len + 2);
                *LengthVIE += (pEid->Len + 2);            
                DBGPRINT(RT_DEBUG_INFO, ("PeerBeaconAndProbeRspSanity - unrecognized EID = %d\n", pEid->Eid));
                break;
        }
        
        Length = Length + 2 + pEid->Len;  // Eid[1] + Len[1]+ content[Len]
        pEid = (PEID_STRUCT)((UCHAR*)pEid + 2 + pEid->Len);        
    }

    // For some 11a AP. it did not have the channel EID, patch here
    if (pAd->LatchRfRegs.Channel > 14)
    {
        *pChannel = pAd->LatchRfRegs.Channel;					 
        Sanity |= 0x4;		
    }

    if (Sanity != 0x7)
    {
        DBGPRINT(RT_DEBUG_WARN, ("PeerBeaconAndProbeRspSanity - missing field, Sanity=0x%02x\n", Sanity));
        return FALSE;
    }
    else
    {
        return TRUE;
    }

}

BOOLEAN PeerAddBAReqActionSanity(
    IN PRTMP_ADAPTER pAd, 
    IN VOID *pMsg, 
    IN ULONG MsgLen,
	OUT PUCHAR pAddr2)
{
	PFRAME_802_11 pFrame = (PFRAME_802_11)pMsg;
	PFRAME_ADDBA_REQ pAddFrame;
	
	pAddFrame = (PFRAME_ADDBA_REQ)(pMsg);
	if (MsgLen < (sizeof(FRAME_ADDBA_REQ)))
	{
		DBGPRINT(RT_DEBUG_ERROR,("PeerAddBAReqActionSanity: ADDBA Request frame length size = %d incorrect\n", MsgLen));
		return FALSE;
	}
	// we support immediate BA.
	if (pAddFrame->BaParm.BAPolicy != IMMED_BA)
	{
		DBGPRINT(RT_DEBUG_ERROR,("PeerAddBAReqActionSanity: ADDBA Request Ba Policy[%d] not support\n", pAddFrame->BaParm.BAPolicy));
		DBGPRINT(RT_DEBUG_ERROR,("ADDBA Request. tid=%x, Bufsize=%x, rsv=%x \n", pAddFrame->BaParm.TID, pAddFrame->BaParm.BufSize, pAddFrame->BaParm.Rsv));
		return FALSE;
	}

	if (pAddFrame->BaParm.TID &0xfff0)
	{
		DBGPRINT(RT_DEBUG_ERROR,("PeerAddBAReqActionSanity: ADDBA Request incorrect TID = %d\n", pAddFrame->BaParm.TID));
		return FALSE;
	}
	COPY_MAC_ADDR(pAddr2, pFrame->Hdr.Addr2);
	
	return TRUE;
}

BOOLEAN PeerAddBARspActionSanity(
    IN PRTMP_ADAPTER pAd, 
    IN VOID *pMsg, 
    IN ULONG MsgLen)
{
	PFRAME_802_11 pFrame = (PFRAME_802_11)pMsg;
	PFRAME_ADDBA_RSP pAddFrame;
	
	pAddFrame = (PFRAME_ADDBA_RSP)(pMsg);
	if (MsgLen < (sizeof(FRAME_ADDBA_RSP)))
	{
		DBGPRINT(RT_DEBUG_ERROR,("PeerAddBARspActionSanity: ADDBA Response frame length size = %d incorrect\n", MsgLen));
		return FALSE;
	}
	
	// we support immediate BA.
	if (pAddFrame->BaParm.BAPolicy != IMMED_BA)
	{
		DBGPRINT(RT_DEBUG_ERROR,("PeerAddBARspActionSanity: ADDBA Response Ba Policy[%d] not support\n", pAddFrame->BaParm.BAPolicy));
		return FALSE;
	}

	// we support immediate BA.
	if (pAddFrame->BaParm.TID &0xfff0)
	{
		DBGPRINT(RT_DEBUG_ERROR,("PeerAddBARspActionSanity: ADDBA Response incorrect TID = %d\n", pAddFrame->BaParm.TID));
		return FALSE;
	}
	return TRUE;

}

BOOLEAN PeerDelBAActionSanity(
    IN PRTMP_ADAPTER pAd, 
    IN UCHAR Wcid, 
    IN VOID *pMsg, 
    IN ULONG MsgLen )
{
	PFRAME_802_11 pFrame = (PFRAME_802_11)pMsg;
	PFRAME_DELBA_REQ  pDelFrame;
	if (MsgLen != (sizeof(FRAME_DELBA_REQ)))
		return FALSE;
	
	if (Wcid >= MAX_LEN_OF_MAC_TABLE)
		return FALSE;
	
	pDelFrame = (PFRAME_DELBA_REQ)(pMsg);
	if (pDelFrame->DelbaParm.TID &0xfff0)
		return FALSE;
	
	return TRUE;
}
#endif // move to cmm_sanity.c

/* 
    ==========================================================================
    Description:
        
	IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
BOOLEAN GetTimBit(
    IN CHAR *Ptr, 
    IN USHORT Aid, 
    OUT UCHAR *TimLen, 
    OUT UCHAR *BcastFlag, 
    OUT UCHAR *DtimCount, 
    OUT UCHAR *DtimPeriod,
    OUT UCHAR *MessageToMe) 
{
    UCHAR          BitCntl, N1, N2, MyByte, MyBit;
    CHAR          *IdxPtr;

    IdxPtr = Ptr;
    
    IdxPtr ++;
    *TimLen = *IdxPtr;
    
    // get DTIM Count from TIM element
    IdxPtr ++;
    *DtimCount = *IdxPtr;
    
    // get DTIM Period from TIM element
    IdxPtr++;
    *DtimPeriod = *IdxPtr;
        
    // get Bitmap Control from TIM element
    IdxPtr++;
    BitCntl = *IdxPtr;

    if ((*DtimCount == 0) && (BitCntl & 0x01)) 
        *BcastFlag = TRUE;
    else 
        *BcastFlag = FALSE;
    
    // Parse Partial Virtual Bitmap from TIM element
    N1 = BitCntl & 0xfe;    // N1 is the first bitmap byte#
    N2 = *TimLen - 4 + N1;  // N2 is the last bitmap byte#
    
    if ((Aid < (N1 << 3)) || (Aid >= ((N2 + 1) << 3)))
        *MessageToMe = FALSE;
    else
    {
        MyByte = (Aid >> 3) - N1;                       // my byte position in the bitmap byte-stream
        MyBit = Aid % 16 - ((MyByte & 0x01)? 8:0);

        IdxPtr += (MyByte + 1);

        //if (*IdxPtr)
        //    DBGPRINT(RT_DEBUG_WARN, ("TIM bitmap = 0x%02x\n", *IdxPtr));
            
        if (*IdxPtr & (0x01 << MyBit))
            *MessageToMe = TRUE;
        else 
            *MessageToMe = FALSE;
    }

    return TRUE;
}

#if 0 // move to cmm_sanity.c
// IRQL = DISPATCH_LEVEL
UCHAR ChannelSanity(
    IN PRTMP_ADAPTER pAd, 
    IN UCHAR channel)
{
    int i;

    for (i = 0; i < pAd->ChannelListNum; i ++)
    {
        if (channel == pAd->ChannelList[i].Channel)
            return 1;
    }
    return 0;
}
#endif

#if 0 	//  It is omitted by AlbertY
/*
	========================================================================
	Routine Description:
		Sanity check pairwise key on Encryption::Ndis802_11Encryption1Enabled
		
	Arguments:
		pAdapter - Pointer to our adapter
		pBuf 	 - Pointer to NDIS_802_11_KEY structure

	Return Value:
		NDIS_STATUS_SUCCESS
		NDIS_STATUS_INVALID_DATA
        
	IRQL = DISPATCH_LEVEL

	Note:
		For OID_802_11_ADD_KEY setting, on old wep stuff also need to verify
		the structure of NIDS_802_11_KEY
	========================================================================
*/
NDIS_STATUS	RTMPWepKeySanity(
	IN	PRTMP_ADAPTER	pAd,
	IN	PVOID			pBuf)
{
	PNDIS_802_11_KEY	pKey;
	UCHAR				KeyIdx;
	NDIS_STATUS			Status = NDIS_STATUS_SUCCESS;
	BOOLEAN 			bTxKey; 		// Set the key as transmit key
	BOOLEAN 			bPairwise;		// Indicate the key is pairwise key
	UCHAR               CipherAlg;

        DBGPRINT(RT_DEBUG_TRACE, ("RTMPWepKeySanity  \n"));
	pKey = (PNDIS_802_11_KEY) pBuf;
	KeyIdx = (UCHAR)pKey->KeyIndex & 0xff;
	// Bit 31 of Add-key, Tx Key
	bTxKey		   = (pKey->KeyIndex & 0x80000000) ? TRUE : FALSE;
	// Bit 30 of Add-key PairwiseKey
	bPairwise	   = (pKey->KeyIndex & 0x40000000) ? TRUE : FALSE;

	// 1. Check Group / Pairwise Key
	if (bPairwise)	// Pairwise Key
	{
		// 1. Check KeyIdx 
		// it is a shared key
		if (KeyIdx > 4)
			return (NDIS_STATUS_INVALID_DATA);

		// 2. Check bTx, it must be true, otherwise, return NDIS_STATUS_INVALID_DATA
		if (bTxKey == FALSE)
			return(NDIS_STATUS_INVALID_DATA);

		// 3. If BSSID is all 0xff, return NDIS_STATUS_INVALID_DATA
		if (MAC_ADDR_EQUAL(pKey->BSSID, BROADCAST_ADDR))
			return(NDIS_STATUS_INVALID_DATA);

		// check key length
		if ((pKey->KeyLength != 5) && (pKey->KeyLength != 13))
			return(NDIS_STATUS_INVALID_DATA);
	}
	else
	{
		// Group Key
		// 1. Check BSSID, if not current BSSID or Bcast, return NDIS_STATUS_INVALID_DATA
		if ((! MAC_ADDR_EQUAL(pKey->BSSID, BROADCAST_ADDR)) &&
			(! MAC_ADDR_EQUAL(pKey->BSSID, pAd->CommonCfg.Bssid)))
			return(NDIS_STATUS_INVALID_DATA);

		// 2. Check Key index for supported Group Key
		if (KeyIdx > 4)
			return(NDIS_STATUS_INVALID_DATA);
	}
     
	pAd->SharedKey[BSS0][KeyIdx].KeyLen = (UCHAR) pKey->KeyLength;
	NdisMoveMemory(pAd->SharedKey[BSS0][KeyIdx].Key, &pKey->KeyMaterial, pKey->KeyLength);

	if (pKey->KeyLength == 5)
		CipherAlg = CIPHER_WEP64;
	else
		CipherAlg = CIPHER_WEP128;

	if (pAd->StaCfg.bCkipOn && (pAd->StaCfg.CkipFlag & 0x10)) // CKIP KP is required
	{
		if (CipherAlg == CIPHER_WEP64)  
			CipherAlg = CIPHER_CKIP64;
		else
			CipherAlg = CIPHER_CKIP128;
	}

	// always expand the KEY to 16-byte here for efficiency sake. so that in case CKIP is used
	// sometime later we don't have to do key expansion for each TX in RTMPHardTransmit().
	// However, we shouldn't change pAd->SharedKey[BSS0][KeyIdx].KeyLen
	if (pKey->KeyLength < 16)
	{
		for(i = 1; i < (16 / pKey->KeyLength); i++)
		{
			NdisMoveMemory(&pAd->SharedKey[BSS0][KeyIdx].Key[i * pKey->KeyLength], 
						&pKey->KeyMaterial[0], 
						pKey->KeyLength);
		}
		NdisMoveMemory(&pAd->SharedKey[BSS0][KeyIdx].Key[i * pKey->KeyLength], 
						&pKey->KeyMaterial[0], 
						16 - (i * pKey->KeyLength));
	}

	pAd->SharedKey[BSS0][KeyIdx].CipherAlg = CipherAlg;
	if (pKey->KeyIndex & 0x80000000)
	{
		// Default key for tx (shared key)
		pAd->StaCfg.DefaultKeyId = (UCHAR) KeyIdx;
		
		// Add BSSID to WCTable. because this is Tx wep key.
		RTMPAddBSSIDCipher(pAd, BSSID_WCID, pKey, pAd->SharedKey[BSS0][KeyIdx].CipherAlg);
	}

	DBGPRINT(RT_DEBUG_TRACE, ("RTMPWepKeySanity==> CipherAlg = %d \n", CipherAlg));
	AsicAddSharedKeyEntry(pAd, BSS0, (UCHAR)KeyIdx, CipherAlg, pAd->SharedKey[BSS0][KeyIdx].Key, NULL, NULL);

        DBGPRINT(RT_DEBUG_TRACE, ("RTMPWepKeySanity<==  \n"));
	return (Status);
}
#endif

#if 0 // move to cm_sanity.
/* 
    ==========================================================================
    Description:
        MLME message sanity check
    Return:
        TRUE if all parameters are OK, FALSE otherwise
        
	IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
BOOLEAN MlmeAddBAReqSanity(
    IN PRTMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen, 
    OUT PUCHAR pAddr2) 
{
    PMLME_ADDBA_REQ_STRUCT   pInfo;

    pInfo = (MLME_ADDBA_REQ_STRUCT *)Msg;

    if ((MsgLen != sizeof(MLME_ADDBA_REQ_STRUCT)))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("MlmeAddBAReqSanity fail - message lenght not correct.\n"));
        return FALSE;
    }	
	
    if ((pInfo->Wcid >= MAX_LEN_OF_MAC_TABLE))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("MlmeAddBAReqSanity fail - The peer Mac is not associated yet.\n"));
        return FALSE;
    }	

   /* if ((pInfo->BaBufSize > MAX_RX_REORDERBUF) || (pInfo->BaBufSize < 2))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("MlmeAddBAReqSanity fail - Rx Reordering buffer too big or too small\n"));
        return FALSE;
    }	
*/
    if ((pInfo->pAddr[0]&0x01) == 0x01)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("MlmeAddBAReqSanity fail - broadcast address not support BA\n"));
        return FALSE;
    }
	
    
    return TRUE;
}


/* 
    ==========================================================================
    Description:
        MLME message sanity check
    Return:
        TRUE if all parameters are OK, FALSE otherwise
        
	IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
BOOLEAN MlmeDelBAReqSanity(
    IN PRTMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen) 
{
	MLME_DELBA_REQ_STRUCT *pInfo;
	pInfo = (MLME_DELBA_REQ_STRUCT *)Msg;

    if ((MsgLen != sizeof(MLME_DELBA_REQ_STRUCT)))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("MlmeDelBAReqSanity fail - message lenght not correct.\n"));
        return FALSE;
    }	
	
    if ((pInfo->Wcid >= MAX_LEN_OF_MAC_TABLE))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("MlmeDelBAReqSanity fail - The peer Mac is not associated yet.\n"));
        return FALSE;
    }	

    if ((pInfo->TID & 0xf0))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("MlmeDelBAReqSanity fail - The peer TID is incorrect.\n"));
        return FALSE;
    }	
    
    return TRUE;
}
#endif


