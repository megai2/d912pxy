#include <stdio.h>
#include <stdarg.h>
#include "GTypes.h"
#include "P7_Cproxy.h"

////////////////////////////////////////////////////////////////////////////////
//user macro for trace function calling simplification
#define P7_CRITICAL(i_pFormat, ...)\
    P7_TRACE_ADD(g_hTrace, 0, P7_TRACE_LEVEL_CRITICAL, g_pModule, i_pFormat, __VA_ARGS__)


////////////////////////////////////////////////////////////////////////////////
//example variables
static hP7_Client       g_hClient = NULL;

static hP7_Telemetry    g_hTel    = NULL;

static hP7_Trace        g_hTrace  = NULL;
static hP7_Trace_Module g_pModule = NULL;
static tUINT16          g_bTID1   = 0;
static tUINT16          g_bTID2   = 0;

static tUINT64          g_qwI     = 0;


////////////////////////////////////////////////////////////////////////////////
//Example of trace embedding into existing log function
void MyLogFunction(const tXCHAR *i_pFormat, ...)
{
    va_list l_pVl;
    va_start(l_pVl, i_pFormat);
    P7_Trace_Embedded(g_hTrace,
                      0,   
                      P7_TRACE_LEVEL_INFO, 
                      g_pModule,
                      (tUINT16)__LINE__,
                      __FILE__,
                      __FUNCTION__,
                      &i_pFormat,
                      &l_pVl
                     );
    va_end(l_pVl);
}


////////////////////////////////////////////////////////////////////////////////
//main
int main(int i_iArgC, char* i_pArgV[])
{
    //Set crash handler 
    P7_Set_Crash_Handler();

    //create client
    g_hClient = P7_Client_Create(TM("/P7.Sink=Baical /P7.Addr=127.0.0.1 /P7.Pool=16000"));
    //using the client create telemetry & trace channels
    g_hTel    = P7_Telemetry_Create(g_hClient, TM("TelemetryChannel"), NULL);
    g_hTrace  = P7_Trace_Create(g_hClient, TM("TraceChannel"), NULL);

    if (    (NULL == g_hClient)
         || (NULL == g_hTel)
         || (NULL == g_hTrace)
       )
    {
        printf("Initialization error\n");
        goto l_lblExit;
    }

    //register current application module (it isn't obligatory)
    g_pModule = P7_Trace_Register_Module(g_hTrace, TM("Main"));
    //register current application thread (it isn't obligatory)
    P7_Trace_Register_Thread(g_hTrace, TM("Main"), 0);

    //adding 2 counters into common group
    if (!P7_Telemetry_Create_Counter(g_hTel, TM("Group/counter 1"), 0, 0, 1023, 1000, 1, &g_bTID1))
    {
        printf("can't create counter\n");
        goto l_lblExit;
    }

    if (!P7_Telemetry_Create_Counter(g_hTel, TM("Group/counter 2"), 0, 0, 1023, 1000, 1, &g_bTID2))
    {
        printf("can't create counter\n");
        goto l_lblExit;
    }

    //delivering trace messages
    for (g_qwI = 0ULL; g_qwI < 1000ULL; g_qwI ++)
    {
        P7_TRACE_ADD(g_hTrace, 0, P7_TRACE_LEVEL_TRACE, g_pModule, TM("Test trace message #%I64d"), g_qwI);
        P7_TRACE_ADD(g_hTrace, 0, P7_TRACE_LEVEL_INFO,  g_pModule, TM("Test info message #%I64d"), g_qwI);
        P7_TRACE_ADD(g_hTrace, 0, P7_TRACE_LEVEL_ERROR, g_pModule, TM("Test error message #%I64d"), g_qwI);

        //user defined macro
        P7_CRITICAL(TM("Test critical message #%I64d"), g_qwI);
    }

    MyLogFunction(TM("Test embedded function, iteration: %I64d"), g_qwI);

    //delivering telemetry samples
    for (g_qwI = 0ULL; g_qwI < 100000ULL; g_qwI ++)
    {
        P7_Telemetry_Put_Value(g_hTel, g_bTID1, (tDOUBLE)(g_qwI & 0x3FFull));
        P7_Telemetry_Put_Value(g_hTel, g_bTID2, (tDOUBLE)((g_qwI + 11ull) & 0x3FFull));
    }

    //unregister current application thread (it is obligatory if thread was registered)
    P7_Trace_Unregister_Thread(g_hTrace, 0);


l_lblExit:
    if (g_hTel)
    {
        P7_Telemetry_Release(g_hTel);
        g_hTel = NULL;
    }

    if (g_hTrace)
    {
        P7_Trace_Release(g_hTrace);
        g_hTrace = NULL;
    }

    if (g_hClient)
    {
        P7_Client_Release(g_hClient);
        g_hClient = NULL;
    }

    P7_Clr_Crash_Handler();

    return 0;
}//main

