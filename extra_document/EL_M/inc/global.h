/*
 * ============================================================================
 * COPYRIGHT
 *              Pax CORPORATION PROPRIETARY INFORMATION
 *   This software is supplied under the terms of a license agreement or
 *   nondisclosure agreement with Pax Corporation and may not be copied
 *   or disclosed except in accordance with the terms in that agreement.
 *      Copyright (C) 2016 Pax Corporation. All rights reserved.
 * Module Date:2016/08/09     
 * Module Auth:huangwp     
 * Description:global var
 *      
 * ============================================================================
 */

#ifndef _GLOBAL_H
#define _GLOBAL_H

#include <String.h>
#include <Stdarg.h>
#include <Stdlib.h>
#include <Stdio.h>

#include <posapi.h>

#include "comm.h"
#include "bt.h"
#include "ui.h"
#include "sofTimer.h" 
#include "protocol.h"
#include "xmlparse.h"
#include "file.h"
#include "appLib.h"
#include "UiXmlFileOp.h"

#include "ped_encrypt.h"

#include "EmvIIApi.h"
#include "EmvApi.h"
#include "CL_common.h"
#include "CLEntryApi.h"
#include "L2_Device.h"
#include "ClssApi_MC.h"
#include "ClssApi_WAVE.h"

#include "transmain.h"
#include "parammgmt.h"
#include "emvproc.h"
#include "clssproc.h"

#include "EmvXmlFileOp.h"
#include "ClssXmlFileOp.h"


#include "filedownload.h"

#include "remotedownload.h"

//emvtest.h for testing
#include "emvtest.h"

#include "debug.h"

#define _D180S_	// for D180S terminal


//debug switch
//#define APP_DEBUG
//#define FILE_DEBUG
//#define PROTOCOL_DEBUG
//#define COMM_DEBUG
//#define UI_XML_FILE_OP_DEBUG
//#define XML_DEBUG
//#define BT_DEBUG
//#define MPOS_PROTIMS_DEBUG
//#define CLSS_PARSE_DEBUG
//#define EMV_PARSE_DEBUG
//#define FILEDOWNLOAD_DEBUG
//#define PED_ENC_DEBUG_
//#define PARAMMGMT_DEBUG_	
//#define CLSS_DEBUG_
//#define EMV_DEBUG_
//#define TRANS_DEBUG_
//#define UI_DEBUG_


// **** Timer **********************************
#define TIMER_TRANS  3

// **** Timer end *******************************

// ==== COMM ==================================
//#define COMM_DEBUG_	// Added by Zhangyuan

#define BUF_LEN_24		24
#define BUF_LEN_512		512
#define BUF_LEN_1024	1024
#define BUF_LEN_2042	2042
#define BUF_LEN_2048	2048

#define EL_RESPONSE_CMD_OFFSET			8
#define EL_RESPONSE_STATUS_OFFSET		10
#define EL_RESPONSE_DATA_OFFSET			14

typedef struct{	
   short addr;	/* eeprom地址 */
   char *szBuf;	/* 数据指针 */
   int len;	/* 数据长度*/
}T_ConfigArg;

extern unsigned char g_ucCommPort;
extern int g_UsbOpenFlag;
extern int g_BtOpenFlag;
extern int g_ConnectFlag;
extern int ProtocolType;
extern int g_IdeUi;
extern uchar g_UiFileExist;

extern uchar g_pucErrMsg[ERR_MSG_OFFSET];

// ==== COMM END ==============================

// ==== FILE ==================================
#define BLK_LEN         1024

extern FILE_INFO_T g_stFileInfo[MAX_FILE_CNT];

// ==== FILE END ==============================

typedef struct _tagCURRENCY_CONFIG 
{
	uchar	szName[3+1];	        // short for currency, e.g. "HKD", "USD"
	uchar	sCurrencyCode[2];		// ISO-4217   currency code, e.g. NTD"\x09\x01"
    uchar   sCountryCode[2];        // ISO-3166-1 Country or area Code, e.g. Taiwan Province"\x01\x58"
	uchar	ucDecimal;		        // Decimal currency. e.g. JPY is 0, USD，HKD are 2, some are 3
	uchar	ucIgnoreDigit;	        // ignore digits at tail, when convert amount from ISO8583 field 4
}CURRENCY_CONFIG;

extern uchar g_ucIsFallBack;

#define EMV_CAPK_LIST_FILE		"emvCapkListFile"
#define PAYPASS_TORN_LOG_FILE 	"KernalFileMCTornLog"
#define EMV_APP_LIST_FILE 		"emvAppListFile"


// ====== Table file block distribution =============

#define TABLE_FS_BLOCK_SIZE					1024

#define FILE_EMV_PARAM_INFO_BLOCK_START		190
#define FILE_EMV_AID_INFO_BLOCK_START		191
#define FILE_EMV_CAPK_INFO_BLOCK_START		195

#define FILE_EMV_CAPK_BLOCK_START			200
#define FILE_EMV_REVOKE_CAPK_BLOCK_START	300

#define FILE_EMV_AID_BLOCK_START			400
#define FILE_EMV_PARAMETER_BLOCK_START		500
#define FILE_EMV_EXTM_PARAM_BLOCK_START		510
#define FILE_EMV_MCK_PARAM_BLOCK_START		511
//Added by Kevin Liu for paypass torn log.
#define FILE_EMV_BLOCK_START_TORN_LOG		900
#define FILE_EMV_BLOCK_START_TLV_DATA		1000




// ====== Table file block distribution  end ========


// ======== Remote Download Begin=========================

extern unsigned char	g_ucRebootFlag;

// ======== Remote Download End ==========================

#define EUI_
// ==== UI PAGE ===
#define EL_COMM_WELCOME_PAGE          "Idle.xml"

#define EL_UI_PAGE_TRANS_PROCESSING				"Processing.xml"
#define EL_UI_PAGE_TRANS_APP_SELECT				"AppSelect.xml"
#define EL_UI_PAGE_TRANS_APP_SELECT_AGAIN		"AppSelectAgain.xml"
#define EL_UI_PAGE_TRANS_PIN_VERIFY_OK			"PinVerifyOK.xml"
#define EL_UI_PAGE_TRANS_LAST_ENTER_PIN			"LastEnterPin.xml"
#define EL_UI_PAGE_TRANS_PIN_ERR_TRY_AGAIN		"PinError.xml"
#define EL_UI_PAGE_TRANS_TIMEOUT					"Timeout.xml"
#define EL_UI_PAGE_TRANS_PED_ERR					"PedErr.xml"
#define EL_UI_PAGE_TRANS_ENTER_PIN					"EnterPIN.xml"
#define EL_UI_PAGE_TRANS_INVALID_CARD				"InvalidCard.xml"
#define EL_UI_PAGE_TRANS_NEED_ONLINE				"NeedOnline.xml"
#define EL_UI_PAGE_TRANS_APPROVED					"TxnApproved.xml"
#define EL_UI_PAGE_TRANS_DECLINED					"TxnDeclined.xml"

#define EL_UI_PAGE_TRANS_GET_CARD					"GetCard.xml"
#define EL_UI_PAGE_TRANS_FALL_BACK				"Fallback.xml"
#define EL_UI_PAGE_TRANS_TAP_AGAIN				"TapCardAgain.xml"
#define EL_UI_PAGE_TRANS_REMOVE_CARD				"RemoveCard.xml"
#define EL_UI_PAGE_TRANS_SEE_PHONE				"SeePhone.xml"
#define EL_UI_PAGE_TRANS_AMOUNT_CONFIRM			"AmountConfirm.xml"
#define EL_UI_PAGE_TRANS_INSERT_CARD				"InsertCard.xml"
#define EL_UI_PAGE_TRANS_RESWIPE_CARD			"ReswipeCard.xml"
#define EL_UI_PAGE_TRANS_READ_CARD_ERR			"ReadCardErr.xml"
#define EL_UI_PAGE_TRANS_OPEN_PICC_ERR			"OpenPiccErr.xml"
#define EL_UI_PAGE_TRANS_TOO_MANY_CARDS			"TooManyCards.xml"
#define EL_UI_PAGE_TRANS_TRY_ANOTHER_INTERFACE  "TryAnotherInter.xml"


#endif	// _GLOBAL_H

// end of file

