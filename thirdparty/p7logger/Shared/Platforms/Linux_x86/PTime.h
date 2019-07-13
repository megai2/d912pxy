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
#ifndef PTIME_H
#define PTIME_H

#include <sys/time.h>
#include <time.h>

//time offset from January 1, 1601 to January 1, 1970, resolution 100ns
#define TIME_OFFSET_1601_1970                            (116444736000000000ULL)

#define TIME_HRS_100NS                                            36000000000ull
#define TIME_MIN_100NS                                              600000000ull
#define TIME_SEC_100NS                                               10000000ull
#define TIME_MLSC_100NS                                                 10000ull
#define TIME_MCSC_100NS                                                    10ull


////////////////////////////////////////////////////////////////////////////////
//GetTickCount
static __attribute__ ((unused)) tUINT32 GetTickCount()
{
    tUINT64 l_qwReturn; //Warn:without initialization !
    //timeval l_sTime;
    //gettimeofday(&l_sTime, NULL);

    struct timespec l_sTime = {0, 0};
    clock_gettime(CLOCK_MONOTONIC, &l_sTime);
    
    l_qwReturn  = l_sTime.tv_sec;
    l_qwReturn *= 1000;
    l_qwReturn += l_sTime.tv_nsec/1000000;
    
    return (tUINT32)l_qwReturn;    
}//GetTickCount


////////////////////////////////////////////////////////////////////////////////
//GetPerformanceCounter
static __attribute__ ((unused)) tUINT64 GetPerformanceCounter()
{
    tUINT64 l_qwReturn      = 0;
    struct timespec l_sTime = {0, 0};
    
    clock_gettime(CLOCK_MONOTONIC, &l_sTime);
    
    l_qwReturn  = (tUINT64)(l_sTime.tv_sec) * 10000000;
    l_qwReturn += (tUINT64)(l_sTime.tv_nsec) / 100;
    
    return l_qwReturn;
}//GetPerformanceCounter


////////////////////////////////////////////////////////////////////////////////
//GetPerformanceFrequency
static __attribute__ ((unused)) tUINT64 GetPerformanceFrequency()
{
    return 10000000; //100 nano second
}//GetPerformanceFrequency


////////////////////////////////////////////////////////////////////////////////
//UnpackLocalTime
//convert a 64-bit value of 100-nanosecond intervals since January 1, 1601 (UTC)
//to readable form
static __attribute__ ((unused)) void UnpackLocalTime(tUINT64  i_qwTime,
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
    tUINT32 l_dwReminder = i_qwTime % TIME_MLSC_100NS; //micro & 100xNanoseconds
    tUINT32 l_dwNano     = i_qwTime % 10;
    tUINT32 l_dwMicro    = l_dwReminder / 10;

    i_qwTime -= l_dwReminder;

    tUINT32 l_dwMilli = (i_qwTime % TIME_SEC_100NS) / TIME_MLSC_100NS;

    i_qwTime -= TIME_OFFSET_1601_1970;

    time_t  l_llTime = i_qwTime / TIME_SEC_100NS;
    tm     *l_pTime  = localtime(&l_llTime);
    if (l_pTime)
    {
        o_rYear         = 1900 + l_pTime->tm_year;
        o_rMonth        = 1 + l_pTime->tm_mon;
        o_rDay          = l_pTime->tm_mday;
        o_rHour         = l_pTime->tm_hour;
        o_rMinutes      = l_pTime->tm_min;
        o_rSeconds      = l_pTime->tm_sec;
        o_rMilliseconds = l_dwMilli;
        o_rMicroseconds = l_dwMicro;
        o_rNanoseconds  = l_dwNano;
    }
    else
    {
        o_rYear         = 0;
        o_rMonth        = 0;
        o_rDay          = 0;
        o_rHour         = 0;
        o_rMinutes      = 0;
        o_rSeconds      = 0;
        o_rMilliseconds = l_dwMilli;
        o_rMicroseconds = l_dwMicro;
        o_rNanoseconds  = l_dwNano;
    }
}//UnpackLocalTime


////////////////////////////////////////////////////////////////////////////////
//PackLocalTime
//convert date & time to 64-bit value of 100-nanosecond intervals since January 1, 1601 (UTC)
static __attribute__ ((unused)) tUINT64 PackLocalTime(tUINT32 i_uiYear,
                                                      tUINT32 i_uiMonth,
                                                      tUINT32 i_uiDay,
                                                      tUINT32 i_uiHour,
                                                      tUINT32 i_uiMinutes,
                                                      tUINT32 i_uiSeconds,
                                                      tUINT32 i_uiMilliseconds,
                                                      tUINT32 i_uiMicroseconds,
                                                      tUINT32 i_uiNanoseconds
                                                     )
{
    tm l_stTime = {0};
    l_stTime.tm_year  = (int)i_uiYear - 1900;
    l_stTime.tm_mon   = (int)i_uiMonth - 1;
    l_stTime.tm_mday  = (int)i_uiDay;
    l_stTime.tm_hour  = (int)i_uiHour;
    l_stTime.tm_min   = (int)i_uiMinutes;
    l_stTime.tm_sec   = (int)i_uiSeconds;
    l_stTime.tm_isdst = -1;

    time_t l_qwTimeT = mktime(&l_stTime);

    tUINT64 l_qwReturn = l_qwTimeT * TIME_SEC_100NS; 
    l_qwReturn += TIME_OFFSET_1601_1970;

    l_qwReturn += (i_uiMilliseconds * TIME_MLSC_100NS) + (i_uiMicroseconds * 10ull) + (i_uiNanoseconds / 100ull);

    return l_qwReturn;
}//PackLocalTime


////////////////////////////////////////////////////////////////////////////////
//GetSecondOfDay
static inline tUINT32 GetSecondOfDay()
{
    time_t l_llRawtime;
    time(&l_llRawtime);
    tm *l_pTime = localtime(&l_llRawtime);

    if (l_pTime)
    {
        return 3600 * (tUINT32)l_pTime->tm_hour + (tUINT32)l_pTime->tm_min * 60 + l_pTime->tm_sec;
    }

    return 0;
}//GetSecondOfDay


////////////////////////////////////////////////////////////////////////////////
//GetEpochTime
//return a 64-bit value of 100-nanosecond intervals since January 1, 1601 (UTC).
static __attribute__ ((unused)) void GetEpochTime(tUINT32 *o_pHi, tUINT32 *o_pLow)
{
    tUINT64        l_qwResult = 0;
    struct timeval l_sTime    = {0, 0};
    
    gettimeofday(&l_sTime, NULL);

    l_qwResult  = (tUINT64)(l_sTime.tv_sec) * TIME_SEC_100NS;
    l_qwResult += (tUINT64)(l_sTime.tv_usec) * TIME_MCSC_100NS;
    l_qwResult += TIME_OFFSET_1601_1970;

    if (o_pHi)
    {
        *o_pHi  = (tUINT32)(l_qwResult >> 32);
    }
    
    if (o_pLow)
    {
        *o_pLow = (tUINT32)(l_qwResult & 0xFFFFFFFF);
    }
}//GetEpochTime

#endif //PTIME_H
