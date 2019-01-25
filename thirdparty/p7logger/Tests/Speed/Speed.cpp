#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>


#include "GTypes.h"

#include "PLock.h"
#include "PTime.h"
#include "PThreadShell.h"
#include "Ticks.h"
#include "UTF.h"
#include "AList.h"
#include "PString.h"
#include "PProcess.h"
#include "PConsole.h"

#include "P7_Client.h"
#include "P7_Trace.h"


tBOOL Print2Buffer(char       *o_pBuffer, 
                   const char *i_pFormat,
                   ...
                  )
{
    if (    (NULL == o_pBuffer)
         || (NULL == i_pFormat)
       )
    {
        return FALSE;
    }

    va_list arglist;
    va_start(arglist, i_pFormat);
    vsprintf(o_pBuffer, i_pFormat, arglist);
    va_end(arglist);

    return TRUE;
} 


#define ITERATIONS_COUNT                                                (100000)


int main(int i_iArgC, char* i_pArgV[])
{
    printf("This test measures the time it is necessary for:\n");
    printf(" 1. print %d times simple string to buffer\n", ITERATIONS_COUNT);
    printf(" 2. print %d times simple string to console\n", ITERATIONS_COUNT);
    printf("    this test combine test #1 and console printing\n");
    printf(" 3. print & deliver %d times simple string to trace server\n", ITERATIONS_COUNT);
    printf("    server will receive:\n");
    printf("       * Text message\n");
    printf("       * Level (error, warning, .. etc)\n");
    printf("       * Time with 100ns granularity\n");
    printf("       * Current thread ID\n");
    printf("       * Current module ID\n");
    printf("       * Current processor number\n");
    printf("       * Function name\n");
    printf("       * File name\n");
    printf("       * File line number\n");
    printf("       * Sequence number\n");
    printf("*************************************************************************\n");
    printf("Press ENTER to start ....\n");

    Get_Char();

    tUINT64 l_qwTime   = 0;
    tUINT64 l_qwTime_1 = 0;
    tUINT64 l_qwTime_2 = 0;
    tUINT64 l_qwTime_3 = 0;

    ////////////////////////////////////////////////////////////////////////////
    //Test 1
    char l_pText[4096];
    l_qwTime = GetPerformanceCounter();

    for (tUINT32 l_dwI = 0; l_dwI < ITERATIONS_COUNT; l_dwI ++)
    {
        Print2Buffer(l_pText, "[%s][%s][%d][%d][%d] Test format string, iteration is %d",
                     __FILE__,
                     __FUNCTION__, 
                     __LINE__, 
                     CProc::Get_Thread_Id(),
                     CProc::Get_Processor(),
                     l_dwI);
    }

    l_qwTime_1 = (GetPerformanceCounter() - l_qwTime) / (GetPerformanceFrequency() / 1000ull);


    ////////////////////////////////////////////////////////////////////////////
    //Test 2
    l_qwTime = GetPerformanceCounter();

    for (tUINT32 l_dwI = 0; l_dwI < ITERATIONS_COUNT; l_dwI ++)
    {
        printf(l_pText, "[%s][%s][%d][%d][%d] Test format string, iteration is %d",
               __FILE__,
               __FUNCTION__, 
               __LINE__, 
               CProc::Get_Thread_Id(),
               CProc::Get_Processor(),
               l_dwI
              );
    }

    l_qwTime_2 = (GetPerformanceCounter() - l_qwTime) / (GetPerformanceFrequency() / 1000ull);


    ////////////////////////////////////////////////////////////////////////////
    //Test 3
    IP7_Client *l_pClient = P7_Create_Client(TM("/P7.PSize=1472 /P7.Pool=65536"));
    IP7_Trace  *l_pTrace  = P7_Create_Trace(l_pClient, TM("Speed Test"));

    if (    (l_pClient)
         && (l_pTrace)
       )
    {
        l_qwTime = GetPerformanceCounter();

        for (tUINT32 l_dwI = 0; l_dwI < ITERATIONS_COUNT; l_dwI ++)
        {
            l_pTrace->P7_QTRACE(1, 0, TM("Test format string, iteration is %d"), l_dwI);
        }

        l_qwTime_3 = (GetPerformanceCounter() - l_qwTime) / (GetPerformanceFrequency() / 1000ull);
    }

    if (l_pTrace)
    {
        l_pTrace->Release();
        l_pTrace = NULL;
    }

    if (l_pClient)
    {
        l_pClient->Release();
        l_pClient = NULL;
    }


    printf("\n*****************************************************************\n");
    printf("Test results:\n");

    

    printf("  Test 1 (to buffer) duration: %d ms, %d per second\n", 
           (tUINT32)l_qwTime_1,
           (tUINT32)((1000.0 / (tDOUBLE)l_qwTime_1) * (tDOUBLE)ITERATIONS_COUNT)
          );
    printf("  Test 2 (to console) duration: %d ms, %d per second\n", 
           (tUINT32)l_qwTime_2,
           (tUINT32)((1000.0 / (tDOUBLE)l_qwTime_2) * (tDOUBLE)ITERATIONS_COUNT)
          );
    printf("  Test 3 (to P7 trace) duration: %d ms, %d per second\n", 
           (tUINT32)l_qwTime_3,
           (tUINT32)((1000.0 / (tDOUBLE)l_qwTime_3) * (tDOUBLE)ITERATIONS_COUNT)
          );

    return 0;
}

