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
#ifndef IPERFORMANCE_INFO_H
#define IPERFORMANCE_INFO_H


////////////////////////////////////////////////////////////////////////////////
class IPerformanceInfo
{
public:
    enum eCounter
    {
        eCounterSystemCpu = 0,
        eCounterProcessCpu,
        eCounterProcessHandles,
        eCounterProcessThreads,
        eCounterProcessMemory,

        eCounterTotal
    };

public:
    virtual tBOOL   Refresh()                                               = 0;
    virtual tINT64  Get(IPerformanceInfo::eCounter i_eCounter)              = 0;
    virtual tINT32  Add_Ref()                                               = 0;
    virtual tINT32  Release()                                               = 0;
};


#endif //IPERFORMANCE_INFO_H
