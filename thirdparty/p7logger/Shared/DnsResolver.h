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
#ifndef DNS_RESOLVER_H
#define DNS_RESOLVER_H

#include "AList.h"
#include "RBTree.h"
#include "PSocket.h"
#include "PAtomic.h"
#include "PLock.h"
#include "IMEvent.h"
#include "PMEvent.h"
#include "PThreadShell.h"
#include "PTime.h"

#if !defined(NI_MAXHOST)
    #define NI_MAXHOST      1025  /* Max size of a fully-qualified domain name */
#endif

#define DNS_RESOLVE_TIME_OUT_MIN                    (50)
#define DNS_RESOLVE_TIME_OUT_MAX                    (500)

#define DNS_RESOLVER_EXIT_EVENT                     (MEVENT_SIGNAL_0)
#define DNS_RESOLVER_EVENT                          (MEVENT_SIGNAL_0 + 1)

#define DNS_NAME_EVENT                              (MEVENT_SIGNAL_0)


////////////////////////////////////////////////////////////////////////////////
//CDNS_Item
class CDNS_Item
{                            
    tXCHAR          *m_pName;      //host name 
    tXCHAR           m_pIP[96];
    size_t           m_szName_Max; //host name max length
    size_t           m_szName;     //host name current length
    sockaddr_storage m_sSAS;       //host Socket address V4/V6
    CMEvent          m_cEvent; 
public:
    ////////////////////////////////////////////////////////////////////////////
    //CDNS_Item
    CDNS_Item(sockaddr *i_pSA)
        : m_pName(NULL)
        , m_szName_Max(0)
        , m_szName(0)
    {
        if (AF_INET == i_pSA->sa_family)
        {
            memcpy(&m_sSAS, i_pSA, sizeof(sockaddr_in));
            ((sockaddr_in*)&m_sSAS)->sin_port = 0;
        }
        else if (AF_INET6 == i_pSA->sa_family)
        {
            memcpy(&m_sSAS, i_pSA, sizeof(sockaddr_in6));
            ((sockaddr_in6*)&m_sSAS)->sin6_port = 0;
        }

        if (Print_SAddr((sockaddr*)&m_sSAS, m_pIP, LENGTH(m_pIP)))
        {
            size_t l_szIP = PStrLen(m_pIP);

            for (size_t l_szI = 0; l_szI < l_szIP; l_szI ++)
            {
                if (TM(':') == m_pIP[l_szI])
                {
                    m_pIP[l_szI] = TM('x');
                }
            }
        }
        else
        {
            PStrCpy(m_pIP, LENGTH(m_pIP), TM("Unknown")); 
        }

        m_cEvent.Init(1, EMEVENT_SINGLE_AUTO);
    }//CDNS_Item

    ////////////////////////////////////////////////////////////////////////////
    //~CDNS_Item
    ~CDNS_Item()
    {
        if (m_pName)
        {
            delete [] m_pName;
            m_pName = NULL;
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    //Get_SA
    const sockaddr *Get_SA()
    {
        return (sockaddr*)&m_sSAS;
    }//Get_SA


    ////////////////////////////////////////////////////////////////////////////
    //Get_Event
    CMEvent *Get_Event()
    {
        return &m_cEvent;
    }//Get_Event

    ////////////////////////////////////////////////////////////////////////////
    //Update
    tBOOL Update(tXCHAR *i_pName)
    {
        tXCHAR *l_pName  = (i_pName) ? i_pName : m_pIP;
        size_t   l_szName = PStrLen(l_pName);

        if (l_szName > m_szName_Max)
        {
            if (m_pName)
            {
                delete [] m_pName;
                m_pName = NULL;
            }

            m_szName_Max = (l_szName + 1 + 15) & (~15UL);
            m_pName      = new tXCHAR[m_szName_Max];
        }

        if (NULL != m_pName)
        {
            m_szName = l_szName;
            PStrCpy(m_pName, m_szName_Max, l_pName);
        }
        else
        {
            m_szName     = 0;
            m_szName_Max = 0;
        }

        return (m_pName) ? TRUE : FALSE;
    }//Update


    ////////////////////////////////////////////////////////////////////////////
    //Get_Name()
    tBOOL Get_Name(tXCHAR *o_pName, size_t i_szLength_Max)
    {
        if (    (o_pName)
             && (1 < i_szLength_Max)
           )
        {
            if (m_szName >= i_szLength_Max)
            {
                PStrNCpy(o_pName, i_szLength_Max, m_pName, i_szLength_Max - 1);
            }
            else
            {
                PStrCpy(o_pName, i_szLength_Max, m_pName);
            }

            return TRUE;
        }

        return FALSE;
    }//Get_Name()

    ////////////////////////////////////////////////////////////////////////////
    //Is_Greater -> (m_sSA > i_pSA) == TRUE
    tBOOL Is_Greater(sockaddr *i_pSA)
    {
        if (m_sSAS.ss_family > i_pSA->sa_family)
        {
            return TRUE;
        }
        else if (m_sSAS.ss_family < i_pSA->sa_family)
        {
            return FALSE;
        }

        if (AF_INET  == i_pSA->sa_family)
        {
            return (((sockaddr_in*)&m_sSAS)->sin_addr.s_addr > ((sockaddr_in*)i_pSA)->sin_addr.s_addr);
        }
        else if (AF_INET6 == i_pSA->sa_family)
        {
            return (0 < memcmp(((sockaddr_in6*)&m_sSAS)->sin6_addr.s6_addr,
                               ((sockaddr_in6*)i_pSA)->sin6_addr.s6_addr,
                               sizeof(in6_addr)
                              )
                   );
        }

        return FALSE;
    }//Is_Greater

    ////////////////////////////////////////////////////////////////////////////
    //Is_Equal
    tBOOL Is_Equal(sockaddr *i_pSA)
    {
        if (m_sSAS.ss_family == i_pSA->sa_family)
        {
            if (AF_INET  == i_pSA->sa_family)
            {
                return (((sockaddr_in*)&m_sSAS)->sin_addr.s_addr == ((sockaddr_in*)i_pSA)->sin_addr.s_addr);
            }
            else if (AF_INET6 == i_pSA->sa_family)
            {
                return (0 == memcmp(((sockaddr_in6*)&m_sSAS)->sin6_addr.s6_addr,
                                    ((sockaddr_in6*)i_pSA)->sin6_addr.s6_addr,
                                    sizeof(in6_addr)
                                   )
                       );
            }
        }

        return FALSE;
    }//Is_Equal
};
                              

////////////////////////////////////////////////////////////////////////////////
//CDNS_Resolver
class CDNS_Resolver:
    public CRBTree<CDNS_Item*, sockaddr*>
{
    volatile tINT32    m_lTimeOut;
    tLOCK              m_sCS; 
    CThShell::tTHREAD  m_hThread;
    tBOOL              m_bIsThread;
    CMEvent            m_cEvent;
    CBList<CDNS_Item*> m_cRecords;
    CBList<CDNS_Item*> m_cResolving;
public:
    ////////////////////////////////////////////////////////////////////////////
    //CDNS_Resolver()
    CDNS_Resolver()
        : CRBTree<CDNS_Item*, sockaddr*>(32, FALSE)
        , m_lTimeOut(DNS_RESOLVE_TIME_OUT_MAX)
        , m_bIsThread(FALSE)
        , m_cRecords(32)
    {
        LOCK_CREATE(m_sCS);

        m_cEvent.Init(2, EMEVENT_SINGLE_AUTO, EMEVENT_MULTI);

        if (CThShell::Create(&Static_Routine, this, &m_hThread, TM("DNSR")))
        {
            m_bIsThread = TRUE;
        }

    }//~CDNS_Resolver()

    ////////////////////////////////////////////////////////////////////////////
    //~CDNS_Resolver()
    ~CDNS_Resolver()
    {
        if (m_bIsThread)
        {
            m_cEvent.Set(DNS_RESOLVER_EXIT_EVENT);
            if (TRUE == CThShell::Close(m_hThread, 15000))
            {
                m_hThread   = 0;//NULL;
                m_bIsThread = FALSE;
            }
        }

        m_cResolving.Clear(FALSE);
        m_cRecords.Clear(TRUE);

        LOCK_DESTROY(m_sCS);
    }//~CDNS_Resolver()

    ////////////////////////////////////////////////////////////////////////////
    //Get_Name()
    tBOOL Get_Name(sockaddr *i_pKey, tXCHAR *o_pName, size_t i_szName, tBOOL *o_pNew)
    {
        CDNS_Item *l_pRec    = NULL; 
        tBOOL     l_bAdd    = FALSE;

        if (    (NULL == i_pKey)
             || (    (AF_INET  != i_pKey->sa_family)
                  && (AF_INET6 != i_pKey->sa_family)
                )
             || (NULL == o_pName)
             || (16 > i_szName)
           )
        {
            return FALSE;
        }

        LOCK_ENTER(m_sCS);
        l_pRec = this->Find(i_pKey);
        if (NULL == l_pRec)
        {
            l_pRec = new CDNS_Item(i_pKey);
            if (l_pRec)
            {
                this->Push(l_pRec, i_pKey);
                m_cRecords.Add_After(m_cRecords.Get_Last(), l_pRec);
                l_pRec->Update(NULL);
                l_bAdd = TRUE;
            }

            if (o_pNew)
            {
                *o_pNew = 1;
            }
        }
        LOCK_EXIT(m_sCS);

        if (l_bAdd)
        {
            LOCK_ENTER(m_sCS);
            m_cResolving.Add_After(m_cResolving.Get_Last(), l_pRec);
            m_cEvent.Set(DNS_RESOLVER_EVENT);
            LOCK_EXIT(m_sCS);

            l_pRec->Get_Event()->Wait((tUINT32)m_lTimeOut);
        }//if (NULL == l_pRec)

        LOCK_ENTER(m_sCS);
        if (l_pRec)
        {
            l_pRec->Get_Name(o_pName, i_szName);
        }
        LOCK_EXIT(m_sCS);

        return (NULL != l_pRec);
    }//Get_Name()

protected:
    //////////////////////////////////////////////////////////////////////////// 
    //Return
    //TRUE  - if (i_pKey < i_pData::key)
    //FALSE - otherwise
    tBOOL Is_Key_Less(sockaddr *i_pKey, CDNS_Item *i_pData) 
    {
        return i_pData->Is_Greater(i_pKey);
    }

    //////////////////////////////////////////////////////////////////////////// 
    //Return
    //TRUE  - if (i_pKey == i_pData::key)
    //FALSE - otherwise
    tBOOL Is_Qual(sockaddr *i_pKey, CDNS_Item *i_pData) 
    {
        return i_pData->Is_Equal(i_pKey);
    }

private:
    //////////////////////////////////////////////////////////////////////////// 
    //Routine_Static
    static THSHELL_RET_TYPE THSHELL_CALL_TYPE Static_Routine(void *i_pContext)
    {
        CDNS_Resolver * l_pRoutine = (CDNS_Resolver *)i_pContext;
        if (l_pRoutine)
        {
            l_pRoutine->Routine();
        }

        CThShell::Cleanup();
        return THSHELL_RET_OK;
    } 

    //////////////////////////////////////////////////////////////////////////// 
    //Routine
    void Routine()
    {
        tXCHAR      l_pName[NI_MAXHOST];
        tBOOL       l_bExit     = FALSE;
        pAList_Cell l_pEl       = NULL;
        CDNS_Item  *l_pRec      = NULL; 
        tUINT32     l_dwWFMOR   = 0;
        tINT32      l_lWaitMax  = -1;
        tUINT32     l_dwTick    = 0;

        while (FALSE == l_bExit)
        {
            l_dwWFMOR = m_cEvent.Wait(1000);

            if (DNS_RESOLVER_EXIT_EVENT == l_dwWFMOR)
            {
                l_bExit = TRUE;
            }
            else if (DNS_RESOLVER_EVENT  == l_dwWFMOR)
            {
                LOCK_ENTER(m_sCS);
                l_pRec = m_cResolving.Get_Data(m_cResolving.Get_First());
                m_cResolving.Del(m_cResolving.Get_First(), FALSE);
                LOCK_EXIT(m_sCS);

                if (l_pRec)
                {
                    l_dwTick = GetTickCount();
                    if (Resolve_SA(l_pRec->Get_SA(), l_pName, LENGTH(l_pName)))
                    {
                        l_dwTick = CTicks::Difference(GetTickCount(), l_dwTick);
                        LOCK_ENTER(m_sCS);
                        l_pRec->Update(l_pName);
                        LOCK_EXIT(m_sCS);
                    }
                    else
                    {
                        l_dwTick = 0;

                        LOCK_ENTER(m_sCS);
                        l_pRec->Update(NULL);
                        LOCK_EXIT(m_sCS);
                    }
                    l_pRec->Get_Event()->Set(DNS_NAME_EVENT);
                }

                //Update waiting timeout based on maximum waiting of response 
                //from DNS server
                if ((tINT32)l_dwTick > l_lWaitMax)
                {
                    //set timeout with max limit of DNS_RESOLVE_TIME_OUT
                    if (l_dwTick > DNS_RESOLVE_TIME_OUT_MAX)
                    {
                        l_lWaitMax = DNS_RESOLVE_TIME_OUT_MAX;
                    }
                    else if (l_dwTick < DNS_RESOLVE_TIME_OUT_MIN)
                    {
                        l_lWaitMax = DNS_RESOLVE_TIME_OUT_MIN;
                    }
                    else
                    {
                        l_lWaitMax = (tINT32)l_dwTick;
                    }
                    
                    if (l_lWaitMax != m_lTimeOut)
                    {
                        ATOMIC_SET(&m_lTimeOut, l_lWaitMax);
                    }
                }
            }
            else if (MEVENT_TIME_OUT == l_dwWFMOR)
            {
                LOCK_ENTER(m_sCS);
                l_pEl = m_cRecords.Get_Next(l_pEl);
                l_pRec = m_cRecords.Get_Data(l_pEl);
                LOCK_EXIT(m_sCS);

                if (l_pRec)
                {
                    if (Resolve_SA(l_pRec->Get_SA(), l_pName, LENGTH(l_pName)))
                    {
                        LOCK_ENTER(m_sCS);
                        l_pRec->Update(l_pName);
                        LOCK_EXIT(m_sCS);
                    }
                    else
                    {
                        LOCK_ENTER(m_sCS);
                        l_pRec->Update(NULL);
                        LOCK_EXIT(m_sCS);
                    }
                }
            }
        }
    }//Routine

    //////////////////////////////////////////////////////////////////////////// 
    //Resolve_SA
    tBOOL Resolve_SA(const sockaddr *i_pSA, tXCHAR *o_pName, tUINT32 i_dwLength)
    {
        tUINT32 l_dwIP_Size = 0;
        tBOOL   l_bReturn   = FALSE;

        if (NULL == i_pSA)
        {
            return l_bReturn;
        }


        if (AF_INET == i_pSA->sa_family)
        {
            l_dwIP_Size = sizeof(sockaddr_in);
        }
        else if (AF_INET6 == i_pSA->sa_family)
        {
            l_dwIP_Size = sizeof(sockaddr_in6);
        }                                             

    #ifdef UTF8_ENCODING 
        int l_iRes = getnameinfo(i_pSA, l_dwIP_Size, 
                                 o_pName, i_dwLength, 
                                 NULL, 0, 
                                 NI_NAMEREQD
                                );
    #else
        int l_iRes = GetNameInfoW(i_pSA, l_dwIP_Size, 
                                  o_pName, i_dwLength, 
                                  NULL, 0, 
                                  NI_NAMEREQD
                                 );
    #endif                             


        if (0 == l_iRes) 
        {
            l_bReturn = TRUE;
        }

        return l_bReturn;
    }//Resolve_SA
};//CDNS_Resolver

#endif //DNS_RESOLVER_H