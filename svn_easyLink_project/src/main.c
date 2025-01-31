
#include "..\inc\global.h"


//#define _STAND_ALONE_TEST 1

Clss_TransParam		g_stClssTransParam;
EMV_PARAM	g_stEmvParam;
extern EMVParamInfo			g_stEmvParamInfo;

// amount type 
#define AMOUNT		0
#define RFDAMOUNT	1
#define ORGAMOUNT	2
#define TIPAMOUNT	3

#define DISP_LINE_LEFT      0x80
#define DISP_LINE_RIGHT     0x40
#define DISP_LINE_CENTER    0x20
#define DISP_SCR_CENTER     0x0100
#define DISP_LINE_REVER     0x0200
#define DISP_PART_REVER     0x0400

#define PORT_USB		11

// 刷卡/插卡事件
#define NO_SWIPE_INSERT		0x00	// 没有刷卡/插卡
#define CARD_SWIPED			0x01	// 刷卡
#define CARD_INSERTED		0x02	// 插卡
#define CARD_KEYIN			0x04	// 手输卡号

#define SKIP_DETECT_ICC		0x20	// 不检查ICC插卡事件 // 防止用户拔卡过快[12/12/2006 Tommy]
#define FALLBACK_SWIPE		0x40	// 刷卡(FALLBACK)
#define SKIP_CHECK_ICC		0x80	// 不检查service code

#ifndef uchar
#define uchar unsigned char
#endif
#ifndef uint
#define uint unsigned int
#endif
#ifndef ulong
#define ulong unsigned long
#endif
#ifndef ushort
#define ushort unsigned short

#endif

const APPINFO AppInfo={
	"EasyLink",
	"EasyLink_AID",
	"1.00.00",
	"PAX TECHNOLOGY",
	"EasyLink Lite",
	"\x20\x17\x04\x26\x10\x45\x00",
	0,
	0,
	0,
	""
};

int event_main(ST_EVENT_MSG *msg)
{
	SystemInit();
	return 0;
}

int ReadFileInfo(void)
{
	int iRet=0, iCnt=0;
	ST_TABLE_FS_INFO stTblFsInfo;

	iRet = TableFsInit();
	if(TBL_RET_NOT_FMT == iRet)
	{
		for(iCnt=0; iCnt<3; iCnt++)
		{
			iRet = TableFsFormat(TABLE_FS_BLOCK_SIZE);
			if(iRet == TBL_RET_OK)
			{
				break;
			}
		}
	}

	memset(&stTblFsInfo, 0, sizeof(stTblFsInfo));
	TableFsGetInfo(&stTblFsInfo);
	if(stTblFsInfo.usBlkLen != TABLE_FS_BLOCK_SIZE)
	{
		// prompt error
	}

	TableFsRead(FILE_EMV_PARAM_INFO_BLOCK_START, 0, sizeof(EMVParamInfo), (uchar *)&g_stEmvParamInfo);

	return iRet;
}


// 删除前导字符
// delete the specified continuous characters on the left of the string
void PubTrimHeadChars(uchar *pszString, uchar ucRemoveChar)
{
	uchar	*p;

	if( !pszString || !*pszString )
	{
		return;
	}

	for(p=pszString; *p && *p==ucRemoveChar; p++);
	if( p!=pszString )
	{
		while( (*pszString++ = *p++) );
	}
}

// 填充前导字符,以便达到uiTargetLen长度
// Pad leading charactes till specific length.
void PubAddHeadChars( uchar *pszString, uint uiTargetLen, uchar ucAddChar )
{
	uint	uiLen;

	if (pszString == NULL)
	{
		return;
	}

	uiLen = (uint)strlen((char *)pszString);
	if( uiLen>=uiTargetLen )
	{
		return;
	}

	memmove(pszString+uiTargetLen-uiLen, pszString, uiLen+1);
	memset(pszString, ucAddChar, uiTargetLen-uiLen);
}


int CalcDispX(uchar *ucStr, uchar ucFontSize)
{
	int iLen = 0;

	if(!ucStr)
	{
		return -1;
	}

	iLen = strlen(ucStr);
	if( ucFontSize == 1 )
	{
		if(iLen < 16)
		{
			if(iLen>(int)strlen("Welcome"))
			{
				iLen=32-((iLen-strlen("Welcome"))/2)*8;
			}
			else
			{
				iLen=32+((strlen("Welcome")-iLen)/2)*8;
			}
		}
		else
		{	
			iLen = 0;
		}
	}
	else if( ucFontSize == 0)
	{
		iLen = 0;	// x = 0
	}

	return iLen;
}


int InputAmount(unsigned char *pucAmount, int iMinLen, int iMaxLen)
{
	int i=0, iLen=0, iTemp=0;
	unsigned char ucKey=0x00, aucBuff[12], aucAmount[12+1];;

	memset(aucBuff, 0, sizeof(aucBuff));
	memset(aucAmount, 0, sizeof(aucAmount));
	//memcpy(aucAmount, "0.00", 4);

	setCustomizedFont(1);
	while(1)
	{
		//		sprintf((char *)aucAmount, "%10.2s", (char *)aucBuff);
		iLen = strlen(aucBuff);
		if(iLen > 0)
		{
			memset(aucAmount, 0, sizeof(aucAmount));

			if(iLen < 3)
			{
				strcpy(aucAmount, "0.");
				strcpy(aucAmount+2, aucBuff);
			}
			else
			{
				strcpy(aucAmount, aucBuff);
				aucAmount[iLen] = aucAmount[iLen-1];
				aucAmount[iLen-1] = aucAmount[iLen-2];
				aucAmount[iLen-2] = '.';
			}
			//ScrTextOut(70, 30, aucAmount);
			//PubDispString(aucAmount, 3|DISP_LINE_RIGHT);

			ScrTextOut(40, 30, aucAmount);
		}		

		if(kbhit()!=NOKEY)
		{
			ucKey = getkey();
		}
		else
			continue;

		if((ucKey>=KEY0) && (ucKey<=KEY9))
		{
			if(i==iMaxLen)
			{
				Beep();
				continue;
			}
			aucBuff[i] = ucKey;
			i++;
			//ScrClrLine(2,5);
			continue;
		}
		switch(ucKey)
		{
		case KEYCLEAR:if(i==0)
						  Beep();
					  else
					  {;
			aucBuff[i-1]='\0';
			i--;
			ScrClrLine(2,5);
					  }
					  break;
		case KEYENTER:if(i >= iMinLen) 
					  {
						  Beep();
						  //pucAmount = aucBuff;
						  memcpy(pucAmount, aucBuff, i);
						  return 0;
						  break;
					  }
					  else
						  return 0;
		case KEYCANCEL:return 1;
		default:return -1;
		}
	}


}

// 获取终端当前时间,格式:YYYYMMDDhhmmss
// format : YYYYMMDDhhmmss
void GetDateTime(uchar *pszDateTime)
{
	uchar	sCurTime[7];

	GetTime(sCurTime);
	sprintf((char *)pszDateTime, "%02X%02X%02X%02X%02X%02X%02X",
		(sCurTime[0]>0x80 ? 0x19 : 0x20), sCurTime[0], sCurTime[1],
		sCurTime[2], sCurTime[3], sCurTime[4], sCurTime[5]);
}


void GetTransAmount(void)
{
	unsigned char aucAmount[12+1], aucCashBackAmount[12+1];

	ScrCls();
	PubDispString("PLS INPUT AMT: ", 0|DISP_LINE_LEFT);
	InputAmount(aucAmount, 1, 10);

	g_stClssTransParam.ulAmntAuth = PubAsc2Long(aucAmount, strlen(aucAmount), &g_stClssTransParam.ulAmntAuth);

	strcpy(g_stClssTransParam.aucTransDate, "");

	ScrCls();
	PubDispString("CASHBACK: ", 0|DISP_LINE_LEFT);
	PubDispString("YES or NO ?", 4|DISP_LINE_RIGHT);
	if(PubYesNo(5) == 0)
	{
		ScrCls();
		PubDispString("PLS CASHBACK: ", 0|DISP_LINE_LEFT);
		InputAmount(aucCashBackAmount, 1, 10);
		g_stClssTransParam.ulAmntOther = PubAsc2Long(aucCashBackAmount, strlen(aucCashBackAmount), &g_stClssTransParam.ulAmntOther);
	}

}

void InitUSB()
{
	PortOpen(PORT_USB, (uchar *)"115200,8,n,1");
}


void RemoveCard(void)
{
	ScrCls();
	PubDispString("PLS REMOVE CARD", 0|DISP_LINE_LEFT);
	Beef(1,500); 
	DelayMs(500);
	Beef(1,500); 
	while(PiccDetect(0, NULL, NULL, NULL, NULL) == 0);
}

void loadTestKey(void)
{
	int iRet;
	ST_KEY_INFO stTpkInfo;	//TPK
	ST_KCV_INFO stKcvInfo;

	//PedErase(); 
	memset(&stTpkInfo, 0, sizeof(ST_KEY_INFO));
	stTpkInfo.ucSrcKeyType = PED_TPK;
	stTpkInfo.ucSrcKeyIdx = 0;
	stTpkInfo.ucDstKeyType = PED_TPK;
	stTpkInfo.ucDstKeyIdx = 1;
	stTpkInfo.iDstKeyLen = 16;
	memcpy(stTpkInfo.aucDstKeyValue, "\x11\x22\x33\x44\x55\x66\x77\x88\x99\x11\x22\x33\x44\x55\x66\x77", 16);

	memset(&stKcvInfo, 0, sizeof(ST_KCV_INFO));
	stKcvInfo.iCheckMode = 0x00;

	iRet = PedWriteKey(&stTpkInfo, &stKcvInfo);
#ifdef DEBUG_
	AppLog("[loadTestKey] PedWriteKey=%d, TPK\n", iRet, 1);
#endif
}


int TestParamManage(void)
{
	int iRet=0;
	unsigned char ucData[200]={0};
	unsigned char *pTLVData = "\x57\x13\x55\x09\x89\x00\x04\x04\x24\x78\xD2\x20\x52\x21\x00\x00\x03\x11\x00\x00\x0F\x5A\x08\x55\x09\x89\x00\x04\x04\x24\x78\x5F\x24\x03\x22\x05\x31\x5F\x2A\x02\x03\x44\x5F\x34\x01\x01\x82\x02\x19\x80\x84\x07\xA0\x00\x00\x00\x04\x10\x10\x95\x05\x00\x00\x00\x00\x00\x9A\x03\x16\x09\x19\x9C\x01\x00\x9F\x02\x06\x00\x00\x00\x00\x06\x53\x9F\x03\x06\x00\x00\x00\x00\x00\x00\x9F\x08\x02\x00\x02\x9F\x09\x02\x00\x02\x9F\x10\x12\x01\x10\xA0\x40\x01\x24\xC0\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xFF\x9F\x1A\x02\x03\x44\x9F\x1E\x08\x11\x22\x33\x44\x55\x66\x77\x88\x9F\x26\x08\x28\xC7\x76\xE2\xC8\x76\x79\x9C\x9F\x27\x01\x80\x9F\x33\x03\xE0\xF8\xC8\x9F\x34\x03\x1F\x03\x02\x9F\x35\x01\x22\x9F\x36\x02\x00\x8D\x9F\x37\x04\x1F\x8C\x34\xB9\x9F\x53\x01\x52";
	unsigned char *pConfigData = "\x02\x01\x04\x30\x30\x33\x30\x02\x02\x01\x01\x02\x03\x01\x99";
	unsigned char *pConfigTag = "\x02\x01\x02\x02\x02\x03";
	unsigned char ucFailedTagListOut[100]={0};
	unsigned int uiFailedTagListLenOut=0;
	unsigned char ucTmp[128]={0};

	//test set EMV TLV data.
	ucData[0] = PARAMMGMT_DATATYPE_EMV;
	memcpy(ucData + 3, pTLVData, 186);
	ucData[1] = 0;
	ucData[2] = 186;

	iRet = SetData(ucData, ucFailedTagListOut, &uiFailedTagListLenOut);
#ifdef DEBUG_
	AppLog("SetData set EMV TLV data.\n");
	AppLog("SetData = %d\n", iRet);
	PubBcd2Asc(ucFailedTagListOut, uiFailedTagListLenOut, ucTmp);
	AppLog("ucFailedTagListOut = %s\n", ucTmp);
	AppLog("uiFailedTagListLenOut = %d\n", uiFailedTagListLenOut);
#endif

	//test set Config data.
	memset(ucData, 0, sizeof(ucData));
	memset(ucFailedTagListOut, 0, sizeof(ucFailedTagListOut));
	memset(ucTmp, 0, sizeof(ucTmp));
	uiFailedTagListLenOut = 0;
	ucData[0] = PARAMMGMT_DATATYPE_CONFIG;
	memcpy(ucData + 3, pConfigData, 15);
	ucData[1] = 0;
	ucData[2] = 15;

	iRet = SetData(ucData, ucFailedTagListOut, &uiFailedTagListLenOut);
#ifdef DEBUG_
	AppLog("SetData set config data.\n");
	AppLog("SetData = %d\n", iRet);
	PubBcd2Asc(ucFailedTagListOut, uiFailedTagListLenOut, ucTmp);
	AppLog("ucFailedTagListOut = %s\n", ucTmp);
	AppLog("uiFailedTagListLenOut = %d\n", uiFailedTagListLenOut);
#endif

	//test get Config data.
	memset(ucData, 0, sizeof(ucData));
	memset(ucFailedTagListOut, 0, sizeof(ucFailedTagListOut));
	memset(ucTmp, 0, sizeof(ucTmp));
	uiFailedTagListLenOut = 0;

	ucData[0] = PARAMMGMT_DATATYPE_CONFIG;
	memcpy(ucData + 3, pConfigTag, 6);
	ucData[1] = 0;
	ucData[2] = 6;

	iRet = GetData(ucData, ucFailedTagListOut, &uiFailedTagListLenOut);
#ifdef DEBUG_
	AppLog("GetData get config data.\n");
	AppLog("GetData = %d\n", iRet);
	PubBcd2Asc(ucFailedTagListOut, uiFailedTagListLenOut, ucTmp);
	AppLog("ucTLVOut = %s\n", ucTmp);
	AppLog("uiTLVLenOut = %d\n", uiFailedTagListLenOut);
#endif

	return iRet;
}

void SysInfoInit(void)
{
	uchar ucTermInfo[30]={0}, ucTemp[32]={0};
	int iWidth=0, iHeight=0;

	memset(ucTemp, 0, sizeof(ucTemp));
	ReadSN(ucTemp);
	SetConfigurationData(CMD_TERM_SN, ucTemp, strlen(ucTemp));

	GetTermInfo(ucTermInfo);
	SetConfigurationData(CMD_TERM_MODEL_CODE, ucTermInfo, 1);
	SetConfigurationData(CMD_TERM_PRINTER_INFO, ucTermInfo + 1, 1);
	SetConfigurationData(CMD_TERM_MODEM_EXIST, ucTermInfo + 2, 1);
	SetConfigurationData(CMD_TERM_PCI_EXIST, ucTermInfo + 5, 1);
	SetConfigurationData(CMD_TERM_USB_EXIST, ucTermInfo + 7, 1);
	SetConfigurationData(CMD_TERM_LAN_EXIST, ucTermInfo + 8, 1);
	SetConfigurationData(CMD_TERM_GPRS_EXIST, ucTermInfo + 9, 1);
	SetConfigurationData(CMD_TERM_CDMA_EXIST, ucTermInfo + 10, 1);
	SetConfigurationData(CMD_TERM_WIFI_EXIST, ucTermInfo + 11, 1);
	SetConfigurationData(CMD_TERM_RF_EXIST, ucTermInfo + 12, 1);
	SetConfigurationData(CMD_TERM_IC_EXIST, ucTermInfo + 15, 1);
	SetConfigurationData(CMD_TERM_MAG_EXIST, ucTermInfo + 16, 1);
	SetConfigurationData(CMD_TERM_TILT_EXIST, ucTermInfo + 17, 1);
	SetConfigurationData(CMD_TERM_WCDMA_EXIST, ucTermInfo + 18, 1);
	SetConfigurationData(CMD_TERM_TOUCHSCREEN_EXIST, ucTermInfo + 19, 1);

	if ( ucTermInfo[0] ) {
		memset(ucTemp, 0, sizeof(ucTemp));
		ucTemp[0] = 0;
		SetConfigurationData(CMD_TERM_COLORSCREEN_EXIST, ucTemp, 1);
	}

	ScrGetLcdSize(&iWidth, &iHeight);
	memset(ucTemp, 0, sizeof(ucTemp));
	ucTemp[0] = iWidth / 256;
	ucTemp[1] = iWidth % 256;
	ucTemp[2] = iHeight / 256;
	ucTemp[3] = iHeight % 256;
	SetConfigurationData(CMD_TERM_SCREEN_SIZE, ucTemp, 4);

	SetConfigurationData(CMD_TERM_APP_NAME, AppInfo.AppName, strlen(AppInfo.AppName));
	SetConfigurationData(CMD_TERM_APP_VERSION, AppInfo.AppVer, strlen(AppInfo.AppVer));

	//g_tTermInfo.ucTermAppSoName,
	//g_tTermInfo.ucTermAppSoVer,
	//g_tTermInfo.ucTermPubSoName,
	//g_tTermInfo.ucTermPubSoVer,

	//test, init.
	memset(ucTemp, 0, sizeof(ucTemp));
	ucTemp[0] = '\x00';
	ucTemp[1] = '\x00';
	SetConfigurationData(CMD_TERM_APP_VERSION, ucTemp, 2);

	memset(ucTemp, 0, sizeof(ucTemp));
	ucTemp[0] = APPPARAM_FALLBACK_ALLOW;
	SetConfigurationData(CMD_APP_FALLBACK_ALLOW, ucTemp, 1);

	memset(ucTemp, 0, sizeof(ucTemp));
	ucTemp[0] = (uchar)BatteryCheck();
	SetConfigurationData(CMD_TERM_BATTERY_INFO, ucTemp, 1);

	// Added by ZhangYuan 2016-12-19, set default value
	memset(ucTemp, 0, sizeof(ucTemp));
	ucTemp[0] = 6;
	SetConfigurationData(CMD_APP_PAN_MASK_START_POS, ucTemp, 1);

	//g_tTermInfo.ucTermRestFSSize,
	//g_tTermInfo.ucTermOSName,
	//g_tTermInfo.ucTermOSVer,
}



#ifdef _STAND_ALONE_TEST
int main(void)
{
	int iRet=0;
	unsigned char aucDateTime[24+1], ucEntryMode=0x00;
	unsigned char ucTmp[128]={0};

	SystemInit();

#ifndef _WIN32
	EMVCoreInit();
#endif

	InitUSB();

	//	WriteEMVPARA();

	EMVLoadAppFromFile();

	loadTestKey();

#if 1
	SetTime("\x08\x12\x31\x10\x10\10");
#endif

	TestParamManage();


	g_stClssTransParam.ulTransNo = 1;
	g_stClssTransParam.ucTransType = EMV_GOODS;

	memset(aucDateTime, 0, sizeof(aucDateTime));
	GetDateTime(aucDateTime); // 20 160818 143702

	PubAsc2Bcd(aucDateTime+2, 6, g_stClssTransParam.aucTransDate);
	PubAsc2Bcd(aucDateTime+8, 6, g_stClssTransParam.aucTransTime);

	//	sprintf((char *)g_stClssTransParam.aucTransDate, "%.3s",  aucDateTime+1);
	//	sprintf((char *)g_stClssTransParam.aucTransTime, "%.3s",  aucDateTime+4);

	memcpy(g_stEmvParam.CountryCode, "/x08/x00", 2);
	memcpy(g_stEmvParam.ReferCurrCode, "/x08/x00", 2);
	memcpy(g_stEmvParam.TransCurrCode, "/x08/x00", 2);

	ScrCls();
	PubDispString("Start", 0|DISP_LINE_LEFT);

	while (1) {
		GetTransAmount();

		ScrCls();
		PubDispString("Processing...", 0|DISP_LINE_LEFT);

		iRet = StartTransaction();
		if(iRet == EL_EMV_RET_TRANS_REQUEST_GO_ONLINE)
		{
			// online processing

			iRet = CompleteTransaction();
		}

		RemoveCard();

		ScrCls();
		PubDispString("Next Transaction ", 0|DISP_LINE_LEFT);
		getkey();
	}
	return 0;
}
#else
int main(void)
{
	SystemInit();
	//InitComm();	
	SysFileInit();
#ifndef _WIN32
	EMVCoreInit();
#endif

	//PedGenerate();
	SysInfoInit();
	ReadCommParam();

	InitComm();

	//	WriteEMVPARA();

	//	EMVLoadAppFromFile();

	//loadTestKey();

	if(PdkFile_Exist(UI_FILE_NAME) != PDK_RET_OK)
	{
		g_UiFileExist = 0;//not exist
		{
			char pDispStr[DISP_MAX_LEN+1] = "PLS DOWN PARAM FILE";
			ScrCls();
			ScrPrint(0,2,0,"%s",pDispStr);
			PubWaitKey(30);
			ScrCls();
		}
	}

	while(1)
	{
		//	    MainMenu();
		//		InitComm();
		Processor();
	}

	return 0;
}
#endif
