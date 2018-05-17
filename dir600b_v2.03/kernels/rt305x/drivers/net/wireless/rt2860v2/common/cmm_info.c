#include	"rt_config.h"

/*
    ==========================================================================
    Description:
        Get Driver version.

    Return:
    ==========================================================================
*/
INT Set_DriverVersion_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PUCHAR			arg)
{
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		DBGPRINT(RT_DEBUG_TRACE, ("Driver version-%s\n", AP_DRIVER_VERSION));
#endif // CONFIG_AP_SUPPORT //

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		DBGPRINT(RT_DEBUG_TRACE, ("Driver version-%s\n", STA_DRIVER_VERSION));
#endif // CONFIG_STA_SUPPORT //

    return TRUE;
}

/* 
    ==========================================================================
    Description:
        Set Country Region.
        This command will not work, if the field of CountryRegion in eeprom is programmed.
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT Set_CountryRegion_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PUCHAR			arg)
{
	ULONG region;
		
	region = simple_strtol(arg, 0, 10);

#ifdef EXT_BUILD_CHANNEL_LIST
	return -EOPNOTSUPP;
#endif // EXT_BUILD_CHANNEL_LIST //

	// Country can be set only when EEPROM not programmed
	if (pAd->CommonCfg.CountryRegion & 0x80)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Set_CountryRegion_Proc::parameter of CountryRegion in eeprom is programmed \n"));
		return FALSE;
	}
	
	if((region >= 0) && (region <= REGION_MAXIMUM_BG_BAND))
	{
		pAd->CommonCfg.CountryRegion = (UCHAR) region;
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Set_CountryRegion_Proc::parameters out of range\n"));
		return FALSE;
	}

	// if set country region, driver needs to be reset
	BuildChannelList(pAd);

	DBGPRINT(RT_DEBUG_TRACE, ("Set_CountryRegion_Proc::(CountryRegion=%d)\n", pAd->CommonCfg.CountryRegion));
	
	return TRUE;
}

/* 
    ==========================================================================
    Description:
        Set Country Region for A band.
        This command will not work, if the field of CountryRegion in eeprom is programmed.
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT Set_CountryRegionABand_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PUCHAR			arg)
{
	ULONG region;
		
	region = simple_strtol(arg, 0, 10);

#ifdef EXT_BUILD_CHANNEL_LIST
	return -EOPNOTSUPP;
#endif // EXT_BUILD_CHANNEL_LIST //

	// Country can be set only when EEPROM not programmed
	if (pAd->CommonCfg.CountryRegionForABand & 0x80)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Set_CountryRegionABand_Proc::parameter of CountryRegion in eeprom is programmed \n"));
		return FALSE;
	}
	
	if((region >= 0) && (region <= REGION_MAXIMUM_A_BAND))
	{
		pAd->CommonCfg.CountryRegionForABand = (UCHAR) region;
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Set_CountryRegionABand_Proc::parameters out of range\n"));
		return FALSE;
	}

	// if set country region, driver needs to be reset
	BuildChannelList(pAd);

	DBGPRINT(RT_DEBUG_TRACE, ("Set_CountryRegionABand_Proc::(CountryRegion=%d)\n", pAd->CommonCfg.CountryRegionForABand));
	
	return TRUE;
}

/* 
    ==========================================================================
    Description:
        Set Wireless Mode
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_WirelessMode_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PUCHAR			arg)
{
	ULONG	WirelessMode;
//#ifdef CONFIG_AP_SUPPORT
//	INT		i;
//#endif // CONFIG_AP_SUPPORT //
	INT		success = TRUE;

	WirelessMode = simple_strtol(arg, 0, 10);

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		switch(WirelessMode)
		{
			case PHY_11BG_MIXED:	// 0
			case PHY_11B:			// 1
			case PHY_11A:			// 2
			case PHY_11ABG_MIXED:	// 3
			case PHY_11G:			// 4
			case PHY_11ABGN_MIXED:	// 5
			case PHY_11N_2_4G:		// 6
			case PHY_11GN_MIXED:	// 7
			case PHY_11AN_MIXED:	// 8
			case PHY_11BGN_MIXED:	// 9
			case PHY_11AGN_MIXED:	// 10
			case PHY_11N_5G:		// 11
				pAd->CommonCfg.PhyMode = WirelessMode;
				break;

			default :
				success = FALSE;
				break;
		}

		BuildChannelList(pAd);

		//for(i=0; i<pAd->ApCfg.BssidNum;i++)
		{
			RTMPSetPhyMode(pAd, pAd->CommonCfg.PhyMode);
		}
	}
#endif // CONFIG_AP_SUPPORT //

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		if (WirelessMode <= PHY_11AGN_MIXED)
		{
			RTMPSetPhyMode(pAd, WirelessMode);
			if (WirelessMode >= PHY_11ABGN_MIXED)
			{
				pAd->CommonCfg.BACapability.field.AutoBA = TRUE;
				pAd->CommonCfg.REGBACapability.field.AutoBA = TRUE;
			}
			else
			{
				pAd->CommonCfg.BACapability.field.AutoBA = FALSE;
				pAd->CommonCfg.REGBACapability.field.AutoBA = FALSE;
			}

			// Set AdhocMode rates
			if (pAd->StaCfg.BssType == BSS_ADHOC)
			{
				if (WirelessMode == PHY_11B)
					pAd->StaCfg.AdhocMode = 0;
				else if ((WirelessMode == PHY_11BG_MIXED) || (WirelessMode == PHY_11ABG_MIXED))
					pAd->StaCfg.AdhocMode = 1;
				else if ((WirelessMode == PHY_11A) || (WirelessMode == PHY_11G))
					pAd->StaCfg.AdhocMode = 2;

				MlmeUpdateTxRates(pAd, FALSE, 0);
				MakeIbssBeacon(pAd);           // re-build BEACON frame
				AsicEnableIbssSync(pAd);       // copy to on-chip memory
			}
		}
		else
		{
			success = FALSE;
		}
	}
#endif // CONFIG_STA_SUPPORT //

	// it is needed to set SSID to take effect
	if (success == TRUE)
	{
		SetCommonHT(pAd);
		DBGPRINT(RT_DEBUG_TRACE, ("Set_WirelessMode_Proc::(=%ld)\n", WirelessMode));
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Set_WirelessMode_Proc::parameters out of range\n"));
	}
	
	return success;
}

/* 
    ==========================================================================
    Description:
        Set Channel
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_Channel_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PUCHAR			arg)
{
#ifdef CONFIG_AP_SUPPORT
	INT		i;
#endif // CONFIG_AP_SUPPORT //
 	INT		success = TRUE;
	UCHAR	Channel;	

	Channel = (UCHAR) simple_strtol(arg, 0, 10);

	// check if this channel is valid
	if (ChannelSanity(pAd, Channel) == TRUE)
	{
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
			pAd->CommonCfg.Channel = Channel;        

			if (MONITOR_ON(pAd))
			{
				N_ChannelCheck(pAd);
				if (pAd->CommonCfg.PhyMode >= PHY_11ABGN_MIXED &&
					pAd->CommonCfg.RegTransmitSetting.field.BW == BW_40)
				{
					if (pAd->CommonCfg.RegTransmitSetting.field.EXTCHA == EXTCHA_ABOVE)
					{
						pAd->CommonCfg.CentralChannel = pAd->CommonCfg.Channel + 2;
						AsicSwitchChannel(pAd, pAd->CommonCfg.CentralChannel, FALSE);
						AsicLockChannel(pAd, pAd->CommonCfg.CentralChannel);
					}
					else
					{
						pAd->CommonCfg.CentralChannel = pAd->CommonCfg.Channel - 2;
						AsicSwitchChannel(pAd, pAd->CommonCfg.CentralChannel, FALSE);
						AsicLockChannel(pAd, pAd->CommonCfg.CentralChannel);
					}
					DBGPRINT(RT_DEBUG_TRACE, ("BW_40, control_channel(%d), CentralChannel(%d) \n", 
								pAd->CommonCfg.Channel, pAd->CommonCfg.CentralChannel));
				}
				else
				{
					AsicSwitchChannel(pAd, pAd->CommonCfg.Channel, FALSE);
					AsicLockChannel(pAd, pAd->CommonCfg.Channel);
					DBGPRINT(RT_DEBUG_TRACE, ("BW_20, Channel(%d)\n", pAd->CommonCfg.Channel));
				}
			}
		}
#endif // CONFIG_STA_SUPPORT //
		success = TRUE;
	}
	else
	{
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			Channel = FirstChannel(pAd);
			DBGPRINT(RT_DEBUG_WARN,("This channel is out of channel list, set as the first channel(%d) \n ", Channel));
		}
#endif // CONFIG_AP_SUPPORT //

#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
			success = FALSE;
#endif // CONFIG_STA_SUPPORT //
	}

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		if (((pAd->CommonCfg.PhyMode == PHY_11A)
			|| (pAd->CommonCfg.PhyMode == PHY_11AN_MIXED))
			&& (pAd->CommonCfg.bIEEE80211H == TRUE))
		{
			for (i = 0; i < pAd->ChannelListNum; i++)
			{
				if (pAd->ChannelList[i].Channel == Channel)
				{
					if (pAd->ChannelList[i].RemainingTimeForUse > 0)
					{
						DBGPRINT(RT_DEBUG_ERROR, ("ERROR: previous detection of a radar on this channel(Channel=%d)\n", Channel));
						success = FALSE;
						break;
					}
					else
					{
						DBGPRINT(RT_DEBUG_TRACE, ("RemainingTimeForUse %d ,Channel %d\n", pAd->ChannelList[i].RemainingTimeForUse, Channel));
					}
				}
			}
		}

		if (success == TRUE)
		{
			pAd->CommonCfg.Channel = Channel;
			N_ChannelCheck(pAd);

			if ((pAd->CommonCfg.Channel > 14 )
				&& (pAd->CommonCfg.bIEEE80211H == TRUE))
			{
				if (pAd->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
				{
					APStop(pAd);
					APStartUp(pAd);
				}
				else
				{
					pAd->CommonCfg.RadarDetect.RDMode = RD_SWITCHING_MODE;
					pAd->CommonCfg.RadarDetect.CSCount = 0;
				}
			}
			else
			{
				APStop(pAd);
				APStartUp(pAd);
			}
		}
	}
#endif // CONFIG_AP_SUPPORT //

	if (success == TRUE)
		DBGPRINT(RT_DEBUG_TRACE, ("Set_Channel_Proc::(Channel=%d)\n", pAd->CommonCfg.Channel));

	return success;
}

/* 
    ==========================================================================
    Description:
        Set Short Slot Time Enable or Disable
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_ShortSlot_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PUCHAR			arg)
{
	ULONG ShortSlot;

	ShortSlot = simple_strtol(arg, 0, 10);

	if (ShortSlot == 1)
		pAd->CommonCfg.bUseShortSlotTime = TRUE;
	else if (ShortSlot == 0)
		pAd->CommonCfg.bUseShortSlotTime = FALSE;
	else
		return FALSE;  //Invalid argument 
	
	DBGPRINT(RT_DEBUG_TRACE, ("Set_ShortSlot_Proc::(ShortSlot=%d)\n", pAd->CommonCfg.bUseShortSlotTime));

	return TRUE;
}

/* 
    ==========================================================================
    Description:
        Set Tx power
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_TxPower_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PUCHAR			arg)
{
	ULONG TxPower;
	INT   success = FALSE;

	TxPower = (ULONG) simple_strtol(arg, 0, 10);
	if (TxPower <= 100)
	{
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
			pAd->CommonCfg.TxPowerPercentage = TxPower;
#endif // CONFIG_AP_SUPPORT //

#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
			pAd->CommonCfg.TxPowerDefault = TxPower;
			pAd->CommonCfg.TxPowerPercentage = pAd->CommonCfg.TxPowerDefault;
		}
#endif // CONFIG_STA_SUPPORT //
		success = TRUE;
	}
	else
		success = FALSE;

	DBGPRINT(RT_DEBUG_TRACE, ("Set_TxPower_Proc::(TxPowerPercentage=%ld)\n", pAd->CommonCfg.TxPowerPercentage));

	return success;
}

/* 
    ==========================================================================
    Description:
        Set 11B/11G Protection
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_BGProtection_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PUCHAR			arg)
{
	switch (simple_strtol(arg, 0, 10))
	{
		case 0: //AUTO
			pAd->CommonCfg.UseBGProtection = 0;
			break;
		case 1: //Always On
			pAd->CommonCfg.UseBGProtection = 1;
			break;
		case 2: //Always OFF
			pAd->CommonCfg.UseBGProtection = 2;
			break;		
		default:  //Invalid argument 
			return FALSE;
	}

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		APUpdateCapabilityAndErpIe(pAd);
#endif // CONFIG_AP_SUPPORT //

	DBGPRINT(RT_DEBUG_TRACE, ("Set_BGProtection_Proc::(BGProtection=%ld)\n", pAd->CommonCfg.UseBGProtection));	

	return TRUE;
}

/* 
    ==========================================================================
    Description:
        Set TxPreamble
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_TxPreamble_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PUCHAR			arg)
{
	RT_802_11_PREAMBLE	Preamble;

	Preamble = simple_strtol(arg, 0, 10);

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	if (Preamble == Rt802_11PreambleAuto)
		return FALSE;
#endif // CONFIG_AP_SUPPORT //

	switch (Preamble)
	{
		case Rt802_11PreambleShort:
			pAd->CommonCfg.TxPreamble = Preamble;
#ifdef CONFIG_STA_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
				MlmeSetTxPreamble(pAd, Rt802_11PreambleShort);
#endif // CONFIG_STA_SUPPORT //
			break;
		case Rt802_11PreambleLong:
#ifdef CONFIG_STA_SUPPORT
		case Rt802_11PreambleAuto:
			// if user wants AUTO, initialize to LONG here, then change according to AP's
			// capability upon association.
#endif // CONFIG_STA_SUPPORT //
			pAd->CommonCfg.TxPreamble = Preamble;
#ifdef CONFIG_STA_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
				MlmeSetTxPreamble(pAd, Rt802_11PreambleLong);
#endif // CONFIG_STA_SUPPORT //
			break;
		default: //Invalid argument 
			return FALSE;
	}

	DBGPRINT(RT_DEBUG_TRACE, ("Set_TxPreamble_Proc::(TxPreamble=%ld)\n", pAd->CommonCfg.TxPreamble));

	return TRUE;
}

/* 
    ==========================================================================
    Description:
        Set RTS Threshold
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_RTSThreshold_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PUCHAR			arg)
{
	 NDIS_802_11_RTS_THRESHOLD           RtsThresh;

	RtsThresh = simple_strtol(arg, 0, 10);

	if((RtsThresh > 0) && (RtsThresh <= MAX_RTS_THRESHOLD))
		pAd->CommonCfg.RtsThreshold  = (USHORT)RtsThresh;
#ifdef CONFIG_STA_SUPPORT
	else if (RtsThresh == 0)
		pAd->CommonCfg.RtsThreshold = MAX_RTS_THRESHOLD;
#endif // CONFIG_STA_SUPPORT //
	else
		return FALSE; //Invalid argument 

	DBGPRINT(RT_DEBUG_TRACE, ("Set_RTSThreshold_Proc::(RTSThreshold=%d)\n", pAd->CommonCfg.RtsThreshold));

	return TRUE;
}

/* 
    ==========================================================================
    Description:
        Set Fragment Threshold
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_FragThreshold_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PUCHAR			arg)
{
	 NDIS_802_11_FRAGMENTATION_THRESHOLD     FragThresh;

	FragThresh = simple_strtol(arg, 0, 10);

	if ( (FragThresh >= MIN_FRAG_THRESHOLD) && (FragThresh <= MAX_FRAG_THRESHOLD))
		pAd->CommonCfg.FragmentThreshold =  (USHORT)FragThresh;
#ifdef CONFIG_STA_SUPPORT
	else if (FragThresh == 0)
		pAd->CommonCfg.FragmentThreshold = MAX_FRAG_THRESHOLD;
#endif // CONFIG_STA_SUPPORT //
	else
		return FALSE; //Invalid argument 

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		if (pAd->CommonCfg.FragmentThreshold == MAX_FRAG_THRESHOLD)
			pAd->CommonCfg.bUseZeroToDisableFragment = TRUE;
		else
			pAd->CommonCfg.bUseZeroToDisableFragment = FALSE;
	}
#endif // CONFIG_STA_SUPPORT //

	DBGPRINT(RT_DEBUG_TRACE, ("Set_FragThreshold_Proc::(FragThreshold=%d)\n", pAd->CommonCfg.FragmentThreshold));

	return TRUE;
}

/* 
    ==========================================================================
    Description:
        Set TxBurst
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_TxBurst_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PUCHAR			arg)
{
	ULONG TxBurst;

	TxBurst = simple_strtol(arg, 0, 10);
	if (TxBurst == 1)
		pAd->CommonCfg.bEnableTxBurst = TRUE;
	else if (TxBurst == 0)
		pAd->CommonCfg.bEnableTxBurst = FALSE;
	else
		return FALSE;  //Invalid argument 
	
	DBGPRINT(RT_DEBUG_TRACE, ("Set_TxBurst_Proc::(TxBurst=%d)\n", pAd->CommonCfg.bEnableTxBurst));

	return TRUE;
}

#ifdef AGGREGATION_SUPPORT
/* 
    ==========================================================================
    Description:
        Set TxBurst
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_PktAggregate_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PUCHAR			arg)
{
	ULONG aggre;

	aggre = simple_strtol(arg, 0, 10);

	if (aggre == 1)
		pAd->CommonCfg.bAggregationCapable = TRUE;
	else if (aggre == 0)
		pAd->CommonCfg.bAggregationCapable = FALSE;
	else
		return FALSE;  //Invalid argument 

#ifdef CONFIG_AP_SUPPORT
#ifdef PIGGYBACK_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		pAd->CommonCfg.bPiggyBackCapable = pAd->CommonCfg.bAggregationCapable;
		RTMPSetPiggyBack(pAd, pAd->CommonCfg.bPiggyBackCapable);
	}
#endif // PIGGYBACK_SUPPORT //
#endif // CONFIG_AP_SUPPORT //

	DBGPRINT(RT_DEBUG_TRACE, ("Set_PktAggregate_Proc::(AGGRE=%d)\n", pAd->CommonCfg.bAggregationCapable));

	return TRUE;
}
#endif

/* 
    ==========================================================================
    Description:
        Set IEEE80211H.
        This parameter is 1 when needs radar detection, otherwise 0
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_IEEE80211H_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PUCHAR			arg)
{
    ULONG ieee80211h;

	ieee80211h = simple_strtol(arg, 0, 10);

	if (ieee80211h == 1)
		pAd->CommonCfg.bIEEE80211H = TRUE;
	else if (ieee80211h == 0)
		pAd->CommonCfg.bIEEE80211H = FALSE;
	else
		return FALSE;  //Invalid argument 
	
	DBGPRINT(RT_DEBUG_TRACE, ("Set_IEEE80211H_Proc::(IEEE80211H=%d)\n", pAd->CommonCfg.bIEEE80211H));

	return TRUE;
}

#ifdef WSC_INCLUDED
INT	Set_WscGenPinCode_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PUCHAR			arg)
{
    PWSC_CTRL   pWscControl = NULL;
    POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
    UCHAR	    apidx = pObj->ioctl_if;
    
    // Only support WPS in ra0 now, 2006.11.10
    if (apidx != MAIN_MBSSID)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("Set_WscGenPinCode_Proc:: Only support WPS in ra0 or apcli0 now.\n"));
        return FALSE;
    }

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
#ifdef APCLI_SUPPORT
	    if (pObj->ioctl_if_type == INT_APCLI)
	    {
	        pWscControl = &pAd->ApCfg.ApCliTab[apidx].WscControl;
	        DBGPRINT(RT_DEBUG_TRACE, ("IF(apcli%d) Set_WscGenPinCode_Proc:: This command is from apcli interface now.\n", apidx));
	    }
	    else
#endif // APCLI_SUPPORT //
	    {
			pWscControl = &pAd->ApCfg.MBSSID[apidx].WscControl;
	        DBGPRINT(RT_DEBUG_TRACE, ("IF(ra%d) Set_WscGenPinCode_Proc:: This command is from ra interface now.\n", apidx));
	    }

		pWscControl->WscEnrolleePinCode = WscRandomGeneratePinCode(pAd, 0);
	}
#endif // CONFIG_AP_SUPPORT //

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
    	pWscControl = &pAd->StaCfg.WscControl;
		pWscControl->WscEnrolleePinCode = WscRandomGeneratePinCode(pAd, 0);
	}
#endif // CONFIG_STA_SUPPORT //

	DBGPRINT(RT_DEBUG_TRACE, ("Set_WscGenPinCode_Proc:: Enrollee PinCode\t\t%08u\n", pWscControl->WscEnrolleePinCode));

	return TRUE;
}

INT Set_WscVendorPinCode_Proc(
    IN  PRTMP_ADAPTER   pAd, 
    IN  PUCHAR          arg)
{
    PWSC_CTRL   pWscControl;
    POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
    UCHAR       apidx = pObj->ioctl_if;
    UINT PinCode = simple_strtol(arg, 0, 10); // When PinCode is 03571361, return value is 3571361.

    // Only support WPS in ra0 now, 2006.11.10
    if (apidx != MAIN_MBSSID)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("Set_WscVendorPinCode_Proc:: Only support WPS in ra0 or apcli0 now.\n"));
        return FALSE;
    }

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
#ifdef APCLI_SUPPORT
	    if (pObj->ioctl_if_type == INT_APCLI)
	    {
	        pWscControl = &pAd->ApCfg.ApCliTab[apidx].WscControl;
	        DBGPRINT(RT_DEBUG_TRACE, ("IF(apcli%d) Set_WscVendorPinCode_Proc:: This command is from apcli interface now.\n", apidx));
	    }
	    else
#endif // APCLI_SUPPORT //
	    {
	        pWscControl = &pAd->ApCfg.MBSSID[apidx].WscControl;
	        DBGPRINT(RT_DEBUG_TRACE, ("IF(ra%d) Set_WscVendorPinCode_Proc:: This command is from ra interface now.\n", apidx));
	    }
	    if ( ValidateChecksum(PinCode) )
	    {
	        DBGPRINT(RT_DEBUG_TRACE, ("%s - WSC_VENDOR_PIN_CODE, value = %d\n", __FUNCTION__, PinCode));
	        pWscControl->WscEnrolleePinCode = PinCode;
	    }
	    else
	    {
	        DBGPRINT(RT_DEBUG_ERROR, ("%s - WSC_VENDOR_PIN_CODE: invalid pin code (%d)\n", __FUNCTION__, PinCode));
	        return FALSE;
	    }
	}
#endif // CONFIG_AP_SUPPORT //

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
 	{
 		pWscControl = &pAd->StaCfg.WscControl;

	    if ( ValidateChecksum(PinCode) )
	    {
	        DBGPRINT(RT_DEBUG_TRACE, ("%s - WSC_VENDOR_PIN_CODE, value = %d\n", __FUNCTION__, PinCode));
	        pWscControl->WscEnrolleePinCode = PinCode;
	    }
	    else
	    {
	        DBGPRINT(RT_DEBUG_ERROR, ("%s - WSC_VENDOR_PIN_CODE: invalid pin code (%d)\n", __FUNCTION__, PinCode));
	        return FALSE;
	    }
 	}
#endif // CONFIG_STA_SUPPORT //

    return TRUE;
}
#endif // WSC_INCLUDED //

#ifdef DBG
/* 
    ==========================================================================
    Description:
        For Debug information
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_Debug_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PUCHAR			arg)
{
	DBGPRINT(RT_DEBUG_TRACE, ("==> Set_Debug_Proc *******************\n"));

    if(simple_strtol(arg, 0, 10) <= RT_DEBUG_LOUD)
        RTDebugLevel = simple_strtol(arg, 0, 10);

	DBGPRINT(RT_DEBUG_TRACE, ("<== Set_Debug_Proc(RTDebugLevel = %ld)\n", RTDebugLevel));

	return TRUE;
}
#endif

INT	Show_DescInfo_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PUCHAR			arg)
{
#ifdef RT2860
	INT i, QueIdx=0;
//  ULONG	RegValue;
        PRT28XX_RXD_STRUC pRxD;
        PTXD_STRUC pTxD;
	PRTMP_TX_RING	pTxRing = &pAd->TxRing[QueIdx];	
	PRTMP_MGMT_RING	pMgmtRing = &pAd->MgmtRing;	
	PRTMP_RX_RING	pRxRing = &pAd->RxRing;	
	
	for(i=0;i<TX_RING_SIZE;i++)
	{	
	    pTxD = (PTXD_STRUC) pTxRing->Cell[i].AllocVa;
	    printk("Desc #%d\n",i);
	    hex_dump("Tx Descriptor", (char *)pTxD, 16);
	    printk("pTxD->DMADONE = %x\n", pTxD->DMADONE);
	}    
	printk("---------------------------------------------------\n");
	for(i=0;i<MGMT_RING_SIZE;i++)
	{	
	    pTxD = (PTXD_STRUC) pMgmtRing->Cell[i].AllocVa;
	    printk("Desc #%d\n",i);
	    hex_dump("Mgmt Descriptor", (char *)pTxD, 16);
	    printk("pMgmt->DMADONE = %x\n", pTxD->DMADONE);
	}    
	printk("---------------------------------------------------\n");
	for(i=0;i<RX_RING_SIZE;i++)
	{	
	    pRxD = (PRT28XX_RXD_STRUC) pRxRing->Cell[i].AllocVa;
	    printk("Desc #%d\n",i);
	    hex_dump("Rx Descriptor", (char *)pRxD, 16);
	}
#endif // RT2860 //

	return TRUE;
}

/* 
    ==========================================================================
    Description:
        Reset statistics counter

    Arguments:
        pAdapter            Pointer to our adapter
        arg                 

    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_ResetStatCounter_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PUCHAR			arg)
{
	//UCHAR           i;
	//MAC_TABLE_ENTRY *pEntry;
    
	DBGPRINT(RT_DEBUG_TRACE, ("==>Set_ResetStatCounter_Proc\n"));

	// add the most up-to-date h/w raw counters into software counters
	NICUpdateRawCounters(pAd);
    
	NdisZeroMemory(&pAd->WlanCounters, sizeof(COUNTER_802_11));
	NdisZeroMemory(&pAd->Counters8023, sizeof(COUNTER_802_3));
	NdisZeroMemory(&pAd->RalinkCounters, sizeof(COUNTER_RALINK));

	// Reset HotSpot counter
#if 0 // ToDo.
	for (i = 0; i < MAX_LEN_OF_MAC_TABLE; i++)
	{
		pEntry = &pAd->MacTab.Content[i];

		if ((pEntry->Valid == FALSE) || (pEntry->Sst != SST_ASSOC))
			continue;

		pEntry->HSCounter.LastDataPacketTime = 0;
		pEntry->HSCounter.TotalRxByteCount= 0;
		pEntry->HSCounter.TotalTxByteCount= 0;
	}
#endif    

#ifdef CONFIG_AP_SUPPORT
#endif // CONFIG_AP_SUPPORT //

	return TRUE;
}

/*
	========================================================================
	
	Routine Description:
		Add WPA key process.
		In Adhoc WPANONE, bPairwise = 0;  KeyIdx = 0;

	Arguments:
		pAd 					Pointer to our adapter
		pBuf							Pointer to the where the key stored

	Return Value:
		NDIS_SUCCESS					Add key successfully

	IRQL = DISPATCH_LEVEL
	
	Note:
		
	========================================================================
*/
#if 0 // remove by AlbertY
NDIS_STATUS RTMPWPAAddKeyProc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PVOID			pBuf)
{
	PNDIS_802_11_KEY	pKey;
	ULONG				KeyIdx;
//	NDIS_STATUS 		Status;
//	ULONG 	offset;	// unused variable, snowpin 2006.07.13
	
	PUCHAR		pTxMic, pRxMic;
	BOOLEAN 	bTxKey; 		// Set the key as transmit key
	BOOLEAN 	bPairwise;		// Indicate the key is pairwise key
	BOOLEAN 	bKeyRSC;		// indicate the receive  SC set by KeyRSC value.
								// Otherwise, it will set by the NIC.
	BOOLEAN 	bAuthenticator; // indicate key is set by authenticator.
	UCHAR		apidx = BSS0;
	
	pKey = (PNDIS_802_11_KEY) pBuf;
	KeyIdx = pKey->KeyIndex & 0xff;
	// Bit 31 of Add-key, Tx Key
	bTxKey		   = (pKey->KeyIndex & 0x80000000) ? TRUE : FALSE;
	// Bit 30 of Add-key PairwiseKey
	bPairwise	   = (pKey->KeyIndex & 0x40000000) ? TRUE : FALSE;
	// Bit 29 of Add-key KeyRSC
	bKeyRSC 	   = (pKey->KeyIndex & 0x20000000) ? TRUE : FALSE;
	// Bit 28 of Add-key Authenticator
	bAuthenticator = (pKey->KeyIndex & 0x10000000) ? TRUE : FALSE;

	DBGPRINT(RT_DEBUG_TRACE,("RTMPWPAAddKeyProc==>pKey->KeyIndex = %x. bPairwise= %d\n", pKey->KeyIndex, bPairwise));
	// 1. Check Group / Pairwise Key
	if (bPairwise)	// Pairwise Key
	{
		// 1. KeyIdx must be 0, otherwise, return NDIS_STATUS_INVALID_DATA
		if (KeyIdx != 0)
			return(NDIS_STATUS_INVALID_DATA);
		
		// 2. Check bTx, it must be true, otherwise, return NDIS_STATUS_INVALID_DATA
		if (bTxKey == FALSE)
			return(NDIS_STATUS_INVALID_DATA);

		// 3. If BSSID is all 0xff, return NDIS_STATUS_INVALID_DATA
		if (MAC_ADDR_EQUAL(pKey->BSSID, BROADCAST_ADDR))
			return(NDIS_STATUS_INVALID_DATA);

		// 3.1 Check Pairwise key length for TKIP key. For AES, it's always 128 bits
		//if ((pAdapter->PortCfg.WepStatus == Ndis802_11Encryption2Enabled) && (pKey->KeyLength != LEN_TKIP_KEY))
		if ((pAd->StaCfg.PairCipher == Ndis802_11Encryption2Enabled) && (pKey->KeyLength != LEN_TKIP_KEY))
			return(NDIS_STATUS_INVALID_DATA);

		pAd->SharedKey[apidx][KeyIdx].Type = PAIRWISE_KEY;

		if (pAd->ApCfg.MBSSID[apidx].AuthMode == Ndis802_11AuthModeWPA2)
		{
			// Send media specific event to start PMKID caching
			RTMPIndicateWPA2Status(pAd);
		}
	}
	else
	{
		// 1. Check BSSID, if not current BSSID or Bcast, return NDIS_STATUS_INVALID_DATA
		if ((! MAC_ADDR_EQUAL(pKey->BSSID, BROADCAST_ADDR)) &&
			(! MAC_ADDR_EQUAL(pKey->BSSID, pAd->ApCfg.MBSSID[apidx].Bssid)))
			return(NDIS_STATUS_INVALID_DATA);

		// 2. Check Key index for supported Group Key
		if (KeyIdx >= GROUP_KEY_NUM)
			return(NDIS_STATUS_INVALID_DATA);
		
		// 3. Set as default Tx Key if bTxKey is TRUE
		if (bTxKey == TRUE)
			pAd->ApCfg.MBSSID[apidx].DefaultKeyId = (UCHAR) KeyIdx;

		pAd->SharedKey[apidx][KeyIdx].Type = GROUP_KEY;
	}
			
	// 4. Select RxMic / TxMic based on Supp / Authenticator
	if (pAd->ApCfg.MBSSID[apidx].AuthMode == Ndis802_11AuthModeWPANone)
	{
		// for WPA-None Tx, Rx MIC is the same
		pTxMic = (PUCHAR) (&pKey->KeyMaterial) + 16;
		pRxMic = pTxMic;
	}
	else if (bAuthenticator == TRUE)
	{
		pTxMic = (PUCHAR) (&pKey->KeyMaterial) + 16;
		pRxMic = (PUCHAR) (&pKey->KeyMaterial) + 24;
	}
	else
	{
		pRxMic = (PUCHAR) (&pKey->KeyMaterial) + 16;
		pTxMic = (PUCHAR) (&pKey->KeyMaterial) + 24;
	}
		
	// 6. Check RxTsc
	if (bKeyRSC == TRUE)
	{
		NdisMoveMemory(pAd->SharedKey[apidx][KeyIdx].RxTsc, &pKey->KeyRSC, 6);			
		NdisMoveMemory(pAd->MacTab.Content[BSSID_WCID].PairwiseKey.RxTsc, &pKey->KeyRSC, 6);			
	}
	else
	{
		NdisZeroMemory(pAd->SharedKey[apidx][KeyIdx].RxTsc, 6);		
	}
		
	// 7. Copy information into Pairwise Key structure.
	// pKey->KeyLength will include TxMic and RxMic, therefore, we use 16 bytes hardcoded.
	pAd->SharedKey[apidx][KeyIdx].KeyLen = (UCHAR) pKey->KeyLength;		
	pAd->MacTab.Content[BSSID_WCID].PairwiseKey.KeyLen = (UCHAR)pKey->KeyLength;
	NdisMoveMemory(pAd->SharedKey[BSS0][KeyIdx].Key, &pKey->KeyMaterial, 16);
	NdisMoveMemory(pAd->MacTab.Content[BSSID_WCID].PairwiseKey.Key, &pKey->KeyMaterial, 16);
	if (pKey->KeyLength == LEN_TKIP_KEY)
	{
		// Only Key lenth equal to TKIP key have these
		NdisMoveMemory(pAd->SharedKey[apidx][KeyIdx].RxMic, pRxMic, 8);
		NdisMoveMemory(pAd->SharedKey[apidx][KeyIdx].TxMic, pTxMic, 8);
		NdisMoveMemory(pAd->MacTab.Content[BSSID_WCID].PairwiseKey.RxMic, pRxMic, 8);
		NdisMoveMemory(pAd->MacTab.Content[BSSID_WCID].PairwiseKey.TxMic, pTxMic, 8);
	}

	COPY_MAC_ADDR(pAd->SharedKey[BSS0][KeyIdx].BssId, pKey->BSSID);

	// Init TxTsc to one based on WiFi WPA specs
	pAd->SharedKey[apidx][KeyIdx].TxTsc[0] = 1;
	pAd->SharedKey[apidx][KeyIdx].TxTsc[1] = 0;
	pAd->SharedKey[apidx][KeyIdx].TxTsc[2] = 0;
	pAd->SharedKey[apidx][KeyIdx].TxTsc[3] = 0;
	pAd->SharedKey[apidx][KeyIdx].TxTsc[4] = 0;
	pAd->SharedKey[apidx][KeyIdx].TxTsc[5] = 0;
	// 4. Init TxTsc to one based on WiFi WPA specs
	pAd->MacTab.Content[BSSID_WCID].PairwiseKey.TxTsc[0] = 1;
	pAd->MacTab.Content[BSSID_WCID].PairwiseKey.TxTsc[1] = 0;
	pAd->MacTab.Content[BSSID_WCID].PairwiseKey.TxTsc[2] = 0;
	pAd->MacTab.Content[BSSID_WCID].PairwiseKey.TxTsc[3] = 0;
	pAd->MacTab.Content[BSSID_WCID].PairwiseKey.TxTsc[4] = 0;
	pAd->MacTab.Content[BSSID_WCID].PairwiseKey.TxTsc[5] = 0;

	if (pAd->ApCfg.MBSSID[apidx].WepStatus == Ndis802_11Encryption3Enabled)
	{	
		pAd->SharedKey[apidx][KeyIdx].CipherAlg = CIPHER_AES;
		pAd->MacTab.Content[BSSID_WCID].PairwiseKey.CipherAlg = CIPHER_AES;
	}
	else if (pAd->ApCfg.MBSSID[apidx].WepStatus == Ndis802_11Encryption2Enabled)
	{	
		pAd->SharedKey[apidx][KeyIdx].CipherAlg = CIPHER_TKIP;
		pAd->MacTab.Content[BSSID_WCID].PairwiseKey.CipherAlg = CIPHER_TKIP;
	}
	else if (pAd->ApCfg.MBSSID[apidx].WepStatus == Ndis802_11Encryption1Enabled)
	{
		if (pAd->SharedKey[apidx][KeyIdx].KeyLen == 5)
		{
			pAd->SharedKey[apidx][KeyIdx].CipherAlg = CIPHER_WEP64;
			pAd->MacTab.Content[BSSID_WCID].PairwiseKey.CipherAlg = CIPHER_WEP64;
		}
		else if (pAd->SharedKey[apidx][KeyIdx].KeyLen == 13)
		{
			pAd->SharedKey[apidx][KeyIdx].CipherAlg = CIPHER_WEP128;
			pAd->MacTab.Content[BSSID_WCID].PairwiseKey.CipherAlg = CIPHER_WEP128;
		}
		else
		{
			pAd->SharedKey[apidx][KeyIdx].CipherAlg = CIPHER_NONE;
			pAd->MacTab.Content[BSSID_WCID].PairwiseKey.CipherAlg = CIPHER_NONE;
		}
	}
	else
	{
		pAd->SharedKey[apidx][KeyIdx].CipherAlg = CIPHER_NONE;
		pAd->MacTab.Content[BSSID_WCID].PairwiseKey.CipherAlg = CIPHER_NONE;
	}

	if ((pAd->OpMode == OPMODE_STA))  // Pairwise Key. Add BSSID to WCTable
	{
		pAd->MacTab.Content[BSSID_WCID].PairwiseKey.CipherAlg = pAd->SharedKey[BSS0][KeyIdx].CipherAlg;
		pAd->MacTab.Content[BSSID_WCID].PairwiseKey.KeyLen = pAd->SharedKey[BSS0][KeyIdx].KeyLen;
	}

	if ((pAd->ApCfg.MBSSID[apidx].AuthMode == Ndis802_11AuthModeWPA2) || 
		(pAd->ApCfg.MBSSID[apidx].AuthMode == Ndis802_11AuthModeWPA2PSK))
	{
		//
		// On WPA2, Update Group Key Cipher.
		//
		if (!bPairwise)
		{
			if (pAd->StaCfg.GroupCipher == Ndis802_11Encryption3Enabled)
				pAd->SharedKey[apidx][KeyIdx].CipherAlg = CIPHER_AES;
			else if (pAd->StaCfg.GroupCipher == Ndis802_11Encryption2Enabled)
				pAd->SharedKey[apidx][KeyIdx].CipherAlg = CIPHER_TKIP;
		}
	}

	DBGPRINT(RT_DEBUG_TRACE, ("pAd->SharedKey[%d][%d].CipherAlg = %d\n", apidx, KeyIdx, pAd->SharedKey[apidx][KeyIdx].CipherAlg));

#if 0
	DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s Key #%d", CipherName[pAd->SharedKey[apidx][KeyIdx].CipherAlg],KeyIdx));
	for (i = 0; i < 16; i++)
	{
		DBGPRINT_RAW(RT_DEBUG_TRACE, ("%02x:", pAd->SharedKey[apidx][KeyIdx].Key[i]));
	}
	DBGPRINT_RAW(RT_DEBUG_TRACE, ("\n	  Rx MIC Key = "));
	for (i = 0; i < 8; i++)
	{
		DBGPRINT_RAW(RT_DEBUG_TRACE, ("%02x:", pAd->SharedKey[apidx][KeyIdx].RxMic[i]));
	}
	DBGPRINT_RAW(RT_DEBUG_TRACE, ("\n	  Tx MIC Key = "));
	for (i = 0; i < 8; i++)
	{
		DBGPRINT_RAW(RT_DEBUG_TRACE, ("%02x:", pAd->SharedKey[apidx][KeyIdx].TxMic[i]));
	}
	DBGPRINT_RAW(RT_DEBUG_TRACE, ("\n	  RxTSC = "));
	for (i = 0; i < 6; i++)
	{
		DBGPRINT_RAW(RT_DEBUG_TRACE, ("%02x:", pAd->SharedKey[apidx][KeyIdx].RxTsc[i]));
	}
#endif	  
	DBGPRINT_RAW(RT_DEBUG_TRACE, ("\n pKey-> BSSID:%02x:%02x:%02x:%02x:%02x:%02x \n",
		pKey->BSSID[0],pKey->BSSID[1],pKey->BSSID[2],pKey->BSSID[3],pKey->BSSID[4],pKey->BSSID[5]));

	if ((bTxKey) && (pAd->OpMode == OPMODE_STA))  // Pairwise Key. Add BSSID to WCTable
		RTMPAddBSSIDCipher(pAd, BSSID_WCID, pKey, pAd->SharedKey[BSS0][KeyIdx].CipherAlg);
	

	// No matter pairwise key or what leyidx is, always has a copy at on-chip SharedKeytable.
	AsicAddSharedKeyEntry(pAd, 
						  apidx, 
						  (UCHAR)KeyIdx, 
						  pAd->SharedKey[apidx][KeyIdx].CipherAlg,
						  pAd->SharedKey[apidx][KeyIdx].Key, 
						  pAd->SharedKey[apidx][KeyIdx].TxMic,
						  pAd->SharedKey[apidx][KeyIdx].RxMic);

	// The WCID key specified in used at Tx. For STA, always use pairwise key.

	// ad-hoc mode need to specify WAP Group key with WCID index=BSS0Mcast_WCID. Let's always set this key here.
/*	if (bPairwise == FALSE)
	{
		offset = MAC_IVEIV_TABLE_BASE + (BSS0Mcast_WCID * HW_IVEIV_ENTRY_SIZE);
		NdisZeroMemory(IVEIV, 8);
		// 1. IV/EIV
		// Specify key index to find shared key.
		if ((pAd->SharedKey[BSS0][KeyIdx].CipherAlg==CIPHER_TKIP) ||
			(pAd->SharedKey[BSS0][KeyIdx].CipherAlg==CIPHER_AES))
		IVEIV[3] = 0x20;		// Eiv bit on. keyid always 0 for pairwise key
		IVEIV[3] |= (KeyIdx<< 6);	// groupkey index is not 0
		for (i=0; i<8; i++)
		{
			RTMP_IO_WRITE8(pAd, offset+i, IVEIV[i]);
		}

		// 2. WCID Attribute UDF:3, BSSIdx:3, Alg:3, Keytable:use share key, BSSIdx is 0
		WCIDAttri = (pAd->SharedKey[BSS0][KeyIdx].CipherAlg<<1)|PAIRWISEKEYTABLE;
		offset = MAC_WCID_ATTRIBUTE_BASE + (BSS0Mcast_WCID* HW_WCID_ATTRI_SIZE);
		RTMP_IO_WRITE32(pAd, offset, WCIDAttri);
		
	}

*/

   if (pAd->SharedKey[apidx][KeyIdx].Type == GROUP_KEY)
	{
		// 802.1x port control
		pAd->StaCfg.PortSecured = WPA_802_1X_PORT_SECURED;
		DBGPRINT(RT_DEBUG_TRACE,("!!WPA_802_1X_PORT_SECURED!!\n"));

	}

	return (NDIS_STATUS_SUCCESS);
}
#endif 

BOOLEAN RTMPCheckStrPrintAble(
    IN  CHAR *pInPutStr, 
    IN  UCHAR strLen)
{
    UCHAR i=0;
    
    for (i=0; i<strLen; i++)
    {
        if ((pInPutStr[i] < 0x21) ||
            (pInPutStr[i] > 0x7E))
            return FALSE;
    }
    
    return TRUE;
}

/*
	========================================================================
	
	Routine Description:
		Remove WPA Key process

	Arguments:
		pAd 					Pointer to our adapter
		pBuf							Pointer to the where the key stored

	Return Value:
		NDIS_SUCCESS					Add key successfully

	IRQL = DISPATCH_LEVEL
	
	Note:
		
	========================================================================
*/
#ifdef CONFIG_STA_SUPPORT
VOID    RTMPSetDesiredRates(
    IN  PRTMP_ADAPTER   pAdapter,
    IN  LONG            Rates)
{
    NDIS_802_11_RATES aryRates;

    memset(&aryRates, 0x00, sizeof(NDIS_802_11_RATES));
    switch (pAdapter->CommonCfg.PhyMode)
    {
        case PHY_11A: // A only
            switch (Rates)
            {
                case 6000000: //6M
                    aryRates[0] = 0x0c; // 6M
                    pAdapter->StaCfg.DesiredTransmitSetting.field.MCS = MCS_0;
                    break;
                case 9000000: //9M
                    aryRates[0] = 0x12; // 9M
                    pAdapter->StaCfg.DesiredTransmitSetting.field.MCS = MCS_1;
                    break;
                case 12000000: //12M
                    aryRates[0] = 0x18; // 12M
                    pAdapter->StaCfg.DesiredTransmitSetting.field.MCS = MCS_2;
                    break;
                case 18000000: //18M
                    aryRates[0] = 0x24; // 18M
                    pAdapter->StaCfg.DesiredTransmitSetting.field.MCS = MCS_3;
                    break;
                case 24000000: //24M
                    aryRates[0] = 0x30; // 24M
                    pAdapter->StaCfg.DesiredTransmitSetting.field.MCS = MCS_4;
                    break;
                case 36000000: //36M
                    aryRates[0] = 0x48; // 36M
                    pAdapter->StaCfg.DesiredTransmitSetting.field.MCS = MCS_5;
                    break;
                case 48000000: //48M
                    aryRates[0] = 0x60; // 48M
                    pAdapter->StaCfg.DesiredTransmitSetting.field.MCS = MCS_6;
                    break;
                case 54000000: //54M
                    aryRates[0] = 0x6c; // 54M
                    pAdapter->StaCfg.DesiredTransmitSetting.field.MCS = MCS_7;
                    break;
                case -1: //Auto
                default:
                    aryRates[0] = 0x6c; // 54Mbps
                    aryRates[1] = 0x60; // 48Mbps
                    aryRates[2] = 0x48; // 36Mbps
                    aryRates[3] = 0x30; // 24Mbps
                    aryRates[4] = 0x24; // 18M
                    aryRates[5] = 0x18; // 12M
                    aryRates[6] = 0x12; // 9M
                    aryRates[7] = 0x0c; // 6M
                    pAdapter->StaCfg.DesiredTransmitSetting.field.MCS = MCS_AUTO;
                    break;
            }
            break;
        case PHY_11BG_MIXED: // B/G Mixed
        case PHY_11B: // B only
        case PHY_11ABG_MIXED: // A/B/G Mixed
        default:
            switch (Rates)
            {
                case 1000000: //1M
                    aryRates[0] = 0x02;
                    pAdapter->StaCfg.DesiredTransmitSetting.field.MCS = MCS_0;
                    break;
                case 2000000: //2M
                    aryRates[0] = 0x04;
                    pAdapter->StaCfg.DesiredTransmitSetting.field.MCS = MCS_1;
                    break;
                case 5000000: //5.5M
                    aryRates[0] = 0x0b; // 5.5M
                    pAdapter->StaCfg.DesiredTransmitSetting.field.MCS = MCS_2;
                    break;
                case 11000000: //11M
                    aryRates[0] = 0x16; // 11M
                    pAdapter->StaCfg.DesiredTransmitSetting.field.MCS = MCS_3;
                    break;
                case 6000000: //6M
                    aryRates[0] = 0x0c; // 6M
                    pAdapter->StaCfg.DesiredTransmitSetting.field.MCS = MCS_0;
                    break;
                case 9000000: //9M
                    aryRates[0] = 0x12; // 9M
                    pAdapter->StaCfg.DesiredTransmitSetting.field.MCS = MCS_1;
                    break;
                case 12000000: //12M
                    aryRates[0] = 0x18; // 12M
                    pAdapter->StaCfg.DesiredTransmitSetting.field.MCS = MCS_2;
                    break;
                case 18000000: //18M
                    aryRates[0] = 0x24; // 18M
                    pAdapter->StaCfg.DesiredTransmitSetting.field.MCS = MCS_3;
                    break;
                case 24000000: //24M
                    aryRates[0] = 0x30; // 24M
                    pAdapter->StaCfg.DesiredTransmitSetting.field.MCS = MCS_4;
                    break;
                case 36000000: //36M
                    aryRates[0] = 0x48; // 36M
                    pAdapter->StaCfg.DesiredTransmitSetting.field.MCS = MCS_5;
                    break;
                case 48000000: //48M
                    aryRates[0] = 0x60; // 48M
                    pAdapter->StaCfg.DesiredTransmitSetting.field.MCS = MCS_6;
                    break;
                case 54000000: //54M
                    aryRates[0] = 0x6c; // 54M
                    pAdapter->StaCfg.DesiredTransmitSetting.field.MCS = MCS_7;
                    break;
                case -1: //Auto
                default:
                    if (pAdapter->CommonCfg.PhyMode == PHY_11B)
                    { //B Only
                        aryRates[0] = 0x16; // 11Mbps
                        aryRates[1] = 0x0b; // 5.5Mbps
                        aryRates[2] = 0x04; // 2Mbps
                        aryRates[3] = 0x02; // 1Mbps
                    }
                    else
                    { //(B/G) Mixed or (A/B/G) Mixed
                        aryRates[0] = 0x6c; // 54Mbps
                        aryRates[1] = 0x60; // 48Mbps
                        aryRates[2] = 0x48; // 36Mbps
                        aryRates[3] = 0x30; // 24Mbps
                        aryRates[4] = 0x16; // 11Mbps
                        aryRates[5] = 0x0b; // 5.5Mbps
                        aryRates[6] = 0x04; // 2Mbps
                        aryRates[7] = 0x02; // 1Mbps
                    }
                    pAdapter->StaCfg.DesiredTransmitSetting.field.MCS = MCS_AUTO;
                    break;
            }
            break;
    }

    NdisZeroMemory(pAdapter->CommonCfg.DesireRate, MAX_LEN_OF_SUPPORTED_RATES);
    NdisMoveMemory(pAdapter->CommonCfg.DesireRate, &aryRates, sizeof(NDIS_802_11_RATES));
    DBGPRINT(RT_DEBUG_TRACE, (" RTMPSetDesiredRates (%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x)\n",
        pAdapter->CommonCfg.DesireRate[0],pAdapter->CommonCfg.DesireRate[1],
        pAdapter->CommonCfg.DesireRate[2],pAdapter->CommonCfg.DesireRate[3],
        pAdapter->CommonCfg.DesireRate[4],pAdapter->CommonCfg.DesireRate[5],
        pAdapter->CommonCfg.DesireRate[6],pAdapter->CommonCfg.DesireRate[7] ));
    // Changing DesiredRate may affect the MAX TX rate we used to TX frames out
    MlmeUpdateTxRates(pAdapter, FALSE, 0);
}

NDIS_STATUS RTMPWPARemoveKeyProc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PVOID			pBuf)
{
	PNDIS_802_11_REMOVE_KEY pKey;
	ULONG					KeyIdx;
	NDIS_STATUS 			Status = NDIS_STATUS_FAILURE;
	BOOLEAN 	bTxKey; 		// Set the key as transmit key
	BOOLEAN 	bPairwise;		// Indicate the key is pairwise key
	BOOLEAN 	bKeyRSC;		// indicate the receive  SC set by KeyRSC value.
								// Otherwise, it will set by the NIC.
	BOOLEAN 	bAuthenticator; // indicate key is set by authenticator.
	INT 		i;

	DBGPRINT(RT_DEBUG_TRACE,("---> RTMPWPARemoveKeyProc\n"));
	
	pKey = (PNDIS_802_11_REMOVE_KEY) pBuf;
	KeyIdx = pKey->KeyIndex & 0xff;
	// Bit 31 of Add-key, Tx Key
	bTxKey		   = (pKey->KeyIndex & 0x80000000) ? TRUE : FALSE;
	// Bit 30 of Add-key PairwiseKey
	bPairwise	   = (pKey->KeyIndex & 0x40000000) ? TRUE : FALSE;
	// Bit 29 of Add-key KeyRSC
	bKeyRSC 	   = (pKey->KeyIndex & 0x20000000) ? TRUE : FALSE;
	// Bit 28 of Add-key Authenticator
	bAuthenticator = (pKey->KeyIndex & 0x10000000) ? TRUE : FALSE;

	// 1. If bTx is TRUE, return failure information
	if (bTxKey == TRUE)
		return(NDIS_STATUS_INVALID_DATA);

	// 2. Check Pairwise Key
	if (bPairwise)
	{
		// a. If BSSID is broadcast, remove all pairwise keys.
		// b. If not broadcast, remove the pairwise specified by BSSID
		for (i = 0; i < SHARE_KEY_NUM; i++)
		{
			if (MAC_ADDR_EQUAL(pAd->SharedKey[BSS0][i].BssId, pKey->BSSID))
			{
				DBGPRINT(RT_DEBUG_TRACE,("RTMPWPARemoveKeyProc(KeyIdx=%d)\n", i));
				pAd->SharedKey[BSS0][i].KeyLen = 0;
				pAd->SharedKey[BSS0][i].CipherAlg = CIPHER_NONE;
				AsicRemoveSharedKeyEntry(pAd, BSS0, (UCHAR)i);
				Status = NDIS_STATUS_SUCCESS;
				break;
			}
		}
	}
	// 3. Group Key
	else
	{
		// a. If BSSID is broadcast, remove all group keys indexed
		// b. If BSSID matched, delete the group key indexed.
		DBGPRINT(RT_DEBUG_TRACE,("RTMPWPARemoveKeyProc(KeyIdx=%ld)\n", KeyIdx));
		pAd->SharedKey[BSS0][KeyIdx].KeyLen = 0;
		pAd->SharedKey[BSS0][KeyIdx].CipherAlg = CIPHER_NONE;
		AsicRemoveSharedKeyEntry(pAd, BSS0, (UCHAR)KeyIdx);
		Status = NDIS_STATUS_SUCCESS;
	}

	return (Status);
}
#endif // CONFIG_STA_SUPPORT //


#ifdef CONFIG_STA_SUPPORT
/*
	========================================================================
	
	Routine Description:
		Remove All WPA Keys

	Arguments:
		pAd 					Pointer to our adapter

	Return Value:
		None

	IRQL = DISPATCH_LEVEL
	
	Note:
		
	========================================================================
*/
VOID	RTMPWPARemoveAllKeys(
	IN	PRTMP_ADAPTER	pAd)
{

	UCHAR 	i;
	
	DBGPRINT(RT_DEBUG_TRACE,("RTMPWPARemoveAllKeys(AuthMode=%d, WepStatus=%d)\n", pAd->StaCfg.AuthMode, pAd->StaCfg.WepStatus));

	// For WEP/CKIP, there is no need to remove it, since WinXP won't set it again after
	// Link up. And it will be replaced if user changed it.
	if (pAd->StaCfg.AuthMode < Ndis802_11AuthModeWPA)
		return;

	// For WPA-None, there is no need to remove it, since WinXP won't set it again after
	// Link up. And it will be replaced if user changed it.
	if (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPANone)
		return;

	// set BSSID wcid entry of the Pair-wise Key table as no-security mode
	AsicRemovePairwiseKeyEntry(pAd, BSS0, BSSID_WCID);

	// set all shared key mode as no-security. 
	for (i = 0; i < SHARE_KEY_NUM; i++)
    {
		DBGPRINT(RT_DEBUG_TRACE,("remove %s key #%d\n", CipherName[pAd->SharedKey[BSS0][i].CipherAlg], i));
		NdisZeroMemory(&pAd->SharedKey[BSS0][i], sizeof(CIPHER_KEY));  						

		AsicRemoveSharedKeyEntry(pAd, BSS0, i);
	}

}
#endif // CONFIG_STA_SUPPORT //	

#if 0 // No supply currently for STA
/*
	========================================================================
	
	Routine Description:
		Construct and indicate WPA2 Media Specific Status

	Arguments:
		pAdapter	Pointer to our adapter

	Return Value:
		None

	IRQL = DISPATCH_LEVEL
	
	Note: only for STATION
		
	========================================================================
*/
VOID	RTMPIndicateWPA2Status(
	IN	PRTMP_ADAPTER	pAd)
{
	UCHAR 	apidx=0;

	struct
	{
		NDIS_802_11_STATUS_TYPE 			Status;
		NDIS_802_11_PMKID_CANDIDATE_LIST	List;
	}	Candidate;

	Candidate.Status = Ndis802_11StatusType_PMKID_CandidateList;
	Candidate.List.Version = 1;
	// This might need to be fixed to accomadate with current saved PKMIDs
	Candidate.List.NumCandidates = 1;
	NdisMoveMemory(&Candidate.List.CandidateList[0].BSSID, pAd->ApCfg.MBSSID[apidx].Bssid, 6);
	Candidate.List.CandidateList[0].Flags = 0;
	NdisMIndicateStatus(pAd->AdapterHandle, NDIS_STATUS_MEDIA_SPECIFIC_INDICATION, &Candidate, sizeof(Candidate));	
	DBGPRINT(RT_DEBUG_TRACE,("RTMPIndicateWPA2Status\n"));
}
#endif

#if 0
VOID	RTMPAddWcidCipher(
	IN	PRTMP_ADAPTER	pAd,
	IN	PNDIS_802_11_KEY	pKey,
	IN	UCHAR	CipherAlg,
	IN    UCHAR  Wcid)
{
	PUCHAR		pTxMic, pRxMic;
	BOOLEAN 	bKeyRSC, bAuthenticator;		// indicate the receive  SC set by KeyRSC value.
	UCHAR	i;
	ULONG		WCIDAttri;
	ULONG 	offset;
	UCHAR	KeyIdx, IVEIV[8];
	ULONG		Value;
	
	DBGPRINT(RT_DEBUG_TRACE, ("RTMPAddWcidCipher to Mactable[%d]==> \n", Wcid));
	
	// Bit 29 of Add-key KeyRSC
	bKeyRSC 	   = (pKey->KeyIndex & 0x20000000) ? TRUE : FALSE;
	// Bit 28 of Add-key Authenticator
	bAuthenticator = (pKey->KeyIndex & 0x10000000) ? TRUE : FALSE;
	KeyIdx = (UCHAR)pKey->KeyIndex&0xff;
	
	pAd->MacTab.Content[Wcid].PairwiseKey.KeyLen = (UCHAR)pKey->KeyLength;

	if (pAd->ApCfg.MBSSID[apidx].AuthMode == Ndis802_11AuthModeWPANone)
	{
		// for WPA-None Tx, Rx MIC is the same
		pTxMic = (PUCHAR) (&pKey->KeyMaterial) + 16;
		pRxMic = pTxMic;
	}
	else if (bAuthenticator == TRUE)
	{
		pTxMic = (PUCHAR) (&pKey->KeyMaterial) + 16;
		pRxMic = (PUCHAR) (&pKey->KeyMaterial) + 24;
	}
	else
	{
		pRxMic = (PUCHAR) (&pKey->KeyMaterial) + 16;
		pTxMic = (PUCHAR) (&pKey->KeyMaterial) + 24;
	}
	
	// 2. Record Security Key.
	NdisMoveMemory(pAd->MacTab.Content[Wcid].PairwiseKey.Key, &pKey->KeyMaterial, 16);
	if (pKey->KeyLength == LEN_TKIP_KEY)
	{
		// Only Key lenth equal to TKIP key have these
		NdisMoveMemory(pAd->MacTab.Content[Wcid].PairwiseKey.RxMic, pRxMic, 8);
		NdisMoveMemory(pAd->MacTab.Content[Wcid].PairwiseKey.TxMic, pTxMic, 8);
	}
	// 3. Check RxTsc. And used to init to ASIC IV. 
	if (bKeyRSC == TRUE)
		NdisMoveMemory(pAd->MacTab.Content[Wcid].PairwiseKey.RxTsc, &pKey->KeyRSC, 6);		   
	else
		NdisZeroMemory(pAd->MacTab.Content[Wcid].PairwiseKey.RxTsc, 6);	   

	// 4. Init TxTsc to one based on WiFi WPA specs
	pAd->MacTab.Content[Wcid].PairwiseKey.TxTsc[0] = 1;
	pAd->MacTab.Content[Wcid].PairwiseKey.TxTsc[1] = 0;
	pAd->MacTab.Content[Wcid].PairwiseKey.TxTsc[2] = 0;
	pAd->MacTab.Content[Wcid].PairwiseKey.TxTsc[3] = 0;
	pAd->MacTab.Content[Wcid].PairwiseKey.TxTsc[4] = 0;
	pAd->MacTab.Content[Wcid].PairwiseKey.TxTsc[5] = 0;
	

	offset = PAIRWISE_KEY_TABLE_BASE + (Wcid * HW_KEY_ENTRY_SIZE);
	for (i=0; i<pKey->KeyLength; i++)
	{
		RTMP_IO_WRITE8(pAd, offset+i, pKey->KeyMaterial[i]);
	}
	
	offset = PAIRWISE_KEY_TABLE_BASE + (Wcid * HW_KEY_ENTRY_SIZE) + 0x10;
	for (i=0; i<8; i++)
	{
		RTMP_IO_WRITE8(pAd, offset+i, *(pTxMic+i));
	}
	offset = PAIRWISE_KEY_TABLE_BASE + (Wcid * HW_KEY_ENTRY_SIZE) + 0x18;
	for (i=0; i<8; i++)
	{
		RTMP_IO_WRITE8(pAd, offset+i, *(pRxMic+i));
	}

	offset = MAC_IVEIV_TABLE_BASE + (Wcid * HW_IVEIV_ENTRY_SIZE);
	NdisZeroMemory(IVEIV, 8);
	// IV/EIV
	if ((CipherAlg == CIPHER_TKIP) || (CipherAlg == CIPHER_TKIP_NO_MIC) || (CipherAlg == CIPHER_AES))
	{
		IVEIV[3] = 0x20;	// Eiv bit on. keyid always 0 for pairwise key
	}
	// default key idx needs to set.   in TKIP/AES KeyIdx = 0 , WEP KeyIdx is default tx key.  
	{
		IVEIV[3] |= (KeyIdx<< 6);	
	}
	for (i=0; i<8; i++)
	{
		RTMP_IO_WRITE8(pAd, offset+i, IVEIV[i]);
	}

	// WCID Attribute UDF:3, BSSIdx:3, Alg:3,  BSSIdx is 0
	//always use pairwise key table. And softap always use BSSIdx=0 
	WCIDAttri = (CipherAlg<<1)|PAIRWISEKEYTABLE;
	offset = MAC_WCID_ATTRIBUTE_BASE + (Wcid* HW_WCID_ATTRI_SIZE);
	RTMP_IO_WRITE32(pAd, offset, WCIDAttri);
	RTMP_IO_READ32(pAd, offset, &Value);
	DBGPRINT(RT_DEBUG_TRACE, ("Wcid[%d] WCIDAttri : offset = %x, WCIDAttri = %x\n", Wcid, offset, WCIDAttri));
	
	DBGPRINT(RT_DEBUG_TRACE,("AddWcid[%d]: Alg=%s, KeyLength = %d\n",  Wcid, CipherName[CipherAlg], pKey->KeyLength));
	DBGPRINT(RT_DEBUG_TRACE,("	pKey->KeyIndex = %x\n", pKey->KeyIndex));
	DBGPRINT(RT_DEBUG_TRACE,("	Key =\n"));
	for (i=0; i<pKey->KeyLength; i++)
		DBGPRINT_RAW(RT_DEBUG_TRACE,(" %x:", pKey->KeyMaterial[i]));
	DBGPRINT(RT_DEBUG_TRACE,("	 \n"));
	DBGPRINT(RT_DEBUG_TRACE,("	TxMIC  = %02x:%02x:%02x:%02x:...\n", pTxMic[0],pTxMic[1],pTxMic[2],pTxMic[3]));
	DBGPRINT(RT_DEBUG_TRACE,("RxMic = %02x:%02x:%02x:%02x:...\n", pRxMic[0],pRxMic[1],pRxMic[2],pRxMic[3]));

}

#endif /* 0 */
/*
	========================================================================
	
	Routine Description:
	 	As STA's BSSID is a WC too, it uses shared key table.
	 	This function write correct unicast TX key to ASIC WCID.
 	 	And we still make a copy in our MacTab.Content[BSSID_WCID].PairwiseKey.
		Caller guarantee TKIP/AES always has keyidx = 0. (pairwise key)
		Caller guarantee WEP calls this function when set Txkey,  default key index=0~3.
		
	Arguments:
		pAd 					Pointer to our adapter
		pKey							Pointer to the where the key stored

	Return Value:
		NDIS_SUCCESS					Add key successfully

	IRQL = DISPATCH_LEVEL
	
	Note:
		
	========================================================================
*/
#if 0	// removed by AlbertY
VOID	RTMPAddBSSIDCipher(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR	Aid,
	IN	PNDIS_802_11_KEY	pKey,
	IN  UCHAR   CipherAlg)
{
	PUCHAR		pTxMic, pRxMic;
	BOOLEAN 	bKeyRSC, bAuthenticator;		// indicate the receive  SC set by KeyRSC value.
	UCHAR	    i;
	ULONG		WCIDAttri;
	ULONG 	offset;
	UCHAR	KeyIdx, IVEIV[8];
	ULONG		Value;
	UCHAR		apidx;
	
	DBGPRINT(RT_DEBUG_TRACE, ("RTMPAddBSSIDCipher==> Aid = %d\n",Aid));
	
	// Bit 29 of Add-key KeyRSC
	bKeyRSC 	   = (pKey->KeyIndex & 0x20000000) ? TRUE : FALSE;
	// Bit 28 of Add-key Authenticator
	bAuthenticator = (pKey->KeyIndex & 0x10000000) ? TRUE : FALSE;
	KeyIdx = (UCHAR)pKey->KeyIndex&0xff;
	
	if (KeyIdx > 4)
		return;
   	
	apidx = pAd->MacTab.Content[Aid].apidx;


	if (CipherAlg == CIPHER_TKIP)
	{	if (pAd->ApCfg.MBSSID[apidx].AuthMode == Ndis802_11AuthModeWPANone)
	{	
			// for WPA-None Tx, Rx MIC is the same
			pTxMic = (PUCHAR) (&pKey->KeyMaterial) + 16;
			pRxMic = pTxMic;
	}
		else if (bAuthenticator == TRUE)
	{	
			pTxMic = (PUCHAR) (&pKey->KeyMaterial) + 16;
			pRxMic = (PUCHAR) (&pKey->KeyMaterial) + 24;
	}
		else
	{
			pRxMic = (PUCHAR) (&pKey->KeyMaterial) + 16;
			pTxMic = (PUCHAR) (&pKey->KeyMaterial) + 24;
		}
		
		offset = PAIRWISE_KEY_TABLE_BASE + (Aid * HW_KEY_ENTRY_SIZE) + 0x10;
		for (i=0; i<8; )
		{
			Value = *(pTxMic+i);
			Value += (*(pTxMic+i+1)<<8);
			Value += (*(pTxMic+i+2)<<16);
			Value += (*(pTxMic+i+3)<<24);
			RTMP_IO_WRITE32(pAd, offset+i, Value);
			i+=4;
		}
		offset = PAIRWISE_KEY_TABLE_BASE + (Aid * HW_KEY_ENTRY_SIZE) + 0x18;
		for (i=0; i<8; )
		{
			Value = *(pRxMic+i);
			Value += (*(pRxMic+i+1)<<8);
			Value += (*(pRxMic+i+2)<<16);
			Value += (*(pRxMic+i+3)<<24);
			RTMP_IO_WRITE32(pAd, offset+i, Value);
			i+=4;
	}
		// Only Key lenth equal to TKIP key have these
		NdisMoveMemory(pAd->MacTab.Content[Aid].PairwiseKey.RxMic, pRxMic, 8);
		NdisMoveMemory(pAd->MacTab.Content[Aid].PairwiseKey.TxMic, pTxMic, 8);
		
		DBGPRINT(RT_DEBUG_TRACE,("	TxMIC = %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n", pTxMic[0],pTxMic[1],pTxMic[2],pTxMic[3], pTxMic[4],pTxMic[5],pTxMic[6],pTxMic[7]));
		DBGPRINT(RT_DEBUG_TRACE,("	RxMic = %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n", pRxMic[0],pRxMic[1],pRxMic[2],pRxMic[3], pRxMic[4],pRxMic[5],pRxMic[6],pRxMic[7]));

	}
	// 2. Record Security Key.
	pAd->MacTab.Content[BSSID_WCID].PairwiseKey.KeyLen= (UCHAR)pKey->KeyLength;
	NdisMoveMemory(pAd->MacTab.Content[Aid].PairwiseKey.Key, &pKey->KeyMaterial, pKey->KeyLength);
	pAd->MacTab.Content[Aid].PairwiseKey.CipherAlg = CipherAlg;
	// 3. Check RxTsc. And used to init to ASIC IV. 
	if (bKeyRSC == TRUE)
		NdisMoveMemory(pAd->MacTab.Content[Aid].PairwiseKey.RxTsc, &pKey->KeyRSC, 6);		   
	else
		NdisZeroMemory(pAd->MacTab.Content[Aid].PairwiseKey.RxTsc, 6);	   

	// 4. Init TxTsc to one based on WiFi WPA specs
	pAd->MacTab.Content[Aid].PairwiseKey.TxTsc[0] = 1;
	pAd->MacTab.Content[Aid].PairwiseKey.TxTsc[1] = 0;
	pAd->MacTab.Content[Aid].PairwiseKey.TxTsc[2] = 0;
	pAd->MacTab.Content[Aid].PairwiseKey.TxTsc[3] = 0;
	pAd->MacTab.Content[Aid].PairwiseKey.TxTsc[4] = 0;
	pAd->MacTab.Content[Aid].PairwiseKey.TxTsc[5] = 0;
	
	offset = PAIRWISE_KEY_TABLE_BASE + (Aid * HW_KEY_ENTRY_SIZE);
	for (i=0; i<pKey->KeyLength; i++)
	{
		RTMP_IO_WRITE8(pAd, offset+i, pKey->KeyMaterial[i]);
	}
	
	offset = SHARED_KEY_TABLE_BASE + (KeyIdx * HW_KEY_ENTRY_SIZE);	
	for(i = 0;i<pKey->KeyLength; i++)
	{
		RTMP_IO_WRITE8(pAd, offset+i, pKey->KeyMaterial[i]);
	}
	
	offset = MAC_IVEIV_TABLE_BASE + (Aid * HW_IVEIV_ENTRY_SIZE);
	NdisZeroMemory(IVEIV, 8);
	Value = 0;

	// IV/EIV
	if ((CipherAlg == CIPHER_TKIP) || (CipherAlg == CIPHER_TKIP_NO_MIC) || (CipherAlg == CIPHER_AES))
	{
		IVEIV[3] = 0x20;	// Eiv bit on. keyid always 0 for pairwise key
		Value = 0x20000000;
	}
	else // default key idx needs to set.   in TKIP/AES KeyIdx = 0 , WEP KeyIdx is default tx key.  
	{
		IVEIV[3] |= (KeyIdx<< 6);	
		Value += (KeyIdx<< 30);
	}
	RTMP_IO_WRITE32(pAd, offset, Value);
	Value = 0;
	RTMP_IO_WRITE32(pAd, (offset+4), Value);

	// WCID Attribute UDF:3, BSSIdx:3, Alg:3, Keytable:1=PAIRWISE KEY, BSSIdx is 0
	if ((CipherAlg == CIPHER_TKIP) || (CipherAlg == CIPHER_TKIP_NO_MIC) || (CipherAlg == CIPHER_AES))
		WCIDAttri = (CipherAlg<<1)|SHAREDKEYTABLE;
	else
		WCIDAttri = (CipherAlg<<1)|SHAREDKEYTABLE;
	
	offset = MAC_WCID_ATTRIBUTE_BASE + (Aid* HW_WCID_ATTRI_SIZE);
	RTMP_IO_WRITE32(pAd, offset, WCIDAttri);
	RTMP_IO_READ32(pAd, offset, &Value);
	DBGPRINT(RT_DEBUG_TRACE, ("BSSID_WCID : offset = %x, WCIDAttri = %x\n", offset, WCIDAttri));
	
	DBGPRINT(RT_DEBUG_TRACE,("AddBSSIDasWCIDEntry: Alg=%s, KeyLength = %d\n", CipherName[CipherAlg], pKey->KeyLength));
	DBGPRINT(RT_DEBUG_TRACE,("Key [idx=%x] [KeyLen = %d]\n", pKey->KeyIndex, pKey->KeyLength));
	for (i=0; i<pKey->KeyLength; i++)
		DBGPRINT_RAW(RT_DEBUG_TRACE,(" %x:", pKey->KeyMaterial[i]));
	DBGPRINT(RT_DEBUG_TRACE,("	 \n"));
}
#endif
/*
	========================================================================
	Routine Description:
		Change NIC PHY mode. Re-association may be necessary. possible settings
		include - PHY_11B, PHY_11BG_MIXED, PHY_11A, and PHY_11ABG_MIXED 

	Arguments:
		pAd - Pointer to our adapter
		phymode  - 

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL
	
	========================================================================
*/
VOID	RTMPSetPhyMode(
	IN	PRTMP_ADAPTER	pAd,
	IN	ULONG phymode)
{
	INT i;
	// the selected phymode must be supported by the RF IC encoded in E2PROM

	// if no change, do nothing
	/* bug fix
	if (pAd->CommonCfg.PhyMode == phymode)
		return;
    */
	pAd->CommonCfg.PhyMode = (UCHAR)phymode;

	DBGPRINT(RT_DEBUG_TRACE,("RTMPSetPhyMode : PhyMode=%d, channel=%d \n", pAd->CommonCfg.PhyMode, pAd->CommonCfg.Channel));
#ifdef EXT_BUILD_CHANNEL_LIST
	BuildChannelListEx(pAd);
#else
	BuildChannelList(pAd);
#endif // EXT_BUILD_CHANNEL_LIST //

	// sanity check user setting
	for (i = 0; i < pAd->ChannelListNum; i++)
	{
		if (pAd->CommonCfg.Channel == pAd->ChannelList[i].Channel)
			break;
	}

	if (i == pAd->ChannelListNum)
	{
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		if (pAd->CommonCfg.Channel != 0)
				pAd->CommonCfg.Channel = FirstChannel(pAd);
#endif // CONFIG_AP_SUPPORT //
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
			pAd->CommonCfg.Channel = FirstChannel(pAd);
#endif // CONFIG_STA_SUPPORT //
		DBGPRINT(RT_DEBUG_ERROR, ("RTMPSetPhyMode: channel is out of range, use first channel=%d \n", pAd->CommonCfg.Channel));
	}
	
	NdisZeroMemory(pAd->CommonCfg.SupRate, MAX_LEN_OF_SUPPORTED_RATES);
	NdisZeroMemory(pAd->CommonCfg.ExtRate, MAX_LEN_OF_SUPPORTED_RATES);
	NdisZeroMemory(pAd->CommonCfg.DesireRate, MAX_LEN_OF_SUPPORTED_RATES);
	switch (phymode) {
		case PHY_11B:
			pAd->CommonCfg.SupRate[0]  = 0x82;	  // 1 mbps, in units of 0.5 Mbps, basic rate
			pAd->CommonCfg.SupRate[1]  = 0x84;	  // 2 mbps, in units of 0.5 Mbps, basic rate
			pAd->CommonCfg.SupRate[2]  = 0x8B;	  // 5.5 mbps, in units of 0.5 Mbps, basic rate
			pAd->CommonCfg.SupRate[3]  = 0x96;	  // 11 mbps, in units of 0.5 Mbps, basic rate
			pAd->CommonCfg.SupRateLen  = 4;
			pAd->CommonCfg.ExtRateLen  = 0;
			pAd->CommonCfg.DesireRate[0]  = 2;	   // 1 mbps, in units of 0.5 Mbps
			pAd->CommonCfg.DesireRate[1]  = 4;	   // 2 mbps, in units of 0.5 Mbps
			pAd->CommonCfg.DesireRate[2]  = 11;    // 5.5 mbps, in units of 0.5 Mbps
			pAd->CommonCfg.DesireRate[3]  = 22;    // 11 mbps, in units of 0.5 Mbps
			//pAd->CommonCfg.HTPhyMode.field.MODE = MODE_CCK; // This MODE is only FYI. not use
			break;

		case PHY_11G:
		case PHY_11N_2_4G:
		case PHY_11BG_MIXED:
		case PHY_11ABG_MIXED:
		case PHY_11ABGN_MIXED:
		case PHY_11BGN_MIXED:
		case PHY_11GN_MIXED:
			pAd->CommonCfg.SupRate[0]  = 0x82;	  // 1 mbps, in units of 0.5 Mbps, basic rate
			pAd->CommonCfg.SupRate[1]  = 0x84;	  // 2 mbps, in units of 0.5 Mbps, basic rate
			pAd->CommonCfg.SupRate[2]  = 0x8B;	  // 5.5 mbps, in units of 0.5 Mbps, basic rate
			pAd->CommonCfg.SupRate[3]  = 0x96;	  // 11 mbps, in units of 0.5 Mbps, basic rate
			pAd->CommonCfg.SupRate[4]  = 0x12;	  // 9 mbps, in units of 0.5 Mbps
			pAd->CommonCfg.SupRate[5]  = 0x24;	  // 18 mbps, in units of 0.5 Mbps
			pAd->CommonCfg.SupRate[6]  = 0x48;	  // 36 mbps, in units of 0.5 Mbps
			pAd->CommonCfg.SupRate[7]  = 0x6c;	  // 54 mbps, in units of 0.5 Mbps
			pAd->CommonCfg.SupRateLen  = 8;
			pAd->CommonCfg.ExtRate[0]  = 0x0C;	  // 6 mbps, in units of 0.5 Mbps
			pAd->CommonCfg.ExtRate[1]  = 0x18;	  // 12 mbps, in units of 0.5 Mbps
			pAd->CommonCfg.ExtRate[2]  = 0x30;	  // 24 mbps, in units of 0.5 Mbps
			pAd->CommonCfg.ExtRate[3]  = 0x60;	  // 48 mbps, in units of 0.5 Mbps
			pAd->CommonCfg.ExtRateLen  = 4;
			pAd->CommonCfg.DesireRate[0]  = 2;	   // 1 mbps, in units of 0.5 Mbps
			pAd->CommonCfg.DesireRate[1]  = 4;	   // 2 mbps, in units of 0.5 Mbps
			pAd->CommonCfg.DesireRate[2]  = 11;    // 5.5 mbps, in units of 0.5 Mbps
			pAd->CommonCfg.DesireRate[3]  = 22;    // 11 mbps, in units of 0.5 Mbps
			pAd->CommonCfg.DesireRate[4]  = 12;    // 6 mbps, in units of 0.5 Mbps
			pAd->CommonCfg.DesireRate[5]  = 18;    // 9 mbps, in units of 0.5 Mbps
			pAd->CommonCfg.DesireRate[6]  = 24;    // 12 mbps, in units of 0.5 Mbps
			pAd->CommonCfg.DesireRate[7]  = 36;    // 18 mbps, in units of 0.5 Mbps
			pAd->CommonCfg.DesireRate[8]  = 48;    // 24 mbps, in units of 0.5 Mbps
			pAd->CommonCfg.DesireRate[9]  = 72;    // 36 mbps, in units of 0.5 Mbps
			pAd->CommonCfg.DesireRate[10] = 96;    // 48 mbps, in units of 0.5 Mbps
			pAd->CommonCfg.DesireRate[11] = 108;   // 54 mbps, in units of 0.5 Mbps
			break;

		case PHY_11A:
		case PHY_11AN_MIXED:
		case PHY_11AGN_MIXED:
		case PHY_11N_5G:
			pAd->CommonCfg.SupRate[0]  = 0x8C;	  // 6 mbps, in units of 0.5 Mbps, basic rate
			pAd->CommonCfg.SupRate[1]  = 0x12;	  // 9 mbps, in units of 0.5 Mbps
			pAd->CommonCfg.SupRate[2]  = 0x98;	  // 12 mbps, in units of 0.5 Mbps, basic rate
			pAd->CommonCfg.SupRate[3]  = 0x24;	  // 18 mbps, in units of 0.5 Mbps
			pAd->CommonCfg.SupRate[4]  = 0xb0;	  // 24 mbps, in units of 0.5 Mbps, basic rate
			pAd->CommonCfg.SupRate[5]  = 0x48;	  // 36 mbps, in units of 0.5 Mbps
			pAd->CommonCfg.SupRate[6]  = 0x60;	  // 48 mbps, in units of 0.5 Mbps
			pAd->CommonCfg.SupRate[7]  = 0x6c;	  // 54 mbps, in units of 0.5 Mbps
			pAd->CommonCfg.SupRateLen  = 8;
			pAd->CommonCfg.ExtRateLen  = 0;
			pAd->CommonCfg.DesireRate[0]  = 12;    // 6 mbps, in units of 0.5 Mbps
			pAd->CommonCfg.DesireRate[1]  = 18;    // 9 mbps, in units of 0.5 Mbps
			pAd->CommonCfg.DesireRate[2]  = 24;    // 12 mbps, in units of 0.5 Mbps
			pAd->CommonCfg.DesireRate[3]  = 36;    // 18 mbps, in units of 0.5 Mbps
			pAd->CommonCfg.DesireRate[4]  = 48;    // 24 mbps, in units of 0.5 Mbps
			pAd->CommonCfg.DesireRate[5]  = 72;    // 36 mbps, in units of 0.5 Mbps
			pAd->CommonCfg.DesireRate[6]  = 96;    // 48 mbps, in units of 0.5 Mbps
			pAd->CommonCfg.DesireRate[7]  = 108;   // 54 mbps, in units of 0.5 Mbps
			//pAd->CommonCfg.HTPhyMode.field.MODE = MODE_OFDM; // This MODE is only FYI. not use
			break;

		default:
			break;
	}

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		UINT	apidx;
		
		for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++)
		{
			MlmeUpdateTxRates(pAd, FALSE, apidx);
		}	
#ifdef WDS_SUPPORT
		for (apidx = 0; apidx < MAX_WDS_ENTRY; apidx++)
		{				
			MlmeUpdateTxRates(pAd, FALSE, apidx + MIN_NET_DEVICE_FOR_WDS);			
		}
#endif // WDS_SUPPORT //
#ifdef APCLI_SUPPORT
		for (apidx = 0; apidx < MAX_APCLI_NUM; apidx++)
		{				
			MlmeUpdateTxRates(pAd, FALSE, apidx + MIN_NET_DEVICE_FOR_APCLI);			
		}
#endif // APCLI_SUPPORT //		
	}
#endif // CONFIG_AP_SUPPORT //
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
#if 0 
		HTPhymode.PhyMode = pAd->CommonCfg.PhyMode;
		HTPhymode.TransmitNo = (UCHAR)pAd->Antenna.field.TxPath;
		HTPhymode.HtMode = (UCHAR)pAd->CommonCfg.RegTransmitSetting.field.HTMODE;
		HTPhymode.ExtOffset = (UCHAR)pAd->CommonCfg.RegTransmitSetting.field.EXTCHA;
    	HTPhymode.MCS = (UCHAR)pAd->StaCfg.DesiredTransmitSetting.field.MCS;
		HTPhymode.BW = (UCHAR)pAd->CommonCfg.RegTransmitSetting.field.BW;
		HTPhymode.STBC = (UCHAR)pAd->CommonCfg.RegTransmitSetting.field.STBC;
		HTPhymode.SHORTGI = (UCHAR)pAd->CommonCfg.RegTransmitSetting.field.ShortGI;

		pAd->CommonCfg.CentralChannel = pAd->CommonCfg.Channel;
		if (pAd->CommonCfg.PhyMode >= PHY_11ABGN_MIXED)
			RTMPSetHT(pAd, &HTPhymode);
#endif
		UCHAR	DesiredMCS = (UCHAR) pAd->StaCfg.DesiredTransmitSetting.field.MCS;

		pAd->CommonCfg.CentralChannel = pAd->CommonCfg.Channel;
        if ((DesiredMCS != MCS_AUTO) &&
			(pAd->StaCfg.BssType == BSS_ADHOC))
		{
			if (pAd->StaCfg.AdhocMode == ADHOC_11B)
                RTMPSetDesiredRates(pAd, (LONG) (RateIdToMbps[DesiredMCS] * 1000000));
			else
                RTMPSetDesiredRates(pAd, (LONG) (RateIdToMbps[DesiredMCS+4] * 1000000));
		}
		MlmeUpdateTxRates(pAd, FALSE, 0);
	}
#endif // CONFIG_STA_SUPPORT //

#if 0
    if (pAd->OpMode == OPMODE_STA)
        MlmeUpdateHtTxRates(pAd, BSSID_WCID);
    else
		MlmeUpdateHtTxRates(pAd, 0xff);
#endif	
	//AsicSetSlotTime(pAd, TRUE); //FALSE);

	pAd->CommonCfg.BandState = UNKNOWN_BAND;	
//	MakeIbssBeacon(pAd);	// supported rates may change
}



/*
	========================================================================
	Routine Description:
		Caller ensures we has 802.11n support.
		Calls at setting HT from AP/STASetinformation

	Arguments:
		pAd - Pointer to our adapter
		phymode  - 

	========================================================================
*/
VOID	RTMPSetHT(
	IN	PRTMP_ADAPTER	pAd,
	IN	OID_SET_HT_PHYMODE *pHTPhyMode)
{
	//ULONG	*pmcs;
	UINT32	Value = 0;
	UCHAR	BBPValue = 0;
	UCHAR	BBP3Value = 0;
	UCHAR	RxStream = pAd->CommonCfg.RxStream;
#ifdef CONFIG_AP_SUPPORT
	INT		apidx;
#endif // CONFIG_AP_SUPPORT //

	DBGPRINT(RT_DEBUG_TRACE, ("RTMPSetHT : HT_mode(%d), ExtOffset(%d), MCS(%d), BW(%d), STBC(%d), SHORTGI(%d)\n",
										pHTPhyMode->HtMode, pHTPhyMode->ExtOffset, 
										pHTPhyMode->MCS, pHTPhyMode->BW,
										pHTPhyMode->STBC, pHTPhyMode->SHORTGI));
			
	// Don't zero supportedHyPhy structure.
	RTMPZeroMemory(&pAd->CommonCfg.HtCapability, sizeof(pAd->CommonCfg.HtCapability));
	RTMPZeroMemory(&pAd->CommonCfg.AddHTInfo, sizeof(pAd->CommonCfg.AddHTInfo));
	RTMPZeroMemory(&pAd->CommonCfg.NewExtChanOffset, sizeof(pAd->CommonCfg.NewExtChanOffset));
	RTMPZeroMemory(&pAd->CommonCfg.DesiredHtPhy, sizeof(pAd->CommonCfg.DesiredHtPhy));

   	if (pAd->CommonCfg.bRdg)
	{
		pAd->CommonCfg.HtCapability.ExtHtCapInfo.PlusHTC = 1;
		pAd->CommonCfg.HtCapability.ExtHtCapInfo.RDGSupport = 1;
	}
	else
	{
		pAd->CommonCfg.HtCapability.ExtHtCapInfo.PlusHTC = 0;
		pAd->CommonCfg.HtCapability.ExtHtCapInfo.RDGSupport = 0;
	}

#if 0
	// pAd->CommonCfg.BASize is read from Registry.
	if (pAd->CommonCfg.BACapability.field.RxBAWinLimit & 0xc0)  //0x40 = 64
	{
	pAd->CommonCfg.HtCapability.HtCapParm.MaxRAmpduFactor = 3;
	pAd->CommonCfg.DesiredHtPhy.MaxRAmpduFactor = 3;
		//  Our BaSixe in TxWI definition is 6 bit. So Max setting is 63.
	}
	else if (pAd->CommonCfg.BACapability.field.RxBAWinLimit & 0x20)  // 0x20 = 32
	{
		pAd->CommonCfg.HtCapability.HtCapParm.MaxRAmpduFactor = 2;
		pAd->CommonCfg.DesiredHtPhy.MaxRAmpduFactor = 2;
	}
	else if (pAd->CommonCfg.BACapability.field.RxBAWinLimit & 0x10) // 0x10 = 16
	{
		pAd->CommonCfg.HtCapability.HtCapParm.MaxRAmpduFactor = 1;
		pAd->CommonCfg.DesiredHtPhy.MaxRAmpduFactor = 1;
	}
	else
	{
		pAd->CommonCfg.HtCapability.HtCapParm.MaxRAmpduFactor = 0;
		pAd->CommonCfg.DesiredHtPhy.MaxRAmpduFactor = 0;
	}
#else
	pAd->CommonCfg.HtCapability.HtCapParm.MaxRAmpduFactor = 3;
	pAd->CommonCfg.DesiredHtPhy.MaxRAmpduFactor = 3;
#endif

	DBGPRINT(RT_DEBUG_TRACE, ("RTMPSetHT : RxBAWinLimit = %d\n", pAd->CommonCfg.BACapability.field.RxBAWinLimit));

	// Mimo power save, A-MSDU size, 
	pAd->CommonCfg.DesiredHtPhy.AmsduEnable = (USHORT)pAd->CommonCfg.BACapability.field.AmsduEnable;
	pAd->CommonCfg.DesiredHtPhy.AmsduSize = (UCHAR)pAd->CommonCfg.BACapability.field.AmsduSize;
	pAd->CommonCfg.DesiredHtPhy.MimoPs = (UCHAR)pAd->CommonCfg.BACapability.field.MMPSmode;
	pAd->CommonCfg.DesiredHtPhy.MpduDensity = (UCHAR)pAd->CommonCfg.BACapability.field.MpduDensity;

	pAd->CommonCfg.HtCapability.HtCapInfo.AMsduSize = (USHORT)pAd->CommonCfg.BACapability.field.AmsduSize;
	pAd->CommonCfg.HtCapability.HtCapInfo.MimoPs = (USHORT)pAd->CommonCfg.BACapability.field.MMPSmode;
	pAd->CommonCfg.HtCapability.HtCapParm.MpduDensity = (UCHAR)pAd->CommonCfg.BACapability.field.MpduDensity;
	
	DBGPRINT(RT_DEBUG_TRACE, ("RTMPSetHT : AMsduSize = %d, MimoPs = %d, MpduDensity = %d, MaxRAmpduFactor = %d\n", 
													pAd->CommonCfg.DesiredHtPhy.AmsduSize, 
													pAd->CommonCfg.DesiredHtPhy.MimoPs,
													pAd->CommonCfg.DesiredHtPhy.MpduDensity,
													pAd->CommonCfg.DesiredHtPhy.MaxRAmpduFactor));
	
	if(pHTPhyMode->HtMode == HTMODE_GF)
	{
		pAd->CommonCfg.HtCapability.HtCapInfo.GF = 1;
		pAd->CommonCfg.DesiredHtPhy.GF = 1;
	}
	else
		pAd->CommonCfg.DesiredHtPhy.GF = 0;
	
	// Decide Rx MCSSet
	switch (RxStream)
	{
		case 1:			
			pAd->CommonCfg.HtCapability.MCSSet[0] =  0xff;
			pAd->CommonCfg.HtCapability.MCSSet[1] =  0x00;
			break;

		case 2:
		case 3:				
			pAd->CommonCfg.HtCapability.MCSSet[0] =  0xff;
			pAd->CommonCfg.HtCapability.MCSSet[1] =  0xff;
			break;
	}

	if (pAd->CommonCfg.bForty_Mhz_Intolerant && (pAd->CommonCfg.Channel <= 14) && (pHTPhyMode->BW == BW_40) )
	{
		pHTPhyMode->BW = BW_20;
		pAd->CommonCfg.HtCapability.HtCapInfo.Forty_Mhz_Intolerant = 1;
	}

	if(pHTPhyMode->BW == BW_40)
	{
		pAd->CommonCfg.HtCapability.MCSSet[4] = 0x1; // MCS 32
		pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth = 1;
		if (pAd->CommonCfg.Channel <= 14) 		
			pAd->CommonCfg.HtCapability.HtCapInfo.CCKmodein40 = 1;

		pAd->CommonCfg.DesiredHtPhy.ChannelWidth = 1;
		pAd->CommonCfg.AddHTInfo.AddHtInfo.RecomWidth = 1;
		pAd->CommonCfg.AddHTInfo.AddHtInfo.ExtChanOffset = (pHTPhyMode->ExtOffset == EXTCHA_BELOW)? (EXTCHA_BELOW): EXTCHA_ABOVE;
		// Set Regsiter for extension channel position.
		RTMP_IO_READ32(pAd, TX_BAND_CFG, &Value);
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BBP3Value);
		if ((pHTPhyMode->ExtOffset == EXTCHA_BELOW))
		{
			Value |= 0x1;
			BBP3Value |= (0x20);
			RTMP_IO_WRITE32(pAd, TX_BAND_CFG, Value);
		}
		else if ((pHTPhyMode->ExtOffset == EXTCHA_ABOVE))
		{
			Value &= 0xfe;
			BBP3Value &= (~0x20);
			RTMP_IO_WRITE32(pAd, TX_BAND_CFG, Value);
		} 
		// Turn on BBP 40MHz mode now only as AP . 
		// Sta can turn on BBP 40MHz after connection with 40MHz AP. Sta only broadcast 40MHz capability before connection.
		if ((pAd->OpMode == OPMODE_AP) || ADHOC_ON(pAd))
		{
			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &BBPValue);
			BBPValue &= (~0x18);
			BBPValue |= 0x10;
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, BBPValue);

			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BBP3Value);
			pAd->CommonCfg.BBPCurrentBW = BW_40;
		}
	}
	else
	{
		pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth = 0;
		pAd->CommonCfg.DesiredHtPhy.ChannelWidth = 0;
		pAd->CommonCfg.AddHTInfo.AddHtInfo.RecomWidth = 0;
		pAd->CommonCfg.AddHTInfo.AddHtInfo.ExtChanOffset = EXTCHA_NONE;
		pAd->CommonCfg.CentralChannel = pAd->CommonCfg.Channel;
		// Turn on BBP 20MHz mode by request here.
		{
			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &BBPValue);
			BBPValue &= (~0x18);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, BBPValue);
			pAd->CommonCfg.BBPCurrentBW = BW_20;
		}
	}
		
	if(pHTPhyMode->STBC == STBC_USE)
	{
		pAd->CommonCfg.HtCapability.HtCapInfo.TxSTBC = 1;
		pAd->CommonCfg.DesiredHtPhy.TxSTBC = 1;
		pAd->CommonCfg.HtCapability.HtCapInfo.RxSTBC = 1;
		pAd->CommonCfg.DesiredHtPhy.RxSTBC = 1;
	}
	else
	{
		pAd->CommonCfg.DesiredHtPhy.TxSTBC = 0;
		pAd->CommonCfg.DesiredHtPhy.RxSTBC = 0;
	}

	if(pHTPhyMode->SHORTGI == GI_400)
	{
		pAd->CommonCfg.HtCapability.HtCapInfo.ShortGIfor20 = 1;
		pAd->CommonCfg.HtCapability.HtCapInfo.ShortGIfor40 = 1;
		pAd->CommonCfg.DesiredHtPhy.ShortGIfor20 = 1;
		pAd->CommonCfg.DesiredHtPhy.ShortGIfor40 = 1;
	}
	else
	{
		pAd->CommonCfg.HtCapability.HtCapInfo.ShortGIfor20 = 0;
		pAd->CommonCfg.HtCapability.HtCapInfo.ShortGIfor40 = 0;
		pAd->CommonCfg.DesiredHtPhy.ShortGIfor20 = 0;
		pAd->CommonCfg.DesiredHtPhy.ShortGIfor40 = 0;
	}
	
	// We support link adaptation for unsolicit MCS feedback, set to 2.
	pAd->CommonCfg.HtCapability.ExtHtCapInfo.MCSFeedback = MCSFBK_NONE; //MCSFBK_UNSOLICIT;
	pAd->CommonCfg.AddHTInfo.ControlChan = pAd->CommonCfg.Channel;
	// 1, the extension channel above the control channel. 
	
#if 0 // remove by AlbertY	
	// Only for updating Registry transmit setting. However, real transmit setting is from OID setting or via negociation.
	pAd->CommonCfg.RegTransmitSetting.field.EXTCHA = pHTPhyMode->ExtOffset;
	pAd->CommonCfg.RegTransmitSetting.field.BW = pHTPhyMode->BW;
	pAd->CommonCfg.RegTransmitSetting.field.ShortGI = pHTPhyMode->SHORTGI;
	pAd->CommonCfg.RegTransmitSetting.field.STBC = pHTPhyMode->STBC;
	pAd->CommonCfg.RegTransmitSetting.field.HTMODE = pHTPhyMode->HtMode;
	pAd->CommonCfg.RegTransmitSetting.field.MCS = pHTPhyMode->MCS;
#endif
	
	// EDCA parameters used for AP's own transmission
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

		pAd->CommonCfg.APEdcaParm.Cwmax[0] = 6;
		pAd->CommonCfg.APEdcaParm.Cwmax[1] = 10;
		pAd->CommonCfg.APEdcaParm.Cwmax[2] = 4;
		pAd->CommonCfg.APEdcaParm.Cwmax[3] = 3;

		pAd->CommonCfg.APEdcaParm.Txop[0]  = 0;
		pAd->CommonCfg.APEdcaParm.Txop[1]  = 0;
		pAd->CommonCfg.APEdcaParm.Txop[2]  = 94;	
		pAd->CommonCfg.APEdcaParm.Txop[3]  = 47;	
	}
	AsicSetEdcaParm(pAd, &pAd->CommonCfg.APEdcaParm);
	
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++)
		{				
			RTMPSetIndividualHT(pAd, apidx);			
		}
#ifdef WDS_SUPPORT
		for (apidx = 0; apidx < MAX_WDS_ENTRY; apidx++)
		{				
			RTMPSetIndividualHT(pAd, apidx + MIN_NET_DEVICE_FOR_WDS);			
		}
#endif // WDS_SUPPORT //
#ifdef APCLI_SUPPORT
		for (apidx = 0; apidx < MAX_APCLI_NUM; apidx++)
		{				
			RTMPSetIndividualHT(pAd, apidx + MIN_NET_DEVICE_FOR_APCLI);			
		}
#endif // APCLI_SUPPORT //
	}
#endif // CONFIG_AP_SUPPORT //

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
		RTMPSetIndividualHT(pAd, 0xFF);				
		}
#endif // CONFIG_STA_SUPPORT //

#ifdef MESH_SUPPORT
	RTMPSetIndividualHT(pAd, MIN_NET_DEVICE_FOR_MESH);			
#endif // MESH_SUPPORT //
}

/*
	========================================================================
	Routine Description:
		Caller ensures we has 802.11n support.
		Calls at setting HT from AP/STASetinformation

	Arguments:
		pAd - Pointer to our adapter
		phymode  - 

	========================================================================
*/
VOID	RTMPSetIndividualHT(
	IN	PRTMP_ADAPTER		pAd,
	IN	UCHAR				apidx)
{	
	PRT_HT_PHY_INFO		pDesired_ht_phy = NULL;
	UCHAR	TxStream = pAd->CommonCfg.TxStream;		
	UCHAR	DesiredMcs	= MCS_AUTO;
						
	do
	{
#ifdef MESH_SUPPORT	
		if (apidx >= MIN_NET_DEVICE_FOR_MESH)
		{
			pDesired_ht_phy = &pAd->MeshTab.DesiredHtPhyInfo;									
			DesiredMcs = pAd->MeshTab.DesiredTransmitSetting.field.MCS;							
			pAd->MeshTab.bAutoTxRateSwitch = (DesiredMcs == MCS_AUTO) ? TRUE : FALSE;
			break;
		}
#endif // MESH_SUPPORT //

#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
#ifdef APCLI_SUPPORT	
			if (apidx >= MIN_NET_DEVICE_FOR_APCLI)
			{				
				UCHAR	idx = apidx - MIN_NET_DEVICE_FOR_APCLI;
						
				pDesired_ht_phy = &pAd->ApCfg.ApCliTab[idx].DesiredHtPhyInfo;									
				DesiredMcs = pAd->ApCfg.ApCliTab[idx].DesiredTransmitSetting.field.MCS;							
				pAd->ApCfg.ApCliTab[idx].bAutoTxRateSwitch = (DesiredMcs == MCS_AUTO) ? TRUE : FALSE;
					break;
			}
#endif // APCLI_SUPPORT //

#ifdef WDS_SUPPORT
			if (apidx >= MIN_NET_DEVICE_FOR_WDS)
			{				
				UCHAR	idx = apidx - MIN_NET_DEVICE_FOR_WDS;
						
				pDesired_ht_phy = &pAd->WdsTab.WdsEntry[idx].DesiredHtPhyInfo;									
				DesiredMcs = pAd->WdsTab.WdsEntry[idx].DesiredTransmitSetting.field.MCS;							
				pAd->WdsTab.WdsEntry[idx].bAutoTxRateSwitch = (DesiredMcs == MCS_AUTO) ? TRUE : FALSE;
				break;
			}
#endif // WDS_SUPPORT //
			if (apidx < pAd->ApCfg.BssidNum)
			{								
				pDesired_ht_phy = &pAd->ApCfg.MBSSID[apidx].DesiredHtPhyInfo;									
				DesiredMcs = pAd->ApCfg.MBSSID[apidx].DesiredTransmitSetting.field.MCS;			
				pAd->ApCfg.MBSSID[apidx].bWmmCapable = TRUE;	
				pAd->ApCfg.MBSSID[apidx].bAutoTxRateSwitch = (DesiredMcs == MCS_AUTO) ? TRUE : FALSE;
				break;
			}

			DBGPRINT(RT_DEBUG_ERROR, ("RTMPSetIndividualHT: invalid apidx(%d)\n", apidx));
			return;
		}			
#endif // CONFIG_AP_SUPPORT //
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{		
			pDesired_ht_phy = &pAd->StaCfg.DesiredHtPhyInfo;					
			DesiredMcs = pAd->StaCfg.DesiredTransmitSetting.field.MCS;
			pAd->StaCfg.bAutoTxRateSwitch = (DesiredMcs == MCS_AUTO) ? TRUE : FALSE;
				break;
		}	
#endif // CONFIG_STA_SUPPORT //
	} while (FALSE);

	if (pDesired_ht_phy == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("RTMPSetIndividualHT: invalid apidx(%d)\n", apidx));
		return;
	}
	RTMPZeroMemory(pDesired_ht_phy, sizeof(RT_HT_PHY_INFO));

	DBGPRINT(RT_DEBUG_TRACE, ("RTMPSetIndividualHT : Desired MCS = %d\n", DesiredMcs));
	// Check the validity of MCS 
	if ((TxStream == 1) && ((DesiredMcs >= MCS_8) && (DesiredMcs <= MCS_15)))
	{
		DBGPRINT(RT_DEBUG_WARN, ("RTMPSetIndividualHT: MCS(%d) is invalid in 1S, reset it as MCS_7\n", DesiredMcs));
		DesiredMcs = MCS_7;		
	}

	if ((pAd->CommonCfg.DesiredHtPhy.ChannelWidth == BW_20) && (DesiredMcs == MCS_32))
	{
		DBGPRINT(RT_DEBUG_WARN, ("RTMPSetIndividualHT: MCS_32 is only supported in 40-MHz, reset it as MCS_0\n"));
		DesiredMcs = MCS_0;		
	}
	   		
	pDesired_ht_phy->bHtEnable = TRUE;
					 
	// Decide desired Tx MCS
	switch (TxStream)
	{
		case 1:
			if (DesiredMcs == MCS_AUTO)
			{
				pDesired_ht_phy->MCSSet[0]= 0xff;
				pDesired_ht_phy->MCSSet[1]= 0x00;
			}
			else if (DesiredMcs <= MCS_7)
			{
				pDesired_ht_phy->MCSSet[0]= 1<<DesiredMcs;
				pDesired_ht_phy->MCSSet[1]= 0x00;
			}			
			break;

		case 2:
			if (DesiredMcs == MCS_AUTO)
			{
				pDesired_ht_phy->MCSSet[0]= 0xff;
				pDesired_ht_phy->MCSSet[1]= 0xff;
			}
			else if (DesiredMcs <= MCS_15)
			{
				ULONG mode;
				
				mode = DesiredMcs / 8;
				if (mode < 2)
					pDesired_ht_phy->MCSSet[mode] = (1 << (DesiredMcs - mode * 8));
			}			
			break;
	}							

	if(pAd->CommonCfg.DesiredHtPhy.ChannelWidth == BW_40)
	{
		if (DesiredMcs == MCS_AUTO || DesiredMcs == MCS_32)
			pDesired_ht_phy->MCSSet[4] = 0x1;
	}

	// update HT Rate setting				
    if (pAd->OpMode == OPMODE_STA)
        MlmeUpdateHtTxRates(pAd, BSS0);
    else
	    MlmeUpdateHtTxRates(pAd, apidx);

#if 0	// remove by AlbertY
	if (pHTPhyMode->MCS != MCS_AUTO)
	{		
		pTx_ht->field.STBC		= pHTPhyMode->STBC;
		pTx_ht->field.ShortGI	= pHTPhyMode->SHORTGI;
		pTx_ht->field.MCS		= pHTPhyMode->MCS;
		pTx_ht->field.MODE		= pHTPhyMode->HtMode;
		pTx_ht->field.BW		= pHTPhyMode->BW;
	
#ifdef CONFIG_STA_SUPPORT
		if (pAd->OpMode == OPMODE_STA)
		{									
			pAd->MacTab.Content[BSSID_WCID].HTPhyMode.field.STBC	= pTx_ht->field.STBC;
			pAd->MacTab.Content[BSSID_WCID].HTPhyMode.field.ShortGI	= pTx_ht->field.ShortGI;
			pAd->MacTab.Content[BSSID_WCID].HTPhyMode.field.MCS		= pTx_ht->field.MCS;
			pAd->MacTab.Content[BSSID_WCID].HTPhyMode.field.MODE	= pTx_ht->field.MODE;
		}
#endif // CONFIG_STA_SUPPORT //
	}
#endif 
}


/*
	========================================================================
	Routine Description:
		Update HT IE from our capability.
		
	Arguments:
		Send all HT IE in beacon/probe rsp/assoc rsp/action frame.
		
	
	========================================================================
*/
VOID	RTMPUpdateHTIE(
	IN	RT_HT_CAPABILITY	*pRtHt,
	IN		UCHAR				*pMcsSet,
	OUT		HT_CAPABILITY_IE *pHtCapability,
	OUT		ADD_HT_INFO_IE		*pAddHtInfo)
{
	RTMPZeroMemory(pHtCapability, sizeof(HT_CAPABILITY_IE));
	RTMPZeroMemory(pAddHtInfo, sizeof(ADD_HT_INFO_IE));
	
		pHtCapability->HtCapInfo.ChannelWidth = pRtHt->ChannelWidth;
		pHtCapability->HtCapInfo.MimoPs = pRtHt->MimoPs;
		pHtCapability->HtCapInfo.GF = pRtHt->GF;
		pHtCapability->HtCapInfo.ShortGIfor20 = pRtHt->ShortGIfor20;
		pHtCapability->HtCapInfo.ShortGIfor40 = pRtHt->ShortGIfor40;
		pHtCapability->HtCapInfo.TxSTBC = pRtHt->TxSTBC;
		pHtCapability->HtCapInfo.RxSTBC = pRtHt->RxSTBC;
		pHtCapability->HtCapInfo.AMsduSize = pRtHt->AmsduSize;
		pHtCapability->HtCapParm.MaxRAmpduFactor = pRtHt->MaxRAmpduFactor;
		pHtCapability->HtCapParm.MpduDensity = pRtHt->MpduDensity;

		pAddHtInfo->AddHtInfo.ExtChanOffset = pRtHt->ExtChanOffset ;
		pAddHtInfo->AddHtInfo.RecomWidth = pRtHt->RecomWidth;
		pAddHtInfo->AddHtInfo2.OperaionMode = pRtHt->OperaionMode;
		pAddHtInfo->AddHtInfo2.NonGfPresent = pRtHt->NonGfPresent;
		RTMPMoveMemory(pAddHtInfo->MCSSet, /*pRtHt->MCSSet*/pMcsSet, 4); // rt2860 only support MCS max=32, no need to copy all 16 uchar.
	
        DBGPRINT(RT_DEBUG_TRACE,("RTMPUpdateHTIE <== \n"));
}

/*
	========================================================================
	Description:
		Add Client security information into ASIC WCID table and IVEIV table.
    Return:
	========================================================================
*/
VOID	RTMPAddWcidAttributeEntry(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			BssIdx,
	IN 	UCHAR		 	KeyIdx,
	IN 	UCHAR		 	CipherAlg,
	IN 	MAC_TABLE_ENTRY *pEntry)
{
	UINT32		WCIDAttri = 0;
	USHORT		offset;
	UCHAR		IVEIV = 0;
	USHORT		Wcid = 0;
#ifdef CONFIG_AP_SUPPORT
	BOOLEAN		IEEE8021X = FALSE;
#endif // CONFIG_AP_SUPPORT //

#ifdef MESH_SUPPORT			
	if (BssIdx >= MIN_NET_DEVICE_FOR_MESH)
	{		
		if (pEntry)		
		{
			BssIdx -= MIN_NET_DEVICE_FOR_MESH;
			Wcid = pEntry->Aid;
		}	
		else
		{
			DBGPRINT(RT_DEBUG_WARN, ("RTMPAddWcidAttributeEntry: Mesh link doesn't need to set Group WCID Attribute. \n"));	
			return;
		}
	}	
	else
#endif // MESH_SUPPORT //
	{
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
#ifdef APCLI_SUPPORT
			if (BssIdx >= MIN_NET_DEVICE_FOR_APCLI)
			{
				if (pEntry)		
					BssIdx -= MIN_NET_DEVICE_FOR_APCLI;		
				else
				{
					DBGPRINT(RT_DEBUG_WARN, ("RTMPAddWcidAttributeEntry: AP-Client link doesn't need to set Group WCID Attribute. \n"));	
					return;
				}
			}	
			else 
#endif // APCLI_SUPPORT //
#ifdef WDS_SUPPORT
			if (BssIdx >= MIN_NET_DEVICE_FOR_WDS)
			{
				if (pEntry)		
					BssIdx = BSS0;		
				else
				{
					DBGPRINT(RT_DEBUG_WARN, ("RTMPAddWcidAttributeEntry: WDS link doesn't need to set Group WCID Attribute. \n"));	
					return;
				}
			}	
			else
#endif // WDS_SUPPORT //
			{
				if (BssIdx >= pAd->ApCfg.BssidNum)
				{
					DBGPRINT(RT_DEBUG_ERROR, ("RTMPAddWcidAttributeEntry: The BSS-index(%d) is out of range for MBSSID link. \n", BssIdx));	
					return;
				}
			}

			// choose wcid number
			if (pEntry)
				Wcid = pEntry->Aid;
			else
				GET_GroupKey_WCID(Wcid, BssIdx);		

			if (BssIdx < pAd->ApCfg.BssidNum)
				IEEE8021X = pAd->ApCfg.MBSSID[BssIdx].IEEE8021X;
		}
#endif // CONFIG_AP_SUPPORT //
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
			if (BssIdx > BSS0)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("RTMPAddWcidAttributeEntry: The BSS-index(%d) is out of range for Infra link. \n", BssIdx));	
				return;
			}

			// 1.	In ADHOC mode, the AID is wcid number. And NO mesh link exists.
			// 2.	In Infra mode, the AID:1 MUST be wcid of infra STA. 
			//					   the AID:2~ assign to mesh link entry. 	
			if (pEntry && ADHOC_ON(pAd))			
				Wcid = pEntry->Aid;
			else if (pEntry && INFRA_ON(pAd))
				Wcid = BSSID_WCID;
			else
				Wcid = MCAST_WCID;
		}
#endif // CONFIG_STA_SUPPORT //
	}

	// Update WCID attribute table
	offset = MAC_WCID_ATTRIBUTE_BASE + (Wcid * HW_WCID_ATTRI_SIZE);
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		// 1.	Wds-links and Mesh-links always use Pair-wise key table. 	
		// 2. 	When the CipherAlg is TKIP, AES or the dynamic WEP is enabled, 
		// 		it needs to set key into Pair-wise Key Table.
		// 3.	The pair-wise key security mode is set NONE, it means as no security.
		if (pEntry && (pEntry->ValidAsWDS || pEntry->ValidAsMesh))
			WCIDAttri = (BssIdx<<4) | (CipherAlg<<1) | PAIRWISEKEYTABLE;
		else if ((pEntry) && 
				((CipherAlg == CIPHER_TKIP) || 
				 (CipherAlg == CIPHER_TKIP_NO_MIC) || 
				 (CipherAlg == CIPHER_AES) || 
				 (CipherAlg == CIPHER_NONE) || 
				 (IEEE8021X == TRUE)))
			WCIDAttri = (BssIdx<<4) | (CipherAlg<<1) | PAIRWISEKEYTABLE;
		else
			WCIDAttri = (BssIdx<<4) | (CipherAlg<<1) | SHAREDKEYTABLE;
	}
#endif // CONFIG_AP_SUPPORT //

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		if (pEntry && pEntry->ValidAsMesh)
			WCIDAttri = (CipherAlg<<1) | PAIRWISEKEYTABLE;
		else
		WCIDAttri = (CipherAlg<<1) | SHAREDKEYTABLE;
	}
#endif // CONFIG_STA_SUPPORT //
		
	RTMP_IO_WRITE32(pAd, offset, WCIDAttri);

		
	// Update IV/EIV table
	offset = MAC_IVEIV_TABLE_BASE + (Wcid * HW_IVEIV_ENTRY_SIZE);

	// WPA mode
	if ((CipherAlg == CIPHER_TKIP) || (CipherAlg == CIPHER_TKIP_NO_MIC) || (CipherAlg == CIPHER_AES))
	{	
		// Eiv bit on. keyid always is 0 for pairwise key 			
		IVEIV = (KeyIdx <<6) | 0x20;	
	}	 
	else
	{
		// WEP KeyIdx is default tx key. 
		IVEIV = (KeyIdx << 6);	
	}

	// For key index and ext IV bit, so only need to update the position(offset+3).
#ifdef RT2860	
	RTMP_IO_WRITE8(pAd, offset+3, IVEIV);
#endif // RT2860 //
#ifdef RT2870
	RTUSBMultiWrite_OneByte(pAd, offset+3, &IVEIV);
#endif // RT2870 //
	
	DBGPRINT(RT_DEBUG_TRACE,("RTMPAddWcidAttributeEntry: WCID #%d, KeyIndex #%d, Alg=%s\n",Wcid, KeyIdx, CipherName[CipherAlg]));
	DBGPRINT(RT_DEBUG_TRACE,("	WCIDAttri = 0x%x \n",  WCIDAttri));	

}

/* 
    ==========================================================================
    Description:
        Parse encryption type
Arguments:
    pAdapter                    Pointer to our adapter
    wrq                         Pointer to the ioctl argument

    Return Value:
        None

    Note:
    ==========================================================================
*/
CHAR *GetEncryptType(CHAR enc)
{
    if(enc == Ndis802_11WEPDisabled)
        return "NONE";
    if(enc == Ndis802_11WEPEnabled)
    	return "WEP";
    if(enc == Ndis802_11Encryption2Enabled)
    	return "TKIP";
    if(enc == Ndis802_11Encryption3Enabled)
    	return "AES";
	if(enc == Ndis802_11Encryption4Enabled)
    	return "TKIPAES";
    else
    	return "UNKNOW";
}

CHAR *GetAuthMode(CHAR auth)
{
    if(auth == Ndis802_11AuthModeOpen)
    	return "OPEN";
    if(auth == Ndis802_11AuthModeShared)
    	return "SHARED";
	if(auth == Ndis802_11AuthModeAutoSwitch)
    	return "AUTOWEP";
    if(auth == Ndis802_11AuthModeWPA)
    	return "WPA";
    if(auth == Ndis802_11AuthModeWPAPSK)
    	return "WPAPSK";
    if(auth == Ndis802_11AuthModeWPANone)
    	return "WPANONE";
    if(auth == Ndis802_11AuthModeWPA2)
    	return "WPA2";
    if(auth == Ndis802_11AuthModeWPA2PSK)
    	return "WPA2PSK";
	if(auth == Ndis802_11AuthModeWPA1WPA2)
    	return "WPA1WPA2";
	if(auth == Ndis802_11AuthModeWPA1PSKWPA2PSK)
    	return "WPA1PSKWPA2PSK";
    else
    	return "UNKNOW";
}		

#if 1 //#ifndef UCOS
/* 
    ==========================================================================
    Description:
        Get site survey results
	Arguments:
	    pAdapter                    Pointer to our adapter
	    wrq                         Pointer to the ioctl argument

    Return Value:
        None

    Note:
        Usage:
        		1.) UI needs to wait 4 seconds after issue a site survey command
        		2.) iwpriv ra0 get_site_survey
        		3.) UI needs to prepare at least 4096bytes to get the results
    ==========================================================================
*/
#define	LINE_LEN	(4+33+20+8+10+9+7+3)	// Channel+SSID+Bssid+WepStatus+AuthMode+Signal+WiressMode+NetworkType
#ifdef CONFIG_STA_SUPPORT
#ifdef WSC_STA_SUPPORT
#define	LINE_LEN2	(4+33+20+8+10+9+7+3+4)	// Channel+SSID+Bssid+WepStatus+AuthMode+Signal+WiressMode+NetworkType+WPS
#endif // WSC_STA_SUPPORT //
#endif // CONFIG_STA_SUPPORT //
VOID RTMPIoctlGetSiteSurvey(
	IN	PRTMP_ADAPTER	pAdapter, 
	IN	struct iwreq	*wrq)
{
	CHAR		*msg;
	INT 		i=0;	 
	INT 		Status=0;
	CHAR		Ssid[MAX_LEN_OF_SSID +1];
    INT         Rssi = 0, max_len = LINE_LEN;
	UINT        Rssi_Quality = 0;
	NDIS_802_11_NETWORK_TYPE    wireless_mode;

#ifdef CONFIG_STA_SUPPORT
#ifdef WSC_STA_SUPPORT
    max_len = LINE_LEN2;
#endif // WSC_STA_SUPPORT //
#endif // CONFIG_STA_SUPPORT //

	os_alloc_mem(NULL, (PUCHAR *)&msg, sizeof(CHAR)*((MAX_LEN_OF_BSS_TABLE)*max_len));

	if (msg == NULL)
	{   
		DBGPRINT(RT_DEBUG_TRACE, ("RTMPIoctlGetSiteSurvey - msg memory alloc fail.\n"));
		return;
	}

	memset(msg, 0 ,(MAX_LEN_OF_BSS_TABLE)*max_len );
	memset(Ssid, 0 ,(MAX_LEN_OF_SSID +1));
	sprintf(msg,"%s","\n");
	sprintf(msg+strlen(msg),"%-4s%-33s%-20s%-8s%-10s%-9s%-7s%-3s\n",
	    "Ch", "SSID", "BSSID", "Enc", "Auth", "Siganl(%)", "W-Mode", " NT");
	
#ifdef CONFIG_STA_SUPPORT
#ifdef WSC_STA_SUPPORT
    sprintf(msg+strlen(msg)-1,"%-4s\n", " WPS");
#endif // WSC_STA_SUPPORT //
#endif // CONFIG_STA_SUPPORT //

	for(i=0;i<MAX_LEN_OF_BSS_TABLE;i++)
	{
		if( pAdapter->ScanTab.BssEntry[i].Channel==0)
			break;

		if((strlen(msg)+max_len ) >= IW_SCAN_MAX_DATA)
			break;

		//Channel
		sprintf(msg+strlen(msg),"%-4d", pAdapter->ScanTab.BssEntry[i].Channel);
		//SSID
		memcpy(Ssid, pAdapter->ScanTab.BssEntry[i].Ssid, pAdapter->ScanTab.BssEntry[i].SsidLen);
		Ssid[pAdapter->ScanTab.BssEntry[i].SsidLen] = '\0';
		sprintf(msg+strlen(msg),"%-33s", Ssid);      
		//BSSID
		sprintf(msg+strlen(msg),"%02x:%02x:%02x:%02x:%02x:%02x   ", 
			pAdapter->ScanTab.BssEntry[i].Bssid[0], 
			pAdapter->ScanTab.BssEntry[i].Bssid[1],
			pAdapter->ScanTab.BssEntry[i].Bssid[2], 
			pAdapter->ScanTab.BssEntry[i].Bssid[3], 
			pAdapter->ScanTab.BssEntry[i].Bssid[4], 
			pAdapter->ScanTab.BssEntry[i].Bssid[5]);
		//Encryption Type
		sprintf(msg+strlen(msg),"%-8s",GetEncryptType(pAdapter->ScanTab.BssEntry[i].WepStatus));
		//Authentication Mode
		sprintf(msg+strlen(msg),"%-10s",GetAuthMode(pAdapter->ScanTab.BssEntry[i].AuthMode));
		// Rssi
		Rssi = (INT)(pAdapter->ScanTab.BssEntry[i].Rssi - pAdapter->BbpRssiToDbmDelta);
		if (Rssi >= -50)
			Rssi_Quality = 100;
		else if (Rssi >= -80)    // between -50 ~ -80dbm
			Rssi_Quality = (UINT)(24 + ((Rssi + 80) * 26)/10);
		else if (Rssi >= -90)   // between -80 ~ -90dbm
			Rssi_Quality = (UINT)(((Rssi + 90) * 26)/10);
		else    // < -84 dbm
			Rssi_Quality = 0;
		sprintf(msg+strlen(msg),"%-9d", Rssi_Quality);
		// Wireless Mode
		wireless_mode = NetworkTypeInUseSanity(&pAdapter->ScanTab.BssEntry[i]);
		if (wireless_mode == Ndis802_11FH ||
			wireless_mode == Ndis802_11DS)
			sprintf(msg+strlen(msg),"%-7s", "11b");
		else if (wireless_mode == Ndis802_11OFDM5)
			sprintf(msg+strlen(msg),"%-7s", "11a");
		else if (wireless_mode == Ndis802_11OFDM5_N)
			sprintf(msg+strlen(msg),"%-7s", "11a/n");
		else if (wireless_mode == Ndis802_11OFDM24)
			sprintf(msg+strlen(msg),"%-7s", "11b/g");
		else if (wireless_mode == Ndis802_11OFDM24_N)
			sprintf(msg+strlen(msg),"%-7s", "11b/g/n");
		else
			sprintf(msg+strlen(msg),"%-7s", "unknow");
		//Network Type		
		if (pAdapter->ScanTab.BssEntry[i].BssType == BSS_ADHOC)
			sprintf(msg+strlen(msg),"%-3s", " Ad");
		else
			sprintf(msg+strlen(msg),"%-3s", " In");

        sprintf(msg+strlen(msg),"\n");
#ifdef CONFIG_STA_SUPPORT
#ifdef WSC_STA_SUPPORT
        //WPS
        if (pAdapter->ScanTab.BssEntry[i].bWpsAP)
			sprintf(msg+strlen(msg)-1,"%-4s\n", " YES");
		else
			sprintf(msg+strlen(msg)-1,"%-4s\n", "  NO");
#endif // WSC_STA_SUPPORT //
#endif // CONFIG_STA_SUPPORT //
	}

	wrq->u.data.length = strlen(msg);
	Status = copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length);

	DBGPRINT(RT_DEBUG_TRACE, ("RTMPIoctlGetSiteSurvey - wrq->u.data.length = %d\n", wrq->u.data.length));
	os_free_mem(NULL, (PUCHAR)msg);
}


#define	MAC_LINE_LEN	(14+4+4+10+10+10+6+6)	// Addr+aid+psm+datatime+rxbyte+txbyte+current tx rate+last tx rate
VOID RTMPIoctlGetMacTable(
	IN PRTMP_ADAPTER pAd, 
	IN struct iwreq *wrq)
{
	INT i;
	RT_802_11_MAC_TABLE MacTab;
	char *msg;

	MacTab.Num = 0;
	for (i=0; i<MAX_LEN_OF_MAC_TABLE; i++)
	{
		if (pAd->MacTab.Content[i].ValidAsCLI && (pAd->MacTab.Content[i].Sst == SST_ASSOC))
		{
			COPY_MAC_ADDR(MacTab.Entry[MacTab.Num].Addr, &pAd->MacTab.Content[i].Addr);
			MacTab.Entry[MacTab.Num].Aid = (UCHAR)pAd->MacTab.Content[i].Aid;
			MacTab.Entry[MacTab.Num].Psm = pAd->MacTab.Content[i].PsMode;
			MacTab.Entry[MacTab.Num].MimoPs = pAd->MacTab.Content[i].MmpsMode;
#if 0 // ToDo
			MacTab.Entry[MacTab.Num].HSCounter.LastDataPacketTime = pAd->MacTab.Content[i].HSCounter.LastDataPacketTime;
			MacTab.Entry[MacTab.Num].HSCounter.TotalRxByteCount = pAd->MacTab.Content[i].HSCounter.TotalRxByteCount;
			MacTab.Entry[MacTab.Num].HSCounter.TotalTxByteCount = pAd->MacTab.Content[i].HSCounter.TotalTxByteCount;
#endif
			MacTab.Entry[MacTab.Num].TxRate.field.MCS = pAd->MacTab.Content[i].HTPhyMode.field.MCS;
			MacTab.Entry[MacTab.Num].TxRate.field.BW = pAd->MacTab.Content[i].HTPhyMode.field.BW;
			MacTab.Entry[MacTab.Num].TxRate.field.ShortGI = pAd->MacTab.Content[i].HTPhyMode.field.ShortGI;
			MacTab.Entry[MacTab.Num].TxRate.field.STBC = pAd->MacTab.Content[i].HTPhyMode.field.STBC;
			MacTab.Entry[MacTab.Num].TxRate.field.rsv = pAd->MacTab.Content[i].HTPhyMode.field.rsv;
			MacTab.Entry[MacTab.Num].TxRate.field.MODE = pAd->MacTab.Content[i].HTPhyMode.field.MODE;
			MacTab.Entry[MacTab.Num].TxRate.word = pAd->MacTab.Content[i].HTPhyMode.word;
			MacTab.Num += 1;
		}
	}
	wrq->u.data.length = sizeof(RT_802_11_MAC_TABLE);
	if (copy_to_user(wrq->u.data.pointer, &MacTab, wrq->u.data.length))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s: copy_to_user() fail\n", __FUNCTION__));
	}

	msg = (CHAR *) kmalloc(sizeof(CHAR)*(MAX_LEN_OF_MAC_TABLE*MAC_LINE_LEN), MEM_ALLOC_FLAG);
	memset(msg, 0 ,MAX_LEN_OF_MAC_TABLE*MAC_LINE_LEN );
	sprintf(msg,"%s","\n");
	sprintf(msg+strlen(msg),"%-14s%-4s%-4s%-10s%-10s%-10s%-6s%-6s\n",
		"MAC", "AID", "PSM", "LDT", "RxB", "TxB","CTxR", "LTxR");
	
	for (i=0; i<MAX_LEN_OF_MAC_TABLE; i++)
	{
		PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[i];
		if (pEntry->ValidAsCLI && (pEntry->Sst == SST_ASSOC))
		{
			if((strlen(msg)+MAC_LINE_LEN ) >= (MAX_LEN_OF_MAC_TABLE*MAC_LINE_LEN) )
				break;	
			sprintf(msg+strlen(msg),"%02x%02x%02x%02x%02x%02x  ",
				pEntry->Addr[0], pEntry->Addr[1], pEntry->Addr[2],
				pEntry->Addr[3], pEntry->Addr[4], pEntry->Addr[5]);
			sprintf(msg+strlen(msg),"%-4d", (int)pEntry->Aid);
			sprintf(msg+strlen(msg),"%-4d", (int)pEntry->PsMode);
			sprintf(msg+strlen(msg),"%-10d",0/*pAd->MacTab.Content[i].HSCounter.LastDataPacketTime*/); // ToDo
			sprintf(msg+strlen(msg),"%-10d",0/*pAd->MacTab.Content[i].HSCounter.TotalRxByteCount*/); // ToDo
			sprintf(msg+strlen(msg),"%-10d",0/*pAd->MacTab.Content[i].HSCounter.TotalTxByteCount*/); // ToDo
			sprintf(msg+strlen(msg),"%-6d",RateIdToMbps[pAd->MacTab.Content[i].CurrTxRate]);
			sprintf(msg+strlen(msg),"%-6d\n",0/*RateIdToMbps[pAd->MacTab.Content[i].LastTxRate]*/); // ToDo
		}
	} 
	// for compatible with old API just do the printk to console
	//wrq->u.data.length = strlen(msg);
	//if (copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s", msg));
	}

	kfree(msg);
}
#endif // UCOS //

INT	Set_BASetup_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PUCHAR			arg)
{
    UCHAR mac[6], tid;
	char *token, sepValue[] = ":", DASH = '-';
	INT i;
    MAC_TABLE_ENTRY *pEntry;

/*
	The BASetup inupt string format should be xx:xx:xx:xx:xx:xx-d, 
		=>The six 2 digit hex-decimal number previous are the Mac address, 
		=>The seventh decimal number is the tid value.
*/
	//printk("\n%s\n", arg);
	
	if(strlen(arg) < 19)  //Mac address acceptable format 01:02:03:04:05:06 length 17 plus the "-" and tid value in decimal format.
		return FALSE;

	token = strchr(arg, DASH);
	if ((token != NULL) && (strlen(token)>1))
	{
		tid = simple_strtol((token+1), 0, 10);
		if (tid > 15)
			return FALSE;
		
		*token = '\0';
		for (i = 0, token = rstrtok(arg, &sepValue[0]); token; token = rstrtok(NULL, &sepValue[0]), i++)
		{
			if((strlen(token) != 2) || (!isxdigit(*token)) || (!isxdigit(*(token+1))))
				return FALSE;
			AtoH(token, (PUCHAR)(&mac[i]), 1);
		}
		if(i != 6)
			return FALSE;

		printk("\n%02x:%02x:%02x:%02x:%02x:%02x-%02x\n", mac[0], mac[1],
				mac[2], mac[3], mac[4], mac[5], tid);

	    pEntry = MacTableLookup(pAd, mac);

    	if (pEntry) {
        	printk("\nSetup BA Session: Tid = %d\n", tid);
	        BAOriSessionSetUp(pAd, pEntry, tid, 0, 100, TRUE);
    	}

		return TRUE;
	}

	return FALSE;

}

INT	Set_BADecline_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PUCHAR			arg)
{
	ULONG bBADecline;

	bBADecline = simple_strtol(arg, 0, 10);

	if (bBADecline == 0)
	{
		pAd->CommonCfg.bBADecline = FALSE;
	}
	else if (bBADecline == 1)
	{
		pAd->CommonCfg.bBADecline = TRUE;
	}
	else 
	{
		return FALSE; //Invalid argument
	}

	DBGPRINT(RT_DEBUG_TRACE, ("Set_BADecline_Proc::(BADecline=%d)\n", pAd->CommonCfg.bBADecline));

	return TRUE;
}

INT	Set_BAOriTearDown_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PUCHAR			arg)
{
    UCHAR mac[6], tid;
	char *token, sepValue[] = ":", DASH = '-';
	INT i;
    MAC_TABLE_ENTRY *pEntry;

    //printk("\n%s\n", arg);
/*
	The BAOriTearDown inupt string format should be xx:xx:xx:xx:xx:xx-d, 
		=>The six 2 digit hex-decimal number previous are the Mac address, 
		=>The seventh decimal number is the tid value.
*/
    if(strlen(arg) < 19)  //Mac address acceptable format 01:02:03:04:05:06 length 17 plus the "-" and tid value in decimal format.
		return FALSE;

	token = strchr(arg, DASH);
	if ((token != NULL) && (strlen(token)>1))
	{
		tid = simple_strtol((token+1), 0, 10);
		if (tid > 15)
			return FALSE;
		
		*token = '\0';
		for (i = 0, token = rstrtok(arg, &sepValue[0]); token; token = rstrtok(NULL, &sepValue[0]), i++)
		{
			if((strlen(token) != 2) || (!isxdigit(*token)) || (!isxdigit(*(token+1))))
				return FALSE;
			AtoH(token, (PUCHAR)(&mac[i]), 1);
		}
		if(i != 6)
			return FALSE;

	    printk("\n%02x:%02x:%02x:%02x:%02x:%02x-%02x", mac[0], mac[1],
	           mac[2], mac[3], mac[4], mac[5], tid);

	    pEntry = MacTableLookup(pAd, mac);

	    if (pEntry) {
	        printk("\nTear down Ori BA Session: Tid = %d\n", tid);
        BAOriSessionTearDown(pAd, pEntry->Aid, tid, FALSE, TRUE);
	    }

		return TRUE;
	}

	return FALSE;

}

INT	Set_BARecTearDown_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PUCHAR			arg)
{
    UCHAR mac[6], tid;
	char *token, sepValue[] = ":", DASH = '-';
	INT i;
    MAC_TABLE_ENTRY *pEntry;

    //printk("\n%s\n", arg);
/*
	The BARecTearDown inupt string format should be xx:xx:xx:xx:xx:xx-d, 
		=>The six 2 digit hex-decimal number previous are the Mac address, 
		=>The seventh decimal number is the tid value.
*/
    if(strlen(arg) < 19)  //Mac address acceptable format 01:02:03:04:05:06 length 17 plus the "-" and tid value in decimal format.
		return FALSE;

	token = strchr(arg, DASH);
	if ((token != NULL) && (strlen(token)>1))
	{
		tid = simple_strtol((token+1), 0, 10);
		if (tid > 15)
			return FALSE;
		
		*token = '\0';
		for (i = 0, token = rstrtok(arg, &sepValue[0]); token; token = rstrtok(NULL, &sepValue[0]), i++)
		{
			if((strlen(token) != 2) || (!isxdigit(*token)) || (!isxdigit(*(token+1))))
				return FALSE;
			AtoH(token, (PUCHAR)(&mac[i]), 1);
		}
		if(i != 6)
			return FALSE;

		printk("\n%02x:%02x:%02x:%02x:%02x:%02x-%02x", mac[0], mac[1],
		       mac[2], mac[3], mac[4], mac[5], tid);

		pEntry = MacTableLookup(pAd, mac);

		if (pEntry) {
		    printk("\nTear down Rec BA Session: Tid = %d\n", tid);
		    BARecSessionTearDown(pAd, pEntry->Aid, tid, FALSE);
		}

		return TRUE;
	}

	return FALSE;

}

INT	Set_HtBw_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PUCHAR			arg)
{
	ULONG HtBw;

	HtBw = simple_strtol(arg, 0, 10);
	if (HtBw == BW_40)
		pAd->CommonCfg.RegTransmitSetting.field.BW  = BW_40;
	else if (HtBw == BW_20)
		pAd->CommonCfg.RegTransmitSetting.field.BW  = BW_20;
	else
		return FALSE;  //Invalid argument 

	SetCommonHT(pAd);
	
	DBGPRINT(RT_DEBUG_TRACE, ("Set_HtBw_Proc::(HtBw=%d)\n", pAd->CommonCfg.RegTransmitSetting.field.BW));

	return TRUE;
}

INT	Set_HtMcs_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PUCHAR			arg)
{
	ULONG HtMcs, Mcs_tmp;
#ifdef CONFIG_AP_SUPPORT    
    POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
#endif // CONFIG_AP_SUPPORT //	
#ifdef CONFIG_STA_SUPPORT    
    BOOLEAN bAutoRate = FALSE;
#endif // CONFIG_STA_SUPPORT //

	Mcs_tmp = simple_strtol(arg, 0, 10);
		
	if (Mcs_tmp <= 15 || Mcs_tmp == 32)			
		HtMcs = Mcs_tmp;	
	else
		HtMcs = MCS_AUTO;	

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		pAd->ApCfg.MBSSID[pObj->ioctl_if].DesiredTransmitSetting.field.MCS = HtMcs;
		DBGPRINT(RT_DEBUG_TRACE, ("Set_HtMcs_Proc::(HtMcs=%d) for ra%d\n", 
				pAd->ApCfg.MBSSID[pObj->ioctl_if].DesiredTransmitSetting.field.MCS, pObj->ioctl_if));
	}
#endif // CONFIG_AP_SUPPORT //
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		pAd->StaCfg.DesiredTransmitSetting.field.MCS = HtMcs;
		DBGPRINT(RT_DEBUG_TRACE, ("Set_HtMcs_Proc::(HtMcs=%d)\n", pAd->StaCfg.DesiredTransmitSetting.field.MCS));

		if ((pAd->CommonCfg.PhyMode < PHY_11ABGN_MIXED) ||
			(pAd->MacTab.Content[BSSID_WCID].HTPhyMode.field.MODE < MODE_HTMIX))
		{
	        if ((pAd->StaCfg.DesiredTransmitSetting.field.MCS != MCS_AUTO) &&
				(HtMcs >= 0 && HtMcs <= 3) &&
	            (pAd->StaCfg.DesiredTransmitSetting.field.FixedTxMode == FIXED_TXMODE_CCK))
			{
				RTMPSetDesiredRates(pAd, (LONG) (RateIdToMbps[HtMcs] * 1000000));
			}
	        else if ((pAd->StaCfg.DesiredTransmitSetting.field.MCS != MCS_AUTO) &&
					(HtMcs >= 0 && HtMcs <= 7) &&
	            	(pAd->StaCfg.DesiredTransmitSetting.field.FixedTxMode == FIXED_TXMODE_OFDM))
			{
				RTMPSetDesiredRates(pAd, (LONG) (RateIdToMbps[HtMcs+4] * 1000000));
			}
			else
				bAutoRate = TRUE;

			if (bAutoRate)
			{
	            pAd->StaCfg.DesiredTransmitSetting.field.MCS = MCS_AUTO;
				RTMPSetDesiredRates(pAd, -1);
			}
	        DBGPRINT(RT_DEBUG_TRACE, ("Set_HtMcs_Proc::(FixedTxMode=%d)\n",pAd->StaCfg.DesiredTransmitSetting.field.FixedTxMode));
		}
        if (ADHOC_ON(pAd))
            return TRUE;
	}
#endif // CONFIG_STA_SUPPORT //

	SetCommonHT(pAd);
	
	return TRUE;
}

INT	Set_HtGi_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PUCHAR			arg)
{
	ULONG HtGi;

	HtGi = simple_strtol(arg, 0, 10);
		
	if ( HtGi == GI_400)			
		pAd->CommonCfg.RegTransmitSetting.field.ShortGI = GI_400;
	else if ( HtGi == GI_800 )
		pAd->CommonCfg.RegTransmitSetting.field.ShortGI = GI_800;
	else 
		return FALSE; //Invalid argument 	

	SetCommonHT(pAd);
	
	DBGPRINT(RT_DEBUG_TRACE, ("Set_HtGi_Proc::(ShortGI=%d)\n",pAd->CommonCfg.RegTransmitSetting.field.ShortGI));

	return TRUE;
}


INT	Set_HtTxBASize_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PUCHAR			arg)
{
	UCHAR Size;

	Size = simple_strtol(arg, 0, 10);
		
	if (Size <=0 || Size >=64)
	{
		Size = 8;
	}
	pAd->CommonCfg.TxBASize = Size-1;
	DBGPRINT(RT_DEBUG_ERROR, ("Set_HtTxBASize ::(TxBASize= %d)\n", Size));

	return TRUE;
}


INT	Set_HtOpMode_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PUCHAR			arg)
{

	ULONG Value;
	
	Value = simple_strtol(arg, 0, 10);

	if (Value == HTMODE_GF)
		pAd->CommonCfg.RegTransmitSetting.field.HTMODE  = HTMODE_GF;
	else if ( Value == HTMODE_MM )
		pAd->CommonCfg.RegTransmitSetting.field.HTMODE  = HTMODE_MM;
	else 
		return FALSE; //Invalid argument 	

	SetCommonHT(pAd);
	
	DBGPRINT(RT_DEBUG_TRACE, ("Set_HtOpMode_Proc::(HtOpMode=%d)\n",pAd->CommonCfg.RegTransmitSetting.field.HTMODE));

	return TRUE;

}	

INT	Set_HtStbc_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PUCHAR			arg)
{

	ULONG Value;
	
	Value = simple_strtol(arg, 0, 10);
	
	if (Value == STBC_USE)
		pAd->CommonCfg.RegTransmitSetting.field.STBC = STBC_USE;
	else if ( Value == STBC_NONE )
		pAd->CommonCfg.RegTransmitSetting.field.STBC = STBC_NONE;
	else 
		return FALSE; //Invalid argument 	

	SetCommonHT(pAd);
	
	DBGPRINT(RT_DEBUG_TRACE, ("Set_Stbc_Proc::(HtStbc=%d)\n",pAd->CommonCfg.RegTransmitSetting.field.STBC));

	return TRUE;											
}

INT	Set_HtHtc_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PUCHAR			arg)
{

	ULONG Value;
	
	Value = simple_strtol(arg, 0, 10);
	if (Value == 0)
		pAd->HTCEnable = FALSE;
	else if ( Value ==1 )
        pAd->HTCEnable = TRUE;
	else 
		return FALSE; //Invalid argument 	
	
	DBGPRINT(RT_DEBUG_TRACE, ("Set_HtHtc_Proc::(HtHtc=%d)\n",pAd->HTCEnable));

	return TRUE;		
}
			
INT	Set_HtExtcha_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PUCHAR			arg)
{

	ULONG Value;
	
	Value = simple_strtol(arg, 0, 10);
	
	if (Value == 0)			
		pAd->CommonCfg.RegTransmitSetting.field.EXTCHA  = EXTCHA_BELOW;
	else if ( Value ==1 )
        pAd->CommonCfg.RegTransmitSetting.field.EXTCHA = EXTCHA_ABOVE;
	else 
		return FALSE; //Invalid argument 	
	
	SetCommonHT(pAd);

	DBGPRINT(RT_DEBUG_TRACE, ("Set_HtExtcha_Proc::(HtExtcha=%d)\n",pAd->CommonCfg.RegTransmitSetting.field.EXTCHA));

	return TRUE;			
}

INT	Set_HtMpduDensity_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PUCHAR			arg)
{
	ULONG Value;
	
	Value = simple_strtol(arg, 0, 10);
	
	if (Value <=7 && Value >= 0)
		pAd->CommonCfg.BACapability.field.MpduDensity = Value;
	else
		pAd->CommonCfg.BACapability.field.MpduDensity = 4;

	SetCommonHT(pAd);

	DBGPRINT(RT_DEBUG_TRACE, ("Set_HtMpduDensity_Proc::(HtMpduDensity=%d)\n",pAd->CommonCfg.BACapability.field.MpduDensity));

	return TRUE;																																	
}

INT	Set_HtBaWinSize_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PUCHAR			arg)
{
	ULONG Value;
	
	Value = simple_strtol(arg, 0, 10);

	// for intel IOT 
	Value = 64;

	if (Value >=1 && Value <= 64)
	{
		pAd->CommonCfg.REGBACapability.field.RxBAWinLimit = Value;
		pAd->CommonCfg.BACapability.field.RxBAWinLimit = Value;
	}
	else
	{
        pAd->CommonCfg.REGBACapability.field.RxBAWinLimit = 64;
		pAd->CommonCfg.BACapability.field.RxBAWinLimit = 64;
	}
	
	SetCommonHT(pAd);

	DBGPRINT(RT_DEBUG_TRACE, ("Set_HtBaWinSize_Proc::(HtBaWinSize=%d)\n",pAd->CommonCfg.BACapability.field.RxBAWinLimit));

	return TRUE;																																	
}		

INT	Set_HtRdg_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PUCHAR			arg)
{
	ULONG Value;
	
	Value = simple_strtol(arg, 0, 10);
	
	if (Value == 0)			
		pAd->CommonCfg.bRdg = FALSE;
	else if ( Value ==1 )
	{
		pAd->HTCEnable = TRUE;
        pAd->CommonCfg.bRdg = TRUE;
	}
	else 
		return FALSE; //Invalid argument
	
	SetCommonHT(pAd);	
		
	DBGPRINT(RT_DEBUG_TRACE, ("Set_HtRdg_Proc::(HtRdg=%d)\n",pAd->CommonCfg.bRdg));

	return TRUE;																																	
}		

INT	Set_HtLinkAdapt_Proc(																																																																																																																																																																																																																																																																																																																			
	IN	PRTMP_ADAPTER	pAd, 
	IN	PUCHAR			arg)
{
	ULONG Value;
	
	Value = simple_strtol(arg, 0, 10);
	if (Value == 0)
		pAd->bLinkAdapt = FALSE;
	else if ( Value ==1 )
	{
			pAd->HTCEnable = TRUE;
			pAd->bLinkAdapt = TRUE;
	}
	else
		return FALSE; //Invalid argument
	
	DBGPRINT(RT_DEBUG_TRACE, ("Set_HtLinkAdapt_Proc::(HtLinkAdapt=%d)\n",pAd->bLinkAdapt));

	return TRUE;																																	
}		

INT	Set_HtAmsdu_Proc(																																																																																																																																																																																																																																																																																																																			
	IN	PRTMP_ADAPTER	pAd, 
	IN	PUCHAR			arg)
{
	ULONG Value;
	
	Value = simple_strtol(arg, 0, 10);
	if (Value == 0)
		pAd->CommonCfg.BACapability.field.AmsduEnable = FALSE;
	else if ( Value == 1 )
        pAd->CommonCfg.BACapability.field.AmsduEnable = TRUE;
	else
		return FALSE; //Invalid argument
	
	SetCommonHT(pAd);	
		
	DBGPRINT(RT_DEBUG_TRACE, ("Set_HtAmsdu_Proc::(HtAmsdu=%d)\n",pAd->CommonCfg.BACapability.field.AmsduEnable));

	return TRUE;																																	
}			

INT	Set_HtAutoBa_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PUCHAR			arg)
{
	ULONG Value;
	
	Value = simple_strtol(arg, 0, 10);
	if (Value == 0)
		pAd->CommonCfg.BACapability.field.AutoBA = FALSE;
    else if (Value == 1)	
		pAd->CommonCfg.BACapability.field.AutoBA = TRUE;
	else
		return FALSE; //Invalid argument
	
    pAd->CommonCfg.REGBACapability.field.AutoBA = pAd->CommonCfg.BACapability.field.AutoBA;
	SetCommonHT(pAd);	
		
	DBGPRINT(RT_DEBUG_TRACE, ("Set_HtAutoBa_Proc::(HtAutoBa=%d)\n",pAd->CommonCfg.BACapability.field.AutoBA));

	return TRUE;				
		
}		
																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																						
INT	Set_HtProtect_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PUCHAR			arg)
{
	ULONG Value;

	Value = simple_strtol(arg, 0, 10);
	if (Value == 0)
		pAd->CommonCfg.bHTProtect = FALSE;
    else if (Value == 1)	
		pAd->CommonCfg.bHTProtect = TRUE;
	else
		return FALSE; //Invalid argument

	DBGPRINT(RT_DEBUG_TRACE, ("Set_HtProtect_Proc::(HtProtect=%d)\n",pAd->CommonCfg.bHTProtect));

	return TRUE;
}

INT	Set_SendPSMPAction_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PUCHAR			arg)
{
    UCHAR mac[6], mode;
	char *token, sepValue[] = ":", DASH = '-';
	INT i;
    MAC_TABLE_ENTRY *pEntry;

    //printk("\n%s\n", arg);
/*
	The BARecTearDown inupt string format should be xx:xx:xx:xx:xx:xx-d, 
		=>The six 2 digit hex-decimal number previous are the Mac address, 
		=>The seventh decimal number is the mode value.
*/
    if(strlen(arg) < 19)  //Mac address acceptable format 01:02:03:04:05:06 length 17 plus the "-" and mode value in decimal format.
		return FALSE;

   	token = strchr(arg, DASH);
	if ((token != NULL) && (strlen(token)>1))
	{
		mode = simple_strtol((token+1), 0, 10);
		if (mode > MMPS_ENABLE)
			return FALSE;
		
		*token = '\0';
		for (i = 0, token = rstrtok(arg, &sepValue[0]); token; token = rstrtok(NULL, &sepValue[0]), i++)
		{
			if((strlen(token) != 2) || (!isxdigit(*token)) || (!isxdigit(*(token+1))))
				return FALSE;
			AtoH(token, (PUCHAR)(&mac[i]), 1);
		}
		if(i != 6)
			return FALSE;

		printk("\n%02x:%02x:%02x:%02x:%02x:%02x-%02x", mac[0], mac[1],
		       mac[2], mac[3], mac[4], mac[5], mode);

		pEntry = MacTableLookup(pAd, mac);

		if (pEntry) {
		    printk("\nSendPSMPAction MIPS mode = %d\n", mode);
		    SendPSMPAction(pAd, pEntry->Aid, mode);
		}

		return TRUE;
	}

	return FALSE;


}

INT	Set_HtMIMOPSmode_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PUCHAR			arg)
{
	ULONG Value;
	
	Value = simple_strtol(arg, 0, 10);
	
	if (Value <=3 && Value >= 0)
		pAd->CommonCfg.BACapability.field.MMPSmode = Value;
	else
		pAd->CommonCfg.BACapability.field.MMPSmode = 3;

	SetCommonHT(pAd);

	DBGPRINT(RT_DEBUG_TRACE, ("Set_HtMIMOPSmode_Proc::(MIMOPS mode=%d)\n",pAd->CommonCfg.BACapability.field.MMPSmode));

	return TRUE;																																	
}

#ifdef CONFIG_AP_SUPPORT
/* 
    ==========================================================================
    Description:
        Set Tx Stream number
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_HtTxStream_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PUCHAR			arg)
{
	ULONG 	Value;	
		
	Value = simple_strtol(arg, 0, 10);

	if ((Value <= 2) && (Value >= 1) && (Value <= pAd->Antenna.field.TxPath))
		pAd->CommonCfg.TxStream = Value;
	else
		pAd->CommonCfg.TxStream = pAd->Antenna.field.TxPath;

	SetCommonHT(pAd);

	APStop(pAd);
	APStartUp(pAd);

	DBGPRINT(RT_DEBUG_TRACE, ("Set_HtTxStream_Proc::(Tx Stream=%d)\n",pAd->CommonCfg.TxStream));
		
	return TRUE;
}

/* 
    ==========================================================================
    Description:
        Set Rx Stream number
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_HtRxStream_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PUCHAR			arg)
{
	ULONG 	Value;	
		
	Value = simple_strtol(arg, 0, 10);

	if ((Value <= 3) && (Value >= 1) && (Value <= pAd->Antenna.field.RxPath))
		pAd->CommonCfg.RxStream = Value;
	else
		pAd->CommonCfg.RxStream = pAd->Antenna.field.RxPath;

	SetCommonHT(pAd);

	APStop(pAd);
	APStartUp(pAd);

	DBGPRINT(RT_DEBUG_TRACE, ("Set_HtRxStream_Proc::(Rx Stream=%d)\n",pAd->CommonCfg.RxStream));
		
	return TRUE;
}
#endif // CONFIG_AP_SUPPORT //

INT	Set_ForceShortGI_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PUCHAR			arg)
{
	ULONG Value;

	Value = simple_strtol(arg, 0, 10);
	if (Value == 0)
		pAd->WIFItestbed.bShortGI = FALSE;
	else if (Value == 1)	
		pAd->WIFItestbed.bShortGI = TRUE;
	else
		return FALSE; //Invalid argument

	SetCommonHT(pAd);

	DBGPRINT(RT_DEBUG_TRACE, ("Set_ForceShortGI_Proc::(ForceShortGI=%d)\n", pAd->WIFItestbed.bShortGI));

	return TRUE;
}



INT	Set_ForceGF_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PUCHAR			arg)
{
	ULONG Value;

	Value = simple_strtol(arg, 0, 10);
	if (Value == 0)
		pAd->WIFItestbed.bGreenField = FALSE;
	else if (Value == 1)	
		pAd->WIFItestbed.bGreenField = TRUE;
	else
		return FALSE; //Invalid argument

	SetCommonHT(pAd);

	DBGPRINT(RT_DEBUG_TRACE, ("Set_ForceGF_Proc::(ForceGF=%d)\n", pAd->WIFItestbed.bGreenField));

	return TRUE;
}

INT	Set_HtMimoPs_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PUCHAR			arg)
{
	ULONG Value;

	Value = simple_strtol(arg, 0, 10);
	if (Value == 0)
		pAd->CommonCfg.bMIMOPSEnable = FALSE;
	else if (Value == 1)	
		pAd->CommonCfg.bMIMOPSEnable = TRUE;
	else
		return FALSE; //Invalid argument

	DBGPRINT(RT_DEBUG_TRACE, ("Set_HtMimoPs_Proc::(HtMimoPs=%d)\n",pAd->CommonCfg.bMIMOPSEnable));

	return TRUE;
}



//#endif // UCOS //

INT	SetCommonHT(
	IN	PRTMP_ADAPTER	pAd)
{
	OID_SET_HT_PHYMODE		SetHT;
	
	if (pAd->CommonCfg.PhyMode < PHY_11ABGN_MIXED)
		return FALSE;
				
	SetHT.PhyMode = pAd->CommonCfg.PhyMode;
	SetHT.TransmitNo = ((UCHAR)pAd->Antenna.field.TxPath);
	SetHT.HtMode = (UCHAR)pAd->CommonCfg.RegTransmitSetting.field.HTMODE;
	SetHT.ExtOffset = (UCHAR)pAd->CommonCfg.RegTransmitSetting.field.EXTCHA;
	SetHT.MCS = MCS_AUTO;
	SetHT.BW = (UCHAR)pAd->CommonCfg.RegTransmitSetting.field.BW;
	SetHT.STBC = (UCHAR)pAd->CommonCfg.RegTransmitSetting.field.STBC;
	SetHT.SHORTGI = (UCHAR)pAd->CommonCfg.RegTransmitSetting.field.ShortGI;		

	RTMPSetHT(pAd, &SetHT);

#if 0
#ifdef CONFIG_AP_SUPPORT
	if (pAd->OpMode == OPMODE_AP)
	{
		INT	apidx;

		// set common HT setting
		RTMPSetHT(pAd, &SetHT);

		for (apidx; apidx < pAd->ApCfg.BssidNum; apidx++)
		{
		    SetHT.MCS = (UCHAR)pAd->ApCfg.MBSSID[apidx].DesiredTransmitSetting.field.MCS;							

			// set HT setting per BSS
			RTMPSetIndividualHT(pAd, &SetHT, apidx);			
		}
	}
#endif // CONFIG_AP_SUPPORT //

#ifdef CONFIG_STA_SUPPORT
	if (pAd->OpMode == OPMODE_STA)
	{
		SetHT.MCS = (UCHAR)pAd->StaCfg.DesiredTransmitSetting.field.MCS;
		RTMPSetHT(pAd, &SetHT);
		RTMPSetIndividualHT(pAd, &SetHT, 0xFF);
	}
#endif // CONFIG_STA_SUPPORT //
#endif // 0 //
	
	return TRUE;
}

INT	Set_FixedTxMode_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PUCHAR			arg)
{
#ifdef CONFIG_AP_SUPPORT    
    POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
#endif // CONFIG_AP_SUPPORT //		
	UCHAR	fix_tx_mode = FIXED_TXMODE_HT;

	if (strcmp(arg, "OFDM") == 0)
	{
		fix_tx_mode = FIXED_TXMODE_OFDM;
	}	
	else if (strcmp(arg, "CCK") == 0)
	{
        fix_tx_mode = FIXED_TXMODE_CCK;
	}
	
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		pAd->ApCfg.MBSSID[pObj->ioctl_if].DesiredTransmitSetting.field.FixedTxMode = fix_tx_mode;
#endif // CONFIG_AP_SUPPORT //
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		pAd->StaCfg.DesiredTransmitSetting.field.FixedTxMode = fix_tx_mode;
#endif // CONFIG_STA_SUPPORT //
	
	DBGPRINT(RT_DEBUG_TRACE, ("Set_FixedTxMode_Proc::(FixedTxMode=%d)\n", fix_tx_mode));

	return TRUE;
}

#ifdef CONFIG_APSTA_MIXED_SUPPORT
INT	Set_OpMode_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PUCHAR			arg)
{
	ULONG Value;

	Value = simple_strtol(arg, 0, 10);

#ifdef RT2860
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
#endif // RT2860 //
#ifdef RT2870
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP))
#endif // RT2870 //
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Can not switch operate mode on interface up !! \n"));
		return FALSE;
	}

	if (Value == 0)
		pAd->OpMode = OPMODE_STA;
	else if (Value == 1)	
		pAd->OpMode = OPMODE_AP;
	else
		return FALSE; //Invalid argument

	DBGPRINT(RT_DEBUG_TRACE, ("Set_OpMode_Proc::(OpMode=%s)\n", pAd->OpMode == 1 ? "AP Mode" : "STA Mode"));

	return TRUE;
}
#endif // CONFIG_APSTA_MIXED_SUPPORT //

#ifdef RT2870

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,5,0)
static void dump_urb(struct urb* purb)
{
	printk("urb                  :0x%08lx\n", (unsigned long)purb);
	printk("\tdev                   :0x%08lx\n", (unsigned long)purb->dev);
	printk("\t\tdev->state          :0x%d\n", purb->dev->state);
	printk("\tpipe                  :0x%08x\n", purb->pipe);
	printk("\tstatus                :%d\n", purb->status);
	printk("\ttransfer_flags        :0x%08x\n", purb->transfer_flags);
	printk("\ttransfer_buffer       :0x%08lx\n", (unsigned long)purb->transfer_buffer);
	printk("\ttransfer_buffer_length:%d\n", purb->transfer_buffer_length);
	printk("\tactual_length         :%d\n", purb->actual_length);
	printk("\tsetup_packet          :0x%08lx\n", (unsigned long)purb->setup_packet);
	printk("\tstart_frame           :%d\n", purb->start_frame);
	printk("\tnumber_of_packets     :%d\n", purb->number_of_packets);
	printk("\tinterval              :%d\n", purb->interval);
	printk("\terror_count           :%d\n", purb->error_count);
	printk("\tcontext               :0x%08lx\n", (unsigned long)purb->context);
	printk("\tcomplete              :0x%08lx\n\n", (unsigned long)purb->complete);
}
#else
static void dump_urb(struct urb* purb)
{
	return;
}
#endif


INT	Show_RxBulk_Proc(
	IN RTMP_ADAPTER *pAd,
	IN PUCHAR		arg)
{
	int i;
	RX_CONTEXT	*pRxContext;
	unsigned long IrqFlags;

	
	printk("Bulk-In Info:\n");
	RTMP_IRQ_LOCK(&pAd->BulkInLock, IrqFlags);
	printk("\tPendingRx=%d, BIIdx=%d, BIReadIdx=%d, BIPos=%ld, BIRPos=%ld, BIReq=x%lx, BIComplete=0x%lx, BICFail0x%lx\n", 
			pAd->PendingRx, pAd->NextRxBulkInIndex, pAd->NextRxBulkInReadIndex, pAd->NextRxBulkInPosition, 
			pAd->ReadPosition, pAd->BulkInReq, pAd->BulkInComplete, pAd->BulkInCompleteFail);
	printk("\tDetailed RxContext Info:\n");
	for (i = 0; i < RX_RING_SIZE; i++)
	{
		pRxContext = &pAd->RxContext[i];
 		printk("\t\tRxContext[%d]: IRPPending=%d, InUse=%d, Readable=%d, bRxHandling=%d, BIOffset=%ld!\n"
				, i, pRxContext->IRPPending, pRxContext->InUse, pRxContext->Readable, pRxContext->bRxHandling, pRxContext->BulkInOffset);
		if (pRxContext->IRPPending)
			dump_urb(pRxContext->pUrb);
	}
	RTMP_IRQ_UNLOCK(&pAd->BulkInLock, IrqFlags);
	
	return TRUE;
}

INT Show_TxBulk_Proc(
	IN RTMP_ADAPTER *pAd,
	IN PUCHAR		arg)
{
	int i;
	HT_TX_CONTEXT	*pHTTXContext;
	TX_CONTEXT		*pMLMEContext, *pPsPollContext, *pNULLContext;
	unsigned long IrqFlags;
	UINT32 value;

	printk("\nTx Data Ring Info:\n");
	for (i = 0 ; i < NUM_OF_TX_RING; i++)
	{
		RTMP_IRQ_LOCK(&pAd->TxContextQueueLock[i], IrqFlags);
		pHTTXContext = &pAd->TxContext[i];
		printk("TxContext[%d]:CWPos=%ld, NBPos=%ld, ENBPos=%ld, bCopy=%d, pending=%d!\n", 
			i, pHTTXContext->CurWritePosition, pHTTXContext->NextBulkOutPosition, pHTTXContext->ENextBulkOutPosition, pHTTXContext->bCopySavePad, pAd->BulkOutPending[i]);
		printk("\tBulkOut Req=0x%lx, Complete=0x%lx, Other=0x%lx\n", pAd->BulkOutReq, pAd->BulkOutComplete, pAd->BulkOutCompleteOther);
		if (pAd->BulkOutPending[i] == TRUE)
			dump_urb(pHTTXContext->pUrb);
		RTMP_IRQ_UNLOCK(&pAd->TxContextQueueLock[i], IrqFlags);
	}

	printk("\nTx Ctrl Ring Info:\n");
	{
		pNULLContext = &pAd->NullContext;
		pPsPollContext = &pAd->PsPollContext;
		printk("NullContext: InUse=%d, bWaitingBulkOut=%d, IRPPending=%d, BulkOutSize=%ld!\n", 
				pNULLContext->InUse, pNULLContext->bWaitingBulkOut, pNULLContext->IRPPending, pNULLContext->BulkOutSize);
		printk("PsPollContext: InUse=%d, bWaitingBulkOut=%d, IRPPending=%d, BulkOutSize=%ld!\n", 
				pPsPollContext->InUse, pPsPollContext->bWaitingBulkOut, pPsPollContext->IRPPending, pPsPollContext->BulkOutSize);
	}

	printk("\nTx Mgmt Ring Info:\n");
	RTMP_IRQ_LOCK(&pAd->BulkOutLock[MGMTPIPEIDX], IrqFlags);	
	printk("\tTxDmaIdx=%d, TxCpuIdx=%d, tTxSwFreeIdx=%d!\n", pAd->MgmtRing.TxDmaIdx, pAd->MgmtRing.TxCpuIdx, pAd->MgmtRing.TxSwFreeIdx);
	for(i = 0; i < MGMT_RING_SIZE; i++)
	{
		pMLMEContext = (PTX_CONTEXT)pAd->MgmtRing.Cell[i].AllocVa;
					
		printk("MgmtRing.Cell[%d]:InUse=%d, bWaitingBulkOut=%d, IRPPending=%d, BulkOutSize=%ld!\n", i, pMLMEContext->InUse, 
				pMLMEContext->bWaitingBulkOut, pMLMEContext->IRPPending, pMLMEContext->BulkOutSize);
	}
	RTMP_IRQ_UNLOCK(&pAd->BulkOutLock[MGMTPIPEIDX], IrqFlags);
	
	RTUSBReadMACRegister(pAd, 0x438, &value);
	printk("Register 0x438=0x%x!\n", value);
	RTUSBReadMACRegister(pAd, 0x43c, &value);	
	printk("Register 0x43c=0x%x!\n", value);
	
	return TRUE;
}

#endif // RT2870 //
