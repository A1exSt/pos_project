//============================================================================
// Name        : filedownload.h
// Author      :tuxq
// Version     :
// Copyright   : Your copyright notice
// Description : Ansi-style
//============================================================================

#ifndef FILEDOWNLOAD_H
#define FILEDOWNLOAD_H

#define FILEBLKLIST_MAX		20
#define FILENAME_LENGTH_MAX 32
#define BLK_LENGTH_MAX 2048

#define UI_FILE "ui.xml"
#define EMV_FILE "emv.xml"

#define UI_FILE_BLK_START	1000
#define UI_FILE_BLK_END		1000
#define EMV_FILE_BLK_START	2000
#define EMV_FILE_BLK_END	2000

//file type
#define APP_FILE_M			0x00//.bin  reserve
#define	FONT_FILE			0x01//.font
#define MONITOR_FILE		0x02//.bin
//#define PARA_FILE			0x03//.ui.xml.clss.emv
#define PARAM_FILE_EMV		0x03
#define PARAM_FILE_CLSS		0x04
#define PARAM_FILE_UI		0x05
#define	PARAM_FILE_LNG		0x06
#define PARAM_FILE_BMP		0x07
#define FILE_TYPE_UNKNOWN		0xff	//Added by ZhangYuan 2016-11-01,  unknown file type

#define FILE_NAME_MAX_LEN	32
typedef struct _SFileInfo{
	uchar ucFileName[FILE_NAME_MAX_LEN+1];		//filename
	uint uiFileSize;			//file size
	uint uiFileOffset;			//file offset
	uint uiCurFileSize;			//current trans file size
	uchar ucFileContent[BLK_LENGTH_MAX];	//current trans file content
}SFileInfo;

typedef struct _SFileBlkInfo{
	uchar	ucFileName[FILENAME_LENGTH_MAX];	//filename
	ushort usFileBlkStart;						//file block start location
	ushort usFileBlkEnd;						//file block end location
	ushort usFileBlkCurIndex;					//file block current index
	ushort usFileBlkCurOffset;					//file block current offset
}SFileBlkInfo;


enum FILEDOWNLOAD_RET												
{
	FILEDOWNLOAD_ERR_BASE						=(7000),
//	FILEDOWNLOAD_ERR_BLKOVERSIZE				=(FILEDOWNLOAD_ERR_BASE+1),	//block oversize
	FILEDOWNLOAD_ERR_INVALID_PARAM				=(FILEDOWNLOAD_ERR_BASE+1), // invalid parameter 
//	FILEDOWNLOAD_ERR_PARAFILE_FAIL				=(FILEDOWNLOAD_ERR_BASE+2),	//parameter file save failed
//	FILEDOWNLOAD_ERR_FIRMFILE_FAIL				=(FILEDOWNLOAD_ERR_BASE+3),	//firmware file save failed
	FILEDOWNLOAD_ERR_DOWNLOAD_FILE_FAIL			=(FILEDOWNLOAD_ERR_BASE+2),	//firmware file save failed 
	FILEDOWNLOAD_ERR_FILE_OVERSIZE				=(FILEDOWNLOAD_ERR_BASE+3), // file over size
	FILEDOWNLOAD_ERR_NOT_ALLOWED				=(FILEDOWNLOAD_ERR_BASE+4),	// file not allowed
	FILEDOWNLOAD_ERR_PARSE_ERR					=(FILEDOWNLOAD_ERR_BASE+5),	// parse file failed
//	FILEDOWNLOAD_ERR_FIRMWARE_STEP				=(FILEDOWNLOAD_ERR_BASE+4),	//firmware file step error
//	FILEDOWNLOAD_ERR_FIRMWARE_SAVE_BEGIN		=(FILEDOWNLOAD_ERR_BASE+5),	//firmware file step begin failed
//	FILEDOWNLOAD_ERR_FIRMWARE_SAVE_WRITE		=(FILEDOWNLOAD_ERR_BASE+6),	//firmware file step write failed
//	FILEDOWNLOAD_ERR_FIRMWARE_SAVE_END			=(FILEDOWNLOAD_ERR_BASE+7),	//firmware file step end failed
	
};

int	DownloadFile(const uchar * pucRequest);

#endif
