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
#pragma once


////////////////////////////////////////////////////////////////////////////////
class CMEvent
    : virtual public IMEvent
{
    struct sMEvent
    {
        eMEvent_Type eType;
        HANDLE       hEvent;
    };

    tUINT8   m_bCount;
    sMEvent *m_pEvents;
    tBOOL    m_bInit;
    HANDLE  *m_pHandles;
public:
    ////////////////////////////////////////////////////////////////////////////
    //CMEvent
    CMEvent()
        : m_bCount(0)
        , m_pEvents(NULL)
        , m_bInit(FALSE)
        , m_pHandles(NULL)
    {
    }//CMEvent


    ////////////////////////////////////////////////////////////////////////////
    //~CMEvent
    virtual ~CMEvent()
    {
        Cleanup();

        m_bCount = 0;
    }//~CMEvent


    ////////////////////////////////////////////////////////////////////////////
    //Init
    tBOOL Init(tUINT8 i_bCount, ...)
    {
        if (    (TRUE == m_bInit)
             || (0    >= i_bCount)
           )
        {
            return FALSE;
        }

        va_list l_pVA  = NULL;
        tUINT8  l_bIDX = 0;

        m_pEvents = new sMEvent[i_bCount];
        if (NULL == m_pEvents)
        {
            goto l_lExit;
        }

        m_pHandles = new HANDLE[i_bCount];
        if (NULL == m_pHandles)
        {
            goto l_lExit;
        }

        memset(m_pEvents, 0, sizeof(sMEvent) * i_bCount);

        va_start(l_pVA, i_bCount);
        while (l_bIDX < i_bCount)
        {
            m_pEvents[l_bIDX].eType = (eMEvent_Type)va_arg(l_pVA, int); //Real type is "eMEvent_Type", MinGW warning

            if (EMEVENT_SINGLE_AUTO == m_pEvents[l_bIDX].eType)
            {
                m_pEvents[l_bIDX].hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
            }
            else if (EMEVENT_SINGLE_MANUAL == m_pEvents[l_bIDX].eType)
            {
                m_pEvents[l_bIDX].hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
            }
            else if (EMEVENT_MULTI == m_pEvents[l_bIDX].eType)
            {
                m_pEvents[l_bIDX].hEvent = CreateSemaphore(NULL, 0, 0xFFFFFF, NULL);
            }

            m_pHandles[l_bIDX] = m_pEvents[l_bIDX].hEvent;

            l_bIDX++;
        }

        va_end(l_pVA);

        m_bInit  = TRUE;
        m_bCount = i_bCount;

    l_lExit:

        if (FALSE == m_bInit)
        {
            Cleanup();
        }

        return m_bInit;
    }//Init


    ////////////////////////////////////////////////////////////////////////////
    //Set
    tBOOL Set(tUINT32 i_dwID)
    {
        if (i_dwID >= m_bCount)
        {
            return FALSE;
        }

        if (EMEVENT_MULTI == m_pEvents[i_dwID].eType)
        {
            ReleaseSemaphore(m_pEvents[i_dwID].hEvent, 1, NULL);
        }
        else
        {
            SetEvent(m_pEvents[i_dwID].hEvent);
        }

        return TRUE;
    }//Set


    ////////////////////////////////////////////////////////////////////////////
    //Clr
    tBOOL Clr(tUINT32 i_dwID)
    {
        if (    (i_dwID >= m_bCount)
             || (EMEVENT_SINGLE_MANUAL != m_pEvents[i_dwID].eType)
           )
        {
            return FALSE;
        }

        ResetEvent(m_pEvents[i_dwID].hEvent);
        return TRUE;
    }//Clr

    ////////////////////////////////////////////////////////////////////////////
    //Wait
    tUINT32 Wait()
    {
        return Wait(INFINITE);
    }//Wait

    ////////////////////////////////////////////////////////////////////////////
    //Wait
    tUINT32 Wait(tUINT32 i_dwMSec)
    {
        DWORD l_dwRes = WaitForMultipleObjects(m_bCount, 
                                               m_pHandles,
                                               FALSE, 
                                               i_dwMSec
                                              );

        if (    (WAIT_TIMEOUT == l_dwRes)
             || (WAIT_FAILED  == l_dwRes)
           )
        {
            return MEVENT_TIME_OUT;
        }


        return (MEVENT_SIGNAL_0 + (l_dwRes - WAIT_OBJECT_0));
    }//Wait

private:
    ////////////////////////////////////////////////////////////////////////////
    //Cleanup
    void Cleanup()
    {
        if (m_pHandles)
        {
            delete [] m_pHandles;
            m_pHandles = NULL; 
        }

        if (m_pEvents)
        {
            for (tUINT32 l_dwI = 0; l_dwI < m_bCount; l_dwI++)
            {
                if (m_pEvents[l_dwI].hEvent)
                {
                    CloseHandle(m_pEvents[l_dwI].hEvent);
                    m_pEvents[l_dwI].hEvent = NULL;
                }
            }

            delete [] m_pEvents;
            m_pEvents = NULL;
        }
    }//Cleanup
};
