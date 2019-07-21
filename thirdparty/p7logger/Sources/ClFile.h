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

#ifndef CLFILE_H
#define CLFILE_H

#include "IFile.h"
#include "PFile.h"


////////////////////////////////////////////////////////////////////////////////
class CClFile:
    public CClient
{
protected:
    struct sBuffer
    {
        tUINT8 *pBuffer;
        size_t  szUsed;

        sBuffer(size_t i_szBuffer)
        {
            pBuffer = (tUINT8*)malloc(i_szBuffer);
            szUsed  = 0;
        }

        ~sBuffer()
        {
            if (pBuffer)
            {
                free(pBuffer);
                pBuffer = NULL;
            }
            szUsed  = 0;
        }
    };

    enum eRolling
    {
        EROLLING_NONE,
        EROLLING_MEGABYTES,
        EROLLING_HOURS,
        EROLLING_TIME
    };

private:
    tINT32 volatile    m_lReject_Mem;
    tINT32 volatile    m_lReject_Con;
    tINT32 volatile    m_lReject_Int;

    CMEvent            m_cEvent;
    tBOOL              m_bThread;
    CThShell::tTHREAD  m_hThread;

    CBList<sBuffer*>   m_cBuffer_Empty;
    CBList<sBuffer*>   m_cBuffer_Ready;
    sBuffer           *m_pBuffer_Current;
    tUINT32            m_dwBuffer_Size;
    tUINT32            m_dwBuffers_Count;
    CPFile             m_cFile;
    CWString           m_pDir;
    sP7File_Header     m_cHeader;
    eRolling           m_eRolling;
    tUINT64            m_qwRolling_Value;
    tUINT32            m_dwFile_Tick;
    tUINT64            m_qwFile_Size;
    tUINT32            m_dwFiles_Max_Count;
    tUINT64            m_qwFiles_Max_Size;
    CBList<CWString*>  m_cFiles;
    CUintList          m_cSecondsList;
    tUINT32            m_dwIndex;

public:
    CClFile(tXCHAR **i_pArgs,
            tINT32   i_iCount
           );
    virtual ~CClFile();

private:
    eClient_Status Init_Base(tXCHAR **i_pArgs,
                             tINT32   i_iCount
                            );

    eClient_Status Init_Pool(tXCHAR **i_pArgs,
                             tINT32   i_iCount
                            );

    eClient_Status Init_File(tXCHAR **i_pArgs,
                             tINT32   i_iCount
                            );

    eClient_Status Init_Thread(tXCHAR **i_pArgs,
                               tINT32   i_iCount
                              );

    eClient_Status Create_File();

    tBOOL          Parse_Rolling_Time(const tXCHAR *i_pTime);

public:
    eClient_Status Sent(tUINT32            i_dwChannel_ID,
                        sP7C_Data_Chunk   *i_pChunks, 
                        tUINT32            i_dwCount,
                        tUINT32            i_dwSize
                       );

    tBOOL          Get_Info(sP7C_Info *o_pInfo);
    tBOOL          Flush();

private:
    void  Roll();
    void  Update_Channels_Status(tBOOL i_bConnected, tUINT32 i_dwResets);

    static THSHELL_RET_TYPE THSHELL_CALL_TYPE Static_Routine(void *i_pContext)
    {
        CClFile *l_pRoutine = static_cast<CClFile *>(i_pContext);
        if (l_pRoutine)
        {
            l_pRoutine->Routine();
        }

        CThShell::Cleanup();
        return THSHELL_RET_OK;
    } 

    void  Routine();
};


#endif //CLFILE_H
