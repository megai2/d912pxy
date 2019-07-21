#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>
#include <math.h>
#include <stdint.h>

#include "GTypes.h"

#include "Length.h"

#include "PAtomic.h"
#include "PLock.h"
#include "PString.h"
#include "PTime.h"
#include "IMEvent.h"
#include "PMEvent.h"
#include "PThreadShell.h"
#include "CRC32.h"
#include "RBTree.h"
#include "AList.h"

#include "Ticks.h"
#include "PString.h"
#include "WString.h"
#include "PFileSystem.h"
#include "PConsole.h"

#include "UTF.h"
#include "PProcess.h"
#include "IJournal.h"
#include "PJournal.h"

#include "P7_Client.h"
#include "P7_Trace.h"
#include "P7_Telemetry.h"


#if !defined(UINTMAX_MAX)
    typedef uint64_t uintmax_t;
    #define UINTMAX_MAX	0xffffffffffffffffU
#endif

#if !defined(INTMAX_MAX)
    typedef int64_t intmax_t;
    #define INTMAX_MIN		(-0x7fffffffffffffff - 1)
    #define INTMAX_MAX		0x7fffffffffffffff
#endif


#if defined(_MSC_VER) 

    #pragma warning(disable : 4267)
    #pragma warning(disable : 4995)
    #pragma warning(disable : 4996)
    //allow to see dump of memory leaks at the end of test in VS debug output
    #define _CRTDBG_MAP_ALLOC
    #include <crtdbg.h>

    #define DUMP_MEMORY_LEAKS()  _CrtDumpMemoryLeaks()
    #define SPRINTF              swprintf
    #define STRNICMP             strnicmp

#elif defined (__GNUC__)

    #define DUMP_MEMORY_LEAKS()  printf("memory leaks detection is missing !")
    #define SPRINTF              sprintf
    #define STRNICMP             strncasecmp

#endif



#define TEST_HELP                                                      "/?"
#define TEST_NUMBER                                                    "/Test="


////////////////////////////////////////////////////////////////////////////////
//                                   TEST 1                                   //
////////////////////////////////////////////////////////////////////////////////
// Here we made simple test Create and destroy P7 client and P7 trace         //
// and using shared objects for P7 client & trace                             //
////////////////////////////////////////////////////////////////////////////////

#define TEST_01_COUNT                                                  "/Count="
#define TEST_01_P7_TRACE_SHARE_NAME                         TM("P7.Trace.Share")
#define TEST_01_P7_CLIENT_SHARE_NAME                              TM("P7.Share")


////////////////////////////////////////////////////////////////////////////////
//Test_01
int Test_01(int i_iArgC, char* i_pArgV[])
{
    IP7_Client        *l_pClient  = NULL;
    IP7_Client        *l_pClientS = NULL;
    IP7_Trace         *l_pTrace   = NULL;
    IP7_Trace::hModule l_hModule  = NULL;
    IP7_Trace         *l_pTraceS  = NULL;
    tUINT32            l_dwCount  = 1000;
    tXCHAR             l_pChannel[128];
    tBOOL              l_bError   = FALSE;

    for (int l_iI = 0; l_iI < i_iArgC; l_iI++)
    {
        if (0 == STRNICMP(i_pArgV[l_iI], TEST_HELP, LENGTH(TEST_HELP)-1))
        {
            printf("This test create and destroy P7 client and P7 trace in loop\n");
            printf("Command line arguments:\n");
            printf("/Count    : Count iterations\n");
            printf("            Default value - 1000, min 100, max - uint32\n");
            printf("            Example: /Count=10000\n");
            goto l_lExit;
        }
        else if (0 == STRNICMP(i_pArgV[l_iI], TEST_01_COUNT, LENGTH(TEST_01_COUNT)-1))
        {
            sscanf(i_pArgV[l_iI] + LENGTH(TEST_01_COUNT)-1, "%d", &l_dwCount);
            if (10 > l_dwCount)
            {
                l_dwCount = 10;
            }
        }
    }

    printf("Start: Iterations count %d\n", l_dwCount);
    
    for (tUINT32 l_dwI = 0; l_dwI < l_dwCount; l_dwI++)
    {
        printf("Iteration = %d\n", l_dwI);

        l_pClient = P7_Create_Client(NULL);

        if (NULL == l_pClient)
        {
            printf("Error: P7 engine was not initialized. Iteration = %d, Exit\n", l_dwI);
            l_bError = TRUE;
        }
        else
        {
            l_pClient->Share(TEST_01_P7_CLIENT_SHARE_NAME);
            l_pClientS = P7_Get_Shared(TEST_01_P7_CLIENT_SHARE_NAME);

            if (l_pClientS)
            {
                l_pClient->Release();
                l_pClient = l_pClientS;
                l_pClientS = NULL;
            }
            else
            {
                printf("Error: P7_Get_Shared failed. Iteration = %d\n", l_dwI);
            }


            for (tUINT32 l_dwJ = 0; l_dwJ < 2; l_dwJ++)
            {
                SPRINTF(l_pChannel, TM("Chanel[%03d - %02d]"), l_dwI, l_dwJ);
                l_pTrace = P7_Create_Trace(l_pClient, l_pChannel);

                if (l_pTrace)
                {
                    if (!l_pTrace->Register_Thread(TM("Application"), 0))
                    {
                        printf("Error: Register_Thread failed. Iteration = %d\n", l_dwJ);
                    }

                    if (!l_pTrace->Register_Module(TM("Test #1"), &l_hModule))
                    {
                        printf("Error: Register_Module failed. Iteration = %d\n", l_dwJ);
                    }

                    for (tUINT32 l_dwK = 0; l_dwK < 2000; l_dwK ++)
                    {
                        l_pTrace->P7_INFO(l_hModule, TM("Iteration %d"), l_dwK);
                    }

                    if (l_pTrace->Share(TEST_01_P7_TRACE_SHARE_NAME))
                    {
                        l_pTraceS = P7_Get_Shared_Trace(TEST_01_P7_TRACE_SHARE_NAME);
                        if (l_pTraceS)
                        {
                            if (!l_pTraceS->Register_Module(TM("Test #1"), &l_hModule))
                            {
                                printf("Error: Register_Module failed. Iteration = %d\n", l_dwJ);
                            }

                            for (tUINT32 l_dwK = 0; l_dwK < 2000; l_dwK ++)
                            {
                                l_pTraceS->P7_INFO(l_hModule, TM("Share: Iteration %d"), l_dwK);
                            }

                            l_pTraceS->Release();
                        }
                    }

                    if (!l_pTrace->Unregister_Thread(0))
                    {
                        printf("Error: Unregister_Thread() failed\n");
                    }

                    l_pTrace->Release();
                    l_pTrace = NULL;

                    CThShell::Sleep(50);
                }
                else
                {
                    printf("Error: P7_Create_Trace failed. Iteration = %d, Exit\n", l_dwI);
                    l_bError = TRUE;
                }
            }

            l_pClient->Release();
            l_pClient = NULL;
        }

        if (l_bError)
        {
            break;
        }
    }

l_lExit:
    if (l_pClient)
    {
        l_pClient->Release();
        l_pClient = NULL;
    }

    return 0;
}//Test_01


////////////////////////////////////////////////////////////////////////////////
//                                   TEST 2                                   //
////////////////////////////////////////////////////////////////////////////////
// Here we check ability of few P7 trace to work in parallel                  //
////////////////////////////////////////////////////////////////////////////////

#define TEST_02_CHANNELS_COUNT_MAX                                          (10)

//from 1(~1000 traces per second) to 10(~10 000 traces per second)
#define TEST_02_SPEED_MAX                                                   (10)


#define TEST_02_CONSOLE_ARG_COUNT                                  "/Count="
#define TEST_02_CONSOLE_ARG_SPEED                                  "/Speed="
#define TEST_02_CONSOLE_ARG_DURATION                               "/Duration="


struct sTest_02_Thread
{
    IP7_Client       *pClient;
    IMEvent          *iEvent;
    tUINT32           dwIndex;
    tUINT32           dwSpeed;
    CThShell::tTHREAD hThread;
    tBOOL             bThread;
};


////////////////////////////////////////////////////////////////////////////////
//Test_02_Embedded_Trace
static void Test_02_Embedded_Trace(IP7_Trace         *i_pTrace, 
                                   IP7_Trace::hModule i_hModule,
                                   const tXCHAR      *i_pFormat, 
                                   ...
                                  )
{
    if (i_pTrace)
    {
        va_list l_pVl;
        va_start(l_pVl, i_pFormat);
        i_pTrace->Trace_Embedded(0, 
                                 EP7TRACE_LEVEL_INFO, 
                                 i_hModule, 
                                 (tUINT16)__LINE__,
                                 __FILE__,
                                 __FUNCTION__,
                                 &i_pFormat,
                                 &l_pVl
                                );
        va_end(l_pVl);
    }
}//Test_02_Embedded_Trace


////////////////////////////////////////////////////////////////////////////////
//Test_02_Routine
static THSHELL_RET_TYPE THSHELL_CALL_TYPE Test_02_Routine(void *i_pContext)
{
    sTest_02_Thread   *l_pParam     = (sTest_02_Thread*)i_pContext;
    const size_t       l_szModules  = 16;
    IP7_Trace::hModule l_hModules[l_szModules] = {};
    IP7_Trace         *l_pTrace     = NULL;
    tUINT32            l_dwSent     = 0;
    tUINT32            l_dwRejected = 0;
    tUINT64            l_qwIDX      = 0;
    tUINT32            l_dwTime     = GetTickCount();
    tXCHAR             l_pName[256];

    if (NULL == l_pParam)
    {
        printf("Error: thread input parameter is NULL\n");
        return 0;
    }

    SPRINTF(l_pName, TM("Trace Channel %d"), l_pParam->dwIndex);

    l_pTrace = P7_Create_Trace(l_pParam->pClient, l_pName);

    if (NULL == l_pTrace)
    {
        printf("Thread %d: Error: P7_Create_Trace() failed\n", l_pParam->dwIndex);
        return 0;
    }

    for (size_t l_szI = 0; l_szI < l_szModules; l_szI++)
    {
        SPRINTF(l_pName, TM("Module #%d:%d"), l_pParam->dwIndex, (int)l_szI);
        if (!l_pTrace->Register_Module(l_pName, &l_hModules[l_szI]))
        {
            printf("Thread %d: Error: Register_Module() failed\n", l_pParam->dwIndex);
        }
    }

    SPRINTF(l_pName, TM("Thread #%d"), l_pParam->dwIndex);
    if (!l_pTrace->Register_Thread(l_pName, 0))
    {
        printf("Thread %d: Error: Register_Thread() failed\n", l_pParam->dwIndex);
    }

    l_pTrace->Set_Verbosity(NULL, EP7TRACE_LEVEL_TRACE);

    Test_02_Embedded_Trace(l_pTrace,
                           l_hModules[0],
                           TM("Create thread %X"), 
                           CProc::Get_Thread_Id()
                          );

    while (MEVENT_TIME_OUT == l_pParam->iEvent->Wait(10))
    {
        for (tUINT32 l_dwI = 0; l_dwI < 10 * l_pParam->dwSpeed; l_dwI++)
        {
            l_qwIDX++;
            if (l_pTrace->P7_QTRACE(1, 
                                    l_hModules[(size_t)l_qwIDX % l_szModules],
                                    TM("Test 1[%I64d] %d, %d, %s %d"), 
                                    l_qwIDX, 
                                    10, 
                                    20, 
                                    TM("Ups01"), 
                                    l_dwSent
                                   )
               )
            {
                l_dwSent ++;
            }
            else
            {
                l_dwRejected ++;
            }

            if (0 == (l_qwIDX % 10))
            {
                l_qwIDX ++;
                if (l_pTrace->P7_DEBUG(l_hModules[(size_t)l_qwIDX % l_szModules], TM("Debug message(3) %s %d, %I64d"), TM("P7_DEBUG"), l_dwSent, l_qwIDX))
                {
                    l_dwSent ++;
                }
                else
                {
                    l_dwRejected ++;
                }
            }

            if (0 == (l_qwIDX % 22))
            {
                l_qwIDX ++;
                if (l_pTrace->P7_INFO(l_hModules[(size_t)l_qwIDX % l_szModules], TM("Info message(1) %s"), TM("P7_INFO")))
                {
                    l_dwSent ++;
                }
                else
                {
                    l_dwRejected ++;
                }
            }

            if (0 == (l_qwIDX % 153))
            {
                l_qwIDX ++;
                if (l_pTrace->P7_WARNING(l_hModules[(size_t)l_qwIDX % l_szModules], TM("Warning message(1) %d"), l_dwSent))
                {
                    l_dwSent ++;
                }
                else
                {
                    l_dwRejected ++;
                }
            }


            if (0 == (l_qwIDX % 155))
            {
                l_qwIDX ++;
                if (l_pTrace->P7_ERROR(l_hModules[(size_t)l_qwIDX % l_szModules], TM("ERROR message(2) %I64d, %s"), l_qwIDX, TM("P7_ERROR")))
                {
                    l_dwSent ++;
                }
                else
                {
                    l_dwRejected ++;
                }
            }
        }//for (tUINT32 l_dwI = 0; l_dwI < 10 * TEST_SPEED; l_dwI++)


        if (CTicks::Difference(GetTickCount(), l_dwTime) >= 1000)
        {
            printf("Thread %d: Traces %d (%d) / %d ms\n",
                    l_pParam->dwIndex,
                    l_dwSent,
                    l_dwRejected,
                    CTicks::Difference(GetTickCount(), l_dwTime)
                   );
            l_dwTime = GetTickCount();
            l_dwSent  = 0;
            l_dwRejected  = 0;
        }
    }//while (WAIT_TIMEOUT == WaitForSingleObject(g_pEvent_Exit, 10))

    Test_02_Embedded_Trace(l_pTrace, 
                           l_hModules[0],
                           TM("Leave thread %X"), 
                           CProc::Get_Thread_Id()
                          );

    l_pTrace->P7_CRITICAL(l_hModules[0], TM("All done, bye bye"), 0);

    if (!l_pTrace->Unregister_Thread(0))
    {
        printf("Error: Unregister_Thread() failed\n");
    }

    if (l_pTrace)
    {
        l_pTrace->Release();
        l_pTrace = NULL;
    }

    CThShell::Cleanup();

    return THSHELL_RET_OK;
}//Test_02_Routine


////////////////////////////////////////////////////////////////////////////////
//Test_02
int Test_02(int i_iArgC, char* i_pArgV[])
{
    IP7_Client     *l_pClient    = NULL;
    tBOOL           l_bExit      = FALSE;
    tUINT32         l_dwCount    = 1;
    tUINT32         l_dwSpeed    = 1;
    tUINT32         l_dwDuration = 0;
    tUINT32         l_dwStart    = GetTickCount();
    CMEvent        *l_pEvent     = NULL; 
    sTest_02_Thread l_pThreads[TEST_02_CHANNELS_COUNT_MAX];

    memset(l_pThreads, 0, sizeof(l_pThreads));

    for (int l_iI = 0; l_iI < i_iArgC; l_iI++)
    {
        if (0 == STRNICMP(i_pArgV[l_iI], TEST_HELP, LENGTH(TEST_HELP)-1))
        {
            printf("This test create one P7 client and defined by user amount of threads\n");
            printf("each thread create own P7 Trace object and start sending traces with\n");
            printf("defined by used speed\n");
            printf("Command line arguments:\n");
            printf("/Count    : Count of simultaneously opened streams\n");
            printf("            Default value - 2, min 1, max 10\n");
            printf("            Example: /Count=10\n");
            printf("/Speed    : Delivery Speed, min 1, max 10000\n");
            printf("            Default value - 1\n");
            printf("            Example: /Speed=5\n");
            printf("/Duration : Working duration in seconds\n");
            printf("            Default value is 0 - infinite\n");
            printf("            Example: /Duration=20\n");
            printf("/P7.Help  : P7 engine command line help\n");
            goto l_lExit;
        }
        else if (0 == STRNICMP(i_pArgV[l_iI], TEST_02_CONSOLE_ARG_COUNT, LENGTH(TEST_02_CONSOLE_ARG_COUNT)-1))
        {
            sscanf(i_pArgV[l_iI] + LENGTH(TEST_02_CONSOLE_ARG_COUNT)-1, "%d", &l_dwCount);
            if (    (1 > l_dwCount)
                 || (10 < l_dwCount)
               )
            {
                l_dwCount = 2;
            }
        }
        else if (0 == STRNICMP(i_pArgV[l_iI], TEST_02_CONSOLE_ARG_SPEED, LENGTH(TEST_02_CONSOLE_ARG_SPEED)-1))
        {
            sscanf(i_pArgV[l_iI] + LENGTH(TEST_02_CONSOLE_ARG_SPEED)-1, "%d", &l_dwSpeed);
            if (    (1 > l_dwSpeed)
                 || (10000 < l_dwSpeed)
               )
            {
                l_dwSpeed = 1;
            }
        }
        else if (0 == STRNICMP(i_pArgV[l_iI], TEST_02_CONSOLE_ARG_DURATION, LENGTH(TEST_02_CONSOLE_ARG_DURATION)-1))
        {
            sscanf(i_pArgV[l_iI] + LENGTH(TEST_02_CONSOLE_ARG_DURATION)-1, "%d", &l_dwDuration);
        }
    }

    l_pClient = P7_Create_Client(NULL);
    if (NULL == l_pClient)
    {
        printf("Error: P7 engine was not initialized. Exit\n");
        goto l_lExit;
    }

    l_pEvent = new CMEvent(); 

    if (    (NULL == l_pEvent)
         || (FALSE == l_pEvent->Init(1, EMEVENT_SINGLE_MANUAL))
       )
    {
        printf("Error: Event was not created\n");
        goto l_lExit;
    }

    printf("Start: Duration %d, Delivery speed %d, Channels count %d. Press Esc to exit\n",
           l_dwDuration,
           l_dwSpeed,
           l_dwCount
          );

    for (tUINT32 l_dwI = 0; l_dwI < l_dwCount; l_dwI++)
    {
        l_pThreads[l_dwI].dwIndex = l_dwI;
        l_pThreads[l_dwI].pClient = l_pClient;
        l_pThreads[l_dwI].dwSpeed = l_dwSpeed;
        l_pThreads[l_dwI].iEvent  = l_pEvent;
        l_pThreads[l_dwI].bThread = TRUE;

        if (FALSE == CThShell::Create(&Test_02_Routine, 
                                      &l_pThreads[l_dwI], 
                                      &l_pThreads[l_dwI].hThread,
                                      TM("Test 02")           
                                     )
           )
        {
            l_pThreads[l_dwI].bThread = FALSE;
            printf("Failed to create thread\n");
        }
    }

    while (FALSE == l_bExit)
    {
        CThShell::Sleep(500);

        if (    (Is_Key_Hit())
             && (27 == Get_Char())
           )
        {
            printf("Esc ... exiting\n");
            l_bExit = TRUE;
        }

        if (    (l_dwDuration)
             && (CTicks::Difference(GetTickCount(), l_dwStart) >= (l_dwDuration * 1000))
           )
        {
            printf("Working time is expired ... exiting\n");
            l_bExit = TRUE;
        }
    }

    l_pEvent->Set(0);

l_lExit:

    for (tUINT32 l_dwI = 0; l_dwI < l_dwCount; l_dwI++)
    {
        if (l_pThreads[l_dwI].bThread)
        {
            if (FALSE == CThShell::Close(l_pThreads[l_dwI].hThread, 60000))
            {
                printf("ERROR: thread %d timeout !\n", l_dwI);
            }
        }
    }

    if (l_pEvent)
    {
        delete l_pEvent;
        l_pEvent = NULL;
    }

    if (l_pClient)
    {
        l_pClient->Release();
        l_pClient = NULL;
    }

    return 0;
}//Test_02



////////////////////////////////////////////////////////////////////////////////
//                                   TEST 3                                   //
////////////////////////////////////////////////////////////////////////////////
// Here we check ability of few P7 trace to work with different formats       //
////////////////////////////////////////////////////////////////////////////////

#define TEST_03_COUNT                                                  "/Count="

////////////////////////////////////////////////////////////////////////////////
//Test_03
int Test_03(int i_iArgC, char* i_pArgV[])
{
    IP7_Client        *l_pClient  = NULL;
    tBOOL              l_bExit    = FALSE;
    tUINT32            l_dwCount  = 1;
    IP7_Trace         *l_pTrace   = NULL;
    IP7_Trace::hModule l_hModule  = NULL;
    const tXCHAR      *l_pName    = TM("Formats test");

    UNUSED_ARG(l_bExit);

    for (int l_iI = 0; l_iI < i_iArgC; l_iI++)
    {
        if (0 == STRNICMP(i_pArgV[l_iI], TEST_HELP, LENGTH(TEST_HELP)-1))
        {
            printf("This test create one P7 client and one P7.Trace for sending\n");
            printf("different traces with all possible argument's formats\n");
            printf("Command line arguments:\n");
            printf("/Count    : Count iterations\n");
            printf("            Default value - 1, min 1, max - uint32\n");
            printf("            Example: /Count=10000\n");
            goto l_lExit;
        }
        else if (0 == STRNICMP(i_pArgV[l_iI], TEST_03_COUNT, LENGTH(TEST_03_COUNT)-1))
        {
            sscanf(i_pArgV[l_iI] + LENGTH(TEST_01_COUNT) - 1, "%d", &l_dwCount);
            if (1 > l_dwCount)
            {
                l_dwCount = 1;
            }
        }
    }


    l_pClient = P7_Create_Client(NULL);
    if (NULL == l_pClient)
    {
        printf("Error: P7 engine was not initialized. Exit\n");
        goto l_lExit;
    }

    l_pTrace = P7_Create_Trace(l_pClient, l_pName);
    if (NULL == l_pClient)
    {
        printf("Error: P7.Trace engine was not initialized. Exit\n");
        goto l_lExit;
    }


    if (!l_pTrace->Register_Module(TM("Test #3"), &l_hModule))
    {
        printf("Error: Register_Module() failed\n");
    }


    if (!l_pTrace->Register_Thread(TM("Application"), 0))
    {
        printf("Error: Register_Thread() failed\n");
    }

    for (tUINT32 l_iI = 0; l_iI < l_dwCount; l_iI++)
    {
        ////////////////////////////////////////////////////////////////////////////
        l_pTrace->P7_DEBUG(l_hModule, TM("Strings test"), 0);
        l_pTrace->P7_INFO(l_hModule, 
                          TM("String: %s (%%s = 12345 abcde .. z // \\ ABCDE .. Z)"), 
                          TM("12345 abcde .. z // \\ ABCDE .. Z")
                         );

        l_pTrace->P7_INFO(l_hModule, 
                          TM("String: %.*s (%%.*s = ([x]s is unsupported by P7))"), 
                          0, TM("12345 abcde .. z // \\ ABCDE .. Z")
                         );

        l_pTrace->P7_INFO(l_hModule, 
                          TM("String: %ls (%%ls = 12345 abcde .. z // \\ ABCDE .. Z)"), 
                          L"12345 abcde .. z // \\ ABCDE .. Z"
                         );

        l_pTrace->P7_INFO(l_hModule, 
                          TM("String: %.10ls (%%.10ls = ([x]s is unsupported by P7))"), 
                          L"12345 abcde .. z // \\ ABCDE .. Z"
                         );

        l_pTrace->P7_INFO(l_hModule, 
                          TM("String: %hs (%%hs = 12345 abcde .. z // \\ ABCDE .. Z)"), 
                          "12345 abcde .. z // \\ ABCDE .. Z"
                         );

        l_pTrace->P7_INFO(l_hModule, 
                          TM("String: %.10hs (%%.10hs = ([x]s is unsupported by P7))"), 
                          "12345 abcde .. z // \\ ABCDE .. Z"
                         );

        l_pTrace->P7_INFO(l_hModule, 
                          TM("String: %ws (%%ws = 12345 abcde .. z // \\ ABCDE .. Z)"), 
                          TM("12345 abcde .. z // \\ ABCDE .. Z")
                         );

        l_pTrace->P7_INFO(l_hModule, 
                          TM("String: %.10ws (%%.10ws = ([x]s is unsupported by P7))"), 
                          TM("12345 abcde .. z // \\ ABCDE .. Z")
                         );

        l_pTrace->P7_INFO(l_hModule, 
                          TM("String: %hS (%%hS = 12345 abcde .. z // \\ ABCDE .. Z)"), 
                          "12345 abcde .. z // \\ ABCDE .. Z"
                         );

        l_pTrace->P7_INFO(l_hModule, 
                          TM("String: %.*hS (%%.*hS = ([x]s is unsupported by P7))"), 
                          10, "12345 abcde .. z // \\ ABCDE .. Z"
                         );

        l_pTrace->P7_INFO(l_hModule,
                          TM("String: %s (%%s = NULL)"),
                          NULL
                         );

        l_pTrace->P7_INFO(l_hModule,
                          TM("String: %.10s (%%.10s = ([x]s is unsupported by P7))"),
                          NULL
                         );

        l_pTrace->P7_INFO(l_hModule,
                          TM("String: %hs (%%hs = NULL)"),
                          NULL
                         );

        l_pTrace->P7_INFO(l_hModule,
                          TM("String: %ls (%%ls = NULL)"),
                          NULL
                         );

        l_pTrace->P7_INFO(l_hModule,
                          TM("String: %s (%%s = \"\")"),
                          TM("")
                         );

        l_pTrace->P7_INFO(l_hModule,
                          TM("String: %hs (%%hs = \"\")"),
                          ""
                         );

        l_pTrace->P7_INFO(l_hModule,
                          TM("String: %ls (%%ls = \"\")"),
                          L""
                         );


        ////////////////////////////////////////////////////////////////////////////
        l_pTrace->P7_DEBUG(l_hModule, TM("Characters test"), 0);

        l_pTrace->P7_INFO(l_hModule, 
                          TM("Char: %c (%%c = G)"), 
                          TM('G')
                         );

        l_pTrace->P7_INFO(l_hModule, 
                          TM("Char: %hc (%%hc = G)"), 
                          'G'
                         );

        l_pTrace->P7_INFO(l_hModule, 
                          TM("Char: %C (%%C = F)"), 
                          'F'
                         );

        l_pTrace->P7_INFO(l_hModule, 
                          TM("Char: %hC (%%hC = R)"), 
                          'R'
                         );


        l_pTrace->P7_INFO(l_hModule, 
                          TM("Char: %lc (%%lc = S)"), 
                          TM('S')
                         );

        l_pTrace->P7_INFO(l_hModule, 
                          TM("Char: %wc (%%wc = Z)"), 
                          TM('Z')
                         );


        ////////////////////////////////////////////////////////////////////////////
        l_pTrace->P7_DEBUG(l_hModule, TM("Integer values"), 0);

        l_pTrace->P7_INFO(l_hModule, 
                          TM("Decamical %d (%%d = 1234567890) | %i (%%i = 1234567890) | %u (%%u = 1234567890) "), 
                          (tINT32)1234567890,
                          (tINT32)1234567890,
                          (tUINT32)1234567890
                         );

        l_pTrace->P7_INFO(l_hModule, 
                          TM("Decamical %jd (%%jd = 1234567890) | %ji (%%ji = 1234567890) | %ju (%%ju = 1234567890) "), 
                          (intmax_t)1234567890,
                          (intmax_t)1234567890,
                          (uintmax_t)1234567890
                         );

        l_pTrace->P7_INFO(l_hModule, 
                          TM("Octal  %o (%%o = 11145401322) | Octal  %jo (%%jo = 11145401322)"), 
                          (tUINT32)1234567890,
                          (uintmax_t)1234567890
                         );

        l_pTrace->P7_INFO(l_hModule, 
                          TM("Hex  %x (%%x = 499602d2) | %X (%%X = 499602D2), %jX (%%jX = 499602D2)"), 
                          (tUINT32)1234567890,
                          (tUINT32)1234567890,
                          (uintmax_t)1234567890
                         );

        l_pTrace->P7_INFO(l_hModule, 
                          TM("Decamical %ld (%%ld = 1234567890) | %li (%%li = 1234567890) | %lu (%%lu = 1234567890) "), 
                          (long int)1234567890,
                          (long int)1234567890,
                          (unsigned long int)1234567890
                         );

        l_pTrace->P7_INFO(l_hModule, 
                          TM("Octal  %lo (%%lo = 11145401322)"), 
                          (unsigned long int)1234567890
                         );


        l_pTrace->P7_INFO(l_hModule, 
                          TM("Hex  %lx (%%lx = 499602d2) | %lX (%%lX = 499602D2)"), 
                          (unsigned long int)1234567890,
                          (unsigned long int)1234567890
                         );


        l_pTrace->P7_INFO(l_hModule, 
                          TM("Decamical %lld (%%lld = 1234567890987) | %lli (%%lli = 1234567890987ULL)"), 
                          (tINT64)1234567890987ULL,
                          (tINT64)1234567890987ULL
                         );


        l_pTrace->P7_INFO(l_hModule, 
                          TM("Octal  %llo (%%llo = 21756176604053ULL)"), 
                          (tUINT64)1234567890987ULL
                         );


        l_pTrace->P7_INFO(l_hModule, 
                          TM("Hex  %llx (%%llx = ffaabbccee12) | %llX (%%llX = FFAABBCCEE12)"), 
                          (tUINT64)0xffaabbccee12ULL,
                          (tUINT64)0xFFAABBCCEE12ULL
                         );



        l_pTrace->P7_INFO(l_hModule, 
                          TM("Decamical %hd (%%hd = 32700) | %hi (%%hi = 32700) | %hu (%%hu = 65500) "), 
                          (tINT16)32700,
                          (tINT16)32700,
                          (tUINT16)65500
                         );

        l_pTrace->P7_INFO(l_hModule, 
                          TM("Octal  %ho (%%ho = 77674)"), 
                          (tUINT16)32700
                         );


        l_pTrace->P7_INFO(l_hModule, 
                          TM("Hex  %hx (%%hx = AABB) | %hX (%%hX = AABB)"), 
                          (tUINT16)0xAABB,
                          (tUINT16)0xAABB
                         );



        l_pTrace->P7_INFO(l_hModule, 
                          TM("Decamical %I32d (%%I32d = 1234567890) | %I32i (%%I32i = 1234567890) | %I32u (%%I32u = 1234567890) "), 
                          (tINT32)1234567890,
                          (tINT32)1234567890,
                          (tUINT32)1234567890
                         );

        l_pTrace->P7_INFO(l_hModule, 
                          TM("Octal  %I32o (%%I32o = 11145401322)"), 
                          (tUINT32)1234567890
                         );


        l_pTrace->P7_INFO(l_hModule, 
                          TM("Hex  %I32x (%%I32x = 499602d2) | %I32X (%%I32X = 499602D2)"), 
                          (tUINT32)1234567890,
                          (tUINT32)1234567890
                         );



        l_pTrace->P7_INFO(l_hModule, 
                          TM("Decamical %I64d (%%I64d = 1234567890987) | %I64i (%%I64i = 1234567890987)"), 
                          (tINT64)1234567890987ULL,
                          (tINT64)1234567890987ULL
                         );


        l_pTrace->P7_INFO(l_hModule, 
                          TM("Octal  %I64o (%%I64o = 21756176604053)"), 
                          (tUINT64)1234567890987ULL
                         );


        l_pTrace->P7_INFO(l_hModule, 
                          TM("Hex  %llx (%%llx = ffaabbccee12) | %llX (%%llX = FFAABBCCEE12)"),
                          (tUINT64)0xffaabbccee12ULL,
                          (tUINT64)0xFFAABBCCEE12ULL
                         );

        l_pTrace->P7_INFO(l_hModule,
                          TM("Bin %b (%%b = 10101010101010101) | %#b (%%#b = b10101010101010101)"),
                          (tUINT32)0x15555,
                          (tUINT32)0x15555
                         );


        ////////////////////////////////////////////////////////////////////////////
        l_pTrace->P7_DEBUG(l_hModule, TM("Floating point"), 0);


        l_pTrace->P7_INFO(l_hModule, 
                          TM("%f (%%f 123456.7890) | %e (%%e) | %E (%%E) | %G (%%G) | %g (%%g) | %A (%%A) | %a (%%a)"), 
                          (tDOUBLE)123456.7890,
                          (tDOUBLE)123456.7890,
                          (tDOUBLE)123456.7890,
                          (tDOUBLE)123456.7890,
                          (tDOUBLE)123456.7890,
                          (tDOUBLE)123456.7890,
                          (tDOUBLE)123456.7890
                         );


        ////////////////////////////////////////////////////////////////////////////
        l_pTrace->P7_DEBUG(l_hModule, TM("Pointer"), 0);

    #if defined(GTX64)
        l_pTrace->P7_DEBUG(l_hModule, 
                           TM("%p (0xDEADBEEFCDCDCDCD), %s (Some string)"), 
                           0xDEADBEEFCDCDCDCDull,
                           TM("Some string"));
    #else
        l_pTrace->P7_DEBUG(l_hModule, 
                           TM("%p (0xDEADBEEF), %s (Some string)"), 
                           0xDEADBEEFu,
                           TM("Some string"));
    #endif

        ////////////////////////////////////////////////////////////////////////////
        l_pTrace->P7_DEBUG(l_hModule, TM("Long VA test"), 0);

        l_pTrace->P7_INFO(l_hModule, 
                          TM("%s (String1), %.*s ([x]s is unsupported by P7) %d(1), %d(2), %d(3), %d(4) ")
                          TM("%I64X(ABCDEF12345678) %d(5), %d(6), %d(7), %d(8) ")
                          TM("%llx(abcdef12345678) %d(9), %d(10), %d(11), %d(12) ")
                          TM("%f(987654321.12345), %d(13), %jd(100500), %d(14), %d(15), %d(16) ")
                          TM("%c(X) %d(17), %d(18), %d(19), %d(20) ")
                          TM("%hd(32000) %d(21), %d(22), %d(23), %d(24) ")
                          TM("%s(String2) %d(25), %d(26), %d(27), %d(28), %.04f(123456.7890), % 5lld(   30)"),
                          TM("String1"),
                          100,
                          TM("String2"),
                          (tINT32)1,
                          (tINT32)2,
                          (tINT32)3,
                          (tINT32)4,
                          (tUINT64)0xABCDEF12345678ULL,
                          (tINT32)5,
                          (tINT32)6,
                          (tINT32)7,
                          (tINT32)8,
                          (tUINT64)0xABCDEF12345678ULL,
                          (tINT32)9,
                          (tINT32)10,
                          (tINT32)11,
                          (tINT32)12,
                          (tDOUBLE)987654321.12345,
                          (tINT32)13,
                          (uintmax_t)100500,
                          (tINT32)14,
                          (tINT32)15,
                          (tINT32)16,
                          TM('X'),
                          (tINT32)17,
                          (tINT32)18,
                          (tINT32)19,
                          (tINT32)20,
                          (tINT16)32000,
                          (tINT32)21,
                          (tINT32)22,
                          (tINT32)23,
                          (tINT32)24,
                          TM("String2"),
                          (tINT32)25,
                          (tINT32)26,
                          (tINT32)27,
                          (tINT32)28,
                          (tDOUBLE)123456.789,
                          (tUINT64)30ull
                         );

        ////////////////////////////////////////////////////////////////////////////
        l_pTrace->P7_DEBUG(l_hModule, TM("Format tests"), 0);

        l_pTrace->P7_INFO(l_hModule,
                          TM("Hex  %#X (%%#X = 0xDEADBEEF) | {%#*X} (%%#*X = {  0xDEADBEEF}})"),
                          (tUINT32)0xDEADBEEF,
                          12,
                          (tUINT32)0xDEADBEEF
                         );

        l_pTrace->P7_INFO(l_hModule,
                          TM("Hex  %#012X (%%#012X = 0x00DEADBEEF) | {%#-*.*X} (%%#-14.10X = {0x00DEADBEEF  }})"),
                          (tUINT32)0xDEADBEEF,
                          14,
                          10,
                          (tUINT32)0xDEADBEEF
                         );

        l_pTrace->P7_INFO(l_hModule,
                          TM("Hex  {%+10d} (%%+10d = {  +1234567}) | {%+*.*d} (%%+*.*d = { +0001234567}})"),
                          (tUINT32)1234567,
                          12,
                          10,
                          (tUINT32)1234567
                         );

    }
    if (!l_pTrace->Unregister_Thread(0))
    {
        printf("Error: Unregister_Thread() failed\n");
    }

l_lExit:
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

    return 0;
}//Test_03


////////////////////////////////////////////////////////////////////////////////
//                                   TEST 4                                   //
////////////////////////////////////////////////////////////////////////////////
// Here we send so much traces as we can                                      //
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
//Test_04
int Test_04(int i_iArgC, char* i_pArgV[])
{
    IP7_Client        *l_pClient    = NULL;
    tBOOL              l_bExit      = FALSE;
    tUINT32            l_dwTime     = GetTickCount();
    tUINT32            l_dwSent     = 0;
    tUINT32            l_dwRejected = 0;
    IP7_Trace         *l_pTrace     = NULL;
    IP7_Trace::hModule l_hModule    = NULL;
    const tXCHAR      *l_pName      = TM("Speed test");

    for (int l_iI = 0; l_iI < i_iArgC; l_iI++)
    {
        if (0 == STRNICMP(i_pArgV[l_iI], TEST_HELP, LENGTH(TEST_HELP)-1))
        {
            printf("This test create one P7 client and one P7.Trace for sending\n");
            printf("so much traces as you hardware can and print statistics\n");
            goto l_lExit;
        }
    }

    l_pClient = P7_Create_Client(TM("/P7.PSize=1472 /P7.Pool=32768"));
    if (NULL == l_pClient)
    {
        printf("Error: P7 engine was not initialized. Exit\n");
        goto l_lExit;
    }

    l_pTrace = P7_Create_Trace(l_pClient, l_pName);
    if (NULL == l_pClient)
    {
        printf("Error: P7.Trace engine was not initialized. Exit\n");
        goto l_lExit;
    }

    if (!l_pTrace->Register_Module(TM("Test #4"), &l_hModule))
    {
        printf("Error: Register_Module() failed\n");
    }


    if (!l_pTrace->Register_Thread(TM("Application"), 0))
    {
        printf("Error: Register_Thread() failed\n");
    }


    ////////////////////////////////////////////////////////////////////////////
    while (FALSE == l_bExit)
    {
        for (tUINT32 l_dwI = 0; l_dwI < 10000; l_dwI ++)
        {
            if (l_pTrace->P7_QDEBUG(0, l_hModule, TM("Strings test %d"), l_dwI))
            {
                l_dwSent ++;
            }
            else
            {
                l_dwRejected ++;
            }
        }

        if (CTicks::Difference(GetTickCount(), l_dwTime) >= 1000)
        {
            printf("Sent = %d Rejected = (%d) / %d ms\n",
                    l_dwSent,
                    l_dwRejected,
                    CTicks::Difference(GetTickCount(), l_dwTime)
                   );

            l_dwTime      = GetTickCount();
            l_dwSent      = 0;
            l_dwRejected  = 0;
            
            if (    (Is_Key_Hit())
                 && (27 == Get_Char())
               )
            {
                printf("Esc ... exiting\n");
                l_bExit = TRUE;
            }
            
        }
    }//while (FALSE == l_bExit)
    
    l_pTrace->P7_INFO(l_hModule, TM("Done !"), 0);

    if (!l_pTrace->Unregister_Thread(0))
    {
        printf("Error: Unregister_Thread() failed\n");
    }

l_lExit:
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

    return 0;
}//Test_04


////////////////////////////////////////////////////////////////////////////////
//                                   TEST 5                                   //
////////////////////////////////////////////////////////////////////////////////
// Here we test telemetry creation and destroy and simple interface tests     //
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
//Test_05

#define TEST_05_COUNTERS_COUNT                                               (5)

int Test_05(int i_iArgC, char* i_pArgV[])
{
    IP7_Client    *l_pClient    = NULL;
    IP7_Telemetry *l_pTelemetry = NULL;
    tUINT32        l_dwCount    = 1000;
    tBOOL          l_bError     = FALSE;
    tXCHAR         l_pChannel[128];
    tXCHAR         l_pCounter[128];
    tUINT16        l_pID[TEST_05_COUNTERS_COUNT];

    for (int l_iI = 0; l_iI < i_iArgC; l_iI++)
    {
        if (0 == STRNICMP(i_pArgV[l_iI], TEST_HELP, LENGTH(TEST_HELP)-1))
        {
            printf("This test create and destroy P7 and telemetry in loop\n");
            printf("Command line arguments:\n");
            printf("/Count    : Count iterations\n");
            printf("            Default value - 1000, min 1, max - uint32\n");
            printf("            Example: /Count=10000\n");
            goto l_lExit;
        }
        else if (0 == STRNICMP(i_pArgV[l_iI], TEST_01_COUNT, LENGTH(TEST_01_COUNT)-1))
        {
            sscanf(i_pArgV[l_iI] + LENGTH(TEST_01_COUNT) - 1, "%d", &l_dwCount);
            if (1 > l_dwCount)
            {
                l_dwCount = 1;
            }
        }
    }

    printf("Start: Iterations count %d\n", l_dwCount);
    
    for (tUINT32 l_dwI = 0; l_dwI < l_dwCount; l_dwI++)
    {
       printf("Iteration = %d\n", l_dwI);

        l_pClient = P7_Create_Client(NULL);

        if (NULL == l_pClient)
        {
            printf("Error: P7 engine was not initialized. Iteration = %d, Exit\n", l_dwI);
            l_bError = TRUE;
        }
        else
        {
            for (tUINT32 l_dwJ = 0; l_dwJ < 3; l_dwJ++)
            {
                SPRINTF(l_pChannel, TM("Telemetry[%03d - %02d]"), l_dwI, l_dwJ);
                l_pTelemetry = P7_Create_Telemetry(l_pClient, l_pChannel);

                if (l_pTelemetry)
                {
                    memset(l_pID, 0, sizeof(l_pID));

                    if (FALSE == l_bError)
                    {
                        for (tUINT32 l_dwK = 0; l_dwK < TEST_05_COUNTERS_COUNT; l_dwK++)
                        {
                            SPRINTF(l_pCounter, TM("Counter %02d"), l_dwK);

                            if (FALSE == l_pTelemetry->Create(l_pCounter, 
                                                              0,
                                                              0,
                                                              10000,
                                                              9000, 
                                                              1,
                                                              &l_pID[l_dwK]
                                                             )
                               )
                            {
                                printf("Error: Failed to create counter. Iteration = %d:%d:%d Exit\n",
                                       l_dwI,
                                       l_dwJ,
                                       l_dwK
                                       );
                                l_bError = TRUE;
                            }
                        }
                    }


                    if (FALSE == l_bError)
                    {
                        for (tUINT32 l_dwK = 0; l_dwK < TEST_05_COUNTERS_COUNT; l_dwK++)
                        {
                            SPRINTF(l_pCounter, TM("Counter %02d"), l_dwK);

                            if (FALSE == l_pTelemetry->Find(l_pCounter, &l_pID[l_dwK]))
                            {
                                printf("Error: Failed to find counter. Iteration = %d:%d:%d Exit\n",
                                       l_dwI,
                                       l_dwJ,
                                       l_dwK
                                       );
                                l_bError = TRUE;
                            }
                        }
                    }

                    if (FALSE == l_bError)
                    {
                        SPRINTF(l_pCounter, TM("Counter"));

                        tUINT16 l_bID = 0;
                        if (TRUE == l_pTelemetry->Find(l_pCounter, &l_bID))
                        {
                            printf("Error: Failed to find counter. Iteration = %d:%d Exit\n",
                                    l_dwI,
                                    l_dwJ
                                    );
                            l_bError = TRUE;
                        }
                    }

                    if (FALSE == l_bError)
                    {
                        for (tUINT32 l_dwJ = 0; l_dwJ < 100000; l_dwJ ++)
                        {
                            l_pTelemetry->Add(l_pID[l_dwJ % TEST_05_COUNTERS_COUNT], l_dwJ);
                        }
                    }

                    l_pTelemetry->Release();
                    l_pTelemetry = NULL;

                    CThShell::Sleep(500);
                }
                else
                {
                    printf("Error: P7_Create_Telemetry failed. Iteration = %d, Exit\n", l_dwI);
                    l_bError = TRUE;
                }
            }

            l_pClient->Release();
            l_pClient = NULL;
        }

        if (l_bError)
        {
            break;
        }
    }

l_lExit:
    if (l_pClient)
    {
        l_pClient->Release();
        l_pClient = NULL;
    }

    return 0;
}//Test_05



////////////////////////////////////////////////////////////////////////////////
//                                   TEST 6                                   //
////////////////////////////////////////////////////////////////////////////////
// Here we send so much telemetry samples as we can                           //
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
//Test_06

#define TEST_06_COUNTERS_COUNT                                               (2)

int Test_06(int i_iArgC, char* i_pArgV[])
{
    IP7_Client    *l_pClient    = NULL;
    IP7_Telemetry *l_pTelemetry = NULL;

    tBOOL          l_bError     = FALSE;
    tBOOL          l_bExit      = FALSE;
    tUINT32        l_dwSent     = 0;
    tUINT32        l_dwRejected = 0;
    tUINT32        l_dwTime     = 0;

    tXCHAR         l_pCounter[128];
    tUINT16        l_pID[TEST_06_COUNTERS_COUNT];

    UNUSED_ARG(l_bError);
    UNUSED_ARG(l_pCounter);


    for (int l_iI = 0; l_iI < i_iArgC; l_iI++)
    {
        if (0 == STRNICMP(i_pArgV[l_iI], TEST_HELP, LENGTH(TEST_HELP)-1))
        {
            printf("This test send telemetry samples with max. possible speed\n");
            goto l_lExit;
        }
    }

    l_pClient = P7_Create_Client(TM("/P7.PSize=1472 /P7.Pool=32768"));

    if (NULL == l_pClient)
    {
        printf("Error: P7 engine was not initialized");
        goto l_lExit;
    }

    l_pTelemetry = P7_Create_Telemetry(l_pClient, TM("Tel. Speed Test"));
    if (NULL == l_pTelemetry)
    {
        printf("Error: P7 Telemetry was not initialized");
        goto l_lExit;
    }


    memset(l_pID, 0, sizeof(l_pID));

    for (tUINT32 l_dwK = 0; l_dwK < TEST_06_COUNTERS_COUNT; l_dwK++)
    {
        SPRINTF(l_pCounter, TM("Counter %02d"), l_dwK);

        if (FALSE == l_pTelemetry->Create(l_pCounter,
                                          0, 
                                          0,
                                          10000,
                                          9500,
                                          1,
                                          &l_pID[l_dwK]
                                         )
           )
        {
            printf("Error: Failed to create counter %d Exit\n", l_dwK);
            goto l_lExit;
        }
    }

    ////////////////////////////////////////////////////////////////////////////

    l_dwTime = GetTickCount();

    while (FALSE == l_bExit)
    {
         for (tUINT32 l_dwI = 0; l_dwI < 10000; l_dwI ++)
         {
             if (l_pTelemetry->Add(l_pID[l_dwI % TEST_06_COUNTERS_COUNT], l_dwI))
             {
                 l_dwSent ++;
             }
             else
             {
                 l_dwRejected ++;
             }
         }

        if (CTicks::Difference(GetTickCount(), l_dwTime) >= 1000)
        {
            printf("Sent = %d Rejected = (%d) / %d ms\n",
                    l_dwSent,
                    l_dwRejected,
                    CTicks::Difference(GetTickCount(), l_dwTime)
                   );
            l_dwTime     = GetTickCount();
            l_dwSent     = 0;
            l_dwRejected = 0;
            
            if (    (Is_Key_Hit())
                 && (27 == Get_Char())
               )
            {
                printf("Esc ... exiting\n");
                l_bExit = TRUE;
            }
            
        }
    }//while (FALSE == l_bExit)


l_lExit:
    if (l_pTelemetry)
    {
        l_pTelemetry->Release();
        l_pTelemetry = NULL;
    }

    if (l_pClient)
    {
        l_pClient->Release();
        l_pClient = NULL;
    }

    return 0;
}//Test_06




////////////////////////////////////////////////////////////////////////////////
//                                   TEST 7                                   //
////////////////////////////////////////////////////////////////////////////////
// Here we check shared memory                                                //
////////////////////////////////////////////////////////////////////////////////

#define SHARED_TRACE      TM("P7.Trace.Shared")
#define SHARED_TELEMETRY  TM("P7.Trace.Telemetry")

////////////////////////////////////////////////////////////////////////////////
//Test_07
int Test_07(int i_iArgC, char* i_pArgV[])
{
    IP7_Client        *l_pClient   = NULL;
    IP7_Trace         *l_pTrace    = NULL;
    IP7_Trace::hModule l_hModule    = NULL;
    IP7_Telemetry     *l_pTelemetry = NULL;
    tBOOL              l_bExit      = FALSE;
    tUINT16            l_bID        = 0;
    tUINT32            l_dwTime     = GetTickCount();
    tUINT32            l_dwIter     = 0;
    tBOOL              l_bError     = FALSE;

    for (int l_iI = 0; l_iI < i_iArgC; l_iI++)
    {
        if (0 == STRNICMP(i_pArgV[l_iI], TEST_HELP, LENGTH(TEST_HELP)-1))
        {
            printf("This test ckeck wokring of shared memory and next functions:\n"
                   " - IP7_Trace::Share(...)\n"
                   " - P7_Get_Shared_Trace\n"
                   " - IP7_Telemetry::Share(...)\n"
                   " - P7_Get_Shared_Telemetry(...)"
                  );

            goto l_lExit;
        }
    }

    l_pClient = P7_Create_Client(TM("/P7.PSize=1472 /P7.Pool=32768"));

    if (NULL == l_pClient)
    {
        printf("Error: P7 engine was not initialized");
        goto l_lExit;
    }

    printf("Entering to working cycle, press Esc to quit\n");

    ////////////////////////////////////////////////////////////////////////////
    while (FALSE == l_bExit)
    {
        l_pTelemetry = P7_Create_Telemetry(l_pClient, TM("Telemetry"));
        if (NULL == l_pTelemetry)
        {
            printf("Error: P7 Telemetry was not initialized");
            goto l_lExit;
        }

        l_pTrace = P7_Create_Trace(l_pClient, TM("Trace"));
        if (NULL == l_pTelemetry)
        {
            printf("Error: P7 trace was not initialized");
            goto l_lExit;
        }

        if (!l_pTrace->Register_Thread(TM("Application"), 0))
        {
            printf("Error: Register_Thread() failed\n");
        }

        if (FALSE == l_pTelemetry->Create(TM("Counter"), 0, 0, 1000, 950, 1, &l_bID))
        {
            printf("Error: Failed to create counter Exit\n");
            goto l_lExit;
        }

        if (FALSE == l_pTelemetry->Share(SHARED_TELEMETRY))
        {
            printf("Error: Failed to share Telemetry instance, exit\n");
            goto l_lExit;
        }

        if (FALSE == l_pTrace->Share(SHARED_TRACE))
        {
            printf("Error: Failed to share Trace instance, exit\n");
            goto l_lExit;
        }

        for (tUINT32 l_dwI = 0; l_dwI < 10; l_dwI++)
        {
            IP7_Trace     *l_pSTtrace    = P7_Get_Shared_Trace(SHARED_TRACE);
            IP7_Telemetry *l_pSTelemetry = P7_Get_Shared_Telemetry(SHARED_TELEMETRY);

            if (    (l_pSTtrace)
                 && (l_pSTelemetry)
               )
            {
                if (!l_pSTtrace->Register_Module(TM("Test #7"), &l_hModule))
                {
                    printf("Error: Register_Module() failed\n");
                }

                for (tUINT32 l_dwJ = 0; l_dwJ < 1000; l_dwJ++)
                {
                    l_pSTtrace->P7_TRACE(l_hModule, TM("Test:%d"), l_dwJ);
                    l_pSTelemetry->Add(l_bID, (tDOUBLE)l_dwJ);
                }
            }
            else
            {
                l_bError = TRUE;
            }

            if (l_pSTtrace)
            {
                l_pSTtrace->Release();
                l_pSTtrace = NULL;
            }

            if (l_pSTelemetry)
            {
                l_pSTelemetry->Release();
                l_pSTelemetry = NULL;
            }

            if (l_bError)
            {
                break;
            }

            CThShell::Sleep(1);
        }//for (tUINT32 l_dwI = 0; l_dwI < 10; l_dwI++)


        if (CTicks::Difference(GetTickCount(), l_dwTime) >= 1000)
        {
            l_dwTime     = GetTickCount();

            if (    (Is_Key_Hit())
                 && (27 == Get_Char())
               )
            {
                printf("Esc ... exiting\n");
                l_bExit = TRUE;
            }

        }

        if (l_pTelemetry)
        {
            l_pTelemetry->Release();
            l_pTelemetry = NULL;
        }


        if (!l_pTrace->Unregister_Thread(0))
        {
            printf("Error: Unregister_Thread() failed\n");
        }

        if (l_pTrace)
        {
            l_pTrace->Release();
            l_pTrace = NULL;
        }

        if (l_bError)
        {
            break;
        }

        printf("Iteration = %d\n", l_dwIter++);
    }//while (FALSE == l_bExit)

l_lExit:
    if (l_pTelemetry)
    {
        l_pTelemetry->Release();
        l_pTelemetry = NULL;
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

    return 0;
}//Test_07


////////////////////////////////////////////////////////////////////////////////
//                                   TEST 8                                   //
////////////////////////////////////////////////////////////////////////////////
// Here we create so much telemetry counters as we can                        //
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
//Test_08

#define TEST_08_COUNTERS_COUNT                                           (65534)
#define TEST_08_COUNTERS_ACTIVE_COUNT                   256 //TEST_08_COUNTERS_COUNT/64

int Test_08(int i_iArgC, char* i_pArgV[])
{
    IP7_Client    *l_pClient    = NULL;
    IP7_Telemetry *l_pTelemetry = NULL;

    tBOOL          l_bError     = FALSE;
    tBOOL          l_bExit      = FALSE;
    tUINT32        l_dwSent     = 0;
    tUINT32        l_dwRejected = 0;
    tUINT32        l_dwTime     = 0;
    tDOUBLE        l_dbVal      = 0;

    tXCHAR         l_pCounter[4096];
    const tXCHAR  *l_pExtraText[] = {TM("You can do anything, but not everything"),
                                     TM("Perfection is achieved, not when there is nothing more to add, but when there is nothing left to take away"), 
                                     TM("The richest man is not he who has the most, but he who needs the least"), 
                                     TM("Courage is not the absence of fear, but rather the judgement that something else is more important than fear."), 
                                     TM("When hungry, eat your rice; when tired, close your eyes. Fools may laugh at me, but wise men will know what I mean.")};
    tUINT16        l_pID[TEST_08_COUNTERS_COUNT];

    UNUSED_ARG(l_bError);
    UNUSED_ARG(l_pCounter);


    for (int l_iI = 0; l_iI < i_iArgC; l_iI++)
    {
        if (0 == STRNICMP(i_pArgV[l_iI], TEST_HELP, LENGTH(TEST_HELP)-1))
        {
            printf("This test send telemetry samples with max. possible speed\n");
            goto l_lExit;
        }
    }

    l_pClient = P7_Create_Client(TM("/P7.PSize=1472 /P7.Pool=32768"));

    if (NULL == l_pClient)
    {
        printf("Error: P7 engine was not initialized");
        goto l_lExit;
    }

    l_pTelemetry = P7_Create_Telemetry(l_pClient, TM("Tel. Speed Test"));
    if (NULL == l_pTelemetry)
    {
        printf("Error: P7 Telemetry was not initialized");
        goto l_lExit;
    }


    memset(l_pID, 0, sizeof(l_pID));

    for (tUINT32 l_dwK = 0; l_dwK < TEST_08_COUNTERS_COUNT; l_dwK++)
    {
        SPRINTF(l_pCounter, TM("Test counter group #%04d/Counter Index #%04d {%s}"), 
                l_dwK / 256,
                l_dwK,
                l_pExtraText[l_dwK % LENGTH(l_pExtraText)]
               );

        if (FALSE == l_pTelemetry->Create(l_pCounter,
                                          0.0, 
                                          0.0,
                                          10000.0,
                                          9500.0,
                                          (l_dwK >= (TEST_08_COUNTERS_COUNT - TEST_08_COUNTERS_ACTIVE_COUNT)) ? TRUE : FALSE,
                                          &l_pID[l_dwK]
                                         )
           )
        {
            printf("Error: Failed to create counter %d Exit\n", l_dwK);
            goto l_lExit;
        }
    }

    ////////////////////////////////////////////////////////////////////////////

    l_dwTime = GetTickCount();

    while (FALSE == l_bExit)
    {
        for (tUINT32 l_dwI = 0; l_dwI < TEST_08_COUNTERS_ACTIVE_COUNT; l_dwI ++)
        {
            l_dbVal += 0.5;
            if (l_pTelemetry->Add(l_pID[TEST_08_COUNTERS_COUNT - l_dwI - 1], l_dbVal))
            {
                l_dwSent ++;
            }
            else
            {
                l_dwRejected ++;
            }
        }

        if (l_dbVal > 10000.0)
        {
            l_dbVal = 0.0;
            CThShell::Sleep(1);
        }

        if (CTicks::Difference(GetTickCount(), l_dwTime) >= 1000)
        {
            printf("Sent = %d Rejected = (%d) / %d ms\n",
                    l_dwSent,
                    l_dwRejected,
                    CTicks::Difference(GetTickCount(), l_dwTime)
                   );
            l_dwTime     = GetTickCount();
            l_dwSent     = 0;
            l_dwRejected = 0;
            
            if (    (Is_Key_Hit())
                 && (27 == Get_Char())
               )
            {
                printf("Esc ... exiting\n");
                l_bExit = TRUE;
            }
            
        }
    }//while (FALSE == l_bExit)


l_lExit:
    if (l_pTelemetry)
    {
        l_pTelemetry->Release();
        l_pTelemetry = NULL;
    }

    if (l_pClient)
    {
        l_pClient->Release();
        l_pClient = NULL;
    }

    return 0;
}//Test_08


////////////////////////////////////////////////////////////////////////////////
//                                   MAIN                                     //
////////////////////////////////////////////////////////////////////////////////
int main(int i_iArgC, char* i_pArgV[])
{
    printf("******************************************************************\n");
    printf("P7 Traces test tool\n");
    printf("******************************************************************\n");

    int l_iTest    = -1;
    int l_iReturn  = 0;

    for (int l_iI = 0; l_iI < i_iArgC; l_iI++)
    {
        if (0 == STRNICMP(i_pArgV[l_iI], TEST_NUMBER, LENGTH(TEST_NUMBER)-1))
        {
            sscanf(i_pArgV[l_iI] + LENGTH(TEST_NUMBER)-1, "%d", &l_iTest);
        }
    }

    if (1 == l_iTest)
    {
        l_iReturn = Test_01(i_iArgC, i_pArgV);
    }
    else if (2 == l_iTest)
    {
        l_iReturn = Test_02(i_iArgC, i_pArgV);
    }
    else if (3 == l_iTest)
    {
        l_iReturn = Test_03(i_iArgC, i_pArgV);
    }
    else if (4 == l_iTest)
    {
        l_iReturn = Test_04(i_iArgC, i_pArgV);
    }
    else if (5 == l_iTest)
    {
        l_iReturn = Test_05(i_iArgC, i_pArgV);
    }
    else if (6 == l_iTest)
    {
        l_iReturn = Test_06(i_iArgC, i_pArgV);
    }
    else if (7 == l_iTest)
    {
        l_iReturn = Test_07(i_iArgC, i_pArgV);
    }
    else if (8 == l_iTest)
    {
        l_iReturn = Test_08(i_iArgC, i_pArgV);
    }
    else
    {
        printf("ERROR: Test number is not specified or out of range\n");
        printf("Test command line arguments:\n");
        printf("   /Test : test number to execute. Min value = 1, max value = 6.\n");
        printf("           Example: /Test=1\n");
        printf("           To get help about test: /Test=1 /?\n");
        printf("   /?    : show help for specific test.\n");
        printf("           Example: /Test=1 /?\n");
        PRINTF(CLIENT_HELP_STRING);
    }

    
    
    DUMP_MEMORY_LEAKS();

    return l_iReturn;
}


//"${OUTPUT_PATH}" /P7.Addr=192.168.0.3 /P7.Port=9009 /P7.Verb=1 /P7.Window=128 /P7.PSize=1474 /Test=3 /Speed=10 /Count=3 /P7.Pool=4096
