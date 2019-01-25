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
// This header file provides access to P7 library functionality from C language/
// or from any other language using dll/so interface                           /
//                                                                             /
//                        +-------------------+                                /
//                        |       Sink        |                                /
//                        | * Network (Baical)|                                /
//                        | * FileBin         |                                /
//                        | * FileTxt         |                                /
//                        | * Console         |                                /
//                        | * Syslog          |                                /
//                        | * Auto            |                                /
//                        | * Null            |                                /
//                        +---------^---------+                                /
//                                  |                                          /
//                                  |                                          /
//                        +---------+---------+                                /
//                        |     P7 Client     |                                /
//                        |    [ Channels ]   |                                /
//                        |  +-+ +-+    +--+  |                                /
//                        |  |0| |1| ...|31|  |                                /
//                        |  +^+ +^+    +-^+  |                                /
//                        +---|---|-------|---+                                /
//                            |   |       |                                    /
//                      +-----+   |       +----------+                         /
//                      |         |                  |                         /
//                      |         |                  |                         /
//                  +---+---+ +---+--------+     +---+---+                     /
//                  | Trace | | Telemetry  |     | Trace |                     /
//                  |Channel| |  channel   | ... |Channel|                     /
//                  +-------+ +------------+     +-------+                     /
//                                                                             /
////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//          Documentation is located in <P7>/Documentation/P7.pdf             //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
#ifndef P7_PROXY_H
#define P7_PROXY_H

#ifdef __cplusplus
extern "C" {
#endif

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                            P7 client                                       //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


typedef void* hP7_Client;

typedef tUINT64 (__cdecl *fnGet_Time_Stamp)(void *i_pContext);
typedef void (__cdecl *fnConnect)(void *i_pContext, tBOOL i_bConnected);

////////////////////////////////////////////////////////////////////////////////
//P7_Client_Create - function creates new P7 client, client is used as transport
//engine for telemetry & trace channels,  every  client  can  handle  up  to  32 
//channels. 
//See documentation for details.
////////////////////////////////////////////////////////////////////////////////
//N.B: P7 client in addition will analyze  your  application  command  line  and 
//     arguments specified through command line will have higher  priority  then 
//     i_sArgs arguments
extern P7_EXPORT hP7_Client __cdecl P7_Client_Create(const tXCHAR *i_pArgs);

//dll/so function prototype
typedef hP7_Client (__cdecl *fnP7_Client_Create)(const tXCHAR *i_pArgs);


////////////////////////////////////////////////////////////////////////////////
//This functions allows you to get P7 client  instance  if  it  was  created  by 
//someone else inside current process and shared using  P7_Client_Share()  func.
//If no instance was registered inside current process -  function  will  return 
//NULL. Do not forget to call P7_Client_Free() when you finish your work.
//See documentation for details.
extern P7_EXPORT hP7_Client __cdecl P7_Client_Get_Shared(const tXCHAR *i_pName);

//dll/so function prototype
typedef hP7_Client (__cdecl *fnP7_Client_Get_Shared)(const tXCHAR *i_pName);


////////////////////////////////////////////////////////////////////////////////
//P7_Client_Share - function to share current P7 client object in address  space 
//of the current process, to get shared client use function P7_Client_Get_Shared
//See documentation for details.
extern P7_EXPORT tBOOL __cdecl P7_Client_Share(hP7_Client    i_hClient, 
                                               const tXCHAR *i_pName
                                              );
//dll/so function prototype
typedef tBOOL (__cdecl *fnP7_Client_Share)(hP7_Client    i_hClient, 
                                           const tXCHAR *i_pName
                                          );


////////////////////////////////////////////////////////////////////////////////
//P7_Client_Add_Ref - increase reference counter for the client
//See documentation for details.
extern P7_EXPORT tINT32 __cdecl P7_Client_Add_Ref(hP7_Client i_hClient);

//dll/so function prototype
typedef tINT32 (__cdecl *fnP7_Client_Add_Ref)(hP7_Client i_hClient);


////////////////////////////////////////////////////////////////////////////////
//P7_Client_Release - function release the client, reference counter  technology 
//See documentation for details.
extern P7_EXPORT tINT32 __cdecl P7_Client_Release(hP7_Client i_hClient);

//dll/so function prototype
typedef tINT32 (__cdecl *fnP7_Client_Release)(hP7_Client i_hClient);


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//                          P7 Crash processor                                //
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//This function setup crash handler to catch and process exceptions like 
// * access violation/segmentation fault
// * division by zero
// * pure virtual call
// * etc.
//When crash occurs handler will call P7_Exceptional_Flush function automatically 
extern P7_EXPORT void __cdecl P7_Set_Crash_Handler();

//dll/so function prototype
typedef void (__cdecl *fnP7_Set_Crash_Handler)();

////////////////////////////////////////////////////////////////////////////////
//This function clears crash handler
extern P7_EXPORT void __cdecl P7_Clr_Crash_Handler();

//dll/so function prototype
typedef void (__cdecl *fnP7_Clr_Crash_Handler)();


////////////////////////////////////////////////////////////////////////////////
//Function allows to flush (deliver) not  delivered/saved  P7  buffers  for  all
//opened clients and related channels owned by process in CASE OF your app/proc.
//crash.
//This function do not call system  memory allocation functions  only  writes to 
//file/socket. 
//Classical scenario: your application has been crushed, you catch the moment of
//crush and call this function once.
//Has to be used if integrator implements own crash handling mechanism.
//N.B.: DO NOT USE OTHER P7 FUNCTION AFTER CALLING OF THIS FUNCTION
//See documentation for details.
extern P7_EXPORT void __cdecl P7_Exceptional_Flush();

//dll/so function prototype
typedef void (__cdecl *fnP7_Exceptional_Flush)();



////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                            P7 Telemetry                                    //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

typedef void* hP7_Telemetry;

typedef void (__cdecl *fnTelemetry_Enable)(void *i_pContext, tUINT8 i_bId, tBOOL i_bEnable);

typedef struct 
{
    void              *pContext;              //context to be passed back to callbacks 
    tUINT64            qwTimestamp_Frequency; //count ticks per second, works in cooperation with pTimestamp_Callback 
    fnGet_Time_Stamp   pTimestamp_Callback;   //callback for getting user timestamps, works in cooperation with qwTimestamp_Frequency 
    fnTelemetry_Enable pEnable_Callback;      //Callback to notify user when counter has been enables/disabled remotely
    fnConnect          pConnect_Callback;     //Callback notifies user when connection with Baical is established/closed
} stTelemetry_Conf;


////////////////////////////////////////////////////////////////////////////////
//P7_Telemetry_Create - function create new instance of IP7_Telemetry object
//See documentation for details.
extern P7_EXPORT hP7_Telemetry __cdecl P7_Telemetry_Create(hP7_Client              i_hClient,
                                                           const tXCHAR           *i_pName,
                                                           const stTelemetry_Conf *i_pConf
                                                          );

//dll/so function prototype
typedef hP7_Telemetry (__cdecl *fnP7_Telemetry_Create)(hP7_Client              i_hClient,
                                                       const tXCHAR           *i_pName,
                                                       const stTelemetry_Conf *i_pConf
                                                      );



////////////////////////////////////////////////////////////////////////////////
//P7_Telemetry_Share - function to share current P7 telemetry object in  address  
//space of the current process, to get shared client use function 
//P7_Telemetry_Get_Shared
//See documentation for details.
extern P7_EXPORT tBOOL __cdecl P7_Telemetry_Share(hP7_Telemetry i_hTelemetry, 
                                                  const tXCHAR *i_pName
                                                 );

//dll/so function prototype
typedef tBOOL (__cdecl *fnP7_Telemetry_Share)(hP7_Telemetry i_hTelemetry, 
                                              const tXCHAR *i_pName
                                             );


////////////////////////////////////////////////////////////////////////////////
//This functions allow you to get P7 telemetry instance if  it  was  created  by 
//someone other inside current process. If no instance was registered inside
//current process - function will return NULL. Do not forget to call Release
//on interface when you finish your work
//See documentation for details.
extern P7_EXPORT hP7_Telemetry __cdecl P7_Telemetry_Get_Shared(const tXCHAR *i_pName);

//dll/so function prototype
typedef hP7_Telemetry (__cdecl *fnP7_Telemetry_Get_Shared)(const tXCHAR *i_pName);


////////////////////////////////////////////////////////////////////////////////
//P7_Telemetry_Create_Counter - create new  telemetry  counter,  max  amount  of 
//counters - 256, if you need more - we recommends you to create another telem.
//channel using P7_Telemetry_Create() function
//See documentation for details.
extern P7_EXPORT tBOOL __cdecl P7_Telemetry_Create_Counter(hP7_Telemetry i_hTelemetry,
                                                           const tXCHAR *i_pName, 
                                                           tINT64        i_llMin,
                                                           tINT64        i_llMax,
                                                           tINT64        i_llAlarm,
                                                           tUINT8        i_bOn,
                                                           tUINT8       *o_pCounter_ID 
                                                          );

//dll/so function prototype
typedef tBOOL (__cdecl *fnP7_Telemetry_Create_Counter)(hP7_Telemetry i_hTelemetry,
                                                       const tXCHAR *i_pName, 
                                                       tINT64        i_llMin,
                                                       tINT64        i_llMax,
                                                       tINT64        i_llAlarm,
                                                       tUINT8        i_bOn,
                                                       tUINT8       *o_pCounter_ID 
                                                      );


////////////////////////////////////////////////////////////////////////////////
//P7_Telemetry_Put_Value - add counter smaple value
//See documentation for details.
extern P7_EXPORT tBOOL __cdecl P7_Telemetry_Put_Value(hP7_Telemetry i_hTelemetry,
                                                      tUINT8        i_bCounter_ID,
                                                      tINT64        i_llValue
                                                      );

//dll/so function prototype
typedef tBOOL (__cdecl *fnP7_Telemetry_Put_Value)(hP7_Telemetry i_hTelemetry,
                                                  tUINT8        i_bCounter_ID,
                                                  tINT64        i_llValue
                                                 );
                                                   

////////////////////////////////////////////////////////////////////////////
//P7_Telemetry_Find_Counter - find counter ID by name
//See documentation for details.
extern P7_EXPORT tBOOL __cdecl P7_Telemetry_Find_Counter(hP7_Telemetry i_hTelemetry,
                                                         const tXCHAR *i_pName,
                                                         tUINT8       *o_pCounter_ID
                                                        );

//dll/so function prototype
typedef tBOOL (__cdecl *fnP7_Telemetry_Find_Counter)(hP7_Telemetry i_hTelemetry,
                                                     const tXCHAR *i_pName,
                                                     tUINT8       *o_pCounter_ID
                                                    );


////////////////////////////////////////////////////////////////////////////////
//P7_Telemetry_Add_Ref - increase reference counter for the telemetry channel
//See documentation for details.
extern P7_EXPORT tINT32 __cdecl P7_Telemetry_Add_Ref(hP7_Telemetry i_hTelemetry);

//dll/so function prototype
typedef tINT32 (__cdecl *fnP7_Telemetry_Add_Ref)(hP7_Telemetry i_hTelemetry);


////////////////////////////////////////////////////////////////////////////////
//P7_Telemetry_Release  -   function releases the telemetry,  reference  counter 
//technology is used, it mean the same client instance  can  be  used  from  few 
//program parts, and every instance has to be released to completely destroy the 
//P7 telemetry object
//See documentation for details.
extern P7_EXPORT tINT32 __cdecl P7_Telemetry_Release(hP7_Telemetry i_hTelemetry);

//dll/so function prototype
typedef tINT32 (__cdecl *fnP7_Telemetry_Release)(hP7_Telemetry i_hTelemetry);


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                               P7 Trace                                     //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

typedef void* hP7_Trace;
typedef void* hP7_Trace_Module;

////////////////////////////////////////////////////////////////////////////////
//Trace levels:
#define  P7_TRACE_LEVEL_TRACE     0
#define  P7_TRACE_LEVEL_DEBUG     1
#define  P7_TRACE_LEVEL_INFO      2
#define  P7_TRACE_LEVEL_WARNING   3
#define  P7_TRACE_LEVEL_ERROR     4
#define  P7_TRACE_LEVEL_CRITICAL  5


typedef void (__cdecl *fnTrace_Verbosity)(void            *i_pContext, 
                                          hP7_Trace_Module i_hModule, 
                                          tUINT32          i_dwVerbosity);

typedef struct 
{
    void              *pContext;              //context to be passed back to callbacks 
    tUINT64            qwTimestamp_Frequency; //count ticks per second, works in cooperation with pTimestamp_Callback 
    fnGet_Time_Stamp   pTimestamp_Callback;   //callback for getting user timestamps, works in cooperation with qwTimestamp_Frequency 
    fnTrace_Verbosity  pVerbosity_Callback;   //Callback to notify user when verbosity has been changed
    fnConnect          pConnect_Callback;     //Callback notifies user when connection with Baical is established/closed
} stTrace_Conf;


////////////////////////////////////////////////////////////////////////////////
//P7_TRACE_ADD macro for trace function calling simplification
#define P7_TRACE_ADD(i_hTrace,\
                     i_wID,\
                     i_eLevel,\
                     i_dwModuleID,\
                     i_pFormat,\
                     ...)\
  P7_Trace_Add(i_hTrace,\
               i_wID,\
               i_eLevel,\
               i_dwModuleID,\
               (tUINT16)__LINE__,\
               __FILE__,\
               __FUNCTION__,\
               i_pFormat,\
               __VA_ARGS__)
                                                                           

////////////////////////////////////////////////////////////////////////////////
//P7_Trace_Create_Trace - function create new instance of IP7_Trace object
//See documentation for details.
extern P7_EXPORT hP7_Trace __cdecl P7_Trace_Create(hP7_Client          i_hClient,
                                                   const tXCHAR       *i_pName,
                                                   const stTrace_Conf *i_pConf
                                                  );

//dll/so function prototype
typedef hP7_Trace (__cdecl *fnP7_Trace_Create)(hP7_Client          i_hClient,
                                               const tXCHAR       *i_pName,
                                               const stTrace_Conf *i_pConf
                                              );


////////////////////////////////////////////////////////////////////////////////
//This functions allow you to get P7 trace instance if it was created by someone
//other inside current process. If no instance  was  registered  inside  current 
//process - function will return NULL. Do not forget to call Release
//on interface when you finish your work
//See documentation for details.
extern P7_EXPORT hP7_Trace __cdecl P7_Trace_Get_Shared(const tXCHAR *i_pName);

//dll/so function prototype
typedef hP7_Trace (__cdecl *fnP7_Trace_Get_Shared)(const tXCHAR *i_pName);


////////////////////////////////////////////////////////////////////////////////
//P7_Trace_Share - function to share current P7 trace object in address space of 
//the current process, to get shared trace channel use function 
//P7_Trace_Get_Shared()
//See documentation for details.
extern P7_EXPORT tBOOL __cdecl P7_Trace_Share(hP7_Trace     i_hTrace, 
                                              const tXCHAR *i_pName
                                             );

//dll/so function prototype
typedef tBOOL (__cdecl *fnP7_Trace_Share)(hP7_Trace     i_hTrace, 
                                          const tXCHAR *i_pName
                                         );


////////////////////////////////////////////////////////////////////////////////
//P7_Trace_Set_Verbosity - function to set minimal trace verbosity,  all  traces 
//with less priority will be rejected by channel. You may set  verbosity  online 
//from Baical server
//See documentation for details.
extern P7_EXPORT void __cdecl P7_Trace_Set_Verbosity(hP7_Trace        i_hTrace, 
                                                     hP7_Trace_Module i_hModule, 
                                                     tUINT32          i_dwVerbosity
                                                    );


//dll/so function prototype
typedef void (__cdecl *fnP7_Trace_Set_Verbosity)(hP7_Trace        i_hTrace, 
                                                 hP7_Trace_Module i_hModule, 
                                                 tUINT32          i_dwVerbosity
                                                );


////////////////////////////////////////////////////////////////////////////////
//P7_Trace_Register_Thread - function used to specify name for current/special
//thread.
//See documentation for details.
extern P7_EXPORT tBOOL __cdecl P7_Trace_Register_Thread(hP7_Trace     i_hTrace, 
                                                        const tXCHAR *i_pName,
                                                        tUINT32       i_dwThreadId
                                                       );


//dll/so function prototype
typedef tBOOL (__cdecl *fnP7_Trace_Register_Thread)(hP7_Trace     i_hTrace, 
                                                    const tXCHAR *i_pName,
                                                    tUINT32       i_dwThreadId
                                                   );



////////////////////////////////////////////////////////////////////////////////
//P7_Trace_Unregister_Thread - function used to unregister current/special thread.
//See documentation for details.
extern P7_EXPORT tBOOL __cdecl P7_Trace_Unregister_Thread(hP7_Trace i_hTrace, 
                                                          tUINT32   i_dwThreadId
                                                         );
            

//dll/so function prototype
typedef tBOOL (__cdecl *fnP7_Trace_Unregister_Thread)(hP7_Trace i_hTrace, 
                                                      tUINT32   i_dwThreadId
                                                     );



////////////////////////////////////////////////////////////////////////////////
//P7_Trace_Register_Module - function used to register module for following usage
//by P7_Trace_Add/P7_Trace_Embedded/P7_Trace_Managed functions
//See documentation for details.
extern P7_EXPORT hP7_Trace_Module __cdecl P7_Trace_Register_Module(hP7_Trace     i_hTrace, 
                                                                   const tXCHAR *i_pName
                                                                  );


//dll/so function prototype
typedef hP7_Trace_Module (__cdecl *fnP7_Trace_Register_Module)(hP7_Trace     i_hTrace, 
                                                               const tXCHAR *i_pName
                                                              );


////////////////////////////////////////////////////////////////////////////////
//P7_Trace_Add - send trace message to Baical server.
//You may use macro P7_TRACE_ADD to simplify calling of this function
////////////////////////////////////////////////////////////////////////////////
//N.B.: DO NOT USE VARIABLE for format string! You  should always use CONSTANT 
//      TEXT like L"My Format %d, %s"
//      USE THIS FUNCTION ONLY FROM C/C++ LANGUAGES! If you want to use trace
//      functionality from other languages please call Trace_Managed()  function
////////////////////////////////////////////////////////////////////////////////
//See documentation for details.
extern P7_EXPORT tBOOL __cdecl P7_Trace_Add(hP7_Trace        i_hTrace,
                                            tUINT16          i_wTrace_ID,   
                                            tUINT32          i_dwLevel, 
                                            hP7_Trace_Module i_hModule,
                                            tUINT16          i_wLine,
                                            const char      *i_pFile,
                                            const char      *i_pFunction,
                                            const tXCHAR    *i_pFormat,
                                            ...
                                            );

//dll/so function prototype
typedef tBOOL (__cdecl *fnP7_Trace_Add)(hP7_Trace        i_hTrace,
                                        tUINT16          i_wTrace_ID,   
                                        tUINT32          i_dwLevel, 
                                        hP7_Trace_Module i_hModule,
                                        tUINT16          i_wLine,
                                        const char      *i_pFile,
                                        const char      *i_pFunction,
                                        const tXCHAR    *i_pFormat,
                                        ...
                                       );



////////////////////////////////////////////////////////////////////////////////
//P7_Trace_Embedded - send trace message to Baical server.
//this function is a copy of P7_Trace_Add function,  but  it  is  dedicated  for 
//embedding into your log function with variable arguments list:
//Usage scenario: you already have function with variable arguments, but body of 
//your function may be replaced by Trace_Embedded.
//See documentation for details.
extern P7_EXPORT tBOOL __cdecl P7_Trace_Embedded(hP7_Trace        i_hTrace,
                                                 tUINT16          i_wTrace_ID,   
                                                 tUINT32          i_dwLevel, 
                                                 hP7_Trace_Module i_hModule,
                                                 tUINT16          i_wLine,
                                                 const char      *i_pFile,
                                                 const char      *i_pFunction,
                                                 const tXCHAR   **i_ppFormat,
                                                 va_list         *i_pVa_List
                                                );

//dll/so function prototype
typedef tBOOL (__cdecl *fnP7_Trace_Embedded)(hP7_Trace        i_hTrace,
                                             tUINT16          i_wTrace_ID,   
                                             tUINT32          i_dwLevel, 
                                             hP7_Trace_Module i_hModule,
                                             tUINT16          i_wLine,
                                             const char      *i_pFile,
                                             const char      *i_pFunction,
                                             const tXCHAR   **i_ppFormat,
                                             va_list         *i_pVa_List
                                            );


////////////////////////////////////////////////////////////////////////////////
//P7_Trace_Managed - send trace message to Baical server.
//This function is intended for use by MANAGED languages like C#, Python, etc
//It is slightly slower than P7_Trace_Add.
//See documentation for details.
extern P7_EXPORT tBOOL __cdecl P7_Trace_Managed(hP7_Trace        i_hTrace,
                                                tUINT16          i_wTrace_ID,   
                                                tUINT32          i_dwLevel,
                                                hP7_Trace_Module i_hModule,
                                                tUINT16          i_wLine,
                                                const tXCHAR    *i_pFile,
                                                const tXCHAR    *i_pFunction,
                                                const tXCHAR    *i_pMessage
                                               );

//dll/so function prototype
typedef tBOOL (__cdecl *fnP7_Trace_Managed)(hP7_Trace        i_hTrace,
                                            tUINT16          i_wTrace_ID,   
                                            tUINT32          i_dwLevel,
                                            hP7_Trace_Module i_hModule,
                                            tUINT16          i_wLine,
                                            const tXCHAR    *i_pFile,
                                            const tXCHAR    *i_pFunction,
                                            const tXCHAR    *i_pMessage
                                           );


////////////////////////////////////////////////////////////////////////////////
//P7_Trace_Add_Ref - increase reference counter for the telemetry channel
//See documentation for details.
extern P7_EXPORT tINT32 __cdecl P7_Trace_Add_Ref(hP7_Trace i_hTrace);

//dll/so function prototype
typedef tINT32 (__cdecl *fnP7_Trace_Add_Ref)(hP7_Trace i_hTrace);


////////////////////////////////////////////////////////////////////////////////
//P7_Trace_Release  - function releases the  trace  object,  reference   counter 
//technology is used, it mean the same trace  instance  can  be  used  from  few 
//program parts, and every instance has to be released to completely destroy the 
//P7 trace object
//See documentation for details.
extern P7_EXPORT tINT32 __cdecl P7_Trace_Release(hP7_Trace i_hTrace);

//dll/so function prototype
typedef tINT32 (__cdecl *fnP7_Trace_Release)(hP7_Trace i_hTrace);


#ifdef __cplusplus
}
#endif

#endif //P7_PROXY_H
