////////////////////////////////////////////////////////////////////////////////
//                                                                             /
// 2012-2019 (c) Baical                                                        /
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
#define RESET_FLAG_HEADER                                         (0x1)
#define RESET_FLAG_COUNTER                                        (0x2)
#define TELEMETRY_SHARED_PREFIX                                   TM("Trc_")
#define TELEMETRY_ON_CONNECT_EXIT_SIGNAL                          (MEVENT_SIGNAL_0)

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
CP7Tel_Counter::CP7Tel_Counter(tUINT16       i_wID,
                               tUINT8        i_bOn,
                               tDOUBLE       i_dbMin,
                               tDOUBLE       i_dbAlarmMin,
                               tDOUBLE       i_dbMax,
                               tDOUBLE       i_dbAlarmMax,
                               const tXCHAR *i_pName,
                               tUINT32       i_uHash
                              )
    : m_bInitialized(TRUE)
    , m_pName(PStrDub(i_pName))
    , m_bDelivered(FALSE)
    , m_pHeader(NULL)
    , m_wSeqN(0)
    , m_uHash(i_uHash)
    , pTreeNext(NULL)
{
    size_t l_szCounter = sizeof(sP7Tel_Counter_v2) - sizeof(sP7Tel_Counter_v2::pName);
    size_t l_szName    = PStrLen(i_pName) + 1;
    l_szCounter += l_szName * sizeof(tWCHAR);

    m_pHeader = (sP7Tel_Counter_v2*)malloc(l_szCounter);

    memset(m_pHeader, 0, l_szCounter);

    INIT_EXT_HEADER(m_pHeader->sCommonRaw, EP7USER_TYPE_TELEMETRY_V2, EP7TEL_TYPE_COUNTER, l_szCounter);
    //m_sCounter.sCommon.dwSize     = sizeof(sP7Tel_Counter);
    //m_sCounter.sCommon.dwType     = EP7USER_TYPE_TELEMETRY;
    //m_sCounter.sCommon.dwSubType  = EP7TEL_TYPE_COUNTER;

    m_pHeader->wID        = i_wID;
    m_pHeader->bOn        = i_bOn;
    m_pHeader->dbMin      = i_dbMin; 
    m_pHeader->dbAlarmMin = i_dbAlarmMin;
    m_pHeader->dbMax      = i_dbMax;
    m_pHeader->dbAlarmMax = i_dbAlarmMax;

    PUStrCpy(m_pHeader->pName, (tUINT32)l_szName, i_pName);
} // CP7Tel_Counter   


////////////////////////////////////////////////////////////////////////////////
// ~CP7Tel_Counter                                       
CP7Tel_Counter::~CP7Tel_Counter()
{
    if (m_pHeader)
    {
        free(m_pHeader);
        m_pHeader = NULL;
    }

    if (m_pName)
    {
        PStrFreeDub(m_pName);
        m_pName = NULL;
    }
}// ~CP7Trace_Item                                       


////////////////////////////////////////////////////////////////////////////////
// Get_Hash                                       
tUINT32 CP7Tel_Counter::Get_Hash(const tXCHAR *i_pName)
{
    //Here is RedBlackTree keys are calculated
    //Hash description: http://isthe.com/chongo/tech/comp/fnv/#FNV-param
    //Hash parameters investigation (collisions, randomnessification)
    //http://programmers.stackexchange.com/questions/49550/which-hashing-algorithm-is-best-for-uniqueness-and-speed
    //Collisions:
    // - Is collisions are possible ? Yes, FNV-1a produce 4 coll. on list of 
    //   216,553 English words. 
    // - If collision happens and even calculated key is not unique - is it
    //   dangerous ? No, in tree we will have additional list (containing 
    //   elementS with the same key) where exact item can be found
    tUINT32 l_uReturn = 2166136261ul;

    while (*i_pName)
    {
        l_uReturn = (l_uReturn ^ (tUINT32)*i_pName) * 16777619ul;
        i_pName++;
    }

    return l_uReturn;
}


////////////////////////////////////////////////////////////////////////////////
// Has_Name                                       
tBOOL CP7Tel_Counter::Has_Name(const tXCHAR *i_pName)
{
    return (0 == PStrCmp(i_pName , m_pName)) ? TRUE : FALSE;
}// Has_Name


////////////////////////////////////////////////////////////////////////////////
// Is_Initialized                                       
tBOOL CP7Tel_Counter::Is_Initialized()
{
    return m_bInitialized;
}// Is_Initialized


////////////////////////////////////////////////////////////////////////////////
// Enable 
void CP7Tel_Counter::Enable(tUINT16 i_wOn)
{
    m_pHeader->bOn = i_wOn;
}// Enable



////////////////////////////////////////////////////////////////////////////////
// CP7Telemetry                                       
CP7Telemetry::CP7Telemetry(IP7_Client *i_pClient, const tXCHAR *i_pName, const stTelemetry_Conf *i_pConf)
    : m_lReference(1)
    , m_pClient(i_pClient)
    , m_dwChannel_ID(0)
    , m_bInitialized(TRUE)
    , m_bActive(TRUE)
    , m_pCounters(256)
    , m_bIsHeaderDelivered(FALSE)
    , m_pChunks(NULL)
    , m_dwChunks_Max_Count(64)
    , m_bIs_Channel(FALSE)
    , m_hShared(NULL)
    , m_bOnConnect_Thread(FALSE)
    , m_hOnConnect_Thread(0) //NULL
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
        if (FALSE == m_cOnConnect_Event.Init(1, EMEVENT_SINGLE_MANUAL))
        {
            m_bInitialized = FALSE;
        }
    }

    if (m_bInitialized)
    {
        INIT_EXT_HEADER(m_sHeader_Info.sCommonRaw, EP7USER_TYPE_TELEMETRY_V2, EP7TEL_TYPE_INFO, sizeof(sP7Tel_Info));
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

        INIT_EXT_HEADER(m_sValue.sCommonRaw, EP7USER_TYPE_TELEMETRY_V2, EP7TEL_TYPE_VALUE, sizeof(sP7Tel_Value_v2));
        //m_sValue.sCommon.dwSize         = sizeof(sP7Tel_Value); 
        //m_sValue.sCommon.dwType         = EP7USER_TYPE_TELEMETRY;
        //m_sValue.sCommon.dwSubType      = EP7TEL_TYPE_VALUE;
    }

    if (m_bInitialized)
    {
        m_bIs_Channel  = (ECLIENT_STATUS_OK == m_pClient->Register_Channel(this));
        m_bInitialized = m_bIs_Channel;
    }

    m_bActive = m_bInitialized;

}// CP7Telemetry


////////////////////////////////////////////////////////////////////////////////
// ~CP7Telemetry                                       
CP7Telemetry::~CP7Telemetry()
{
    LOCK_ENTER(m_sCS);
    LOCK_EXIT(m_sCS);

    if (m_bOnConnect_Thread)
    {
        m_cOnConnect_Event.Set(TELEMETRY_ON_CONNECT_EXIT_SIGNAL);
        CThShell::Close(m_hOnConnect_Thread, 1000);
        m_hOnConnect_Thread = 0;
    }

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

    m_pCounters.Clear(TRUE);

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
void CP7Telemetry::On_Receive(tUINT32 i_dwChannel, tUINT8 *i_pBuffer, tUINT32 i_dwSize, tBOOL i_bBigEndian)
{
    UNUSED_ARG(i_dwChannel);
    UNUSED_ARG(i_bBigEndian);

    LOCK_ENTER(m_sCS);

    if (    (i_pBuffer)
         && (i_dwSize >= sizeof(sP7Ext_Header))
       )
    {
        sP7Ext_Raw l_sHeader = *(sP7Ext_Raw*)i_pBuffer;

        if (EP7USER_TYPE_TELEMETRY_V2 == GET_EXT_HEADER_TYPE(l_sHeader))
        {
            if (EP7TEL_TYPE_ENABLE == GET_EXT_HEADER_SUBTYPE(l_sHeader))
            {
                sP7Tel_Enable_v2 *l_pEnable = (sP7Tel_Enable_v2*)i_pBuffer;

                if (m_pCounters[l_pEnable->wID])
                {
                    m_pCounters[l_pEnable->wID]->Enable(l_pEnable->bOn);

                    if (m_sConf.pEnable_Callback)
                    {
                        m_sConf.pEnable_Callback(m_sConf.pContext, l_pEnable->wID, l_pEnable->bOn);
                    }
                }
            }
            else if (EP7TEL_TYPE_DELETE == GET_EXT_HEADER_SUBTYPE(l_sHeader))
            {
                Flush();
                m_bActive = FALSE;
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
            if (m_bOnConnect_Thread)
            {
                m_cOnConnect_Event.Set(TELEMETRY_ON_CONNECT_EXIT_SIGNAL);
                CThShell::Close(m_hOnConnect_Thread, 1000);
                m_hOnConnect_Thread = 0;
                m_bOnConnect_Thread = FALSE;
            }

            m_bIsHeaderDelivered = FALSE;

            pAList_Cell l_pEl = NULL;
            while ((l_pEl = m_pCounters.Get_Next(l_pEl)))
            {
                CP7Tel_Counter *l_pCounter = m_pCounters.Get_Data(l_pEl);
                if (l_pCounter)
                {
                    l_pCounter->m_bDelivered = FALSE;
                }
            }

            m_cOnConnect_Event.Clr(TELEMETRY_ON_CONNECT_EXIT_SIGNAL);
            if (CThShell::Create(&Static_OnConnect_Routine,
                                 this,
                                 &m_hOnConnect_Thread,
                                 TM("P7:Tel:OnConnect") 
                                )
                )
            {
                m_bOnConnect_Thread = TRUE;
            }
        }
        else
        {
            m_bIsHeaderDelivered = FALSE;
            if (m_bOnConnect_Thread)
            {
                m_cOnConnect_Event.Set(TELEMETRY_ON_CONNECT_EXIT_SIGNAL);
                CThShell::Close(m_hOnConnect_Thread, 1000);
                m_hOnConnect_Thread = 0;
                m_bOnConnect_Thread = FALSE;
            }

            m_cOnConnect_Event.Clr(TELEMETRY_ON_CONNECT_EXIT_SIGNAL);

            pAList_Cell l_pEl = NULL;
            while ((l_pEl = m_pCounters.Get_Next(l_pEl)))
            {
                CP7Tel_Counter *l_pCounter = m_pCounters.Get_Data(l_pEl);
                if (l_pCounter)
                {
                    l_pCounter->m_bDelivered = FALSE;
                }
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
         && (m_bActive)
       )
    {
        //nothing special for crash event
    }

    Flush();

    LOCK_EXIT(m_sCS);
}// On_Flush                                      


////////////////////////////////////////////////////////////////////////////////
// Create, i_pName is case sensitive and should be unique 
tBOOL CP7Telemetry::Create(const tXCHAR *i_pName, 
                           tDOUBLE       i_dbMin,
                           tDOUBLE       i_dbAlarmMin,
                           tDOUBLE       i_dbMax,
                           tDOUBLE       i_dbAlarmMax,
                           tBOOL         i_bOn,
                           tUINT16      *o_pID 
                          )
{
    tBOOL           l_bReturn  = FALSE;
    sCounterMap    *l_pMap     = NULL;
    tUINT32         l_uiId     = 0;
    CP7Tel_Counter *l_pCounter = NULL;
    tUINT32         l_uHash    = 0;

    if (    (FALSE == m_bActive)
         || (NULL == i_pName)
         || (NULL == o_pID)
       )
    {
        return FALSE;
    }

    LOCK_ENTER(m_sCS);

    *o_pID = P7TELEMETRY_INVALID_ID_V2;

    l_uHash = CP7Tel_Counter::Get_Hash(i_pName);

    ////////////////////////////////////////////////////////////////////////////
    //find by name already existing counter
    l_pMap = m_cCounters_Map.Find(l_uHash);
    if (l_pMap)
    {
        l_pCounter = l_pMap->pCounter;
        while (l_pCounter)
        {
            if (l_pCounter->Has_Name(i_pName))
            {
                //found, and all parameters are the same ... 
                if (    (l_pCounter->m_pHeader->dbMin      == i_dbMin)
                     && (l_pCounter->m_pHeader->dbMax      == i_dbMax)
                     && (l_pCounter->m_pHeader->dbAlarmMin == i_dbAlarmMin)
                     && (l_pCounter->m_pHeader->dbAlarmMax == i_dbAlarmMax)
                   )
                {
                    *o_pID    = l_pCounter->m_pHeader->wID;
                    l_bReturn = TRUE;
                    goto l_lblExit;
                }
                else //name is equal, but some additional parameters are different
                {
                    goto l_lblExit;
                }
            }
            l_pCounter = l_pCounter->pTreeNext;
        }
    }

    l_pCounter = NULL;


    if (P7TELEMETRY_INVALID_ID_V2 <= m_pCounters.Count())
    {
        goto l_lblExit;
    }

    l_uiId     = m_pCounters.Count();
    l_pCounter = new CP7Tel_Counter((tUINT16)l_uiId,
                                    i_bOn,
                                    i_dbMin,
                                    i_dbAlarmMin,
                                    i_dbMax,
                                    i_dbAlarmMax,
                                    i_pName,
                                    l_uHash
                                   );

    if (    (NULL == l_pCounter)
         || (FALSE == l_pCounter->Is_Initialized())
       )
    {
        if (l_pCounter)
        {
            delete l_pCounter;
            l_pCounter = NULL;
        }

        goto l_lblExit;
    }

    if (l_pMap)
    {
        l_pCounter->pTreeNext = l_pMap->pCounter;
        l_pMap->pCounter = l_pCounter;
    }
    else
    {
        m_cCounters_Map.Push(new sCounterMap(l_pCounter), l_uHash);
    }

    if (m_sStatus.bConnected)
    {
        sP7C_Data_Chunk *l_pChunk  = m_pChunks;
        tBOOL            l_bHeader = FALSE;
        tUINT32          l_dwSize  = 0;

        //connection was lost, we need to resend initial data 
        if (!m_bIsHeaderDelivered)
        {
            l_bHeader        = TRUE;
            l_pChunk->dwSize = sizeof(m_sHeader_Info);
            l_pChunk->pData  = &m_sHeader_Info;
            l_dwSize        += l_pChunk->dwSize;
            l_pChunk ++;
        }

        l_pChunk->pData  = l_pCounter->m_pHeader;
        l_pChunk->dwSize = GET_EXT_HEADER_SIZE(l_pCounter->m_pHeader->sCommonRaw);//m_pCounters[m_dwUsed]->m_sCounter.sCommon.dwSize;
        l_dwSize        += l_pChunk->dwSize;
        l_pChunk ++;

        if (ECLIENT_STATUS_OK == m_pClient->Sent(m_dwChannel_ID, m_pChunks, (tUINT32)(l_pChunk - m_pChunks), l_dwSize))
        {
            l_pCounter->m_bDelivered = TRUE;
            if (l_bHeader)
            {
                m_bIsHeaderDelivered = TRUE;
            }
        }
    }

    l_bReturn = TRUE;
    *o_pID    = (tUINT16)l_uiId;
    m_pCounters.Add_After(m_pCounters.Get_Last(), l_pCounter);

l_lblExit:
    LOCK_EXIT(m_sCS);

    return l_bReturn;
} //Create


////////////////////////////////////////////////////////////////////////////////
// Find, i_pName is case sensitive
tBOOL CP7Telemetry::Find(const tXCHAR *i_pName, tUINT16 *o_pID)
{
    sCounterMap *l_pMap    = NULL;
    tBOOL        l_bReturn = FALSE;

    if (o_pID)
    {
        *o_pID = P7TELEMETRY_INVALID_ID_V2;
    }
    else
    {
        return FALSE;
    }

    if (    (FALSE == m_bActive)
         || (NULL == i_pName)
       )
    {
        return FALSE;
    }

    LOCK_ENTER(m_sCS);
    l_pMap = m_cCounters_Map.Find(CP7Tel_Counter::Get_Hash(i_pName));
    if (l_pMap)
    {
        CP7Tel_Counter *l_pCounter = l_pMap->pCounter;
        while (l_pCounter)
        {
            if (l_pCounter->Has_Name(i_pName))
            {
                *o_pID = l_pCounter->m_pHeader->wID;
                l_bReturn = TRUE;
                break;
            }
            l_pCounter = l_pCounter->pTreeNext;
        }
    }
    LOCK_EXIT(m_sCS);

    return l_bReturn;
}// Find

////////////////////////////////////////////////////////////////////////////////
// Add uint64 version
tBOOL CP7Telemetry::AddU64(tUINT16 i_bID, tUINT64 i_dbValue)
{
    return Add(i_bID, (tDOUBLE)i_dbValue);
}

////////////////////////////////////////////////////////////////////////////////
// Add  
tBOOL CP7Telemetry::Add(tUINT16 i_bID, tDOUBLE i_dbValue)
{
    tBOOL            l_bReturn  = TRUE;
    tUINT32          l_dwSize   = 0;
    tUINT8           l_bReset   = 0;
    CP7Tel_Counter  *l_pCounter; //will be initialized lated
    sP7C_Data_Chunk *l_pChunk;   //will be initialized lated

    if (P7TELEMETRY_INVALID_ID_V2 == i_bID)
    {
        return FALSE;
    }

    LOCK_ENTER(m_sCS);

    l_pCounter = m_pCounters[i_bID];

    ////////////////////////////////////////////////////////////////////////////
    //check ID
    if (    (NULL  == l_pCounter)
         || (FALSE == m_bActive)
         || (FALSE == m_sStatus.bConnected)
         || (!l_pCounter->m_pHeader->bOn)
       )
    {
        l_bReturn = FALSE;
        goto l_lblExit;
    }

    l_pChunk = m_pChunks;

    //connection was lost, we need to resend initial data 
    if (!m_bIsHeaderDelivered)
    {
        l_bReset          |= RESET_FLAG_HEADER;
        l_pChunk->dwSize   = sizeof(m_sHeader_Info);
        l_pChunk->pData    = &m_sHeader_Info;
        l_dwSize          += l_pChunk->dwSize;
        l_pChunk ++;
    }

    //counters descriptions have to send again
    if (!l_pCounter->m_bDelivered)
    {
        l_bReset        |= RESET_FLAG_COUNTER;
        l_pChunk->pData  = l_pCounter->m_pHeader;
        l_pChunk->dwSize = GET_EXT_HEADER_SIZE(l_pCounter->m_pHeader->sCommonRaw);//(*l_pIter)->m_sCounter.sCommon.dwSize;
        l_dwSize        += l_pChunk->dwSize;
        l_pChunk ++;
    }

    m_sValue.wID     = i_bID;
    m_sValue.dbValue = i_dbValue;
    m_sValue.wSeqN   = l_pCounter->m_wSeqN ++;

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

    if (ECLIENT_STATUS_OK == m_pClient->Sent(m_dwChannel_ID,
                                             m_pChunks,
                                             (tUINT32)(l_pChunk - m_pChunks),
                                             l_dwSize
                                            )
       )
    {
        if (l_bReset & RESET_FLAG_HEADER)
        {
            m_bIsHeaderDelivered = TRUE;
        }
        if (l_bReset & RESET_FLAG_COUNTER)
        {
            l_pCounter->m_bDelivered = TRUE;
        }
    }
    else
    {
        l_bReturn = FALSE;
    }

l_lblExit:
    LOCK_EXIT(m_sCS);

    return l_bReturn;
}// Add  


////////////////////////////////////////////////////////////////////////////////
// Set_Enable                                      
tBOOL CP7Telemetry::Set_Enable(tUINT16 i_wID, tBOOL i_bEnable)
{
    tBOOL l_bReturn = FALSE;

    if (!m_bInitialized)
    {
        return l_bReturn;
    }

    LOCK_ENTER(m_sCS);
    if (m_bActive)
    {
        CP7Tel_Counter *l_pCounter = m_pCounters[i_wID];
        if (l_pCounter)
        {
            l_bReturn = TRUE;
            l_pCounter->m_pHeader->bOn = (tUINT16)i_bEnable;
        }
    }
    LOCK_EXIT(m_sCS);
    return l_bReturn;
}


////////////////////////////////////////////////////////////////////////////////
// Get_Enable                                      
tBOOL CP7Telemetry::Get_Enable(tUINT16 i_wID)
{
    tBOOL l_bReturn = FALSE;
    if (!m_bInitialized)
    {
        return l_bReturn;
    }

    LOCK_ENTER(m_sCS);
    CP7Tel_Counter *l_pCounter = m_pCounters[i_wID];
    if (l_pCounter)
    {
        l_bReturn = (tBOOL)l_pCounter->m_pHeader->bOn;
    }
    LOCK_EXIT(m_sCS);
    return l_bReturn;
}


////////////////////////////////////////////////////////////////////////////////
// Get_Min                                      
tDOUBLE CP7Telemetry::Get_Min(tUINT16 i_wID)
{
    tDOUBLE l_dbReturn = 0.0;
    if (!m_bInitialized)
    {
        return l_dbReturn;
    }

    LOCK_ENTER(m_sCS);
    CP7Tel_Counter *l_pCounter = m_pCounters[i_wID];
    if (l_pCounter)
    {
        l_dbReturn = l_pCounter->m_pHeader->dbMin;
    }
    LOCK_EXIT(m_sCS);
    return l_dbReturn;
}


////////////////////////////////////////////////////////////////////////////////
// Get_Max                                      
tDOUBLE CP7Telemetry::Get_Max(tUINT16 i_wID)
{
    tDOUBLE l_dbReturn = 0.0;
    if (!m_bInitialized)
    {
        return l_dbReturn;
    }

    LOCK_ENTER(m_sCS);
    CP7Tel_Counter *l_pCounter = m_pCounters[i_wID];
    if (l_pCounter)
    {
        l_dbReturn = l_pCounter->m_pHeader->dbMax;
    }
    LOCK_EXIT(m_sCS);
    return l_dbReturn;
}


////////////////////////////////////////////////////////////////////////////////
// Get_Name                                      
const tXCHAR *CP7Telemetry::Get_Name(tUINT16 i_wID)
{
    const tXCHAR *l_pReturn = NULL;
    if (!m_bInitialized)
    {
        return l_pReturn;
    }

    LOCK_ENTER(m_sCS);
    CP7Tel_Counter *l_pCounter = m_pCounters[i_wID];
    if (l_pCounter)
    {
        l_pReturn = l_pCounter->m_pName;
    }
    LOCK_EXIT(m_sCS);
    return l_pReturn;
}


////////////////////////////////////////////////////////////////////////////////
// Get_Count                                      
tUINT16 CP7Telemetry::Get_Count()
{
    tUINT16 l_wReturn = 0;
    if (!m_bInitialized)
    {
        return l_wReturn;
    }

    LOCK_ENTER(m_sCS);
    l_wReturn = m_pCounters.Count();
    LOCK_EXIT(m_sCS);
    return l_wReturn;
}


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
    if (FALSE == m_bActive)
    {
        return;
    }

    m_bActive = FALSE;

    //inform server about channel closing, I didn't check status, just
    //sending data
    if (m_bIsHeaderDelivered)
    {
        sP7Ext_Raw l_sHeader;
        INIT_EXT_HEADER(l_sHeader, EP7USER_TYPE_TELEMETRY_V2, EP7TEL_TYPE_CLOSE, sizeof(sP7Ext_Raw));
        sP7C_Data_Chunk l_sChunk = {&l_sHeader, sizeof(sP7Ext_Raw)};
        m_pClient->Sent(m_dwChannel_ID, &l_sChunk, 1, l_sChunk.dwSize);
    }
}// On_Flush                                      


////////////////////////////////////////////////////////////////////////////////
// OnConnect_Routine                                      
void CP7Telemetry::OnConnect_Routine()
{
    tUINT32          l_uTimeOut   = 0;
    pAList_Cell      l_pCounterEl = NULL;
    tBOOL            l_bExit      = FALSE;
    CP7Tel_Counter * l_pCounter   = NULL;
    sP7C_Data_Chunk  l_stChunk;

    while (FALSE == l_bExit)
    {
        if (TELEMETRY_ON_CONNECT_EXIT_SIGNAL == m_cOnConnect_Event.Wait(l_uTimeOut))
        {
            break;
        }

        if (LOCK_TRY(m_sCS))
        {
            if (!m_bIsHeaderDelivered)
            {
                sP7C_Data_Chunk  l_stChunk;
                l_stChunk.dwSize = sizeof(m_sHeader_Info);
                l_stChunk.pData  = &m_sHeader_Info;

                if (ECLIENT_STATUS_OK == m_pClient->Sent(m_dwChannel_ID, &l_stChunk, 1, l_stChunk.dwSize))
                {
                    m_bIsHeaderDelivered = TRUE;
                }
            }

            if (m_bIsHeaderDelivered)
            {
                tBOOL l_bDelivering = TRUE;

                if (!l_pCounterEl)
                {
                    l_pCounterEl = m_pCounters.Get_First();
                }

                while (    (l_pCounterEl)
                        && (l_bDelivering) 
                      )
                {
                    l_pCounter = m_pCounters.Get_Data(l_pCounterEl);
                    if (l_pCounter)
                    {
                        if (!l_pCounter->m_bDelivered)
                        {
                            l_stChunk.pData  = l_pCounter->m_pHeader;
                            l_stChunk.dwSize = GET_EXT_HEADER_SIZE(l_pCounter->m_pHeader->sCommonRaw);

                            if (ECLIENT_STATUS_OK == m_pClient->Sent(m_dwChannel_ID, &l_stChunk, 1, l_stChunk.dwSize))
                            {
                                l_pCounter->m_bDelivered = TRUE;
                                l_pCounterEl = m_pCounters.Get_Next(l_pCounterEl);
                                l_uTimeOut   = 0;
                            }
                            else
                            {
                                l_uTimeOut    = 5;
                                l_bDelivering = FALSE;
                            }
                        }
                        else
                        {
                            l_pCounterEl = m_pCounters.Get_Next(l_pCounterEl);
                        }
                    }
                }
                
                if (!l_pCounterEl)
                {
                    l_bExit = TRUE;
                }
            }
            LOCK_EXIT(m_sCS);
        }
    }

    m_cOnConnect_Event.Clr(TELEMETRY_ON_CONNECT_EXIT_SIGNAL);
}


