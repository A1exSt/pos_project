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
* Description:protocol module

*      
* ============================================================================
*/

#include "..\inc\global.h"


#define LOG_TAG	__FILE__
#ifdef PROTOCOL_DEBUG		
#define EL_LOG(enLogPrio, pucTag, pucFunc, iLine, pucFmtStr, ...)		\
	El_Log(enLogPrio, pucTag, pucFunc, iLine, pucFmtStr, ##__VA_ARGS__)

#define EL_LOG_HEX(enLogPrio, pucTag, pucFunc, iLine, pucMsg, pucHexData, iHexDataLen)	\
	El_Log_Hex(enLogPrio, pucTag, pucFunc, iLine, pucMsg, pucHexData, iHexDataLen)

#else
#define EL_LOG_HEX(enLogPrio, pucTag, pucFunc, iLine, pucMsg, pucHexData, iHexDataLen)
#define EL_LOG(enLogPrio, pucTag, pucFunc, iLine, pucFmtStr, ...)

#endif // PROTOCOL_DEBUG



static ushort sg_usPaNo = MSG_PANO_BASE;
static ushort sg_usFrNo = 0;


static uint gusFrSize = MSG_FRAME_LEN - MSG_FRAMME_HEAD_TAIL_LEN; 
static uint gusPaSize = MSG_PACKAGE_LEN;


static int giMsgComPort = -1;
uchar ucKey;

uchar ucMsgGenLRC(const uchar * pszBuf,int iLen, uchar ucPreLrc)
{

	uchar ucTmp = ucPreLrc;
	int iLoop = 0;

	for(iLoop = 0; iLoop < iLen; iLoop++)
	{
		ucTmp ^= pszBuf[iLoop]; 
	}
	return ucTmp;
}


/*
* ============================================================================
* Function:    iMsgPortOpen
* Description: open communicate port
* Returns:     
* Parameter:   [in]int iPort

* ============================================================================
*/

static int iMsgPortOpen(int iPort)
{
	return PortOpen(iPort,"115200,8,n,1");
}


/*
* ============================================================================
* Function:    iMsgPortReset
* Description: reset buffer
* Returns:     
* Parameter:   [in]int iPort

* ============================================================================
*/

static int iMsgPortReset(int iPort)
{
	return PortReset(iPort);
}


/*
* ============================================================================
* Function:    iMsgPortRecvs
* Description: receive data
* Returns:     
* Parameter:   [in]int iPort
[in]uchar *pszBuf
[in]int iRcvLen
[in]nt iTimeOutMs

* ============================================================================
*/

static int iMsgPortRecvs(int iPort, uchar *pszBuf, int iRcvLen, int iTimeOutMs)
{
	int iActLen = -1;

	iActLen = PortRecvs(iPort,(char*)pszBuf,iRcvLen, iTimeOutMs);
	return iActLen;
}

/*
* ============================================================================
* Function:    iMsgPortSends
* Description: send data
* Returns:     
* Parameter:   [in]int iPort
[in]uchar *pszBuf
[in]int iSendLen
[in]nt iTimeOutMs

* ============================================================================
*/

static int iMsgPortSends(int iPort, const uchar *pszBuf, int iSendLen, int iTimeOut)
{
	uchar ucRet;

	ucRet = PortSends(iPort,(char*)pszBuf,iSendLen);	
	if(ucRet)
	{
		return -ucRet;
	}
	else 
	{
		return ucRet;
	}
}

static int iMsgSndOutOfStep()
{
	int iRet = 0;
	uchar aucMsgTmp[32] = {0};

	aucMsgTmp[0] = MSG_STX;
	aucMsgTmp[1] = MSG_PROTOCOL_VER;
	aucMsgTmp[2] = PANO_OUTOFSTEP>>8;
	aucMsgTmp[3] = PANO_OUTOFSTEP;
	aucMsgTmp[4] = 00;
	aucMsgTmp[5] = 01;
	aucMsgTmp[6] = 0x00;
	aucMsgTmp[7] = 0x00;
	aucMsgTmp[8] = ucMsgGenLRC(aucMsgTmp, 8,0x00);
	aucMsgTmp[9] = MSG_ETX_END;

	EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "response outofstep\n"); 

	iRet = iMsgPortSends(giMsgComPort,aucMsgTmp,10,500);
	iMsgPortReset(giMsgComPort);
	return iRet;
}

static int iMsgSndNACK(ushort usFrNo, uchar ucNack)
{
	int iRet = 0;
	uchar aucMsgTmp[32] = {0};

	aucMsgTmp[0] = MSG_STX;
	aucMsgTmp[1] = MSG_PROTOCOL_VER;
	aucMsgTmp[2] = PANO_CONFIRM >> 8;
	aucMsgTmp[3] = PANO_CONFIRM;
	aucMsgTmp[4] = usFrNo >> 8;
	aucMsgTmp[5] = usFrNo >> 0;
	aucMsgTmp[6] = 0x00;
	aucMsgTmp[7] = 0x01;
	aucMsgTmp[8] = ucNack;
	aucMsgTmp[9] = ucMsgGenLRC(aucMsgTmp, 9,0x00);
	aucMsgTmp[10] = MSG_ETX_END;

	EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "response confirm\n"); 

	iRet = iMsgPortSends(giMsgComPort,aucMsgTmp ,11 ,500);
	return iRet;
}
static int iMsgSndSyn()
{
	int iRet = 0;
	uchar aucMsgTmp[32] = {0};

	aucMsgTmp[0] = MSG_STX;
	aucMsgTmp[1] = MSG_PROTOCOL_VER;
	aucMsgTmp[2] = PANO_SYN >> 8;
	aucMsgTmp[3] = PANO_SYN;
	aucMsgTmp[4] = 00;
	aucMsgTmp[5] = 01;
	aucMsgTmp[6] = 0x00;
	aucMsgTmp[7] = 0x08;
	aucMsgTmp[8] = gusPaSize >> 24;
	aucMsgTmp[9] = gusPaSize >> 16;
	aucMsgTmp[10] = gusPaSize >> 8;
	aucMsgTmp[11] = gusPaSize >> 0;

	aucMsgTmp[12] = gusFrSize >> 24;
	aucMsgTmp[13] = gusFrSize >> 16;
	aucMsgTmp[14] = gusFrSize >> 8;
	aucMsgTmp[15] = gusFrSize >> 0;
	aucMsgTmp[16] = ucMsgGenLRC(aucMsgTmp, 16, 0x00);
	aucMsgTmp[17] = MSG_ETX_END;

	EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "response syn\n"); 

	iRet=iMsgPortSends(giMsgComPort, aucMsgTmp, 18,500);
	sg_usPaNo = MSG_PANO_BASE;
	return iRet;
}

static int iMsgRcvMsg(int *piResult, uint uiDataBufLen, uchar *pucDataBuf, ulong ulTimeOutMs, uchar *pucInfoOut, const uchar *pucHeadData)
{
	int iRet = 0;
	int iTmp = 0;
	uint iLen = 0;
	int iRcvLen = 0;
	uchar ucTmp = 0;
	uchar aucMsgTmp[32] = {0};
	uchar aucTeset[32] = {0};
	uchar ucTmptest = 0;
	int iRetry = MSG_FRAME_RETRY;
	int i = 0;

	*piResult = MSG_RET_TIMEOUT;
	SofTimerSet(TM_Msg_CMD, ulTimeOutMs);
	if(pucHeadData == NULL || ProtocolType == PROTOCOL_NORMAL_MODE || sg_usFrNo > 1)
	{
		do
		{
			iRet = iMsgPortRecvs(giMsgComPort, aucMsgTmp, 1, 0);
			if(iRet != 1) 
			{
				continue;
			}
			if(aucMsgTmp[0] != MSG_STX)
			{
				SofTimerSet(TM_Msg_CMD, MSG_RECV_CHAR_TIMEOUT);
				continue;
			}

			//protocol ver 1B +PaNo 2B+frame No 2B+len 2B 
			iRet = iMsgPortRecvs(giMsgComPort, aucMsgTmp + 1, 7, MSG_RECV_CHAR_TIMEOUT*7);
			if(iRet != 7)
			{
				SofTimerSet(TM_Msg_CMD, ulTimeOutMs);
				continue;			
			}
			else
			{	
				*piResult = MSG_RET_OK;
				break;
			}
		}while(SofTimerCheck(TM_Msg_CMD));
	}
	else
	{   
		memset(aucMsgTmp, 0, sizeof(aucMsgTmp));
		memcpy(aucMsgTmp, pucHeadData, 5);
		do
		{
			iRet = iMsgPortRecvs(giMsgComPort, aucMsgTmp + 5, 3, MSG_RECV_CHAR_TIMEOUT*3);
			EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "PROTOCOL_COMPATIBLE_MODE, iMsgPortRecvs=%d\n", iRet);
			EL_LOG_HEX(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "Value: ", &aucMsgTmp[5], 3);
			if(iRet != 3) 
			{

				SofTimerSet(TM_Msg_CMD, MSG_RECV_CHAR_TIMEOUT*2);
				continue;
			}
			else
			{	
				*piResult = MSG_RET_OK;
				break;
			}
		}while(SofTimerCheck(TM_Msg_CMD));		
	}

	if(*piResult == MSG_RET_TIMEOUT)
	{
		return  MSG_RET_TIMEOUT;
	}

	EL_LOG_HEX(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "aucMsgTmp: ", aucMsgTmp, 8);
	iLen = ((uchar)aucMsgTmp[6] << 8) + (uchar)aucMsgTmp[7];
	EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "RecvLen=%d\n", iLen);
	if(iLen > uiDataBufLen)//buffer will over flow
	{
		*piResult = MSG_RET_BUFFER_OVERFLOW;
		return MSG_RET_BUFFER_OVERFLOW;
	}
	iRcvLen = 0;

	SofTimerSet(TM_Msg_CMD, ulTimeOutMs);
	do
	{
		iTmp = iMsgPortRecvs(giMsgComPort, pucDataBuf + iRcvLen, iLen, MSG_RECV_CHAR_TIMEOUT*iLen);
		EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "iMsgPortRecvs=%d\n", iTmp);
		if(iTmp > 0)
		{
			iRcvLen += iTmp;
			iLen -= iTmp;
			SofTimerSet(TM_Msg_CMD, ulTimeOutMs);
		}
	}	while((iLen > 0) && SofTimerCheck(TM_Msg_CMD)/*&&s_ulSofTimerCheck(TM_Msg_TOT_CMD)*/);

	EL_LOG_HEX(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "msg body: ", pucDataBuf, iRcvLen);
	if(iLen > 0)
	{
		*piResult = MSG_RET_RCV_FAIL;
		return MSG_RET_RCV_FAIL;
	}

	//recv LRC+ETX
	iRet = iMsgPortRecvs(giMsgComPort, aucMsgTmp + 8, 2, ulTimeOutMs);	
	EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "LRC+ETX, iRet = %d \n", iRet);
	if(iRet != 2)
	{
		*piResult = MSG_RET_RCV_FAIL;
		return MSG_RET_RCV_FAIL;
	}

	EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "lrc = %02x, etx = %02x\n", aucMsgTmp[8], aucMsgTmp[9]);

	ucTmp = ucMsgGenLRC(aucMsgTmp, 8, 0);
	ucTmptest = ucMsgGenLRC(pucDataBuf, iRcvLen, ucTmp);
	EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "ucMsgGenLRC = %02x\n", ucTmptest);	
	if(aucMsgTmp[8] != ucMsgGenLRC(pucDataBuf, iRcvLen, ucTmp))
	{
		return MSG_RET_LRC_ERR;
	}
	if(aucMsgTmp[9] == MSG_ETX_CONINUE)
	{
		*piResult = MSG_RET_HAS_NEXT_FRAME;
	}
	else *piResult = MSG_RET_OK;

	if(pucInfoOut)
	{
		memcpy(pucInfoOut,aucMsgTmp,10);
	}

	return  iRcvLen;
}

static int iMsgRcvNACK(ushort usRcvFrNo, uchar* pucNack)
{
	int iRet = 0;
	uchar aucMsgTmp[32] = {0};
	uchar aucDataBuf[1024] = {0};
	uchar dispbuf[150] = {0};
	ushort usPano = 0;
	ushort usFrno = 0;
	int iRetry = MSG_FRAME_RETRY;
	int iResult = 0;

	int * piResult = &iResult;

	iRet = iMsgRcvMsg(piResult, sizeof(aucDataBuf), aucDataBuf, MSG_RECV_ACK_TIMEOUT, aucMsgTmp, NULL);
	EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "iMsgRcvMsg = %d\n", iRet);
	if(iRet < 0)
	{
		return iRet;
	}

	usPano = ((ushort )aucMsgTmp[2] << 8) + ((ushort )aucMsgTmp[3]);
	usFrno = ((ushort )aucMsgTmp[4] << 8) + ((ushort )aucMsgTmp[5]);

	if(usPano < MSG_PANO_BASE)
	{//receive control frame
		if(usPano == PANO_SYN)//sys frame
		{
			EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "PANO_SYN, usPano = %d\n", usPano);
			iMsgSndSyn();
			*piResult = MSG_RET_HAS_SYN;
			return MSG_RET_HAS_SYN;
		}
		else if(usPano == PANO_CONFIRM)
		{
			if(usFrno != usRcvFrNo)
			{
				EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "PANO_CONFIRM, outstep: usPano[%d], usFrno[%d]\n", usPano, usFrno);
				//had already outstep
				iMsgSndOutOfStep();
				*piResult = MSG_RET_OUTOFSTEP;
				return MSG_RET_OUTOFSTEP;
			}
			*pucNack = aucDataBuf[0];
			return 0;
		}
		else
		{
			EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "other control frames: usPano[%d]\n", usPano);
			return MSG_RET_ERROR;//ignore other control frames
		}

	}
	else
	{
		EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "outstep: usPano[%d]\n", usPano);
		//had already outstep
		iMsgSndOutOfStep();
		*piResult = MSG_RET_OUTOFSTEP;
		return MSG_RET_OUTOFSTEP;
	}
	return MSG_RET_ERROR;
}


static int iMsgRcvFrame(int *piResult, uint uiFrameDataBufLen, uchar*pucFrameDataBuf, ulong ulTimeOutMs, uchar *pucFrameInfoOut, const uchar *pucHeadData)
{
	int iRet = 0;
	int iRcvLen = 0;
	uchar aucMsgTmp[32] = {0};
	uchar dispbuf[150] = {0};
	ushort usPano = 0;
	ushort usFrno = 0;
	int iRetry = MSG_FRAME_RETRY;
	int iResend = 0;

Retry_Rcv_Frame:
	if(iRetry == MSG_FRAME_RETRY && iResend == 0)
	{
		//becasuse the first time maybe have reveive part head(5bytes)
		iRet = iMsgRcvMsg(piResult, uiFrameDataBufLen, pucFrameDataBuf, ulTimeOutMs, aucMsgTmp, pucHeadData);
	}
	else
	{
		//after the first time, must reeveive the whole head;
		iRet = iMsgRcvMsg(piResult, uiFrameDataBufLen, pucFrameDataBuf, ulTimeOutMs, aucMsgTmp, NULL);
	}
	EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "iMsgRcvFrame after iRet = %d\n", iRet);
	if(iRet < 0)
	{
		if(iRet == MSG_RET_LRC_ERR)
		{
			EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "lrc error: %d", iRet);
			//��NAK
			iMsgSndNACK(sg_usFrNo, MSG_NAK);
			if(iRetry > 0)
			{
				iRetry--;
				goto Retry_Rcv_Frame;
			}
			else
			{
				*piResult = MSG_RET_RETRY_FAIL;
				return MSG_RET_RETRY_FAIL;
			}
		}
		else 
		{
			return iRet;
		}
	}
	else
	{
		iRcvLen = iRet;

		usPano = ((ushort )aucMsgTmp[2] << 8) + ((ushort)aucMsgTmp[3]);
		usFrno = ((ushort )aucMsgTmp[4]<<8)+((ushort)aucMsgTmp[5]);
		EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "sg_usPaNo = %d, sg_usFrNo = %d, usPano = %d, usFrno = %d\n", sg_usPaNo, sg_usFrNo, usPano, usFrno);

		if(usPano < MSG_PANO_BASE)
		{//�յ�����֡
			if(usPano == PANO_SYN)//sys frames
			{  
				EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "PANO_SYN, usPano = %d\n", usPano);
				iMsgSndSyn();
				*piResult = MSG_RET_HAS_SYN;
				return MSG_RET_HAS_SYN;
			}
			return MSG_RET_RCV_FAIL;

		}

		if(usPano != sg_usPaNo || (usFrno != sg_usFrNo && usFrno != sg_usFrNo - 1))
		{
			//already out step
			EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "out step: usPano[%d], usFrno[%d]\n", usPano, usFrno);
			iMsgSndOutOfStep();
			*piResult = MSG_RET_OUTOFSTEP;
			return MSG_RET_OUTOFSTEP;
		}
		if(usFrno == sg_usFrNo - 1)
		{//resend framess
			EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "need to resend: usFrno[%d]\n", usFrno);
			//respnose ACK
			iMsgSndNACK(sg_usFrNo, MSG_ACK);
			iResend++;
			goto Retry_Rcv_Frame;//ignore other control frames
		}
		EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "package normal: usFrno[%d]\n", usFrno);
		//response ACK
		iRet = iMsgSndNACK(sg_usFrNo, MSG_ACK);


	}
	if(aucMsgTmp[9] == MSG_ETX_CONINUE)
	{
		*piResult = MSG_RET_HAS_NEXT_FRAME;
	}
	else 
	{
		*piResult = 0;
	}

	if(pucFrameInfoOut)
	{
		memcpy(pucFrameInfoOut,aucMsgTmp,10);
	}

	return  iRcvLen;

}

static int iMsgSndFrame(int *piResult, uchar ucEndFlg, int iFramDataLen, const uchar*pucFramDataBuf, ulong ulTimeOutMs)
{
	int iRet = 0;//,iSndOffset,iTmp;
	uchar ucTmp = 0;
	uchar ucNack = 0;
	uchar aucMsgTmp[32] = {0};
	int iRetry = MSG_FRAME_RETRY;
	int i = 0;
	//int iTestCnt=2;

	aucMsgTmp[0] = MSG_STX;
	aucMsgTmp[1] = MSG_PROTOCOL_VER;
	aucMsgTmp[2] = sg_usPaNo >> 8;
	aucMsgTmp[3] = sg_usPaNo >> 0;
	aucMsgTmp[4] = sg_usFrNo >> 8;
	aucMsgTmp[5] = sg_usFrNo >> 0;
	aucMsgTmp[6] = iFramDataLen >> 8;
	aucMsgTmp[7] = iFramDataLen;

	ucTmp = ucMsgGenLRC(aucMsgTmp, 8, 0x00);
	aucMsgTmp[8] = ucMsgGenLRC(pucFramDataBuf, iFramDataLen, ucTmp);
	if(ucEndFlg)
	{
		aucMsgTmp[9] = MSG_ETX_END;
	}
	else 
	{
		aucMsgTmp[9] = MSG_ETX_CONINUE;
	}

Retry_Send_Frame:

	iRet = iMsgPortSends(giMsgComPort, aucMsgTmp, 8, ulTimeOutMs);
	if(iRet != 0)
	{
		EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "iMsgPortSends = %d\n", iRet);
		return iRet;
	}
	EL_LOG_HEX(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "aucMsgTmp: ", aucMsgTmp, 8);

	iRet = iMsgPortSends(giMsgComPort, pucFramDataBuf, iFramDataLen, ulTimeOutMs);
	if(iRet!=0)
	{
		EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "iMsgPortSends = %d, iFramDataLen = %d\n", iRet, iFramDataLen);
		return iRet;
	}
	//AppHexLog("[iMsgSndFrame] pucFramDataBuf: ", pucFramDataBuf, iFramDataLen);

	iRet = iMsgPortSends(giMsgComPort, aucMsgTmp + 8, 2, ulTimeOutMs);
	if(iRet!=0) 
	{
		EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "lrc etx, iMsgPortSends = %d\n", iRet);
		return iRet;
	}
	EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "lrc=%02x, etx=%02x\n", aucMsgTmp[8],aucMsgTmp[9]);

	while(1)
	{
		//receive nack
		iRet = iMsgRcvNACK(sg_usFrNo, &ucNack);
		if((iRet == 0) && (ucNack == MSG_ACK))
		{
			return 0;
		}
		else if((iRet == 0) && ( ucNack == MSG_NAK) || iRet == MSG_RET_TIMEOUT)//retry if rcv nak and timeout 
		{	
			if(iRetry > 0)
			{
				iRetry--;
				goto Retry_Send_Frame;		
			}
			else 
			{
				return iRet;
			}
		}
		else if(iRet == MSG_RET_OUTOFSTEP)
		{
			if(iRetry > -1)
			{
				iRetry--;
				continue;
			}
			return iRet;
		}
	}
	return iRet;

}

int iMsgRcv(uint uiMsgDataBufLen, uchar *pucMsgDataBuf, ushort *usCommand, ulong ulTimeOutMs, uchar *pucHeadData)
{
	int iLen = 0;
	int iRet = 0;
	uchar aucFramInfo[16] = {0};
	int iResult = 0;
	uchar ucTemp[2048] = {0};
	int iFirstFrameLen = 0;

	if(giMsgComPort < 0)
	{
		return MSG_RET_NOT_INIT;
	}

MsgRcv_Retry:

	sg_usFrNo = 0;
	for(;;)
	{
		sg_usFrNo++;
		//iRet = iMsgRcvFrame(&iResult, uiMsgDataBufLen - iLen, pucMsgDataBuf + iLen, ulTimeOutMs, aucFramInfo);
		memset(ucTemp , 0 , sizeof(ucTemp));

		//becase if is compitibal mode and protocol is command protocol(not remote protocol), if first reveive syn, must do else to recevie the whole head
		if(iRet != MSG_RET_HAS_SYN)
		{
			iRet = iMsgRcvFrame(&iResult, uiMsgDataBufLen - iLen, ucTemp, ulTimeOutMs, aucFramInfo, pucHeadData);
		}
		else
		{
			iRet = iMsgRcvFrame(&iResult, uiMsgDataBufLen - iLen, ucTemp, ulTimeOutMs, aucFramInfo, NULL);
		}
		if(iRet < 0)
		{
			if(iRet == MSG_RET_HAS_SYN) 
			{
				goto MsgRcv_Retry;
			}
			return iRet;
		}
		else
		{
			iLen += iRet;
		}

		EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "iMsgRcvFrame=%d\n", iLen);
		EL_LOG_HEX(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "ucTemp: ", ucTemp, iRet);

		if(sg_usFrNo <= 1)
		{
			*usCommand = GetCommand(ucTemp);
			//strcpy(pucMsgDataBuf, ucTemp + 2);
			iFirstFrameLen = iLen -2;
			memcpy(pucMsgDataBuf, ucTemp + 2, iFirstFrameLen);
		}
		else
		{
			//strcat(pucMsgDataBuf, ucTemp + 2);
			memcpy(pucMsgDataBuf + iFirstFrameLen, ucTemp, iRet);
		}

		EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "iResult=%d\n", iResult);

		if(iResult == MSG_RET_HAS_NEXT_FRAME)
		{
			continue;//continue;
		}
		else
		{
			return iLen;
		}

	}
	return -1;
}


static int iCmdMsgSnd(int iMsgDataLen, const uchar *pucMsgData, ulong ulTimeOutMs)
{
	int iLen = 0;
	int iRet = 0;
	int i = 0;
	int iMax = 0;

	uchar ucEndFlg;
	int iResult;

	sg_usFrNo = 0;
	if(giMsgComPort < 0)
	{
		return MSG_RET_NOT_INIT;
	}

	iResult = 0;
	iRet = MSG_RET_OK;
	iMax = (iMsgDataLen + gusFrSize - 1) / gusFrSize;
	EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "iMax=%d\n", iMax);
	for(i = 0; i < iMax; i++)
	{
		sg_usFrNo++;
		if(i == iMax-1)
		{
			ucEndFlg = 1;
			iLen = iMsgDataLen % gusFrSize;
		}
		else 
		{
			iLen = gusFrSize;
			ucEndFlg = 0;//not end
		}
		iRet = iMsgSndFrame(&iResult, ucEndFlg, iLen, pucMsgData + i*gusFrSize, ulTimeOutMs);

		if(iRet)
		{
			break;
		}
	}
	if(iResult == MSG_RET_OK)
	{
		sg_usPaNo++;
		if(sg_usPaNo == 0)
		{
			sg_usPaNo = 1000;
		}
	}
	return iRet;
}


int iMsgInit(int iPort,int iPaSize,int iFrSize)
{
	int iRet;
	gusPaSize = iPaSize;
	gusFrSize = iFrSize;

	iRet = iMsgPortOpen(iPort);
	if(iRet)
	{
		return iRet;
	}

	giMsgComPort = iPort;

	return iRet;
}



static int iReceiveHead(uchar *pucHeadData, ulong ulTimeOutMs)
{
	int iRet = -1;
	int iResult = MSG_RET_TIMEOUT;

	SofTimerSet(TM_Msg_CMD, ulTimeOutMs);
	do
	{
		iRet = iMsgPortRecvs(giMsgComPort, pucHeadData, 1, 0);
		if(iRet != 1) 
		{
			continue;
		}
		if(pucHeadData[0] != MSG_STX)
		{
			SofTimerSet(TM_Msg_CMD, MSG_RECV_CHAR_TIMEOUT);
			continue;
		}

		//if portocol is downloadprotocol   2-3byte is command
		iRet = iMsgPortRecvs(giMsgComPort, pucHeadData + 1, 4, MSG_RECV_CHAR_TIMEOUT*4);
		if(iRet != 4)
		{
			SofTimerSet(TM_Msg_CMD, ulTimeOutMs);
			continue;			
		}
		else
		{	
			iResult = MSG_RET_OK;
			return iResult;
		}
	}while(SofTimerCheck(TM_Msg_CMD));

	if(iResult == MSG_RET_TIMEOUT)
	{
		return  MSG_RET_TIMEOUT;
	}

	return iRet;
}


//receive the file downlaod message   
static int iReceiveMessage(const uchar *HeadData, uchar *pucMsgDataBuf, ushort ulTimeOutMs)
{
	uchar Lrc = 0;
	int iRet = 0;
	ushort Len = 0;
	ushort ReceiveLen = 0;
	int i = 0;
	uchar Buffer[2] = {0};
	ushort ThisLen = 0;

	Len = (HeadData[3] << 8) + HeadData[4];

	EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "Len=%d\n", Len);

	Lrc = HeadData[1] ^ HeadData[2];
	Lrc ^= HeadData[3] ^ HeadData[4];

	while (ReceiveLen < Len) 
	{	
		ThisLen = Len - ReceiveLen;
		if(ThisLen > 240)
		{
			ThisLen = 240;
		}

		iRet = PortRecvs(giMsgComPort, pucMsgDataBuf + ReceiveLen, ThisLen, ulTimeOutMs);

		if (iRet < 0)
		{
			return iRet;
		}

		ReceiveLen += iRet;
	}

	for (i = 0; i < Len; ++i)
	{
		Lrc ^= pucMsgDataBuf[i];
	}

	// receive LRC
	iRet = PortRecvs(giMsgComPort, Buffer, 1, ulTimeOutMs);
	EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "PortRecvs=%d, Lrc_Buffer[0] = %X, Lrc = %X\n", iRet, Buffer[0], Lrc);
	if (iRet != 1)
	{
		return iRet;
	}

	// check LRC
	if (Buffer[0] != Lrc)
	{
		//Lcdprintf("Send NAK\r\n");
		// send NAK
		PortSends(giMsgComPort, "\x15", 1);
		return MSG_RET_LRC_ERR;
	}
	else
	{
		//Lcdprintf("Send ACK\r\n");
		// send ACK
		PortSends(giMsgComPort, "\x06", 1);


		return ReceiveLen;
	}

}

int iReceiveRequest(uint uiMsgDataBufLen, uchar *pucMsgDataBuf, ushort *usCommand, ushort ulTimeOutMs)
{
	int iRet = 0;
	uchar ucHeadData[16] = {0};
	//ProtocolType = PROTOCOL_COMPATIBLE_MODE;

	EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "ProtocolType = %d\n",ProtocolType);
	if(ProtocolType == PROTOCOL_NORMAL_MODE)
	{
		iRet = iMsgRcv(uiMsgDataBufLen, pucMsgDataBuf, usCommand, ulTimeOutMs, NULL);
		if(iRet < 0)
		{
			return iRet;
		}
		EL_LOG_HEX(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "iMsgRcv, Value: ", ucHeadData, 16);
	}
	else if(ProtocolType == PROTOCOL_COMPATIBLE_MODE)
	{
		iRet = iReceiveHead(ucHeadData, ulTimeOutMs);
		if(iRet < 0)
		{	
			EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "iReceiveHead=%d\n",iRet);
			return iRet;
		}
		*usCommand = (ucHeadData[1] << 8) + ucHeadData[2];
		EL_LOG_HEX(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "Value: ", ucHeadData, 5);
		if(isFindFileDownLoadCmd(*usCommand) == PROTOCOL_REMOTEDOWNLOAD_COMMAND)
		{
			iRet = iReceiveMessage(ucHeadData, pucMsgDataBuf, ulTimeOutMs);
			if(iRet < 0)
			{
				EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "iReceiveMessage=%d\n",iRet);
			}

			if(iRet < 0)
			{
				return iRet;
			}
		}
		else
		{
			iRet = iMsgRcv(uiMsgDataBufLen, pucMsgDataBuf, usCommand, ulTimeOutMs, ucHeadData);
			EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "iMsgRcv=%d\n",iRet);
			if(iRet < 0)
			{
				return iRet;
			}
		}
	}
	return iRet;


}

/* Please fill the responseBuffer with full format (STX + command + length + data + LRC) */
static int iDownFileiMsgSnd(uchar *pucResponseBuffer, int Length)
{
	int iRet = 0;
	int iRetry;
	uchar acknowledge;
	uchar aucLrc = 0;
	iRetry = 0;

	aucLrc = ucMsgGenLRC(pucResponseBuffer + 1, Length, 0x00);
	pucResponseBuffer[0] = MSG_STX;
	pucResponseBuffer[Length] = aucLrc ;
	while (iRetry < 3)
	{
		iRet = PortSends(giMsgComPort, pucResponseBuffer, Length+1);
		if (iRet != 0)
		{
			return iRet;
		}
		// Receive ACK/NAK
		iRet = PortRecvs(giMsgComPort, &acknowledge, 1, 1000);

		if (iRet != 1)
		{
			++iRetry;
			continue;
		}

		// ACK received, process DONE!
		if (MSG_ACK == acknowledge)
		{
			return 0;
		}
		// no valid ACK/NAK received, try again till 3
		else if (acknowledge != MSG_NAK)
		{
			++iRetry;
		}

		EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "aucLrc = %02x,Length = %d, acknowledge = %02x\n", aucLrc,Length, acknowledge);

		// NAK received, just retry
	}

	return iRet;
}

int iSendResponse(const uchar *pucMsgResponse , int iResponseLen, int iReturnCode, ushort usCommand)
{
	uchar pucData[BUF_LEN_2048] = {0};
	int i = 0;
	int iRet = 0;

	if((usCommand >> 8) == EL_PROTOCOL_COMMANS)
	{
		pucData[0] = usCommand >> 8;
		pucData[1] = usCommand >> 0;
		pucData[2] = iReturnCode >> 24;
		pucData[3] = iReturnCode >> 16;
		pucData[4] = iReturnCode >> 8;
		pucData[5] = iReturnCode >> 0;

		memcpy(pucData + 6, pucMsgResponse, iResponseLen);
		iRet = iCmdMsgSnd(iResponseLen + 6, pucData, 5000);
	}
	else if((usCommand >> 8) == EL_PROTOCOL_DOWNLOAD || (usCommand >> 8) == EL_GET_INFO_COMMANS)
	{
		/* Please fill the responseBuffer with full format (STX + command + length + data + LRC) */
		pucData[1] = usCommand >> 8;
		pucData[2] = usCommand >> 0;
		pucData[3] = (iResponseLen + 4 - ERR_MSG_OFFSET) >> 8;
		pucData[4] = (iResponseLen + 4 - ERR_MSG_OFFSET) >> 0;
		pucData[5] = iReturnCode >> 24;
		pucData[6] = iReturnCode >> 16;
		pucData[7] = iReturnCode >> 8;
		pucData[8] = iReturnCode >> 0;


		memcpy(pucData + 9, pucMsgResponse, iResponseLen);
		iRet = iDownFileiMsgSnd(pucData, iResponseLen + 9 - ERR_MSG_OFFSET);
		EL_LOG(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "iDownFileiMsgSnd = %d,iResponseLen = %d\n", iRet,iResponseLen);
		EL_LOG_HEX(LOG_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, "EL_PROTOCOL_DOWNLOAD pucData: ", pucData, iResponseLen + 10);
	}
	memset(g_pucErrMsg , 0, sizeof(g_pucErrMsg));
	return iRet;

}


#if 0
int TestMsg(void)
{
	int iRet,i;
	uchar aucRcv[4096],aucSnd[4096];
	//PortOpen(11,"115200,8,n,1");
	uchar ucCnt=0xa0;
	iMsgInit(COM_USB,4096,900);
	while(1)
	{
		iRet =iMsgRcv(sizeof(aucRcv),aucRcv,30000);
		if(iRet<0)
		{
			PortReset(11);
			continue;
		} 
		ucCnt++;
		for(i=0;i<iRet;i++)
		{
			aucSnd[i]=aucRcv[i]+1;
		}
		iRet = iMsgSnd(iRet,aucSnd,30000);
	}
}
#endif
