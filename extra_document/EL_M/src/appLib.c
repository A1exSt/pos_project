
#include "..\inc\global.h"

static char *pszAlphaTbl[10] =
{
	"0,*# ~`!@$%^&-+=(){}[]<>_|\\:;\"\'?/",
	"1QqZz.", "2AaBbCc", "3DdEeFf",
	"4GgHhIi", "5JjKkLl", "6MmNnOo",
	"7PpRrSs", "8TtUuVv", "9WwXxYy",
};
static uchar sgMinLineNo;

#define Attr_a			0
#define Attr_b			1
#define Attr_n			2
#define Attr_z			3
#define Attr_UnUsed		4
#define Attr_Over		5

#define Attr_var1		0
#define Attr_var2		1
#define Attr_fix		2


/********************** Internal structure declaration *********************/
typedef struct _tagFIELD_ATTR
{
	uchar	eElementAttr;
	uchar	eLengthAttr;
	uint		uiLength;
} FIELD_ATTR;


/********************** Internal functions declaration *********************/
static void GetBaseName(uchar *pszFullPath, uchar *pszBaseName);
static void MapChar2Index(uchar ch, int *piRow, int *piCol);
static void ShowWorkBuf(uchar ucMode, uchar *pszWorkBuff, int iLen);
static uchar IsLeapYear(unsigned long ulYear);

//static int UnPackElement(FIELD_ATTR *pAttr, uchar *pusIn, uchar *pusOut,uint *puiInLen);
static uchar ScrSend(uchar *psData, ushort nDataLen, ushort nStartLine);
static uchar PortSendstr(uchar ucChannel, uchar *psStr, ushort nStrLen);
static uchar UnitSend(uchar *psData, ushort nDataLen, uchar ucDevice, uchar ucMode, ushort nStartLine);
static void disp_text(unsigned char *mess,unsigned char line,unsigned char loc, unsigned char inkey,unsigned char back);
static int prepare_screen_text(SMART_MENU * sm, int start_index, int hilight, unsigned int mode, unsigned char with_disp, int * scr_item_num,  int * itemList);
static int GetNextHiliIndex(SMART_MENU *sm, int hilight_index);
static int GetPrevHiliIndex(SMART_MENU *sm, int hilight_index);
static int GetFirstHiliIndex(SMART_MENU *pstSmartMenu);

uchar LibDefault_DispLeftRightString(uchar lineno, uchar Mode, uchar *str);

void PubDispString(void *pszStr, ushort nPosition);

//高位在前的字符串表示转换为整型数
//Convert the character string with high bit in the front to integer
ulong PubChar2Long(uchar *psSource, uint uiSCnt, ulong *pulTarget)
{
	uint    i;
	ulong ulTmp;

	if (psSource == NULL)
	{
		return 0;
	}

	ulTmp = 0L;
	for(i = 0; i < uiSCnt; i++)
	{
		ulTmp |= ((ulong)psSource[i] << 8 * (uiSCnt - i - 1));
	}

	if (pulTarget != NULL)
	{
		*pulTarget = ulTmp;
	}
	return ulTmp;
}

//将整数转换为BCD格式的串存放,1234 --> 0x12 0x34
//Converted integer to BCD string,1234 --> 0x12 0x34
void PubLong2Bcd(ulong ulSource, uint uiTCnt, uchar *psTarget)
{
	uchar    szFmt[30], szBuf[30];

	sprintf((char*)szFmt, "%%0%dlu", uiTCnt*2);
	sprintf((char*)szBuf, (char *)szFmt, ulSource);
	if( psTarget != NULL )
	{
		PubAsc2Bcd(szBuf, uiTCnt * 2, psTarget);
	}
}

//转换BCD串为整数
//Convert BCD string to integer
ulong PubBcd2Long(uchar *psSource, uint uiSCnt, ulong *pulTarget)
{
	uchar   szBuf[30]={0};
	ulong   ulTmp;

	if ((psSource == NULL))
	{
		return 0;
	}

	PubBcd2Asc0(psSource, uiSCnt, szBuf);

	ulTmp = (ulong)atol((char *)szBuf);
	if( pulTarget != NULL )
	{
		*pulTarget = ulTmp;
	}

	return ulTmp;
}

//将数字串转变成长整数,相当于atol(),但是该函数不要求零字符结束
//Convert number string to long integer, similar to atol(). This function don't request end with '\0'.
ulong PubAsc2Long(uchar *psString, uint uiLength, ulong *pulOut)
{
	uchar    szBuff[15+1];
	ulong    ulTmp;

	sprintf((char *)szBuff, "%.*s", uiLength <= 15 ? uiLength : 15, psString);
	ulTmp =  (ulong)atol((char*)szBuff);
	if (pulOut != NULL)
	{
		*pulOut = ulTmp;
	}
	return ulTmp;
}

//整型数转换成高位在前的字符串表示(按照网络字节格式存放)
//Convert integer to string which high bit at the front. (Store it according to network byte format)
void PubLong2Char(ulong ulSource, uint uiTCnt, uchar *psTarget)
{
	uint    i;

	for(i = 0; i < uiTCnt; i++)
	{
		psTarget[i] = (uchar)(ulSource >> (8 * (uiTCnt - i - 1)));
	}
}

void setCustomizedFont(int fontType)
{
	ST_FONT singleFont, doubleFont;

	memset(&singleFont, 0, sizeof(ST_FONT));
	memset(&doubleFont, 0, sizeof(ST_FONT));

	singleFont.CharSet = CHARSET_WEST;
	doubleFont.CharSet = CHARSET_GB2312;

	if (0 == fontType){
		singleFont.Width = 6;
		singleFont.Height = 8;
		doubleFont.Width = 16;
		doubleFont.Height = 16;
	}
	else if (1 == fontType){
		singleFont.Width = 8;
		singleFont.Height = 16;
		doubleFont.Width = 16;
		doubleFont.Height = 16;
	}
	else{
		singleFont.Width = 12;
		singleFont.Height = 24;
		doubleFont.Width = 24;
		doubleFont.Height = 24;
	}

	ScrSelectFont(&singleFont, &doubleFont);
}


int GetPanFromTrack2(char *cardno ,uchar *pszPAN)
{
	int   iPanLen;
	char *p, pszTemp[41];
	int iLen=0, i=0;


	memset(pszTemp, 0, sizeof(pszTemp));

	iLen = strlen(cardno);

	for(i=0;i<iLen;i++)
	{
		if(cardno[i] == 'D') //实际读到的数据时D，但是标准要求是=. 因此要替换
		{
			cardno[i] = '=';
			break;
		}
	}

	memcpy(pszTemp, cardno, iLen);



	p = strchr((char *)pszTemp, '=');

	if( p==NULL )
	{
		return -1;
	}
	iPanLen = strlen(pszTemp) - strlen(p);

	if( iPanLen<13 || iPanLen>19 )
	{
		return -1;
	}

	sprintf((char *)pszPAN, "%.*s", iPanLen, pszTemp);

	return 0;
}


void PubShowTitle(uchar bClrScreen, uchar *pszTitle)
{
	int	 iLen;

	//	PubASSERT(pszTitle!=NULL);
	if (pszTitle==NULL)
	{
		return;
	}

	if (bClrScreen)
	{
		ScrCls();
	}

	iLen = strlen((char *)pszTitle);
	if (iLen > NUM_MAXZHCHARS)
	{
		iLen = NUM_MAXZHCHARS;
	}

	// Support right-to-left language
	if (sg_stAppLibConfig.bStrFromRightToLeft)
	{
		//pszTitle[iLen] = 0;
		sg_stAppLibConfig.pFnShowLeftRightString(0, APPLIB_SHIFT_CENTER|APPLIB_EFT_REVERSE, pszTitle);
	}
	else
	{
#ifdef _D_SERIES_
		DtScrSetRowFont(0, 0, CFONT|REVER);
		ScrGotoxyEx(4,10);//hdadd
		Lcdprintf("%*s", NUM_MAXZHCHARS+10, "");
		DtScrSetRowFont((uchar)((NUM_MAXCOLS-8*iLen)/2), 0, CFONT|REVER);
		Lcdprintf("%.*s", iLen, (char *)pszTitle);
#else
		ScrPrint(0, 0, CFONT|REVER, "%*s", NUM_MAXZHCHARS, "");
		ScrPrint((uchar)((NUM_MAXCOLS-8*iLen)/2), 0, CFONT|REVER,
			"%.*s", iLen, (char *)pszTitle);
#endif
	}
}
uchar LibDefault_DispLeftRightString(uchar lineno, uchar Mode, uchar *str)
{
	ushort nModeOfPubDispStr = 0;

	if ((Mode & 0xF0)==APPLIB_EFT_REVERSE)
	{
		nModeOfPubDispStr |= DISP_LINE_REVER;
	}
	if ((Mode & 0x0F)==APPLIB_SHIFT_LEFT)
	{
		nModeOfPubDispStr |= DISP_LINE_LEFT;
	}
	if ((Mode & 0x0F)==APPLIB_SHIFT_RIGHT)
	{
		nModeOfPubDispStr |= DISP_LINE_RIGHT;
	}
	if ((Mode & 0x0F)==APPLIB_SHIFT_CENTER)
	{
		nModeOfPubDispStr |= DISP_LINE_CENTER;
	}
	PubDispString(str, nModeOfPubDispStr);
	return 0;
}



uchar PubKeyPressed(void)
{
	return (kbhit()==0x00);
}


uchar PubWaitKey(short iWaitTime)
{
	uchar   ucKey, bChkTimer;

	if (iWaitTime>=0)
	{
		bChkTimer = TRUE;
		TimerSet(TIMER_TEMPORARY, (short)(iWaitTime*10));
	}
	else
	{
		bChkTimer = FALSE;
	}

	while (1)
	{
		if (PubKeyPressed())
		{
			ucKey = getkey();
			break;
		}
		else if (bChkTimer && TimerCheck(TIMER_TEMPORARY)==0)
		{
			ucKey = NOKEY;
			break;
		}
	}   // end of while (1)

	return ucKey;
}

/*
void PubDispString(void *pszStr, ushort nPosition)
{
	uchar mode, x, y, buffer[60];
	uchar ucReverFlag;
	uint	uiLen=0;
	
	if (pszStr == NULL)
	{
		return;
	}
	
	mode  = nPosition & 0x00f0;
	y     = nPosition & 0x000f;

	uiLen = strlen(pszStr);
	if (uiLen > NUM_MAXZHCHARS)
	{
		uiLen = NUM_MAXZHCHARS;
	}

    // Support right-to-left language
	if (sg_stAppLibConfig.bStrFromRightToLeft)
	{
		switch(mode)
		{
		case DISP_LINE_CENTER:
			#ifdef _D_SERIES_
			DtScrClrLine(y, (uchar) (y + 1));
			#else
			ScrClrLine(y, (uchar) (y + 1));
			#endif
			x = (64 - uiLen * 4);
			mode = APPLIB_SHIFT_CENTER;
			break;
			
		case DISP_LINE_RIGHT:
			x = 0;
			mode = APPLIB_SHIFT_LEFT;
			break;
		case DISP_LINE_LEFT:
		default:
			x = (128 - uiLen * 8);
			mode = APPLIB_SHIFT_RIGHT;
			break;
			
		}
		if (nPosition&DISP_SCR_CENTER)
		{
			ScrCls();
			x = (64 - uiLen * 4);
			y = 3;
		}
		ucReverFlag = 0;
		if (nPosition&DISP_LINE_REVER || nPosition&DISP_PART_REVER)
		{
			mode |= APPLIB_EFT_REVERSE;
		}

		memset(buffer, 0, sizeof(buffer));
		memcpy(buffer, pszStr, uiLen);
		#ifdef _D_SERIES_
		DtScrClrLine(y, (uchar)(y+1));
		#else
		ScrClrLine(y, (uchar)(y+1));
		#endif
		//ScrPrint(x, y, (uchar)(CFONT|ucReverFlag), "%s", (char *)buffer);
		if (nPosition&DISP_LINE_REVER)
        {
            #ifdef _D_SERIES_
			DtScrSetRowFont(0, y, CFONT|REVER);
			Lcdprintf("%*s", NUM_MAXZHCHARS, "");
			#else
			ScrPrint(0, y, CFONT|REVER, "%*s", NUM_MAXZHCHARS, "");
			#endif
        }

        sg_stAppLibConfig.pFnShowLeftRightString(y, mode, buffer);
	}
	else
	{
		switch(mode)
		{
		case DISP_LINE_CENTER:
			#ifdef _D_SERIES_
			DtScrClrLine(y, (uchar) (y + 1));
			#else
			ScrClrLine(y, (uchar) (y + 1));
			#endif
			x = (64 - uiLen * 4);
			break;
			
		case DISP_LINE_RIGHT:
			x = (128 - uiLen * 8);
			break;
			
		case DISP_LINE_LEFT:
		default:
			x = 0;
			break;
		}

		if (nPosition&DISP_SCR_CENTER)
		{
			ScrCls();
			x = (64 - uiLen * 4);
			y = 3;
		}
		ucReverFlag = 0;
		if (nPosition&DISP_LINE_REVER || nPosition&DISP_PART_REVER)
		{
			ucReverFlag = REVER;
		}

		memset(buffer, 0, sizeof(buffer));
		memcpy(buffer, pszStr, uiLen);
	
		
		#ifdef _D_SERIES_
		DtScrClrLine(y, (uchar)(y+1));
		if (nPosition&DISP_LINE_REVER)
		{
			DtScrSetRowFont(0, y, CFONT|REVER);
			ScrGotoxyEx(4,(y*8)*3 +10);//hdadd
			Lcdprintf("%*s", NUM_MAXZHCHARS+10, "");//12*24font  26
		}
		DtScrSetRowFont(x, y, (uchar)(CFONT|ucReverFlag));
		//DtScrClrLine(y,y);
		Lcdprintf( "%s", (char *)buffer);
		#else
		ScrClrLine(y, (uchar)(y+1));
		if (nPosition&DISP_LINE_REVER)
			ScrPrint(0, y, CFONT|REVER, "%*s", NUM_MAXZHCHARS, "");
		ScrPrint(x, y, (uchar)(CFONT|ucReverFlag), "%s", (char *)buffer);
		#endif
	}
}

*/

void PubDispString(void *pszStr, ushort nPosition)
{
	uchar mode, x, y, ucLen, buffer[60];
	uchar ucReverFlag;

	if (pszStr == NULL)
	{
		return;
	}
	mode  = nPosition & 0x00f0;
	y     = nPosition & 0x000f;

	ucLen = strlen(pszStr);

	if (ucLen > NUM_MAXZHCHARS)
	{
		ucLen = NUM_MAXZHCHARS;
	}

	switch(mode)
	{
	case DISP_LINE_CENTER:
		ScrClrLine(y, (uchar) (y + 1));
		x = (64 - ucLen * 4);
		break;

	case DISP_LINE_RIGHT:
		x = (128 - ucLen * 8);
		break;

	case DISP_LINE_LEFT:
	default:
		x = 0;
		break;
	}

	if (nPosition & DISP_SCR_CENTER)
	{
		ScrCls();
		x = (64 - ucLen * 4);
		y = 3;
	}
	ucReverFlag = 0;

	if (nPosition & DISP_LINE_REVER || nPosition & DISP_PART_REVER)
	{
		ucReverFlag = REVER;
	}
	memset(buffer, 0, sizeof(buffer));
	memcpy(buffer, pszStr, ucLen);
	ScrClrLine(y, (uchar)(y + 1));

	if (nPosition & DISP_LINE_REVER)
	{
		ScrPrint(0, y, CFONT | REVER, "%*s", NUM_MAXZHCHARS, "");
	}
	ScrPrint(x, y, (uchar)(CFONT | ucReverFlag), "%s", (char *)buffer);
}


// ========= Added by Idina 2015-09-14 ============ //

// Get string/numeric string
uchar PubGetString(uchar ucMode, uchar ucMinlen, uchar ucMaxlen, uchar *pszOut, uchar ucTimeOut)
{
	uchar ucKey, bClearInData, szWorkBuff[MPOS_INPUT_LEN_GETSTRING + 1];
	int iLen, iIndex, iLastKey;

	if (ucMaxlen < ucMinlen)
	{
		return 0xFE;
	}

	if( (ucMode & MPOS_INPUT_CARRY_IN) || (ucMode & MPOS_INPUT_ECHO_IN) )
	{
		//      memcpy(szWorkBuff, pszOut, strlen(szWorkBuff));
		sprintf((char *)szWorkBuff, "%.*s", MPOS_INPUT_LEN_GETSTRING, pszOut);
	}
	else
	{
		memset(szWorkBuff, 0, sizeof(szWorkBuff));
	}
	iLen = strlen((char *)szWorkBuff);

	if( iLen > 0 )
	{
		MapChar2Index(szWorkBuff[iLen - 1], &iLastKey, &iIndex);
	}
	else
	{
		iLastKey = -1;
		iIndex   = 0;
	}
	sgMinLineNo  = 0xFF;
	bClearInData = (ucMode & MPOS_INPUT_ECHO_IN);

	while( 1 )
	{
		ShowWorkBuf(ucMode, szWorkBuff, iLen);
		ucKey = PubWaitKey(ucTimeOut);

		if( ucKey == KEYCANCEL)
		{
			return 0xFF;
		}
		if(ucKey == NOKEY)
		{
			return 0x01;
		}
		else if( ucKey == KEYENTER )
		{
			bClearInData = FALSE;
			if( iLen < (int)ucMinlen )
			{
				continue;
			}
			sprintf((char *)pszOut, "%s", szWorkBuff);
			break;
		}
		else if( ucKey >= KEY0 && ucKey <= KEY9 )
		{
			if( bClearInData )
			{
				// clear in buffer
				szWorkBuff[0] = 0;
				iLen          = 0;
				iLastKey      = -1;
				bClearInData  = FALSE;
			}

			// save key in data
			if( iLen < (int)ucMaxlen && iLen < MPOS_INPUT_LEN_GETSTRING)
			{
				szWorkBuff[iLen++] = ucKey;
				szWorkBuff[iLen]   = 0;
				iLastKey = ucKey - KEY0;
				iIndex   = 0;
			}
		}

		else if( ucKey == KEYCLEAR )
		{
			bClearInData = FALSE;

			if( iLen <= 0 )
			{
				continue;
			}
			szWorkBuff[--iLen] = 0;

			if( iLen > 0 )
			{
				MapChar2Index(szWorkBuff[iLen - 1], &iLastKey, &iIndex);
			}
			else
			{
				iLastKey = -1;
				iIndex   = 0;
			}
		}
		else if( ucKey == KEYALPHA )
		{
			bClearInData = FALSE;

			if( !(ucMode & MPOS_INPUT_ALPHA_IN) || iLastKey < 0 || iLen < 1 )
			{
				continue;
			}
			iIndex = (iIndex + 1) % strlen(pszAlphaTbl[iLastKey]);
			szWorkBuff[iLen - 1] = pszAlphaTbl[iLastKey][iIndex];
		}
	}

	return 0;
}


void ShowWorkBuf(uchar ucMode, uchar *pszWorkBuff, int iLen)
{
	int iLineNum, iCnt, iLeftBytes;
	uchar ucLineNo, sTemp[NUM_MAXZHCHARS];

	iLineNum = (iLen + NUM_MAXZHCHARS - 1) / NUM_MAXZHCHARS;

	if( iLineNum >= NUM_MAXZHLINES )
	{
		ScrCls();
	}
	else
	{
		sgMinLineNo = MIN(sgMinLineNo, (NUM_MAXLINENO - iLineNum * 2 + 1));
		ScrClrLine(sgMinLineNo, NUM_MAXLINENO);
	}
	memset(sTemp, '*', sizeof(sTemp));
	ucLineNo = NUM_MAXLINENO - 1;

	for(iCnt = 1; iCnt <= 3; iCnt++)
	{
		if( (iCnt * NUM_MAXZHCHARS) <= iLen )
		{
			iLeftBytes = NUM_MAXZHCHARS;

			if( ucMode & MPOS_INPUT_PASS_IN )
			{
				ScrPrint(0, ucLineNo, CFONT, "%.*s", iLeftBytes, sTemp);
			}
			else
			{
				ScrPrint(0, ucLineNo, CFONT, "%.*s", iLeftBytes, pszWorkBuff + iLen - iCnt * NUM_MAXZHCHARS);
			}
		}
		else
		{
			iLeftBytes = iLen - (iCnt - 1) * NUM_MAXZHCHARS;

			if( iLeftBytes <= 0 && iCnt > 1 )
			{
				break;
			}

			if( ucMode & MPOS_INPUT_PASS_IN )
			{
				ScrPrint(0, ucLineNo, CFONT, "%*.*s", NUM_MAXZHCHARS, iLeftBytes, sTemp);
			}
			else
			{
				ScrPrint(0, ucLineNo, CFONT, "%*.*s", NUM_MAXZHCHARS, iLeftBytes, pszWorkBuff);
			}
			break;
		}
		ucLineNo -= 2;
	}
}

void MapChar2Index(uchar ch, int *piRow, int *piCol)
{
	int i;
	char    *p;

	for(i = 0; i < sizeof(pszAlphaTbl) / sizeof(pszAlphaTbl[0]); i++)
	{
		for(p = pszAlphaTbl[i]; *p; p++)
		{
			if( *p == toupper(ch) )
			{
				*piRow = i;
				*piCol = p - pszAlphaTbl[i];
				break;
			}
		}
	}
}

// ========= Added end =========================//
//将二进制源串分解成双倍长度可读的16进制串,0x12AB-->"12AB"
//Convert BIN string to readable HEX string, which have double length of BIN string. 0x12AB-->"12AB"
void PubBcd2Asc(uchar *psIn, uint uiLength, uchar *psOut)
{
	static const uchar ucHexToChar[16] = {"0123456789ABCDEF"};
	uint   uiCnt;

	if ((psIn == NULL) || (psOut == NULL))
	{
		return;
	}

	for(uiCnt = 0; uiCnt < uiLength; uiCnt++)
	{
		psOut[2*uiCnt]   = ucHexToChar[(psIn[uiCnt] >> 4)];
		psOut[2*uiCnt + 1] = ucHexToChar[(psIn[uiCnt] & 0x0F)];
	}
}


// 同vOneTwo()函数,并在目标串后添一 '\0'
//Similar with function PubOne2Two(), and add '\0' at the end of object string
void PubBcd2Asc0(uchar *psIn, uint uiLength, uchar *pszOut)
{
	if ((psIn == NULL) || (pszOut == NULL))
	{
		return;
	}

	PubBcd2Asc(psIn, uiLength, pszOut);

	pszOut[2*uiLength] = 0;
}
//Delete the specified continuous characters on the right of the string
void PubTrimTailChars(uchar *pszString, uchar ucRemoveChar)
{
	int		i, iLen;

	if( !pszString || !*pszString )
	{
		return;
	}

	iLen = strlen((char *)pszString);
	for(i=iLen-1; i>=0; i--)
	{
		if( pszString[i]!=ucRemoveChar )
		{
			break;
		}
		pszString[i] = 0;
	}
}

int PubExtractPAN(uchar *pszPAN, uchar *pszOutPan)
{
	int iLength = 0;

	iLength = strlen((uchar*)pszPAN);
	if( iLength<13 || iLength>19 )
	{
		return -1;
	}

	memset(pszOutPan, '0', 16);
	memcpy(pszOutPan+4, pszPAN+iLength-13, 12);
	pszOutPan[16] = 0;

	return 0;
}

void PubAsc2Bcd(uchar *psIn, uint uiLength, uchar *psOut)
{
	uchar   tmp;
	uint    i;

	if ((psIn == NULL) || (psOut == NULL))
	{
		return;
	}

	for(i = 0; i < uiLength; i += 2)
	{
		tmp = psIn[i];
		if( tmp > '9' )
		{
			tmp = (uchar)toupper((int)tmp) - 'A' + 0x0A;
		}
		else
		{
			tmp &= 0x0F;
		}
		psOut[i / 2] = (tmp << 4);

		tmp = psIn[i+1];
		if( tmp > '9' )
		{
			tmp = toupper((char)tmp) - 'A' + 0x0A;
		}else
		{
			tmp &= 0x0F;
		}
		psOut[i/2] |= tmp;
	}
}



// added by idina 2015-06-18 
extern void ScrClrRect(int left, int top, int right, int bottom);	// This API is needed here. But why it's not defined in posapi.h ??
int SelectMenu(uchar *pszPrompt, int iItemNum, MenuItem *pstMenu, unsigned short uiTimeOut)
{
	int		iTotalPage, iCurrentPage;
	uchar	ucKey;
	char pDispStr[DISP_MAX_LEN+1]="Select App:";

	if ( (0 >= iItemNum) || (NULL == pstMenu) )
	{
		return -1;
	}

	iTotalPage = (iItemNum+1)/2; 	// Get the total page to show all items
	iCurrentPage = 0;
	ScrCls();
	if (NULL == pszPrompt)
	{
		//PubDispString(pDispStr, 0|DISP_LINE_LEFT);
		PubDispString(pDispStr, 0|DISP_LINE_REVER|DISP_LINE_CENTER);
	}
	else
	{
		//PubDispString(pszPrompt, 0|DISP_LINE_LEFT);
		PubDispString(pszPrompt, 0|DISP_LINE_REVER|DISP_LINE_CENTER);
	}
	while (1)
	{
		ScrClrLine(2, 5);	// clear the items on previous page
		//ScrClrRect(91, 0, 127, 16);		// clear the "up" "down" promption on previous page
		if (iCurrentPage*2 < iItemNum)
		{
			ScrPrint(0, 2, CFONT, "1.%.14s", pstMenu[iCurrentPage*2].szMenuName);
		}
		if (iCurrentPage*2+1 < iItemNum)
		{
			ScrPrint(0, 4, CFONT, "2.%.14s", pstMenu[iCurrentPage*2+1].szMenuName);
		}

		// take the last 36 dots of line 0 and line 1 to display prompt to tell user page down or page up.
		// 36 dots comes from:  strlen("6-DOWN") is 6 characters. With ASCII font, it will take 6*6 dots to display this.

		kbflush();

		if(uiTimeOut == 0)
		{
			ucKey = getkey();
		}
		else
		{
			ucKey = PubWaitKey(uiTimeOut);
		}	
		if (KEY1 == ucKey)
		{
			if (iCurrentPage*2 < iItemNum)
			{
				return iCurrentPage * 2;
			}
		}
		else if (KEY2 == ucKey)
		{
			if (iCurrentPage * 2 + 1 < iItemNum)
			{
				return iCurrentPage * 2 + 1;
			}
		}
		else if (KEYENTER== ucKey)	// key 6 to page down. if it's the last page, then turn to the first page
		{
			if ((iTotalPage-1) == iCurrentPage)
			{
				iCurrentPage = 0;
			}
			else
			{
				iCurrentPage++;
			}
			continue;
		}
		else if (NOKEY == ucKey)
		{
			return -2;
		}
		else if (KEYCANCEL == ucKey)
		{
			return -3;
		}
	} 

	return -1;
}
// added end

// ============ Added by Idina 2015-10-08 to support smart menu ======= //


void PubShowMsg(uchar ucLine, uchar *pszMsg)
{
	int		iLen;
	uchar	ucMode;

	PubASSERT(pszMsg!=NULL);
	if (pszMsg==NULL)
	{
		return;
	}

	ucMode =  (ucLine & REVER) | CFONT;
	ucLine &= ~REVER;
	ScrClrLine(ucLine, (uchar)(ucLine+1));

	iLen = strlen((char *)pszMsg);
	if (iLen > NUM_MAXZHCHARS)
	{
		iLen = NUM_MAXZHCHARS;
	}

	ScrPrint((uchar)((NUM_MAXCOLS-8*iLen)/2), ucLine, ucMode,
		"%.*s", iLen, (char *)pszMsg);
}

void PubShowTwoMsg(uchar ucLine, uchar *pszMsg1, uchar *pszMsg2)
{
	if (ucLine>4)
	{
		ucLine = 4;
	}
	PubShowMsg(ucLine, pszMsg1);
	PubShowMsg((uchar)(ucLine+2), pszMsg2);
}


// 格式化一屏显示的文本
// 返回下一屏第一个菜单项在sm->content中的索引（如果下一屏已经没有菜单，则返回sm->num）
// 参数 pstSmartMenu - 菜单
//      iStartIndex - 本屏第一个菜单项在sm->content中的索引
//      iHilightIndex - 需要高亮显示的菜单项在sm->content中的索引（只在SM_ARROW_SEL模式下有效）
//      uiMode-显示模式
//      ucWithDisp - 1 显示 0 不显示，只计算
//      piScrItemNum  本屏的菜单项个数，用于输出
static int prepare_screen_text(SMART_MENU *pstSmartMenu, int iStartIndex, int iHilightIndex, 
							   uint uiMode, uchar ucWithDisp, int *piScrItemNum, int * itemList)
{
	int iCurLine = 0;
	int iNumIndex;
	int i;
	int iCurItem;
	int iFlag = 0;
	char buf[SM_LINE_WIDTH * 2 +1];
	char buf1[SM_LINE_WIDTH * 2 +1];
	int iHilightLine=0;
	int iEndIndex;
	char menu_text[4][SM_LINE_WIDTH + 1];
	ushort nDispMode;
	ushort nReverFlag;
	int k = 0;

	memset(buf, 0x00, sizeof(buf));  
	memset(buf1, 0x00, sizeof(buf)); 

	*piScrItemNum = 0;

	for(i = 0; i < 4; i++) 
	{
		memset(menu_text[i], 0, sizeof(menu_text[i]));
	}

	//标题
	if(pstSmartMenu->pstContent[0].iValue < 0) 
	{
		if (strlen(pstSmartMenu->pstContent[0].pszText) > SM_LINE_WIDTH)
		{
			memcpy(menu_text[0], pstSmartMenu->pstContent[0].pszText, SM_LINE_WIDTH);
		}
		else
		{
			strcpy(menu_text[0], pstSmartMenu->pstContent[0].pszText);
		}
		iCurLine = 1;
	}

	if(uiMode & SM_ZERO_START)
	{
		iNumIndex = 0;
	}
	else
	{
		iNumIndex = 1;
	}

	iCurItem = iStartIndex;

	iEndIndex = pstSmartMenu->iNum;

	for(i = iCurItem; i < pstSmartMenu->iNum; i++) 
	{	
		if(pstSmartMenu->sMask[i] == SM_OFF) 
		{
			continue;
		}

		if(i == iHilightIndex)
		{
			iHilightLine = iCurLine;
		}

		itemList[k++] = i;

		if(uiMode & SM_ARROW_SEL)
		{
			sprintf(buf, "%s", pstSmartMenu->pstContent[i].pszText);
		}
		else
		{
			sprintf(buf, "%d-%s", iNumIndex, pstSmartMenu->pstContent[i].pszText);
		}

		/*处理超长的菜单项,但未考虑半个汉字问题:-(*/
		if(strlen(buf) > 16) 
		{
			buf[16] = 0;         
		}

		iNumIndex++;

		//使用方向键选择，强制每行一条
		if((uiMode & SM_1ITEM_PER_LINE) || (uiMode & SM_ARROW_SEL)) 
		{
			strcpy(menu_text[iCurLine], buf);
			(*piScrItemNum) ++;
			iCurLine++;
			if(iCurLine == 4) 
			{
				iEndIndex = i+1;
				break;
			}
		}
		else 
		{
			if(iFlag == 0) 
			{ //cur_line行当前无内容
				strcpy(menu_text[iCurLine], buf);
				(*piScrItemNum) ++;
				iFlag = 1;
			}
			else 
			{          //cur_line已有内容
				if(strlen(buf) + strlen(menu_text[iCurLine]) <= 16)
				{//要加入的项可以放入当前行
					strcpy(buf1, menu_text[iCurLine]);
					memset(menu_text[iCurLine], ' ', 16);
					memcpy(menu_text[iCurLine], buf1, strlen(buf1));
					memcpy(menu_text[iCurLine]+8, buf, strlen(buf));
					menu_text[iCurLine][16] = 0;
					iCurLine++;
					(*piScrItemNum) ++;
					if(iCurLine == 4)
					{
						iEndIndex = i+1;
						break;
					}
					iFlag = 0;
				}
				else
				{ //另起一行
					if(iCurLine == 3)
					{
						iEndIndex = i;
						break;
					}
					iCurLine++;
					strcpy(menu_text[iCurLine], buf);
					(*piScrItemNum) ++;
					iFlag = 1;
				}
			}
		}
	}

	if(ucWithDisp) 
	{
		for(i = 0; i < 4; i++) 
		{
			if(pstSmartMenu->pstContent[0].iValue < 0 && i ==0) 
			{	
				nDispMode = 0;
				if (uiMode & SM_MIDDLE_TITLE)
				{
					nDispMode |= DISP_LINE_CENTER;
				}

				PubDispString(menu_text[i],(ushort)((i*2)|nDispMode|DISP_LINE_REVER));
			}
			else
			{
				nDispMode = 0;
				if (uiMode & SM_MIDDLE_ITEM)
				{
					nDispMode |= DISP_LINE_CENTER;
				}

				if ((uchar)((i == iHilightLine) && (uiMode & SM_ARROW_SEL)))
				{
					nReverFlag = DISP_PART_REVER;
				}
				else
				{
					nReverFlag = 0;
				}

				PubDispString(menu_text[i],(ushort)((i*2)|nDispMode|nReverFlag));
			}
		}
	}

	//返回下一个有效菜单索引，如果没有就返回sm->num
	for(i = iEndIndex ; i < pstSmartMenu->iNum; i++) 
	{
		if(pstSmartMenu->sMask[i] == SM_ON) 
			break;
	}

	return i;
}


////////////////////////////////////// 智能菜单 //////////////////////////////////////

int PubSmartMenuEx(SMART_MENU *pstSmartMenu, uint uiMode, int iTimeout)
{

	int i;
	int iStartIndex;
	int iEndIndex;
	uchar ucKey;    
	int ScrIndex[10];	//记录每一屏的第一个菜单项在sm->content中的索引    
	int iCurScr = 0;	//当前的菜单所在的屏数   
	int iHilightIndex;	//当前的高亮项目索引
	int iRet;    
	uchar ucStartKey;
	int iScrItemNum;
	int itemList[16];
	int iTargetIdx;

	for(i = 0; i<16; i++) itemList[i] = 0;

	if (pstSmartMenu == NULL)
	{
		return -1;
	}

	if(pstSmartMenu->pstContent == NULL) 
	{
		PubShowTwoMsg(2, "SMART MENU", "NOT INIT'ED");
		getkey();
		return -1;
	}

	if(uiMode & SM_ZERO_START)
	{
		ucStartKey = KEY0;
	}
	else
	{
		ucStartKey = KEY1;
	}

	//处理标题
	if(pstSmartMenu->pstContent[0].iValue < 0) 
	{
		iStartIndex = 1;
	}
	else
	{
		iStartIndex = 0;
	}

	//找到第一个有效的菜单项
	iStartIndex = GetFirstHiliIndex(pstSmartMenu);
	if(iStartIndex < 0) 
	{
		return -1;
	}

	iHilightIndex = iStartIndex;

	//确定每一屏的第一个菜单项的索引
	for(i = 0; i < 10; i++)
	{
		ScrIndex[i] = -1;
	}

	ScrIndex[0] = iStartIndex;
	i = 1;
	while(iStartIndex < pstSmartMenu->iNum) 
	{
		iEndIndex = prepare_screen_text(pstSmartMenu, iStartIndex, iHilightIndex, uiMode, 0, &iScrItemNum, itemList);
		ScrIndex[i++] = iEndIndex;
		iStartIndex = iEndIndex;
	}

	iStartIndex = ScrIndex[0];
	iHilightIndex =iStartIndex;
	iCurScr = 0;
	while(1) 
	{
		iStartIndex = ScrIndex[iCurScr];
		iEndIndex = prepare_screen_text(pstSmartMenu, iStartIndex, iHilightIndex, uiMode, 1, &iScrItemNum, itemList);

		if(uiMode & SM_ARROW_SEL) 
		{
			if(iHilightIndex > ScrIndex[0]) 
			{
				ScrSetIcon(MENU_ICON_UP, MENU_OPENICON);
			}
			else 
			{
				ScrSetIcon(MENU_ICON_UP, MENU_CLOSEICON);
			}

			if((ScrIndex[iCurScr + 1] >= pstSmartMenu->iNum )
				&& (iHilightIndex == iStartIndex + iScrItemNum -1 ))
			{
				ScrSetIcon(MENU_ICON_DOWN, MENU_CLOSEICON);
			}
			else 
			{
				ScrSetIcon(MENU_ICON_DOWN, MENU_OPENICON);
			}
		}
		else 
		{
			if(iCurScr > 0) 
			{
				ScrSetIcon(MENU_ICON_UP, MENU_OPENICON);
			}
			else 
			{
				ScrSetIcon(MENU_ICON_UP, MENU_CLOSEICON);
			}

			if(ScrIndex[iCurScr + 1] >= pstSmartMenu->iNum )
			{
				ScrSetIcon(MENU_ICON_DOWN, MENU_CLOSEICON);
			}
			else 
			{
				ScrSetIcon(MENU_ICON_DOWN, MENU_OPENICON);
			}
		}

		if (iTimeout==0)
		{
			ucKey = PubWaitKey(-1);
		}
		else
		{
			ucKey = PubWaitKey((short)iTimeout);
		}

		if (ucKey == KEYCANCEL || ucKey == NOKEY)
		{
			return -1;
		}

		if(uiMode & SM_ARROW_SEL) 
		{
			if(ucKey == KEYENTER) 
			{
				if(pstSmartMenu->pstContent[iHilightIndex].pstSubMenu) 
				{
					iRet = PubSmartMenuEx(pstSmartMenu->pstContent[iHilightIndex].pstSubMenu, uiMode, iTimeout);
					if(uiMode & SM_DIRECT_RETURN) 
					{
						return iRet;
					}
					else if(iRet > 0)
					{
						return iRet;
					}
				}
				else 
				{
					return pstSmartMenu->pstContent[iHilightIndex].iValue;
				}
			}
			else if(ucKey == KEYUP) 
			{
				iHilightIndex = GetPrevHiliIndex(pstSmartMenu, iHilightIndex);
				//本屏还能显示
				if(iHilightIndex >= iStartIndex) 
				{
					continue;
				}
				//本屏不能显示
				else 
				{
					if(iCurScr > 0) 
					{
						for(i = iCurScr; i > 0; i++) 
						{
							if(iHilightIndex >= ScrIndex[i -1] && iHilightIndex < ScrIndex[i]) 
							{
								iCurScr = i - 1;
								break;
							}
						}
					}
					continue;
				}
			}
			else if(ucKey == KEYDOWN) 
			{
				iHilightIndex = GetNextHiliIndex(pstSmartMenu, iHilightIndex);

				for(i = iCurScr + 1; i < 10; i++) 
				{
					if(ScrIndex[i] == -1) 
					{
						iCurScr = i -1;
						break;
					}
					if(iHilightIndex >= ScrIndex[i -1] && iHilightIndex < ScrIndex[i]) 
					{
						iCurScr = i - 1;
						break;
					}   
				}
				continue;
			}
		}
		else 
		{
			if(ucKey == KEYUP) 
			{
				if(iCurScr > 0) 
				{
					iCurScr--;
				}

				iStartIndex = ScrIndex[iCurScr];
			}
			else if(ucKey == KEYDOWN || ucKey == KEYENTER) 
			{
				if(iEndIndex < pstSmartMenu->iNum) 
				{
					iCurScr++;
					iStartIndex = iEndIndex;    
				}
			}
			else if(ucKey >= ucStartKey && ucKey < ucStartKey + iScrItemNum ) 
			{
				iTargetIdx = itemList[ucKey - ucStartKey];
				if(pstSmartMenu->pstContent[iTargetIdx].pstSubMenu)
				{
					iRet = PubSmartMenuEx(pstSmartMenu->pstContent[iTargetIdx].pstSubMenu, uiMode, iTimeout);
					if(uiMode & SM_DIRECT_RETURN) 
					{
						return iRet;
					}
					else if(iRet > 0)
					{
						return iRet;
					}
				}
				else 
				{
					return pstSmartMenu->pstContent[iTargetIdx].iValue;
				}
			}
		}
	}	
}

/********************************************************************************
* 入  参: sm - 智能菜单指针
*        
* 出  参: 无
* 返回值: 第一有效菜单项，入无则返回-1
*          
********************************************************************************/
static int GetFirstHiliIndex(SMART_MENU *pstSmartMenu)
{
	int i = 0;
	int j;
	if(pstSmartMenu->pstContent->iValue < 0) i = 1;

	for(j = i; j < pstSmartMenu->iNum; j++) 
	{
		if(pstSmartMenu->sMask[j] == SM_ON) 
		{
			return j;
		}
	}

	return -1;
}

/********************************************************************************
* 函数名: GetNextHiliIndex
* 功  能: 返回下一个高亮显示的菜单项的索引
* 入  参: sm - 智能菜单指针
*         hilight_index - 当前高亮显示的菜单的索引
* 出  参: 无
* 返回值: 下一个高亮显示的菜单项的索引，如果没有下一个可供高亮显示的菜单
*          则返回当前的高亮菜单索引
********************************************************************************/
static int GetNextHiliIndex(SMART_MENU *pstSmartMenu, int iHilightIndex)
{
	int i = 0;

	for(i = iHilightIndex+1; i < pstSmartMenu->iNum; i++) 
	{
		if(pstSmartMenu->sMask[i] == SM_ON) 
		{
			return i;
		}
	}
	return iHilightIndex;

}

/********************************************************************************
* 函数名: GetNextHiliIndex
* 功  能: 返回上一个高亮显示的菜单项的索引
* 入  参: sm - 智能菜单指针
*         hilight_index - 当前高亮显示的菜单的索引
* 出  参: 无
* 返回值: 上一个高亮显示的菜单项的索引，如果没有上一个可供高亮显示的菜单
*          则返回当前的高亮菜单索引
********************************************************************************/
static int GetPrevHiliIndex(SMART_MENU *pstSmartMenu, int iHilightIndex)
{
	int iLow = 0;
	int i;

	if(pstSmartMenu->pstContent[0].iValue < 0)
	{
		iLow = 1;
	}

	if(iHilightIndex == iLow) 
	{
		return iHilightIndex;
	}

	for(i = iHilightIndex -1; i >= iLow; i--) 
	{
		if(pstSmartMenu->sMask[i] == SM_ON) 
		{
			return i;
		}
	}

	return iHilightIndex;
}


void PubSmInit(SMART_MENU *pstSmartMenu, MENU_ITEM *pstMenuItem)
{
	int i=0;

	if (pstSmartMenu == NULL || pstMenuItem == NULL)
	{
		return;
	}

	pstSmartMenu->pstContent = pstMenuItem;
	pstSmartMenu->iNum = 0;
	for(i = 0; i < SM_MAX_LINE; i++) 
	{
		if (pstMenuItem[i].pszText == NULL)
		{
			return;
		}

		if(strcmp(pstMenuItem[i].pszText, "") == 0) 
		{
			break;
		}

		pstSmartMenu->sMask[i] = SM_ON;
		pstSmartMenu->iNum = i+1;
	}
}


void PubSmMask(SMART_MENU *pstSmartMenu, const char *pszName, int iFlag)
{
	int i=0;

	if (pstSmartMenu == NULL || pszName == NULL || pstSmartMenu->pstContent == NULL)
	{
		return;
	}

	for(i = 0; i < SM_MAX_LINE; i++) 
	{
		if(strcmp(pstSmartMenu->pstContent[i].pszText, "") == 0) 
		{
			break;
		}

		if(strcmp(pstSmartMenu->pstContent[i].pszText, pszName) == 0) 
		{
			if (iFlag != SM_OFF)
			{
				iFlag = SM_ON;
			}
			pstSmartMenu->sMask[i] = iFlag;
			break;
		}
	}
}


uchar PubYesNo(ushort nWaitTime)
{
	uchar   ucTmp;

	while (1)
	{
		ucTmp = PubWaitKey(nWaitTime);
		if (ucTmp==KEYENTER)
		{
			return  0;
		}
		if (ucTmp==KEYCANCEL || ucTmp==NOKEY)
		{
			return  0xFF;
		}
	}
}


////////////////////////////////////// 计算 //////////////////////////////////////


void PubDes(uchar ucMode, uchar *psData, uchar *psKey, uchar *psResult)
{
	uchar   sTmp[8];

	PubASSERT(ucMode==ONE_ENCRYPT  || ucMode==ONE_DECRYPT ||
		ucMode==TRI_ENCRYPT  || ucMode==TRI_DECRYPT ||
		ucMode==TRI_ENCRYPT3 || ucMode==TRI_DECRYPT3);

	if ((psData == NULL) || (psKey == NULL) || (psResult == NULL))
	{
		return;
	}

	switch(ucMode)
	{
	case ONE_ENCRYPT:
		des(psData, psResult, psKey, ENCRYPT);
		break;

	case ONE_DECRYPT:
		des(psData, psResult, psKey, DECRYPT);
		break;

	case TRI_ENCRYPT:
		des(psData,   psResult, psKey,   ENCRYPT);
		des(psResult, sTmp,     psKey+8, DECRYPT);
		des(sTmp,     psResult, psKey,   ENCRYPT);
		break;

	case TRI_DECRYPT:
		des(psData,   psResult, psKey,   DECRYPT);
		des(psResult, sTmp,     psKey+8, ENCRYPT);
		des(sTmp,     psResult, psKey,   DECRYPT);
		break;

	case TRI_ENCRYPT3:
		des(psData,   psResult, psKey,    ENCRYPT);
		des(psResult, sTmp,     psKey+8,  DECRYPT);
		des(sTmp,     psResult, psKey+16, ENCRYPT);
		break;

	case TRI_DECRYPT3:
		des(psData,   psResult, psKey+16, DECRYPT);
		des(psResult, sTmp,     psKey+8,  ENCRYPT);
		des(sTmp,     psResult, psKey,    DECRYPT);
		break;
	}
}


void PubCalcMac(uchar ucMode, uchar *psKey, uchar *psMsg, uint uiLen, uchar *psMac)
{
	uchar   sOutMac[8];
	uint	uiOffset, i;

	PubASSERT(ucMode==MAC_FAST || ucMode==MAC_ANSIX99);
	memset(sOutMac, 0, sizeof(sOutMac));
	uiOffset = 0;

	if ((psKey == NULL) || (psMsg == NULL) || (psMac == NULL))
	{
		return;
	}

	while(uiLen>uiOffset)
	{
		if (uiLen-uiOffset<=8)
		{
			for (i=0; i<uiLen-uiOffset; i++)
			{
				sOutMac[i] ^= psMsg[uiOffset+i];
			}
			break;
		}
		for (i=0; i<8; i++)
		{
			sOutMac[i] ^= psMsg[uiOffset+i];
		}
		if (ucMode==MAC_ANSIX99)
		{
			PubDes(ONE_ENCRYPT, sOutMac, psKey, sOutMac);
		}
		uiOffset += 8;
	}

	PubDes(ONE_ENCRYPT, sOutMac, psKey, psMac);
}

// 计算LRC
void PubCalcLRC(uchar *psData, ushort uiLength, uchar *pucInit)
{
	uchar ucInit;

	if ((psData == NULL) || (pucInit == NULL))
	{
		return;
	}

	ucInit = *psData++;
	uiLength--;

	while( uiLength>0 )
	{
		ucInit ^= *psData++;
		uiLength--;
	}

	*pucInit = ucInit;
}

////////////////////////////////////// 声音 //////////////////////////////////////

void PubLongBeep(void)
{
	Beef(6, 800);
}


void PubBeepOk(void)
{
	uchar	i;

	for (i=0; i<3; i++)
	{
		Beef(6, 60);
		DelayMs(80);
	}
}


void PubBeepErr(void)
{
	Beef(1, 200);
	DelayMs(200);
}

unsigned long DEVICE_GetTickCount(void)
{
	return (unsigned long)GetTimerCount();
}
//unsigned long DEVICE_AES(void)
//{
//	return 0;
//}



//#ifdef _POS_DEBUG
#if 1

// Debug 使用, 功能：显示文件名与行数
void DispAssert(uchar *pszFileName, ulong ulLineNo)
{
	uchar	szFName[30];

	PubShowTitle(TRUE, (uchar *)"Assert Failure");
	GetBaseName(pszFileName, szFName);
	ScrPrint(0, 2, CFONT, "FILE:%.11s", szFName);
	ScrPrint(0, 4, CFONT, "LINE:%ld", ulLineNo);
	PubShowMsg(6, (uchar *)"PRESS ENTER EXIT");
	PubLongBeep();

	while(!PubYesNo(60));
}


// get basename of a full path name
void GetBaseName(uchar *pszFullPath, uchar *pszBaseName)
{
	uchar	*pszTmp;

	*pszBaseName = 0;
	if (!pszFullPath || !*pszFullPath)
	{
		return;
	}

	pszTmp = &pszFullPath[strlen((char *)pszFullPath)-1];
	while( pszTmp>=pszFullPath && *pszTmp!='\\' && *pszTmp!='/' )
	{
		pszTmp--;
	}
	sprintf((char *)pszBaseName, "%s", (char *)(pszTmp+1));
}


#endif

// ============ Added end ===================================== //


