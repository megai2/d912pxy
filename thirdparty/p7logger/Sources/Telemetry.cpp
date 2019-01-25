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
// This file provide telemetry functionality                                   /
////////////////////////////////////////////////////////////////////////////////
#include "CommonClient.h"
#include "Telemetry.h"


#define RESET_UNDEFINED                                           (0xFFFFFFFFUL)
#define RESET_FLAG_CHANNEL                                        (0x1)
#define RESET_FLAG_COUNTER                                        (0x2)
#define TELEMETRY_SHARED_PREFIX                                   TM("Trc_")

extern "C" 
{

////////////////////////////////////////////////////////////////////////////////
//P7_Create_Telemetry
P7_EXPORT IP7_Telemetry * __cdecl P7_Create_Telemetry(IP7_Client             *i_pClient, 
                                                      const tXCHAR           *i_pName, 
                                                      const stTelemetry_Conf *i_pConf
                                                      )
{
    //telemetry isn't supported by next sinks
    if (    (!i_pClient)
         || (IP7_Client::eText <= i_pClient->Get_Type())
       )
    {
        return NULL;
    }

    //Check parameters
    if (i_pConf)
    {
        if (    (    (!i_pConf->qwTimestamp_Frequency)
                  && (i_pConf->pTimestamp_Callback)
                )
             || (    (i_pConf->qwTimestamp_Frequency)
                  && (!i_pConf->pTimestamp_Callback)
                )
           )
        {
            return NULL;    
        }
    }

    CP7Telemetry *l_pReturn = new CP7Telemetry(i_pClient, i_pName, i_pConf);

    //if not initialized - remove
    if (     (l_pReturn)
         &&  (TRUE != l_pReturn->Is_Initialized())
       )
    {
        l_pReturn->Release();
        l_pReturn = NULL;
    }

    return static_cast<IP7_Telemetry *>(l_pReturn);
}//P7_Create_Telemetry


////////////////////////////////////////////////////////////////////////////////
//P7_Get_Shared_Trace
P7_EXPORT IP7_Telemetry * __cdecl P7_Get_Shared_Telemetry(const tXCHAR *i_pName)
{
    IP7_Telemetry *l_pReturn = NULL;
    tUINT32        l_dwLen1  = PStrLen(TELEMETRY_SHARED_PREFIX);
    tUINT32        l_dwLen2  = PStrLen(i_pName);
    tXCHAR        *l_pName   = (tXCHAR *)malloc(sizeof(tXCHAR) * (l_dwLen1 + l_dwLen2 + 16));

    if (l_pName)
    {
        PStrCpy(l_pName, l_dwLen1 + l_dwLen2 + 16, TELEMETRY_SHARED_PREFIX);
        PStrCpy(l_pName + l_dwLen1, l_dwLen2 + 16, i_pName);
        if (CShared::E_OK == CShared::Lock(l_pName, 250))
        {
            if (CShared::Read(l_pName, (tUINT8*)&l_pReturn, sizeof(IP7_Telemetry*)))
            {
                if (l_pReturn)
                {
                    l_pReturn->Add_Ref();
                }
            }
            else
            {
                l_pReturn = NULL;
            }
        }

        CShared::UnLock(l_pName);
        free(l_pName);
        l_pName = NULL;
    }

   return l_pReturn;
}//P7_Get_Shared_Trace

} //extern "C"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// CP7Tel_Counter                                       
CP7Tel_Counter::CP7Tel_Counter(tUINT8        i_bID,
                               tUINT8        i_bOn,
                               tINT64        i_llMin,
                               tINT64        i_llMax,
                               tINT64        i_llAlarm,
                               const tXCHAR *i_pName
                              )
    : m_bInitialized(TRUE)
    , m_bSeqN(0)
{
    memset(&m_sCounter, 0, sizeof(sP7Tel_Counter));

    INIT_EXT_HEADER(m_sCounter.sCommonRaw, EP7USER_TYPE_TELEMETRY, EP7TEL_TYPE_COUNTER, sizeof(sP7Tel_Counter));
    //m_sCounter.sCommon.dwSize     = sizeof(sP7Tel_Counter);
    //m_sCounter.sCommon.dwType     = EP7USER_TYPE_TELEMETRY;
    //m_sCounter.sCommon.dwSubType  = EP7TEL_TYPE_COUNTER;

    m_sCounter.bID     = i_bID;
    m_sCounter.bOn     = i_bOn;
    m_sCounter.llAlarm = i_llAlarm;
    m_sCounter.llMax   = i_llMax;
    m_sCounter.llMin   = i_llMin;
    
    PUStrCpy(m_sCounter.pName,
             P7TELEMETRY_NAME_LENGTH,
             i_pName
            );
         
} // CP7Tel_Counter   


////////////////////////////////////////////////////////////////////////////////
// ~CP7Tel_Counter                                       
CP7Tel_Counter::~CP7Tel_Counter()
{
}// ~CP7Trace_Item                                       


////////////////////////////////////////////////////////////////////////////////
// Has_Name                                       
tBOOL CP7Tel_Counter::Has_Name(const tWCHAR *i_pName)
{
    tBOOL   l_bResult = TRUE;
    tWCHAR *l_pName   = m_sCounter.pName;
    tUINT32 l_dwLen   = 0;


    if (    (FALSE == m_bInitialized)
         || (NULL == i_pName)
       )
    {
        return FALSE;
    }

    while (    (*i_pName)
            || (*l_pName)
          )
    {
        if ((*i_pName) != (*l_pName))
        {
            l_bResult = FALSE;
            break;
        }
        else
        {
            i_pName ++;
            l_pName ++;
        }

        l_dwLen ++;

        if (P7TELEMETRY_COUNTER_NAME_LENGTH <= l_dwLen)
        {
            break;
        }
    }

    return l_bResult;
}// Has_Name


////////////////////////////////////////////////////////////////////////////////
// Is_Initialized                                       
tBOOL CP7Tel_Counter::Is_Initialized()
{
    return m_bInitialized;
}// Is_Initialized


////////////////////////////////////////////////////////////////////////////////
// Enable 
void CP7Tel_Counter::Enable(tUINT8 i_bOn)
{
    m_sCounter.bOn = i_bOn;
}// Enable



////////////////////////////////////////////////////////////////////////////////
// CP7Telemetry                                       
CP7Telemetry::CP7Telemetry(IP7_Client *i_pClient, const tXCHAR *i_pName, const stTelemetry_Conf *i_pConf)
    : m_lReference(1)
    , m_pClient(i_pClient)
    , m_dwChannel_ID(0)
    , m_bInitialized(TRUE)
    , m_dwUsed(0)
    , m_dwResets_Channel(RESET_UNDEFINED)
    , m_dwResets_Counters(RESET_UNDEFINED)
    , m_pChunks(NULL)
    , m_dwChunks_Max_Count(P7TELEMETRY_COUNTERS_MAX_COUNT + 8)
    , m_bIs_Channel(FALSE)
    , m_hShared(NULL)
{
    if (i_pConf)
    {
        m_sConf = *i_pConf;
    }
    else
    {
        memset(&m_sConf, 0, sizeof(m_sConf));
    }

    memset(&m_sCS, 0, sizeof(m_sCS));
   
    LOCK_CREATE(m_sCS);

    memset(&m_sHeader_Info, 0, sizeof(m_sHeader_Info));
    memset(&m_sValue, 0, sizeof(m_sValue));

    memset(m_pCounters, 0, sizeof(m_pCounters));
    m_pChunks = new sP7C_Data_Chunk[m_dwChunks_Max_Count];  

    m_sStatus.bConnected = TRUE;
    m_sStatus.dwResets   = 0;

    if (NULL == m_pClient)
    {
        m_bInitialized = FALSE;
    }
    else
    {
        m_pClient->Add_Ref();
    }

    if (m_bInitialized)
    {
        INIT_EXT_HEADER(m_sHeader_Info.sCommonRaw, EP7USER_TYPE_TELEMETRY, EP7TEL_TYPE_INFO, sizeof(sP7Trace_Info));
        //m_sHeader_Info.sCommon.dwSize    = sizeof(sP7Trace_Info);
        //m_sHeader_Info.sCommon.dwType    = EP7USER_TYPE_TELEMETRY;
        //m_sHeader_Info.sCommon.dwSubType = EP7TEL_TYPE_INFO;

        if (i_pName)
        {
            PUStrCpy(m_sHeader_Info.pName, P7TRACE_NAME_LENGTH, i_pName);
        }
        else
        {
            PUStrCpy(m_sHeader_Info.pName, P7TRACE_NAME_LENGTH, TM("Unknown"));
        }

        if (m_sConf.qwTimestamp_Frequency)
        {
            m_sHeader_Info.qwTimer_Frequency = m_sConf.qwTimestamp_Frequency;
            m_sHeader_Info.qwTimer_Value     = m_sConf.pTimestamp_Callback(m_sConf.pContext);
        }
        else
        {
            m_sHeader_Info.qwTimer_Frequency = GetPerformanceFrequency();
            m_sHeader_Info.qwTimer_Value     = GetPerformanceCounter();
        }
    
        GetEpochTime(&m_sHeader_Info.dwTime_Hi, &m_sHeader_Info.dwTime_Lo);

        m_sHeader_Info.qwFlags   = 0;

        INIT_EXT_HEADER(m_sValue.sCommonRaw, EP7USER_TYPE_TELEMETRY, EP7TEL_TYPE_VALUE, sizeof(sP7Tel_Value));
        //m_sValue.sCommon.dwSize         = sizeof(sP7Tel_Value); 
        //m_sValue.sCommon.dwType         = EP7USER_TYPE_TELEMETRY;
        //m_sValue.sCommon.dwSubType      = EP7TEL_TYPE_VALUE;
    }

    if (m_bInitialized)
    {
        m_bIs_Channel  = (ECLIENT_STATUS_OK == m_pClient->Register_Channel(this));
        m_bInitialized = m_bIs_Channel;
    }
}// CP7Telemetry


////////////////////////////////////////////////////////////////////////////////
// ~CP7Telemetry                                       
CP7Telemetry::~CP7Telemetry()
{
    LOCK_ENTER(m_sCS);
    LOCK_EXIT(m_sCS);

    if (m_hShared)
    {
        CShared::Close(m_hShared);
        m_hShared = NULL;
    }

    if (m_bIs_Channel)
    {
        Flush();

        m_pClient->Unregister_Channel(m_dwChannel_ID);    
    }

    for (tUINT32 l_dwI = 0; l_dwI < P7TELEMETRY_COUNTERS_MAX_COUNT; l_dwI++)
    {
        if (m_pCounters[l_dwI])
        {
            delete m_pCounters[l_dwI];
            m_pCounters[l_dwI] = NULL;
        }
    }

    if (m_pClient)
    {
        m_pClient->Release();
        m_pClient = NULL;
    }

    if (m_pChunks)
    {
        delete [] m_pChunks;
        m_pChunks = NULL;
    }


    LOCK_DESTROY(m_sCS);
}// ~CP7Telemetry                                      


////////////////////////////////////////////////////////////////////////////////
// Is_Initialized                                      
tBOOL CP7Telemetry::Is_Initialized()
{
    return m_bInitialized;
}// Is_Initialized


////////////////////////////////////////////////////////////////////////////////
// On_Init                                      
void CP7Telemetry::On_Init(sP7C_Channel_Info *i_pInfo)
{
    if (i_pInfo)
    {
        m_dwChannel_ID = i_pInfo->dwID;
    }
}// On_Init


////////////////////////////////////////////////////////////////////////////////
// On_Receive                                      
void CP7Telemetry::On_Receive(tUINT32 i_dwChannel, tUINT8 *i_pBuffer, tUINT32 i_dwSize)
{
    UNUSED_ARG(i_dwChannel);

    LOCK_ENTER(m_sCS);

    if (    (i_pBuffer)
         && (i_dwSize > sizeof(sP7Ext_Header))
       )
    {
        sP7Ext_Raw l_sHeader = *(sP7Ext_Raw*)i_pBuffer;

        if (    (EP7USER_TYPE_TELEMETRY == GET_EXT_HEADER_TYPE(l_sHeader))
             && (EP7TEL_TYPE_ENABLE == GET_EXT_HEADER_SUBTYPE(l_sHeader))
           )
        {
            sP7Tel_Enable *l_pEnable = (sP7Tel_Enable*)i_pBuffer;

            if (m_pCounters[l_pEnable->bID])
            {
                m_pCounters[l_pEnable->bID]->Enable(l_pEnable->bOn);

                if (m_sConf.pEnable_Callback)
                {
                    m_sConf.pEnable_Callback(m_sConf.pContext, l_pEnable->bID, l_pEnable->bOn);
                }
            }
        }
    }

    LOCK_EXIT(m_sCS);
}// On_Receive 


////////////////////////////////////////////////////////////////////////////////
// On_Status                                      
void CP7Telemetry::On_Status(tUINT32 i_dwChannel, const sP7C_Status *i_pStatus)
{
    UNUSED_ARG(i_dwChannel);

    LOCK_ENTER(m_sCS);

    if (i_pStatus)
    {
        m_sStatus = *i_pStatus;

        if (m_sConf.pConnect_Callback)
        {
            m_sConf.pConnect_Callback(m_sConf.pContext, m_sStatus.bConnected);
        }

        //if connection was established - sent counters description
        if (m_sStatus.bConnected)
        {
            CP7Tel_Counter **l_pIter  = m_pCounters;
            sP7C_Data_Chunk *l_pChunk = m_pChunks;
            tUINT32          l_dwSize = 0;

            l_pChunk->dwSize   = sizeof(m_sHeader_Info);
            l_pChunk->pData    = &m_sHeader_Info;
            l_dwSize          += l_pChunk->dwSize;
            l_pChunk ++;

            while (*l_pIter)
            {
                l_pChunk->pData  = &((*l_pIter)->m_sCounter);
                l_pChunk->dwSize = GET_EXT_HEADER_SIZE((*l_pIter)->m_sCounter.sCommonRaw);//(*l_pIter)->m_sCounter.sCommon.dwSize;
                l_dwSize        += l_pChunk->dwSize;
                l_pChunk ++;
                l_pIter++;
            }

            if (ECLIENT_STATUS_OK == m_pClient->Sent(m_dwChannel_ID, m_pChunks, (tUINT32)(l_pChunk - m_pChunks), l_dwSize))
            {
                m_dwResets_Counters = m_sStatus.dwResets;
                m_dwResets_Channel  = m_sStatus.dwResets;
            }
        }
    }

    LOCK_EXIT(m_sCS);
}// On_Status


////////////////////////////////////////////////////////////////////////////////
// On_Flush - external call                                     
void CP7Telemetry::On_Flush(tUINT32 i_dwChannel, tBOOL *io_pCrash)
{
    UNUSED_ARG(i_dwChannel);

    LOCK_ENTER(m_sCS);

    if (    (io_pCrash)
         && (TRUE == *io_pCrash)
         && (m_bInitialized)
       )
    {
        //nothing special for crash event
    }

    Flush();

    LOCK_EXIT(m_sCS);
}// On_Flush                                      


////////////////////////////////////////////////////////////////////////////////
// Create, i_pName is case sensitive and should be unique 
tBOOL CP7Telemetry::Create(const tXCHAR  *i_pName, 
                           tINT64         i_llMin,
                           tINT64         i_llMax,
                           tINT64         i_llAlarm,
                           tUINT8         i_bOn,
                           tUINT8        *o_pID 
                          )
{
    tBOOL        l_bReturn = FALSE;
    sCounterMap *l_pMap    = NULL;

    if (    (FALSE == m_bInitialized)
         || (NULL == i_pName)
         || (NULL == o_pID)
         || (PStrLen(i_pName) >= P7TELEMETRY_NAME_LENGTH)
       )
    {
        return FALSE;
    }

    LOCK_ENTER(m_sCS);

    *o_pID = P7TELEMETRY_INVALID_ID;

    ////////////////////////////////////////////////////////////////////////////
    //find by name already existing counter
    l_pMap = m_cCounters_Map.Find(i_pName);
    if (    (l_pMap)
         && (m_pCounters[l_pMap->bID])
       )
    {   //found, and all parameters are the same ... 
        if (    (m_pCounters[l_pMap->bID]->m_sCounter.llMin   == i_llMin)
             && (m_pCounters[l_pMap->bID]->m_sCounter.llMax   == i_llMax)
             && (m_pCounters[l_pMap->bID]->m_sCounter.llAlarm == i_llAlarm)
           )
        {
            *o_pID    = l_pMap->bID;
            l_bReturn = TRUE;
            goto l_lblExit;
        }
        else //name is equal, but some additional parameters are different
        {
            goto l_lblExit;
        }
    }

    if (P7TELEMETRY_INVALID_ID <= m_dwUsed)
    {
        goto l_lblExit;
    }

    m_pCounters[m_dwUsed] = new CP7Tel_Counter((tUINT8)m_dwUsed,
                                               i_bOn,
                                               i_llMin,
                                               i_llMax,
                                               i_llAlarm,
                                               i_pName
                                              );
    if (    (NULL == m_pCounters[m_dwUsed])
         || (FALSE == m_pCounters[m_dwUsed]->Is_Initialized())
       )
    {
        if (m_pCounters[m_dwUsed])
        {
            delete m_pCounters[m_dwUsed];
            m_pCounters[m_dwUsed] = NULL;
        }

        goto l_lblExit;
    }

    m_cCounters_Map.Push(new sCounterMap(i_pName, m_dwUsed), i_pName);

    if (m_sStatus.bConnected)
    {
        sP7C_Data_Chunk *l_pChunk = m_pChunks;
        tUINT8           l_bReset = 0;
        tUINT32          l_dwSize = 0;

        //connection was lost, we need to resend initial data 
        if (m_dwResets_Channel != m_sStatus.dwResets)
        {
            m_dwResets_Channel = m_sStatus.dwResets;

            l_bReset          |= RESET_FLAG_CHANNEL;
            l_pChunk->dwSize   = sizeof(m_sHeader_Info);
            l_pChunk->pData    = &m_sHeader_Info;
            l_dwSize          += l_pChunk->dwSize;
            l_pChunk ++;
        }

        l_pChunk->pData  = &(m_pCounters[m_dwUsed]->m_sCounter);
        l_pChunk->dwSize = GET_EXT_HEADER_SIZE(m_pCounters[m_dwUsed]->m_sCounter.sCommonRaw);//m_pCounters[m_dwUsed]->m_sCounter.sCommon.dwSize;
        l_dwSize        += l_pChunk->dwSize;
        l_pChunk ++;

        if (ECLIENT_STATUS_OK != m_pClient->Sent(m_dwChannel_ID, m_pChunks, (tUINT32)(l_pChunk - m_pChunks), l_dwSize))
        {
            m_dwResets_Counters = RESET_UNDEFINED;

            if (RESET_FLAG_CHANNEL & l_bReset)
            {
                m_dwResets_Channel = RESET_UNDEFINED;
            }
        }
    }
    else
    {
        m_dwResets_Counters = RESET_UNDEFINED;
    }

    l_bReturn = TRUE;
    *o_pID    = (tUINT8)m_dwUsed;
    m_dwUsed ++;

l_lblExit:
    LOCK_EXIT(m_sCS);

    return l_bReturn;
} //Create


////////////////////////////////////////////////////////////////////////////////
// Find, i_pName is case sensitive
tBOOL CP7Telemetry::Find(const tXCHAR *i_pName, tUINT8 *o_pID)
{
    sCounterMap *l_pMap    = NULL;
    tBOOL        l_bReturn = FALSE;
    
    if (    (FALSE == m_bInitialized)
         || (NULL == i_pName)
         || (NULL == o_pID)
       )
    {
        return FALSE;
    }

    *o_pID = P7TELEMETRY_INVALID_ID;

    LOCK_ENTER(m_sCS);

    l_pMap = m_cCounters_Map.Find(i_pName);
    if (    (l_pMap)
         && (m_pCounters[l_pMap->bID])
       )
    {
        l_bReturn = TRUE; 
        *o_pID    = l_pMap->bID;
    }

    LOCK_EXIT(m_sCS);

    return l_bReturn;
}// Find


////////////////////////////////////////////////////////////////////////////////
// Add  
tBOOL CP7Telemetry::Add(tUINT8 i_bID, tINT64 i_llValue)
{
    tBOOL            l_bReturn  = TRUE;
    tUINT32          l_dwSize   = 0;
    tUINT8           l_bReset   = 0;
    CP7Tel_Counter  *l_pCounter; //will be initialized lated
    sP7C_Data_Chunk *l_pChunk;   //will be initialized lated

    if (P7TELEMETRY_INVALID_ID == i_bID)
    {
        return FALSE;
    }

    LOCK_ENTER(m_sCS);

    ////////////////////////////////////////////////////////////////////////////
    //check ID
    if (    (P7TELEMETRY_COUNTERS_MAX_COUNT <= i_bID)
         || (NULL  == m_pCounters[i_bID])
         || (FALSE == m_bInitialized)
         || (FALSE == m_sStatus.bConnected)
       )
    {
        l_bReturn = FALSE;
        goto l_lblExit;
    }

    l_pChunk   = m_pChunks;
    l_pCounter = m_pCounters[i_bID];

    //connection was lost, we need to resend initial data 
    if (m_dwResets_Channel != m_sStatus.dwResets)
    {
        m_dwResets_Channel = m_sStatus.dwResets;

        l_bReset          |= RESET_FLAG_CHANNEL;
        l_pChunk->dwSize   = sizeof(m_sHeader_Info);
        l_pChunk->pData    = &m_sHeader_Info;
        l_dwSize          += l_pChunk->dwSize;
        l_pChunk ++;
    }

    //counters descriptions have to send again
    if (m_dwResets_Counters != m_sStatus.dwResets)
    {
        CP7Tel_Counter **l_pIter = m_pCounters;

        while (*l_pIter)
        {
            l_pChunk->pData  = &((*l_pIter)->m_sCounter);
            l_pChunk->dwSize = GET_EXT_HEADER_SIZE((*l_pIter)->m_sCounter.sCommonRaw);//(*l_pIter)->m_sCounter.sCommon.dwSize;
            l_dwSize        += l_pChunk->dwSize;
            l_pChunk ++;
            l_pIter++;
        }

        m_dwResets_Counters = m_sStatus.dwResets;
        l_bReset           |= RESET_FLAG_COUNTER;
    }

    if (l_pCounter->m_sCounter.bOn)
    {
        m_sValue.bID     = i_bID;
        m_sValue.llValue = i_llValue;
        m_sValue.bSeqN   = l_pCounter->m_bSeqN ++;

        if (! m_sConf.pTimestamp_Callback )
        {
            m_sValue.qwTimer = GetPerformanceCounter();
        }
        else
        {
            m_sValue.qwTimer = m_sConf.pTimestamp_Callback(m_sConf.pContext);
        }
        
        l_pChunk->pData  = &m_sValue;
        l_pChunk->dwSize = GET_EXT_HEADER_SIZE(m_sValue.sCommonRaw);//m_sValue.sCommon.dwSize;
        l_dwSize        += l_pChunk->dwSize;

        l_pChunk ++;
    }
    else if (0 >= l_dwSize)
    {
        goto l_lblExit;
    }


    if (ECLIENT_STATUS_OK != m_pClient->Sent(m_dwChannel_ID,
                                             m_pChunks,
                                             (tUINT32)(l_pChunk - m_pChunks),
                                             l_dwSize
                                            )
       )
    {
        if (l_bReset & RESET_FLAG_CHANNEL)
        {
            m_dwResets_Channel = RESET_UNDEFINED;
        }
        if (l_bReset & RESET_FLAG_COUNTER)
        {
            m_dwResets_Counters = RESET_UNDEFINED;
        }

        l_bReturn = FALSE;
    }

l_lblExit:
    LOCK_EXIT(m_sCS);

    return l_bReturn;
}// Add  


////////////////////////////////////////////////////////////////////////////////
// Share                                      
tBOOL CP7Telemetry::Share(const tXCHAR *i_pName)
{
    tBOOL l_bReturn = FALSE;

    LOCK_ENTER(m_sCS);
    if (NULL == m_hShared)
    {
        void *l_pTrace = static_cast<IP7_Telemetry*>(this);

        tUINT32 l_dwLen1 = PStrLen(TELEMETRY_SHARED_PREFIX);
        tUINT32 l_dwLen2 = PStrLen(i_pName);
        tXCHAR *l_pName = (tXCHAR *)malloc(sizeof(tXCHAR) * (l_dwLen1 + l_dwLen2 + 16));

        if (l_pName)
        {
            PStrCpy(l_pName, l_dwLen1 + l_dwLen2 + 16, TELEMETRY_SHARED_PREFIX);
            PStrCpy(l_pName + l_dwLen1, l_dwLen2 + 16, i_pName);
            l_bReturn = CShared::Create(&m_hShared, l_pName, (tUINT8*)&l_pTrace, sizeof(l_pTrace));
            free(l_pName);
            l_pName = NULL;
        }

    }
    LOCK_EXIT(m_sCS);

    return l_bReturn;
}// Share


////////////////////////////////////////////////////////////////////////////////
// On_Flush - internal call                                      
void CP7Telemetry::Flush()
{
    if (FALSE == m_bInitialized)
    {
        return;
    }

    m_bInitialized = FALSE;

    //inform server about channel closing, I didn't check status, just
    //sending data
    if (RESET_UNDEFINED != m_dwResets_Counters)
    {
        sP7Ext_Raw l_sHeader;
        INIT_EXT_HEADER(l_sHeader, EP7USER_TYPE_TELEMETRY, EP7TEL_TYPE_CLOSE, sizeof(sP7Ext_Raw));
        sP7C_Data_Chunk l_sChunk = {&l_sHeader, sizeof(sP7Ext_Raw)};

        m_pClient->Sent(m_dwChannel_ID, &l_sChunk, 1, l_sChunk.dwSize);
    }
}// On_Flush                                      


