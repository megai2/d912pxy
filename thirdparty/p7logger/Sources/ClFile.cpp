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
// This header file provide functionality to deliver data to file              /
////////////////////////////////////////////////////////////////////////////////

#include "CommonClient.h"
#include "ClFile.h"

#define  THREAD_IDLE_TIMEOUT                                                (10)
#define  THREAD_WRITE_TIMEOUT                                            (15000)

#define  THREAD_EXIT_SIGNAL                                    (MEVENT_SIGNAL_0)
#define  THREAD_DATA_SIGNAL                                (MEVENT_SIGNAL_0 + 1)

#define  MIN_BUFFER_SIZE                                                 (16384)
#define  MAX_BUFFER_SIZE                                                (131072)
#define  MIN_BUFFERS_COUNT                                                   (3)

#define  DEF_FILES_COUNT                                                     (0)
#define  MIN_FILES_COUNT                                                     (1)
#define  MAX_FILES_COUNT                                                  (4096)

#define  DEF_POOL_SIZE                                    (MAX_BUFFER_SIZE * 16)
#define  MIN_POOL_SIZE                     (MIN_BUFFER_SIZE * MIN_BUFFERS_COUNT)


#define  P7_EXT                                                        TM("p7d")

#define  TAIL_SIZE(iBuffer)         (m_dwBuffer_Size - (tUINT32)iBuffer->szUsed)

////////////////////////////////////////////////////////////////////////////////
//CClFile()
CClFile::CClFile(tXCHAR **i_pArgs, tINT32 i_iCount)
    : CClient(IP7_Client::eFileBin, i_pArgs, i_iCount)
    , m_lReject_Mem(0)
    , m_lReject_Con(0)
    , m_lReject_Int(0)
    , m_bThread(FALSE)
    , m_hThread(0) //NULL
    , m_pBuffer_Current(0)
    , m_dwBuffer_Size(0)
    , m_dwBuffers_Count(0)
    , m_eRolling(EROLLING_NONE)
    , m_qwRolling_Value(0)
    , m_dwFile_Tick(0)
    , m_qwFile_Size(0)
    , m_dwFiles_Max_Count(DEF_FILES_COUNT)
    , m_qwFiles_Max_Size(0u)
    , m_dwIndex(0)
{
    memset(&m_cHeader,  0, sizeof(m_cHeader));

    //1. base init
    if (ECLIENT_STATUS_OK == m_eStatus)
    {
        m_eStatus = Init_Base(i_pArgs, i_iCount);
    }

    //2. Initialize Pool
    if (ECLIENT_STATUS_OK == m_eStatus)
    {
        m_eStatus = Init_Pool(i_pArgs, i_iCount);
    }
   
    //3. Initialize file
    if (ECLIENT_STATUS_OK == m_eStatus)
    {
        m_eStatus = Init_File(i_pArgs, i_iCount);
    }

    //4. Initialize variables
    if (ECLIENT_STATUS_OK == m_eStatus)
    {
        m_eStatus = Init_Thread(i_pArgs, i_iCount);
    }

    //5. Initialize crash handler
    if (ECLIENT_STATUS_OK == m_eStatus)
    {
        Init_Crash_Handler(i_pArgs, i_iCount);
    }
}//CClFile()


////////////////////////////////////////////////////////////////////////////////
//~CClFile()
CClFile::~CClFile()
{
    Uninit_Crash_Handler();

    Flush();

    if (m_pBuffer_Current)
    {
        delete m_pBuffer_Current;
        m_pBuffer_Current = NULL;
    }

    m_cBuffer_Empty.Clear(TRUE);
    m_cBuffer_Ready.Clear(TRUE);

    m_cFiles.Clear(TRUE);
    m_cSecondsList.Clear(TRUE);

    CClient::Unshare();
}//~CClFile()


////////////////////////////////////////////////////////////////////////////////
//Init_Base
eClient_Status CClFile::Init_Base(tXCHAR **i_pArgs,
                                  tINT32   i_iCount
                                 )
{
    eClient_Status l_eReturn    = ECLIENT_STATUS_OK;
    tUINT32        l_dwHTime    = 0;
    tUINT32        l_dwLTime    = 0;
    tXCHAR        *l_pArg_Value = NULL;
    tWCHAR         l_pProc_Name[TPACKET_PROCESS_NAME_MAX_LEN * 2];

    ////////////////////////////////////////////////////////////////////////////
    //initialize header
    memset(l_pProc_Name, 0, sizeof(tWCHAR) * LENGTH(l_pProc_Name));
    memset(&m_cHeader,  0, sizeof(m_cHeader));
        
    CProc::Get_Process_Time(&l_dwHTime, &l_dwLTime);

    m_cHeader.qwMarker                = P7_DAMP_FILE_MARKER_V1;
    m_cHeader.dwProcess_ID            = CProc::Get_Process_ID();
    m_cHeader.dwProcess_Start_Time_Hi = l_dwHTime;
    m_cHeader.dwProcess_Start_Time_Lo = l_dwLTime;


    l_pArg_Value = Get_Argument_Text_Value(i_pArgs, i_iCount,
                                            (tXCHAR*)CLIENT_COMMAND_LINE_NAME);
    if (!l_pArg_Value)
    {
        CProc::Get_Process_Name(m_cHeader.pProcess_Name, P7_DAMP_FILE_PROCESS_LENGTH);
    }
    else
    {
    #ifdef UTF8_ENCODING
        Convert_UTF8_To_UTF16((const char*)l_pArg_Value, 
                              m_cHeader.pProcess_Name, 
                              P7_DAMP_FILE_PROCESS_LENGTH
                             );
    #else
        PStrCpy((tXCHAR*)m_cHeader.pProcess_Name,
                P7_DAMP_FILE_PROCESS_LENGTH, 
                l_pArg_Value
               );
    #endif                             
    }

    CSys::Get_Host_Name(m_cHeader.pHost_Name, P7_DAMP_FILE_HOST_LENGTH);

    return l_eReturn;
}//Init_Base


////////////////////////////////////////////////////////////////////////////////
//Init_Pool
eClient_Status CClFile::Init_Pool(tXCHAR **i_pArgs,
                                  tINT32   i_iCount
                                 )
{
    eClient_Status l_eReturn         = ECLIENT_STATUS_OK;
    tXCHAR        *l_pArg_Value      = NULL;
    tUINT32        l_dwPool_Size     = DEF_POOL_SIZE;
    tUINT32        l_dwBuffer_Size   = MAX_BUFFER_SIZE * 2;
    tUINT32        l_dwBuffers_Count = 0;
    pAList_Cell    l_pEl             = NULL;

    ////////////////////////////////////////////////////////////////////////////
    //getting pool size
    l_pArg_Value = Get_Argument_Text_Value(i_pArgs, i_iCount,
                                           (tXCHAR*)CLIENT_COMMAND_POOL_SIZE);
    if (l_pArg_Value)
    {
        l_dwPool_Size = 1024 * (tUINT32)PStrToInt(l_pArg_Value);

        if (MIN_POOL_SIZE > l_dwPool_Size)
        {
            l_dwPool_Size = MIN_POOL_SIZE;
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    //calculate buffer size and buffers count
    //Starting from largest buffer size trying to calculate how much buffers we
    //can allocate, is amount is smaller than minimum - device buffer size by 2
    //and repeat procedure
    while (l_dwBuffers_Count < MIN_BUFFERS_COUNT)
    {
        l_dwBuffer_Size /= 2; 
        l_dwBuffers_Count = l_dwPool_Size / l_dwBuffer_Size;
    }

    if (    (MIN_BUFFER_SIZE   > l_dwBuffer_Size)
         || (MIN_BUFFERS_COUNT > l_dwBuffers_Count)
       )
    {
        JOURNAL_ERROR(m_pLog, 
                      TM("Pool: Memory calculation error, buffer size = %d, buffers count = %d"),
                      l_dwBuffer_Size,
                      l_dwBuffers_Count
                    );
        l_eReturn = ECLIENT_STATUS_INTERNAL_ERROR;
        goto l_lblExit;
    }

    ////////////////////////////////////////////////////////////////////////////
    //allocate buffers
    m_dwBuffer_Size   = l_dwBuffer_Size;
    m_dwBuffers_Count = l_dwBuffers_Count;

    for (tUINT32 l_dwI = 0; l_dwI < m_dwBuffers_Count; l_dwI++)
    {
        sBuffer *l_pBuffer = new sBuffer(m_dwBuffer_Size);
        if (l_pBuffer)
        {
            m_cBuffer_Empty.Add_After(NULL, l_pBuffer);
        }
        else
        {
            JOURNAL_ERROR(m_pLog, TM("Pool: Memory calculation failed"));
            l_eReturn = ECLIENT_STATUS_INTERNAL_ERROR;
            goto l_lblExit;
        }
    }


    ////////////////////////////////////////////////////////////////////////////
    //copy header to buffer
    l_pEl             = m_cBuffer_Empty.Get_First();
    m_pBuffer_Current = m_cBuffer_Empty.Get_Data(l_pEl);
    m_cBuffer_Empty.Del(l_pEl, FALSE);

    if (m_dwBuffer_Size > sizeof(m_cHeader))
    {
        memcpy(m_pBuffer_Current->pBuffer,
               &m_cHeader,
               sizeof(m_cHeader)
              );

        m_pBuffer_Current->szUsed += sizeof(m_cHeader);
    }
    else
    {
        JOURNAL_ERROR(m_pLog, TM("Pool: Not enough memory"));
        l_eReturn = ECLIENT_STATUS_INTERNAL_ERROR;
        goto l_lblExit;
    }


l_lblExit:
    return l_eReturn; 
}//Init_Pool


////////////////////////////////////////////////////////////////////////////////
//Init_File
eClient_Status CClFile::Init_File(tXCHAR **i_pArgs,
                                  tINT32   i_iCount
                                 )
{
    eClient_Status  l_eReturn  = ECLIENT_STATUS_OK;
    tXCHAR         *l_pArgV    = NULL;
    pAList_Cell     l_pStart   = NULL;
    tUINT32         l_dwDirLen = 0;

    m_pDir.Realloc(4096);

    ////////////////////////////////////////////////////////////////////////////
    //get maximum count of the log files
    l_pArgV = Get_Argument_Text_Value(i_pArgs, i_iCount,
                                      (tXCHAR*)CLIENT_COMMAND_LINE_FILES_COUNT_MAX
                                     );
    if (l_pArgV)
    {
        m_dwFiles_Max_Count = (tUINT32)PStrToInt(l_pArgV);

        if (    (MIN_FILES_COUNT > m_dwFiles_Max_Count)
             || (MAX_FILES_COUNT < m_dwFiles_Max_Count)
           )
        {
            m_dwFiles_Max_Count = DEF_FILES_COUNT;
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
            JOURNAL_ERROR(m_pLog, TM("Can't create directory: %s"), l_pArgV);
            l_eReturn = ECLIENT_STATUS_NOT_ALLOWED;
            goto l_lblExit;
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    //enumerate files & sorting ....
    if (    (DEF_FILES_COUNT != m_dwFiles_Max_Count)
         || (m_qwFiles_Max_Size)
       )
    {
        CFSYS::Enumerate_Files(&m_cFiles, &m_pDir, TM("*.") P7_EXT, 0);

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
        JOURNAL_ERROR(m_pLog, TM("File creation failed"), l_pArgV);
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
            JOURNAL_ERROR(m_pLog, TM("Rolling value is not correct = %s"), l_cRolling.Get());
        }

        if (EROLLING_MEGABYTES == m_eRolling)
        {
            m_qwRolling_Value *= 0x100000ULL; //megabytes
        }
        else if (EROLLING_HOURS == m_eRolling)
        {
            if (m_qwRolling_Value > 1000)
            {
                JOURNAL_ERROR(m_pLog, TM("Rolling value is more than 1000 hours, cutting"));
                m_qwRolling_Value = 1000;
            }

            m_qwRolling_Value *= 3600000ULL; //milliseconds in one hour
        }
    }//if (l_pArgV)

l_lblExit:
    return l_eReturn;
}//Init_File


////////////////////////////////////////////////////////////////////////////////
//Init_Thread
eClient_Status CClFile::Init_Thread(tXCHAR **i_pArgs,
                                    tINT32   i_iCount
                                   )
{
    eClient_Status l_eReturn = ECLIENT_STATUS_OK;

    UNUSED_ARG(i_pArgs);
    UNUSED_ARG(i_iCount);

    if (ECLIENT_STATUS_OK == l_eReturn)
    {
        if (FALSE == m_cEvent.Init(2, EMEVENT_SINGLE_AUTO, EMEVENT_MULTI))
        {
            l_eReturn = ECLIENT_STATUS_INTERNAL_ERROR;
            JOURNAL_ERROR(m_pLog, TM("Exit event wasn't created !"));
        }
    }

    if (ECLIENT_STATUS_OK == l_eReturn)
    {
        if (FALSE == CThShell::Create(&Static_Routine,
                                      this,
                                      &m_hThread,
                                      TM("P7:Comm") 
                                     )
           )
        {
            l_eReturn      = ECLIENT_STATUS_INTERNAL_ERROR;
            JOURNAL_ERROR(m_pLog, TM("Communication thread wasn't created !"));
        }
        else
        {
            m_bThread = TRUE;
        }
    }

    return l_eReturn;
}//Init_Thread


////////////////////////////////////////////////////////////////////////////////
// Create_File                                      
eClient_Status CClFile::Create_File()
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
                TM("/%04d%02d%02d-%02d%02d%02d%03d.") P7_EXT,
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
        JOURNAL_ERROR(m_pLog, TM("Can't create file: %s"), l_cFilePath.Get());
        l_eReturn = ECLIENT_STATUS_INTERNAL_ERROR;
    }
    else 
    {
        m_cFiles.Add_After(m_cFiles.Get_Last(), new CWString(l_cFilePath.Get()));

        if (DEF_FILES_COUNT != m_dwFiles_Max_Count)
        {
            while (m_dwFiles_Max_Count < m_cFiles.Count())
            {
                pAList_Cell l_pEl   = m_cFiles.Get_First();
                CWString   *l_pPath = m_cFiles.Get_Data(l_pEl);
                if (l_pPath)
                {
                    if (!CFSYS::Delete_File(l_pPath->Get()))
                    {
                        JOURNAL_ERROR(m_pLog, TM("Can't delete file: %s"), l_pPath->Get());
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

            while (l_qwTotalSize > ((tUINT64)m_qwFiles_Max_Size))
            {
                pAList_Cell l_pEl = m_cFiles.Get_First();
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
                        JOURNAL_ERROR(m_pLog, TM("Can't delete file: %s"), l_pPath->Get());
                    }
                }

                m_cFiles.Del(l_pEl, TRUE);
            }
        }
    }

    m_dwFile_Tick = GetTickCount();

    return l_eReturn;
}// Create_File


////////////////////////////////////////////////////////////////////////////////
//Parse_Rolling_Time
tBOOL CClFile::Parse_Rolling_Time(const tXCHAR *i_pTime)
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


////////////////////////////////////////////////////////////////////////////////
//Sent
eClient_Status CClFile::Sent(tUINT32          i_dwChannel_ID,
                             sP7C_Data_Chunk *i_pChunks, 
                             tUINT32          i_dwCount,
                             tUINT32          i_dwSize
                            )
{
    eClient_Status   l_eReturn       = ECLIENT_STATUS_OK;
    tUINT32          l_dwTotal_Size  = i_dwSize + (tUINT32)sizeof(sH_User_Raw);
    sH_User_Raw      l_sHeader       = {INIT_USER_HEADER(l_dwTotal_Size, i_dwChannel_ID)};
    sP7C_Data_Chunk  l_sHeader_Chunk = {&l_sHeader, (tUINT32)sizeof(l_sHeader)};
    //Wanr: variables without default value!
    tBOOL            l_bExit;
    sP7C_Data_Chunk *l_pChunk;
    tUINT32          l_dwChunk_Offs;
    pAList_Cell      l_pEl;
    tUINT32          l_dwFree_Size;

    if (ECLIENT_STATUS_OK != m_eStatus)
    {
        ATOMIC_INC(&m_lReject_Int);
        return m_eStatus;
    }

    if (    (NULL                            == i_pChunks)
         || (0                               >= i_dwCount)
         || (USER_PACKET_MAX_SIZE            <= l_dwTotal_Size) 
         || (USER_PACKET_CHANNEL_ID_MAX_SIZE <= i_dwChannel_ID)
       )
    {
        ATOMIC_INC(&m_lReject_Int);
        return ECLIENT_STATUS_WRONG_PARAMETERS;
    }

    //N.B. We do not check i_dwSize and real size of all chunks in release mode!!!
#ifdef _DEBUG
    tUINT32 l_dwReal_Size = 0;
    for (tUINT32 l_dwI = 0; l_dwI < i_dwCount; l_dwI ++)
    {
        if (i_pChunks[l_dwI].pData)
        {
            l_dwReal_Size += i_pChunks[l_dwI].dwSize;
        }
        else
        {
            break;
        }
    }

    if (l_dwReal_Size != i_dwSize)
    {
        ATOMIC_INC(&m_lReject_Int);
        return ECLIENT_STATUS_WRONG_PARAMETERS;
    }
#endif


    LOCK_ENTER(m_hCS);

    if (FALSE == m_bConnected)
    {
        l_eReturn = ECLIENT_STATUS_OFF;
        ATOMIC_INC(&m_lReject_Con);
        goto l_lExit;
    }

    l_dwFree_Size =   (m_cBuffer_Empty.Count() * m_dwBuffer_Size) 
                    + ((m_pBuffer_Current) ? (m_dwBuffer_Size - (tUINT32)m_pBuffer_Current->szUsed) : 0);

    //if size is more than available ...
    if (l_dwFree_Size < l_dwTotal_Size)
    {
        l_eReturn = ECLIENT_STATUS_NO_FREE_BUFFERS;
        ATOMIC_INC(&m_lReject_Mem);
        goto l_lExit;
    }


    //because we add header as chunk
    i_dwCount ++;

    l_bExit        = FALSE;
    l_pChunk       = &l_sHeader_Chunk; //&i_pChunks[0];
    l_dwChunk_Offs = 0;
    
    while (FALSE == l_bExit)
    {
        //if packet is null we need to extract another one 
        if (NULL == m_pBuffer_Current)
        {
            l_pEl = m_cBuffer_Empty.Get_First();
            m_pBuffer_Current = m_cBuffer_Empty.Get_Data(l_pEl);
            m_cBuffer_Empty.Del(l_pEl, FALSE); 
        }

        while (    (m_pBuffer_Current)
                && (i_dwCount)
              )
        {
            //if packet free size is larger or equal to chunk size
            if ( TAIL_SIZE(m_pBuffer_Current) >= l_pChunk->dwSize )
            {
                memcpy(m_pBuffer_Current->pBuffer + m_pBuffer_Current->szUsed, 
                       ((tUINT8*)l_pChunk->pData) + l_dwChunk_Offs,
                       l_pChunk->dwSize
                      );

                m_pBuffer_Current->szUsed += l_pChunk->dwSize;

                //current chunk was moved, we reduce chunks amount 
                --i_dwCount;

                if (0 >= i_dwCount)
                {
                    l_bExit = TRUE;
                }
                else
                {
                    //we are finish with that chunk
                    //l_pChunk->dwSize = 0; 
                    l_dwChunk_Offs = 0;

                    if (l_pChunk == &l_sHeader_Chunk)
                    {
                        l_pChunk = &i_pChunks[0];
                    }
                    else
                    {
                        //go to next chunk
                        l_pChunk ++;
                    }

                    //if packet is filled - put it to data queue
                    if (0 >= TAIL_SIZE(m_pBuffer_Current))
                    {
                        m_cBuffer_Ready.Add_After(m_cBuffer_Ready.Get_Last(), 
                                                  m_pBuffer_Current
                                                 );
                        m_qwFile_Size    += m_pBuffer_Current->szUsed;
                        m_pBuffer_Current = NULL;
                        m_cEvent.Set(THREAD_DATA_SIGNAL);
                    }
                }
            }
            else //if chunk data is greater than packet free space
            {
                memcpy(m_pBuffer_Current->pBuffer + m_pBuffer_Current->szUsed, 
                       ((tUINT8*)l_pChunk->pData) + l_dwChunk_Offs,
                       TAIL_SIZE(m_pBuffer_Current)
                      );
                l_dwChunk_Offs   += TAIL_SIZE(m_pBuffer_Current);
                l_pChunk->dwSize -= TAIL_SIZE(m_pBuffer_Current);

                m_pBuffer_Current->szUsed += TAIL_SIZE(m_pBuffer_Current);

                m_cBuffer_Ready.Add_After(m_cBuffer_Ready.Get_Last(), 
                                          m_pBuffer_Current
                                         );
                m_qwFile_Size    += m_pBuffer_Current->szUsed;
                m_pBuffer_Current = NULL;
                m_cEvent.Set(THREAD_DATA_SIGNAL);
            }
        } //while ( (m_pBuffer_Current) && (i_dwCount) )
    } //while (FALSE == l_bExit)

l_lExit:

    LOCK_EXIT(m_hCS);

    return l_eReturn;
}//Sent


////////////////////////////////////////////////////////////////////////////////
//Get_Info
tBOOL CClFile::Get_Info(sP7C_Info *o_pInfo)
{
    if (NULL == o_pInfo)
    {
        return FALSE;
    }

    LOCK_ENTER(m_hCS);

    o_pInfo->dwMem_Alloc = m_dwBuffer_Size * m_dwBuffers_Count;
    o_pInfo->dwMem_Free  = m_cBuffer_Empty.Count() * m_dwBuffer_Size;
    o_pInfo->dwMem_Used  = m_cBuffer_Ready.Count() * m_dwBuffer_Size;

    o_pInfo->dwReject_Mem = m_lReject_Mem;
    o_pInfo->dwReject_Con = m_lReject_Con;
    o_pInfo->dwReject_Int = m_lReject_Int;
    LOCK_EXIT(m_hCS);

    return TRUE;
}//Get_Info


////////////////////////////////////////////////////////////////////////////////
//Flush
tBOOL CClFile::Flush()
{
    tBOOL l_bStack_Trace = TRUE;

    ////////////////////////////////////////////////////////////////////////////
    //notify channels about closing
    sP7C_Status l_sStatus = {FALSE, 0};
    l_sStatus.bConnected = FALSE;
    l_sStatus.dwResets   = 0;

    LOCK_ENTER(m_hCS_Reg);
    for (tUINT32 l_dwI = 0; l_dwI < USER_PACKET_CHANNEL_ID_MAX_SIZE; l_dwI++)
    {
        if (m_pChannels[l_dwI])
        {
            m_pChannels[l_dwI]->On_Flush(l_dwI, &l_bStack_Trace);
            m_pChannels[l_dwI]->On_Status(l_dwI, &l_sStatus);
        }
    }
    LOCK_EXIT(m_hCS_Reg);

    m_cEvent.Set(THREAD_EXIT_SIGNAL);

    if (m_bThread)
    {
        if (TRUE == CThShell::Close(m_hThread, 60000))
        {
            m_hThread = 0;//NULL;
            m_bThread = FALSE;
        }
        else
        {
            JOURNAL_CRITICAL(m_pLog, TM("Can't close file thread !"));
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    //Lock
    LOCK_ENTER(m_hCS);

    if (m_bConnected)
    {
        pAList_Cell l_pEl = NULL;
        while ((l_pEl = m_cBuffer_Ready.Get_Next(l_pEl)))
        {
            sBuffer *l_pBuffer = m_cBuffer_Ready.Get_Data(l_pEl);
            if (l_pBuffer)
            {
                //m_qwFile_Size += l_pBuffer->szUsed; //for debug

                if (l_pBuffer->szUsed != m_cFile.Write(l_pBuffer->pBuffer,
                                                       l_pBuffer->szUsed,
                                                       FALSE
                                                      )
                   )
                {
                    JOURNAL_CRITICAL(m_pLog, TM("Can't write to file !"));
                    m_bConnected = FALSE;
                    break;
                }
            }
        }
    }

    if (m_pBuffer_Current)
    {
        if (    (m_bConnected)
             && (m_pBuffer_Current->szUsed)
           )
        {
            //m_qwFile_Size += m_pBuffer_Current->szUsed; //for debug

            if (m_pBuffer_Current->szUsed != m_cFile.Write(m_pBuffer_Current->pBuffer,
                                                           m_pBuffer_Current->szUsed,
                                                           FALSE
                                                          )
                )
            {
                JOURNAL_CRITICAL(m_pLog, TM("Can't write to file !"));
                m_bConnected = FALSE;
            }
        }
    }

    m_bConnected = FALSE;

    LOCK_EXIT(m_hCS);
    //unlock
    ////////////////////////////////////////////////////////////////////////////

    m_cFile.Close(TRUE);

    return TRUE;
}//Flush


////////////////////////////////////////////////////////////////////////////////
//Roll
void CClFile::Roll()
{
    CBList<sBuffer*> l_cBuffers;
    sBuffer         *l_pBuffer = NULL;
    pAList_Cell      l_pEl     = NULL;
    tBOOL            l_bError  = FALSE;
    tBOOL            l_bHeader = FALSE;
    tBOOL            l_bUpdate = FALSE;

    if (FALSE == m_bConnected)
    {
        return;
    }

    ////////////////////////////////////////////////////////////////////////////
    //
    LOCK_ENTER(m_hCS);
    //move ready buffers to temporary list
    while ((l_pEl = m_cBuffer_Ready.Get_First()))
    {
        l_cBuffers.Add_After(l_cBuffers.Get_Last(),
                             m_cBuffer_Ready.Get_Data(l_pEl)
                            );

        m_cBuffer_Ready.Del(l_pEl, FALSE);
    }

    if (m_pBuffer_Current)
    {
        l_cBuffers.Add_After(l_cBuffers.Get_Last(), m_pBuffer_Current);
        m_pBuffer_Current = NULL;
    }

    ////////////////////////////////////////////////////////////////////////////
    //update reset count and file size
    m_dwConnection_Resets ++;
    m_qwFile_Size = 0;

    ////////////////////////////////////////////////////////////////////////////
    //copy header to current buffer
    l_pEl = m_cBuffer_Empty.Get_First();
    if (l_pEl)
    {
        m_pBuffer_Current = m_cBuffer_Empty.Get_Data(l_pEl);
        m_cBuffer_Empty.Del(l_pEl, FALSE);

        memcpy(m_pBuffer_Current->pBuffer,
               &m_cHeader,
               sizeof(m_cHeader)
              );

        m_pBuffer_Current->szUsed += sizeof(m_cHeader);
        l_bHeader                  = TRUE;
    }

    LOCK_EXIT(m_hCS); 
    //
    ////////////////////////////////////////////////////////////////////////////

    //m_dwConnection_Resets will be updated !
    Update_Channels_Status(m_bConnected, m_dwConnection_Resets); 


    ////////////////////////////////////////////////////////////////////////////
    //writing data
    l_pEl = NULL;
    while ((l_pEl = l_cBuffers.Get_Next(l_pEl)))
    {
        l_pBuffer = l_cBuffers.Get_Data(l_pEl);

        if (l_pBuffer->szUsed > m_cFile.Write(l_pBuffer->pBuffer, l_pBuffer->szUsed, FALSE))
        {
            JOURNAL_CRITICAL(m_pLog, TM("Not possible to write data"));
            l_bError = TRUE;
            break;
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    //push buffers to empty list
    LOCK_ENTER(m_hCS);

    if (l_bError)
    {
        m_bConnected = FALSE;
        l_bUpdate    = TRUE;
    }

    while ((l_pEl = l_cBuffers.Get_First()))
    {
        l_pBuffer = l_cBuffers.Get_Data(l_pEl);

        l_pBuffer->szUsed = 0;

        if (FALSE == l_bHeader) //write header if it was not write before
        {
            memcpy(l_pBuffer->pBuffer,
                   &m_cHeader,
                   sizeof(m_cHeader)
                  );

            l_pBuffer->szUsed += sizeof(m_cHeader);

            l_bHeader = TRUE;

            m_cBuffer_Empty.Add_After(NULL, l_pBuffer);
        }
        else
        {
            m_cBuffer_Empty.Add_After(m_cBuffer_Empty.Get_Last(), l_pBuffer);
        }

        l_cBuffers.Del(l_pEl, FALSE);
    }//while (l_pEl = l_cBuffers.Get_First())

    LOCK_EXIT(m_hCS); 

    ////////////////////////////////////////////////////////////////////////////
    //create new file
    m_cFile.Close(TRUE);

    if (m_bConnected)
    {
        if (ECLIENT_STATUS_OK != Create_File())
        {
            JOURNAL_CRITICAL(m_pLog, TM("Not possible to create file"));

            LOCK_ENTER(m_hCS);
            m_bConnected = FALSE;
            LOCK_EXIT(m_hCS); 

            l_bUpdate = TRUE;
        }
    }

    if (l_bUpdate)
    {
        Update_Channels_Status(m_bConnected, m_dwConnection_Resets); 
    }
}


////////////////////////////////////////////////////////////////////////////////
//Update_Channels_Status
void CClFile::Update_Channels_Status(tBOOL i_bConnected, tUINT32 i_dwResets)
{
    sP7C_Status l_sStatus = {FALSE, 0};
    //LOCK_ENTER(m_hCS); 
    l_sStatus.bConnected = i_bConnected;
    l_sStatus.dwResets   = i_dwResets;
    //LOCK_EXIT(m_hCS); 

    LOCK_ENTER(m_hCS_Reg);
    for (tUINT32 l_dwI = 0; l_dwI < USER_PACKET_CHANNEL_ID_MAX_SIZE; l_dwI++)
    {
        if (m_pChannels[l_dwI])
        {
            m_pChannels[l_dwI]->On_Status(l_dwI, &l_sStatus);
        }
    }
    LOCK_EXIT(m_hCS_Reg);
}//Update_Channels_Status


////////////////////////////////////////////////////////////////////////////////
//Routine
void CClFile::Routine()
{
    pAList_Cell l_pEl            = NULL;
    tBOOL       l_bExit          = FALSE;
    tUINT32     l_dwSignal       = MEVENT_TIME_OUT;
    tUINT32     l_dwWait_TimeOut = THREAD_IDLE_TIMEOUT;
    tUINT32     l_dwIteration    = 0;
    tBOOL       l_bRoll          = FALSE;
    sBuffer    *l_pBuffer        = NULL;
    tUINT32     l_uiSecond       = GetSecondOfDay(); 
    tUINT32     l_uWriteTick     = GetTickCount();

    while (FALSE == l_bExit)
    {
        l_dwSignal = m_cEvent.Wait(l_dwWait_TimeOut);

        if (THREAD_EXIT_SIGNAL == l_dwSignal)
        {
            l_bExit = TRUE;
        }
        else if (    (THREAD_DATA_SIGNAL == l_dwSignal) //one buffer to write!
                  || (    (MEVENT_TIME_OUT == l_dwSignal)
                       && (CTicks::Difference(GetTickCount(), l_uWriteTick) > THREAD_WRITE_TIMEOUT)
                     )
                )
        {
            l_dwWait_TimeOut = 0;

            ////////////////////////////////////////////////////////////////////
            //extract buffer
            LOCK_ENTER(m_hCS);

            l_pBuffer = NULL;
            l_bRoll   = FALSE;

            if (    (EROLLING_MEGABYTES == m_eRolling)
                 && (m_qwRolling_Value <= m_qwFile_Size)
               )
            {
                l_bRoll = TRUE;
            }
            else
            {
                l_pEl = m_cBuffer_Ready.Get_First();
                if (l_pEl)
                {
                    l_pBuffer = m_cBuffer_Ready.Get_Data(l_pEl);
                    m_cBuffer_Ready.Del(l_pEl, FALSE);
                }
            }

            LOCK_EXIT(m_hCS); 

            ////////////////////////////////////////////////////////////////////
            //writing data
            if (l_pBuffer)
            {
                if (    (m_bConnected)
                     && (l_pBuffer->szUsed > m_cFile.Write(l_pBuffer->pBuffer,
                                                           l_pBuffer->szUsed, 
                                                           FALSE
                                                          )
                       )
                   )
                {
                    JOURNAL_CRITICAL(m_pLog, TM("Not possible to write data"));

                    Update_Channels_Status(FALSE, m_dwConnection_Resets);

                    LOCK_ENTER(m_hCS);
                    m_bConnected = FALSE;
                    LOCK_EXIT(m_hCS); 
                }

                LOCK_ENTER(m_hCS);
                l_pBuffer->szUsed = 0;
                m_cBuffer_Empty.Add_After(NULL, l_pBuffer);
                LOCK_EXIT(m_hCS); 
            }
            else if (l_bRoll) //rolling files
            {
                Roll();
            }

            l_uWriteTick = GetTickCount();
        }
        else if (MEVENT_TIME_OUT == l_dwSignal)
        {
            l_dwWait_TimeOut = THREAD_IDLE_TIMEOUT;
        }

        if (EROLLING_HOURS == m_eRolling)
        {
            if (    (0 == (l_dwIteration & 0x3F))
                 && (CTicks::Difference(GetTickCount(), m_dwFile_Tick) > m_qwRolling_Value)
               )
            {
                Roll();
                l_uWriteTick = GetTickCount();
            }
        }
        else if (EROLLING_TIME == m_eRolling)
        {
            if (0 == (l_dwIteration & 0x3F))
            {
                tUINT32     l_uiNextSecond = GetSecondOfDay(); 
                pAList_Cell l_pEl         = NULL;

                while ((l_pEl = m_cSecondsList.Get_Next(l_pEl)))
                {
                    tUINT32 l_uiIem = m_cSecondsList.Get_Data(l_pEl);
                    if (    (l_uiSecond <= l_uiIem)
                         && (l_uiNextSecond > l_uiIem)
                       )
                    {
                        Roll();
                        l_uWriteTick = GetTickCount();
                        break;
                    }
                }

                l_uiSecond = l_uiNextSecond;
            }
        }

        l_dwIteration++;
    }
}//Comm_Routine
