#ifndef _PED_ENCRYPT_H
#define _PED_ENCRYPT_H



int EncryptData(const char *pRequest ,char *pResponse,uint *pRspLen);
int PedEncryptData(const uchar *pucDataIn, uint uiDataLen,uchar *pucEncDataOut,uint *puiEncDataLen);
int PedEncryptPin(const uchar *pCardNo, uchar *pucPinBlockOut,uchar *pKsnOut);
int GetPinBlock(const char *pRequest,char *pResponse,uint *pRspLen);
int PedGenerate(void);
/*
enum
{
	TDES=1,
	DUKPT,
	RSA,
};
**/
/*NOTICE: PED return  PED_RET_OK or PED_RET_ERR_NO_PIN_INPUT(1301) are correct*/
#define		EL_RET_OK								0					/*ped return ok*/
#define		EL_SECURITY_RET_BASE					3000
#define		EL_SECURITY_RET_NO_KEY					(EL_SECURITY_RET_BASE + 1)  /*key doesnot exist*/
#define		EL_SECURITY_RET_PARAM_INVALID			(EL_SECURITY_RET_BASE + 2)  /*Parameter error or invalid*/
#define		EL_SECURITY_RET_ENCRYPT_DATA_ERR		(EL_SECURITY_RET_BASE + 3)  /*Encrypt data error*/
#define		EL_SECURITY_RET_GET_PIN_BLOCK_ERR		(EL_SECURITY_RET_BASE + 4)  /*Get pin block error*/
#define     EL_SECURITY_RET_NO_PIN_INPUT			(EL_SECURITY_RET_BASE + 5)  /*Not input pin*/
#define		EL_SECURITY_RET_INPUT_CANCEL			(EL_SECURITY_RET_BASE + 6)  /*user cancel*/
#define		EL_SECURITY_RET_INPUT_TIMEOUT			(EL_SECURITY_RET_BASE + 7)  /*input timeout*/

#endif