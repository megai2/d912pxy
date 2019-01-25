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
////////////////////////////////////////////////////////////////////////////////
// This header file describe buffers pool                                      /
////////////////////////////////////////////////////////////////////////////////
#ifndef PACKETSPOOL_H
#define PACKETSPOOL_H

#include <new>

////////////////////////////////////////////////////////////////////////////////
//CBuffers_Pool
class CBuffers_Pool
{
protected:
    struct sChunk
    {
        tUINT8 *pData;
        sChunk *pNext;

        sChunk()
        {
            pData = NULL;
            pNext = NULL;
        }
    };

    //put volatile variables at the top, to obtain 32 bit alignment. 
    //Project has 8 tUINT8s alignment by default
    volatile tINT32    m_lReference_Count;
    volatile tUINT32   m_dwSize;
    volatile tUINT32  *m_pSize;
    tUINT32            m_dwMax_Size;
    tUINT32            m_dwChunk_Size;
    tUINT32            m_dwPacket_Size;

    CBList<CTPacket*>  m_cBuffers;
    tUINT32            m_dwCount;
    tUINT32            m_dwMax_Count;
    tUINT32            m_dwExtracted;

    tUINT32            m_dwBuffer_Size;
    tLOCK              m_hCS;
    IJournal          *m_pLog;
    tUINT32            m_dwID;
    tBOOL              m_bInitialized;

    sChunk            *m_pChunk_First;
public:
    ////////////////////////////////////////////////////////////////////////////
    //CBuffers_Pool::CBuffers_Pool
    CBuffers_Pool(IJournal  *i_pLog, 
                  tUINT32    i_dwChunk_Size,
                  tUINT32    i_dwMax_Size,
                  tUINT32    i_dwBuffer_Size,
                  tUINT32    i_dwID
                 )
       : m_lReference_Count(0)
       , m_dwSize(0)
       , m_pSize(&m_dwSize)
       , m_dwMax_Size(i_dwMax_Size)
       , m_dwChunk_Size(i_dwChunk_Size)
       , m_dwPacket_Size(0)

       , m_dwCount(0)
       , m_dwMax_Count(0)
       , m_dwExtracted(0)
       , m_dwBuffer_Size(i_dwBuffer_Size)
       , m_pLog(i_pLog)
       , m_dwID(i_dwID)
       , m_bInitialized(TRUE)
       , m_pChunk_First(NULL)
    {
        if (m_pLog)
        {
            m_pLog->Add_Ref();
        }

        LOCK_CREATE(m_hCS);

        m_dwPacket_Size = sizeof(CTPacket) + i_dwBuffer_Size;
        m_dwChunk_Size  = ((m_dwChunk_Size / m_dwPacket_Size) + 1) * m_dwPacket_Size;
        m_dwMax_Count   = i_dwMax_Size / m_dwPacket_Size;
        //m_bInitialized  = Allocate(m_dwChunk_Size);
    }//CBuffers_Pool::CBuffers_Pool


    ////////////////////////////////////////////////////////////////////////////
    //CBuffers_Pool::~CBuffers_Pool
    ~CBuffers_Pool()
    {
        if (m_dwExtracted)
        {
            JOURNAL_ERROR(m_pLog,  
                          TM("%d buffers are missed !."), 
                          (tUINT32)m_dwExtracted
                         );
        }

        if (0 < Get_Reference_Count())
        {
            JOURNAL_ERROR(m_pLog, TM("Reference count is greater than 0 !"));
        }

        m_cBuffers.Clear(FALSE);
 
        sChunk  *l_pNext = NULL;
        while (m_pChunk_First)
        {
            l_pNext = m_pChunk_First->pNext;

            if (m_pChunk_First->pData)
            {
                delete [] m_pChunk_First->pData;
            }

            delete m_pChunk_First;

            m_pChunk_First = l_pNext;
        }

        if (m_pLog)
        {
            m_pLog->Release();
            m_pLog = NULL;
        }

        LOCK_DESTROY(m_hCS);
    }//CBuffers_Pool::~CBuffers_Pool


    ////////////////////////////////////////////////////////////////////////////
    //CBuffers_Pool::Allocate
    tBOOL Allocate(tUINT32 i_dwSize)
    {
        CTPacket *l_pPacket = NULL; 
        sChunk   *l_pChunk  = NULL;
        tUINT8   *l_pData   = NULL;

        if ((*m_pSize) >= m_dwMax_Size)
        {
            JOURNAL_ERROR(m_pLog,  
                          TM("Failed, Memory limit is reached, ID = %d Count = %d, Max = %d. Buffer Size = %d, Memory limit = %d"), 
                          (tUINT32)m_dwID,
                          (tUINT32)m_dwCount,
                          (tUINT32)m_dwMax_Count,
                          (tUINT32)m_dwBuffer_Size,
                          (tUINT32)m_dwMax_Size
                         );

            return FALSE;
        }

        if (NULL == (l_pChunk = new sChunk()))
        {
            JOURNAL_ERROR(m_pLog,  TM("Buffer allocation failed."));
            return FALSE;
        }

        l_pChunk->pData = new tUINT8[i_dwSize];
        if (NULL == l_pChunk->pData)
        {
            JOURNAL_ERROR(m_pLog,  TM("Buffer allocation failed."));
            delete l_pChunk;
            return FALSE;
        }

        LOCK_ENTER(m_hCS); 

        l_pChunk->pNext = m_pChunk_First;
        m_pChunk_First  = l_pChunk;
        l_pData         = l_pChunk->pData;

        ATOMIC_ADD((volatile tINT32*)m_pSize, i_dwSize);

        for (tUINT32 l_dwI = 0; l_dwI < (i_dwSize / m_dwPacket_Size); l_dwI++)
        {
            l_pPacket = new (l_pData) CTPacket(l_pData + sizeof(CTPacket),
                                               m_dwBuffer_Size,
                                               m_dwID
                                              );
            l_pData += m_dwPacket_Size;
            m_cBuffers.Add_After(m_cBuffers.Get_Last(), l_pPacket);
            m_dwCount ++;
        }

        LOCK_EXIT(m_hCS); 

        return TRUE;
    }//CBuffers_Pool::Allocate

    ////////////////////////////////////////////////////////////////////////////
    //CBuffers_Pool::Get_Initialized
    tBOOL Get_Initialized()
    {
        return m_bInitialized;
    }//CBuffers_Pool::Get_Initialized


    ////////////////////////////////////////////////////////////////////////////
    //CBuffers_Pool::Get_Buffer_Size
    tUINT32 Get_Buffer_Size()
    {
        return m_dwBuffer_Size;
    }//CBuffers_Pool::Get_Buffer_Size


    ////////////////////////////////////////////////////////////////////////////
    //CBuffers_Pool::Get_Free_Count
    tUINT32 Get_Free_Count()
    {
        tUINT32 l_dwReturn = 0;

        LOCK_ENTER(m_hCS); 

        l_dwReturn = m_dwMax_Count - m_dwExtracted; 

        LOCK_EXIT(m_hCS); 

        return l_dwReturn;
    }//CBuffers_Pool::Get_Free_Count


    ////////////////////////////////////////////////////////////////////////////
    //CBuffers_Pool::Get_Memory_Info
    void Get_Memory_Info(tUINT32 *o_dwUsed, 
                         tUINT32 *o_dwFree, 
                         tUINT32 *o_pAllocated
                        )
    {
        LOCK_ENTER(m_hCS); 

        if (o_dwUsed)
        {
            *o_dwUsed = m_dwExtracted * m_dwBuffer_Size;
        }

        if (o_dwFree)
        {
            *o_dwFree = (m_dwMax_Count - m_dwExtracted) * m_dwBuffer_Size;
        }

        if (o_pAllocated)
        {
            *o_pAllocated = *m_pSize;
        }

        LOCK_EXIT(m_hCS); 
    }//CBuffers_Pool::Get_Memory_Info


    ////////////////////////////////////////////////////////////////////////////
    //CBuffers_Pool::Pull_Buffer
    CTPacket *Pull_Buffer()
    {
        CTPacket    *l_pReturn = NULL;
        pAList_Cell  l_pCell   = NULL;

        LOCK_ENTER(m_hCS); 

        if (0 >= m_cBuffers.Count())
        {
            Allocate(m_dwChunk_Size);
        }

        l_pCell = m_cBuffers.Get_First();

        if (l_pCell)
        {
            l_pReturn = m_cBuffers.Get_Data(l_pCell);
            m_cBuffers.Del(l_pCell, FALSE);
            m_dwExtracted ++;
        }

        LOCK_EXIT(m_hCS); 
        return l_pReturn;
    }//CBuffers_Pool::Pull_Buffer


    ////////////////////////////////////////////////////////////////////////////
    //CBuffers_Pool::Push_Buffer
    void Push_Buffer(CTPacket *i_pBuffer)
    {
        if (    (NULL == i_pBuffer)
             || (0 == m_dwExtracted)
           )
        {
            JOURNAL_ERROR(m_pLog, TM("Wrong parameters."));
            return;
        }


        LOCK_ENTER(m_hCS); 

        m_cBuffers.Add_After(m_cBuffers.Get_Last(), i_pBuffer);
        m_dwExtracted --;

        LOCK_EXIT(m_hCS); 
    }//CBuffers_Pool::Push_Buffer


    ////////////////////////////////////////////////////////////////////////////
    //CBuffers_Pool::Inc_Reference_Count
    tINT32 Inc_Reference_Count()
    {
        tINT32 l_lReturn = 0;

        LOCK_ENTER(m_hCS); 

        m_lReference_Count ++;
        l_lReturn = m_lReference_Count;

        LOCK_EXIT(m_hCS); 

        return l_lReturn;
    }//CBuffers_Pool::Inc_Reference_Count


    ////////////////////////////////////////////////////////////////////////////
    //CBuffers_Pool::Dec_Reference_Count
    tINT32 Dec_Reference_Count()
    {
        tINT32 l_lReturn = 0;

        LOCK_ENTER(m_hCS); 

        m_lReference_Count --;
        l_lReturn = m_lReference_Count;

        LOCK_EXIT(m_hCS); 

        return l_lReturn;
    }//CBuffers_Pool::Dec_Reference_Count


    ////////////////////////////////////////////////////////////////////////////
    //CBuffers_Pool::Get_Reference_Count
    tINT32 Get_Reference_Count()
    {
        tINT32 l_lReturn = 0;

        LOCK_ENTER(m_hCS); 

        l_lReturn = m_lReference_Count;

        LOCK_EXIT(m_hCS); 

        return l_lReturn;
    }//CBuffers_Pool::Get_Reference_Count


    ////////////////////////////////////////////////////////////////////////////
    //CBuffers_Pool::Set_External_Size
    void Set_External_Size(volatile tUINT32 *i_pSize)
    {
        LOCK_ENTER(m_hCS); 
        m_pSize = i_pSize;
        LOCK_EXIT(m_hCS); 
    }//CBuffers_Pool::Set_External_Size
};

#endif //PACKETSPOOL_H
