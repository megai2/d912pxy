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


////////////////////////////////////////////////////////////////////////////////
//CShared
class CShared
{
    struct sShared
    {
        HANDLE  hMemory;
        HANDLE  hMutex;
    };

    enum eType
    {
        ETYPE_MUTEX   = 0,
        ETYPE_FILE       ,
        ETYPE_MAX        
    };

public:
    typedef sShared *hShared;

    enum eLock
    {
        E_OK,
        E_TIMEOUT,
        E_ERROR,
        E_NOT_EXISTS
    };

    ////////////////////////////////////////////////////////////////////////////
    //CShared::Create
    static tBOOL Create(CShared::hShared *o_pHandle,
                        const tXCHAR     *i_pName, 
                        const tUINT8     *i_pData, 
                        tUINT16           i_wSize
                       )
    {
        sShared *l_pShared  = NULL;
        tBOOL    l_bReturn  = TRUE;
        DWORD    l_dwLen    = 0;
        wchar_t *l_pName    = NULL;
        BOOL     l_bRelease = FALSE;
        tUINT8  *l_pBuffer  = NULL;


        if (    (NULL == i_pName)
             || (NULL == i_pData)
             || (0    >= i_wSize)
             || (NULL == o_pHandle)
           )
        {
            l_bReturn = FALSE;
            goto l_lblExit;
        }

        l_pShared = new sShared;
        if (NULL == l_pShared)
        {
            l_bReturn = FALSE;
            goto l_lblExit;
        }

        memset(l_pShared, 0, sizeof(sShared));

        l_dwLen = (DWORD)wcslen(i_pName) + 128;
        l_pName = (wchar_t*)malloc(sizeof(wchar_t) * l_dwLen);

        if (NULL == l_pName)
        {
            l_bReturn = FALSE;
            goto l_lblExit;
        }


        ////////////////////////////////////////////////////////////////////////////
        //create mutex and own it
        Create_Name(l_pName, l_dwLen, ETYPE_MUTEX, i_pName);

        l_pShared->hMutex = CreateMutexW(NULL, TRUE, l_pName);
        if (    (NULL == l_pShared->hMutex)
             || (ERROR_ALREADY_EXISTS == GetLastError())
           )
        {
            l_bReturn = FALSE;
            goto l_lblExit;
        }

        l_bRelease = TRUE;

        ////////////////////////////////////////////////////////////////////////////
        //create shared memory object
        Create_Name(l_pName, l_dwLen, ETYPE_FILE, i_pName);

        l_pShared->hMemory = CreateFileMappingW(INVALID_HANDLE_VALUE, // use paging file
                                                NULL,                 // default security
                                                PAGE_READWRITE,       // read/write access
                                                0,                    // maximum object size (high-order tUINT32)
                                                i_wSize,              // maximum object size (low-order tUINT32)
                                                l_pName               // name of mapping object
                                               );     
    
        if (    (NULL == l_pShared->hMemory)
             || (ERROR_ALREADY_EXISTS == GetLastError())
           )
        {
            l_bReturn = FALSE;
            goto l_lblExit;
        }
    
        l_pBuffer = (tUINT8*)MapViewOfFile(l_pShared->hMemory,    
                                           FILE_MAP_ALL_ACCESS,
                                           0,
                                           0,
                                           i_wSize
                                          );
    
        if (NULL == l_pBuffer)
        {
            l_bReturn = FALSE;
            goto l_lblExit;
        }

        *o_pHandle = (CShared::hShared)l_pShared;

        __try
        {
            memcpy(l_pBuffer, i_pData, i_wSize);
        }
    
        __except (   GetExceptionCode() == EXCEPTION_IN_PAGE_ERROR 
                   ? EXCEPTION_EXECUTE_HANDLER 
                   : EXCEPTION_CONTINUE_SEARCH
                 )
        {
            l_bReturn  = FALSE;
            goto l_lblExit;
        }

    l_lblExit:
        if (l_pName)
        {
            free(l_pName);
            l_pName = NULL;
        }

        if (l_pBuffer)
        {
            UnmapViewOfFile(l_pBuffer);
            l_pBuffer = NULL;
        }

        if (l_bRelease)
        {
            ReleaseMutex(l_pShared->hMutex);
        }

        if (FALSE == l_bReturn)
        {
            Close((hShared)l_pShared);
            l_pShared = NULL;

            if (o_pHandle)
            {
                *o_pHandle = NULL;
            }
        }

        return l_bReturn;
    }//Shared_Create


    ////////////////////////////////////////////////////////////////////////////
    //Read
    static tBOOL Read(const tXCHAR  *i_pName, 
                      tUINT8        *o_pData,
                      tUINT16        i_wSize
                     )
    {
        HANDLE   l_hMemory  = NULL;
        tBOOL    l_bReturn  = TRUE;
        DWORD    l_dwLen    = 0;
        wchar_t *l_pName    = NULL;
        tUINT8  *l_pBuffer  = NULL;

        if (    (NULL == i_pName)
             || (NULL == o_pData)
             || (0    >= i_wSize)
           )
        {
            l_bReturn = FALSE;
            goto l_lblExit;
        }

        l_dwLen = (DWORD)wcslen(i_pName) + 128;
        l_pName = (wchar_t*)malloc(sizeof(wchar_t) * l_dwLen);

        if (NULL == l_pName)
        {
            l_bReturn = FALSE;
            goto l_lblExit;
        }

        ////////////////////////////////////////////////////////////////////////
        //open shared memory object
        Create_Name(l_pName, l_dwLen, ETYPE_FILE, i_pName);

        l_hMemory = OpenFileMappingW(FILE_MAP_ALL_ACCESS,  // read/write access
                                     FALSE,                // do not inherit the name
                                     l_pName               // name of mapping object
                                    );           

        if (NULL == l_hMemory)
        {
            l_bReturn = FALSE;
            goto l_lblExit;
        }
    
        l_pBuffer = (tUINT8*)MapViewOfFile(l_hMemory,    
                                           FILE_MAP_READ,
                                           0,
                                           0,
                                           i_wSize
                                          );
    
        if (NULL == l_pBuffer)
        {
            l_bReturn = FALSE;
            goto l_lblExit;
        }
    
        __try
        {
            memcpy(o_pData, l_pBuffer, i_wSize);
        }
    
        __except (   (GetExceptionCode() == EXCEPTION_IN_PAGE_ERROR)
                   ? EXCEPTION_EXECUTE_HANDLER 
                   : EXCEPTION_CONTINUE_SEARCH
                 )
        {
            l_bReturn = FALSE;
            goto l_lblExit;
        }

    l_lblExit:
        if (l_pName)
        {
            free(l_pName);
            l_pName = NULL;
        }

        if (l_pBuffer)
        {
            UnmapViewOfFile(l_pBuffer);
            l_pBuffer = NULL;
        }

        if (l_hMemory)
        {
            CloseHandle(l_hMemory);
            l_hMemory = NULL;
        }

        return l_bReturn;
    }//Read



    ////////////////////////////////////////////////////////////////////////////
    //Write
    static tBOOL Write(const tXCHAR  *i_pName, 
                       const tUINT8  *i_pData,
                       tUINT16        i_wSize
                      )
    {
        HANDLE   l_hMemory  = NULL;
        tBOOL    l_bReturn  = TRUE;
        DWORD    l_dwLen    = 0;
        wchar_t *l_pName    = NULL;
        tUINT8  *l_pBuffer  = NULL;

        if (    (NULL == i_pName)
             || (NULL == i_pData)
             || (0    >= i_wSize)
           )
        {
            l_bReturn = FALSE;
            goto l_lblExit;
        }

        l_dwLen = (DWORD)wcslen(i_pName) + 128;
        l_pName = (wchar_t*)malloc(sizeof(wchar_t) * l_dwLen);

        if (NULL == l_pName)
        {
            l_bReturn = FALSE;
            goto l_lblExit;
        }

        ////////////////////////////////////////////////////////////////////////
        //open shared memory object
        Create_Name(l_pName, l_dwLen, ETYPE_FILE, i_pName);

        l_hMemory = OpenFileMappingW(FILE_MAP_ALL_ACCESS,  // read/write access
                                     FALSE,                // do not inherit the name
                                     l_pName               // name of mapping object
                                    );           

        if (NULL == l_hMemory)
        {
            l_bReturn = FALSE;
            goto l_lblExit;
        }
    
        l_pBuffer = (tUINT8*)MapViewOfFile(l_hMemory,    
                                           FILE_MAP_WRITE,
                                           0,
                                           0,
                                           i_wSize
                                          );
    
        if (NULL == l_pBuffer)
        {
            l_bReturn = FALSE;
            goto l_lblExit;
        }
    
        __try
        {
            memcpy(l_pBuffer, i_pData, i_wSize);
        }
    
        __except (   (GetExceptionCode() == EXCEPTION_IN_PAGE_ERROR)
                   ? EXCEPTION_EXECUTE_HANDLER 
                   : EXCEPTION_CONTINUE_SEARCH
                 )
        {
            l_bReturn = FALSE;
            goto l_lblExit;
        }

    l_lblExit:
        if (l_pName)
        {
            free(l_pName);
            l_pName = NULL;
        }

        if (l_pBuffer)
        {
            UnmapViewOfFile(l_pBuffer);
            l_pBuffer = NULL;
        }

        if (l_hMemory)
        {
            CloseHandle(l_hMemory);
            l_hMemory = NULL;
        }

        return l_bReturn;
    }//Write


    ////////////////////////////////////////////////////////////////////////////
    //Lock
    static eLock Lock(const tXCHAR  *i_pName, tUINT32 i_dwTimeout_ms)
    {
        HANDLE   l_hMutex   = NULL;
        eLock    l_eReturn  = CShared::E_OK;
        DWORD    l_dwLen    = 0;
        wchar_t *l_pName    = NULL;

        if (NULL == i_pName)
        {
            l_eReturn = CShared::E_ERROR;
            goto l_lblExit;
        }

        l_dwLen = (DWORD)wcslen(i_pName) + 128;
        l_pName = (wchar_t*)malloc(sizeof(wchar_t) * l_dwLen);

        if (NULL == l_pName)
        {
            l_eReturn = CShared::E_ERROR;
            goto l_lblExit;
        }

        ////////////////////////////////////////////////////////////////////////
        //open mutex and own it
        Create_Name(l_pName, l_dwLen, ETYPE_MUTEX, i_pName);

        l_hMutex = OpenMutexW(MUTEX_ALL_ACCESS, FALSE, l_pName);
        if (NULL == l_hMutex)
        {
            l_eReturn = CShared::E_NOT_EXISTS;
            goto l_lblExit;
        }

        if (WAIT_OBJECT_0 != WaitForSingleObject(l_hMutex, i_dwTimeout_ms))
        {
            l_eReturn = CShared::E_TIMEOUT;
            goto l_lblExit;
        }

    l_lblExit:
        if (l_pName)
        {
            free(l_pName);
            l_pName = NULL;
        }

        if (l_hMutex)
        {
            CloseHandle(l_hMutex);
            l_hMutex = NULL;
        }

        return l_eReturn;
    }//Lock


    ////////////////////////////////////////////////////////////////////////////
    //UnLock
    static tBOOL UnLock(const tXCHAR  *i_pName)
    {
        HANDLE   l_hMutex   = NULL;
        tBOOL    l_bReturn  = TRUE;
        DWORD    l_dwLen    = 0;
        wchar_t *l_pName    = NULL;

        if (NULL == i_pName)
        {
            l_bReturn = FALSE;
            goto l_lblExit;
        }

        l_dwLen = (DWORD)wcslen(i_pName) + 128;
        l_pName = (wchar_t*)malloc(sizeof(wchar_t) * l_dwLen);

        if (NULL == l_pName)
        {
            l_bReturn = FALSE;
            goto l_lblExit;
        }

        ////////////////////////////////////////////////////////////////////////
        //open mutex and own it
        Create_Name(l_pName, l_dwLen, ETYPE_MUTEX, i_pName);

        l_hMutex = OpenMutexW(MUTEX_ALL_ACCESS, FALSE, l_pName);
        if (NULL == l_hMutex)
        {
            l_bReturn = FALSE;
            goto l_lblExit;
        }

        ReleaseMutex(l_hMutex);

    l_lblExit:
        if (l_pName)
        {
            free(l_pName);
            l_pName = NULL;
        }

        if (l_hMutex)
        {
            CloseHandle(l_hMutex);
            l_hMutex = NULL;
        }

        return l_bReturn;
    }//UnLock


    ////////////////////////////////////////////////////////////////////////////
    //Close
    static tBOOL Close(hShared i_hShared)
    {
        if (NULL == i_hShared)
        {
            return FALSE;
        }

        if (i_hShared->hMutex)
        {
            WaitForSingleObject(i_hShared->hMutex, 3000);
        }

        if (i_hShared->hMemory)
        {
            CloseHandle(i_hShared->hMemory);
            i_hShared->hMemory = NULL;
        }

        if (i_hShared->hMutex)
        {
            CloseHandle(i_hShared->hMutex);
            i_hShared->hMutex = NULL;
        }

        delete i_hShared;
        i_hShared = NULL;

        return TRUE;
    }//Shared_Close

private:
    ////////////////////////////////////////////////////////////////////////////
    //Create_Name
    static tBOOL Create_Name(tXCHAR        *o_pName, 
                             size_t         i_szName,
                             eType          i_eType,
                             const tXCHAR  *i_pPostfix
                            )
    {
        if (    (NULL == o_pName)
             || (16 >= i_szName)
             || (ETYPE_MAX <= i_eType)
             || (NULL == i_pPostfix)
           )
        {
            return FALSE;
        }

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

        if (ETYPE_MUTEX == i_eType)
        {
            swprintf_s(o_pName, 
                       i_szName, 
                       L"Local\\m%d%d%d%s", 
                       GetCurrentProcessId(), 
                       l_tProcess_Time.dwHighDateTime,
                       l_tProcess_Time.dwLowDateTime,
                       i_pPostfix
                      );
        }
        else if (ETYPE_FILE == i_eType)
        {
            swprintf_s(o_pName, 
                       i_szName, 
                       L"Local\\f%d%d%d%s", 
                       GetCurrentProcessId(), 
                       l_tProcess_Time.dwHighDateTime,
                       l_tProcess_Time.dwLowDateTime,
                       i_pPostfix
                      );
        }

        return TRUE;
    }//Create_Name
};








