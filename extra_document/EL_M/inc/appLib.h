

#ifndef _APPLIB_H
#define _APPLIB_H
#include <posapi.h>



#ifndef TRUE
#define TRUE	1
#define FALSE	0
#endif

#ifndef uchar
	typedef unsigned char	uchar;
#endif

#ifndef ushort
	typedef unsigned short	ushort;
#endif

#ifndef uint
	typedef unsigned int	uint;
#endif

#ifndef ulong
	typedef unsigned long	ulong;
#endif


// macros for screen functions
#define NUM_MAXZHCHARS      16          // 每行最多字符数目(大字体)
#define NUM_MAXCOLS         128         // 每行最多列数
#define NUM_MAXCOLNO        (NUM_MAXCOLS-1)     // 最大列号
#define NUM_MAXLINES        6           // 最多行数
#define NUM_MAXLINENO       (NUM_MAXLINES-1)    // 最大行号
#define NUM_MAXZHLINES      3           // 最大中文行数
#define NUM_MAXCHARS		21

#define DISP_LINE_LEFT		0x80
#define DISP_LINE_RIGHT		0x40
#define DISP_LINE_CENTER	0x20
#define DISP_SCR_CENTER		0x0100
#define DISP_LINE_REVER		0x0200
#define DISP_PART_REVER		0x0400

// menu display mode
#define MENU_NOSPEC		0x00	// 自动显示
#define MENU_USESNO		0x01	// 使用菜单自己的编号(优先于MENU_AUTOSNO)
#define MENU_AUTOSNO	0x02	// 自动编号
#define MENU_CENTER		0x10	// 中间对齐显示
#define MENU_PROMPT		0x20	// 强制提示信息在底部
#define MENU_ASCII		0x40	// 强制显示小字体
#define MENU_CFONT		0x80	// 强制显示大字体(优先于MENU_ASCII)
#define TIMER_TEMPORARY			4       // 短时间的定时器(其他模块可以共用)
#define TIMER_TMP				3		// added by Idina 2015-08-04


#define     APPLIB_SHIFT_RIGHT             0x00
#define     APPLIB_SHIFT_CENTER            0x01
#define     APPLIB_SHIFT_LEFT              0x02
#define     APPLIB_EFT_REVERSE             0x80


// macros for debug output
#define DEVICE_SCR   0x01
#define DEVICE_PRN   0x02
#define DEVICE_COM1  0x04
#define DEVICE_COM2  0x08

#define ASC_MODE	0x10
#define HEX_MODE	0x20
#define ISO_MODE	0x40
#define TLV_MODE	0x80


// macros for vDes()
#define ONE_DECRYPT     0           // 单DES解密
#define ONE_ENCRYPT     1           // 单DES加密
#define TRI_DECRYPT     2           // 三DES解密(16 bytes key)
#define TRI_ENCRYPT     3           // 三DES加密(16 bytes key)
#define TRI_DECRYPT3    4           // 三DES解密(24 bytes key)
#define TRI_ENCRYPT3    5           // 三DES加密(24 bytes key)

// macros for vCalcMac()
#define MAC_ANSIX99     0           // ANSI9.9 standard MAC algorithm
#define MAC_FAST        1           // HyperCom fast MAC algorithm

#define DISP_LINE_LEFT		0x80
#define DISP_LINE_RIGHT		0x40
#define DISP_LINE_CENTER	0x20
#define DISP_SCR_CENTER		0x0100
#define DISP_LINE_REVER		0x0200
#define DISP_PART_REVER		0x0400



typedef void (*MenuFunc)(void);
typedef struct _tagMenuItem{
	uchar		bVisible;						// TRUE: show it, FALSE: hide
    uchar       szMenuName[NUM_MAXCHARS+1];		// menu name
    MenuFunc    pfMenuFunc;						// callback function(if set)
}MenuItem;

//智能菜单相关定义

//每行容纳的字符数（以英文字符计算，汉字占2个位置）
#define SM_LINE_WIDTH  16
//最大菜单行数
#define SM_MAX_LINE    32

/*
 * 单个菜单项
 */
struct SMART_MENU;

typedef struct MENU_ITEM{
    char                pszText[NUM_MAXZHCHARS];//*pszText;  /*菜单文字提示*/
    int                 iValue;      /*对应的交易号*/    
    struct SMART_MENU * pstSubMenu;   /*子菜单，如无，则为NULL*/
}MENU_ITEM;


typedef struct SMART_MENU{
    MENU_ITEM       *pstContent;
    int             iNum;        /*菜单项数 */
    uchar			sMask[SM_MAX_LINE];
}SMART_MENU;



#define  MENU_ICON_UP       7   /*up 向上*/
#define  MENU_ICON_DOWN     8   /*down 向下*/
    /*action - 指定动作,    1:点亮，0:熄灭。*/
#define  MENU_CLOSEICON     0   /*关闭图标[针对所有图标]*/
#define  MENU_OPENICON      1   /* 显示图标[针对打印机、IC卡、锁、扬声器、向上、向下]*/

//每行显示一个菜单项（缺省显示两个）
#define SM_1ITEM_PER_LINE 0x00000001

//使用上下箭头和确认键选择（缺省以数字键选择）
#define SM_ARROW_SEL      (0x00000001<<1)

//从0开始计算菜单项，SM_ARROW_SEL置位时无效（缺省从1开始）
#define SM_ZERO_START     (0x00000001<<2)

//居中显示标题
#define SM_MIDDLE_TITLE   (0x00000001<<3)

//居中显示菜单项
#define SM_MIDDLE_ITEM    (0x00000001<<4)

//按取消键直接返回
#define SM_DIRECT_RETURN  (0x00000001<<5)

#define SM_ON   1
#define SM_OFF  0


// Support right-to-left language
// This is the lib's default right-to-left display function
// But for language like Farsi, should use external function:
//     customize the function in your application like this:
//     
//
typedef uchar (*FNPTR_ShowStringRightToLeft)(uchar lineno, uchar Mode, uchar *str);

typedef struct _tagAPPLIB_CONFIG 
{
    uchar   bStrFromRightToLeft;
    FNPTR_ShowStringRightToLeft pFnShowLeftRightString;
}APPLIB_CONFIG;

uchar LibDefault_DispLeftRightString(uchar lineno, uchar Mode, uchar *str);


// AppLib's default config:
//     display direction is from left to right
//     right-to-left display function is set to lib's default : LibDefault_DispLeftRightString() )
static APPLIB_CONFIG sg_stAppLibConfig = {0, LibDefault_DispLeftRightString};


#define DISP_MAX_LEN 21				// added by idina 201506-18 




#define MAX(a, b)       ( (a)>=(b) ? (a) : (b) )
#define MIN(a, b)       ( (a)<=(b) ? (a) : (b) )

// ========= Added by Idina 2015-09-14 ============== //
#define MPOS_INPUT_ALLOW_ZERO_IN		0x80
#define MPOS_INPUT_BIG_IN				0x40
#define MPOS_INPUT_AMOUNT_IN			0x20
#define MPOS_INPUT_NUM_IN				0x01    // 数字输入
#define MPOS_INPUT_ALPHA_IN				0x02    // 字母数字混合输入
#define MPOS_INPUT_PASS_IN				0x04    // 秘密输入
#define MPOS_INPUT_CARRY_IN				0x08    // 有传入数据
#define MPOS_INPUT_ECHO_IN				0x10    // 回显传入数据,按键则清除原数据
#define MPOS_INPUT_LEN_GETSTRING		512     // for PubGetString() use

// ========= Added end ==========================  //

ulong PubAsc2Long(uchar *psString, uint uiLength, ulong *pulOut);
void PubLong2Char(ulong ulSource, uint uiTCnt, uchar *psTarget);
ulong PubChar2Long(uchar *psSource, uint uiSCnt, ulong *pulTarget);
void PubLong2Bcd(ulong ulSource, uint uiTCnt, uchar *psTarget);
ulong PubBcd2Long(uchar *psSource, uint uiSCnt, ulong *pulTarget);


void PubBeepErr(void);
void setCustomizedFont(int fontType);

void PubDispString(void *pszStr, ushort nPosition);
uchar PubGetString(uchar ucMode, uchar ucMinlen, uchar ucMaxlen, uchar *pszOut, uchar ucTimeOut); // added by Idina 2015-09-14

void PubShowTitle(uchar bClrScreen, uchar *pszTitle);
uchar PubKeyPressed(void);
uchar PubWaitKey(short iWaitTime);
int PubGetMenu(uchar *pszPrompt, MenuItem *pstMenu, uchar ucMode, ushort nTimeOut);
void PubBcd2Asc0(uchar *psIn, uint uiLength, uchar *pszOut);
void PubTrimTailChars(uchar *pszString, uchar ucRemoveChar);
void PubBcd2Asc(uchar *psIn, uint uiLength, uchar *psOut);

int GetPanFromTrack2(char *cardno ,uchar *pszPAN);
int PubExtractPAN(uchar *pszPAN, uchar *pszOutPan);
void PubAsc2Bcd(uchar *psIn, uint uiLength, uchar *psOut);
void PubBcd2Asc0(uchar *psIn, uint uiLength, uchar *pszOut);
 void PubTrimTailChars(uchar *pszString, uchar ucRemoveChar);


int SelectMenu(uchar *pszPrompt, int iItemNum, MenuItem *pstMenu, unsigned short uiTimeOut); // added by idina 2015-06-18

/********************************************************************************
功能: 智能菜单
输入参数: pstSmartMenu   - 智能菜单指针
          uiMode - 菜单显示模式 可以为以下参数的组合，设为0则取采用缺省方式
                 SM_1ITEM_PER_LINE 每行显示一个菜单项
                （缺省显示两个,但SM_ARROW_SEL模式下强制每行一个）
                 SM_ARROW_SEL  使用上下箭头和确认键选择（缺省以数字键选择）
                 SM_ZERO_START  从0开始计算菜单项(SM_ARROW_SEL模式下无效)
                （缺省从1开始）
                 SM_MIDDLE_TITLE   居中显示标题 (缺省居左)
                 SM_MIDDLE_ITEM  居中显示菜单项(缺省居左)
        iTimeout - 超时时间（以秒为单位），设为零表示无超时 
输出参数: 无
返回:    >0   选定的交易号
         -1   取消（包括超时）
 ********************************************************************************/
int PubSmartMenuEx(SMART_MENU *pstSmartMenu, uint uiMode, int iTimeout);


/********************************************************************************
 * 函数名: PubSmInit
 * 功  能: 绑定只能菜单名称和内容
 * 入  参: pstMenuItem	- 初始菜单内容
 * 出  参: pstSmartMenu	- 智能菜单指针
 * 返回值: 无
 ********************************************************************************/
void PubSmInit(SMART_MENU *pstSmartMenu, MENU_ITEM *pstMenuItem);


/************************************************************************
功能: 按菜单名称开关菜单项目
输入参数: pszName		- 菜单项名称
          iFlag			- 操作 SM_ON / SM_OFF
输出参数: pstSmartMenu	- 菜单指针
返回:无
************************************************************************/
void PubSmMask(SMART_MENU *pstSmartMenu, const char *pszName, int iFlag);



#ifdef _POS_DEBUG

/****************************************************************************
功能： 	显示文件名和行数
输入参数：	pszFileName:       显示的文件名
                ulLineNo：         显示的行数
输出参数：	无
返回：  	无
****************************************************************************/
void DispAssert(uchar *pszFileName, ulong ulLineNo);

    // debug macro for boolean expression
	#define _POS_DEBUG_WAIT		15
    #define PubASSERT(expr)                                           \
                if( !(expr) ){                                      \
                    DispAssert((uchar *)__FILE__, (ulong)(__LINE__));       \
                }

    // print string debug information
    #define PubTRACE0(sz)                                             \
            {                                                       \
                ScrCls();                                           \
                ScrPrint(0, 0, CFONT, "%s", sz);                    \
                PubWaitKey(_POS_DEBUG_WAIT);							\
            }

    #define PubTRACE1(sz, p1)                                         \
            {                                                       \
                ScrCls();                                           \
                ScrPrint(0, 0, CFONT, sz, p1);                      \
                PubWaitKey(_POS_DEBUG_WAIT);							\
            }

    #define PubTRACE2(sz, p1, p2)                                     \
            {                                                       \
                ScrCls();                                           \
                ScrPrint(0, 0, CFONT, sz, p1, p2);                  \
                PubWaitKey(_POS_DEBUG_WAIT);							\
            }

    #define PubTRACE3(sz, p1, p2, p3)                                 \
            {                                                       \
                ScrCls();                                           \
                ScrPrint(0, 0, CFONT, sz, p1, p2, p3);              \
                PubWaitKey(_POS_DEBUG_WAIT);							\
            }

    #define PubTRACEHEX(t, s, l)    DispHexMsg((t), (s), (l), _POS_DEBUG_WAIT)

#else /* _POS_DEBUG */

    #define PubASSERT(expr)
    #define PubTRACE0(sz)
    #define PubTRACE1(sz, p1)
    #define PubTRACE2(sz, p1, p2)
    #define PubTRACE3(sz, p1, p2, p3)
    #define PubTRACEHEX(t, s, l)

#endif /* _POS_DEBUG */


/****************************************************************************
 Function:      多功能DES加密/解密运算
 Param In:
    ucMode      DES加密/解密模式:
                ONE_ENCRYPT --> 单DES加密
                ONE_DECRYPT --> 单DES解密
                TRI_ENCRYPT --> 三DES加密
                TRI_DECRYPT --> 三DES解密
    psData:     待加密/解密的数据(8字节)
    psKey:      DES运算的密钥(单倍长度或者两倍长度,根据ucMode确定)
 Param Out:
    psResult    DES运算的结果(8字节)
 Return Code:   none
****************************************************************************/
void PubDes(uchar ucMode, uchar *psData, uchar *psKey, uchar *psResult);	//

/****************************************************************************
 Function:      报文MAC运算函数
 Param In:
    ucMode      报文MAC运算模式:
                MAC_FAST    --> 快速MAC运算(HyperCom快速方式)
                MAC_ANSIX99 --> ANSI x9.9 方式
    psKey:      MAC运算的密钥(8字节)
    psMsg:      计算MAC的数据
    uiLength:   计算MAC的数据的长度
 Param Out:
    psMAC       报文的MAC(8字节)
 Return Code:   none
****************************************************************************/
void PubCalcMac(uchar ucMode, uchar *psKey, uchar *psMsg, uint uiLen, uchar *psMAC);
	
/****************************************************************************
功能： 	        计算LRC
输入参数	psData    需要计算的数据
                uiLength  数据的长度
输出参数	*pucInit  LRC值
返回：  	无
****************************************************************************/
void PubCalcLRC(uchar *psData, ushort uiLength, uchar *pucInit);

uchar PubYesNo(ushort nWaitTime);

void PubShowMsg(uchar ucLine, uchar *pszMsg);

#endif

