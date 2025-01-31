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
* Description:soft timer module

*      
* ============================================================================
*/


#include "..\inc\global.h"

static ulong sg_ulSofTimer[SOF_TIMER_TOT][2];

//check if timer is inited , 0 means has benn inited
static  uchar sg_ucSofTMInit = 0xff;

/*
* ============================================================================
* Function:    SofTimerInit
* Description: init soft timer
* Returns:     
* Parameter:  
* ============================================================================
*/

void SofTimerInit()
{
	int i = 0;
	if(sg_ucSofTMInit) 
	{
		for(i = 0; i < SOF_TIMER_TOT; i++)
		{
			sg_ulSofTimer[i][1] = 0;
		}
		sg_ucSofTMInit = 0;
	}
}


void SofTimerSet(uchar ucTMID, ulong  ulMs)
{
	unsigned char ucTimerID = ucTMID % SOF_TIMER_TOT;

	if(sg_ucSofTMInit)
	{
		SofTimerInit();
	}

	sg_ulSofTimer[ucTimerID][0] = GetTimerCount() / 10;

	sg_ulSofTimer[ucTimerID][1] = (ulMs + 9) / 10;
}

ulong SofTimerCheck(uchar ucTMID)
{
	ulong ulRet = -1;
	ulong ulCount = -1;
	uchar ucTimerID = ucTMID % SOF_TIMER_TOT;

	if(sg_ucSofTMInit)
	{
		SofTimerInit();

	}
	if(sg_ulSofTimer[ucTimerID][1] == 0) 
	{
		return 0;
	}
	ulCount =  GetTimerCount() / 10;
	if(ulCount >= sg_ulSofTimer[ucTimerID][0])
	{
		if(sg_ulSofTimer[ucTimerID][1] < ulCount - sg_ulSofTimer[ucTimerID][0])
		{
			ulRet = 0;
		}
		else 
		{
			ulRet = sg_ulSofTimer[ucTimerID][1] - ( ulCount - sg_ulSofTimer[ucTimerID][0]);
		}
	}
	else 
	{
		if(sg_ulSofTimer[ucTimerID][1] < (ulCount+(0xffffffff - sg_ulSofTimer[ucTimerID][0])))
		{
			ulRet = 0;
		}
		else  
		{
			ulRet = sg_ulSofTimer[ucTimerID][1] - (ulCount+0xffffffff - sg_ulSofTimer[ucTimerID][0]);
		}
	}
	return ulRet;

}
/**  @} */
