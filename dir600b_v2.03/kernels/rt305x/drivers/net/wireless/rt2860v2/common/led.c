#include <linux/config.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/timer.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>

#include "rt_config.h"
#include "define.h"
#include "rt28xx.h"

//#include "rt2860.h"
//#include "2570.h"
//#pragma SAVE
//#pragma	OPTIMIZE(8)

#define LED_CHECK_INTERVAL 	50 /* 50ms */

typedef union   _sAbstractMacLedCfg{
        struct  {
            unsigned char AbstractMacLedCfg_0:1;
            unsigned char AbstractMacLedCfg_1:1;
            unsigned char AbstractMacLedCfg_2:1;
            unsigned char AbstractMacLedCfg_3:1;
            unsigned char AbstractMacLedCfg_4:1;
            unsigned char AbstractMacLedCfg_5:1;
            unsigned char AbstractMacLedCfg_6:1;
            unsigned char AbstractMacLedCfg_7:1;
        }       field;
        unsigned char	byte;
}       sAbstractMacLedCfg, *PsAbstractMacLedCfg;

typedef union   _sTemp{
        struct  {
            unsigned char Temp_0:1;
            unsigned char Temp_1:1;
            unsigned char Temp_2:1;
            unsigned char Temp_3:1;
            unsigned char Temp_4:1;
            unsigned char Temp_5:1;
            unsigned char Temp_6:1;
            unsigned char Temp_7:1;
        }       field;
        unsigned char   byte;
}       sTemp, *PsTemp;

RALINK_TIMER_STRUCT LedCheckTimer;
void ChgSignalStrengthLed(void);
void ChgMacLedCfg(void);
void LedCtrlMain(void);
void SetLinkStatus(void);

enum { MacLedOff=0, MacLedFastBlink=1, MacLedSlowBlink=2, MacLedOn=3	};

unsigned char LinkStatus;
unsigned char SignalStrength;
unsigned char LedBlinkTimer= 0xff;
unsigned char GPIOPolarity;
//unsigned char LedBlinkTimerEn;
unsigned long BlinkFor8sTimer;
unsigned char CurrentLedCfg;
unsigned char CurrentLedPolarity;
volatile unsigned char *McuIntStatus = (unsigned char *)(RT2860_CSR_ADDR + 0x0414);
volatile unsigned char *GPIODir = (unsigned char *)(RT2860_CSR_ADDR + 0x0229);
volatile unsigned char *GPIO = (unsigned char *)(RT2860_CSR_ADDR + 0x0228);
volatile unsigned long *MacLedCfg = (unsigned long *)(RT2860_CSR_ADDR + 0x102c);
union uLedParameter LedParameter;
unsigned char Debug[16];
sAbstractMacLedCfg AbstractMacLedCfg;
sTemp Temp;
unsigned char cnt ;


#if 0
xdata	BYTE	Debug[16]				_at_	0x7040;
bdata	BYTE	AbstractMacLedCfg		_at_	0x2d;
sbit	AbstractMacLedCfg_7	= AbstractMacLedCfg ^ 7;
sbit	AbstractMacLedCfg_6	= AbstractMacLedCfg ^ 6;
sbit	AbstractMacLedCfg_5	= AbstractMacLedCfg ^ 5;
sbit	AbstractMacLedCfg_4	= AbstractMacLedCfg ^ 4;
sbit	AbstractMacLedCfg_3	= AbstractMacLedCfg ^ 3;
sbit	AbstractMacLedCfg_2	= AbstractMacLedCfg ^ 2;
sbit	AbstractMacLedCfg_1	= AbstractMacLedCfg ^ 1;
sbit	AbstractMacLedCfg_0	= AbstractMacLedCfg ^ 0;

bdata	BYTE	Temp					_at_	0x2e;
sbit	Temp_7	= Temp ^ 7;
sbit	Temp_6	= Temp ^ 6;
sbit	Temp_5	= Temp ^ 5;
sbit	Temp_4	= Temp ^ 4;
sbit	Temp_3	= Temp ^ 3;
sbit	Temp_2	= Temp ^ 2;
sbit	Temp_1	= Temp ^ 1;
sbit	Temp_0	= Temp ^ 0;

data	BYTE CurrentLedPolarity			_at_	0x5e;
data	BYTE CurrentLedCfg				_at_	0x5f;
#endif

// Produce CurrentLedCfg and CurrentLedPolarity from LedCfgAct/LedCfgAG/LedPolarity and LinkStatus
// CurrentLedCfg format:
//  [7:4] LED ACT configuration
// 	  0x00: Solid off when Tx
//    0x01: Solid on when Tx
//    0x10: Blink when transmitting data and management packet
//    0x11: Blink when transmitting data, management packet and beacon
//    01xx: Slow blink when no traffic
//  [3:2] LED A configuration
//  [1:0] LED G configuration
//    00: Off		01: On		10: Slow blink		11: Fast blink
// AbstractMacLedCfg format:
//  [5:4] ACT configuration
//  [3:2] LED A configuration
//  [1:0] LED G configuration
//    00: Off		01: On		10: Slow blink		11: Fast blink
// CurrentLedPolarity format:
//  [3] LED ACT polarity inversion when link to A
//  [2] LED ACT polarity
//  [1] LED A
//  [0] LED G
//
//						H2MMailbox
//	SetLinkStatusEntry()	<------------- Driver
//		|
//		V
//	SetLinkStatus()
//		|
//		+--->	CurrentLedPolarity, CurrentLedCfg are generated
//		
//	LedCtrlMainEntry()	<----------------- Driven by 50ms/500ms tick when LedMode is not 0
//		|
//		V
//	LedCtrlMain()
//		|
//		V
//	ChgMacLedCfg()	(The default LED control module)
//		^
//		|----	AbstractMacLedCfg is generated here. MAC LED state is changed in ChgMacLedCfg()
//		

void LedCtrlMainEntry(void)	{	LedCtrlMain();		}	// @ 0x1022
void SetLinkStatusEntry(void)	{	SetLinkStatus();	}	// @ 0x1025
static BYTE FastBlinkOnce;								// Make sure fast_blink blinks at least once
BYTE CheckTimerEbl=0;

// Called when host sets LED mode.
// Called in interrupt level
void SetLinkStatus()
{
	BYTE TempLedCfgAct=0, TempLedCfgAG=0;

	switch(LinkStatus & 0xf0)
	{
		case 0x00:		// Radio off
		{
			TempLedCfgAG		= LedParameter.Idv.LedCfgAGLo;
			TempLedCfgAct		= LedParameter.Idv.LedCfgActLo;
			CurrentLedPolarity	= LedParameter.Idv.LedPolarityLo;
			break;
		}
		case 0x20:		// Radio on but not link up
		{
			TempLedCfgAG		= LedParameter.Idv.LedCfgAGLo	 >> 4;
			TempLedCfgAct		= LedParameter.Idv.LedCfgActLo	 >> 4;
			CurrentLedPolarity	= LedParameter.Idv.LedPolarityLo >> 4;
  			break;
		}
		case 0x60:		// Radio on and link to G
		{
			TempLedCfgAG		= LedParameter.Idv.LedCfgAGHi;
			TempLedCfgAct		= LedParameter.Idv.LedCfgActHi;
			CurrentLedPolarity	= LedParameter.Idv.LedPolarityHi;
  			break;
		}
		case 0xa0:		// Radio on and link to A
		{
			TempLedCfgAG		= LedParameter.Idv.LedCfgAGHi	 >> 4;
			TempLedCfgAct		= LedParameter.Idv.LedCfgActHi	 >> 4;
			CurrentLedPolarity	= LedParameter.Idv.LedPolarityHi >> 4;
  			break;
		}
		case 0x10:		// WPS
		{
			if(LedParameter.Idv.LedMode == 0x04)
			{
				ClearBit(CurrentLedPolarity, 2);
				TempLedCfgAct		= 0x09;	// Force ACT to solid on/off
			}
			if(LedParameter.Idv.LedMode == 0x05)
			{
				SetBit(CurrentLedPolarity, 2);
				TempLedCfgAct		= 0x09;	// Force ACT to solid on/off
			}
			break;
		}
	}
	BlinkFor8sTimer=0;
	FastBlinkOnce=0;
	CurrentLedCfg = (TempLedCfgAct << 4) | (TempLedCfgAG & 0x0f);
	Debug[4]  = CurrentLedCfg;
	Debug[5]  = CurrentLedPolarity;
	Debug[6]  = LedParameter.Idv.LedCfgActLo;
	Debug[7]  = LedParameter.Idv.LedCfgAGLo;
	Debug[8]  = LedParameter.Idv.LedPolarityLo;
	Debug[9]  = LinkStatus;
	Debug[10] = TempLedCfgAct;
	Debug[11] = TempLedCfgAG;
	//LedBlinkTimerEn = 1;

	if(CheckTimerEbl==0){
		RTMPInitTimer(NULL, &LedCheckTimer, LedCtrlMain, NULL, TRUE);
		LedCtrlMain();
		CheckTimerEbl=1;
	}
}



// Entry for Led control.  Triggered by timer per 50ms
void LedCtrlMain()
{
	Debug[0] ++;
	Debug[2] = LedParameter.Idv.LedMode;


	switch(LedParameter.Idv.LedMode)
	{
		case 0:		// Hardware controlled mode. Just ignore it.
			return;
		case 0x40:	// In addition to mode 0, set signal strength LED
			ChgSignalStrengthLed();
		default:
			ChgMacLedCfg();
			break;
	}

	if(LedBlinkTimer!=0)
		LedBlinkTimer--;
	else
		LedBlinkTimer = 0xff;

	RTMPSetTimer(&LedCheckTimer, LED_CHECK_INTERVAL);
//	DisableInt();
//	TaskStatus4	= 0;
//	EnableInt();
}


// Change LED State according to CurrentLedPolarity, CurrentLedCfg, and Tx activity
// The result is stored in AbstractMacLedCfg (Abstract MacLedCfg).
// AbstractMacLedCfg is an abstract layer. Since MAC's LED_CFG doesn't match firmware requirement totally,
// this abstract layer is to hide these difference.  
void ChgMacLedCfg()
{
	BYTE DataTxActivity, BeaconTxActivity;
//	DataTxActivity		= (McuIntStatus & 0x0e);		// Check if there is Tx Traffic

//	DisableInt();
	if(*McuIntStatus & 0x0e)	// Check if there is Tx Traffic
	{
		if(LedParameter.Idv.LedMode==2 && LinkStatus==0x20)	
			BlinkFor8sTimer = 0x800;
		DataTxActivity = 1;
	}
	else {
		DataTxActivity = BlinkFor8sTimer;
	}
//	EnableInt();

	BeaconTxActivity	= GetBit(*McuIntStatus, 4);		// Check if there are beacon
	*McuIntStatus		= 0x1e;							// Clear Tx and beacon Tx complete interrupt

	
	// Step 1. Generate AbstractMacLedCfg based their own polarity
	AbstractMacLedCfg.byte = (CurrentLedCfg & 0x0f);
	if(FastBlinkOnce || (DataTxActivity && GetBit(CurrentLedCfg, 5)) || (BeaconTxActivity && ((CurrentLedCfg & 0x30) == 0x30)))		// Tx data or Tx beacon
	{
		if(FastBlinkOnce == 0)	
			FastBlinkOnce = 12;			// Fast blink when Tx
		else					
			FastBlinkOnce --;
		AbstractMacLedCfg.field.AbstractMacLedCfg_4 = 1;
		AbstractMacLedCfg.field.AbstractMacLedCfg_5 = 1;
	}
	else if(/*DataTxActivity &&*/ GetBit(CurrentLedCfg, 6))	// 2006/12/26
	{														// Slow blink if not Tx
		AbstractMacLedCfg.field.AbstractMacLedCfg_4 = 0;
		AbstractMacLedCfg.field.AbstractMacLedCfg_5 = 1;
	}
	else
	{
		AbstractMacLedCfg.field.AbstractMacLedCfg_4 = 1;							// Solid on/off if not Tx (Temporarily set to on)
		AbstractMacLedCfg.field.AbstractMacLedCfg_5 = 0;
	}

	Debug[6] = AbstractMacLedCfg.byte;

	// Step 2. Generate correct AbstractMacLedCfg by referring to CurrentLedPolarity
	// if( (Drive high or low) & (Drive low) )	
	if((!AbstractMacLedCfg.field.AbstractMacLedCfg_1) && (!GetBit(CurrentLedPolarity, 0)))	
		AbstractMacLedCfg.field.AbstractMacLedCfg_0 = !AbstractMacLedCfg.field.AbstractMacLedCfg_0;
	if((!AbstractMacLedCfg.field.AbstractMacLedCfg_3) && (!GetBit(CurrentLedPolarity, 1)))	
		AbstractMacLedCfg.field.AbstractMacLedCfg_2 = !AbstractMacLedCfg.field.AbstractMacLedCfg_2;
	if((!AbstractMacLedCfg.field.AbstractMacLedCfg_5) && (!GetBit(CurrentLedPolarity, 2)))	
		AbstractMacLedCfg.field.AbstractMacLedCfg_4 = !AbstractMacLedCfg.field.AbstractMacLedCfg_4;

	Debug[7] = AbstractMacLedCfg.byte;
	
	// Step 3. The real Led Control. Map AbstractMacLedCfg to MAC register definition
	// MacLedCfg[3] definition:
	//  [30]   : Polarity (fixed to positive or 1)
	//  [29:28]: A
	//  [27:26]: G
	//  [25:24]: ACT
	//  00: off, 01: blink, 10: slow blink, 11: on (For firmware internal definition, 01: on, 11: blink)
	Temp.byte = 0x40;
	if(!AbstractMacLedCfg.field.AbstractMacLedCfg_1){	
		Temp.field.Temp_2 = Temp.field.Temp_3 = AbstractMacLedCfg.field.AbstractMacLedCfg_0;			//Not blink case
	}
	else
	{
		if(AbstractMacLedCfg.field.AbstractMacLedCfg_0)	
			Temp.field.Temp_2 = Temp.field.Temp_3 = GetBitX(LedBlinkTimer, 3);		//Fast blink
		else					
			Temp.field.Temp_2 = Temp.field.Temp_3 = GetBitX(LedBlinkTimer, 7);		//Slow bink
	}		
	if(!AbstractMacLedCfg.field.AbstractMacLedCfg_3){
		Temp.field.Temp_4 = Temp.field.Temp_5 = AbstractMacLedCfg.field.AbstractMacLedCfg_2;
	}
	else
	{
		if(AbstractMacLedCfg.field.AbstractMacLedCfg_2)	
			Temp.field.Temp_4 = Temp.field.Temp_5 = GetBitX(LedBlinkTimer, 3);		//Fast blink
		else					
			Temp.field.Temp_4 = Temp.field.Temp_5 = GetBitX(LedBlinkTimer, 7);		//Slow bink
	}		
	if(!AbstractMacLedCfg.field.AbstractMacLedCfg_5){	
		Temp.field.Temp_0 = Temp.field.Temp_1 = AbstractMacLedCfg.field.AbstractMacLedCfg_4;
	}
	else
	{
		if(AbstractMacLedCfg.field.AbstractMacLedCfg_4)	
			Temp.field.Temp_0 = Temp.field.Temp_1 = GetBitX(LedBlinkTimer, 3);		//Fast blink
		else					
			Temp.field.Temp_0 = Temp.field.Temp_1 = GetBitX(LedBlinkTimer, 7);		//Slow bink
	}		
	{
		/*
		 * Spec. 2.1 Pin Description Summary
		 * Green LED -> LED_ACT_N (ACT)
		 * Red LED -> LED_RDYA_N (A)
		 * Yellow LED ->LED_RDYG_N (G)
		 *
		 * Spec. Register Description - MAC: LED_CFG (offset:0x102C)
		 * [30]: LED_POL
		 * [29:28]: Y_LED_MODE -> (G)
		 * [27:26]: G_LED_MODE -> (ACT)
		 * [25:24]: R_LED_MODE -> (A)
		 *
		 * Original LED driver definition
		 * Temp.byte[6]:LED_POL
		 * Temp.byte[5:4]:LED_A
		 * Temp.byte[3:2]:LED_G
		 * Temp.byte[1:0]:LED_ACT
		 *
		 * We need to extract related bits and remap it.
		 *
		 */
		//if(cnt%255==0) {
		//printk("********** before *MacLedCfg = %x\n", *MacLedCfg);
		BYTE LED_A, LED_ACT, LED_G, LED_POL;
		LED_POL = (Temp.byte & 0x40)>>6; //extract bit[6]
		LED_A = (Temp.byte & 0x30)>>4; //extract bit[5:4] 
		LED_G = (Temp.byte & 0x0c)>>2; //extract bit[3:2]
		LED_ACT = Temp.byte & 0x03;    //extract bit[1:0]
		Temp.byte = (LED_POL<<6) | (LED_G<<4) | (LED_ACT<<2) | LED_A;

		*MacLedCfg &=0x00FFFFFF;
		*MacLedCfg |= Temp.byte<<24;
		//printk("********** after *MacLedCfg = %x\n", *MacLedCfg);
		//}
		cnt++;
	}
}


void ChgSignalStrengthLed()
{
	//printk("%s:%s() %d\n", __FILE__, __FUNCTION__, __LINE__);
	*GPIODir = 0x00;			// Set GPIO to output
	*GPIO	= GPIOPolarity ? SignalStrength : ~SignalStrength;
}

	
