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
////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                            P7.Tace                                         //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//          Documentation is located in <P7>/Documentation/P7.pdf             //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
#ifndef P7_TRACE_H
#define P7_TRACE_H

#include "P7_Client.h"
#include "P7_Cproxy.h"
#include <stdarg.h> 

#define TRACE_DEFAULT_SHARED_NAME                                 TM("P7.Trace")

////////////////////////////////////////////////////////////////////////////////
//P7.Trace uses optimization to read variable arguments directly from the stack/
//but some systems optimizes storing of var.args  and  uses processor resistors/ 
//stack re-ordering, extra-padding and other techniques, so if you detect than /
//library print wrong values - please activate this macro  manually  for  your /
//platform/project                                                             /
////////////////////////////////////////////////////////////////////////////////
//#define P7TRACE_NO_VA_ARG_OPTIMIZATION

//disable va_arg optimization for Linux, too many ways how GCC + Linux uses 
//stack and registers to store va_arg
#if defined(__linux__)
    #define P7TRACE_NO_VA_ARG_OPTIMIZATION
#endif

////////////////////////////////////////////////////////////////////////////////
//in some platforms access to 64 not aligned variable is  illegal,  if  it  the/ 
//case please active the macro                                                 /
////////////////////////////////////////////////////////////////////////////////
//#define P7TRACE_64BITS_ALIGNED_ACCESS


////////////////////////////////////////////////////////////////////////////////
enum eP7Trace_Level
{
    EP7TRACE_LEVEL_TRACE        = 0,
    EP7TRACE_LEVEL_DEBUG           ,
    EP7TRACE_LEVEL_INFO            ,
    EP7TRACE_LEVEL_WARNING         ,
    EP7TRACE_LEVEL_ERROR           ,
    EP7TRACE_LEVEL_CRITICAL        ,

    EP7TRACE_LEVEL_COUNT
};


////////////////////////////////////////////////////////////////////////////////
#define P7_DELIVER(i_wID, i_eLevel, i_hModule, ...) Trace(i_wID,\
                                                                     i_eLevel,\
                                                                     i_hModule,\
                                                                     (tUINT16)__LINE__,\
                                                                     __FILE__,\
                                                                     __FUNCTION__,\
                                                                     __VA_ARGS__)

////////////////////////////////////////////////////////////////////////////////
#define P7_QTRACE(i_wID, i_hModule, ...)    P7_DELIVER(i_wID,\
                                                                  EP7TRACE_LEVEL_TRACE,\
                                                                  i_hModule,\
                                                                  __VA_ARGS__)

#define P7_TRACE(i_hModule, ...)            P7_QTRACE(0,\
                                                                 i_hModule,\
                                                                 __VA_ARGS__)

////////////////////////////////////////////////////////////////////////////////
#define P7_QDEBUG(i_wID, i_hModule,  ...)    P7_DELIVER(i_wID,\
                                                                  EP7TRACE_LEVEL_DEBUG,\
                                                                  i_hModule,\
                                                                  __VA_ARGS__)

#define P7_DEBUG(i_hModule,  ...)            P7_QDEBUG(0,\
                                                                 i_hModule,\
                                                                 __VA_ARGS__)

////////////////////////////////////////////////////////////////////////////////
#define P7_QINFO(i_wID, i_hModule,  ...)     P7_DELIVER(i_wID,\
                                                                  EP7TRACE_LEVEL_INFO,\
                                                                  i_hModule,\
                                                                  __VA_ARGS__)

#define P7_INFO(i_hModule,  ...)             P7_QINFO(0,\
                                                                i_hModule,\
                                                                __VA_ARGS__)

////////////////////////////////////////////////////////////////////////////////
#define P7_QWARNING(i_wID, i_hModule,  ...)  P7_DELIVER(i_wID,\
                                                                  EP7TRACE_LEVEL_WARNING,\
                                                                  i_hModule,\
                                                                  __VA_ARGS__)

#define P7_WARNING(i_hModule,  ...)          P7_QWARNING(0,\
                                                                   i_hModule,\
                                                                   __VA_ARGS__)

////////////////////////////////////////////////////////////////////////////////
#define P7_QERROR(i_wID, i_hModule,  ...)    P7_DELIVER(i_wID,\
                                                                  EP7TRACE_LEVEL_ERROR,\
                                                                  i_hModule,\
                                                                  __VA_ARGS__)

#define P7_ERROR(i_hModule,  ...)            P7_QERROR(0,\
                                                                 i_hModule,\
                                                                 __VA_ARGS__)

////////////////////////////////////////////////////////////////////////////////
#define P7_QCRITICAL(i_wID, i_hModule,  ...) P7_DELIVER(i_wID,\
                                                                  EP7TRACE_LEVEL_CRITICAL,\
                                                                  i_hModule,\
                                                                  __VA_ARGS__)

#define P7_CRITICAL(i_hModule,  ...)         P7_QCRITICAL(0,\
                                                                    i_hModule,\
                                                                    __VA_ARGS__)

////////////////////////////////////////////////////////////////////////////////
/////////////////////////////IP7_Trace//////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
class IP7_Trace
    : public IP7C_Channel
{
public:
    typedef void* hModule;

    ////////////////////////////////////////////////////////////////////////////
    //Set_Verbosity - function to set trace verbosity, traces with less priority 
    //                will be rejected. You may set verbosity online from Baical
    //                server. See documentation for details.
    virtual void Set_Verbosity(IP7_Trace::hModule i_hModule, 
                               eP7Trace_Level     i_eVerbosity
                              )                                             = 0;

    ////////////////////////////////////////////////////////////////////////////
    //Register_Thread - function used to specify name for current thread. Allows
    //                  you to have nice trace message formatting on Baical side
    //                  and in addition to ThreadId get Thread Name. 
    //                  Call the function from the newly created thread & call
    //                  Unregister_Thread() right before exiting from thread
    //                  body function.
    //N.B.: If internally your application creates & destroys thousands of 
    //      threads - usage of this functions is not good choice due to server 
    //      overhead to store & manage this information - all threads info will 
    //      be stored in RAM and may consume CPU to organize them in RB tree.
    virtual tBOOL Register_Thread(const tXCHAR *i_pName,
                                  tUINT32       i_dwThreadId
                                 )                                          = 0;
    virtual tBOOL Unregister_Thread(tUINT32 i_dwThreadId)                   = 0;


    ////////////////////////////////////////////////////////////////////////////
    //Register_Module - function is used register application module for using 
    //                  it by trace messages, if module with such name is 
    //                  already registered - existing handle will be returned
    virtual tBOOL Register_Module(const tXCHAR       *i_pName,
                                  IP7_Trace::hModule *o_hModule
                                 )                                          = 0;


    ////////////////////////////////////////////////////////////////////////////
    //Share  - function to share current P7.Trace object in address space of the
    //         current process, to get shared instance use function
    //         P7_Get_Shared_Trace(tXCHAR *i_pName). 
    //         See documentation for details.
    virtual tBOOL Share(const tXCHAR *i_pName)                              = 0;

    ////////////////////////////////////////////////////////////////////////////
    //Trace - send trace message to Baical server. See documentation for details
    //N.B.: USE THIS FUNCTION ONLY FROM C/C++ LANGUAGES ! if you want to use    
    //      trace functionality from other languages please call Trace_Managed()
    virtual tBOOL Trace(tUINT16            i_wTrace_ID,   
                        eP7Trace_Level     i_eLevel, 
                        IP7_Trace::hModule i_hModule,
                        tUINT16            i_wLine,
                        const char        *i_pFile,
                        const char        *i_pFunction,
                        const tXCHAR      *i_pFormat,
                        ...
                       )                                                    = 0;

    ////////////////////////////////////////////////////////////////////////////
    //N.B: FUNCTION IS OBSOLETE,                                              //
    //     Please use instead function Trace_Embedded(..., va_list*)          //
    ////////////////////////////////////////////////////////////////////////////
    //Trace_Embedded - send trace message
    //this function is a copy of Trace function, but it is dedicated to embedded 
    //usage (you already have function with variable arguments, but body of your
    //function may be replaced by Trace_Embedded).
    //See documentation for details.
    virtual tBOOL Trace_Embedded(tUINT16            i_wTrace_ID,   
                                 eP7Trace_Level     i_eLevel, 
                                 IP7_Trace::hModule i_hModule,
                                 tUINT16            i_wLine,
                                 const char        *i_pFile,
                                 const char        *i_pFunction,
                                 const tXCHAR     **i_ppFormat
                                )                                           = 0;

    ////////////////////////////////////////////////////////////////////////////
    //Trace_Embedded - send trace message
    //this function is a copy of Trace function, but it is dedicated to embedded 
    //usage (you already have function with variable arguments, but body of your
    //function may be replaced by Trace_Embedded).
    //See documentation for details.
    virtual tBOOL Trace_Embedded(tUINT16            i_wTrace_ID,   
                                 eP7Trace_Level     i_eLevel, 
                                 IP7_Trace::hModule i_hModule,
                                 tUINT16            i_wLine,
                                 const char        *i_pFile,
                                 const char        *i_pFunction,
                                 const tXCHAR     **i_ppFormat,
                                 va_list           *i_pVa_List
                                )                                           = 0;

    ////////////////////////////////////////////////////////////////////////////
    //Trace_Managed - send trace message
    //this function is intended for use by MANAGED languages like C#,python, etc
    //See documentation for details.
    virtual tBOOL Trace_Managed(tUINT16            i_wTrace_ID,   
                                eP7Trace_Level     i_eLevel, 
                                IP7_Trace::hModule i_hModule,
                                tUINT16            i_wLine,
                                const tXCHAR      *i_pFile,
                                const tXCHAR      *i_pFunction,
                                const tXCHAR      *i_pMessage
                               )                                            = 0;
};


////////////////////////////////////////////////////////////////////////////////
//P7_Create_Trace - function create new instance of IP7_Trace object
//See documentation for details.
extern "C" P7_EXPORT IP7_Trace * __cdecl P7_Create_Trace(IP7_Client         *i_pClient,
                                                         const tXCHAR       *i_pName,
                                                         const stTrace_Conf *i_pConf = 0
                                                        );


////////////////////////////////////////////////////////////////////////////////
//This functions allow you to get P7 trace instance if it was created by 
//someone other inside current process. If no instance was registered inside
//current process - function will return NULL. Do not forget to call Release
//on interface when you finish your work.
//See documentation for details.
extern "C" P7_EXPORT IP7_Trace  * __cdecl P7_Get_Shared_Trace(const tXCHAR *i_pName);

#endif //P7_TRACE_H
