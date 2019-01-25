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

#define TIME_HRS_100NS                                            36000000000ull
#define TIME_MIN_100NS                                              600000000ull
#define TIME_SEC_100NS                                               10000000ull
#define TIME_MSC_100NS                                                  10000ull

////////////////////////////////////////////////////////////////////////////////
//GetTickCount - exist in OS
//tUINT32 GetTickCount()
//{
//}//GetTickCount


////////////////////////////////////////////////////////////////////////////////
//GetPerformanceCounter
static inline tUINT64 GetPerformanceCounter()
{
    LARGE_INTEGER l_qwValue;
    l_qwValue.QuadPart = 0;
    QueryPerformanceCounter(&l_qwValue);
    return l_qwValue.QuadPart;
}//GetPerformanceCounter


////////////////////////////////////////////////////////////////////////////////
//GetPerformanceFrequency
static inline tUINT64 GetPerformanceFrequency()
{
    LARGE_INTEGER l_qwValue;
    l_qwValue.QuadPart = 0;
    QueryPerformanceFrequency(&l_qwValue);
    return l_qwValue.QuadPart;
}//GetPerformanceFrequency


////////////////////////////////////////////////////////////////////////////////
//GetEpochTime
//return a 64-bit value of 100-nanosecond intervals since January 1, 1601 (UTC).
static inline void GetEpochTime(tUINT32 *o_pHi, tUINT32 *o_pLow)
{
    SYSTEMTIME l_sSTime = {0};
    FILETIME   l_sFTime = {0};
    GetSystemTime(&l_sSTime);
    
    SystemTimeToFileTime(&l_sSTime, &l_sFTime);
    if (o_pHi)
    {
        *o_pHi = l_sFTime.dwHighDateTime;
    }
    
    if (o_pLow)
    {
        *o_pLow = l_sFTime.dwLowDateTime;
    }
}//GetEpochTime


////////////////////////////////////////////////////////////////////////////////
//GetLocalTime
//convert a 64-bit value of 100-nanosecond intervals since January 1, 1601 (UTC)
//to readable form
static inline void GetLocalTime(tUINT64  i_qwTime, 
                                tUINT32 &o_rYear, 
                                tUINT32 &o_rMonth,
                                tUINT32 &o_rDay,
                                tUINT32 &o_rHour,
                                tUINT32 &o_rMinutes,
                                tUINT32 &o_rSeconds,
                                tUINT32 &o_rMilliseconds,
                                tUINT32 &o_rMicroseconds,
                                tUINT32 &o_rNanoseconds
                               )
{
    tUINT32 l_dwReminder = i_qwTime % TIME_MSC_100NS; //micro & 100xNanoseconds
    tUINT32 l_dwNano     = i_qwTime % 10;
    tUINT32 l_dwMicro    = l_dwReminder / 10;

    i_qwTime -= l_dwReminder;

    SYSTEMTIME l_sTime      = {0};
    FILETIME   l_sLocalTime = {0};

    FileTimeToLocalFileTime((FILETIME*)&i_qwTime, &l_sLocalTime);
    FileTimeToSystemTime(&l_sLocalTime, &l_sTime);

    o_rYear         = l_sTime.wYear;
    o_rMonth        = l_sTime.wMonth;
    o_rDay          = l_sTime.wDay;
    o_rHour         = l_sTime.wHour;
    o_rMinutes      = l_sTime.wMinute;
    o_rSeconds      = l_sTime.wSecond;
    o_rMilliseconds = l_sTime.wMilliseconds;
    o_rMicroseconds = l_dwMicro;
    o_rNanoseconds  = l_dwNano;
}//GetEpochTime


////////////////////////////////////////////////////////////////////////////////
//GetSecondOfDay
static inline tUINT32 GetSecondOfDay()
{
    SYSTEMTIME l_sTime = {0};

    GetLocalTime(&l_sTime);

    return 3600 * (tUINT32)l_sTime.wHour + (tUINT32)l_sTime.wMinute * 60 + (tUINT32)l_sTime.wSecond;
}//GetSecondOfDay


////////////////////////////////////////////////////////////////////////////////
//GetEpochTime
//return a 64-bit value of 100-nanosecond intervals since January 1, 1601 (UTC).
static inline tUINT64 GetEpochTime()
{
    SYSTEMTIME l_sSTime = {0};
    FILETIME   l_sFTime = {0};
    GetSystemTime(&l_sSTime);
    
    SystemTimeToFileTime(&l_sSTime, &l_sFTime);

    return (((tUINT64)l_sFTime.dwHighDateTime) << 32ull) + (tUINT64)l_sFTime.dwLowDateTime;
}//GetEpochTime


////////////////////////////////////////////////////////////////////////////////
//GetEpochLocalTime
//return a 64-bit value of 100-nanosecond intervals since January 1, 1601 (UTC).
static inline tUINT64 GetEpochLocalTime()
{
    SYSTEMTIME l_sSTime = {0};
    FILETIME   l_sFTime = {0};
    GetLocalTime(&l_sSTime);
    
    SystemTimeToFileTime(&l_sSTime, &l_sFTime);

    return (((tUINT64)l_sFTime.dwHighDateTime) << 32ull) + (tUINT64)l_sFTime.dwLowDateTime;
}//GetEpochTime
