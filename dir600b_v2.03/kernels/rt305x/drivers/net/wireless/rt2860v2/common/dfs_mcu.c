#include <rt_config.h>

// Radar detection and carrier detection for 2880-SW-MCU

#if defined(DFS_SUPPORT) || defined(CARRIER_DETECTION_SUPPORT)

static inline void TimerCB_Radar(PRTMP_ADAPTER pAd);
static inline void TimerCB_Carrier(PRTMP_ADAPTER pAd);


void TimerCB(PRTMP_ADAPTER pAd)
{
	// what is this mean???
	if (pAd->CommonCfg.McuRadarProtection == 1)
	{
#ifdef DFS_DEBUG
		printk("###\n");
#endif // DFS_DEBUG //
		return;
	}

#ifdef CONFIG_AP_SUPPORT	
#ifdef DFS_SUPPORT
	if (pAd->CommonCfg.McuRadarCmd & RADAR_DETECTION)
		TimerCB_Radar(pAd);
#endif
#endif // CONFIG_AP_SUPPORT //

#ifdef CARRIER_DETECTION_SUPPORT
	if (pAd->CommonCfg.McuRadarCmd & CARRIER_DETECTION)
		TimerCB_Carrier(pAd);
#endif

}

#ifdef DFS_DEBUG
static unsigned int dbg_idx=0;

#define DFS_DEBUG_BEGIN				0x1
#define DFS_DEBUG_CTS_KICK			0x2
#define DFS_DEBUG_CTS_SENT			0x3
#define DFS_DEBUG_WAIT_CTS			0x4
#define DFS_DEBUG_DETECT_NO_CTS		0x5
#define DFS_DEBUG_DETECT_AFTER_CTS	0x6
#define DFS_DEBUG_WAIT_CTS_TIMEOUT	0x7
#define DFS_DEBUG_IN_DETECTION		0x8
#define DFS_DEBUG_FREE_TX			0x9
#define DFS_DEBUG_LONG_REPORT		0x10
#define DFS_DEBUG_SHORT_REPORT		0x11
#define DFS_DEBUG_SWITCH_PRI		0x12
#define DFS_DEBUG_SWITCH_EXT		0x13
#define DFS_DEBUG_0001				0x14


void DFS_Debug(UCHAR i, PRTMP_ADAPTER pAd)
{
	if (dbg_idx < 100)
	{
		pAd->CommonCfg.Detect1msIndex[dbg_idx] = i;
		pAd->CommonCfg.Detect1msTick[dbg_idx] = pAd->CommonCfg.McuRadarTick;
		pAd->CommonCfg.Detect1msDetectCount[dbg_idx] = pAd->CommonCfg.McuRadarDetectCount;
		pAd->CommonCfg.Detect1msState[dbg_idx] = pAd->CommonCfg.McuRadarState;
		pAd->CommonCfg.Detect1msEvent[dbg_idx] = pAd->CommonCfg.McuRadarEvent;

		RTMP_IO_READ32(pAd, PBF_LIFE_TIMER, &pAd->CommonCfg.Detect1msTime[dbg_idx]);

		dbg_idx++;
	}
	if (dbg_idx == 100)
	{
		printk("data collect done\n");
		dbg_idx++;
	}
}

void DFS_Dump(PRTMP_ADAPTER pAd)
{
	int i;
	for (i = 0; i < 100; i++)
	{
		printk("%02x : %02d %d ", pAd->CommonCfg.Detect1msIndex[i], pAd->CommonCfg.Detect1msTick[i], pAd->CommonCfg.Detect1msDetectCount[i]);
		printk("%d %x (%x)\n", pAd->CommonCfg.Detect1msState[i], pAd->CommonCfg.Detect1msEvent[i], (pAd->CommonCfg.Detect1msTime[i] - ((i == 0)? 0 : pAd->CommonCfg.Detect1msTime[i-1])));
	}
	
	dbg_idx=0;
}
#endif // DFS_DEBUG //

#define CTS_WAIT_LOOP		500

#ifdef CONFIG_AP_SUPPORT
#ifdef DFS_SUPPORT
static inline void TimerCB_Radar(PRTMP_ADAPTER pAd)
{
	ULONG Value;

	if (pAd->CommonCfg.McuRadarEvent & RADAR_EVENT_CARRIER_DETECTING)
	{
		pAd->CommonCfg.McuRadarTick++;
		return;
	}
	else
		pAd->CommonCfg.McuRadarTick++;
	

	if ((pAd->CommonCfg.McuRadarTick >= pAd->CommonCfg.McuRadarPeriod) && (pAd->CommonCfg.McuRadarState != DO_DETECTION))
	{

#ifdef DFS_DEBUG
			if (pAd->CommonCfg.McuRadarDebug & RADAR_LOG)
			{
				DFS_Debug(DFS_DEBUG_BEGIN, pAd);
			}
#endif // DFS_DEBUG //

		/* Roger add to fix false detection(long pulse only) in the first 60 seconds */
		if (pAd->CommonCfg.W56_debug)
		{
			ULONG time;
			RTMP_IO_READ32(pAd, PBF_LIFE_TIMER, &pAd->CommonCfg.W56_hw_2);
			RTMP_IO_READ32(pAd, CH_IDLE_STA, &Value);
			
			pAd->CommonCfg.W56_hw_sum += pAd->CommonCfg.W56_hw_2 - pAd->CommonCfg.W56_hw_1;
			pAd->CommonCfg.W56_hw_1 = pAd->CommonCfg.W56_hw_2;
			pAd->CommonCfg.W56_idx++;
			pAd->CommonCfg.W56_1s += Value;
			if ((pAd->CommonCfg.W56_idx % 100) == 0)
			{
				int i;
				//printk("~~~ %d   %d   ", pAd->CommonCfg.W56_idx, pAd->CommonCfg.W56_hw_sum - pAd->CommonCfg.W56_1s);
				if (pAd->CommonCfg.W56_hw_sum > pAd->CommonCfg.W56_1s)
					pAd->CommonCfg.W56_4s[(pAd->CommonCfg.W56_idx / 100) % 4] = pAd->CommonCfg.W56_hw_sum - pAd->CommonCfg.W56_1s;
				else
					pAd->CommonCfg.W56_4s[(pAd->CommonCfg.W56_idx / 100) % 4] = 0;
				Value = 0;
				for (i = 0; i < 4; i++)
					Value += pAd->CommonCfg.W56_4s[i];
				//printk("%d\n", Value);
				pAd->CommonCfg.W56_total = Value;
				pAd->CommonCfg.W56_hw_sum = 0;
				pAd->CommonCfg.W56_1s = 0;
			}
		}

		// Start of period
		pAd->CommonCfg.McuRadarTick = 0;
		pAd->CommonCfg.McuRadarDetectCount = 0;
		pAd->CommonCfg.McuRadarState = WAIT_CTS_BEING_SENT;
		pAd->CommonCfg.McuRadarEvent |= RADAR_EVENT_RADAR_DETECTING;


		if ((pAd->CommonCfg.McuRadarCtsProtect == 1)
#ifdef CARRIER_DETECTION_SUPPORT
			|| (isCarrierDetectExist(pAd))
#endif
			)
		{
			// no CTS protect
			pAd->CommonCfg.McuRadarState = DO_DETECTION;
			MCURadarDetect(pAd);

#ifdef DFS_DEBUG
			if (pAd->CommonCfg.McuRadarDebug & RADAR_LOG)
			{
				DFS_Debug(DFS_DEBUG_DETECT_NO_CTS, pAd);
			}
#endif // DFS_DEBUG //
			
		}
		else if (pAd->CommonCfg.McuRadarCtsProtect == 2 || pAd->CommonCfg.McuRadarCtsProtect == 3)
		{
			int i;
						
			if (pAd->CommonCfg.McuRadarCtsProtect == 3)
			{
				
				RTMP_IO_READ32(pAd, 0x7784, &Value);
				Value &= 0xffff00ff;
				RTMP_IO_WRITE32(pAd, 0x7784, Value);
				
				// pAd->CommonCfg.McuRadarCtsProtect == 3, need to kick CTS two times
				RTMP_IO_WRITE32(pAd, PBF_CTRL, 0x40);
				for (i = 0; i < CTS_WAIT_LOOP; i++)
				{
					RTMP_IO_READ32(pAd, PBF_CTRL, &Value);
					if (!Value)
						break;
				}
				
				if (i >= CTS_WAIT_LOOP)
				{
					pAd->CommonCfg.McuRadarEvent |= RADAR_EVENT_CTS_KICKED;
					return;
				}
			}


			RTMP_IO_READ32(pAd, 0x7784, &Value);
			Value &= 0xffff00ff;
			Value |= (DFS_CTS_WCID << 8);
			RTMP_IO_WRITE32(pAd, 0x7784, Value);

#ifdef DFS_DEBUG
			if (pAd->CommonCfg.McuRadarDebug & RADAR_LOG)
			{
				DFS_Debug(DFS_DEBUG_CTS_KICK, pAd);
			}
#endif // DFS_DEBUG //
			// kick CTS
			RTMP_IO_WRITE32(pAd, PBF_CTRL, 0x40);

		}
		return;
	}

	if (pAd->CommonCfg.McuRadarState == WAIT_CTS_BEING_SENT)
	{

#ifdef DFS_DEBUG
			if (pAd->CommonCfg.McuRadarDebug & RADAR_LOG)
			{
				DFS_Debug(DFS_DEBUG_WAIT_CTS, pAd);
			}
#endif // DFS_DEBUG //

		// check event or timeout
		if (pAd->CommonCfg.McuRadarEvent & RADAR_EVENT_CTS_SENT)
		{
			pAd->CommonCfg.McuRadarEvent &= ~RADAR_EVENT_CTS_SENT;
			pAd->CommonCfg.McuRadarDetectCount = 1;
			pAd->CommonCfg.McuRadarState = DO_DETECTION;

			{
				UCHAR BBPR115;
				BBP_IO_READ8_BY_REG_ID(pAd, BBP_R115, &BBPR115);
			}

#ifdef DFS_DEBUG			
			if (pAd->CommonCfg.McuRadarDebug & RADAR_LOG)
			{
				DFS_Debug(DFS_DEBUG_DETECT_AFTER_CTS, pAd);
			}
#endif // DFS_DEBUG //
			return;
		}
		
		if (pAd->CommonCfg.McuRadarDetectCount >= (4-1)) // timeout 4ms, start from 0
		{
			pAd->CommonCfg.McuRadarState = FREE_FOR_TX; // timeout 5ms, give up
			pAd->CommonCfg.McuRadarEvent &= ~(RADAR_EVENT_RADAR_DETECTING | RADAR_EVENT_CTS_KICKED);
			pAd->CommonCfg.McuRadarTick = pAd->CommonCfg.McuRadarPeriod;

#ifdef DFS_DEBUG
			if (pAd->CommonCfg.McuRadarDebug & RADAR_LOG)
			{
				DFS_Debug(DFS_DEBUG_WAIT_CTS_TIMEOUT, pAd);
			}
#endif // DFS_DEBUG //

			RTMP_IO_WRITE32(pAd, TX_RTS_CFG, pAd->CommonCfg.OldRtsRetryLimit);
			// Restore BBP
			BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R65, pAd->CommonCfg._R65);
			BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, pAd->CommonCfg._R66);
			BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R113, 0x01);

			// EnableNormalTx
			RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &Value);
			Value |= 0x04;
			RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, Value);
			
		}

		if (pAd->CommonCfg.McuRadarEvent & RADAR_EVENT_CTS_KICKED)
		{
			int i;

			for (i = 0; i < CTS_WAIT_LOOP; i++)
			{
				RTMP_IO_READ32(pAd, PBF_CTRL, &Value);
				if (!Value)
					break;
			}
				
			if (i >= CTS_WAIT_LOOP)
			{
				pAd->CommonCfg.McuRadarDetectCount++;
				return;
			}
			
			RTMP_IO_READ32(pAd, 0x7784, &Value);
			Value &= 0xffff00ff;
			Value |= (DFS_CTS_WCID << 8) ;
			RTMP_IO_WRITE32(pAd, 0x7784, Value);

#ifdef DFS_DEBUG
			if (pAd->CommonCfg.McuRadarDebug & RADAR_LOG)
			{
				DFS_Debug(DFS_DEBUG_CTS_KICK, pAd);
			}
#endif // DFS_DEBUG //

			// kick CTS
			RTMP_IO_WRITE32(pAd, PBF_CTRL, 0x40);
			
			pAd->CommonCfg.McuRadarEvent &= ~RADAR_EVENT_CTS_KICKED;
		}

		
		pAd->CommonCfg.McuRadarDetectCount++;
	}
	else if (pAd->CommonCfg.McuRadarState == DO_DETECTION)
	{
		UCHAR BBPR115;
		

#ifdef DFS_DEBUG
			if (pAd->CommonCfg.McuRadarDebug & RADAR_LOG)
			{
				DFS_Debug(DFS_DEBUG_IN_DETECTION, pAd);
			}
#endif // DFS_DEBUG //

		BBP_IO_READ8_BY_REG_ID(pAd, BBP_R115, &BBPR115);

		if (pAd->CommonCfg.RadarDetect.RDDurRegion == FCC) 
		{
#ifdef DFS_FCC_BW40_FIX
			if  ( (pAd->CommonCfg.McuRadarDetectCount == (pAd->CommonCfg.RadarDetect.DfsSessionTime + pAd->CommonCfg.RadarDetect.DfsSessionFccOff - 1)) )
#else
			if  ((pAd->CommonCfg.McuRadarDetectCount % 2) == 0)
#endif // DFS_FCC_BW40_FIX //
			{
				UCHAR bbp;
			

#ifdef DFS_DEBUG
				if (pAd->CommonCfg.McuRadarDebug & RADAR_SIMULATE2) 
				{
					if (pAd->CommonCfg.McuRadarCmd & RADAR_DETECTION)
					{
						RTMP_IO_WRITE32(pAd, 0x7100, 0x400);
						pAd->CommonCfg.McuRadarEvent |= RADAR_EVENT_WIDTH_RADAR;
					}
					pAd->CommonCfg.McuRadarDebug &= ~RADAR_SIMULATE2;

				}
#endif


				if (BBPR115 & 0x2)
				{
					BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R115, 0);
#ifdef DFS_DEBUG
					if (pAd->CommonCfg.McuRadarDebug & RADAR_LOG)
					{
						DFS_Debug(DFS_DEBUG_LONG_REPORT, pAd);
					}
#endif // DFS_DEBUG //

					Value = BBPR115 << 24;
					BBP_IO_READ8_BY_REG_ID(pAd, BBP_R116, &bbp);
					Value |= (bbp << 16);
					BBP_IO_READ8_BY_REG_ID(pAd, BBP_R117, &bbp);
					Value |= (bbp << 8);
					BBP_IO_READ8_BY_REG_ID(pAd, BBP_R118, &bbp);
					Value |= bbp;
#ifdef DFS_FCC_BW40_FIX
					RTMP_IO_WRITE32(pAd, 0x7100, Value);
#else
					RTMP_IO_WRITE32(pAd, 0x7100 + (4 * (pAd->CommonCfg.McuRadarDetectCount / 2 - 1)) , Value);
#endif				
					BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R113, 0x01);
					BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R113, 0x21);
				
					pAd->CommonCfg.McuRadarEvent |= RADAR_EVENT_WIDTH_RADAR;
				}
				else
				{
					RTMP_IO_WRITE32(pAd, 0x7100 + (4 * (pAd->CommonCfg.McuRadarDetectCount / 2 - 1)) , 0);
				}
				
				
				
			}

#ifdef DFS_FCC_BW40_FIX
			if (pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40)
			{
				if (pAd->CommonCfg.McuRadarDetectCount == ((pAd->CommonCfg.RadarDetect.DfsSessionTime + pAd->CommonCfg.RadarDetect.DfsSessionFccOff) - 1))
				{
				   	ULONG		Value, byteValue = 0;

#ifdef DFS_DEBUG
			if (pAd->CommonCfg.McuRadarDebug & RADAR_LOG)
			{
				DFS_Debug(DFS_DEBUG_SWITCH_EXT, pAd);
			}
#endif

					// Switch to Extend Channel
					if (pAd->CommonCfg.AddHTInfo.AddHtInfo.ExtChanOffset == EXTCHA_BELOW)
					{
						//  RX : control channel move to lower 
						RTMP_BBP_IO_READ8_BY_REG_ID(pAd, 3, &byteValue);
						byteValue &= (~0x20);
						RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, 3, byteValue);
					}
					else
					{
						//  RX : control channel move to upper 
						RTMP_BBP_IO_READ8_BY_REG_ID(pAd, 3, &byteValue);
						byteValue |= (0x20);
						RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, 3, byteValue);
					}

					//DisableNormalTx
					RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &Value);
					RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, 0); // Toggle Tx Rx
					RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, 0xc); // Toggle Tx Rx
					RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, Value);
				}
			}
#endif // DFS_FCC_BW40_FIX //
		}
		else if ((pAd->CommonCfg.RadarDetect.RDDurRegion == JAP_W56) && ((pAd->CommonCfg.McuRadarDetectCount % 2) == 0))
		{
			UCHAR bbp;
			
			if (BBPR115 & 0x2)
			{

					BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R115, 0);

#ifdef DFS_DEBUG
					if (pAd->CommonCfg.McuRadarDebug & RADAR_LOG)
					{
						DFS_Debug(DFS_DEBUG_LONG_REPORT, pAd);
					}
#endif // DFS_DEBUG //

				Value = BBPR115 << 24;
				BBP_IO_READ8_BY_REG_ID(pAd, BBP_R116, &bbp);
				Value |= (bbp << 16);
				BBP_IO_READ8_BY_REG_ID(pAd, BBP_R117, &bbp);
				Value |= (bbp << 8);
				BBP_IO_READ8_BY_REG_ID(pAd, BBP_R118, &bbp);
				Value |= bbp;
				RTMP_IO_WRITE32(pAd, 0x7100 + (4 * (pAd->CommonCfg.McuRadarDetectCount / 2 - 1)) , Value);
			
				BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R113, 0x01);
				BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R113, 0x21);
			
				pAd->CommonCfg.McuRadarEvent |= RADAR_EVENT_WIDTH_RADAR;
				
			}
			else
			{
				RTMP_IO_WRITE32(pAd, 0x7100 + (4 * (pAd->CommonCfg.McuRadarDetectCount / 2 - 1)) , 0);
			}
						
		}
		
		if (pAd->CommonCfg.McuRadarDetectCount >= (pAd->CommonCfg.McuRadarDetectPeriod - 1))
		{

#ifdef DFS_FCC_BW40_FIX

			// Go Back to Primary Channel
			if (pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40)
			{
				ULONG		Value, byteValue = 0;

				if (pAd->CommonCfg.AddHTInfo.AddHtInfo.ExtChanOffset == EXTCHA_BELOW)
				{
					//  RX : control channel move to upper 
					RTMP_BBP_IO_READ8_BY_REG_ID(pAd, 3, &byteValue);
					byteValue |= (0x20);
					RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, 3, byteValue);
				}
				else
				{
					//  RX : control channel move to lower 
					RTMP_BBP_IO_READ8_BY_REG_ID(pAd, 3, &byteValue);
					byteValue &= (~0x20);
					RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, 3, byteValue);
				}


#ifdef DFS_DEBUG
				if (pAd->CommonCfg.McuRadarDebug & RADAR_LOG)
				{
					DFS_Debug(DFS_DEBUG_SWITCH_PRI, pAd);
				}
#endif // DFS_DEBUG //

			}		
#endif // DFS_FCC_BW40_FIX


			if (pAd->CommonCfg.McuRadarEvent & RADAR_EVENT_WIDTH_RADAR)
			{
				pAd->CommonCfg.McuRadarEvent &= ~(RADAR_EVENT_WIDTH_RADAR);
				RadarSMDetect(pAd, RADAR_WIDTH);
			}
			
#ifdef DFS_DEBUG
			if (pAd->CommonCfg.McuRadarDebug & RADAR_SIMULATE) 
			{
				if (pAd->CommonCfg.McuRadarCmd & RADAR_DETECTION)
				{
					RadarSMDetect(pAd, RADAR_PULSE);
				}
			}
#endif // DFS_DEBUG //
			

			if (BBPR115 & 0x1)
			{
				BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R115, 0);
#ifdef DFS_DEBUG
				if (pAd->CommonCfg.McuRadarDebug & RADAR_LOG)
				{
					DFS_Debug(DFS_DEBUG_SHORT_REPORT, pAd);
				}
#endif // DFS_DEBUG //
				// pulse radar detected
				if (pAd->CommonCfg.McuRadarCmd & RADAR_DETECTION)
				{
					RadarSMDetect(pAd, RADAR_PULSE);
				}
			}


			pAd->CommonCfg.McuRadarState = FREE_FOR_TX;
			pAd->CommonCfg.McuRadarEvent &= ~(RADAR_EVENT_RADAR_DETECTING);

			RTMP_IO_WRITE32(pAd, TX_RTS_CFG, pAd->CommonCfg.OldRtsRetryLimit);
			// Restore BBP
			BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R65, pAd->CommonCfg._R65);
			BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, pAd->CommonCfg._R66);
			BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R113, 0x01);

#ifdef DFS_DEBUG
			if ((pAd->CommonCfg.McuRadarDebug & RADAR_GPIO_DEBUG))
				_RTMP_IO_WRITE32(0xa0300630, 0x2000);
#endif // DFS_DEBUG //
			// EnableNormalTx
			RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &Value);
			Value |= 0x04;
			RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, Value);

		}
		pAd->CommonCfg.McuRadarDetectCount++;

	}
	else // FREE_FOR_TX
	{

#ifdef DFS_DEBUG
			if (pAd->CommonCfg.McuRadarDebug & RADAR_LOG)
			{
				DFS_Debug(DFS_DEBUG_FREE_TX, pAd);
			}
#endif // DFS_DEBUG //

		if (pAd->CommonCfg.McuRadarEvent & RADAR_EVENT_CTS_SENT)
		{

#ifdef DFS_DEBUG
			if (pAd->CommonCfg.McuRadarDebug & RADAR_LOG)
			{
				DFS_Debug(DFS_DEBUG_0001, pAd);
			}
#endif // DFS_DEBUG //

			pAd->CommonCfg.McuRadarEvent &= ~RADAR_EVENT_CTS_SENT;
			pAd->CommonCfg.McuRadarDetectCount = 1;

			RTMP_IO_WRITE32(pAd, TX_RTS_CFG, pAd->CommonCfg.OldRtsRetryLimit);
			// Restore BBP
			BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R65, pAd->CommonCfg._R65);
			BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, pAd->CommonCfg._R66);
			BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R113, 0x01);


#ifdef DFS_DEBUG
			if ((pAd->CommonCfg.McuRadarDebug & RADAR_GPIO_DEBUG))
				_RTMP_IO_WRITE32(0xa0300630, 0x2000);
#endif // DFS_DEBUG //
			// EnableNormalTx
			RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &Value);
			Value |= 0x04;
			RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, Value);
		}
	}	
}
#endif // DFS_SUPPORT //
#endif // CONFIG_AP_SUPPORT //

static inline void TimerCB_Carrier(PRTMP_ADAPTER pAd)
{
	ULONG Value;

	if (pAd->CommonCfg.McuRadarEvent & RADAR_EVENT_RADAR_DETECTING)
	{
		pAd->CommonCfg.McuCarrierTick++;
		return;
	}
	else
		pAd->CommonCfg.McuCarrierTick++;


	if (pAd->CommonCfg.McuCarrierTick >= pAd->CommonCfg.McuCarrierPeriod)
	{
		// Start of period
		pAd->CommonCfg.McuCarrierTick = 0;
		pAd->CommonCfg.McuCarrierDetectCount = 0;
		pAd->CommonCfg.McuCarrierState = WAIT_CTS_BEING_SENT;
		pAd->CommonCfg.McuRadarEvent |= RADAR_EVENT_CARRIER_DETECTING;

		if ((pAd->CommonCfg.McuCarrierCtsProtect == 1)
#ifdef CARRIER_DETECTION_SUPPORT
			|| (isCarrierDetectExist(pAd))
#endif // CARRIER_DETECTION_SUPPORT //
			)
		{
			// no CTS protect
			pAd->CommonCfg.McuCarrierState = DO_DETECTION;
			MCURadarDetect(pAd);
		}
		else if (pAd->CommonCfg.McuCarrierCtsProtect == 2)
		{
			// kick CTS
			RTMP_IO_WRITE32(pAd, PBF_CTRL, 0x80);
		}
		return;
	}

	if (pAd->CommonCfg.McuCarrierState == WAIT_CTS_BEING_SENT)
	{
		// check event or timeout
		if (pAd->CommonCfg.McuRadarEvent & RADAR_EVENT_CTS_CARRIER_SENT)
		{
			pAd->CommonCfg.McuRadarEvent &= ~RADAR_EVENT_CTS_CARRIER_SENT;
			pAd->CommonCfg.McuCarrierDetectCount = 1;
			pAd->CommonCfg.McuCarrierState = DO_DETECTION;

			return;
		}
		
		if (pAd->CommonCfg.McuCarrierDetectCount >= (5-1)) // timeout 5ms, start from 0
		{
			pAd->CommonCfg.McuCarrierState = FREE_FOR_TX; // timeout 5ms, give up
			pAd->CommonCfg.McuRadarEvent &= ~(RADAR_EVENT_CARRIER_DETECTING);

		}
		
		pAd->CommonCfg.McuCarrierDetectCount++;
	}
	else if (pAd->CommonCfg.McuCarrierState == DO_DETECTION)
	{
		if (pAd->CommonCfg.McuCarrierDetectCount >= (pAd->CommonCfg.McuCarrierDetectPeriod - 1))
		{
			UCHAR BBPR115;

			BBP_IO_READ8_BY_REG_ID(pAd, BBP_R115, &BBPR115);

			CarrierDetectionCheck(pAd);

			pAd->CommonCfg.McuCarrierState = FREE_FOR_TX;
			pAd->CommonCfg.McuRadarEvent &= ~(RADAR_EVENT_CARRIER_DETECTING);

			RTMP_IO_WRITE32(pAd, TX_RTS_CFG, pAd->CommonCfg.OldRtsRetryLimit);
			// Restore BBP

			BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, pAd->CommonCfg._R66);
			BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R69, pAd->CommonCfg._R69);
			BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R70, pAd->CommonCfg._R70);
			BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R73, pAd->CommonCfg._R73);

#ifdef DFS_DEBUG
			if ((pAd->CommonCfg.McuRadarDebug & RADAR_GPIO_DEBUG))
				_RTMP_IO_WRITE32(0xa0300630, 0x2000);
#endif // DFS_DEBUG //
			// EnableNormalTx
			RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &Value);
			Value |= 0x04;
			RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, Value);

			

		}
		pAd->CommonCfg.McuCarrierDetectCount++;
	}
	else // FREE_FOR_TX
	{
		if (pAd->CommonCfg.McuRadarEvent & RADAR_EVENT_CTS_CARRIER_SENT)
		{
			pAd->CommonCfg.McuRadarEvent &= ~RADAR_EVENT_CTS_CARRIER_SENT;
			pAd->CommonCfg.McuCarrierDetectCount = 1;

			RTMP_IO_WRITE32(pAd, TX_RTS_CFG, pAd->CommonCfg.OldRtsRetryLimit);
			// Restore BBP
			BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, pAd->CommonCfg._R66);
			BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R69, pAd->CommonCfg._R69);
			BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R70, pAd->CommonCfg._R70);
			BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R73, pAd->CommonCfg._R73);


#ifdef DFS_DEBUG
			if ((pAd->CommonCfg.McuRadarDebug & RADAR_GPIO_DEBUG))
				_RTMP_IO_WRITE32(0xa0300630, 0x2000);
#endif // DFS_DEBUG //
			// EnableNormalTx
			RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &Value);
			Value |= 0x04;
			RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, Value);
		}
	}	
}


void inline MCURadarDetect(PRTMP_ADAPTER pAd)
{
	ULONG Value;
	UCHAR bbp;
	
			BBP_IO_READ8_BY_REG_ID(pAd, BBP_R113, &bbp);
			if (bbp & 0x20)
				return;

			//DisableNormalTx
			RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &Value);
			RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, 0);
			RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, 0xc);
			Value &= (ULONG)~(0x04);
			RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, Value);
						
#ifdef DFS_DEBUG
			if ((pAd->CommonCfg.McuRadarDebug & RADAR_GPIO_DEBUG))
				if (pAd->CommonCfg.McuRadarCtsProtect != 1)
					_RTMP_IO_WRITE32(0xa030062c, 0x2000);
#endif // DFS_DEBUG //


			RTMP_IO_READ32(pAd, TX_RTS_CFG, &pAd->CommonCfg.OldRtsRetryLimit);
			RTMP_IO_WRITE32(pAd, TX_RTS_CFG, 0);
			
			//change BBP now
			if (pAd->CommonCfg.McuRadarEvent & RADAR_EVENT_CARRIER_DETECTING)
			{
				UCHAR BBPR4;
				
				// Clear False CCA
				RTMP_IO_READ32(pAd, RX_STA_CNT1, &Value);
				
				// save old value
				BBP_IO_READ8_BY_REG_ID(pAd, BBP_R66, &pAd->CommonCfg._R66);
				BBP_IO_READ8_BY_REG_ID(pAd, BBP_R69, &pAd->CommonCfg._R69);
				BBP_IO_READ8_BY_REG_ID(pAd, BBP_R70, &pAd->CommonCfg._R70);
				BBP_IO_READ8_BY_REG_ID(pAd, BBP_R73, &pAd->CommonCfg._R73);

				BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &BBPR4);
				if (BBPR4 & 0x18) // BW 40
				{
					if (pAd->CommonCfg.Channel > 14)
					{
						// BW 40, A band
						BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, 0x50);
					}
					else
					{
						// BW 40, G band
						BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, 0x40);
					}
					BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R69, 0x30);
					BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R70, 0x30);
					BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R73, 0x08);
				}
				else // BW 20
				{
					if (pAd->CommonCfg.Channel > 14)
					{
						// BW 20, A band
						BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, 0x60);
					}
					else
					{
						// BW 20, G band
						BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, 0x30);
					}
					BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R69, 0x28);
					BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R70, 0x28);
					BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R73, 0x08);
				}
				
			}
			else // RADAR_DETECTION
			{
				// save old value				
				BBP_IO_READ8_BY_REG_ID(pAd, BBP_R65, &pAd->CommonCfg._R65);
				BBP_IO_READ8_BY_REG_ID(pAd, BBP_R66, &pAd->CommonCfg._R66);

				BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R65, pAd->CommonCfg.R65);
				BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, pAd->CommonCfg.R66);
				BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R113, 0x21);
			}		
			return;
}

#ifdef CONFIG_RALINK_RT3052

#define CARRIER_DETECT_RECHECK_TIME		10
#define CARRIER_DETECT_CRITIRIA			300

void RTMPHandleRadarInterrupt(PRTMP_ADAPTER  pAd)
{
	UINT32 value, delta;
	
	RTMP_IO_READ32(pAd, PBF_LIFE_TIMER, &value);
	delta = (value >> 4) - pAd->CommonCfg.CarrierDetect.TimeStamp;
	if (pAd->CommonCfg.CarrierDetect.CarrierDebug)
		DBGPRINT(RT_DEBUG_TRACE, ("~%d\n", delta));
	pAd->CommonCfg.CarrierDetect.TimeStamp = value >> 4;
	
	if (pAd->CommonCfg.CarrierDetect.CD_State == CD_NORMAL)
	{
		if ((delta < pAd->CommonCfg.CarrierDetect.criteria) && (pAd->CommonCfg.CarrierDetect.recheck))
			pAd->CommonCfg.CarrierDetect.recheck --;

		if (pAd->CommonCfg.CarrierDetect.recheck == 0)	
		{
			// declare carrier sense
			pAd->CommonCfg.CarrierDetect.CD_State = CD_SILENCE;
			pAd->CommonCfg.CarrierDetect.recheck = CARRIER_DETECT_RECHECK_TIME;
			DBGPRINT(RT_DEBUG_TRACE, ("Carrier Detected\n"));

			// disconnect all STAs behind AP.
			//MacTableReset(pAd);
			
			// stop all TX actions including Beacon sending.
			AsicDisableSync(pAd);

		}
	}
	else if (pAd->CommonCfg.CarrierDetect.CD_State == CD_SILENCE)
	{
		// check if carrier gone??
		if ((delta > pAd->CommonCfg.CarrierDetect.criteria * 2) && (pAd->CommonCfg.CarrierDetect.recheck))
		{
			pAd->CommonCfg.CarrierDetect.CD_State = CD_NORMAL;
			pAd->CommonCfg.CarrierDetect.recheck = CARRIER_DETECT_RECHECK_TIME;
			DBGPRINT(RT_DEBUG_TRACE, ("Carrier gone\n"));
			// start all TX actions.
			APMakeAllBssBeacon(pAd);
			APUpdateAllBeaconFrame(pAd);
			AsicEnableBssSync(pAd);
		}
		
	}
	
	if (pAd->CommonCfg.CarrierDetect.Enable)
	{
		ToneRadarProgram(pAd);
		ToneRadarEnable(pAd);
	}
}



static void ToneRadarProgram(PRTMP_ADAPTER pAd)
{
	UCHAR bbp;
	// programe delta delay & division bit
	BBP_IO_WRITE8_BY_REG_ID(pAd, 184, 0xf0);
	bbp = pAd->CommonCfg.CarrierDetect.delta << 4;
	bbp |= (pAd->CommonCfg.CarrierDetect.div_flag & 0x1) << 3;
	BBP_IO_WRITE8_BY_REG_ID(pAd, 185, bbp);
	
	// program threshold
	BBP_IO_WRITE8_BY_REG_ID(pAd, 184, 0x34);
	BBP_IO_WRITE8_BY_REG_ID(pAd, 185, pAd->CommonCfg.CarrierDetect.threshold3);
	
	BBP_IO_WRITE8_BY_REG_ID(pAd, 184, 0x24);
	BBP_IO_WRITE8_BY_REG_ID(pAd, 185, pAd->CommonCfg.CarrierDetect.threshold2);
	
	BBP_IO_WRITE8_BY_REG_ID(pAd, 184, 0x14);
	BBP_IO_WRITE8_BY_REG_ID(pAd, 185, pAd->CommonCfg.CarrierDetect.threshold1);

	BBP_IO_WRITE8_BY_REG_ID(pAd, 184, 0x04);
	BBP_IO_WRITE8_BY_REG_ID(pAd, 185, pAd->CommonCfg.CarrierDetect.threshold0);
}

static void ToneRadarEnable(PRTMP_ADAPTER pAd)
{
	BBP_IO_WRITE8_BY_REG_ID(pAd, 184, 0x05);
}


void NewCarrierDetectionStart(PRTMP_ADAPTER pAd)
{	
	// Init Carrier Detect
	if (pAd->CommonCfg.CarrierDetect.Enable)
	{
		pAd->CommonCfg.CarrierDetect.TimeStamp = 0;
		pAd->CommonCfg.CarrierDetect.delta = 7;
		pAd->CommonCfg.CarrierDetect.div_flag = 0;
		pAd->CommonCfg.CarrierDetect.threshold0 = 0xff;
		pAd->CommonCfg.CarrierDetect.threshold1 = 0xff;
		pAd->CommonCfg.CarrierDetect.threshold2 = 0xff;
		pAd->CommonCfg.CarrierDetect.threshold3 = 0x0f;
		pAd->CommonCfg.CarrierDetect.recheck = CARRIER_DETECT_RECHECK_TIME;
		pAd->CommonCfg.CarrierDetect.criteria = CARRIER_DETECT_CRITIRIA;
		ToneRadarProgram(pAd);
		ToneRadarEnable(pAd);
	}
	
}


#endif

#endif // #if defined(DFS_SUPPORT) || defined(CARRIER_DETECTION_SUPPORT)



