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
#pragma once

#include "PTime.h"
#include "Ticks.h"

class CTickHi
{
    tUINT64 m_qwCounter;
    tUINT64 m_qwMarkIn;
    tUINT64 m_qwMarkOut;
    tUINT64 m_qwResolution;
public:
    CTickHi()
    {
        m_qwMarkIn     = 0ull;
        m_qwMarkOut    = 0ull;
        m_qwCounter    = 0ull;
        m_qwResolution = GetPerformanceFrequency();

        Reset();
        Start();
    }

    void Start()
    {
        m_qwMarkIn = GetPerformanceCounter();
    }

    void Stop()
    {
        m_qwMarkOut = GetPerformanceCounter();
        m_qwCounter += m_qwMarkOut - m_qwMarkIn;
    }

    void Reset()
    {
        m_qwCounter = 0;
    }

    //Second == 100 000;
    tUINT32 Get()
    {
        return (tUINT32)(m_qwCounter * 100000ULL / m_qwResolution);
    }
};

class CTickLow
{
    tUINT32 m_dwStart;
    tUINT32 m_dwStop;
public:
    CTickLow()
    {
        m_dwStart = 0;
        m_dwStop = 0;

        Reset();
        Start();
    }

    void Start()
    {
        m_dwStart = GetTickCount();
    }

    void Stop()
    {
        m_dwStop = GetTickCount();
    }

    void Reset()
    {
        m_dwStop = 0;
    }

    //frequency 1 000 per second
    tUINT32 Get()
    {
        return CTicks::Difference(m_dwStop, m_dwStart);
    }
};
