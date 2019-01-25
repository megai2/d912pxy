#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>


#include "GTypes.h"

#include "Length.h"
#include "PAtomic.h"
#include "PLock.h"
#include "PString.h"
#include "PTime.h"
#include "IMEvent.h"
#include "PMEvent.h"
#include "PThreadShell.h"
#include "Ticks.h"
#include "PConsole.h"

#include "P7_Client.h"
#include "P7_Trace.h"
#include "P7_Telemetry.h"


struct sThread
{
    IP7_Trace        *pTrace;
    IMEvent          *iEvent;
    CThShell::tTHREAD hThread;
    tBOOL             bThread;
    volatile LONG     lCount;
};


#if defined(_MSC_VER) 
    #pragma warning(disable : 4267)
    #pragma warning(disable : 4995)
    #pragma warning(disable : 4996)
    //allow to see dump of memory leaks at the end of test in VS debug output
    #define _CRTDBG_MAP_ALLOC
    #include <crtdbg.h>

    #define DUMP_MEMORY_LEAKS()  _CrtDumpMemoryLeaks()
    #define VSPRINTF             vswprintf

#elif defined (__GNUC__)

    #define DUMP_MEMORY_LEAKS()  printf("memory leaks detection is missing !")
    #define VSPRINTF             vsprintf

#endif


////////////////////////////////////////////////////////////////////////////////
//C2Buffer
class C2Buffer
    : public IP7_Trace
{
private:
    tXCHAR   m_pBuffer[1024];
public:
    C2Buffer() {}
    ~C2Buffer() {}

    tINT32 Add_Ref() { return 0;}

    tINT32 Release() {return 0;}
    void Set_Verbosity(eP7Trace_Level i_eVerbosity) {}
    tBOOL Share(const tXCHAR *i_pName) {return FALSE;}
    tBOOL Trace(tUINT16         i_wTrace_ID,   
                eP7Trace_Level  i_eLevel, 
                tUINT16         i_wModule_ID,
                tUINT16         i_wLine,
                const char     *i_pFile,
                const char     *i_pFunction,
                const tXCHAR   *i_pFormat,
                ...
               )
    {
        va_list l_pArgList;
        va_start(l_pArgList, i_pFormat);
        VSPRINTF(m_pBuffer, i_pFormat, l_pArgList);
        va_end(l_pArgList);

        return TRUE;
    }

    tBOOL Trace_Embedded(tUINT16       i_wTrace_ID,   
                         eP7Trace_Level i_eLevel, 
                         tUINT16        i_wModule_ID,
                         tUINT16        i_wLine,
                         const char    *i_pFile,
                         const char    *i_pFunction,
                         const tXCHAR **i_ppFormat
                       )
    {
        return FALSE;
    }
};//C2Buffer


////////////////////////////////////////////////////////////////////////////////
//C2Console
class C2Console
    : public IP7_Trace
{
private:
    tXCHAR   m_pBuffer[1024];
public:
    C2Console() {}
    ~C2Console() {}

    tINT32 Add_Ref() { return 0;}

    tINT32 Release() {return 0;}
    void Set_Verbosity(eP7Trace_Level i_eVerbosity) {}
    tBOOL Share(const tXCHAR *i_pName) {return FALSE;}
    tBOOL Trace(tUINT16         i_wTrace_ID,   
                eP7Trace_Level  i_eLevel, 
                tUINT16         i_wModule_ID,
                tUINT16         i_wLine,
                const char     *i_pFile,
                const char     *i_pFunction,
                const tXCHAR   *i_pFormat,
                ...
               )
    {
        va_list l_pArgList;
        va_start(l_pArgList, i_pFormat);
        VSPRINTF(m_pBuffer, i_pFormat, l_pArgList);
        va_end(l_pArgList);

        PRINTF(m_pBuffer);

        return TRUE;
    }

    tBOOL Trace_Embedded(tUINT16       i_wTrace_ID,   
                         eP7Trace_Level i_eLevel, 
                         tUINT16        i_wModule_ID,
                         tUINT16        i_wLine,
                         const char    *i_pFile,
                         const char    *i_pFunction,
                         const tXCHAR **i_ppFormat
                       )
    {
        return FALSE;
    }
};//C2Console


////////////////////////////////////////////////////////////////////////////////
//Routine
static THSHELL_RET_TYPE THSHELL_CALL_TYPE Routine(void *i_pContext)
{
    sThread *l_pParam = (sThread*)i_pContext;
    tUINT32  l_dwCount = 0;

    if (NULL == l_pParam)
    {
        printf("Error: thread input parameter is NULL\n");
        return 0;
    }

    while (MEVENT_TIME_OUT == l_pParam->iEvent->Wait(0))
    {
        l_dwCount = 0;
        for (tUINT32 l_dwI = 0; l_dwI < 250; l_dwI++)
        {
            if (l_pParam->pTrace->P7_QTRACE(1, 
                                            0,
                                            TM("Trace test [%d] %d, %d"), 
                                            l_dwI, 
                                            10, 
                                            20
                                           )
               )
            {
                l_dwCount ++;
            }
        }//for (tUINT32 l_dwI = 0; l_dwI < 10 * TEST_SPEED; l_dwI++)

        ATOMIC_ADD(&(l_pParam->lCount), l_dwCount);

    }//while (WAIT_TIMEOUT == WaitForSingleObject(g_pEvent_Exit, 10))

    CThShell::Cleanup();

    return THSHELL_RET_OK;
}//Routine

#define T2B (0)
#define T2C (1)
#define T2T (2)

int main(int i_iArgC, char* i_pArgV[])
{
    C2Console       l_c2Console;
    C2Buffer        l_c2Buffer;
    sThread         l_pThreads[3];
    tUINT8          l_pID[3];

    IP7_Client     *l_pClient    = P7_Create_Client(TM("/P7.Addr=::1 /P7.Pool=32768"));
    IP7_Trace      *l_pTrace     = P7_Create_Trace(l_pClient, TM("Trace channel"));
    IP7_Telemetry  *l_pTelemetry = P7_Create_Telemetry(l_pClient, TM("Telemetry"));

    tBOOL           l_bExit      = FALSE;
    tUINT32         l_dwStart    = GetTickCount();
    CMEvent        *l_pEvent     = NULL; 
    tUINT32         l_dwIter     = 0;


    printf("This runs in parallel 3 threads:\n");
    printf(" 1. printing to buffer\n");
    printf(" 2. printing to console\n");
    printf(" 3. printing to trace channel\n");
    printf("Every thread will calculate amount of prints and will deliver\n");
    printf("such information to telemetry channel\n");
    printf("Start, press Esc to exit ...\n");

    memset(l_pThreads, 0, sizeof(l_pThreads));

    l_pEvent = new CMEvent(); 

    if (    (NULL  == l_pEvent)
         || (FALSE == l_pEvent->Init(1, EMEVENT_SINGLE_MANUAL))
         || (NULL  == l_pClient)
         || (NULL  == l_pTrace)
         || (NULL  == l_pTelemetry)
       )
    {
        printf("Error: initialization failed\n");
        goto l_lExit;
    }

    if (    (FALSE == l_pTelemetry->Create(TM("To Buffer"), 0, 2000000, 2000000, TRUE, &l_pID[T2B]))
         || (FALSE == l_pTelemetry->Create(TM("To Console"), 0, 10000000, 2000000, TRUE, &l_pID[T2C]))
         || (FALSE == l_pTelemetry->Create(TM("To Trace"), 0, 2000000, 2000000, TRUE, &l_pID[T2T]))
       )
    {
        printf("Can't create counters\n");
        goto l_lExit;
    }

    l_pThreads[T2B].iEvent = static_cast<IMEvent*>(l_pEvent);
    l_pThreads[T2B].pTrace = static_cast<IP7_Trace*>(&l_c2Buffer);

    l_pThreads[T2C].iEvent = static_cast<IMEvent*>(l_pEvent);
    l_pThreads[T2C].pTrace = static_cast<IP7_Trace*>(&l_c2Console);

    l_pThreads[T2T].iEvent = static_cast<IMEvent*>(l_pEvent);
    l_pThreads[T2T].pTrace = l_pTrace;

    for (tUINT32 l_dwI = 0; l_dwI < LENGTH(l_pThreads); l_dwI++)
    {
        l_pThreads[l_dwI].bThread = TRUE;

        if (FALSE == CThShell::Create(&Routine, 
                                      &l_pThreads[l_dwI], 
                                      &l_pThreads[l_dwI].hThread,
                                      TM("Speed test")           
                                     )
           )
        {
            l_pThreads[l_dwI].bThread = FALSE;
            l_bExit = TRUE;
            printf("Failed to create thread\n");
        }
    }


    while (    (FALSE == l_bExit)
            && (300 > l_dwIter)
          )
    {
        CThShell::Sleep(2);
        l_dwIter ++;
        l_pTelemetry->Add(l_pID[T2B], (tINT64)l_pThreads[T2B].lCount);
        l_pTelemetry->Add(l_pID[T2C], (tINT64)l_pThreads[T2C].lCount);
        l_pTelemetry->Add(l_pID[T2T], (tINT64)l_pThreads[T2T].lCount);

        // if (    (Is_Key_Hit())
        //         && (27 == Get_Char())
        //     )
        // {
        //     printf("Esc ... exiting\n");
        //     l_bExit = TRUE;
        // }
        // else
        // {
        //     l_pTelemetry->Add(l_pID[T2B], (tINT64)l_pThreads[T2B].lCount);
        //     l_pTelemetry->Add(l_pID[T2C], (tINT64)l_pThreads[T2C].lCount);
        //     l_pTelemetry->Add(l_pID[T2T], (tINT64)l_pThreads[T2T].lCount);
        // }
    }

    l_pEvent->Set(0);
l_lExit:
    for (tUINT32 l_dwI = 0; l_dwI < LENGTH(l_pThreads); l_dwI++)
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

    if (l_pTrace)
    {
        l_pTrace->Release();
        l_pTrace = NULL;
    }

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


//     printf("\n*****************************************************************\n");
//     printf("Test results:\n");
//     printf("  Test 1 duration: %d ms\n", (tUINT32)l_dwTime_1);
//     printf("  Test 2 duration: %d ms\n", (tUINT32)l_dwTime_2);
//     printf("  Test 3 duration: %d ms\n", (tUINT32)l_dwTime_3);

    DUMP_MEMORY_LEAKS();

    return 0;
}

