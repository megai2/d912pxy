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
#include "P7_Cproxy.h"

extern "C" 
{
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                            P7 client                                       //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
//P7_Client_Create
P7_EXPORT hP7_Client __cdecl P7_Client_Create(const tXCHAR *i_pName)
{
    return P7_Create_Client(i_pName);
}//P7_Client_Create


////////////////////////////////////////////////////////////////////////////////
//P7_Client_Get_Shared
P7_EXPORT hP7_Client __cdecl P7_Client_Get_Shared(const tXCHAR *i_pName)
{
    return P7_Get_Shared(i_pName);
}//P7_Client_Get_Shared


////////////////////////////////////////////////////////////////////////////////
//P7_Client_Share
P7_EXPORT tBOOL __cdecl P7_Client_Share(hP7_Client i_hClient, const tXCHAR *i_pName)
{
    IP7_Client *l_pClient = static_cast<IP7_Client *>(i_hClient);

    if (!l_pClient)
    {
        return FALSE;
    }

    return l_pClient->Share(i_pName);
}//P7_Client_Share


////////////////////////////////////////////////////////////////////////////////
//P7_Client_Add_Ref
P7_EXPORT tINT32 __cdecl P7_Client_Add_Ref(hP7_Client i_hClient)
{
    IP7_Client *l_pClient = static_cast<IP7_Client *>(i_hClient);

    if (!l_pClient)
    {
        return -1;
    }

    return l_pClient->Add_Ref();
}//P7_Client_Add_Ref


////////////////////////////////////////////////////////////////////////////////
//P7_Client_Release
P7_EXPORT tINT32 __cdecl P7_Client_Release(hP7_Client i_hClient)
{
    IP7_Client *l_pClient = static_cast<IP7_Client *>(i_hClient);

    if (!l_pClient)
    {
        return -1;
    }

    return l_pClient->Release();
}//P7_Client_Release


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                            P7 Telemetry                                    //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
//P7_Telemetry_Create
P7_EXPORT hP7_Telemetry __cdecl P7_Telemetry_Create(hP7_Client              i_hClient,
                                                    const tXCHAR           *i_pName,
                                                    const stTelemetry_Conf *i_pConf
                                                    )
{
    return P7_Create_Telemetry(static_cast<IP7_Client *>(i_hClient), i_pName, i_pConf); 
}//P7_Telemetry_Create


////////////////////////////////////////////////////////////////////////////////
//P7_Telemetry_Share
P7_EXPORT tBOOL __cdecl P7_Telemetry_Share(hP7_Telemetry i_hTelemetry, 
                                           const tXCHAR *i_pName
                                          )
{
    IP7_Telemetry *l_pTel = static_cast<IP7_Telemetry *>(i_hTelemetry);
    if (NULL == l_pTel)
    {
        return FALSE;
    }

    return l_pTel->Share(i_pName);
}//P7_Telemetry_Share


////////////////////////////////////////////////////////////////////////////////
//P7_Telemetry_Get_Shared
P7_EXPORT hP7_Telemetry __cdecl P7_Telemetry_Get_Shared(const tXCHAR *i_pName)
{
    return P7_Get_Shared_Telemetry(i_pName);
}//P7_Telemetry_Get_Shared


////////////////////////////////////////////////////////////////////////////////
//P7_Telemetry_Create_Counter
P7_EXPORT tBOOL __cdecl P7_Telemetry_Create_Counter(hP7_Telemetry i_hTelemetry,
                                                    const tXCHAR *i_pName, 
                                                    tINT64        i_llMin,
                                                    tINT64        i_llMax,
                                                    tINT64        i_llAlarm,
                                                    tUINT8        i_bOn,
                                                    tUINT8       *o_pCounter_ID 
                                                   )
{
    IP7_Telemetry *l_pTel = static_cast<IP7_Telemetry *>(i_hTelemetry);
    if (NULL == l_pTel)
    {
        return FALSE;
    }

    return l_pTel->Create(i_pName, 
                          i_llMin,
                          i_llMax,
                          i_llAlarm,
                          i_bOn,
                          o_pCounter_ID 
                         );
}//P7_Telemetry_Create_Counter


////////////////////////////////////////////////////////////////////////////////
//P7_Telemetry_Put_Value
P7_EXPORT tBOOL __cdecl P7_Telemetry_Put_Value(hP7_Telemetry i_hTelemetry,
                                               tUINT8        i_bCounter_ID,
                                               tINT64        i_llValue
                                               )
{
    IP7_Telemetry *l_pTel = static_cast<IP7_Telemetry *>(i_hTelemetry);
    if (NULL == l_pTel)
    {
        return FALSE;
    }

    return l_pTel->Add(i_bCounter_ID, i_llValue);
}//P7_Telemetry_Put_Value


////////////////////////////////////////////////////////////////////////////////
//P7_Telemetry_Find_Counter
P7_EXPORT tBOOL __cdecl P7_Telemetry_Find_Counter(hP7_Telemetry i_hTelemetry,
                                                  const tXCHAR *i_pName,
                                                  tUINT8       *o_pCounter_ID
                                                 )
{
    IP7_Telemetry *l_pTel = static_cast<IP7_Telemetry *>(i_hTelemetry);
    if (NULL == l_pTel)
    {
        return FALSE;
    }

    return l_pTel->Find(i_pName, o_pCounter_ID);
}//P7_Telemetry_Find_Counter


////////////////////////////////////////////////////////////////////////////////
//P7_Telemetry_Add_Ref
P7_EXPORT tINT32 __cdecl P7_Telemetry_Add_Ref(hP7_Telemetry i_hTelemetry)
{
    IP7_Telemetry *l_pTel = static_cast<IP7_Telemetry *>(i_hTelemetry);
    if (NULL == l_pTel)
    {
        return -1;
    }

    return l_pTel->Add_Ref();
}//P7_Telemetry_Add_Ref


////////////////////////////////////////////////////////////////////////////////
//P7_Telemetry_Release
P7_EXPORT tINT32 __cdecl P7_Telemetry_Release(hP7_Telemetry i_hTelemetry)
{
    IP7_Telemetry *l_pTel = static_cast<IP7_Telemetry *>(i_hTelemetry);
    if (NULL == l_pTel)
    {
        return -1;
    }

    return l_pTel->Release();
}//P7_Telemetry_Release


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                               P7 Trace                                     //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
//P7_Trace_Create_Trace
P7_EXPORT hP7_Trace __cdecl P7_Trace_Create(hP7_Client          i_hClient,
                                            const tXCHAR       *i_pName,
                                            const stTrace_Conf *i_pConf
                                           )
{
    return P7_Create_Trace(static_cast<IP7_Client *>(i_hClient), i_pName, i_pConf); 
}//P7_Trace_Create_Trace


////////////////////////////////////////////////////////////////////////////////
//P7_Trace_Get_Shared
P7_EXPORT hP7_Trace __cdecl P7_Trace_Get_Shared(const tXCHAR *i_pName)
{
    return P7_Get_Shared_Trace(i_pName);
}//P7_Trace_Get_Shared


////////////////////////////////////////////////////////////////////////////////
//P7_Trace_Share
P7_EXPORT tBOOL __cdecl P7_Trace_Share(hP7_Trace     i_hTrace, 
                                       const tXCHAR *i_pName
                                      )
{
    IP7_Trace *l_pTrace = static_cast<IP7_Trace *>(i_hTrace);
    if (NULL == l_pTrace)
    {
        return FALSE;
    }

    return l_pTrace->Share(i_pName);
}//P7_Trace_Share


////////////////////////////////////////////////////////////////////////////////
//P7_Trace_Set_Verbosity
P7_EXPORT void __cdecl P7_Trace_Set_Verbosity(hP7_Trace        i_hTrace, 
                                              hP7_Trace_Module i_hModule, 
                                              tUINT32          i_dwLevel
                                             )
{
    IP7_Trace *l_pTrace = static_cast<IP7_Trace *>(i_hTrace);
    if (NULL == l_pTrace)
    {
        return;
    }

    l_pTrace->Set_Verbosity(i_hModule, (eP7Trace_Level)i_dwLevel);
}//P7_Trace_Set_Verbosity



////////////////////////////////////////////////////////////////////////////////
//P7_Trace_Register_Thread
P7_EXPORT tBOOL __cdecl P7_Trace_Register_Thread(hP7_Trace     i_hTrace, 
                                                 const tXCHAR *i_pName,
                                                 tUINT32       i_dwThreadId
                                                )
{
    IP7_Trace *l_pTrace = static_cast<IP7_Trace *>(i_hTrace);
    if (NULL == l_pTrace)
    {
        return FALSE;
    }

    return l_pTrace->Register_Thread(i_pName, i_dwThreadId);
}//P7_Trace_Register_Thread


////////////////////////////////////////////////////////////////////////////////
//P7_Trace_Unregister_Thread
P7_EXPORT tBOOL __cdecl P7_Trace_Unregister_Thread(hP7_Trace  i_hTrace, 
                                                   tUINT32    i_dwThreadId
                                                  )
{
    IP7_Trace *l_pTrace = static_cast<IP7_Trace *>(i_hTrace);
    if (NULL == l_pTrace)
    {
        return FALSE;
    }

    return l_pTrace->Unregister_Thread(i_dwThreadId);
}//P7_Trace_Unregister_Thread


////////////////////////////////////////////////////////////////////////////////
//P7_Trace_Register_Module
P7_EXPORT hP7_Trace_Module __cdecl P7_Trace_Register_Module(hP7_Trace     i_hTrace, 
                                                            const tXCHAR *i_pName 
                                                           )
{
    IP7_Trace       *l_pTrace = static_cast<IP7_Trace *>(i_hTrace);
    hP7_Trace_Module l_hMpodule = NULL;

    if (NULL == l_pTrace)
    {
        return NULL;
    }

    if (l_pTrace->Register_Module(i_pName, &l_hMpodule))
    {
        return l_hMpodule;
    }

    return NULL;
}//P7_Trace_Register_Module


////////////////////////////////////////////////////////////////////////////////
//P7_Trace_Add
P7_EXPORT tBOOL __cdecl P7_Trace_Add(hP7_Trace        i_hTrace,
                                     tUINT16          i_wTrace_ID,   
                                     tUINT32          i_dwLevel, 
                                     hP7_Trace_Module i_hModule,
                                     tUINT16          i_wLine,
                                     const char      *i_pFile,
                                     const char      *i_pFunction,
                                     const tXCHAR    *i_pFormat,
                                     ...
                                    )
{
    IP7_Trace *l_pTrace = static_cast<IP7_Trace *>(i_hTrace);
    if (NULL == l_pTrace)
    {
        return FALSE;
    }

#if defined(P7TRACE_NO_VA_ARG_OPTIMIZATION)
    va_list l_pVl;
    va_start(l_pVl, i_pFormat);
    tBOOL l_bRet = l_pTrace->Trace_Embedded(i_wTrace_ID, 
                                            (eP7Trace_Level)i_dwLevel, 
                                            i_hModule,
                                            i_wLine,
                                            i_pFile,
                                            i_pFunction,
                                            &i_pFormat,
                                            &l_pVl
                                           );
    va_end(l_pVl);
    return l_bRet;
#else
    return l_pTrace->Trace_Embedded(i_wTrace_ID, 
                                    (eP7Trace_Level)i_dwLevel, 
                                    i_hModule,
                                    i_wLine,
                                    i_pFile,
                                    i_pFunction,
                                    &i_pFormat
                                   );
#endif
}//P7_Trace_Add


////////////////////////////////////////////////////////////////////////////////
//P7_Trace_Embedded
P7_EXPORT tBOOL __cdecl P7_Trace_Embedded(hP7_Trace        i_hTrace,
                                          tUINT16          i_wTrace_ID,   
                                          tUINT32          i_dwLevel, 
                                          hP7_Trace_Module i_hModule,
                                          tUINT16          i_wLine,
                                          const char      *i_pFile,
                                          const char      *i_pFunction,
                                          const tXCHAR   **i_ppFormat,
                                          va_list         *i_pVa_List
                                         )
{
    IP7_Trace *l_pTrace = static_cast<IP7_Trace *>(i_hTrace);
    if (NULL == l_pTrace)
    {
        return FALSE;
    }

    return l_pTrace->Trace_Embedded(i_wTrace_ID, 
                                    (eP7Trace_Level)i_dwLevel, 
                                    i_hModule,
                                    i_wLine,
                                    i_pFile,
                                    i_pFunction,
                                    i_ppFormat,
                                    i_pVa_List
                                   );
}//P7_Trace_Embedded


////////////////////////////////////////////////////////////////////////////////
//P7_Trace_Managed
P7_EXPORT extern tBOOL __cdecl P7_Trace_Managed(hP7_Trace        i_hTrace,
                                                tUINT16          i_wTrace_ID,   
                                                tUINT32          i_dwLevel,
                                                hP7_Trace_Module i_hModule,
                                                tUINT16          i_wLine,
                                                const tXCHAR    *i_pFile,
                                                const tXCHAR    *i_pFunction,
                                                const tXCHAR    *i_pMessage
                                               )
{
    IP7_Trace *l_pTrace = static_cast<IP7_Trace *>(i_hTrace);
    if (NULL == l_pTrace)
    {
        return FALSE;
    }

    return l_pTrace->Trace_Managed(i_wTrace_ID, 
                                   (eP7Trace_Level)i_dwLevel, 
                                   i_hModule,
                                   i_wLine,
                                   i_pFile,
                                   i_pFunction,
                                   i_pMessage
                                  );
}//P7_Trace_Managed


////////////////////////////////////////////////////////////////////////////////
//P7_Trace_Add_Ref
P7_EXPORT tINT32 __cdecl P7_Trace_Add_Ref(hP7_Trace i_hTrace)
{
    IP7_Trace *l_pTrace = static_cast<IP7_Trace *>(i_hTrace);
    if (NULL == l_pTrace)
    {
        return -1;
    }

    return l_pTrace->Add_Ref();
}//P7_Trace_Add_Ref


////////////////////////////////////////////////////////////////////////////////
//P7_Trace_Release
P7_EXPORT tINT32 __cdecl P7_Trace_Release(hP7_Trace i_hTrace)
{
    IP7_Trace *l_pTrace = static_cast<IP7_Trace *>(i_hTrace);
    if (NULL == l_pTrace)
    {
        return -1;
    }

    return l_pTrace->Release();
}//P7_Trace_Release


} //extern "C" 

