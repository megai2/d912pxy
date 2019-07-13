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
#ifndef PPERFORMANCE_INFO_H
#define PPERFORMANCE_INFO_H

#include "IPerformanceInfo.h"

#include <Pdh.h>
#include <pdhmsg.h>
#include <Psapi.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CPerformanceInfo
    : public IPerformanceInfo
{
    volatile tINT32 m_iRCnt;
    PDH_STATUS      m_hPdhStatus;
    HQUERY          m_hPdhQuery;
    HCOUNTER        m_hPdhCounter[IPerformanceInfo::eCounterTotal];
    SYSTEM_INFO     m_sSysInfo;

public:
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CPerformanceInfo(const tXCHAR *i_pProcessName)
        : m_iRCnt(1)
        , m_hPdhStatus(ERROR_SUCCESS)
        , m_hPdhQuery(NULL)
    {
        tXCHAR   l_pExeName[256];
        CWString l_cPhdPath;
        GetSystemInfo( &m_sSysInfo );
        memset(m_hPdhCounter, 0, sizeof(m_hPdhCounter));
        if (!m_sSysInfo.dwNumberOfProcessors)
        {
            m_sSysInfo.dwNumberOfProcessors = 1;
        }

        if (GetModuleBaseNameW(GetCurrentProcess(), NULL, l_pExeName, LENGTH(l_pExeName)))
        {
            tXCHAR *l_pExt = wcsrchr(l_pExeName, TM('.'));
            if (l_pExt)
            {
                *l_pExt = 0;
            }
        }
        else
        {
            PStrCpy(l_pExeName, LENGTH(l_pExeName), i_pProcessName);
        }

        l_cPhdPath.Realloc(4096);

        ////////////////////////////////////////////////////////////////////////////
        //create PHD counters
        //If counters is missing == PDH_CSTATUS_NO_OBJECT
        //http://blogs.msdn.com/b/martinv/archive/2008/11/18/missing-performance-counters.aspx
        //C:\Windows\System32> lodctr /R
        //After that all Performance Counters were available again!
        if (ERROR_SUCCESS == PdhOpenQuery(NULL, NULL, &m_hPdhQuery))
        {
            m_hPdhStatus = PdhAddEnglishCounterW(m_hPdhQuery, 
                                                 TM("\\Processor(_Total)\\% Processor Time"), 
                                                 0, 
                                                 &m_hPdhCounter[IPerformanceInfo::eCounterSystemCpu]);
            if (ERROR_SUCCESS != m_hPdhStatus)
            {
                m_hPdhCounter[IPerformanceInfo::eCounterSystemCpu] = NULL;
            }

            l_cPhdPath.Set(TM(""));
            swprintf_s(l_cPhdPath.Get(), l_cPhdPath.Max_Length(), TM("\\Process(%s)\\%% Processor Time"), l_pExeName);
            if (ERROR_SUCCESS != PdhAddEnglishCounterW(m_hPdhQuery, l_cPhdPath.Get(), 0, &m_hPdhCounter[IPerformanceInfo::eCounterProcessCpu]))
            {
                m_hPdhCounter[IPerformanceInfo::eCounterProcessCpu] = NULL;
            }

            l_cPhdPath.Set(TM(""));
            swprintf_s(l_cPhdPath.Get(), l_cPhdPath.Max_Length(), TM("\\Process(%s)\\Handle Count"), l_pExeName);
            if (ERROR_SUCCESS != PdhAddEnglishCounterW(m_hPdhQuery, l_cPhdPath.Get(), 0, &m_hPdhCounter[IPerformanceInfo::eCounterProcessHandles]))
            {
                m_hPdhCounter[IPerformanceInfo::eCounterProcessHandles] = NULL;
            }

            l_cPhdPath.Set(TM(""));
            swprintf_s(l_cPhdPath.Get(), l_cPhdPath.Max_Length(), TM("\\Process(%s)\\Thread Count"), l_pExeName);
            if (ERROR_SUCCESS != PdhAddEnglishCounterW(m_hPdhQuery, l_cPhdPath.Get(), 0, &m_hPdhCounter[IPerformanceInfo::eCounterProcessThreads]))
            {
                m_hPdhCounter[IPerformanceInfo::eCounterProcessThreads] = NULL;
            }

            l_cPhdPath.Set(TM(""));
            swprintf_s(l_cPhdPath.Get(), l_cPhdPath.Max_Length(), TM("\\Process(%s)\\Working Set"), l_pExeName);
            if (ERROR_SUCCESS != PdhAddEnglishCounterW(m_hPdhQuery, l_cPhdPath.Get(), 0, &m_hPdhCounter[IPerformanceInfo::eCounterProcessMemory]))
            {
                m_hPdhCounter[IPerformanceInfo::eCounterProcessMemory] = NULL;
            }
        }//if (ERROR_SUCCESS == PdhOpenQuery(NULL, NULL, &m_hPdhQuery))


    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual ~CPerformanceInfo()
    {
        if (m_hPdhCounter[IPerformanceInfo::eCounterProcessCpu])
        {
            PdhRemoveCounter(m_hPdhCounter[IPerformanceInfo::eCounterProcessCpu]);
            m_hPdhCounter[IPerformanceInfo::eCounterProcessCpu] = NULL;
        }

        if (m_hPdhCounter[IPerformanceInfo::eCounterSystemCpu])
        {
            PdhRemoveCounter(m_hPdhCounter[IPerformanceInfo::eCounterSystemCpu]);
            m_hPdhCounter[IPerformanceInfo::eCounterSystemCpu] = NULL;
        }


        if (m_hPdhCounter[IPerformanceInfo::eCounterProcessHandles])
        {
            PdhRemoveCounter(m_hPdhCounter[IPerformanceInfo::eCounterProcessHandles]);
            m_hPdhCounter[IPerformanceInfo::eCounterProcessHandles] = NULL;
        }

        if (m_hPdhCounter[IPerformanceInfo::eCounterProcessThreads])
        {
            PdhRemoveCounter(m_hPdhCounter[IPerformanceInfo::eCounterProcessThreads]);
            m_hPdhCounter[IPerformanceInfo::eCounterProcessThreads] = NULL;
        }

        if (m_hPdhCounter[IPerformanceInfo::eCounterProcessMemory])
        {
            PdhRemoveCounter(m_hPdhCounter[IPerformanceInfo::eCounterProcessMemory]);
            m_hPdhCounter[IPerformanceInfo::eCounterProcessMemory] = NULL;
        }

        if (m_hPdhQuery) 
        {
            PdhCloseQuery(m_hPdhQuery);
            m_hPdhQuery = NULL;
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    tBOOL Refresh()
    {
        if (    (m_hPdhQuery)
             && (ERROR_SUCCESS == PdhCollectQueryData(m_hPdhQuery))
           )
        {
            return TRUE;
        }

        return FALSE;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    tINT64 Get(IPerformanceInfo::eCounter i_eCounter)
    {
        PDH_FMT_COUNTERVALUE l_tPdhValue = {0};

        if (    (i_eCounter >= IPerformanceInfo::eCounterTotal)
             || (!m_hPdhCounter[i_eCounter])
           )
        {
            return 0;
        }

        if (ERROR_SUCCESS == PdhGetFormattedCounterValue(m_hPdhCounter[i_eCounter], PDH_FMT_LARGE, NULL, &l_tPdhValue))
        {
            if (IPerformanceInfo::eCounterProcessCpu == i_eCounter)
            {
                l_tPdhValue.largeValue /= m_sSysInfo.dwNumberOfProcessors;
            }
        }

        return l_tPdhValue.largeValue;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    tINT32  Add_Ref()
    {
        return ATOMIC_INC(&m_iRCnt);
    }
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    tINT32  Release()  
    {
        volatile tINT32 l_iReturn = ATOMIC_DEC(&m_iRCnt);

        if (0 >= l_iReturn)
        {
            delete this;
        }
        
        return l_iReturn;
    }
};

#endif //PPERFORMANCE_INFO_H
