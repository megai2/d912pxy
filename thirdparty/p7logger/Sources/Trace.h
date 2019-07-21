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
// This file provide trace functionality                                       /
////////////////////////////////////////////////////////////////////////////////
#ifndef TRACE_H
#define TRACE_H

#define P7TRACE_ITEM_BLOCK_ASTRING                                          (-1)
#define P7TRACE_ITEM_BLOCK_USTRING16                                        (-2)
#define P7TRACE_ITEM_BLOCK_USTRING8                                         (-3)
#define P7TRACE_ITEM_BLOCK_USTRING32                                        (-4)
#define P7TRACE_ITEM_BLOCK_ASTRING_FIX                                      (-5)
#define P7TRACE_ITEM_BLOCK_WSTRING_FIX                                      (-6)

#define P7TRACE_KEY_LENGTH                                                   (2)

//#define P7TRACE_MAP_FILE_NAME                                  L"Local\\P7Trace"
//#define P7TRACE_MAP_FILE_SIZE          (sizeof(CP7Trace*) + sizeof(IP7_Client*))

#define P7TRACE_THREADS_POOL_SIZE                                           (32)
#define P7TRACE_THREADS_MAX_SIZE                                           (128)

#define P7TRACE_MODULES_POOL_SIZE                                           (32)
#define P7TRACE_MODULES_MAX_SIZE                                           (512)
#define P7TRACE_MEMORY_ALIGNMENT                                             (8)


////////////////////////////////////////////////////////////////////////////////
//Memory manager, used for trace descriptions, avoid calling New & malloc
class CMemoryManager
{
public:
    enum eBuffer
    {
        eFileName,
        eFunctionName,
        eArguments,
        eVaValues,
        eCount
    };

protected:
    struct sBlock
    {
        void   *pData;
        sBlock *pNext;
    };

    struct sBuffer
    {
        void   *pData;
        size_t  szBuffer;
    };

    sBlock *m_pCurrent; //dynamic, internal buffers - allocated once
    size_t  m_szCurrent;
    size_t  m_szCurrentUsed;
    size_t  m_szTotal;
    size_t  m_szMax;
    size_t  m_szRecommended;
    sBlock *m_pBusy;
    sBuffer m_pBuffers[CMemoryManager::eCount]; //static buffers, reusable

public:
    ////////////////////////////////////////////////////////////////////////////
    //CMemoryManager
    CMemoryManager(size_t i_szInitial, size_t i_szMax = 0xFFFFFFF)
        : m_pCurrent(NULL)
        , m_szCurrent(i_szInitial)
        , m_szCurrentUsed(0)
        , m_szTotal(i_szInitial)
        , m_szMax(i_szMax)
        , m_szRecommended(i_szInitial)
        , m_pBusy(NULL)
    {
        memset(m_pBuffers, 0, sizeof(m_pBuffers));
        m_pCurrent = (sBlock*)malloc(m_szCurrent + sizeof(sBlock));
        if (m_pCurrent)
        {
            m_pCurrent->pData = (tUINT8*)m_pCurrent + sizeof(sBlock);
            m_pCurrent->pNext = NULL;
        }
    }//CMemoryManager


    ////////////////////////////////////////////////////////////////////////////
    //~CMemoryManager
    ~CMemoryManager()
    {
        while (m_pCurrent)
        {
            sBlock *l_pTmp = m_pCurrent;
            m_pCurrent = m_pCurrent->pNext;
            free(l_pTmp);
        }

        while (m_pBusy)
        {
            sBlock *l_pTmp = m_pBusy;
            m_pBusy = m_pBusy->pNext;
            free(l_pTmp);
        }

        for (size_t l_szI = 0; l_szI < LENGTH(m_pBuffers); l_szI++)
        {
            if (m_pBuffers[l_szI].pData)
            {
                free(m_pBuffers[l_szI].pData);
                m_pBuffers[l_szI].pData = NULL;
            }
        }
    }//~CMemoryManager

    ////////////////////////////////////////////////////////////////////////////
    //Reuse - function is used for reusable temporary buffers
    void *Reuse(CMemoryManager::eBuffer i_eBuffer, size_t i_szBuffer)
    {
        sBuffer *l_pBuffer = &m_pBuffers[i_eBuffer];

        if (    (l_pBuffer->pData)
             && (l_pBuffer->szBuffer >= i_szBuffer)
           )
        {
            return l_pBuffer->pData;
        }

        size_t l_szNew = (i_szBuffer + (size_t)255) & (~(size_t)255);
        void  *l_pNew  = realloc(l_pBuffer->pData, l_szNew);
        if (l_pNew)
        {
            l_pBuffer->pData    = l_pNew;
            l_pBuffer->szBuffer = l_szNew;
        }

        return l_pNew;
    }//Reuse

    ////////////////////////////////////////////////////////////////////////////
    //Size
    size_t Size(CMemoryManager::eBuffer i_eBuffer)
    {
        return m_pBuffers[i_eBuffer].szBuffer;
    }//Size

    ////////////////////////////////////////////////////////////////////////////
    //Alloc - function is used for constant buffers used until class is a live
    void *Alloc(size_t i_szBuffer)
    {
        tUINT8 *l_pReturn = NULL;

        i_szBuffer = (i_szBuffer + P7TRACE_MEMORY_ALIGNMENT - 1) & ~(P7TRACE_MEMORY_ALIGNMENT - 1);

        if (    (m_pCurrent)
             && ((m_szCurrent - m_szCurrentUsed) >= i_szBuffer)
           )
        {
            l_pReturn = (tUINT8*)m_pCurrent->pData + m_szCurrentUsed;
            m_szCurrentUsed += i_szBuffer;
        }
        else if ((m_szTotal + i_szBuffer) <= m_szMax)
        {
            //add current buffer to busy queue
            m_pCurrent->pNext = m_pBusy;
            m_pBusy           = m_pCurrent;

            //allocate new buffer
            m_szCurrentUsed = 0;
            m_szCurrent     = (i_szBuffer <= m_szRecommended) ? m_szRecommended : i_szBuffer;
            m_pCurrent      = (sBlock*)malloc(sizeof(sBlock) + m_szCurrent);

            if (m_pCurrent)
            {
                m_pCurrent->pData  = (tUINT8*)m_pCurrent + sizeof(sBlock);
                m_pCurrent->pNext  = NULL;
                l_pReturn          = (tUINT8*)m_pCurrent->pData;
                m_szCurrentUsed   += i_szBuffer;
                m_szTotal         += m_szCurrent;
            }
        }

        return l_pReturn;
    }//Alloc
};

////////////////////////////////////////////////////////////////////////////////
//sData_Block
template <typename tData_Type>
struct sData_Block
{
    tUINT32     dwCount;
    tUINT32     dwUsed;
    tData_Type *pData;

    sData_Block(tUINT32 i_dwCount)
        : dwCount(i_dwCount)
        , dwUsed(0)
        , pData((tData_Type*)malloc(i_dwCount * sizeof(tData_Type)))
    {
        if (pData)
        {
            memset(pData, 0, i_dwCount * sizeof(tData_Type));
        }
    }

    ~sData_Block()
    {
        if (pData)
        {
            free(pData);
            pData = NULL;
        }

        dwCount = 0;
        dwUsed  = 0;
    }
};//sData_Block


////////////////////////////////////////////////////////////////////////////////
//CP7Trace_Desc
class CP7Trace_Desc
{
    tUINT16        m_wID;
    tUINT16        m_wModuleID;
    //count of connections drops... see sP7C_Status. If this value and 
    //value from IP7_Client are different - it mean we loose connection
    tUINT32        m_dwResets; 
    tUINT32        m_dwSize;
    tUINT8        *m_pBuffer;//Buffer will contain sP7Trace_Format

    tINT32        *m_pBlocks;
    tINT32         m_dwBlocks_Count;
    sP7Trace_Arg  *m_pArgs;
    tUINT32        m_dwArgs_Count;

    tKeyType       m_pKey[P7TRACE_KEY_LENGTH]; 

    tBOOL          m_bInitialized;
public:
    CP7Trace_Desc(CMemoryManager &i_rMemory,
                  tUINT16         i_wID,
                  tUINT16         i_wLine, 
                  tUINT16         i_wModuleID,
                  const char     *i_pFile,
                  const char     *i_pFunction,
                  const tXCHAR  **i_pFormat,
                  tKeyType        i_pKeys[P7TRACE_KEY_LENGTH],
                  tUINT32         i_dwFlags
                 );

    CP7Trace_Desc(CMemoryManager &i_rMemory,
                  tUINT16         i_wID,
                  tUINT16         i_wLine, 
                  tUINT16         i_wModuleID,
                  const tXCHAR   *i_pFile,
                  const tXCHAR   *i_pFunction,
                  tKeyType        i_pKeys[P7TRACE_KEY_LENGTH]
                 );

    ~CP7Trace_Desc() {}

    tBOOL    Is_Initialized();
    tUINT8  *Get_Buffer(tUINT32 *o_pSize);
    tINT32  *Get_Blocks(tUINT32 *o_pCount);
    void     Set_Resets(tUINT32 i_dwDrops);
    tUINT32  Get_Resets();
    tUINT32  Get_Arguments_Count();
    const sP7Trace_Arg *Get_Arguments(tUINT32 &o_rCount);
    tBOOL    Is_Equal(tKeyType *i_pKey);
    tBOOL    Is_Greater(tKeyType *i_pKey);

    tUINT16  Get_ID();
    tUINT16  Get_MID();
};//CP7Trace_Desc


////////////////////////////////////////////////////////////////////////////////
//CDesc_Tree
class CDesc_Tree:
    public CRBTree<CP7Trace_Desc*, tKeyType*>
{
public:
    ////////////////////////////////////////////////////////////////////////////
    //~CDesc_Tree
    virtual ~CDesc_Tree()
    {
        Clear();
    }//~CRBTree

protected:
    //////////////////////////////////////////////////////////////////////////// 
    virtual tBOOL Data_Release(CP7Trace_Desc* i_pData)
    {
        UNUSED_ARG(i_pData);
        return TRUE; //memory used from pool, not necessary to return
    }// Data_Release

    //////////////////////////////////////////////////////////////////////////// 
    //Return
    //TRUE  - if (i_pKey < i_pData::key)
    //FALSE - otherwise
    tBOOL Is_Key_Less(tKeyType *i_pKey, CP7Trace_Desc *i_pData) 
    {
        return i_pData->Is_Greater(i_pKey);
    }

    //////////////////////////////////////////////////////////////////////////// 
    //Return
    //TRUE  - if (i_pKey == i_pData::key)
    //FALSE - otherwise
    tBOOL Is_Qual(tKeyType *i_pKey, CP7Trace_Desc *i_pData) 
    {
        return i_pData->Is_Equal(i_pKey);
    }
};//CDesc_Tree



////////////////////////////////////////////////////////////////////////////////
//sModuleMap
struct sModuleMap
{
    sP7Trace_Module *pModule;
    tXCHAR          *pName;

    sModuleMap(const tXCHAR *i_pName, sP7Trace_Module *i_pModule)
        : pModule(i_pModule)
        , pName(PStrDub(i_pName))
    {
    }

    ~sModuleMap()
    {
        if (pName)
        {
            PStrFreeDub(pName);
            pName = NULL;
        }

        pModule = NULL;
    }
};//sModuleMap


////////////////////////////////////////////////////////////////////////////////
//CModules_Tree
class CModules_Tree:
    public CRBTree<sModuleMap*, const tXCHAR*>
{
public:
    CModules_Tree()
        : CRBTree<sModuleMap*, const tXCHAR*>(16, TRUE)
    {
    }

protected:
    //////////////////////////////////////////////////////////////////////////// 
    //Return
    //TRUE  - if (i_pKey < i_pData::key)
    //FALSE - otherwise
    tBOOL Is_Key_Less(const tXCHAR *i_pKey, sModuleMap *i_pData) 
    {
        return (0 < PStrCmp(i_pKey, i_pData->pName)) ? TRUE : FALSE;
    }

    //////////////////////////////////////////////////////////////////////////// 
    //Return
    //TRUE  - if (i_pKey == i_pData::key)
    //FALSE - otherwise
    tBOOL Is_Qual(const tXCHAR *i_pKey, sModuleMap *i_pData) 
    {
        return (0 == PStrCmp(i_pKey, i_pData->pName)) ? TRUE : FALSE;
    }
};//CModules_Tree


////////////////////////////////////////////////////////////////////////////////
#define P7_TRACE_DESC_HARDCODED_COUNT                                     (1024)


class CP7Trace
    : public IP7_Trace
{
    struct sStack_Desc
    {
        enum eType
        {
            eTypeU32,
            eTypeU64,
            eTypeD64
        };

        union uVal
        {
            tUINT32 dw32;
            tUINT64 dw64;
            tDOUBLE db64;
        };

        CP7Trace::sStack_Desc::uVal  uValue;
        CP7Trace::sStack_Desc::eType eType;
        tBOOL bLast;
    };

    typedef sData_Block<sP7Trace_Thread_Start> sThreadsR; //running threads
    typedef sData_Block<sP7Trace_Thread_Stop>  sThreadsS; //stopped threads
    typedef sData_Block<sP7Trace_Module>       sModules;  //modules


    //put volatile variables at the top, to obtain 32 bit alignment. 
    //Project has 8 bytes alignment by default
    tINT32 volatile    m_lReference;
    tUINT32            m_dwSequence;
                      
    IP7_Client        *m_pClient; 
    tUINT32            m_dwChannel_ID;
    CP7Trace_Desc     *m_pDesc_Array[P7_TRACE_DESC_HARDCODED_COUNT];
    CDesc_Tree         m_cDescU_Tree;
    CDesc_Tree         m_cDescM_Tree;
    tUINT16            m_wDesc_Tree_ID;
                      
    tLOCK              m_sCS; 
    tUINT32            m_dwLast_ID;
    tBOOL              m_bInitialized;
    tBOOL              m_bActive;
    eP7Trace_Level     m_eVerbosity;
                      
    sP7Trace_Info      m_sHeader_Info;
    sP7Trace_Data      m_sHeader_Data;
                      
    sP7C_Status        m_sStatus;
                      
    sP7C_Data_Chunk   *m_pChk_Head;
    sP7C_Data_Chunk   *m_pChk_Tail;
    sP7C_Data_Chunk   *m_pChk_Curs;
    tUINT32            m_dwChk_Count;
    tUINT32            m_dwChk_Size;
                      
    tBOOL              m_bIs_Channel;
    tUINT32            m_dwFlags;

    CAList<sThreadsR*> m_cThreadsR;
    CAList<sThreadsS*> m_cThreadsS;

    tUINT16            m_wLast_ModuleID;
    CAList<sModules*>  m_cModules;
    CModules_Tree      m_cModules_Map;

    CShared::hShared   m_hShared;
    CMemoryManager     m_cMemory;

    tUINT8            *m_pVargs;
    size_t             m_szVargs;

    tUINT8             m_pExtensions[4];

    stTrace_Conf       m_sConf;

public:
    CP7Trace(IP7_Client         *i_pClient, 
             const tXCHAR       *i_pName, 
             const stTrace_Conf *i_pConf); //for arguments description see P7_Client.h
    virtual ~CP7Trace();

    tBOOL Is_Initialized();

    IP7C_Channel::eType Get_Type() { return IP7C_Channel::eTrace; }
    void  On_Init(sP7C_Channel_Info *i_pInfo);
    void  On_Receive(tUINT32 i_dwChannel, tUINT8 *i_pBuffer, tUINT32 i_dwSize, tBOOL i_bBigEndian);
    void  On_Status(tUINT32 i_dwChannel, const sP7C_Status *i_pStatus);
    void  On_Flush(tUINT32 i_dwChannel, tBOOL *io_pCrash);

    tBOOL Register_Thread(const tXCHAR *i_pName, tUINT32 i_dwThreadId);
    tBOOL Unregister_Thread(tUINT32 i_dwThreadId);

    tBOOL Register_Module(const tXCHAR *i_pName, IP7_Trace::hModule *o_hModule);

    void  Set_Verbosity(IP7_Trace::hModule i_hModule, eP7Trace_Level i_eVerbosity);

    tBOOL Trace(tUINT16            i_wTrace_ID,   
                eP7Trace_Level     i_eLevel, 
                IP7_Trace::hModule i_hModule,
                tUINT16            i_wLine,
                const char        *i_pFile,
                const char        *i_pFunction,
                const tXCHAR      *i_pFormat,
                ...
               );

    tBOOL Trace_Embedded(tUINT16            i_wTrace_ID,   
                         eP7Trace_Level     i_eLevel, 
                         IP7_Trace::hModule i_hModule,
                         tUINT16            i_wLine,
                         const char        *i_pFile,
                         const char        *i_pFunction,
                         const tXCHAR     **i_ppFormat
                        );

    tBOOL Trace_Embedded(tUINT16            i_wTrace_ID,   
                         eP7Trace_Level     i_eLevel, 
                         IP7_Trace::hModule i_hModule,
                         tUINT16            i_wLine,
                         const char        *i_pFile,
                         const char        *i_pFunction,
                         const tXCHAR     **i_ppFormat,
                         va_list           *i_pVa_List
                        );

    tBOOL Trace_Managed(tUINT16            i_wTrace_ID,   
                        eP7Trace_Level     i_eLevel, 
                        IP7_Trace::hModule i_hModule,
                        tUINT16            i_wLine,
                        const tXCHAR      *i_pFile,
                        const tXCHAR      *i_pFunction,
                        const tXCHAR      *i_pMessage
                       );

    virtual tINT32 Add_Ref()
    {
        return ATOMIC_INC(&m_lReference);
    }

    virtual tINT32 Release()
    {
        tINT32 l_lResult = ATOMIC_DEC(&m_lReference);
        if ( 0 >= l_lResult )
        {
            delete this;
        }

        return l_lResult;
    }

    tBOOL Share(const tXCHAR *i_pName);

private:
    void  Flush();

    tBOOL Trace_Raw(tUINT16            i_wTrace_ID,   
                    eP7Trace_Level     i_eLevel, 
                    IP7_Trace::hModule i_hModule,
                    tUINT16            i_wLine,
                    const char        *i_pFile,
                    const char        *i_pFunction,
                    tKeyType          *i_pKey,
                    const tXCHAR     **i_ppFormat,
                    va_list           *i_pVa_List
                );

    tBOOL Inc_Chunks(tUINT32 i_dwInc);

    tBOOL Is_VarArgs(const CP7Trace::sStack_Desc *i_pDesc, ...);
};

#endif //TRACE_H
