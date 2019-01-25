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
// P7 extensions - interfaces to some additional functionality                 /
//  - IP7_Trace : traces formatting and delivery to Baical server              /
////////////////////////////////////////////////////////////////////////////////
#ifndef P7_EXTENSIONS_H
#define P7_EXTENSIONS_H


#define P7_EXTENSION_TYPE_BITS_COUNT                                         (5)
#define P7_EXTENSION_SUB_TYPE_BITS_COUNT                                     (5)
#define P7_EXTENSION_PACKET_SIZE_BITS_COUNT                                 (22)

#define P7_EXTENSION_MAX_TYPES               (1 << P7_EXTENSION_TYPE_BITS_COUNT)

#define P7TRACE_NAME_LENGTH                                                 (64)
#define P7TRACE_THREAD_NAME_LENGTH                                          (48)
#define P7TRACE_MODULE_NAME_LENGTH                                          (54)
#define P7TELEMETRY_NAME_LENGTH                                             (64)
#define P7TELEMETRY_COUNTER_NAME_LENGTH                                     (64)
#define P7TELEMETRY_COUNTERS_MAX_COUNT                    ((tUINT8)~((tUINT8)0))
#define P7TELEMETRY_INVALID_ID              (P7TELEMETRY_COUNTERS_MAX_COUNT - 1)


#define P7TRACE_INFO_FLAG_BIG_ENDIAN                                    (0x0001)
#define P7TRACE_INFO_FLAG_UNSORTED                                      (0x0002)
#define P7TRACE_INFO_FLAG_EXTENTION                                     (0x0004)

enum eTrace_Arg_Type
{
    P7TRACE_ARG_TYPE_UNK    = 0x00,
    P7TRACE_ARG_TYPE_CHAR   = 0x01,
    P7TRACE_ARG_TYPE_INT8   = 0x01,
    P7TRACE_ARG_TYPE_CHAR16  ,//(0x02)
    P7TRACE_ARG_TYPE_INT16   ,//(0x03)
    P7TRACE_ARG_TYPE_INT32   ,//(0x04)
    P7TRACE_ARG_TYPE_INT64   ,//(0x05)
    P7TRACE_ARG_TYPE_DOUBLE  ,//(0x06)
    P7TRACE_ARG_TYPE_PVOID   ,//(0x07)
    P7TRACE_ARG_TYPE_USTR16  ,//(0x08) //unicode - UTF16 string 
    P7TRACE_ARG_TYPE_STRA    ,//(0x09) //ASCII string           
    P7TRACE_ARG_TYPE_USTR8   ,//(0x0A) //unicode - UTF8 string  
    P7TRACE_ARG_TYPE_USTR32  ,//(0x0B) //unicode - UTF32 string  
    P7TRACE_ARG_TYPE_CHAR32  ,//(0x0C)
    P7TRACE_ARG_TYPE_INTMAX  ,//(0x0D)

    P7TRACE_ARGS_COUNT
};


#if defined(GTX64)
    #define SIZE_OF_ARG(t)   ( (sizeof(t) + 8 - 1) & ~(8 - 1) )

    typedef tUINT64 tKeyType;
#else
    #define SIZE_OF_ARG(t)   ( (sizeof(t) + 4 - 1) & ~(4 - 1) )

    typedef tUINT32 tKeyType;
#endif


////////////////////////////////////////////////////////////////////////////////
// Data types, we can transmit different packets types, here are the list 
// of all supported for the moment packet types
enum eP7User_Type
{
    EP7USER_TYPE_TRACE          =  0, 
    EP7USER_TYPE_TELEMETRY          , 

    EP7USER_TYPE_MAX            = P7_EXTENSION_MAX_TYPES 
};


////////////////////////////////////////////////////////////////////////////////
enum eP7Trace_Type
{
    EP7TRACE_TYPE_INFO          =  0, //OUT
    EP7TRACE_TYPE_DESC              , //OUT
    EP7TRACE_TYPE_DATA              , //OUT
    EP7TRACE_TYPE_VERB              , //IN/OUT
    EP7TRACE_TYPE_CLOSE             , //OUT
    EP7TRACE_TYPE_THREAD_START      , //OUT
    EP7TRACE_TYPE_THREAD_STOP       , //OUT
    EP7TRACE_TYPE_MODULE            , //OUT

    EP7TRACE_TYPE_MAX           = 32 
};

////////////////////////////////////////////////////////////////////////////////
enum eP7Trace_Ext
{
    EP7TRACE_EXT_MODULE_ID      =  0, 
    EP7TRACE_EXT_MAX            = 256 
};


////////////////////////////////////////////////////////////////////////////////
enum eP7Tel_Type
{
    EP7TEL_TYPE_INFO            =  0, //OUT
    EP7TEL_TYPE_COUNTER             , //OUT
    EP7TEL_TYPE_VALUE               , //OUT
    EP7TEL_TYPE_ENABLE              , //IN
    EP7TEL_TYPE_CLOSE               , //OUT

    EP7TEL_TYPE_MAX             = 32 
};


PRAGMA_PACK_ENTER(4) //alignment is now 4, MS Only//////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// common user header for extensions (like trace or telemetry)

#define INIT_EXT_HEADER(iHeaderRaw, iType, iSubType, iSize)\
    iHeaderRaw.dwBits = \
    (((iType) & ((1 << P7_EXTENSION_TYPE_BITS_COUNT) - 1)) |\
    (((iSubType) & ((1 << P7_EXTENSION_SUB_TYPE_BITS_COUNT) - 1)) << P7_EXTENSION_TYPE_BITS_COUNT) |\
    (((iSize) & ((1 << P7_EXTENSION_PACKET_SIZE_BITS_COUNT) - 1)) << (P7_EXTENSION_TYPE_BITS_COUNT + P7_EXTENSION_SUB_TYPE_BITS_COUNT)))

#define GET_EXT_HEADER_SIZE(iHeaderRaw)\
    (iHeaderRaw.dwBits >> (P7_EXTENSION_TYPE_BITS_COUNT + P7_EXTENSION_SUB_TYPE_BITS_COUNT))

#define GET_EXT_HEADER_TYPE(iHeaderRaw)\
    (iHeaderRaw.dwBits & ((1 << P7_EXTENSION_TYPE_BITS_COUNT) - 1))

#define GET_EXT_HEADER_SUBTYPE(iHeaderRaw)\
    ((iHeaderRaw.dwBits >> P7_EXTENSION_TYPE_BITS_COUNT) & ((1 << P7_EXTENSION_SUB_TYPE_BITS_COUNT) - 1))

#define SET_EXT_HEADER_SIZE(iHeaderRaw, iSize)\
    iHeaderRaw.dwBits = \
    (iHeaderRaw.dwBits & ((1 << (P7_EXTENSION_TYPE_BITS_COUNT + P7_EXTENSION_SUB_TYPE_BITS_COUNT)) - 1)) |\
    (((iSize) & ((1 << P7_EXTENSION_PACKET_SIZE_BITS_COUNT) - 1)) << (P7_EXTENSION_TYPE_BITS_COUNT + P7_EXTENSION_SUB_TYPE_BITS_COUNT))

struct sP7Ext_Header
{
   tUINT32 dwType   :P7_EXTENSION_TYPE_BITS_COUNT;     //eP7User_Type
   tUINT32 dwSubType:P7_EXTENSION_SUB_TYPE_BITS_COUNT;  
   //max 4 mb, value should include size of this header
   tUINT32 dwSize   :P7_EXTENSION_PACKET_SIZE_BITS_COUNT; 
   //At the end of structure we put serialized data
} ATTR_PACK(4);

struct sP7Ext_Raw //mapping of sP7Ext_Header
{
   tUINT32 dwBits;
} ATTR_PACK(4);

GASSERT(sizeof(sP7Ext_Header) == sizeof(sP7Ext_Raw));

//N.B.: extension packets can follow one by one in data packets.
PRAGMA_PACK_EXIT()//4///////////////////////////////////////////////////////////



PRAGMA_PACK_ENTER(2) //alignment is now 2, MS Only//////////////////////////////

#define P7_DAMP_FILE_MARKER_V1                           (0x45D2AC71ECF32CA6ULL)
#define P7_DAMP_FILE_HOST_LENGTH                                           (256)
#define P7_DAMP_FILE_PROCESS_LENGTH                                        (256)

//P7 damp file header
struct sP7File_Header
{
    tUINT64 qwMarker;
    tUINT32 dwProcess_ID;
    tUINT32 dwProcess_Start_Time_Hi;
    tUINT32 dwProcess_Start_Time_Lo;
    tWCHAR  pProcess_Name[P7_DAMP_FILE_PROCESS_LENGTH];
    tWCHAR  pHost_Name[P7_DAMP_FILE_HOST_LENGTH];
} ATTR_PACK(2);



////////////////////////////////////////////////////////////////////////////////
//                                 Trace                                      //
////////////////////////////////////////////////////////////////////////////////

//trace info header
struct sP7Trace_Info
{
    union
    {
        sP7Ext_Header sCommon;
        sP7Ext_Raw    sCommonRaw;
    };
    //Contains a 64-bit value representing the number of 100-nanosecond intervals 
    //since January 1, 1601 (UTC). In windows we use FILETIME structure for 
    //representing
    tUINT32       dwTime_Hi;
    tUINT32       dwTime_Lo;
    //Hi resolution timer value, we get this value when we retrieve current time.
    //using difference between this value and timer value for every trace we can
    //calculate time of the trace event with hi resolution
    tUINT64       qwTimer_Value;
    //timer's count heartbeats in second
    tUINT64       qwTimer_Frequency;
    tUINT64       qwFlags; 
    tWCHAR        pName[P7TRACE_NAME_LENGTH];
} ATTR_PACK(2);


//this structure describe each argument inside variable arguments list
//all arguments are serialized data block 
struct sP7Trace_Arg
{
    //argument's type - one of P7TRACE_ARG_TYPE_XXX
    tUINT8 bType; 
    //Size - how many bytes is used by argument inside block, this value is not
    //       directly depend on type, usually it depend on processor architecture
    //       for example "char" on WIN32 this is 4 bytes, but for EventTrace 
    //       engine this is only 1 byte.
    //       N.B.: All strings has 0 size.
    tUINT8 bSize; 
} ATTR_PACK(2);

//trace description header
struct sP7Trace_Format
{
    union
    {
        sP7Ext_Header sCommon;
        sP7Ext_Raw    sCommonRaw;
    };
    tUINT16       wID;
    tUINT16       wLine;
    tUINT16       wModuleID;  //Module ID, who send trace
    tUINT16       wArgs_Len;  //arguments count
    //At the end of structure we put serialized data:
    //sP7Trace_Arg [dwArgs_Len]   - array of arguments
    //wchar_t      Format[]       - null terminated string
    //char         FileName[]     - null terminated string
    //char         FunctionName[] - null terminated string
} ATTR_PACK(2);

//trace data header
struct sP7Trace_Data
{
    union
    {
        sP7Ext_Header sCommon;
        sP7Ext_Raw    sCommonRaw;
    };
    tUINT16       wID;          //trace ID
    tUINT8        bLevel;       //eP7Trace_Level
    tUINT8        bProcessor;   //Processor number
    tUINT32       dwThreadID;   //Thread ID
    tUINT32       dwSequence;   //sequence number
    tUINT64       qwTimer;      //High resolution timer value
    //At the end of structure we put serialized data:
    // - trace variable arguments values
    // - extensions [data X bits][type 8 bits], [data X bits][type 8 bits], ... [count 8 bits]
} ATTR_PACK(2);

//trace ext header for Module ID
struct sP7Trace_Ext_Mid
{
    tUINT16 wMID; 
} ATTR_PACK(2);


//trace verbosity header
struct sP7Trace_Verb
{
    union
    {
        sP7Ext_Header sCommon;
        sP7Ext_Raw    sCommonRaw;
    };
    eP7Trace_Level   eVerbosity;
    tUINT16          wModuleID;
} ATTR_PACK(2);


//Thread start info
struct sP7Trace_Thread_Start
{
    union
    {
        sP7Ext_Header sCommon;
        sP7Ext_Raw    sCommonRaw;
    };
    tUINT32       dwThreadID;                        //Thread ID
    tUINT64       qwTimer;                           //High resolution timer value
    char          pName[P7TRACE_THREAD_NAME_LENGTH]; //Thread name (UTF-8) 
} ATTR_PACK(2);

//Thread stop info
struct sP7Trace_Thread_Stop
{
    union
    {
        sP7Ext_Header sCommon;
        sP7Ext_Raw    sCommonRaw;
    };
    tUINT32       dwThreadID; //Thread ID
    tUINT64       qwTimer;    //High resolution timer value
} ATTR_PACK(2);

//Module info
struct sP7Trace_Module
{
    union
    {
        sP7Ext_Header sCommon;
        sP7Ext_Raw    sCommonRaw;
    };
    tUINT16        wModuleID; 
    eP7Trace_Level eVerbosity;
    char           pName[P7TRACE_MODULE_NAME_LENGTH]; //name (UTF-8) 
} ATTR_PACK(2);


////////////////////////////////////////////////////////////////////////////////
//                             Telemetry                                      //
////////////////////////////////////////////////////////////////////////////////

//telemetry info header
struct sP7Tel_Info
{
    union
    {
        sP7Ext_Header sCommon;
        sP7Ext_Raw    sCommonRaw;
    };
    //Contains a 64-bit value representing the number of 100-nanosecond intervals 
    //since January 1, 1601 (UTC). In windows we use FILETIME structure for 
    //representing
    tUINT32       dwTime_Hi;
    tUINT32       dwTime_Lo;
    //Hi resolution timer value, we get this value when we retrieve current time.
    //using difference between this value and timer value for every trace we can
    //calculate time of the trace event with hi resolution
    tUINT64       qwTimer_Value;
    //timer's count heartbeats in second
    tUINT64       qwTimer_Frequency;
    tUINT64       qwFlags; 
    tWCHAR        pName[P7TELEMETRY_NAME_LENGTH];
} ATTR_PACK(2);


//Telemetry counter description
struct sP7Tel_Counter
{
    union
    {
        sP7Ext_Header sCommon;
        sP7Ext_Raw    sCommonRaw;
    };
    tUINT8        bID;
    tUINT8        bOn;
    tINT64        llMin;
    tINT64        llMax;
    tINT64        llAlarm;
    tWCHAR        pName[P7TELEMETRY_COUNTER_NAME_LENGTH];
} ATTR_PACK(2);

//telemetry counter On/Off verbosity header
struct sP7Tel_Enable
{
    union
    {
        sP7Ext_Header sCommon;
        sP7Ext_Raw    sCommonRaw;
    };
    tUINT8           bID;
    tUINT8           bOn;
} ATTR_PACK(2);


//Telemetry counter value
struct sP7Tel_Value
{
    union
    {
        sP7Ext_Header sCommon;
        sP7Ext_Raw    sCommonRaw;
    };
    tUINT8        bID;
    tUINT8        bSeqN; 
    tUINT64       qwTimer;      //High resolution timer value
    tINT64        llValue;
} ATTR_PACK(2);



PRAGMA_PACK_EXIT()//2///////////////////////////////////////////////////////////


#endif //P7_EXTENSIONS_H
