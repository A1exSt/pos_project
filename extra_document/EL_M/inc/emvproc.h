/*------------------------------------------------------------
* FileName: EmvProc.h
* Author: ZhangYuan
* Date: 2016-08-08
------------------------------------------------------------*/


#ifndef EMV_PROC_H_
#define EMV_PROC_H_


// IC���۶���
#define ICC_USER			0
#define ICC_CUSTOMER		0       //�ͻ�����


// ====== Error code definition ========
#define EL_EMV_RET_BASE							4500
#define EL_EMV_RET_FALLBACK						(EL_EMV_RET_BASE + 1)
//#define EL_EMV_RET_USER_CANCEL					(EL_EMV_RET_BASE + 2)
//#define EL_EMV_RET_CARD_DATA_ERR				(EL_EMV_RET_BASE + 3)
//#define EL_EMV_RET_NO_APP						(EL_EMV_RET_BASE + 4)
//#define EL_EMV_RET_TRANS_FAIL					(EL_EMV_RET_BASE + 5)
//#define EL_EMV_RET_TRANS_DENIAL					(EL_EMV_RET_BASE + 6)
//#define EL_EMV_RET_TRANS_DECLINED				(EL_EMV_RET_BASE + 7)
//#define EL_EMV_RET_TRANS_REQUEST_GO_ONLINE		(EL_EMV_RET_BASE + 8)


// ====== Functions ========
// test
int EMVLoadAppFromFile();

//
//int EMVInitTransaction(); //Added by ZhangYuan 2016-11-24
int EMVStartTransaction(unsigned char *pucACType);
int EMVCompleteTransaction();

#endif
