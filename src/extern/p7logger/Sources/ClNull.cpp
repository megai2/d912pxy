////////////////////////////////////////////////////////////////////////////////
//                                                                             /
// 2012-2017 (c) Baical                                                        /
//                                                                             /
// This library is free software; you can redistribute it and/or               /
// modify it under the terms of the GNU Lesser General Public                  /
// License as published by the Free Software Foundation; either                /
// version 3.0 of the License, or (at your option) any later version.          /
//                                                                             /
// This library is distributed in the hope that it will be useful,             /
// but WITHOUT ANY WARRANTY; without even the implied warranty of              /
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU           /
// Lesser General Public License for more details.                             /
//                                                                             /
// You should have received a copy of the GNU Lesser General Public            /
// License along with this library.                                            /
//                                                                             /
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// This header file provide functionality to deliver data to NULL              /
////////////////////////////////////////////////////////////////////////////////

#include "CommonClient.h"
#include "ClNull.h"

////////////////////////////////////////////////////////////////////////////////
//CClNull()
CClNull::CClNull(tXCHAR **i_pArgs,
                 tINT32   i_iCount
                )
    : CClient(IP7_Client::eNull, i_pArgs, i_iCount)
{
    Init_Base(i_pArgs, i_iCount);
}//CClNull()


////////////////////////////////////////////////////////////////////////////////
//~CClNull()
CClNull::~CClNull()
{
    Flush();
    CClient::Unshare();
}//~CClNull()


////////////////////////////////////////////////////////////////////////////////
//Init_Base
eClient_Status CClNull::Init_Base(tXCHAR **i_pArgs,
                                  tINT32   i_iCount
                                 )
{
    UNUSED_ARG(i_pArgs);
    UNUSED_ARG(i_iCount);

    return ECLIENT_STATUS_OK;
}//Init_Base


////////////////////////////////////////////////////////////////////////////////
//Sent
eClient_Status CClNull::Sent(tUINT32          i_dwChannel_ID,
                             sP7C_Data_Chunk *i_pChunks, 
                             tUINT32          i_dwCount,
                             tUINT32          i_dwSize
                            )
{
    UNUSED_ARG(i_dwChannel_ID);
    UNUSED_ARG(i_pChunks);
    UNUSED_ARG(i_dwCount);
    UNUSED_ARG(i_dwSize);

    return  ECLIENT_STATUS_OK;
}//Sent


////////////////////////////////////////////////////////////////////////////////
//Get_Status
tBOOL CClNull::Get_Status(sP7C_Status *o_pStatus)
{
    if (NULL == o_pStatus)
    {
        return FALSE;
    }

    o_pStatus->bConnected = FALSE;
    o_pStatus->dwResets   = 0;

    return TRUE;
}//Get_Status


////////////////////////////////////////////////////////////////////////////////
//Get_Info
tBOOL CClNull::Get_Info(sP7C_Info *o_pInfo)
{
    if (NULL == o_pInfo)
    {
        return FALSE;
    }

    o_pInfo->dwReject_Mem = 0;
    o_pInfo->dwReject_Con = 0;
    o_pInfo->dwReject_Int = 0;

    return TRUE;
}//Get_Info


////////////////////////////////////////////////////////////////////////////////
//Flush
tBOOL CClNull::Flush()
{
    return TRUE;
}//Flush
