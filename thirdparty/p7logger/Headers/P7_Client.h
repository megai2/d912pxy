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
// This header file provide client functionality                               /
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
#ifndef P7_CLIENT_H
#define P7_CLIENT_H

#include "GTypes.h"
#include "P7_Version.h"

#define CLIENT_DEFAULT_SHARED_NAME                             TM("P7.Client")

//Values: P7 sink (logs, traces & telemetry destination)
//There are 3 possible values:
// * Baical - over network to Baical server directly
// * File   - to file
// * Auto   - to Baical if connection is established, otherwise to file
//            N.B.: connection timeout is 250 ms.
// * Null   - all data will be dropped
// Default value: Baical
#define CLIENT_COMMAND_LINE_SINK                               TM("/P7.Sink=")

#define CLIENT_SINK_BAICAL                                     TM("Baical")
#define CLIENT_SINK_FILE_BIN                                   TM("FileBin")
#define CLIENT_SINK_FILE_BIN_LEGACY                            TM("File")
#define CLIENT_SINK_FILE_TXT                                   TM("FileTxt")
#define CLIENT_SINK_CONSOLE                                    TM("Console")
#define CLIENT_SINK_SYSLOG                                     TM("Syslog")
#define CLIENT_SINK_AUTO                                       TM("Auto")
#define CLIENT_SINK_NULL                                       TM("Null")


//Values: P7 client name, max length is about 96 characters, by default it is
//name of the process. Useful in case when P7 is used from script language and
//name of host process isn't usable - for example Python.exe
#define CLIENT_COMMAND_LINE_NAME                               TM("/P7.Name=")

////////////////////////////////////////////////////////////////////////////////
//                            Sink=Baical arguments                            /
////////////////////////////////////////////////////////////////////////////////
//Values: server address
//IPV4 address : XXX.XXX.XXX.XXX
//IPV6 address : ::1, etc.
//NetBios Name : any name
#define CLIENT_COMMAND_LINE_BAICAL_ADDRESS                     TM("/P7.Addr=")

//Values: server specified port
//default: 9009
#define CLIENT_COMMAND_LINE_BAICAL_PORT                        TM("/P7.Port=")

//Values: size of the data packet
// Min: 512
// Max: 65535
// Recommended: your network MTU size
// default: 1472
#define CLIENT_COMMAND_PACKET_BAICAL_SIZE                      TM("/P7.PSize=")

//size of the transmission window in packets. Sometimes is useful to manage it
//if server aggressively loose incoming packets, this parameter is for precise
//tuning, in most of the cases stay untouched and protocol manage it
//Min = 1
//max = ((pool size / packet size) / 2)
#define CLIENT_COMMAND_WINDOW_BAICAL_SIZE                      TM("/P7.Window=")

//specifies exit timeout in seconds, used to deliver last data chunks to server
//max value is 15 seconds
#define CLIENT_COMMAND_LINE_BAICAL_EXIT_TIMEOUT                TM("/P7.Eto=")

////////////////////////////////////////////////////////////////////////////////
//                          Sink=File arguments                                /
////////////////////////////////////////////////////////////////////////////////
//Value: path to destination file
//default: <current process dir>/P7logs
#define CLIENT_COMMAND_LINE_DIR                                TM("/P7.Dir=")

//Value: define rolling type
// Xmb     - rolling every X megabytes, for example /P7.Roll=10mb
// Xhr     - rolling every X hours, for example /P7.Roll=24hr, max = 1000hr
// HH:MMtm - rolling by time (00:00 -> 23:59), for example: 
//   * rolling at 12:00 -> P7.Roll=12:00tm
//   * rolling at 12:00 and 00:00 -> P7.Roll=00:00,12:00tm
//default: rolling is off
#define CLIENT_COMMAND_LINE_FILE_ROLLING                       TM("/P7.Roll=")

//Value: define maximum P7 logs files in destination folder /P7.Dir="MyDir"
//in case if count of files is larger than specified value - oldest files will 
//be removed
//default : off (0)
//min     : 1
//max     : 4096
#define CLIENT_COMMAND_LINE_FILES_COUNT_MAX                   TM("/P7.Files=")


//Value: define maximum P7 logs files cumulative size in MB at destination 
//folder /P7.Dir="MyDir" in case if size of files is larger than specified 
//value - oldest files will be removed
//default : off (0)
//min     : 1
//max     : 4294967296
#define CLIENT_COMMAND_LINE_FILES_SIZE_MAX                    TM("/P7.FSize=")


////////////////////////////////////////////////////////////////////////////////
//                          general settings                                   /
////////////////////////////////////////////////////////////////////////////////
//Values:
// 0 : Debug
// 1 : Info
// 2 : Warnings
// 3 : Errors
// 4 : Critical
// default: logging is off
#define CLIENT_COMMAND_LOG_VERBOSITY                           TM("/P7.Verb=")

//Manage internal logging
//Values:
// 0 : Off
// 1 : On
//default: On(1)
#define CLIENT_COMMAND_LOG_ON                                  TM("/P7.On=")

//size of the internal buffers pool in kilobytes. Minimal 16(kb), max is not 
//specified
//default value = 1mb
#define CLIENT_COMMAND_POOL_SIZE                               TM("/P7.Pool=")


//specifies log message format for such sinks like: FileTxt, Console, SysLog 
//format example: 
// /P7.Format="%cn #%ix [%tf] %lv Tr:#%ti:%tn CPU:%cc Md:%mn {%fs:%fl:%fn} %ms"

#define CLIENT_COMMAND_FORMAT                                  TM("/P7.Format=")

#define CLIENT_FMT_CHANNEL                                     TM("cn")
                                                               
#define CLIENT_FMT_MSG_ID                                      TM("id")
#define CLIENT_FMT_MSG_INDEX                                   TM("ix")
                                                               
#define CLIENT_FMT_TIME_FULL                                   TM("tf")
#define CLIENT_FMT_TIME_MEDIUM                                 TM("tm")
#define CLIENT_FMT_TIME_SHORT                                  TM("ts")
#define CLIENT_FMT_TIME_DIFF                                   TM("td")
#define CLIENT_FMT_TIME_COUNT                                  TM("tc")
                                                               
#define CLIENT_FMT_LEVEL                                       TM("lv")
                                                               
#define CLIENT_FMT_THREAD_ID                                   TM("ti")
#define CLIENT_FMT_THREAD_NAME                                 TM("tn")
                                                               
#define CLIENT_FMT_CPU_CORE                                    TM("cc")
                                                               
#define CLIENT_FMT_MODULE_ID                                   TM("mi")
#define CLIENT_FMT_MODULE_NAME                                 TM("mn")
                                                               
#define CLIENT_FMT_FILE_FULL                                   TM("ff")
#define CLIENT_FMT_FILE_SHORT                                  TM("fs")
#define CLIENT_FMT_FILE_LINE                                   TM("fl")
#define CLIENT_FMT_FUNCTION                                    TM("fn")
                                                               
#define CLIENT_FMT_MSG                                         TM("ms")


//Syslog facility digit https://tools.ietf.org/html/rfc3164#page-8
#define CLIENT_COMMAND_SYSLOG_FACILITY                         TM("/P7.Facility=")


//Values: No values
#define CLIENT_COMMAND_LOG_HELP                                TM("/P7.Help")


////////////////////////////////////////////////////////////////////////////////
//                          Trace  settings                                    /
////////////////////////////////////////////////////////////////////////////////

//Override verbosity for all trace streams and modules
//Values:
//0 = EP7TRACE_LEVEL_TRACE
//1 = EP7TRACE_LEVEL_DEBUG   
//2 = EP7TRACE_LEVEL_INFO    
//3 = EP7TRACE_LEVEL_WARNING 
//4 = EP7TRACE_LEVEL_ERROR   
//5 = EP7TRACE_LEVEL_CRITICAL
//Example: /P7.Trc.Verb=5 
#define CLIENT_COMMAND_TRACE_VERBOSITY                       TM("/P7.Trc.Verb=")


#define CLIENT_HELP_STRING\
    TM("P7 arguments:\n")\
    TM(" -General arguments: \n")\
    TM("   /P7.Sink   - Select data flow direction, there are few values:\n")\
    TM("                * Baical  - to Baical server over network(fast and efficient)\n")\
    TM("                * FileBin - to local binary file (fast and efficient)\n")\
    TM("                * FileTxt - to local text file (slow comparing to FileBin/Baical)\n")\
    TM("                * Syslog  - to syslog server (slow comparing to FileBin/Baical)\n")\
    TM("                * Console - to console (slow comparing to FileBin/Baical)\n")\
    TM("                * Auto    - to Baical server if connection is istablished, otherwise to file\n")\
    TM("                            N.B.: connection timeout is 250 ms !\n")\
    TM("                * Null    - drop all incoming data\n")\
    TM("                Examples:\n")\
    TM("                /P7.Sink=Baical\n")\
    TM("                /P7.Sink=File\n")\
    TM("                /P7.Sink=Null\n")\
    TM("   /P7.Name   - P7 client name, max length is about 96 characters, by default it is\n")\
    TM("                name of the  host process. Useful in case when P7 is used from scrip\n")\
    TM("                language and name of host process is not suitable - for example Python.exe\n")\
    TM("                Example:\n")\
    TM("                /P7.Name=Client\n")\
    TM("   /P7.On     - Enable/Disable P7 network engine, By default P7 is on\n")\
    TM("                Examples:\n")\
    TM("                /P7.On=1\n")\
    TM("                /P7.On=0\n")\
    TM("   /P7.Verb   - Set logging verbosity level. This option allow to write\n")\
    TM("                to text log file or stdout all internal engine messages.\n")\
    TM("                Do not specify /P7.Verb parameter to switch off the logging\n")\
    TM("                P7 internal logging has next verbosity levels:\n")\
    TM("                0 : Info\n")\
    TM("                1 : Debug\n")\
    TM("                2 : Warnings\n")\
    TM("                3 : Errors\n")\
    TM("                4 : Critical\n")\
    TM("                Example: /P7.Verb=4\n")\
    TM("   /P7.Trc.Verb- Set verbosity level for all trace streams and associated modules\n")\
    TM("                Has next verbosity levels:\n")\
    TM("                0 : Trace\n")\
    TM("                1 : Debug\n")\
    TM("                2 : Info\n")\
    TM("                3 : Warnings\n")\
    TM("                4 : Errors\n")\
    TM("                5 : Critical\n")\
    TM("                Example: /P7.Trc.Verb=4\n")\
    TM("   /P7.Pool   - Set size of the internal buffers pool in kilobytes. Minimal 16(kb)\n")\
    TM("                maximal is limited by your OS and HW. Default value = 4mb\n")\
    TM("                Example, 1 Mb allocation: /P7.Pool=1024\n")\
    TM(" -Sink=Baical, -Sink=Syslog : \n")\
    TM("   /P7.Addr   - Set server address (IPV4, IPV6, NetBios name)\n")\
    TM("                Examples:\n")\
    TM("                /P7.Addr=127.0.0.1\n")\
    TM("                /P7.Addr=::1\n")\
    TM("                /P7.Addr=MyPC\n")\
    TM("                Default address is 127.0.0.1\n")\
    TM("   /P7.Port   - Set server port, default port is 9009\n")\
    TM("                Example: /P7.Port=9010\n")\
    TM("   /P7.PSize  - Set packet size. Min value 512 bytes, Max - 65535, Default - 512\n")\
    TM("                You should specify optimum packet size\n")\
    TM("                for your network, usually it is MTU.\n")\
    TM("                Example: /P7.PSize=1476\n")\
    TM(" -Sink=FileBin, -Sink=FileTxt: \n")\
    TM("   /P7.Dir    - Directory where P7 files will be created, if it is not specified\n")\
    TM("                process directory will be used.\n")\
    TM("                Examples:\n")\
    TM("                /P7.Dir=/home/user/logs/\n")\
    TM("                /P7.Dir=C:\\Logs\\\n")\
    TM("   /P7.Files  - defines maximum P7 logs files in destination folder\n")\
    TM("                in case if count of files is larger than specified value\n")\
    TM("                oldest files will be removed\n")\
    TM("   /P7.Roll   - use it to specify files rolling value & type. There are 2 rolling types:\n")\
    TM("                * Rolling by file size, measured in megabytes\n")\
    TM("                * Rolling by time, measured in hours, 1000 hours max\n")\
    TM("                Rolling value consists of 2 parts:\n")\
    TM("                * digit\n")\
    TM("                * postfix: hr, mb\n")\
    TM("                Examples:\n")\
    TM("                /P7.Roll=100mb\n")\
    TM("                /P7.Roll=24hr\n")\
    TM(" -Sink=FileTxt, -Sink=Console, -Sink=Syslog: \n")\
    TM("   /P7.Format - log message format sting, example: /P7.Format=\"{%%cn} [%%tf] %%lv %%ms\"\n")\
    TM("                * cn - channel name \n")\
    TM("                * id - message ID\n")\
    TM("                * ix - message index\n")\
    TM("                * tf - full time YY.MM.DD HH.MM.SS.mils.mics.nans\n")\
    TM("                * tm - time medium HH.MM.SS.mils.mics.nans\n")\
    TM("                * ts - time short MM.SS.mils.mics.nans\n")\
    TM("                * td - time difference between current and prev. one +SS.mils.mics.nans\n")\
    TM("                * tc - time stamp in 100 nanoseconds intervals\n")\
    TM("                * lv - level (error, warning, etc)\n")\
    TM("                * ti - thread ID\n")\
    TM("                * tn - thread name\n")\
    TM("                * cc - CPU core\n")\
    TM("                * mi - module ID\n")\
    TM("                * mn - module name\n")\
    TM("                * ff - file path + name\n")\
    TM("                * fs - file name\n")\
    TM("                * fl - file line\n")\
    TM("                * fn - function name\n")\
    TM("                * ms - message\n")\
    TM(" -Sink=Null: \n")\
    TM("   there is no arguments\n")\
 


enum eClient_Status
{
    //Regular statuses
    ECLIENT_STATUS_OK                   = 0,
    ECLIENT_STATUS_OFF                     ,
    ECLIENT_STATUS_INTERNAL_ERROR          ,

    //Temporary statuses             
    ECLIENT_STATUS_DISCONNECTED            ,
    ECLIENT_STATUS_NO_FREE_BUFFERS         ,
    ECLIENT_STATUS_NOT_ALLOWED             ,
    ECLIENT_STATUS_WRONG_PARAMETERS        ,
    ECLIENT_STATUS_WRONG_FORMAT
};

PRAGMA_PACK_ENTER(4) //alignment is now 4, MS Only//////////////////////////////

struct sP7C_Status
{
    tBOOL  bConnected;
    //count of the connection drops, when connection has been reinitialized
    tUINT32 dwResets;
}ATTR_PACK(4);


struct sP7C_Data_Chunk
{
    void    *pData;
    tUINT32  dwSize;
}ATTR_PACK(4);


struct sP7C_Channel_Info
{
    tUINT32 dwID;
}ATTR_PACK(4);

struct sP7C_Info
{
    //sockaddr_storage sServer;
    tUINT32            dwMem_Used;
    tUINT32            dwMem_Free;
    tUINT32            dwMem_Alloc;
    tUINT32            dwReject_Mem; //chunks rejected counter - no memory
    tUINT32            dwReject_Con; //chunks rejected counter - no connection
    tUINT32            dwReject_Int; //chunks rejected counter - internal errors
}ATTR_PACK(4);

PRAGMA_PACK_EXIT()//4///////////////////////////////////////////////////////////


class /*__declspec(novtable)*/ IP7C_Channel
{
public:
    enum eType
    {
        eTrace,
        eTelemetry,
        eCount
    };
    ////////////////////////////////////////////////////////////////////////////
    //Get_Type - get channel type, depending on type cast to IP7_Telemetry or
    //           IP7_Trace is available
    virtual IP7C_Channel::eType Get_Type()                                  = 0;

    ////////////////////////////////////////////////////////////////////////////
    //Add_Ref - increase object's reference count
    //          See documentation for details.
    virtual tINT32 Add_Ref()                                                = 0;

    ////////////////////////////////////////////////////////////////////////////
    //Release - decrease object's reference count. If reference count less or
    //          equal to 0 - object will be destroyed
    //          See documentation for details.
    virtual tINT32 Release()                                                = 0;


    virtual void On_Init(sP7C_Channel_Info *i_pInfo)                        = 0;
    virtual void On_Receive(tUINT32 i_dwChannel, 
                            tUINT8 *i_pBuffer, 
                            tUINT32 i_dwSize,
                            tBOOL   i_bBigEndian
                            )                                               = 0;

    virtual void On_Status(tUINT32            i_dwChannel, 
                           const sP7C_Status *i_pStatus)                    = 0;

    virtual void On_Flush(tUINT32 i_dwChannel, tBOOL *io_pCrash)            = 0;
};


//Define the max channels count, and max packet size for IP7_Client::Sent(...)
#define USER_PACKET_CHANNEL_ID_BITS_COUNT                                    (5)
#define USER_PACKET_SIZE_BITS_COUNT       (32-USER_PACKET_CHANNEL_ID_BITS_COUNT)
#define USER_PACKET_MAX_SIZE                  (1 << USER_PACKET_SIZE_BITS_COUNT)
#define USER_PACKET_CHANNEL_ID_MAX_SIZE (1 << USER_PACKET_CHANNEL_ID_BITS_COUNT)


class /*__declspec(novtable)*/ IP7_Client
{
public:
    enum eType
    {
        eBaical,
        eFileBin,
        eAuto,
        eNull,
        eText,
        eCount
    };
    virtual IP7_Client::eType Get_Type()                                    = 0;

    virtual tINT32            Add_Ref()                                     = 0;
    virtual tINT32            Release()                                     = 0;
                              
    virtual eClient_Status    Get_Status()                                  = 0;
    virtual tBOOL             Get_Status(sP7C_Status *o_pStatus)            = 0;
    virtual tBOOL             Get_Info(sP7C_Info *o_pInfo)                  = 0;
    //N.B. If you use Baikal server to receive data - please do not mix
    //     different packets formats (Trace + Telemetry for example) on the same
    //     channel, in this case one data format will be dropped by server
    virtual eClient_Status    Register_Channel(IP7C_Channel *i_pChannel)    = 0;
    virtual eClient_Status    Unregister_Channel(tUINT32 i_dwID)            = 0;
                              
    virtual eClient_Status    Sent(tUINT32          i_dwChannel_ID,
                                   sP7C_Data_Chunk *i_pChunks, 
                                   tUINT32          i_dwCount,
                                   tUINT32          i_dwSize)               = 0;
                              
    ////////////////////////////////////////////////////////////////////////////
    //Share  - function to share current P7 object in address space of
    //         the current process, see documentation for details
    virtual tBOOL             Share(const tXCHAR *i_pName)                  = 0;

    virtual const tXCHAR     *Get_Argument(const tXCHAR  *i_pName)          = 0;

    virtual size_t            Get_Channels_Count()                          = 0;
    virtual IP7C_Channel     *Get_Channel(size_t i_szIndex)                 = 0;
};


////////////////////////////////////////////////////////////////////////////////
//P7_Create_Client - function creates new P7 client.
//See documentation for details
////////////////////////////////////////////////////////////////////////////////
//N.B: P7 client in addition will analyze  your  application  command  line  and 
//     arguments specified through command line will have higher  priority  then 
//     i_sArgs arguments
extern "C" P7_EXPORT IP7_Client * __cdecl P7_Create_Client(const tXCHAR *i_pArgs);


////////////////////////////////////////////////////////////////////////////////
//This functions allows you to get P7 instance if it was created by someone else 
//inside current process. 
//See documentation for details.
extern "C" P7_EXPORT IP7_Client  * __cdecl P7_Get_Shared(const tXCHAR *i_pName);




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
extern "C" P7_EXPORT void __cdecl P7_Set_Crash_Handler();

////////////////////////////////////////////////////////////////////////////////
//This function clears crash handler
extern "C" P7_EXPORT void __cdecl P7_Clr_Crash_Handler();


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
extern "C" P7_EXPORT void __cdecl P7_Exceptional_Flush();



#endif //P7_CLIENT_H
