
#include "..\inc\EmvApi.h"
#include "..\inc\L2_Device.h"

#ifndef NULL
#define NULL 0
#endif

#define EMV_API_ID_EMVCoreInit             0x01
#define EMV_API_ID_EMVGetParameter         0x02
#define EMV_API_ID_EMVSetParameter         0x03
#define EMV_API_ID_EMVGetScriptResult      0x06
#define EMV_API_ID_EMVReadVerInfo          0x07
#define EMV_API_ID_EMVSetScriptProcMethod  0x08
#define EMV_API_ID_EMVGetParamFlag         0x09
#define EMV_API_ID_EMVGetMCKParam          0x0A
#define EMV_API_ID_EMVSetMCKParam          0x0B
#define EMV_API_ID_EMVSetConfigFlag        0x0C
#define EMV_API_ID_EMVSetAmount            0x0D
#define EMV_API_ID_EMVGetFinalAppPara      0x0E
#define EMV_API_ID_EMVModFinalAppPara      0x0F
#define EMV_API_ID_EMVGetLabelList         0x10
#define EMV_API_ID_EMVAddCAPK              0x21
#define EMV_API_ID_EMVGetCAPK              0x22
#define EMV_API_ID_EMVDelAllCAPK           0x23
#define EMV_API_ID_EMVDelAllApp            0x26
#define EMV_API_ID_EMVAddRevocList		   0x27
#define EMV_API_ID_EMVDelRevocList         0x28
#define EMV_API_ID_EMVDelAllRevocList      0x29
#define EMV_API_ID_EMVAppSelect            0x31
#define EMV_API_ID_EMVReadAppData          0x32
#define EMV_API_ID_EMVCardAuth             0x33
#define EMV_API_ID_EMVStartTrans           0x34
#define EMV_API_ID_EMVCompleteTrans        0x35
#define EMV_API_ID_EMVInitTLVData          0x51
#define EMV_API_ID_EMVSwitchClss           0x52

#define  MAX_API_CALLED_LOG_NUM 41
static unsigned char gl_ucPinInput = 0; //cEMVGetHolderPwd has been called or not? 0-no; 1-yes
static unsigned char gl_nEnableConfirmAmt  = 0;
static int gl_nInputAmtFlag = 0;//transaction amount has been input or not? 0-no; 1-yes
static unsigned char gl_aucAPICalledLog[MAX_API_CALLED_LOG_NUM];// record the order of each API called.
static unsigned char gl_ucAPICalledLogLen = 0;
static unsigned char gl_ucEmvPciUsedFlg = 1;// D180 shall be PCI mode default [7/21/2016 zhoujie]
static unsigned long gl_ulAuthAmt = 0, gl_ulCashBackAmt = 0;

#ifndef _WIN32

int EMV_II_GetTLVData(unsigned int unTag, unsigned int unExpectOutlen, unsigned char *pucDataOut, unsigned int *punOutLen );
int EMV_II_SetTLVData(unsigned int unTag, unsigned char *pucData, unsigned int unLen);
int EMV_II_DelAidList(unsigned char  *pucAID, unsigned char ucAidLen);
int EMV_II_DelCAPK(unsigned char KeyID, unsigned char *RID);
int EMV_II_DelRevocList(unsigned char ucIndex, unsigned char *pucRID);
int EMV_II_DelAppFromCandList(int nAppIndex, EMV_II_CANDLIST *ptCandList, int *pnAppNum);
int EMV_II_GetCandList(EMV_II_CANDLIST *ptCandList, int *pnAppNum);
int EMV_II_GetLogItemChild(unsigned char ucLogType, unsigned short Tag, unsigned char *TagData, int *TagLen);
int EMV_II_GetLogData(unsigned char *paucLogDataOut, int *pnLenOut);
int EMV_II_InitTransLog(void);
int EMV_II_GetVerifyICCStatus(unsigned char *pucSWA, unsigned char *pucSWB);
int EMV_II_GetScriptResult(unsigned char *Result, int *RetLen);
void EMV_II_SetAutoRunFlag(unsigned char ucAutoRunFlag);

void LongToStr_emv(unsigned long ldat, unsigned char *str)
{
	if (str == NULL)
	{
		return;
	}
	
    str[0] = (unsigned char)(ldat >> 24);
    str[1] = (unsigned char)(ldat >> 16);
    str[2] = (unsigned char)(ldat >> 8);
    str[3] = (unsigned char)(ldat);
}

void OneTwo_emv(unsigned char *One,unsigned short len,unsigned char *Two)
{
    unsigned char  i;
    static unsigned char TAB[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
	
	if (One == NULL || Two == NULL)
	{
		return;
	}
	
    for (i = 0; i < len; i++) 
	{
        Two[i * 2] = TAB[One[i] >> 4];
        Two[i * 2 + 1] = TAB[One[i] & 0x0f];
    }
}

static int Toupper_emv(int nIn)
{
	int nOut=0; 
	
	if (nIn >= 'a' && nIn <= 'z')
	{ 
		nOut = nIn - ('a' - 'A');
	}
	else
	{
		nOut = nIn;
	}
	
    return nOut;	
}

void TwoOne_emv(unsigned char *in, unsigned short in_len, unsigned char *out)
{
	unsigned char tmp;
	unsigned short i;
	
	if (in == NULL || out == NULL)
	{
		return;
	}
	
	for (i = 0; i < in_len; i += 2) 
	{
		tmp = in[i];
		if (tmp > '9')
			tmp = Toupper_emv(tmp) - ('A' - 0x0A);
		else
			tmp &= 0x0f;
		tmp <<= 4;
		out[i / 2] = tmp;
		
		tmp = in[i + 1];
		if (tmp > '9')
			tmp = Toupper_emv(tmp) - ('A' - 0x0A);
		else
			tmp &= 0x0f;
		out[i / 2] += tmp;
	}
}

unsigned char * StringSum_emv(unsigned char * strIn1, unsigned char * strIn2, unsigned char * strOut, int len, int hexFlag)
{
	unsigned char szTmp[20] = {0};
	unsigned char szAmt1[20] = {0};
	unsigned char szAmt2[20] = {0};

	int i, actLen;
	unsigned char a, b;
	int flag = 0;

	if (strIn1 == NULL || strIn2 == NULL || strOut == NULL || len <= 0)
		return NULL;

	actLen = len;

	if (hexFlag)
	{
		OneTwo_emv(strIn1, (unsigned short)len, szAmt1);
		OneTwo_emv(strIn2, (unsigned short)len, szAmt2);
		actLen = len * 2;
	}
	else
	{
		memcpy(szAmt1, strIn1, len);
		memcpy(szAmt2, strIn2, len);
	}
	
	memset(szTmp, 0, sizeof(szTmp));
	for (i = actLen - 1; i >= 0; i--)
	{
		a = szAmt1[i] - 0x30;
		b = szAmt2[i] - 0x30;

		if (a + b + flag>= 10)
		{
			szTmp[i] = a + b + flag - 10 + 0x30;
			flag = 1;
		}
		else
		{
			szTmp[i] = a + b + flag + 0x30;
			flag = 0;
		}
	}

	if (hexFlag)
	{
		TwoOne_emv(szTmp, (unsigned short)actLen, strOut);
	}
	else
	{
		memcpy(strOut, szTmp, actLen);
		strOut[actLen] = '\0';
	}
	
	return strOut;
}

unsigned char *SearchTLV_emv(int DolList, unsigned short Tag, unsigned char *dat, unsigned char *datend, int *TagLen)
{
    int i, j, len, tTag;

	if (dat == NULL || datend == NULL)
	{
		return NULL;
	}

    while (dat < datend) 
	{
        tTag = *dat++;
        if (tTag == 0xFF || tTag == 0x00)
			continue;
        if ((tTag & 0x1F) == 0x1F)
		{
            tTag <<= 8;
            tTag += *dat++;
            if (tTag & 0x80) 
			{
                while (dat < datend && (*dat & 0x80)) 
					dat++;
                if (dat >= datend) 
					return NULL;
                tTag = 0;
            }
        }
        if (*dat & 0x80)
		{
            i = (*dat & 0x7F);
            if (dat + i > datend) 
				return NULL;
            dat++;
            for (j = 0, len = 0; j < i; j++) 
			{
                len <<= 8;
                len += *dat++;
            }
        }
        else len = *dat++;

        if (tTag == Tag) 
		{
            if (TagLen != NULL) 
				*TagLen = len;
            return dat;
        }
        if (tTag & 0xFF00)
		{
            if (tTag & 0x2000)
				continue;
        }
        else if (tTag & 0x20)
			continue;
        if (!DolList)
			dat += len;
    }
    return NULL;
}

int EMVGetDebugInfo(int nExpAssistInfoLen, unsigned char *paucAssistInfo, int *pnErrorCode)
{
	if(pnErrorCode == NULL)
	{
		return EMV_PARAM_ERR;
	}
	EMV_II_GetDebugInfo(nExpAssistInfoLen, paucAssistInfo, pnErrorCode);
	return EMV_OK;
}

int EMVGetAPICalledLog(unsigned char *paucAPICalledLog, unsigned char *pucLogLen)
{
	if (paucAPICalledLog == NULL || pucLogLen == NULL)
	{
		return EMV_PARAM_ERR;
	}
	*pucLogLen = gl_ucAPICalledLogLen;
	if (*pucLogLen)
	{
		memcpy(paucAPICalledLog, gl_aucAPICalledLog, gl_ucAPICalledLogLen);
	}
	return EMV_OK;	
}

int SetAPICalledLog_emv(unsigned char ucAPI_ID)
{
	if (gl_ucAPICalledLogLen >= MAX_API_CALLED_LOG_NUM )
	{
		return EMV_OVERFLOW;
	}
	gl_aucAPICalledLog[gl_ucAPICalledLogLen] = ucAPI_ID;
	gl_ucAPICalledLogLen ++;

	return EMV_OK;
}

int CheckAPICalled_emv(unsigned char ucAPI_ID)
{
	int i=0;

	for (i=0;i<gl_ucAPICalledLogLen;i++)
	{
		if (gl_aucAPICalledLog[i] == ucAPI_ID)
		{
			 return 1;
		}
	}
	return 0;
}

int InitAPICalledLog_emv(void)
{
	memset(gl_aucAPICalledLog, 0, sizeof(gl_aucAPICalledLog));
	gl_ucAPICalledLogLen=0;
	return EMV_OK;
}

int EMVCoreInit(void)
{
	InitAPICalledLog_emv();
	EMV_II_CoreInit();
	SetAPICalledLog_emv(EMV_API_ID_EMVCoreInit);
    return 0;
}

void EMVGetParameter(EMV_PARAM *Param)
{
	EMV_II_TERMPARAM tTermParam;

	SetAPICalledLog_emv(EMV_API_ID_EMVGetParameter);
	if (Param == NULL)
	{
		return;
	}
	memset(&tTermParam, 0, sizeof(EMV_II_TERMPARAM));
	EMV_II_GetTermParam(&tTermParam);
	memcpy(Param->MerchName, tTermParam.aucMerchName, sizeof(tTermParam.aucMerchName));
	memcpy(Param->MerchCateCode, tTermParam.aucMerchCateCode, sizeof(tTermParam.aucMerchCateCode));
	memcpy(Param->MerchId, tTermParam.aucMerchId, sizeof(tTermParam.aucMerchId));
	memcpy(Param->TermId, tTermParam.aucTermId, sizeof(tTermParam.aucTermId));
	Param->TerminalType = tTermParam.ucTerminalType;
	memcpy(Param->Capability, tTermParam.aucCapability, sizeof(tTermParam.aucCapability));
	memcpy(Param->ExCapability, tTermParam.aucExCapability, sizeof(tTermParam.aucExCapability));
	Param->TransCurrExp = tTermParam.ucTransCurrExp;
	Param->ReferCurrExp = tTermParam.ucReferCurrExp;
	memcpy(Param->ReferCurrCode, tTermParam.aucReferCurrCode, sizeof(tTermParam.aucReferCurrCode));
	memcpy(Param->CountryCode, tTermParam.aucCountryCode, sizeof(tTermParam.aucCountryCode));
	memcpy(Param->TransCurrCode, tTermParam.aucTransCurrCode, sizeof(tTermParam.aucTransCurrCode));
	Param->ReferCurrCon = tTermParam.ulReferCurrCon;
	Param->TransType = tTermParam.ucTransType;
	Param->ForceOnline = tTermParam.ucForceOnline;
	Param->GetDataPIN = tTermParam.ucGetDataPIN;
	Param->SurportPSESel = tTermParam.ucSurportPSESel;
}

void EMVSetParameter(EMV_PARAM *Param)
{
	EMV_II_TERMPARAM tTermParam;

	SetAPICalledLog_emv(EMV_API_ID_EMVSetParameter);
	if (Param == NULL)
	{
		return;
	}
	memset(&tTermParam, 0, sizeof(EMV_II_TERMPARAM));
	EMV_II_GetTermParam(&tTermParam);

	memcpy(tTermParam.aucMerchName, Param->MerchName, sizeof(tTermParam.aucMerchName));
	memcpy(tTermParam.aucMerchCateCode, Param->MerchCateCode, sizeof(tTermParam.aucMerchCateCode));
	memcpy(tTermParam.aucMerchId, Param->MerchId, sizeof(tTermParam.aucMerchId));
	memcpy(tTermParam.aucTermId, Param->TermId, sizeof(tTermParam.aucTermId));
	tTermParam.ucTerminalType = Param->TerminalType;
	memcpy(tTermParam.aucCapability, Param->Capability, sizeof(tTermParam.aucCapability));
	memcpy(tTermParam.aucExCapability, Param->ExCapability, sizeof(tTermParam.aucExCapability));
	tTermParam.ucTransCurrExp = Param->TransCurrExp;
	tTermParam.ucReferCurrExp = Param->ReferCurrExp;
	memcpy(tTermParam.aucReferCurrCode, Param->ReferCurrCode, sizeof(tTermParam.aucReferCurrCode));
	memcpy(tTermParam.aucCountryCode, Param->CountryCode, sizeof(tTermParam.aucCountryCode));
	memcpy(tTermParam.aucTransCurrCode, Param->TransCurrCode, sizeof(tTermParam.aucTransCurrCode));
	tTermParam.ulReferCurrCon = Param->ReferCurrCon;
	tTermParam.ucTransType = Param->TransType;
	tTermParam.ucForceOnline = Param->ForceOnline;
	tTermParam.ucGetDataPIN = Param->GetDataPIN;
	tTermParam.ucSurportPSESel = Param->SurportPSESel;

	EMV_II_SetTermParam(&tTermParam);
}

int EMVGetTLVData(unsigned short Tag, unsigned char *DataOut, int *OutLen)
{	
	if (DataOut == NULL || OutLen == NULL)
	{
		return EMV_PARAM_ERR;
	}

	if (!CheckAPICalled_emv(EMV_API_ID_EMVCoreInit))
	{// EMVCoreInit has not been called.
		return EMV_DATA_ERR;
	}

	return EMV_II_GetTLVData((unsigned int)Tag, 0xff, DataOut, (unsigned int *)OutLen);
}

int EMVSetTLVData(unsigned short Tag, unsigned char *Data, int len)
{
	if (Data == NULL)
	{
		return EMV_DATA_ERR;
	}

	if (!CheckAPICalled_emv(EMV_API_ID_EMVCoreInit))
	{// EMVCoreInit has not been called.
		return EMV_DATA_ERR;
	}

	return EMV_II_SetTLVData((unsigned int)Tag, Data, (unsigned int)len);
}

int EMVGetScriptResult(unsigned char *Result, int *RetLen)
{
	SetAPICalledLog_emv(EMV_API_ID_EMVGetScriptResult);
	if (Result==NULL || RetLen==NULL)
	{
		return EMV_PARAM_ERR;
	}
	return EMV_II_GetScriptResult(Result, RetLen);
}

int EMVAddApp(EMV_APPLIST *App)
{
	int nRet =0;

	if (App == NULL)
	{
		return EMV_PARAM_ERR;
	}

	nRet=EMV_II_AddAidList(App->AID,App->AidLen,App->SelFlag);
	if (nRet)
	{
		return nRet;
	}
	return EMV_OK;
}

int EMVGetApp(int Index, EMV_APPLIST *App)
{
	int nRet =0;
	unsigned char aucAID[17];
	unsigned char ucAidLen=0,ucSelFlg=0;

	if (App == NULL)
	{
		return EMV_PARAM_ERR;
	}

	memset(App,0,sizeof(EMV_APPLIST));
	memset(aucAID,0,sizeof(aucAID));

	nRet=EMV_II_GetAidList(Index,aucAID,&ucAidLen,&ucSelFlg);
	if(nRet)
	{
		return nRet;
	}
	memcpy(App->AID,aucAID,ucAidLen);
	App->AidLen=ucAidLen;
	App->SelFlag=ucSelFlg;
	return EMV_OK;
}

int EMVDelAllApp(void)
{
	SetAPICalledLog_emv(EMV_API_ID_EMVDelAllApp);
	EMV_II_DelAllAidList();
	return EMV_OK;
}

int  EMVModFinalAppPara(EMV_APPLIST *ptEMV_APP)
{
	EMV_II_AIDPARAM tAidParam;

	SetAPICalledLog_emv(EMV_API_ID_EMVModFinalAppPara);
	if (ptEMV_APP == NULL)
	{
		return EMV_PARAM_ERR;
	}
	memset(&tAidParam, 0, sizeof(EMV_II_AIDPARAM));
	tAidParam.ucTargetPer = ptEMV_APP->TargetPer;
	tAidParam.ucMaxTargetPer = ptEMV_APP->MaxTargetPer;
	tAidParam.ucFloorLimitCheck = ptEMV_APP->FloorLimitCheck;
	tAidParam.ucRandTransSel = ptEMV_APP->RandTransSel;
	tAidParam.ucVelocityCheck = ptEMV_APP->VelocityCheck;
	tAidParam.ulFloorLimit = ptEMV_APP->FloorLimit;
	tAidParam.ulThreshold = ptEMV_APP->Threshold;
	memcpy(tAidParam.aucTACDenial, ptEMV_APP->TACDenial, sizeof(tAidParam.aucTACDenial));
	memcpy(tAidParam.aucTACOnline, ptEMV_APP->TACOnline, sizeof(tAidParam.aucTACOnline));
	memcpy(tAidParam.aucTACDefault, ptEMV_APP->TACDefault, sizeof(tAidParam.aucTACDefault));
	memcpy(tAidParam.aucAcquierId, ptEMV_APP->AcquierId, sizeof(tAidParam.aucAcquierId));
	memcpy(tAidParam.aucdDOL, ptEMV_APP->dDOL, sizeof(tAidParam.aucdDOL));
	memcpy(tAidParam.auctDOL, ptEMV_APP->tDOL, sizeof(tAidParam.auctDOL));
	memcpy(tAidParam.aucVersion, ptEMV_APP->Version, sizeof(tAidParam.aucVersion));
	memcpy(tAidParam.aucRiskManData, ptEMV_APP->RiskManData, sizeof(tAidParam.aucRiskManData));
	
	return  EMV_II_SetAidParam(&tAidParam); 
}

int  EMVGetFinalAppPara(EMV_APPLIST *ptEMV_APP)
{
	EMV_II_AIDPARAM tAidParam;
	int nRet=0;

	SetAPICalledLog_emv(EMV_API_ID_EMVGetFinalAppPara);
	if (ptEMV_APP == NULL)
	{
		return EMV_PARAM_ERR;
	}
	
	memset(&tAidParam, 0, sizeof(EMV_II_AIDPARAM));
	nRet = EMV_II_GetAidParam(&tAidParam);
	if(nRet)
	{
		return nRet;
	}
	
	ptEMV_APP->TargetPer = tAidParam.ucTargetPer;
	ptEMV_APP->MaxTargetPer = tAidParam.ucMaxTargetPer;
	ptEMV_APP->FloorLimitCheck = tAidParam.ucFloorLimitCheck;
	ptEMV_APP->RandTransSel = tAidParam.ucRandTransSel;
	ptEMV_APP->VelocityCheck = tAidParam.ucVelocityCheck;
	ptEMV_APP->FloorLimit = tAidParam.ulFloorLimit;
	ptEMV_APP->Threshold = tAidParam.ulThreshold;
	memcpy(ptEMV_APP->TACDenial, tAidParam.aucTACDenial, sizeof(tAidParam.aucTACDenial));
	memcpy(ptEMV_APP->TACOnline, tAidParam.aucTACOnline, sizeof(tAidParam.aucTACOnline));
	memcpy(ptEMV_APP->TACDefault, tAidParam.aucTACDefault, sizeof(tAidParam.aucTACDefault));
	memcpy(ptEMV_APP->AcquierId, tAidParam.aucAcquierId, sizeof(tAidParam.aucAcquierId));
	memcpy(ptEMV_APP->dDOL, tAidParam.aucdDOL, sizeof(tAidParam.aucdDOL));
	memcpy(ptEMV_APP->tDOL, tAidParam.auctDOL, sizeof(tAidParam.auctDOL));
	memcpy(ptEMV_APP->Version, tAidParam.aucVersion, sizeof(tAidParam.aucVersion));
	memcpy(ptEMV_APP->RiskManData, tAidParam.aucRiskManData, sizeof(tAidParam.aucRiskManData));
	return EMV_OK;
}

int  EMVGetLabelList(APPLABEL_LIST  *ptAppLabel, int *pnAppNum)
{
	int nRet=0, i=0;
	EMV_II_CANDLIST tCandAppList[MAX_APP_ITEMS];

	SetAPICalledLog_emv(EMV_API_ID_EMVGetLabelList);
	if (ptAppLabel == NULL || pnAppNum == NULL )
	{
		return EMV_PARAM_ERR;
	}

	memset(tCandAppList, 0, MAX_APP_ITEMS*sizeof(EMV_II_CANDLIST));
	nRet = EMV_II_GetCandList(tCandAppList, pnAppNum);
	if (nRet)
	{
		return nRet;
	}

	if (*pnAppNum)
	{
		for (i=0; i < (*pnAppNum); i++)
		{
			memcpy(ptAppLabel[i].aucAppPreName, tCandAppList[i].aucAppPreName, sizeof(ptAppLabel[i].aucAppPreName));
			memcpy(ptAppLabel[i].aucAppLabel, tCandAppList[i].aucAppLabel, sizeof(ptAppLabel[i].aucAppLabel));
			memcpy(ptAppLabel[i].aucIssDiscrData, tCandAppList[i].aucIssDiscrData, sizeof(ptAppLabel[i].aucIssDiscrData));
			memcpy(ptAppLabel[i].aucAID, tCandAppList[i].aucAID, sizeof(ptAppLabel[i].aucAID));
			ptAppLabel[i].ucAidLen = tCandAppList[i].ucAidLen;
		}
		return EMV_OK;
	}
	return EMV_NO_DATA;
}

int EMVAddCAPK(EMV_CAPK  *capk)
{
	
	SetAPICalledLog_emv(EMV_API_ID_EMVAddCAPK);
	if (capk == NULL)
	{
		return EMV_PARAM_ERR;
	}
	
	return EMV_II_AddCAPK((EMV_II_CAPK *)capk);
	   
}

int  EMVGetCAPK(int Index, EMV_CAPK  *capk)
{
    EMV_CAPK stCapk;
    int nRet;
    
	SetAPICalledLog_emv(EMV_API_ID_EMVGetCAPK);
	if (capk == NULL)
	{
		return EMV_PARAM_ERR;
	}
    memcpy(&stCapk, capk, sizeof(stCapk));
	nRet = EMV_II_GetCAPK(Index, (EMV_II_CAPK *)&stCapk);
    memcpy(capk, &stCapk, sizeof(stCapk));
    return nRet;
}

int  EMVDelAllCAPK(void)// add [3/25/2014 ZhouJie]
{
	SetAPICalledLog_emv(EMV_API_ID_EMVDelAllCAPK);
	EMV_II_DelAllCAPK();
	return EMV_OK;
}

int  EMVAddRevocList(EMV_REVOCLIST *pRevocList)
{
    EMV_II_REVOCLIST stRevocList;
    int nRet;
    
	SetAPICalledLog_emv(EMV_API_ID_EMVAddRevocList);
	if (pRevocList == NULL)
	{
		return EMV_PARAM_ERR;
	}
    memset(&stRevocList, 0 ,sizeof(stRevocList));
    memcpy(stRevocList.aucRid, pRevocList->ucRid, sizeof(pRevocList->ucRid));
    stRevocList.ucIndex = pRevocList->ucIndex;
    memcpy(stRevocList.aucCertSn, pRevocList->ucCertSn, sizeof(pRevocList->ucCertSn));
    
	nRet = EMV_II_AddRevocList(&stRevocList);
    
    memcpy(pRevocList->ucRid, stRevocList.aucRid, sizeof(pRevocList->ucRid));
    pRevocList->ucIndex = stRevocList.ucIndex;
    memcpy(pRevocList->ucCertSn, stRevocList.aucCertSn, sizeof(pRevocList->ucCertSn));    
    return nRet;
}

void EMVDelAllRevocList(void)
{
	SetAPICalledLog_emv(EMV_API_ID_EMVDelAllRevocList);
	EMV_II_DelAllRevocList();
	return;
}

int SaveAmtToEMVII_emv(unsigned char * paucAuthAmount, unsigned char * paucCashBackAmount)
{
	unsigned char aucStrAmtSum[20] = {0};
	unsigned char aucNumAmtSum[20] = {0};
	unsigned char aucStrAmtOther[20] = {0};
	unsigned char aucNumAmtOther[20] = {0};
	unsigned char i=0;
	unsigned char ucTransType=0;
	int nLen, nRet=0;
	unsigned long ulAuthAmt=0,ulCashBackAmt=0; 

	if (paucAuthAmount == NULL)
	{
		return EMV_DATA_ERR;
	}

	EMV_II_GetTLVData(0x9C, 1, &ucTransType, &nLen);
	if(ucTransType == 0x09)//cashback
	{
		if(paucCashBackAmount == NULL)
		{
			return EMV_DATA_ERR;
		}
		memcpy(aucNumAmtOther, paucCashBackAmount, 6);
		OneTwo_emv(aucNumAmtOther, 6, aucStrAmtOther);

		StringSum_emv(paucAuthAmount, paucCashBackAmount, aucNumAmtSum, 6, 1);

		if ((memcmp(aucNumAmtSum, paucAuthAmount, 6) < 0) || (memcmp(aucNumAmtSum, paucCashBackAmount, 6) < 0))
		{
			return EMV_DATA_ERR;
		}
		OneTwo_emv(aucNumAmtSum, 6, aucStrAmtSum);
	}
	else
	{
		memcpy(aucNumAmtSum, paucAuthAmount, 6);
		OneTwo_emv(aucNumAmtSum, 6, aucStrAmtSum);
	}

	//check if the Amount is right
	for(i=0; i<12; i++)
	{
		if(aucStrAmtSum[i] > '9')
		{
			return EMV_DATA_ERR;
		}
		if(ucTransType == 0x09)//cashback
		{
			if(aucStrAmtOther[i] > '9')
			{
				return EMV_DATA_ERR;
			}
		}
	}

	if (memcmp(aucNumAmtSum, "\x00\x42\x94\x96\x72\x95", 6) >= 0)
	{
		ulAuthAmt = 0xffffffff;
	}
	else
	{
		ulAuthAmt = atol((char*)aucStrAmtSum);
	}
	
	if(ucTransType == 0x09)//cashback
	{
		if (memcmp(aucNumAmtOther, "\x00\x42\x94\x96\x72\x95", 6) >= 0)
		{
			ulCashBackAmt = 0xffffffff;
		}
		else
		{
			ulCashBackAmt = atol((char*)aucStrAmtOther);
		}
	}

	memset(aucStrAmtSum, 0, sizeof(aucStrAmtSum));
	LongToStr_emv(ulAuthAmt, aucStrAmtSum);
	nRet=EMV_II_SetTLVData(0x81, aucStrAmtSum, 4);
	if (nRet)
	{
		return nRet;
	}
	nRet=EMV_II_SetTLVData(0x9F02, aucNumAmtSum, 6);
	if (nRet)
	{
		return nRet;
	}
	if (ucTransType == 0x09)//cashback
	{
		memset(aucStrAmtOther, 0, sizeof(aucStrAmtOther));
		LongToStr_emv(ulCashBackAmt, aucStrAmtOther);
		nRet=EMV_II_SetTLVData(0x9F04, aucStrAmtOther, 4);
		if (nRet)
		{
			return nRet;
		}
		nRet=EMV_II_SetTLVData(0x9F03, aucNumAmtOther, 6);
		if (nRet)
		{
			return nRet;
		}
	}
	
	return EMV_OK;
}

int InputAmt_emv(void)
{
	int nRet=0,nLen=0;
    unsigned char ucTransType=0;
	unsigned char aucAuthAmt[14], aucCashBackAmt[14];
	
    nRet=EMV_II_GetTLVData(0x9C, 1, &ucTransType,(unsigned int *)&nLen);
	if(nRet)
	{
		return nRet;
	}
	
	if(ucTransType & EMV_CASHBACK)	
	{
		nRet = cEMVInputAmount(&gl_ulAuthAmt, &gl_ulCashBackAmt);
    }
    else
    {
		nRet = cEMVInputAmount(&gl_ulAuthAmt, NULL);
    }
	
    if (nRet)
    {
		return nRet;
    }

	gl_nInputAmtFlag=1;

	memset(aucAuthAmt, 0, sizeof(aucAuthAmt));
	memset(aucCashBackAmt, 0, sizeof(aucCashBackAmt));
	
	sprintf((char *)aucAuthAmt, "%012lu", gl_ulAuthAmt);
	TwoOne_emv(aucAuthAmt,12,aucAuthAmt);
	sprintf((char *)aucCashBackAmt, "%012lu", gl_ulCashBackAmt);
	TwoOne_emv(aucCashBackAmt,12,aucCashBackAmt);
	nRet = SaveAmtToEMVII_emv(aucAuthAmt, aucCashBackAmt);

	return nRet;
}

void ConvCandList_emv(EMV_II_CANDLIST tCandAppList, EMV_CANDLIST *ptCandAppList_old)
{
	memcpy(ptCandAppList_old->aucAID, tCandAppList.aucAID, tCandAppList.ucAidLen);
	ptCandAppList_old->ucAidLen = tCandAppList.ucAidLen;
	ptCandAppList_old->ucPriority = tCandAppList.ucPriority;
	strcpy(ptCandAppList_old->aucAppLabel, tCandAppList.aucAppLabel);
	strcpy(ptCandAppList_old->aucAppName, tCandAppList.aucAppName);
	strcpy(ptCandAppList_old->aucAppPreName, tCandAppList.aucAppPreName);
	strcpy(ptCandAppList_old->aucIssDiscrData, tCandAppList.aucIssDiscrData);
}

int  EMVAppSelectForLog(int Slot, unsigned char ucFlg)
{
	int nRet = 0, i=0;
	EMV_II_CANDLIST atCandAppList[MAX_APP_ITEMS];
	EMV_CANDLIST atCandAppList_old[MAX_APP_ITEMS];
	int nAppNum = 0;			
	int nSelTryCnt=0;			
	int nAppSelNo=0,nI=0;

	DEVICE_SetIccSlot((unsigned char)Slot);
	nRet = DEVICE_IccReset();
	if (nRet)
	{
		return 	ICC_RESET_ERR;
	}

	memset(atCandAppList, 0, sizeof(atCandAppList));
    if(ucFlg == 0) 
        ucFlg = 1;
    else
        ucFlg = 0;
    EMV_II_SetAppSelectForLog_PBOC(1, ucFlg);
	nRet = EMV_II_AppSelect(atCandAppList,&nAppNum);
	if(nRet)
	{
		return nRet;
	}
	
	nSelTryCnt=0;
	
	if ((nAppNum == 1) && (!nSelTryCnt) && !(atCandAppList[0].ucPriority & 0x80))
	{
		nRet=EMV_II_FinalSelect(0,atCandAppList, &nAppNum);
	}
	else while(1)
	{
		memset(atCandAppList_old, 0, sizeof(atCandAppList_old));
		for (i=0;i<nAppNum;i++)
		{
			ConvCandList_emv(atCandAppList[i], &atCandAppList_old[i]);
		}
		nRet = cEMVCandAppSel(nSelTryCnt, atCandAppList_old, nAppNum);
		nSelTryCnt++;
		if(nRet<0)
		{
			return nRet;
		}
		nAppSelNo=nRet;
		nRet= EMV_II_FinalSelect((unsigned char)nAppSelNo,atCandAppList, &nAppNum);
		
		if (nRet != EMV_SELECT_NEXT)
		{
			break;
		}
	}	

	if (nRet)
	{
		return nRet;
	}

	return EMV_OK;
}

int EMVAppSelect(int Slot, unsigned long TransNo) 
{
	unsigned char aucPDOL[256], ucTransType;
	int InmputAmt=0, nPDOLLen=0, nLen=0;
	int nRet =0 , nRet1 = 0;
	EMV_II_CANDLIST atCandAppList[MAX_APP_ITEMS];
	EMV_CANDLIST atCandAppList_old[MAX_APP_ITEMS];
	int nAppNum = 0, i=0;	
	int nSelTryCnt=0;		
	int nAppSelNo=0,nI=0;
	unsigned char buf[20], buff[8];
	
	SetAPICalledLog_emv(EMV_API_ID_EMVAppSelect);

	gl_nEnableConfirmAmt = 0;

	DEVICE_SetIccSlot((unsigned char)Slot);
	nRet = DEVICE_IccReset();
	if (nRet)
	{
		return 	ICC_RESET_ERR;
	}

	memset(atCandAppList, 0, sizeof(atCandAppList));
    EMV_II_SetAppSelectForLog_PBOC(0, 0);
	nRet = EMV_II_AppSelect(atCandAppList,&nAppNum);
	if(nRet)
	{
		return nRet;
	}
	
	nSelTryCnt=0;

	while(1)
	{
		if ((nAppNum == 1) && (!nSelTryCnt) && !(atCandAppList[0].ucPriority & 0x80))
		{
			nRet=EMV_II_FinalSelect(0,atCandAppList, &nAppNum);
		}
		else while(2)
		{
			memset(atCandAppList_old, 0, sizeof(atCandAppList_old));
			for (i=0;i<nAppNum;i++)
			{
				ConvCandList_emv(atCandAppList[i], &atCandAppList_old[i]);
			}
			nRet = cEMVCandAppSel(nSelTryCnt, atCandAppList_old, nAppNum);
			nSelTryCnt++;
			if(nRet<0)
			{
				return nRet;
			}
			nAppSelNo=nRet;
			nRet= EMV_II_FinalSelect((unsigned char)nAppSelNo,atCandAppList, &nAppNum);
			if (nRet != EMV_SELECT_NEXT)
			{
				break;
			}
		}//while (2)

		if (nRet)
		{
			return nRet;
		}

		nRet = cEMVSetParam(); 
		if (nRet) 
		{
			return nRet;
        }
		while (3) 
		{
			InmputAmt = 1;
			memset(aucPDOL, 0, sizeof(aucPDOL));
			EMV_II_GetTLVData(0x9F38, sizeof(aucPDOL), aucPDOL, &nPDOLLen);

			if (SearchTLV_emv(1, 0x81, aucPDOL, aucPDOL+nPDOLLen, NULL) != NULL) 
			{
				break;
			}

			if (SearchTLV_emv(1, 0x9F02, aucPDOL, aucPDOL+nPDOLLen, NULL) != NULL) 
			{
				break;
			}
		
			EMV_II_GetTLVData(0x9C, 1, &ucTransType, &nLen);
			if (ucTransType == 0x09
				&& (SearchTLV_emv(1, 0x9F03, aucPDOL, aucPDOL+nPDOLLen, NULL) == NULL
				|| SearchTLV_emv(1, 0x9F04, aucPDOL, aucPDOL+nPDOLLen, NULL) == NULL))
			{
				break;
			}
			
			InmputAmt = 0;
			break;
        }//while (3)

		gl_nInputAmtFlag=0;
		
		if (InmputAmt)
		{
			nRet = InputAmt_emv();
			if(nRet)
			{
				return nRet;
			}
		}
		
		sprintf((char *)buf, "%08lu", TransNo);
		memset(buff, 0, sizeof(buff));
		TwoOne_emv(buf, 8, buff);
		nRet = EMVSetTLVData(0x9F41, buff, 4);

		nRet = EMV_II_InitApp(atCandAppList, &nAppNum);	
		if(( nRet == EMV_APP_BLOCK )||(nRet == ICC_RSP_6985 ))
		{
            if(nAppNum == 0)
                return nRet;
			continue;
		}
		break;
	}//while (1)
	return nRet;
}

int EMVReadAppData(void)
{
	int nRet = 0;

	SetAPICalledLog_emv(EMV_API_ID_EMVReadAppData);
	EMV_II_InitTransLog();

	if (!gl_nInputAmtFlag)
	{

		nRet = InputAmt_emv();
		if (nRet)
		{
			return nRet;
		}	
	}
	return EMV_II_ReadAppData();
}


int EMVCardAuth(void)
{
	SetAPICalledLog_emv(EMV_API_ID_EMVCardAuth);
	return EMV_II_CardAuth();
}

int HolderVerify_emv(void)
{
	int nRet =0,nPwdRst=0;
	unsigned char ucCVMType=0,ucPINCnt=0;
	unsigned char aucPINData[30];
	unsigned char aucPINForamt[30];	
	unsigned char ucPINTryCount =0;	
	int nTryFlg=0; 	
	
	unsigned char ucBypassedOne=0;

	EMV_EXTMPARAM tmpExTmParam;	
	EMV_MCKPARAM tMCKParam;

	memset(&tMCKParam, 0, sizeof(EMV_MCKPARAM));
	memset(&tmpExTmParam, 0, sizeof(EMV_EXTMPARAM));
	tMCKParam.pvoid = &tmpExTmParam;
	EMVGetMCKParam(&tMCKParam);

	memset(aucPINData,0,sizeof(aucPINData));
	memset(aucPINForamt,0,sizeof(aucPINForamt));

	while(1)
	{
		nRet=EMV_II_StartCVM(&ucCVMType,&ucPINCnt);
		if(nRet)
		{
			if(nRet == EMV_QUIT_CVM)
				return EMV_OK;
			else
				return nRet;
		}

		if(ucCVMType == EMV_CVM_CERTIFICATE)
		{
			nPwdRst = cCertVerify();
			nRet=EMV_II_CompleteCVM(nPwdRst,NULL,&ucPINTryCount);
		}
		else if(ucCVMType == EMV_CVM_SIGNATURE || ucCVMType == EMV_CVM_FAIL_CVM || ucCVMType == EMV_CVM_NO_CVM)	
		{
			nRet=EMV_II_CompleteCVM(0,NULL,&ucPINTryCount);	
		}
		else if(ucCVMType == EMV_CVM_ONLINE_PIN)
		{
			if (ucBypassedOne == 1 && tmpExTmParam.ucBypassAllFlg == 1)
			{
				nRet = EMV_NEXT_CVM;
				continue;
			}
			nPwdRst = cEMVGetHolderPwd(0,0,NULL);
			gl_ucPinInput = 1;
			nRet =EMV_II_CompleteCVM(nPwdRst,NULL,&ucPINTryCount);
			if (nRet == EMV_NEXT_CVM) 
			{
				ucBypassedOne = 1;
				continue;
			}
		}
		else
		{
			while(2)
			{
				if (ucBypassedOne == 1 && tmpExTmParam.ucBypassAllFlg == 1) 
				{
					nRet = EMV_NEXT_CVM;
	                break;
				}
				nPwdRst = cEMVGetHolderPwd(nTryFlg, ucPINCnt, aucPINData);
				gl_ucPinInput = 1;
				if(DEVICE_IccGetTxnIF() == DEVICE_CONTACT_TXNIF)
				{
					if (gl_ucEmvPciUsedFlg == 1)
					{
						aucPINForamt[0] = 0x00;
					}
					else
					{
						aucPINForamt[0] = 0x01;
					}
					aucPINForamt[1] = 0x00;
					memcpy(&aucPINForamt[2],aucPINData,aucPINForamt[1]);
					nRet=EMV_II_CompleteCVM(nPwdRst,aucPINForamt,&ucPINTryCount);
				}
				else
				{
					aucPINForamt[0] = 0x01;
					aucPINForamt[1] = (unsigned char)(strlen(aucPINData));
					memcpy(&aucPINForamt[2],aucPINData,aucPINForamt[1]);
					nRet=EMV_II_CompleteCVM(nPwdRst,aucPINForamt,&ucPINTryCount);
				}
				if(nRet)
				{
					if(nRet == EMV_NEXT_CVM && tmpExTmParam.ucBypassAllFlg == 1)
					{
						ucBypassedOne = 1;
						break;
					}

					if(nRet	== EMV_PIN_TRYAGAIN)
					{
						nTryFlg =1;
						ucPINCnt = ucPINTryCount;
						memset(aucPINData,0,sizeof(aucPINData));
						continue;
					}
				}
				else//  [9/29/2016 zhoujie]
				{
					cEMVVerifyPINOK();
				}
				break;
			}
		}//if(ucCVMType)

		if(nRet != EMV_NEXT_CVM)
			return nRet;
	}
}

int  EMVStartTrans (unsigned long ulAuthAmt, unsigned long ulCashBackAmt, unsigned char *pACType)
{
	unsigned char buff[20]; 
	int ret, len; 
	int ucECOnlinePINFlg;
	unsigned char aucAuthAmt[12], aucCashBackAmt[12];
 
	SetAPICalledLog_emv(EMV_API_ID_EMVStartTrans);
 	if (pACType == NULL)
	{
		return EMV_PARAM_ERR;
	}
	*pACType = AC_AAC; 

	//if big amount is not set, ulAuthAmt and ulCashBackAmt will be saved here.
	memset(buff, 0, sizeof(buff));
	EMV_II_GetTLVData(0x9F02, sizeof(buff), buff, &len);
	if (!(len != 0 && (memcmp(buff, "\x00\x42\x94\x96\x72\x95", 6) >= 0)))
	{
		memset(aucAuthAmt, 0, sizeof(aucAuthAmt));
		memset(aucCashBackAmt, 0, sizeof(aucCashBackAmt));
		
		sprintf((char *)aucAuthAmt, "%012lu", ulAuthAmt);
		TwoOne_emv(aucAuthAmt,12,aucAuthAmt);
		sprintf((char *)aucCashBackAmt, "%012lu", ulCashBackAmt);
		TwoOne_emv(aucCashBackAmt,12,aucCashBackAmt);
		
		SaveAmtToEMVII_emv(aucAuthAmt, aucCashBackAmt);
	}

    EMV_II_ProcRestric();

	gl_ucPinInput = 0;
	ret = 0;
    ret = HolderVerify_emv(); 
    if (ret)
	{
		return ret;  
	}
	
 	if (gl_nEnableConfirmAmt && gl_ucPinInput == 0)
    {
    	ret = cEMVGetHolderPwd(2, 0, NULL);
    	if (ret) 
    		return ret;
    }
    EMV_II_RiskManagement(0);

	ret = EMV_II_TermActAnalyse(pACType);

	EMV_II_GetParamFlag(0x03, &ucECOnlinePINFlg);
	if(ucECOnlinePINFlg == 1)
	{
		cEMVGetHolderPwd(0, 0, NULL);
	}
	return ret;

}

int EMVCompleteTrans(int nCommuStatus, unsigned char *paucScript, int *pnScriptLen, unsigned char *pACType)
{
	unsigned char aucAppScriptRst[80];
	int nAppScrRstLen=0;

	SetAPICalledLog_emv(EMV_API_ID_EMVCompleteTrans);

	if (paucScript==NULL || pnScriptLen==NULL || pACType==NULL)
	{
		return EMV_PARAM_ERR;
	}
	memset(aucAppScriptRst, 0, sizeof(aucAppScriptRst));
	return EMV_II_CompleteTrans(nCommuStatus, paucScript, pnScriptLen, aucAppScriptRst,&nAppScrRstLen, pACType);
}

//(PBOC2.0)
int ReadLogRecord(int RecordNo)
{
	int nRet=0,nLogLen;
	unsigned char aucLogData[300];

	memset(aucLogData, 0, sizeof(aucLogData));
	nRet = EMV_II_ReadLogRecord_PBOC(0, RecordNo,aucLogData,&nLogLen);

	return nRet;
}

int GetLogItem(unsigned short Tag, unsigned char *TagData, int *TagLen)
{
	if(TagData == NULL || TagLen == NULL)
	{
		return EMV_PARAM_ERR;
	}
	return EMV_II_GetLogItemChild(0, Tag, TagData, TagLen);
}

int EMVReadSingleLoadLog(int nRecordNoIn)
{
	int nRet=0,nLogLen;
	unsigned char aucLogData[300];

	memset(aucLogData, 0, sizeof(aucLogData));
	nRet = EMV_II_ReadLogRecord_PBOC(1, nRecordNoIn,aucLogData,&nLogLen);
	
	return nRet;
}

int EMVGetSingleLoadLogItem(unsigned short usTagIn, unsigned char *paucDataOut, int *pnLenOut)
{
	if(paucDataOut == NULL || pnLenOut == NULL)
	{
		return EMV_PARAM_ERR;
	}
	return EMV_II_GetLogItemChild(1, usTagIn, paucDataOut, pnLenOut);	
}

int EMVReadAllLoadLogs(unsigned char *paucLogDataOut, int *pnLenOut)
{
	if (paucLogDataOut==NULL || pnLenOut==NULL)
	{
		return EMV_PARAM_ERR;
	}

	return EMV_II_ReadLogRecord_PBOC(2, 0, paucLogDataOut, pnLenOut);
}

int EMVGetLogData(unsigned char *paucLogDataOut, int *pnLenOut)
{
	if (paucLogDataOut == NULL || pnLenOut == NULL)
	{
		return EMV_PARAM_ERR;
	}

	return EMV_II_GetLogData(paucLogDataOut,pnLenOut);
}

int EMVGetCardECBalance(unsigned long *plBalance)
{
	if (plBalance==NULL)
	{
		return EMV_PARAM_ERR;
	}

	return EMV_II_GetCardECBalance_PBOC(plBalance);
}

int EMVSetTmECParam(EMV_TMECPARAM *tParam)//for PBOC 
{
	EMV_II_TMECPARAM tEMVECPARAM;

	if (tParam==NULL)
	{
		return EMV_DATA_ERR;
	}

	memset(&tEMVECPARAM, 0, sizeof(tEMVECPARAM));
	tEMVECPARAM.ucECTSIFlg = tParam->ucECTSIFlg;
	tEMVECPARAM.ucECTSIVal = tParam->ucECTSIVal;
	tEMVECPARAM.ucECTTLFlg = tParam->ucECTTLFlg;
	tEMVECPARAM.ulECTTLVal = tParam->ulECTTLVal;
	return EMV_II_SetTmECParam_PBOC(&tEMVECPARAM);
}

int EMVSetMCKParam(EMV_MCKPARAM *pMCKParam)
{
	EMV_II_TERMPARAM tTermParam;
	EMV_EXTMPARAM tExTmParam;

	
	if (pMCKParam == NULL) 
	{
		return EMV_DATA_ERR;
	}
	
	SetAPICalledLog_emv(EMV_API_ID_EMVSetMCKParam);

	memset(&tTermParam, 0, sizeof(tTermParam));
	EMV_II_GetTermParam(&tTermParam);

	tTermParam.ucBypassPin = pMCKParam->ucBypassPin;
	tTermParam.ucBatchCapture = pMCKParam->ucBatchCapture;
	if (pMCKParam->pvoid != NULL)
	{
		memset(&tExTmParam, 0, sizeof(tExTmParam));
		memcpy(&tExTmParam, pMCKParam->pvoid, sizeof(EMV_EXTMPARAM));
		tTermParam.ucUseTermAIPFlg = tExTmParam.ucUseTermAIPFlg;
		memcpy(tTermParam.aucTermAIP, tExTmParam.aucTermAIP, sizeof(tTermParam.aucTermAIP));
		tTermParam.ucBypassAllFlg = tExTmParam.ucBypassAllFlg;
	}
	EMV_II_SetTermParam(&tTermParam);
	return EMV_OK;
}

int EMVGetMCKParam(EMV_MCKPARAM *pMCKParam)
{
	EMV_II_TERMPARAM tTermParam;
	EMV_EXTMPARAM tExTmParam;

	SetAPICalledLog_emv(EMV_API_ID_EMVGetMCKParam);
	if (pMCKParam == NULL) 
	{
		return EMV_DATA_ERR;
	}

	memset(&tTermParam, 0, sizeof(tTermParam));
	EMV_II_GetTermParam(&tTermParam);
	
	pMCKParam->ucBypassPin = tTermParam.ucBypassPin;
	pMCKParam->ucBatchCapture = tTermParam.ucBatchCapture;
	memset(&tExTmParam, 0, sizeof(tExTmParam));
	tExTmParam.ucUseTermAIPFlg = tTermParam.ucUseTermAIPFlg;
	memcpy(tExTmParam.aucTermAIP, tTermParam.aucTermAIP, sizeof(tTermParam.aucTermAIP));
	tExTmParam.ucBypassAllFlg = tTermParam.ucBypassAllFlg;
	if(pMCKParam->pvoid != NULL)
	{
		memcpy(pMCKParam->pvoid,&tExTmParam,sizeof(sizeof(tExTmParam)));

	}
	
	return EMV_OK;
}


int EMVReadVerInfo(char *paucVer)
{
	SetAPICalledLog_emv(EMV_API_ID_EMVReadVerInfo);
	if(paucVer == NULL)
	{
		return EMV_PARAM_ERR;
	}

	return EMV_II_ReadVerInfo(paucVer);
}

void EMVSetConfigFlag(int nConfigflag)
{
	EMV_II_TERMPARAM tTermParam;
	
	SetAPICalledLog_emv(EMV_API_ID_EMVSetConfigFlag);
	memset(&tTermParam, 0, sizeof(tTermParam));
	EMV_II_GetTermParam(&tTermParam);

	if (nConfigflag & 0x0001)
	{
		tTermParam.ucAdviceFlg = 1;
	}
	if (nConfigflag & 0x0002)
	{
		gl_nEnableConfirmAmt = 1;
	}

	EMV_II_SetTermParam(&tTermParam);
}

void EMVSetScriptProcMethod(unsigned char ucScriptMethodIn)
{
	EMV_II_TERMPARAM tTermParam;
	
	SetAPICalledLog_emv(EMV_API_ID_EMVSetScriptProcMethod);
	memset(&tTermParam, 0, sizeof(tTermParam));
	EMV_II_GetTermParam(&tTermParam);
	
	tTermParam.ucScriptMethod = ucScriptMethodIn;
	
	EMV_II_SetTermParam(&tTermParam);
}

void EMVInitTLVData(void)
{
	SetAPICalledLog_emv(EMV_API_ID_EMVInitTLVData);
	EMV_II_InitTLVDataClss_PBOC();
}

int EMVSwitchClss(Clss_TransParam *ptTransParam,unsigned char *pucSelData, int nSelLen,
				  unsigned char *pucGPOData, int nGPOLen)
{
	SetAPICalledLog_emv(EMV_API_ID_EMVSwitchClss);
	if (ptTransParam==NULL || pucSelData==NULL|| pucGPOData==NULL)
	{
		return EMV_PARAM_ERR;
	}

	return EMV_II_SwitchClss_PBOC(ptTransParam, pucSelData, nSelLen, pucGPOData, nGPOLen);
}

int EMVSetAmount(unsigned char * szAuthAmount, unsigned char * szCashBackAmount)
{
	SetAPICalledLog_emv(EMV_API_ID_EMVSetAmount);
	if (szAuthAmount == NULL)
	{
		return EMV_DATA_ERR;
	}

	return SaveAmtToEMVII_emv(szAuthAmount, szCashBackAmount);
}

int EMVGetParamFlag(unsigned char ucParam, int *pnFlg)
{
	SetAPICalledLog_emv(EMV_API_ID_EMVGetParamFlag);
	if (pnFlg == NULL)
	{
		return EMV_PARAM_ERR;
	}
	return EMV_II_GetParamFlag(ucParam, pnFlg);
}

#ifndef _LINUX_TERM
int EMVSetPCIModeParam(unsigned char ucPciMode, unsigned char *pucExpectPinLen,unsigned long ulTimeoutMs)
{
#ifndef _PCI_PINPAD_
	
	if (ucPciMode >= 1)
	{
		return EMV_DATA_ERR;
	}
	gl_ucEmvPciUsedFlg = 0;

	DEVICE_SetPinInputParam((unsigned char *)"0", 0);
	return 0;
#else
	if (ucPciMode > 1  || (ucPciMode == 1 && pucExpectPinLen == NULL))
	{
		return EMV_DATA_ERR;
	}
	gl_ucEmvPciUsedFlg = ucPciMode;

	DEVICE_SetPinInputParam(pucExpectPinLen, ulTimeoutMs);
	return 0;
#endif
}
#endif

int EMVClearTransLog(void)
{
	EMV_II_InitTransLog();
	return 0;
}

// for quick test, skip offline PIN enter
void EMVSetAutoRunFlag(unsigned char ucAutoRunFlag)
{
	EMV_II_SetAutoRunFlag(ucAutoRunFlag);
}

int EMVGetVerifyICCStatus(unsigned char *pucSWA, unsigned char *pucSWB)
{
	if (pucSWA == NULL || pucSWB == NULL)
	{
		return EMV_PARAM_ERR;
	}
	
	return EMV_II_GetVerifyICCStatus(pucSWA, pucSWB);
}
#endif // endif _WIN32
