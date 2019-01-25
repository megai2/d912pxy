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
#ifndef PPROCESS_H
#define PPROCESS_H

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <fcntl.h>
#include <asm/param.h> //HZ macro !
#include <sched.h>

class CProc
{
public:
    ///////////////////////////////////////////////////////////////////////////////
    //Get_ArgV
    static tXCHAR **Get_ArgV(tINT32 *o_pCount)
    {
        const tUINT16 l_wLen    = 4096;
        tXCHAR       *l_pBuffer = new tXCHAR[l_wLen];
        tXCHAR      **l_pReturn = NULL;
        int           l_pFile   = -1;
        tINT32        l_iRead   = -1;
        tBOOL         l_bNew    = TRUE;
        tINT32        l_iIDX    = 0;

        if (    (NULL == l_pBuffer)
            || (NULL == o_pCount)    
        )
        {
            goto l_lblExit;
        }

        l_pFile = open("/proc/self/cmdline", O_RDONLY);

        if (-1 == l_pFile)
        {
            goto l_lblExit;
        }

        l_iRead = read(l_pFile, l_pBuffer, l_wLen * sizeof(tXCHAR));

        if (0 >= l_iRead)
        {
            goto l_lblExit;
        }

        //calculate count of items//////////////////////////////////////////////////
        *o_pCount = 0;
        for (tINT32 l_iI = 0; l_iI < l_iRead; l_iI++)
        {
            if (    (0 == l_pBuffer[l_iI])
                || ((l_iI + 1) == l_iRead)    
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
        for (tINT32 l_iI = 0; l_iI < l_iRead; l_iI++)
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

        if (-1 != l_pFile)
        {
            close(l_pFile);
        }

        if (NULL == l_pReturn)
        {
            if (l_pBuffer)
            {
                delete [] l_pBuffer;
                l_pBuffer = NULL;
            }
        }

        return l_pReturn;
    //  return = CommandLineToArgvW(GetCommandLineW(), o_pCount);
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

        l_iLen = (tINT32)strlen(i_pCmdLine) + 1;
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
        const int  l_iLen     = 8192;
        char      *l_pBuffer  = new char[l_iLen];
        int        l_iRead    = 0;
        int        l_pFile    = -1;
        char      *l_pIter    = NULL; 
        long long  l_llBtime  = 0;
        long long  l_llStime  = 0;
        int        l_iSscanf  = 0;
        tBOOL      l_bResult  = FALSE;

        if (    (NULL == l_pBuffer)
            || (NULL == o_pHTime)    
            || (NULL == o_pLTime)   
        )
        {
            goto l_lblExit;
        }

        *o_pHTime = 0;
        *o_pLTime = 0;

        //read global stat//////////////////////////////////////////////////////////
        l_pFile = open("/proc/stat", O_RDONLY);
        if (-1 == l_pFile)
        {
            goto l_lblExit;
        }

        l_iRead = read(l_pFile, l_pBuffer, l_iLen);
        close(l_pFile);

        if (0 >= l_iRead)
        {
            goto l_lblExit;
        }

        if (l_iRead < l_iLen)
        {
            l_pBuffer[l_iRead -1] = 0;
        }
        else
        {
            l_pBuffer[l_iLen -1] = 0;
        }

        //printf((char*)l_pBuffer, 0);
        //printf("\nLen = %d\n", l_iRead);

        //get "btime" value/////////////////////////////////////////////////////////
        {
            const char l_pBtime[] = "btime ";
            int        l_iBLen    = strlen(l_pBtime);

            l_pIter = l_pBuffer;
            for (int l_iI = 0; l_iI < l_iRead; l_iI++)
            {
                if (0 == strncmp(l_pBtime, l_pIter, l_iBLen))
                {
                    l_iSscanf = sscanf(l_pIter + l_iBLen, "%lld", &l_llBtime);
                    break;
                }
                else
                {
                    l_pIter++;
                }
            }
        }

        if (0 >= l_iSscanf)
        {
            goto l_lblExit;
        }

        //read process statistics///////////////////////////////////////////////////
        l_pFile = open("/proc/self/stat", O_RDONLY);

        if (-1 == l_pFile)
        {
            goto l_lblExit;
        }

        l_iRead = read(l_pFile, l_pBuffer, l_iLen);
        close(l_pFile);

        if (0 >= l_iRead)
        {
            goto l_lblExit;
        }

        if (l_iRead < l_iLen)
        {
            l_pBuffer[l_iRead -1] = 0;
        }
        else
        {
            l_pBuffer[l_iLen -1] = 0;
        }

        //get process start time////////////////////////////////////////////////////
        {
            int l_iItem = 1;
            l_pIter = l_pBuffer;
            for (int l_iI = 0; l_iI < l_iRead; l_iI++)
            {
                if (' ' == *l_pIter)
                {
                    if (21 == l_iItem)
                    {
                        l_iSscanf = sscanf(l_pIter, " %lld", &l_llStime);
                        break;
                    }
                    else
                    {
                        l_iItem ++;
                    }
                }

                l_pIter++;
            }
        }

        if (0 >= l_iSscanf)
        {
            goto l_lblExit;
        }

        //printf((char*)l_pBuffer, 0);
        //printf("\nLen = %d\n", l_iRead);

        //printf("\n Len = %d, BTime = %lld, STime = %lld\n", 
        //       l_iLen, 
        //       l_llBtime, 
        //       l_llStime
        //      );

        //HZ defined in #include <asm/param.h>
        l_llStime  = l_llBtime + (l_llStime / HZ);
        l_llStime  = l_llStime * 10000000;
        l_llStime += TIME_OFFSET_1601_1970; //defined in "PTime.h"

        *o_pHTime = (l_llStime >> 32);
        *o_pLTime = (l_llStime & 0xFFFFFFFF);

        l_bResult = TRUE;

        //{
        //    time_t l_sTime = time(NULL);
        //    struct tm *l_pTime = localtime(&l_sTime);;
        //
        //    printf("Current time: MM=%d DD=%d, HH=%d, MM=%d, SS=%d\n",
        //            l_pTime->tm_mon,
        //            l_pTime->tm_mday,
        //            l_pTime->tm_hour,
        //            l_pTime->tm_min,
        //            l_pTime->tm_sec
        //            );
        //
        //    l_pTime = localtime((time_t*)&l_llResult);
        //    printf("Process Time: MM=%d DD=%d, HH=%d, MM=%d, SS=%d\n",
        //            l_pTime->tm_mon,
        //            l_pTime->tm_mday,
        //            l_pTime->tm_hour,
        //            l_pTime->tm_min,
        //            l_pTime->tm_sec
        //            );
        //}

    l_lblExit:
        if (l_pBuffer)
        {
            delete [] l_pBuffer;
            l_pBuffer = NULL;
        }

        return l_bResult;

        //FILETIME l_tProcess_Time = {0};
        //FILETIME l_tStub_01      = {0};
        //FILETIME l_tStub_02      = {0};
        //FILETIME l_tStub_03      = {0};
        //
        //GetProcessTimes(GetCurrentProcess(), 
        //                &l_tProcess_Time, 
        //                &l_tStub_01,
        //                &l_tStub_02,
        //                &l_tStub_03
        //                );
        //
        //*o_pHTime = l_tProcess_Time.dwHighDateTime,
        //*o_pLTime = l_tProcess_Time.dwLowDateTime,
    }//Get_Process_Time()


    ////////////////////////////////////////////////////////////////////////////////
    //Get_Process_Name
    static tBOOL Get_Process_Name(tACHAR *o_pName, tINT32 i_iMax_Len)
    {
        int   l_iRead   = 0;
        char *l_pName   = NULL;
        char *l_pDst    = o_pName;
        tBOOL l_bReturn = FALSE;

        if (    (NULL == o_pName)
            || (32   >= i_iMax_Len)
            || (NULL == o_pName)
        )
        {
            goto l_lblExit;
        }

        l_iRead = readlink("/proc/self/exe", o_pName, i_iMax_Len - 1);
        if (0 >= l_iRead) 
        {
            // I guess we're not running on the right version of unix
            goto l_lblExit;
        }

        if (l_iRead >= i_iMax_Len)
        {
            l_iRead = i_iMax_Len - 1;
        }

        o_pName[l_iRead] = '\0';

        l_pName = o_pName + l_iRead;

        while (l_pName != o_pName)
        {
            if (    ('/' == *l_pName)
                || ('\\' == *l_pName)   
            )
            {
                l_pName ++;
                break;
            }
            else
            {
                l_pName --;
            }
        }

        while (*l_pName)
        {
            *l_pDst++=*l_pName++;
        }
        *l_pDst = 0;

        l_bReturn = TRUE;

    l_lblExit:
        return l_bReturn;
    }//Get_Process_Name

    ////////////////////////////////////////////////////////////////////////////////
    //Get_Process_Name
    static tBOOL Get_Process_Name(tWCHAR *o_pName, tINT32 i_iMax_Len)
        {
        int   l_iLen    = 4096;
        char *l_pBuffer = new char[l_iLen];
        tBOOL l_bReturn = FALSE;


        if (    (l_pBuffer)
             && (Get_Process_Name(l_pBuffer, l_iLen))
           )
        {
            l_bReturn = TRUE;
            Convert_UTF8_To_UTF16(l_pBuffer, o_pName, i_iMax_Len);
        }
        else
        {
            Convert_UTF8_To_UTF16("Unknown", o_pName, i_iMax_Len);
        }
    
        if (l_pBuffer)
        {
            delete [] l_pBuffer;
            l_pBuffer = NULL;
        }

        return l_bReturn;
    }//Get_Process_Name


    ////////////////////////////////////////////////////////////////////////////////
    //Get_Process_Path
    static tBOOL Get_Process_Path(tXCHAR *o_pPath, tINT32 i_iMax_Len)
    {
        int   l_iRead   = 0;
        tBOOL l_bReturn = FALSE;

        if (    (NULL == o_pPath)
             || (32   >= i_iMax_Len)
           )
        {
            goto l_lblExit;
        }

        l_iRead = readlink("/proc/self/exe", o_pPath, i_iMax_Len - 4);
        if (0 >= l_iRead)
        {
            // I guess we're not running on the right version of unix
            goto l_lblExit;
        }

        if (l_iRead >= i_iMax_Len)
        {
            // buffer is too small
            goto l_lblExit;
        }

        o_pPath[l_iRead] = '\0';

        while (l_iRead)
        {
            if (    ('/' == o_pPath[l_iRead])
                 || ('\\' == o_pPath[l_iRead])
               )
            {
                break;
            }
            else
            {
                o_pPath[l_iRead--] = 0;
            }
        }

        l_bReturn = TRUE;

    l_lblExit:

        if (TRUE != l_bReturn)
        {
            strcpy(o_pPath, "./");
        }

        return l_bReturn;
    }//Get_Process_Path


    ////////////////////////////////////////////////////////////////////////////////
    //Get_Process_ID
    static tUINT32 Get_Process_ID()
    {
        return getpid();
        //return GetCurrentProcessId();
    }//Get_Process_ID
    
    
    ////////////////////////////////////////////////////////////////////////////////
    //Get_Thread_Id
    static tUINT32 Get_Thread_Id()
    {
        return (int32_t)syscall(SYS_gettid);
        //return pthread_self();
        //return gettid();
    }//Get_Thread_Id
    
    
    ////////////////////////////////////////////////////////////////////////////////
    //Get_Processor
    static __forceinline tUINT32 Get_Processor()
    {
#ifdef __ARM_ARCH_5TEJ__
        return 0;        
#else
        return sched_getcpu();        
#endif
    }//Get_Processor
};

#endif //PPROCESS_H
