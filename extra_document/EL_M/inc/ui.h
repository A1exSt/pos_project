//============================================================================
// Name        : ui.h
// Author      :tuxq
// Version     :
// Copyright   : Your copyright notice
// Description : Ansi-style
//============================================================================
#ifndef PRINTER_H
#define PRINTER_H

#include "posapi.h"

#define TRUE 	1
#define FALSE 	0

#define PAGENAME_LENGTH_MAX 		32
#define WIDGETNAME_LENGTH_MAX    	32

#define WIDGETVALUE_LENGTH_MAX	32

#define WIDGET_NUM_MAX			10

#define CMD_INFO_TITLE				"title"             //标题
#define CMD_INFO_VALUE				"value"           //当前值
#define CMD_INFO_EDIT                  	"edit"
#define CMD_INFO_PROMPT			"prompt"        //显示的字符串
#define CMD_INFO_LOGO				"logo"

#define FONT_0_MAX	21
#define FONT_1_MAX  16

#define TAG_POS		1
#define TAG_MOBILE	2

#define NUM			0xA0
#define ALPHA		0x90
#define DECIMAL		0xA2
#define PASSWARD	0xB8
#define PIN_PAD		0xD0

//return code
enum UI_RET													//scanner module		
{
	EL_UI_RET_BASE						=(2000),
	EL_UI_RET_INVALID_WIDGETNAME		=(EL_UI_RET_BASE + 1),	//invalid widget name
	EL_UI_RET_TIME_OUT					=(EL_UI_RET_BASE + 2),	//time out
	EL_UI_RET_INVALID_PAGE			=(EL_UI_RET_BASE +3),	//invalid page
	EL_UI_RET_PARSEUI_FAILED			=(EL_UI_RET_BASE +4),	//parseui failed
	EL_UI_RET_VALUESIZEERROR			=(EL_UI_RET_BASE +5),	//widget value size error
	EL_UI_RET_INPUT_TYPE_ERROR 		=(EL_UI_RET_BASE + 6),	//input type not find
	EL_UI_RET_INVALID_WIDGETVALUE 	=(EL_UI_RET_BASE + 7),	//invalid widget value
	EL_UI_RET_USER_CANCEL				=(EL_UI_RET_BASE + 8), 	//cancel exit
	EL_UI_RET_MENUITEMNUM_ERROR		=(EL_UI_RET_BASE + 9),	//menuitem number = 0
};

//widget type
typedef enum{
	TEXTBOX = 1,
	PICTUREBOX,
	INPUTBOX,
	MENUITEM,
}ENUM_WIDGET_INDEX;

//widget attributes
typedef enum {
	NAME = 1,
	X,
	Y,
	VALUE,
	FONTSIZE,
	TEXTALIGN,
	MODE,
	KEYACCEPT,
	MINLEN,
	MAXLEN,
}ENUM_WIDGET_ATTR;

//align type
typedef enum{
	ALIGN_LEFT = 1,
	ALIGN_CENTER,
	ALIGN_RIGHT,
}ENUM_ALIGN_ATTR;

//key type
typedef enum{
	BUTTON_NONE = 1,
	BUTTON_ENTER,
	BUTTON_CANCEL,
	BUTTON_ENTER_CANCEL,
}ENUM_KEYACCEPT;

//input type
typedef enum{
	INPUT_TYPE_NUM = 1,
	INPUT_TYPE_ALPHA,
	INPUT_TYPE_DECIMAL,
	INPUT_TYPE_PASSWARD,
	INPUT_TYPE_PINPAD,
}ENUM_INPUTTYPE;

//textbox attributes
typedef struct {
	uchar name[WIDGETNAME_LENGTH_MAX];
	uchar x;
	uchar y;
	uchar value[WIDGETVALUE_LENGTH_MAX];
	uchar fontsize;
	uchar textalign;
	uchar mode;
	uchar keyaccept;
}ST_TEXTBOX_WIDGET;

//picturebox attributes
typedef struct {
	uchar name[WIDGETNAME_LENGTH_MAX];		//widget name
	uchar x;
	uchar y;
	uchar value[WIDGETVALUE_LENGTH_MAX];		//picture path
}ST_PICTURE_WIDGET;

//inputbox attributes
typedef struct {
	uchar name[WIDGETNAME_LENGTH_MAX];
	uchar x;
	uchar y;
	uchar value[WIDGETVALUE_LENGTH_MAX];
	uchar fontsize;
	uchar textalign;
	uchar mode;
	uchar type;
	uchar keyaccept;//delete
	uchar minlen;
	uchar maxlen;
}ST_INPUTBOX_WIDGET;

//menu attributes
typedef struct {
	uchar name[WIDGETNAME_LENGTH_MAX];
	uchar x;
	uchar y;
	uchar value[WIDGETVALUE_LENGTH_MAX];
	uchar fontsize;
	uchar textalign;
}ST_MENU_WIDGET;

typedef struct _st_disp{
	uchar ucWidgetName[WIDGETNAME_LENGTH_MAX];
	uchar ucText[WIDGETVALUE_LENGTH_MAX];
}ST_DISP;

typedef struct _st_picture{
	uchar ucWidgetName[WIDGETNAME_LENGTH_MAX];
	uchar ucPicture[1024];
}ST_PICTURE;


typedef struct _st_inputtype{
	uchar ucTypeIndex;
	uchar ucTypeValue;
}ST_INPUTTYPE;

typedef struct _st_data{
	uchar ucWidgetName[WIDGETNAME_LENGTH_MAX];
	uchar ucWidgetValue[WIDGETVALUE_LENGTH_MAX];
}ST_DATA;

int EShowMsgBox(const uchar *pucPageName, const ST_DATA  *pucMsg, const ushort usTimeout);
int EShowInputBox(const uchar *pucPageName, const ST_DATA  *pucMsg, uchar *pucResponse, const int iRspBuffLen, const ushort usTimeout);
int EShowMenuList(const uchar *pucPageName, const ST_DATA  *pucMsg,  int *piSelect,const ushort usTimeout);

int ShowMsgBox(const uchar * pucRequest);

static int SetShowAlign(uchar ucXLocation, char * pszInfo, int iAlign, int iFontSize);
static void SetCustomizedFont(int iFontType);
static int SetElement(const uchar *buff, const ST_DATA *pucMsg, uchar *pucKeyAccept);
#endif
