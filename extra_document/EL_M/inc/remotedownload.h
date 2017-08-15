/*------------------------------------------------------------
* FileName: remotedownload.h
* Author:	ZhangYuan
* Date:		2016-08-30
------------------------------------------------------------*/

#ifndef REMOTE_DOWNLOAD_H_
#define REMOTE_DOWNLOAD_H_

#define MPOS_RESPONSE_DATA_OFFSET		9

#define MPOS_STATUS_OK					0x00
#define MPOS_STATUS_CMD_NOTSPT			-0xffff
#define MPOS_STATUS_NOTASKLIST			1
#define MPOS_STATUS_FORGET_SET_TASK		2
#define MPOS_STATUS_FILENO_NOT_SAME		3
#define MPOS_STATUS_TASK_NUM_OVERFLOW	4
#define MPOS_STATUS_FIRMWARE_SAVE_ERR	5
#define MPOS_STATUS_PARA_SAVE_ERR		6
#define MPOS_STATUS_SYS_ERR				7		// system error, like file operation error

#define TMS_TASK_NUM_MAX				10	

#define MPOS_FILE_REMOTEDOWNLOAD_BLK_IDX	3100

// task
#define PROTIMS_DOWNLOAD_MONITOR         0x00    // ���ؼ���ļ�
#define PROTIMS_CREATE_FILE_SYSTEM       0x01    // �����ļ�ϵͳ
#define PROTIMS_DOWNLOAD_FONT            0x02    // �����ֿ�
#define PROTIMS_DELETE_APPLICATION       0x03    // ɾ��Ӧ�ó���
#define PROTIMS_DOWNLOAD_APPLICATION     0x04    // ����Ӧ�ó���
#define PROTIMS_DOWNLOAD_PARA_FILE       0x05    // ���ز����ļ�
#define PROTIMS_DELETE_PARA_FILE         0x06    // ɾ�������ļ�
#define PROTIMS_DELETE_ALL_APPLICATION   0x07    // ɾ������Ӧ�ó����丽���ļ������������ļ�����־��
#define PROTIMS_WNET_FIRMWARE_UPDATE     0x09    // ����ģ�������������												//20130416
#define PROTIMS_DOWNLOAD_DLL             0x0A	 // Ӧ�ö�̬���ļ���������	
#define PROTIMS_DELLETE_DLL              0x0B    // ɾ����̬���ļ�
#define PROTIMS_DOWNLOAD_USPUK			 0x10
#define PROTIMS_DOWNLOAD_UAPUK			 0x11
#define PROTIMS_DOWNLOAD_PUBFILE		 0x12
#define PROTIMS_DELETE_PUBFILE			 0x13


//////////////////////////////////////////////////////////////////////////
// download status flag for ucDone in TASK_TABLE
#define PROTIMS_NONE_STATUS					0
#define PROTIMS_DOWNLOADING_STATUS			1
#define PROTIMS_DOWNLOADED_STATUS			2
#define PROTIMS_UPDATED_STATUS				3
// Task List
typedef struct
{
	unsigned int	iFileSize;			// ��ǰ�����С�ߴ�
	unsigned char	ucTaskNo;			// �����
	unsigned char	ucTaskType;			// ��������
	unsigned char	aucFileName[33];	// �ļ���
	unsigned char	aucAppName[33];		// Ӧ�ó�����
	unsigned char	aucVersion[21];		// �汾��
	unsigned char	ucForceUpdate;		//ǿ�Ƹ��±�־
	unsigned char	Rev[5];				// ��������
	unsigned char	ucDone;				// 0:δ���� 1:������ 2:������� 3:�������
}TASK_TABLE;	//total 42 + 4*4 + 22 = 80

int GetTermSN(unsigned char *pucRspDataOut, int *piLenOut);
int GetTermExtSN(unsigned char *pucRspDataOut, int *piLenOut);
int GetTermVerInfo(unsigned char *pucRspDataOut, int *piLenOut);
int GetTerminalInfo(unsigned char *pucRspDataOut, int *piLenOut);
int RemoteDownload_TaskInfo(const unsigned char *pucReqData, unsigned char *pucRspData, int *piRspLen);
int RemoteDownload_SaveData(const unsigned char *pucReqData, unsigned char *pucRspData, int *piRspLen);
int RemoteDownload_TaskAsk(const unsigned char *pucReqData, unsigned char *pucRspData, int *piRspLen, unsigned char *pucSendRspFlag);



#endif // REMOTE_DOWNLOAD_H_

