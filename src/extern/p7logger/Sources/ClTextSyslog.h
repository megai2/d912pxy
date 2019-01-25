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
// This header file provide printing to console                                /
////////////////////////////////////////////////////////////////////////////////

#ifndef CLTEXTSYSLOG_H
#define CLTEXTSYSLOG_H

#define TXT_SYSLOG_DEFAULT_PORT                                        TM("514")

//RFC:
//https://tools.ietf.org/html/rfc5424

////////////////////////////////////////////////////////////////////////////////
class CClTextSyslog
    : public CClTextSink
{
protected:
    tBOOL        m_bIsWinsock;
    CUDP_Socket *m_pSocket;
    tUINT32      m_pSeverity[EP7TRACE_LEVEL_COUNT];
    tUINT32      m_dwFacility;
    tACHAR       m_pHostName[96];
    tACHAR       m_pProcName[96];
    //minimal size of UDP packet is 576 bytes minus header and extra = 512
    //to drive the size of the buffer please use CLIENT_COMMAND_PACKET_BAICAL_SIZE
    //parameter, for example /P7.PSize=1478
    //Message size RFC: https://tools.ietf.org/html/rfc5426#page-3
    tACHAR      *m_pBuffer; 
    size_t       m_szBuffer;
    tUINT32      m_dwProcId;

public:
    ////////////////////////////////////////////////////////////////////////////
    //CClTextSyslog
    CClTextSyslog()
        : m_bIsWinsock(FALSE)
        , m_pSocket(NULL)
        , m_dwFacility(1)
        , m_pBuffer(0)
        , m_szBuffer(512)
        , m_dwProcId(0)
    {
        CSys::Get_Host_Name(m_pHostName, LENGTH(m_pHostName));
        CProc::Get_Process_Name(m_pProcName, LENGTH(m_pProcName));
        m_dwProcId = CProc::Get_Process_ID();

        tACHAR *l_pIter = m_pHostName;
        while (*l_pIter)
        {
            if (' ' == (*l_pIter))
            {
                *l_pIter = '_';
            }
            l_pIter++;
        }

        l_pIter = m_pProcName;
        while (*l_pIter)
        {
            if (' ' == (*l_pIter))
            {
                *l_pIter = '_';
            }
            l_pIter++;
        }

        //Mapping P7 levels to syslog severity
        //https://tools.ietf.org/html/rfc3164#page-8
        m_pSeverity[EP7TRACE_LEVEL_TRACE]    = 7; //Debug: debug-level messages
        m_pSeverity[EP7TRACE_LEVEL_DEBUG]    = 7; //Debug: debug-level messages
        m_pSeverity[EP7TRACE_LEVEL_INFO]     = 6; //Informational: informational messages
        m_pSeverity[EP7TRACE_LEVEL_WARNING]  = 4; //Warning: warning conditions
        m_pSeverity[EP7TRACE_LEVEL_ERROR]    = 3; //Error: error conditions
        m_pSeverity[EP7TRACE_LEVEL_CRITICAL] = 2; //Critical: critical conditions
    }//CClTextSyslog

    ////////////////////////////////////////////////////////////////////////////
    //~CClTextSyslog
    virtual ~CClTextSyslog()
    {
        if (m_bIsWinsock)
        {
            WSA_UnInit();
            m_bIsWinsock = FALSE;
        }

        if (m_pSocket)
        {
            delete m_pSocket;
            m_pSocket = NULL;
        }

        if (m_pBuffer)
        {
            free(m_pBuffer);
            m_pBuffer = NULL;
        }
    }//~CClTextSyslog

    ////////////////////////////////////////////////////////////////////////////
    //Initialize
    virtual eClient_Status Initialize(tXCHAR **i_pArgs, tINT32 i_iCount)
    {
        eClient_Status l_eReturn = ECLIENT_STATUS_OK; 

        if (FALSE == WSA_Init())
        {
            PRINTF(TM("Windows Socket initialization fails!"));
            l_eReturn = ECLIENT_STATUS_INTERNAL_ERROR;
        }
        else
        {
            m_bIsWinsock = TRUE;
        }

        if (ECLIENT_STATUS_OK == l_eReturn)
        {
            tXCHAR     *l_pAddr = NULL;
            tXCHAR     *l_pPort = NULL;
            tADDR_INFO *l_pInfo = NULL;
            tADDR_INFO *l_pNext = NULL;
            tADDR_INFO  l_tHint;

            memset(&l_tHint, 0, sizeof(l_tHint));

            l_pAddr = Get_Argument_Text_Value(i_pArgs, i_iCount,
                                              (tXCHAR*)CLIENT_COMMAND_LINE_BAICAL_ADDRESS);
            if (NULL == l_pAddr)
            {
                l_pAddr = (tXCHAR*)TM("127.0.0.1");
            }

            l_pPort = Get_Argument_Text_Value(i_pArgs, i_iCount,
                                              (tXCHAR*)CLIENT_COMMAND_LINE_BAICAL_PORT);

            if (NULL == l_pPort)
            {
                l_pPort = (tXCHAR*)TXT_SYSLOG_DEFAULT_PORT;
            }

            l_tHint.ai_family   = AF_UNSPEC; //AF_INET;
            l_tHint.ai_socktype = SOCK_DGRAM;
            l_tHint.ai_protocol = IPPROTO_UDP;
        
            l_eReturn = ECLIENT_STATUS_INTERNAL_ERROR;

            //http://msdn.microsoft.com/en-us/library/windows/desktop/ms738520(v=vs.85).aspx
            //http://linux.die.net/man/3/getaddrinfo
            if (0 == GET_ADDR_INFO(l_pAddr, l_pPort, &l_tHint, &l_pInfo))
            {
                 for(l_pNext = l_pInfo; l_pNext != NULL ;l_pNext=l_pNext->ai_next) 
                 {
                     if (    (    (AF_INET  == l_pNext->ai_family)
                               || (AF_INET6 == l_pNext->ai_family)
                             )
                          && (SOCK_DGRAM  == l_pNext->ai_socktype)
                          && (IPPROTO_UDP == l_pNext->ai_protocol)
                        )
                     {
                         m_pSocket = new CUDP_Socket(NULL, 
                                                    (sockaddr*) l_pNext->ai_addr,
                                                    FALSE);
                         if (m_pSocket)
                         {
                             if (m_pSocket->Initialized())
                             {
                                 l_eReturn = ECLIENT_STATUS_OK;
                                 break;
                             }
                             else
                             {
                                 delete m_pSocket;
                                 m_pSocket = NULL;
                             }
                         }
                     }
                 }
            } //if (0 == GetAddrInfoW

            if (l_pInfo)
            {
                FREE_ADDR_INFO(l_pInfo);
                l_pInfo = NULL;
            }
        }

        if (ECLIENT_STATUS_OK == l_eReturn)
        {
            tXCHAR *l_pValue = Get_Argument_Text_Value(i_pArgs, i_iCount, 
                                                       (tXCHAR*)CLIENT_COMMAND_SYSLOG_FACILITY);
            if (l_pValue)
            {
                m_dwFacility = (tUINT32)PStrToInt(l_pValue);
            }

            l_pValue = Get_Argument_Text_Value(i_pArgs, i_iCount, 
                                               (tXCHAR*)CLIENT_COMMAND_PACKET_BAICAL_SIZE);
            if (l_pValue)
            {
                m_szBuffer = (tUINT32)PStrToInt(l_pValue);
            }

            if (512 > m_szBuffer)
            {
                m_szBuffer = 512;
            }

            if (m_szBuffer > 65512)
            {
                m_szBuffer = 65512;
            }

            m_pBuffer = (tACHAR*)malloc(m_szBuffer);
            if (!m_pBuffer)
            {
                PRINTF(TM("Memory allocation fails!"));
                l_eReturn = ECLIENT_STATUS_INTERNAL_ERROR;
            }
        }

        return l_eReturn;
    }//Initialize

    ////////////////////////////////////////////////////////////////////////////
    //Log
    virtual eClient_Status Log(const CClTextSink::sLog &i_rRawLog, 
                               const tXCHAR            *i_pFmtLog, 
                               size_t                   i_szFmtLog
                              )
    {
        tINT32 l_iLen = PSPrint(m_pBuffer, 
                                m_szBuffer,
                                "<%d>1 %04d-%02d-%02dT%02d:%02d:%02d.%03dZ %s %s %d - - ", 
                                m_dwFacility * 8 + m_pSeverity[i_rRawLog.eLevel],
                                i_rRawLog.dwYear,
                                i_rRawLog.dwMonth,
                                i_rRawLog.dwDay,
                                i_rRawLog.dwHour,
                                i_rRawLog.dwMinutes,
                                i_rRawLog.dwSeconds,
                                i_rRawLog.dwMilliseconds,
                                m_pHostName, 
                                m_pProcName, 
                                m_dwProcId
                               );
        if (0 < l_iLen)
        {
        #ifdef UTF8_ENCODING
            if ((m_szBuffer - l_iLen) <= i_szFmtLog)
            {
                i_szFmtLog = m_szBuffer - l_iLen - 1; //overflow isn't possible
            }

            memcpy(m_pBuffer + l_iLen, i_pFmtLog, i_szFmtLog);
            l_iLen += (tINT32)i_szFmtLog;
            m_pBuffer[l_iLen] = 0;
        #else
            tINT32 l_iTmp = Convert_UTF16_To_UTF8(i_pFmtLog, 
                                                  m_pBuffer + l_iLen, 
                                                  (tUINT32)(m_szBuffer - l_iLen)
                                                 );
            if (0 < l_iTmp)
            {
                l_iLen += l_iTmp;
            }
        #endif                             

            m_pSocket->Send(m_pBuffer, l_iLen + 1);
        }

        return ECLIENT_STATUS_OK;
    }//Log
};

#endif //CLTEXTSYSLOG_H
