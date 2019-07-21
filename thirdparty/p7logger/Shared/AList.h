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
//******************************************************************************
// This header file contains simple template lists                             *
// 1. List with customizable memory manager (CListBase)                           *
// 2. List with pooling of internal elements (CListPool)                          *
//******************************************************************************
#ifndef ALIST_H
#define ALIST_H

typedef void *pAList_Cell;

////////////////////////////////////////////////////////////////////////////////
//CListBase
template <typename tData_Type>
class CListBase
{
protected:
    struct tCell
    {
        tData_Type  m_pData;
        tCell      *m_pNext;
        tCell      *m_pPrev;
    };

    typedef tCell* pIndex_Cell;

    tCell        *m_pFirst;
    tCell        *m_pLast;

    tUINT32       m_dwCount;
    //HANDLE        m_hLock_Mutex;

    pIndex_Cell  *m_pIndex;
    tBOOL         m_bBrokenIndex;
    tUINT32       m_dwIndexCount;

    tBOOL         m_bInitialized;

public:
    ////////////////////////////////////////////////////////////////////////////
    //CListBase::CListBase
    CListBase()
    {
        m_bInitialized     = TRUE;
        m_pFirst           = NULL;
        m_pLast            = NULL;
        m_dwCount          = 0;
        m_pIndex           = NULL;
        m_bBrokenIndex     = TRUE;
        m_dwIndexCount     = 0;
        //m_hLock_Mutex      = CreateMutex(NULL, FALSE, NULL);

        //if (NULL == m_hLock_Mutex)
        //{
        //    m_bInitialized = FALSE;
        //}
    }//CListBase::AList

    
    ////////////////////////////////////////////////////////////////////////////
    //CListBase::~CListBase
    virtual ~CListBase()
    {
        Index_Release();

        //if (m_hLock_Mutex)
        //{
        //    CloseHandle(m_hLock_Mutex);
        //    m_hLock_Mutex = NULL;
        //}

        if (m_dwCount)
        {
#ifdef _DEBUG
    #if defined(_WIN32) || defined(_WIN64)
            OutputDebugStringW(L"Warning: List is not empty\n");
    #endif
#endif
        }
    }//CListBase::~CListBase
    
   
    ////////////////////////////////////////////////////////////////////////////
    //CListBase::Get_Initialized
    inline tBOOL Get_Initialized()
    {
        return m_bInitialized;
    }//CListBase::Get_Initialized

    ////////////////////////////////////////////////////////////////////////////
    //CListBase::Del
    inline tBOOL Del(pAList_Cell i_pCell, tBOOL i_bFree_Data)
    {
        tCell  *l_pCell   = static_cast<tCell*>(i_pCell);
        tBOOL l_bResult = TRUE;

        if (l_pCell)
        {
            l_bResult = Cell_Release(l_pCell, TRUE, i_bFree_Data);
        }

        return l_bResult;
    }//CListBase::Del


    ////////////////////////////////////////////////////////////////////////////
    //CListBase::Extract
    inline tBOOL Extract(pAList_Cell i_pCell)
    {
        tCell *l_pCell   = static_cast<tCell*>(i_pCell);
        tBOOL  l_bResult = TRUE;

        if (l_pCell)
        {
            l_bResult = Cell_Release(l_pCell, FALSE, FALSE);
            l_pCell->m_pNext = NULL;
            l_pCell->m_pPrev = NULL;
        }

        return l_bResult;
    }//CListBase::Extract

    ////////////////////////////////////////////////////////////////////////////
    //CListBase::Pull_First
    inline tData_Type Pull_First()
    {
        tData_Type l_pReturn = m_pFirst->m_pData;
        Cell_Release(m_pFirst, TRUE, FALSE);
        return l_pReturn;
    }

    ////////////////////////////////////////////////////////////////////////////
    //CListBase::Pull_Last
    inline tData_Type Pull_Last()
    {
        tData_Type l_pReturn = m_pLast->m_pData;
        Cell_Release(m_pLast, TRUE, FALSE);
        return l_pReturn;
    }

    ////////////////////////////////////////////////////////////////////////////
    //CListBase::Push_First
    inline pAList_Cell Push_First(tData_Type i_pData)
    {
        return Add_After(NULL, i_pData);
    }

    ////////////////////////////////////////////////////////////////////////////
    //CListBase::Push_Last
    inline pAList_Cell Push_Last(tData_Type i_pData)
    {
        return Add_After(m_pLast, i_pData);
    }

    ////////////////////////////////////////////////////////////////////////////
    //CListBase::Add_After
    inline pAList_Cell Add_After(pAList_Cell i_pCell, tData_Type i_pData)
    {
        tCell *l_pNew_Cell = NULL;
        tCell *l_pList_Cell = static_cast<tCell *>(i_pCell);

        l_pNew_Cell = Cell_Allocate();

        if (l_pNew_Cell)
        {
            memset(l_pNew_Cell, 0, sizeof(tCell));
            l_pNew_Cell->m_pData = i_pData;
            l_pNew_Cell->m_pNext = NULL;
            l_pNew_Cell->m_pPrev = NULL;

            Put_After(l_pList_Cell, l_pNew_Cell);
        }

        return static_cast<pAList_Cell>(l_pNew_Cell);
    }//CListBase::Add_After


    ////////////////////////////////////////////////////////////////////////////
    //CListBase::Put_After
    inline tBOOL Put_After(pAList_Cell i_pList_Cell, pAList_Cell i_pExt_Cell)
    {
        tCell   *l_pList_Cell = static_cast<tCell*>(i_pList_Cell);
        tCell   *l_pExt_Cell  = static_cast<tCell*>(i_pExt_Cell);
        tBOOL           l_bResult    = TRUE;

        if (NULL == l_pExt_Cell)
        {
            return FALSE;
        }

        m_dwCount++;

        if (l_pList_Cell)
        {
            l_pExt_Cell->m_pNext = l_pList_Cell->m_pNext;
            l_pExt_Cell->m_pPrev = l_pList_Cell;
            if (l_pList_Cell->m_pNext)
            {
                l_pList_Cell->m_pNext->m_pPrev = l_pExt_Cell;
            }
            l_pList_Cell->m_pNext = l_pExt_Cell;
        }


        if ((NULL == l_pList_Cell) && (m_pFirst))
        {
            m_pFirst->m_pPrev = l_pExt_Cell;
            l_pExt_Cell->m_pNext = m_pFirst;
            m_pFirst = l_pExt_Cell;
        }

        if ((m_pLast) && (m_pLast == l_pList_Cell))
        {
            m_pLast = l_pExt_Cell;
        }

        if (!m_pFirst)
        {
            m_pFirst = l_pExt_Cell;
            m_pLast = l_pExt_Cell;
        }

        m_bBrokenIndex = TRUE;
     
        return l_bResult;
    }//CListBase::Put_After


    ////////////////////////////////////////////////////////////////////////////
    //CListBase::Get_Prev
    inline pAList_Cell Get_Prev(pAList_Cell i_pCell)
    {
        tCell *l_pCell = static_cast<tCell*>(i_pCell);
        pAList_Cell  l_pResult = NULL;
        
        if (l_pCell)
        {
            l_pResult = static_cast<pAList_Cell>(l_pCell->m_pPrev);
        }
        else
        {
            l_pResult = static_cast<pAList_Cell>(m_pLast);
        }

        return l_pResult;
    }//CListBase::Get_Prev


    ////////////////////////////////////////////////////////////////////////////
    //CListBase::Get_Next
    inline pAList_Cell Get_Next(pAList_Cell i_pCell)
    {
        tCell *l_pCell = static_cast<tCell *>(i_pCell);
        pAList_Cell  l_pResult = NULL;
        
        if (l_pCell)
        {
            l_pResult = static_cast<pAList_Cell>(l_pCell->m_pNext);
        }
        else
        {
            l_pResult = static_cast<pAList_Cell>(m_pFirst);
        }

        return l_pResult;
    }//CListBase::Get_Next

    ////////////////////////////////////////////////////////////////////////////
    //CListBase::Get_Data
    inline tData_Type Get_Data(pAList_Cell i_pCell)
    {
        tCell *l_pCell = static_cast<tCell*>(i_pCell);
        tData_Type   l_pResult = 0;
        if (l_pCell)
        {
            l_pResult = l_pCell->m_pData;
        }

        return l_pResult;
    }//CListBase::Get_Data

    
    ////////////////////////////////////////////////////////////////////////////
    //CListBase::Set_Data
    inline tBOOL Set_Data(pAList_Cell i_pCell, tBOOL i_bFree_Old_Data, tData_Type i_pData)
    {
        tCell      *l_pCell   = static_cast<tCell*>(i_pCell);
        tData_Type  l_pResult = NULL;
        if (l_pCell)
        {
            if (i_bFree_Old_Data)
            {
                Data_Release(l_pCell->m_pData);
            }

            l_pCell->m_pData = i_pData;
        }

        return TRUE;
    }//CListBase::Set_Data


    ////////////////////////////////////////////////////////////////////////////
    //CListBase::Get_ByIndex
    inline pAList_Cell Get_ByIndex(tUINT32 i_dwIDX)
    {
        tCell *l_pReturn   = NULL;
        tUINT32  l_dwI;

        if (m_bBrokenIndex)
        {
            Index_Build();
        }

        if  (i_dwIDX < m_dwCount) 
        {
            if (    (FALSE == m_bBrokenIndex)
                 && (m_pIndex) 
               )
            {
                l_pReturn = m_pIndex[i_dwIDX];
            }
            else
            {
                l_pReturn = m_pFirst;
                for (l_dwI = 0; l_dwI < i_dwIDX; l_dwI++)
                {
                    if (l_pReturn)
                    {
                        l_pReturn = l_pReturn->m_pNext;
                    }
                    else
                    {
                        break;
                    }
                }
            }
        }

        return l_pReturn;
    }//CListBase::Get_ByIndex


    ////////////////////////////////////////////////////////////////////////////
    //CListBase::operator[]
    inline tData_Type operator[](tUINT32 i_dwIDX)
    {
        tCell *l_pReturn = static_cast<tCell*>(Get_ByIndex(i_dwIDX));

        return (l_pReturn) ? l_pReturn->m_pData : NULL;
    }//CListBase::operator[]


    ////////////////////////////////////////////////////////////////////////////
    //CListBase::Put_Data
    inline tBOOL Put_Data(pAList_Cell i_pCell, tData_Type i_pData, tBOOL i_bFree_Old_Data)
    { 
        tBOOL  l_bResult = TRUE;
        tCell *l_pCell   = static_cast<tCell*>(i_pCell);
        if (l_pCell)
        {
            if ( i_bFree_Old_Data )
            {
                Data_Release(l_pCell->m_pData);
                l_pCell->m_pData = NULL;
            }

            l_pCell->m_pData = i_pData;
        }
        else
        {
            l_bResult = FALSE;
        }


        return l_bResult;
    }//CListBase::Put_Data

    
    ////////////////////////////////////////////////////////////////////////////
    //CListBase::Lock
    //void Lock()
    //{
    //    WaitForSingleObject(m_hLock_Mutex,  INFINITE);        
    //}//CListBase::Lock


    ////////////////////////////////////////////////////////////////////////////
    //CListBase::UnLock
    //void UnLock()
    //{
    //    ReleaseMutex(m_hLock_Mutex);
    //}//CListBase::UnLock


    ////////////////////////////////////////////////////////////////////////////
    //CListBase::Clear
    inline void Clear(tBOOL i_bClearData)
    {
        while (NULL != m_pFirst)
        {
            if (TRUE != Cell_Release(m_pFirst, TRUE, i_bClearData))
            {
                break;
            }
        }
    }//CListBase::Clear


    ////////////////////////////////////////////////////////////////////////////
    //CListBase::Get_First
    inline pAList_Cell Get_First()
    {
        return static_cast<pAList_Cell>(m_pFirst);
    }//CListBase::Get_First


    ////////////////////////////////////////////////////////////////////////////
    //CListBase::Get_Last
    inline pAList_Cell Get_Last()
    {
        return static_cast<pAList_Cell>(m_pLast);
    }//CListBase::Get_Last


    ////////////////////////////////////////////////////////////////////////////
    //CListBase::Count
    inline tUINT32 Count()
    {
        return m_dwCount;
    }//CListBase::Count

private:
    ////////////////////////////////////////////////////////////////////////////
    //CListBase::Cell_Release
    inline tBOOL Cell_Release(tCell *i_pCell, tBOOL i_bFree_Cell, tBOOL i_bFree_Data)
    {
        tBOOL l_bResult = TRUE;

        if (NULL == i_pCell)       
        {
            return FALSE;
        }

        if ((NULL != i_pCell->m_pPrev) && (NULL != i_pCell->m_pNext)) 
        {
            i_pCell->m_pPrev->m_pNext = i_pCell->m_pNext;
            i_pCell->m_pNext->m_pPrev = i_pCell->m_pPrev;
        }
        else
        { 
            l_bResult = FALSE;

            if (i_pCell == m_pFirst)
            {
                m_pFirst = i_pCell->m_pNext;

                if (m_pFirst != 0)
                {
                    m_pFirst->m_pPrev = 0;
                }

                l_bResult = TRUE;  
            }

            if (i_pCell == m_pLast)
            {
                m_pLast = i_pCell->m_pPrev;
                if (m_pLast != 0) 
                {
                    m_pLast->m_pNext = 0; 
                }

                l_bResult = TRUE;  
            }
        }

        if (TRUE == l_bResult) 
        {
            if (i_bFree_Data)
            {
                Data_Release(i_pCell->m_pData);
                i_pCell->m_pData = 0;
            }

            if (i_bFree_Cell)
            {
                Cell_Free(i_pCell);
            }
        }

        m_bBrokenIndex = TRUE;

        m_dwCount--;

        return l_bResult;
    }//CListBase::Cell_Release


    ////////////////////////////////////////////////////////////////////////////
    //CListBase::Index_Build
    inline tBOOL Index_Build()
    {
        tBOOL l_bResult = TRUE;

        if ( (m_pIndex) && (m_dwCount > m_dwIndexCount) )
        {
            Index_Release();
        }

        if (NULL == m_pIndex)
        {
            m_dwIndexCount = m_dwCount + 128;
            m_pIndex = (pIndex_Cell*)this->MemAlloc(sizeof(pIndex_Cell) * m_dwIndexCount);
        }

        if (NULL == m_pIndex)
        {
            l_bResult = FALSE;
            m_dwIndexCount = 0;
        }

        if (TRUE == l_bResult)
        {
            pAList_Cell l_pEl = NULL;

            memset(m_pIndex, 0, m_dwIndexCount * sizeof(pIndex_Cell));
            
            pIndex_Cell *l_pIndexLocal = m_pIndex;

            while ((l_pEl = Get_Next(l_pEl)) != NULL)
            {
               *l_pIndexLocal = static_cast<tCell*>(l_pEl);
               l_pIndexLocal ++;
            }

            m_bBrokenIndex = FALSE;
        }

        return l_bResult;
    }//CListBase::Index_Build


    ////////////////////////////////////////////////////////////////////////////
    //CListBase::Index_Release
    inline tBOOL Index_Release()
    {
        if (m_pIndex)
        {
            this->MemFree(m_pIndex);
            m_pIndex = NULL;
        }

        m_dwIndexCount = 0;
        m_bBrokenIndex = TRUE;
        return TRUE;
    }//CListBase::Index_Release

protected:
    ////////////////////////////////////////////////////////////////////////////
    //CListBase::Data_Release
    // override this function in derived class to implement specific data 
    // destructor(structures as example)
    virtual tBOOL Data_Release(tData_Type i_pData) = 0;
    
    ////////////////////////////////////////////////////////////////////////////
    //CListBase::MemAlloc
    //override this function in derived class to implement own memory allocation
    //mechanism, it will be used everywhere in this class
    virtual void *MemAlloc(size_t i_szSize)
    {
        return new tUINT8[i_szSize];
    }//CListBase::MemAlloc

    
    ////////////////////////////////////////////////////////////////////////////
    //CListBase::MemFree
    //override this function in derived class to implement own memory deallocation
    //mechanism, it will be used everywhere in this class
    virtual void MemFree(void * i_pMemory)
    {
        delete [] ((tUINT8*)i_pMemory);
    }//CListBase::MemFree

    
    ////////////////////////////////////////////////////////////////////////////
    //CListBase::Cell_Allocate
    virtual tCell *Cell_Allocate()
    {
        return (tCell*)this->MemAlloc(sizeof(tCell));
    }//Cell_Allocate

    
    ////////////////////////////////////////////////////////////////////////////
    //CListBase::Cell_Free
    virtual void Cell_Free(tCell *i_pCell)
    {
        this->MemFree(i_pCell);
    }//Cell_Free
};

////////////////////////////////////////////////////////////////////////////////
//CAList
template <typename tData_Type>
class CAList
        : public CListBase<tData_Type>
{
protected:
    ////////////////////////////////////////////////////////////////////////////
    //CListBase::Data_Release
    // override this function in derived class to implement specific data
    // destructor(structures as example)
    virtual tBOOL Data_Release(tData_Type i_pData)
    {
        if (NULL == i_pData)
        {
            return FALSE;
        }

        delete i_pData;
        return TRUE;
    }//CListBase::Data_Release
};


////////////////////////////////////////////////////////////////////////////////
//This list has pool for list's cells, this technology speedup allocation and 
//deallocation of new list's elements (6-10 times).
//List intended for situation when you has very intensive adding & deleting 
//process, but it will use a lot of memory if you will add 1 million items and
//then delete 999 thousands, after that all 999 thousands internal list cells
//will be stored in internal pool, for the time being mechanism of pool packing
//is not developed.
//N.B.: be careful with list, use it only when you absolutely sure in what are 
//      you doing.
template <typename tData_Type>
class CListPool:
    public CListBase<tData_Type>
{
private:
    #define TCELL typename CListBase<tData_Type>::tCell
    
    struct tPool_Segment
    {
        TCELL         *m_pCells;
        tUINT32        m_dwCount;
        tPool_Segment *m_pNext;
    };

    tPool_Segment  *m_pPool_First;
    TCELL          *m_pPool_Cell_First;

    //count of cell allocated for every pool segment
    tUINT32         m_dwPool_Size; 
public:
    ////////////////////////////////////////////////////////////////////////////
    //CListPool::CListPool
    CListPool(tUINT32 i_dwPool_Size = 128)
       : CListBase<tData_Type>()
       , m_dwPool_Size(i_dwPool_Size)
    {
        m_pPool_First      = NULL;
        m_pPool_Cell_First = NULL;
    }//CListPool::CListPool


    ////////////////////////////////////////////////////////////////////////////
    //CListPool::~CListPool
    virtual ~CListPool()
    {
        tPool_Segment *l_pPool_Tmp = NULL;
        while (m_pPool_First)
        {
            l_pPool_Tmp = m_pPool_First;
            m_pPool_First = m_pPool_First->m_pNext;

            Free_Pool_Segment(l_pPool_Tmp);
        }
    }//CListPool::~CListPool

private:
    ////////////////////////////////////////////////////////////////////////////
    //CListPool::Create_Pool_Segment
    inline tBOOL Create_Pool_Segment()
    {
        tBOOL l_bReturn = FALSE;

        tPool_Segment *l_pPool = (tPool_Segment*)CListBase<tData_Type>::MemAlloc(sizeof(tPool_Segment));
        
        if (l_pPool)
        {
            memset(l_pPool, 0, sizeof(tPool_Segment));
            l_pPool->m_dwCount = m_dwPool_Size;
            l_pPool->m_pCells  = (TCELL*)this->MemAlloc(sizeof(TCELL) * l_pPool->m_dwCount);
            
            if (l_pPool->m_pCells)
            {
                TCELL *l_pCell = NULL;

                memset(l_pPool->m_pCells, 
                       0, 
                       sizeof(tPool_Segment) * l_pPool->m_dwCount
                      );

                l_pCell = l_pPool->m_pCells;

                for (tUINT32 l_dwIDX = 1; l_dwIDX < l_pPool->m_dwCount; l_dwIDX++)
                {
                    l_pCell->m_pNext = (l_pCell + 1);
                    l_pCell ++;
                }

                l_bReturn          = TRUE;
                l_pPool->m_pNext   = m_pPool_First;
                m_pPool_First      = l_pPool;
                l_pCell->m_pNext   = m_pPool_Cell_First;
                m_pPool_Cell_First = l_pPool->m_pCells;
            }
        }

        if (FALSE == l_bReturn)
        {
            Free_Pool_Segment(l_pPool);
        }

        return l_bReturn;
    }//CListPool::Create_Pool_Segment


    ////////////////////////////////////////////////////////////////////////////
    //CListPool::Free_Pool_Segment
    inline void Free_Pool_Segment(tPool_Segment *io_pPool)
    {
        if (io_pPool)
        {
            if (io_pPool->m_pCells)
            {
                this->MemFree(io_pPool->m_pCells);
                io_pPool->m_pCells = NULL;
            }
            this->MemFree(io_pPool);
        }
    }//CListPool::Free_Pool_Segment

protected:
    ////////////////////////////////////////////////////////////////////////////
    //CListPool::Cell_Allocate
    virtual TCELL *Cell_Allocate()
    {
        TCELL *l_pReturn = NULL;
        if (NULL == m_pPool_Cell_First)
        {
            Create_Pool_Segment();
        }

        l_pReturn = m_pPool_Cell_First;

        if (m_pPool_Cell_First)
        {
            m_pPool_Cell_First = m_pPool_Cell_First->m_pNext;
        }

        return l_pReturn;
    }//CListPool::Cell_Allocate

    
    ////////////////////////////////////////////////////////////////////////////
    //CListPool::Cell_Free
    virtual void Cell_Free(TCELL *i_pCell)
    {
        memset(i_pCell, 0, sizeof(TCELL));
        i_pCell->m_pNext   = m_pPool_Cell_First;
        m_pPool_Cell_First = i_pCell;
    }//CListPool::Cell_Free
};


////////////////////////////////////////////////////////////////////////////////
//CBList
template <typename tData_Type>
class CBList
        : public CListPool<tData_Type>
{
public:
    ////////////////////////////////////////////////////////////////////////////
    //CListPool::CListPool
    CBList(tUINT32 i_dwPool_Size = 128)
       : CListPool<tData_Type>(i_dwPool_Size)
    {
    }//CBList::CBList

protected:
    ////////////////////////////////////////////////////////////////////////////
    //CListBase::Data_Release
    // override this function in derived class to implement specific data
    // destructor(structures as example)
    virtual tBOOL Data_Release(tData_Type i_pData)
    {
        if (NULL == i_pData)
        {
            return FALSE;
        }

        delete i_pData;
        return TRUE;
    }//CListBase::Data_Release
};

#endif //ALIST_H
