#include "..\inc\global.h"


#define LOG_TAG	__FILE__
#ifdef PED_ENC_DEBUG_		
#define EL_LOG(enLogPrio, pucTag, pucFunc, iLine, pucFmtStr, ...)		\
	El_Log(enLogPrio, pucTag, pucFunc, iLine, pucFmtStr, ##__VA_ARGS__)

#define EL_LOG_HEX(enLogPrio, pucTag, pucFunc, iLine, pucMsg, pucHexData, iHexDataLen)	\
	El_Log_Hex(enLogPrio, pucTag, pucFunc, iLine, pucMsg, pucHexData, iHexDataLen)

#else
#define EL_LOG_HEX(enLogPrio, pucTag, pucFunc, iLine, pucMsg, pucHexData, iHexDataLen)
#define EL_LOG(enLogPrio, pucTag, pucFunc, iLine, pucFmtStr, ...)

#endif // PED_ENC_DEBUG_



//#define TEST
#define EUI
//#define PED_DEBUG

static uchar *RetToString(int ReturnCode)
{
	switch(ReturnCode)
	{
	case 0:
		return "Success";
	case EL_SECURITY_RET_NO_KEY:
		return "NO Key";
	case EL_SECURITY_RET_PARAM_INVALID:
		return "Parameter Invalid";
	case EL_SECURITY_RET_ENCRYPT_DATA_ERR:
		return "Encrypt Data Error";
	case EL_SECURITY_RET_GET_PIN_BLOCK_ERR:
		return "Get PINBLOCK Error";
	case EL_SECURITY_RET_NO_PIN_INPUT:
		return "NO PIN Input";
	case EL_SECURITY_RET_INPUT_CANCEL:
		return "User Canceled";
	case EL_SECURITY_RET_INPUT_TIMEOUT:
		return "Timeout";
	default:
		return "Unkown Error";
	}
}

#ifdef TEST
int PedGenerate(void)
{
	int iRet;
	ST_KCV_INFO  stKcv;
	ST_KEY_INFO st_key_info;
	ST_KCV_INFO st_kcv_info;

	ScrCls();
	//	iRet = PedErase();
	//	if(iRet != PED_RET_OK)
	//    {
	//	 ScrClrLine(0,1);
	//	 ScrPrint(0,0,1,"PedErase fail iRet=%d", iRet);
	//	 getkey();
	//	 return 1;
	//	}
	memset(&stKcv,0,sizeof(stKcv));
	stKcv.iCheckMode=0x00;
	iRet=PedWriteTIK(1,0,16,"jeffjeffjeffjeff","\x98\x76\x54\x32\x10\x00\x00\x20",&stKcv);
	if(iRet != PED_RET_OK)
	{
		ScrClrLine(0,3);
		ScrPrint(0,0,1,"Tik,fail,Ret=%d",iRet);
		getkey();
		return 1;
	}
	memset(&st_key_info, 0, sizeof(st_key_info));
	memset(&st_kcv_info, 0, sizeof(st_kcv_info));
	st_key_info.ucSrcKeyIdx = 0;//ucTmpSrcKeyIdx;
	st_key_info.ucSrcKeyType = 0x03;//ucTmpSrcKeyType;
	st_key_info.ucDstKeyIdx = 0x02;//ucTmpDstKeyIdx;
	st_key_info.ucDstKeyType = 0x03;//ucTmpDstKeyType;
	st_key_info.iDstKeyLen = 8;//iTmpDstKeyLen;
	memcpy(st_key_info.aucDstKeyValue, "xiehuan1", 8);
	iRet = PedWriteKey(&st_key_info, &st_kcv_info);
	if(iRet != PED_RET_OK)
	{
		ScrClrLine(0,3);
		ScrPrint(0,0,1,"TPK,fail,Ret=%d,Type=%d",iRet,st_key_info.ucDstKeyType);
		getkey();
		return 1;
	}
	memset(&st_key_info, 0, sizeof(st_key_info));
	memset(&st_kcv_info, 0, sizeof(st_kcv_info));
	st_key_info.ucSrcKeyIdx = 0;//ucTmpSrcKeyIdx;
	st_key_info.ucSrcKeyType = 0x05;//ucTmpSrcKeyType;
	st_key_info.ucDstKeyIdx = 0x01;//ucTmpDstKeyIdx;
	st_key_info.ucDstKeyType = 0x05;//ucTmpDstKeyType;
	st_key_info.iDstKeyLen = 8;//iTmpDstKeyLen;
	memcpy(st_key_info.aucDstKeyValue, "xiehuan1", 8);
	iRet = PedWriteKey(&st_key_info, &st_kcv_info);
	if(iRet != PED_RET_OK)
	{
		ScrClrLine(0,3);
		ScrPrint(0,0,1,"TDK,fail,Ret=%d,Type=%d",iRet,st_key_info.ucDstKeyType);
		getkey();
		return 1;
	}
	return 0;
}

char* rsa_modul[]={
	/*512  bits*/ "B4AA1431083709281D20727A307C973C39DBF1F15668F1D496F1019DF0D5968C5ACA29DFB1E1BEF4AC51E8B3DD13D6DF5AA7EFFE303DF540B75BBBAE200D5733",
	/*1024 bits*/ "D801061C2E4FA925B0AEB6397F799F202BDD34F2FE31F103B272AF4B3BF5E1664E36ACFA04A1C09C8525FEF5B698F35674430B9BA166133E96EF638224D3FF7DD327D12FB5364889ED71924412FC4CA55A7D1291AC4360C2C6FE58A542507354066DD8D3AFD8B7DAE289A6E402ECB342B0F8286893A55B13D25517BBEECC59E1",
	/*1152 bits*/ "CC354770DFB6396DE71A2945C599EB26490AA213A3CA15B3BFF52A1FFAA54193FAF9CCACBEAFBCB4986CB24A9A09EF2C1778B392E44927A9EFD6527E1F40C8861E9193AF56623455C850261D6F4135B7C29C6B279FC7600D61813552C23FB565591D171E83699AC1438E98DA9C7CB4F9D419B67E202EBECF4C7FE338AD427A4DA3E088D0CD6A787A60159B147CDF0DFD",
	/*1984 bits*/ "C5CBB1DCDDA4F5629B6F6E905FE5E50B64480A38B260707D3FAEB13A65344C5A84A6AFA444C92CFF9F38BDE544835003530D1065AC01C05530B03779292AA2BB545EE7400EF4D2C65934513F0154D600CEF3C2D64E48B521D18A4C249AA1D09BBA04D5E76AB76576ABD74C618A234C23BCA80F287925435F6CB9C0F7F6EC7301015A36B44F334E3543E1F7E0A639945412BCB2C7A2797F77C04347EDCA5ECC64AAC347D94C434D2B58819EF1B9333348DC1F7A3A3BFC605E237F4521F9233DB303F31065AC959F69C664D291D6F0831BDA2904A782D242CF8F8CCC22EA99D327FC88D279D40A93CACEEC4450CF6C644BB1990CA7945A8855",
	/*2016 bits*/ "BAFD26ADA1F48360D37DC0433193F477AAFE547FDFE9E6980D1E95730B5385AA6E5EA6EEA0BAF6D0FE6806B1C6FFD5F0EFC5FCDD9DF73C030A24BC445891E96FFFC13DA007B85EEE587A320C844356A43EE239CA0F9B2EDF0CBA322C4F47D819F4E59BF09AED6487A65F71DB38BAED69A20E5DDA691B30B003130AFE69CF36BFAA4157E19E66CCF3695320671963C3DAF471063F8E9A0C2447D3F87139F29A9AF1D9D6C77DB83E8CAD419E668554A6C506EC4A4B0EFB35E2A838CC5C6ECE0D9C3A62165A89B130FC621C73A66E6198396E58E297324226E7B6EA87F7DED538A6E9AB7F71718446724D6000F2B8F9953E6429E9DDEC06BA36A12D7F65",
	/*2040 bits*/ "CD8C026A01F093F584696BFF6CC174E78C79432C3314954B342EC65A4123A6598676195A230B5C250B66D108DEC62B3FBC235AC402E603A309BA9AAA511ADA81EAEA85C2F04AD659FA0817A76A3CFBDF15119CD609C733D4586CED091F7935222A95B8FFED5ECA1FF3A089F93C6E3E0117F7D1ECCD4983FAB38FAED5F9D3E9639BDF86509D142DE5907BF88176D7DC80879288584D3841AC188CF2A9C5D950A1D50DD3AA3075C065636E3033F903E8555CF63493A4309D5E3F8964692D29A6BE90C48E6555C501F393D6E22B8D02FCA7A56B06EF16F2A89417F000DC9A78AFA0E995B3E444A52FACA0D11C0BFACFF2E86FA82763FC70F64EA2A173F0409399",
	/*2048 bits*/ "D33145CAA137116B6D925FB912E0B4B40ECD6706C1E4FD5A1B9551EFC60A410ABF83E7EA9D076875C5EED212A394FC975C3056AF94C0A0C1FFF6DDA04CFF1BC467867A939AB526C26677001E3348F75EEEBB8FECA87DCE754F1347FC2F601C0109BB40ED1E73E9A4EB65F1F9A2CBA968D787F1EF50F8BFC5DE4D770FD53584F79E8D1FCE7DB81D1BC654F2D08C5546B47B7D4E0981A1246E6E63DBB753A2BA75441A1A31E6DD4246B207CC58C8507DFB19579D06C8EBA2962C3915C02CA67703E97B7505F092F38C6A93C2ED202EBAC83C0CA03DF84D179794888632095D90CC40B5F8914195F3F446B91DA40BA344202FBB729BFE490994D440CD546D54D0D9"
};
char* rsa_private[]={
	/*512  bits*/ "B465A003D5D01EB48F60F6EE4BBC1CC7667ED8F7C2F0C7891FAF478E0E0D3864FDEDE009C56D1C710B1B1990357FF0EB3BEC0FEA9AE192A243E224AD0B30661",
	/*1024 bits*/ "BDDD3887B0D568B2AE7903C7CA9B1719EA348884469793685284A83C9339EFA4437F9FD02C1E19F5FABB893E66004F7460C14604C87BB6D2BF4D30C486A6A9DD4B60315563ABA17AEA6F61FCAA6168604424C125681CA4993914877BF6D10AAF1ED280985D372B0A989FDE1CF0BF4E7649DD15081CF17598698168373B820721",
	/*1152 bits*/ "8D84D6EFC879D6AEED9BD9C27AC0C29013CA84AC2BB45EF9E52C1877751295B1E21ABFD76F8706B5F9744AD31C9189E8FA93318FB919619D8992F43467A053D76ED3E08F80C42A1393AE6692B0B074A05871296E07C8715BC3010F68ACA16B12C1E82D68733CC200DEE6F8BD5C87A78655D919A416D4E54D1F89A5ECB9C6A37D2CF49E896CCDB36577A4B49B33A90E99",
	/*1984 bits*/ "9D66D8D6D38727435E4051F3106D8955046B93BEC4AC9C37FFCA8BFCD576C5E02EB9DB02553EE6D1512ABD6B4A9758C01C8243EF34EF7BD80F53DC014E3240A7E49F555593D15E3E3BD4B766F52F46D203DED5C8E72953036395B3BC5955B6058705BC1759C432393ACCAD5ADDABF33669363041DC0E3C4C1EE0BF5EEDAB826199FF88F9753D63627BF3E2CCADA46E2FB30D435394A77CD1259FF8D7D878F9FECC0921328241181A7AE2C89F0746E3640C401208BE2A4CEE006340B3F064D05113B023D821A93B0CD2257A3C40C3A21845028964F8CE4A89556C81245E8431565688A21BD933356FF2EBEF85DA77237FC521E4B9A8EA2BC1",
	/*2016 bits*/ "2AF0BB932020109389BDD50B19358AEC783E1395920C1AC24C30FBCD018ADAA53AD74DAA7B0AB4676624E974A5E8566F1000FD82BF25E988212A3279D7851A8077D11E9FC3661996CE86B13BA29BFC2797B147E81911600024E215678B854A211BD626AB68489FCF6EEB783009A513A4306CF02C99FA7E81CD12F0716046AAA66C20595E3B711177B777B262BC9439919918246FAD92C565CE44AD2C04A65B7894B08071293C53D04B4309065B85F30DA92CDDA83FFA2FF6318E7D2BAB3C903F3629F144172D8EC20D5E9756E4787F7B743F8293EC426BEBAD7FA4CCE02FA5C504F66C006738D11EA6E584CD1AC72E3BCE30917D1FEA326ADDA022F9",
	/*2040 bits*/ "087CA974DB13A5E853494F0E42DD52086582744594EE5AA3BA49DC22C9E1C0F29D5CAEFB4BCB2248DD49A783E83EE276453C6879036D9FE4D17804FED74E768B3AB9E83FFE4FF8BDCA571F32E86E10304BDC684F6623F57DE510CA50C01DEF15B4106E8D12AE01468476B27BBA050B465C48FD3FFBD1F74318BAEC0B25BF9C3832FB313921DCDBFBF255CAA3DE9262F701332F8D7F4673AD03AEBA92FBDBA1281A1F7F71553DA967CD79AADADBB2B4C80687EAA2EE4934A67E90EE35201CBBDA215A2D9F1DA9AE1549D094F1372C02B44FA70AFE4631E95FEA677120EE2FCD38E2E618015AB820352296F0BFC02D12BAC6A965F4B4E237A9D87003AC409889",
	/*2048 bits*/ "A44FC899FF5625E26C2C9EF7123DEBB9CF6E75C2E7262F0B653C86B4436384F944C9EEAD35A126DFC2B0AD96BCD8D4B13A938D4E2C30B6A259708B3C05CBD1A1BC5DFF8E49A6B95234804F822FAA8C73BAD009093A8A2F104C06975218D5F0D3C9EB7B7740E3341C10DD8F270746E7365FC230345ECE39FE4B97FF933CF1D7D19A21B57F05B86101C125099948EA70FB8B9D807942DB19997536C04BB04AD077DA9D455757154C52AE2896F68D08F53C5F2737562F3A9D467DF3B1C61337B271F4D866E97EACAD63F1AC8376EBF4C5153BC0939BD57BF30CEAFD7A55A8688509A67384C909F3DC0782C71E984673D34507B31A376E73DD9F9CB2005C363C9125"
};
char *rsa_exp[]= {
	/*24 bits*/  "010001",
	/*32 bits*/  "00010001"
};

void vTwo2One(uchar *psIn, uint nLength, uchar *psOut)
{
	uchar   tmp;
	uint    i;

	for(i=0; i<nLength; i+=2){
		tmp = psIn[i];
		if( tmp>'9' ){
			tmp = toupper(tmp) - 'A' + 0x0A;
		}else{
			tmp &= 0x0F;
		}
		psOut[i/2] = (tmp<<4);

		tmp = psIn[i+1];
		if( tmp>'9' ){
			tmp = toupper(tmp) - 'A' + 0x0A;
		}else{
			tmp &= 0x0F;
		}
		psOut[i/2] |= tmp;
	}
}
/**
int testRSA()
{
uchar szMK[128];
uchar szPKey[128];
uchar szExpKey[4];
uchar szDataIn[256];
uchar szDataOut[256];
uchar szTmpBuf[256];
uchar prnBuf[513];
uchar szBuf[256];
ST_RSA_KEY pstRsaKeyIn;
int iRet;

memset(szMK,0,sizeof(szMK));
vTwo2One(rsa_modul[1],256,szMK);
memset(szPKey,0,sizeof(szPKey));
vTwo2One(rsa_private[1],256,szPKey);
memset(szExpKey,0,sizeof(szExpKey));
vTwo2One(rsa_exp[1],8,szExpKey);
memset(szDataIn,0,sizeof(szDataIn));
memcpy(szDataIn,"\x04\x00\x04\x08\x09",5);
memset(szDataOut,0,sizeof(szDataOut));
RSARecover(szMK,128,szExpKey,4,szDataIn,szDataOut);
memset(szBuf,0,sizeof(szBuf));
memcpy(szBuf,szDataOut,128);

#ifdef TEST
memcpy(request,szBuf,128);
#endif
memset(szTmpBuf,0,sizeof(szTmpBuf));
RSARecover(szMK,128,szPKey,128,szBuf,szTmpBuf);
if(memcmp(szDataIn,szTmpBuf,256))
{
ScrPrint(0,2,1,"diff");
return 1;
}	
ScrClrLine(0,3);
ScrPrint(0,0,1,"SAME");
ScrPrint(0,2,1,"%x,%x,%x,%x,%x,%x",szTmpBuf[0],szTmpBuf[1],szTmpBuf[2],szTmpBuf[3],szTmpBuf[4],szTmpBuf[5]);
getkey();
memset(&pstRsaKeyIn,0,sizeof(ST_RSA_KEY));
pstRsaKeyIn.iModulusLen=1024;
memcpy(pstRsaKeyIn.aucModulus+512-128,szMK,128);
pstRsaKeyIn.iExponentLen=32;
memcpy(pstRsaKeyIn.aucExponent+508,szExpKey,4);
memcpy(pstRsaKeyIn.aucKeyInfo,"RSA PUBLIC",10);
iRet=PedWriteRsaKey(1,&pstRsaKeyIn);
if(iRet!=0)
{
ScrClrLine(2,3);
ScrPrint(0,2,1,"RSA1 ERR:%d",iRet);
getkey();
return iRet;
}
return 0;
}
**/
#endif

//#define TEST_FOR_NOTMS
int GetPinBlock(const char *pRequest,char *pResponse,uint *pRspLen)
{
	uchar szInputCardNo[24+1];
	int iRet=0,iPinEnType=0, iResRet=0;

	EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "pRequest[0]=%02x\n", pRequest[0]);

	*pRspLen=0;
	memset(szInputCardNo,0,sizeof(szInputCardNo));
	if(pRequest[0]!=0)             
	{
		if(pRequest[0]<13 || pRequest[0]>19) 
		{
			*pRspLen=4;
			EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "EL_SECURITY_RET_PARAM_INVALID, pRequest[0]=%02x\n", pRequest[0]);
			ComPoseInfoMsg(RetToString,EL_SECURITY_RET_PARAM_INVALID,EL_SECURITY_RET_PARAM_INVALID,__LINE__);
			return EL_SECURITY_RET_PARAM_INVALID;
		}
		memcpy(szInputCardNo,pRequest+1,pRequest[0]); 
	}
	else   /*如果上位机不提供卡号，pRequest[0]为0（卡号长度为0），此时从应用参数中去取*/
	{
		//memcpy(szInputCardNo,g_tTransParam.ucTrackTwoData,strlen((char*)g_tTransParam.ucTrackTwoData));//从应用中获取卡号
		if(strlen(g_tTransParam.ucPAN) == 0)
		{
			EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "EL_SECURITY_RET_PARAM_INVALID, strlen(g_tTransParam.ucPAN) == 0\n");
			ComPoseInfoMsg(RetToString,EL_SECURITY_RET_PARAM_INVALID,EL_SECURITY_RET_PARAM_INVALID,__LINE__);
			return EL_SECURITY_RET_PARAM_INVALID;
		}
		memcpy(szInputCardNo,g_tTransParam.ucPAN,strlen(g_tTransParam.ucPAN));
	}
	iRet=PedEncryptPin(szInputCardNo,pResponse,pResponse+8);
	EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "PedEncryptPin=%d, szInputCardNo=%s\n", iRet, szInputCardNo);
	if(iRet!=EL_RET_OK)
	{
		switch(iRet)
		{
		case PED_RET_ERR_NO_KEY:
			iResRet = EL_SECURITY_RET_NO_KEY;
			break;

		case PED_RET_ERR_NO_PIN_INPUT:
			iResRet = EL_SECURITY_RET_NO_PIN_INPUT;
			break;

		case PED_RET_ERR_INPUT_CANCEL:
			iResRet = EL_SECURITY_RET_INPUT_CANCEL;
			break;

		case PED_RET_ERR_INPUT_TIMEOUT:
			iResRet = EL_SECURITY_RET_INPUT_TIMEOUT;
			break;

		case EL_SECURITY_RET_PARAM_INVALID:
			iResRet = EL_SECURITY_RET_PARAM_INVALID;
			break;

		default:
			iResRet = EL_SECURITY_RET_GET_PIN_BLOCK_ERR; 
			break;
		}

		ComPoseInfoMsg(RetToString, iResRet, iRet, __LINE__);
		return iResRet;
	}
	/*从秘钥类型判定长度，秘钥索引需要从应用参数中取*/
	iPinEnType=g_tAppParam.ucPINEncryptionType[0];
#ifdef PED_DEBUG 
	iPinEnType = APPPARAM_PIN_ENCRYPT_TYPE_TDES;
#endif
	EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "iPinEnType=%02x\n", iPinEnType);
	switch(iPinEnType)
	{
	case APPPARAM_PIN_ENCRYPT_TYPE_TDES:
		*pRspLen=8;
		break;
	case APPPARAM_PIN_ENCRYPT_TYPE_DUKPT:
		*pRspLen=18;
		break;
	default:
		*pRspLen=0;
		break;
	}

	EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "*pRspLen=%d\n", *pRspLen);

	return iRet;
}


/******************************************************************
1．pCardNo 代表传入的卡号
2．pucPinBlockOut 代表输出加密后PinBlock数据
3. KsnOut代表输出的Ksn数据(如果是TDS加密就不用获取Ksn)
******************************************************************/


int PedEncryptPin(const uchar *pCardNo, uchar *pucPinBlockOut,uchar *pKsnOut) 
{
	int	  iPinKeyIndex=0,iPinEnType=0;
	int   iRet=0,iResRet=0,i=0,j=0;
	uchar szPinBlockOut[8];
	uchar szKsnOutPut[10];
	uchar szInputCardNo[24+1];
	uchar szShiftCardNo[19+1];
	uchar pMode=0,szRandData[8];
	uchar aucResponse[128];

	EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "pCardNo=%s\n", pCardNo);

	if((strlen((uchar*)pCardNo)<13 || strlen((uchar*)pCardNo)>19) || pucPinBlockOut==NULL || pKsnOut==NULL) 
	{
		EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "EL_SECURITY_RET_PARAM_INVALID, pCardNo=%s\n", pCardNo);
		//		ComPoseInfoMsg(RetToString,EL_SECURITY_RET_PARAM_INVALID,EL_SECURITY_RET_PARAM_INVALID,__LINE__);
		return EL_SECURITY_RET_PARAM_INVALID;
	}

	memset(szPinBlockOut,0,sizeof(szPinBlockOut));
	memset(szKsnOutPut,0,sizeof(szKsnOutPut));
	memset(szShiftCardNo,0,sizeof(szShiftCardNo));
	memset(szInputCardNo,0,sizeof(szInputCardNo));
	memcpy(szInputCardNo,pCardNo,strlen((char*)pCardNo));

	if(g_tAppParam.ucPINBlockMode[0] == 0x01 || g_tAppParam.ucPINBlockMode[0] == 0x02)	// Edit by ZhangYuan 2016-10-18 most of the time, the pMode would be 0x00, no need to generate the random number
	{
		for(i=0;i<8;i++)  //生成8个0xA到0xF的随机数
		{
			while(1)
			{
				if((j=rand()&0x0F)>=0x0A )
				{
					szRandData[i]=j;
					break;
				}
			}
		}
	}

	iRet=PubExtractPAN(szInputCardNo,szShiftCardNo);
	EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "PubExtractPAN=%d, szInputCardNo=%s, szShiftCardNo=%s\n", iRet, szInputCardNo, szShiftCardNo);
	if(iRet)           /*截取pan错误*/
	{
		//		 ComPoseInfoMsg(RetToString,EL_SECURITY_RET_PARAM_INVALID,EL_SECURITY_RET_PARAM_INVALID,__LINE__);
		return EL_SECURITY_RET_PARAM_INVALID; 
	}
	memset(szInputCardNo,0,sizeof(szInputCardNo));
	/*秘钥索引和pin加密模式需要从应用参数中去取*/
	iPinKeyIndex=g_tAppParam.ucPINEncryptionKeyIdx[0]; 
	iPinEnType=g_tAppParam.ucPINEncryptionType[0];
	pMode=g_tAppParam.ucPINBlockMode[0];
#ifdef PED_DEBUG
	iPinKeyIndex = 2;
	iPinEnType = APPPARAM_PIN_ENCRYPT_TYPE_TDES;
	pMode = 0x00;
#endif 
	EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "iPinKeyIndex=%02x, iPinEnType=%02x, pMode=%02x\n", iPinKeyIndex, iPinEnType, pMode);
	/****************************************
	mode:	0x00 ISO9564 格式0 
	0x01 ISO9564 格式1 
	0x02 ISO9564 格式3
	0x03 HK EPS专用格式
	****************************************/
	switch(pMode)/*mode 需要在应用参数中取*/
	{
	case 0x00:  //位移位后的16位主账号
		memcpy(szInputCardNo,szShiftCardNo,16);
		break;
	case 0x01:                  //8bytes.可以是随机数\交易流水号、时间戳
		memcpy(szInputCardNo,szRandData,8);
		break;
	case 0x02:    //为卡号移位后的16位加上8字节任意数（每个字节高四位和第四位必须在0xA--0xF）
		memcpy(szInputCardNo,szShiftCardNo,16);
		memcpy(szInputCardNo+16,szRandData,8);
		break;
#ifdef TEST           /*流水号从应用参数中取*/
	case 0x03://为交易流水号ISN（6 Byte）
		memcpy(szInputCardNo,szShiftCardNo,6);  //iSN
		break;
#endif
	default:
		EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "EL_SECURITY_RET_PARAM_INVALID, pMode=%02x\n", pMode);
		//		 ComPoseInfoMsg(RetToString,EL_SECURITY_RET_PARAM_INVALID,EL_SECURITY_RET_PARAM_INVALID,__LINE__);
		return EL_SECURITY_RET_PARAM_INVALID;  //err mode
	}

	///*秘钥索引需要从应用参数中取*/
#ifdef EUI
	EShowInputBox(EL_UI_PAGE_TRANS_ENTER_PIN,NULL, aucResponse, sizeof(aucResponse), 0);
#else
	ScrCls();
	ScrPrint(0,0,0,"pls input:");
	ScrGotoxy(55, 4);
#endif
	switch(iPinEnType)
	{
	case APPPARAM_PIN_ENCRYPT_TYPE_TDES:  //移位后的pCardNo
		iRet=PedGetPinBlock(iPinKeyIndex, "0,4,5,6,7,8,9,10,11,12",szInputCardNo, szPinBlockOut,pMode,60000);
		EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "PedGetPinBlock=%d\n", iRet);
		EL_LOG_HEX(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "PIN BLOCK: ", szPinBlockOut, 8);
		break;

	case APPPARAM_PIN_ENCRYPT_TYPE_DUKPT:
		iRet=PedGetPinDukpt(iPinKeyIndex, "0,4,5,6,7,8,9,10,11,12",szInputCardNo, szKsnOutPut, szPinBlockOut,pMode,60000);
		EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "PedGetPinDukpt=%d, iPinEnType=%02x, pMode=%02x\n", iRet);
		EL_LOG_HEX(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "PIN BLOCK: ", szPinBlockOut, 8);
		EL_LOG_HEX(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "KSN: ", szKsnOutPut, 10);
		break;

	default:
#ifndef EUI
		// TODO
		ScrCls();
#else // EUI_

#endif // EUI_

		EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "EL_SECURITY_RET_PARAM_INVALID, iPinEnType=%02x\n", iPinEnType);
		//		ComPoseInfoMsg(RetToString,EL_SECURITY_RET_PARAM_INVALID,EL_SECURITY_RET_PARAM_INVALID,__LINE__);
		return EL_SECURITY_RET_PARAM_INVALID;   //err encrypt index
	}
#ifndef EUI
	// TODO
	ScrCls();
#else // EUI_

#endif // EUI_

	EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "iRet=%d\n", iRet);
	if(iRet == PED_RET_OK)
	{
		if(APPPARAM_PIN_ENCRYPT_TYPE_TDES == iPinEnType)
		{
			memcpy(pucPinBlockOut,szPinBlockOut,8);
		}
		else if(APPPARAM_PIN_ENCRYPT_TYPE_DUKPT == iPinEnType)
		{
			memcpy(pucPinBlockOut,szPinBlockOut,8);
			memcpy(pKsnOut,szKsnOutPut,10);
		}
	}

	return iRet;
}	


int EncryptData(const char *pRequest ,char *pResponse,uint *pRspLen)   //给上位机调用
{
	int iRet=0;
	uint uiTempLen=0, uiDataLen=0;

	uiDataLen = (pRequest[0] << 8) + pRequest[1];
	EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "uiDataLen=%d\n", uiDataLen);
	if(uiDataLen ==0 || uiDataLen > BUF_LEN_2048)
	{
		*pRspLen=4;
		ComPoseInfoMsg(RetToString, EL_SECURITY_RET_PARAM_INVALID, uiDataLen, __LINE__);
		return EL_SECURITY_RET_PARAM_INVALID;  //Encryption data length err 
	}
	iRet=PedEncryptData(pRequest + 2,(pRequest[0] << 8) + pRequest[1],pResponse+2,&uiTempLen);
	EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "PedEncryptData=%d\n", iRet);
	EL_LOG_HEX(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "Encrypted data:  ", pResponse+2, uiTempLen);
	if(iRet != PED_RET_OK)
	{
		switch(iRet)
		{
		case EL_SECURITY_RET_PARAM_INVALID:
			ComPoseInfoMsg(RetToString,EL_SECURITY_RET_PARAM_INVALID,iRet,__LINE__);
			iRet = EL_SECURITY_RET_PARAM_INVALID;
			break;

		case PED_RET_ERR_NO_KEY:
			ComPoseInfoMsg(RetToString,PED_RET_ERR_NO_KEY,iRet,__LINE__);
			iRet = PED_RET_ERR_NO_KEY;
			break;

		default:
			ComPoseInfoMsg(RetToString,EL_SECURITY_RET_ENCRYPT_DATA_ERR,iRet,__LINE__);
			iRet = EL_SECURITY_RET_ENCRYPT_DATA_ERR;
			break;
		}
	}

	//iRet=PedEncryptData(pRequest + 1,pRequest[0],pResponse+2,&uiTempLen);
	pResponse[0]=(uiTempLen >>8) & 0xFF;
	pResponse[1]=uiTempLen & 0xFF;
	*pRspLen=uiTempLen+2;
	return iRet;
}


/******************************************************************
1．	pucDataIn 代表要输入加密的数据
2．	uiDataLen 代表要输入加密数据长度
3．pucEncDataOut 代表输出加密后的数据
4. puiEncDataLen 代表输出加密后数据长度
******************************************************************/

int PedEncryptData(const uchar *pucDataIn, uint uiDataLen,uchar *pucEncDataOut,uint *puiEncDataLen) 
{
	uint dataLen=0;
	int iDataEncIndex=0,iDataEncType=0,iRet=0;
	uchar szOutput[2048];
	uchar szInput[2048];
	ST_RSA_KEY pstRsaKeyOut;

	if(NULL==pucDataIn || uiDataLen==0 || pucEncDataOut==NULL)
	{
		//		ComPoseInfoMsg(RetToString,EL_SECURITY_RET_PARAM_INVALID,EL_SECURITY_RET_PARAM_INVALID,__LINE__);
		return EL_SECURITY_RET_PARAM_INVALID;
	}
	memset(szOutput,0,sizeof(szOutput));

	iDataEncType=g_tAppParam.ucDataEncryptionType[0];   /* 需要从应用参数中去取*/
	iDataEncIndex=g_tAppParam.ucDataEncryptionKeyIdx[0];
#ifdef PED_DEBUG
	iDataEncType = APPPARAM_DATA_ENCRYPT_TYPE_TDES;
	iDataEncIndex = 1;
#endif

	EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "DataIn: ", pucDataIn, uiDataLen);
	EL_LOG_HEX(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "iDataEncType=%02x, iDataEncIndex=%02x\n", iDataEncType, iDataEncIndex);

	switch (iDataEncType)
	{	
	case APPPARAM_DATA_ENCRYPT_TYPE_TDES:
		memset(szInput,0,sizeof(szInput));
		memcpy(szInput, pucDataIn, MIN(uiDataLen, sizeof(szInput)));
		if(0== uiDataLen %8) 
		{
			dataLen= uiDataLen; // 加密的数据是8的倍数,不足八位补0X00
		}
		else
		{
			dataLen=( uiDataLen/8+1)*8;
		}

		iRet=PedCalcDES(iDataEncIndex,szInput, dataLen,szOutput,0x01);/*PDK需要从应用参数中去取*/
		EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "EncryptData: ", szOutput, dataLen);
		if(iRet!=0) 
		{
			return iRet;
		}
		memcpy(pucEncDataOut, szOutput,dataLen);  
		*puiEncDataLen =dataLen; 
		break; 

	case APPPARAM_DATA_ENCRYPT_TYPE_RSA:	
		memset(&pstRsaKeyOut,0,sizeof(ST_RSA_KEY));;
		memset(szInput,0,sizeof(szInput));
		iRet=PedReadRsaKey(iDataEncIndex,&pstRsaKeyOut);//获取公钥模长，确定加密信息长度,秘钥索引取测试索引1
		EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "PedReadRsaKey=%d, pstRsaKeyOut.iModulusLen=%d\n", iRet, pstRsaKeyOut.iModulusLen);
		if(iRet != PED_RET_OK)
		{
			return iRet;
		}

		if(uiDataLen > pstRsaKeyOut.iModulusLen/8) //=== Added by ZhangYuan 2016-12-19 ==
		{
			return EL_SECURITY_RET_PARAM_INVALID;
		}

		memcpy(szInput,pucDataIn, MIN(uiDataLen, sizeof(szInput)));  

		iRet=PedRsaRecover(iDataEncIndex,szInput,szOutput,NULL); //秘钥索引需要去应用参数中去取
		EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "PedRsaRecover=%d\n", iRet);
		EL_LOG_HEX(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "Encrypt Data: ", szOutput, pstRsaKeyOut.iModulusLen/8);
		if(iRet != PED_RET_OK)
		{
			return iRet;
		}
		*puiEncDataLen= pstRsaKeyOut.iModulusLen/8;
		memcpy(pucEncDataOut,szOutput,pstRsaKeyOut.iModulusLen/8);//从PedReadRsaKey（）函数中获取模长
		break;	
		/**	
		case APPPARAM_DATA_ENCRYPT_TYPE_NONE:		//如果加密方式为none,数据原样返回
		memcpy(pucEncDataOut,pucDataIn,uiDataLen); 
		*puiEncDataLen=uiDataLen;
		break;
		**/

	case APPPARAM_DATA_ENCRYPT_TYPE_MAC:	// Added by ZhangYuan 2017-03-22
		iRet = PedGetMac(g_tAppParam.ucDataEncryptionKeyIdx[0], pucDataIn, uiDataLen, pucEncDataOut, 0x00);
		if(iRet == PED_RET_OK)
		{
			*puiEncDataLen = 8;
		}
		break;

	default:
		//		    ComPoseInfoMsg(RetToString,EL_SECURITY_RET_PARAM_INVALID,iRet,__LINE__);
		return EL_SECURITY_RET_PARAM_INVALID;
	}
	return EL_RET_OK;
}										

