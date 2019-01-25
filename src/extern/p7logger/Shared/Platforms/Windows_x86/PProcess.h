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

class CProc
{
public:
    ///////////////////////////////////////////////////////////////////////////////
    //Get_ArgV
    static tXCHAR **Get_ArgV(tINT32 *o_pCount)
    {
        return Get_ArgV(GetCommandLineW(), o_pCount);
    }//Get_ArgV


    ///////////////////////////////////////////////////////////////////////////////
    //Get_ArgV
    static tXCHAR **Get_ArgV(const tXCHAR *i_pCmdLine, tINT32 *o_pCount)
    {
        tINT32        l_iLen    = 0;//strlen(i_pCmdLine);
        tXCHAR       *l_pBuffer = NULL;//new tXCHAR[l_iLen];
        tXCHAR      **l_pReturn = NULL;
        tBOOL         l_bNew    = TRUE;
        tBOOL         l_bStr    = FALSE;
        tINT32        l_iIDX    = 0;

        if (    (NULL == i_pCmdLine)
             || (NULL == o_pCount)    
        )
        {
            goto l_lblExit;
        }

        l_iLen = (tINT32)wcslen(i_pCmdLine) + 1;
        l_pBuffer = new tXCHAR[l_iLen];

        if (NULL == l_pBuffer)
        {
            goto l_lblExit;
        }

        for (tINT32 l_iI = 0; l_iI < l_iLen; l_iI++)
        {
            if (TM('\"') == i_pCmdLine[l_iI])
            {
                l_bStr = ! l_bStr;
            }
            else
            {
                if (    (TM(' ') == i_pCmdLine[l_iI])
                     && (!l_bStr)
                   )
                {
                    l_pBuffer[l_iIDX++] = 0;
                }
                else
                {
                    l_pBuffer[l_iIDX++] = i_pCmdLine[l_iI];
                }
                
            }
        }  

        l_iLen = l_iIDX;

        //calculate count of items//////////////////////////////////////////////////
        *o_pCount = 0;
        for (tINT32 l_iI = 0; l_iI < l_iLen; l_iI++)
        {
            if (    (0 == l_pBuffer[l_iI])
                 || ((l_iI + 1) == l_iLen)    
               )
            {
                (*o_pCount) ++;
            }
        }

        if (0 >= *o_pCount)
        {
            goto l_lblExit;
        }

        //allocate result///////////////////////////////////////////////////////////
        l_pReturn = new tXCHAR*[*o_pCount];

        if (NULL == l_pReturn)
        {
            goto l_lblExit;
        }

        //fill result///////////////////////////////////////////////////////////////
        l_bNew = TRUE;
        l_iIDX = 0;
        for (tINT32 l_iI = 0; l_iI < l_iLen; l_iI++)
        {
            if (l_bNew)
            {
                l_pReturn[l_iIDX++] = &l_pBuffer[l_iI];
                l_bNew = FALSE;
            }

            if (0 == l_pBuffer[l_iI])
            {
                l_bNew = TRUE;
            }
        }

    l_lblExit:

        return l_pReturn;
    //  return = CommandLineToArgvW(i_pCmdLine, o_pCount);
    }//Get_ArgV


    ///////////////////////////////////////////////////////////////////////////////
    //Free_ArgV
    static void Free_ArgV(tXCHAR **i_pArgV)
    {
        if (NULL == i_pArgV)
        {
            return;
        }

        if (i_pArgV[0])
        {
            delete [] i_pArgV[0];
        }

        delete [] i_pArgV; 
        //LocalFree(i_pArgV);
    }//Free_ArgV


    ////////////////////////////////////////////////////////////////////////////////
    //Get_Process_Time()
    //return a 64-bit value divided into 2 parts representing the number of 
    //100-nanosecond intervals since January 1, 1601 (UTC).
    static tBOOL Get_Process_Time(tUINT32 *o_pHTime, tUINT32 *o_pLTime)
    {
        FILETIME l_tProcess_Time = {0};
        FILETIME l_tStub_01      = {0};
        FILETIME l_tStub_02      = {0};
        FILETIME l_tStub_03      = {0};
        
        GetProcessTimes(GetCurrentProcess(), 
                        &l_tProcess_Time, 
                        &l_tStub_01,
                        &l_tStub_02,
                        &l_tStub_03
                        );
        
        *o_pHTime = l_tProcess_Time.dwHighDateTime;
        *o_pLTime = l_tProcess_Time.dwLowDateTime;

        return TRUE;
    }//Get_Process_Time()


    ////////////////////////////////////////////////////////////////////////////////
    //Get_Process_Name
    static tBOOL Get_Process_Name(tWCHAR *o_pName, tINT32 i_iMax_Len)
    {
        BOOL           l_bReturn       = FALSE;
        const tUINT32  l_dwMax_Len     = 32768;
        wchar_t       *l_pProcess_Path = NULL;
        wchar_t       *l_pProcess_Name = NULL;
        
        if (    (NULL == o_pName)
             || (64   >= i_iMax_Len)
           )
        {
            return FALSE;
        }

        l_pProcess_Path = new wchar_t[l_dwMax_Len];

        if (NULL == l_pProcess_Path)
        {
            return FALSE;
        }

        if (GetModuleFileNameW(GetModuleHandleW(NULL), 
                               l_pProcess_Path, 
                               l_dwMax_Len
                              )
           )
        {
            if (    (l_pProcess_Name = wcsrchr(l_pProcess_Path, L'\\'))
                 || (l_pProcess_Name = wcsrchr(l_pProcess_Path, L'/'))
               )
            {
                l_pProcess_Name ++;
                l_bReturn = TRUE;
            }
        }


        if (l_bReturn)
        {
            wcscpy_s((wchar_t*)o_pName, i_iMax_Len, l_pProcess_Name);
        }

        delete [] l_pProcess_Path;

        return l_bReturn;
    }//Get_Process_Name


    ////////////////////////////////////////////////////////////////////////////////
    //Get_Process_Name
    static tBOOL Get_Process_Name(tACHAR *o_pName, tINT32 i_iMax_Len)
    {
        const tINT32 l_iSize = 128;
        tWCHAR       l_pName[l_iSize];

        if (Get_Process_Name(l_pName, l_iSize))
        {
            Convert_UTF16_To_UTF8(l_pName, o_pName, (tUINT32)i_iMax_Len);
        }
        else
        {
            strcpy_s(o_pName, (rsize_t)i_iMax_Len, "Unknown:Error");
        }

        return true;
    }//Get_Process_Name


    ////////////////////////////////////////////////////////////////////////////////
    //Get_Process_Path
    static tBOOL Get_Process_Path(tXCHAR *o_pPath, size_t i_szName)
    {
        BOOL           l_bReturn = FALSE;
        wchar_t       *l_pName   = NULL;
        
        if (    (NULL == o_pPath)
             || (256  >= i_szName)
           )
        {
            return FALSE;
        }

        if (GetModuleFileNameW(GetModuleHandleW(NULL), 
                               o_pPath, 
                               (DWORD)i_szName
                              )
           )
        {
            if (    (l_pName = wcsrchr(o_pPath, L'\\'))
                 || (l_pName = wcsrchr(o_pPath, L'/'))
               )
            {
                l_pName ++;
                l_pName[0] = 0;
                l_bReturn = TRUE;
            }
        }

        return l_bReturn;
    }//Get_Process_Path


    ////////////////////////////////////////////////////////////////////////////////
    //Get_Process_ID
    static tUINT32 Get_Process_ID()
    {
        return GetCurrentProcessId();
    }//Get_Process_ID


    ////////////////////////////////////////////////////////////////////////////////
    //Get_Thread_Id
    static tUINT32 Get_Thread_Id()
    {
        return GetCurrentThreadId();
    }//Get_Thread_Id


    ////////////////////////////////////////////////////////////////////////////////
    //Get_Processor
    static __forceinline tUINT32 Get_Processor()
    {
        #if defined(GTX64) || !defined(_MSC_VER)
           //return 0xFFFF; //don't know how to get processor number ... yet.
           return GetCurrentProcessorNumber();
        #else
           _asm
           {
               mov eax, 1
               cpuid
               shr ebx, 24
               mov eax, ebx
           }
        #endif
    }//Get_Processor
};
