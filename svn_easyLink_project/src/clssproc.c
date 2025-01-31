/*------------------------------------------------------------
* FileName: clssproc.c
* Author: liukai
* Date: 2016-07-28
------------------------------------------------------------*/
#include "..\inc\global.h"

#define LOG_TAG	__FILE__
#ifdef CLSS_DEBUG_		
#define EL_LOG(enLogPrio, pucTag, pucFunc, iLine, pucFmtStr, ...)		\
	El_Log(enLogPrio, pucTag, pucFunc, iLine, pucFmtStr, ##__VA_ARGS__)

#define EL_LOG_HEX(enLogPrio, pucTag, pucFunc, iLine, pucMsg, pucHexData, iHexDataLen)	\
	El_Log_Hex(enLogPrio, pucTag, pucFunc, iLine, pucMsg, pucHexData, iHexDataLen)

#else

#define EL_LOG_HEX(enLogPrio, pucTag, pucFunc, iLine, pucMsg, pucHexData, iHexDataLen)
#define EL_LOG(enLogPrio, pucTag, pucFunc, iLine, pucFmtStr, ...)


#endif // CLSS_DEBUG_	


static Clss_TransParam sg_tClssTransParam;
static unsigned char sg_aucOutcomeParamSet[8]={0};
static unsigned char sg_aucUserInterReqData[22]={0};
static unsigned char sg_aucErrIndication[6]={0};
static CLSS_DISC_DATA_MC sg_tDiscretionayData;
static uchar sg_ucAppType = KERNTYPE_DEF;
static int sg_iAppTornLogNum=0;//number of Tornlog
static CLSS_TORN_LOG_RECORD sg_tAppTornTransLog[5];//Torn Transaction Log
static unsigned char sg_aucFinalAID[17]={0};
static unsigned char sg_ucFinalAIDLen=0;
static unsigned char sg_ucDRLSupportFlg=1;
static unsigned char sg_aucProID[17]={0};
static int sg_iProIDLen=0;
static Clss_ProgramID sg_tProgInfo;
static unsigned char sg_ucTransPath=0;
static unsigned char sg_ucACType=0;
static CLSS_OUTCOME_DATA  sg_tOutComeData;
static Clss_PreProcInfo sg_tClssPreProcInfo;

extern EMV_PARAM	g_stEmvParam;
extern EMVParamInfo		g_stEmvParamInfo;
extern Clss_TransParam	g_stClssTransParam;

//functions
//*******************************************************************************
static void AppInitDiscData_MC(void);
static void AppCreateOutcomeData_MC(int iRet);
static void AppInitPaymentData_MC(void);
static void AppSetAppType(unsigned char ucType);
static unsigned char AppGetAppType(void);
static int AppGetTornLog_MC(CLSS_TORN_LOG_RECORD *ptTornLog, int *pnTornNum);
static int AppSaveTornLog_MC(CLSS_TORN_LOG_RECORD *ptTornLog, int nTornNum);
static int AppCleanTornLog_MC(void);
static int AppLoadTornLog_MC(void);
static void SetFinalSelectAID(uchar *paucAID, uchar ucAIDLen);
static void AppSetTransPath(uchar ucTransPath);
static unsigned char AppGetTransPath(void);
static void TwoOne_app(unsigned char *pucIn, unsigned short usInlen, unsigned char *pucOut);
static int SetDETData(unsigned char *pucTag, unsigned char ucTagLen, unsigned char *pucData, unsigned char ucDataLen);
static void InitPaymentData(void);
static void InitDiscData(void);
static int AppFindMatchProID(unsigned char *pucProID, int iProIDLen);
static int AppSaveTransDataOutput_MC(uchar ucMsgFlag);
static int AppSetClss_capk(uchar index, uchar *rid, uchar kerId);
static int AppSetClss_RevLst(uchar index, uchar *rid, uchar kerId);
static void AppSetTransACType(uchar ucType);
static uchar AppGetTransACType(void);


static void Inter_DisplayMsg(int iMsg);
static void AppRemovePicc(void);
static int ClssTransInit(void);
static int ClssPreProcTxnParam(void);
static void ClssBaseParameterSet_WAVE(void);
static void ClssTermParamSet_MC(void);
static int AppSetCurAppType(unsigned char ucKernType);
static int AppTransProc_VISA(unsigned char ucTransPath, unsigned char ucACType);
static int AppTransProc_MC(uchar ucTransPath, uchar *pucACType);
static int ClssProcFlow_VISA(uchar *pucFinalData, int iFinalDataLen, Clss_PreProcInterInfo tClssProcInterInfo);
static int ClssProcFlow_MC(uchar *pucFinalData, int iFinalDataLen, Clss_PreProcInterInfo tClssProcInterInfo);
static int ClssProcFlow_ALL(void);
static int AppConv_CreateOutCome(int nRet, uchar ucACType, CLSS_OUTCOME_DATA *pstOutComeData);

static int ClssCompleteTrans_WAVE(uchar ucInOnlineResult, uchar aucIAuthData[], int nIAuthDataLen,  uchar aucScript[], int nScriptLen);

//*******************************************************************************

int ELMap2AppErrCode(int iErrCode)
{
	int szErrCodeMap[][2] = {
		{EMV_OK,			EL_RET_OK},
		{ICC_RESET_ERR, 	EL_TRANS_RET_ICC_RESET_ERR},
		{ICC_CMD_ERR,		EL_TRANS_RET_ICC_CMD_ERR},
		{ICC_BLOCK, 		EL_TRANS_RET_ICC_BLOCK},
		{EMV_RSP_ERR,		EL_TRANS_RET_EMV_RSP_ERR},
		{EMV_APP_BLOCK, 	EL_TRANS_RET_EMV_APP_BLOCK},
		{EMV_NO_APP,		EL_TRANS_RET_EMV_NO_APP},
		{EMV_USER_CANCEL,	EL_TRANS_RET_EMV_USER_CANCEL},
		{EMV_TIME_OUT, 		EL_TRANS_RET_EMV_TIME_OUT},
		{EMV_DATA_ERR,		EL_TRANS_RET_EMV_DATA_ERR},
		{EMV_NOT_ACCEPT,	EL_TRANS_RET_EMV_NOT_ACCEPT},
		{EMV_DENIAL,		EL_TRANS_RET_EMV_DATA_ERR},
		{EMV_KEY_EXP,		EL_TRANS_RET_EMV_KEY_EXP},
		{EMV_NO_PINPAD, 	EL_TRANS_RET_EMV_NO_PINPAD},
		{EMV_NO_PASSWORD,	EL_TRANS_RET_EMV_NO_PASSWORD},
		{EMV_SUM_ERR,		EL_TRANS_RET_EMV_SUM_ERR},
		{EMV_NOT_FOUND, 	EL_TRANS_RET_EMV_NOT_FOUND},
		{EMV_NO_DATA,		EL_TRANS_RET_EMV_NO_DATA},
		{EMV_OVERFLOW,		EL_TRANS_RET_EMV_OVERFLOW},
		{NO_TRANS_LOG,		EL_TRANS_RET_NO_TRANS_LOG},
		{RECORD_NOTEXIST,	EL_TRANS_RET_RECORD_NOTEXIST},
		{LOGITEM_NOTEXIST,	EL_TRANS_RET_LOGITEM_NOTEXIST},
		{ICC_RSP_6985,		EL_TRANS_RET_ICC_RSP_6985},
		{EMV_PARAM_ERR, 	EL_TRANS_RET_PARAM_ERR},
		{CLSS_USE_CONTACT,	EL_TRANS_RET_CLSS_USE_CONTACT},
		{EMV_FILE_ERR,		EL_TRANS_RET_EMV_FILE_ERR},
		{CLSS_TERMINATE,	EL_TRANS_RET_CLSS_TERMINATE},
		{CLSS_FAILED,		EL_TRANS_RET_CLSS_FAILED},
		{CLSS_DECLINE,		EL_TRANS_RET_CLSS_DECLINE},
		{CLSS_TRY_ANOTHER_CARD, 	EL_TRANS_RET_CLSS_TRY_ANOTHER_CARD},
		{CLSS_WAVE2_OVERSEA,		EL_TRANS_RET_CLSS_WAVE2_OVERSEA},
		{CLSS_WAVE2_TERMINATED, 	EL_TRANS_RET_CLSS_WAVE2_TERMINATED},
		{CLSS_WAVE2_US_CARD,		EL_TRANS_RET_CLSS_WAVE2_US_CARD},
		{CLSS_WAVE3_INS_CARD,		EL_TRANS_RET_CLSS_WAVE3_INS_CARD},
		{CLSS_RESELECT_APP, 		EL_TRANS_RET_CLSS_RESELECT_APP},
		{CLSS_CARD_EXPIRED, 		EL_TRANS_RET_CLSS_CARD_EXPIRED},
		{EMV_NO_APP_PPSE_ERR,		EL_TRANS_RET_EMV_NO_APP_PPSE_ERR},
		{CLSS_USE_VSDC, 			EL_TRANS_RET_CLSS_USE_VSDC},
		{CLSS_CVMDECLINE,			EL_TRANS_RET_CLSS_CVMDECLINE},
		{CLSS_REFER_CONSUMER_DEVICE,		EL_TRANS_RET_CLSS_REFER_CONSUMER_DEVICE},
		{CLSS_LAST_CMD_ERR, 		EL_TRANS_RET_CLSS_LAST_CMD_ERR},
		{CLSS_API_ORDER_ERR,		EL_TRANS_RET_CLSS_API_ORDER_ERR},
		{EL_TRANS_RET_EMV_FAIL, 	EL_TRANS_RET_EMV_FAIL},
		{EL_TRANS_RET_EMV_DECLINED, 		EL_TRANS_RET_EMV_DECLINED},

		{EL_TRANS_RET_END, 		EL_TRANS_RET_END},
	};

	int i=0;

	while(szErrCodeMap[i][0] != EL_TRANS_RET_END)
	{
		if(szErrCodeMap[i][0] == iErrCode)
		{			
			return szErrCodeMap[i][1];
		}
		i++;
	}

	return iErrCode;	// return, if not found
}

uchar *ClssProc_RetToString(int ReturnCode)
{
	switch(ReturnCode)
	{
	case EMV_OK:
		return "Success";
	case EL_TRANS_RET_ICC_RESET_ERR:
		return "ICC Reset Error";
	case EL_TRANS_RET_ICC_CMD_ERR:
		return "ICC Command Error";
	case EL_TRANS_RET_ICC_BLOCK:
		return "ICC Blocked";
	case EL_TRANS_RET_EMV_RSP_ERR:
		return "ICC Response Error";
	case EL_TRANS_RET_EMV_APP_BLOCK:
		return "ICC APP Block";
	case EL_TRANS_RET_EMV_NO_APP:
		return "No APP Supported by Terminal";
	case EL_TRANS_RET_EMV_USER_CANCEL:
		return "User Cancel";
	case EL_TRANS_RET_EMV_TIME_OUT:
		return "Timeout";
	case EL_TRANS_RET_EMV_DATA_ERR:
		return "Card Data Error";
	case EL_TRANS_RET_EMV_NOT_ACCEPT:
		return "Transaction Is Not Accepted";
	case EL_TRANS_RET_EMV_DENIAL:
		return "Transaction Denied";
	case EL_TRANS_RET_EMV_KEY_EXP:
		return "CAPK is Expired";
	case EL_TRANS_RET_EMV_NO_PINPAD:
		return "No PINPAD";
	case EL_TRANS_RET_EMV_NO_PASSWORD:
		return "No PIN Entered";

	case EL_TRANS_RET_EMV_SUM_ERR:
		return "Checksum of CAPK is error";
	case EL_TRANS_RET_EMV_NOT_FOUND:
		return "Not Found";
	case EL_TRANS_RET_EMV_NO_DATA:
		return "No Data Available";
	case EL_TRANS_RET_EMV_OVERFLOW:
		return "Memory is overflow";
	case EL_TRANS_RET_NO_TRANS_LOG:
		return "There is no Transaction log";
	case EL_TRANS_RET_RECORD_NOTEXIST:
		return "Record Not Exist";
	case EL_TRANS_RET_LOGITEM_NOTEXIST:
		return "Log Item Not Exist";
	case EL_TRANS_RET_ICC_RSP_6985:
		return "ICC Respond 6985";
//	case EL_TRANS_RET_CLSS_USE_CONTACT:
//		return "User Contact Interface";
	case EL_TRANS_RET_EMV_FILE_ERR:
		return "File error";
	case EL_TRANS_RET_CLSS_TERMINATE:
		return "Transaction Terminated";
	case EL_TRANS_RET_CLSS_FAILED:
		return "Transaction failed";
	case EL_TRANS_RET_CLSS_DECLINE:
		return "Transaction is Declined";
	case EL_TRANS_RET_CLSS_TRY_ANOTHER_CARD:
		return "Try Another Card";
	case EL_TRANS_RET_PARAM_ERR:
		return "Parameter Error";
	case EL_TRANS_RET_CLSS_WAVE2_OVERSEA:
		return "International Transaction";
	case EL_TRANS_RET_CLSS_WAVE2_TERMINATED:
		return "Transaction is Terminated";
	case EL_TRANS_RET_CLSS_WAVE2_US_CARD:
		return "US card";
	case EL_TRANS_RET_CLSS_WAVE3_INS_CARD:
		return "Need to use IC card for the transaction";
	case EL_TRANS_RET_CLSS_RESELECT_APP:
		return "Reselect";
	case EL_TRANS_RET_CLSS_CARD_EXPIRED:
		return "Card is expired";
	case EL_TRANS_RET_EMV_NO_APP_PPSE_ERR:
		return "No application is supported";
	case EL_TRANS_RET_CLSS_USE_VSDC:
		return "Switch to contactless PBOC";
	case EL_TRANS_RET_CLSS_CVMDECLINE:
		return "CVM result in decline for AE";
	case EL_TRANS_RET_CLSS_REFER_CONSUMER_DEVICE:
		return "Please See Phone";
	case EL_TRANS_RET_CLSS_LAST_CMD_ERR:
		return "The last read record command is error";
	case EL_TRANS_RET_CLSS_API_ORDER_ERR:
		return "APIs are called in wrong order";
	default:
		return "Transaction Failed";
	}
}

#ifndef _WIN32
int Clss_GetTLVData(unsigned int uiTag, uchar *pucData, int *piDatalen, uchar ucFlag)
{
	int iRet = 0, iLen = 0;
	uchar ucTagList[3] = {0};
	uchar ucTagListLen = 0;
	uchar ucDataOut[100] = {0};
	uint uiActualDataOutLen = 0;

	if(ucFlag == KERNTYPE_VIS)
	{
		iRet = Clss_GetTLVData_Wave(uiTag, pucData, &iLen);
	}

	else if((ucFlag= KERNTYPE_MC))
	{
		if(uiTag < 0xFF){
			ucTagListLen = 1;
		}else if((uiTag > 0xFF) && (uiTag < 0xFFFF)){
			ucTagListLen = 2;
		}else{
			ucTagListLen = 3;
		}
		memset(ucTagList, 0 ,sizeof(ucTagList));
		PubLong2Char(uiTag, ucTagListLen, ucTagList);
		if(ucFlag == KERNTYPE_MC) {
			iRet = Clss_GetTLVDataList_MC(ucTagList, ucTagListLen,
				sizeof(ucDataOut), ucDataOut, &uiActualDataOutLen);
		} else {
			return EMV_PARAM_ERR;
		}
		if(EMV_OK == iRet)
		{
			memcpy(pucData, ucDataOut, uiActualDataOutLen);
			iLen = uiActualDataOutLen;
		}
	}
	else {
		return EMV_PARAM_ERR;
	}
	*piDatalen = iLen;

	return iRet;
}

static void AppInitDiscData_MC(void)
{
	memset(&sg_tDiscretionayData, 0, sizeof(CLSS_DISC_DATA_MC));
}

static void AppCreateOutcomeData_MC(int iRet)
{
	int nErrorCode=0;

	switch(iRet)
	{
	case CLSS_USE_CONTACT:
		sg_aucErrIndication[1]=0;
		sg_aucErrIndication[3]=0;
		sg_aucErrIndication[4]=0;

		sg_aucErrIndication[1] = L2_EMPTY_CANDIDATE_LIST;//S53.13

		sg_aucUserInterReqData[0]=MI_ERROR_OTHER_CARD;//S53.14 for paypass 3.0.1 by zhoujie
		sg_aucUserInterReqData[1]=MI_IDLE;
		memcpy(sg_aucUserInterReqData+2, MSG_HOLD_TIME_VALUE, 3);
		//		nSendTransDataOutput_MC(CLSS_DATA_UIRD);//S53.14

		sg_aucOutcomeParamSet[0] = OC_END_APPLICATION;//S53.15 for paypass 3.0.1 by zhoujie
		sg_aucErrIndication[5]=MI_ERROR_OTHER_CARD;
		sg_tDiscretionayData.ucErrIndicFlg = 1;
		//		nSendTransDataOutput_MC(CLSS_DATA_OCPS | CLSS_DATA_DISD);//S53.15
		break;
	case ICC_BLOCK:
		sg_aucUserInterReqData[0]=MI_ERROR_OTHER_CARD;//s52.8 for paypass 3.0.1 by zhoujie
		sg_aucUserInterReqData[1]=MI_IDLE;
		memcpy(sg_aucUserInterReqData+2, MSG_HOLD_TIME_VALUE, 3);

		sg_aucOutcomeParamSet[0] = OC_END_APPLICATION;//s52.9 for paypass 3.0.1 by zhoujie
		sg_aucErrIndication[5]=MI_ERROR_OTHER_CARD;
		sg_tDiscretionayData.ucErrIndicFlg = 1;
		//		nSendTransDataOutput_MC(T_UIRD | T_OCPS | T_DISD);//S52.8 S52.9
		break;
	case EMV_NO_APP_PPSE_ERR:
		Clss_GetErrorCode_Entry(&nErrorCode);
		if(nErrorCode == EMV_DATA_ERR)//S52.11
		{
			sg_aucErrIndication[1]=L2_PARSING_ERROR;//S52.11 for paypass 3.0.1 by zhoujie

			sg_aucUserInterReqData[0]=MI_ERROR_OTHER_CARD;//S52.18 for paypass 3.0.1 by zhoujie
			sg_aucUserInterReqData[1]=MI_IDLE;
			memcpy(sg_aucUserInterReqData+2, MSG_HOLD_TIME_VALUE, 3);

			sg_aucOutcomeParamSet[0] = OC_END_APPLICATION;//S52.19 for paypass 3.0.1 by zhoujie
			sg_aucErrIndication[5]=MI_ERROR_OTHER_CARD;
			sg_tDiscretionayData.ucErrIndicFlg = 1;
			//			nSendTransDataOutput_MC(T_UIRD | T_OCPS | T_DISD);//S52.18 S52.19
		}
		else if (nErrorCode == EMV_RSP_ERR || nErrorCode == EMV_APP_BLOCK)
		{
			sg_aucUserInterReqData[0]=MI_ERROR_OTHER_CARD;//s52.8 for paypass 3.0.1 by zhoujie
			sg_aucUserInterReqData[1]=MI_IDLE;
			memcpy(sg_aucUserInterReqData+2, MSG_HOLD_TIME_VALUE, 3);

			sg_aucOutcomeParamSet[0] = OC_END_APPLICATION;//s52.9 for paypass 3.0.1 by zhoujie
			sg_aucErrIndication[5]=MI_ERROR_OTHER_CARD;
			sg_tDiscretionayData.ucErrIndicFlg = 1;
			//			nSendTransDataOutput_MC(T_UIRD | T_OCPS | T_DISD);//S52.8 S52.9
		}
		break;
	case EMV_NO_APP:
		sg_aucErrIndication[1] = L2_EMPTY_CANDIDATE_LIST;//S52.14

		sg_aucUserInterReqData[0]=MI_ERROR_OTHER_CARD;//S52.18 for paypass 3.0.1 by zhoujie
		sg_aucUserInterReqData[1]=MI_IDLE;
		memcpy(sg_aucUserInterReqData+2, MSG_HOLD_TIME_VALUE, 3);

		sg_aucOutcomeParamSet[0] = OC_END_APPLICATION;//S52.19 for paypass 3.0.1 by zhoujie
		sg_aucErrIndication[5]=MI_ERROR_OTHER_CARD;
		sg_tDiscretionayData.ucErrIndicFlg = 1;
		//		nSendTransDataOutput_MC(T_UIRD | T_OCPS | T_DISD);//S52.18 S52.19
		break;
	default:
		Clss_GetErrorCode_Entry(&nErrorCode);
		if(nErrorCode == EMV_NO_DATA)
		{
			sg_aucErrIndication[1] = L2_CARD_DATA_MISSING;//S1.7
		}
		else //EMV_DATA_ERR and CLSS_RESELECT_APP
		{
			sg_aucErrIndication[1] = L2_PARSING_ERROR;//S1.7
		}
		sg_aucOutcomeParamSet[0] = OC_SELECT_NEXT;
		sg_aucOutcomeParamSet[1] = OC_C;
		AppInitDiscData_MC();
		sg_tDiscretionayData.ucErrIndicFlg = 1;
		//		nSendTransDataOutput_MC(CLSS_DATA_OCPS | CLSS_DATA_DISD);//S1.8
		break;
	}
}

static void AppInitPaymentData_MC(void)//KS.2 paypass 3.0.1 by zhoujie
{
	memset(sg_aucOutcomeParamSet, 0, sizeof(sg_aucOutcomeParamSet));
	sg_aucOutcomeParamSet[0] = OC_END_APPLICATION;//for s51.13
	sg_aucOutcomeParamSet[1] = OC_NA;//s51.3
	sg_aucOutcomeParamSet[2] = OC_NA;
	sg_aucOutcomeParamSet[3] = OC_NA;
	sg_aucOutcomeParamSet[4] |= 0x10;
	sg_aucOutcomeParamSet[5] = OC_NA;
	sg_aucOutcomeParamSet[6] = 0xFF;

	memset(sg_aucUserInterReqData, 0, sizeof(sg_aucUserInterReqData));
	sg_aucUserInterReqData[0] = MI_NA;
	sg_aucUserInterReqData[1] = MI_NA;
	memcpy(sg_aucUserInterReqData+2, MSG_HOLD_TIME_VALUE, 3);

	memset(sg_aucErrIndication, 0, sizeof(sg_aucErrIndication));
	sg_aucErrIndication[5] = MI_NA;

	AppInitDiscData_MC();
}

static void AppSetAppType(unsigned char ucType)
{
	uchar ucTemp[32]={0};

	memset(ucTemp, 0, sizeof(ucTemp));
	sg_ucAppType = ucType;

	switch (ucType) {
	case KERNTYPE_DEF:
		ucTemp[0] = CLSS_KERNEL_DEF;
		break;
	case KERNTYPE_JCB:
		ucTemp[0] = CLSS_KERNEL_JCB;
		break;
	case KERNTYPE_MC:
		ucTemp[0] = CLSS_KERNEL_MC;
		break;
	case KERNTYPE_VIS:
		ucTemp[0] = CLSS_KERNEL_VISA;
		break;
	case KERNTYPE_PBOC:
		ucTemp[0] = CLSS_KERNEL_PBOC;
		break;
	case KERNTYPE_AE:
		ucTemp[0] = CLSS_KERNEL_AE;
		break;
	case KERNTYPE_ZIP:
		ucTemp[0] = CLSS_KERNEL_ZIP;
		break;
	case KERNTYPE_FLASH:
		ucTemp[0] = CLSS_KERNEL_FLASH;
		break;
	case KERNTYPE_EFT:
		ucTemp[0] = CLSS_KERNEL_EFT;
		break;
	case KERNTYPE_RFU:
		ucTemp[0] = CLSS_KERNEL_RFU;
		break;
	default:
		ucTemp[0] = CLSS_KERNEL_DEF;
		break;
	}

	SetConfigurationData(CMD_TXN_CUR_CLSS_TYPE, ucTemp, 1);
}

static unsigned char AppGetAppType(void)
{
	return sg_ucAppType;
}

static int AppGetTornLog_MC(CLSS_TORN_LOG_RECORD *ptTornLog, int *pnTornNum)
{
	if (ptTornLog == NULL || pnTornNum== NULL)
	{
		return EMV_PARAM_ERR;
	}
	memcpy(ptTornLog, sg_tAppTornTransLog, sg_iAppTornLogNum * sizeof(CLSS_TORN_LOG_RECORD));
	*pnTornNum = sg_iAppTornLogNum;
	return 0;
}

static int AppSaveTornLog_MC(CLSS_TORN_LOG_RECORD *ptTornLog, int nTornNum)//save Torn Log in file
{
	int iFd=0, iRet=0;

	if (ptTornLog == NULL)
	{
		return EMV_PARAM_ERR;
	}

	memcpy(sg_tAppTornTransLog, ptTornLog, nTornNum*sizeof(CLSS_TORN_LOG_RECORD));
	sg_iAppTornLogNum = nTornNum;

	iFd = PdkFile_Open(PAYPASS_TORN_LOG_FILE);
	if ( iFd < 0 )
	{
		return EMV_FILE_ERR;
	}
	iRet = PdkFile_Write(iFd, (uchar *)&nTornNum, 1);
	if ( iRet != 1 )
	{
		PdkFile_Close(iRet);
		return EMV_FILE_ERR;
	}
	iRet = PdkFile_Write(iFd, (uchar *)ptTornLog, nTornNum*sizeof(CLSS_TORN_LOG_RECORD));
	if ( iRet != nTornNum*sizeof(CLSS_TORN_LOG_RECORD) )
	{
		PdkFile_Close(iRet);
		return EMV_FILE_ERR;
	}

	PdkFile_Close(iRet);
	return EMV_OK;
}

static int AppLoadTornLog_MC(void)//read Torn Log from file
{
	int iFd=0, iRet=0;

	iFd = PdkFile_Open(PAYPASS_TORN_LOG_FILE);
	if ( iFd < 0 )
	{
		return EMV_FILE_ERR;
	}
	memset(sg_tAppTornTransLog, 0, sizeof(sg_tAppTornTransLog));
	sg_iAppTornLogNum = 0;

	iRet = PdkFile_Read(iFd, (uchar *)&sg_iAppTornLogNum, 1);
	if ( iRet != 1 )
	{
		PdkFile_Close(iRet);
		return EMV_FILE_ERR;
	}
	if ( sg_iAppTornLogNum == 0 )
	{
		PdkFile_Close(iRet);
		return NO_TRANS_LOG;
	}
	iRet = PdkFile_Read(iFd, (uchar *)sg_tAppTornTransLog, sg_iAppTornLogNum*sizeof(CLSS_TORN_LOG_RECORD));
	if ( iRet < 0 )
	{
		PdkFile_Close(iRet);
		return EMV_FILE_ERR;
	}

	PdkFile_Close(iRet);
	return EMV_OK;	
}

static int AppCleanTornLog_MC(void)
{
	unsigned char aucBuff[8]={0};
	int iUpdatedFlg=0, iRet=0;

	GetTime(aucBuff);//  [11/18/2011 zhoujie]
	if ( sg_iAppTornLogNum == 0 ) {
		return 0;
	}
	Clss_SetTornLog_MC_MChip(sg_tAppTornTransLog, sg_iAppTornLogNum);
	iRet = Clss_CleanTornLog_MC_MChip(aucBuff, 6, 0);
	if ( iRet ) {
		return 0;
	}

	sg_iAppTornLogNum = 0;
	iUpdatedFlg = 0;
	memset(sg_tAppTornTransLog, 0, sizeof(sg_tAppTornTransLog));
	Clss_GetTornLog_MC_MChip(sg_tAppTornTransLog, &sg_iAppTornLogNum, &iUpdatedFlg);

	return 0;
}


// add for paypass 3.0 [12/30/2014 jiangjy]
static void SetFinalSelectAID(uchar *paucAID, uchar ucAIDLen)
{
	if ( ucAIDLen && paucAID != NULL ) {
		memcpy(sg_aucFinalAID, paucAID, ucAIDLen);
	}
	sg_ucFinalAIDLen = ucAIDLen;
	EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "sg_aucFinalAID: ", sg_aucFinalAID, sg_ucFinalAIDLen);
}

static void AppSetTransPath(uchar ucTransPath)
{
	uchar ucTemp[32]={0};

	memset(ucTemp, 0, sizeof(ucTemp));
	sg_ucTransPath = ucTransPath;

	switch (ucTransPath) {
	case CLSS_PATH_NORMAL:
		ucTemp[0] = TRANS_PATH_TYPE_CLSS_PATH_NORMAL;
		break;
	case CLSS_VISA_MSD:
		ucTemp[0] = TRANS_PATH_TYPE_CLSS_VISA_MSD;
		break;
	case CLSS_VISA_QVSDC:
		ucTemp[0] = TRANS_PATH_TYPE_CLSS_VISA_QVSDC;
		break;
	case CLSS_VISA_VSDC:
		ucTemp[0] = TRANS_PATH_TYPE_CLSS_VISA_VSDC;
		break;
	case CLSS_VISA_CONTACT:
		ucTemp[0] = TRANS_PATH_TYPE_CLSS_VISA_CONTACT;
		break;
	case CLSS_MC_MAG:
		ucTemp[0] = TRANS_PATH_TYPE_CLSS_MC_MAG;
		break;
	case CLSS_MC_MCHIP:
		ucTemp[0] = TRANS_PATH_TYPE_CLSS_MC_MCHIP;
		break;
	case CLSS_VISA_WAVE2:
		ucTemp[0] = TRANS_PATH_TYPE_CLSS_VISA_WAVE2;
		break;
	case CLSS_JCB_WAVE2:
		ucTemp[0] = TRANS_PATH_TYPE_CLSS_JCB_WAVE2;
		break;
	case CLSS_VISA_MSD_CVN17:
		ucTemp[0] = TRANS_PATH_TYPE_CLSS_VISA_MSD_CVN17;
		break;
	case CLSS_VISA_MSD_LEGACY:
		ucTemp[0] = TRANS_PATH_TYPE_CLSS_VISA_MSD_LEGACY;
		break;
	default:
		ucTemp[0] = TRANS_PATH_TYPE_CLSS_PATH_NORMAL;
		break;
	}
	SetConfigurationData(CMD_TXN_PATH_TYPE, ucTemp, 1);
}

static unsigned char AppGetTransPath(void)
{
	return sg_ucTransPath;
}

static void TwoOne_app(unsigned char *pucIn, unsigned short usInlen, unsigned char *pucOut)
{
	int i=0;
	unsigned char ucTmp=0;

	for ( i = 0; i < usInlen; i += 2 ) {
		ucTmp = pucIn[i];
		if ( ucTmp > '9' ) {
			ucTmp = toupper(ucTmp) - ('A' - 0x0A);
		}
		else {
			ucTmp &= 0x0f;
		}
		ucTmp <<= 4;
		pucOut[i / 2] = ucTmp;

		ucTmp = pucIn[i + 1];
		if ( ucTmp > '9' ) {
			ucTmp = toupper(ucTmp) - ('A' - 0x0A);
		}
		else {
			ucTmp &= 0x0f;
		}
		pucOut[i / 2] += ucTmp;
	}
}

static int SetDETData(unsigned char *pucTag, unsigned char ucTagLen, unsigned char *pucData, unsigned char ucDataLen)
{
	int iRet=0;
	uchar aucBuff[256]={0},ucBuffLen=0;

	if ( (pucTag == NULL) || (pucData == NULL) ) {
		return CLSS_PARAM_ERR;
	}
	memset(aucBuff, 0, sizeof(aucBuff));
	memcpy(aucBuff, pucTag, ucTagLen);
	ucBuffLen = ucTagLen;
	aucBuff[ucBuffLen++] = ucDataLen;
	memcpy(aucBuff+ucBuffLen, pucData, ucDataLen);
	ucBuffLen += ucDataLen;
	if ( AppGetAppType() == KERNTYPE_MC ) 
	{
		iRet = Clss_SetTLVDataList_MC(aucBuff, ucBuffLen);
	}
	return iRet;
}

static void InitPaymentData(void)//KS.2 paypass 3.0.1 by zhoujie
{
	Clss_SetTLVDataList_MC("\x9F\x7E\x01\x01", 4);//Mobile Support Indicator

	memset(sg_aucOutcomeParamSet, 0, sizeof(sg_aucOutcomeParamSet));
	sg_aucOutcomeParamSet[0] = OC_END_APPLICATION;//for s51.13
	sg_aucOutcomeParamSet[1] = OC_NA;//s51.3
	sg_aucOutcomeParamSet[2] = OC_NA;
	sg_aucOutcomeParamSet[3] = OC_NA;
	sg_aucOutcomeParamSet[4] |= 0x10;
	sg_aucOutcomeParamSet[5] = OC_NA;
	sg_aucOutcomeParamSet[6] = 0xFF;

	memset(sg_aucUserInterReqData, 0, sizeof(sg_aucUserInterReqData));
	sg_aucUserInterReqData[0] = MI_NA;
	sg_aucUserInterReqData[1] = MI_NA;
	memcpy(sg_aucUserInterReqData+2, MSG_HOLD_TIME_VALUE, 3);

	memset(sg_aucErrIndication, 0, sizeof(sg_aucErrIndication));
	sg_aucErrIndication[5] = MI_NA;

	InitDiscData();
	//	gl_ucTornLogRecordExist = 0;
}

static void InitDiscData(void)//Initialize Discretionary Data by zhoujie
{
	memset(&sg_tDiscretionayData,0,sizeof(sg_tDiscretionayData));
}

static int AppFindMatchProID(unsigned char *pucProID, int iProIDLen)
{
	int i=0, j=0, iRet=0, iFd=0, iNum=0;
	uchar ucPartFindFlg=0;
	uchar ucLastProIDLen=0;
	uchar ucLastMatch=0;
	PAYWAVE_PROGREAM_ID_NODE_T stPayWavePrgIdNode = {0};

	if ( pucProID == NULL)
	{
		return EMV_PARAM_ERR;
	}

	iNum = ClssGetNodeNum(CLSS_PAYWAVE_PROGRAM_ID_NODE);
	if ( iNum <= 0 ) {
		return iNum;
	}
	for(i = 0; i < iNum; i++)
	{
		memset(&stPayWavePrgIdNode, 0, sizeof(stPayWavePrgIdNode));
		iRet = GetPayWaveProgramIdData(&stPayWavePrgIdNode, sizeof(stPayWavePrgIdNode), i);
		if ( iRet < 0 ) {
			return iRet;
		}

		if (iProIDLen == stPayWavePrgIdNode.ucPrgramIdLen)//Full Match
		{
			if (memcmp(pucProID, stPayWavePrgIdNode.aucProgramId, iProIDLen) == 0)
			{
				memset(&sg_tProgInfo, 0, sizeof(Clss_ProgramID));
				sg_tProgInfo.ulTermFLmt = stPayWavePrgIdNode.tPreProcDRL.ulTermFLmt;
				sg_tProgInfo.ulRdClssTxnLmt = stPayWavePrgIdNode.tPreProcDRL.ulRdClssTxnLmt;
				sg_tProgInfo.ulRdCVMLmt = stPayWavePrgIdNode.tPreProcDRL.ulRdCVMLmt;
				sg_tProgInfo.ulRdClssFLmt =  stPayWavePrgIdNode.tPreProcDRL.ulRdClssFLmt;	
				memcpy(sg_tProgInfo.aucProgramId, stPayWavePrgIdNode.aucProgramId, sizeof(stPayWavePrgIdNode.aucProgramId));
				sg_tProgInfo.ucPrgramIdLen = stPayWavePrgIdNode.ucPrgramIdLen;
				sg_tProgInfo.ucAmtZeroNoAllowed = stPayWavePrgIdNode.tPreProcDRL.ucZeroAmtNoAllowed;
				sg_tProgInfo.ucStatusCheckFlg = stPayWavePrgIdNode.tPreProcDRL.ucStatusCheckFlg;
				sg_tProgInfo.ucTermFLmtFlg = stPayWavePrgIdNode.tPreProcDRL.ucTermFLmtFlg;
				sg_tProgInfo.ucRdClssTxnLmtFlg = stPayWavePrgIdNode.tPreProcDRL.ucRdClssTxnLmtFlg;
				sg_tProgInfo.ucRdCVMLmtFlg = stPayWavePrgIdNode.tPreProcDRL.ucRdCVMLmtFlg;
				sg_tProgInfo.ucRdClssFLmtFlg = stPayWavePrgIdNode.tPreProcDRL.ucRdClssFLmtFlg;
				//				memcpy(&sg_tClssPreProcInfo, &stPayWavePrgIdNode.tPreProcDRL, sizeof(Clss_PreProcInfo));
				return EMV_OK;
			}
		}
		else if (iProIDLen > stPayWavePrgIdNode.ucPrgramIdLen && stPayWavePrgIdNode.ucPrgramIdLen > 0)//Partial Match
		{
			if ((memcmp(pucProID, stPayWavePrgIdNode.aucProgramId, stPayWavePrgIdNode.ucPrgramIdLen)) == 0)
			{
				if (ucPartFindFlg == 0)//don't find Application Program ID
				{
					ucLastMatch = i;//the first fit Application Program ID
					ucPartFindFlg = 1;
					ucLastProIDLen = stPayWavePrgIdNode.ucPrgramIdLen;
				}
				else //之前有部分匹配的Application Program ID
				{
					if (stPayWavePrgIdNode.ucPrgramIdLen > ucLastProIDLen)// record the maximum length
					{
						ucLastMatch = i;
						ucLastProIDLen = stPayWavePrgIdNode.ucPrgramIdLen;
					}
				}

			}
		}

	}

	if (ucPartFindFlg == 1)
	{
		//		memcpy(&sg_tClssPreProcInfo, &stPayWavePrgIdNode.tPreProcDRL, sizeof(Clss_PreProcInfo));
		memset(&sg_tProgInfo, 0, sizeof(Clss_ProgramID));
		sg_tProgInfo.ulTermFLmt = stPayWavePrgIdNode.tPreProcDRL.ulTermFLmt;
		sg_tProgInfo.ulRdClssTxnLmt = stPayWavePrgIdNode.tPreProcDRL.ulRdClssTxnLmt;
		sg_tProgInfo.ulRdCVMLmt = stPayWavePrgIdNode.tPreProcDRL.ulRdCVMLmt;
		sg_tProgInfo.ulRdClssFLmt =  stPayWavePrgIdNode.tPreProcDRL.ulRdClssFLmt;	
		memcpy(sg_tProgInfo.aucProgramId, stPayWavePrgIdNode.aucProgramId, sizeof(stPayWavePrgIdNode.aucProgramId));
		sg_tProgInfo.ucPrgramIdLen = stPayWavePrgIdNode.ucPrgramIdLen;
		sg_tProgInfo.ucAmtZeroNoAllowed = stPayWavePrgIdNode.tPreProcDRL.ucZeroAmtNoAllowed;
		sg_tProgInfo.ucStatusCheckFlg = stPayWavePrgIdNode.tPreProcDRL.ucStatusCheckFlg;
		sg_tProgInfo.ucTermFLmtFlg = stPayWavePrgIdNode.tPreProcDRL.ucTermFLmtFlg;
		sg_tProgInfo.ucRdClssTxnLmtFlg = stPayWavePrgIdNode.tPreProcDRL.ucRdClssTxnLmtFlg;
		sg_tProgInfo.ucRdCVMLmtFlg = stPayWavePrgIdNode.tPreProcDRL.ucRdCVMLmtFlg;
		sg_tProgInfo.ucRdClssFLmtFlg = stPayWavePrgIdNode.tPreProcDRL.ucRdClssFLmtFlg;

		return EMV_OK;
	}


	/*
	if ( pucProID == NULL ) {
	return EMV_PARAM_ERR;
	} else {
	iFd = PdkFile_Open(EMV_APP_LIST_FILE);
	if ( iFd < 0 ) {
	return iFd;
	}

	for ( i=0; i < g_stEmvParamInfo.usAidNum; i++ ) {
	memset(&tEMVAPP, 0, sizeof(EMV_APPLIST));
	iRet = PdkFile_Read(iFd, (unsigned char *)&tEMVAPP, sizeof(EMV_APPLIST));
	#ifdef CLSS_DEBUG_
	AppLog("[AppFindMatchProID] EMVGetApp = %d \n", iRet);
	#endif
	if ( iRet != sizeof(EMV_APPLIST) ) {
	break;
	}		
	iRet = Clss_AddAidList_Entry(tEMVAPP.AID, tEMVAPP.AidLen, tEMVAPP.SelFlag, KERNTYPE_DEF);
	#ifdef CLSS_DEBUG_
	AppLog("[AppFindMatchProID] Clss_AddAidList_Entry = %d \n", iRet);
	#endif
	if ( iRet != EMV_OK ) {
	continue;
	}
	memset(&sg_tProgInfo, 0, sizeof(Clss_ProgramID));
	sg_tProgInfo.ulTermFLmt = sg_tClssPreProcInfo.ulTermFLmt;
	sg_tProgInfo.ulRdClssTxnLmt = sg_tClssPreProcInfo.ulRdClssTxnLmt;
	sg_tProgInfo.ulRdCVMLmt = sg_tClssPreProcInfo.ulRdCVMLmt;
	sg_tProgInfo.ulRdClssFLmt =  sg_tClssPreProcInfo.ulRdClssFLmt;	
	memcpy(sg_tProgInfo.aucProgramId, pucProID, 17);
	sg_tProgInfo.ucPrgramIdLen = iProIDLen;
	sg_tProgInfo.ucAmtZeroNoAllowed = 0;
	sg_tProgInfo.ucStatusCheckFlg = sg_tClssPreProcInfo.ucStatusCheckFlg;
	sg_tProgInfo.ucTermFLmtFlg = sg_tClssPreProcInfo.ucTermFLmtFlg;
	sg_tProgInfo.ucRdClssTxnLmtFlg = sg_tClssPreProcInfo.ucRdClssTxnLmtFlg;
	sg_tProgInfo.ucRdCVMLmtFlg = sg_tClssPreProcInfo.ucRdCVMLmtFlg;
	sg_tProgInfo.ucRdClssFLmtFlg = sg_tClssPreProcInfo.ucRdClssFLmtFlg;
	}
	}

	PdkFile_Close(iFd);
	*/

	return EMV_OK;
}

int ClssSendTransDataOutput_MC(uchar ucMsgFlag)
{
	return AppSaveTransDataOutput_MC(ucMsgFlag);
}

uchar *App_GetFirstTLV(uchar *dat, uchar *datend, uchar *pucTag, uint *punLen)//  [1/9/2013 ZhouJie]
{
	uint i, j, unTempDataLen=0;
	uchar ucTagLen=0;
	uchar *pucDataStart=NULL, *pucDataEnd=NULL, *pucTempTag=NULL;
	if (dat == NULL || datend == NULL || pucTag == NULL || punLen == NULL)
	{
		return NULL;
	}
	pucDataStart = dat;
	pucDataEnd = datend;
	while (pucDataStart < pucDataEnd)
	{
		ucTagLen=0;
		pucTempTag = pucDataStart++;
		ucTagLen++;
		if (pucTempTag[0] == 0x00)
		{
			continue;
		}
		if ((pucTempTag[0] & 0x1F) == 0x1F)
		{
			ucTagLen++;
			while (pucDataStart < pucDataEnd && (*pucDataStart & 0x80))
			{
				pucDataStart++;
				ucTagLen++;
			}
			pucDataStart++;
		}
		if (pucDataStart >= pucDataEnd) // no length // G06-9260[11/24/2011 zhoujie]
		{
			return NULL;
		}
		if (*pucDataStart & 0x80)
		{
			i = (*pucDataStart & 0x7F);
			if (pucDataStart + i > pucDataEnd)
			{
				return NULL;
			}
			pucDataStart++;
			for (j = 0; j < i; j++)
			{
				unTempDataLen <<= 8;
				unTempDataLen += *pucDataStart++;
			}
		}
		else
		{
			unTempDataLen = *pucDataStart++;
		}
		memcpy(pucTag, pucTempTag, ucTagLen);
		*punLen = unTempDataLen;
		return pucDataStart;
	}
	return NULL;
}

uchar AppGetTagLen(uchar *pucTag)
{
	uchar ucTagLen=0;
	int i;
	uchar szTmpTag[4];

	if (pucTag == NULL)
	{
		return 0;
	}
	memset (szTmpTag, 0, sizeof(szTmpTag));
	for (i=0; i<4; i++)
	{
		if (pucTag[i] != 0x00)
		{
			memcpy(szTmpTag, &pucTag[i], 4-i);
			break;
		}
	}
	ucTagLen = 4-i;

	return ucTagLen;
}

unsigned char *App_SearchTLV(int DolList, unsigned char *pucTag, unsigned char *dat, unsigned char *datend, int *nLen)
{
	uint unTempLen;
	uchar aucTempTag[4];
	uchar *pucValue=NULL;
	uchar *pucDataStart, *pucDataEnd;
	if (dat == NULL || datend == NULL || pucTag == NULL)
	{
		return NULL;
	}
	memset(aucTempTag, 0, sizeof(aucTempTag));//for three byte of tag
	pucDataStart = dat;
	pucDataEnd = datend;
	while (pucDataStart < pucDataEnd) 
	{
		if (*pucDataStart == 0x00)
		{
			pucDataStart++;
			continue;
		}
		pucValue = App_GetFirstTLV(pucDataStart, pucDataEnd, aucTempTag, &unTempLen);
		if (pucValue == NULL)//can't find a TLV
		{
			return NULL;
		}
		if (!memcmp(aucTempTag, pucTag, AppGetTagLen(pucTag))) //successful
		{
			if (nLen != NULL) 
			{
				*nLen = unTempLen;
			}
			return pucValue;
		}
		pucDataStart = pucValue;
		if (aucTempTag[0] & 0x20) 
		{
			continue;
		}
		if (DolList) 
		{
			continue;
		}
		pucDataStart += unTempLen;//continue search next TLV
	}
	return NULL;
}

static int AppSaveTransDataOutput_MC(uchar ucMsgFlag)
{
	uint unLen=0, unBuffLen=0;
	uchar aucBuff[1024]={0};

	if(ucMsgFlag & CLSS_DATA_OCPS)
	{
		Clss_GetTLVDataList_MC("\xDF\x81\x29", 3, sizeof(sg_aucOutcomeParamSet), sg_aucOutcomeParamSet, &unLen);
	}
	if(ucMsgFlag & CLSS_DATA_UIRD)
	{
		Clss_GetTLVDataList_MC("\xDF\x81\x16", 3, sizeof(sg_aucUserInterReqData), sg_aucUserInterReqData, &unLen);
	}
	if(ucMsgFlag & CLSS_DATA_DISD)
	{
		Clss_GetTLVDataList_MC("\xDF\x81\x15", 3, sizeof(sg_aucErrIndication), sg_aucErrIndication, &unLen);
		memset(aucBuff, 0, sizeof(aucBuff));
		Clss_GetTLVDataList_MC("\xFF\x81\x06", 3, sizeof(aucBuff), aucBuff, &unBuffLen);
		if (App_SearchTLV(0, (uchar *)"\x9F\x42", aucBuff, aucBuff+unBuffLen, &unLen))
		{
			if (unLen)
			{
				sg_tDiscretionayData.ucAppCurrCodeFlg = APP_TAG_EXIST_WITHVAL;
			}
			else
			{
				sg_tDiscretionayData.ucAppCurrCodeFlg = APP_TAG_EXIST_NOVAL;
			}
		}
		else
		{
			sg_tDiscretionayData.ucAppCurrCodeFlg = APP_TAG_NOT_EXIST;
		}
		if (App_SearchTLV(0, (uchar *)"\xDF\x81\x04", aucBuff, aucBuff+unBuffLen, &unLen))
		{
			if (unLen)
			{
				sg_tDiscretionayData.ucBalBeforeGACFlg = APP_TAG_EXIST_WITHVAL;
			}
			else
			{
				sg_tDiscretionayData.ucBalBeforeGACFlg = APP_TAG_EXIST_NOVAL;
			}
		}
		else
		{
			sg_tDiscretionayData.ucBalBeforeGACFlg = APP_TAG_NOT_EXIST;
		}
		if (App_SearchTLV(0, (uchar *)"\xDF\x81\x05", aucBuff, aucBuff+unBuffLen, &unLen))
		{
			if (unLen)
			{
				sg_tDiscretionayData.ucBalAfterGACFlg = APP_TAG_EXIST_WITHVAL;
			}
			else
			{
				sg_tDiscretionayData.ucBalAfterGACFlg = APP_TAG_EXIST_NOVAL;
			}
		}
		else
		{
			sg_tDiscretionayData.ucBalAfterGACFlg = APP_TAG_NOT_EXIST;
		}
		if (App_SearchTLV(0, (uchar *)"\xDF\x81\x02", aucBuff, aucBuff+unBuffLen, &unLen))
		{
			if (unLen)
			{
				sg_tDiscretionayData.ucDSSum3Flg = APP_TAG_EXIST_WITHVAL;
			}
			else
			{
				sg_tDiscretionayData.ucDSSum3Flg = APP_TAG_EXIST_NOVAL;
			}
		}
		else
		{
			sg_tDiscretionayData.ucDSSum3Flg = APP_TAG_NOT_EXIST;
		}
		if (App_SearchTLV(0, (uchar *)"\xDF\x81\x0B", aucBuff, aucBuff+unBuffLen, &unLen))
		{
			if (unLen)
			{
				sg_tDiscretionayData.ucDSSumStFlg = APP_TAG_EXIST_WITHVAL;
			}
			else
			{
				sg_tDiscretionayData.ucDSSumStFlg = APP_TAG_EXIST_NOVAL;
			}
		}
		else
		{
			sg_tDiscretionayData.ucDSSumStFlg = APP_TAG_NOT_EXIST;
		}
		if (App_SearchTLV(0, (uchar *)"\xDF\x81\x15", aucBuff, aucBuff+unBuffLen, &unLen))
		{
			if (unLen)
			{
				Clss_GetTLVDataList_MC((uchar*)"\xDF\x81\x15", 3, sizeof(sg_aucErrIndication), sg_aucErrIndication, &unLen);
				sg_tDiscretionayData.ucErrIndicFlg = APP_TAG_EXIST_WITHVAL;
			}
			else
			{
				sg_tDiscretionayData.ucErrIndicFlg = APP_TAG_EXIST_NOVAL;
			}
		}
		else
		{
			sg_tDiscretionayData.ucErrIndicFlg = APP_TAG_NOT_EXIST;
		}
		if (App_SearchTLV(0, (uchar *)"\xDF\x81\x0E", aucBuff, aucBuff+unBuffLen, &unLen))
		{
			if (unLen)
			{
				sg_tDiscretionayData.ucPostGACPutDataStFlg = APP_TAG_EXIST_WITHVAL;
			}
			else
			{
				sg_tDiscretionayData.ucPostGACPutDataStFlg = APP_TAG_EXIST_NOVAL;
			}
		}
		else
		{
			sg_tDiscretionayData.ucPostGACPutDataStFlg = APP_TAG_NOT_EXIST;
		}
		if (App_SearchTLV(0, (uchar *)"\xDF\x81\x0F", aucBuff, aucBuff+unBuffLen, &unLen))
		{
			if (unLen)
			{
				sg_tDiscretionayData.ucPreGACPutDataStFlg = APP_TAG_EXIST_WITHVAL;
			}
			else
			{
				sg_tDiscretionayData.ucPreGACPutDataStFlg = APP_TAG_EXIST_NOVAL;
			}
		}
		else
		{
			sg_tDiscretionayData.ucPreGACPutDataStFlg = APP_TAG_NOT_EXIST;
		}
		if (App_SearchTLV(0, (uchar *)"\x9F\x6E", aucBuff, aucBuff+unBuffLen, &unLen))
		{
			if (unLen)
			{
				sg_tDiscretionayData.ucThirdPartyDataFlg = APP_TAG_EXIST_WITHVAL;
			}
			else
			{
				sg_tDiscretionayData.ucThirdPartyDataFlg = APP_TAG_EXIST_NOVAL;
			}
		}
		else
		{
			sg_tDiscretionayData.ucThirdPartyDataFlg = APP_TAG_NOT_EXIST;
		}
		if (App_SearchTLV(0, (uchar *)"\xFF\x81\x01", aucBuff, aucBuff+unBuffLen, &unLen))
		{
			if (unLen)
			{
				sg_tDiscretionayData.ucTornRecordFlg = APP_TAG_EXIST_WITHVAL;
			}
			else
			{
				sg_tDiscretionayData.ucTornRecordFlg = APP_TAG_EXIST_NOVAL;
			}
		}
		else
		{
			sg_tDiscretionayData.ucTornRecordFlg = APP_TAG_NOT_EXIST;
		}
		if (App_SearchTLV(0, (uchar *)"\xDF\x81\x2A", aucBuff, aucBuff+unBuffLen, &unLen))
		{
			if (unLen)
			{
				sg_tDiscretionayData.ucDDCardTrack1Flg = APP_TAG_EXIST_WITHVAL;
			}
			else
			{
				sg_tDiscretionayData.ucDDCardTrack1Flg = APP_TAG_EXIST_NOVAL;
			}
		}
		else
		{
			sg_tDiscretionayData.ucDDCardTrack1Flg = APP_TAG_NOT_EXIST;
		}
		if (App_SearchTLV(0, (uchar *)"\xDF\x81\x2B", aucBuff, aucBuff+unBuffLen, &unLen))
		{
			if (unLen)
			{
				sg_tDiscretionayData.ucDDCardTrack2Flg = APP_TAG_EXIST_WITHVAL;
			}
			else
			{
				sg_tDiscretionayData.ucDDCardTrack2Flg = APP_TAG_EXIST_NOVAL;
			}
		}
		else
		{
			sg_tDiscretionayData.ucDDCardTrack2Flg = APP_TAG_NOT_EXIST;
		}
	}
	return EMV_OK;
}

static int AppSetClss_capk(uchar ucKeyIndex, uchar *pucRid, uchar ucKernelId)
{
	int i = 0, iRet = -1;
	EMV_CAPK pstEmvCapk;

	iRet = EMVLoadSpecificCapk(pucRid, ucKeyIndex, &pstEmvCapk);
	EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "EMVLoadSpecificCapk = %d, ucKeyIndex=%02x\n", iRet, ucKeyIndex);
	EL_LOG_HEX(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "RID: ", pucRid, 5);
	if ( iRet == 0 ) 
	{
		if ( ucKernelId == KERNTYPE_VIS ) 
		{
			iRet = Clss_AddCAPK_Wave(&pstEmvCapk);
			EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "Clss_AddCAPK_Wave = %d \n", iRet);
		}
		else if ( ucKernelId == KERNTYPE_MC ) 
		{
			iRet = Clss_AddCAPK_MC_MChip(&pstEmvCapk);
			EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "Clss_AddCAPK_MC_MChip = %d \n", iRet);
		}
		else 
		{
			return -1;
		}
	}

	return iRet;
}


static int AppSetClss_RevLst(uchar ucKeyIndex, uchar *pucRid, uchar ucKernelId)
{
	int i = 0, iRet = 0;
	EMV_REVOCLIST tRevocList = {0};

	iRet = EMVLoadSpecificRevoke(pucRid, ucKeyIndex, &tRevocList);
	EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "EMVLoadSpecificRevoke = %d \n", iRet);
	if ( iRet == 0 ) 
	{
		if ( ucKernelId == KERNTYPE_VIS ) 
		{
			iRet = Clss_AddRevocList_Wave(&tRevocList);
			EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "Clss_AddRevocList_Wave = %d \n", iRet);
		}
		else if ( ucKernelId == KERNTYPE_MC ) 
		{
			iRet = Clss_AddRevocList_MC_MChip(&tRevocList);
			EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "Clss_AddRevocList_MC_MChip = %d \n", iRet);
		}
		else 
		{
			return EL_TRANS_RET_EMV_NOT_FOUND;
		}
	}



	/*
	memcpy(tRevocList.ucRid, rid, 5);
	tRevocList.ucIndex = index;
	memcpy(tRevocList.ucCertSn, "\x00\x07\x11", 3);

	if (kerId == KERNTYPE_VIS)
	{
	Clss_AddRevocList_Wave(&tRevocList);
	}
	//	else		//modified by kevinliu 2015/10/19
	else if (kerId == KERNTYPE_MC)
	{
	Clss_AddRevocList_MC_MChip(&tRevocList);
	}
	else
	{
	return TRANS_EMV_NOT_FOUND;
	}
	*/

	return iRet;
}

static void AppSetTransACType(uchar ucType)
{
	sg_ucACType = ucType;
}

static uchar AppGetTransACType(void)
{
	return sg_ucACType;
}

static void Inter_DisplayMsg(int iMsg)
{
	switch ( iMsg ) {
	case MSG_CLEAR_DISPLAY:
		break;
	case MSG_CARD_READ_OK:
		Beep();
		break;
	case MSG_REMOVE_CARD:
#ifdef EUI_
		EShowMsgBox(EL_UI_PAGE_TRANS_REMOVE_CARD, NULL, 0);
#else
		ScrCls();
		PubDispString("PLS REMOVE CARD", 0|DISP_LINE_CENTER);
#endif
		Beep();
		break;
	case ICC_BLOCK:
		Beep();
		break;
	case MSG_TRY_AGAIN:
#ifdef EUI_
		EShowMsgBox(EL_UI_PAGE_TRANS_TAP_AGAIN, NULL, 0);
#else
		ScrCls();
		PubDispString("PLS TAP AGAIN", 0|DISP_LINE_CENTER);
#endif
		Beep();
		break;
	case MSG_SEE_PHONE:
#ifdef EUI_
		EShowMsgBox(EL_UI_PAGE_TRANS_SEE_PHONE, NULL, 0);
#else
		ScrCls();
		PubDispString("PLS SEE PHONE", 0|DISP_LINE_CENTER);
#endif
		Beep();
		break;
	case MSG_APPROVED:
#ifdef EUI_
		EShowMsgBox(EL_UI_PAGE_TRANS_APPROVED, NULL, 0);
#else
		ScrCls();
		PubDispString("TXN APPROVED", 0|DISP_LINE_CENTER);
#endif
		Beep();
		break;
	case MSG_DECLINED:
#ifdef EUI_
		EShowMsgBox(EL_UI_PAGE_TRANS_DECLINED, NULL, 0);
#else
		ScrCls();
		PubDispString("TXN APPROVED", 0|DISP_LINE_CENTER);
#endif
		Beep();
		break;
	case MSG_APPROVED_SIGN:
		Beep();
		break;
	case MSG_OTHER_CARD:
		Beep();
		break;
	case MSG_INSERT_CARD: 
#ifdef EUI_
		EShowMsgBox(EL_UI_PAGE_TRANS_INSERT_CARD, NULL, 0);
#else
		ScrCls();
		PubDispString("PLS INSERT CARD", 0|DISP_LINE_CENTER);
#endif
		Beep();
		break;
	case MSG_AUTHORISING:
		Beep();
		break;
	case MSG_TAP_CARD_AGAIN:
#ifdef EUI_
		EShowMsgBox(EL_UI_PAGE_TRANS_TAP_AGAIN, NULL, 0);
#else
		ScrCls();
		PubDispString("PLS TAP AGAIN", 0|DISP_LINE_CENTER);
#endif
		Beep();
		break;
	case MSG_TAP_CARD_WITH_AMOUNT:
		Beep();
		break;
	case MSG_TAP_CARD_WITH_BALANCE:
		Beep();
		break;
	case MSG_TRY_ANOTHER_INTERFACE:
#ifdef EUI_
		EShowMsgBox(EL_UI_PAGE_TRANS_TRY_ANOTHER_INTERFACE, NULL, 0);
#else
		ScrCls();
		PubDispString("TRY ANOTHER", 0|DISP_LINE_CENTER);
		PubDispString("INTERFACE", 2|DISP_LINE_CENTER);
#endif
		Beep();
		break;
	case MSG_END_APPLICATION:
		Beep();
		break;
	default:
		break;
	}
	//PubWaitKey(30);//delay
	DelayMs(1000);
}

static void AppRemovePicc(void)
{
	uchar ucRet = 0; 
	int i = 0;

	Beep();
	//  [4/11/2011 yingl]
	ucRet = PiccRemove('r', 0);
	if ((ucRet == 0) || (ucRet == 1)) 
	{  
		PiccClose();
		return;
	}

	i = 0;
	while (ucRet) 
	{
		ucRet = PiccRemove('r', 0); 
		if ((ucRet == 2) || (ucRet == 3))
		{
			break;
		} 
		else
		{
			if (i == 0) {
				//TODO Display remove card here.
#ifdef EUI_
				EShowMsgBox(EL_UI_PAGE_TRANS_REMOVE_CARD, NULL, 0);
#else
				ScrCls();
				PubDispString("PLS REMOVE CARD", 0|DISP_LINE_LEFT);
#endif
				Beep();
				i++;
			}
		}
	}

	PiccClose();
}


static int ClssTransInit(void)
{
	int i=0, iRet=0, iFd=0, iNum=0, j=0;
	EMV_APPLIST tEMVAPP;
	uchar ucTemp[32]={0};
	PAYPASS_AID_NODE_T stPayPassAidNode = {0};
	PAYWAVE_AID_NODE_T stPayWaviAidNode = {0};

	Clss_CoreInit_Entry();
	Clss_DelAllAidList_Entry();
	Clss_DelAllPreProcInfo();
	Clss_SetMCVersion_Entry(0x03);

	//set transaction type contactless.
	memset(ucTemp, 0, sizeof(ucTemp));
	ucTemp[0] = TRANS_TYPE_CONTACTLESS;
	SetConfigurationData(CMD_TXN_CUR_TXN_TYPE, ucTemp, 1);

	//paywave
	iNum = ClssGetNodeNum(CLSS_PAYWAVE_AID_NODE);
	EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "PAYWAVE, ClssGetNodeNum = %d \n", iNum);
	if ( iNum <= 0 ) {
		return iNum;
	}
	for(i = 0; i < iNum; i++)
	{
		memset(&stPayWaviAidNode, 0, sizeof(stPayWaviAidNode));
		iRet = GetPayWaveAidData(&stPayWaviAidNode, sizeof(stPayWaviAidNode), i);
		if ( iRet < 0 ) 
		{
			EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "PAYWAVE, GetPayWaveAidData = %d \n", iRet);
			continue;
		}

		//TODO, add aidlen in PAYWAVE_AID_NODE_T
		iRet = Clss_AddAidList_Entry(stPayWaviAidNode.aucAID,stPayWaviAidNode.ucAidLen, stPayWaviAidNode.ucSelFlag, KERNTYPE_DEF);
		EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "PAYWAVE, Clss_AddAidList_Entry = %d \n", iRet);
		EL_LOG_HEX(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "AID: ", stPayWaviAidNode.aucAID, stPayWaviAidNode.ucAidLen);
		if ( iRet != EMV_OK ) {
			continue;
		}

		memset(&sg_tClssPreProcInfo, 0, sizeof(Clss_PreProcInfo));
		memcpy(sg_tClssPreProcInfo.aucAID, stPayWaviAidNode.aucAID, stPayWaviAidNode.ucAidLen);
		sg_tClssPreProcInfo.ucAidLen = stPayWaviAidNode.ucAidLen;
		sg_tClssPreProcInfo.ucKernType = KERNTYPE_DEF;
		sg_tClssPreProcInfo.ulTermFLmt = stPayWaviAidNode.stFlmtByTransType[0].ulTermFLmt;
		sg_tClssPreProcInfo.ulRdClssTxnLmt = stPayWaviAidNode.stFlmtByTransType[0].ulcRdClssTxnLmt;
		sg_tClssPreProcInfo.ulRdCVMLmt = stPayWaviAidNode.stFlmtByTransType[0].ulRdCVMLmt;
		sg_tClssPreProcInfo.ulRdClssFLmt = stPayWaviAidNode.stFlmtByTransType[0].ulRdClssFLmt;

		sg_tClssPreProcInfo.ucCrypto17Flg = stPayWaviAidNode.ucCrypto17Flg;
		sg_tClssPreProcInfo.ucZeroAmtNoAllowed = stPayWaviAidNode.ucZeroAmtNoAllowed;
		sg_tClssPreProcInfo.ucStatusCheckFlg = stPayWaviAidNode.ucStatusCheckFlg;
		memcpy(sg_tClssPreProcInfo.aucReaderTTQ, stPayWaviAidNode.aucReaderTTQ, sizeof(stPayWaviAidNode.aucReaderTTQ));

		sg_tClssPreProcInfo.ucTermFLmtFlg = stPayWaviAidNode.stFlmtByTransType[0].ucTermFLmtFlg;
		sg_tClssPreProcInfo.ucRdClssTxnLmtFlg = stPayWaviAidNode.stFlmtByTransType[0].ucRdClssTxnLmtFlg;
		sg_tClssPreProcInfo.ucRdCVMLmtFlg = stPayWaviAidNode.stFlmtByTransType[0].ucRdCVMLmtFlg;
		sg_tClssPreProcInfo.ucRdClssFLmtFlg = stPayWaviAidNode.stFlmtByTransType[0].ucRdClssFLmtFlg;

		for (j = 0; j < MAX_TRANSTYPE_NUM; j++) {
			if ( stPayWaviAidNode.stFlmtByTransType[j].ucTransType == g_stClssTransParam.ucTransType) {

				sg_tClssPreProcInfo.ulTermFLmt = stPayWaviAidNode.stFlmtByTransType[j].ulTermFLmt;
				sg_tClssPreProcInfo.ulRdClssTxnLmt = stPayWaviAidNode.stFlmtByTransType[j].ulcRdClssTxnLmt;
				sg_tClssPreProcInfo.ulRdCVMLmt = stPayWaviAidNode.stFlmtByTransType[j].ulRdCVMLmt;
				sg_tClssPreProcInfo.ulRdClssFLmt = stPayWaviAidNode.stFlmtByTransType[j].ulRdClssFLmt;

				sg_tClssPreProcInfo.ucCrypto17Flg = stPayWaviAidNode.ucCrypto17Flg;
				sg_tClssPreProcInfo.ucZeroAmtNoAllowed = stPayWaviAidNode.ucZeroAmtNoAllowed;
				sg_tClssPreProcInfo.ucStatusCheckFlg = stPayWaviAidNode.ucStatusCheckFlg;
				memset(sg_tClssPreProcInfo.aucReaderTTQ, 0, sizeof(sg_tClssPreProcInfo.aucReaderTTQ));
				memcpy(sg_tClssPreProcInfo.aucReaderTTQ, stPayWaviAidNode.aucReaderTTQ, sizeof(stPayWaviAidNode.aucReaderTTQ));

				sg_tClssPreProcInfo.ucTermFLmtFlg = stPayWaviAidNode.stFlmtByTransType[j].ucTermFLmtFlg;
				sg_tClssPreProcInfo.ucRdClssTxnLmtFlg = stPayWaviAidNode.stFlmtByTransType[j].ucRdClssTxnLmtFlg;
				sg_tClssPreProcInfo.ucRdCVMLmtFlg = stPayWaviAidNode.stFlmtByTransType[j].ucRdCVMLmtFlg;
				sg_tClssPreProcInfo.ucRdClssFLmtFlg = stPayWaviAidNode.stFlmtByTransType[j].ucRdClssFLmtFlg;

				break;
			}
		}

		EL_LOG_HEX(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "PAYWAVE, aucAID: ", sg_tClssPreProcInfo.aucAID, sg_tClssPreProcInfo.ucAidLen);
		EL_LOG_HEX(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "aucReaderTTQ: ", sg_tClssPreProcInfo.aucReaderTTQ, 4);
		EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "PAYWAVE, ucAidLen = %d \n", sg_tClssPreProcInfo.ucAidLen);
		EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "PAYWAVE, ucCrypto17Flg = %d \n", sg_tClssPreProcInfo.ucCrypto17Flg);
		EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "PAYWAVE, ucKernType = %02x \n", sg_tClssPreProcInfo.ucKernType);
		EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "PAYWAVE, ucRdClssFLmtFlg = %d \n", sg_tClssPreProcInfo.ucRdClssFLmtFlg);
		EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "PAYWAVE, ucRdClssTxnLmtFlg = %d \n", sg_tClssPreProcInfo.ucRdClssTxnLmtFlg);
		EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "PAYWAVE, ucRdCVMLmtFlg = %d \n", sg_tClssPreProcInfo.ucRdCVMLmtFlg);
		EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "PAYWAVE, ucStatusCheckFlg = %d \n", sg_tClssPreProcInfo.ucStatusCheckFlg);
		EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "PAYWAVE, ucTermFLmtFlg = %d \n", sg_tClssPreProcInfo.ucTermFLmtFlg);            
		EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "PAYWAVE, ucZeroAmtNoAllowed = %d \n", sg_tClssPreProcInfo.ucZeroAmtNoAllowed);  
		EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "PAYWAVE, ulRdClssFLmt = %ld \n", sg_tClssPreProcInfo.ulRdClssFLmt);             
		EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "PAYWAVE, ulRdClssTxnLmt = %ld \n", sg_tClssPreProcInfo.ulRdClssTxnLmt);         
		EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "PAYWAVE, ulRdCVMLmt = %ld \n", sg_tClssPreProcInfo.ulRdCVMLmt);                 
		EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "PAYWAVE, ulTermFLmt = %ld \n", sg_tClssPreProcInfo.ulTermFLmt); 

		Clss_SetPreProcInfo_Entry(&sg_tClssPreProcInfo);
	}

	//paypass
	iNum = ClssGetNodeNum(CLSS_PAYPASS_AID_NODE);
	EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "PAYPASS, ClssGetNodeNum = %d \n", iNum); 
	if ( iNum <= 0 ) 
	{
		return iNum;
	}
	for(i = 0; i < iNum; i++) 
	{
		memset(&stPayPassAidNode, 0, sizeof(stPayPassAidNode));
		iRet = GetPayPassAidData(&stPayPassAidNode, sizeof(stPayPassAidNode), i);
		if ( iRet < 0 ) 
		{
			EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "PAYPASS, GetPayPassAidData = %d \n", iRet);
			continue;;
		}

		//TODO, add aidlen in PAYPASS_AID_NODE_T

		iRet = Clss_AddAidList_Entry(stPayPassAidNode.aucAID, stPayPassAidNode.ucAidLen, stPayPassAidNode.ucSelFlag, KERNTYPE_DEF);
		EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "PAYPASS, Clss_AddAidList_Entry = %d \n", iRet);
		EL_LOG_HEX(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "PAYPASS, AID: ", stPayPassAidNode.aucAID, stPayPassAidNode.ucAidLen);
		if ( iRet != EMV_OK ) {
			continue;
		}
	}

	return EMV_OK;
}


void LoadClssTLVFromFS(void)
{
	int iRet=0, i=0, iTagListLen=0, iTagLen=0, iDataLen=0, iLengthLen=0, iOffset=0, iLengthSubsequentByte=0, iCount=0;
	unsigned short usTag=0;
	unsigned char aucBuf[4096+1], aucData[256+1];

	memset(aucBuf, 0, sizeof(aucBuf));
	iRet = GetAllTLVDataFromMem(aucBuf, &iTagListLen);
	EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "GetAllTLVDataFromFS = %d \n", iRet);
	if(iRet != EMV_OK)
	{
		return;
	}

	while(iOffset < iTagListLen)
	{		
		//calculate tag length, refer to EMV tag Field structure
		if ( 0x1F == (aucBuf[iOffset] & 0x1F) ) 
		{
			if ( 0x80 == (aucBuf[iOffset + 1] & 0x80) ) 
			{
				iTagLen = 3;
			} 
			else 
			{
				iTagLen = 2;
				usTag = aucBuf[iOffset] * 256 + aucBuf[iOffset + 1];
			}
		} 
		else 
		{
			iTagLen = 1;
			usTag = aucBuf[iOffset];
		}

		//check length
		if ( (unsigned int)(iOffset + iTagLen) > iTagListLen ) 
		{
			break;
		}

		//calculate data length, refer to EMV tag Field structure
		iDataLen = 0;
		iLengthSubsequentByte = 0;
		if ( 0x80 == (aucBuf[iOffset + iTagLen] & 0x80) ) 
		{
			iLengthSubsequentByte = aucBuf[iOffset + iTagLen] & 0x7F;
			for ( i = iLengthSubsequentByte - 1; i > 0; i++ ) 
			{
				iDataLen += aucBuf[iOffset + iTagLen + i] + 256 * iCount;
				iCount++;
			}
		} else 
		{
			iDataLen = aucBuf[iOffset + iTagLen];
		}

		//check length
		iLengthLen = 1 + iLengthSubsequentByte;
		if ( (unsigned int)(iOffset + iTagLen + iLengthLen + iDataLen) > iTagListLen) 
		{
			break;
		}

		//get data for EMV and Paywave
		memset(aucData, 0, sizeof(aucData));
		memcpy(aucData, aucBuf + iOffset + iTagLen + iLengthLen, iDataLen);

#ifndef _WIN32

		//todo add set function here.
		if ( g_tTransParam.ucCurrentCLSSType[0] == KERNTYPE_VIS ) 
		{
			iRet = Clss_SetTLVData_Wave(usTag, aucBuf+iOffset+iTagLen+iLengthLen, iDataLen);
		} else if ( g_tTransParam.ucCurrentCLSSType[0] == KERNTYPE_MC ) 
		{
			iRet = Clss_SetTLVDataList_MC(aucBuf + iOffset, iTagLen + iLengthLen + iDataLen);
		} else 
		{
			return;
		}
#endif

		EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "Clss_SetTLVData ret = %d, TAG: %02x\n", iRet, usTag);

		iOffset = iOffset + iTagLen + iLengthLen + iDataLen;
	}
}

static int ClssPreProcTxnParam(void)
{
	int iRet=0;
	unsigned char aucDateTime[24+1];

	//	LoadPreProcParamFromFS();

	//pre-process
	memset(&sg_tClssTransParam, 0, sizeof(Clss_TransParam));
	memcpy(&sg_tClssTransParam, &g_stClssTransParam, sizeof(Clss_TransParam));

	EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "ulAmntAuth = %ld, ulAmntOther = %ld, ucTransType = %02x\n", sg_tClssTransParam.ulAmntAuth, sg_tClssTransParam.ulAmntOther, sg_tClssTransParam.ucTransType);
	EL_LOG_HEX(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "aucTransTime: ", sg_tClssTransParam.aucTransTime, 3);
	EL_LOG_HEX(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "aucTransDate: ", sg_tClssTransParam.aucTransDate, 3);

	iRet = Clss_PreTransProc_Entry(&sg_tClssTransParam);
	//returns EMV_OK	CLSS_PARAM_ERR		CLSS_USE_CONTACT
	if ( iRet != EMV_OK ) {
		return iRet;
	}
	return EMV_OK;

}

static void ClssBaseParameterSet_WAVE(void)
{
	int i = 0, iRet = 0;
	Clss_ReaderParam tClssParam = {0};
	Clss_VisaAidParam tVisaAidParam = {0};
	PAYWAVE_TERM_CAPABILITY_NODE_T stPayWaveTermCapNode = {0};
	CLSS_OTHER_CONF_NODE_T stClssOtherConfNode = {0};
	unsigned char ucTemp[128] = {0};

	memset(&stPayWaveTermCapNode, 0, sizeof(stPayWaveTermCapNode));
	memset(&stClssOtherConfNode, 0, sizeof(stClssOtherConfNode));
	iRet = GetClssOtherCofData(&stClssOtherConfNode, sizeof(stClssOtherConfNode), i);
	if ( iRet < 0 ) {
		return iRet;
	}
	iRet = GetPayWaveTermCapData(&stPayWaveTermCapNode, sizeof(stPayWaveTermCapNode), i);
	if ( iRet < 0 ) {
		return iRet;
	}

	memset(&tClssParam, 0, sizeof(tClssParam));
	tClssParam.ulReferCurrCon = PubBcd2Long(stClssOtherConfNode.aucRefCurrenceConVerRate, 2, NULL); 
	sprintf(ucTemp, "%s %s", stClssOtherConfNode.aucMerChantName, stClssOtherConfNode.aucMerChantLocalAddr);
	tClssParam.usMchLocLen = strlen(ucTemp);
	memcpy(tClssParam.aucMchNameLoc, ucTemp, tClssParam.usMchLocLen);
	memcpy(tClssParam.aucMerchCatCode, stClssOtherConfNode.aucMerchantCateCode, sizeof(tClssParam.aucMerchCatCode));
	memcpy(tClssParam.aucMerchantID, stClssOtherConfNode.aucMerchantId, sizeof(tClssParam.aucMerchantID));
	memcpy(tClssParam.AcquierId, stClssOtherConfNode.aucTerminalId, sizeof(tClssParam.AcquierId));
	memcpy(tClssParam.aucTmID, g_tTermInfo.ucTermSN, strlen(g_tTermInfo.ucTermSN));

	tClssParam.ucTmType = stPayWaveTermCapNode.ucTermType;

	tClssParam.aucTmCap[0] = stPayWaveTermCapNode.ucCardDataInput;
	tClssParam.aucTmCap[1] = stPayWaveTermCapNode.ucCVMSupport;
	tClssParam.aucTmCap[2] = stPayWaveTermCapNode.ucOffDataAuth;

	tClssParam.aucTmCapAd[0] = stPayWaveTermCapNode.ucTransTypeCapa;
	tClssParam.aucTmCapAd[1] = 0x00;
	tClssParam.aucTmCapAd[2] = stPayWaveTermCapNode.ucTermInput;
	tClssParam.aucTmCapAd[3] = stPayWaveTermCapNode.ucTermOutput;
	tClssParam.aucTmCapAd[4] = 0x01; //todo

	memcpy(tClssParam.aucTmCntrCode, stClssOtherConfNode.aucCountryCode, sizeof(tClssParam.aucTmCntrCode));
	memcpy(tClssParam.aucTmRefCurCode,  stClssOtherConfNode.aucRefCurrencyCode, sizeof(tClssParam.aucTmRefCurCode));
	memcpy(tClssParam.aucTmTransCur, stClssOtherConfNode.aucCurrencyCode, sizeof(tClssParam.aucTmTransCur));
	tClssParam.ucTmTransCurExp = stClssOtherConfNode.ucCurrencyExponent;
	tClssParam.ucTmRefCurExp = stClssOtherConfNode.ucRefCurrencyExponent;

	EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "AcquierId = %s\n", tClssParam.AcquierId);
	EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "aucMchNameLoc = %s \n", tClssParam.aucMchNameLoc);
	EL_LOG_HEX(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "aucMerchCatCode: ", tClssParam.aucMerchCatCode, 2);         
	EL_LOG_HEX(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "aucTmCap: ", tClssParam.aucTmCap, 3);                       
	EL_LOG_HEX(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "aucTmCapAd: ", tClssParam.aucTmCapAd, 5);                   
	EL_LOG_HEX(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "aucTmCntrCode: ", tClssParam.aucTmCntrCode, 2); 
	EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "aucTmID = %s \n", tClssParam.aucTmID);  
	EL_LOG_HEX(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "aucTmRefCurCode: ", tClssParam.aucTmRefCurCode, 2);                               
	EL_LOG_HEX(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "aucTmTransCur: ", tClssParam.aucTmTransCur, 2);      
	EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "ucTmRefCurExp = %02x \n", tClssParam.ucTmRefCurExp);    
	EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "ucTmTransCurExp = %02x \n", tClssParam.ucTmTransCurExp);
	EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "ucTmType = %02x \n", tClssParam.ucTmType);              
	EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "ulReferCurrCon = %d \n", tClssParam.ulReferCurrCon);    
	EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "usMchLocLen = %d \n", tClssParam.usMchLocLen);      

	Clss_SetReaderParam_Wave(&tClssParam);

	memset(&tVisaAidParam,0,sizeof(Clss_VisaAidParam));
	tVisaAidParam.ucCvmReqNum = 2;
	tVisaAidParam.aucCvmReq[0] = RD_CVM_REQ_SIG;
	tVisaAidParam.aucCvmReq[1] = RD_CVM_REQ_ONLINE_PIN;

	tVisaAidParam.ucDomesticOnly = 0x00; // 01(default):only supports domestic cl transaction
	tVisaAidParam.ucEnDDAVerNo = 0;// fDDA ver 00 & 01 are all supported
	tVisaAidParam.ulTermFLmt = 5000;
	Clss_SetVisaAidParam_Wave(&tVisaAidParam);


	/*
	Clss_GetReaderParam_Wave(&tClssParam);
	memcpy(tClssParam.aucTmCap,"\xE0\xE1\xC8",3);
	memcpy(tClssParam.aucTmCapAd,"\xE0\x00\xF0\xA0\x01",5);
	tClssParam.ucTmType = 0x22;

	memcpy(tClssParam.aucTmCntrCode, g_stEmvParam.CountryCode, 2);
	memcpy(tClssParam.aucTmRefCurCode, g_stEmvParam.ReferCurrCode, 2);
	memcpy(tClssParam.aucTmTransCur, g_stEmvParam.TransCurrCode, 2);
	Clss_SetReaderParam_Wave(&tClssParam);
	// move [1/8/2015 jiangjy]
	// 	memset(&ClssVisaAidParam,0,sizeof(Clss_VisaAidParam));
	// 	ClssVisaAidParam.ulTermFLmt = 0;
	// 	Clss_SetVisaAidParam_Wave(&ClssVisaAidParam);

	memset(&tVisaAidParam,0,sizeof(Clss_VisaAidParam));
	tVisaAidParam.ucCvmReqNum = 2;
	tVisaAidParam.aucCvmReq[0] = RD_CVM_REQ_SIG;
	tVisaAidParam.aucCvmReq[1] = RD_CVM_REQ_ONLINE_PIN;

	tVisaAidParam.ucDomesticOnly = 0x00; // 01(default):only supports domestic cl transaction
	tVisaAidParam.ucEnDDAVerNo = 0;// fDDA ver 00 & 01 are all supported
	tVisaAidParam.ulTermFLmt = 5000;
	Clss_SetVisaAidParam_Wave(&tVisaAidParam);
	*/

	//added be Kevinliu 20161026
	LoadClssTLVFromFS();
}

static void ClssTermParamSet_MC(void)//for paypass
{
	int i = 0, iRet = 0, iNum=0;
	unsigned char aucBuff[50]={0},aucAmount[6]={0};
	PAYPASS_CONF_T stPayPassConfNode = {0};
	PAYPASS_AID_NODE_T stPayPassAidNode = {0};
	CLSS_OTHER_CONF_NODE_T stClssOtherConfNode = {0};

	Clss_SetTLVDataList_MC("\x5F\x57\x00", 3);//Account type no value
	Clss_SetTLVDataList_MC("\x9F\x01\x00", 3);//Acquire id no value
	Clss_SetTLVDataList_MC("\x9F\x1E\x08\x11\x22\x33\x44\x55\x66\x77\x88", 11);//Interface Device Serial Number with any value
	Clss_SetTLVDataList_MC("\x9F\x15\x02\x00\x01", 5);//Merchant Category Code with any value
	Clss_SetTLVDataList_MC("\x9F\x16\x00", 3);//Merchant Identifier no value
	Clss_SetTLVDataList_MC("\x9F\x4E\x00", 3);//Merchant Name and Location no value
	Clss_SetTLVDataList_MC("\x9F\x33\x00", 3);//terminal capability no value
	Clss_SetTLVDataList_MC("\x9F\x1C\x00", 3);//terminal id no value

	Clss_SetTagPresent_MC("\xDF\x81\x04", 0);//Balance Before GAC
	Clss_SetTagPresent_MC("\xDF\x81\x05", 0);//Balance After GAC
	Clss_SetTagPresent_MC("\xDF\x81\x2D", 0);//Message Hold Time

	Clss_SetTLVDataList_MC("\x9F\x7E\x00", 3);//Mobile Support Indicator

	Clss_SetTLVDataList_MC("\xDF\x81\x08\x00", 4);//DS AC Type
	Clss_SetTLVDataList_MC("\xDF\x60\x00", 3);//DS Input (Card)
	Clss_SetTLVDataList_MC("\xDF\x81\x09\x00", 4);//DS Input (Term)
	Clss_SetTLVDataList_MC("\xDF\x62\x00", 3);//DS ODS Info
	Clss_SetTLVDataList_MC("\xDF\x81\x0A\x00", 4);//DS ODS Info For Reader
	Clss_SetTLVDataList_MC("\xDF\x63\x00", 3);//DS ODS Term

	Clss_SetTagPresent_MC("\xDF\x81\x10", 0);//Proceed To First Write Flag Tag not present
	Clss_SetTagPresent_MC("\xDF\x81\x12", 0);//Tags To Read Tag not present
	Clss_SetTagPresent_MC("\xFF\x81\x02", 0);//Tags To Write Before Gen AC
	Clss_SetTagPresent_MC("\xFF\x81\x03", 0);//Tags To Write After Gen AC
	Clss_SetTagPresent_MC("\xDF\x81\x27", 0);//Time Out Value Tag not present

	//modified by kevin liu 20160628
	SetDETData("\x9F\x5C", 2, "\x7A\x45\x12\x3E\xE5\x9C\x7F\x40", 8);//DS Requested Operator ID
	Clss_SetTagPresent_MC("\xDF\x81\x0D", 0);
	Clss_SetTagPresent_MC("\x9F\x70", 0);
	Clss_SetTagPresent_MC("\x9F\x75", 0);

	SetDETData("\x9F\x09", 2, "\x00\x02", 2);
	//	SetDETData("\x9F\x40", 2, "\x00\x00\x00\x00\x00", 5);

	//amount
	memset(aucAmount, 0, sizeof(aucAmount));
	memset(aucBuff, 0, sizeof(aucBuff));
	sprintf((char *)aucBuff, "%012ld", sg_tClssTransParam.ulAmntAuth);
	TwoOne_app(aucBuff, 12, aucAmount);
	SetDETData("\x9F\x02", 2, aucAmount, 6);

	//amount other
	memset(aucAmount, 0, sizeof(aucAmount));
	memset(aucBuff, 0, sizeof(aucBuff));
	sprintf((char *)aucBuff, "%012ld", sg_tClssTransParam.ulAmntOther);
	TwoOne_app(aucBuff, 12, aucAmount);
	SetDETData("\x9F\x03", 2, aucAmount, 6);
	SetDETData("\x9C", 1, &sg_tClssTransParam.ucTransType, 1);
	SetDETData("\x9A", 1, sg_tClssTransParam.aucTransDate, 3);
	SetDETData("\x9F\x21", 2, sg_tClssTransParam.aucTransTime, 3);

	memset(&stPayPassConfNode, 0, sizeof(stPayPassConfNode));
	iRet = GetPayPassConfData(&stPayPassConfNode, sizeof(stPayPassConfNode), i);
	if ( iRet < 0 ) 
	{
		return iRet;
	}

	SetDETData("\xDF\x81\x1B", 3, &stPayPassConfNode.ucKernelConf, 1); // by ZhangYuan 2017-03-23, load KernelConfiguration from CLSS.XML, set to kernel	

	Clss_SetTLVDataList_MC("\x9F\x7E\x01\x01", 4);//by ZhangYuan 2017-03-30, Mobile Support Indicator

	SetDETData("\xDF\x81\x17", 3, &stPayPassConfNode.ucCardDataInput, 1);//card data input capability
	SetDETData("\xDF\x81\x18", 3, &stPayPassConfNode.ucCVMCapability_CVMRequired, 1);//ONLINE PIN :40 /SIG:20
	SetDETData("\xDF\x81\x19", 3, &stPayPassConfNode.ucCVMCapability_NoCVMRequired, 1);//NO CVM
	SetDETData("\xDF\x81\x1F", 3, &stPayPassConfNode.ucSecurityCapability, 1);//08:CDA      //40:DDA 
	SetDETData("\xDF\x81\x1E", 3, &stPayPassConfNode.ucMagneticCVM, 1); //Modified by Kevin Liu 20160801
	SetDETData("\xDF\x81\x2C", 3, &stPayPassConfNode.ucMageticNoCVM, 1);

	SetDETData("\xDF\x81\x1A", 3, "\x9F\x6A\x04",3);//Default UDOL

	SetDETData("\xDF\x81\x1C", 3, &stPayPassConfNode.ucTornLeftTime, 1);
	SetDETData("\xDF\x81\x1D", 3, &stPayPassConfNode.ucMaximumTornNumber, 1);

	SetDETData("\xDF\x81\x0C", 3, "\x02", 1);

	SetDETData("\x9F\x35", 2, &stPayPassConfNode.ucTerminalType, 1);

	SetDETData("\x9F\x40", 2, stPayPassConfNode.aucAdditionalTerminalCapability, 5);

	EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "ucKernelConf = %02x \n", stPayPassConfNode.ucKernelConf);										
	EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "ucCardDataInput = %02x \n", stPayPassConfNode.ucCardDataInput); 							   
	EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "ucCVMCapability_CVMRequired = %02x \n", stPayPassConfNode.ucCVMCapability_CVMRequired); 		
	EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "ucCVMCapability_NoCVMRequired = %02x \n", stPayPassConfNode.ucCVMCapability_NoCVMRequired);    
	EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "ucSecurityCapability = %02x \n", stPayPassConfNode.ucSecurityCapability);						
	EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "ucMagneticCVM = %02x \n", stPayPassConfNode.ucMagneticCVM); 								   
	EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "ucMageticNoCVM = %02x \n", stPayPassConfNode.ucMageticNoCVM);									
	EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "ucTerminalType = %02x \n", stPayPassConfNode.ucTerminalType);	 
	EL_LOG_HEX(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "aucAdditionalTerminalCapability: ", stPayPassConfNode.aucAdditionalTerminalCapability, 5); 


	//from contactless other configuration.
	memset(&stClssOtherConfNode, 0, sizeof(stClssOtherConfNode));
	iRet = GetClssOtherCofData(&stClssOtherConfNode, sizeof(stClssOtherConfNode), i);
	if ( iRet < 0 ) 
	{
		return iRet;
	}

	SetDETData("\x9F\x1A", 2, stClssOtherConfNode.aucCountryCode, 2);
	SetDETData("\x5F\x2A", 2, stClssOtherConfNode.aucCurrencyCode, 2);

	//set the parameters according to the aid.
	iNum = ClssGetNodeNum(CLSS_PAYPASS_AID_NODE);
	if ( iNum <= 0 ) 
	{
		return iNum;
	}
	for(i = 0; i < iNum; i++)
	{
		memset(&stPayPassAidNode, 0, sizeof(stPayPassAidNode));
		iRet = GetPayPassAidData(&stPayPassAidNode, sizeof(stPayPassAidNode), i);
		if ( iRet < 0 ) 
		{
			return iRet;
		}

		EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "i = %d\n", i);
		EL_LOG_HEX(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "aucTermRisk: ", stPayPassAidNode.aucTermRisk, 8);

		if ( memcmp(sg_aucFinalAID, stPayPassAidNode.aucAID, stPayPassAidNode.ucAidLen) == 0)  // Modified by ZhangYuan 2017-02-21, if equal, memcmp == 0
		{
			//			if ( memcmp(sg_aucFinalAID, "\xA0\x00\x00\x00\x04\x10\x10",7) == 0 ) {
			//				//MCD
			//				SetDETData("\xDF\x81\x1B", 3, "\x20", 1);
			//			}
			//			else if ( memcmp(sg_aucFinalAID,  "\xA0\x00\x00\x00\x04\x30\x60", 7) == 0 ) {
			//				//Mestro  NO MAG-STRIPE 
			//				SetDETData("\xDF\x81\x1B", 3, "\xA0", 1);
			//			}

			iRet = SetDETData("\x9F\x1D", 2, stPayPassAidNode.aucTermRisk, 8);	// Added by ZhangYuan 2017-03-11
			if(g_stClssTransParam.ucTransType == 0x20) //by ZhangYuan 2017-03-17, refund rquired AAC
			{
				SetDETData("\xDF\x81\x23", 3, "\x00\x00\x00\x00\x00\x00", 6);//floor limit
			}
			else
			{
				unsigned char aucMyBuf[32+1]={0}, aucMyAmt[64+1]={0};

				memset(aucMyAmt, 0, sizeof(aucMyAmt));
				memset(aucMyBuf, 0, sizeof(aucMyBuf));
				sprintf((char *)aucMyBuf, "%012ld", stPayPassAidNode.ulClssFloorLimit); //paypass notice
				TwoOne_app(aucMyBuf, 12, aucMyAmt);
				SetDETData("\xDF\x81\x23", 3, aucMyAmt, 6);//floor limit
			}

			SetDETData("\xDF\x81\x24", 3, stPayPassAidNode.aucTxnLmt_NoOnDevice, 6); //No On-device CVM
			SetDETData("\xDF\x81\x25", 3, stPayPassAidNode.aucTxnLmt_OnDevice, 6); //On-device CVM
			SetDETData("\xDF\x81\x26", 3, stPayPassAidNode.aucRDCVMLmt, 6);//cvm limit

			SetDETData("\xDF\x81\x20", 3, stPayPassAidNode.aucTACDefault, 5);

			if(g_stClssTransParam.ucTransType == 0x20) //by ZhangYuan 2017-03-17, refund rquired AAC
			{
				SetDETData("\xDF\x81\x21", 3, (uchar *)"\xFF\xFF\xFF\xFF\xFF", 5);//TAC denial
			}
			else
			{
				SetDETData("\xDF\x81\x21", 3, stPayPassAidNode.aucTACDenial, 5);
			}
			SetDETData("\xDF\x81\x22", 3, stPayPassAidNode.aucTACOnline, 5);//TAC online

			EL_LOG_HEX(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "AID: ", stPayPassAidNode.aucAID, stPayPassAidNode.ucAidLen);                                 
			EL_LOG_HEX(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "sg_aucFinalAID: ", sg_aucFinalAID, sg_ucFinalAIDLen);                                       
			EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "ulClssFloorLimit = %ld \n", stPayPassAidNode.ulClssFloorLimit);           
			EL_LOG_HEX(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "aucTxnLmt_NoOnDevice: ", stPayPassAidNode.aucTxnLmt_NoOnDevice, 6);    
			EL_LOG_HEX(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "aucTxnLmt_OnDevice: ", stPayPassAidNode.aucTxnLmt_OnDevice, 6);         
			EL_LOG_HEX(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "aucRDCVMLmt: ", stPayPassAidNode.aucRDCVMLmt, 6);                      
			EL_LOG_HEX(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "aucTACDefault: ", stPayPassAidNode.aucTACDefault, 5);                   
			EL_LOG_HEX(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "aucTACDenial: ", stPayPassAidNode.aucTACDenial, 5);                 
			EL_LOG_HEX(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "aucTACOnline: ", stPayPassAidNode.aucTACOnline, 5);        

			break;
		}
	}




	/*
	SetDETData("\xDF\x81\x17", 3, "\xE0", 1);//card data input capability
	SetDETData("\xDF\x81\x18", 3, "\x60", 1);//ONLINE PIN :40 /SIG:20
	SetDETData("\xDF\x81\x19", 3, "\x08", 1);//NO CVM
	SetDETData("\xDF\x81\x1F", 3, "\xC8", 1);//08:CDA      //40:DDA 
	SetDETData("\xDF\x81\x1A", 3, "\x9F\x6A\x04",3);//Default UDOL
	SetDETData("\x9F\x6D", 2, "\x00\x01", 2);
	SetDETData("\xDF\x81\x1E", 3, "\x20", 1); //Modified by Kevin Liu 20160801
	SetDETData("\xDF\x81\x2C", 3, "\x00", 1);

	memset(aucAmount, 0, sizeof(aucAmount));
	memset(aucBuff, 0, sizeof(aucBuff));

	sprintf((char *)aucBuff, "%012ld", sg_tClssTransParam.ulAmntAuth);

	TwoOne_app(aucBuff, 12, aucAmount);
	SetDETData("\x9F\x02", 2, aucAmount, 6);

	memset(aucAmount, 0, sizeof(aucAmount));
	memset(aucBuff, 0, sizeof(aucBuff));

	sprintf((char *)aucBuff, "%012ld", sg_tClssTransParam.ulAmntOther);

	TwoOne_app(aucBuff, 12, aucAmount);
	SetDETData("\x9F\x03", 2, aucAmount, 6);

	SetDETData("\x9C", 1, &sg_tClssTransParam.ucTransType, 1);
	SetDETData("\x9A", 1, sg_tClssTransParam.aucTransDate, 3);
	SetDETData("\x9F\x21", 2, sg_tClssTransParam.aucTransTime, 3);

	SetDETData("\xDF\x81\x20", 3, "\x04\x00\x00\x00\x00", 5);
	SetDETData("\xDF\x81\x21", 3, "\x04\x00\x00\x00\x00", 5);
	SetDETData("\xDF\x81\x22", 3, "\xF8\x50\xAC\xF8\x00", 5);//TAC online

	//[1/9/2015 jiangjy] limit  set for AID
	SetDETData("\xDF\x81\x23", 3, "\x00\x00\x00\x00\x50\x00", 6);//floor limit
	SetDETData("\xDF\x81\x24", 3, "\x00\x00\x00\x10\x00\x00", 6);
	SetDETData("\xDF\x81\x25", 3, "\x00\x00\x00\x10\x00\x00", 6);
	SetDETData("\xDF\x81\x26", 3, "\x00\x00\x00\x00\x30\x00", 6);//cvm limit

	SetDETData("\x9F\x35", 2, "\x22", 1);

	Clss_SetTagPresent_MC("\xDF\x81\x30", 0);
	SetDETData("\xDF\x81\x1C", 3, "\x00\x00", 2);
	SetDETData("\xDF\x81\x1D", 3, "\x00", 1);
	SetDETData("\xDF\x81\x0C", 3, "\x02", 1);
	Clss_SetTagPresent_MC("\xDF\x81\x2D", 0);
	SetDETData("\x9F\x1A", 2, g_stEmvParam.CountryCode, 2);
	SetDETData("\x5F\x2A", 2, g_stEmvParam.TransCurrCode, 2);
	if ( sg_ucFinalAIDLen == 0 ) {
	return;
	}
	if ( memcmp(sg_aucFinalAID, "\xA0\x00\x00\x00\x04\x10\x10",7) == 0 ) {
	//MCD
	SetDETData("\xDF\x81\x1B", 3, "\x20", 1);  
	//Tag 9F1D set based on definition
	//		Clss_AddProprietaryData_MC(CombiTLVElement((uchar*)"\x9F\x1D", 2, (uchar *)"\x6C\xF8\x00\x00\x00\x00\x00\x00", 8, CLSS_SRC_TM, 0));
	}
	else if ( memcmp(sg_aucFinalAID,  "\xA0\x00\x00\x00\x04\x30\x60", 7) == 0 ) {
	//Mestro  NO MAG-STRIPE 
	SetDETData("\xDF\x81\x1B", 3, "\xA0", 1);
	//Tag 9F1D
	//		Clss_AddProprietaryData_MC(CombiTLVElement((uchar*)"\x9F\x1D", 2, (uchar *)"\x40\xF8\x80\x00\x00\x00\x00\x00", 8, CLSS_SRC_TM, 0));
	}
	*/

	//added be Kevinliu 20161026
	LoadClssTLVFromFS();
}


static int AppSetCurAppType(unsigned char ucKernType)
{
	int iRet=0;

	if ( ucKernType == KERNTYPE_VIS ) 
	{
		AppSetAppType(KERNTYPE_VIS);
		Clss_CoreInit_Wave();
	}
	else if ( ucKernType == KERNTYPE_MC ) 
	{
		AppSetAppType(KERNTYPE_MC);
		Clss_CoreInit_MC(1);
		Clss_SetParam_MC("\x04", 1);// set the timer number for kernel [1/21/2014 ZhouJie]
		AppCleanTornLog_MC();
	}
	else 
	{
		return CLSS_TERMINATE;
	}
	return EMV_OK;
}


static int AppTransProc_VISA(unsigned char ucTransPath, unsigned char ucACType)
{
	int iRet=0;
	int iTempLen=0;
	unsigned char ucPkIndex=0;
	unsigned char ucDDAFailFlg=0;

	switch ( ucTransPath ) {
	case CLSS_VISA_MSD:
	case CLSS_VISA_MSD_CVN17:
	case CLSS_VISA_MSD_LEGACY:
		break;
	case CLSS_VISA_QVSDC:
		AppRemovePicc();
		iRet = Clss_ProcRestric_Wave();
		EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "Clss_ProcRestric_Wave = %d \n", iRet);
		if ( iRet ) 
		{
			return iRet;
		}

		if (ucACType == AC_TC && sg_tClssTransParam.ucTransType != 0x20)//REFUND
		{
			Clss_DelAllRevocList_Wave();
			Clss_DelAllCAPK_Wave();

			if ( Clss_GetTLVData_Wave(0x8F, &ucPkIndex,  &iTempLen) == 0)  
			{
				//if (AppConv_GetTLVData(0x9F06, aucAid, &nTempLen) == 0)//get aid
				{
					AppSetClss_capk(ucPkIndex, sg_aucFinalAID, sg_ucAppType);// capk
					AppSetClss_RevLst(ucPkIndex, sg_aucFinalAID, sg_ucAppType);
				}

				iRet = Clss_CardAuth_Wave(&ucACType, &ucDDAFailFlg);//capk
				EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "Clss_CardAuth_Wave = %d, ucACType=%02x\n", iRet, ucACType);
				if (iRet != 0) 
				{					
					if (iRet != CLSS_USE_CONTACT) 
					{
						iRet =  CLSS_TERMINATE;	
					}
				}
				else
				{
					if (ucDDAFailFlg == 1) 
					{
						iRet =  CLSS_TERMINATE;							
					}
					else
					{
						//modified by Kevin Liu 20160621
						//						Inter_DisplayMsg(MSG_APPROVED);
					}
				}
			}
			else
			{
				if (ucACType == AC_ARQC || ucACType == AC_TC)
				{
					iRet = 0;
				}
				else // (ucACType == AC_AAC)
				{
					iRet = EMV_DENIAL;
				}
			}
		}

		break;
	case CLSS_VISA_WAVE2:
		if (ucACType == AC_TC)
		{
			Clss_DelAllRevocList_Wave();
			Clss_DelAllCAPK_Wave();

			if ( Clss_GetTLVData_Wave(0x8F, &ucPkIndex,  &iTempLen) == 0)
			{
				//if (AppConv_GetTLVData(0x9F06, aucAid, &nTempLen) == 0)
				{
					AppSetClss_capk(ucPkIndex, sg_aucFinalAID, sg_ucAppType);
					AppSetClss_RevLst(ucPkIndex, sg_aucFinalAID, sg_ucAppType);
				}
			}
			// internal authenticate, offline approval
			iRet = Clss_CardAuth_Wave(&ucACType, &ucDDAFailFlg);
			EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "Clss_CardAuth_Wave = %d, ACType=%02x \n", iRet, ucACType);
			if (iRet)
			{
				return EMV_DENIAL; // 0424-3
			}
		}
		AppRemovePicc();


		break;
	default:
		//AppRemovePicc(PICC_LED_NONE);
		iRet = CLSS_TERMINATE;
		break;
	}

	if (iRet == 0)
	{
		AppSetTransACType(ucACType);
	}
	return iRet;
}

static int AppTransProc_MC(uchar ucTransPath, uchar *pucACType)
{
	int iRet=0,nTempLen=0;
	uchar ucPkIndex,aucAid[17]={0};
	CLSS_TORN_LOG_RECORD atAppTornTransLog[5];//Torn Transaction Log
	int nAppTornLogNum = 0;//number of Tornlog
	int nUpdatedFlg = 0;

	if (ucTransPath == CLSS_MC_MCHIP) // 0x06)
	{
		Clss_DelAllRevocList_MC_MChip();
		Clss_DelAllCAPK_MC_MChip();

		if (Clss_GetTLVDataList_MC((uchar*)"\x8F", 1, 1, &ucPkIndex, (uint *)&nTempLen) == 0)
		{
			if (Clss_GetTLVDataList_MC((uchar*)"\x4F", 1, sizeof(aucAid), aucAid, (uint *)&nTempLen) == 0)
			{

				AppSetClss_capk(ucPkIndex, aucAid, sg_ucAppType);// CAPK [1/4/2015 jiangjy]
				AppSetClss_RevLst(ucPkIndex, aucAid, sg_ucAppType);
			}
		}

		AppGetTornLog_MC(atAppTornTransLog, &nAppTornLogNum);
		if (nAppTornLogNum)
		{
			Clss_SetTornLog_MC_MChip(atAppTornTransLog, nAppTornLogNum);
		}
		iRet = Clss_TransProc_MC_MChip(pucACType);
		EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "Clss_TransProc_MC_MChip = %d, pucACType=%02x\n", iRet, *pucACType);
		memset(atAppTornTransLog, 0, sizeof(atAppTornTransLog));
		nAppTornLogNum = 0;
		nUpdatedFlg = 0;
		Clss_GetTornLog_MC_MChip(atAppTornTransLog, &nAppTornLogNum, &nUpdatedFlg);
		if (nUpdatedFlg)
		{
			AppSaveTornLog_MC(atAppTornTransLog, nAppTornLogNum);
		}
	}
	else if (ucTransPath == CLSS_MC_MAG) // 0x05)
	{
		iRet = Clss_TransProc_MC_Mag(pucACType);
		EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "Clss_TransProc_MC_Mag = %d \n", iRet);
	}
	else
	{
		iRet = CLSS_TERMINATE;
	}

	if (iRet != ICC_CMD_ERR)//3G10-9301(Trx1_CCC_Res) 3G10-9300(Trx2_CCC_Res)
	{
		AppRemovePicc();//AppRemovePicc(PICC_LED_GREEN);
		Inter_DisplayMsg(MSG_CARD_READ_OK);
	}
	if (iRet)
	{
		return iRet;
	}

	AppSetTransACType(*pucACType);

	return EMV_OK;
}

static int ClssProcFlow_VISA(uchar *pucFinalData, int iFinalDataLen, Clss_PreProcInterInfo tClssProcInterInfo)
{
#ifdef CLSS_DEBUG
	Clss_SetDebug_Wave(1);
#endif

	int iRet=0;
	uchar ucAcType=0, ucPathType=0, ucTemp[100]={0};

	//modified by Kevinliu 20161101, do it after select data. 
	//	ClssBaseParameterSet_WAVE();
	SetFinalSelectAID(pucFinalData+1, pucFinalData[0]);
	iRet = Clss_SetFinalSelectData_Wave(pucFinalData, iFinalDataLen);
	EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "Clss_SetFinalSelectData_Wave = %d\n", iRet);
	if( iRet != EMV_OK ) {
		return iRet;
	}

	ClssBaseParameterSet_WAVE();

	if( sg_ucDRLSupportFlg == 1 ) 
	{
		if ( Clss_GetTLVData_Wave(0x9F5A, sg_aucProID, &sg_iProIDLen) == EMV_OK ) 
		{
			iRet = AppFindMatchProID(sg_aucProID, sg_iProIDLen);
			EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "AppFindMatchProID = %d\n", iRet);
			if ( iRet == EMV_OK ) 
			{
				iRet = Clss_SetDRLParam_Wave(sg_tProgInfo);
				EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "Clss_SetDRLParam_Wave = %d\n", iRet);
			}
		}
		iRet = Clss_PreTransProc_Entry(&g_stClssTransParam);   //support DRL, call again
		EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "Again iRet=%d\n", iRet);
		if ( iRet ) 
		{
			return iRet;
		}
	}
	iRet = Clss_SetTransData_Wave(&sg_tClssTransParam, &tClssProcInterInfo);
	EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "Clss_SetTransData_Wave=%d\n", iRet);
	if ( iRet != EMV_OK ) {
		return iRet;
	}

	ucAcType = 0;
	memset(ucTemp, 0, sizeof(ucTemp));
	iRet = Clss_Proctrans_Wave(ucTemp, &ucAcType);
	EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "Clss_Proctrans_Wave iRet = %d, ucAcType=%02x\n ", iRet, ucAcType);
	EL_LOG_HEX(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "tClssProcInterInfo.aucReaderTTQ: ", tClssProcInterInfo.aucReaderTTQ, 5);
	if ( iRet ){
		if ( iRet == CLSS_RESELECT_APP ) 
		{
			iRet = Clss_DelCurCandApp_Entry();
			if ( iRet ) {
				return iRet;
			}
			return iRet;
		}
		//see phone
		else if ( (iRet == CLSS_REFER_CONSUMER_DEVICE) && ((tClssProcInterInfo.aucReaderTTQ[0] & 0x20) == 0x20) ) 
		{
			Inter_DisplayMsg(MSG_SEE_PHONE);
			iRet= App_Try_Again;
			DelayMs(1200);
			return iRet;
		}
		else if ( iRet == CLSS_USE_CONTACT ) 
		{
//			Inter_DisplayMsg(MSG_TRY_ANOTHER_INTERFACE);
			return CLSS_USE_CONTACT;
		}
		else 
		{
			return iRet;
		}
	}

	ucPathType = ucTemp[0];
	AppSetTransPath(ucPathType);

	iRet = AppTransProc_VISA(ucPathType, ucAcType);
	EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "AppTransProc_VISA = %d\n", iRet);

	return iRet;
}

static int ClssProcFlow_MC(uchar *pucFinalData, int iFinalDataLen, Clss_PreProcInterInfo tClssProcInterInfo)
{
#ifdef CLSS_DEBUG
	Clss_SetDebug_MC(1);
#endif
	int iRet=0;
	uchar ucAcType=0, ucPathType=0;

	Clss_SetCBFun_SendTransDataOutput_MC(ClssSendTransDataOutput_MC);
	SetFinalSelectAID(pucFinalData+1, pucFinalData[0]);
	//removed by Kevin Liu, new version library need to set data after Clss_SetFinalSelectData_MC.
	//ClssTermParamSet_MC();
	AppLoadTornLog_MC();	//added by Kevin Liu 20160728
	iRet = Clss_SetFinalSelectData_MC(pucFinalData, iFinalDataLen);
	EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "Clss_SetFinalSelectData_MC = %d\n", iRet);
	//the return code is not EMV_OK, Application should get DF8129). [12/29/2014 jiangjy]
	if ( iRet == CLSS_RESELECT_APP ) 
	{
		iRet = Clss_DelCurCandApp_Entry();
		if ( iRet != 0 ) 
		{
			InitPaymentData();
			sg_aucUserInterReqData[0]=MI_ERROR_OTHER_CARD;
			sg_aucUserInterReqData[1]=MI_NOT_READY;
			memcpy(sg_aucUserInterReqData+2, MSG_HOLD_TIME_VALUE, 3);

			sg_aucErrIndication[5]=MI_ERROR_OTHER_CARD;
			sg_aucErrIndication[1] = L2_EMPTY_CANDIDATE_LIST;//S51.11
			sg_aucOutcomeParamSet[0] = OC_END_APPLICATION;
			sg_tDiscretionayData.ucErrIndicFlg = 1;
			//					nSendTransDataOutput_MC(T_UIRD | T_OCPS | T_DISD);//S51.11 S51.12
			//			return ERR_TRAN_FAIL;
			return iRet;
		}
		return iRet;
	}
	else if ( iRet ) 
	{
		return iRet;
	}
	//added by Kevin Liu, new version library need to set data after Clss_SetFinalSelectData_MC.
	ClssTermParamSet_MC();
	iRet = Clss_InitiateApp_MC();
	EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "Clss_InitiateApp_MC = %d\n", iRet);
	//the return code is not EMV_OK, Application should get DF8129)
	if ( iRet == CLSS_RESELECT_APP ) 
	{
		iRet = Clss_DelCurCandApp_Entry();
		if ( iRet ) 
		{
			InitPaymentData();// paypass 3.0.1 by zhoujie
			sg_aucUserInterReqData[0]=MI_ERROR_OTHER_CARD;//S51.11 for paypass 3.0.1 by zhoujie
			sg_aucUserInterReqData[1]=MI_NOT_READY;
			memcpy(sg_aucUserInterReqData+2, MSG_HOLD_TIME_VALUE, 3);

			sg_aucErrIndication[5]=MI_ERROR_OTHER_CARD;
			sg_aucErrIndication[1] = L2_EMPTY_CANDIDATE_LIST;//S51.11
			sg_aucOutcomeParamSet[0] = OC_END_APPLICATION;
			sg_tDiscretionayData.ucErrIndicFlg = 1;
			//					nSendTransDataOutput_MC(T_UIRD | T_OCPS | T_DISD);//S51.11 S51.12
			//			return ERR_TRAN_FAIL;
			return iRet;
		}
		return iRet;
	}
	else if ( iRet ) 
	{
		return iRet;
	}
	ucPathType = 0;
	iRet = Clss_ReadData_MC(&ucPathType);
	EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "Clss_ReadData_MC = %d\n", iRet);
	//f the return code is not EMV_OK, Application should get DF8129)
	if ( iRet ) {
		if ( iRet == CLSS_RESELECT_APP ) {
			iRet = Clss_DelCurCandApp_Entry();
			if ( iRet ) {
				InitPaymentData();
				sg_aucUserInterReqData[0]=MI_ERROR_OTHER_CARD;
				sg_aucUserInterReqData[1]=MI_NOT_READY;
				memcpy(sg_aucUserInterReqData+2, MSG_HOLD_TIME_VALUE, 3);

				sg_aucErrIndication[5]=MI_ERROR_OTHER_CARD;
				sg_aucErrIndication[1] = L2_EMPTY_CANDIDATE_LIST;//S51.11
				sg_aucOutcomeParamSet[0] = OC_END_APPLICATION;
				sg_tDiscretionayData.ucErrIndicFlg = 1;

				return iRet;
			}
			return iRet;
		}
		else {
			return iRet;
		}
	}

	AppSetTransPath(ucPathType);

	iRet = AppTransProc_MC(ucPathType, &ucAcType);

	EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "AppTransProc_MC = %d, ucAcType=%02x\n", iRet, ucAcType);
	EL_LOG_HEX(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "sg_aucUserInterReqData: ", sg_aucUserInterReqData, sizeof(sg_aucUserInterReqData));
	EL_LOG_HEX(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "sg_aucOutcomeParamSet: ", sg_aucOutcomeParamSet, sizeof(sg_aucOutcomeParamSet));
#ifdef CLSS_DEBUG_
	{
		int mRet = 0, uiMyBufLen=0;
		unsigned char aucMyBuf[256+1] = {0};

		memset(aucMyBuf, 0, sizeof(aucMyBuf));
		mRet = Clss_GetTLVDataList_MC("\xDF\x81\x15", 3, sizeof(aucMyBuf), aucMyBuf, &uiMyBufLen);
		EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "Clss_GetTLVDataList_MC=%d, DF8115\n", mRet);
		EL_LOG_HEX(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "DF8115: ", aucMyBuf, uiMyBufLen);

		memset(aucMyBuf, 0, sizeof(aucMyBuf));
		mRet = Clss_GetTLVDataList_MC("\xDF\x81\x16", 3, sizeof(aucMyBuf), aucMyBuf, &uiMyBufLen);
		EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "Clss_GetTLVDataList_MC=%d, DF8116\n", mRet);
		EL_LOG_HEX(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "DF8116: ", aucMyBuf, uiMyBufLen);

		memset(aucMyBuf, 0, sizeof(aucMyBuf));
		mRet = Clss_GetTLVDataList_MC("\xDF\x81\x29", 3, sizeof(aucMyBuf), aucMyBuf, &uiMyBufLen);
		EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "Clss_GetTLVDataList_MC=%d, DF8129\n", mRet);
		EL_LOG_HEX(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "DF8129: ", aucMyBuf, uiMyBufLen);
	}

#endif
	if ( sg_aucUserInterReqData[0] == MI_SEE_PHONE ) 
	{
		Inter_DisplayMsg(MSG_SEE_PHONE);
	}
	if ( (sg_aucOutcomeParamSet[0] == OC_TRY_AGAIN) || (sg_aucOutcomeParamSet[1] != OC_NA) ) 
	{
		iRet = App_Try_Again;
	}

	return iRet;
}

static int ClssProcFlow_ALL(void)
{
	int	 iRet=0, iLen=0;
	unsigned char ucTemp[300]={0}, ucKernType=0;
	Clss_PreProcInterInfo tClssProcInterInfo;

	Clss_SetMCVersion_Entry(3);
	//app select
	iRet = Clss_AppSlt_Entry(0,0);
	EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "Clss_AppSlt_Entry = %d \n", iRet);
	//returns	EMV_OK	CLSS_PARAM_ERR	ICC_CMD_ERR	ICC_BLOCK	EMV_NO_APP	EMV_APP_BLOCK	EMV_NO_APP_PPSE_ERR
	if ( iRet != EMV_OK ) 
	{
		AppCreateOutcomeData_MC(iRet);
		return iRet;
	}

	while ( 1 ) 
	{
		AppInitPaymentData_MC();
		iLen = 0;
		ucKernType = 0;
		memset(ucTemp, 0, sizeof(ucTemp));
		iRet = Clss_FinalSelect_Entry(&ucKernType, ucTemp, &iLen);
		EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "Clss_FinalSelect_Entry = %d \n", iRet);
		//returns	EMV_OK	CLSS_PARAM_ERR	ICC_CMD_ERR	EMV_RSP_ERR	EMV_NO_APP	EMV_APP_BLOCK	ICC_BLOCK
		//CLSS_USE_CONTACT	EMV_DATA_ERR	CLSS_RESELECT_APP
		if( iRet != EMV_OK ) 
		{
			return iRet;
		}
		//VISA MASTERCARD
		if ( (ucKernType != KERNTYPE_VIS) && (ucKernType != KERNTYPE_MC) ) 
		{
			return EL_TRANS_RET_EMV_NO_APP;
		}

		//get pre-process data
		memset(&tClssProcInterInfo, 0, sizeof(Clss_PreProcInterInfo));
		iRet = Clss_GetPreProcInterFlg_Entry(&tClssProcInterInfo);
		EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "Clss_GetPreProcInterFlg_Entry = %d \n", iRet);
		//returns	EMV_OK	CLSS_PARAM_ERR	CLSS_USE_CONTACT
		if ( iRet != EMV_OK ) {
			return iRet;
		}

		iLen = 0;
		memset(ucTemp, 0, sizeof(ucTemp));
		iRet = Clss_GetFinalSelectData_Entry(ucTemp, &iLen);
		EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "Clss_GetFinalSelectData_Entry = %d \n", iRet);
		//returns	EMV_OK	CLSS_PARAM_ERR
		if ( iRet != EMV_OK ) {
			return iRet;
		}

		iRet = AppSetCurAppType(ucKernType);
		switch ( AppGetAppType() ) 
		{
		case KERNTYPE_VIS:
			iRet = ClssProcFlow_VISA(ucTemp, iLen, tClssProcInterInfo);
			EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "ClssProcFlow_VISA = %d \n", iRet);
			break;
		case KERNTYPE_MC:
			iRet = ClssProcFlow_MC(ucTemp, iLen, tClssProcInterInfo);
			EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "ClssProcFlow_MC = %d \n", iRet);
			break;
		default:
			break;
		}
		if ( iRet == EMV_OK ) 
		{
			break;
		}
		else if ( iRet == CLSS_RESELECT_APP ) 
		{
			continue;
		}
		//		else if(iRet == App_Try_Again)	// by ZhangYuan 2017-03-22
		//		{
		//#ifdef EUI_
		//			EShowMsgBox(EL_UI_PAGE_TRANS_TAP_AGAIN, NULL, 0);
		//#else
		//			ScrCls();
		//			PubDispString("PLS TAP AGAIN", 0|DISP_LINE_LEFT);
		//#endif
		//			break;
		//		}
		else 
		{
			AppRemovePicc();
			break;
		}
	}
	return iRet;
}

static int AppConv_CreateOutCome(int nRet, uchar ucACType, CLSS_OUTCOME_DATA *pstOutComeData)
{
	uchar aucOutcomeParamSet[8] = {0};
	uchar ucCVMType = 0;
	uint unLen=0;

	if(pstOutComeData == NULL)
	{
		return EMV_PARAM_ERR;
	}
	memset(pstOutComeData, 0, sizeof(CLSS_OUTCOME_DATA));
	pstOutComeData->unPathType = AppGetTransPath();
	switch (AppGetAppType())
	{
	case KERNTYPE_VIS:
		if (!nRet)
		{
			nRet = Clss_GetCvmType_Wave(&pstOutComeData->ucCVMType);//  [1/22/2015 jiangjy]
			if (nRet)
			{
				pstOutComeData->ucTransRet = CLSS_DECLINED;
			}
			else
			{
				if(ucACType == AC_TC)
				{
					pstOutComeData->ucTransRet = CLSS_APPROVE;
				}
				else if(ucACType == AC_ARQC)//AC_ARQC
				{
					pstOutComeData->ucTransRet = CLSS_ONLINE_REQUEST;
				}
				else
				{
					pstOutComeData->ucTransRet = CLSS_DECLINED;
				}
			}

		}
		else if (nRet == EMV_DENIAL || nRet == CLSS_DECLINE)
		{
			pstOutComeData->ucTransRet = CLSS_DECLINED;
		}
		else if (nRet == CLSS_USE_CONTACT)
		{
			pstOutComeData->ucTransRet = CLSS_TRY_ANOTHER_INTERFACE;
		}
		else
		{
			pstOutComeData->ucTransRet = CLSS_END_APPLICATION;
		}
		break;
	case KERNTYPE_MC:
		memset(aucOutcomeParamSet, 0, sizeof(aucOutcomeParamSet));
		Clss_GetTLVDataList_MC("\xDF\x81\x29", 3, sizeof(aucOutcomeParamSet), aucOutcomeParamSet, &unLen);
		EL_LOG_HEX(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "DF8129: ", aucOutcomeParamSet, 8);
		switch(aucOutcomeParamSet[0] & 0xF0)
		{
		case CLSS_OC_APPROVED:
			pstOutComeData->ucTransRet = CLSS_APPROVE;
			break;
		case CLSS_OC_DECLINED:
			pstOutComeData->ucTransRet = CLSS_DECLINED;
			break;
		case CLSS_OC_ONLINE_REQUEST:
			pstOutComeData->ucTransRet = CLSS_ONLINE_REQUEST;
			break;
		case CLSS_OC_TRY_ANOTHER_INTERFACE:
			pstOutComeData->ucTransRet = CLSS_TRY_ANOTHER_INTERFACE;
			break;
		default://CLSS_OC_END_APPLICATION
			pstOutComeData->ucTransRet = CLSS_END_APPLICATION;
			break;
		}
		switch(aucOutcomeParamSet[3]&0xF0)
		{
		case CLSS_OC_NO_CVM:
			pstOutComeData->ucCVMType = CLSS_CVM_NO;
			break;
		case CLSS_OC_OBTAIN_SIGNATURE:
			pstOutComeData->ucCVMType = CLSS_CVM_SIG;
			break;
		case CLSS_OC_ONLINE_PIN:
			pstOutComeData->ucCVMType = CLSS_CVM_ONLINE_PIN;
			break;
		case CLSS_OC_CONFIRM_CODE_VER:
			pstOutComeData->ucCVMType = CLSS_CVM_OFFLINE_PIN;
			break;
		default:
			pstOutComeData->ucCVMType = CLSS_CVM_NO;
			break;
		}
		break;
	default:
		break;
	}

	EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "nRet = %d, ucTransRet=%02x, ucCVMType=%02x \n", nRet, pstOutComeData->ucTransRet, pstOutComeData->ucCVMType);

	return nRet;

}

int ClssCompleteTrans(void)
{
	int iRet = 0;
	unsigned char ucKernelType = 0, aucRspCode[3] = {0}, ucOnlineResult = 0;
	unsigned char aucAuthCode[6+1] = {0}, ucAuthData[16+1]={0}, ucIssuerScript[1024+1]={0};
	int iAuthDataLen=0, iIssuerScriptLen=0, iTemp=0;

	ucKernelType = AppGetAppType();
	ucOnlineResult = g_tTransParam.ucOnlineResult[0];

	//	memcpy(aucRspCode, g_tTransParam.ucRspCode, 2);
	iAuthDataLen = g_tTransParam.ucAuthDataLen[0] * 256 + g_tTransParam.ucAuthDataLen[1];
	memcpy(ucAuthData, g_tTransParam.ucAuthData, iAuthDataLen);
	iIssuerScriptLen = g_tTransParam.ucIssuerScriptLen[0] * 256 + g_tTransParam.ucIssuerScriptLen[1];
	memcpy(ucIssuerScript, g_tTransParam.ucIssuerScript, iIssuerScriptLen);

	/*
	memset(ucTemp, 0, sizeof(ucTemp));
	iTemp = 0;
	if( Clss_GetTLVData(0x91, ucTemp, &iTemp, ucKernelType) == EMV_OK ) {
	memcpy(ucAuthData, ucTemp, MIN(iTemp, 16));
	iAuthDataLen = MIN(iTemp, 16);
	}

	memset(ucTemp, 0, sizeof(ucTemp));
	iTemp = 0;
	if( Clss_GetTLVData(0x71, ucTemp, &iTemp, ucKernelType) == EMV_OK ) {
	ucIssuerScript[iIssuerScriptLen++] = (uchar)0x71;
	if( iTemp > 127 )
	{
	ucIssuerScript[iIssuerScriptLen++] = 0x81;
	}
	ucIssuerScript[iIssuerScriptLen++] = (uchar)iTemp;
	memcpy(&ucIssuerScript[iIssuerScriptLen], ucTemp, iTemp);
	iIssuerScriptLen += iTemp;
	}

	memset(ucTemp, 0, sizeof(ucTemp));
	iTemp = 0;
	if( Clss_GetTLVData(0x72, ucTemp, &iTemp, ucKernelType) == EMV_OK ) {
	ucIssuerScript[iIssuerScriptLen++] = (uchar)0x72;
	if( iTemp > 127 )
	{
	ucIssuerScript[iIssuerScriptLen++] = 0x81;
	}
	ucIssuerScript[iIssuerScriptLen++] = (uchar)iTemp;
	memcpy(&ucIssuerScript[iIssuerScriptLen], ucTemp, iTemp);
	iIssuerScriptLen += iTemp;
	}
	*/

	if ( ucKernelType == KERNTYPE_VIS ) 
	{
		iRet = ClssCompleteTrans_WAVE(ucOnlineResult, ucAuthData, iAuthDataLen, ucIssuerScript, iIssuerScriptLen);
	}

	AppRemovePicc();

	return iRet;

}


static int ClssCompleteTrans_WAVE(uchar ucInOnlineResult, uchar aucIAuthData[], int nIAuthDataLen,  uchar aucScript[], int nScriptLen)
{
	uchar aucBuff[256] = {0};
	uchar aucCTQ[32] = {0};
	int iCTQLen = 0, iLen = 0, iRet = 0;
	uchar ucKernType = 0;

	if ( (nIAuthDataLen == 0) && (nScriptLen == 0) ) 
	{
		return EMV_NO_DATA;
	}

	if ( ucInOnlineResult != ONLINE_APPROVE ) 
	{
		return -1;
	}

#ifdef EUI_
	EShowMsgBox(EL_UI_PAGE_TRANS_TAP_AGAIN, NULL, 0);
#else
	ScrCls();
	PubDispString("PLS TAP AGAIN", 0|DISP_LINE_LEFT);
#endif
	iRet = ClssDetectTapCard();
	if ( iRet ) 
	{
		return iRet;
	}

	memset(aucCTQ, 0, sizeof(aucCTQ));

	if ((sg_tClssPreProcInfo.aucReaderTTQ[2] & 0x80) == 0x80
		&& Clss_GetTLVData_Wave(0x9F6C, aucCTQ, &iCTQLen) == 0
		&& (aucCTQ[1] & 0x40) == 0x40)
	{
		memset(aucBuff, 0, sizeof(aucBuff));
		iRet = Clss_FinalSelect_Entry(&ucKernType, aucBuff, &iLen);
		if (iRet != 0)
		{
			return iRet;
		}

		iRet = Clss_IssuerAuth_Wave(aucIAuthData, nIAuthDataLen);
		if(iRet)
		{
			return iRet;
		}
		iRet = Clss_IssScriptProc_Wave(aucScript, nScriptLen);
		if(iRet)
		{
			return iRet;
		}


	}
	return EMV_OK;
}

int ClssDetectTapCard(void)
{
	int iRet = 0, iTime = 0;
	uchar ucKey = 0;

	//tap card
	iRet = PiccOpen();
	if(iRet != 0)
	{
#ifdef EUI_
		EShowMsgBox(EL_UI_PAGE_TRANS_OPEN_PICC_ERR, NULL, 0);
#else
		ScrCls();
		PubDispString("OPEN PICC ERR", 0|DISP_LINE_LEFT);
#endif
		// ==== Modified by ZhangYuan 2016-10-17 ======
		//	getkey();	
		PubWaitKey(10);
		// ==== end ==============================

		return iRet;
	}
	TimerSet(3,500);
	kbflush();
	while(1)
	{
		iTime = TimerCheck(3);
		if(!iTime)
		{
			return -1;
		}

		if(kbhit() != NOKEY)
		{
			ucKey = getkey();
			if(ucKey == KEYCANCEL)
			{
				return EMV_USER_CANCEL;
			}
		}
		else	//modified by kevinliu 2015/12/09 It's hard to cancel before modified.
		{
			iRet = PiccDetect(0, NULL, NULL, NULL, NULL);
			if(iRet == 0)
				break;
			else if(iRet == 3|| iRet==5 || iRet==6)
			{
				DelayMs(100);
				continue;
			}
			else if(iRet == 4)
			{
#ifdef EUI_
				EShowMsgBox(EL_UI_PAGE_TRANS_TOO_MANY_CARDS, NULL, 0);
#else
				ScrCls();
				PubDispString("TOO MANY CARDS", 0|DISP_LINE_LEFT);
#endif
				// ==== Modified by ZhangYuan 2016-10-17 ======
				//	getkey();	
				Beep();
				DelayMs(100);
				// ==== end ==============================
				return -1;
			}
			else// if(iRet == 1 || iRet==2)
			{
				return -1;
			}
		}
	}
	return 0;
}


//EMV contactless card process
int ClssStartTransaction(void)
{
	int i=0, iRet=0, iLen=0, iDataOut=0;
	unsigned char ucAcType=0, ucMSDPath=0;
	unsigned char ucTemp[1024]={0}, ucTemp2[1024]={0}, ucPAN[19+1]={0}, ucExpDate[4+1]={0};

	ClssTransInit();	//removed by Kevin Liu 20161129

	//prepare transaction parameter
	iRet = ClssPreProcTxnParam();
	EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "ClssPreProcTxnParam = %d \n", iRet);
	if ( iRet != EMV_OK ) {
		AppRemovePicc();
		ComPoseInfoMsg(ClssProc_RetToString, ELMap2AppErrCode(iRet), iRet, __LINE__);
//		return EL_TRANS_RET_BASE - iRet;
		return ELMap2AppErrCode(iRet);
	}

	while ( 1 ) {
		//contactless transaction flow
		iRet = ClssProcFlow_ALL();
		EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "ClssProcFlow_ALL = %d \n", iRet);
		if ( iRet )
		{
			if ( iRet == App_Try_Again ) 
			{
				DelayMs(1200);
				//				continue;
				return App_Try_Again; // by ZhangYuan 2017-03-22
			}
			else if( iRet == CLSS_USE_CONTACT)
			{
				Inter_DisplayMsg(MSG_TRY_ANOTHER_INTERFACE);
				return CLSS_USE_CONTACT;
			}
			AppRemovePicc();
			ComPoseInfoMsg(ClssProc_RetToString, ELMap2AppErrCode(iRet), iRet, __LINE__);
//			return EL_TRANS_RET_BASE - iRet;
			return ELMap2AppErrCode(iRet);
		}

		ucAcType = AppGetTransACType();
		SetConfigurationData(CMD_TXN_CARD_PROCESSING_RESULT, &ucAcType, 1);		// Added by ZhangYuan 2017-04-17

		//AC TYPE
		if ( (iRet == 0) && (AppGetTransACType() == AC_AAC) && (AppGetAppType() == KERNTYPE_VIS) ) 
		{
//			iRet = EL_TRANS_RET_BASE - EMV_DENIAL;
			iRet = ELMap2AppErrCode(EMV_DENIAL);
		}
		//create output
		iRet = AppConv_CreateOutCome(iRet, ucAcType, &sg_tOutComeData);
		EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "AppConv_CreateOutCome = %d, ACType=%02x, CVMType=%02x \n", iRet, ucAcType, sg_tOutComeData.ucCVMType);
		// === Added by ZhangYuan 2016-12-21, CVM Type ========
		if (sg_tOutComeData.ucCVMType == CLSS_CVM_NO)
			sg_tOutComeData.ucCVMType = 0x00;
		else if (sg_tOutComeData.ucCVMType == CLSS_CVM_SIG)
			sg_tOutComeData.ucCVMType = 0x01;
		else if (sg_tOutComeData.ucCVMType == CLSS_CVM_ONLINE_PIN)
			sg_tOutComeData.ucCVMType = 0x02;
		else if (sg_tOutComeData.ucCVMType == CLSS_CVM_OFFLINE_PIN)
			sg_tOutComeData.ucCVMType = 0x03;
		else if (sg_tOutComeData.ucCVMType == CLSS_CVM_CONSUMER_DEVICE)
			sg_tOutComeData.ucCVMType = 0x04;	
		else
			sg_tOutComeData.ucCVMType = 0x00;
		SetConfigurationData(CMD_TXN_CVM_TYPE, &sg_tOutComeData.ucCVMType, 1);	
		// ==== Added end ===================================

		if ( iRet ) 
		{
			if ( iRet == App_Try_Again ) 
			{
				DelayMs(1200);
				//				continue;
				return App_Try_Again; // by ZhangYuan 2017-03-22
			}
			else if( iRet == CLSS_USE_CONTACT)
			{
				Inter_DisplayMsg(MSG_TRY_ANOTHER_INTERFACE);
				return CLSS_USE_CONTACT;
			}
			AppRemovePicc();
			ComPoseInfoMsg(ClssProc_RetToString, iRet = ELMap2AppErrCode(iRet), iRet, __LINE__);
//			return EL_TRANS_RET_BASE - iRet;
			return  ELMap2AppErrCode(iRet);
		}
		else 
		{
			break;
		}
	}


	//process data
	if(g_tTransParam.ucCurrentCLSSType[0] == KERNTYPE_VIS)
	{
		//get MSD and Wave2 track 1 data(ASCII)
		Clss_nGetTrack1MapData_Wave (ucTemp, &iDataOut);
		SetConfigurationData(CMD_TXN_TRACK_ONE_DATA, ucTemp, iDataOut);

		if(g_tTransParam.ucCurrentPathType[0]== CLSS_VISA_MSD)
		{
			Clss_GetMSDType_Wave(&ucMSDPath);
			//get MSD track 2 data
			Clss_nGetTrack2MapData_Wave (ucTemp, &iDataOut);
			SetConfigurationData(CMD_TXN_TRACK_TWO_DATA, ucTemp, iDataOut);
		}
		//chip or MSD without trk2map data
		if(strlen((char *)g_tTransParam.ucTrackTwoData) == 0)
		{
			//get track 2 data from ICC
			iDataOut = 0;
			memset(ucTemp, 0, sizeof(ucTemp));
			iRet = Clss_GetTLVData_Wave(0x57, ucTemp, &iDataOut);
			if(iRet == EMV_OK)
			{
				PubBcd2Asc0(ucTemp, iDataOut, ucTemp2);
				iDataOut = iDataOut*2;
				ucTemp2[iDataOut] = '\0';
				SetConfigurationData(CMD_TXN_TRACK_TWO_DATA, ucTemp2, iDataOut);
			}
		}
	}
	else if(g_tTransParam.ucCurrentCLSSType[0] == KERNTYPE_MC)
	{
		iDataOut = 0;
		memset(ucTemp, 0, sizeof(ucTemp));
		//get track 1 data only for paypass
		//		iRet = Clss_GetTLVData(0x56 , glProcInfo.szTrack1, &iLen, KERNTYPE_MC);
		iRet = Clss_GetTLVDataList_MC("\x56", 1, sizeof(ucTemp), ucTemp, &iDataOut);
		SetConfigurationData(CMD_TXN_TRACK_ONE_DATA, ucTemp, iDataOut);

		//get track 2 data for paypass
		iDataOut = 0;
		memset(ucTemp, 0, sizeof(ucTemp));
		if (g_tTransParam.ucCurrentPathType[0] == CLSS_MC_MAG)
		{
			//			iRet = Clss_GetTLVData(0x9F6B, ucTemp, &iDataOut, KERNTYPE_MC);
			iRet = Clss_GetTLVDataList_MC("\x9f\x6b", 2, sizeof(ucTemp), ucTemp, &iDataOut);
		}
		else if (g_tTransParam.ucCurrentPathType[0] == CLSS_MC_MCHIP)
		{
			//			iRet = Clss_GetTLVData(0x57, ucTemp, &iDataOut, KERNTYPE_MC);
			iRet = Clss_GetTLVDataList_MC("\x57", 1, sizeof(ucTemp), ucTemp, &iDataOut);
		}
		if(iRet == EMV_OK)
		{
			PubBcd2Asc0(ucTemp, iDataOut, ucTemp2);
			iDataOut = iDataOut*2;
			ucTemp2[iDataOut] = '\0';
			SetConfigurationData(CMD_TXN_TRACK_TWO_DATA, ucTemp2, iDataOut);
		}
	}

	for(i=0; i<iDataOut; i++)
	{
		if(g_tTransParam.ucTrackTwoData[i] == 'D')
		{
			g_tTransParam.ucTrackTwoData[i] = '=';
			break;
		}
	}

	GetPanFromTrack(g_tTransParam.ucTrackTwoData, g_tTransParam.ucTrackThreeData, ucPAN, ucExpDate);
	memcpy(g_tTransParam.ucPAN, ucPAN, strlen(ucPAN));
	memcpy(g_tTransParam.ucExpDate, ucExpDate, strlen(ucExpDate));

	SetConfigurationData(CMD_TXN_EXPIRE_DATE, ucExpDate, strlen(ucExpDate));	// Added by ZhangYuan 2016-12-19

	EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "ucPAN = %s\n", ucPAN);

	return EMV_OK;
}

#endif   //#ifndef _WIN32

