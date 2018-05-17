#include "../pcm_ctrl.h"
extern int g729ab_decode_init();
extern int g729ab_encode_init();
int voice_init_codec(codec_data_type* pcodec)
{	
	switch(pcodec->type)
	{
		case G711ULAW_CODEC:
		case G711ALAW_CODEC:
#if	defined(PCM_SW_L2U)||defined(PCM_SW_L2A) 		
			pcodec->pPCMBuf8 = kmalloc(PCM_8KHZ_SAMPLES*PCM_SAMPLE_SIZE, GFP_KERNEL);
			pcodec->PCMBufWordLen = PCM_8KHZ_SAMPLES;
			pcodec->BitBufByteLen = PCM_8KHZ_SAMPLES*PCM_SAMPLE_SIZE;
			if(pcodec->pPCMBuf8==NULL)
				goto CODEC_OOM;
			pcodec->pBitBuf = kmalloc(PCM_8KHZ_SAMPLES, GFP_KERNEL);
			if(pcodec->pBitBuf==NULL)
				goto CODEC_OOM;
#else
			pcodec->type = 0;
			pcodec->BitBufByteLen = 0;	
			pcodec->PCMBufWordLen = 0;
#endif				
			break;
		case G729AB_CODEC:
#if	defined(PCM_SW_G729AB)	
			pcodec->PCMBufWordLen = PCM_8KHZ_SAMPLES;
			pcodec->pPCMBuf8 = kmalloc(PCM_8KHZ_SAMPLES*PCM_SAMPLE_SIZE, GFP_KERNEL);
			if(pcodec->pPCMBuf8==NULL)
				goto CODEC_OOM;
			pcodec->pBitBuf = kmalloc((PCM_8KHZ_SAMPLES+2)*PCM_SAMPLE_SIZE, GFP_KERNEL);
			if(pcodec->pBitBuf==NULL)
				goto CODEC_OOM;
			set_g729encVoIPCh(pcodec->ch);
			set_g729decVoIPCh(pcodec->ch);
			g729ab_encode_init();
			g729ab_decode_init();
#else
			pcodec->type = 0;
			pcodec->BitBufByteLen = 0;
			pcodec->PCMBufWordLen = 0;	
#endif			
			break;
		default:
			break;	
	}
	
	return 0;
	
CODEC_OOM:
	return -1;		
}	

void voice_encode_frame(codec_data_type* pcodec)
{
	switch(pcodec->type)
	{
		case G711ULAW_CODEC:
#if	defined(PCM_SW_L2U)		
			ulaw_compress(pcodec->PCMBufWordLen, pcodec->pPCMBuf16, pcodec->pBitBuf);
#else
			pcodec->BitBufByteLen = 0;				
#endif			
			break;
		case G711ALAW_CODEC:
#if	defined(PCM_SW_L2A)		
			alaw_compress(pcodec->PCMBufWordLen, pcodec->pPCMBuf16, pcodec->pBitBuf);
#else
			pcodec->BitBufByteLen = 0;				
#endif			
			break;
		case G729AB_CODEC:
#if	defined(PCM_SW_G729AB)			
			set_g729encVoIPCh(pcodec->ch);
			pcodec->BitBufByteLen = g729ab_encode_frame(pcodec->pPCMBuf16, pcodec->pBitBuf, 1);
#else
			pcodec->BitBufByteLen = 0;			
#endif			
			break;
		default:
			break;		
	}	
}

void voice_decode_frame(codec_data_type* pcodec)
{
	switch(pcodec->type)
	{
		case G711ULAW_CODEC:
#if	defined(PCM_SW_L2U)		
			ulaw_expand(pcodec->PCMBufWordLen, pcodec->pBitBuf, pcodec->pPCMBuf16);
#endif			
			break;
		case G711ALAW_CODEC:
#if	defined(PCM_SW_L2A)			
			alaw_expand(pcodec->PCMBufWordLen, pcodec->pBitBuf, pcodec->pPCMBuf16);
#endif			
			break;
		case G729AB_CODEC:
#if	defined(PCM_SW_G729AB)			
			set_g729decVoIPCh(pcodec->ch);
			g729ab_decode_frame(pcodec->pBitBuf, (short*)pcodec->pPCMBuf16, 0);
#endif			
			break;
		default:
			break;		
	}	
}

void voice_release_codec(codec_data_type* pcodec)
{
#if	defined(PCM_SW_L2U)||defined(PCM_SW_L2A)||defined(PCM_SW_G729AB)	
	if(pcodec->pPCMBuf8)
		kfree(pcodec->pPCMBuf8);
	if(pcodec->pBitBuf)
		kfree(pcodec->pBitBuf);
#endif		
	//pcodec->PCMBufWordLen = 0;
	//pcodec->ch = 0;
	//pcodec->type = 0;
	
	return;	
}

/*
int CHtoSessionMapping(int ch)
{
	return maptbl[ch]
}
*/	