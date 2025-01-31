/*
 * ============================================================================
 * COPYRIGHT
 *              Pax CORPORATION PROPRIETARY INFORMATION
 *   This software is supplied under the terms of a license agreement or
 *   nondisclosure agreement with Pax Corporation and may not be copied
 *   or disclosed except in accordance with the terms in that agreement.
 *      Copyright (C) 2016 Pax Corporation. All rights reserved.
 * Module Date:2016/09/05     
 * Module Auth:huangwp     
 * Description:file module

 *      
 * ============================================================================
 */


#ifndef _FILE_H
#define _FILE_H

#ifndef PDK_SEEK_CUR
#define PDK_SEEK_CUR	0
#endif
#ifndef PDK_SEEK_SET	 
#define	PDK_SEEK_SET	1
#endif
#ifndef PDK_SEET_END
#define	PDK_SEEK_END	2
#endif

#define FILE_ERR_BASE                   -2400
#define FILE_ERR_INVALID_PARAM          (FILE_ERR_BASE-1)
#define FILE_ERR_OPEN_FAIL              (FILE_ERR_BASE-2)
#define FILE_ERR_WRITE_FAIL             (FILE_ERR_BASE-3)
#define FILE_ERR_READ_FAIL              (FILE_ERR_BASE-4)
#define FILE_ERR_MEM_OVERFLOW           (FILE_ERR_BASE-5)
#define FILE_ERR_END_OVERFLOW           (FILE_ERR_BASE-6)
#define FILE_ERR_NOT_EXIST              (FILE_ERR_BASE-7)
#define FILE_ERR_NOT_OPENED             (FILE_ERR_BASE-8)
#define FILE_ERR_OPENED                 (FILE_ERR_BASE-9)
#define FILE_ERR_OTHER                  (FILE_ERR_BASE-10)

#define PDK_RET_OK                     0

//file seek

#define MAX_FILE_SIZE                  (1024*100)
#define MAX_FILE_CNT                   60
#define FILE_BLK_NUM                   100
#define SAVE_FILE_STAART_BLK           50

#define FILE_NAME_LEN                  32

//file type
#define FILE_TYPE_SMALL                0     //smallsize file's max size is 10k
#define FILE_TYPE_MID                  1     //middsize file's max size is 30K
#define FILE_TYPE_LARGE                2     //large file's max size is 100k


//file's number
#define SMALL_SIZE_FILE_CNT            30    //the number of samll size file
#define MID_SIZE_FILE_CNT              20	 //the number of mid size file
#define LARGE_SIZE_FILE_CNT            10    //the number of lager size file

//file size
#define SAMLL_FILE_SIZE                10   //10K
#define MID_FILE_SIZE                  30   //30K
#define LARGE_FILE_SIZE                100  //100k

//file size
#define SAMLL_FILE_BLK_NUM            (SMALL_SIZE_FILE_CNT*SAMLL_FILE_SIZE)//300
#define MID_FILE_BLK_NUM              (MID_SIZE_FILE_CNT*MID_FILE_SIZE)//600
#define LARGE_FILE_BLK_NUM            (LARGE_SIZE_FILE_CNT*LARGE_FILE_SIZE)//1000

#define FILE_INFO_NAME                "File_Info"

#define FILE_INFO_START_BLK            1


typedef struct FileInfo
{
	uchar ucFileName[FILE_NAME_LEN];
	int iStartBlk;//fd
	//ushort iBlkCnt;
	uchar iUseFlag;//use to flag if this array[i] is used
	uchar iOpenFlag;
	uint iCurrent;//seek
	uint iLen;
}FILE_INFO_T;

typedef struct FileTable
{
	uchar ucFileName[FILE_NAME_LEN]; 
	uchar iFileType;
	
}FILE_TABLE_T;


typedef struct FileStatistics
{
	int Cnt;
	ulong Total;

}stFileStatistics;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int PdkFile_Open(const char *pszFileName);
int PdkFile_Exist(const char * pszFileName);
int PdkFile_Read(int iFd, uchar *psDataOut, uint uiLen);
int PdkFile_Write(int iFd, const uchar *psDataIn, uint uiLen);
int PdkFile_Close(int iFd);
int PdkFile_Seek(int iFd, int iOffSet, uint uiFromWhere);
int PdkFile_GetSize(const char * pszFileName);
int PdkFile_Truncate(int iFd, uint uiLen);
int PdkFile_Remove (const char * pszFileName);
void SysFileInit(void);
int Test_File(void);



#ifdef __cplusplus
}
#endif /* __cplusplus */

#pragma pack()


#endif

