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
// This header file provide functionality to deliver data to baical server     /
// over UDP                                                                    /
////////////////////////////////////////////////////////////////////////////////
       
#include "CommonClient.h"
#include "ClBaical.h"


#define COMMUNICATION_THREAD_IDLE_TIMEOUT                                 (5)
#define COMMUNICATION_THREAD_EXIT_TIMEOUT                                 (5000)
#define COMMUNICATION_RESPONSE_TIMEOUT                                    (100)
#define COMMUNICATION_DATA_SEGMENT_MAX_DURATION                           (750)
#define COMMUNICATION_IDLE_TIMEOUT                                        (1000)
#define COMMUNICATION_LOSSES_MAX                                          (60) //percents

#define COMMUNICATION_MAX_DELIVERY_FAILS                                  (10)
#define SOCKET_RECEIVE_RESPONSE_TIMEOUT                                   (10) 

#define  CONNECTION_SIGNAL                                 (MEVENT_SIGNAL_0)

#define  THREAD_EXIT_SIGNAL                                (MEVENT_SIGNAL_0)
#define  THREAD_DATA_SIGNAL                                (MEVENT_SIGNAL_0 + 1)


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
//CClBaical
CClBaical::CClBaical(tXCHAR **i_pArgs, tINT32   i_iCount)
    : CClient(IP7_Client::eBaical, i_pArgs, i_iCount)
    , m_lReject_Mem(0)
    , m_lReject_Con(0)
    , m_lReject_Int(0)

    , m_bIs_Winsock(FALSE)
    , m_pSocket(NULL)
    , m_wClient_ID(0xFFFF)
    , m_pBPool(NULL)

    , m_pData_Wnd(NULL)
    , m_pData_Wnd_Cell(NULL)
    , m_bData_Wnd_Fixed(FALSE)
    , m_dwData_Wnd_Max_Count(0)
    , m_dwData_Wnd_Size(0)
    , m_dwData_Wnd_TimeStamp(0)
    , m_dwData_Wnd_First_ID(TPACKET_MAX_ID + 1)
    , m_dwData_Wnd_Last_ID(TPACKET_MAX_ID + 1)
    , m_bData_Resending(FALSE)

    , m_dwDelivery_Fails(0)
    , m_bIs_Response_Waiting(FALSE)

    , m_pData_Queue_Out(NULL)
    , m_pData_Queue_In(NULL)

    , m_dwLast_Packet_ID(0)
    , m_dwServiceTimeStamp(0)

    , m_dwExit_TimeOut(COMMUNICATION_THREAD_EXIT_TIMEOUT)

    , m_pPacket_Control(NULL)
    , m_bBig_Endian(FALSE)

    , m_bConn_Event(TRUE)
    , m_bComm_Thread(FALSE)
    , m_hComm_Thread(0) //NULL
    , m_bChnl_Thread(FALSE)
    , m_hChnl_Thread(0) //NULL

    , m_bLocalHost(FALSE)
    , m_bExtension(FALSE)

    , m_pHdrUsrData(NULL)
    , m_pHdrUsrPacket(NULL)
    , m_uHdrUsrSize(0)
    , m_uHdrUsrChannel(USER_PACKET_CHANNEL_ID_MAX_SIZE)
{
    memset(&m_hCS_Data_Out, 0, sizeof(m_hCS_Data_Out));
    memset(&m_hCS_Data_In,  0, sizeof(m_hCS_Data_In));
    
    //1. base init
    if (ECLIENT_STATUS_OK == m_eStatus)
    {
        m_eStatus = Init_Base(i_pArgs, i_iCount);
    }

    //2. initialize network layer and sockets
    if (ECLIENT_STATUS_OK == m_eStatus)
    {
        m_eStatus = Init_Sockets(i_pArgs, i_iCount);
    }

    //3. Initialize Pool
    if (ECLIENT_STATUS_OK == m_eStatus)
    {
        m_eStatus = Init_Pool(i_pArgs, i_iCount);
    }
   
    //4. Initialize variables
    if (ECLIENT_STATUS_OK == m_eStatus)
    {
        m_eStatus = Init_Members(i_pArgs, i_iCount);
    }

    //5. Initialize crash handler
    if (ECLIENT_STATUS_OK == m_eStatus)
    {
        Init_Crash_Handler(i_pArgs, i_iCount);
    }

}//CClBaical


////////////////////////////////////////////////////////////////////////////////
//~CClBaical
CClBaical::~CClBaical()
{
    Uninit_Crash_Handler();

    Flush();

    CClient::Unshare();

    if (m_pData_Queue_Out)
    {
        pAList_Cell l_pElement = NULL;

        while ((l_pElement = m_pData_Queue_Out->Get_First()))
        {
            CTPacket *l_pPacket = m_pData_Queue_Out->Get_Data(l_pElement);
            m_pData_Queue_Out->Del(l_pElement, FALSE);
            m_pBPool->Push_Buffer(l_pPacket);
        }

        m_pData_Queue_Out->Clear(TRUE);
        delete m_pData_Queue_Out;
        m_pData_Queue_Out = NULL;
    }


    if (m_pData_Queue_In)
    {
        pAList_Cell l_pElement = NULL;

        while ((l_pElement = m_pData_Queue_In->Get_First()))
        {
            CTPacket *l_pPacket = m_pData_Queue_In->Get_Data(l_pElement);
            m_pData_Queue_In->Del(l_pElement, FALSE);
            m_pBPool->Push_Buffer(l_pPacket);
        }

        m_pData_Queue_In->Clear(TRUE);
        delete m_pData_Queue_In;
        m_pData_Queue_In = NULL;
    }


    if (m_pData_Wnd)
    {
        pAList_Cell l_pElement = NULL;

        while ((l_pElement = m_pData_Wnd->Get_First()))
        {
            CTPacket *l_pPacket = m_pData_Wnd->Get_Data(l_pElement);
            m_pData_Wnd->Del(l_pElement, FALSE);
            m_pBPool->Push_Buffer(l_pPacket);
        }

        m_pData_Wnd->Clear(TRUE);
        delete m_pData_Wnd;
        m_pData_Wnd = NULL;
    }

    //stop sockets
    if (m_bIs_Winsock)
    {
        WSA_UnInit();
    }

    if (m_pSocket)
    {
        delete m_pSocket;
        m_pSocket = NULL;
    }

    //clear pool
    if (m_pBPool)
    {
        delete m_pBPool;
        m_pBPool = NULL;
    }
    
    LOCK_DESTROY(m_hCS_Data_Out);
    LOCK_DESTROY(m_hCS_Data_In);
} //~CClBaical

////////////////////////////////////////////////////////////////////////////////
//Init_Base
eClient_Status CClBaical::Init_Base(tXCHAR **i_pArgs,
                                    tINT32   i_iCount
                                   )
{
    eClient_Status  l_eReturn = ECLIENT_STATUS_OK;
    UNUSED_ARG(i_pArgs);
    UNUSED_ARG(i_iCount);

    return l_eReturn;
}//Init_Base


////////////////////////////////////////////////////////////////////////////////
//Init_Sockets
eClient_Status CClBaical::Init_Sockets(tXCHAR **i_pArgs,
                                       tINT32   i_iCount
                                      )
{
    eClient_Status l_eReturn = ECLIENT_STATUS_OK;

    if (FALSE == WSA_Init())
    {
        JOURNAL_CRITICAL(m_pLog, 
                         TM("Windows Socket initialization fail !"));

        l_eReturn = ECLIENT_STATUS_INTERNAL_ERROR;
    }
    else
    {
        m_bIs_Winsock = TRUE;
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
            l_pPort = (tXCHAR*)TM("9009");
        }


        if (    (0 == PStrICmp(l_pAddr, TM("127.0.0.1")))
             || (0 == PStrICmp(l_pAddr, TM("::1")))
           )
        {
            m_bLocalHost = TRUE;
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
                     m_pSocket = new CUDP_Socket(m_pLog, 
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
    }//if (ECLIENT_STATUS_OK == l_eReturn)

    return l_eReturn;
}//Init_Sockets


////////////////////////////////////////////////////////////////////////////////
//Init_Pool
eClient_Status CClBaical::Init_Pool(tXCHAR **i_pArgs,
                                    tINT32   i_iCount
                                   )
{
    tXCHAR  *l_pArg_Value    = NULL;
    tUINT32  l_dwMax_Memory  = 0x100000; //1mb by default
    tUINT32  l_dwPacket_Size = TPACKET_DEF_SIZE;

    ////////////////////////////////////////////////////////////////////////////
    //packet size
    l_pArg_Value = Get_Argument_Text_Value(i_pArgs, i_iCount, 
                                           (tXCHAR*)CLIENT_COMMAND_PACKET_BAICAL_SIZE);
    if (l_pArg_Value)
    {
        l_dwPacket_Size = (tUINT32)PStrToInt(l_pArg_Value);

        if (TPACKET_MIN_SIZE > l_dwPacket_Size)
        {
            l_dwPacket_Size = TPACKET_MIN_SIZE;
        }
        else if (TPACKET_MAX_SIZE < l_dwPacket_Size)
        {
            l_dwPacket_Size = TPACKET_MAX_SIZE;
        }
    }

    if (m_bLocalHost)
    {
        l_dwPacket_Size = TPACKET_MAX_SIZE;
    }

    ////////////////////////////////////////////////////////////////////////////
    //pool size
    l_pArg_Value = Get_Argument_Text_Value(i_pArgs, i_iCount,
                                           (tXCHAR*)CLIENT_COMMAND_POOL_SIZE);
    if (l_pArg_Value)
    {
        l_dwMax_Memory = 1024 * (tUINT32)PStrToInt(l_pArg_Value);

        if (16384 > l_dwMax_Memory)
        {
            l_dwMax_Memory = 16384;
        }
    }

    //check that memory is enough to store 20 packets
    if (l_dwMax_Memory < (l_dwPacket_Size * 20))
    {
        l_dwMax_Memory = l_dwPacket_Size * 20;

        JOURNAL_WARNING(m_pLog, 
                        TM("Pool size is not enought to store 20 packets, enlarge up to %d"),
                        l_dwMax_Memory
                        );
    }


    ////////////////////////////////////////////////////////////////////////////
    //window size

    //size of the transmission window in packets. Sometimes is useful to manage it
    //if server aggressively loose incoming packets
    //Min = 1
    //max = ((pool size / packet size) / 2)

    //here is possible maximal window length
    tUINT32 l_uWnd_Max_Total = (SERVER_REPORT_SIZE - SERVER_REPORT_HEADER_SIZE) * 8;
    //if window size is more than half of all memory we resize the window length
    tUINT32 l_uWnd_Max_Mem   = (l_dwMax_Memory / l_dwPacket_Size) / 2;
    //if window size is more than socket buffer size, truncate to it
    tUINT32 l_uWnd_Max_Net   = (m_pSocket->GetSendBufferSize() / l_dwPacket_Size) - 1;
    tUINT32 l_uWnd_Max_Usr   = l_uWnd_Max_Total;

   
    l_pArg_Value = Get_Argument_Text_Value(i_pArgs, i_iCount,
                                           (tXCHAR*)CLIENT_COMMAND_WINDOW_BAICAL_SIZE);
    if (l_pArg_Value)
    {
        l_uWnd_Max_Usr = (tUINT32)PStrToInt(l_pArg_Value);

        if (1 > l_uWnd_Max_Usr)
        {
            l_uWnd_Max_Usr = 1;
        }
        
        m_bData_Wnd_Fixed = TRUE;
    }

    //select minimum window size
    m_dwData_Wnd_Max_Count = l_uWnd_Max_Total;
    if (l_uWnd_Max_Mem < m_dwData_Wnd_Max_Count) m_dwData_Wnd_Max_Count = l_uWnd_Max_Mem;
    if (l_uWnd_Max_Net < m_dwData_Wnd_Max_Count) m_dwData_Wnd_Max_Count = l_uWnd_Max_Net;
    if (l_uWnd_Max_Usr < m_dwData_Wnd_Max_Count) m_dwData_Wnd_Max_Count = l_uWnd_Max_Usr;


    
    ////////////////////////////////////////////////////////////////////////////
    //initialize pool
    m_pBPool = new CBuffers_Pool(m_pLog, 
                                 l_dwMax_Memory / 10,   //10% 
                                 l_dwMax_Memory,        //100%
                                 l_dwPacket_Size, 
                                 0
                                );

    if (    (NULL  != m_pBPool)
         && (FALSE == m_pBPool->Get_Initialized())
       )
    {
        JOURNAL_CRITICAL(m_pLog, TM("Pool initialization failed"));

        delete m_pBPool;
        m_pBPool = NULL;
    }

    JOURNAL_INFO(m_pLog, 
                 TM("Pool: Max memory = %d, Packet size = %d, Window length = %d"),
                 l_dwMax_Memory,
                 l_dwPacket_Size,
                 m_dwData_Wnd_Max_Count
                );

    return (NULL != m_pBPool) ? ECLIENT_STATUS_OK : ECLIENT_STATUS_INTERNAL_ERROR;
}//Init_Pool


////////////////////////////////////////////////////////////////////////////////
//Init_Members
eClient_Status CClBaical::Init_Members(tXCHAR **i_pArgs,
                                       tINT32   i_iCount
                                      )
{
    eClient_Status l_eReturn    = ECLIENT_STATUS_OK;
    tXCHAR        *l_pArg_Value = NULL;

    if (ECLIENT_STATUS_OK == l_eReturn)
    {
        LOCK_CREATE(m_hCS_Data_Out);
        LOCK_CREATE(m_hCS_Data_In);
    }

    if (ECLIENT_STATUS_OK == l_eReturn)
    {
        m_pData_Queue_Out = new CBList<CTPacket*>();
        if (NULL == m_pData_Queue_Out)
        {
            l_eReturn = ECLIENT_STATUS_INTERNAL_ERROR;
            JOURNAL_CRITICAL(m_pLog, TM("Data queue [out] initialization failed"));
        }
    }


    if (ECLIENT_STATUS_OK == l_eReturn)
    {
        m_pData_Queue_In = new CBList<CTPacket*>();
        if (NULL == m_pData_Queue_In)
        {
            l_eReturn = ECLIENT_STATUS_INTERNAL_ERROR;
            JOURNAL_CRITICAL(m_pLog, TM("Data queue [in] initialization failed"));
        }
    }

    if (ECLIENT_STATUS_OK == l_eReturn)
    {
        m_pData_Wnd = new CBList<CTPacket*>();
        if (NULL == m_pData_Wnd)
        {
            l_eReturn = ECLIENT_STATUS_INTERNAL_ERROR;
            JOURNAL_CRITICAL(m_pLog, TM("Data lagoon initialization failed"));
        }
    }

    if (ECLIENT_STATUS_OK == l_eReturn)
    {
        tUINT32 l_dwHTime  = 0;
        tUINT32 l_dwLTime  = 0;
        tWCHAR  l_pProc_Name[TPACKET_PROCESS_NAME_MAX_LEN];
        tUINT32 l_dwEndian = 0x1;
        tUINT8  l_bLittleE = *(tUINT8*)&l_dwEndian;
        
        l_pProc_Name[0] = 0;
        
        CProc::Get_Process_Time(&l_dwHTime, &l_dwLTime);

        l_pArg_Value = Get_Argument_Text_Value(i_pArgs, i_iCount,
                                               (tXCHAR*)CLIENT_COMMAND_LINE_NAME);
        if (!l_pArg_Value)
        {
            CProc::Get_Process_Name(l_pProc_Name, LENGTH(l_pProc_Name));
        }
        else
        {
        #ifdef UTF8_ENCODING
            Convert_UTF8_To_UTF16((const char*)l_pArg_Value, 
                                  l_pProc_Name, 
                                  LENGTH(l_pProc_Name)
                                 );
        #else
            PStrCpy((tXCHAR*)l_pProc_Name, LENGTH(l_pProc_Name), l_pArg_Value);
        #endif                             
        }

        m_cPacket_Hello.Fill(CLIENT_PROTOCOL_VERSION,
                             (tUINT16)m_pBPool->Get_Buffer_Size(),
                             CProc::Get_Process_ID(),
                             l_dwHTime,
                             l_dwLTime,
                             l_pProc_Name
                            );

        Inc_Packet_ID(&m_dwLast_Packet_ID);

        if (!l_bLittleE)
        {
            m_bBig_Endian = TRUE;
            m_cPacket_Hello.Set_Flag(TPACKET_FLAG_BIG_ENDIAN_CLN);
            m_cPacket_Data_Report.Set_Flag(TPACKET_FLAG_BIG_ENDIAN_CLN);
            m_cPacket_Alive.Set_Flag(TPACKET_FLAG_BIG_ENDIAN_CLN);
            m_cPacket_Bye.Set_Flag(TPACKET_FLAG_BIG_ENDIAN_CLN);
        }

        m_cPacket_Hello.Set_Flag(TPACKET_FLAG_EXTENSION);

    #if defined(GTX64)
        m_cPacket_Hello.Set_Flag(TPACKET_FLAG_ARCH_64);
    #endif

        m_cPacket_Hello.Finalize(m_dwLast_Packet_ID, m_wClient_ID);

        m_pPacket_Control = static_cast<CTPacket*>(&m_cPacket_Hello);
    }

    if (ECLIENT_STATUS_OK == l_eReturn)
    {
        l_pArg_Value = Get_Argument_Text_Value(i_pArgs, i_iCount,
                                               (tXCHAR*)CLIENT_COMMAND_LINE_BAICAL_EXIT_TIMEOUT);
        if (l_pArg_Value)
        {
            m_dwExit_TimeOut = PStrToInt(l_pArg_Value);
            if (15 < m_dwExit_TimeOut)
            {
                m_dwExit_TimeOut = 15;
            }

            m_dwExit_TimeOut *= 1000;
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    //threads initialization
    
    if (ECLIENT_STATUS_OK == l_eReturn)
    {
        if (FALSE == m_cConn_Event.Init(1, EMEVENT_SINGLE_MANUAL))
        {
            l_eReturn = ECLIENT_STATUS_INTERNAL_ERROR;
            JOURNAL_ERROR(m_pLog, TM("Connection event wasn't created !"));
        }
    }

    if (ECLIENT_STATUS_OK == l_eReturn)
    {
        if (FALSE == m_cComm_Event.Init(1, EMEVENT_SINGLE_AUTO))
        {
            l_eReturn = ECLIENT_STATUS_INTERNAL_ERROR;
            JOURNAL_ERROR(m_pLog, TM("Exit event wasn't created !"));
        }
    }

    if (ECLIENT_STATUS_OK == l_eReturn)
    {
        if (FALSE == CThShell::Create(&Static_Comm_Routine,
                                      this,
                                      &m_hComm_Thread,
                                      TM("P7:Comm") 
                                     )
           )
        {
            l_eReturn      = ECLIENT_STATUS_INTERNAL_ERROR;
            JOURNAL_ERROR(m_pLog, TM("Communication thread wasn't created !"));
        }
        else
        {
            m_bComm_Thread = TRUE;
        }
    }

    if (ECLIENT_STATUS_OK == l_eReturn)
    {
        if (FALSE == m_cChnl_Event.Init(2, EMEVENT_SINGLE_AUTO, EMEVENT_MULTI))
        {
            l_eReturn = ECLIENT_STATUS_INTERNAL_ERROR;
            JOURNAL_ERROR(m_pLog, TM("Exit event wasn't created !"));
        }
    }

    if (ECLIENT_STATUS_OK == l_eReturn)
    {
        if (FALSE == CThShell::Create(&Static_Chnl_Routine, 
                                      this,
                                      &m_hChnl_Thread,
                                      TM("P7:Channel")               
                                     )
           )
        {
            l_eReturn      = ECLIENT_STATUS_INTERNAL_ERROR;
            JOURNAL_ERROR(m_pLog, TM("Channel thread wasn't created !"));
        }
        else
        {
            m_bChnl_Thread = TRUE;
        }
    }


    return l_eReturn;
}//Init_Members


////////////////////////////////////////////////////////////////////////////////
//Inc_Packet_ID
void CClBaical::Inc_Packet_ID(tUINT32 * o_pPacketID)
{
    ++(*o_pPacketID);
    if ( TPACKET_MAX_ID < (*o_pPacketID) )
    {
        (*o_pPacketID) = 1;
    }
}//Inc_Packet_ID


////////////////////////////////////////////////////////////////////////////////
//Process_Incoming_Packet
tBOOL CClBaical::Process_Incoming_Packet(CTPacket *i_pPacket)
{
    tBOOL l_bResult = TRUE;

    if (NULL == i_pPacket)
    {
        l_bResult = FALSE;
    }

    //check packet integrity
    if (    (l_bResult) 
         && (TRUE == i_pPacket->Is_Damaged())
       )
    {
        JOURNAL_ERROR(m_pLog,
                      TM("Packet is corrupted (CRC32) Packet ID=%d, Size=%d, Type=%d, CRC=0x%X"), 
                      i_pPacket->Get_ID(),
                      i_pPacket->Get_Size(),
                      i_pPacket->Get_Type(),
                      i_pPacket->Get_Crc()
                     );

        m_bIs_Response_Waiting = FALSE;
        m_dwDelivery_Fails ++;
        l_bResult = FALSE;
    }

    if (FALSE == m_bIs_Response_Waiting)
    {
        l_bResult = FALSE;
        JOURNAL_WARNING(m_pLog,
                        TM("Drop packet, ID = %d"), 
                        i_pPacket->Get_ID()
                       );
    }

    if (l_bResult)
    {
        m_wClient_ID = i_pPacket->Get_Client_ID();
    }

    if (l_bResult)
    {
        eTPacket_Type l_eType = i_pPacket->Get_Type();
        if (ETPT_ACKNOWLEDGMENT == l_eType) 
        {
            CTPAcknowledgment l_cServerResponse(i_pPacket);

            //verify that response source ID is equal to control ID
            if (    (m_pPacket_Control) 
                 && (l_cServerResponse.Get_Source_ID() == m_pPacket_Control->Get_ID()) 
               )
            {
                m_bIs_Response_Waiting = FALSE;
                m_pData_Wnd_Cell       = NULL;
                m_dwDelivery_Fails     = 0;

                if (l_cServerResponse.Get_Result())
                {
                    if (ETPT_CLIENT_HELLO == m_pPacket_Control->Get_Type())
                    {
                        LOCK_ENTER(m_hCS_Data_In);
                        if (TPACKET_FLAG_EXTENSION & i_pPacket->Get_Flags())
                        {
                            m_bExtension = TRUE;
                        }
                        else
                        {
                            m_bExtension = FALSE;
                        }
                        LOCK_EXIT(m_hCS_Data_In);
                    }

                    //if this is response to data report packet ... 
                    if (ETPT_CLIENT_DATA_REPORT == m_pPacket_Control->Get_Type())
                    {
                        pAList_Cell l_pElement = NULL;
                        while ((l_pElement = m_pData_Wnd->Get_First()) != NULL)
                        {
                            CTPacket * l_pPacket = m_pData_Wnd->Get_Data(l_pElement);
                            m_pData_Wnd->Del(l_pElement, FALSE);
                            m_pBPool->Push_Buffer(l_pPacket);
                        }

                        m_dwData_Wnd_First_ID = TPACKET_MAX_ID + 1;
                        m_dwData_Wnd_Last_ID  = TPACKET_MAX_ID + 1;
                        m_dwData_Wnd_Size     = 0;
                        m_bData_Resending     = FALSE;
                    }
                    else if (m_bConn_Event)
                    {
                        m_bConn_Event = FALSE;
                        m_cConn_Event.Set(CONNECTION_SIGNAL);
                    }

                    //control packet is local class packet
                    //it is not necessary to return it to pool
                    m_pPacket_Control = NULL; 
                }
                else //server set this flag to 0 when it not recognize this client
                {
                    Reset_Connetion();
                }

                if (TPACKET_FLAG_EXTENSION & i_pPacket->Get_Flags())
                {
                    Parse_Extensions(l_cServerResponse.Get_Buffer() + ACKNOWLEDGMENT_SIZE, 
                                     (size_t)l_cServerResponse.Get_Size() - ACKNOWLEDGMENT_SIZE
                                    );
                }

                if (TPACKET_FLAG_EXTRA_DATA & i_pPacket->Get_Flags())
                {
                    LOCK_ENTER(m_hCS_Data_In); 
                    m_pData_Queue_In->Add_After(m_pData_Queue_In->Get_Last(), i_pPacket);
                    LOCK_EXIT(m_hCS_Data_In); 

                    m_cChnl_Event.Set(THREAD_DATA_SIGNAL);

                    i_pPacket = NULL;
                }
            }
        }
        else if (ETPT_SERVER_DATA_REPORT == l_eType)
        {
            CTPData_Window_Report l_cServerReport(i_pPacket);

            if (l_cServerReport.Get_Source_ID() == m_pPacket_Control->Get_ID() )
            {
                tUINT32 l_dwTotal      = m_pData_Wnd->Count();
                m_bIs_Response_Waiting = FALSE;
                m_pData_Wnd_Cell       = NULL;
                m_bData_Resending      = TRUE;

                pAList_Cell l_pEl      = m_pData_Wnd->Get_First();
                pAList_Cell l_pEl_Exch = NULL;
                while (l_pEl)
                {
                    CTPacket *l_pPacket = m_pData_Wnd->Get_Data(l_pEl);

                    l_pEl_Exch = m_pData_Wnd->Get_Next(l_pEl);

                    if (FALSE == l_cServerReport.Is_ID(m_cPacket_Data_Report.Get_First_ID(), 
                                                       l_pPacket->Get_ID()
                                                      )
                       )
                    {
                        m_pData_Wnd->Del(l_pEl, FALSE);
                        m_pBPool->Push_Buffer(l_pPacket);
                    }

                    l_pEl = l_pEl_Exch;
                }

                if (m_pData_Wnd->Count())
                {
                    JOURNAL_ERROR(m_pLog,
                                  TM("Server report: count of lost packets: %d, total: %d"), 
                                  m_pData_Wnd->Count(),
                                  l_dwTotal
                                 );

                    if (    (!m_bData_Wnd_Fixed)
                         && ((m_pData_Wnd->Count() * 100 / l_dwTotal) >= COMMUNICATION_LOSSES_MAX)
                       )
                    {
                        JOURNAL_ERROR(m_pLog,
                                      TM("Recalculate transmission window due to network losses, new length: %d"), 
                                      m_dwData_Wnd_Max_Count
                                     );
                        m_dwData_Wnd_Max_Count = (l_dwTotal - m_pData_Wnd->Count());
                    }

                }
                else
                {
                    m_bData_Resending = FALSE;
                    JOURNAL_ERROR(m_pLog,
                                  TM("Server report: empty report is received !")
                                 );
                }

                m_pPacket_Control = NULL;
            }
        } // else if (ETPT_SERVER_DATA_REPORT == l_eType)
    }//if (l_bResult)

    if (i_pPacket)
    {
        m_pBPool->Push_Buffer(i_pPacket);
    }

    return l_bResult;
}//Process_Incoming_Packet


////////////////////////////////////////////////////////////////////////////////
//Get_Delivered_Packet
CTPacket * CClBaical::Get_Delivered_Packet()
{
    CTPacket *l_pReturn = NULL;
    tBOOL     l_bFilled = FALSE;

    if (TRUE == m_bIs_Response_Waiting)
    {
        //if timeout is exceed we flush flag of waiting and resend the same control packet 
        if (CTicks::Difference(GetTickCount(), m_dwServiceTimeStamp) >= COMMUNICATION_RESPONSE_TIMEOUT)
        {
            JOURNAL_ERROR(m_pLog,
                          TM("Server Response waiting timeout. Resending ...")
                         );

            m_bIs_Response_Waiting = FALSE;
            m_dwDelivery_Fails ++;
        }
        else
        {
            l_bFilled = TRUE;
        }
    }

    //Do we have control packet
    if (    (FALSE == l_bFilled) 
         && (m_pPacket_Control)
       )
    {
        m_dwServiceTimeStamp   = GetTickCount();
        l_bFilled              = TRUE;
        m_bIs_Response_Waiting = TRUE;
        l_pReturn              = m_pPacket_Control;
    }

    //if we sending data packets and some limits are exceed .... 
    //we send report packet
    if (    (FALSE == l_bFilled)
         && (FALSE == m_bData_Resending) 
         && (m_pData_Wnd->Count()) 
       )
    {
        if (    (CTicks::Difference(GetTickCount(), m_dwData_Wnd_TimeStamp) >= COMMUNICATION_DATA_SEGMENT_MAX_DURATION) 
             || (m_dwData_Wnd_Max_Count <= m_pData_Wnd->Count())  
           )
        {
            l_pReturn = Create_Data_Wnd_Report();
            l_bFilled = TRUE;
        }
    } //if ((FALSE == l_bFilled) && (0 == m_dwDelivery_Fails))


    if (FALSE == l_bFilled)
    {
        if (FALSE == m_bData_Resending)
        {
            l_pReturn = Pull_Firt_Data_Packet();
            if (l_pReturn)
            {
                //if data window is empty - this is first data packet
                //we need to check ID for overflow, to be sure that all ID 
                //inside data window will be sequential
                if ( (0 == m_pData_Wnd->Count()) &&  
                     ((m_dwLast_Packet_ID + m_dwData_Wnd_Max_Count + 128) >= TPACKET_MAX_ID)
                   )
                {
                    m_dwLast_Packet_ID = 1;
                }
                else
                {
                    Inc_Packet_ID(&m_dwLast_Packet_ID);
                }

                //we store time when we start sending data
                if (0 == m_pData_Wnd->Count())
                {
                    m_dwData_Wnd_TimeStamp = GetTickCount();

                    //JOURNAL_DEBUG(m_pLog,
                    //              L"Start sending data");
                }

                if (m_bBig_Endian)
                {
                    l_pReturn->Set_Flag(TPACKET_FLAG_BIG_ENDIAN_CLN);
                }

                l_pReturn->Finalize(m_dwLast_Packet_ID, m_wClient_ID);

                m_dwData_Wnd_Size   += l_pReturn->Get_Size();
                m_pData_Wnd->Add_After(m_pData_Wnd->Get_Last(), l_pReturn);
                m_dwServiceTimeStamp = GetTickCount();
                l_bFilled            = TRUE;
            }
            else if (m_pData_Wnd->Count())
            {
                l_pReturn = Create_Data_Wnd_Report();
                l_bFilled = TRUE;
            }
        }//if (FALSE == m_bData_Resending)
        else if (m_pData_Wnd->Count()) 
        {
             //if redeliver all packets from data window
             if (m_pData_Wnd_Cell == m_pData_Wnd->Get_Last()) 
             {
                 if (    (TPACKET_MAX_ID > m_dwData_Wnd_First_ID) 
                      && (TPACKET_MAX_ID > m_dwData_Wnd_Last_ID)
                    )
                 {
                     l_pReturn = &m_cPacket_Data_Report;
                     m_pPacket_Control = &m_cPacket_Data_Report;

                     Inc_Packet_ID(&m_dwLast_Packet_ID);

                     m_cPacket_Data_Report.Fill(m_dwData_Wnd_First_ID, m_dwData_Wnd_Last_ID);
                     m_cPacket_Data_Report.Finalize(m_dwLast_Packet_ID, m_wClient_ID);

                     m_bIs_Response_Waiting = TRUE;
                     m_dwServiceTimeStamp   = GetTickCount();
                     m_pData_Wnd_Cell       = NULL;
                     l_bFilled              = TRUE;
                 }
             }
             else
             {
                 m_pData_Wnd_Cell     = m_pData_Wnd->Get_Next(m_pData_Wnd_Cell);
                 l_pReturn            = m_pData_Wnd->Get_Data(m_pData_Wnd_Cell);
                 m_dwServiceTimeStamp = GetTickCount();
                 l_bFilled            = TRUE;
             }
        }//else if (m_pData_Wnd->Count()) 

    } //if (FALSE == l_bFilled)


    //nothing to send, let's sent alive packet.
    if (    (FALSE == l_bFilled)
         && (0 == m_pData_Wnd->Count()) 
         && (CTicks::Difference(GetTickCount(), m_dwServiceTimeStamp) >= COMMUNICATION_IDLE_TIMEOUT)
       )
    {
        l_pReturn = &m_cPacket_Alive;
        Inc_Packet_ID(&m_dwLast_Packet_ID);
        l_pReturn->Finalize(m_dwLast_Packet_ID, m_wClient_ID);
        m_bIs_Response_Waiting = TRUE;
        m_dwServiceTimeStamp   = GetTickCount();
        l_bFilled              = TRUE;
        m_pPacket_Control      = l_pReturn;
    }

    return l_pReturn;
}//Get_Delivered_Packet


////////////////////////////////////////////////////////////////////////////////
//Is_Ready_To_Exit
tBOOL CClBaical::Is_Ready_To_Exit()
{
    tBOOL l_bResult = TRUE;
    if (FALSE == m_bConnected)
    {
        return TRUE;
    }

    LOCK_ENTER(m_hCS_Data_Out);

    l_bResult = ( (0 == m_pData_Queue_Out->Count()) && (0 == m_pData_Wnd->Count()) );

    LOCK_EXIT(m_hCS_Data_Out);

    return l_bResult;
}//Is_Ready_To_Exit


////////////////////////////////////////////////////////////////////////////////
//Set_Connected
void CClBaical::Set_Connected(tBOOL i_bConnected)
{
    if (i_bConnected == m_bConnected)
    {
        return;
    }

    ////////////////////////////////////////////////////////////////////////////
    //update internal state
    LOCK_ENTER(m_hCS); 
    m_bConnected  = i_bConnected;

    if (i_bConnected)
    {
        m_dwConnection_Resets ++;
    }
    LOCK_EXIT(m_hCS); 
    //
    ////////////////////////////////////////////////////////////////////////////

    JOURNAL_ERROR(m_pLog,
                  TM("Set connection : %s"), 
                  (m_bConnected) ? TM("ON") : TM("OFF")
                 );
}//Set_Connection_Status


////////////////////////////////////////////////////////////////////////////////
//Reset_Connetion
void CClBaical::Reset_Connetion()
{
    CTPacket * l_pPacket = NULL;

    JOURNAL_WARNING(m_pLog, TM("Reset Connection"));

    if (m_pPacket_Control)
    {
        m_pPacket_Control = NULL;
    }

    //m_dwDelivery_Fails = 0;
    m_bIs_Response_Waiting  = FALSE;
    m_pData_Wnd_Cell        = NULL;
    m_dwData_Wnd_First_ID   = TPACKET_MAX_ID + 1;
    m_dwData_Wnd_Last_ID    = TPACKET_MAX_ID + 1;
    m_dwData_Wnd_Size       = 0;
    m_bData_Resending       = FALSE;

    pAList_Cell l_pElement = NULL;
    while ((l_pElement = m_pData_Wnd->Get_First()) != NULL)
    {
        l_pPacket = m_pData_Wnd->Get_Data(l_pElement);
        m_pData_Wnd->Del(l_pElement, FALSE);
        m_pBPool->Push_Buffer(l_pPacket);
    }

    //shutdown all connections for clients and update reset counter
    Update_Channels_Status(FALSE, m_dwConnection_Resets + 1);

    //protect list against user calls
    LOCK_ENTER(m_hCS);

    while ((l_pPacket = Pull_Firt_Data_Packet()) != NULL)
    {
        m_pBPool->Push_Buffer(l_pPacket);
    }

    m_dwConnection_Resets ++;

    LOCK_EXIT(m_hCS);

    //if connection is established - inform all clients about it
    if (m_bConnected)
    {
        LOCK_ENTER(m_hCS);
        m_dwConnection_Resets ++;
        LOCK_EXIT(m_hCS);
        Update_Channels_Status(m_bConnected, m_dwConnection_Resets);
    }

    Inc_Packet_ID(&m_dwLast_Packet_ID);
    m_cPacket_Hello.Finalize(m_dwLast_Packet_ID, m_wClient_ID);

    m_pPacket_Control = static_cast<CTPacket*>(&m_cPacket_Hello);
}//Reset_Connetion


////////////////////////////////////////////////////////////////////////////////
//RoutineCreate_Data_Wnd_Report
CTPacket * CClBaical::Create_Data_Wnd_Report()
{
    CTPacket *l_pResult = NULL;

    m_dwData_Wnd_First_ID = TPACKET_MAX_ID + 1;
    m_dwData_Wnd_Last_ID = TPACKET_MAX_ID + 1;

    CTPacket *l_pLagoonPacket = m_pData_Wnd->Get_Data(m_pData_Wnd->Get_First());
    if (l_pLagoonPacket)
    {
        m_dwData_Wnd_First_ID = l_pLagoonPacket->Get_ID();
    }

    l_pLagoonPacket = m_pData_Wnd->Get_Data(m_pData_Wnd->Get_Last());
    if (l_pLagoonPacket)
    {
        m_dwData_Wnd_Last_ID = l_pLagoonPacket->Get_ID();
    }

    if (    (TPACKET_MAX_ID > m_dwData_Wnd_First_ID) 
         && (TPACKET_MAX_ID > m_dwData_Wnd_Last_ID)
       )
    {
        Inc_Packet_ID(&m_dwLast_Packet_ID);
        m_cPacket_Data_Report.Fill(m_dwData_Wnd_First_ID, m_dwData_Wnd_Last_ID);
        m_cPacket_Data_Report.Finalize(m_dwLast_Packet_ID, m_wClient_ID);

        l_pResult              = &m_cPacket_Data_Report;
        m_bIs_Response_Waiting = TRUE;
        m_dwServiceTimeStamp   = GetTickCount();
        m_pPacket_Control      = l_pResult;
    }

    return l_pResult;
}//RoutineCreate_Data_Wnd_Report


////////////////////////////////////////////////////////////////////////////////
//Pull_Firt_Data_Packet
CTPacket *CClBaical::Pull_Firt_Data_Packet()
{
    CTPacket * l_pResult = NULL;
    
    LOCK_ENTER(m_hCS_Data_Out); 

    pAList_Cell l_pElement = m_pData_Queue_Out->Get_First();
    if (l_pElement)
    {
        l_pResult = m_pData_Queue_Out->Get_Data(l_pElement);
        m_pData_Queue_Out->Del(l_pElement, FALSE);
    }

    if (m_pHdrUsrPacket == l_pResult)
    {
        m_pHdrUsrPacket  = 0;
        m_pHdrUsrData    = 0;
        m_uHdrUsrSize    = 0;
        m_uHdrUsrChannel = USER_PACKET_CHANNEL_ID_MAX_SIZE;
    }

    LOCK_EXIT(m_hCS_Data_Out); 

    return l_pResult;
}//Pull_Firt_Data_Packet


////////////////////////////////////////////////////////////////////////////////
//Pull_Last_Data_Packet
CTPacket *CClBaical::Reuse_Data_Packet(CTPData &i_rData, tUINT32 i_uChannel_ID, tUINT32 i_uSize)
{
    CTPacket *l_pResult = NULL;
    
    LOCK_ENTER(m_hCS_Data_Out); 

    pAList_Cell l_pElement = m_pData_Queue_Out->Get_Last();
    if (l_pElement)
    {
        l_pResult = m_pData_Queue_Out->Get_Data(l_pElement);
        m_pData_Queue_Out->Del(l_pElement, FALSE);
    }

    if (    (m_pHdrUsrPacket)
         && (m_uHdrUsrChannel == i_uChannel_ID)
         && ((i_uSize + m_uHdrUsrSize) <= CHANNEL_PACKETS_UNION_MAX_SIZE)
       )
    {
        m_uHdrUsrSize += i_uSize;
        sH_User_Raw l_sHeader = { INIT_USER_HEADER(m_uHdrUsrSize, m_uHdrUsrChannel) };
        memcpy(m_pHdrUsrData, &l_sHeader, sizeof(l_sHeader));

        if (l_pResult)
        {
            i_rData.Attach(l_pResult);
            if (0 >= i_rData.Get_Tail_Size())
            {
                i_rData.Detach();
                m_pData_Queue_Out->Add_After(m_pData_Queue_Out->Get_Last(), l_pResult);
                l_pResult = m_pBPool->Pull_Buffer();
                i_rData.Attach(l_pResult);
                i_rData.Initialize();
            }
        }
        else
        {
            l_pResult = m_pBPool->Pull_Buffer();
            i_rData.Attach(l_pResult);
            i_rData.Initialize();
        }
    }
    else
    {
        m_uHdrUsrSize    = i_uSize + (tUINT32)sizeof(sH_User_Raw);
        m_uHdrUsrChannel = i_uChannel_ID;

        sH_User_Raw l_sHeader = { INIT_USER_HEADER(m_uHdrUsrSize, m_uHdrUsrChannel) };

        if (l_pResult)
        {
            i_rData.Attach(l_pResult);
            if (sizeof(l_sHeader) >= i_rData.Get_Tail_Size())
            {
                i_rData.Detach();
                m_pData_Queue_Out->Add_After(m_pData_Queue_Out->Get_Last(), l_pResult);
                l_pResult = m_pBPool->Pull_Buffer();
                i_rData.Attach(l_pResult);
                i_rData.Initialize();
            }
        }
        else
        {
            l_pResult = m_pBPool->Pull_Buffer();
            i_rData.Attach(l_pResult);
            i_rData.Initialize();
        }

        m_pHdrUsrPacket = l_pResult;
        m_pHdrUsrData   = i_rData.Get_Tail();

        memcpy(m_pHdrUsrData, &l_sHeader, sizeof(l_sHeader));
        i_rData.Append_Size((tUINT16)sizeof(l_sHeader));
    }

    LOCK_EXIT(m_hCS_Data_Out); 

    return l_pResult;
}//Pull_Last_Data_Packet


////////////////////////////////////////////////////////////////////////////////
//Push_Last_Data_Packet
tBOOL CClBaical::Push_Last_Data_Packet(CTPacket *i_pPacket)
{
    if (NULL == i_pPacket)
    {
        return FALSE;
    }

    LOCK_ENTER(m_hCS_Data_Out); 

    m_pData_Queue_Out->Add_After(m_pData_Queue_Out->Get_Last(), i_pPacket);

    LOCK_EXIT(m_hCS_Data_Out); 

    return TRUE;
}//Push_Last_Data_Packet

////////////////////////////////////////////////////////////////////////////////
//Parse_Extensions
void CClBaical::Parse_Extensions(tUINT8 *i_pBuffer, size_t i_szBuffer)
{
    while (i_szBuffer > sizeof(sH_Ext))
    {
        sH_Ext *l_pExt = (sH_Ext*)i_pBuffer;

        if (ETPE_SERVER_NETWORK_INFO == l_pExt->wType)
        {
            sH_Ext_Srv_Info *l_pInfo = (sH_Ext_Srv_Info*)(i_pBuffer + sizeof(sH_Ext));
            //for the time being ignore almost all fields, except buffer size on server side:
            if (FALSE == m_bData_Wnd_Fixed)
            {
                //even if it will overflow we will check it in next IF
                tUINT32 l_uiNewWndSize = (l_pInfo->dwSocket_Buffer / m_pBPool->Get_Buffer_Size()) - 1; 

                //we will apply only server wnd value if it is less then local to fit in all limitations for both sides
                if (l_uiNewWndSize < m_dwData_Wnd_Max_Count)
                {
                    m_dwData_Wnd_Max_Count = l_uiNewWndSize;
                }
            }
        }
        else if (ETPE_USER_DATA == l_pExt->wType)
        {
            //skip here
        }
        else
        {
            JOURNAL_ERROR(m_pLog, TM("Unknown extension packet [%d:%d]"), (tUINT32)l_pExt->wType, (tUINT32)l_pExt->wSize);
        }

        i_pBuffer  += l_pExt->wSize;
        i_szBuffer -= l_pExt->wSize;
    }
}


////////////////////////////////////////////////////////////////////////////////
//Parse_User_Data
void CClBaical::Parse_User_Data(tUINT8 *i_pBuffer, size_t i_szBuffer)
{
    tUINT32       l_dwChannelId   = 0;
    tUINT32       l_dwPacketSize  = 0;
    IP7C_Channel *l_pChannel      = NULL;
    tUINT8       *l_pStop         = i_pBuffer + i_szBuffer;

    LOCK_ENTER(m_hCS_Reg);

    while (i_pBuffer < l_pStop)
    {
        l_dwChannelId  = GET_USER_HEADER_CHANNEL_ID(((sH_User_Raw*)i_pBuffer)->dwBits);
        l_dwPacketSize = GET_USER_HEADER_SIZE(((sH_User_Raw*)i_pBuffer)->dwBits);

        if (USER_PACKET_CHANNEL_ID_MAX_SIZE > l_dwChannelId)
        {
            l_pChannel = m_pChannels[l_dwChannelId];
        }

        if (l_pChannel)
        {
            l_pChannel->On_Receive(l_dwChannelId,
                                    i_pBuffer + sizeof(sH_User_Data), 
                                    l_dwPacketSize - sizeof(sH_User_Data),
                                    m_bBig_Endian
                                    );
        }
        else
        {
            JOURNAL_ERROR(m_pLog, 
                            TM("Channel %d is not registered!"), 
                            l_dwChannelId
                            );
        }

        i_pBuffer += l_dwPacketSize;
    }

    LOCK_EXIT(m_hCS_Reg);
}


////////////////////////////////////////////////////////////////////////////////
//Update_Channels_Status
void CClBaical::Update_Channels_Status(tBOOL i_bConnected, tUINT32 i_dwResets)
{
    sP7C_Status l_sStatus = {FALSE, 0};
    l_sStatus.bConnected = i_bConnected;
    l_sStatus.dwResets   = i_dwResets;

    LOCK_ENTER(m_hCS_Reg);
    for (tUINT32 l_dwI = 0; l_dwI < USER_PACKET_CHANNEL_ID_MAX_SIZE; l_dwI++)
    {
        if (m_pChannels[l_dwI])
        {
            m_pChannels[l_dwI]->On_Status(l_dwI, &l_sStatus);
        }
    }
    LOCK_EXIT(m_hCS_Reg);
}//Update_Channels_Status


////////////////////////////////////////////////////////////////////////////////
//Comm_Routine
void CClBaical::Comm_Routine()
{
    tUINT32           l_dwExit_Signal_Time = 0;
    tBOOL             l_bExit_Signal       = FALSE;
    tBOOL             l_bExit              = FALSE;
    tUINT32           l_dwSignal           = MEVENT_TIME_OUT;
    tUINT32           l_dwWait_TimeOut     = COMMUNICATION_THREAD_IDLE_TIMEOUT;
    CTPacket         *l_pReceived_Packet   = NULL;
    CTPacket         *l_pSent_Packet       = NULL;
    tUINT32           l_dwReceived         = 0;
    tBOOL             l_bConnected_Cur     = FALSE;
    sockaddr_storage  l_tAddress;

    memset(&l_tAddress, 0, sizeof(l_tAddress));

    while (FALSE == l_bExit)
    {
        l_dwSignal = m_cComm_Event.Wait(l_dwWait_TimeOut);

        if (THREAD_EXIT_SIGNAL == l_dwSignal)
        {
            l_dwExit_Signal_Time = GetTickCount();
            l_bExit_Signal       = TRUE;
        }

        //have to close thread ....
        if ( l_bExit_Signal )
        {
            if (Is_Ready_To_Exit()) 
            {
                Inc_Packet_ID(&m_dwLast_Packet_ID);
                m_cPacket_Bye.Finalize(m_dwLast_Packet_ID, m_wClient_ID);

                if (UDP_SOCKET_OK != m_pSocket->Send((char*)m_cPacket_Bye.Get_Buffer(), 
                                                      m_cPacket_Bye.Get_Size())
                   )
                {
                    JOURNAL_ERROR(m_pLog, TM("<<< m_pSocket->Send Failed"));
                }

                l_bExit = TRUE;
            }
            else if (CTicks::Difference(GetTickCount(), l_dwExit_Signal_Time) > m_dwExit_TimeOut)
            {
                JOURNAL_ERROR(m_pLog, 
                              TM("Exit imeout is exceed, not of all packets were sent !"));
                l_bExit = TRUE;
            }
        }

        if (    (MEVENT_TIME_OUT == l_dwSignal)
             && (FALSE == l_bExit) 
           )
        {
            //initiate timeout value, if we receive something or will be ready to send we will put 0 as value
            l_dwWait_TimeOut = COMMUNICATION_THREAD_IDLE_TIMEOUT;

            if (NULL == l_pReceived_Packet)
            {
                l_pReceived_Packet = m_pBPool->Pull_Buffer();
            }

            if (l_pReceived_Packet)
            {
                l_dwReceived = 0;
                if (UDP_SOCKET_OK == m_pSocket->Recv(&l_tAddress, 
                                                      (char*)l_pReceived_Packet->Get_Buffer(), 
                                                      l_pReceived_Packet->Get_Buffer_Size(), 
                                                     &l_dwReceived,
                                                      m_bIs_Response_Waiting ? SOCKET_RECEIVE_RESPONSE_TIMEOUT : 0
                                                    )
                   )
                {
                    if (    (l_dwReceived) 
                         && (FALSE == m_pSocket->Is_Server_Address(&l_tAddress)) 
                       )
                    {
                        l_dwReceived = 0;
                        JOURNAL_ERROR(m_pLog, 
                                      TM("<<<: Packet from wrong adress !")); 
                    }

                    if (l_dwReceived)
                    {
                        PACKET_PRINT(m_pLog, TM("[INP]"), l_pReceived_Packet);

                        //l_pPacket will be freed inside function or stored ...
                        Process_Incoming_Packet(l_pReceived_Packet);

                        ////////////////////////////////////////////////////////
                        // We pass packet to the stream, now packet it is head 
                        // pain of the stream, we can forget about it
                        ////////////////////////////////////////////////////////
                        l_pReceived_Packet = NULL;
                        l_dwWait_TimeOut = 0;
                    }
                }
            }

            while ((l_pSent_Packet = Get_Delivered_Packet()) != NULL)
            {
                l_dwWait_TimeOut = 0;
                PACKET_PRINT(m_pLog, TM("[OUT]"), l_pSent_Packet);

                if (UDP_SOCKET_OK != m_pSocket->Send((char*)l_pSent_Packet->Get_Buffer(), 
                                                      l_pSent_Packet->Get_Size())
                   )
                {
                    //nothing to do ... continue sending ....
                    //error message will be printed inside socket object
                }
            }

            l_bConnected_Cur = m_bConnected;

            if (COMMUNICATION_MAX_DELIVERY_FAILS <= m_dwDelivery_Fails)
            {
                l_bConnected_Cur = FALSE;
            }
            else
            {
                l_bConnected_Cur = TRUE;
            }

            if (l_bConnected_Cur != m_bConnected)
            {
                if (FALSE == l_bConnected_Cur)
                {
                    //clear connection event, it used to wait for connection
                    m_bConn_Event = TRUE;
                    m_cConn_Event.Clr(CONNECTION_SIGNAL);

                    Set_Connected(FALSE);
                    Reset_Connetion();// << Update_Channels_Status(...)
                    JOURNAL_INFO(m_pLog, TM("------->Drop Connection<-------"));
                }
                else
                {
                    Set_Connected(TRUE);
                    //after Set_Connected because they can update m_bConnected,
                    //m_dwConnection_Resets
                    Update_Channels_Status(m_bConnected, m_dwConnection_Resets);
                    JOURNAL_INFO(m_pLog, TM("<-------Establish Connection------->"));
                }
            }
        } // if (WAIT_TIMEOUT == l_dwSignal)
    }//while (FALSE == l_bExit)


    if (l_pReceived_Packet)
    {
        m_pBPool->Push_Buffer(l_pReceived_Packet);
        l_pReceived_Packet = NULL;
    }
}//Comm_Routine


////////////////////////////////////////////////////////////////////////////////
//Chnl_Routine
void CClBaical::Chnl_Routine()
{
    tBOOL              l_bExit         = FALSE;
    tUINT32            l_dwSignal      = 0;
    pAList_Cell        l_pElement      = NULL;
    CTPacket          *l_pPacket       = NULL;

    while (FALSE == l_bExit)
    {
        l_dwSignal = m_cChnl_Event.Wait(INFINITE);

        if (THREAD_EXIT_SIGNAL == l_dwSignal)
        {
            l_bExit = TRUE;
        }
        else if (THREAD_DATA_SIGNAL == l_dwSignal)
        {
            l_pPacket = NULL;

            LOCK_ENTER(m_hCS_Data_In); 
            l_pElement = m_pData_Queue_In->Get_First();
            if (l_pElement)
            {
                l_pPacket = m_pData_Queue_In->Get_Data(l_pElement);
                m_pData_Queue_In->Del(l_pElement, FALSE);
            }
            LOCK_EXIT(m_hCS_Data_In); 

            if (l_pPacket)
            {
                if (TPACKET_FLAG_EXTENSION & l_pPacket->Get_Flags()) //new server case v>4.3
                {
                    tUINT8 *l_pBuffer  = l_pPacket->Get_Buffer() + ACKNOWLEDGMENT_SIZE;
                    size_t  l_szBuffer = (size_t)l_pPacket->Get_Size() - ACKNOWLEDGMENT_SIZE;

                    while (l_szBuffer > sizeof(sH_Ext))
                    {
                        sH_Ext *l_pExt = (sH_Ext*)l_pBuffer;

                        if (ETPE_SERVER_NETWORK_INFO == l_pExt->wType)
                        {
                            //skip, was processed by communication thread
                        }
                        else if (ETPE_USER_DATA == l_pExt->wType)
                        {
                            Parse_User_Data(l_pBuffer + sizeof(sH_Ext), l_pExt->wSize - sizeof(sH_Ext));
                        }
                        else
                        {
                            JOURNAL_ERROR(m_pLog, TM("Unknown extension packet [%d:%d]"), (tUINT32)l_pExt->wType, (tUINT32)l_pExt->wSize);
                        }

                        l_pBuffer  += l_pExt->wSize;
                        l_szBuffer -= l_pExt->wSize;
                    }

                }
                else //old server case v<=4.3
                {
                    Parse_User_Data(l_pPacket->Get_Buffer() + ACKNOWLEDGMENT_SIZE,
                                    (size_t)l_pPacket->Get_Size() - ACKNOWLEDGMENT_SIZE);
                }

                m_pBPool->Push_Buffer(l_pPacket);
                l_pPacket = NULL;
            }
            else
            {
                JOURNAL_ERROR(m_pLog, 
                              TM("Get event [Data In], but no buffers were found!")
                             );
            }
        } //else if ( (WAIT_OBJECT_0 + 1) == l_dwSignal)
    }
}//Chnl_Routine


////////////////////////////////////////////////////////////////////////////////
//Get_Info
tBOOL CClBaical::Get_Info(sP7C_Info *o_pInfo)
{
    if (NULL == o_pInfo)
    {
        return FALSE;
    }

    LOCK_ENTER(m_hCS); 

    //memcpy(&(o_pInfo->sServer), m_pSocket->Get_Address(), sizeof(sockaddr_storage));

    m_pBPool->Get_Memory_Info((tUINT32*)&o_pInfo->dwMem_Used,
                              (tUINT32*)&o_pInfo->dwMem_Free,
                              (tUINT32*)&o_pInfo->dwMem_Alloc
                             );

    o_pInfo->dwReject_Mem = m_lReject_Mem;
    o_pInfo->dwReject_Con = m_lReject_Con;
    o_pInfo->dwReject_Int = m_lReject_Int;

    LOCK_EXIT(m_hCS); 

    return TRUE;
}//Get_Info


////////////////////////////////////////////////////////////////////////////////
//Flush
tBOOL CClBaical::Flush()
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


    m_cComm_Event.Set(THREAD_EXIT_SIGNAL);
    m_cChnl_Event.Set(THREAD_EXIT_SIGNAL);

    if (m_bComm_Thread)
    {
        if (TRUE == CThShell::Close(m_hComm_Thread, 60000))
        {
            m_hComm_Thread = 0;//NULL;
            m_bComm_Thread = FALSE;
        }
        else
        {
            JOURNAL_CRITICAL(m_pLog, TM("Can't close communication thread !"));
        }
    }

    if (m_bChnl_Thread)
    {
        if (TRUE == CThShell::Close(m_hChnl_Thread, 60000))
        {
            m_hChnl_Thread = 0;//NULL;
            m_bChnl_Thread = FALSE;
        }
        else
        {
            JOURNAL_CRITICAL(m_pLog, TM("Can't close channels thread !"));
        }
    }

    //to prevent sending new data
    LOCK_ENTER(m_hCS);
    m_bConnected = FALSE;
    LOCK_EXIT(m_hCS);

    return TRUE;
}//Flush


////////////////////////////////////////////////////////////////////////////////
//Connection_Wait
tBOOL CClBaical::Connection_Wait(tUINT32 i_dwMilliseconds)
{
    if (CONNECTION_SIGNAL == m_cConn_Event.Wait(i_dwMilliseconds))
    {
        return TRUE;
    }

    return FALSE;
}//Connection_Wait


////////////////////////////////////////////////////////////////////////////////
//Sent
eClient_Status CClBaical::Sent(tUINT32          i_dwChannel_ID,
                               sP7C_Data_Chunk *i_pChunks, 
                               tUINT32          i_dwCount,
                               tUINT32          i_dwSize
                              )
{
    eClient_Status   l_eReturn       = ECLIENT_STATUS_OK;
    CTPacket        *l_pPacket       = NULL;
    tUINT32          l_dwTotal_Size  = i_dwSize + (tUINT32)sizeof(sH_User_Raw);
    tUINT32          l_dwPacket_Size = 0;
    //Wanr: variables without default value!
    tBOOL            l_bExit;
    tUINT32          l_dwChunk_Offs;
    CTPData          l_cData;

    if (ECLIENT_STATUS_OK != m_eStatus)
    {
        ATOMIC_INC(&m_lReject_Int);
        return m_eStatus;
    }

    if (    (NULL                            == i_pChunks)
         || (0                               >= i_dwCount)
         || (USER_PACKET_MAX_SIZE            <= l_dwTotal_Size) 
         || (USER_PACKET_CHANNEL_ID_MAX_SIZE <= i_dwChannel_ID)
       )
    {
        //InterlockedIncrement(&m_lReject_Int);
        return ECLIENT_STATUS_WRONG_PARAMETERS;
    }

    //N.B. We do not check i_dwSize and real size of all chunks in release mode!!!
#ifdef _DEBUG
    //tUINT32 l_dwReal_Size = 0;
    //for (tUINT32 l_dwI = 0; l_dwI < i_dwCount; l_dwI ++)
    //{
    //    if (i_pChunks[l_dwI].pData)
    //    {
    //        l_dwReal_Size += i_pChunks[l_dwI].dwSize;
    //    }
    //    else
    //    {
    //        break;
    //    }
    //}
    //
    //if (l_dwReal_Size != i_dwSize)
    //{
    //    ATOMIC_INC(&m_lReject_Int);
    //    return ECLIENT_STATUS_WRONG_PARAMETERS;
    //}
#endif

    LOCK_ENTER(m_hCS);

    if (FALSE == m_bConnected)
    {
        ATOMIC_INC(&m_lReject_Con);
        l_eReturn = ECLIENT_STATUS_OFF;
        goto l_lExit;
    }

    l_dwPacket_Size = m_pBPool->Get_Buffer_Size() - CLIENT_DATA_HEADER_SIZE;

    //if size is more than available ...
    //If we will use all buffers from Pool for data - we can cause DoS.
    //no packets in Pool for incoming data from server, communication is broken
    if (   (l_dwTotal_Size + (l_dwPacket_Size * 3) )
         > (l_dwPacket_Size * m_pBPool->Get_Free_Count())
       )
    {
        ATOMIC_INC(&m_lReject_Mem);
        l_eReturn = ECLIENT_STATUS_NO_FREE_BUFFERS;
        goto l_lExit;
    }

    l_bExit        = FALSE;
    l_dwChunk_Offs = 0;

    //extract last packet form data queue, and update header if new packet belongs to the same channel!
    l_pPacket = Reuse_Data_Packet(l_cData, i_dwChannel_ID, i_dwSize);

    while (    (FALSE == l_bExit)
            && (l_pPacket)
          )
    {
        //if packet free size is larger or equal to chunk size
        if ( l_cData.Get_Tail_Size() >= i_pChunks->dwSize )
        {
            memcpy(l_cData.Get_Tail(), 
                   ((tUINT8*)i_pChunks->pData) + l_dwChunk_Offs,
                   i_pChunks->dwSize
                   );

            l_cData.Append_Size((tUINT16)i_pChunks->dwSize);

            //current chunk was moved, we reduce chunks amount 
            --i_dwCount;

            //if there is no more chunks - move packet to queue
            if (0 >= i_dwCount)
            {
                Push_Last_Data_Packet(l_pPacket);
                l_cData.Detach();
                l_pPacket = NULL;
                l_bExit   = TRUE;
            }
            else
            {
                //we are finish with that chunk
                //i_pChunks->dwSize = 0; 
                l_dwChunk_Offs = 0;

                //go to next chunk
                i_pChunks ++;

                //if packet is filled - put it to data queue
                if (0 >= l_cData.Get_Tail_Size())
                {
                    Push_Last_Data_Packet(l_pPacket);
                    l_cData.Detach();

                    l_pPacket = m_pBPool->Pull_Buffer();
                    if (l_pPacket)
                    {
                        l_cData.Attach(l_pPacket);
                        l_cData.Initialize();
                    }
                    else
                    {
                        l_eReturn = ECLIENT_STATUS_NO_FREE_BUFFERS;
                        l_bExit   = TRUE;
                        ATOMIC_INC(&m_lReject_Mem);
                    }
                }
            }
        }
        else //if chunk data is greater than packet free space
        {
            memcpy(l_cData.Get_Tail(), 
                   ((tUINT8*)i_pChunks->pData) + l_dwChunk_Offs,
                   l_cData.Get_Tail_Size()
                   );
            l_dwChunk_Offs   += l_cData.Get_Tail_Size();
            i_pChunks->dwSize -= l_cData.Get_Tail_Size();

            l_cData.Append_Size(l_cData.Get_Tail_Size());

            Push_Last_Data_Packet(l_pPacket);
            l_cData.Detach();

            l_pPacket = m_pBPool->Pull_Buffer();
            if (l_pPacket)
            {
                l_cData.Attach(l_pPacket);
                l_cData.Initialize();
            }
            else
            {
                l_eReturn = ECLIENT_STATUS_NO_FREE_BUFFERS;
                l_bExit   = TRUE;
                ATOMIC_INC(&m_lReject_Mem);
            }
        }
    } //while (FALSE == l_bExit)


l_lExit:

    LOCK_EXIT(m_hCS);

    return l_eReturn;

}//Sent
