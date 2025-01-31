/*
 * ============================================================================
 * COPYRIGHT
 *              Pax CORPORATION PROPRIETARY INFORMATION
 *   This software is supplied under the terms of a license agreement or
 *   nondisclosure agreement with Pax Corporation and may not be copied
 *   or disclosed except in accordance with the terms in that agreement.
 *      Copyright (C) 2016 Pax Corporation. All rights reserved.
 * Module Date:2016/10/08     
 * Module Auth:huangwp     
 * Description:emv xml file operarte module

 *      
 * ============================================================================
 */


#ifndef _XML_EMV_FILE_OP_H
#define _XML_EMV_FILE_OP_H

//#define EMV_PARSE_TEST


#define EMV_XML_FILE_OP_ERR_BASE                  	  -5000
#define EMV_XML_FILE_OP_ERR_INVALID_PARAM             (EMV_XML_FILE_OP_ERR_BASE-1)
#define EMV_XML_FILE_OP_ERR_ONELINE_OVER_1024         (EMV_XML_FILE_OP_ERR_BASE-2)   //oneline max len is 1024 byte
#define EMV_XML_FILE_OP_ERR_NODE_NOT_EXIST            (EMV_XML_FILE_OP_ERR_BASE-3)
#define EMV_XML_FILE_OP_ERR_FUNCTIONE_NOT_FOUNTD      (EMV_XML_FILE_OP_ERR_BASE-4)
#define EMV_XML_FILE_OP_ERR_RECORD_NOT_FOUNTD     	  (EMV_XML_FILE_OP_ERR_BASE-5)
#define EMV_XML_FILE_OP_ERR_AID_OVER_FLOW      	  	  (EMV_XML_FILE_OP_ERR_BASE-6)




#define EMV_XML_FILE_OP_RETURN_BASE                   5000
#define EMV_XML_FILE_OP_NOT_FOUND_NODE                (EMV_XML_FILE_OP_RETURN_BASE+1)
#define EMV_XML_FILE_OP_FOUND_NODE              	  (EMV_XML_FILE_OP_RETURN_BASE+2)
#define EMV_XML_FILE_OP_NOT_FOUND_RETURN              (EMV_XML_FILE_OP_RETURN_BASE+3)
#define EMV_XML_FILE_LIST_END              		      (EMV_XML_FILE_OP_RETURN_BASE+4)
#define EMV_XML_FILE_NODE_END              		      (EMV_XML_FILE_OP_RETURN_BASE+5)


#define EMV_XML_FILE_OP_OK                            (EMV_XML_FILE_OP_RETURN_BASE+6)

#define EMV_BUF_NODE_VALUE_LEN                1024
#define EMV_TEMP_BUF_LEN                      2048

#define EMV_FILE_READ_BUF_LEN                 1024
#define EMV_FILE_ONELINE_BUF_LEN              1024


#define EMV_PRASED_NODE_CNT                   6
#define EMV_PRASED_RECORD_MAX_NODE_CNT        10

//#define EMV_XML_FILE_NAME                "emv_xml.emv"
#define EMV_XML_FILE_NAME                  "emv_param.emv"

#define EMV_XML_FILE_HAVE_PARSED           "emv_parsed.emv"     //to store the struct data that had been parsed


#define EMV_AID_NODE                       "AID"
#define EMV_CAPK_NODE                      "CAPK"
#define EMV_ICS_NODE                       "ICS"
#define EMV_REVOKEDCERTIFICATE_NODE        "REVOKEDCERTIFICATE"
#define EMV_TERMINALCONFIGURATION_NODE     "TERMINALCONFIGURATION"
#define EMV_CARD_SCHEM_CONF_NODE           "CARDSCHEME"



//typedef int (*EMV_PARSE_CB)(const uchar *pucXmlOneLine);
typedef int (*EMV_PARSE_CB)(const uchar *pucXmlOneLine, void *Data);

typedef struct Emv_Parse_Func
{
	int iType;
	EMV_PARSE_CB vFunc;
}EMV_PARSE_FUNC;


typedef enum{
	AID = 1,
	CAPK,
	ICS,
	REVOKCERTIFICATE,
}EMV_NODE_TYPE;

typedef enum{
	PARTIALAIDSELECTION = 10,
	APPLICATIONID,//11
	IFUSELOCALNAME,//12
	LOACALAIDNAME,//13
	TERMINALAIDVERSION,//14
	TACDENIAL,//15
	TACONLINE,//16
	TACDEFAULT,//17
	FLOORLIMIT,//18
	THRESHOLD,//19
	TARGETPERCENTAGE,//20
	MAXTARGETPERCENTAGE,//21
	TERMINALDEFAULTTDOL,//22
	TERMINALDEFAULTDDOL,//23
	TERMINALRISKMANAGEMENTDATA,//24
}AID_CHILD_NODE;

typedef enum{
	RID = 30,
	KEYID,
	HASHARITHMETICINDEX,
	RSAARITHMRTICINDEX,
	MODULELENGTH,
	MODULE,
	EXPONENTLENGTH,
	EXPONENT,
	EXPIREDATE,
	CHECKSUM,
}CAPK_CHILD_NODE;

typedef enum{
	TYPE = 50,
	TERMINALTYPE,
	CARDDATAINPUTCAPABILITY,
	CVMCAPABILITY,
	SECURITYCAPABILITY,
	ADDITIONALTERMINALCAPABILITIES,
	GETDATAFORPINTRYCOUNTER,
	BYPASSPINENTRY,
	SUBSEQUENBYPASSPINENTRY,
	EXCEPTIONFILESUPPORTED,
	FORCEDONLINECAPABILITY,
	ISSUERFEFERRALSSUPPORTED,
	CONFIGURATIONCHECKSUM,
}ICS_CHILD_NODE;


typedef enum{
	REVOKERID = 70,
	REVOKEKEYID,
	CERTIFICATESN,
}REVOCATION_CHILD_NODE;

typedef enum{
	MERCHANTID = 90,
	TERMINALID,
	TERMINALCOUNTRYCODE,
	TERMINALCURRENCYCODE,
	TERMINALCURRENTCYEXPONENT,
	REFERENCECURRENCYCODE,
	REFERENCECURRENTEXPONENT,
	CONVERSIONRATIO,
	MECHANTNAME,
	MERCHANTLOCATION,
	MERCHANTCATEGORYCODE,
}TERMINAL_CHILD_NODE;

typedef enum{
	CARD_SCHEME_CONF_RID = 110,
	CARD_SCHEME_ICS_TYPE,
	CARD_SCHEME_ICS_AID,
}CARD_SCHEME_CONF_CHILD_NODE;



typedef struct EmvAidNode
{
	uchar SelFlag;
	uchar AID[17];
	uchar AidLen;
	int IfUseLocalName;
	uchar AppName[33]; 
	uchar Version[3];
	uchar TACDenial[6];
	uchar TACOnline[6]; 	 
	uchar TACDefault[6];
	ulong FloorLimit;
	ulong Threshold;  
	uchar TargetPer;
	uchar MaxTargetPer; 
	uchar tDOL[256]; 
	uchar dDOL[256];
	uchar RiskManData[10]; 
	
}EMV_AID_NODE_T;

typedef struct {
	uchar RID[5];                 //Registered Application Provider Identifier
	uchar KeyID;                  //Key Index
	uchar HashInd;                //Hash arithmetic indicator
	uchar ArithInd;               //RSA arithmetic indicator
	uchar ModulLen;               //Length of Module
	uchar Modul[248];             //Module
	uchar ExponentLen;            //Length of exponent
	uchar Exponent[3];            //Exponent
	uchar ExpDate[3];             //Expiration Date (YYMMDD) 
	uchar CheckSum[20];           //Check Sum of Key
}EMV_CAPK_NODE_T;


typedef struct EmvIcsNode
{
	uchar Type[128];
	uchar ConfigurationCheckSum[4]; 
	uchar TerminalType;
	uchar CardDataInputCapability;
	uchar CVMCapability; 
	uchar SecurityCapability;
	uchar AdditionalTerminalCapabilities[5];
	int GetDataForPINTryCounter; 	 
	int BypassPINEntry;
	int SubsequentBypassPINEntry;
	int ExceptionFileSupported;  
	int ForcedOnlineCapability;
	int IssuerReferralsSupported; 
}EMV_ICS_NODE_T;

typedef  struct 
{
	uchar ucRid[5];               // Registered Application Provider Identifier
	uchar ucIndex;                // Certification Authenticate Public Key Index.
	uchar ucCertSn[3];            // Issuer Certificate Serial Number.
}EMV_REVOCLIST_T;

typedef struct EmvCardSchemConf
{
	uchar ucRid[5];            
	uchar ucIcsType[128];
	uchar ucAid[17];
}EMV_CARD_SCHEM_CONF_T;



typedef struct EmvTerminalInfoNode
{
	uchar aucMerchName[256];
	uchar aucMerchantLocation[32];
	uchar aucMerchId[32];
	uchar aucTermId[32];
	uchar aucCountryCode[2];
	uchar aucTransCurrCode[2]; 
	uchar aucReferCurrCode[2];
	uchar aucMerchCateCode[2];
	uchar ucTerminalCurrencyExponent;
	uchar ucReferenceCurrencyExponent;
	uchar ucConversionRatio;
	
}EMV_TERMINALINFO_NODE_T;


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


int ParseEmvFile(const uchar *pucFileName);
int SaveParsedDataToFile(const uchar *pucFileName, const uchar *pucInData, int iDataSize, int iOffset);

int GetEmvAidData(EMV_AID_NODE_T *pstEmvAidNode, int iSize, int iAidIndex);
int TestGetEmvAid();

int GetEmvCapkData(EMV_CAPK_NODE_T *pstEmvCapkNode, int iSize, int iCapkIndex);
int TestGetEmvCapk();

int GetEmvIcsData(EMV_ICS_NODE_T *pstEmvIcsNode, int iSize, int iIcsIndex);
int TestGetEmvIcs();

int GetEmvRevokeData(EMV_REVOCLIST_T *pstEmvRevokeNode, int iSize, int iRevokeIndex);
int TestGetEmvRevoke();


int GetEmvTermInfoData(EMV_TERMINALINFO_NODE_T *pstEmvTermInfoNode, int iSize, int iTermInfoIndex);
int TestGetEmvTermInfo();

int GetEmvCardSchemConfData(EMV_CARD_SCHEM_CONF_T *pstEmvCardSchemConfNode, int iSize, int iCardSchemConfIndex);
int TestGetCardShcemConf();

int EmvGetNodeNum(const uchar *pucName);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#pragma pack()


#endif

