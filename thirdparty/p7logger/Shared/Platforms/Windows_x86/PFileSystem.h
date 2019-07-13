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
#pragma once

#pragma warning(disable : 4091)

#include "Shlobj.h"

class CFSYS
{
public:
    ////////////////////////////////////////////////////////////////////////////
    //Directory_Exists
    static BOOL Directory_Exists(const wchar_t * i_pPath)
    {
        DWORD l_dwFile_Attr = 0;
        BOOL  l_bRes        = FALSE;
        
        if (NULL == i_pPath)
        {
            return l_bRes;
        }

        l_dwFile_Attr = GetFileAttributesW(i_pPath);
        //check if that directory is exist ...
        if ((FILE_ATTRIBUTE_DIRECTORY & l_dwFile_Attr) && 
            ( ((DWORD)-1) != l_dwFile_Attr)
           )
        {
            l_bRes = TRUE;
        }

        return l_bRes;
    }//Directory_Exists


    ////////////////////////////////////////////////////////////////////////////
    //Directory_Create
    static BOOL Directory_Create(const wchar_t *i_pDirectory)
    {
        if (NULL == i_pDirectory)
        {
            return FALSE;
        }

        wchar_t *l_pDirectory = _wcsdup(i_pDirectory);
        tBOOL    l_bReturn    = FALSE;

        if (NULL == l_pDirectory)
        {
            return FALSE;
        }

        size_t l_szLength = wcslen(l_pDirectory);

        for (size_t l_dwI = 3; /*wcslen(L"C:\\")*/ l_dwI < l_szLength; l_dwI++)
        {
            if (    (L'\\' == l_pDirectory[l_dwI])
                 || (L'/'  == l_pDirectory[l_dwI])
               )
            {
                l_pDirectory[l_dwI] = 0;
                if (FALSE == Directory_Exists(l_pDirectory))
                {
                    CreateDirectoryW(l_pDirectory, NULL);
                }

                l_pDirectory[l_dwI] = L'\\';
            }
        }

        CreateDirectoryW(l_pDirectory, NULL);
        l_bReturn = Directory_Exists(l_pDirectory);

        free(l_pDirectory);
        l_pDirectory = NULL;

        return l_bReturn;
    }//Directory_Create

    ////////////////////////////////////////////////////////////////////////////
    //Directory_Delete
    static BOOL Directory_Delete(const wchar_t *i_pDirectory)
    {
        if (NULL == i_pDirectory)
        {
            return FALSE;
        }

        return RemoveDirectoryW(i_pDirectory);
    }

    ////////////////////////////////////////////////////////////////////////////
    //File_Exists
    static BOOL File_Exists(const wchar_t * i_pFileName)
    {
        HANDLE l_hFind_Handle = NULL;
        WIN32_FIND_DATAW l_tFind_Data;
        BOOL l_bRes = FALSE;

        if (NULL == i_pFileName)
        {
            return l_bRes;
        }

        memset(&l_tFind_Data, 0, sizeof(WIN32_FIND_DATAW));

        l_hFind_Handle = FindFirstFileW(i_pFileName, &l_tFind_Data);
        if (INVALID_HANDLE_VALUE != l_hFind_Handle)
        {
            l_bRes = (0 == (l_tFind_Data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY));
            FindClose(l_hFind_Handle);
            l_hFind_Handle = NULL;
        }

        return l_bRes;
    }//File_Exists


    ////////////////////////////////////////////////////////////////////////////
    // Enumerate_Files
    static void Enumerate_Files(CBList<CWString*> *io_pFilesList, 
                                CWString          *i_pDirectory,
                                const wchar_t     *i_pMask, //for example L"*.dll"
                                DWORD              i_dwDepth = 0xFFFFFF
                               )
    {
        WIN32_FIND_DATAW l_sFind_Info = {0};
        HANDLE           l_hFind      = INVALID_HANDLE_VALUE;
        DWORD            l_dwLength   = i_pDirectory->Length();
        CWString         l_cSearch_Path;


        //**************************************************************************
        // Enumerate all files in current directory
        l_cSearch_Path.Set(i_pDirectory->Get());
        l_cSearch_Path.Append(2, L"\\", i_pMask);

        l_hFind = FindFirstFileW(l_cSearch_Path.Get(), &l_sFind_Info);

        while(INVALID_HANDLE_VALUE != l_hFind)
        {
            if (0 == (l_sFind_Info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                CWString *l_pFile = new CWString();
                if (l_pFile)
                {
                    l_pFile->Set(i_pDirectory->Get());
                    l_pFile->Append(2, L"\\", l_sFind_Info.cFileName);
                    io_pFilesList->Add_After(io_pFilesList->Get_Last(), l_pFile);
                }
            }

            if (! FindNextFileW(l_hFind, &l_sFind_Info))
            {
                FindClose(l_hFind);
                l_hFind = INVALID_HANDLE_VALUE;
            }
        } //while(INVALID_HANDLE_VALUE != hFind)


        //**************************************************************************
        // Enumerate all sub directories in current directory ....
        if (i_dwDepth)
        {
            l_cSearch_Path.Set(i_pDirectory->Get());
            l_cSearch_Path.Append(2, L"\\", i_pMask);

            l_hFind = FindFirstFileW(l_cSearch_Path.Get(), &l_sFind_Info);

            while(INVALID_HANDLE_VALUE != l_hFind)
            {
                if (    (0 != (l_sFind_Info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) 
                     && (0 != wcscmp(l_sFind_Info.cFileName, L".")) 
                     && (0 != wcscmp(l_sFind_Info.cFileName, L".."))
                   )
                {
                    i_pDirectory->Append(2, L"\\", l_sFind_Info.cFileName);
                    Enumerate_Files(io_pFilesList, i_pDirectory, i_pMask, i_dwDepth - 1);
                    i_pDirectory->Trim(l_dwLength);
                }

                if (! FindNextFileW(l_hFind, &l_sFind_Info))
                {
                    FindClose(l_hFind);
                    l_hFind = INVALID_HANDLE_VALUE;
                }
            } //while(INVALID_HANDLE_VALUE != hFind)
        }
    }// Enumerate_Files


    ////////////////////////////////////////////////////////////////////////////
    // Enumerate_Dirs
    static void Enumerate_Dirs(CBList<CWString*> *io_pFilesList, const tXCHAR *i_pRoot)
    {
        WIN32_FIND_DATAW l_sFind_Info = {0};
        HANDLE           l_hFind      = INVALID_HANDLE_VALUE;

        if (!io_pFilesList)
        {
            return;
        }
        
        if (    (i_pRoot)
             && (*i_pRoot)
           )
        {
            CWString l_cSearch_Path;


            //**************************************************************************
            // Enumerate all files in current directory
            l_cSearch_Path.Set(i_pRoot);
            l_cSearch_Path.Append(1, L"\\*");

            l_hFind = FindFirstFileW(l_cSearch_Path.Get(), &l_sFind_Info);

            while(INVALID_HANDLE_VALUE != l_hFind)
            {
                if (    (0 != (l_sFind_Info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) 
                     && (0 != wcscmp(l_sFind_Info.cFileName, L".")) 
                     && (0 != wcscmp(l_sFind_Info.cFileName, L".."))
                   )
                {
                    CWString *l_pFile = new CWString();
                    if (l_pFile)
                    {
                        l_pFile->Set(i_pRoot);
                        l_pFile->Append(2, L"\\", l_sFind_Info.cFileName);
                        io_pFilesList->Add_After(io_pFilesList->Get_Last(), l_pFile);
                    }
                }

                if (! FindNextFileW(l_hFind, &l_sFind_Info))
                {
                    FindClose(l_hFind);
                    l_hFind = INVALID_HANDLE_VALUE;
                }
            } //while(INVALID_HANDLE_VALUE != hFind)
        }
        else
        {
            DWORD   l_dwDrives = GetLogicalDrives();
            DWORD   l_dwBit    = 0;
            tXCHAR  l_pDrive[4];

            l_pDrive[0] = TM('A');
            l_pDrive[1] = TM(':');
            l_pDrive[2] = TM('\\');
            l_pDrive[3] = 0;

            while (l_dwDrives)
            {
                if (l_dwDrives & 1)
                {
                    l_pDrive[0] = (tXCHAR)(l_dwBit + TM('A'));

                    UINT l_uiType = GetDriveTypeW(l_pDrive);
                    if (    (DRIVE_UNKNOWN != l_uiType)
                         && (DRIVE_NO_ROOT_DIR != l_uiType)
                       )
                    {
                        CWString *l_pFile = new CWString();
                        if (l_pFile)
                        {
                            l_pFile->Set(l_pDrive);
                            io_pFilesList->Add_After(io_pFilesList->Get_Last(), l_pFile);
                        }
                    }
                }

                l_dwDrives = l_dwDrives >> 1;
                l_dwBit ++;
            }
        }
    }// Enumerate_Dirs


    ////////////////////////////////////////////////////////////////////////////
    //Get_Version
    static UINT64 Get_Version(const wchar_t *i_pFile)
    {
        DWORD    l_dwUnknown      = 0;
        DWORD    l_dwFileInfoSize = 0;
        UINT64   l_qwReturn       = 0;
        CWString l_cPath;

        if (!i_pFile)
        {
            l_cPath.Realloc(4096);
            if (GetModuleFileNameW(GetModuleHandleW(NULL),
                                   l_cPath.Get(), 
                                   l_cPath.Max_Length()
                                  )
               )
            {
                i_pFile = l_cPath.Get();
            }
            else
            {
                return 0ull;
            }
        }

        l_dwFileInfoSize = GetFileVersionInfoSizeW(i_pFile, &l_dwUnknown);

        if (l_dwFileInfoSize)
        {
            BYTE * l_pFileInfo = new BYTE[l_dwFileInfoSize];
            if (l_pFileInfo)
            {
                memset(l_pFileInfo, 0, l_dwFileInfoSize);

                if (GetFileVersionInfoW(i_pFile, 0, l_dwFileInfoSize, l_pFileInfo) )
                {
                    VS_FIXEDFILEINFO * l_tVersion = NULL;
                    UINT l_dwSize = 0;
                    if (VerQueryValueW(l_pFileInfo, L"\\", (LPVOID *)&l_tVersion, &l_dwSize))
                    {
                        l_qwReturn = (((UINT64)l_tVersion->dwProductVersionMS) << 32) +
                                    l_tVersion->dwProductVersionLS;
                    }
                }

                if (l_pFileInfo)
                {
                    delete [ ] l_pFileInfo;
                    l_pFileInfo = NULL;
                }
            } //if (l_pFileInfo)
        } //if (l_dwFileInfoSize)

        return l_qwReturn;
    }//Get_Version


    ////////////////////////////////////////////////////////////////////////////
    //Get_TextResource
    static tBOOL Get_TextResource(const tXCHAR *i_pFile,
                                  const tXCHAR *i_pName,
                                  tXCHAR       *o_pBuffer,
                                  size_t        i_szBuffer
                                 )
    {
        tBOOL        l_bReturn        = FALSE;
        tUINT32      l_dwUnknown      = 0;
        DWORD        l_dwFileInfoSize = 0;
        tUINT32     *l_pCodePage      = NULL; 
        tUINT8      *l_pFileInfo      = NULL;
        tUINT32      l_dwSize         = 0;
        wchar_t     *l_pValue         = NULL;
        const size_t l_szName         = 256;
        wchar_t  l_pName[l_szName];

        CWString l_cPath;

        if (    (!i_pName)
             || (!o_pBuffer)
             || (32 > i_szBuffer)
             || ((l_szName / 2) < wcslen(i_pName))
           )
        {
            goto l_lblExit;
        }

        if (!i_pFile)
        {
            l_cPath.Realloc(4096);
            if (GetModuleFileNameW(GetModuleHandleW(NULL),
                                   l_cPath.Get(), 
                                   l_cPath.Max_Length()
                                  )
               )
            {
                i_pFile = l_cPath.Get();
            }
            else
            {
                goto l_lblExit;
            }
        }

        l_dwFileInfoSize = GetFileVersionInfoSizeW(i_pFile, (LPDWORD)&l_dwUnknown);

        if (!l_dwFileInfoSize)
        {
            goto l_lblExit;
        }

        l_pFileInfo = (tUINT8*)malloc(l_dwFileInfoSize);

        if (!l_pFileInfo)
        {
            goto l_lblExit;
        }

        memset(l_pFileInfo, 0, l_dwFileInfoSize);

        if (!GetFileVersionInfoW(i_pFile, 0, l_dwFileInfoSize, l_pFileInfo) )
        {
            goto l_lblExit;
        }

        if (!VerQueryValueW(l_pFileInfo, L"\\VarFileInfo\\Translation", (LPVOID*) &l_pCodePage, &l_dwSize))
        {
            goto l_lblExit;
        }

        swprintf_s(l_pName, l_szName,
                   L"\\StringFileInfo\\%08x\\%s", 
                   (tUINT32)((*l_pCodePage) << 16 | (*l_pCodePage) >> 16),
                   i_pName
                  );

        if (!VerQueryValueW(l_pFileInfo, l_pName, (LPVOID *)&l_pValue, &l_dwSize))
        {
            goto l_lblExit;
        }


        wcsncpy_s(o_pBuffer, i_szBuffer, l_pValue, (l_dwSize > i_szBuffer) ? i_szBuffer : l_dwSize);
        o_pBuffer[i_szBuffer-1] = 0;
        l_bReturn = TRUE;

    l_lblExit:
        if (l_pFileInfo)
        {
            free(l_pFileInfo);
            l_pFileInfo = NULL;
        }
        return l_bReturn;
    }//Get_TextResource


    ////////////////////////////////////////////////////////////////////////////
    //Get_Process_Directory
    static BOOL Get_Process_Directory(CWString *o_pDirectory)
    {
        BOOL  l_bReturn  = FALSE;
        DWORD l_dwLength = 4096;

        if (    (NULL == o_pDirectory)
             || (FALSE == o_pDirectory->Realloc(l_dwLength))
           )
        {
            return l_bReturn;
        }

        if (0 != GetModuleFileNameW(GetModuleHandleW(NULL), 
                                    o_pDirectory->Get(), 
                                    o_pDirectory->Max_Length())
           )
        {
            wchar_t *l_pExe_Name = NULL;

            if (    (l_pExe_Name = wcsrchr(o_pDirectory->Get(), L'\\'))
                 || (l_pExe_Name = wcsrchr(o_pDirectory->Get(), L'/'))
               )
            {
                o_pDirectory->Trim((DWORD)(l_pExe_Name - o_pDirectory->Get()));
                l_bReturn = TRUE;
            }
        }

        return l_bReturn;
    }//Get_Process_Directory


    ////////////////////////////////////////////////////////////////////////////
    //Get_Process_Path
    static BOOL Get_Process_Path(CWString *o_pPath)
    {
        BOOL  l_bReturn  = FALSE;
        DWORD l_dwLength = 4096;

        if (    (NULL == o_pPath)
             || (FALSE == o_pPath->Realloc(l_dwLength))
           )
        {
            return l_bReturn;
        }

        if (0 != GetModuleFileNameW(GetModuleHandleW(NULL), 
                                    o_pPath->Get(), 
                                    o_pPath->Max_Length())
           )
        {
            l_bReturn = TRUE;
        }

        return l_bReturn;
    }//Get_Process_Path


    ////////////////////////////////////////////////////////////////////////////
    //Get_File_Size
    static tUINT64 Get_File_Size(const wchar_t *i_pFile)
    {
        WIN32_FILE_ATTRIBUTE_DATA l_sFAD  = {0};
        LARGE_INTEGER             l_sSize = {0};

        if (!GetFileAttributesExW(i_pFile, GetFileExInfoStandard, &l_sFAD))
        {
            return 0; // error condition, could call GetLastError to find out more
        }

        l_sSize.HighPart = l_sFAD.nFileSizeHigh;
        l_sSize.LowPart = l_sFAD.nFileSizeLow;
        return l_sSize.QuadPart;
    }//Get_File_Size


    ////////////////////////////////////////////////////////////////////////////
    //Delete_File
    static BOOL Delete_File(const wchar_t *i_pFile_Name)
    {
        if (NULL == i_pFile_Name)
        {
            return FALSE;
        }

        DWORD l_dwFAttr = GetFileAttributesW(i_pFile_Name);
        l_dwFAttr = l_dwFAttr & (~FILE_ATTRIBUTE_ARCHIVE);
        l_dwFAttr = l_dwFAttr & (~FILE_ATTRIBUTE_READONLY);
        l_dwFAttr = l_dwFAttr & (~FILE_ATTRIBUTE_SYSTEM);
        SetFileAttributesW(i_pFile_Name, l_dwFAttr);
        return DeleteFileW(i_pFile_Name);
    }//Delete_File


    ////////////////////////////////////////////////////////////////////////////
    //GetUserDirectory
    static tBOOL GetUserDirectory(CWString *o_pPath)
    {
        if (    (NULL == o_pPath)
             || (FALSE == o_pPath->Realloc(4096))
           )
        {
            return TRUE;
        }

        if ( S_OK != SHGetFolderPathW(NULL, CSIDL_PERSONAL | CSIDL_FLAG_CREATE, NULL, 0, o_pPath->Get()))
        {
            return FALSE;
        }

        return TRUE;
    }

    ////////////////////////////////////////////////////////////////////////////
    //GetUserDirectory
    static tBOOL GetDirectoryParent(CWString &i_rChild, CWString &o_rParent)
    {
        o_rParent.Set(i_rChild.Get());
        size_t  l_szDir = o_rParent.Length();
        tXCHAR *l_pDir  = o_rParent.Get();

        //cut trailing '/' or '\'
        while (    (l_szDir)
                && (    (TM('\\') == l_pDir[l_szDir - 1])
                     || (TM('/') == l_pDir[l_szDir - 1])
                   )
              )
        {
            l_pDir[--l_szDir] = 0;
        }

        tXCHAR *l_pTemp = wcsrchr(l_pDir, TM('\\'));
        if (!l_pTemp)
        {
            l_pTemp = wcsrchr(l_pDir, TM('/'));
        }

        if (!l_pTemp)
        {
            o_rParent.Set(TM(""));
            return TRUE;
        }

        *l_pTemp = 0;
        l_pTemp--;
        //removing duplicates of '\' '/'
        while (    (TM('\\') == *l_pTemp)
                || (TM('/') == *l_pTemp)
              )
        {
            *l_pTemp = 0;
            l_pTemp--;

            if (l_pDir > l_pTemp)
            {
                break;
            }
        }

        return TRUE;
    }

};


