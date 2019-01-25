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

#define FJOURNAL_FILE_BUFFER_LENGTH                         (0x20000)
#define FJOURNAL_FILE_BUFFER_MINIMUM_REMAINDER              (0x2000)
#define FJOURNAL_THREAD_WAKEUP_INTERVAL                     (2500)
#define FJOURNAL_FILE_EXTENSION                             L"log"
#define FJOURNAL_DESCRIPTION_MAX_LENGTH                     (64)
#define FJOURNAL_MAX_FILES                                  (7)


void __cdecl FJournal_Routine(void *i_pContext);

// 1. Dump to the file
// 2. must have parameters and possibility to export them
// 3. should have own thread to notify about writing, every 1-5 seconds.

class CJournal
    : virtual public IJournal
{
private:
    //put volatile variables at the top, to obtain 32 bit alignment. 
    //Project has 8 bytes alignment by default
    volatile LONG      m_lReference_Counter;

    HANDLE             m_hLog_File;
    wchar_t           *m_pJournal_Buffer;
    wchar_t           *m_pWrite_Buffer;
    DWORD              m_dwBuffer_Used;

    HANDLE             m_hThread;
    HANDLE             m_hEvent_Exit;
    HANDLE             m_hEvent_Write;

    CRITICAL_SECTION   m_tCS;

    SYSTEMTIME         m_tTime;
    wchar_t            m_pTypes_Description[IJournal::eLEVEL_COUNT][FJOURNAL_DESCRIPTION_MAX_LENGTH];
    CWString           m_cFile_Name;
    UINT64             m_pCount[IJournal::eLEVEL_COUNT];

    IJournal::eLevel   m_eVerbosity;

    IJournal::eTime    m_eTime;  //ignored for a while
    tBOOL              m_bFunction;
    tBOOL              m_bType;

    BOOL               m_bInitialized;
    BOOL               m_bError;

    DWORD              m_dwMax_Files;
public: 
    ////////////////////////////////////////////////////////////////////////////
    CJournal()
       : m_lReference_Counter(1)
       , m_hLog_File(INVALID_HANDLE_VALUE)
       , m_pJournal_Buffer(NULL)
       , m_pWrite_Buffer(NULL)
       , m_dwBuffer_Used(0)
       , m_hThread(NULL)
       , m_hEvent_Exit(NULL)
       , m_hEvent_Write(NULL)
       , m_eVerbosity(IJournal::eLEVEL_TRACE)
       , m_eTime(IJournal::eTIME_YYYYMMDDHHMMSSMS)
       , m_bFunction(TRUE)
       , m_bType(TRUE)
       , m_bInitialized(FALSE)
       , m_bError(FALSE)
       , m_dwMax_Files(FJOURNAL_MAX_FILES)
    {
        memset(&m_tCS, 0, sizeof(CRITICAL_SECTION));
        memset(&m_pCount, 0, sizeof(m_pCount));

        InitializeCriticalSection(&m_tCS);

        wcscpy_s(m_pTypes_Description[IJournal::eLEVEL_TRACE],    FJOURNAL_DESCRIPTION_MAX_LENGTH, L"INFO : ");
        wcscpy_s(m_pTypes_Description[IJournal::eLEVEL_DEBUG],    FJOURNAL_DESCRIPTION_MAX_LENGTH, L"DEBUG: ");
        wcscpy_s(m_pTypes_Description[IJournal::eLEVEL_INFO],     FJOURNAL_DESCRIPTION_MAX_LENGTH, L"INFO : ");
        wcscpy_s(m_pTypes_Description[IJournal::eLEVEL_WARNING],  FJOURNAL_DESCRIPTION_MAX_LENGTH, L"WARN.: ");
        wcscpy_s(m_pTypes_Description[IJournal::eLEVEL_ERROR],    FJOURNAL_DESCRIPTION_MAX_LENGTH, L"ERROR: ");
        wcscpy_s(m_pTypes_Description[IJournal::eLEVEL_CRITICAL], FJOURNAL_DESCRIPTION_MAX_LENGTH, L"CRIT.: ");
    }

    ////////////////////////////////////////////////////////////////////////////
    void Set_Files_Max_Count(DWORD i_dwVal)
    {
        m_dwMax_Files = i_dwVal;
    }//Set_Files_Max_Count

    ////////////////////////////////////////////////////////////////////////////
    tBOOL Initialize(const tXCHAR *i_pName)      
    {
        if (TRUE == m_bInitialized)
        {
            return TRUE;
        }

        if  (    (NULL  == i_pName)
              || (TRUE  == m_bError)
            )
        {
            return FALSE;
        }

        CWString l_sDirectory;

        if (FALSE == m_bError)
        {
            m_pJournal_Buffer = new wchar_t[FJOURNAL_FILE_BUFFER_LENGTH];
            m_pWrite_Buffer   = new wchar_t[FJOURNAL_FILE_BUFFER_LENGTH];
            if ( (NULL == m_pJournal_Buffer) || (NULL == m_pWrite_Buffer) )
            {
                m_bError = TRUE;
            }
        }

        if (FALSE == m_bError)
        {
            CFSYS::Get_Process_Directory(&l_sDirectory);
            l_sDirectory.Append(2, L"\\", i_pName);

            if (FALSE == CFSYS::Directory_Exists(l_sDirectory.Get()))
            {
                m_bError = (! CFSYS::Directory_Create(l_sDirectory.Get()));
            }
        }


        if (FALSE == m_bError)
        {
            Remove_Old_Journals(l_sDirectory.Get(), 
                                m_dwMax_Files, 
                                FJOURNAL_FILE_EXTENSION
                               );

            DWORD l_dwIterator = 0;
            SYSTEMTIME l_tTime;
            GetLocalTime(&l_tTime);
            do
            {
                swprintf_s(m_pJournal_Buffer, 
                           FJOURNAL_FILE_BUFFER_LENGTH, 
                           L"%s\\%04d_%02d_%02d-%02d_%02d_%02d-%d.%s",
                           l_sDirectory.Get(),
                           l_tTime.wYear, 
                           l_tTime.wMonth, 
                           l_tTime.wDay,
                           l_tTime.wHour, 
                           l_tTime.wMinute, 
                           l_tTime.wSecond, 
                           l_dwIterator++,
                           FJOURNAL_FILE_EXTENSION
                           );
            } while (CFSYS::File_Exists(m_pJournal_Buffer));

            m_cFile_Name.Set((wchar_t*)m_pJournal_Buffer);

            m_hLog_File = CreateFileW(m_pJournal_Buffer, 
                                      GENERIC_WRITE | GENERIC_READ, 
                                      FILE_SHARE_READ,
                                      NULL, 
                                      OPEN_ALWAYS, 
                                      FILE_ATTRIBUTE_NORMAL,
                                      NULL
                                     );

            if (    (NULL == m_hLog_File) 
                 || (INVALID_HANDLE_VALUE == m_hLog_File)
               )
            {
                m_hLog_File = INVALID_HANDLE_VALUE;
                m_bError    = TRUE;
            }
        }

        if (FALSE == m_bError)
        {
            m_hEvent_Exit  = CreateEventW(NULL, FALSE, FALSE, NULL);
            m_hEvent_Write = CreateEventW(NULL, TRUE, FALSE, NULL);
            if ( (NULL == m_hEvent_Exit) || (NULL == m_hEvent_Write) )
            {
                m_bError = TRUE;
            }
        }

        if (FALSE == m_bError)
        {
            m_pJournal_Buffer[0] = 0xFEFF;
            m_dwBuffer_Used = 1;

            m_hThread = (void*)_beginthread(&Routine_Entry_Point, 0, this);
            if (NULL == m_hThread)
            {
                m_bError = TRUE;
            }
        }

        if (FALSE == m_bError)
        {
            m_bInitialized = TRUE;
        }

        return (! m_bError);
    }

    ////////////////////////////////////////////////////////////////////////////
    void Set_Verbosity(IJournal::eLevel i_eVerbosity)
    {
        EnterCriticalSection(&m_tCS);
        m_eVerbosity = i_eVerbosity;
        LeaveCriticalSection(&m_tCS);
    }

    ////////////////////////////////////////////////////////////////////////////
    void Set_Format(IJournal::eTime i_eTime, tBOOL i_bFunction, tBOOL i_bType)
    {
        EnterCriticalSection(&m_tCS);
        m_eTime     = i_eTime; 
        m_bFunction = i_bFunction;
        m_bType     = i_bType;
        LeaveCriticalSection(&m_tCS);
    }

    ////////////////////////////////////////////////////////////////////////////
    IJournal::eLevel Get_Verbosity()
    {
        IJournal::eLevel l_eResult;
        EnterCriticalSection(&m_tCS);
        l_eResult = m_eVerbosity;
        LeaveCriticalSection(&m_tCS);

        return l_eResult;
    }

    ////////////////////////////////////////////////////////////////////////////
    tBOOL Log(IJournal::eLevel  i_eType, 
              IJournal::hModule i_hModule,
              const char       *i_pFile,
              const char       *i_pFunction, 
              tUINT32           i_dwLine,
              const tXCHAR     *i_pFormat, 
              ...
             )
    {
        tBOOL    l_bResult = TRUE;
        int      l_iCount  = 0;
        wchar_t *l_pBuffer = NULL;

        if (    (FALSE == m_bInitialized)
             || (NULL == i_pFormat) 
             || (i_eType >= IJournal::eLEVEL_COUNT) 
           )
        {
            return FALSE;
        }

        EnterCriticalSection(&m_tCS);

        m_pCount[i_eType]++;

        if (i_eType < m_eVerbosity)
        {
            l_bResult = FALSE;
            goto l_lExit;
        }

        if (    (m_dwBuffer_Used >= FJOURNAL_FILE_BUFFER_LENGTH) 
             || (FJOURNAL_FILE_BUFFER_MINIMUM_REMAINDER >= (FJOURNAL_FILE_BUFFER_LENGTH - m_dwBuffer_Used) )
           )
        {
            SetEvent(m_hEvent_Write);

            while (WAIT_OBJECT_0 == WaitForSingleObject(m_hEvent_Write, 0))
            {
                Sleep(5);
            }
        }


        if (    (l_bResult) 
             && (i_pFunction) 
             && (m_bFunction)
           )
        {
            l_pBuffer = m_pJournal_Buffer + m_dwBuffer_Used;

            while (*i_pFunction)
            {
               (*l_pBuffer++) = (*i_pFunction++);
               m_dwBuffer_Used ++;
            }

            l_iCount = swprintf_s(m_pJournal_Buffer + m_dwBuffer_Used, 
                                  FJOURNAL_FILE_BUFFER_LENGTH - m_dwBuffer_Used,
                                  L"() Ln %04d : ", i_dwLine
                                 );
            if (0 > l_iCount)
            {
                l_bResult = FALSE;
            }
            else
            {
                m_dwBuffer_Used += l_iCount;
            }

            l_pBuffer = m_pJournal_Buffer + m_dwBuffer_Used;
            (*l_pBuffer++) = 13;
            (*l_pBuffer++) = 10;
            m_dwBuffer_Used += 2;
        }

        if (l_bResult)
        {
            GetLocalTime(&m_tTime);
            l_iCount = swprintf_s(m_pJournal_Buffer + m_dwBuffer_Used, 
                                  FJOURNAL_FILE_BUFFER_LENGTH - m_dwBuffer_Used,
                                  L"[%04d.%02d.%02d %02d:%02d:%02d.%03d] %s ",
                                  m_tTime.wYear, m_tTime.wMonth, m_tTime.wDay,
                                  m_tTime.wHour, m_tTime.wMinute, m_tTime.wSecond, m_tTime.wMilliseconds,
                                  m_bType ? m_pTypes_Description[i_eType] : L""
                                 );
            if (0 > l_iCount)
            {
                l_bResult = FALSE;
            }
            else
            {
                m_dwBuffer_Used += l_iCount;
            }
        }

        if (l_bResult)
        {
            va_list l_pArgList;
            va_start(l_pArgList, i_pFormat);
            size_t l_stRemaining = 0; 

            if (S_OK == StringCbVPrintfExW(m_pJournal_Buffer + m_dwBuffer_Used,
                                           (FJOURNAL_FILE_BUFFER_LENGTH - m_dwBuffer_Used) * sizeof(wchar_t),
                                           NULL,
                                           &l_stRemaining,
                                           STRSAFE_IGNORE_NULLS,
                                           i_pFormat,
                                           l_pArgList
                                           )
               )
            {
                m_dwBuffer_Used = FJOURNAL_FILE_BUFFER_LENGTH - (DWORD)(l_stRemaining / sizeof(wchar_t));
            }
            else
            {
                l_bResult = FALSE;
            }

            va_end(l_pArgList);
        }

        if (l_bResult)
        {
            l_pBuffer = m_pJournal_Buffer + m_dwBuffer_Used;
            (*l_pBuffer++) = 13;
            (*l_pBuffer++) = 10;
            m_dwBuffer_Used += 2;
        }

    l_lExit:

        LeaveCriticalSection(&m_tCS);

        return l_bResult;
    }

    ////////////////////////////////////////////////////////////////////////////
    wchar_t *Get_File_Name()
    {
        if (FALSE == m_bInitialized)
        {
            return NULL;
        }

        return m_cFile_Name.Get();
    }

    ////////////////////////////////////////////////////////////////////////////
    tUINT64 Get_Count(IJournal::eLevel i_eType)
    {
        UINT64 l_qwReturn = 0ULL;
        if (i_eType >= IJournal::eLEVEL_COUNT) 
        {
            return 0ULL;
        }

        EnterCriticalSection(&m_tCS);
        l_qwReturn = m_pCount[i_eType];
        LeaveCriticalSection(&m_tCS);

        return l_qwReturn;
    }

    ////////////////////////////////////////////////////////////////////////////
    tINT32 Add_Ref()
    {
        return InterlockedIncrement(&m_lReference_Counter);
    }

    ////////////////////////////////////////////////////////////////////////////
    tINT32 Release()
    {
        volatile tINT32 l_iRCnt = InterlockedDecrement(&m_lReference_Counter);

        if ( 0 >= l_iRCnt)
        {
            delete this;
        }

        return l_iRCnt;
    }

    ////////////////////////////////////////////////////////////////////////////
    BOOL Get_Initialized()
    {
        return m_bInitialized;
    }

    ////////////////////////////////////////////////////////////////////////////
    tBOOL Register_Thread(const tXCHAR *i_pName, tUINT32 i_dwThreadId)
    {
        return FALSE;
    }

    ////////////////////////////////////////////////////////////////////////////
    tBOOL Unregister_Thread(tUINT32 i_dwThreadId)
    {
        return FALSE;
    }

    ////////////////////////////////////////////////////////////////////////////
    tBOOL Register_Module(const tXCHAR *i_pName, IJournal::hModule *o_hModule)
    {
        if (o_hModule)
        {
            *o_hModule = NULL;
        }

        return FALSE;
    }

protected:
    ////////////////////////////////////////////////////////////////////////////
    static void __cdecl Routine_Entry_Point(void *i_pContext)
    {
        CJournal * l_pFJournal = (CJournal *)i_pContext;
        if (l_pFJournal)
        {
            l_pFJournal->Routine();
        }
        _endthread();
    }

    ////////////////////////////////////////////////////////////////////////////
    void Routine()
    {
        BOOL   l_bExit = FALSE;
        HANDLE l_pEvents[] = {m_hEvent_Exit, m_hEvent_Write};
        DWORD  l_dwWFMO_result = 0;
        DWORD  l_dwSize        = 0;

        while (FALSE == l_bExit)
        {
            l_dwWFMO_result = WaitForMultipleObjects(sizeof(l_pEvents)/sizeof(l_pEvents[0]), 
                                                     l_pEvents, 
                                                     FALSE, 
                                                     FJOURNAL_THREAD_WAKEUP_INTERVAL
                                                    );  //Process events
              

            //   Exit signal
            if (WAIT_OBJECT_0 == l_dwWFMO_result)
            {
                l_dwSize = Copy_Bufer(TRUE);
                if (l_dwSize)
                {
                    Write((BYTE*)m_pWrite_Buffer, l_dwSize);
                }

                l_bExit = TRUE;
            }
            else if ( (WAIT_OBJECT_0 + 1) == l_dwWFMO_result)// we have received signal for write 
            {
                l_dwSize = Copy_Bufer(FALSE);
                ResetEvent(m_hEvent_Write);

                if ( (l_dwSize) && (FALSE == Write((BYTE*)m_pWrite_Buffer, l_dwSize)) )
                {
                    l_bExit = TRUE;
                }
            }
            else if (WAIT_TIMEOUT == l_dwWFMO_result)
            {
                l_dwSize = Copy_Bufer(TRUE);

                if ( (l_dwSize) && (FALSE == Write((BYTE*)m_pWrite_Buffer, l_dwSize)) )
                {
                    l_bExit = TRUE;
                }
            }
        }

        m_hThread = NULL;
    }

private:
    ////////////////////////////////////////////////////////////////////////////
    DWORD Copy_Bufer(BOOL i_bLock)
    {
        DWORD l_dwResult = 0;

        if (i_bLock)
        {
            EnterCriticalSection(&m_tCS);
        }

        if (m_dwBuffer_Used)
        {
            l_dwResult = m_dwBuffer_Used * sizeof(wchar_t);
            memcpy_s(m_pWrite_Buffer, FJOURNAL_FILE_BUFFER_LENGTH * sizeof(wchar_t), m_pJournal_Buffer, l_dwResult);
            m_dwBuffer_Used = 0;
        }

        if (i_bLock)
        {
            LeaveCriticalSection(&m_tCS);
        }

        return l_dwResult;
    }

    ////////////////////////////////////////////////////////////////////////////
    BOOL Write(BYTE *i_pBuffer, DWORD i_dwSize)
    {
        BOOL  l_bResult    = TRUE;
        DWORD l_dwReturned = 0;
        DWORD l_dwWriten   = 0;

        if ( (0 >= i_dwSize) || (NULL == i_pBuffer) )
        {
            return FALSE;
        }
        
        while ( l_dwWriten != i_dwSize )
        {
            l_dwReturned = 0;
            if ( 0 == WriteFile(m_hLog_File,
                                i_pBuffer + l_dwWriten,
                                i_dwSize - l_dwWriten,
                                &l_dwReturned, 
                                NULL)
               )
            {
                i_dwSize = 0;
                l_dwWriten = 0;
                l_bResult = FALSE;
            }
            else
            {
                l_dwWriten += l_dwReturned;
            }
        } //while

        return l_bResult;
    }

    ////////////////////////////////////////////////////////////////////////////
    BOOL Remove_Old_Journals(wchar_t *i_pFolder, DWORD i_dwMax_Files, const wchar_t *i_pExtension) 
    {
        wchar_t          l_pFileName[ MAX_PATH ] = L"9999";
        DWORD            l_dwPrev_Error_Mode = 0;
        WIN32_FIND_DATAW l_tFind_Info;
        HANDLE           l_hFind = INVALID_HANDLE_VALUE;
        DWORD            l_dwCount = 0;
        BOOL             l_bResult = TRUE;

        if ( (NULL == i_pFolder) || (NULL == i_pExtension) )
        {
            return FALSE;
        }

        l_dwPrev_Error_Mode = SetErrorMode(SEM_FAILCRITICALERRORS);

        l_dwCount = i_dwMax_Files + 1;

        while ( (l_dwCount > i_dwMax_Files) && (l_bResult) )
        {
            l_dwCount = 0;

            swprintf_s(m_pJournal_Buffer, FJOURNAL_FILE_BUFFER_LENGTH, L"%s\\*.%s", i_pFolder, i_pExtension);
            
            l_hFind = FindFirstFileW(m_pJournal_Buffer, &l_tFind_Info);

            while(INVALID_HANDLE_VALUE != l_hFind)
            {
                if (0 == (l_tFind_Info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                {
                    l_dwCount ++;
                    if (0 <= _wcsicmp(l_pFileName, l_tFind_Info.cFileName))
                    {
                        wcscpy_s(l_pFileName, MAX_PATH, l_tFind_Info.cFileName);
                    }

                    if ( ! FindNextFileW(l_hFind, &l_tFind_Info))
                    {
                        FindClose(l_hFind);
                        l_hFind = INVALID_HANDLE_VALUE;
                    }
                }
            } //while(INVALID_HANDLE_VALUE != hFind)

            SetErrorMode(l_dwPrev_Error_Mode);

            if (l_dwCount > i_dwMax_Files)
            {
                swprintf_s(m_pJournal_Buffer, FJOURNAL_FILE_BUFFER_LENGTH, L"%s\\%s", i_pFolder, l_pFileName);

                DWORD l_dwFAttr = GetFileAttributesW(m_pJournal_Buffer);
                l_dwFAttr = l_dwFAttr & (~FILE_ATTRIBUTE_ARCHIVE);
                l_dwFAttr = l_dwFAttr & (~FILE_ATTRIBUTE_READONLY);
                l_dwFAttr = l_dwFAttr & (~FILE_ATTRIBUTE_SYSTEM);
                SetFileAttributesW(m_pJournal_Buffer, l_dwFAttr);
                l_bResult = DeleteFileW(m_pJournal_Buffer);
                l_dwCount --;
                wcscpy_s(l_pFileName, MAX_PATH, L"9999");
            }    
        }

        return l_bResult;
    }

    ////////////////////////////////////////////////////////////////////////////
    virtual ~CJournal() //use Release() instead
    {
        if (m_hThread)
        {
            if (m_hEvent_Exit)
            {
                SetEvent(m_hEvent_Exit);
                if (WAIT_TIMEOUT == WaitForSingleObject(m_hThread, 150000))
                {
                    OutputDebugStringW(L"~CFJournal::Thread exit failed !\n");
                }
                CloseHandle(m_hEvent_Exit);
                m_hEvent_Exit = NULL;
            }
        }

        if (m_hEvent_Write)
        {
            CloseHandle(m_hEvent_Write);
            m_hEvent_Write = NULL;
        }

        if (INVALID_HANDLE_VALUE != m_hLog_File)
        {
            CloseHandle(m_hLog_File);
            m_hLog_File = INVALID_HANDLE_VALUE;
        }

        if (m_pJournal_Buffer)
        {
            delete [] m_pJournal_Buffer;
            m_pJournal_Buffer = NULL;
        }

        if (m_pWrite_Buffer)
        {
            delete [] m_pWrite_Buffer;
            m_pWrite_Buffer = NULL;
        }

        DeleteCriticalSection(&m_tCS);
    }

};
