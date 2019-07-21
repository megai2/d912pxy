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
////////////////////////////////////////////////////////////////////////////////
// This header file describes interface for text sink                          /
////////////////////////////////////////////////////////////////////////////////

#ifndef CLTEXTSINK_H
#define CLTEXTSINK_H


////////////////////////////////////////////////////////////////////////////////
struct sTraceDesc
{
    tUINT16        wLine;
    tXCHAR        *pFilePath;
    size_t         szFilePath;
    const tXCHAR  *pFileName; //pointer to pFile_Path
    size_t         szFileName;
    tXCHAR        *pFunction;
    size_t         szFunction;
    tUINT32        dwModuleID; //module ID when trace description was created

    sTraceDesc()
        : wLine(0)
        , pFilePath(NULL)
        , szFilePath(0)
        , pFileName(NULL)
        , szFileName(0)
        , pFunction(NULL)
        , szFunction(0)
        , dwModuleID(0)
    {
    }
};


////////////////////////////////////////////////////////////////////////////////
class CClTextSink
{
public:
    struct sLog
    {
        const tXCHAR  *pChannel;          //channel name
        size_t         szChannel;         //channel name length
        tUINT64        qwIndex;           //log item index (per channel)
        tUINT32        dwId;              //log item ID (per channel)
        tUINT32        dwYear;            //year
        tUINT32        dwMonth;           //month
        tUINT32        dwDay;             //day
        tUINT32        dwHour;            //hour
        tUINT32        dwMinutes;         //minute
        tUINT32        dwSeconds;         //seconds
        tUINT32        dwMilliseconds;    //milliseconds
        tUINT32        dwMicroseconds;    //microseconds
        tUINT32        dwNanoseconds;     //nanoseconds
        tUINT64        qwRawTime;         //number of 100-nanosecond intervals since 01.01.1601 (UTC)
        tUINT64        qwRawTimeOffset;   //offset of 100-nanosecond intervals since prev. item
        eP7Trace_Level eLevel;            //log item level
        tUINT32        dwModuleID;        //module ID for current trace item
        const tXCHAR  *pModuleName;       //module name
        size_t         szModuleName;      //module name length
        tUINT32        dwCpuCore;         //current CPU core number
        tUINT32        dwThreadId;        //thread ID
        const tXCHAR  *pThreadName;       //thread name
        size_t         szThreadName;      //thread name length
        sTraceDesc    *pDesc;             //log item description
        const tXCHAR  *pMessage;          //log item message
        size_t         szMessage;         //log item message length
    };

    CClTextSink() {}
    virtual ~CClTextSink() {}

    virtual eClient_Status Initialize(tXCHAR **i_pArgs, tINT32 i_iCount) = 0;
    virtual eClient_Status Log(const CClTextSink::sLog &i_rRawLog, 
                               const tXCHAR            *i_pFmtLog, 
                               size_t                   i_szFmtLog
                              ) = 0;

    virtual eClient_Status TryRoll() { return ECLIENT_STATUS_OK; }
    virtual eClient_Status DumpBuffers() = 0;
};


#endif //CLTEXTSINK_H
