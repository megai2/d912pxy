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
#ifndef PFILESYSTEM_H
#define PFILESYSTEM_H

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <fnmatch.h>
#include <pwd.h>

class CFSYS
{
public:
    ////////////////////////////////////////////////////////////////////////////
    //Directory_Exists
    static tBOOL Directory_Exists(const tACHAR *i_pPath)
    {
        struct stat l_sStat;
        l_sStat.st_mode = 0;
        if (0 == stat(i_pPath, &l_sStat))
        {
            if (0 != (l_sStat.st_mode & S_IFDIR))
            {
                return TRUE;
            }
        }

        return FALSE;
    }//Directory_Exists


    ////////////////////////////////////////////////////////////////////////////
    //Directory_Create
    static tBOOL Directory_Create(const tACHAR *i_pDir)
    {
        if (NULL == i_pDir)
        {
            return FALSE;
        }

        char *l_pDir  = (char *)malloc(strlen(i_pDir) + 1);
        char *l_pIter = l_pDir;

        if (NULL == l_pDir)
        {
            return FALSE;
        }

        strcpy(l_pDir, i_pDir);
        while (*l_pIter)
        {
            if ('/' == (*l_pIter))
            {
                *l_pIter = 0;

                if (    (FALSE == CFSYS::Directory_Exists(l_pDir))
                     && (l_pIter != l_pDir)
                   )
                {
                    mkdir(l_pDir, S_IRWXU | S_IRWXG | S_IROTH);
                }

                *l_pIter = '/';
            }

            l_pIter++;
        }

        free(l_pDir);
        l_pDir = NULL;

        mkdir(i_pDir, S_IRWXU | S_IRWXG | S_IROTH);

        return Directory_Exists((tACHAR*)i_pDir);
    }//Directory_Create


    ////////////////////////////////////////////////////////////////////////////
    //Directory_Delete
    static tBOOL Directory_Delete(const tACHAR *i_pDir)
    {
        if (NULL == i_pDir)
        {
            return FALSE;
        }

        return (0 == rmdir(i_pDir));
    }//Directory_Delete

    ////////////////////////////////////////////////////////////////////////////
    //File_Exists
    static tBOOL File_Exists(const tACHAR * i_pFileName)
    {
        struct stat l_sStat;
        l_sStat.st_mode = 0;
        if (0 == stat(i_pFileName, &l_sStat))
        {
            if (0 != (l_sStat.st_mode & S_IFREG))
            {
                return TRUE;
            }
        }

        return FALSE;
    }//File_Exists


    ////////////////////////////////////////////////////////////////////////////
    // Enumerate_Files
    static void Enumerate_Files(CBList<CWString*> *i_pDll_List,
                                CWString          *i_pDirectory,
                                const char        *i_pMask, //for example L"*.so"
                                tUINT32            i_dwDepth = 0xFFFFFFul
                               )
    {
        tUINT32 l_dwLength   = i_pDirectory->Length();

        if (    (NULL == i_pDll_List)
             || (NULL == i_pDirectory)
             || (NULL == i_pMask)
           )
        {
            return;
        }

        DIR    *l_pDir       = opendir(i_pDirectory->Get());
        dirent *l_pDir_Entry = 0;

        if (!l_pDir)
        {
            return;
        }

        while ((l_pDir_Entry = readdir(l_pDir)) != 0)
        {
            if (DT_REG == l_pDir_Entry->d_type)
            {
                if (0 == fnmatch(i_pMask, l_pDir_Entry->d_name, FNM_NOESCAPE | FNM_CASEFOLD))
                {
                    CWString *i_pFile = new CWString();
                    if (i_pFile)
                    {
                        i_pFile->Set(i_pDirectory->Get());
                        i_pFile->Append(2, L"/", l_pDir_Entry->d_name);
                        i_pDll_List->Add_After(i_pDll_List->Get_Last(), i_pFile);
                    }
                }
            }
            else if (    (DT_DIR == l_pDir_Entry->d_type)
                      && (0 != PStrCmp(l_pDir_Entry->d_name, "."))
                      && (0 != PStrCmp(l_pDir_Entry->d_name, ".."))
                      && (i_dwDepth)
                    )
            {
                i_pDirectory->Append(2, L"/", l_pDir_Entry->d_name);
                Enumerate_Files(i_pDll_List, i_pDirectory, i_pMask, i_dwDepth - 1);
                i_pDirectory->Trim(l_dwLength);
            }
        }

        closedir(l_pDir);
        l_pDir = 0;

    }// Enumerate_Files

	
    ////////////////////////////////////////////////////////////////////////////
    // Enumerate_Dirs
    static void Enumerate_Dirs(CBList<CWString*> *io_pList, const tXCHAR *i_pRoot)
    {
        if (NULL == io_pList)
        {
            return;
        }

		if (!i_pRoot)
		{
		    i_pRoot = "/";
		}
		
        DIR    *l_pDir       = opendir(i_pRoot);
        dirent *l_pDir_Entry = 0;

        if (!l_pDir)
        {
            return;
        }

        while ((l_pDir_Entry = readdir(l_pDir)) != 0)
        {
            if (    (DT_DIR == l_pDir_Entry->d_type)
                 && (0 != PStrCmp(l_pDir_Entry->d_name, "."))
                 && (0 != PStrCmp(l_pDir_Entry->d_name, ".."))
               )
            {
                CWString *i_pFile = new CWString();
                if (i_pFile)
                {
                    i_pFile->Set(i_pRoot);
                    i_pFile->Append(2, "/", l_pDir_Entry->d_name);
                    io_pList->Add_After(io_pList->Get_Last(), i_pFile);
                }
            }
        }

        closedir(l_pDir);
        l_pDir = 0;
    }// Enumerate_Dirs
	
    ////////////////////////////////////////////////////////////////////////////
    //Get_File_Size
    static tUINT64 Get_File_Size(const tACHAR *i_pFile)
    {
        struct stat l_sStat;
        l_sStat.st_size = 0;
        if (0 == stat(i_pFile, &l_sStat))
        {
            return (tUINT64)l_sStat.st_size;
        }

        return 0;
    }//Get_File_Size

    ////////////////////////////////////////////////////////////////////////////
    //Get_Version
    static tUINT64 Get_Version(const char *i_pFile)
    {
        UNUSED_ARG(i_pFile);
        return 0ull;
    }//Get_Version


//    ////////////////////////////////////////////////////////////////////////////
//    //Get_Process_Directory
//    static BOOL Get_Process_Directory(CWString *o_pDirectory)
//    {
//        BOOL  l_bReturn  = FALSE;
//        DWORD l_dwLength = 4096;

//        if (    (NULL == o_pDirectory)
//             || (FALSE == o_pDirectory->Realloc(l_dwLength))
//           )
//        {
//            return l_bReturn;
//        }

//        if (0 != GetModuleFileNameW(GetModuleHandleW(NULL),
//                                    o_pDirectory->Get(),
//                                    o_pDirectory->Max_Length())
//           )
//        {
//            wchar_t *l_pExe_Name = NULL;

//            if (    (l_pExe_Name = wcsrchr(o_pDirectory->Get(), L'\\'))
//                 || (l_pExe_Name = wcsrchr(o_pDirectory->Get(), L'/'))
//               )
//            {
//                o_pDirectory->Trim((DWORD)(l_pExe_Name - o_pDirectory->Get()));
//                l_bReturn = TRUE;
//            }
//        }

//        return l_bReturn;
//    }//Get_Process_Directory

    ////////////////////////////////////////////////////////////////////////////
    //Delete_File
    static tBOOL Delete_File(const char *i_pFile_Name)
    {
        if (NULL == i_pFile_Name)
        {
            return FALSE;
        }

        return (0 == remove(i_pFile_Name));
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

        struct passwd *pw = getpwuid(getuid());

        if (!pw)
        {
            return FALSE;
        }

        o_pPath->Set(pw->pw_dir);
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
        while (    (l_szDir > 1)
                && (TM('/') == l_pDir[l_szDir - 1])
              )
        {
            l_pDir[--l_szDir] = 0;
        }

        tXCHAR *l_pTemp = strrchr(l_pDir, TM('/'));

        if (!l_pTemp)
        {
            o_rParent.Set(TM(""));
            return TRUE;
        }

        l_pTemp++;
        *l_pTemp = 0;
        l_pTemp--;

        //removing duplicates of '/'
        while (    (TM('/') == *l_pTemp)
                && (l_pTemp > l_pDir)
               )
        {
            *l_pTemp = 0;
            l_pTemp--;
        }

        return (*l_pDir) ? TRUE : FALSE;
    }
	
};


#endif //PFILESYSTEM_H
