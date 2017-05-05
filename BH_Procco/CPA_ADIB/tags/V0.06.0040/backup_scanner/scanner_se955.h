#ifndef _SE955_H_
#define _SE955_H_
/*
SE955 ����ǹ֧�ֿ�
xielihong 20081227
*/

#include "posapi.h"

typedef struct 
{
	uchar data[256];
	uchar datalen;
	uchar opcode;
	uchar status;
	uchar no_use;
}SE955_TDATA;

/* ������ API */
int SE955_Open(void);
void SE955_Close(void);
int SE955_StartDecode(void);
int SE955_GetDecodeData(uchar *bartype,uchar *outstr,ushort timeoutms);
int SE955_SetParam1(uchar param,uchar value);
int SE955_SetParam2(uchar extend,uchar param,uchar value);
int SE955_SendCmd(uchar opcode,uchar *data,uchar datalen);

/* �ڶ��� API */
int SE955_SendRecv(SE955_TDATA * indata,SE955_TDATA * outdata,ushort timeoutms);

/* ��һ�� API */
int SE955_RecvPacket(SE955_TDATA * outdata,ushort timeoutms);
int SE955_SendPacket(SE955_TDATA * indata);

/* ������ */
#define SE955_ERROR_OK        0
#define SE955_ERROR_UNKNOWN  -1 /* δ֪���� */
#define SE955_ERROR_OPEN     -2 /* ���ڴ�ʧ��  */
#define SE955_ERROR_NOOPEN   -3 /* û�� */
#define SE955_ERROR_NODECODE -4 /* ��SE955 */
#define SE955_ERROR_CMDNAK   -5 
#define SE955_ERROR_CMDACK    0 
#define SE955_ERROR_PARAM    -6 /* �������� */
#define SE955_ERROR_TIMEOUT  -7 /* ��ʱ */
#define SE955_ERROR_PACKET   -8 /* ���յİ��Ƿ���У�鲻�� */
#define SE955_ERROR_DENY     -9 /* ������Ĳ��������������ڲ������ô����� */
#define SE955_ERROR_PORT     -10 /* ���ڶ�д���� */

/* ���� */
#define SE955_AIM_OFF           0xc4
#define SE955_AIM_ON            0xc5     /* ������׼���ܣ����Ƽ���ʣ�ƽ��ֻ��һ����� */
#define SE955_BEEP              0xe6     /* ������ */
#define SE955_CMD_ACK           0xD0
#define SE955_CMD_NAK           0xD1
#define SE955_DECODE_DATA       0xF3     /* decoder only */
#define SE955_EVENT             0xf6
#define SE955_LED_OFF           0xe8
#define SE955_LED_ON            0xe7
#define SE955_PARAM_DEFAULTS    0xc8     /* �ָ�����Ϊ����ģʽ */
#define SE955_PARAM_REQUEST     0xc7     /* ��ȡ���� */
#define SE955_PARAM_SEND        0xc6     /* ���ò��� */
#define SE955_REPLY_REVISION    0xa4     /* decoder only */
#define SE955_REQUEST_REVISION  0xa3     /* ��ȡ�汾��Ϣ */
#define SE955_SCAN_DISABLE      0xea     /* ����ɨ�裬Ҳ�Ͳ�����ȥ�������� */
#define SE955_SCAN_ENABLE       0xe9     /* ����ɨ�蹦�� */
#define SE955_SLEEP             0xeb     /* �ֶ�����ʡ��ģʽ */
#define SE955_START_DECODE      0xe4     /* ��ʼɨ�� */
#define SE955_STOP_DECODE       0xe5
#define SE955_WAKEUP            0x00     /* not support ֱ�Ӹ����ڷ���0x00 �Ϳ����� */
#define SE955_CUSTOM_DEFAULTS   0x12


#define SE955_NAK_RESEND 1
#define SE955_NAK_DENIED 6
#define SE955_NAK_BAD_CONTEXT  2


/*

ʹ�÷���:
  1. ���豸��һ��ֻ�����һ��
  ret = SE955_Open();

  2. ���ò����������ڹػ���ʧЧ
  ������param �� value ֵ������� datasheet
  ret = SE955_SetParam1(param,value);
  ret = SE955_SetParam2(extend,param,value);
  SE955_SetParam1(0x80,0x01); // low power,auto sleep

  3. �������ֻ�����ڷ��� SE955_CMD_ACK ������
     ֧�ֵ������� 
	   SE955_AIM_OFF     SE955_AIM_ON          SE955_BEEP          SE955_LED_OFF 
	   SE955_LED_ON      SE955_PARAM_DEFAULTS  SE955_PARAM_SEND    SE955_SCAN_DISABLE 
	   SE955_SCAN_ENABLE SE955_SLEEP           SE955_START_DECODE  SE955_STOP_DECODE
   ʾ��:
  //	ret = SE955_SendCmd(SE955_AIM_ON,0,0); // �����Ļ������񼤹��һ����ֻ��һ����
	ret = SE955_SendCmd(SE955_AIM_OFF,0,0);
	ret = SE955_SendCmd(SE955_BEEP,"\x18",1);
	ret = SE955_SendCmd(SE955_LED_OFF,"\x01",1);
	DelayMs(1000);
	ret = SE955_SendCmd(SE955_LED_ON,"\x01",1);
	ret = SE955_SendCmd(SE955_SCAN_DISABLE,0,0);
	ret = SE955_SendCmd(SE955_SCAN_ENABLE,NULL,0);
	ret = SE955_SendCmd(SE955_SLEEP,0,0);
//	ret = SE955_SendCmd(SE955_PARAM_DEFAULTS,0,0); // reset to factory setting

  4. ��������
  uchar data[256];
  uchar bartype;
  int ret;
  ret = SE955_StartDecode();
  if (ret == SE955_ERROR_OK)
  {
     ret = SE955_GetDecodeData(&bartype,data,3000); // ������Ĭ��ɨ��3��
	 if (ret == SE955_ERROR_OK)
	 {
	    // �����ɹ�
	 }
	 else
	 {
		// ������Ҫ�ǳ�ʱ��
	   }
	 
	}
	
  5. �ر��豸
   SE955_Close();

  6. �ڶ��㷢�պ�������Ҫ���ڵ������޷���ɵĳ��ϣ���Ҫ�ǲ�����״̬�ȵĻ�ȡ
  SE955_SendRecv ��Ҫ���� SE955_PARAM_REQUEST SE955_REQUEST_REVISION
   ʾ��
  	{
		SE955_TDATA indata;
		SE955_TDATA outdata;
		indata.opcode = SE955_PARAM_REQUEST;
		indata.status = 0;
		indata.datalen = 1;
		memcpy(indata.data,"\xfe",1); // get all params 
		ret = SE955_SendRecv(&indata,&outdata,1000);
	}
	{
		SE955_TDATA indata;
		SE955_TDATA outdata;
		indata.opcode = SE955_REQUEST_REVISION;
		indata.status = 0;
		indata.datalen = 0;
		ret = SE955_SendRecv(&indata,&outdata,1000);
	}

  ��ע��
  1. Ĭ������low powerģʽ���Զ��͹���
  2. SE955_SetParam �ػ�ʧЧ
  3. ����ֶ�ʹdecoder������continuousģʽ(����ɨ��ǹ����)����ʱû��Ҫ���� SE955_StartDecode
     ��ע��Ҫ��������SE955_GetDecodeData�����հ�
     ��Ϊ
	 1) decoder ����յ����ݣ����Ͼͻᷢ����
	 2) decoder ���û���յ�ȷ�ϰ�(CMD_ACK)�����ط�
  
*/


#endif
