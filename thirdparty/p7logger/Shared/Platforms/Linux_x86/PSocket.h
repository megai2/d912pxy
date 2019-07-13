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
#ifndef PSOCKET_H
#define PSOCKET_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <ifaddrs.h>
#include "AList.h"
#include <string.h>


#ifdef __ARM_ARCH_5TEJ__
  #define SERVER_RECV_BUFFER_SIZE                                       (0x200000) //2mb
  #define SERVER_SEND_BUFFER_SIZE                                       (0x40000)  //256kb

  #define CLIENT_RECV_BUFFER_SIZE                                       (0x4000)  //16 kb
  #define CLIENT_SEND_BUFFER_SIZE                                       (0x10000)  //64 kb
#else
  #define SERVER_RECV_BUFFER_SIZE                                       (0x200000) //2mb
  #define SERVER_SEND_BUFFER_SIZE                                       (0x40000)  //256kb

  #define CLIENT_RECV_BUFFER_SIZE                                       (0x40000)  //256 kb
  #define CLIENT_SEND_BUFFER_SIZE                                       (0x40000)  //256 kb
#endif

typedef int       tSOCKET; 
//typedef SOCKET      tSOCKET; 

typedef socklen_t  tADDR_LEN; 
//typedef int  tADDR_LEN; 

typedef addrinfo  tADDR_INFO; 
//typedef addrinfoW tADDR_INFO;


#define INVALID_SOCKET_VAL                                                  (-1)
                                                              //(INVALID_SOCKET)

//#define CONNECTION_IN_PROGRESS                                     (EINPROGRESS)
                                                              //(WSAEWOULDBLOCK)





#define CLOSE_SOCKET(i_Socket)                                   close(i_Socket)
                                                       //closesocket(m_hSocket);


#define GET_SOCKET_ERROR()                                                 errno
                                                             //WSAGetLastError();

#define SOCKET_ERROR                                                        (-1)

#define GET_ADDR_INFO(i_Node, i_Service, i_Hints, o_Res) getaddrinfo(i_Node,\
                                                                     i_Service,\
                                                                     i_Hints,\
                                                                     o_Res)                                  
//GetAddrInfoW

#define FREE_ADDR_INFO(i_Info) freeaddrinfo(i_Info)    
//FreeAddrInfoW

////////////////////////////////////////////////////////////////////////////////
//WSA_Init
static __attribute__ ((unused)) tBOOL WSA_Init()
{
    return TRUE;
    
    //WSADATA l_tWSA;
    //
    //if (0 != WSAStartup(MAKEWORD(1,1), &l_tWSA))
    //{
    //    return FALSE;
    //}
    //return TRUE;
}//WSA_Init


////////////////////////////////////////////////////////////////////////////////
//WSA_UnInit
static __attribute__ ((unused)) void WSA_UnInit()
{
    //WSACleanup();
}//WSA_UnInit

////////////////////////////////////////////////////////////////////////////////
//PEnumIps
static __attribute__ ((unused)) tBOOL PEnumIps(CBList<sockaddr_storage*> *io_pList)
{
    if (!io_pList)
    {
        return FALSE;
    }

    io_pList->Clear(TRUE);

    struct ifaddrs *l_pIfAddr = NULL;

    if (-1 == getifaddrs(&l_pIfAddr))
    {
        return FALSE;
    }

    for (struct ifaddrs *l_pAddr = l_pIfAddr; l_pAddr != NULL; l_pAddr = l_pAddr->ifa_next)
    {
        if (    (l_pAddr->ifa_addr == NULL)
             || (    (AF_INET != l_pAddr->ifa_addr->sa_family)
                  && (AF_INET6 != l_pAddr->ifa_addr->sa_family)
                )
           )
        {
            continue;
        }

        sockaddr_storage *l_pNew = new sockaddr_storage;
        if (AF_INET == l_pAddr->ifa_addr->sa_family)
        {
            memcpy(l_pNew, l_pAddr->ifa_addr, sizeof(sockaddr_in));
        }
        else if (AF_INET6 == l_pAddr->ifa_addr->sa_family)
        {
            memcpy(l_pNew, l_pAddr->ifa_addr, sizeof(sockaddr_in6));
        }
        else
        {
            memset(l_pNew, 0, sizeof(sockaddr_storage));
        }

        io_pList->Add_After(NULL, l_pNew);
    }

    freeifaddrs(l_pIfAddr);

    return TRUE;
}//PEnumIps


////////////////////////////////////////////////////////////////////////////////
//Print_SAddr
static __attribute__ ((unused)) tBOOL Print_SAddr(sockaddr *i_pAddress, XCHAR *o_pIP, tUINT32 i_dwLen)
{
    tBOOL l_bReturn = FALSE;

    if (    (NULL == i_pAddress)
         || (NULL == o_pIP)
         || (15   >= i_dwLen)   
       )
    {
        return l_bReturn;
    }
    
    if (AF_INET6 == i_pAddress->sa_family) 
    {
        if (NULL != inet_ntop(i_pAddress->sa_family, 
                              &((sockaddr_in6*)i_pAddress)->sin6_addr,
                              o_pIP,
                              i_dwLen
                             )
           )
        {
            l_bReturn = TRUE;
        }
    }
    else if (AF_INET == i_pAddress->sa_family) 
    {
        if (NULL != inet_ntop(i_pAddress->sa_family, 
                              &((sockaddr_in*)i_pAddress)->sin_addr,
                              o_pIP,
                              i_dwLen
                             )
           )
        {
            l_bReturn = TRUE;
        }
    }
    
    //WSAAddressToString(i_pAddress, 
    //                    m_dwAddress_Size, 
    //                    NULL, 
    //                    l_pIP, 
    //                    &l_dwSize);
    
    return l_bReturn;
}//Print_SAddr


////////////////////////////////////////////////////////////////////////////////
//Disable_PortUnreachable_ICMP
static __attribute__ ((unused)) tBOOL Disable_PortUnreachable_ICMP(tSOCKET i_hSocket)
{
    UNUSED_ARG(i_hSocket);

    //tUINT32  l_dwBytesReturned = 0;
    //tBOOL    l_bNewBehavior    = FALSE;
    //tUINT32  l_dwStatus        = 0;
    //
    //
    //l_dwStatus = WSAIoctl(m_hSocket, 
    //                    SIO_UDP_CONNRESET,
    //                    &l_bNewBehavior, 
    //                    sizeof(l_bNewBehavior),
    //                    NULL, 
    //                    0,
    //                    &l_dwBytesReturned,
    //                    NULL,
    //                    NULL
    //                    );
    //return (SOCKET_ERROR == l_dwStatus) ? FALSE : TRUE;
   
    //don't know how to do it under Linux yet.
    return TRUE;
}//Disable_PortUnreachable_ICMP

#endif //PSOCKET_H
