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
//Printf Type Field Characters:                                                /
// http://msdn.microsoft.com/en-us/library/hf4y5e3w.aspx                       /
//Printf Type Field Characters:                                                /
// http://msdn.microsoft.com/en-us/library/hf4y5e3w(v=vs.100).aspx             /
//Size and Distance Specification                                              /
// http://msdn.microsoft.com/en-us/library/tcxf1dw6(v=vs.100).aspx             /
////////////////////////////////////////////////////////////////////////////////
#include "CommonClient.h"
#include "Trace.h"
#include <wchar.h>
#include <stdint.h>

#define RESET_UNDEFINED                                           (0xFFFFFFFFUL)
#define RESET_FLAG_CHANNEL                                        (0x1)
#define RESET_FLAG_TRACE                                          (0x2)

#define FORMAT_CRASH_MESSAGE          TM("Abnormal program termination, stack:")
#define FORMAT_STACK_MESSAGE          TM("  ->[%02d] (0x%016I64X) %s, line %d")
#define FORMAT_FILE_UNKNOWN           "-------"
#define FORMAT_FUNC_UNKNOWN           "-------"
#define TRACE_SHARED_PREFIX                                       TM("Trc_")
#define TRACE_EXTRA_CHUNKS                                        (5)

#if !defined(UINTMAX_MAX)
    typedef uint64_t uintmax_t;
    #define UINTMAX_MAX	0xffffffffffffffffU
#endif

#if !defined(INTMAX_MAX)
    typedef int64_t intmax_t;
     #define INTMAX_MIN		(-0x7fffffffffffffff - 1)
     #define INTMAX_MAX		0x7fffffffffffffff
#endif

#define ADD_ARG(i_bType, i_bSize, oResult)\
{\
    oResult = FALSE;\
    if (m_dwArgs_Count < l_dwArgs_Max)\
    {\
        l_pArgs[m_dwArgs_Count].bType = i_bType;\
        l_pArgs[m_dwArgs_Count].bSize = i_bSize;\
        m_dwArgs_Count ++;\
        oResult = TRUE;\
    }\
    else if ( m_dwArgs_Count >= l_dwArgs_Max )\
    {\
        tUINT32       l_dwLength = m_dwArgs_Count + 16;\
        sP7Trace_Arg *l_pTMP     = (sP7Trace_Arg*)i_rMemory.Reuse(CMemoryManager::eArguments,\
                                                                  l_dwLength * sizeof(sP7Trace_Arg)\
                                                                  );\
        if (l_pTMP)\
        {\
            l_pArgs      = l_pTMP;\
            l_dwArgs_Max = l_dwLength;\
            l_pArgs[m_dwArgs_Count].bType = i_bType;\
            l_pArgs[m_dwArgs_Count].bSize = i_bSize;\
            m_dwArgs_Count ++;\
            oResult = TRUE;\
        }\
    }\
}

extern "C" 
{

////////////////////////////////////////////////////////////////////////////////
//P7_Create_Client
P7_EXPORT IP7_Trace * __cdecl P7_Create_Trace(IP7_Client         *i_pClient, 
                                              const tXCHAR       *i_pName,
                                              const stTrace_Conf *i_pConf
                                             )
{
    //Check parameters
    if (i_pConf)
    {
        if (    (    (!i_pConf->qwTimestamp_Frequency)
                  && (i_pConf->pTimestamp_Callback)
                )
             || (    (i_pConf->qwTimestamp_Frequency)
                  && (!i_pConf->pTimestamp_Callback)
                )
           )
        {
            return NULL;    
        }
    }

    CP7Trace *l_pReturn = new CP7Trace(i_pClient, i_pName, i_pConf);

    //if not initialized - remove
    if (    (l_pReturn)
         &&  (TRUE != l_pReturn->Is_Initialized())
       )
    {
        l_pReturn->Release();
        l_pReturn = NULL;
    }

    return static_cast<IP7_Trace *>(l_pReturn);
}//P7_Create_Client


////////////////////////////////////////////////////////////////////////////////
//P7_Get_Shared_Trace
P7_EXPORT IP7_Trace * __cdecl P7_Get_Shared_Trace(const tXCHAR *i_pName)
{
    size_t     l_szPadding = 16;
    IP7_Trace *l_pReturn   = NULL;
    tUINT32    l_dwLen1    = PStrLen(TRACE_SHARED_PREFIX);
    tUINT32    l_dwLen2    = PStrLen(i_pName);
    tXCHAR    *l_pName     = (tXCHAR *)malloc(sizeof(tXCHAR) * (l_dwLen1 + l_dwLen2 + l_szPadding));

    if (l_pName)
    {
        PStrCpy(l_pName, l_dwLen1 + l_dwLen2 + l_szPadding, TRACE_SHARED_PREFIX);
        PStrCpy(l_pName + l_dwLen1, l_dwLen2 + l_szPadding, i_pName);
        if (CShared::E_OK == CShared::Lock(l_pName, 250))
        {
            if (CShared::Read(l_pName, (tUINT8*)&l_pReturn, sizeof(IP7_Trace*)))
            {
                if (l_pReturn)
                {
                    l_pReturn->Add_Ref();
                }
            }
            else
            {
                l_pReturn = NULL;
            }
        }

        CShared::UnLock(l_pName);

        free(l_pName);
        l_pName = NULL;
    }

   return l_pReturn;
}//P7_Get_Shared_Trace

} //extern "C" 


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
enum ePrefix_Type
{
    EPREFIX_TYPE_I64   = 0,
    EPREFIX_TYPE_I32      ,
    EPREFIX_TYPE_LL       ,
    EPREFIX_TYPE_L        ,
    EPREFIX_TYPE_HH       ,
    EPREFIX_TYPE_H        ,
    EPREFIX_TYPE_I        ,
    EPREFIX_TYPE_W        ,
    EPREFIX_TYPE_J        ,
    EPREFIX_TYPE_UNKNOWN  
};

struct sPrefix_Desc
{
    const tXCHAR  *pPrefix;
    tUINT32        dwLen;
    ePrefix_Type   eType;
};



//the order of strings IS VERY important, because we will search for first match 
static const sPrefix_Desc g_pPrefixes[] = { {TM("I64"), 3, EPREFIX_TYPE_I64    }, 
                                            {TM("I32"), 3, EPREFIX_TYPE_I32    },
                                            {TM("ll"),  2, EPREFIX_TYPE_LL     },
                                            {TM("hh"),  2, EPREFIX_TYPE_HH     },
                                            {TM("h"),   1, EPREFIX_TYPE_H      },
                                            {TM("l"),   1, EPREFIX_TYPE_L      },
                                            {TM("I"),   1, EPREFIX_TYPE_I      },
                                            {TM("z"),   1, EPREFIX_TYPE_I      },
                                            {TM("t"),   1, EPREFIX_TYPE_I      },
                                            {TM("w"),   1, EPREFIX_TYPE_W      },
                                            {TM("j"),   1, EPREFIX_TYPE_J      },
                                            {TM("?"),   0, EPREFIX_TYPE_UNKNOWN}
                                          };


////////////////////////////////////////////////////////////////////////////////
// Get_Prefix                                       
const sPrefix_Desc *Get_Prefix(const tXCHAR *i_pFormat)
{
    const sPrefix_Desc *l_pPrefix = &g_pPrefixes[0];
    const sPrefix_Desc *l_pReturn = NULL;

    while (l_pPrefix->dwLen)
    {
        if (0 == PStrNCmp(i_pFormat, l_pPrefix->pPrefix, l_pPrefix->dwLen))
        {
            l_pReturn = l_pPrefix;
            break;
        }

        l_pPrefix ++;
    }

    return l_pReturn;
}// Get_Prefix



////////////////////////////////////////////////////////////////////////////////
// CP7Trace_Desc                                       
CP7Trace_Desc::CP7Trace_Desc(CMemoryManager &i_rMemory,
                             tUINT16         i_wID,
                             tUINT16         i_wLine, 
                             tUINT16         i_wModuleID,
                             const char     *i_pFile,
                             const char     *i_pFunction,
                             const tXCHAR  **i_pFormat,
                             tKeyType        i_pKeys[P7TRACE_KEY_LENGTH],
                             tUINT32         i_dwFlags
                            )
    : m_wID(i_wID)
    , m_wModuleID(i_wModuleID)
    , m_dwResets(RESET_UNDEFINED)
    , m_dwSize(0)
    , m_pBuffer(NULL)

    , m_pBlocks(NULL)
    , m_dwBlocks_Count(0)
    , m_pArgs(NULL)
    , m_dwArgs_Count(0)
   
    , m_bInitialized(TRUE)
{
    UNUSED_ARG(i_dwFlags);

    tUINT32       l_dwFile_Size  = 0;
    tUINT32       l_dwFunc_Size  = 0;
    tUINT32       l_dwForm_Size  = 0;
    const tXCHAR *l_pFormat      = *i_pFormat;
    sP7Trace_Arg *l_pArgs        = NULL;  
    tUINT32       l_dwArgs_Max   = 0;
    tUINT8        l_bSize        = 0;
    
    m_bInitialized = (NULL != l_pFormat);

    m_pKey[0] = i_pKeys[0];
    m_pKey[1] = i_pKeys[1];

    if (m_bInitialized)
    {
        ePrefix_Type  l_ePrefix   = EPREFIX_TYPE_UNKNOWN;
        const tXCHAR *l_pIterator = l_pFormat;
        tBOOL         l_bPercent  = FALSE;

        while (    (*l_pIterator)
                && (m_bInitialized)
              )
        {
            if (FALSE == l_bPercent)
            {
                if (TM('%') == (*l_pIterator))
                {
                    //we can get "%%" in this case we should ignore "%"
                    l_bPercent = ! l_bPercent;//TRUE;
                    l_ePrefix  = EPREFIX_TYPE_UNKNOWN;
                    l_bSize    = 0;
                }
            }
            else
            {
                switch (*l_pIterator)
                {
                    case TM('*'):
                    {
                        ADD_ARG(P7TRACE_ARG_TYPE_INT32, SIZE_OF_ARG(tUINT32), m_bInitialized);
                        l_bSize = 1;
                        break;
                    }
                    case TM('I'):
                    case TM('l'):
                    case TM('h'):
                    case TM('w'):
                    case TM('z'):
                    case TM('j'):
                    case TM('t'):
                    {
                        const sPrefix_Desc *l_pPrefix = Get_Prefix(l_pIterator);
                        if (l_pPrefix)
                        {
                            l_ePrefix = l_pPrefix->eType;

                            if (1 < l_pPrefix->dwLen)
                            {
                                l_pIterator += (l_pPrefix->dwLen - 1);
                            }
                        }
                        break;
                    }
                    case TM('d'):
                    case TM('b'):
                    case TM('i'):
                    case TM('o'):
                    case TM('u'):
                    case TM('x'):
                    case TM('X'):
                    {
                        if (EPREFIX_TYPE_UNKNOWN == l_ePrefix)
                        {
                            //4 bytes integer
                            ADD_ARG(P7TRACE_ARG_TYPE_INT32, SIZE_OF_ARG(tUINT32), m_bInitialized);
                        }
                        else if (    (EPREFIX_TYPE_LL  == l_ePrefix)
                                  || (EPREFIX_TYPE_I64 == l_ePrefix)
                                )
                        {
                            //8 bytes integer
                            ADD_ARG(P7TRACE_ARG_TYPE_INT64, SIZE_OF_ARG(tUINT64), m_bInitialized);
                        }
                        else if (EPREFIX_TYPE_H == l_ePrefix)
                        {
                            //2 bytes integer
                            ADD_ARG(P7TRACE_ARG_TYPE_INT16, SIZE_OF_ARG(tUINT16), m_bInitialized);
                        }
                        else if (EPREFIX_TYPE_HH == l_ePrefix)
                        {
                            //1 bytes integer
                            ADD_ARG(P7TRACE_ARG_TYPE_INT8, SIZE_OF_ARG(tUINT8), m_bInitialized);
                        }
                        else if (EPREFIX_TYPE_L == l_ePrefix)
                        {
                        #if defined(__linux__)
                            #ifdef GTX64
                                ADD_ARG(P7TRACE_ARG_TYPE_INT64, SIZE_OF_ARG(tUINT64), m_bInitialized);
                            #else
                                ADD_ARG(P7TRACE_ARG_TYPE_INT32, SIZE_OF_ARG(tUINT32), m_bInitialized);
                            #endif
                        #else
                            ADD_ARG(P7TRACE_ARG_TYPE_INT32, SIZE_OF_ARG(tUINT32), m_bInitialized);
                        #endif
                        }
                        else if  (EPREFIX_TYPE_I32 == l_ePrefix)
                        {
                            //4 bytes integer
                            ADD_ARG(P7TRACE_ARG_TYPE_INT32, SIZE_OF_ARG(tUINT32), m_bInitialized);
                        }
                        else if (EPREFIX_TYPE_I  == l_ePrefix)
                        {
                            #ifdef GTX64
                                ADD_ARG(P7TRACE_ARG_TYPE_INT64, SIZE_OF_ARG(tUINT64), m_bInitialized);
                            #else
                                ADD_ARG(P7TRACE_ARG_TYPE_INT32, SIZE_OF_ARG(tUINT32), m_bInitialized);
                            #endif
                        }
                        else if (EPREFIX_TYPE_J == l_ePrefix)
                        {
                            //8 bytes integer for most of the platforms, truncate for others
                            ADD_ARG(P7TRACE_ARG_TYPE_INTMAX, SIZE_OF_ARG(uintmax_t), m_bInitialized);
                        }
                        else //by default
                        {
                            //4 bytes integer
                            ADD_ARG(P7TRACE_ARG_TYPE_INT32, SIZE_OF_ARG(tUINT32), m_bInitialized);
                        }

                        l_bPercent = FALSE;
                        break;
                    }
                    case TM('s'):
                    case TM('S'):
                    {
                        if (EPREFIX_TYPE_H == l_ePrefix)
                        {
                            ADD_ARG(P7TRACE_ARG_TYPE_STRA, l_bSize, m_bInitialized); //SIZE_OF_ARG(char*)    
                        }
#ifdef UTF8_ENCODING    //%S NOT PORTABLE, DO NOT USE IT!, use %ls or %hs 
                        else if (TM('S') == (*l_pIterator))
                        {
                            ADD_ARG(P7TRACE_ARG_TYPE_USTR32, l_bSize, m_bInitialized); //SIZE_OF_ARG(char*)    
                        }
#else
                        else if (TM('S') == (*l_pIterator))
                        {
                            ADD_ARG(P7TRACE_ARG_TYPE_STRA, l_bSize, m_bInitialized); //SIZE_OF_ARG(char*)    
                        }
#endif                             
                        else if (EPREFIX_TYPE_L == l_ePrefix)
                        {
#ifdef UTF8_ENCODING
                            ADD_ARG(P7TRACE_ARG_TYPE_USTR32, l_bSize, m_bInitialized); //SIZE_OF_ARG(char*)     
#else
                            ADD_ARG(P7TRACE_ARG_TYPE_USTR16, l_bSize, m_bInitialized); //SIZE_OF_ARG(wchar_t*)     
#endif                             
                        }
                        else
                        {
#ifdef UTF8_ENCODING
                            ADD_ARG(P7TRACE_ARG_TYPE_USTR8, l_bSize, m_bInitialized); //SIZE_OF_ARG(char*)     
#else
                            ADD_ARG(P7TRACE_ARG_TYPE_USTR16, l_bSize, m_bInitialized); //SIZE_OF_ARG(wchar_t*)     
#endif                             
                        }

                        l_bPercent = FALSE;
                        break;
                    }
                    //case TM('n'): ignored, not supported
                    case TM('p'):
                    {
                        ADD_ARG(P7TRACE_ARG_TYPE_PVOID, SIZE_OF_ARG(void*), m_bInitialized);
                        l_bPercent = FALSE;
                        break;
                    }
                    case TM('e'):
                    case TM('E'):
                    case TM('f'):
                    case TM('g'):
                    case TM('G'):
                    case TM('a'):
                    case TM('A'):
                    {
                        //8 bytes double
                        ADD_ARG(P7TRACE_ARG_TYPE_DOUBLE, SIZE_OF_ARG(tDOUBLE), m_bInitialized);
                        l_bPercent = FALSE;
                        break;
                    }
                    case TM('c'):
                    case TM('C'):
                    {
                        if (EPREFIX_TYPE_H == l_ePrefix)
                        {
                            //1 bytes character
                            ADD_ARG(P7TRACE_ARG_TYPE_CHAR, SIZE_OF_ARG(char), m_bInitialized);
                        }
                        else if (EPREFIX_TYPE_L == l_ePrefix)
                        {
#ifdef UTF8_ENCODING
                            ADD_ARG(P7TRACE_ARG_TYPE_CHAR32, SIZE_OF_ARG(wchar_t), m_bInitialized); 
#else
                            ADD_ARG(P7TRACE_ARG_TYPE_CHAR16, SIZE_OF_ARG(wchar_t), m_bInitialized); 
#endif                             
                        }
                        else if (TM('c') == (*l_pIterator))
                        {
#ifdef UTF8_ENCODING
                            ADD_ARG(P7TRACE_ARG_TYPE_CHAR, SIZE_OF_ARG(char), m_bInitialized); 
#else
                            ADD_ARG(P7TRACE_ARG_TYPE_CHAR16, SIZE_OF_ARG(wchar_t), m_bInitialized); 
#endif                             
                        }
                        else
                        {
                            //1 bytes character
                            ADD_ARG(P7TRACE_ARG_TYPE_CHAR, SIZE_OF_ARG(char), m_bInitialized);
                        }

                        l_bPercent = FALSE;
                        break;
                    }
                    case TM('%'):
                    {
                        l_bPercent = FALSE;
                        break;
                    }
                } //switch (*l_pIterator)

                if (    (TM('0') <= (*l_pIterator))
                     && (TM('9') >= (*l_pIterator))
                   )
                {
                    l_bSize = 1;
                }
            }

            l_pIterator ++;
        } //while (    (*l_pIterator)

        //if (FALSE == m_bInitialized)
        //{
        //    m_dwArgs_Count = 0;
        //    l_pFormat    = TM("WRONG TRACE ARGUMENTS !");
        //}
    } //if (m_bInitialized)

    if (m_bInitialized)
    {
        m_dwSize = sizeof(sP7Trace_Format);
                   
        if (l_pArgs)
        {
            m_dwSize += sizeof(sP7Trace_Arg) * m_dwArgs_Count;
        }

        if (i_pFile)
        {
            l_dwFile_Size += (tUINT32)strlen(i_pFile);
        }
        l_dwFile_Size ++; //last 0

        if (i_pFunction)
        {
            l_dwFunc_Size += (tUINT32)strlen(i_pFunction);
        }
        l_dwFunc_Size ++; //last 0

        l_dwForm_Size = (PUStrLen(l_pFormat) + 1)  * sizeof(tWCHAR);
        m_dwSize     += l_dwForm_Size + l_dwFunc_Size + l_dwFile_Size;


        m_pBuffer = (tUINT8*)i_rMemory.Alloc(m_dwSize);
        if (NULL == m_pBuffer)
        {
            m_bInitialized = FALSE;
        }
    }


    //calculate stack blocks max size
    if (    (m_bInitialized)
         && (l_pArgs)
       )
    {
        tBOOL  l_bCumulative = FALSE;

        //allocate blocks for every variable
        m_dwBlocks_Count = 0;
        m_pBlocks        = (tINT32*)i_rMemory.Alloc(sizeof(tINT32) * (m_dwArgs_Count));
        
        if (m_pBlocks) //fill the blocks
        {
            memset(m_pBlocks, 0, sizeof(tINT32) * m_dwArgs_Count);
            l_bCumulative = FALSE;

            for (tUINT32 l_dwI = 0; l_dwI < m_dwArgs_Count; l_dwI++)
            {
                if (P7TRACE_ARG_TYPE_PVOID >= l_pArgs[l_dwI].bType) //most probable case
                {
                    if (FALSE == l_bCumulative)
                    {
                        m_dwBlocks_Count ++;   
                        l_bCumulative = TRUE;
                    }

                    m_pBlocks[m_dwBlocks_Count - 1] += l_pArgs[l_dwI].bSize;
                }
                else if (P7TRACE_ARG_TYPE_USTR16 == l_pArgs[l_dwI].bType)
                {
                    if (0 == l_pArgs[l_dwI].bSize)
                    {
                        m_pBlocks[m_dwBlocks_Count] = P7TRACE_ITEM_BLOCK_USTRING16;  
                    }
                    else
                    {
                        m_pBlocks[m_dwBlocks_Count] = P7TRACE_ITEM_BLOCK_WSTRING_FIX;  
                        l_pArgs[l_dwI].bSize        = 0;
                    }

                    m_dwBlocks_Count ++;
                    l_bCumulative = FALSE;
                }
                else if (    (P7TRACE_ARG_TYPE_STRA == l_pArgs[l_dwI].bType)
                          || (P7TRACE_ARG_TYPE_USTR8 == l_pArgs[l_dwI].bType)
                        )
                {
                    if (0 == l_pArgs[l_dwI].bSize)
                    {
                        m_pBlocks[m_dwBlocks_Count] = P7TRACE_ITEM_BLOCK_ASTRING;  
                    }
                    else
                    {
                        m_pBlocks[m_dwBlocks_Count] = P7TRACE_ITEM_BLOCK_ASTRING_FIX;  
                        l_pArgs[l_dwI].bSize        = 0;
                    }

                    m_dwBlocks_Count ++;
                    l_bCumulative = FALSE;
                }
                else if (P7TRACE_ARG_TYPE_USTR32 == l_pArgs[l_dwI].bType)
                {
                    if (0 == l_pArgs[l_dwI].bSize)
                    {
                        m_pBlocks[m_dwBlocks_Count] = P7TRACE_ITEM_BLOCK_USTRING32;  
                    }
                    else
                    {
                        m_pBlocks[m_dwBlocks_Count] = P7TRACE_ITEM_BLOCK_WSTRING_FIX;  
                        l_pArgs[l_dwI].bSize        = 0;
                    }

                    m_dwBlocks_Count ++;
                    l_bCumulative = FALSE;
                }
                else
                {
                    if (FALSE == l_bCumulative)
                    {
                        m_dwBlocks_Count ++;   
                        l_bCumulative = TRUE;
                    }

                    m_pBlocks[m_dwBlocks_Count - 1] += l_pArgs[l_dwI].bSize;
                }
            }
        }
        else
        {
            m_bInitialized = FALSE;
        }
    }

    //forming trace format header with all related info
    if (m_bInitialized)
    {
        tUINT32 l_dwOffset = 0;

        sP7Trace_Format *l_pHeader_Desc = (sP7Trace_Format *)(m_pBuffer + l_dwOffset);

        INIT_EXT_HEADER(l_pHeader_Desc->sCommonRaw, EP7USER_TYPE_TRACE, EP7TRACE_TYPE_DESC, m_dwSize);
        //l_pHeader_Desc->sCommon.dwSize     = m_dwSize;
        //l_pHeader_Desc->sCommon.dwType     = EP7USER_TYPE_TRACE;
        //l_pHeader_Desc->sCommon.dwSubType  = EP7TRACE_TYPE_DESC;

        l_pHeader_Desc->wArgs_Len          = (tUINT16)m_dwArgs_Count;
        l_pHeader_Desc->wID                = i_wID;
        l_pHeader_Desc->wLine              = i_wLine;
        l_pHeader_Desc->wModuleID          = i_wModuleID;
        l_dwOffset                        += sizeof(sP7Trace_Format);


        if (l_pArgs)
        {
            memcpy(m_pBuffer + l_dwOffset, 
                   l_pArgs,
                   sizeof(sP7Trace_Arg) * m_dwArgs_Count
                  );
            m_pArgs = (sP7Trace_Arg*)(m_pBuffer + l_dwOffset);
            l_dwOffset += sizeof(sP7Trace_Arg) * m_dwArgs_Count;
        }

        //we do not verify address of l_pFormat because we do it before
        PUStrCpy((tWCHAR*)(m_pBuffer + l_dwOffset),
                 (m_dwSize - l_dwOffset) / sizeof(tWCHAR),
                 l_pFormat
                );
        
        l_dwOffset += l_dwForm_Size;

        if (i_pFile)
        {
            memcpy(m_pBuffer + l_dwOffset, i_pFile, l_dwFile_Size);
            l_dwOffset += l_dwFile_Size;
        }
        else
        {
            m_pBuffer[l_dwOffset ++] = 0; 
        }

        if (i_pFunction)
        {
            memcpy(m_pBuffer + l_dwOffset, i_pFunction, l_dwFunc_Size);
            l_dwOffset += l_dwFunc_Size;
        }
        else
        {
            m_pBuffer[l_dwOffset ++] = 0; 
        }
    }
} // CP7Trace_Desc   


////////////////////////////////////////////////////////////////////////////////
// CP7Trace_Desc                                       
CP7Trace_Desc::CP7Trace_Desc(CMemoryManager &i_rMemory,
                             tUINT16         i_wID,
                             tUINT16         i_wLine, 
                             tUINT16         i_wModuleID,
                             const tXCHAR   *i_pFile,
                             const tXCHAR   *i_pFunction,
                             tKeyType        i_pKeys[P7TRACE_KEY_LENGTH]
                            )
    : m_wID(i_wID)
    , m_wModuleID(i_wModuleID)
    , m_dwResets(RESET_UNDEFINED)
    , m_dwSize(0)
    , m_pBuffer(NULL)

    , m_pBlocks(NULL)
    , m_dwBlocks_Count(0)

    , m_pArgs(NULL)
    , m_dwArgs_Count(0)
   
    , m_bInitialized(TRUE)
{
    const tXCHAR *l_pFormat     = TM("%s");
    tUINT32       l_dwFile_Size = 0;
    tUINT32       l_dwFunc_Size = 0;
    tUINT32       l_dwForm_Size = 0;
    char         *l_pFile       = NULL;  
    char         *l_pFunction   = NULL;  
    sP7Trace_Arg *l_pArgs       = NULL;  
    tUINT32       l_dwArgs_Max  = 0;

    if (i_pKeys)
    {
        m_pKey[0] = i_pKeys[0];
        m_pKey[1] = i_pKeys[1];
    }
    else
    {
        m_pKey[0] = 0;
        m_pKey[1] = 0;
    }

    if (m_bInitialized)
    {
        #ifdef UTF8_ENCODING
            ADD_ARG(P7TRACE_ARG_TYPE_USTR8, 0, m_bInitialized); //SIZE_OF_ARG(char*)     
        #else
            ADD_ARG(P7TRACE_ARG_TYPE_USTR16, 0, m_bInitialized); //SIZE_OF_ARG(wchar_t*)     
        #endif                             
    } //if (m_bInitialized)

    if (m_bInitialized)
    {
        m_dwSize = sizeof(sP7Trace_Format);
                   
        if (l_pArgs)
        {
            m_dwSize += sizeof(sP7Trace_Arg) * m_dwArgs_Count;
        }

        if (i_pFile)
        {
            l_dwFile_Size = (tUINT32)PStrLen(i_pFile);

        #ifdef UTF8_ENCODING
            l_pFile        = const_cast<char*>(i_pFile);    
        #else
            l_dwFile_Size *= 5;
            l_pFile        = (char*)i_rMemory.Reuse(CMemoryManager::eFileName, l_dwFile_Size);
            l_dwFile_Size  = Convert_UTF16_To_UTF8((const tWCHAR *)i_pFile, 
                                                   l_pFile, 
                                                   l_dwFile_Size
                                                  );
        #endif                             
        }
        l_dwFile_Size ++; //last 0

        if (i_pFunction)
        {
            l_dwFunc_Size = (tUINT32)PStrLen(i_pFunction);

        #ifdef UTF8_ENCODING
            l_pFunction    = const_cast<char*>(i_pFunction);    
        #else
            l_dwFunc_Size *= 5;
            l_pFunction    = (char*)i_rMemory.Reuse(CMemoryManager::eFunctionName, l_dwFunc_Size);
            l_dwFunc_Size  = Convert_UTF16_To_UTF8((const tWCHAR *)i_pFunction, 
                                                   l_pFunction, 
                                                   l_dwFunc_Size
                                                  );
        #endif                             
        }
        l_dwFunc_Size ++; //last 0

        l_dwForm_Size = (PUStrLen(l_pFormat) + 1)  * sizeof(tWCHAR);
        m_dwSize     += l_dwForm_Size + l_dwFunc_Size + l_dwFile_Size;

        m_pBuffer = (tUINT8*)i_rMemory.Alloc(m_dwSize);
        if (NULL == m_pBuffer)
        {
            m_bInitialized = FALSE;
        }
    }

    if (m_bInitialized)
    {
        tUINT32          l_dwOffset     = 0;
        sP7Trace_Format *l_pHeader_Desc = (sP7Trace_Format *)(m_pBuffer + l_dwOffset);

        INIT_EXT_HEADER(l_pHeader_Desc->sCommonRaw, EP7USER_TYPE_TRACE, EP7TRACE_TYPE_DESC, m_dwSize);
        //l_pHeader_Desc->sCommon.dwSize     = m_dwSize;
        //l_pHeader_Desc->sCommon.dwType     = EP7USER_TYPE_TRACE;
        //l_pHeader_Desc->sCommon.dwSubType  = EP7TRACE_TYPE_DESC;

        l_pHeader_Desc->wArgs_Len          = (tUINT16)m_dwArgs_Count;
        l_pHeader_Desc->wID                = i_wID;
        l_pHeader_Desc->wLine              = i_wLine;
        l_pHeader_Desc->wModuleID          = i_wModuleID;
        l_dwOffset                        += sizeof(sP7Trace_Format);

        if (l_pArgs)
        {
            memcpy(m_pBuffer + l_dwOffset, 
                   l_pArgs,
                   sizeof(sP7Trace_Arg) * m_dwArgs_Count
                  );
            l_dwOffset += sizeof(sP7Trace_Arg) * m_dwArgs_Count;
        }

        //I don't verify address of l_pFormat because I did it before
        PUStrCpy((tWCHAR*)(m_pBuffer + l_dwOffset),
                 (m_dwSize - l_dwOffset) / sizeof(tWCHAR),
                 l_pFormat
                );
        
        l_dwOffset += l_dwForm_Size;

        if (l_pFile)
        {
            memcpy(m_pBuffer + l_dwOffset, l_pFile, l_dwFile_Size);
            l_dwOffset += l_dwFile_Size;
        }
        else
        {
            m_pBuffer[l_dwOffset ++] = 0; 
        }

        if (l_pFunction)
        {
            memcpy(m_pBuffer + l_dwOffset, l_pFunction, l_dwFunc_Size);
            l_dwOffset += l_dwFunc_Size;
        }
        else
        {
            m_pBuffer[l_dwOffset ++] = 0; 
        }
    }

} // CP7Trace_Desc   


////////////////////////////////////////////////////////////////////////////////
// Is_Initialized                                       
tBOOL CP7Trace_Desc::Is_Initialized()
{
    return m_bInitialized;
}// Is_Initialized


////////////////////////////////////////////////////////////////////////////////
// Get_Buffer                                       
tUINT8 *CP7Trace_Desc::Get_Buffer(tUINT32 *o_pSize)
{
    if (NULL != o_pSize)
    {
        *o_pSize = m_dwSize;
    }
    return m_pBuffer;
}// Get_Buffer                                       


////////////////////////////////////////////////////////////////////////////////
// Get_Blocks                                       
tINT32 *CP7Trace_Desc::Get_Blocks(tUINT32 *o_pCount)
{
    if (NULL == o_pCount)
    {
        return NULL;
    }

    *o_pCount = m_dwBlocks_Count;

    return m_pBlocks;
}// Get_Blocks


////////////////////////////////////////////////////////////////////////////////
// Set_Resets 
void CP7Trace_Desc::Set_Resets(tUINT32 i_dwResets)
{
    m_dwResets = i_dwResets;
}// Set_Resets                                       


////////////////////////////////////////////////////////////////////////////////
// Get_Resets                                       
tUINT32 CP7Trace_Desc::Get_Resets()
{
    return m_dwResets;
}// Get_Resets                                       


////////////////////////////////////////////////////////////////////////////////
// Get_Arguments_Count
tUINT32 CP7Trace_Desc::Get_Arguments_Count()
{
    return m_dwArgs_Count;
}// Get_Arguments_Count

////////////////////////////////////////////////////////////////////////////////
// Get_Arguments
const sP7Trace_Arg *CP7Trace_Desc::Get_Arguments(tUINT32 &o_rCount)
{
    o_rCount = m_dwArgs_Count;
    return m_pArgs;
}// Get_Arguments


////////////////////////////////////////////////////////////////////////////////
// Is_Equal                  
GASSERT(P7TRACE_KEY_LENGTH == 2);
tBOOL CP7Trace_Desc::Is_Equal(tKeyType *i_pKey)
{
    return ((i_pKey[0] == m_pKey[0]) && (i_pKey[1] == m_pKey[1]));
}// Is_Equal


////////////////////////////////////////////////////////////////////////////////
// Is_Greater (m_pKey > i_pKey) == TRUE
GASSERT(P7TRACE_KEY_LENGTH == 2);
tBOOL CP7Trace_Desc::Is_Greater(tKeyType *i_pKey)
{
    if (i_pKey[0] < m_pKey[0])
    {
        return TRUE;
    }

    if ((i_pKey[0] == m_pKey[0]) && (i_pKey[1] < m_pKey[1]))
    {
        return TRUE;
    }

    return FALSE;
}// Is_Greater


////////////////////////////////////////////////////////////////////////////////
// Get_ID                                       
tUINT16 CP7Trace_Desc::Get_ID()
{
    return m_wID;
} //Get_ID

////////////////////////////////////////////////////////////////////////////////
// Get_MID                                       
tUINT16 CP7Trace_Desc::Get_MID()
{
    return m_wModuleID;
} // Get_MID



////////////////////////////////////////////////////////////////////////////////
// CP7Trace                                       
CP7Trace::CP7Trace(IP7_Client         *i_pClient, 
                   const tXCHAR       *i_pName,
                   const stTrace_Conf *i_pConf)
    : m_lReference(1)
    , m_dwSequence(0)
    , m_pClient(i_pClient)
    , m_dwChannel_ID(0)
    , m_wDesc_Tree_ID(P7_TRACE_DESC_HARDCODED_COUNT)
    , m_dwLast_ID(0)
    , m_bInitialized(TRUE)
    , m_bActive(TRUE)
    , m_eVerbosity(EP7TRACE_LEVEL_TRACE)
    , m_pChk_Head(NULL)
    , m_pChk_Tail(NULL)
    , m_pChk_Curs(NULL)
    , m_dwChk_Count(0)
    , m_dwChk_Size(0)
    , m_bIs_Channel(FALSE)
    , m_dwFlags(0)
    , m_wLast_ModuleID(1)
    , m_hShared(NULL)
    , m_cMemory(10240)
    , m_pVargs(NULL)
    , m_szVargs(0)
{
    if (i_pConf)
    {
        m_sConf = *i_pConf;
    }
    else
    {
        memset(&m_sConf, 0, sizeof(m_sConf));
    }

    memset(&m_sCS, 0, sizeof(m_sCS));
    
    LOCK_CREATE(m_sCS);

    memset(&m_sHeader_Info, 0, sizeof(m_sHeader_Info));
    memset(&m_sHeader_Data, 0, sizeof(m_sHeader_Data));
    memset(m_pDesc_Array, 0, sizeof(m_pDesc_Array));

    m_sStatus.bConnected = TRUE;
    m_sStatus.dwResets   = 0;

    if (NULL == m_pClient)
    {
        m_bInitialized = FALSE;
    }
    else
    {
        const tXCHAR *l_pVerbosiry = m_pClient->Get_Argument(CLIENT_COMMAND_TRACE_VERBOSITY);
        if (l_pVerbosiry)
        {
            eP7Trace_Level l_eVerbosity = (eP7Trace_Level)PStrToInt(l_pVerbosiry);
            if (    (l_eVerbosity < EP7TRACE_LEVEL_COUNT)
                 && (l_eVerbosity >= EP7TRACE_LEVEL_TRACE)
               )
            {
                m_eVerbosity = l_eVerbosity;
            }
        }

        m_pClient->Add_Ref();
    }

    if (m_bInitialized)
    {
        m_bInitialized = Inc_Chunks(256);
    }

    if (m_bInitialized)
    {
        INIT_EXT_HEADER(m_sHeader_Info.sCommonRaw, EP7USER_TYPE_TRACE, EP7TRACE_TYPE_INFO, sizeof(m_sHeader_Info));
        //m_sHeader_Info.sCommon.dwSize    = sizeof(sP7Trace_Info);
        //m_sHeader_Info.sCommon.dwType    = EP7USER_TYPE_TRACE;
        //m_sHeader_Info.sCommon.dwSubType = EP7TRACE_TYPE_INFO;

        INIT_EXT_HEADER(m_sHeader_Data.sCommonRaw, EP7USER_TYPE_TRACE, EP7TRACE_TYPE_DATA, 0);
        //m_sHeader_Data.sCommon.dwType    = EP7USER_TYPE_TRACE;
        //m_sHeader_Data.sCommon.dwSubType = EP7TRACE_TYPE_DATA;

        if (i_pName)
        {
            PUStrCpy(m_sHeader_Info.pName, P7TRACE_NAME_LENGTH, i_pName);
        }
        else
        {
            PUStrCpy(m_sHeader_Info.pName, P7TRACE_NAME_LENGTH, TM("Unknown"));
        }

        if (m_sConf.qwTimestamp_Frequency)
        {
            m_sHeader_Info.qwTimer_Frequency = m_sConf.qwTimestamp_Frequency;
            m_sHeader_Info.qwTimer_Value     = m_sConf.pTimestamp_Callback(m_sConf.pContext);
        }
        else
        {
            m_sHeader_Info.qwTimer_Frequency = GetPerformanceFrequency();
            m_sHeader_Info.qwTimer_Value     = GetPerformanceCounter();
        }

        GetEpochTime(&m_sHeader_Info.dwTime_Hi, &m_sHeader_Info.dwTime_Lo);

        m_sHeader_Info.qwFlags = P7TRACE_INFO_FLAG_EXTENTION;

        //Add main header to delivery chunks list
        m_pChk_Curs->dwSize = sizeof(m_sHeader_Info);
        m_pChk_Curs->pData  = &m_sHeader_Info;
        m_dwChk_Size       += m_pChk_Curs->dwSize;

        m_pChk_Curs ++;
    }

    ///////////////////////////////////////////////////////////////////////////
    // This code is dedicated to printing stack in case of crash, main idea is
    // preallocate trace messages and all related memory for quick stack dump
    // without allocation any new memory, but for the moment this functionality
    // is frozen
    // if (m_bInitialized)
    // {
    //     tKeyType l_pKey[P7TRACE_KEY_LENGTH];
    //     l_pKey[0] = (tKeyType)FORMAT_FUNC_UNKNOWN;
    //     l_pKey[1] = (tKeyType)FORMAT_CRASH_MESSAGE;
    //             
    //     m_wDesc_Tree_ID ++;
    //     m_cDescU_Tree.Push(new CP7Trace_Desc(m_wDesc_Tree_ID, 
    //                                          0, 
    //                                          0,
    //                                          FORMAT_FILE_UNKNOWN,
    //                                          FORMAT_FUNC_UNKNOWN, 
    //                                          FORMAT_CRASH_MESSAGE
    //                                          ), 
    //                         l_pKey
    //                         );
    // 
    //     l_pKey[0] = (tKeyType)FORMAT_FUNC_UNKNOWN;
    //     l_pKey[1] = (tKeyType)FORMAT_STACK_MESSAGE;
    //             
    //     m_wDesc_Tree_ID ++;
    //     m_cDescU_Tree.Push(new CP7Trace_Desc(m_wDesc_Tree_ID, 
    //                                          0, 
    //                                          0,
    //                                          FORMAT_FILE_UNKNOWN,
    //                                          FORMAT_FUNC_UNKNOWN, 
    //                                          FORMAT_STACK_MESSAGE
    //                                          ), 
    //                         l_pKey
    //                         );
    // }
    ////////////////////////////////////////////////////////////////////////////
    // tUINT32                    l_dwCount = 0;
    // const CStackTrace::sStack *l_pStack  = m_cStack_Trace.Get_Stack(&l_dwCount, 0);
    // 
    // *io_pCrash = FALSE;
    // 
    // if (l_dwCount)
    // {
    //     Trace(0,
    //             EP7TRACE_LEVEL_CRITICAL, 
    //             0, 
    //             0, 
    //             FORMAT_FILE_UNKNOWN,
    //             FORMAT_FUNC_UNKNOWN, 
    //             FORMAT_CRASH_MESSAGE
    //             );
    // 
    //     for (tUINT32 l_dwI = 0; l_dwI < l_dwCount; l_dwI ++)
    //     {
    //         Trace(0,
    //                 EP7TRACE_LEVEL_INFO, 
    //                 0, 
    //                 0, 
    //                 FORMAT_FILE_UNKNOWN,
    //                 FORMAT_FUNC_UNKNOWN, 
    //                 FORMAT_STACK_MESSAGE,
    //                 l_dwI,
    //                 l_pStack[l_dwI].pAddr,
    //                 l_pStack[l_dwI].pName,
    //                 l_pStack[l_dwI].dwLine
    //                 );
    //     }
    // 
    // }

   
    if (m_bInitialized)
    {
        m_bIs_Channel  = (ECLIENT_STATUS_OK == m_pClient->Register_Channel(this));
        m_bInitialized = m_bIs_Channel;
    }

#if defined(P7TRACE_NO_VA_ARG_OPTIMIZATION)
    if (m_bInitialized)
    {
        m_szVargs = 256;
        m_pVargs  = (tUINT8*)m_cMemory.Reuse(CMemoryManager::eVaValues, m_szVargs);
    }
#else
    if (m_bInitialized)
    {
        CP7Trace::sStack_Desc l_pDesc[6];
        l_pDesc[0].uValue.dw32 = 0xDEADBEEFu; 
        l_pDesc[0].eType       = CP7Trace::sStack_Desc::eTypeU32;
        l_pDesc[0].bLast       = FALSE;

        l_pDesc[1].uValue.dw64 = 0x99ABCDEF12345678ull; 
        l_pDesc[1].eType       = CP7Trace::sStack_Desc::eTypeU64;
        l_pDesc[1].bLast       = FALSE;

        l_pDesc[2].uValue.dw32 = 0xABBADEFEu; 
        l_pDesc[2].eType       = CP7Trace::sStack_Desc::eTypeU32;
        l_pDesc[2].bLast       = FALSE;

        l_pDesc[3].uValue.dw32 = 0x98765432u; 
        l_pDesc[3].eType       = CP7Trace::sStack_Desc::eTypeU32;
        l_pDesc[3].bLast       = FALSE;

        l_pDesc[4].uValue.db64 = (tDOUBLE)362764.9; 
        l_pDesc[4].eType       = CP7Trace::sStack_Desc::eTypeD64;
        l_pDesc[4].bLast       = FALSE;

        l_pDesc[5].uValue.dw64 = 0x791A90204E0b4C6Dull; 
        l_pDesc[5].eType       = CP7Trace::sStack_Desc::eTypeU64;
        l_pDesc[5].bLast       = TRUE;

        if (!Is_VarArgs(l_pDesc, 
                        l_pDesc[0].uValue.dw32, 
                        l_pDesc[1].uValue.dw64,
                        l_pDesc[2].uValue.dw32,
                        l_pDesc[3].uValue.dw32,
                        l_pDesc[4].uValue.db64,
                        l_pDesc[5].uValue.dw64
                       )
           )
        {
            printf("ERROR: P7.Trace  detects  that  current  architecture/compiler\n");
            printf("       doesn't support location of variadic arguments on stack\n");
            printf("       to make library works properly  please  activate  macro\n");
            printf("       \"P7TRACE_NO_VA_ARG_OPTIMIZATION\"  in file  \"P7_Trace.h\"\n");
            printf("       Sorry for inconvenience and have a nice day!\n");
        }
    }
#endif

    m_bActive = m_bInitialized;
}


////////////////////////////////////////////////////////////////////////////////
// ~CP7Trace                                       
CP7Trace::~CP7Trace()
{
    LOCK_ENTER(m_sCS);
    LOCK_EXIT(m_sCS);

    if (m_hShared)
    {
        CShared::Close(m_hShared);
        m_hShared = NULL;
    }

    if (m_bIs_Channel)
    {
        Flush();

        m_pClient->Unregister_Channel(m_dwChannel_ID);    
    }

    //memory used from pool, not necessary to return
    memset(m_pDesc_Array, 0, sizeof(m_pDesc_Array));

    m_cThreadsR.Clear(TRUE);
    m_cThreadsS.Clear(TRUE);
    m_cModules.Clear(TRUE);

    m_cDescU_Tree.Clear();
    m_cDescM_Tree.Clear();

    if (m_pClient)
    {
        m_pClient->Release();
        m_pClient = NULL;
    }

    if (m_pChk_Head)
    {
        free(m_pChk_Head);
        m_pChk_Head = NULL;
    }

    m_pChk_Curs   = NULL;
    m_dwChk_Count = 0;

    m_pVargs  = NULL;
    m_szVargs = 0;

    LOCK_DESTROY(m_sCS);
}// ~CP7Trace                                      


////////////////////////////////////////////////////////////////////////////////
// Is_Initialized                                      
tBOOL CP7Trace::Is_Initialized()
{
    return m_bInitialized;
}// Is_Initialized


////////////////////////////////////////////////////////////////////////////////
// On_Init                                      
void CP7Trace::On_Init(sP7C_Channel_Info *i_pInfo)
{
    if (i_pInfo)
    {
        m_dwChannel_ID = i_pInfo->dwID;
    }
}// On_Init


////////////////////////////////////////////////////////////////////////////////
// On_Receive                                      
void CP7Trace::On_Receive(tUINT32 i_dwChannel, tUINT8 *i_pBuffer, tUINT32 i_dwSize, tBOOL i_bBigEndian)
{
    UNUSED_ARG(i_dwChannel);

    LOCK_ENTER(m_sCS);

    if (    (i_pBuffer)
         && (i_dwSize >= sizeof(sP7Ext_Header))
       )
    {
        sP7Ext_Raw l_sHeader = *(sP7Ext_Raw*)i_pBuffer;

        if (EP7USER_TYPE_TRACE == GET_EXT_HEADER_TYPE(l_sHeader))
        {
            if (EP7TRACE_TYPE_VERB == GET_EXT_HEADER_SUBTYPE(l_sHeader))
            {
                sP7Trace_Module *l_pModule = NULL;

                if (i_bBigEndian)
                {
                    ((sP7Trace_Verb*)i_pBuffer)->wModuleID  = htons(((sP7Trace_Verb*)i_pBuffer)->wModuleID);
                    ((sP7Trace_Verb*)i_pBuffer)->eVerbosity = (eP7Trace_Level)htonl(((sP7Trace_Verb*)i_pBuffer)->eVerbosity);
                }

                //if it is old protocol and field wModuleID isn't included into command
                //or if module ID is 0
                if (    (i_dwSize < sizeof(sP7Trace_Verb))
                     || (!((sP7Trace_Verb*)i_pBuffer)->wModuleID)
                   )
                {
                    m_eVerbosity = ((sP7Trace_Verb*)i_pBuffer)->eVerbosity;
                }
                else //set module verbosity
                {
                    tUINT16     l_wModuleID = ((sP7Trace_Verb*)i_pBuffer)->wModuleID - 1;
                    pAList_Cell l_pEl       = NULL;
                    sModules   *l_pModules  = NULL; 

                    l_pEl = NULL;
                    while ((l_pEl = m_cModules.Get_Next(l_pEl)))
                    {
                        l_pModules = m_cModules.Get_Data(l_pEl);
                        if (l_pModules->dwUsed > l_wModuleID)
                        {
                            l_pModules->pData[l_wModuleID].eVerbosity = ((sP7Trace_Verb*)i_pBuffer)->eVerbosity;
                            l_pModule = &l_pModules->pData[l_wModuleID];
                            break;
                        }
                        else
                        {
                            l_wModuleID -= l_pModules->dwUsed;
                        }
                    }//while (l_pEl = m_cThreadsR.Get_Next(l_pEl))
                }

                if (m_sConf.pVerbosity_Callback)
                {
                    m_sConf.pVerbosity_Callback(m_sConf.pContext, l_pModule, ((sP7Trace_Verb*)i_pBuffer)->eVerbosity);
                }
            }
            else if (EP7TRACE_TYPE_DELETE == GET_EXT_HEADER_SUBTYPE(l_sHeader))
            {
                Flush();
                m_bActive = FALSE;
            }
        }
    }

    LOCK_EXIT(m_sCS);
}// On_Receive 


////////////////////////////////////////////////////////////////////////////////
// On_Status                                      
void CP7Trace::On_Status(tUINT32 i_dwChannel, const sP7C_Status *i_pStatus)
{
    UNUSED_ARG(i_dwChannel);

    LOCK_ENTER(m_sCS);
    if (i_pStatus)
    {
        //if connection was established - sent threads info ...
        if (    (i_pStatus->dwResets != m_sStatus.dwResets)
             && (i_pStatus->bConnected)
           )
        {
            pAList_Cell  l_pEl       = NULL;
            sThreadsR   *l_pThreadsR = NULL;
            sThreadsS   *l_pThreadsS = NULL;
            sModules    *l_pModules  = NULL; 

            m_pChk_Curs  = m_pChk_Head;
            m_dwChk_Size = 0;

            //Add main header to delivery chunks list
            m_pChk_Curs->dwSize = sizeof(m_sHeader_Info);
            m_pChk_Curs->pData  = &m_sHeader_Info;
            m_dwChk_Size       += m_pChk_Curs->dwSize;

            m_pChk_Curs ++;

            //add active threads desc. to delivery chunks list
            l_pEl = NULL;
            while ((l_pEl = m_cThreadsR.Get_Next(l_pEl)))
            {
                l_pThreadsR = m_cThreadsR.Get_Data(l_pEl);
                if (l_pThreadsR->dwUsed)
                {
                    m_pChk_Curs->pData  = l_pThreadsR->pData;
                    m_pChk_Curs->dwSize = sizeof(sP7Trace_Thread_Start) * l_pThreadsR->dwUsed;
                    m_dwChk_Size       += m_pChk_Curs->dwSize;
                    m_pChk_Curs ++;

                    if (m_pChk_Curs >= m_pChk_Tail)
                    {
                        Inc_Chunks(64);
                    }
                }
            }//while (l_pEl = m_cThreadsR.Get_Next(l_pEl))

            //add stopped threads desc. to delivery chunks list
            l_pEl = NULL;
            while ((l_pEl = m_cThreadsS.Get_Next(l_pEl)))
            {
                l_pThreadsS = m_cThreadsS.Get_Data(l_pEl);
                if (l_pThreadsR->dwUsed)
                {
                    m_pChk_Curs->pData  = l_pThreadsS->pData;
                    m_pChk_Curs->dwSize = sizeof(sP7Trace_Thread_Stop) * l_pThreadsS->dwUsed;
                    m_dwChk_Size       += m_pChk_Curs->dwSize;
                    m_pChk_Curs ++;

                    if (m_pChk_Curs >= m_pChk_Tail)
                    {
                        Inc_Chunks(64);
                    }
                }
            }//while (l_pEl = m_cThreadsR.Get_Next(l_pEl))

            //add modules descriptions to chunks list
            l_pEl = NULL;
            while ((l_pEl = m_cModules.Get_Next(l_pEl)))
            {
                l_pModules = m_cModules.Get_Data(l_pEl);
                if (l_pModules->dwUsed)
                {
                    m_pChk_Curs->pData  = l_pModules->pData;
                    m_pChk_Curs->dwSize = sizeof(sP7Trace_Module) * l_pModules->dwUsed;
                    m_dwChk_Size       += m_pChk_Curs->dwSize;
                    m_pChk_Curs ++;

                    if (m_pChk_Curs >= m_pChk_Tail)
                    {
                        Inc_Chunks(64);
                    }
                }
            }//while (l_pEl = m_cThreadsR.Get_Next(l_pEl))

        }//(i_pStatus->dwResets != m_sStatus.dwResets) ...

        m_sStatus = *i_pStatus;

        if (m_sConf.pConnect_Callback)
        {
            m_sConf.pConnect_Callback(m_sConf.pContext, m_sStatus.bConnected);
        }
    }
    LOCK_EXIT(m_sCS);
}// On_Status


////////////////////////////////////////////////////////////////////////////////
// On_Flush - external call                                     
void CP7Trace::On_Flush(tUINT32 i_dwChannel, tBOOL *io_pCrash)
{
    UNUSED_ARG(i_dwChannel);

    LOCK_ENTER(m_sCS);

    if (    (io_pCrash)
         && (TRUE == *io_pCrash)
         && (m_bActive)
       )
    {
        //nothing special for crash event
    }

    Flush();

    LOCK_EXIT(m_sCS);
}// On_Flush                                      


////////////////////////////////////////////////////////////////////////////////
// Register_Thread                                      
tBOOL CP7Trace::Register_Thread(const tXCHAR *i_pName, tUINT32 i_dwThreadId)
{
    tBOOL l_bExtended = FALSE;
    tBOOL l_bReturn   = TRUE;

    if (NULL == i_pName)
    {
        return FALSE;
    }

    LOCK_ENTER(m_sCS);

    sP7Trace_Thread_Start *l_pThread  = NULL;
    sThreadsR             *l_pThreads = m_cThreadsR.Get_Data(m_cThreadsR.Get_Last());
    sP7C_Data_Chunk       *l_pChunk   = m_pChk_Head;

    if (    (NULL == l_pThreads)
         || (l_pThreads->dwUsed >= l_pThreads->dwCount)
       )
    {
        if ((m_cThreadsR.Count() * P7TRACE_THREADS_POOL_SIZE) < P7TRACE_THREADS_MAX_SIZE)
        {
            l_pThreads = new sThreadsR(P7TRACE_THREADS_POOL_SIZE);
            m_cThreadsR.Add_After(m_cThreadsR.Get_Last(), l_pThreads);
        }
        else
        {
            l_bReturn = FALSE;
            goto l_lblExit;
        }
    }

    l_pThread = &l_pThreads->pData[l_pThreads->dwUsed];
    l_pThreads->dwUsed ++;

    INIT_EXT_HEADER(l_pThread->sCommonRaw, EP7USER_TYPE_TRACE, EP7TRACE_TYPE_THREAD_START, sizeof(sP7Trace_Thread_Start));
    //l_pThread->sCommon.dwSize    = sizeof(sP7Trace_Thread_Start);
    //l_pThread->sCommon.dwType    = EP7USER_TYPE_TRACE;
    //l_pThread->sCommon.dwSubType = EP7TRACE_TYPE_THREAD_START;

    l_pThread->dwThreadID = (!i_dwThreadId) ? CProc::Get_Thread_Id() : i_dwThreadId;

    if (! m_sConf.pTimestamp_Callback )
    {
        l_pThread->qwTimer = GetPerformanceCounter();
    }
    else
    {
        l_pThread->qwTimer = m_sConf.pTimestamp_Callback(m_sConf.pContext);
    }

#ifdef UTF8_ENCODING
    PStrCpy(l_pThread->pName, P7TRACE_THREAD_NAME_LENGTH, i_pName);
#else
    Convert_UTF16_To_UTF8((const tWCHAR*)i_pName, 
                          l_pThread->pName, 
                          P7TRACE_THREAD_NAME_LENGTH
                         );
#endif                             

    //look for existing chunks which can be updated
    while (l_pChunk < m_pChk_Curs)
    {
        //if new thread address is beginning of existing chunk pointer
        if (((tUINT8*)l_pThread + sizeof(sP7Trace_Thread_Start)) == l_pChunk->pData)
        {
            l_pChunk->pData   = l_pThread;
            l_pChunk->dwSize += sizeof(sP7Trace_Thread_Start);
            m_dwChk_Size     += sizeof(sP7Trace_Thread_Start);
            l_bExtended       = TRUE;
            break; 
        }
        //if new thread address is continuation of existing chunk pointer
        else if (((tUINT8*)l_pChunk->pData + l_pChunk->dwSize) == (tUINT8*)l_pThread)
        {
            l_pChunk->dwSize += sizeof(sP7Trace_Thread_Start);
            m_dwChk_Size     += sizeof(sP7Trace_Thread_Start);
            l_bExtended       = TRUE;
            break; 
        }

        l_pChunk ++;
    }

    //if chunk was not extended ...
    if (FALSE == l_bExtended)
    {
        //Add thread header to delivery chunks list
        m_pChk_Curs->dwSize = sizeof(sP7Trace_Thread_Start);
        m_pChk_Curs->pData  = l_pThread;
        m_dwChk_Size       += m_pChk_Curs->dwSize;
        m_pChk_Curs ++;

        if (m_pChk_Curs >= m_pChk_Tail)
        {
            Inc_Chunks(64);
        }
    }

    if (    (m_sStatus.bConnected)
         && (ECLIENT_STATUS_OK == m_pClient->Sent(m_dwChannel_ID, 
                                                  m_pChk_Head,  
                                                  (tUINT32)(m_pChk_Curs - m_pChk_Head), 
                                                  m_dwChk_Size
                                                 )
           )
      )
    {
        m_pChk_Curs  = m_pChk_Head;
        m_dwChk_Size = 0;
    }

l_lblExit:
    LOCK_EXIT(m_sCS);

    return l_bReturn;
}//Register_Thread


////////////////////////////////////////////////////////////////////////////////
// Unregister_Thread                                      
tBOOL CP7Trace::Unregister_Thread(tUINT32 i_dwThreadId)
{
    tBOOL                  l_bReturn  = FALSE;
    sThreadsR             *l_pStarted = NULL;
    sThreadsS             *l_pStopped = NULL;
    sP7Trace_Thread_Stop  *l_pStop    = NULL;
    pAList_Cell            l_pEl      = NULL;

    if (!i_dwThreadId)
    {
        i_dwThreadId = CProc::Get_Thread_Id();
    }

    LOCK_ENTER(m_sCS);

    while ((l_pEl = m_cThreadsR.Get_Next(l_pEl)))
    {
        l_pStarted = m_cThreadsR.Get_Data(l_pEl);
        for (tUINT32 l_dwI = 0; l_dwI < l_pStarted->dwUsed; l_dwI++)
        {
            if (i_dwThreadId == l_pStarted->pData[l_dwI].dwThreadID)
            {
                l_bReturn = TRUE;
                break;
            }
        }
    }

    //such thread isn't registered !
    if (!l_bReturn)
    {
        goto l_lblExit;
    }

    //save current Stop command in queue
    l_pStopped = m_cThreadsS.Get_Data(m_cThreadsS.Get_Last());

    if (    (NULL == l_pStopped)
         || (l_pStopped->dwUsed >= l_pStopped->dwCount)
       )
    {
        if ((m_cThreadsS.Count() * P7TRACE_THREADS_POOL_SIZE) < P7TRACE_THREADS_MAX_SIZE)
        {
            l_pStopped = new sThreadsS(P7TRACE_THREADS_POOL_SIZE);
            m_cThreadsS.Add_After(m_cThreadsS.Get_Last(), l_pStopped);
        }
        else
        {
            l_bReturn = FALSE;
            goto l_lblExit;
        }
    }

    l_pStop = &l_pStopped->pData[l_pStopped->dwUsed];
    l_pStopped->dwUsed ++;

    INIT_EXT_HEADER(l_pStop->sCommonRaw, EP7USER_TYPE_TRACE, EP7TRACE_TYPE_THREAD_STOP, sizeof(sP7Trace_Thread_Stop));
    //l_pStop->sCommon.dwSize    = sizeof(sP7Trace_Thread_Stop);
    //l_pStop->sCommon.dwType    = EP7USER_TYPE_TRACE;
    //l_pStop->sCommon.dwSubType = EP7TRACE_TYPE_THREAD_STOP;

    l_pStop->dwThreadID = i_dwThreadId;
    if (! m_sConf.pTimestamp_Callback )
    {
        l_pStop->qwTimer = GetPerformanceCounter();
    }
    else
    {
        l_pStop->qwTimer = m_sConf.pTimestamp_Callback(m_sConf.pContext);
    }
                 
    if (m_pChk_Curs >= m_pChk_Tail)
    {
        Inc_Chunks(64);
    }

    m_pChk_Curs->pData  = l_pStop;
    m_pChk_Curs->dwSize = GET_EXT_HEADER_SIZE(l_pStop->sCommonRaw);

    m_dwChk_Size        += m_pChk_Curs->dwSize;
    m_pChk_Curs ++;

    if (    (m_sStatus.bConnected)
         && (ECLIENT_STATUS_OK == m_pClient->Sent(m_dwChannel_ID,
                                                  m_pChk_Head,
                                                  (tUINT32)(m_pChk_Curs - m_pChk_Head),
                                                  m_dwChk_Size
                                                 )
            )
       )
    {
        m_pChk_Curs  = m_pChk_Head;
        m_dwChk_Size = 0;

        //delete all closed threads and related active threads
        while ((l_pEl = m_cThreadsS.Get_Last()))
        {
            l_pStopped = m_cThreadsS.Get_Data(l_pEl);

            for (tUINT32 l_dwI = 0; l_dwI < l_pStopped->dwUsed; l_dwI++)
            {
                sP7Trace_Thread_Start *l_pCurrent = NULL;
                sP7Trace_Thread_Start *l_pLast    = NULL;
                pAList_Cell            l_pElR     = NULL;

                //looking through active threads to find thread by ID
                while ((l_pElR = m_cThreadsR.Get_Next(l_pElR)))
                {
                    l_pStarted = m_cThreadsR.Get_Data(l_pElR);

                    for (tUINT32 l_dwJ = 0; l_dwJ < l_pStarted->dwUsed; l_dwJ++)
                    {
                        if (l_pStopped->pData[l_dwI].dwThreadID == l_pStarted->pData[l_dwJ].dwThreadID)
                        {
                            l_pCurrent = &l_pStarted->pData[l_dwJ];
                            break;
                        }
                    }

                    if (l_pCurrent)
                    {
                        sThreadsR *l_pLastR = m_cThreadsR.Get_Data(m_cThreadsR.Get_Last());

                        if (    (l_pLastR)
                             && (l_pLastR->dwUsed)
                           )
                        {
                            l_pLast = &l_pLastR->pData[l_pLastR->dwUsed - 1];

                            if (l_pLast != l_pCurrent)
                            {
                                memcpy(l_pCurrent, l_pLast, sizeof(sP7Trace_Thread_Start));
                            }

                            l_pLastR->dwUsed --;

                            if (    (0 == l_pLastR->dwUsed)
                                 && (1 < m_cThreadsR.Count())
                               )
                            {
                                m_cThreadsR.Del(m_cThreadsR.Get_Last(), TRUE);
                            }
                        }

                        break;
                    }
                }//while (l_pElR = m_cThreadsR.Get_Next(l_pElR))
            }//for (tUINT32 l_dwI = 0; l_dwI < l_pStopped->dwUsed; l_dwI++)

            m_cThreadsS.Del(l_pEl, TRUE);
        }//while (l_pEl = m_cThreadsS.Get_Last())
    } //m_pClient->Sent(...)

l_lblExit:
    LOCK_EXIT(m_sCS);

    return l_bReturn;
}// Unregister_Thread


////////////////////////////////////////////////////////////////////////////////
// Register_Module                                      
tBOOL CP7Trace::Register_Module(const tXCHAR *i_pName, IP7_Trace::hModule *o_hModule)
{
    if (    (NULL == i_pName)
         || (NULL == o_hModule)
       )
    {
        return FALSE;
    }

    LOCK_ENTER(m_sCS);
    tBOOL            l_bReturn   = TRUE;
    sP7Trace_Module *l_pModule   = NULL;
    sModules        *l_pModules  = m_cModules.Get_Data(m_cModules.Get_Last());
    sP7C_Data_Chunk *l_pChunk    = m_pChk_Head;
    tBOOL            l_bExtended = FALSE;
    sModuleMap      *l_pMap      = NULL;

    //looking through existing modules -if found - return it
    l_pMap = m_cModules_Map.Find(i_pName);
    if (l_pMap)
    {
        *o_hModule = l_pMap->pModule;
        goto l_lblExit;
    }

    //module isn't exists - create new one
    if (    (NULL == l_pModules)
         || (l_pModules->dwUsed >= l_pModules->dwCount)
       )
    {
        if ((m_cModules.Count() * P7TRACE_MODULES_POOL_SIZE) < P7TRACE_MODULES_MAX_SIZE)
        {
            l_pModules = new sModules(P7TRACE_MODULES_POOL_SIZE);
            m_cModules.Add_After(m_cModules.Get_Last(), l_pModules);
        }
        else
        {
            l_bReturn = FALSE;
            goto l_lblExit;
        }
    }

    l_pModule = &l_pModules->pData[l_pModules->dwUsed];
    l_pModules->dwUsed ++;

    INIT_EXT_HEADER(l_pModule->sCommonRaw, EP7USER_TYPE_TRACE, EP7TRACE_TYPE_MODULE, sizeof(sP7Trace_Module));
    //l_pModule->sCommon.dwSize    = sizeof(sP7Trace_Module);
    //l_pModule->sCommon.dwType    = EP7USER_TYPE_TRACE;
    //l_pModule->sCommon.dwSubType = EP7TRACE_TYPE_MODULE;

    l_pModule->eVerbosity        = m_eVerbosity;
    l_pModule->wModuleID         = m_wLast_ModuleID++;

#ifdef UTF8_ENCODING
    PStrCpy(l_pModule->pName, P7TRACE_THREAD_NAME_LENGTH, i_pName);
#else
    Convert_UTF16_To_UTF8((const tWCHAR*)i_pName, 
                          l_pModule->pName, 
                          P7TRACE_MODULE_NAME_LENGTH
                         );
#endif                             

    //add module to tree 
    l_pMap = new sModuleMap(i_pName, l_pModule);
    if (!m_cModules_Map.Push(l_pMap, i_pName))
    {
        delete l_pMap;
        l_pMap = NULL;
    }

    //look for existing chunks which can be updated
    while (l_pChunk < m_pChk_Curs)
    {
        //if new thread address is beginning of existing chunk pointer
        if (((tUINT8*)l_pModule + sizeof(sP7Trace_Module)) == l_pChunk->pData)
        {
            l_pChunk->pData   = l_pModule;
            l_pChunk->dwSize += sizeof(sP7Trace_Module);
            m_dwChk_Size     += sizeof(sP7Trace_Module);
            l_bExtended       = TRUE;
            break; 
        }
        //if new thread address is continuation of existing chunk pointer
        else if (((tUINT8*)l_pChunk->pData + l_pChunk->dwSize) == (tUINT8*)l_pModule)
        {
            l_pChunk->dwSize += sizeof(sP7Trace_Module);
            m_dwChk_Size     += sizeof(sP7Trace_Module);
            l_bExtended       = TRUE;
            break; 
        }

        l_pChunk ++;
    }

    //if chunk was not extended ...
    if (FALSE == l_bExtended)
    {
        //Add thread header to delivery chunks list
        m_pChk_Curs->dwSize = sizeof(sP7Trace_Module);
        m_pChk_Curs->pData  = l_pModule;
        m_dwChk_Size       += m_pChk_Curs->dwSize;
        m_pChk_Curs ++;

        if (m_pChk_Curs >= m_pChk_Tail)
        {
            Inc_Chunks(64);
        }
    }

    //deliver
    if (    (m_sStatus.bConnected)
         && (ECLIENT_STATUS_OK == m_pClient->Sent(m_dwChannel_ID, 
                                                  m_pChk_Head,  
                                                  (tUINT32)(m_pChk_Curs - m_pChk_Head), 
                                                  m_dwChk_Size
                                                 )
           )
      )
    {
        m_pChk_Curs  = m_pChk_Head;
        m_dwChk_Size = 0;
    }

    *o_hModule = l_pModule;

l_lblExit:
    LOCK_EXIT(m_sCS);

    return l_bReturn;
}// Register_Module


////////////////////////////////////////////////////////////////////////////////
// Set_Verbosity                                      
void CP7Trace::Set_Verbosity(IP7_Trace::hModule i_hModule, eP7Trace_Level i_eVerbosity)
{
    LOCK_ENTER(m_sCS);
    sP7C_Data_Chunk *l_pChunk = NULL;
    tUINT32          l_dwSize = m_dwChk_Size;
    sP7Trace_Verb    l_sCmd;

    if (i_hModule)
    {
        ((sP7Trace_Module*)i_hModule)->eVerbosity = i_eVerbosity;
    }
    else
    {
        m_eVerbosity = i_eVerbosity;
    }

    if (m_pChk_Curs >= m_pChk_Tail)
    {
        if (!Inc_Chunks(64))
        {
            goto l_lblExit;
        }
    }

    l_pChunk = m_pChk_Curs;


    INIT_EXT_HEADER(l_sCmd.sCommonRaw, EP7USER_TYPE_TRACE, EP7TRACE_TYPE_VERB, sizeof(sP7Trace_Verb));
    //l_sCmd.sCommon.dwSize    = sizeof(sP7Trace_Verb);
    //l_sCmd.sCommon.dwType    = EP7USER_TYPE_TRACE;
    //l_sCmd.sCommon.dwSubType = EP7TRACE_TYPE_VERB;

    l_sCmd.eVerbosity        = i_eVerbosity;
    l_sCmd.wModuleID         = (i_hModule) ? ((sP7Trace_Module*)i_hModule)->wModuleID : 0;

    l_pChunk->pData          = &l_sCmd;
    l_pChunk->dwSize         = GET_EXT_HEADER_SIZE(l_sCmd.sCommonRaw);
    l_dwSize                += GET_EXT_HEADER_SIZE(l_sCmd.sCommonRaw);

    l_pChunk++;

    //deliver
    if (    (m_sStatus.bConnected)
         && (ECLIENT_STATUS_OK == m_pClient->Sent(m_dwChannel_ID, 
                                                  m_pChk_Head,  
                                                  (tUINT32)(l_pChunk - m_pChk_Head), 
                                                  l_dwSize
                                                 )
           )
      )
    {
        m_pChk_Curs  = m_pChk_Head;
        m_dwChk_Size = 0;
    }

l_lblExit:
    LOCK_EXIT(m_sCS);
}// Set_Verbosity


////////////////////////////////////////////////////////////////////////////////
// Trace                                      
tBOOL CP7Trace::Trace(tUINT16            i_wTrace_ID,
                      eP7Trace_Level     i_eLevel, 
                      IP7_Trace::hModule i_hModule,
                      tUINT16            i_wLine,
                      const char        *i_pFile,
                      const char        *i_pFunction,
                      const tXCHAR      *i_pFormat,
                      ...
                     )
{
#if !defined(P7TRACE_NO_VA_ARG_OPTIMIZATION)
    return Trace_Raw(i_wTrace_ID, 
                     i_eLevel, 
                     i_hModule, 
                     i_wLine, 
                     i_pFile, 
                     i_pFunction, 
                     (tKeyType*)&i_pFunction,
                     &i_pFormat,
                     NULL
                    );
#else
    va_list l_pVl;
    va_start(l_pVl, i_pFormat);
    tKeyType l_pKey[2] = {(tKeyType)i_pFunction, (tKeyType)i_pFormat};
    tBOOL l_bRet = Trace_Raw(i_wTrace_ID, 
                             i_eLevel, 
                             i_hModule, 
                             i_wLine, 
                             i_pFile, 
                             i_pFunction, 
                             l_pKey,
                             &i_pFormat,
                             &l_pVl
                            );
    va_end(l_pVl);
    return l_bRet;
#endif
}// Trace                                      


////////////////////////////////////////////////////////////////////////////////
// Share                                      
tBOOL CP7Trace::Share(const tXCHAR *i_pName)
{
    tBOOL l_bReturn = FALSE;

    LOCK_ENTER(m_sCS);
    if (NULL == m_hShared)
    {
        void *l_pTrace = static_cast<IP7_Trace*>(this);

        tUINT32 l_dwLen1 = PStrLen(TRACE_SHARED_PREFIX);
        tUINT32 l_dwLen2 = PStrLen(i_pName);
        tXCHAR *l_pName = (tXCHAR *)malloc(sizeof(tXCHAR) * (l_dwLen1 + l_dwLen2 + 16));

        if (l_pName)
        {
            PStrCpy(l_pName, l_dwLen1 + l_dwLen2 + 16, TRACE_SHARED_PREFIX);
            PStrCpy(l_pName + l_dwLen1, l_dwLen2 + 16, i_pName);
            l_bReturn = CShared::Create(&m_hShared, l_pName, (tUINT8*)&l_pTrace, sizeof(l_pTrace));
            free(l_pName);
            l_pName = NULL;
        }
    }
    LOCK_EXIT(m_sCS);

    return l_bReturn;
}// Share


////////////////////////////////////////////////////////////////////////////////
// On_Flush - internal call                                      
void CP7Trace::Flush()
{
    if (FALSE == m_bActive)
    {
        return;
    }

    m_bActive = FALSE;

    sP7C_Data_Chunk *l_pChunk  = m_pChk_Curs;
    tUINT32          l_dwSize  = m_dwChk_Size;
    sP7Ext_Raw       l_sHeader;

    INIT_EXT_HEADER(l_sHeader, EP7USER_TYPE_TRACE, EP7TRACE_TYPE_CLOSE, sizeof(sP7Ext_Raw));

    l_pChunk->pData  = &l_sHeader;
    l_pChunk->dwSize = sizeof(sP7Ext_Raw);
    l_dwSize        += sizeof(sP7Ext_Raw);

    l_pChunk++;

    if (l_pChunk >= m_pChk_Tail)
    {
        Inc_Chunks(64);
    }

    if (ECLIENT_STATUS_OK == m_pClient->Sent(m_dwChannel_ID,
                                             m_pChk_Head,
                                             (tUINT32)(l_pChunk - m_pChk_Head),
                                             l_dwSize
                                            )
       )
    {
        m_pChk_Curs  = m_pChk_Head;
        m_dwChk_Size = 0;
    }
}// On_Flush                                      


////////////////////////////////////////////////////////////////////////////////
// Trace_Embedded                                      
tBOOL CP7Trace::Trace_Embedded(tUINT16            i_wTrace_ID,   
                               eP7Trace_Level     i_eLevel, 
                               IP7_Trace::hModule i_hModule,
                               tUINT16            i_wLine,
                               const char        *i_pFile,
                               const char        *i_pFunction,
                               const tXCHAR     **i_ppFormat
                              )
{
#if !defined(P7TRACE_NO_VA_ARG_OPTIMIZATION)
    tKeyType l_pKey[P7TRACE_KEY_LENGTH] = {(tKeyType)i_pFunction, (tKeyType)*i_ppFormat};

    return Trace_Raw(i_wTrace_ID, 
                     i_eLevel, 
                     i_hModule, 
                     i_wLine, 
                     i_pFile, 
                     i_pFunction, 
                     l_pKey,
                     i_ppFormat,
                     NULL
                    );
#else
    UNUSED_ARG(i_wTrace_ID);
    UNUSED_ARG(i_eLevel);
    UNUSED_ARG(i_hModule);
    UNUSED_ARG(i_wLine);
    UNUSED_ARG(i_pFile);
    UNUSED_ARG(i_pFunction);
    UNUSED_ARG(i_ppFormat);

    static tBOOL g_bVaArgError = FALSE;
    if (!g_bVaArgError)
    {
        g_bVaArgError = TRUE;
        printf("P7 TRACE ERROR: Trace_Embedded function is obsolete!\n");
    }
    return FALSE;
#endif
}// Trace_Embedded


////////////////////////////////////////////////////////////////////////////////
// Trace_Embedded                                      
tBOOL CP7Trace::Trace_Embedded(tUINT16            i_wTrace_ID,   
                               eP7Trace_Level     i_eLevel, 
                               IP7_Trace::hModule i_hModule,
                               tUINT16            i_wLine,
                               const char        *i_pFile,
                               const char        *i_pFunction,
                               const tXCHAR     **i_ppFormat,
                               va_list           *i_pVa_List
                              )
{
    tKeyType l_pKey[P7TRACE_KEY_LENGTH] = {(tKeyType)i_pFunction, (tKeyType)*i_ppFormat};

    return Trace_Raw(i_wTrace_ID, 
                     i_eLevel, 
                     i_hModule, 
                     i_wLine, 
                     i_pFile, 
                     i_pFunction, 
                     l_pKey,
                     i_ppFormat,
                     i_pVa_List
                    );
}


////////////////////////////////////////////////////////////////////////////////
//Trace_Managed  
tBOOL CP7Trace::Trace_Managed(tUINT16            i_wTrace_ID,   
                              eP7Trace_Level     i_eLevel, 
                              IP7_Trace::hModule i_hModule,
                              tUINT16            i_wLine,
                              const tXCHAR      *i_pFile,
                              const tXCHAR      *i_pFunction,
                              const tXCHAR      *i_pMessage
                             )
{
    tBOOL            l_bReturn  = TRUE;
    tUINT32          l_dwSize   = 0;
    CP7Trace_Desc   *l_pDesc    = NULL; 
    tBOOL            l_bDesc    = FALSE;
    size_t           l_szTrace  = 0;
    sP7C_Data_Chunk *l_pChunk; //we do not initialize it here, we do it later

    LOCK_ENTER(m_sCS);

    m_dwSequence ++;

    if (    (i_eLevel < m_eVerbosity)
         || (FALSE == m_bActive)
         || (FALSE == m_sStatus.bConnected)
         || (    (i_hModule)
              && (i_eLevel < ((sP7Trace_Module*)i_hModule)->eVerbosity)
            )
       )
    {
        l_bReturn = FALSE;
        goto l_lblExit;
    }

    ////////////////////////////////////////////////////////////////////////////
    //if user specify direct Trace ID
    if (    (1 <= i_wTrace_ID)
         && (P7_TRACE_DESC_HARDCODED_COUNT > i_wTrace_ID)
       )
    {
        l_pDesc = m_pDesc_Array[i_wTrace_ID];
        if (NULL == l_pDesc)
        {
            l_pDesc = new (m_cMemory.Alloc(sizeof(CP7Trace_Desc)))
                          CP7Trace_Desc(m_cMemory,
                                        i_wTrace_ID, 
                                        i_wLine,
                                        (i_hModule) ? ((sP7Trace_Module*)i_hModule)->wModuleID : 0,
                                        i_pFile, 
                                        i_pFunction, 
                                        (tKeyType*)NULL
                                       );
            m_pDesc_Array[i_wTrace_ID] = l_pDesc;
        }
    }
    else //if we should use map to find it
    {
        tKeyType      l_pKeys[P7TRACE_KEY_LENGTH] = {0ul, 2166136261ul};
        const tXCHAR *l_pFile                     = i_pFile;

        //Here is RedBlackTree keys are calculated, key is consisting of 2 parts
        //l_pKeys[0] = [16 bits: i_pFile string length][16 bits: line number]
        //l_pKeys[1] = FNV-1a 32bits hash of i_pFile
        //Hash description: http://isthe.com/chongo/tech/comp/fnv/#FNV-param
        //Hash parameters investigation (collisions, randomnessification)
        //http://programmers.stackexchange.com/questions/49550/which-hashing-algorithm-is-best-for-uniqueness-and-speed
        //Collisions:
        // - Is collisions are possible ? Yes, FNV-1a produce 4 coll. on list of 
        //   216,553 English words. But count of uniq. trace ID  is  limited  to 
        //   64k, and in addition to hash I'm using second key containing string 
        //   length and source code line number. 
        // - If collision happens and even calculated key is not unique - is it
        //   dangerous ? No, even in this  hypothetical  situation  the  maximal 
        //   risk is displaying wrong [i_wLine, i_pFile, i_pFunction] values on
        //   Baical side
        // - May we prevent not unique key issue ? Yes, but it will  require  in
        //   addition checking all 3 parameters  [i_wLine, i_pFile, i_pFunction]
        //   after key searching in tree, this is  heavy  string  operation  for 
        //   every trace message
        // Conclusion:
        // - real risk of hash collision is low on 64k unique traces
        // - risk of calculating not unique key is much lower than collision due
        //   to different parameters combination in addition to hash
        // - data damage in case of  not  unique  key  isn't  significant,  main 
        //   trace message and all its parameters will  be  delivered  properly,
        //   except [i_wLine, i_pFile, i_pFunction].
        // So additional protection wasn't implemented
        while (*l_pFile)
        {
            l_pKeys[0] ++; //string length
            l_pKeys[1] = (tKeyType)(l_pKeys[1] ^ (tXCHAR)*l_pFile) * 16777619ul;
            l_pFile++;
        }

        l_pKeys[0] = (l_pKeys[0] << 16) + (tKeyType)i_wLine;

        l_pDesc = m_cDescM_Tree.Find(l_pKeys);
        if (    (NULL == l_pDesc)
             && (0xFFFF > m_wDesc_Tree_ID)
           )
        {
            m_wDesc_Tree_ID ++;
            l_pDesc = new (m_cMemory.Alloc(sizeof(CP7Trace_Desc)))
                          CP7Trace_Desc(m_cMemory, 
                                        m_wDesc_Tree_ID, 
                                        i_wLine, 
                                        (i_hModule) ? ((sP7Trace_Module*)i_hModule)->wModuleID : 0,
                                        i_pFile, 
                                        i_pFunction, 
                                        l_pKeys
                                       );
            //in stack we have pointer of the function and then format, 2 values
            m_cDescM_Tree.Push(l_pDesc, l_pKeys);
        }
    }

    if (    (NULL  == l_pDesc)
         || (FALSE == l_pDesc->Is_Initialized())
       )
    {
        l_bReturn = FALSE;
        goto l_lblExit;
    }

    //increase chunks count if it is necessary
    if ((m_pChk_Curs + 8) >= m_pChk_Tail)
    {
        if (FALSE == Inc_Chunks(16))
        {
            l_bReturn = FALSE;
            goto l_lblExit;
        }
    }

    l_pChunk = m_pChk_Curs;
    l_dwSize = m_dwChk_Size;

    //trace description have to send again
    if (m_sStatus.dwResets != l_pDesc->Get_Resets())
    {
        l_pDesc->Set_Resets(m_sStatus.dwResets);

        l_bDesc         = TRUE;
        l_pChunk->pData = l_pDesc->Get_Buffer(&l_pChunk->dwSize);
        l_dwSize       += l_pChunk->dwSize;
        l_pChunk ++;
    }

    m_sHeader_Data.bLevel     = (tUINT8)i_eLevel;
    m_sHeader_Data.bProcessor = (tUINT8)CProc::Get_Processor();
    m_sHeader_Data.dwThreadID = CProc::Get_Thread_Id();
    m_sHeader_Data.wID        = l_pDesc->Get_ID();
    m_sHeader_Data.dwSequence = m_dwSequence;

    if (! m_sConf.pTimestamp_Callback )
    {
        m_sHeader_Data.qwTimer = GetPerformanceCounter();
    }
    else
    {
       m_sHeader_Data.qwTimer = m_sConf.pTimestamp_Callback(m_sConf.pContext);
    }

    l_pChunk->dwSize = sizeof(m_sHeader_Data);
    l_pChunk->pData  = &m_sHeader_Data;

    //we should also add all variable parameters length ... later
    l_szTrace = sizeof(m_sHeader_Data);
    //SET_EXT_HEADER_SIZE(m_sHeader_Data.sCommonRaw, );
    //m_sHeader_Data.sCommon.dwSize = sizeof(m_sHeader_Data); 

    l_pChunk ++;

    //adding string message to chunk
    if (!i_pMessage)
    {
        i_pMessage = TM("NULL");
    }

    l_pChunk->dwSize = (tUINT32)((PStrLen(i_pMessage) + 1) * sizeof(tXCHAR));
    l_pChunk->pData  = (void*)i_pMessage;
    l_szTrace       += l_pChunk->dwSize;

    l_pChunk++;

    //Put extensions.../////////////////////////////////////////////////////////////
    if (i_hModule)
    {
    #if defined(__linux__) //fix alignment and GCC warnings
        memcpy(m_pExtensions, &(((sP7Trace_Module*)i_hModule)->wModuleID), sizeof(tUINT16));
    #else
        *(tUINT16*)m_pExtensions = ((sP7Trace_Module*)i_hModule)->wModuleID;
    #endif

        m_pExtensions[2]         = (tUINT8)EP7TRACE_EXT_MODULE_ID;
        m_pExtensions[3]         = 1;
        l_pChunk->pData          = m_pExtensions;
        l_pChunk->dwSize         = 4u;
    }
    else
    {
        m_pExtensions[0]         = 0;
        l_pChunk->pData          = m_pExtensions;
        l_pChunk->dwSize         = 1u;
    }

    l_szTrace += l_pChunk->dwSize;
    SET_EXT_HEADER_SIZE(m_sHeader_Data.sCommonRaw, l_szTrace);
    l_pChunk ++;


    l_dwSize += (tUINT32)l_szTrace;

    if (ECLIENT_STATUS_OK != m_pClient->Sent(m_dwChannel_ID,
                                             m_pChk_Head,
                                             (tUINT32)(l_pChunk - m_pChk_Head),
                                             l_dwSize
                                            )
       )
    {
        //if delivery was failed and we try to deliver also trace description
        //we set marker that is should be redelivered next time
        if (l_bDesc)
        {
            l_pDesc->Set_Resets(RESET_UNDEFINED);
        }

        l_bReturn = FALSE;
    }
    else
    {
        m_pChk_Curs  = m_pChk_Head;
        m_dwChk_Size = 0;
    }

l_lblExit:
    LOCK_EXIT(m_sCS);

    return l_bReturn;


    return l_bReturn;
}// Trace_Managed


////////////////////////////////////////////////////////////////////////////////
// Trace_Raw  
__forceinline tBOOL CP7Trace::Trace_Raw(tUINT16            i_wTrace_ID,   
                                        eP7Trace_Level     i_eLevel, 
                                        IP7_Trace::hModule i_hModule,
                                        tUINT16            i_wLine,
                                        const char        *i_pFile,
                                        const char        *i_pFunction,
                                        tKeyType          *i_pKey,
                                        const tXCHAR     **i_ppFormat,
                                        va_list           *i_pVa_List
                                       )
{
    tBOOL            l_bReturn  = TRUE;
    tUINT32          l_dwSize   = 0;
    CP7Trace_Desc   *l_pDesc    = NULL; 
    tINT32          *l_pBlocks  = NULL;
    tUINT32          l_dwBCount = 0;
    tUINT8          *l_pVArgs   = (tUINT8*)(i_ppFormat) + sizeof(tXCHAR*);
    tBOOL            l_bDesc    = FALSE;
    size_t           l_szTrace  = 0;
    sP7C_Data_Chunk *l_pChunk; //we do not initialize it here, we do it later

    LOCK_ENTER(m_sCS);

    m_dwSequence ++;

    if (    (FALSE == m_bActive)
         || (FALSE == m_sStatus.bConnected)
         || (    ((i_hModule)  && (i_eLevel < ((sP7Trace_Module*)i_hModule)->eVerbosity))
              || ((!i_hModule) && (i_eLevel < m_eVerbosity))
            )
       )
    {
        l_bReturn = FALSE;
        goto l_lblExit;
    }

    ////////////////////////////////////////////////////////////////////////////
    //if user specify direct Trace ID
    if (    (1 <= i_wTrace_ID)
         && (P7_TRACE_DESC_HARDCODED_COUNT > i_wTrace_ID)
       )
    {
        l_pDesc = m_pDesc_Array[i_wTrace_ID];
        if (NULL == l_pDesc)
        {
            l_pDesc = new (m_cMemory.Alloc(sizeof(CP7Trace_Desc)))
                          CP7Trace_Desc(m_cMemory,
                                        i_wTrace_ID, 
                                        i_wLine,
                                        (i_hModule) ? ((sP7Trace_Module*)i_hModule)->wModuleID : 0,
                                        i_pFile, 
                                        i_pFunction, 
                                        i_ppFormat,
                                        i_pKey,
                                        m_dwFlags
                                       );
            m_pDesc_Array[i_wTrace_ID] = l_pDesc;

        #if defined(P7TRACE_NO_VA_ARG_OPTIMIZATION)
            if ((l_pDesc->Get_Arguments_Count() * 16) > m_szVargs)
            {
                m_szVargs = l_pDesc->Get_Arguments_Count() * 16;
                m_pVargs  = (tUINT8*)m_cMemory.Reuse(CMemoryManager::eVaValues, m_szVargs);
            }
        #endif
        }
    }
    else //if we should use map to find it
    {
        //This moment should be clarified:
        //Here we made search through RB tree by key. Key consist of 2 values
        // - address of function
        // - address of format string
        //They located in stack [i_pFunction][*i_ppFormat]
        //Function eTrace work with stack of upper function like Trace(...)
        //and we should operate by stack of that function. This is why here
        //we get i_ppFormat, convert it to pointer (4 or 8 bytes depending on OS)
        //and subtract one element to get pointer to the function name.
        l_pDesc = m_cDescU_Tree.Find(i_pKey);
        if (    (NULL == l_pDesc)
             && (0xFFFF > m_wDesc_Tree_ID)
           )
        {
            m_wDesc_Tree_ID ++;
            l_pDesc = new (m_cMemory.Alloc(sizeof(CP7Trace_Desc)))
                          CP7Trace_Desc(m_cMemory,
                                        m_wDesc_Tree_ID, 
                                        i_wLine, 
                                        (i_hModule) ? ((sP7Trace_Module*)i_hModule)->wModuleID : 0,
                                        i_pFile, 
                                        i_pFunction, 
                                        i_ppFormat,
                                        i_pKey,
                                        m_dwFlags
                                       );
            //in stack we have pointer of the function and then format, 2 values
            m_cDescU_Tree.Push(l_pDesc, i_pKey);

        #if defined(P7TRACE_NO_VA_ARG_OPTIMIZATION)
            if ((l_pDesc->Get_Arguments_Count() * 16) > m_szVargs)
            {
                m_szVargs = l_pDesc->Get_Arguments_Count() * 16;
                m_pVargs  = (tUINT8*)m_cMemory.Reuse(CMemoryManager::eVaValues, m_szVargs);
            }
        #endif
        }
    }

    if (    (NULL  == l_pDesc)
         || (FALSE == l_pDesc->Is_Initialized())
         //|| (l_pFormat != l_pDesc->Get_Key())
       )
    {
        l_bReturn = FALSE;
        goto l_lblExit;
    }

    l_pBlocks = l_pDesc->Get_Blocks(&l_dwBCount);
    //increase chunks count if it is necessary
    if (    (l_dwBCount)
         && ((m_pChk_Curs + l_dwBCount + TRACE_EXTRA_CHUNKS) >= m_pChk_Tail)
       )
    {
        if (FALSE == Inc_Chunks(l_dwBCount + TRACE_EXTRA_CHUNKS))
        {
            l_bReturn = FALSE;
            goto l_lblExit;
        }
    }

    l_pChunk = m_pChk_Curs;
    l_dwSize = m_dwChk_Size;

    //trace description have to send again
    if (m_sStatus.dwResets != l_pDesc->Get_Resets())
    {
        l_pDesc->Set_Resets(m_sStatus.dwResets);

        l_bDesc         = TRUE;
        l_pChunk->pData = l_pDesc->Get_Buffer(&l_pChunk->dwSize);
        l_dwSize       += l_pChunk->dwSize;
        l_pChunk ++;
    }

    //we should also add all variable parameters length ... later
    l_szTrace = sizeof(m_sHeader_Data);
    //SET_EXT_HEADER_SIZE(m_sHeader_Data.sCommonRaw, );
    //m_sHeader_Data.sCommon.dwSize = sizeof(m_sHeader_Data); 

    m_sHeader_Data.bLevel     = (tUINT8)i_eLevel;
    m_sHeader_Data.bProcessor = (tUINT8)CProc::Get_Processor();
    m_sHeader_Data.dwThreadID = CProc::Get_Thread_Id();
    m_sHeader_Data.wID        = l_pDesc->Get_ID();
    m_sHeader_Data.dwSequence = m_dwSequence;

    if (! m_sConf.pTimestamp_Callback )
    {
        m_sHeader_Data.qwTimer = GetPerformanceCounter();
    }
    else
    {
       m_sHeader_Data.qwTimer = m_sConf.pTimestamp_Callback(m_sConf.pContext);
    }

    l_pChunk->dwSize = sizeof(m_sHeader_Data);
    l_pChunk->pData  = &m_sHeader_Data;

    l_pChunk ++;

    //case for platforms where variadic arguments located in stack in specific order
    //or using general purposes registers
    #if defined(P7TRACE_NO_VA_ARG_OPTIMIZATION)
    {
        tUINT32 l_dwArgC;
        const sP7Trace_Arg *l_pArgV = l_pDesc->Get_Arguments(l_dwArgC);
        l_pVArgs = m_pVargs;
        while (l_dwArgC--)
        {
        #if defined (GTX32)
            if (4 == l_pArgV->bSize) //most probable case
            {
                *(tUINT32*)l_pVArgs = va_arg(*i_pVa_List, tUINT32);
            }
            else
        #endif
            if (8 == l_pArgV->bSize)
            {
                //GCC sometimes optimizes storing of double values and access to it requires
                //specific case, "bardak i razruha v golovah"
                if (P7TRACE_ARG_TYPE_DOUBLE != l_pArgV->bType)
                {
                //in some platforms access to 64 not aligned variable is illegal, if it 
                //the case, please active the macro P7TRACE_64BITS_ALIGNED_ACCESS in P7_Trace.h
                #if defined(P7TRACE_64BITS_ALIGNED_ACCESS)
                    tUINT64 l_qwVal = va_arg(*i_pVa_List, tUINT64);
                    memcpy(l_pVArgs, &l_qwVal, sizeof(tUINT64));
                #else
                    *(tUINT64*)l_pVArgs = va_arg(*i_pVa_List, tUINT64);
                #endif
                }
                else
                {
                    //in some platforms access to 64 not aligned variable is illegal, if it
                    //the case, please active the macro P7TRACE_64BITS_ALIGNED_ACCESS in P7_Trace.h
                    #if defined(P7TRACE_64BITS_ALIGNED_ACCESS)
                        double l_qwVal = va_arg(*i_pVa_List, double);
                        memcpy(l_pVArgs, &l_qwVal, sizeof(double));
                    #else
                        *(double*)l_pVArgs = va_arg(*i_pVa_List, double);
                    #endif
                }
            }
            else if (0 == l_pArgV->bSize) 
            {
                *(void**)l_pVArgs = va_arg(*i_pVa_List, void*);
                l_pVArgs += SIZE_OF_ARG(void*); //String has 0 size (0 == l_pArgV->bSize)
            }
            else //if (SIZE_OF_ARG(uintmax_t) == l_pArgV->bSize)
            {
                //in some platforms access to 64 not aligned variable is illegal, if it
                //the case, please active the macro P7TRACE_64BITS_ALIGNED_ACCESS in P7_Trace.h
                #if defined(P7TRACE_64BITS_ALIGNED_ACCESS)
                    uintmax_t l_qwVal = va_arg(*i_pVa_List, uintmax_t);
                    memcpy(l_pVArgs, &l_qwVal, sizeof(uintmax_t));
                #else
                    *(uintmax_t*)l_pVArgs = va_arg(*i_pVa_List, uintmax_t);
                #endif
            }

            l_pVArgs += l_pArgV->bSize;
            l_pArgV++;
        }
        l_pVArgs = m_pVargs;
    }
    #endif

    while (l_dwBCount --)
    {
        if (0 <= (*l_pBlocks))
        {
            l_pChunk->dwSize = (*l_pBlocks);
            l_pChunk->pData  = l_pVArgs;
            l_pVArgs        += (*l_pBlocks);
        }
        else if (P7TRACE_ITEM_BLOCK_ASTRING == (*l_pBlocks))
        {
            l_pChunk->pData = *(char**)l_pVArgs;

            if (l_pChunk->pData)
            {
                l_pChunk->dwSize = (tUINT32)strlen(*(char**)l_pVArgs) + 1;
            }
            else
            {
                l_pChunk->pData  = const_cast<char*>("(NULL)");
                l_pChunk->dwSize = 7u;
            }

            l_pVArgs += sizeof(char*);
        }
#ifndef UTF8_ENCODING
        else if (P7TRACE_ITEM_BLOCK_USTRING16 == (*l_pBlocks))
        {
            l_pChunk->pData = *(wchar_t**)l_pVArgs;

            if (l_pChunk->pData)
            {
                l_pChunk->dwSize = (tUINT32)((wcslen(*(wchar_t**)l_pVArgs) + 1) * sizeof(wchar_t));
            }
            else
            {
                l_pChunk->pData  = const_cast<wchar_t*>(L"(NULL)");
                l_pChunk->dwSize = sizeof(wchar_t) * 7u;
            }

            l_pVArgs += sizeof(wchar_t*);
        }
#else   
        else if (P7TRACE_ITEM_BLOCK_USTRING32 == (*l_pBlocks))
        {
            l_pChunk->pData = *(wchar_t**)l_pVArgs;

            if (l_pChunk->pData)
            {
                l_pChunk->dwSize = (tUINT32)((wcslen(*(wchar_t**)l_pVArgs) + 1) * sizeof(wchar_t));
            }
            else
            {
                l_pChunk->pData  = const_cast<wchar_t*>(L"(NULL)");
                l_pChunk->dwSize = sizeof(wchar_t) * 7u;
            }

            l_pVArgs += sizeof(wchar_t*);
        }
#endif
        else if (P7TRACE_ITEM_BLOCK_ASTRING_FIX == (*l_pBlocks))
        {
            l_pChunk->pData  = const_cast<char*>("([x]s is unsupported by P7)");
            l_pChunk->dwSize = sizeof(char) * 28u;
            l_pVArgs += sizeof(char*);
        }
        else if (P7TRACE_ITEM_BLOCK_WSTRING_FIX == (*l_pBlocks))
        {
            l_pChunk->pData  = const_cast<wchar_t*>(L"([x]s is unsupported by P7)");
            l_pChunk->dwSize = sizeof(wchar_t) * 28u;
            l_pVArgs += sizeof(wchar_t*);
        }

        l_szTrace += (size_t)l_pChunk->dwSize;

        l_pChunk ++;
        l_pBlocks++;
    }

    //Put extensions.../////////////////////////////////////////////////////////////
    if (i_hModule)
    {
    #if defined(__linux__) //fix alignment and GCC warnings
        memcpy(m_pExtensions, &(((sP7Trace_Module*)i_hModule)->wModuleID), sizeof(tUINT16));
    #else
        *(tUINT16*)m_pExtensions = ((sP7Trace_Module*)i_hModule)->wModuleID;
    #endif

        m_pExtensions[2]         = (tUINT8)EP7TRACE_EXT_MODULE_ID;
        m_pExtensions[3]         = 1;
        l_pChunk->pData          = m_pExtensions;
        l_pChunk->dwSize         = 4u;
    }
    else
    {
        m_pExtensions[0]         = 0;
        l_pChunk->pData          = m_pExtensions;
        l_pChunk->dwSize         = 1u;
    }

    l_szTrace += l_pChunk->dwSize;
    SET_EXT_HEADER_SIZE(m_sHeader_Data.sCommonRaw, l_szTrace);
    l_pChunk ++;

    l_dwSize += (tUINT32)l_szTrace;//m_sHeader_Data.sCommon.dwSize;

    if (ECLIENT_STATUS_OK != m_pClient->Sent(m_dwChannel_ID,
                                             m_pChk_Head,
                                             (tUINT32)(l_pChunk - m_pChk_Head),
                                             l_dwSize
                                            )
       )
    {
        //if delivery was failed and we try to deliver also trace description
        //we set marker that is should be redelivered next time
        if (l_bDesc)
        {
            l_pDesc->Set_Resets(RESET_UNDEFINED);
        }

        l_bReturn = FALSE;
    }
    else
    {
        m_pChk_Curs  = m_pChk_Head;
        m_dwChk_Size = 0;
    }

l_lblExit:
    LOCK_EXIT(m_sCS);

    return l_bReturn;
}// Trace_Raw


////////////////////////////////////////////////////////////////////////////////
// Inc_Chunks                                      
__forceinline tBOOL CP7Trace::Inc_Chunks(tUINT32 i_dwInc)
{
    size_t           l_szUsed  = m_pChk_Curs - m_pChk_Head;
    sP7C_Data_Chunk *l_pChunks = (sP7C_Data_Chunk*)realloc(m_pChk_Head, 
                                                           (m_dwChk_Count + i_dwInc) * sizeof(sP7C_Data_Chunk)
                                                          );

    if (!l_pChunks)
    {
        return FALSE;
    }


    m_pChk_Head    = l_pChunks;
    m_pChk_Curs    = m_pChk_Head + l_szUsed; //set current to new/old location
    m_dwChk_Count += i_dwInc;
    m_pChk_Tail    = m_pChk_Head + m_dwChk_Count;

    return TRUE;
}// Inc_Chunks


////////////////////////////////////////////////////////////////////////////////
//Is_VarArgs()
//Some processors have funny stack alignment rules, for example arm under 32 bits
//for 64 bits variables request 64 bit address stack alignment, some uses gen.
//registers, some changes order, etc.
tBOOL CP7Trace::Is_VarArgs(const CP7Trace::sStack_Desc *i_pDesc, ...)
{
    const tUINT8 *l_pData   = (const tUINT8 *)&i_pDesc + sizeof(void*);
    tBOOL         l_bReturn = TRUE;
    va_list       l_pVal;

    va_start(l_pVal, i_pDesc);
    while (1)
    {
        if (CP7Trace::sStack_Desc::eTypeU32 == i_pDesc->eType)
        {
            if (*(tUINT32*)l_pData != va_arg(l_pVal, tUINT32))
            {
                l_bReturn = FALSE;
                break;
            }

            l_pData += SIZE_OF_ARG(tUINT32);
        }
        else if (CP7Trace::sStack_Desc::eTypeU64 == i_pDesc->eType)
        {
            if (*(tUINT64*)l_pData != va_arg(l_pVal, tUINT64))
            {
                l_bReturn = FALSE;
                break;
            }

            l_pData += SIZE_OF_ARG(tUINT64);
        }
        else if (CP7Trace::sStack_Desc::eTypeD64 == i_pDesc->eType)
        {
            if (*(tDOUBLE*)l_pData != va_arg(l_pVal, tDOUBLE))
            {
                l_bReturn = FALSE;
                break;
            }

            l_pData += SIZE_OF_ARG(tUINT64);
        }

        if (i_pDesc->bLast)
        {
            break;
        }
        else
        {
            i_pDesc++;
        }
    }

    va_end(l_pVal);

    return l_bReturn;
}
