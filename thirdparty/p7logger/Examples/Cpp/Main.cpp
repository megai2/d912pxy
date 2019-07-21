#include <stdlib.h>
#include <stdio.h>

#include "P7_Trace.h"
#include "P7_Telemetry.h"

void __cdecl Telemetry_Enable(void *i_pContext, tUINT16 i_wId, tBOOL i_bEnable)
{
    printf("Id=%d, Enable=%d\n", (tUINT32)i_wId, (tUINT32)i_bEnable);
}

void __cdecl Connect(void *i_pContext, tBOOL i_bConnect)
{
    printf("Connect=%d\n", (tUINT32)i_bConnect);
}


int main(int i_iArgC, char* i_pArgV[])
{
    IP7_Client        *l_pClient    = NULL;

    IP7_Trace         *l_pTrace     = NULL;
    IP7_Trace::hModule l_hModule    = NULL;

    IP7_Telemetry     *l_pTelemetry = NULL;
    tUINT16            l_bTID1      = 0;
    tUINT16            l_bTID2      = 0;
    tUINT32            l_dwIdx      = 0;
    stTelemetry_Conf   l_stConf     = {};

    P7_Set_Crash_Handler();

    //create P7 client object
    l_pClient = P7_Create_Client(TM("/P7.Pool=32768"));

    if (NULL == l_pClient)
    {
        goto l_lblExit;
    }

    l_stConf.pContext              = NULL;
    l_stConf.pEnable_Callback      = &Telemetry_Enable;
    l_stConf.pTimestamp_Callback   = NULL;
    l_stConf.qwTimestamp_Frequency = 0ull;
    l_stConf.pConnect_Callback     = &Connect;

    //create P7 trace object 1
    l_pTrace = P7_Create_Trace(l_pClient, TM("Trace channel 1"));
    if (NULL == l_pTrace)
    {
        goto l_lblExit;
    }

    l_pTrace->Register_Thread(TM("Application"), 0);
    l_pTrace->Register_Module(TM("Main"), &l_hModule);

    //send few trace messages
    for (tUINT64 l_qwI = 0ULL; l_qwI < 100000ULL; l_qwI ++)
    {
        l_pTrace->P7_TRACE(l_hModule, TM("Test trace message #%d\n, {%I64d}"), l_dwIdx ++, l_qwI);
        l_pTrace->P7_INFO(l_hModule, TM("Test info message #%d, {%I64d}\n"), l_dwIdx ++, l_qwI);
        l_pTrace->P7_DEBUG(l_hModule, TM("Test debug\n message #%d, {%I64d}"), l_dwIdx ++, l_qwI);

        if (0xF == (rand() & 0xF))
        {
            l_pTrace->P7_WARNING(l_hModule, TM("Test warning message #%d, {%I64d}"), l_dwIdx ++, l_qwI);
        }

        if (0xFF == (rand() & 0xFF))
        {
            l_pTrace->P7_ERROR(l_hModule, TM("Test error message #%d, {%I64d}"), l_dwIdx ++, l_qwI);
        }

        if (0xFF0 == (rand() & 0xFFF))
        {
            l_pTrace->P7_CRITICAL(l_hModule, TM("Test critical message #%d, {%I64d}"), l_dwIdx ++, l_qwI);
        }
    }

    l_pTrace->Unregister_Thread(0);

    //create P7 telemetry object
    l_pTelemetry = P7_Create_Telemetry(l_pClient, TM("Telemetry channel 1"), &l_stConf);
    if (NULL == l_pTelemetry)
    {
        goto l_lblExit;
    }
    
    //register telemetry counter, it has values in range 0 ... 1023
    if (FALSE == l_pTelemetry->Create(TM("Group/counter 1"), 0, 0, 1023, 1000, 1, &l_bTID1))
    {
        goto l_lblExit;
    }
    if (FALSE == l_pTelemetry->Create(TM("Group/counter 2"), 0, 0, 1023, 1000, 1, &l_bTID2))
    {
        goto l_lblExit;
    }
    
    for (tUINT64 l_qwI = 0ULL; l_qwI < 100000ULL; l_qwI ++)
    {
        l_pTelemetry->Add(l_bTID1, (tDOUBLE)(l_qwI & 0x3FFull));
        l_pTelemetry->Add(l_bTID2, (tDOUBLE)((l_qwI + 11ull) & 0x3FFull));
    }


l_lblExit:
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
}

