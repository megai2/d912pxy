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

#ifndef CLBAICAL_H
#define CLBAICAL_H


class CClBaical:
    public CClient
{
    tINT32 volatile      m_lReject_Mem;
    tINT32 volatile      m_lReject_Con;
    tINT32 volatile      m_lReject_Int;

    tBOOL                m_bIs_Winsock;
    CUDP_Socket         *m_pSocket;
    tUINT16              m_wClient_ID;
    CBuffers_Pool       *m_pBPool;

    CBList<CTPacket*>   *m_pData_Wnd;
    pAList_Cell          m_pData_Wnd_Cell;
    tBOOL                m_bData_Wnd_Fixed;
    tUINT32              m_dwData_Wnd_Max_Count;
    tUINT32              m_dwData_Wnd_Size;
    tUINT32              m_dwData_Wnd_TimeStamp;
    tUINT32              m_dwData_Wnd_First_ID;
    tUINT32              m_dwData_Wnd_Last_ID;
    tBOOL                m_bData_Resending;

    tUINT32              m_dwDelivery_Fails;
    tBOOL                m_bIs_Response_Waiting;

    CBList<CTPacket*>   *m_pData_Queue_Out;
    //need to synchronize updating m_pData_Queue by user and requests from internal threads
    tLOCK                m_hCS_Data_Out; 

    CBList<CTPacket*>   *m_pData_Queue_In;
    tLOCK                m_hCS_Data_In; 

    tUINT32              m_dwLast_Packet_ID;
    tUINT32              m_dwServiceTimeStamp;

    tUINT32              m_dwExit_TimeOut;

    //permanent packets, they do not consume a lot of memory and more 
    //practical have them always instead of taking them from pool
    CTPacket            *m_pPacket_Control;
    CTPClient_Hello      m_cPacket_Hello;
    CTPData_Window       m_cPacket_Data_Report;
    CTPClient_Ping       m_cPacket_Alive;
    CTPClient_Bye        m_cPacket_Bye;
    tBOOL                m_bBig_Endian;

    CMEvent              m_cConn_Event;
    tBOOL                m_bConn_Event;

    CMEvent              m_cComm_Event;
    tBOOL                m_bComm_Thread;
    CThShell::tTHREAD    m_hComm_Thread;

    CMEvent              m_cChnl_Event;
    tBOOL                m_bChnl_Thread;
    CThShell::tTHREAD    m_hChnl_Thread;

    tBOOL                m_bLocalHost;

    tBOOL                m_bExtension;

    void                *m_pHdrUsrData;
    CTPacket            *m_pHdrUsrPacket;
    tUINT32              m_uHdrUsrSize;
    tUINT32              m_uHdrUsrChannel;

public:
    CClBaical(tXCHAR **i_pArgs,
              tINT32   i_iCount
             );
    virtual ~CClBaical();

private:
    eClient_Status   Init_Base(tXCHAR **i_pArgs,
                               tINT32   i_iCount
                              );
    eClient_Status   Init_Log(tXCHAR **i_pArgs,
                              tINT32   i_iCount
                             );
    eClient_Status   Init_Sockets(tXCHAR **i_pArgs,
                                  tINT32   i_iCount
                                 );
    eClient_Status   Init_Pool(tXCHAR **i_pArgs,
                               tINT32   i_iCount
                              );
    eClient_Status   Init_Members(tXCHAR **i_pArgs,
                                  tINT32   i_iCount
                                 );
    //eClient_Status Init_Threads(tXCHAR **i_pArgs,
    //                            tINT32   i_iCount
    //                           );

    void             Inc_Packet_ID(tUINT32 * o_pPacketID);
                    
    tBOOL            Process_Incoming_Packet(CTPacket *i_pPacket);
    CTPacket        *Get_Delivered_Packet();
    tBOOL            Is_Ready_To_Exit();
                    
    void             Set_Connected(tBOOL i_bConnected);
                    
    void             Reset_Connetion();
    CTPacket        *Create_Data_Wnd_Report();
    inline CTPacket *Pull_Firt_Data_Packet();
    inline CTPacket *Reuse_Data_Packet(CTPData &i_rData, tUINT32 i_uChannel, tUINT32 i_uSize);
    inline tBOOL     Push_Last_Data_Packet(CTPacket *i_pPacket);
                     
    inline void      Parse_Extensions(tUINT8 *i_pBuffer, size_t i_szBuffer);
                     
    inline void      Parse_User_Data(tUINT8 *i_pBuffer, size_t i_szBuffer);
                     
    inline void      Update_Channels_Status(tBOOL i_bConnected, tUINT32 i_dwResets);
                     
    void             Comm_Routine();
    void             Chnl_Routine();
                     
public:              
    tBOOL            Connection_Wait(tUINT32 i_dwMilliseconds);
                     
    eClient_Status   Sent(tUINT32            i_dwChannel_ID,
                          sP7C_Data_Chunk   *i_pChunks, 
                          tUINT32            i_dwCount,
                          tUINT32            i_dwSize
                         );
                     
    tBOOL            Get_Info(sP7C_Info *o_pInfo);
    tBOOL            Flush();

private:
    static THSHELL_RET_TYPE THSHELL_CALL_TYPE Static_Comm_Routine(void *i_pContext)
    {
        CClBaical *l_pRoutine = static_cast<CClBaical *>(i_pContext);
        if (l_pRoutine)
        {
            l_pRoutine->Comm_Routine();
        }

        CThShell::Cleanup();
        return THSHELL_RET_OK;
    } 

    static THSHELL_RET_TYPE THSHELL_CALL_TYPE Static_Chnl_Routine(void *i_pContext)
    {
        CClBaical *l_pRoutine = static_cast<CClBaical *>(i_pContext);
        if (l_pRoutine)
        {
            l_pRoutine->Chnl_Routine();
        }

        CThShell::Cleanup();
        return THSHELL_RET_OK;
    } 
};


#endif //CLBAICAL_H
