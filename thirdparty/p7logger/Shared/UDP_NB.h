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
#ifndef UDP_NB_H
#define UDP_NB_H

#include "PSocket.h"


#define SENT_SELECT_TIMEOUT                                                 (10)

enum eSocket_Status
{
    UDP_SOCKET_OK                   = 0,
    UDP_SOCKET_RECEIVE_ERROR           ,
    UDP_SOCKET_SEND_ERROR              ,
    UDP_SOCKET_CREATE_ERROR            ,
    UDP_SOCKET_SELECT_ERROR            ,
    UDP_SOCKET_WRONG_PARAMETERS        ,
    UDP_SOCKET_NOT_INITIALIZED         ,
    UDP_SOCKET_BIND_ERROR              ,
    UDP_SOCKET_NOT_READY
};

enum eFD_Type
{
    FD_TYPE_WRITE,
    FD_TYPE_READ
};


//Uncomment this macro to artificially create network errors, allows to test
//protocols for durability, value is equal to % of errors
//#define UDP_SOCKET_ERRORS_CHANCE 2.0f 

////////////////////////////////////////////////////////////////////////////////
// Class for providing basic functionality over udp socket
class CUDP_Socket
{
private:
    tSOCKET           m_hSocket;
    IJournal         *m_pLog;
    tBOOL             m_bServer;
    sockaddr_storage  m_tAddress;
    tUINT32           m_dwAddress_Size;
    tINT32            m_iFamily;

    tINT32            m_iSndBufSize;
    tINT32            m_iRcvBufSize;
public:
    ////////////////////////////////////////////////////////////////////////////
    //CUDP_Socket::CUDP_Socket
    CUDP_Socket(IJournal    *i_pLog,
                sockaddr    *i_pAddress,
                tBOOL        i_bServer)
        : m_hSocket(INVALID_SOCKET_VAL)
        , m_pLog(i_pLog)
        , m_bServer(i_bServer)
        , m_dwAddress_Size(0)
        , m_iFamily(AF_UNSPEC)
        , m_iSndBufSize(CLIENT_SEND_BUFFER_SIZE)
        , m_iRcvBufSize(CLIENT_RECV_BUFFER_SIZE)
    {
        eSocket_Status l_eStatus = UDP_SOCKET_OK;

        if (m_pLog)
        {
            m_pLog->Add_Ref();
        }

        if (NULL == i_pAddress)
        {
            l_eStatus = UDP_SOCKET_WRONG_PARAMETERS;
            JOURNAL_ERROR(m_pLog, TM("NULL == i_pAddress"));
        }

        if (UDP_SOCKET_OK == l_eStatus )
        {
            memset(&m_tAddress, 0, sizeof(sockaddr_storage));

            m_iFamily = i_pAddress->sa_family;
            if (AF_INET == i_pAddress->sa_family)
            {
                m_dwAddress_Size = sizeof(sockaddr_in);
                memcpy(&m_tAddress, i_pAddress, m_dwAddress_Size);
            }
            else if (AF_INET6 == i_pAddress->sa_family)
            {
                m_dwAddress_Size = sizeof(sockaddr_in6);
                memcpy(&m_tAddress, i_pAddress, m_dwAddress_Size);
            }
            else
            {
                l_eStatus = UDP_SOCKET_WRONG_PARAMETERS; 
                JOURNAL_ERROR(m_pLog, 
                              TM("Address family is wrong = %d"), 
                              (tUINT32)m_iFamily);
            }

        }

        if (UDP_SOCKET_OK == l_eStatus )
        {
            XCHAR    l_pIP[128] = TM("?");
            tUINT32  l_dwLength = sizeof(l_pIP) / sizeof(l_pIP[0]);
            
            if (Print_SAddr(i_pAddress, l_pIP, l_dwLength)) 
            {
                JOURNAL_INFO(m_pLog, TM("Use address = %s"), l_pIP);
            }

            l_eStatus = Create_Socket(m_iFamily);
        }

        //m_pAddress.sin_family      = AF_INET;
        //m_pAddress.sin_addr.s_addr = inet_addr(i_pIP);
        //m_pAddress.sin_port        = htons(i_wPort);

        if (UDP_SOCKET_OK == l_eStatus )
        {
            if (m_bServer)
            {
                m_iSndBufSize = SERVER_SEND_BUFFER_SIZE;
                m_iRcvBufSize = SERVER_RECV_BUFFER_SIZE;

                if (0 != bind(m_hSocket, (sockaddr*)&m_tAddress, m_dwAddress_Size))
                {
                    JOURNAL_ERROR(m_pLog, 
                                  TM("Bind failed, error=%d !"), 
                                  (tUINT32)GET_SOCKET_ERROR()
                                 );

                    l_eStatus = UDP_SOCKET_BIND_ERROR;
                }
                else
                {
                    tINT32 l_iFlagRcv = SERVER_RECV_BUFFER_SIZE;
                    tINT32 l_iFlagSnd = SERVER_SEND_BUFFER_SIZE;

                    if (    (0 != setsockopt(m_hSocket, SOL_SOCKET, SO_RCVBUF, (char*)&l_iFlagRcv, sizeof(l_iFlagRcv)))
                         || (0 != setsockopt(m_hSocket, SOL_SOCKET, SO_SNDBUF, (char*)&l_iFlagSnd, sizeof(l_iFlagSnd)))
                       )
                    {
                        JOURNAL_ERROR(m_pLog, TM("Failed to set socket options, error=%d !"), (tUINT32)GET_SOCKET_ERROR());
                    }

                    tINT32    l_iFlag  = 0;
                    socklen_t l_szFlag = (socklen_t)sizeof(l_iFlag);

                    if (0 == getsockopt(m_hSocket, SOL_SOCKET, SO_RCVBUF, (char*)&l_iFlag, &l_szFlag))
                    {
                        m_iRcvBufSize = l_iFlag;
                    }

                    l_iFlag  = 0;
                    l_szFlag = (socklen_t)sizeof(l_iFlag);

                    if (0 == getsockopt(m_hSocket, SOL_SOCKET, SO_SNDBUF, (char*)&l_iFlag, &l_szFlag))
                    {
                        m_iSndBufSize = l_iFlag;
                    }
                }
            }
            else //not server
            {
                tINT32    l_iFlag  = CLIENT_RECV_BUFFER_SIZE;
                tINT32    l_iReal  = 0;
                socklen_t l_szSize = sizeof(l_iFlag);
                
                if (SOCKET_ERROR == setsockopt(m_hSocket,
                                               SOL_SOCKET, 
                                               SO_RCVBUF,
                                               (char*)&l_iFlag, 
                                               sizeof(l_iFlag)
                                              )
                   )
                {
                    JOURNAL_ERROR(m_pLog,
                                  TM("setsockopt fail, error=%d !"), 
                                  GET_SOCKET_ERROR()
                                 );
                }

                if (SOCKET_ERROR == getsockopt(m_hSocket, 
                                               SOL_SOCKET, 
                                               SO_RCVBUF, 
                                               (char*)&l_iReal,
                                               &l_szSize
                                              )
                   )
                {
                    JOURNAL_ERROR(m_pLog,
                                  TM("getsockopt fail, error=%d !"), 
                                  GET_SOCKET_ERROR()
                                 );
                }
                else
                {
                    m_iRcvBufSize = l_iReal;
                    if (l_iReal != l_iFlag)
                    {
                        JOURNAL_WARNING(m_pLog,
                                        TM("Socket receive buffer != necessary %d/%d"), 
                                        l_iReal,
                                        l_iFlag
                                       );
                    }
                }
                
                l_iFlag = CLIENT_SEND_BUFFER_SIZE;
                if (SOCKET_ERROR == setsockopt(m_hSocket, 
                                               SOL_SOCKET, 
                                               SO_SNDBUF, 
                                               (char*)&l_iFlag, 
                                               sizeof(l_iFlag)
                                              )
                   )
                {
                    JOURNAL_ERROR(m_pLog,
                                  TM("setsockopt fail, error=%d !"), 
                                  GET_SOCKET_ERROR()
                                 );
                }
                    
                if (SOCKET_ERROR == getsockopt(m_hSocket, 
                                               SOL_SOCKET, 
                                               SO_SNDBUF, 
                                               (char*)&l_iReal, 
                                               &l_szSize
                                              )
                   )
                {
                    JOURNAL_ERROR(m_pLog,
                                  TM("getsockopt fail, error=%d !"), 
                                  GET_SOCKET_ERROR()
                                 );
                }
                else
                {
                    m_iSndBufSize = l_iReal;

                    if (l_iReal != l_iFlag)
                    {
                        JOURNAL_WARNING(m_pLog,
                                        TM("Socket send buffer size is less than necessary %d/%d"), 
                                        l_iReal,
                                        l_iFlag
                                       );
                    }
                }
            }//else //not server
        }

        if (UDP_SOCKET_OK == l_eStatus )
        {
            if (FALSE == Disable_PortUnreachable_ICMP(m_hSocket))
            {
                JOURNAL_ERROR(m_pLog, 
                              TM("WSAIoctl(SIO_UDP_CONNRESET), error=%d !"), 
                              (tUINT32)GET_SOCKET_ERROR()
                             );
            }
        }

        if (    (UDP_SOCKET_OK != l_eStatus) 
             && (INVALID_SOCKET_VAL != m_hSocket)
           )
        {
            CLOSE_SOCKET(m_hSocket);
            m_hSocket = INVALID_SOCKET_VAL;
        }
    }//CUDP_Socket::CUDP_Socket


    ////////////////////////////////////////////////////////////////////////////
    //CUDP_Socket::~CUDP_Socket
    ~CUDP_Socket()
    {
        if (INVALID_SOCKET_VAL != m_hSocket)
        {
            CLOSE_SOCKET(m_hSocket);
            m_hSocket = INVALID_SOCKET_VAL;
        }

        if (m_pLog)
        {
            m_pLog->Release();
            m_pLog = NULL;
        }

    }//CUDP_Socket::~CUDP_Socket


    ////////////////////////////////////////////////////////////////////////////
    //CUDP_Socket::Get_Address
    sockaddr_storage *Get_Address()
    {
        return &m_tAddress;
    }//CUDP_Socket::Get_Address

    ////////////////////////////////////////////////////////////////////////////
    //CUDP_Socket::Initialized
    tBOOL Initialized()
    {
        return (INVALID_SOCKET_VAL != m_hSocket);
    }//CUDP_Socket::Initialized

    ////////////////////////////////////////////////////////////////////////////
    //CUDP_Socket::GetRecvBufferSize
    tINT32 GetRecvBufferSize()
    {
        return m_iRcvBufSize;
    }//CUDP_Socket::GetRecvBufferSize

    ////////////////////////////////////////////////////////////////////////////
    //CUDP_Socket::GetSendBufferSize
    tINT32 GetSendBufferSize()
    {
        return m_iSndBufSize;
    }//CUDP_Socket::GetSendBufferSize

    ////////////////////////////////////////////////////////////////////////////
    //CUDP_Socket::Send
    eSocket_Status Send(const char *i_pBuffer, tUINT32 i_dwSize)
    {
        return Send((sockaddr *)&m_tAddress, m_dwAddress_Size, i_pBuffer, i_dwSize);
    }//CUDP_Socket::Send


    ////////////////////////////////////////////////////////////////////////////
    //CUDP_Socket::Send
    eSocket_Status Send(sockaddr   *i_pAddress, 
                        tUINT32     i_dwAddress_Size,
                        const char *i_pBuffer, 
                        tUINT32     i_dwSize
                       )
    {
        eSocket_Status l_eResult = UDP_SOCKET_OK;

        if (INVALID_SOCKET_VAL == m_hSocket)
        {
            return UDP_SOCKET_NOT_INITIALIZED;
        }

        #if defined(UDP_SOCKET_ERRORS_CHANCE)
        {
            tINT32 l_iChance = rand(); //it generate digits -32767 .. 0 .. 32767
            tINT32 l_iLimit  = (tINT32)((32767.0f * UDP_SOCKET_ERRORS_CHANCE) / 100.0f);

            if (0 > l_iChance) l_iChance = -l_iChance;

            if (l_iLimit >= l_iChance)                               
            {                                                   
                if ((l_iLimit / 2) <= l_iChance) // 50% do not sent        
                {                                               
                    return l_eResult;                           
                }                                               
                else //50% damage size                          
                {                                               
                    i_dwSize = i_dwSize / 2;                    
                }                                               
            }                                                   
        }
        #endif                                                  

        if (    (NULL == i_pBuffer) 
             || (NULL == i_pAddress)
             || (0    == i_dwSize)
           )
        {
            return UDP_SOCKET_WRONG_PARAMETERS;
        }

        tUINT32        l_dwSent       = 0;
        tINT32         l_iRes         = SOCKET_ERROR;
        eSocket_Status l_eReadyResult = UDP_SOCKET_OK;

        while (l_dwSent < i_dwSize)
        {
            l_eReadyResult = Is_Ready(FD_TYPE_WRITE, SENT_SELECT_TIMEOUT);
            if (UDP_SOCKET_OK == l_eReadyResult)
            {
                l_iRes = sendto(m_hSocket, 
                                i_pBuffer + l_dwSent, 
                                i_dwSize - l_dwSent, 
                                0, 
                                i_pAddress, 
                                i_dwAddress_Size
                               );

                if (SOCKET_ERROR == l_iRes)
                {
                    JOURNAL_ERROR(m_pLog,
                                  TM("Send fail, error=%d !"), 
                                  GET_SOCKET_ERROR()
                                 );

                    l_eResult = UDP_SOCKET_SEND_ERROR;
                    break;
                }
                else
                {
                    l_dwSent += l_iRes;
                }
            }
            else if (UDP_SOCKET_NOT_READY != l_eReadyResult)
            {
                l_eResult = UDP_SOCKET_SELECT_ERROR;
                break;
            }
        }

        return l_eResult;
    }//CUDP_Socket::Send


    ////////////////////////////////////////////////////////////////////////////
    //CUDP_Socket::Recv
    eSocket_Status Recv(sockaddr_storage *o_pAddress, 
                        char             *i_pBuffer, 
                        tUINT32           i_dwSize, 
                        tUINT32          *o_pReceived, 
                        tUINT32           i_dwTimeOut
                       )
    {
        eSocket_Status l_eResult   = UDP_SOCKET_NOT_READY;
        eSocket_Status l_eIs_Ready = Is_Ready(FD_TYPE_READ, i_dwTimeOut);

        if (UDP_SOCKET_OK == l_eIs_Ready)
        {
            l_eResult = Recv(o_pAddress, i_pBuffer, i_dwSize, o_pReceived);
        }
        else
        {
            if (UDP_SOCKET_NOT_READY != l_eIs_Ready)
            {
                l_eResult = UDP_SOCKET_SELECT_ERROR;
            }
        }

        return l_eResult;
    }//CUDP_Socket::Recv


    ////////////////////////////////////////////////////////////////////////////
    //CUDP_Socket::Recv
    //Before calling this function you should call Get_IMessage_Size();
    //if there is no message this function will block until message will be recv.
    eSocket_Status Recv(sockaddr_storage *o_pAddress, 
                        char             *i_pBuffer, 
                        tUINT32           i_dwSize, 
                        tUINT32          *o_pReceived
                       )
    {
        eSocket_Status l_eResult = UDP_SOCKET_OK;

        if (INVALID_SOCKET_VAL == m_hSocket)
        {
            return UDP_SOCKET_NOT_INITIALIZED;
        }

        if (    (NULL == i_pBuffer) 
             || (NULL == o_pAddress) 
             || (NULL == o_pReceived)
             || (0    == i_dwSize) 
           )
        {
            return UDP_SOCKET_WRONG_PARAMETERS;
        }

        tADDR_LEN l_lAddr_Size = sizeof(*o_pAddress);
        *o_pReceived = 0;

        tINT32 l_lRecvRes = recvfrom(m_hSocket, 
                                     i_pBuffer, 
                                     i_dwSize, 
                                     0, 
                                     (sockaddr*)o_pAddress, 
                                     &l_lAddr_Size
                                    );

        if (SOCKET_ERROR == l_lRecvRes)
        {
            JOURNAL_ERROR(m_pLog, 
                          TM("recvfrom failed, error=%d !"), 
                          GET_SOCKET_ERROR()
                         );
            
            l_eResult = UDP_SOCKET_RECEIVE_ERROR;
        }
        else
        {
            *o_pReceived = l_lRecvRes;
        }

        return l_eResult;
    }//CUDP_Socket::Recv


    ////////////////////////////////////////////////////////////////////////////
    //CUDP_Socket::Peek
    //This function can read part of the datagram (header for example) and 
    //datagram is not removed from queue
    //Use it carefully, for your own risk
    eSocket_Status Peek(char    *i_pBuffer, 
                        tUINT32  i_dwSize 
                       )
    {
        eSocket_Status l_eResult = UDP_SOCKET_OK;

        if (INVALID_SOCKET_VAL == m_hSocket)
        {
            return UDP_SOCKET_NOT_INITIALIZED;
        }

        if (    (NULL == i_pBuffer) 
             || (0    == i_dwSize) 
           )
        {
            return UDP_SOCKET_WRONG_PARAMETERS;
        }

        tINT32 l_lRecvRes = recv(m_hSocket, 
                                 i_pBuffer, 
                                 i_dwSize, 
                                 MSG_PEEK 
                                );

        if (SOCKET_ERROR == l_lRecvRes)
        {
            tINT32 l_iError = GET_SOCKET_ERROR();
#if  defined(_WIN32) || defined(_WIN64)
            if (WSAEMSGSIZE != l_iError)
#endif 
            {
                
                JOURNAL_ERROR(m_pLog, 
                              TM("recvfrom failed, error=%d !"), 
                              l_iError  
                             );
           
                l_eResult = UDP_SOCKET_RECEIVE_ERROR;
            }
        }
        else if (0 >= l_lRecvRes)
        {
            l_eResult = UDP_SOCKET_NOT_READY;
        }

        return l_eResult;
    }//CUDP_Socket::Peek


    ////////////////////////////////////////////////////////////////////////////
    //CUDP_Socket::Is_Server_Address
    tBOOL Is_Server_Address(sockaddr_storage *i_pAddress)
    {
        if (    (NULL == i_pAddress)
             || (TRUE == m_bServer)
             || (i_pAddress->ss_family != m_tAddress.ss_family)
           )
        {
            return FALSE;
        }

        if (    (AF_INET == i_pAddress->ss_family)
             && (((sockaddr_in*)i_pAddress)->sin_port == ((sockaddr_in*)&m_tAddress)->sin_port)
             && (((sockaddr_in*)i_pAddress)->sin_addr.s_addr == ((sockaddr_in*)&m_tAddress)->sin_addr.s_addr)
           )
        {
            return TRUE;
        }
        else if (    (AF_INET6 == i_pAddress->ss_family)
                  && (((sockaddr_in6*)i_pAddress)->sin6_port == ((sockaddr_in6*)&m_tAddress)->sin6_port)
                  && (0 == memcmp(&((sockaddr_in6*)i_pAddress)->sin6_addr, 
                                  &((sockaddr_in6*)&m_tAddress)->sin6_addr, 
                                  sizeof(in6_addr)
                                 )
                     )
                )
        {
             return TRUE;
        }

        return FALSE;
    }//CUDP_Socket::Is_Server_Address


    ////////////////////////////////////////////////////////////////////////////
    //CUDP_Socket::Is_Ready
    eSocket_Status Is_Ready(eFD_Type i_eType, tUINT32 i_dwTimeOut)
    {
        eSocket_Status l_eReturn = UDP_SOCKET_NOT_READY;

        if (INVALID_SOCKET_VAL == m_hSocket)
        {
            return UDP_SOCKET_NOT_INITIALIZED;
        }

        tINT32  l_iSelect = SOCKET_ERROR;
        fd_set  l_pFDS;
        timeval l_tTimeOut;

        l_tTimeOut.tv_sec  = 0;
        l_tTimeOut.tv_usec = i_dwTimeOut * 1000; 

        FD_ZERO(&l_pFDS);
        FD_SET((tUINT32)m_hSocket, &l_pFDS);
    
        if (FD_TYPE_READ == i_eType)
        {
            l_iSelect = select(((tUINT32)m_hSocket) + 1,
                               &l_pFDS,
                               NULL, 
                               NULL,
                               &l_tTimeOut
                              );
        }
        else
        {
            l_iSelect = select(((tUINT32)m_hSocket) + 1, 
                               NULL, 
                               &l_pFDS, 
                               NULL, 
                               &l_tTimeOut
                              );
        }

        if (SOCKET_ERROR == l_iSelect)
        {
            JOURNAL_ERROR(m_pLog, 
                          TM("Select fail, error=%d !"), 
                          GET_SOCKET_ERROR()
                         );

            l_eReturn = UDP_SOCKET_SELECT_ERROR;
        }
        else if (    (l_iSelect > 0) 
                  && (FD_ISSET(m_hSocket, &l_pFDS))
                )
        {
            l_eReturn = UDP_SOCKET_OK;
        }

        return l_eReturn;
    }//CUDP_Socket::Is_Ready


    ////////////////////////////////////////////////////////////////////////////
    //CUDP_Socket::Create_Socket
    eSocket_Status Create_Socket(tINT32 i_iFamily)
    {
        m_hSocket = socket(i_iFamily, SOCK_DGRAM, IPPROTO_UDP);
        if (INVALID_SOCKET_VAL == m_hSocket)
        {
            JOURNAL_ERROR(m_pLog,
                          TM("Socket creation failed, error=%d !"),
                          GET_SOCKET_ERROR()
                         );

            return UDP_SOCKET_CREATE_ERROR;
        }

        return UDP_SOCKET_OK;
    }//CUDP_Socket::Create_Socket


    ////////////////////////////////////////////////////////////////////////////
    //CUDP_Socket::Get_Input_Data_Size
    //Not used 
    //tUINT32 Get_Input_Data_Size()
    //{
    //    tUINT32 l_dwResult = 0;
    //
    //    if (INVALID_SOCKET_VAL != m_hSocket)
    //    {
    //         Use to determine the amount of data pending in the network's input 
    //        buffer that can be read from socket s. The argp parameter points to an unsigned long value in which ioctlsocket stores the result. FIONREAD returns the amount of data that can be read in a single call to the recv function, which may not be the same as the total amount of data queued on the socket. If s is message oriented (for example, type SOCK_DGRAM), FIONREAD still returns the amount of pending data in the network buffer, however, the amount that can actually be read in a single call to the recv function is limited to the data size written in the send or sendto function call.
    //         http://msdn.microsoft.com/en-us/library/windows/desktop/ms738573(v=vs.85).aspx
    //
    //        if (NO_ERROR != ioctlsocket(m_hSocket, FIONREAD, &l_dwResult))
    //        {
    //            l_dwResult = 0;
    //        }
    //    }
    //
    //    return l_dwResult;
    //}//CUDP_Socket::Get_Input_Data_Size


};//CUDP_Socket


#endif //UDP_NB