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
#ifndef PMEVENT_H
#define PMEVENT_H

#include <semaphore.h>

// Infinite timeout
#define INFINITE                                                      0xFFFFFFFF  

//#define PMEVENT_COND_VARIABLE

////////////////////////////////////////////////////////////////////////////////
//CMEvent - class designed to avoid problem of "wait" POSIX functions. They are 
//          based on abs. time and they are affected by system time changes.
class CMEvent
    : public IMEvent //virtual
{
    struct sEvent
    {
        volatile tINT32 iCounter;
        eMEvent_Type    eType;
        tUINT32         dwIdx;
        sEvent         *pNext; 
    };
        
    pthread_mutex_t  m_sMutex;
    
#ifdef PMEVENT_COND_VARIABLE    
    pthread_cond_t   m_sCV;
#else
    sem_t            m_sSem;
#endif    
    
    tUINT8           m_bCount;
    tBOOL            m_bInit;
    tBOOL            m_bError;
    sEvent          *m_pEvents;
    sEvent          *m_pEvent_Cur; 
    tINT32           m_iSignals;
public:
    ////////////////////////////////////////////////////////////////////////////
    //CMEvent
    CMEvent()
        : m_bCount(0)
        , m_bInit(FALSE)
        , m_bError(FALSE)
        , m_pEvents(NULL)
        , m_pEvent_Cur(NULL)
        , m_iSignals(0)
    {
    }//CMEvent


    ////////////////////////////////////////////////////////////////////////////
    //~CMEvent
    virtual ~CMEvent()
    {
        if (m_bInit)
        {
            pthread_mutex_destroy(&m_sMutex);
            
#ifdef PMEVENT_COND_VARIABLE    
            pthread_cond_destroy(&m_sCV);
#else
            sem_destroy(&m_sSem);
#endif                
            
        }
        
        Cleanup();

        m_bCount = 0;
    }//~CMEvent


    ////////////////////////////////////////////////////////////////////////////
    //Init
    tBOOL Init(tUINT8 i_bCount, ...)
    {
        va_list            l_pVA;
        tUINT8             l_bIDX   = 0;
        tBOOL              l_bCA    = FALSE;
        tBOOL              l_bSig   = FALSE;
        tBOOL              l_bMutex = FALSE;
        pthread_condattr_t l_sCA;
        

        if (    (TRUE == m_bInit)
             || (TRUE == m_bError)   
             || (0    >= i_bCount)
           )
        {
            return FALSE;
        }
        
        
#ifdef PMEVENT_COND_VARIABLE    
        ////////////////////////////////////////////////////////////////////////
        //initialize Cond.Variable
        clockid_t l_iClock = CLOCK_REALTIME;

        if (0 != pthread_condattr_init(&l_sCA))
        {
            goto l_lExit; //error
        }
        else
        {
            l_bCA = TRUE;
        }
        
        //#ifdef __USE_XOPEN2K
        if (0 != pthread_condattr_setclock(&l_sCA, CLOCK_MONOTONIC))
        {
            goto l_lExit; //error
        }
        
        if (    (0 != pthread_condattr_getclock(&l_sCA, &l_iClock))
             || (CLOCK_MONOTONIC != l_iClock)    
           )
        {
            goto l_lExit; //error
        }
        //#else
        //#endif
        
        if (0 != pthread_cond_init(&m_sCV, &l_sCA))
        {
            goto l_lExit; //error
        }
        else
        {
            l_bSig = TRUE;
        }
#else
        ////////////////////////////////////////////////////////////////////////
        //initialize semaphore
        if (0 != sem_init(&m_sSem, 0, 0))
        {
            goto l_lExit; //error
        }
        else
        {
            l_bSig = TRUE;
        }
#endif    
        

        ////////////////////////////////////////////////////////////////////////
        //initialize mutex
        if (0 != pthread_mutex_init(&m_sMutex, NULL))
        {
            m_bError = TRUE;
            goto l_lExit;
        }
        else
        {
            l_bMutex = TRUE;
        }
        
        ////////////////////////////////////////////////////////////////////////
        //initialize events structure
        m_pEvents = new sEvent[i_bCount];
        if (NULL == m_pEvents)
        {
            goto l_lExit;
        }

        memset(m_pEvents, 0, sizeof(sEvent) * i_bCount);
        
        m_pEvent_Cur = &m_pEvents[0];

        va_start(l_pVA, i_bCount);
        while (l_bIDX < i_bCount)
        {
            m_pEvents[l_bIDX].eType    = (eMEvent_Type)va_arg(l_pVA, /*eMEvent_Type*/int);
            m_pEvents[l_bIDX].iCounter = 0;
            m_pEvents[l_bIDX].dwIdx    = l_bIDX;
            
            //make round trip links
            if ((l_bIDX + 1) < i_bCount)           
            {
                m_pEvents[l_bIDX].pNext = &m_pEvents[l_bIDX + 1];
            }
            else
            {
                m_pEvents[l_bIDX].pNext = &m_pEvents[0];
            }

            l_bIDX++;
        }

        va_end(l_pVA);

        m_bInit  = TRUE;
        m_bCount = i_bCount;

    l_lExit:
        if (l_bCA)           
        {
            pthread_condattr_destroy(&l_sCA);
        }
   
        if (FALSE == m_bInit)
        {
            m_bError = TRUE;

            if (l_bMutex)
            {
                pthread_mutex_destroy(&m_sMutex);
            }
            
            if (l_bSig)   
            {
#ifdef PMEVENT_COND_VARIABLE    
                pthread_cond_destroy(&m_sCV);
#else
                sem_destroy(&m_sSem);
#endif                
            }
            
            Cleanup();
        }

        return m_bInit;
    }//Init


    ////////////////////////////////////////////////////////////////////////////
    //Set
    tBOOL Set(tUINT32 i_dwID)
    {
        if (    (i_dwID >= m_bCount)
             || (FALSE  == m_bInit)  
           )
        {
            return FALSE;
        }
        
        pthread_mutex_lock(&m_sMutex);
        
        m_iSignals++;
        m_pEvents[i_dwID].iCounter ++;

#ifdef PMEVENT_COND_VARIABLE    
        pthread_cond_signal(&m_sCV);
#else
        sem_post(&m_sSem);
#endif                
        
        pthread_mutex_unlock(&m_sMutex);

        return TRUE;
    }//Set


    ////////////////////////////////////////////////////////////////////////////
    //Clr
    tBOOL Clr(tUINT32 i_dwID)
    {
        tBOOL l_bReturn = FALSE;
        if (    (i_dwID >= m_bCount)
             || (EMEVENT_SINGLE_MANUAL != m_pEvents[i_dwID].eType)
           )
        {
            return l_bReturn;
        }
        
        pthread_mutex_lock(&m_sMutex);
        
        if (m_pEvents[i_dwID].iCounter)
        {
            m_iSignals --;
            m_pEvents[i_dwID].iCounter --;

            sem_trywait(&m_sSem); //decrease semaphore
            
            l_bReturn = TRUE;
        }
        
        pthread_mutex_unlock(&m_sMutex);
             
        return l_bReturn;
    }//Clr

    
    ////////////////////////////////////////////////////////////////////////////
    //Wait
    tUINT32 Wait()
    {
        tUINT32 l_dwReturn = MEVENT_TIME_OUT;
        
        ////////////////////////////////////////////////////////////////////////
        //LOCK

#ifdef PMEVENT_COND_VARIABLE    
        int     l_iCV_Res  = 0;

        pthread_mutex_lock(&m_sMutex);
        
        while (0 == m_iSignals)
        {
            l_iCV_Res = pthread_cond_wait(&m_sCV, &m_sMutex);
            if (0 != l_iCV_Res)
            {
                break;
            }
        }

        l_dwReturn = Get_Signal();
        
        pthread_mutex_unlock(&m_sMutex);
#else
        sem_wait(&m_sSem);

        pthread_mutex_lock(&m_sMutex);
        l_dwReturn = Get_Signal();
        pthread_mutex_unlock(&m_sMutex);
#endif                
        
        
        //UNLOCK
        ////////////////////////////////////////////////////////////////////////
        
        return l_dwReturn;
    }//Wait

    
    ////////////////////////////////////////////////////////////////////////////
    //Wait
    tUINT32 Wait(tUINT32 i_dwMSec)
    {
        tUINT64         l_qwNano   = (tUINT64)i_dwMSec * 1000000ULL;
        tUINT32         l_dwReturn = MEVENT_TIME_OUT;
        struct timespec l_sTime    = {0, 0};
        
        ////////////////////////////////////////////////////////////////////////
        //LOCK

#ifdef PMEVENT_COND_VARIABLE    
        int l_iRes  = 0;
        pthread_mutex_lock(&m_sMutex);
        clock_gettime(CLOCK_MONOTONIC, &l_sTime);
        
        l_qwNano       += l_sTime.tv_nsec;
        l_sTime.tv_sec += static_cast<__time_t>(l_qwNano / 1000000000ULL);
        l_sTime.tv_nsec = static_cast<long int>(l_qwNano % 1000000000ULL);
        
        while (0 == m_iSignals)
        {
            //ETIMEDOUT
            l_iRes = pthread_cond_timedwait(&m_sCV, &m_sMutex, &l_sTime);
            if (0 != l_iRes)
            {
                break;
            }
        }
        
        l_dwReturn = Get_Signal();
        pthread_mutex_unlock(&m_sMutex);
#else
        clock_gettime(CLOCK_REALTIME, &l_sTime);
        
        l_qwNano       += l_sTime.tv_nsec;
        l_sTime.tv_sec += static_cast<time_t>(l_qwNano/1000000000ULL);
        l_sTime.tv_nsec = static_cast<long>(l_qwNano%1000000000ULL);

        if (0 == sem_timedwait(&m_sSem, &l_sTime))
        {
            pthread_mutex_lock(&m_sMutex);
            l_dwReturn = Get_Signal();
            pthread_mutex_unlock(&m_sMutex);
        }
#endif        
        
        //UNLOCK
        ////////////////////////////////////////////////////////////////////////
        
        return l_dwReturn;
    }//Wait

private:
    ////////////////////////////////////////////////////////////////////////////
    //Get_Signal
    tUINT32 Get_Signal()
    {
        tUINT32  l_dwReturn = MEVENT_TIME_OUT;
        
        if (0 == m_iSignals) //if we register some signal
        {
            return l_dwReturn;
        }
        
        sEvent *l_pStart = m_pEvent_Cur;
        //we will walk through all events in attempt to find signal
        do
        {
            m_pEvent_Cur = m_pEvent_Cur->pNext;

            if (m_pEvent_Cur->iCounter)
            {
                l_dwReturn = m_pEvent_Cur->dwIdx;
                if (EMEVENT_SINGLE_MANUAL == m_pEvent_Cur->eType)
                {
                    sem_post(&m_sSem); //return sem value to signal again
                }
                else
                {
                    m_pEvent_Cur->iCounter --;
                    m_iSignals --;
                }
                
                break;
            }

            //if we do round trip  
        } while (l_pStart != m_pEvent_Cur);

        if (MEVENT_TIME_OUT == l_dwReturn)
        {
            //error: signal event wasn't found !
            m_iSignals --;
        }
        
        return l_dwReturn;
    }//Get_Signal
    
    
    ////////////////////////////////////////////////////////////////////////////
    //Cleanup
    void Cleanup()
    {
        if (m_pEvents)
        {
            delete [] m_pEvents;
            m_pEvents = NULL;
        }
    }//Cleanup
};

#endif //PMEVENT_H
