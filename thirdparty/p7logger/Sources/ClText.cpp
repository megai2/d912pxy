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
// This header file provide functionality to format data into text format      /
////////////////////////////////////////////////////////////////////////////////

#include "CommonClient.h"
#include "ClText.h"

//allow to see dump of memory leaks at the end of test in VS debug output
//#define _CRTDBG_MAP_ALLOC
//#include <crtdbg.h>
//#ifdef _DEBUG
//#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
//#define new DEBUG_NEW
//#endif

#define THREAD_IDLE_TIMEOUT                                                (100)
#define FREE_DATA_WAIT_TIMEOUT                                             (100)
                                 
#define THREAD_EXIT_SIGNAL                                     (MEVENT_SIGNAL_0)
#define THREAD_DATA_SIGNAL                                 (MEVENT_SIGNAL_0 + 1)

#define DATA_FREE_SIGNAL                                       (MEVENT_SIGNAL_0)

#define MIN_BUFFER_SIZE                                                  (16384)
#define MAX_BUFFER_SIZE                                                 (131072)
#define MIN_BUFFERS_COUNT                                                    (3)
                                 
#define DEF_POOL_SIZE                                     (MAX_BUFFER_SIZE * 16)
#define MIN_POOL_SIZE                      (MIN_BUFFER_SIZE * MIN_BUFFERS_COUNT)
#define TRACE_TEXT_LENGTH                                               (0x1000)
                                 
#define TAIL_SIZE(iBuffer)          (m_dwBuffer_Size - (tUINT32)iBuffer->szUsed)

#define TIME_HRS_100NS                                            36000000000ull
#define TIME_MIN_100NS                                              600000000ull
#define TIME_SEC_100NS                                               10000000ull
#define TIME_MSC_100NS                                                  10000ull

#ifndef min
    #define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#if !defined(MAXUINT64)
    #define MAXUINT64  ((tUINT64)~((tUINT64)0))
#endif

#define REALLOC_MESSAGE(i_szValue, i_pClient)\
if ((i_szValue + 256) >= (i_pClient->m_szMsg - (i_pClient->m_pMsgCur - i_pClient->m_pMsg)))\
{\
    size_t l_szNew = i_pClient->m_szMsg + i_szValue + 256;\
    tXCHAR *l_pNew = (tXCHAR *)realloc(i_pClient->m_pMsg, l_szNew * sizeof(tXCHAR));\
    if (l_pNew)\
    {\
        i_pClient->m_pMsgCur = l_pNew + (i_pClient->m_pMsgCur - i_pClient->m_pMsg);\
        i_pClient->m_pMsg  = l_pNew;\
        i_pClient->m_szMsg = l_szNew;\
    }\
}\


struct sField
{
    const tXCHAR      *pField;
    CClText::fnFormat  pFormat;
};

static const sField g_pFields[] =
{
    { CLIENT_FMT_CHANNEL,     CClText::FormatChannel      },
    { CLIENT_FMT_MSG_ID,      CClText::FormatMessageId    },
    { CLIENT_FMT_MSG_INDEX,   CClText::FormatMessageIndex },
    { CLIENT_FMT_TIME_FULL,   CClText::FormatTimeFull     },
    { CLIENT_FMT_TIME_MEDIUM, CClText::FormatTimeMedium   },
    { CLIENT_FMT_TIME_SHORT,  CClText::FormatTimeShort    },
    { CLIENT_FMT_TIME_DIFF,   CClText::FormatTimeDiff     },
    { CLIENT_FMT_TIME_COUNT,  CClText::FormatTimeCount    },
    { CLIENT_FMT_LEVEL,       CClText::FormatLevel        },
    { CLIENT_FMT_THREAD_ID,   CClText::FormatThreadId     },
    { CLIENT_FMT_THREAD_NAME, CClText::FormatThreadName   },
    { CLIENT_FMT_CPU_CORE,    CClText::FormatCpuCore      },
    { CLIENT_FMT_MODULE_ID,   CClText::FormatModuleId     },
    { CLIENT_FMT_MODULE_NAME, CClText::FormatModuleName   },
    { CLIENT_FMT_FILE_FULL,   CClText::FormatFilePath     },
    { CLIENT_FMT_FILE_SHORT,  CClText::FormatFileName     },
    { CLIENT_FMT_FILE_LINE,   CClText::FormatFileLine     },
    { CLIENT_FMT_FUNCTION,    CClText::FormatFunction     },
    { CLIENT_FMT_MSG,         CClText::FormatMsg          }
};
 

#define LEVEL_TEXT_LENGTH                                                    (5)
static const tXCHAR *g_pLevel[EP7TRACE_LEVEL_COUNT] = 
{
    TM("TRACE"),
    TM("DEBUG"),
    TM("INFO "),
    TM("WARN."),
    TM("ERROR"),
    TM("PANIC")
};


////////////////////////////////////////////////////////////////////////////////
CClText::fnFormat GetFormatFunction(const tXCHAR *i_pField)
{
    if (!*i_pField) //at least 2 chars
    {
        return NULL;
    }

    for (size_t l_szI = 0; l_szI < LENGTH(g_pFields); l_szI++)
    {
    #ifdef UTF8_ENCODING
        if ((*(tUINT16*)i_pField) == (*(tUINT16*)g_pFields[l_szI].pField))
    #else
        if ((*(tUINT32*)i_pField) == (*(tUINT32*)g_pFields[l_szI].pField))
    #endif                             
        {
            return g_pFields[l_szI].pFormat;
        }
    }
    return NULL;
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//                                sTraceDescEx
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
sTraceDescEx::sTraceDescEx(const sP7Trace_Format *i_pFormat,
                           CFormatter::sBuffer   *i_pBuffer,
                           tBOOL                  i_bBigEndian
                          )
    : sTraceDesc()
    , pBuffer(NULL)
    , dwSize(0)
    , pFormat(NULL)
    , pArgs(NULL)
    , dwArgsLen(0)
{
    size_t l_szLen     = 0;

    if (i_pFormat)
    {
        dwSize  = i_pFormat->sCommon.dwSize;
        pBuffer = (tUINT8*)malloc(dwSize);
    }

    if (pBuffer)
    {
        memcpy(pBuffer, i_pFormat, dwSize);
        wLine       = i_pFormat->wLine;
        dwArgsLen   = i_pFormat->wArgs_Len;
        dwModuleID  = i_pFormat->wModuleID;
        pArgs       = (sP7Trace_Arg*)(pBuffer + sizeof(sP7Trace_Format));

    #ifdef UTF8_ENCODING
        tWCHAR *l_pFormat     = (tWCHAR *)(pArgs + dwArgsLen);
        size_t  l_szUtf8Chars = 0;
        size_t  l_szUtf8Bytes = 0;
        l_szLen = Get_UTF8_Stat(l_pFormat, l_szUtf8Bytes, l_szUtf8Chars);
        l_szUtf8Bytes += 8;
        
        pFormat = (tXCHAR*)malloc(l_szUtf8Bytes * sizeof(tXCHAR));
        Convert_UTF16_To_UTF8(l_pFormat, (tACHAR*)pFormat, l_szUtf8Bytes);

        pFilePath  = (char*)(l_pFormat + l_szLen + 1);
        szFilePath = strlen(pFilePath);
        pFunction  = (char*)(pFilePath + szFilePath + 1);
        szFunction = strlen(pFunction);
    #else
        char  *l_pFilePath;
        char  *l_pFunction;
        pFormat     = (tXCHAR *)(pArgs + dwArgsLen);
        l_szLen     = PStrLen(pFormat);
        l_pFilePath = (char*)(pFormat + l_szLen + 1);
        szFilePath  = strlen(l_pFilePath);
        l_pFunction = (char*)(l_pFilePath + szFilePath + 1);
        szFunction  = strlen(l_pFunction);
        pFilePath   = (tXCHAR*)malloc((szFilePath + 16)*sizeof(tXCHAR));
        pFunction   = (tXCHAR*)malloc((szFunction + 16)*sizeof(tXCHAR));
        Convert_UTF8_To_UTF16(l_pFilePath, pFilePath, (tUINT32)szFilePath + 1);
        Convert_UTF8_To_UTF16(l_pFunction, pFunction, (tUINT32)szFunction + 1);
    #endif                             

        if (    (pFileName = PStrrChr(pFilePath, TM('\\')))
             || (pFileName = PStrrChr(pFilePath, TM('/')))
           )
        {
            pFileName ++;
        }
        else
        {
            pFileName = pFilePath;
        };

        szFileName = PStrLen(pFileName);

        pFormatter = new CFormatter(pFormat, pArgs, (size_t)dwArgsLen, i_pBuffer);

        if (i_bBigEndian)
        {
            pFormatter->EnableBigEndian();
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
sTraceDescEx::~sTraceDescEx()
{
    if (pBuffer)
    {
        free(pBuffer);
        pBuffer = NULL;
    }

    if (pFormatter)
    {
        delete pFormatter;
        pFormatter = NULL;
    }

#ifdef UTF8_ENCODING
    if (pFormat)
    {
        free(pFormat);
        pFormat = NULL;
    }
#else
    if (pFilePath)
    {
        free(pFilePath);
    }
    if (pFunction)
    {
        free(pFunction);
    }
    pFilePath = NULL;
    pFunction = NULL;
    pFileName = NULL;
#endif                             
}



////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//                                CTxtChannel
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//CTxtChannel()
CTxtChannel::CTxtChannel(const sP7Trace_Info *i_pInfo, tBOOL i_bBig_Endian)
    : m_qwStreamTime(0ull)
    , m_qwTimer_Value(i_pInfo->qwTimer_Value)
    , m_qwTimer_Frequency(i_pInfo->qwTimer_Frequency)
    , m_qwFlags(i_pInfo->qwFlags)
    , m_pText(NULL)
    , m_szText(TRACE_TEXT_LENGTH)
    , m_qwTimeLast(i_pInfo->qwTimer_Value)
    , m_szName(0)
    , m_pBuffer(NULL)
    , m_bBig_Endian(i_bBig_Endian)
{
#ifdef UTF8_ENCODING
    Convert_UTF16_To_UTF8(i_pInfo->pName, (tACHAR*)m_pName, P7TRACE_NAME_LENGTH);
#else
    PStrCpy(m_pName, P7TRACE_NAME_LENGTH, (const wchar_t*)i_pInfo->pName);
#endif            

    m_pBuffer = new CFormatter::sBuffer(4096);

    m_szName = PStrLen(m_pName);

    m_qwStreamTime = (tUINT64)i_pInfo->dwTime_Lo + (((tUINT64)i_pInfo->dwTime_Hi) << 32);

    //fill list by empty elements, prepare data ...
    for (tUINT32 l_dwIDX = 0; l_dwIDX < 1536; l_dwIDX ++)
    {
        m_cDesc.Add_After(NULL, NULL);
    }
    //trick to build index, this is new list and index is not build
    //when we try to access to element by index - we create internal index
    //table
    m_cDesc[0]; 

    //fill list by empty elements, prepare data ...
    for (tUINT32 l_dwIDX = 0; l_dwIDX < 128; l_dwIDX ++)
    {
        m_cModules.Add_After(NULL, NULL);
    }

    //trick to build index, this is new list and index is not build
    //when we try to access to element by index - we create internal index
    //table
    m_cModules[0];

    m_pText = (tXCHAR*)malloc(m_szText * sizeof(tXCHAR));
}//CTxtChannel()

////////////////////////////////////////////////////////////////////////////////
//~CTxtChannel()
CTxtChannel::~CTxtChannel()
{
    m_cDesc.Clear(TRUE);
    m_cModules.Clear(TRUE);
    m_cThreads.Clear();

    if (m_pText)
    {
        free(m_pText);
        m_pText = NULL;
    }

    if (m_pBuffer)
    {
        m_pBuffer->Release();
        m_pBuffer = NULL;
    }
}//~CTxtChannel()


////////////////////////////////////////////////////////////////////////////////
//PutFormat
void CTxtChannel::PutFormat(const sP7Trace_Format *i_pDesc)
{
    //fill by empty elements if ID is larger than elements count
    while (m_cDesc.Count() <= i_pDesc->wID)
    {
        m_cDesc.Add_After(m_cDesc.Get_Last(), NULL);
    }
                
    if (NULL == m_cDesc[i_pDesc->wID])
    {
        m_cDesc.Put_Data(m_cDesc.Get_ByIndex(i_pDesc->wID), 
                         new sTraceDescEx(i_pDesc, m_pBuffer, m_bBig_Endian), 
                         TRUE
                        );
    }
}//PutFormat


////////////////////////////////////////////////////////////////////////////////
//PutModule
void CTxtChannel::PutModule(const sP7Trace_Module *i_pMod)
{
    //fill by empty elements if ID is larger than elements count
    while (m_cModules.Count() <= i_pMod->wModuleID)
    {
        m_cModules.Add_After(m_cModules.Get_Last(), NULL);
    }

    if (NULL == m_cModules[i_pMod->wModuleID])
    {
        sP7TraceMod *l_pMod = new sP7TraceMod();
        l_pMod->eVerbosity = i_pMod->eVerbosity;
    #ifdef UTF8_ENCODING
        memcpy(l_pMod->pName, i_pMod->pName, P7TRACE_MODULE_NAME_LENGTH);
    #else
        Convert_UTF8_To_UTF16(i_pMod->pName, 
                              (tWCHAR*)l_pMod->pName, 
                              P7TRACE_MODULE_NAME_LENGTH
                             );
    #endif            
        l_pMod->szName = PStrLen(l_pMod->pName);

        m_cModules.Put_Data(m_cModules.Get_ByIndex(i_pMod->wModuleID), l_pMod, TRUE);
    }
}//PutModule


////////////////////////////////////////////////////////////////////////////////
//RegisterThread
void CTxtChannel::RegisterThread(const sP7Trace_Thread_Start *i_pStart)
{
    sThread *l_pThread    = new sThread();
    l_pThread->dwThreadID = i_pStart->dwThreadID;
    l_pThread->qwStart    = i_pStart->qwTimer;
    l_pThread->qwStop     = MAXUINT64;

#ifdef UTF8_ENCODING
    memcpy(l_pThread->pName, i_pStart->pName, P7TRACE_THREAD_NAME_LENGTH);
#else
    Convert_UTF8_To_UTF16(i_pStart->pName, 
                          (tWCHAR*)l_pThread->pName, 
                          P7TRACE_THREAD_NAME_LENGTH
                         );
#endif            

    l_pThread->szName = PStrLen(l_pThread->pName);

    sRbThread *l_pRbCurr = m_cThreads.Find(l_pThread->dwThreadID);

    if (l_pRbCurr)
    {
        sRbThread *l_pRbPrev = NULL;
        while (l_pRbCurr)
        {
            if (l_pThread->qwStart > l_pRbCurr->pThread->qwStart)
            {
                sRbThread *l_pRbNew = NULL;
                if (l_pRbPrev) 
                {
                    l_pRbNew = new sRbThread(l_pThread, l_pRbCurr);
                    l_pRbPrev->pNext = l_pRbNew;
                }
                else //if it is first element in queue
                {
                    l_pRbNew  = new sRbThread(NULL, NULL);
                    //Move current (first) to new one
                    *l_pRbNew = *l_pRbCurr;
                    //Set current (first) with new data 
                    l_pRbCurr->pThread = l_pThread;
                    l_pRbCurr->pNext   = l_pRbNew;
                    //initialize pointer to new one
                    l_pRbNew = l_pRbCurr;
                }

                if (    (l_pRbNew->pNext)
                     && (MAXUINT64 == l_pRbNew->pNext->pThread->qwStop)
                   )
                {
                    l_pRbNew->pNext->pThread->qwStop = l_pRbNew->pThread->qwStart - 1ull;
                }

                break;
            }
            else if (l_pThread->qwStart == l_pRbCurr->pThread->qwStart)
            {
                //repeat !!!
                //l_pRbCurr->pThread->qwStop = i_pThread->qwStop;
                delete l_pThread;
                break;
            }
            else
            {
                l_pRbPrev = l_pRbCurr;
                l_pRbCurr = l_pRbCurr->pNext;
            }
        }
    }
    else //not found
    {
        m_cThreads.Push(new sRbThread(l_pThread, NULL), l_pThread->dwThreadID);
    }
}//RegisterThread


////////////////////////////////////////////////////////////////////////////////
//UnregisterThread
void CTxtChannel::UnregisterThread(const sP7Trace_Thread_Stop *i_pStop)
{
    sRbThread *l_pRbCurr = m_cThreads.Find(i_pStop->dwThreadID);

    if (    (l_pRbCurr)
         && (l_pRbCurr->pThread)
       )
    {
        l_pRbCurr->pThread->qwStop = i_pStop->qwTimer;
    }
}//UnregisterThread


////////////////////////////////////////////////////////////////////////////////
//Format()
tBOOL CTxtChannel::Format(sP7Trace_Data *i_pData, CClTextSink::sLog &o_rLog)
{
    tBOOL         l_bReturn   = TRUE;
    sTraceDescEx *l_pDesc     = NULL;
    tUINT8       *l_pValues   = ((tUINT8*)i_pData) + sizeof(sP7Trace_Data);
    sRbThread    *l_pRbThread = NULL;
    int           l_iCount    = 0;
    tUINT8       *l_pExt      = (tUINT8*)i_pData + i_pData->sCommon.dwSize - 1;
    tUINT8        l_bCount    = l_pExt[0];

    l_pDesc = m_cDesc[i_pData->wID];
    
    if (    (NULL == l_pDesc)
         || (!m_pText)
       )
    {                                  
        l_bReturn = FALSE;
        goto l_lblExit;
    }

    o_rLog.dwModuleID = 0u;

    while (l_bCount--) //Parse extensions, there is no need to check the presence, it is always here because of using locally
    {
        --l_pExt;
        if (EP7TRACE_EXT_MODULE_ID == l_pExt[0])
        {
            l_pExt -= 2;
            o_rLog.dwModuleID = *(tUINT16*)l_pExt;
            --l_pExt;
        }
    }

    while (!l_iCount)
    {
        l_iCount = l_pDesc->pFormatter->Format(m_pText, m_szText, l_pValues);

        if (    (l_iCount < 0)
             || ((size_t)(l_iCount + 1) >= m_szText)
           )
        {
            free(m_pText);
            m_szText *= 2;
            m_pText   = (tXCHAR*)malloc(m_szText * sizeof(tXCHAR));
            l_iCount  = 0;
            if (!m_pText)
            {
                break;
            }
        }
    }
    
    o_rLog.pChannel  = m_pName;
    o_rLog.szChannel = m_szName;
    o_rLog.szMessage = l_iCount;

    //Get thread name if it is ...
    o_rLog.pThreadName  = TM("Unkn.");
    o_rLog.szThreadName = 5;

    l_pRbThread = m_cThreads.Find(i_pData->dwThreadID);

    if (l_pRbThread)
    {
        while (l_pRbThread)
        {
            if (    (i_pData->qwTimer >= l_pRbThread->pThread->qwStart)
                 && (i_pData->qwTimer <= l_pRbThread->pThread->qwStop)
               )
            {
                o_rLog.pThreadName  = l_pRbThread->pThread->pName;
                o_rLog.szThreadName = l_pRbThread->pThread->szName;
                break;
            }

            l_pRbThread = l_pRbThread->pNext;
        }
    }

    if (m_cModules[o_rLog.dwModuleID])
    {
        o_rLog.pModuleName  = m_cModules[o_rLog.dwModuleID]->pName;
        o_rLog.szModuleName = m_cModules[o_rLog.dwModuleID]->szName;
    }
    else
    {
        o_rLog.pModuleName  = TM("Unkn.");
        o_rLog.szModuleName = 5;
    }

    UnpackLocalTime(m_qwStreamTime + ((i_pData->qwTimer - m_qwTimer_Value) * TIME_SEC_100NS) / m_qwTimer_Frequency,
                    o_rLog.dwYear,
                    o_rLog.dwMonth,
                    o_rLog.dwDay,
                    o_rLog.dwHour,
                    o_rLog.dwMinutes,
                    o_rLog.dwSeconds,
                    o_rLog.dwMilliseconds,
                    o_rLog.dwMicroseconds,
                    o_rLog.dwNanoseconds
                   );


    o_rLog.qwRawTime       = m_qwStreamTime + ((i_pData->qwTimer - m_qwTimer_Value) * TIME_SEC_100NS) / m_qwTimer_Frequency;
    o_rLog.qwRawTimeOffset = ((i_pData->qwTimer - m_qwTimeLast) * TIME_SEC_100NS) / m_qwTimer_Frequency;
    o_rLog.pChannel        = m_pName;
    o_rLog.qwIndex         = i_pData->dwSequence;
    o_rLog.dwId            = i_pData->wID;
    o_rLog.eLevel          = (eP7Trace_Level)i_pData->bLevel;
    o_rLog.dwCpuCore       = i_pData->bProcessor;
    o_rLog.dwThreadId      = i_pData->dwThreadID;
    o_rLog.pDesc           = static_cast<sTraceDesc*>(l_pDesc);
    o_rLog.pMessage        = m_pText;

    m_qwTimeLast           = i_pData->qwTimer;

l_lblExit:
    return l_bReturn;
}//Format()



////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//                                 CClText
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//CClText()
CClText::CClText(tXCHAR **i_pArgs, tINT32 i_iCount)
    : CClient(IP7_Client::eText, i_pArgs, i_iCount)
    , m_lReject_Mem(0)
    , m_lReject_Con(0)
    , m_lReject_Int(0)
    , m_bThread(FALSE)
    , m_hThread(0) //NULL
    , m_bNoData(FALSE)
    , m_pBuffer_Current(0)
    , m_dwBuffer_Size(0)
    , m_dwBuffers_Count(0)
    , m_bBig_Endian(FALSE)
    , m_pChunk(0)
    , m_szChunkMax(0)
    , m_szChunkUsd(0)
    , m_pFormat(NULL)
    , m_pMsg(NULL)
    , m_pMsgCur(NULL)
    , m_szMsg(0)
    , m_pSink(NULL)
{
    memset(m_pTxtCh, 0, sizeof(m_pTxtCh));

    //1. base init
    if (ECLIENT_STATUS_OK == m_eStatus)
    {
        m_eStatus = Init_Base(i_pArgs, i_iCount);
    }

    //2. Initialize Pool
    if (ECLIENT_STATUS_OK == m_eStatus)
    {
        m_eStatus = Init_Pool(i_pArgs, i_iCount);
    }
   
    //3. Initialize backend
    if (ECLIENT_STATUS_OK == m_eStatus)
    {
        m_eStatus = Init_Backend(i_pArgs, i_iCount);
    }

    //4. Initialize variables
    if (ECLIENT_STATUS_OK == m_eStatus)
    {
        m_eStatus = Init_Thread(i_pArgs, i_iCount);
    }

    //5. Initialize crash handler
    if (ECLIENT_STATUS_OK == m_eStatus)
    {
        Init_Crash_Handler(i_pArgs, i_iCount);
    }
}//CClText()


////////////////////////////////////////////////////////////////////////////////
//~CClText()
CClText::~CClText()
{
    Uninit_Crash_Handler();

    Flush();

    if (m_pSink)
    {
        delete m_pSink;
        m_pSink = NULL;
    }

    if (m_pBuffer_Current)
    {
        delete m_pBuffer_Current;
        m_pBuffer_Current = NULL;
    }

    if (m_pChunk)
    {
        free(m_pChunk);
        m_pChunk = NULL;
    }

    m_szChunkMax = 0;

    m_cBuffer_Empty.Clear(TRUE);
    m_cBuffer_Ready.Clear(TRUE);

    for (size_t l_szI = 0; l_szI < LENGTH(m_pChannels); l_szI++)
    {
        if (m_pTxtCh[l_szI])
        {
            delete m_pTxtCh[l_szI];
            m_pTxtCh[l_szI] = NULL;
        }
    }

    while (m_pFormat)
    {
        sFormat *l_pTmp = m_pFormat;
        m_pFormat = m_pFormat->pNext;

        if (l_pTmp->pStr)
        {
            free(l_pTmp->pStr);
            l_pTmp->pStr = NULL;
        }

        free(l_pTmp);
    }

    CClient::Unshare();


    if (m_pMsg)
    {
        free(m_pMsg);
        m_pMsg = NULL;
    }
    
    m_pMsgCur = NULL;

    //printf("Reject mem:%d, con:%d, init:%d\n", m_lReject_Mem, m_lReject_Con, m_lReject_Int);
}//~CClText()


////////////////////////////////////////////////////////////////////////////////
//Init_Base
eClient_Status CClText::Init_Base(tXCHAR **i_pArgs,
                                  tINT32   i_iCount
                                 )
{
    eClient_Status l_eReturn    = ECLIENT_STATUS_WRONG_FORMAT;
    tXCHAR        *l_pFormatStr = NULL;
    tUINT32        l_dwEndian   = 0x1;
    tUINT8         l_bLittleE   = *(tUINT8*)&l_dwEndian;

    if (!l_bLittleE)
    {
        m_bBig_Endian = TRUE;
    }

    ////////////////////////////////////////////////////////////////////////////
    //getting pool size
    l_pFormatStr = Get_Argument_Text_Value(i_pArgs, i_iCount,
                                           (tXCHAR*)CLIENT_COMMAND_FORMAT);
    if (l_pFormatStr)
    {
        l_eReturn = ParseFormat(l_pFormatStr);
    }

    if (ECLIENT_STATUS_OK != l_eReturn)
    {
        l_eReturn = ParseFormat(TM("%")     CLIENT_FMT_CHANNEL      TM(" ")
                                TM("#%")    CLIENT_FMT_MSG_INDEX    TM(" ")
                                TM("[%")    CLIENT_FMT_TIME_FULL    TM("] ")
                                TM("%")     CLIENT_FMT_LEVEL        TM(" ")
                                TM("Tr:#%") CLIENT_FMT_THREAD_ID    TM(":")
                                TM("%")     CLIENT_FMT_THREAD_NAME  TM(" ")
                                TM("CPU:%") CLIENT_FMT_CPU_CORE     TM(" ")
                                TM("Md:%")  CLIENT_FMT_MODULE_NAME  TM(" ")
                                TM("{%")    CLIENT_FMT_FILE_SHORT   TM(":")
                                TM("%")     CLIENT_FMT_FILE_LINE    TM(":")
                                TM("%")     CLIENT_FMT_FUNCTION     TM("} ")
                                TM("%")     CLIENT_FMT_MSG
                               );
    }

    return l_eReturn; 
}//Init_Base


////////////////////////////////////////////////////////////////////////////////
//Init_Pool
eClient_Status CClText::Init_Pool(tXCHAR **i_pArgs,
                                  tINT32   i_iCount
                                 )
{
    eClient_Status l_eReturn         = ECLIENT_STATUS_OK;
    tXCHAR        *l_pArg_Value      = NULL;
    tUINT32        l_dwPool_Size     = DEF_POOL_SIZE;
    tUINT32        l_dwBuffer_Size   = MAX_BUFFER_SIZE * 2;
    tUINT32        l_dwBuffers_Count = 0;

    ////////////////////////////////////////////////////////////////////////////
    //getting pool size
    l_pArg_Value = Get_Argument_Text_Value(i_pArgs, i_iCount,
                                           (tXCHAR*)CLIENT_COMMAND_POOL_SIZE);
    if (l_pArg_Value)
    {
        l_dwPool_Size = 1024 * (tUINT32)PStrToInt(l_pArg_Value);

        if (MIN_POOL_SIZE > l_dwPool_Size)
        {
            l_dwPool_Size = MIN_POOL_SIZE;
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    //calculate buffer size and buffers count
    //Starting from largest buffer size trying to calculate how much buffers we
    //can allocate, is amount is smaller than minimum - device buffer size by 2
    //and repeat procedure
    while (l_dwBuffers_Count < MIN_BUFFERS_COUNT)
    {
        l_dwBuffer_Size /= 2; 
        l_dwBuffers_Count = l_dwPool_Size / l_dwBuffer_Size;
    }

    if (    (MIN_BUFFER_SIZE   > l_dwBuffer_Size)
         || (MIN_BUFFERS_COUNT > l_dwBuffers_Count)
       )
    {
        JOURNAL_ERROR(m_pLog, 
                      TM("Pool: Memory calculation error, buffer size = %d, buffers count = %d"),
                      l_dwBuffer_Size,
                      l_dwBuffers_Count
                    );
        l_eReturn = ECLIENT_STATUS_INTERNAL_ERROR;
        goto l_lblExit;
    }

    ////////////////////////////////////////////////////////////////////////////
    //allocate buffers
    m_dwBuffer_Size   = l_dwBuffer_Size;
    m_dwBuffers_Count = l_dwBuffers_Count;

    for (tUINT32 l_dwI = 0; l_dwI < m_dwBuffers_Count; l_dwI++)
    {
        sBuffer *l_pBuffer = new sBuffer(m_dwBuffer_Size);
        if (l_pBuffer)
        {
            m_cBuffer_Empty.Add_After(NULL, l_pBuffer);
        }
        else
        {
            JOURNAL_ERROR(m_pLog, TM("Pool: Memory calculation failed"));
            l_eReturn = ECLIENT_STATUS_INTERNAL_ERROR;
            goto l_lblExit;
        }
    }

    m_szChunkMax = 4096;
    m_pChunk  = (tUINT8*)malloc(m_szChunkMax);
    if (NULL == m_pChunk)
    {
        JOURNAL_ERROR(m_pLog, TM("Pool: Memory fragment allocation failed"));
        l_eReturn = ECLIENT_STATUS_INTERNAL_ERROR;
        goto l_lblExit;
    }


    m_szMsg   = 4096;
    m_pMsg    = (tXCHAR*)malloc(m_szMsg * sizeof(tXCHAR));
    m_pMsgCur = m_pMsg;

    if (NULL == m_pMsg)
    {
        JOURNAL_ERROR(m_pLog, TM("Pool: Memory message allocation failed"));
        l_eReturn = ECLIENT_STATUS_INTERNAL_ERROR;
        goto l_lblExit;
    }

l_lblExit:
    return l_eReturn; 
}//Init_Pool


////////////////////////////////////////////////////////////////////////////////
//Init_File
eClient_Status CClText::Init_Backend(tXCHAR **i_pArgs, tINT32 i_iCount)
{
    eClient_Status  l_eReturn  = ECLIENT_STATUS_OK;
    tXCHAR         *l_pSink    = NULL;

    ////////////////////////////////////////////////////////////////////////////
    //getting pool size
    l_pSink = Get_Argument_Text_Value(i_pArgs, i_iCount,
                                      (tXCHAR*)CLIENT_COMMAND_LINE_SINK);
    if (l_pSink)
    {
        if (0 == PStrICmp(l_pSink, CLIENT_SINK_CONSOLE))
        {
            m_pSink = static_cast<CClTextSink *>(new CClTextConsole());
        }
        else if (0 == PStrICmp(l_pSink, CLIENT_SINK_FILE_TXT))
        {
            m_pSink = static_cast<CClTextSink *>(new CClTextFile());
        }
        else if (0 == PStrICmp(l_pSink, CLIENT_SINK_SYSLOG))
        {
            m_pSink = static_cast<CClTextSink *>(new CClTextSyslog());
        }
    }

    if (!m_pSink) 
    {
        m_pSink = static_cast<CClTextSink *>(new CClTextConsole());
    }

    if (m_pSink)
    {
        l_eReturn = m_pSink->Initialize(i_pArgs, i_iCount);
    }

    return l_eReturn;
}//Init_File


////////////////////////////////////////////////////////////////////////////////
//Init_Thread
eClient_Status CClText::Init_Thread(tXCHAR **i_pArgs,
                                    tINT32   i_iCount
                                   )
{
    eClient_Status l_eReturn = ECLIENT_STATUS_OK;

    UNUSED_ARG(i_pArgs);
    UNUSED_ARG(i_iCount);

    if (ECLIENT_STATUS_OK == l_eReturn)
    {
        if (FALSE == m_cEvThread.Init(2, EMEVENT_SINGLE_AUTO, EMEVENT_MULTI))
        {
            l_eReturn = ECLIENT_STATUS_INTERNAL_ERROR;
            JOURNAL_ERROR(m_pLog, TM("Exit event wasn't created !"));
        }

        if (FALSE == m_cEvData.Init(1, EMEVENT_SINGLE_AUTO))
        {
            l_eReturn = ECLIENT_STATUS_INTERNAL_ERROR;
            JOURNAL_ERROR(m_pLog, TM("Exit event wasn't created !"));
        }
    }

    if (ECLIENT_STATUS_OK == l_eReturn)
    {
        if (FALSE == CThShell::Create(&Static_Routine,
                                      this,
                                      &m_hThread,
                                      TM("P7:Comm") 
                                     )
           )
        {
            l_eReturn      = ECLIENT_STATUS_INTERNAL_ERROR;
            JOURNAL_ERROR(m_pLog, TM("Communication thread wasn't created !"));
        }
        else
        {
            m_bThread = TRUE;
        }
    }

    return l_eReturn;
}//Init_Thread


////////////////////////////////////////////////////////////////////////////////
//ParseFormat
eClient_Status CClText::ParseFormat(const tXCHAR *i_pFormat)
{
    eClient_Status l_eReturn  = ECLIENT_STATUS_INTERNAL_ERROR;
    const tXCHAR  *l_pCursor  = NULL;

    l_pCursor = i_pFormat;

    while (*l_pCursor)
    {
        if (TM('%') == *l_pCursor)
        {
            fnFormat l_pFn = GetFormatFunction(l_pCursor + 1);
            if (l_pFn)
            {
                if (AddFormatNode(i_pFormat, l_pCursor - i_pFormat, l_pFn))
                {
                    l_eReturn = ECLIENT_STATUS_OK;
                    l_pCursor += 2;
                    i_pFormat  = l_pCursor+ 1;
                }
                else
                {
                    l_eReturn = ECLIENT_STATUS_INTERNAL_ERROR;
                    break;
                }
            }
        }

        l_pCursor ++;
    }

    if (    (ECLIENT_STATUS_OK == l_eReturn)    
         && (*i_pFormat)
       )
    {
        if (!AddFormatNode(i_pFormat, l_pCursor - i_pFormat, NULL))
        {
            l_eReturn = ECLIENT_STATUS_INTERNAL_ERROR;
        }
    }

    if (ECLIENT_STATUS_OK == l_eReturn)    
    {
        if (!m_pFormat)
        {
            l_eReturn = ECLIENT_STATUS_WRONG_FORMAT;
        }
        else
        {
            sFormat *l_pFormat = m_pFormat;
            l_eReturn = ECLIENT_STATUS_WRONG_FORMAT;
            while (l_pFormat)
            {
                if (l_pFormat->pFormat)
                {
                    l_eReturn = ECLIENT_STATUS_OK;
                    break;
                }

                l_pFormat = l_pFormat->pNext;
            }
        }
    }

    return l_eReturn;
}

////////////////////////////////////////////////////////////////////////////////
//AddFormatNode
CClText::sFormat *CClText::AddFormatNode(const tXCHAR *i_pPrefix, size_t i_szPrefix, fnFormat i_pFn)
{
    sFormat *l_pReturn = (sFormat*)malloc(sizeof(sFormat));

    if (!l_pReturn)
    {
        return l_pReturn;
    }

    memset(l_pReturn, 0, sizeof(sFormat));

    if (!m_pFormat)
    {
        m_pFormat = l_pReturn;
    }
    else
    {
        sFormat *l_pNext = m_pFormat;
        while (l_pNext->pNext) { l_pNext = l_pNext->pNext; }
        l_pNext->pNext = l_pReturn;
    }

    if (i_szPrefix)
    {
        l_pReturn->pStr = (tXCHAR*)malloc((i_szPrefix + 1) * sizeof(tXCHAR));
        if (l_pReturn->pStr)
        {
            memcpy(l_pReturn->pStr, i_pPrefix, i_szPrefix * sizeof(tXCHAR));
            l_pReturn->pStr[i_szPrefix] = 0;
            l_pReturn->szStr = i_szPrefix;
        }
    }

    l_pReturn->pFormat = i_pFn;
    return l_pReturn;
}


////////////////////////////////////////////////////////////////////////////////
//Sent
eClient_Status CClText::Sent(tUINT32          i_dwChannel_ID,
                             sP7C_Data_Chunk *i_pChunks, 
                             tUINT32          i_dwCount,
                             tUINT32          i_dwSize
                            )
{
    eClient_Status   l_eReturn       = ECLIENT_STATUS_OK;
    sH_User_Data     l_sHeader       = {i_dwSize + (tUINT32)sizeof(l_sHeader), i_dwChannel_ID};
    sP7C_Data_Chunk  l_sHeader_Chunk = {&l_sHeader, (tUINT32)sizeof(l_sHeader)};
    //Warn: variables without default values!
    tBOOL            l_bExit;
    sP7C_Data_Chunk *l_pChunk;
    tUINT32          l_dwChunk_Offs;
    pAList_Cell      l_pEl;
    tUINT32          l_dwFree_Size;

    if (ECLIENT_STATUS_OK != m_eStatus)
    {
        ATOMIC_INC(&m_lReject_Int);
        return m_eStatus;
    }

    if (    (NULL                            == i_pChunks)
         || (0                               >= i_dwCount)
         || (USER_PACKET_MAX_SIZE            <= l_sHeader.dwSize) 
         || (USER_PACKET_CHANNEL_ID_MAX_SIZE <= i_dwChannel_ID)
       )
    {
        ATOMIC_INC(&m_lReject_Int);
        return ECLIENT_STATUS_WRONG_PARAMETERS;
    }

    //N.B. We do not check i_dwSize and real size of all chunks in release mode!!!
#ifdef _DEBUG
    tUINT32 l_dwReal_Size = 0;
    for (tUINT32 l_dwI = 0; l_dwI < i_dwCount; l_dwI ++)
    {
        if (i_pChunks[l_dwI].pData)
        {
            l_dwReal_Size += i_pChunks[l_dwI].dwSize;
        }
        else
        {
            break;
        }
    }

    if (l_dwReal_Size != i_dwSize)
    {
        ATOMIC_INC(&m_lReject_Int);
        return ECLIENT_STATUS_WRONG_PARAMETERS;
    }
#endif

    LOCK_ENTER(m_hCS);

    if (FALSE == m_bConnected)
    {
        l_eReturn = ECLIENT_STATUS_OFF;
        ATOMIC_INC(&m_lReject_Con);
        goto l_lExit;
    }

    if (l_sHeader.dwSize >= (m_dwBuffer_Size * m_dwBuffers_Count))
    {
        l_eReturn = ECLIENT_STATUS_NO_FREE_BUFFERS;
        ATOMIC_INC(&m_lReject_Mem);
        goto l_lExit;
    }

    do
    {
        l_dwFree_Size =   (m_cBuffer_Empty.Count() * m_dwBuffer_Size) 
                        + ((m_pBuffer_Current) ? (m_dwBuffer_Size - (tUINT32)m_pBuffer_Current->szUsed) : 0);
    
        //if size is more than available ...
        if (l_dwFree_Size < l_sHeader.dwSize)
        {
            m_bNoData = TRUE;
    
            LOCK_EXIT(m_hCS);
            m_cEvData.Wait(FREE_DATA_WAIT_TIMEOUT);
            LOCK_ENTER(m_hCS);
        }
        else
        {
            break;
        }
    } while (1);

    //because we add header as chunk
    i_dwCount ++;

    l_bExit        = FALSE;
    l_pChunk       = &l_sHeader_Chunk; //&i_pChunks[0];
    l_dwChunk_Offs = 0;
    
    while (FALSE == l_bExit)
    {
        //if packet is null we need to extract another one 
        if (NULL == m_pBuffer_Current)
        {
            l_pEl = m_cBuffer_Empty.Get_First();
            m_pBuffer_Current = m_cBuffer_Empty.Get_Data(l_pEl);
            m_cBuffer_Empty.Del(l_pEl, FALSE); 
        }

        while (    (m_pBuffer_Current)
                && (i_dwCount)
              )
        {
            //if packet free size is larger or equal to chunk size
            if ( TAIL_SIZE(m_pBuffer_Current) >= l_pChunk->dwSize )
            {
                memcpy(m_pBuffer_Current->pBuffer + m_pBuffer_Current->szUsed, 
                       ((tUINT8*)l_pChunk->pData) + l_dwChunk_Offs,
                       l_pChunk->dwSize
                      );

                m_pBuffer_Current->szUsed += l_pChunk->dwSize;

                //current chunk was moved, we reduce chunks amount 
                --i_dwCount;

                if (0 >= i_dwCount)
                {
                    l_bExit = TRUE;
                }
                else
                {
                    //we are finish with that chunk
                    //l_pChunk->dwSize = 0; 
                    l_dwChunk_Offs = 0;

                    if (l_pChunk == &l_sHeader_Chunk)
                    {
                        l_pChunk = &i_pChunks[0];
                    }
                    else
                    {
                        //go to next chunk
                        l_pChunk ++;
                    }

                    //if packet is filled - put it to data queue
                    if (0 >= TAIL_SIZE(m_pBuffer_Current))
                    {
                        m_cBuffer_Ready.Add_After(m_cBuffer_Ready.Get_Last(), 
                                                  m_pBuffer_Current
                                                 );
                        m_pBuffer_Current = NULL;
                        m_cEvThread.Set(THREAD_DATA_SIGNAL);
                    }
                }
            }
            else //if chunk data is greater than packet free space
            {
                memcpy(m_pBuffer_Current->pBuffer + m_pBuffer_Current->szUsed, 
                       ((tUINT8*)l_pChunk->pData) + l_dwChunk_Offs,
                       TAIL_SIZE(m_pBuffer_Current)
                      );
                l_dwChunk_Offs   += TAIL_SIZE(m_pBuffer_Current);
                l_pChunk->dwSize -= TAIL_SIZE(m_pBuffer_Current);

                m_pBuffer_Current->szUsed += TAIL_SIZE(m_pBuffer_Current);

                m_cBuffer_Ready.Add_After(m_cBuffer_Ready.Get_Last(), 
                                          m_pBuffer_Current
                                         );
                m_pBuffer_Current = NULL;
                m_cEvThread.Set(THREAD_DATA_SIGNAL);
            }
        } //while ( (m_pBuffer_Current) && (i_dwCount) )
    } //while (FALSE == l_bExit)

l_lExit:

    LOCK_EXIT(m_hCS);

    return l_eReturn;
}//Sent


////////////////////////////////////////////////////////////////////////////////
//Get_Info
tBOOL CClText::Get_Info(sP7C_Info *o_pInfo)
{
    if (NULL == o_pInfo)
    {
        return FALSE;
    }

    LOCK_ENTER(m_hCS);

    o_pInfo->dwMem_Alloc = m_dwBuffer_Size * m_dwBuffers_Count;
    o_pInfo->dwMem_Free  = m_cBuffer_Empty.Count() * m_dwBuffer_Size;
    o_pInfo->dwMem_Used  = m_cBuffer_Ready.Count() * m_dwBuffer_Size;

    o_pInfo->dwReject_Mem = m_lReject_Mem;
    o_pInfo->dwReject_Con = m_lReject_Con;
    o_pInfo->dwReject_Int = m_lReject_Int;
    LOCK_EXIT(m_hCS);

    return TRUE;
}//Get_Info


////////////////////////////////////////////////////////////////////////////////
//Flush
tBOOL CClText::Flush()
{
    tBOOL l_bStack_Trace = TRUE;

    ////////////////////////////////////////////////////////////////////////////
    //notify channels about closing
    sP7C_Status l_sStatus = {FALSE, 0};
    l_sStatus.bConnected = FALSE;
    l_sStatus.dwResets   = 0;

    LOCK_ENTER(m_hCS_Reg);
    for (tUINT32 l_dwI = 0; l_dwI < USER_PACKET_CHANNEL_ID_MAX_SIZE; l_dwI++)
    {
        if (m_pChannels[l_dwI])
        {
            m_pChannels[l_dwI]->On_Flush(l_dwI, &l_bStack_Trace);
            m_pChannels[l_dwI]->On_Status(l_dwI, &l_sStatus);
        }
    }
    LOCK_EXIT(m_hCS_Reg);

    m_cEvThread.Set(THREAD_EXIT_SIGNAL);

    if (m_bThread)
    {
        if (TRUE == CThShell::Close(m_hThread, 60000))
        {
            m_hThread = 0;//NULL;
            m_bThread = FALSE;
        }
        else
        {
            JOURNAL_CRITICAL(m_pLog, TM("Can't close file thread !"));
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    //Lock
    LOCK_ENTER(m_hCS);

    pAList_Cell l_pEl = NULL;
    while ((l_pEl = m_cBuffer_Ready.Get_Next(l_pEl)))
    {
        sBuffer *l_pBuffer = m_cBuffer_Ready.Get_Data(l_pEl);
        if (l_pBuffer)
        {
            Parse_Buffer(l_pBuffer->pBuffer, l_pBuffer->szUsed);
        }
    }

    if (m_pBuffer_Current)
    {
        Parse_Buffer(m_pBuffer_Current->pBuffer, m_pBuffer_Current->szUsed);
    }

    m_bConnected = FALSE;

    if (m_pSink)
    {
        delete m_pSink;
        m_pSink = NULL;
    }

    LOCK_EXIT(m_hCS);
    //unlock
    ////////////////////////////////////////////////////////////////////////////

    return TRUE;
}//Flush


////////////////////////////////////////////////////////////////////////////////
//Parse_Buffer
eClient_Status CClText::Parse_Buffer(tUINT8 *i_pBuffer, size_t  i_szBuffer)
{
    eClient_Status l_eReturn  = ECLIENT_STATUS_OK;
    size_t         l_szOffset = 0;
    size_t         l_szTail   = 0;
    while (l_szOffset < i_szBuffer)    
    {
        l_szTail = i_szBuffer - l_szOffset;

        if (m_szChunkUsd)
        {
            //if header has been copied 
            if (sizeof(sH_User_Data) <= m_szChunkUsd)    
            {
                if (((sH_User_Data*)m_pChunk)->dwSize > m_szChunkMax)
                {
                    size_t  l_szChunkMax = (((sH_User_Data*)m_pChunk)->dwSize + 1023) & (~1023);
                    tUINT8 *l_pChunk     = (tUINT8*)realloc(m_pChunk, l_szChunkMax);

                    if (!l_pChunk)
                    {
                        l_pChunk = (tUINT8*)malloc(l_szChunkMax);

                        if (    (l_pChunk)
                             && (m_pChunk)
                           )
                        {
                            memcpy(l_pChunk, m_pChunk, m_szChunkMax);
                        }

                        if (m_pChunk) { free(m_pChunk); }
                    }

                    m_pChunk = l_pChunk;

                    if (m_pChunk)
                    {
                        m_szChunkMax = l_szChunkMax;
                    }
                    else
                    {
                        m_szChunkMax = 0;
                    }
                }

                if (m_pChunk)
                {
                    size_t l_szPacketTail = ((sH_User_Data*)m_pChunk)->dwSize - m_szChunkUsd;

                    if (l_szPacketTail > l_szTail)
                    {
                        l_szPacketTail = l_szTail;
                    }

                    memcpy(m_pChunk + m_szChunkUsd, i_pBuffer + l_szOffset, l_szPacketTail);
                    m_szChunkUsd += l_szPacketTail;
                    l_szOffset   += l_szPacketTail;

                    if (m_szChunkUsd == ((sH_User_Data*)m_pChunk)->dwSize)
                    {
                        l_eReturn = Parse_Packet(((sH_User_Data*)m_pChunk)->dwChannel_ID, 
                                                 m_pChunk + sizeof(sH_User_Data), 
                                                 ((sH_User_Data*)m_pChunk)->dwSize - sizeof(sH_User_Data));
                        m_szChunkUsd = 0;
                    }
                }
                else
                {
                    l_eReturn = ECLIENT_STATUS_INTERNAL_ERROR;
                }
            }
            else //copy header reminder
            {
                size_t l_szHeaderTail = sizeof(sH_User_Data) - m_szChunkUsd;
                if (l_szHeaderTail > l_szTail)
                {
                    l_szHeaderTail = l_szTail;
                }

                memcpy(m_pChunk + m_szChunkUsd, i_pBuffer + l_szOffset, l_szHeaderTail);

                m_szChunkUsd += l_szHeaderTail;
                l_szOffset   += l_szHeaderTail;
            }
        } //if buffer contains enough data for analysis 
        else if (    (sizeof(sH_User_Data) < l_szTail)
                  && (((sH_User_Data*)(i_pBuffer + l_szOffset))->dwSize <= l_szTail)
                )
        {
            l_eReturn = Parse_Packet(((sH_User_Data*)(i_pBuffer + l_szOffset))->dwChannel_ID,
                                     i_pBuffer + l_szOffset + sizeof(sH_User_Data),
                                     ((sH_User_Data*)(i_pBuffer + l_szOffset))->dwSize - sizeof(sH_User_Data)
                                     );
            l_szOffset += ((sH_User_Data*)(i_pBuffer + l_szOffset))->dwSize;
        }
        else  //if size of the remaining data is smaller that size of user data
        {
            if (l_szTail > m_szChunkMax)
            {
                size_t  l_szChunkMax = (l_szTail + 1023) & (~1023);
                tUINT8 *l_pChunk     = (tUINT8*)realloc(m_pChunk, l_szChunkMax);

                if (!l_pChunk)
                {
                    l_pChunk = (tUINT8*)malloc(l_szChunkMax);

                    if (    (l_pChunk)
                            && (m_pChunk)
                        )
                    {
                        memcpy(l_pChunk, m_pChunk, m_szChunkMax);
                    }

                    if (m_pChunk) { free(m_pChunk); }
                }

                m_pChunk = l_pChunk;

                if (m_pChunk)
                {
                    m_szChunkMax = l_szChunkMax;
                }
                else
                {
                    m_szChunkMax = 0;
                }
            }

            if (m_pChunk)
            {
                memcpy(m_pChunk, i_pBuffer + l_szOffset, l_szTail);
                m_szChunkUsd = l_szTail;
                l_szOffset  += l_szTail;
            }
            else
            {
                l_eReturn = ECLIENT_STATUS_INTERNAL_ERROR;
            }
        }

        if (ECLIENT_STATUS_OK != l_eReturn)
        {
            break;
        }
    }

    return l_eReturn;
}//Parse_Buffer


////////////////////////////////////////////////////////////////////////////////
//Parse_Packet
eClient_Status CClText::Parse_Packet(tUINT32 i_dwChannel, 
                                     tUINT8 *i_pPacket, 
                                     size_t  i_szPacket
                                    )
{
    eClient_Status l_eReturn  = ECLIENT_STATUS_OK;
    size_t         l_szOffset = 0;
    sP7Ext_Header *l_pHeader  = (sP7Ext_Header *)i_pPacket;
    CTxtChannel  *&l_pChannel = m_pTxtCh[i_dwChannel];
    sP7Ext_Raw     l_sRaw; //not initialized on purpose

    while (l_szOffset < i_szPacket)    
    {
        l_pHeader = (sP7Ext_Header *)(i_pPacket + l_szOffset);

        //N.B.: Bits reconstruction, sP7Ext_Header is formed manually to avoid complier influence to bit fields (Using structures with 
        //      bit fields isn't regulated by standard, every compiler is free to use own bits ordering). Headers formed in Trace or 
        //      Telemetry may be used on different systems and they have to be unified
        l_sRaw.dwBits = ((sP7Ext_Raw*)l_pHeader)->dwBits; 
        l_pHeader->dwType    = GET_EXT_HEADER_TYPE(l_sRaw);
        l_pHeader->dwSubType = GET_EXT_HEADER_SUBTYPE(l_sRaw);
        l_pHeader->dwSize    = GET_EXT_HEADER_SIZE(l_sRaw);

        if (l_pChannel)
        {
            if (EP7TRACE_TYPE_DATA == l_pHeader->dwSubType)
            {
                if (l_pChannel->Format((sP7Trace_Data*)l_pHeader, m_sLog))
                {
                    sFormat *l_pCursor = m_pFormat; 
                    m_pMsgCur = m_pMsg;

                    while (l_pCursor)
                    {
                        if (l_pCursor->pStr)
                        {
                            REALLOC_MESSAGE(l_pCursor->szStr, this);
                            memcpy(m_pMsgCur, l_pCursor->pStr, l_pCursor->szStr * sizeof(tXCHAR));
                            m_pMsgCur += l_pCursor->szStr;
                        }
                    
                        if (l_pCursor->pFormat)
                        {
                            l_pCursor->pFormat(this);
                        }
                    
                        l_pCursor = l_pCursor->pNext;
                    }
                    *m_pMsgCur = 0;

                    m_pSink->Log(m_sLog, m_pMsg, m_pMsgCur - m_pMsg);
                }
            }
            else if (EP7TRACE_TYPE_DESC == l_pHeader->dwSubType)
            {
                l_pChannel->PutFormat((const sP7Trace_Format*)l_pHeader);
            }
            else if (EP7TRACE_TYPE_VERB == l_pHeader->dwSubType) 
            {
            }
            else if (EP7TRACE_TYPE_CLOSE == l_pHeader->dwSubType)
            {
                delete l_pChannel;
                l_pChannel = NULL;
            }
            else if (EP7TRACE_TYPE_THREAD_START == l_pHeader->dwSubType)
            {
                l_pChannel->RegisterThread((const sP7Trace_Thread_Start*)l_pHeader);
            }
            else if (EP7TRACE_TYPE_THREAD_STOP == l_pHeader->dwSubType)
            {
                l_pChannel->UnregisterThread((const sP7Trace_Thread_Stop*)l_pHeader);
            }
            else if (EP7TRACE_TYPE_MODULE == l_pHeader->dwSubType)
            {
                l_pChannel->PutModule((const sP7Trace_Module*)l_pHeader);
            }
        }
        else 
        {
            if (EP7TRACE_TYPE_INFO == l_pHeader->dwSubType)
            {
                if (l_pChannel)
                {
                    delete l_pChannel;
                }

                l_pChannel = new CTxtChannel((const sP7Trace_Info*)l_pHeader, m_bBig_Endian);
            }
            else
            {
                l_eReturn = ECLIENT_STATUS_INTERNAL_ERROR;
            }
        }

        l_szOffset += l_pHeader->dwSize;
    }

    if (l_szOffset > i_szPacket)
    {
        l_eReturn = ECLIENT_STATUS_INTERNAL_ERROR;
    }

    return l_eReturn;
}//Parse_Packet


////////////////////////////////////////////////////////////////////////////////
//Routine
void CClText::Routine()
{
    pAList_Cell    l_pEl            = NULL;
    tBOOL          l_bExit          = FALSE;
    tUINT32        l_dwSignal       = MEVENT_TIME_OUT;
    tUINT32        l_dwRollTime     = GetTickCount();
    tUINT32        l_dwDumpTime     = GetTickCount();
    sBuffer       *l_pBuffer        = NULL;
    eClient_Status l_eStatus        = ECLIENT_STATUS_OK; 

    while (    (FALSE == l_bExit)
            && (ECLIENT_STATUS_OK == l_eStatus)
          )
    {
        l_dwSignal = m_cEvThread.Wait(THREAD_IDLE_TIMEOUT);

        if (THREAD_EXIT_SIGNAL == l_dwSignal)
        {
            l_bExit = TRUE;
        }
        else if (THREAD_DATA_SIGNAL == l_dwSignal) //one buffer to write!
        {
            l_dwDumpTime = GetTickCount();

            ////////////////////////////////////////////////////////////////////
            //extract buffer
            LOCK_ENTER(m_hCS);
            l_pEl     = m_cBuffer_Ready.Get_First();
            l_pBuffer = m_cBuffer_Ready.Get_Data(l_pEl);
            m_cBuffer_Ready.Del(l_pEl, FALSE);
            LOCK_EXIT(m_hCS); 

            ////////////////////////////////////////////////////////////////////
            //format data & put data to free queue back
            if (l_pBuffer)
            {
                l_eStatus = Parse_Buffer(l_pBuffer->pBuffer, l_pBuffer->szUsed);

                LOCK_ENTER(m_hCS);
                l_pBuffer->szUsed = 0;
                m_cBuffer_Empty.Add_After(NULL, l_pBuffer);
                if (m_bNoData)
                {
                    m_cEvData.Set(DATA_FREE_SIGNAL);
                    m_bNoData = FALSE;
                }
                LOCK_EXIT(m_hCS); 
            }
        }
        else if (MEVENT_TIME_OUT == l_dwSignal)
        {
            LOCK_ENTER(m_hCS);
            l_pBuffer = m_pBuffer_Current;
            m_pBuffer_Current = NULL;
            LOCK_EXIT(m_hCS); 
        
            if (l_pBuffer)
            {
                l_eStatus = Parse_Buffer(l_pBuffer->pBuffer, l_pBuffer->szUsed);
        
                LOCK_ENTER(m_hCS);
                l_pBuffer->szUsed = 0;
                m_cBuffer_Empty.Add_After(NULL, l_pBuffer);
                if (m_bNoData)
                {
                    m_cEvData.Set(DATA_FREE_SIGNAL);
                    m_bNoData = FALSE;
                }
                LOCK_EXIT(m_hCS); 
            }
        }

        if (m_pSink)
        {
            if (CTicks::Difference(GetTickCount(), l_dwRollTime) > 60000) //1 minute
            {
                m_pSink->TryRoll();
                l_dwRollTime = GetTickCount();
            }

            if (CTicks::Difference(GetTickCount(), l_dwDumpTime) > 1000) //1 second
            {
                m_pSink->DumpBuffers();
                l_dwDumpTime = GetTickCount();
            }
        }
    }
}//Comm_Routine


////////////////////////////////////////////////////////////////////////////////
//FormatChannel
void CClText::FormatChannel(CClText *i_pClient)
{
    REALLOC_MESSAGE(i_pClient->m_sLog.szChannel + 16, i_pClient);
    memcpy(i_pClient->m_pMsgCur, i_pClient->m_sLog.pChannel, i_pClient->m_sLog.szChannel * sizeof(tXCHAR));
    i_pClient->m_pMsgCur += i_pClient->m_sLog.szChannel;
}

////////////////////////////////////////////////////////////////////////////////
//FormatMessageId
void CClText::FormatMessageId(CClText *i_pClient)
{
    REALLOC_MESSAGE(64, i_pClient);

    tINT32 l_iRes = PSPrint(i_pClient->m_pMsgCur,
                            i_pClient->m_szMsg - (i_pClient->m_pMsgCur - i_pClient->m_pMsg),
                            TM("%04d"),
                            i_pClient->m_sLog.dwId
                           );
    if (0 < l_iRes)
    {
        i_pClient->m_pMsgCur += l_iRes;
    }
}

////////////////////////////////////////////////////////////////////////////////
//FormatMessageIndex
void CClText::FormatMessageIndex(CClText *i_pClient)
{
    REALLOC_MESSAGE(64, i_pClient);

    tINT32 l_iRes = PSPrint(i_pClient->m_pMsgCur,
                            i_pClient->m_szMsg - (i_pClient->m_pMsgCur - i_pClient->m_pMsg),
                            TM("%04lld"),
                            i_pClient->m_sLog.qwIndex
                           );
    if (0 < l_iRes)
    {
        i_pClient->m_pMsgCur += l_iRes;
    }
}

////////////////////////////////////////////////////////////////////////////////
//FormatTimeFull
void CClText::FormatTimeFull(CClText *i_pClient)
{
    REALLOC_MESSAGE(64, i_pClient);

    tINT32 l_iRes = PSPrint(i_pClient->m_pMsgCur,
                            i_pClient->m_szMsg - (i_pClient->m_pMsgCur - i_pClient->m_pMsg),
                            TM("%02d.%02d.%04d %02d:%02d:%02d.%03d'%03d\"%d"),
                            i_pClient->m_sLog.dwDay,
                            i_pClient->m_sLog.dwMonth,
                            i_pClient->m_sLog.dwYear,
                            i_pClient->m_sLog.dwHour,
                            i_pClient->m_sLog.dwMinutes,
                            i_pClient->m_sLog.dwSeconds,
                            i_pClient->m_sLog.dwMilliseconds,
                            i_pClient->m_sLog.dwMicroseconds,
                            i_pClient->m_sLog.dwNanoseconds
                           );
    if (0 < l_iRes)
    {
        i_pClient->m_pMsgCur += l_iRes;
    }
}

////////////////////////////////////////////////////////////////////////////////
//FormatTimeMedium
void CClText::FormatTimeMedium(CClText *i_pClient)
{
    REALLOC_MESSAGE(64, i_pClient);

    tINT32 l_iRes = PSPrint(i_pClient->m_pMsgCur,
                            i_pClient->m_szMsg - (i_pClient->m_pMsgCur - i_pClient->m_pMsg),
                            TM("%02d:%02d:%02d.%03d'%03d\"%d"),
                            i_pClient->m_sLog.dwHour,
                            i_pClient->m_sLog.dwMinutes,
                            i_pClient->m_sLog.dwSeconds,
                            i_pClient->m_sLog.dwMilliseconds,
                            i_pClient->m_sLog.dwMicroseconds,
                            i_pClient->m_sLog.dwNanoseconds
                           );
    if (0 < l_iRes)
    {
        i_pClient->m_pMsgCur += l_iRes;
    }
}

////////////////////////////////////////////////////////////////////////////////
//FormatTimeShort
void CClText::FormatTimeShort(CClText *i_pClient)
{
    REALLOC_MESSAGE(64, i_pClient);

    tINT32 l_iRes = PSPrint(i_pClient->m_pMsgCur,
                            i_pClient->m_szMsg - (i_pClient->m_pMsgCur - i_pClient->m_pMsg),
                            TM("%02d:%02d.%03d'%03d\"%d"),
                            i_pClient->m_sLog.dwMinutes,
                            i_pClient->m_sLog.dwSeconds,
                            i_pClient->m_sLog.dwMilliseconds,
                            i_pClient->m_sLog.dwMicroseconds,
                            i_pClient->m_sLog.dwNanoseconds
                           );
    if (0 < l_iRes)
    {
        i_pClient->m_pMsgCur += l_iRes;
    }
}

////////////////////////////////////////////////////////////////////////////////
//FormatTimeDiff
void CClText::FormatTimeDiff(CClText *i_pClient)
{
    tUINT64 l_qwTime     = i_pClient->m_sLog.qwRawTimeOffset;
    tUINT64 l_dwReminder = l_qwTime % TIME_MSC_100NS; //micro & 100xNanoseconds
    tUINT64 l_dwNano     = l_qwTime % 10;
    tUINT64 l_dwMicro    = l_dwReminder / 10;

    l_qwTime -= l_dwReminder;

    tUINT64 l_qwSec = l_qwTime / TIME_SEC_100NS;
    tUINT64 l_qwMSc = (l_qwTime - (l_qwSec * TIME_SEC_100NS)) / TIME_MSC_100NS;

    REALLOC_MESSAGE(64, i_pClient);

    tINT32 l_iRes = PSPrint(i_pClient->m_pMsgCur,
                            i_pClient->m_szMsg - (i_pClient->m_pMsgCur - i_pClient->m_pMsg),
                            TM("+%06d.%03d'%03d\"%d"),
                            (tUINT32)l_qwSec,
                            (tUINT32)l_qwMSc,
                            (tUINT32)l_dwMicro,
                            (tUINT32)l_dwNano
                           );
    if (0 < l_iRes)
    {
        i_pClient->m_pMsgCur += l_iRes;
    }
}

////////////////////////////////////////////////////////////////////////////////
//FormatTimeCount
void CClText::FormatTimeCount(CClText *i_pClient)
{
    REALLOC_MESSAGE(64, i_pClient);

    tINT32 l_iRes = PSPrint(i_pClient->m_pMsgCur,
                            i_pClient->m_szMsg - (i_pClient->m_pMsgCur - i_pClient->m_pMsg),
                            TM("%019lld"),
                            i_pClient->m_sLog.qwRawTime 
                           );
    if (0 < l_iRes)
    {
        i_pClient->m_pMsgCur += l_iRes;
    }
}

////////////////////////////////////////////////////////////////////////////////
//FormatLevel
void CClText::FormatLevel(CClText *i_pClient)
{
    if (    (i_pClient->m_sLog.eLevel >= EP7TRACE_LEVEL_COUNT)
         || (i_pClient->m_sLog.eLevel < 0)
       )
    {
        i_pClient->m_sLog.eLevel = EP7TRACE_LEVEL_CRITICAL;
    }
    REALLOC_MESSAGE(LEVEL_TEXT_LENGTH + 16, i_pClient);
    memcpy(i_pClient->m_pMsgCur, g_pLevel[i_pClient->m_sLog.eLevel], LEVEL_TEXT_LENGTH * sizeof(tXCHAR));
    i_pClient->m_pMsgCur += LEVEL_TEXT_LENGTH;
}

////////////////////////////////////////////////////////////////////////////////
//FormatThreadId
void CClText::FormatThreadId(CClText *i_pClient)
{
    REALLOC_MESSAGE(64, i_pClient);

    tINT32 l_iRes = PSPrint(i_pClient->m_pMsgCur,
                            i_pClient->m_szMsg - (i_pClient->m_pMsgCur - i_pClient->m_pMsg),
                            TM("0x%05X"),
                            i_pClient->m_sLog.dwThreadId
                           );
    if (0 < l_iRes)
    {
        i_pClient->m_pMsgCur += l_iRes;
    }
}

////////////////////////////////////////////////////////////////////////////////
//FormatThreadName
void CClText::FormatThreadName(CClText *i_pClient)
{
    REALLOC_MESSAGE(i_pClient->m_sLog.szThreadName + 16, i_pClient);
    memcpy(i_pClient->m_pMsgCur, 
           i_pClient->m_sLog.pThreadName, 
           i_pClient->m_sLog.szThreadName * sizeof(tXCHAR)
          );
    i_pClient->m_pMsgCur += i_pClient->m_sLog.szThreadName;
}

////////////////////////////////////////////////////////////////////////////////
//FormatCpuCore
void CClText::FormatCpuCore(CClText *i_pClient)
{
    REALLOC_MESSAGE(64, i_pClient);

    tINT32 l_iRes = PSPrint(i_pClient->m_pMsgCur,
                            i_pClient->m_szMsg - (i_pClient->m_pMsgCur - i_pClient->m_pMsg),
                            TM("%02d"),
                            i_pClient->m_sLog.dwCpuCore
                           );
    if (0 < l_iRes)
    {
        i_pClient->m_pMsgCur += l_iRes;
    }
}

////////////////////////////////////////////////////////////////////////////////
//FormatModuleId
void CClText::FormatModuleId(CClText *i_pClient)
{
    REALLOC_MESSAGE(64, i_pClient);

    tINT32 l_iRes = PSPrint(i_pClient->m_pMsgCur,
                            i_pClient->m_szMsg - (i_pClient->m_pMsgCur - i_pClient->m_pMsg),
                            TM("%03d"),
                            i_pClient->m_sLog.dwModuleID
                           );
    if (0 < l_iRes)
    {
        i_pClient->m_pMsgCur += l_iRes;
    }
}

////////////////////////////////////////////////////////////////////////////////
//FormatModuleName
void CClText::FormatModuleName(CClText *i_pClient)
{
    REALLOC_MESSAGE(i_pClient->m_sLog.szModuleName + 16, i_pClient);
    memcpy(i_pClient->m_pMsgCur, 
           i_pClient->m_sLog.pModuleName, 
           i_pClient->m_sLog.szModuleName * sizeof(tXCHAR)
          );
    i_pClient->m_pMsgCur += i_pClient->m_sLog.szModuleName;
}

////////////////////////////////////////////////////////////////////////////////
//FormatFilePath
void CClText::FormatFilePath(CClText *i_pClient)
{
    REALLOC_MESSAGE(i_pClient->m_sLog.pDesc->szFilePath + 16, i_pClient);
    memcpy(i_pClient->m_pMsgCur, 
           i_pClient->m_sLog.pDesc->pFilePath, 
           i_pClient->m_sLog.pDesc->szFilePath * sizeof(tXCHAR)
          );
    i_pClient->m_pMsgCur += i_pClient->m_sLog.pDesc->szFilePath;
}

////////////////////////////////////////////////////////////////////////////////
//FormatFileName
void CClText::FormatFileName(CClText *i_pClient)
{
    REALLOC_MESSAGE(i_pClient->m_sLog.pDesc->szFileName + 16, i_pClient);
    memcpy(i_pClient->m_pMsgCur, 
           i_pClient->m_sLog.pDesc->pFileName, 
           i_pClient->m_sLog.pDesc->szFileName * sizeof(tXCHAR)
          );
    i_pClient->m_pMsgCur += i_pClient->m_sLog.pDesc->szFileName;
}

////////////////////////////////////////////////////////////////////////////////
//FormatFileLine
void CClText::FormatFileLine(CClText *i_pClient)
{
    REALLOC_MESSAGE(64, i_pClient);

    tINT32 l_iRes = PSPrint(i_pClient->m_pMsgCur,
                            i_pClient->m_szMsg - (i_pClient->m_pMsgCur - i_pClient->m_pMsg),
                            TM("%03d"),
                            (tUINT32)i_pClient->m_sLog.pDesc->wLine
                           );
    if (0 < l_iRes)
    {
        i_pClient->m_pMsgCur += l_iRes;
    }
}

////////////////////////////////////////////////////////////////////////////////
//FormatFunction
void CClText::FormatFunction(CClText *i_pClient)
{
    REALLOC_MESSAGE(i_pClient->m_sLog.pDesc->szFunction + 16, i_pClient);
    memcpy(i_pClient->m_pMsgCur, 
           i_pClient->m_sLog.pDesc->pFunction, 
           i_pClient->m_sLog.pDesc->szFunction * sizeof(tXCHAR)
          );
    i_pClient->m_pMsgCur += i_pClient->m_sLog.pDesc->szFunction;
}

////////////////////////////////////////////////////////////////////////////////
//FormatMsg
void CClText::FormatMsg(CClText *i_pClient)
{
    REALLOC_MESSAGE(i_pClient->m_sLog.szMessage + 16, i_pClient);
    memcpy(i_pClient->m_pMsgCur, 
           i_pClient->m_sLog.pMessage, 
           i_pClient->m_sLog.szMessage * sizeof(tXCHAR)
          );
    i_pClient->m_pMsgCur += i_pClient->m_sLog.szMessage;
}

