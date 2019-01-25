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
////////////////////////////////////////////////////////////////////////////////
// Simple logger to file/stdout                                                /
// Each start it create new file and delete old files if count of files is more/
// than allowed                                                                /
// It works in different thread                                                /
////////////////////////////////////////////////////////////////////////////////
#ifndef IJOURNAL_H
#define IJOURNAL_H

#if !defined(JOURNAL_MODULE)
    #define JOURNAL_MODULE NULL
#endif

#define JADD(i_pLogger, i_eVerb,  ...) if (i_pLogger) (i_pLogger)->Log(i_eVerb,\
                                                                       JOURNAL_MODULE,\
                                                                       __FILE__,\
                                                                       __FUNCTION__,\
                                                                       __LINE__,\
                                                                       __VA_ARGS__);

#define JOURNAL_TRACE(i_pLogger,    ...) JADD(i_pLogger, IJournal::eLEVEL_TRACE,    __VA_ARGS__);
#define JOURNAL_DEBUG(i_pLogger,    ...) JADD(i_pLogger, IJournal::eLEVEL_DEBUG,    __VA_ARGS__);
#define JOURNAL_INFO(i_pLogger,     ...) JADD(i_pLogger, IJournal::eLEVEL_INFO,     __VA_ARGS__);
#define JOURNAL_WARNING(i_pLogger,  ...) JADD(i_pLogger, IJournal::eLEVEL_WARNING,  __VA_ARGS__);
#define JOURNAL_ERROR(i_pLogger,    ...) JADD(i_pLogger, IJournal::eLEVEL_ERROR,    __VA_ARGS__);
#define JOURNAL_CRITICAL(i_pLogger, ...) JADD(i_pLogger, IJournal::eLEVEL_CRITICAL, __VA_ARGS__);


////////////////////////////////////////////////////////////////////////////////
class IJournal
{
public:                       
    typedef void* hModule;

    enum eTime 
    {
        eTIME_YYYYMMDDHHMMSSMS = 0,
        eTIME_YYYYMMDDHHMMSS      ,
        eTIME_HHMMSSMS            ,
        eTIME_HHMMSS
    };

    enum eLevel
    {
        eLEVEL_TRACE     = 0,
        eLEVEL_DEBUG        ,
        eLEVEL_INFO         , 
        eLEVEL_WARNING      ,
        eLEVEL_ERROR        ,
        eLEVEL_CRITICAL     ,

        eLEVEL_COUNT
    };

    virtual tBOOL            Initialize(const tXCHAR *i_pName)              = 0;
    virtual void             Set_Verbosity(IJournal::eLevel i_eVerbosity)   = 0;
    virtual IJournal::eLevel Get_Verbosity()                                = 0;

    virtual tBOOL            Register_Thread(const tXCHAR *i_pName,
                                             tUINT32       i_dwThreadId
                                            )                               = 0;
    virtual tBOOL            Unregister_Thread(tUINT32 i_dwThreadId)        = 0;

    virtual tBOOL            Register_Module(const tXCHAR      *i_pName,
                                             IJournal::hModule *o_hModule
                                            )                               = 0;

    virtual tBOOL            Log(IJournal::eLevel  i_eType, 
                                 IJournal::hModule i_hModule,
                                 const char       *i_pFile,
                                 const char       *i_pFunction, 
                                 tUINT32           i_dwLine,
                                 const tXCHAR     *i_pFormat, 
                                 ...
                                )                                           = 0;

    virtual tUINT64          Get_Count(IJournal::eLevel i_eType)            = 0;
    virtual tINT32           Add_Ref()                                      = 0;
    virtual tINT32           Release()                                      = 0;
};

#endif //IJOURNAL_H
