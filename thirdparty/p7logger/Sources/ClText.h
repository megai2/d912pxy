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
// This header file provide functionality to deliver data to file              /
////////////////////////////////////////////////////////////////////////////////

#ifndef CLTEXT_H
#define CLTEXT_H

#include "ClTextSink.h"
#include "ClTextConsole.h"
#include "ClTextFile.h"
#include "ClTextSyslog.h"
#include "Formatter.h"

////////////////////////////////////////////////////////////////////////////////
struct sTraceDescEx
    : public sTraceDesc
{
    tUINT8       *pBuffer;
    tUINT32       dwSize;
    tXCHAR       *pFormat;
    sP7Trace_Arg *pArgs;
    tUINT32       dwArgsLen;
    CFormatter   *pFormatter;

    sTraceDescEx(const sP7Trace_Format *i_pFormat, 
                 CFormatter::sBuffer   *i_pBuffer, 
                 tBOOL                  i_bBigEndian);
    ~sTraceDescEx();
};


////////////////////////////////////////////////////////////////////////////////
struct sP7TraceMod
{
    eP7Trace_Level eVerbosity;
    tXCHAR         pName[P7TRACE_MODULE_NAME_LENGTH];
    size_t         szName;
};


////////////////////////////////////////////////////////////////////////////////
struct sThread
{
    tUINT32  dwThreadID;                        //Thread ID
    tUINT64  qwStart;                           //High resolution timer value start
    tUINT64  qwStop;                            //High resolution timer value stop
    tXCHAR   pName[P7TRACE_THREAD_NAME_LENGTH]; //Thread name
    size_t   szName;
};


////////////////////////////////////////////////////////////////////////////////
struct sRbThread
{
    sThread   *pThread;
    sRbThread *pNext; 

    sRbThread(sThread *i_pThread, sRbThread *i_pNext = NULL)
    {
        pThread = i_pThread;
        pNext   = i_pNext;
    }

    ~sRbThread()
    {
        if (pThread)
        {
            delete pThread;
            pThread = NULL;
        }

        if (pNext)
        {
            delete pNext;
            pNext = NULL;
        }
    }
};

////////////////////////////////////////////////////////////////////////////////
class CThreads_Tree:
    public CRBTree<sRbThread*, tUINT32>
{
protected:
    //////////////////////////////////////////////////////////////////////////// 
    //Return
    //TRUE  - if (i_pKey < i_pData::key)
    //FALSE - otherwise
    tBOOL Is_Key_Less(tUINT32 i_dwKey, sRbThread *i_pData) 
    {
        return (i_dwKey < i_pData->pThread->dwThreadID) ? TRUE : FALSE;
    }

    //////////////////////////////////////////////////////////////////////////// 
    //Return
    //TRUE  - if (i_pKey == i_pData::key)
    //FALSE - otherwise
    tBOOL Is_Qual(tUINT32 i_dwKey, sRbThread *i_pData) 
    {
        return (i_dwKey == i_pData->pThread->dwThreadID) ? TRUE : FALSE;
    }
};



////////////////////////////////////////////////////////////////////////////////
class CTxtChannel
{
protected:
    //Contains a 64-bit value representing the number of 100-nanosecond intervals 
    //since January 1, 1601 (UTC). In windows we use FILETIME structure for 
    //representing
    tUINT64               m_qwStreamTime;
    //Hi resolution timer value, we get this value when we retrieve current time.
    //using difference between this value and timer value for every trace we can
    //calculate time of the trace event with hi resolution
    tUINT64               m_qwTimer_Value;
    //timer's count heartbeats in second
    tUINT64               m_qwTimer_Frequency;
    tUINT64               m_qwFlags; 

    CBList<sTraceDescEx*> m_cDesc;
    CBList<sP7TraceMod*>  m_cModules;
    CThreads_Tree         m_cThreads;
    tXCHAR               *m_pText;
    size_t                m_szText;
    tUINT64               m_qwTimeLast; 
    tXCHAR                m_pName[P7TRACE_NAME_LENGTH];
    size_t                m_szName;
    CFormatter::sBuffer  *m_pBuffer;
    tBOOL                 m_bBig_Endian;

    //Time conversion for Linux
    //http://stackoverflow.com/questions/2408976/struct-timeval-to-printable-format
public:
    CTxtChannel(const sP7Trace_Info *i_pInfo, tBOOL i_bBig_Endian);
    ~CTxtChannel();
    void  PutFormat(const sP7Trace_Format *i_pDesc);
    void  PutModule(const sP7Trace_Module *i_pMod);
    void  RegisterThread(const sP7Trace_Thread_Start *i_pStart);
    void  UnregisterThread(const sP7Trace_Thread_Stop *i_pStop);
    tBOOL Format(sP7Trace_Data* i_pData, CClTextSink::sLog &o_rLog);
};


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//                                CClText
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
class CClText:
    public CClient
{
public: 
    typedef void (*fnFormat)(CClText *i_pClient);

protected:
    struct sFormat
    {
        tXCHAR  *pStr;
        size_t   szStr;
        fnFormat pFormat;
        sFormat *pNext;
    };

    struct sBuffer
    {
        tUINT8 *pBuffer;
        size_t  szUsed;

        sBuffer(size_t i_szBuffer)
        {
            pBuffer = (tUINT8*)malloc(i_szBuffer);
            szUsed  = 0;
        }

        ~sBuffer()
        {
            if (pBuffer)
            {
                free(pBuffer);
                pBuffer = NULL;
            }
            szUsed  = 0;
        }
    };

private:
    tINT32 volatile    m_lReject_Mem;
    tINT32 volatile    m_lReject_Con;
    tINT32 volatile    m_lReject_Int;

    CMEvent            m_cEvThread;
    tBOOL              m_bThread;
    CThShell::tTHREAD  m_hThread;
    CMEvent            m_cEvData;
    volatile tBOOL     m_bNoData;

    CBList<sBuffer*>   m_cBuffer_Empty;
    CBList<sBuffer*>   m_cBuffer_Ready;
    sBuffer           *m_pBuffer_Current;
    tUINT32            m_dwBuffer_Size;
    tUINT32            m_dwBuffers_Count;

    tBOOL              m_bBig_Endian;

    tUINT8            *m_pChunk;
    size_t             m_szChunkMax;
    size_t             m_szChunkUsd;
    CTxtChannel       *m_pTxtCh[USER_PACKET_CHANNEL_ID_MAX_SIZE];
    CClTextSink::sLog  m_sLog;
    sFormat           *m_pFormat; 
    tXCHAR            *m_pMsg;
    tXCHAR            *m_pMsgCur;
    size_t             m_szMsg;
    CClTextSink       *m_pSink;

public:
    CClText(tXCHAR **i_pArgs, tINT32 i_iCount);
    virtual ~CClText();

private:
    eClient_Status Init_Base(tXCHAR **i_pArgs, tINT32 i_iCount);

    eClient_Status Init_Pool(tXCHAR **i_pArgs, tINT32 i_iCount);

    eClient_Status Init_Backend(tXCHAR **i_pArgs, tINT32 i_iCount);

    eClient_Status Init_Thread(tXCHAR **i_pArgs, tINT32 i_iCount);

    eClient_Status ParseFormat(const tXCHAR *i_pFormat);

    sFormat       *AddFormatNode(const tXCHAR *i_pPrefix,
                                 size_t        i_szPrefix,
                                 fnFormat      i_pFn);


public:
    eClient_Status Sent(tUINT32            i_dwChannel_ID,
                        sP7C_Data_Chunk   *i_pChunks, 
                        tUINT32            i_dwCount,
                        tUINT32            i_dwSize
                       );

    tBOOL          Get_Info(sP7C_Info *o_pInfo);
    tBOOL          Flush();

private:
    eClient_Status Parse_Buffer(tUINT8 *i_pBuffer, size_t  i_szBuffer);
    eClient_Status Parse_Packet(tUINT32 i_dwChannel, tUINT8 *i_pPacket, size_t  i_szPacket);

    static THSHELL_RET_TYPE THSHELL_CALL_TYPE Static_Routine(void *i_pContext)
    {
        CClText *l_pRoutine = static_cast<CClText *>(i_pContext);
        if (l_pRoutine)
        {
            l_pRoutine->Routine();
        }

        CThShell::Cleanup();
        return THSHELL_RET_OK;
    } 

    void  Routine();

public:
    static void FormatChannel(CClText *i_pClient);
    static void FormatMessageId(CClText *i_pClient);
    static void FormatMessageIndex(CClText *i_pClient);
    static void FormatTimeFull(CClText *i_pClient);
    static void FormatTimeMedium(CClText *i_pClient);
    static void FormatTimeShort(CClText *i_pClient);
    static void FormatTimeDiff(CClText *i_pClient);
    static void FormatTimeCount(CClText *i_pClient);
    static void FormatLevel(CClText *i_pClient);
    static void FormatThreadId(CClText *i_pClient);
    static void FormatThreadName(CClText *i_pClient);
    static void FormatCpuCore(CClText *i_pClient);
    static void FormatModuleId(CClText *i_pClient);
    static void FormatModuleName(CClText *i_pClient);
    static void FormatFilePath(CClText *i_pClient);
    static void FormatFileName(CClText *i_pClient);
    static void FormatFileLine(CClText *i_pClient);
    static void FormatFunction(CClText *i_pClient);
    static void FormatMsg(CClText *i_pClient);
};


#endif //CLTEXT_H
