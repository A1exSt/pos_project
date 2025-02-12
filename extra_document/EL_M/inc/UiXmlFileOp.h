/*
 * ============================================================================
 * COPYRIGHT
 *              Pax CORPORATION PROPRIETARY INFORMATION
 *   This software is supplied under the terms of a license agreement or
 *   nondisclosure agreement with Pax Corporation and may not be copied
 *   or disclosed except in accordance with the terms in that agreement.
 *      Copyright (C) 2016 Pax Corporation. All rights reserved.
 * Module Date:2016/09/21     
 * Module Auth:huangwp     
 * Description:UI xml file operarte module

 *      
 * ============================================================================
 */


#ifndef _XML_UI_FILE_OP_H
#define _XML_UI_FILE_OP_H

#define UI_XML_FILE_OP_ERR_BASE                  	 -4000
#define UI_XML_FILE_OP_ERR_INVALID_PARAM             (UI_XML_FILE_OP_ERR_BASE-1)
#define UI_XML_FILE_OP_ERR_PAGE_NOT_EXSIT            (UI_XML_FILE_OP_ERR_BASE-2)
#define UI_XML_FILE_OP_ERR_ONELINE_OVER_1024         (UI_XML_FILE_OP_ERR_BASE-3)
#define UI_XML_FILE_OP_ERR_PAGE_IS_NULL              (UI_XML_FILE_OP_ERR_BASE-4)
#define UI_XML_FILE_OP_ERR_BUF_IS_FULL               (UI_XML_FILE_OP_ERR_BASE-5)


#define UI_XML_FILE_OP_RETURN_BASE                   4000
#define UI_XML_FILE_OP_NOT_FOUND_PAGE                (UI_XML_FILE_OP_RETURN_BASE+1)
#define UI_XML_FILE_OP_FOUND_PAGE              		 (UI_XML_FILE_OP_RETURN_BASE+2)
#define UI_XML_FILE_OP_NOT_FOUND_RETURN              (UI_XML_FILE_OP_RETURN_BASE+3)


#define UI_XML_FILE_OP_OK                             0

//UI BOX NAME
#define UI_TEXT_BOX									 "<TextBox "
#define UI_PICTURE_BOX								 "<PictureBox "
#define UI_INPUT_BOX								 "<InputBox "
#define UI_MENU_ITEM_BOX							 "<MenuItem "


//ATTR NAME
#define UI_PAGE_ATTRA_NAME   		"name"
#define UI_PAGE_ATTRA_X  			"x"
#define UI_PAGE_ATTRA_Y   			"y"
#define UI_PAGE_ATTRA_TEXTALIGN     "textAlign"
#define UI_PAGE_ATTRA_VALUE   		"value"
#define UI_PAGE_ATTRA_MODE   		"mode"
#define UI_PAGE_ATTRA_TYPE   		"type"
#define UI_PAGE_ATTRA_FONTSIZE   	"fontSize"
#define UI_PAGE_ATTRA_KEYACCECPT    "keyAccept"
#define UI_PAGE_ATTRA_MINLEN   		"minlen"
#define UI_PAGE_ATTRA_MAXLEN   		"maxlen"

#define UI_ATTR_VALUE_LEN           1024
#define UI_PICTURE_BOX_VALUE_LEN    768



#define UI_FILE_NAME                "ui_xml_demo.ui"

#define UI_READ_BUF_SIZE            1024
#define UI_XML_FILE_LINE_BUF_SIZE    512

#define UI_PAGE_ROOT_NODE_NAME      "<Page name="
typedef struct Struct_Addr_Table
{
	int iIndex;
	int iAddr;	
}STRUCT_ADDR_TABLE_T;

typedef int (*UI_PARSE_CB)(const uchar *pucXmlOneLine, uchar *pucOutData, int iOffset);

typedef struct Ui_Parse
{
	int UiBoxType;
	UI_PARSE_CB UiParseCb;
	
}UI_PARSE;

typedef struct Ui_Box_func
{
	int iType;
	UI_PARSE_CB vFunc;
}UI_PARSE_FUNC;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int ParseTextBox(const uchar *pucXmlOneLine, uchar *pucOutData, int iOffset);
int ParseInputBox(const uchar *pucXmlOneLine, uchar *pucOutData, int iOffset);
int ParsePictureBox(const uchar *pucXmlOneLine, uchar *pucOutData, int iOffset);
int ParseMenu(const uchar *pucXmlOneLine, uchar *pucOutData, int iOffset);
int ParseUi(const uchar *pucPageName, uchar *pucOutData, int iOutBufSize);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#pragma pack()


#endif

