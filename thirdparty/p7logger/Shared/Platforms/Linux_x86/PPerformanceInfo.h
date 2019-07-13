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

////////////////////////////////////////////////////////////////////////////////
class CPerformanceInfo
    : public IPerformanceInfo
{
    volatile tINT32 m_iRCnt;
public:
    ////////////////////////////////////////////////////////////////////////////
    CPerformanceInfo(const tXCHAR *i_pProcessName)
        : m_iRCnt(1)
    {
        UNUSED_ARG(i_pProcessName);
    }

    virtual ~CPerformanceInfo()
    {
    }

    tBOOL Refresh()
    {
        return TRUE;
    }

    tINT64 Get(IPerformanceInfo::eCounter i_eCounter)
    {
        UNUSED_ARG(i_eCounter);
        return 0;
    }

    ////////////////////////////////////////////////////////////////////////////
    tINT32  Add_Ref()
    {
        return ATOMIC_INC(&m_iRCnt);
    }
    
    ////////////////////////////////////////////////////////////////////////////
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
