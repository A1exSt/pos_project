
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
 * Description:bluetooth communicate set
 *      
 * ============================================================================
 */


#ifndef _BLUETOOTH_H
#define _BLUETOOTH_H
#include "global.h"

enum
{
	BT_RET_OK = 0,
	BT_RET_ERROR_NOTCLOSE = 1,
	BT_RET_ERROR_PORTERROR = 2,
}BT_RETURN_CODE;

enum 
{
	T_eBtCmdGetModet = 0,
    T_eBtCmdSetMode,
    T_eBtCmdGetConfig,
    T_eBtCmdSetConfig,
    T_eBtCmdReset,
    T_eBtCmdDropLink,
    T_eBtCmdGetLinkStatus,
    T_eBtCmdDisableBroadcast,
    T_eBtCmdEnableBroadcast,
    T_eBtCmdDisablePIN,
    T_eBtCmdEnablePIN,
    T_eBtCmdDisableLinkback,
    T_eBtCmdEnableLinkBack,
    T_eBtCmdEnablePasskey,
    T_eBtCmdDisablePasskey,
    T_eBtCmdGetPasskeyPairRequest,
    T_eBtCmdSetPasskeyPairPassword,	
}E_BtCtlCmd_T;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int BtOpen(void);
int BtPairKey(void);

int InitBlueTooth(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#pragma pack()

#endif
