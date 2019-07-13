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
#ifndef PFILE_H
#define PFILE_H

#define PFILE_INVALID_HANDLE                                                (-1)

#ifndef _LARGEFILE64_SOURCE
    #define _LARGEFILE64_SOURCE
#endif
//Headers have to be included in application source files
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define GetLastError() errno

//UTF-8 text file header - {0xEF, 0xBB, 0xBF}

////////////////////////////////////////////////////////////////////////////////
class CPFile
    : public IFile //virtual
{
    volatile tINT32    m_iRCnt;
    //tLOCK              m_hCS;
    int                m_iFile;
public:
    ////////////////////////////////////////////////////////////////////////////
    CPFile()
        : m_iRCnt(1)
        , m_iFile(-1)
    {
        //LOCK_CREATE(m_hCS);
    }

    ////////////////////////////////////////////////////////////////////////////
    virtual ~CPFile()
    {
        //LOCK_DESTROY(m_hCS);
        Close(FALSE);
    }

    ////////////////////////////////////////////////////////////////////////////
    tBOOL IsOpened()
    {
        return (PFILE_INVALID_HANDLE != m_iFile);
    }

    ////////////////////////////////////////////////////////////////////////////
    tBOOL Open(const tXCHAR *i_pName, tUINT32 i_dwFlags)
    {
        //create file if it is not exists, else - open, heh ... engineers
        //friendly POSIX API :-(
        int    l_iFlags = O_CREAT;
        mode_t l_tMode  = 0;

        if (PFILE_INVALID_HANDLE != m_iFile)
        {
            Close(TRUE);
        }

        l_iFlags |= O_RDWR;

        if (ECREATE & i_dwFlags)
        {
            l_iFlags |= O_TRUNC;

            if (EACCESS_WRITE & i_dwFlags)
            {
                l_tMode = S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP | S_IWOTH;
            }
            else if (EACCESS_READ & i_dwFlags)
            {
                l_tMode = S_IRUSR | S_IRGRP | S_IROTH;
            }
        }

        m_iFile = open(i_pName, l_iFlags, l_tMode);

        return (PFILE_INVALID_HANDLE != m_iFile) ? TRUE : FALSE;
    }

    ////////////////////////////////////////////////////////////////////////////
    tBOOL Close(tBOOL i_bFlush)
    {
        tBOOL l_bReturn = TRUE;
        int   l_iTemp   = PFILE_INVALID_HANDLE;

        if (PFILE_INVALID_HANDLE == m_iFile)
        {
            return TRUE;
        }

        if (i_bFlush)
        {
            if (-1 == fsync(m_iFile))
            {
                l_bReturn = FALSE;
                goto l_lExit;
            }
        }

        l_iTemp = m_iFile;
        m_iFile = PFILE_INVALID_HANDLE;

        if (-1 == close(l_iTemp))
        {
            l_bReturn = FALSE;
            goto l_lExit;
        }

    l_lExit:
        return l_bReturn;
    }

    ////////////////////////////////////////////////////////////////////////////
    tBOOL Set_Position(tUINT64 i_qwOffset)
    {
        if (PFILE_INVALID_HANDLE == m_iFile)
        {
            return FALSE;
        }


        if ((off64_t)i_qwOffset != lseek64(m_iFile, (off64_t)i_qwOffset, SEEK_SET))
        {
            return FALSE;
        }

        return TRUE;
    }

    ////////////////////////////////////////////////////////////////////////////
    tUINT64 Get_Position()
    {
        if (PFILE_INVALID_HANDLE == m_iFile)
        {
            return 0ULL;
        }

        return (off64_t)lseek64(m_iFile, 0LL, SEEK_CUR);
    }

    ////////////////////////////////////////////////////////////////////////////
    tUINT64 Get_Size()
    {
        if (PFILE_INVALID_HANDLE == m_iFile)
        {
            return 0ULL;
        }

        off64_t l_llPos = lseek64(m_iFile, 0LL, SEEK_CUR);
        off64_t l_llSize = lseek64(m_iFile, 0LL, SEEK_END);
        lseek64(m_iFile, l_llPos, SEEK_SET);
        return l_llSize;
    }

    ////////////////////////////////////////////////////////////////////////////
    size_t Write(const tUINT8 *i_pBuffer, size_t i_szBuffer, tBOOL i_bFlush)
    {
        if (    (PFILE_INVALID_HANDLE == m_iFile)
             || (NULL == i_pBuffer)
             || (0 == i_szBuffer)
           )
        {
            return 0ULL;
        }

        ssize_t l_szRes     = 0;
        size_t  l_szWritten = 0;
        int     l_iError    = 0;

        while (l_szWritten < i_szBuffer)
        {
            l_szRes = write(m_iFile, i_pBuffer + l_szWritten, i_szBuffer - l_szWritten);
            if (0 < l_szRes)
            {
                l_szWritten += (size_t)l_szRes;
            }
            else
            {
                l_iError = errno;
                printf("%d", l_iError);
                break;
            }
        }

        if (    (i_bFlush)
             && (l_szWritten)
           )
        {
            fsync(m_iFile);
        }

        return l_szWritten;
    }

    ////////////////////////////////////////////////////////////////////////////
    size_t Read(tUINT8 *o_pBuffer, size_t i_szBuffer)
    {
        if (    (PFILE_INVALID_HANDLE == m_iFile)
             || (NULL == o_pBuffer)
             || (0 == i_szBuffer)
           )
        {
            return 0ULL;
        }

        size_t l_szRead = 0;
        ssize_t l_szRes = 0;

        while (l_szRead < i_szBuffer)
        {
            l_szRes = read(m_iFile, o_pBuffer + l_szRead, i_szBuffer - l_szRead);
            if (0 < l_szRes)
            {
                l_szRead += (size_t)l_szRes;
            }
            else
            {
                break;
            }
        }

        return l_szRead;
    }


    ////////////////////////////////////////////////////////////////////////////
    tINT32  Add_Ref()
    {
        return ATOMIC_INC(&m_iRCnt);
    }


    ////////////////////////////////////////////////////////////////////////////
    tINT32  Release()
    {
        volatile tINT32 l_iReturn = ATOMIC_DEC(&m_iRCnt);

        if (0 >= l_iReturn)
        {
            delete this;
        }

        return l_iReturn;
    }
};

#endif //PFILE_H



/*
    CPFile       l_cFile;
    const tUINT8 l_pHeader[] = {0xEF, 0xBB, 0xBF};
    const char   l_pText[]   = "Some Text For me !\xD\xA Another Text\xD\xA Third Text";
    tUINT64      l_qwCount   = 0;
    tUINT8       l_pBuffer[256] = {0};

    l_cFile.Open("./ABC.txt", IFile::ECREATE | IFile::ESHARE_WRITE);

    l_qwCount = l_cFile.Write(l_pHeader, LENGTH(l_pHeader), FALSE);
    l_qwCount = l_cFile.Write((tUINT8*)l_pText, LENGTH(l_pText) - 1, TRUE);

    l_qwCount = l_cFile.Get_Size();

    l_cFile.Set_Position(0);

    l_qwCount = l_cFile.Read(l_pBuffer, LENGTH(l_pBuffer));

    l_cFile.Close(TRUE);
*/
