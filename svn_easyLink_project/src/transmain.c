/*------------------------------------------------------------
* FileName: transmain.c
* Author: liukai
* Date: 2016-08-15
------------------------------------------------------------*/

#include "..\inc\global.h"

#define LOG_TAG	__FILE__
#ifdef TRANS_DEBUG_		

#define EL_LOG(enLogPrio, pucTag, pucFunc, iLine, pucFmtStr,...)		\
	El_Log(enLogPrio, pucTag, pucFunc, iLine, pucFmtStr, ##__VA_ARGS__)

#define EL_LOG_HEX(enLogPrio, pucTag, pucFunc, iLine, pucMsg, pucHexData, iHexDataLen)	\
	El_Log_Hex(enLogPrio, pucTag, pucFunc, iLine, pucMsg, pucHexData, iHexDataLen)

#else

#define EL_LOG_HEX(enLogPrio, pucTag, pucFunc, iLine, pucMsg, pucHexData, iHexDataLen)
#define EL_LOG(enLogPrio, pucTag, pucFunc, iLine, pucFmtStr, ...)


#endif // TRANS_DEBUG_		


extern Clss_TransParam g_stClssTransParam;

#define CARD_ENTRY_MODE_SWIPE		0x01
#define CARD_ENTRY_MODE_INSERT		0x02
#define CARD_ENTRY_MODE_FALLBACK		0x04
#define CARD_ENTRY_MODE_TAP			0x10

uchar g_ucCardEntryMode = CARD_ENTRY_MODE_SWIPE | CARD_ENTRY_MODE_INSERT; 

extern uchar *ClssProc_RetToString(int ReturnCode);

static void ConfigueTransParamInit(void)
{
	unsigned char ucTemp[12]={0};

	SetConfigurationData(CMD_TXN_CUR_TXN_TYPE, ucTemp, 0);
	SetConfigurationData(CMD_TXN_CUR_CLSS_TYPE, ucTemp, 0xFF);	// modified by ZhangYuan 2016-12-19, initialize with 0xff
	SetConfigurationData(CMD_TXN_PATH_TYPE, ucTemp, 0xFF);	// modified by ZhangYuan 2016-12-19, initialize with 0xff
	SetConfigurationData(CMD_TXN_TRACK_ONE_DATA, ucTemp, 0);
	SetConfigurationData(CMD_TXN_TRACK_TWO_DATA, ucTemp, 0);
	SetConfigurationData(CMD_TXN_TRACK_THREE_DATA, ucTemp, 0);
	SetConfigurationData(CMD_TXN_EXPIRE_DATE, ucTemp, 0);
	SetConfigurationData(CMD_TXN_ONLINE_RESULT, ucTemp, 0);
	SetConfigurationData(CMD_TXN_RSP_CODE, ucTemp, 0);
	SetConfigurationData(CMD_TXN_AUTH_CODE, ucTemp, 0);
	SetConfigurationData(CMD_TXN_AUTH_DATA, ucTemp, 0);
	SetConfigurationData(CMD_TXN_AUTH_DATA_LEN, ucTemp, 0);
	SetConfigurationData(CMD_TXN_ISSUER_SCRIPT, ucTemp, 0);
	SetConfigurationData(CMD_TXN_ISSUER_SCRIPT_LEN, ucTemp, 0);
	SetConfigurationData(CMD_TXN_ONLINE_PIN_INPUT, ucTemp, 0);
	SetConfigurationData(CMD_TXN_PIN_BLOCK, ucTemp, 0);
	SetConfigurationData(CMD_TXN_KSN, ucTemp, 0);
	//	SetConfigurationData(CMD_TXN_ICS_TYPE, ucTemp, 0);	//added by Kevin Liu 20161110	// deleted by ZhangYuan 2016-12-19, no need to reset

	SetConfigurationData(CMD_TXN_MASKEDPAN, ucTemp, 0);	// Added by ZhangYuan 2016-12-19
	SetConfigurationData(CMD_TXN_CVM_TYPE, ucTemp, 0);	// Added by ZhangYuan 2016-12-19
	SetConfigurationData(CMD_TXN_CARD_PROCESSING_RESULT, ucTemp, 0);	// Added by ZhangYuan 2017-04-26

	memset(g_tTransParam.ucPAN, 0, sizeof(g_tTransParam.ucPAN));
}

static int SaveClssPreProcParam(const unsigned char *pucTag, int iTagLen, const unsigned char *pucData, int iDataLen)
{
	int i=0;

	//ulAmntAuth
	if ( 0 == memcmp(pucTag, "\x9f\x02", iTagLen) ) {
		if ( iDataLen != 6 ) {
			return EL_TRANS_RET_EMV_DATA_ERR;
		}
		for ( i=0; i < iDataLen; i++ ) {
			if ( pucData[i] != '\x00' ) {
				break;
			}
		}
		PubBcd2Long(pucData + i, iDataLen - i, &g_stClssTransParam.ulAmntAuth);
	} 
	//ulAmntOther
	else if ( 0 == memcmp(pucTag, "\x9f\x03", iTagLen) ) {
		if ( iDataLen != 6 ) {
			return EL_TRANS_RET_EMV_DATA_ERR;
		}
		for ( i=0; i < iDataLen; i++ ) {
			if ( pucData[i] != '\x00' ) {
				break;
			}
		}
		PubBcd2Long(pucData + i, iDataLen - i, &g_stClssTransParam.ulAmntOther);
	}
	//ucTransType
	else if ( 0 == memcmp(pucTag, "\x9c", iTagLen) ) {
		if ( iDataLen != 1 ) {
			return EL_TRANS_RET_EMV_DATA_ERR;
		}
		g_stClssTransParam.ucTransType = pucData[0];
	}
	//aucTransDate
	else if ( 0 == memcmp(pucTag, "\x9a", iTagLen) ) {
		if ( iDataLen != 3 ) {
			return EL_TRANS_RET_EMV_DATA_ERR;
		}
		memcpy(g_stClssTransParam.aucTransDate, pucData, iDataLen);
	}
	//aucTransTime
	else if ( 0 == memcmp(pucTag, "\x9f\x21", iTagLen) ) {
		if ( iDataLen != 3 ) {
			return EL_TRANS_RET_EMV_DATA_ERR;
		}
		memcpy(g_stClssTransParam.aucTransTime, pucData, iDataLen);
	}
	else {
	}

	return EMV_OK;
}

int LoadPreProcParam(void)
{
	int iRet=0, i=0, iTagListLen=0, iTagLen=0, iDataLen=0, iLengthLen=0, iOffset=0, iLengthSubsequentByte=0, iCount=0;
	unsigned short usTag=0;
	unsigned char aucBuf[4096+1]={0}, aucData[256+1]={0};

	memset(&g_stClssTransParam, 0, sizeof(Clss_TransParam));
	memset(aucBuf, 0, sizeof(aucBuf));
	iRet = GetAllTLVDataFromMem(aucBuf, &iTagListLen);
#ifdef DEBUG_
	AppLog("[GetAllTLVDataFromMem] GetAllTLVDataFromFS ret = %d\n", iRet);
#endif

	if ( iRet != EMV_OK ) {
		return iRet;
	}

	while(iOffset < iTagListLen) {		
		//calculate tag length, refer to EMV tag Field structure
		if ( 0x1F == (aucBuf[iOffset] & 0x1F) ) {
			if ( 0x80 == (aucBuf[iOffset + 1] & 0x80) ) {
				iTagLen = 3;
			} else {
				iTagLen = 2;
				usTag = aucBuf[iOffset] * 256 + aucBuf[iOffset + 1];
			}
		} else {
			iTagLen = 1;
			usTag = aucBuf[iOffset];
		}

		//check length
		if ( (unsigned int)(iOffset + iTagLen) > iTagListLen ) {
			break;
		}

		//calculate data length, refer to EMV tag Field structure
		iDataLen = 0;
		iLengthSubsequentByte = 0;
		if ( 0x80 == (aucBuf[iOffset + iTagLen] & 0x80) ) {
			iLengthSubsequentByte = aucBuf[iOffset + iTagLen] & 0x7F;
			for ( i = iLengthSubsequentByte - 1; i > 0; i++ ) {
				iDataLen += aucBuf[iOffset + iTagLen + i] + 256 * iCount;
				iCount++;
			}
		} else {
			iDataLen = aucBuf[iOffset + iTagLen];
		}

		//check length
		iLengthLen = 1 + iLengthSubsequentByte;
		if ( (unsigned int)(iOffset + iTagLen + iLengthLen + iDataLen) > iTagListLen) {
			break;
		}

		//get data for EMV and Paywave
		memset(aucData, 0, sizeof(aucData));
		memcpy(aucData, aucBuf + iOffset + iTagLen + iLengthLen, iDataLen);

		iRet = SaveClssPreProcParam(aucBuf + iOffset, iTagLen, aucData, iDataLen);
		if ( iRet != EMV_OK ) {
			return iRet;
		}

		iOffset = iOffset + iTagLen + iLengthLen + iDataLen;
	}

	return EMV_OK;
}


static int TransInit(void)
{
	int iRet=0;

	ConfigueTransParamInit();
	LoadTLVDataFromFSToMem();
	LoadPreProcParam();

	/* removed by Kevin 20161212, Clss Kernel and EMV kernel will influence each other. init function will clean all app and capk.
	#ifndef _WIN32
	ClssTransInit();
	#endif
	EMVInitTransaction();	// Added by ZhangYuan 2016-11-24
	*/

	MagOpen();

	iRet = PiccOpen();
	if ( iRet != 0 )
	{
		PiccClose();
		return iRet;
	}
	return EMV_OK;
}

// 提示拔出IC卡
void PromptRemoveICC(void)
{
	IccClose(ICC_USER);
	if( IccDetect(ICC_USER)!=0 )	// 如果IC卡已拔出，直接返回。
	{
		return;
	}

	// 显示并等待IC卡拔出

#ifdef EUI_
	EShowMsgBox(EL_UI_PAGE_TRANS_REMOVE_CARD, NULL, 0);
#else
	ScrCls();
	PubDispString("PLS REMOVE CARD", 0|DISP_LINE_LEFT);
#endif

	while( IccDetect(ICC_USER)==0 )
	{
		Beep();
		DelayMs(500);
	}
}

// 从磁道信息分析出卡号(PAN)
// get PAN from track
int GetPanFromTrack(const uchar *pucTrackTwoData, const uchar *pucTrackThreeData, uchar *pszPAN, uchar *pszExpDate)
{
	int		iPanLen;
	char	*p, *pszTemp;

	// 从2磁道开始到'＝'
	// read data from the start of track #2 to '-'
	if( strlen((char *)pucTrackTwoData)>0 )
	{
		pszTemp = (char *)pucTrackTwoData;
	}
	else if( strlen((char *)pucTrackThreeData)>0 )
	{
		pszTemp = (char *)&pucTrackThreeData[2];
	}
	else
	{	// 2、3磁道都没有
		// track #2 and #3 are not existed
		return -1;
	}

	p = strchr((char *)pszTemp, '=');
	if( p==NULL )
	{
		return -1;
	}
	iPanLen = p - pszTemp;
	if( iPanLen<13 || iPanLen>19 )
	{
		return -1;
	}

	sprintf((char *)pszPAN, "%.*s", iPanLen, pszTemp);
	if( pszTemp==(char *)pucTrackTwoData )
	{
		sprintf((char *)pszExpDate, "%.4s", p+1);
	}
	else
	{
		sprintf((char *)pszExpDate, "0000");
	}

	return 0;
}

int ReadMagCardInfo(void)
{
	unsigned char ucRet=0;
	unsigned char ucTrackOneData[79+1]={0}, ucTrackTwoData[40+1]={0}, ucTrackThreeData[107+1]={0};
	unsigned char ucPAN[19+1]={0}, ucExpDate[4+1]={0};

	ucRet = MagRead(ucTrackOneData, ucTrackTwoData, ucTrackThreeData);
	if ( ucRet == 0x00 ) {	//0x00 means error.
#ifdef EUI_
		EShowMsgBox(EL_UI_PAGE_TRANS_READ_CARD_ERR, NULL, 0);
#else
		ScrCls();
		PubDispString("READ CARD ERR", 0|DISP_LINE_LEFT);
#endif
		DelayMs(100);
		ComPoseInfoMsg(ClssProc_RetToString, EL_MAG_RET_READ_ERR, EL_MAG_RET_READ_ERR, __LINE__);	// Added by ZhangYuan 2016-12-07
		return EL_MAG_RET_READ_ERR;
	}

	EL_LOG(LOG_DEBUG, __FILE__, __FUNCTION__, __LINE__, "TrackOneData=%s, TrackTwoData = %s, TrackThreeData = %s\n", ucTrackOneData, ucTrackTwoData, ucTrackThreeData);

	SetConfigurationData(CMD_TXN_TRACK_ONE_DATA, ucTrackOneData, strlen(ucTrackOneData));
	SetConfigurationData(CMD_TXN_TRACK_TWO_DATA, ucTrackTwoData, strlen(ucTrackTwoData));
	SetConfigurationData(CMD_TXN_TRACK_THREE_DATA, ucTrackThreeData, strlen(ucTrackThreeData));

	GetPanFromTrack(ucTrackTwoData, ucTrackThreeData, ucPAN, ucExpDate);
	memcpy(g_tTransParam.ucPAN, ucPAN, strlen(ucPAN));
//	SetConfigurationData(CMD_TXN_PAN_DATA, g_tTransParam.ucPAN, strlen(g_tTransParam.ucPAN));	// Added by ZhangYuan 2017-04-17
	
	memcpy(g_tTransParam.ucExpDate, ucExpDate, strlen(ucExpDate));
	SetConfigurationData(CMD_TXN_EXPIRE_DATE, ucExpDate, strlen(ucExpDate));	// Added by ZhangYuan 2016-12-19

	EL_LOG(LOG_DEBUG, __FILE__, __FUNCTION__, __LINE__, "ucPAN = %s\n", ucPAN);

	return EMV_OK;
}

// Check service code in track 2, whther it is 2 or 6
unsigned char IsChipCard(const uchar *pszTrack2)
{
	char *pszSeperator=NULL;

	if( *pszTrack2==0 )
	{
		return FALSE;
	}

	pszSeperator = strchr((char *)pszTrack2, '=');
	if( pszSeperator==NULL )
	{
		return FALSE;
	}
	if( (pszSeperator[5]=='2') || (pszSeperator[5]=='6') )
	{
		return TRUE;
	}

	return FALSE;
}

// Added by ZhangYuan 2016-12-19
static void MaskPAN(void)
{
	// Masked PAN
	int i=0;
	unsigned char aucMaskPan[32]={0};

	for(i=0;i<strlen(g_tTransParam.ucPAN);i++)
	{
		if((i>=(g_tAppParam.ucPANMaskStartPos[0]))&&(i<(strlen(g_tTransParam.ucPAN)-4)))
		{
			aucMaskPan[i]='*';
		}
		else
		{
			aucMaskPan[i]=g_tTransParam.ucPAN[i];
		}
	}		
	SetConfigurationData(CMD_TXN_MASKEDPAN, aucMaskPan, strlen(g_tTransParam.ucPAN));
}


//Magnetic stripe card process.
static int MagCardStartTransaction(void)
{
	int iRet=0;
	unsigned char ucTemp[32]={0};

	memset(ucTemp, 0, sizeof(ucTemp));
	ucTemp[0] = TRANS_TYPE_MAGNETIC;
	SetConfigurationData(CMD_TXN_CUR_TXN_TYPE, ucTemp, 1);

	iRet = ReadMagCardInfo();
	if ( iRet != 0 ) {
		return iRet;
	}

	if ( (g_ucIsFallBack == TRUE) && (g_tAppParam.ucFallbackAllowFlag[0] == APPPARAM_FALLBACK_ALLOW) ) {
		if ( IsChipCard(g_tTransParam.ucTrackTwoData) ) {
			memset(ucTemp, 0, sizeof(ucTemp));
			ucTemp[0] = TRANS_TYPE_FALLBACK_MAGNETIC;
			SetConfigurationData(CMD_TXN_CUR_TXN_TYPE, ucTemp, 1);
			return EMV_OK;
		} else {
#ifdef EUI_
			EShowMsgBox(EL_UI_PAGE_TRANS_RESWIPE_CARD, NULL, 0);
#else
			ScrCls();
			PubDispString("RESWIPE CARD", 0|DISP_LINE_LEFT);
#endif
			Beep();
			return EL_MAG_RET_RESWIPE;
		}
	} else if ( IsChipCard(g_tTransParam.ucTrackTwoData) ) {
#ifdef EUI_
		EShowMsgBox(EL_UI_PAGE_TRANS_INSERT_CARD, NULL, 0);
#else
		ScrCls();		
		PubDispString("PLZ INSERT CARD", 0|DISP_LINE_LEFT);
#endif
		Beep();
		return EL_MAG_RET_INSERT_CARD;
	} else {
		return EMV_OK;
	}

	return EMV_OK;
}

int StartTransaction(void)
{
	int iRet=0;
	unsigned char ucTemp[48]={0};
	unsigned char szTotalAmt[12+1]={0};
	unsigned char ucACType=AC_AAC;

	g_ucIsFallBack = 0x00;
	g_ucCardEntryMode = CARD_ENTRY_MODE_SWIPE | CARD_ENTRY_MODE_INSERT | CARD_ENTRY_MODE_TAP; 	

	iRet = TransInit();
	if ( iRet ) {
		return iRet;
	}

#ifdef EUI_
	EShowMsgBox(EL_UI_PAGE_TRANS_GET_CARD, NULL, 0);
#else
	ScrCls();
	PubDispString("INSERT/SWIPE/TAP", 0|DISP_LINE_LEFT);
	PubDispString("CARD", 2|DISP_LINE_CENTER);
#endif

	memset(ucTemp, 0, sizeof(ucTemp));
	memset(szTotalAmt, 0, sizeof(szTotalAmt));

	TimerSet(TIMER_TRANS, 600); // Added by ZhangYuan 2016-10-13 add timer for StartTransaction

	while( 1 ) {
		//detect swipe card
		if ( (g_ucCardEntryMode & CARD_ENTRY_MODE_SWIPE || g_ucCardEntryMode & CARD_ENTRY_MODE_FALLBACK) 
			&& 0 == MagSwiped() )  {			
				iRet = MagCardStartTransaction();
				if ( (iRet != EL_MAG_RET_RESWIPE) && (iRet != EL_MAG_RET_INSERT_CARD) ) {
					break;
				}
		}

		//detect insert card
		if ( (g_ucCardEntryMode & CARD_ENTRY_MODE_INSERT) && 0 == IccDetect(0) ) {

			if(g_ucCardEntryMode & CARD_ENTRY_MODE_FALLBACK) // Added by ZhangYuan 2016-10-19
			{
#ifdef EUI_
				EShowMsgBox(EL_UI_PAGE_TRANS_FALL_BACK, NULL, 0);
#else
				ScrCls();
				PubDispString("FALL BACK", 0|DISP_LINE_LEFT);
				PubDispString("PLS SWIPE CARD", 2|DISP_LINE_LEFT);
#endif
				Beep();
				continue;
			}

			iRet = EMVStartTransaction(&ucACType);
			if ( iRet == EL_EMV_RET_FALLBACK ) {
				PromptRemoveICC();
#ifdef EUI_
				EShowMsgBox(EL_UI_PAGE_TRANS_FALL_BACK, NULL, 0);
#else
				ScrCls();
				PubDispString("FALL BACK", 0|DISP_LINE_LEFT);
				PubDispString("PLS SWIPE CARD", 2|DISP_LINE_LEFT);
#endif
				Beep();
				g_ucCardEntryMode &= ~CARD_ENTRY_MODE_INSERT; // Added by ZhangYuan 2016-10-19
				continue;
			}

			if(ucACType != AC_ARQC) // Added by ZhangYuan 2016-11-30, if request go online, not remove ICC
			{
				PromptRemoveICC();	//TODO delete it, just use for test version
			}

			break;
		}

		//detect tap card
		if ( (g_ucCardEntryMode & CARD_ENTRY_MODE_TAP) && 0 == PiccDetect(0, NULL, NULL, NULL, NULL) ) 
		{
#ifndef _WIN32
			iRet = ClssStartTransaction();
#endif
			if(iRet == App_Try_Again)	// by ZhangYuan 2017-03-22
			{
#ifdef EUI_
				EShowMsgBox(EL_UI_PAGE_TRANS_TAP_AGAIN, NULL, 0);
#else
				ScrCls();
				PubDispString("PLS TAP CARD", 0|DISP_LINE_LEFT);
#endif

				g_ucCardEntryMode = CARD_ENTRY_MODE_TAP;
				PiccOpen();
				continue;
			}
			else if(iRet == CLSS_USE_CONTACT || iRet == EL_TRANS_RET_CLSS_USE_CONTACT)	// if CTLS request to use other interface, go to detect MSR/ICC
			{
				g_ucCardEntryMode &= ~CARD_ENTRY_MODE_TAP;
				continue;
			}
			break;
		}

		//detect key input
		if ( 0 == kbhit() ) 
		{
			if ( KEYCANCEL == getkey() ) 
			{
				iRet = EL_TRANS_RET_EMV_USER_CANCEL;
				ComPoseInfoMsg(ClssProc_RetToString, EL_TRANS_RET_EMV_USER_CANCEL, iRet, __LINE__);	// Added by ZhangYuan 2016-12-07
				break;
			}
		}

		// ==== Added by ZhangYuan 2016-10-13 add timer check, if timeout, return
		if( TimerCheck(TIMER_TRANS) == 0x00)
		{
			iRet = EL_TRANS_RET_EMV_TIME_OUT;	// the return code is to be determined
			ComPoseInfoMsg(ClssProc_RetToString, EL_TRANS_RET_EMV_TIME_OUT, iRet, __LINE__);	// Added by ZhangYuan 2016-12-07
			break;
		}
		// === Added end ============================================
	}

	// Added by ZhangYuan 2016-10-17 ========
	ScrCls();
	MagClose();
	PiccClose();
	// Added end ==========================

	if ( iRet != EMV_OK ) {
		return iRet;
	}
	else {	// Added by ZhangYuan 2016-12-19
		MaskPAN();
	}

	return EMV_OK;
}

static unsigned char GetCurrentTxnType(void)
{
	return g_tTransParam.ucCurrentTxnType[0];
}

void ClssKernelInit(void)
{

}

void EMVKernelInit(void)
{

}


int CompleteTransaction(void)
{
	int iRet=0;

	if ( GetCurrentTxnType() == TRANS_TYPE_CONTACTLESS ) {
#ifndef _WIN32
		iRet = ClssCompleteTrans();
		ClssKernelInit();
#endif

	} else if ( GetCurrentTxnType() == TRANS_TYPE_EMV_CONTACT ) {
		iRet = EMVCompleteTransaction();
		PromptRemoveICC();	//Added by ZhangYuan 2016-11-30, TODO delete it, just use for test version
		EMVKernelInit();
	} else {

	}

	if ( iRet != EMV_OK ) {
		return iRet;
	}

	return EMV_OK;
}
