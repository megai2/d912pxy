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
#pragma once

#include <process.h>

#define THSHELL_CALL_TYPE     __stdcall
#define THSHELL_RET_TYPE      tUINT32

#define THSHELL_RET_OK        0
#define THSHELL_RET_NOK       1


////////////////////////////////////////////////////////////////////////////////
class CThShell
{
public:
    ////////////////////////////////////////////////////////////////////////////
    //Base types
    typedef HANDLE  tTHREAD;
    typedef THSHELL_RET_TYPE (THSHELL_CALL_TYPE *tpThreadProc)(void *i_pParameter);


    ////////////////////////////////////////////////////////////////////////////
    //Create
    static tBOOL Create(tpThreadProc  i_pThread_Proc, 
                        void         *i_pPrm, 
                        tTHREAD      *o_pThread,
                        const tXCHAR *i_pName
                       )
    {
        if (    ( NULL == i_pThread_Proc )
             || ( NULL == o_pThread )
           )
        {
            return FALSE;
        }

        *o_pThread = (tTHREAD)_beginthreadex( NULL, 
                                              0, 
                                              i_pThread_Proc,
                                              i_pPrm, 
                                              0, 
                                              NULL
                                            );

        //SetThreadName( *pThread_id, thread_name);

        return (*o_pThread) ? TRUE : FALSE;
    }//Create


    ////////////////////////////////////////////////////////////////////////////
    //Cleanup
    static void Cleanup()
    {
        _endthreadex( 0 );
    }//Thread_Cleanup


    ////////////////////////////////////////////////////////////////////////////
    //Close
    static tBOOL Close(tTHREAD i_hThread, tUINT32 i_dwTimeOut)
    {
        if (NULL == i_hThread)
        {
            return TRUE;
        }

        if ( WAIT_OBJECT_0 == WaitForSingleObject( i_hThread, i_dwTimeOut ) )
        {
            CloseHandle(i_hThread);
            return TRUE;
        }

        return FALSE;
    }//Close


    ////////////////////////////////////////////////////////////////////////////
    //Sleep
    static void Sleep(tUINT32 i_dwTimeOut)
    {
        ::Sleep(i_dwTimeOut);
    }//Sleep
};

