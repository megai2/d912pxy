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
////////////////////////////////////////////////////////////////////////////////
// This header file provide printing to text file                              /
////////////////////////////////////////////////////////////////////////////////

#ifndef CLTEXTFILE_H
#define CLTEXTFILE_H

#include "IFile.h"
#include "PFile.h"

#define TXT_FILE_DEF_FILES_COUNT  0
#define TXT_FILE_EXT              TM("txt")
#define TXT_FILE_WRITE_PERIOD     1000u

////////////////////////////////////////////////////////////////////////////////
class CClTextFile
    : public CClTextSink
{
    enum eRolling
    {
        EROLLING_NONE,
        EROLLING_MEGABYTES,
        EROLLING_HOURS,
        EROLLING_TIME,

        EROLLING_MAX
    };

    CPFile             m_cFile;
    CWString           m_pDir;
    eRolling           m_eRolling;
    tUINT64            m_qwRolling_Value;
    tUINT32            m_dwFile_Tick;
    tUINT32            m_dwWrite_Tick;
    tUINT64            m_qwFile_Size;
    tUINT32            m_dwFiles_Max_Count;
    tUINT64            m_qwFiles_Max_Size;
    CBList<CWString*>  m_cFiles;
    tXCHAR            *m_pBuffer;
    size_t             m_szBuffer;
    size_t             m_szBufferOffs;
    CUintList          m_cSecondsList;
    tUINT32            m_uiSecond;
    tUINT32            m_dwIndex;


public:
    ////////////////////////////////////////////////////////////////////////////
    //CClTextFile
    CClTextFile()
        : m_eRolling(EROLLING_NONE)
        , m_qwRolling_Value(0ull)
        , m_dwFile_Tick(0u)
        , m_dwWrite_Tick(0ull)
        , m_qwFile_Size(0ull)
        , m_dwFiles_Max_Count(0u)
        , m_qwFiles_Max_Size(0u)
        , m_pBuffer(NULL)
        , m_szBuffer(0x10000)
        , m_szBufferOffs(0)
        , m_dwIndex(0)
    {
        m_dwWrite_Tick = GetTickCount();
        m_uiSecond     = GetSecondOfDay(); 
    }

    ////////////////////////////////////////////////////////////////////////////
    //~CClTextFile
    virtual ~CClTextFile()
    {
        m_cFile.Write((tUINT8*)m_pBuffer, m_szBufferOffs * sizeof(tXCHAR), FALSE);
        m_cFile.Close(TRUE);

        m_cFiles.Clear(TRUE);
        if (m_pBuffer)
        {
            free(m_pBuffer);
            m_pBuffer = NULL;
        }
        m_szBufferOffs = 0;

        m_cSecondsList.Clear(TRUE);
    }

    ////////////////////////////////////////////////////////////////////////////
    //Initialize
    virtual eClient_Status Initialize(tXCHAR **i_pArgs, tINT32 i_iCount)
    {
        eClient_Status  l_eReturn  = ECLIENT_STATUS_OK;
        tXCHAR         *l_pArgV    = NULL;
        pAList_Cell     l_pStart   = NULL;
        tUINT32         l_dwDirLen = 0;

        m_pDir.Realloc(4096);

        m_pBuffer = (tXCHAR*)malloc(m_szBuffer * sizeof(tXCHAR));
        if (!m_pBuffer)
        {
            PRINTF(TM("P7:Can't allocate memory\n"));
            l_eReturn = ECLIENT_STATUS_INTERNAL_ERROR;
            goto l_lblExit;
        }

        //unicode marker
    #ifdef UTF8_ENCODING
        m_pBuffer[0]   = (char)0xEF;
        m_pBuffer[1]   = (char)0xBB;
        m_pBuffer[2]   = (char)0xBF;
        m_szBufferOffs = 3;
        m_qwFile_Size  = 3;
    #else
        m_pBuffer[0]   = 0xFEFF;
        m_szBufferOffs = 1;
        m_qwFile_Size  = sizeof(tXCHAR);
    #endif                             

        ////////////////////////////////////////////////////////////////////////////
        //get maximum count of the log files
        l_pArgV = Get_Argument_Text_Value(i_pArgs, i_iCount,
                                          (tXCHAR*)CLIENT_COMMAND_LINE_FILES_COUNT_MAX
                                         );
        if (l_pArgV)
        {
            m_dwFiles_Max_Count = (tUINT32)PStrToInt(l_pArgV);

            if (4096 < m_dwFiles_Max_Count)
            {
                m_dwFiles_Max_Count = TXT_FILE_DEF_FILES_COUNT;
            }
        }


        l_pArgV = Get_Argument_Text_Value(i_pArgs, i_iCount,
                                          (tXCHAR*)CLIENT_COMMAND_LINE_FILES_SIZE_MAX
                                         );
        if (l_pArgV)
        {
            m_qwFiles_Max_Size = (tUINT64)PStrToInt(l_pArgV) * 1048576ull;
        }


        ////////////////////////////////////////////////////////////////////////////
        //get path to storage
        l_pArgV = Get_Argument_Text_Value(i_pArgs, i_iCount,
                                          (tXCHAR*)CLIENT_COMMAND_LINE_DIR
                                         );
        if (l_pArgV)
        {
            m_pDir.Set(l_pArgV);
        }
        else
        {
            CProc::Get_Process_Path(m_pDir.Get(), m_pDir.Max_Length());
            m_pDir.Append(1, TM("/P7logs/"));
        }

        if (FALSE == CFSYS::Directory_Exists(m_pDir.Get()))
        {
            if (FALSE == CFSYS::Directory_Create(m_pDir.Get()))
            {
                PRINTF(TM("P7:Can't create directory: %s\n"), l_pArgV);
                l_eReturn = ECLIENT_STATUS_NOT_ALLOWED;
                goto l_lblExit;
            }
        }

        ////////////////////////////////////////////////////////////////////////////
        //enumerate files & sorting ....
        if (    (TXT_FILE_DEF_FILES_COUNT != m_dwFiles_Max_Count)
             || (m_qwFiles_Max_Size)
           )
        {
            CFSYS::Enumerate_Files(&m_cFiles, &m_pDir, TM("*.") TXT_FILE_EXT, 0);

            l_pStart   = NULL;
            l_dwDirLen = m_pDir.Length();

            while ((l_pStart = m_cFiles.Get_Next(l_pStart)))
            {
                pAList_Cell l_pMin  = l_pStart;
                pAList_Cell l_pIter = l_pStart;

                while ((l_pIter = m_cFiles.Get_Next(l_pIter)))
                {
                    CWString *l_pPathM = m_cFiles.Get_Data(l_pMin);
                    CWString *l_pPathI = m_cFiles.Get_Data(l_pIter);
                    tXCHAR   *l_pNameM = l_pPathM->Get() + l_dwDirLen;
                    tXCHAR   *l_pNameI = l_pPathI->Get() + l_dwDirLen;

                    if (0 < PStrICmp(l_pNameM, l_pNameI))
                    {
                        l_pMin = l_pIter;
                    }
                }

                if (l_pMin != l_pStart)
                {
                    m_cFiles.Extract(l_pMin);
                    m_cFiles.Put_After(m_cFiles.Get_Prev(l_pStart), l_pMin);
                    l_pStart = l_pMin;
                }
            } //while (l_pStart = m_cFiles.Get_Next(l_pStart))
        }

        ////////////////////////////////////////////////////////////////////////////
        //create file
        l_eReturn = Create_File();
        if (ECLIENT_STATUS_OK != l_eReturn)
        {
            PRINTF(TM("P7:File creation failed"));
            goto l_lblExit;
        }


        ////////////////////////////////////////////////////////////////////////////
        //get rolling type
        l_pArgV = Get_Argument_Text_Value(i_pArgs, i_iCount,
                                          (tXCHAR*)CLIENT_COMMAND_LINE_FILE_ROLLING
                                         );
        if (l_pArgV)
        {
            CWString l_cRolling(l_pArgV);
            tUINT32  l_dwLen  = l_cRolling.Length();
            tBOOL    l_bError = FALSE;

            if (3 <= l_dwLen)
            {
                tXCHAR *l_pSuffix = l_cRolling.Get() + l_dwLen - 2;
                if (0 == PStrICmp(l_pSuffix, TM("hr")))
                {
                    m_eRolling = EROLLING_HOURS;
                }
                else if (0 == PStrICmp(l_pSuffix, TM("mb")))
                {
                    m_eRolling = EROLLING_MEGABYTES;
                }
                else if (0 == PStrICmp(l_pSuffix, TM("tm")))
                {
                    m_eRolling = EROLLING_TIME;
                }
                else
                {
                    l_bError = TRUE;
                }

                //cut off suffix
                l_cRolling.Trim(l_dwLen - 2);
            }
            else
            {
                l_bError = TRUE;
            }

            if (FALSE == l_bError)
            {
                if (m_eRolling != EROLLING_TIME)
                {
                    m_qwRolling_Value = PStrToInt(l_cRolling.Get());

                    if (0 >= m_qwRolling_Value)
                    {
                        m_eRolling = EROLLING_NONE;
                        l_bError   = TRUE;
                    }
                }
                else
                {
                    if (!Parse_Rolling_Time(l_cRolling.Get()))
                    {
                        m_eRolling = EROLLING_NONE;
                        l_bError   = TRUE;
                    }
                }
            }

            if (l_bError)
            {
                PRINTF(TM("P7:Rolling value is not correct = %s\n"), l_cRolling.Get());
            }

            if (EROLLING_MEGABYTES == m_eRolling)
            {
                m_qwRolling_Value *= 0x100000ULL; //megabytes
            }
            else if (EROLLING_HOURS == m_eRolling)
            {
                if (m_qwRolling_Value > 1000)
                {
                    PRINTF(TM("P7:Rolling value is more than 1000 hours, cutting\n"));
                    m_qwRolling_Value = 1000;
                }

                m_qwRolling_Value *= 3600000ULL; //milliseconds in one hour
            }
        }//if (l_pArgV)

    l_lblExit:
        return l_eReturn;
    }


    ////////////////////////////////////////////////////////////////////////////////
    //Parse_Rolling_Time
    tBOOL Parse_Rolling_Time(const tXCHAR *i_pTime)
    {
        const tXCHAR *l_pIter   = i_pTime;
        const tXCHAR *l_pBegin  = i_pTime;
        tBOOL         l_bReturn = TRUE;

        while (*l_pIter)
        {
            tUINT32 l_uiSeconds = 0;
            tUINT32 l_uiValue   = 0;

            l_pBegin = l_pIter; //scan hours
            while (    (*l_pIter >= TM('0')) 
                    && (*l_pIter <= TM('9'))
                    )
            {
                l_uiValue = l_uiValue * 10 + (*l_pIter - TM('0'));
                l_pIter ++;
            }

            if (l_pBegin == l_pIter) //no digits
            {
                l_bReturn = FALSE;
                break;
            }

            l_uiSeconds = l_uiValue * 3600; 
                    
            if (*l_pIter != TM(':')) //error
            {
                l_bReturn = FALSE;
                break;
            }

            l_pIter++;

            l_pBegin  = l_pIter; //scan minutes
            l_uiValue = 0;
            while (    (*l_pIter >= TM('0'))
                    && (*l_pIter <= TM('9'))
                    )
            {
                l_uiValue = l_uiValue * 10 + (*l_pIter - TM('0'));
                l_pIter ++;
            }

            if (l_pBegin == l_pIter) //no digits
            {
                l_bReturn = FALSE;
                break;
            }

            l_uiSeconds += l_uiValue * 60; 

            m_cSecondsList.Add_After(m_cSecondsList.Get_Last(), l_uiSeconds);

            if (*l_pIter == TM(','))
            {
                l_pIter++;
            }
        }

        if (!m_cSecondsList.Count())
        {
            l_bReturn = FALSE;
        }

        return l_bReturn;
    }//Parse_Rolling_Time


    ////////////////////////////////////////////////////////////////////////////
    //Log
    virtual eClient_Status Log(const CClTextSink::sLog &i_rRawLog, 
                               const tXCHAR            *i_pFmtLog, 
                               size_t                   i_szFmtLog
                              )
    {
        UNUSED_ARG(i_rRawLog);
        if (    ((i_szFmtLog + 16) > (m_szBuffer - m_szBufferOffs))
             || (CTicks::Difference(GetTickCount(), m_dwWrite_Tick) >= TXT_FILE_WRITE_PERIOD)
           )
        {
            if ((i_szFmtLog + 16) > m_szBuffer)
            {
                if (m_szBufferOffs)
                {
                    m_cFile.Write((tUINT8*)m_pBuffer, m_szBufferOffs * sizeof(tXCHAR), FALSE);
                    m_szBufferOffs = 0u;
                }

                m_cFile.Write((tUINT8*)i_pFmtLog, i_szFmtLog * sizeof(tXCHAR), FALSE);
                m_qwFile_Size += (i_szFmtLog + 2) * sizeof(tXCHAR);
                *(m_pBuffer)     = TM('\r');
                *(m_pBuffer + 1) = TM('\n');
                m_szBufferOffs  += 2;

                TryRoll();

                return ECLIENT_STATUS_OK;
            }
            else
            {
                m_cFile.Write((tUINT8*)m_pBuffer, m_szBufferOffs * sizeof(tXCHAR), FALSE);
                m_szBufferOffs = 0;

                TryRoll();
            }

            m_dwWrite_Tick = GetTickCount();
        }

        memcpy(m_pBuffer + m_szBufferOffs, i_pFmtLog, sizeof(tXCHAR) * i_szFmtLog);
        m_szBufferOffs += i_szFmtLog;
        *(m_pBuffer + m_szBufferOffs)     = TM('\r');
        *(m_pBuffer + m_szBufferOffs + 1) = TM('\n');
        m_szBufferOffs += 2;
        m_qwFile_Size  += (i_szFmtLog + 2) * sizeof(tXCHAR);

        return ECLIENT_STATUS_OK;
    }

    ////////////////////////////////////////////////////////////////////////////
    // TryRoll                                      
    eClient_Status TryRoll()
    {
        if (   (    (EROLLING_HOURS == m_eRolling)
                 && (CTicks::Difference(GetTickCount(), m_dwFile_Tick) > m_qwRolling_Value)
               )
            || (    (EROLLING_MEGABYTES == m_eRolling)
                 && (m_qwRolling_Value <= m_qwFile_Size)
               )
           )
        {
            Roll();
        }
        else if (EROLLING_TIME == m_eRolling)
        {
            tUINT32     l_uiNextSecond = GetSecondOfDay(); 
            pAList_Cell l_pEl          = NULL;

            while ((l_pEl = m_cSecondsList.Get_Next(l_pEl)))
            {
                tUINT32 l_uiIem = m_cSecondsList.Get_Data(l_pEl);
                if (    (m_uiSecond <= l_uiIem)
                     && (l_uiNextSecond > l_uiIem)
                   )
                {
                    Roll();
                    break;
                }
            }

            m_uiSecond = l_uiNextSecond;
        }

        return ECLIENT_STATUS_OK; 
    }// TryRoll


    ////////////////////////////////////////////////////////////////////////////
    // Roll                                      
    void Roll()
    {
        m_cFile.Write((tUINT8*)m_pBuffer, m_szBufferOffs * sizeof(tXCHAR), TRUE);
        m_cFile.Close(TRUE);
        m_qwFile_Size  = 0ull;
        m_szBufferOffs = 0u;
        Create_File();
        m_dwWrite_Tick = GetTickCount();
    }// Roll

    ////////////////////////////////////////////////////////////////////////////
    // Create_File                                      
    eClient_Status Create_File()
    {
        eClient_Status l_eReturn = ECLIENT_STATUS_OK;
        tUINT32        l_dwYear  = 0;
        tUINT32        l_dwMonth = 0;
        tUINT32        l_dwDay   = 0; 
        tUINT32        l_dwHr    = 0; 
        tUINT32        l_dwMin   = 0;
        tUINT32        l_dwSec   = 0; 
        tUINT32        l_dwmSec  = 0;
        CWString       l_cFilePath;
        tXCHAR         l_pFile_Name[64];

        m_cFile.Close(TRUE);

        CSys::Get_DateTime(l_dwYear, 
                           l_dwMonth, 
                           l_dwDay, 
                           l_dwHr, 
                           l_dwMin, 
                           l_dwSec, 
                           l_dwmSec
                          );
        l_dwmSec = 0;
        do
        {
            PSPrint(l_pFile_Name, 
                    LENGTH(l_pFile_Name), 
                    TM("/%04d%02d%02d-%02d%02d%02d%03d.") TXT_FILE_EXT,
                    l_dwYear, 
                    l_dwMonth,
                    l_dwDay,
                    l_dwHr,
                    l_dwMin,
                    l_dwSec,
                    m_dwIndex
                   );

            l_cFilePath.Set(m_pDir.Get());
            l_cFilePath.Append(1, l_pFile_Name);

            m_dwIndex ++;
        } while (TRUE == CFSYS::File_Exists(l_cFilePath.Get()));

        if (FALSE == m_cFile.Open(l_cFilePath.Get(), 
                                  IFile::ECREATE | IFile::EACCESS_WRITE | IFile::ESHARE_READ
                                 )
           )
        {
            PRINTF(TM("P7:Can't create file: %s"), l_cFilePath.Get());
            l_eReturn = ECLIENT_STATUS_INTERNAL_ERROR;
        }
        else 
        {
            m_cFiles.Add_After(m_cFiles.Get_Last(), new CWString(l_cFilePath.Get()));

            if (TXT_FILE_DEF_FILES_COUNT != m_dwFiles_Max_Count)
            {
                while (m_dwFiles_Max_Count < m_cFiles.Count())
                {
                    pAList_Cell l_pEl   = m_cFiles.Get_First();
                    CWString   *l_pPath = m_cFiles.Get_Data(l_pEl);
                    if (l_pPath)
                    {
                        if (!CFSYS::Delete_File(l_pPath->Get()))
                        {
                            PRINTF(TM("P7:Can't delete file: %s"), l_pPath->Get());
                        }
                    }

                    m_cFiles.Del(l_pEl, TRUE);
                }
            }

            if (0 != m_qwFiles_Max_Size)
            {
                pAList_Cell l_pEl = NULL;
                tUINT64     l_qwTotalSize = 0;
                tUINT64     l_qwFileSize = 0;

                while ((l_pEl = m_cFiles.Get_Next(l_pEl)))
                {
                    CWString *l_pPath = m_cFiles.Get_Data(l_pEl);
                    if (l_pPath)
                    {
                        l_qwTotalSize += CFSYS::Get_File_Size(l_pPath->Get());
                    }
                }

                while (l_qwTotalSize > (m_qwFiles_Max_Size))
                {
                    pAList_Cell l_pEl   = m_cFiles.Get_First();
                    CWString   *l_pPath = m_cFiles.Get_Data(l_pEl);
                    if (l_pPath)
                    {
                        l_qwFileSize = CFSYS::Get_File_Size(l_pPath->Get());

                        if (l_qwFileSize < l_qwTotalSize)
                        {
                            l_qwTotalSize -= l_qwFileSize;
                        }
                        else
                        {
                            l_qwTotalSize = 0ull;
                        }

                        if (!CFSYS::Delete_File(l_pPath->Get()))
                        {
                            PRINTF(TM("Can't delete file: %s"), l_pPath->Get());
                        }
                    }

                    m_cFiles.Del(l_pEl, TRUE);
                }
            }
        }

        m_dwFile_Tick = GetTickCount();

        return l_eReturn;
    }// Create_File

    virtual eClient_Status DumpBuffers()
    {
        if (m_szBufferOffs)
        {
            m_cFile.Write((tUINT8*)m_pBuffer, m_szBufferOffs * sizeof(tXCHAR), TRUE);
            m_szBufferOffs = 0;
        }

        return ECLIENT_STATUS_OK;
    }

};

#endif //CLTEXTFILE_H
