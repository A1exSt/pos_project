

#include "scanner_se955.h"


/*
SE955 ����ǹ֧�ֿ�
changelog:
20081227 xielihong
       ��һ�����õİ汾

20090202 xielihong
      1. ��open ��close �����Ż�
      2. ��P90�ϲ���ͨ��

*/

#define SE955_PORT 2

static uchar g_port=0xff;

int SE955_Open(void)
{
	uchar ucRet;
	int ret;
	ucRet = PortOpen(SE955_PORT,"9600,8,n,1");
	if (ucRet  != 0x00)
	{
		return SE955_ERROR_OPEN;
	}
	g_port = SE955_PORT; /* Ҫ�ȸ�ֵ����Ȼ����ĺ��� û������ */
	ret = SE955_SetParam1(0x9f,0x01); // enable ack/nak
	if (ret != 0) 
	{
		g_port = 0xFF;
		PortClose(SE955_PORT);
		return SE955_ERROR_NODECODE;
	}
	DelayMs(100);
	SE955_SetParam1(0xee,0x01); // packeted decode data

	SE955_SetParam1(0x8a,0x08); // triggering: host mode

	SE955_SetParam1(0x80,0x01); // low power,auto sleep

	SE955_SetParam1(0x38,0x00); // do not beep after decode
	g_port = SE955_PORT;
	return SE955_ERROR_OK;
}

void SE955_Close(void)
{
	if (g_port != 0xFF) 
	{
		// ������Ҫ��һЩӰ���û��۸е�ѡ��ر���
		// ����ֱ�ӻص�����״̬��һ��?
		SE955_SendCmd(SE955_AIM_OFF,0,0); // �ر���׼����
		SE955_SetParam1(0x8a,0x08);            // �ر���������
		SE955_SendCmd(SE955_SLEEP,0,0);   // sleep��ʡ��ģʽ
		PortClose(g_port);
		g_port = 0xff;
	}
}

static ushort SE955_GetChecksum(uchar *buf,int size)
{
// SE955 ��checksum Ϊ���ݵ��ֽڵĺͣ���ȡ����
	int i;
	ushort checksum;
	checksum = 0;
	for (i=0;i<size;i++) 
	{
		checksum += buf[i];
	}
	checksum = (ushort)~(checksum) +1; // ȡ����
	return checksum;
}

static void SE955_GetChecksumBuf(uchar *buf,int size,uchar *chksumbuf)
{
	ushort checksum;
	checksum = SE955_GetChecksum(buf,size);
	chksumbuf[0] = checksum >> 8;
	chksumbuf[1] = checksum & 0x00FF;
}

static int SE955_ValidPacket(uchar *buf,int size) // �����size�ǰ��� checksum��
{
	ushort checksum1,checksum2;
	checksum1 = SE955_GetChecksum(buf,size-2);
	checksum2 = buf[size-2];
	checksum2 <<= 8;
	checksum2 |= buf[size-1];
	return checksum1 == checksum2;
}

int SE955_RecvPacket(SE955_TDATA * outdata,ushort timeoutms)
{
	uchar ch;
	uchar ucRet;
	uchar rxLength;
	
	uchar buf[256];
	int bufIdx;

	if (g_port == 0xFF) 
	{
		return SE955_ERROR_NOOPEN;
	}

	// �յ�һ������
	ucRet = PortRecv(g_port,&ch,timeoutms);


	if (ucRet != 0) 
	{
		// ��ʱ��
		return SE955_ERROR_TIMEOUT;
	}

	// �յ���һ��������
	buf[0] = ch;
	bufIdx = 1;
	rxLength = ch;

	while(bufIdx < rxLength+2)
	{
		ucRet = PortRecv(g_port,&ch,100);
		if (ucRet == 0)
		{
			buf[bufIdx] = ch;
			bufIdx++;
		}
		else
		{
			// ���ճ�ʱ��
			PortReset(g_port);
			return SE955_ERROR_TIMEOUT;
		}
	}
	
	if (SE955_ValidPacket(buf,bufIdx)) 
	{
		// ����Ч
		if (outdata != NULL) 
		{
			outdata->opcode = buf[1];
			outdata->status = buf[3];
			outdata->datalen = rxLength-4;
			memcpy(outdata->data,&buf[4],outdata->datalen);
		}
		return SE955_ERROR_OK;
	}
	return SE955_ERROR_PACKET;
}

int SE955_SendPacket(SE955_TDATA * indata)
{
	uchar txBuf[256];
	uchar txLength;
	uchar ucRet;

	if (g_port == 0xFF) 
	{
		return SE955_ERROR_NOOPEN;
	}

	if (indata == NULL) 
	{
		return SE955_ERROR_PARAM;
	}
	
	memset(txBuf,0,sizeof(txBuf));
	PortSends(g_port,txBuf,6); // ���ڻ��� decoder �������ԣ�������Ҫ4���ֽڵ�0x00
	
	txLength = indata->datalen +4;
	txBuf[0] = txLength;
	txBuf[1] = indata->opcode;
	txBuf[2] = 0x04; // host
	txBuf[3] = indata->status;
	memcpy(&txBuf[4],indata->data,indata->datalen);

	SE955_GetChecksumBuf(txBuf,txLength,&txBuf[txLength]);
	ucRet = PortSends(g_port,txBuf,(ushort)(txLength+2));
	if (ucRet == 0x00) 
	{
		return SE955_ERROR_OK;
	}
	else
	{
		//return SE955_ERROR_PORT;
		return ucRet;
	}
}

int SE955_SendRecv(SE955_TDATA * indata,SE955_TDATA * outdata,ushort timeoutms)
{
	int ret;
	int retrys;
	SE955_TDATA myoutdata;

	PortReset(g_port);
	retrys = 2;
	while(retrys)
	{
		ret = SE955_SendPacket(indata);
		if (ret != 0) return ret;
		ret = SE955_RecvPacket(&myoutdata,timeoutms);
		if (ret == 0)
		{
			if (myoutdata.opcode == SE955_CMD_ACK) 
			{
				if (outdata != NULL) 
				{
					*outdata = myoutdata;
				}
				return SE955_ERROR_OK;
			}
			else if(myoutdata.opcode == SE955_CMD_NAK)
			{
				uchar cause;
				cause = myoutdata.data[0];
				if (cause == SE955_NAK_RESEND) 
				{
					// decoder ���� ���ط�
					retrys--;
				}
				else if (cause == SE955_NAK_DENIED) 
				{
					return SE955_ERROR_DENY;
				}
				else
				{
					// δ֪����
					return SE955_ERROR_UNKNOWN;
				}
			}
			else
			{
				if (outdata != NULL) 
				{
					*outdata = myoutdata;
				}
				return SE955_ERROR_OK; // ���������ֱ�ӷ���
			}
		}
		else
		{
			retrys--;
		}
		DelayMs(500);
	}
	return SE955_ERROR_TIMEOUT;
}

/* ֻ����ֻ���� SE955_CMD_ACK �� SE955_CMD_NAK ������ */
int SE955_SendCmd(uchar opcode,uchar *data,uchar datalen)
{
	int ret;
	SE955_TDATA indata;
	SE955_TDATA outdata;
	indata.opcode = opcode;
	indata.status = 0;
	indata.datalen = datalen;
	if (data != NULL) 
	{
		memcpy(indata.data,data,indata.datalen);
	}

	ret = SE955_SendRecv(&indata,&outdata,1500);
	return ret;
}

int SE955_SetParam1(uchar param,uchar value)
{
	uchar indata[16];
	uchar indatalen;
	int ret;
	indata[0] = 0x00;
	indata[1] = param;
	indata[2] = value;
	indatalen = 3;

	ret = SE955_SendCmd(SE955_PARAM_SEND,indata,indatalen);
	return ret;
}

int SE955_SetParam2(uchar extend,uchar param,uchar value)
{
	uchar indata[16];
	uchar indatalen;
	int ret;
	indata[0] = 0x00;
	indata[1] = extend;
	indata[2] = param;
	indata[3] = value;
	indatalen = 4;

	ret = SE955_SendCmd(SE955_PARAM_SEND,indata,indatalen);
	return ret;
}

int SE955_StartDecode(void)
{
	int ret;
	ret = SE955_SendCmd(SE955_START_DECODE,0,0);
	return ret;
}

static int SE955_SendAck(void)
{
	SE955_TDATA indata;
	indata.opcode = SE955_CMD_ACK;
	indata.status = 0;
	indata.datalen = 0;
	return SE955_SendPacket(&indata);
}

int SE955_GetDecodeData(uchar *bartype,uchar *outstr,ushort timeoutms)
{
	int ret;
	SE955_TDATA outdata;

	// ���ص������Ǵ����
	ret = SE955_RecvPacket(&outdata,timeoutms);
	if (ret == 0) 
	{
		SE955_SendAck(); // ����decode ���Ѿ��յ���
		outdata.data[outdata.datalen] = 0; // ��Ӹ��ַ����Ľ�����
		*bartype = outdata.data[0];
		memcpy(outstr,&outdata.data[1],outdata.datalen+1);
	}
	return ret;
}

int SE955_StopDecode(void)
{
	int ret;
	ret = SE955_SendCmd(SE955_STOP_DECODE,0,0);
	return ret;
}

