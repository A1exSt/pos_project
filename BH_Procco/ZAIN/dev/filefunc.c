#include <posapi.h> // use PAXPayPro2010's path, but don't copy to project folder.


#include "defines.h"
#include "os_interface.h"
#include "struct.h"
#include "function.h"
#include "message.h"

#include "typedefs_vx.h"

WORD FileInit(BYTE *fpFileName)
{
    BYTE lbaFileName[5];
    int  Status;
    INT Handle;
    
    StrCpy(lbaFileName,fpFileName);
    Status = Delete(lbaFileName);

    if ( (Handle=NCreate(lbaFileName, -1)) >= 0 )
    {
        Close(Handle);
        return (STAT_INIT_OK);
    }
    else
    {
        return (STAT_NOK);
    }
}

WORD FileReset(BYTE *fpFileName)
{
    if (fexist(fpFileName)>=0)
    {
        return (STAT_OK);
    }
    
    if (FileInit(fpFileName)==STAT_INIT_OK)
        return (STAT_NOK);

    return (STAT_INV);
}

WORD AddTransaction(PTStruct *fpPTStruct )
{
    gTrHandle = Open(FILE_TR, O_RDWR|O_CREATE);
	//ComOutput("gTrHandle",9,"filefunc.c:line%dgTrHandle=%d",__LINE__,gTrHandle);
    if (gTrHandle >= 0)
    {
        seek(gTrHandle, 0, SEEK_END);

        if (Write( gTrHandle, (BYTE *)fpPTStruct, LG_TRANSSTRUCT)==0)
        {
			 Close(gTrHandle); 
             return ( STAT_OK);
        }
    }

    return ( STAT_NOK);
}

WORD AddRefNum(struct saveRetrivalRef *stRetrivalRef)
{
	int iFd;

	iFd = Open(FILE_REF, O_RDWR|O_CREATE);
	//ComOutput("open FILE_TR",strlen("open FILE_TR"),"filefunc.c:line%dRet=%d",__LINE__,iFd);
    if (iFd >= 0)
    {
        seek(iFd, 0, SEEK_END);

        if (Write(iFd,stRetrivalRef,sizeof(struct saveRetrivalRef))==0)
        {
			 Close(iFd); 
             return ( STAT_OK);
        }
    }

    return ( STAT_NOK);
}



//added by Allen 2016-05-30 
int CheckRefNumber(unsigned char* pRRN)
{
	int i, len;
	PTStruct stPtStruct;
    gTrHandle = Open(FILE_TR, O_RDWR);
    if (gTrHandle >= 0)
    {
        seek(gTrHandle, 0, SEEK_SET);
		while((len = read(gTrHandle, &stPtStruct, sizeof(PTStruct))) > 0)
		{
			if(memcmp(stPtStruct.RefNumber, pRRN, 12) == 0)
			{
				memcpy(&gPTStruct, &stPtStruct, sizeof(PTStruct));
				return STAT_OK;
			}
		}
    }
    return ( STAT_NOK);	
}

//int CheckRetrivalRef(unsigned char* RetrivalRef) //added by jeff_xiehuan
int CheckAndUpdateRetrivalRef(unsigned char* RetrivalRef,int flag)
{
	int iFd, len;
	unsigned char ucRead[13];
	struct saveRetrivalRef tempRetRef;
	struct saveRetrivalRef updateRetRef;

    iFd = Open(FILE_REF, O_RDWR);
    if (iFd >= 0)
    {
        seek(iFd, 0, SEEK_SET);
		while(1) 
		{
			memset(&tempRetRef,0,sizeof(tempRetRef));
			len = read(iFd,&tempRetRef,sizeof(struct saveRetrivalRef));
			if(len != sizeof(struct saveRetrivalRef))
			{
				break;
			}
		
			//ComOutput(RetrivalRef,12,"%dcompare,RetrivalRef",1);
			//ComOutput((uchar*)"",0,"filefunc.c,Line:%d,retrivalRef=%s,tranType=%d,hasVoidFlag=%d,gReversalType=%d",
				__LINE__,tempRetRef.retrivalRef,tempRetRef.tranType,tempRetRef.hasVoidFlag,gReversalType);

			if(memcmp(tempRetRef.retrivalRef, RetrivalRef, 12) == 0 && tempRetRef.hasVoidFlag == 0
				&& tempRetRef.tranType == gReversalType)
			{
				switch(flag)
				{
				case 0:
					Close(iFd);
					return STAT_OK;
				case 1:
					memset(&updateRetRef,0,sizeof(struct saveRetrivalRef));
					memcpy(updateRetRef.retrivalRef,RetrivalRef,12);
					updateRetRef.hasVoidFlag = 1;
					updateRetRef.tranType = tempRetRef.tranType;
					seek(iFd, -sizeof(struct saveRetrivalRef), SEEK_CUR);
					write(iFd,&updateRetRef,sizeof(struct saveRetrivalRef));
					Close(iFd);
					return STAT_OK;
				default:
					Close(iFd);
					return STAT_OK;
				}
			
			}
			else if(memcmp(tempRetRef.retrivalRef, RetrivalRef, 12) == 0 && tempRetRef.hasVoidFlag == 1 
				&& tempRetRef.tranType == gReversalType)
			{
				Close(iFd);
				return STAT_HAS_VOID;
			}
			else if(memcmp(tempRetRef.retrivalRef, RetrivalRef, 12) == 0 && tempRetRef.hasVoidFlag == 1 
				&& tempRetRef.tranType != gReversalType)
			{
				Close(iFd);
				return (STAT_NOK);	
			}
		}
    }
	close(iFd);
	//ComOutput(ucRead,12,"filefun.c_line:%d,iFd=%d,len=%d",__LINE__,iFd,len);
    return (STAT_NOK);	
}

//add end

void DeleteTempFile(void)
{
    Delete(FILE_RP);//general parametre temp file
    Delete(FILE_BT);//black card temp file
    Delete(FILE_PT);//Product temp file
    Delete(FILE_TS);//Scheme temp file
    Delete(FILE_TP);//Scheme temp file
    Delete(FILE_TEMPBIN);
    Delete(FILE_TEMPMERCH);
    Delete(csFILE_EMV_TEMP_KEYS);
}

void DeleteAllFile( void )
{
    Delete(FILE_GP);
    Delete(FILE_DP);
    Delete(FILE_BC);
    Delete(FILE_PC);
    Delete(FILE_ST);
    Delete(FILE_PE);
    Delete(FILE_BIN);
    Delete(FILE_MERCH);
    Delete(FILE_RB);
    Delete(FILE_TR);
	Delete(FILE_LAST_EOD);    //ADD BY ALLEN 20151116
    Delete(FILE_MP);
    Delete(FILE_STATE);
    Delete(FILE_R1);
    Delete(FILE_R2);
    Delete(FILE_RP);
    Delete(csFILE_EMV_KEYS);
    Delete(FILE_REVERSE);
}

void PartialFileDelete( void )
{
    Delete(FILE_GP);
    Delete(FILE_DP);
    Delete(FILE_BC);
    Delete(FILE_PC);
    Delete(FILE_ST);
    Delete(FILE_PE);
    Delete(FILE_BIN);
    Delete(FILE_MERCH);
    Delete(csFILE_EMV_KEYS);
    Delete(FILE_RB);
}

WORD InitFiles(void)
{
    WORD lwStatus;		// status to return
    
    lwStatus = STAT_OK;
    if (HDGInit()==STAT_NOK)
        lwStatus=STAT_NOK;
    if (GPRSInit()==STAT_NOK)
        lwStatus=STAT_NOK;
    //added by Allen 2016-05-19
	if (Dial_upInit()==STAT_NOK)
        lwStatus=STAT_NOK;
	//add end
    if (HDDInit()==STAT_NOK)
        lwStatus=STAT_NOK;
    
    if (FileInit(FILE_BC)==STAT_NOK)
        lwStatus=STAT_NOK;
    if (FileInit(FILE_PC)==STAT_NOK)
        lwStatus=STAT_NOK;
    if (FileInit(FILE_ST)==STAT_NOK)
        lwStatus=STAT_NOK;
    if (FileInit(FILE_PE)==STAT_NOK)
        lwStatus=STAT_NOK;
    if (FileInit(csFILE_EMV_KEYS)==STAT_NOK)
        lwStatus=STAT_NOK;
	if (FileInit(FILE_BIN)==STAT_NOK)
        lwStatus=STAT_NOK;
    if (FileInit(FILE_MERCH)==STAT_NOK)
        lwStatus=STAT_NOK;
    if (FileInit(FILE_RB)==STAT_NOK)
        lwStatus=STAT_NOK;
    
    return(lwStatus);
}
//added by Allen 2016-05-23
WORD EndOfDayFlagInit(void)
{
	int lhandle;
	if (fexist(FILE_END_OF_DAY) == -1)
    {
        if (FileInit(FILE_END_OF_DAY)==STAT_NOK)
			return STAT_NOK;
		gEODParam.EODFlag = 1;   //1- end of day automatically
		memcpy(gEODParam.szDateTime, "\x00\x00\x00\x00\x00\x00", 6);
		SaveEODFlag();
		return STAT_OK;
    }
	return STAT_OK;
}
//add end
//added by Allen 2016-05-19
WORD ConnectInit(void)
{
	int lhandle;
	if (fexist(FILE_CONNECT_MODE) == -1)
    {
        if (FileInit(FILE_CONNECT_MODE)==STAT_NOK)
			return STAT_NOK;
		gConnectMode = GPRS;
		SaveConnectMode();
		return STAT_OK;
    }
	GetConnectMode();
	return STAT_OK;
}
//add end
WORD CheckAllFiles(void)
{
    WORD Status;
    
    DeleteTempFile();
    Status = ResetStateFile();
    switch (Status)
    {
    case STAT_OK:
        break;
    case STAT_INV:
        DeleteAllFile();
        break;
    case STAT_NOK:
        PartialFileDelete();
        break;
    }
    
    Status = CheckGeneralFiles();    
    if (Status==STAT_MERCH_NOK)
    {
        gbFLagPartialInitState = OS_FALSE;
        gState.ApplStateInit = OS_NO;
    }
    if (Status==STAT_PARAM_NOK)
    {
        gbFLagPartialInitState = OS_TRUE;
        gState.ApplStateInit = OS_NO;
    }    
    if (!(Status & STAT_INV)) 
    {
        Status |= CheckTransFiles();
    }    
    if (Status & STAT_INV)
    {
        gState.ApplStateInit = OS_NO;
        gbFLagPartialInitState &= OS_TRUE;
    }
    
    return (Status);
}

WORD CheckGeneralFiles(void)
{
    WORD lwStatusHDG;
    WORD lwStatus1, lwStatus2, lwStatus3;
    
    
    lwStatusHDG = HDGReset();
    if (lwStatusHDG==STAT_INV)
        return(STAT_INV);
    
    if (lwStatusHDG==STAT_OK)
    {
        //-----------------------------------------
        lwStatus1 = FileReset(FILE_GPRS_PAR);
        if (lwStatus1==STAT_INV)
            return(STAT_INV);
		//added by Allen 2016-05-19
		lwStatus1 = FileReset(FILE_DIAL_UP_PAR);
        if (lwStatus1==STAT_INV)
            return(STAT_INV);
        //add end
        lwStatus1 = FileReset(FILE_DP);
        if (lwStatus1==STAT_INV)
            return(STAT_INV);

        lwStatus1 = FileReset(FILE_RB);
        if (lwStatus1==STAT_INV)
            return(STAT_INV);

        //-----------------------------------------
        lwStatus2 = FileReset(FILE_BC);
        if (lwStatus2==STAT_INV)
            return(STAT_INV);
        
        //-----------------------------------------
        lwStatus3 = FileReset(csFILE_EMV_KEYS);
        if (lwStatus3==STAT_INV)
            return(STAT_INV);

        lwStatus3 = FileReset(FILE_PC);
        if (lwStatus3==STAT_INV)
            return(STAT_INV);

        lwStatus3 = FileReset(FILE_ST);
        if (lwStatus3==STAT_INV)
            return(STAT_INV);

        lwStatus3 = FileReset(FILE_PE);
        if (lwStatus3==STAT_INV)
            return(STAT_INV);
        
        lwStatus3 = FileReset(FILE_BIN);
        if (lwStatus3==STAT_INV)
            return(STAT_INV);

        lwStatus3 = FileReset(FILE_MERCH);
        if (lwStatus3==STAT_INV)
            return(STAT_INV);        
        
        //-----------------------------------------
        if ((lwStatus1==STAT_NOK)
            ||(lwStatus2==STAT_NOK)
            ||(lwStatus3==STAT_NOK) )
        {
            if (InitFiles()==STAT_NOK)
                return(STAT_INV);
            
            return (STAT_PARAM_NOK);
        }
        else
        {
            return (STAT_OK);
        }
    }

    if (lwStatusHDG==STAT_NOK)
    {
        if (InitFiles()==STAT_NOK)
            return(STAT_INV);
        return (STAT_PARAM_NOK);
    }
    
    return(STAT_NOK);
}

WORD CheckTransFiles(void)     
{
    WORD lwStatus1;
    WORD lwStatus2= STAT_OK;
    WORD lwStatus3= STAT_OK;
    WORD lwStatus4 = STAT_OK;
    
    lwStatus1 = FileReset(FILE_TR);
    
    if ((lwStatus1==STAT_OK) || (lwStatus1==STAT_INV))
    {
        lwStatus2 = FileReset(FILE_R1);	// then the reset of R1
        lwStatus3 = FileReset(FILE_R2);	// and R2 file
        lwStatus4 = FileReset(FILE_R3);
    }
    
    if ((lwStatus1==STAT_NOK ))
    {
        lwStatus2 = FileInit(FILE_R1);
        lwStatus3 = FileInit(FILE_R2);
        lwStatus4 = FileInit(FILE_R3);
    }

    if ((lwStatus2==STAT_NOK ) || (lwStatus2==STAT_INIT_OK ) ||
    	 	(lwStatus4 == STAT_NOK) || (lwStatus4 == STAT_INIT_OK))
        lwStatus3 = FileInit(FILE_R2);
    
    if ((lwStatus3==STAT_NOK ) || (lwStatus3==STAT_INIT_OK ) ||
    		(lwStatus4 == STAT_NOK) || (lwStatus4 == STAT_INIT_OK))
        lwStatus2 = FileInit(FILE_R1);

    if ((lwStatus2==STAT_NOK ) || (lwStatus2==STAT_INIT_OK ) ||
    		(lwStatus3 == STAT_NOK) || (lwStatus3 == STAT_INIT_OK))
        lwStatus4 = FileInit(FILE_R3);
    
    if ((lwStatus1==STAT_OK) && (lwStatus2==STAT_OK) && (lwStatus3==STAT_OK) && (lwStatus4==STAT_OK))
    {
        return (STAT_OK);
    }
    else
    {
        if ((lwStatus1==STAT_INV) || (lwStatus2==STAT_INV) || (lwStatus3==STAT_INV) ||(lwStatus4==STAT_INV))
            return (STAT_INV);
        else
            return (STAT_NOK);
    }
}

unsigned char FileCopy(BYTE *srcFile, BYTE *destFile)     // add by Allen 20151116
{ 
    int srcHandle, destHandle, flag; 
    char buffer[1024];

	memset(buffer, '\0', sizeof(buffer));
     if (fexist(srcFile) == -1)
    {
        return (STAT_NOK);
    }    

    srcHandle = open(srcFile, O_RDWR);
    if (-1 == srcHandle)
    {    
		return (STAT_NOK);
    }
     
    destHandle = open(destFile, O_RDWR|O_CREATE);
    if (-1 == destHandle) 
    {
		return (STAT_NOK);
    }
     
    while ((flag = read(srcHandle, buffer, 1024)) > 0)
    { 
        write(destHandle, buffer, flag);
		memset(buffer, '\0', sizeof(buffer));
    }
     
    close(srcHandle);
    close(destHandle);
     
    return (STAT_OK);
} 

// end of file
