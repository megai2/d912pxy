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
//http://www.codeproject.com/Articles/207464/Exception-Handling-in-Visual-Cplusplus

#ifndef PSIGNAL_H
#define PSIGNAL_H

#include <new.h>
#include <crtdbg.h>
#include "signal.h"
#include "ISignal.h"


static stChContext g_stContext = {0};

////////////////////////////////////////////////////////////////////////////////
//ChSignalHandler
static void __cdecl ChSignalHandler(int i_iSignal)
{
    if (g_stContext.iProcessed)
    {
        return;
    }

    g_stContext.iProcessed ++;

    const char *l_pText = "unknown signal";

    if (SIGABRT == i_iSignal)
    {
        l_pText = "SIGABRT";
    }
    else if (SIGFPE == i_iSignal)
    {
        l_pText = "SIGFPE";
    }
    else if (SIGILL == i_iSignal)
    {
        l_pText = "SIGILL";
    }
    else if (SIGINT == i_iSignal)
    {
        l_pText = "SIGINT";
    }
    else if (SIGSEGV == i_iSignal)
    {
        l_pText = "SIGSEGV";
    }
    else if (SIGTERM == i_iSignal)
    {
        l_pText = "SIGTERM";
    }


    if (g_stContext.pUserHandler)
    {
        g_stContext.pUserHandler((eCrashCode)(eCrashSignal + i_iSignal), l_pText, g_stContext.pUserContext);
    }

    exit(1);
}//ChSignalHandler


////////////////////////////////////////////////////////////////////////////////
//ChUnhandledExceptionFilter
static LONG WINAPI ChUnhandledExceptionFilter(__in struct _EXCEPTION_POINTERS *i_pException)
{
    if (g_stContext.iProcessed)
    {
        return EXCEPTION_CONTINUE_SEARCH;
    }

    if (    (STATUS_ACCESS_VIOLATION            == i_pException->ExceptionRecord->ExceptionCode)
         || (EXCEPTION_ARRAY_BOUNDS_EXCEEDED    == i_pException->ExceptionRecord->ExceptionCode)
         || (EXCEPTION_DATATYPE_MISALIGNMENT    == i_pException->ExceptionRecord->ExceptionCode)
         || (EXCEPTION_FLT_DIVIDE_BY_ZERO       == i_pException->ExceptionRecord->ExceptionCode)
         || (EXCEPTION_FLT_STACK_CHECK          == i_pException->ExceptionRecord->ExceptionCode)
         || (EXCEPTION_ILLEGAL_INSTRUCTION      == i_pException->ExceptionRecord->ExceptionCode)
         || (EXCEPTION_INT_DIVIDE_BY_ZERO       == i_pException->ExceptionRecord->ExceptionCode)
         || (EXCEPTION_NONCONTINUABLE_EXCEPTION == i_pException->ExceptionRecord->ExceptionCode)
         || (EXCEPTION_PRIV_INSTRUCTION         == i_pException->ExceptionRecord->ExceptionCode)
         || (EXCEPTION_STACK_OVERFLOW           == i_pException->ExceptionRecord->ExceptionCode)
         //|| ( == i_pException->ExceptionRecord.ExceptionCode)
      )
    {
        g_stContext.iProcessed ++;

        if (g_stContext.pUserHandler)
        {
            g_stContext.pUserHandler(eCrashException, i_pException, g_stContext.pUserContext);
        }

        exit(1);
    }

    return EXCEPTION_CONTINUE_SEARCH;
}//ChUnhandledExceptionFilter


////////////////////////////////////////////////////////////////////////////////
//ChPurecallHandler
static void ChPurecallHandler(void)
{
    if (g_stContext.iProcessed)
    {
        return;
    }

    g_stContext.iProcessed ++;

    if (g_stContext.pUserHandler)
    {
        g_stContext.pUserHandler(eCrashPureCall, "Pure call", g_stContext.pUserContext);
    }

    exit(1);
}//ChPurecallHandler


////////////////////////////////////////////////////////////////////////////////
//ChMemoryAllocationHandler
static int ChMemoryAllocationHandler(size_t i_szSize)
{
    if (g_stContext.iProcessed)
    {
        return 0;
    }

    g_stContext.iProcessed ++;

    char l_pText[32];

    sprintf_s(l_pText, "%lld", (tINT64)i_szSize);

    if (g_stContext.pUserHandler)
    {
        g_stContext.pUserHandler(eCrashMemAlloc, l_pText, g_stContext.pUserContext);
    }

    exit(1);

    return 0;
}//ChMemoryAllocationHandler


////////////////////////////////////////////////////////////////////////////////
//ChInvalidParameterHandler
static void ChInvalidParameterHandler(const wchar_t* i_pExpression,
                                      const wchar_t* i_pFunction, 
                                      const wchar_t* i_pFile, 
                                      unsigned int   i_dwLine, 
                                      uintptr_t      i_pReserved
                                     )
{
    if (g_stContext.iProcessed)
    {
        return;
    }

    g_stContext.iProcessed ++;

    char           l_pText[148];
    const wchar_t *l_pFunction = i_pFunction ? i_pFunction : L"Func:Unk";
    const wchar_t *l_pFile     = i_pFile ? i_pFile : L"File:Unk";
    size_t         l_szFile    = i_pFile ? wcslen(i_pFile) : 0;
    size_t         l_szFunc    = i_pFunction ? wcslen(i_pFunction) : 0;

    if ( l_szFile > 64 )
    {
        l_szFile -= 64;
    }

    if ( l_szFunc > 64 )
    {
        l_szFunc -= 64;
    }

    sprintf_s(l_pText, "{%S}{%S}{%d}", l_pFunction + l_szFunc, l_pFile + l_szFile, (int)i_dwLine);

    if (g_stContext.pUserHandler)
    {
        g_stContext.pUserHandler(eCrashInvalidParameter, l_pText, g_stContext.pUserContext);
    }

    exit(1);
}//ChInvalidParameterHandler


////////////////////////////////////////////////////////////////////////////////
//ChInstallPrivate
static tBOOL ChInstallPrivate()
{
    ////////////////////////////////////////////////////////////////////////////
    //initialize handlers for all possible cases
    signal(SIGABRT, ChSignalHandler);
    signal(SIGFPE,  ChSignalHandler);
    signal(SIGILL,  ChSignalHandler);
    signal(SIGINT,  ChSignalHandler);
    signal(SIGSEGV, ChSignalHandler);
    signal(SIGTERM, ChSignalHandler);

    //SetUnhandledExceptionFilter(ChUnhandledExceptionFilter);
    AddVectoredExceptionHandler(1, ChUnhandledExceptionFilter);

    _set_purecall_handler(ChPurecallHandler);

    _set_new_mode(1);
    _set_new_handler(ChMemoryAllocationHandler);

    _set_invalid_parameter_handler(ChInvalidParameterHandler);
    
    _CrtSetReportMode(_CRT_ERROR, 0);   
    _CrtSetReportMode(_CRT_ASSERT, 0);

    return TRUE;
}//ChInstallPrivate


////////////////////////////////////////////////////////////////////////////////
//ChInstall
static tBOOL ChInstall()
{
    if (g_stContext.iInstalled)
    {
        return FALSE;
    }


    ////////////////////////////////////////////////////////////////////////////
    //initialize parameters
    memset(&g_stContext, 0, sizeof(g_stContext));
    g_stContext.iInstalled   = 1;
    g_stContext.iProcessed   = 0;

    ChInstallPrivate();

    return TRUE;
}//ChInstall


////////////////////////////////////////////////////////////////////////////////
//ChSetHandler
static tBOOL ChSetHandler(fnCrashHandler i_fnHandler)
{
    if (!g_stContext.iInstalled)
    {
        return FALSE;
    }

    g_stContext.pUserHandler = i_fnHandler;

    return TRUE;
}//ChSetHandler


////////////////////////////////////////////////////////////////////////////////
//ChSetContext
static tBOOL ChSetContext(void *i_pContext)
{
    if (!g_stContext.iInstalled)
    {
        return FALSE;
    }

    g_stContext.pUserContext = i_pContext;

    return TRUE;
}//ChSetContext


////////////////////////////////////////////////////////////////////////////////
//ChUnInstall
static tBOOL ChUnInstall()
{
    return TRUE;
}//ChUnInstall

#endif //PSIGNAL_H