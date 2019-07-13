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
#ifndef PSIGNAL_H
#define PSIGNAL_H

#include <stdio.h>
#include <stdlib.h>
#include "signal.h"
#include <time.h>
#include <errno.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <features.h>
#include "ISignal.h"

//Linux signal handling article:
//http://www.linuxprogrammingblog.com/all-about-linux-signals?page=show

static stChContext g_stContext = {0};


////////////////////////////////////////////////////////////////////////////////
//Get_Signal_Text
static __attribute__ ((unused)) const char *ChGetSignalText(int i_iSignal, siginfo_t* i_pSigInfo)
{
    if (i_iSignal == SIGBUS)
    {
        if (i_pSigInfo->si_code == BUS_ADRALN)      { return "SIGBUS::BUS_ADRALN"; }
        else if (i_pSigInfo->si_code == BUS_ADRERR) { return "SIGBUS::BUS_ADRERR"; }
        else if (i_pSigInfo->si_code == BUS_OBJERR) { return "SIGBUS::BUS_OBJERR"; }
        else                                        { return "SIGBUS::<unknown>";  }
    }
    else if (i_iSignal == SIGFPE)
    {
        if (i_pSigInfo->si_code == FPE_FLTDIV)      { return "SIGFPE::FPE_FLTDIV"; }
        else if (i_pSigInfo->si_code == FPE_FLTINV) { return "SIGFPE::FPE_FLTINV"; }
        else if (i_pSigInfo->si_code == FPE_FLTOVF) { return "SIGFPE::FPE_FLTOVF"; }
        else if (i_pSigInfo->si_code == FPE_FLTRES) { return "SIGFPE::FPE_FLTRES"; }
        else if (i_pSigInfo->si_code == FPE_FLTSUB) { return "SIGFPE::FPE_FLTSUB"; }
        else if (i_pSigInfo->si_code == FPE_FLTUND) { return "SIGFPE::FPE_FLTUND"; }
        else if (i_pSigInfo->si_code == FPE_INTDIV) { return "SIGFPE::FPE_INTDIV"; }
        else if (i_pSigInfo->si_code == FPE_INTOVF) { return "SIGFPE::FPE_INTOVF"; }
        else                                        { return "SIGFPE::<unknown>";  }
    }
    else if (i_iSignal == SIGILL)
    {
        if (i_pSigInfo->si_code == ILL_BADSTK)      { return "SIGILL::ILL_BADSTK"; }
        else if (i_pSigInfo->si_code == ILL_COPROC) { return "SIGILL::ILL_COPROC"; }
        else if (i_pSigInfo->si_code == ILL_ILLOPN) { return "SIGILL::ILL_ILLOPN"; }
        else if (i_pSigInfo->si_code == ILL_ILLADR) { return "SIGILL::ILL_ILLADR"; }
        else if (i_pSigInfo->si_code == ILL_ILLTRP) { return "SIGILL::ILL_ILLTRP"; }
        else if (i_pSigInfo->si_code == ILL_PRVOPC) { return "SIGILL::ILL_PRVOPC"; }
        else if (i_pSigInfo->si_code == ILL_PRVREG) { return "SIGILL::ILL_PRVREG"; }
        else                                        { return "SIGILL::<unknown>";  }
    }
    else if (i_iSignal == SIGSEGV)
    {
        if (i_pSigInfo->si_code == SEGV_MAPERR)     { return "SIGSEGV::SEGV_MAPERR"; }
        else if (i_pSigInfo->si_code == SEGV_ACCERR){ return "SIGSEGV::SEGV_ACCERR"; }
        else                                        { return "SIGSEGV::<unknown>";   }
    }
    
    return "<unknown>::<unknown>"; 
}//Get_Signal_Text


////////////////////////////////////////////////////////////////////////////////
//ChHandler
static __attribute__ ((unused)) void ChHandler(int        i_iSignal, 
                                               siginfo_t *i_pSigInfo, 
                                               void      *i_pContext
                                              )
{
    UNUSED_ARG(i_pContext);
    if (g_stContext.iProcessed)
    {
        return;
    }

    g_stContext.iProcessed = 1;

    const char *l_pText = ChGetSignalText(i_iSignal, i_pSigInfo);

    if (g_stContext.pUserHandler)
    {
        g_stContext.pUserHandler((eCrashCode)(eCrashSignal + i_iSignal), l_pText, g_stContext.pUserContext);
    }

    printf("Process has been terminated by signal {%s}\n", l_pText);
}//ChHandler


////////////////////////////////////////////////////////////////////////////////
//ChInstall
static __attribute__ ((unused)) tBOOL ChInstallPrivate()
{
    struct sigaction l_sSignal;
    struct sigaction l_sSignalPipe;
    tBOOL            l_bReturn  = TRUE;

    ////////////////////////////////////////////////////////////////////////////
    // installing signal handling ...
    memset(&l_sSignalPipe, 0, sizeof(l_sSignalPipe));
    memset(&l_sSignal, 0, sizeof(l_sSignal));

    //remove SIGPIPE signal, should be ignored
    l_sSignalPipe.sa_handler = SIG_IGN;
    sigemptyset(&l_sSignalPipe.sa_mask);
    l_bReturn = (sigaction(SIGPIPE, &l_sSignalPipe, NULL) == 0);

    l_sSignal.sa_flags = SA_RESETHAND | SA_SIGINFO;
    l_sSignal.sa_sigaction = &ChHandler;
    sigemptyset(&l_sSignal.sa_mask);

    l_bReturn &= (sigaction(SIGILL,  &l_sSignal, NULL) == 0);
    l_bReturn &= (sigaction(SIGABRT, &l_sSignal, NULL) == 0);
    l_bReturn &= (sigaction(SIGFPE,  &l_sSignal, NULL) == 0);
    l_bReturn &= (sigaction(SIGBUS,  &l_sSignal, NULL) == 0);
    l_bReturn &= (sigaction(SIGSEGV, &l_sSignal, NULL) == 0);

    if (!l_bReturn)
    {
        printf("ERROR: can't initialize signal handler");
    }
    
    return l_bReturn;
}//ChInstall



////////////////////////////////////////////////////////////////////////////////
//ChInstall
static __attribute__ ((unused)) tBOOL ChInstall()
{
    tBOOL l_bReturn  = TRUE;

    if (g_stContext.iInstalled)
    {
        return FALSE;
    }

    memset(&g_stContext, 0, sizeof(g_stContext));
    g_stContext.iProcessed = 0;

    l_bReturn = ChInstallPrivate();

    if (l_bReturn)
    {
        g_stContext.iInstalled = 1;
    }
    else
    {
        printf("ERROR: can't initialize signal handler");
    }
    
    return l_bReturn;
}//ChInstall


////////////////////////////////////////////////////////////////////////////////
//ChSetHandler
static __attribute__ ((unused)) tBOOL ChSetHandler(fnCrashHandler i_fnHandler)
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
static __attribute__ ((unused)) tBOOL ChSetContext(void *i_pContext)
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
static __attribute__ ((unused)) tBOOL ChUnInstall()
{
    return TRUE;
}//ChUnInstall

#endif //PSIGNAL_H
