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
// This header file provide base client functionality                          /
////////////////////////////////////////////////////////////////////////////////
#ifndef CLIENT_H
#define CLIENT_H

tXCHAR *Get_Argument_Text_Value(tXCHAR       **i_pArgs,
                                tINT32         i_iCount,
                                const tXCHAR  *i_pName
                               );



////////////////////////////////////////////////////////////////////////////////
//CClient
class CClient:
    public IP7_Client
{
protected:
    //put volatile variables at the top, to obtain 32 bit alignment. 
    //Project has 8 bytes alignment by default
    tINT32 volatile         m_lReference;
    volatile eClient_Status m_eStatus;
    tLOCK                   m_hCS_Reg; 
    tLOCK                   m_hCS; 
    IP7C_Channel           *m_pChannels[USER_PACKET_CHANNEL_ID_MAX_SIZE];
    CShared::hShared        m_hShared;
    tBOOL                   m_bCrashMem;
    IJournal               *m_pLog;
    tBOOL                   m_bConnected;
    tUINT32                 m_dwConnection_Resets;
    IP7_Client::eType       m_eType;
    tXCHAR                **m_pArgs;
    tINT32                  m_iArgsCnt;

public:
    CClient(IP7_Client::eType i_eType,
            tXCHAR          **i_pArgs,
            tINT32            i_iCount
           );
    virtual ~CClient();

public:
    virtual IP7_Client::eType Get_Type();

    virtual tINT32            Add_Ref();
    virtual tINT32            Release();
                              
    virtual eClient_Status    Get_Status();
    virtual tBOOL             Get_Status(sP7C_Status *o_pStatus);
    //virtual tBOOL             Get_Info(sP7C_Info *o_pInfo);        
                              
    virtual eClient_Status    Register_Channel(IP7C_Channel *i_pChannel);
    virtual eClient_Status    Unregister_Channel(tUINT32 i_dwID);

    //virtual eClient_Status Sent(tUINT32          i_dwChannel_ID,
    //                            sP7C_Data_Chunk *i_pChunks, 
    //                            tUINT32          i_dwCount,
    //                            tUINT32          i_dwSize);

    virtual tBOOL             Share(const tXCHAR *i_pName);
                              
    virtual const tXCHAR     *Get_Argument(const tXCHAR  *i_pName);

    virtual tBOOL             Unshare();

    //reimplemented in childes, will be called in case of process crash
    virtual tBOOL             Flush() = 0;
                              
protected:                    
    eClient_Status            Init_Log(tXCHAR **i_pArgs, tINT32 i_iCount);
                              
    eClient_Status            Init_Crash_Handler(tXCHAR **i_pArgs, tINT32 i_iCount);
    eClient_Status            Uninit_Crash_Handler();
};                            

             
#endif //CLIENT_H
