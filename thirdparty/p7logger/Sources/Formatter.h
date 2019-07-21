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
// This header file provide functionality of text formatting using var. args.  /
////////////////////////////////////////////////////////////////////////////////

#ifndef FORMATTER_H
#define FORMATTER_H

#include <stdint.h>

#define FORMATTER_NO_WIDTH        0
#define FORMATTER_ARG_WIDTH      -1

#define FORMATTER_NO_PRECISION    0
#define FORMATTER_ARG_PRECISION  -1

#define FORMATTER_MAX_WIDTH       1024

#ifndef min
    #define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#if !defined(UINTMAX_MAX)
    typedef uint64_t uintmax_t;
    #define UINTMAX_MAX	0xffffffffffffffffU
#endif

#if !defined(INTMAX_MAX)
    typedef int64_t intmax_t;
     #define INTMAX_MIN		(-0x7fffffffffffffff - 1)
     #define INTMAX_MAX		0x7fffffffffffffff
#endif


#if !defined(min)
    #define min(a, b)  (((a) < (b)) ? (a) : (b)) 
#endif

////////////////////////////////////////////////////////////////////////////////
class CFormatter
{
public:
    ////////////////////////////////////////////////////////////////////////////
    struct sBuffer
    {
        tINT32 volatile lReference;
        tXCHAR         *pBuffer;
        size_t          szBuffer;

        ////////////////////////////////////////////////////////////////////////
        sBuffer(size_t i_szBuffer)
            : lReference(1)
            , pBuffer(NULL)
            , szBuffer(i_szBuffer)
        {
            if (szBuffer)
            {
                szBuffer = 256;
            }

            pBuffer = (tXCHAR*)malloc(szBuffer * sizeof(tXCHAR));
        }

        ////////////////////////////////////////////////////////////////////////
        tBOOL Realloc(size_t i_szSize)
        {
            tXCHAR *l_pTmp = (tXCHAR*)realloc(pBuffer, i_szSize * sizeof(tXCHAR));
            if (l_pTmp)
            {
                pBuffer  = l_pTmp;
                szBuffer = i_szSize;
                return TRUE;
            }
            
            return FALSE;
        }

        ////////////////////////////////////////////////////////////////////////
        virtual tINT32 Add_Ref()
        {
            return ATOMIC_INC(&lReference);
        }

        ////////////////////////////////////////////////////////////////////////
        virtual tINT32 Release()
        {
            tINT32 l_lResult = ATOMIC_DEC(&lReference);
            if ( 0 >= l_lResult )
            {
                delete this;
            }

            return l_lResult;
        }

    protected:
        ////////////////////////////////////////////////////////////////////////
        virtual ~sBuffer()
        {
            if (pBuffer)
            {
                free(pBuffer);
                pBuffer = NULL;
            }
            szBuffer = 0;
        }
    };//struct sBuffer

protected:
    union uValue
    {
        tINT8     i8;
        tUINT8    u8;
        tINT16    i16;
        tUINT16   u16;
        tINT32    i32;
        tUINT32   u32;
        tINT64    i64;
        tUINT64   u64;
        intmax_t  iMax;
        uintmax_t uMax;
        tDOUBLE   d64;
        tUINT8    pData[32];
    };

    enum eSize
    {
        eSize32,
        eSize64,
        eSizehh,
        eSizeh,
        eSizew,
        eSizeL,
        eSizell,
        eSizel,
        eSizeI,
        eSizez,
        eSizej,
        eSizet,
        eSizeUnk
    };

    enum eType
    {
        eTypeDouble,
        eTypeString,
        eTypeChar,
        eTypeIntDec,
        eTypeUintDec,
        eTypeUintHex,
        eTypeUintHEX,
        eTypeUintOct,
        eTypePointer,
        eTypeUintBin,
        eTypeNone,

        eTypeMax
    };

    ////////////////////////////////////////////////////////////////////////////
    struct sArg
    {
        tBOOL              bFlagLeftAlign;
        tBOOL              bFlagSign;
        tBOOL              bFlagGrid;
        tXCHAR             cPadding;
        tXCHAR            *pPrefix;
        size_t             szPrefix;
        CFormatter::eSize  eSize;
        CFormatter::eType  eType;
        tINT32             iWidth; //0 - no width, -1 - use argument to specify width
        tINT32             iPrecision; //-2 no precision, -1 - use argument to specify precision
        tXCHAR            *pDouble; //format string for double
        sArg              *pNext;

        ////////////////////////////////////////////////////////////////////////
        sArg()
        {
            bFlagLeftAlign = bFlagSign = bFlagGrid = FALSE;
            cPadding   = TM(' ');
            pPrefix    = NULL;
            szPrefix   = 0;
            eSize      = eSizeUnk;
            eType      = eTypeNone;
            iWidth     = FORMATTER_NO_WIDTH;
            iPrecision = FORMATTER_NO_PRECISION;
            pDouble    = NULL; 
            pNext      = NULL;
        }

        ////////////////////////////////////////////////////////////////////////
        ~sArg()
        {
            if (pPrefix)
            {
                free(pPrefix);
                pPrefix = NULL;
            }

            if (pDouble)
            {
                free(pDouble);
                pDouble = NULL;
            }

            if (pNext)
            {
                delete pNext;
                pNext = NULL;
            }
        }
    };//sArg

    sArg         *m_pArgHead;
    sBuffer      *m_pBuffer;
    sP7Trace_Arg *m_pArgs;
    size_t        m_szArgs;
    tBOOL         m_bError;
    tBOOL         m_bLittleEndian;
public:
    ////////////////////////////////////////////////////////////////////////////
    //CFormatter
    CFormatter(const tXCHAR        *i_pFormat,         
               sP7Trace_Arg        *i_pArgs,
               size_t               i_szArgs,
               CFormatter::sBuffer *i_pBuffer = NULL
              )
        : m_pArgHead(NULL)
        , m_pBuffer(i_pBuffer)
        , m_pArgs(i_pArgs)
        , m_szArgs(i_szArgs)
        , m_bError(FALSE)
        , m_bLittleEndian(TRUE)
    {
        const tXCHAR *l_pCursor    = i_pFormat;
        const tXCHAR *l_pHead      = i_pFormat;
        const tXCHAR *l_pPercent   = i_pFormat;
        sArg         *l_pTail      = NULL;
        sArg         *l_pArg       = NULL;
        size_t        l_szArgs     = 0;

        if (!m_pBuffer)
        {
            m_pBuffer = new sBuffer(1024);
        }
        else
        {
            m_pBuffer->Add_Ref();
        }

        while (*l_pCursor)
        {
            if (TM('%') == *l_pCursor)
            {
                l_pCursor++;

                if (TM('%') != *l_pCursor)
                {
                    l_pPercent   = l_pCursor - 1;
                    l_pArg       = AddArg(l_pHead, (size_t)(l_pCursor - l_pHead - 1));
                }
            }

            if (l_pArg)
            {
                ///////////////////////////////////////////////////////////////
                //Flags
                while (1) //check flags
                {
                    if (TM('+') == *l_pCursor) 
                    {   
                        l_pArg->bFlagSign = TRUE;
                        l_pCursor++;
                    }
                    else if (TM('-') == *l_pCursor)
                    {
                        l_pArg->bFlagLeftAlign = TRUE;
                        l_pCursor++;
                    }
                    else if (TM(' ') == *l_pCursor)
                    {
                        l_pCursor++;
                    }
                    else if (TM('#') == *l_pCursor)
                    {
                        l_pArg->bFlagGrid = TRUE;
                        l_pCursor++;
                    }
                    else if (TM('0') == *l_pCursor)
                    {
                        l_pArg->cPadding = TM('0');
                        l_pCursor++;
                    }
                    else if (TM('\'') == *l_pCursor)
                    {
                        //ignored for a while
                    }
                    else
                    {
                        break;
                    }
                }

                ///////////////////////////////////////////////////////////////
                //width
                tINT32 l_iWidth = 0;

                if (TM('*') == *l_pCursor)
                {
                    l_iWidth = FORMATTER_ARG_WIDTH;
                    l_pCursor ++;
                }
                else while (    (TM('0') <= *l_pCursor)
                             && (TM('9') >= *l_pCursor)
                           )
                {
                    l_iWidth = l_iWidth * 10 + ((*l_pCursor) - TM('0'));
                    l_pCursor++;
                }

                if (0 != l_iWidth)
                {
                    l_pArg->iWidth = l_iWidth;
                }

                ///////////////////////////////////////////////////////////////
                //precision
                if (TM('.') == *l_pCursor)
                {
                    l_pCursor ++;

                    if (TM('*') == *l_pCursor)
                    {
                        l_pArg->iPrecision = FORMATTER_ARG_PRECISION;
                        l_pCursor ++;
                    }
                    else 
                    {
                        tINT32 l_iPrecision = 0;
                        while (    (TM('0') <= *l_pCursor)
                                && (TM('9') >= *l_pCursor)
                                )
                        {
                            l_iPrecision = l_iPrecision * 10 + ((*l_pCursor) - TM('0'));
                            l_pCursor++;
                        }

                        if (l_iPrecision)
                        {
                            l_pArg->iPrecision = l_iPrecision;
                        }
                    }
                }

                ///////////////////////////////////////////////////////////////
                //size
                l_pArg->eSize = GetSize(l_pCursor);

                ///////////////////////////////////////////////////////////////
                //type
                tBOOL l_bFound = FALSE;
                if (    (TM('d') == *l_pCursor)
                     || (TM('i') == *l_pCursor)
                   )
                {
                    l_pArg->eType = eTypeIntDec;
                    l_bFound = TRUE;
                }
                else if (TM('x') == *l_pCursor)
                {
                    l_pArg->eType = eTypeUintHex;
                    l_bFound = TRUE;
                }
                else if (TM('X') == *l_pCursor)
                {
                    l_pArg->eType = eTypeUintHEX;
                    l_bFound = TRUE;
                }
                else if (TM('u') == *l_pCursor)
                {
                    l_pArg->eType = eTypeUintDec;
                    l_bFound = TRUE;
                }
                else if (    (TM('c') == *l_pCursor)
                          || (TM('C') == *l_pCursor)
                        )
                {
                    l_pArg->eType = eTypeChar;
                    l_bFound = TRUE;
                }
                else if (    (TM('s') == *l_pCursor)
                          || (TM('S') == *l_pCursor)
                        )
                {
                    l_pArg->eType = eTypeString;
                    l_bFound = TRUE;
                }
                else if (TM('b') == *l_pCursor)
                {
                    l_pArg->eType = eTypeUintBin;
                    l_bFound = TRUE;
                }
                else if (    (TM('a') == *l_pCursor) 
                          || (TM('A') == *l_pCursor)
                          || (TM('f') == *l_pCursor)
                          || (TM('e') == *l_pCursor)
                          || (TM('E') == *l_pCursor)
                          || (TM('g') == *l_pCursor)
                          || (TM('G') == *l_pCursor)
                        )
                {
                    size_t l_szLength = (size_t)(l_pCursor - l_pPercent + 2);
                    l_pArg->pDouble = (tXCHAR*)malloc(l_szLength * sizeof(tXCHAR));
                    if (l_pArg->pDouble)
                    {
                        memcpy(l_pArg->pDouble, l_pPercent, (l_szLength - 1) * sizeof(tXCHAR));
                        l_pArg->pDouble[l_szLength - 1] = 0;
                    }
                    l_bFound = TRUE;
                    l_pArg->eType = eTypeDouble;
                }
                else if (TM('o') == *l_pCursor)
                {
                    l_pArg->eType = eTypeUintOct;
                    l_bFound = TRUE;
                }
                else if (TM('p') == *l_pCursor)//(TM('n') == *l_pCursor) ->>Ignored, not supported
                {
                    l_pArg->bFlagGrid = TRUE;
                    l_pArg->eType = eTypePointer;
                    l_bFound = TRUE;
                }

                if (l_bFound)
                {
                    l_szArgs ++;

                    if (FORMATTER_ARG_PRECISION == l_pArg->iPrecision) l_szArgs ++;
                    if (FORMATTER_ARG_WIDTH     == l_pArg->iWidth)     l_szArgs ++;

                    l_pHead = l_pCursor + 1;
                    if (!l_pTail)
                    {
                        m_pArgHead = l_pArg;
                    }
                    else
                    {
                        l_pTail->pNext = l_pArg;
                    }

                    l_pTail = l_pArg;
                    l_pCursor++;
                }
                else if (l_pArg)
                {
                    delete l_pArg;
                }

                l_pArg = NULL;
            }//if (l_pArg)
            else
            {
                l_pCursor++;
            }
        }

        if (l_pArg)
        {
            delete l_pArg;
            l_pArg = NULL;
        }

        if (l_pCursor != l_pHead)
        {
            l_pArg = AddArg(l_pHead, (size_t)(l_pCursor - l_pHead));

            if (!l_pTail)
            {
                m_pArgHead = l_pArg;
            }
            else
            {
                l_pTail->pNext = l_pArg;
            }
        }

        if (l_szArgs != m_szArgs)
        {
            m_bError = TRUE;
        }
    }//CFormatter

    ////////////////////////////////////////////////////////////////////////////
    //~CFormatter
    virtual ~CFormatter()
    {
        if (m_pArgHead)
        {
            delete m_pArgHead;
            m_pArgHead = NULL;
        }

        if (m_pBuffer)
        {
            m_pBuffer->Release();
            m_pBuffer = NULL;
        }

        m_pArgs  = NULL;
        m_szArgs = 0;
    }//~CFormatter

    ////////////////////////////////////////////////////////////////////////////
    //EnableBigEndian
    void EnableBigEndian()
    {
        m_bLittleEndian = FALSE;
    }

#define FORMAT_SIGN\
    if (l_bMinus) {*l_pIter++ = TM('-');}\
    else if (l_pArg->bFlagSign) {*l_pIter++ = TM('+');}\

#define FORMAT_X_PREFIX\
    if (l_pArg->bFlagGrid) {*l_pIter++ = TM('0'); *l_pIter++ = TM('x');}\

#define FORMAT_0_PREFIX\
    if (l_pArg->bFlagGrid) {*l_pIter++ = TM('0');}\

#define FORMAT_B_PREFIX\
    if (l_pArg->bFlagGrid) {*l_pIter++ = TM('b');}\


#define FORMAT_NO_SIGN
#define FORMAT_NO_PREFIX

#define FORMAT_DIGIT(i_uBase, i_pDigits, SIGN_MACRO, PREFIX_MACRO)\
    l_pDigit  = m_pBuffer->pBuffer + m_pBuffer->szBuffer - 1;\
    l_pTail   = l_pDigit;\
    *l_pDigit = TM('0');\
    if (l_uValue.uMax) l_pDigit++;\
    while(l_uValue.uMax)\
    {\
        *(--l_pDigit) = *(i_pDigits + (size_t)(l_uValue.uMax % i_uBase));\
        l_uValue.uMax /= i_uBase;\
    }\
    l_szAdd       = (size_t)(l_pTail - l_pDigit + 1);\
    l_iPrecision -= (tINT32)l_szAdd;\
    l_iWidth     -= (tINT32)l_szAdd;\
    if (0 < l_iPrecision) l_iWidth -= l_iPrecision;\
    if (0 < l_iWidth)     l_szAdd += l_iWidth;\
    if (0 < l_iPrecision) l_szAdd += l_iPrecision;\
    if ((l_szReturn + l_szAdd + 8) >= i_szBuffer)\
    {\
        l_bError = TRUE;\
        break;\
    }\
    l_pIter = o_pBuffer + l_szReturn;\
    if (FALSE == l_pArg->bFlagLeftAlign)\
    {\
        if (TM(' ') == l_pArg->cPadding)\
        {\
            while (0 < l_iWidth--) *l_pIter++ = l_pArg->cPadding; \
        }\
        SIGN_MACRO\
        PREFIX_MACRO\
        if (TM('0') == l_pArg->cPadding)\
        {\
            while (0 < l_iWidth--) *l_pIter++ = l_pArg->cPadding; \
        }\
        while (0 < l_iPrecision--) *l_pIter++ = TM('0'); \
        l_szAdd = (size_t)(l_pTail - l_pDigit + 1);\
        memcpy(l_pIter, l_pDigit, sizeof(tXCHAR) * l_szAdd);\
        l_pIter += l_szAdd;\
    }\
    else\
    {\
        SIGN_MACRO\
        PREFIX_MACRO\
        while (0 < l_iPrecision--) *l_pIter++ = TM('0'); \
        if (TM('0') == l_pArg->cPadding)\
        {\
            while (0 < l_iWidth--) *l_pIter++ = l_pArg->cPadding; \
        }\
        l_szAdd = (size_t)(l_pTail - l_pDigit + 1);\
        memcpy(l_pIter, l_pDigit, sizeof(tXCHAR) * l_szAdd);\
        l_pIter += l_szAdd;\
        if (TM(' ') == l_pArg->cPadding)\
        {\
            while (0 < l_iWidth--) *l_pIter++ = l_pArg->cPadding; \
        }\
    }\
    l_szReturn = (size_t)(l_pIter - o_pBuffer);\
//#define FORMAT_DIGIT

#define COPY_STR(SRC_TYPE, DST_TYPE)\
    l_pIter = o_pBuffer + l_szReturn;\
    l_pTail = o_pBuffer + i_szBuffer - 4;\
    l_iReturn = 0;\
    while (*(SRC_TYPE*)i_pValues)\
    {\
        if ((l_pTail - l_pIter) > 2)\
        {\
            *l_pIter++ = (DST_TYPE)*(SRC_TYPE*)i_pValues; \
            l_iReturn++;\
        }\
        else\
        {\
            l_iReturn = -1;\
            break;\
        }\
        i_pValues += sizeof(SRC_TYPE);\
    }\
    i_pValues += sizeof(SRC_TYPE);\
//#define COPY_STR

#define COPY_ARG(DstVar, SrcSignificantSize, SrcTotalSize)\
    if (m_bLittleEndian)\
    {\
        memcpy(&(DstVar), i_pValues, min((SrcSignificantSize), (SrcTotalSize)));\
    }\
    else\
    {\
        memcpy(((tUINT8*)&(DstVar)) + (sizeof(DstVar) - (SrcSignificantSize)), \
                i_pValues + ((SrcTotalSize) - (SrcSignificantSize)), \
                SrcSignificantSize\
                );\
    }\
//#define COPY_ARG

    ////////////////////////////////////////////////////////////////////////////
    //Format
    tINT32 Format(tXCHAR       *o_pBuffer,
                  size_t        i_szBuffer, 
                  const tUINT8 *i_pValues
                 )
    {
        const static tXCHAR g_pHEX[]  = TM("0123456789ABCDEF");
        const static tXCHAR g_pHex[]  = TM("0123456789abcdef");
        const static size_t g_pSize[P7TRACE_ARGS_COUNT] = {0,                 //P7TRACE_ARG_TYPE_UNK   
                                                           1,                 //P7TRACE_ARG_TYPE_CHAR, P7TRACE_ARG_TYPE_INT8  
                                                           2,                 //P7TRACE_ARG_TYPE_CHAR16 
                                                           2,                 //P7TRACE_ARG_TYPE_INT16 
                                                           4,                 //P7TRACE_ARG_TYPE_INT32 
                                                           8,                 //P7TRACE_ARG_TYPE_INT64 
                                                           8,                 //P7TRACE_ARG_TYPE_DOUBLE
                                                           0,                 //P7TRACE_ARG_TYPE_PVOID 
                                                           0,                 //P7TRACE_ARG_TYPE_USTR16  
                                                           0,                 //P7TRACE_ARG_TYPE_STRA  
                                                           0,                 //P7TRACE_ARG_TYPE_USTR8  
                                                           0,                 //P7TRACE_ARG_TYPE_USTR32  
                                                           4,                 //P7TRACE_ARG_TYPE_CHAR32  
                                                           sizeof(uintmax_t), //P7TRACE_ARG_TYPE_INTMAX
                                                          };

        sP7Trace_Arg *l_pP7Args  = m_pArgs;
        size_t        l_szReturn = 0;
        tBOOL         l_bError   = FALSE;
        sArg         *l_pArg     = m_pArgHead;
        tINT32        l_iWidth;
        tINT32        l_iPrecision;
        uValue        l_uValue;
        tXCHAR       *l_pDigit;
        tXCHAR       *l_pTail;
        tXCHAR       *l_pIter;
        size_t        l_szAdd;

        if (m_bError)
        {
            return PSPrint(o_pBuffer, i_szBuffer, TM("Format string error"));
        }

        while(l_pArg)
        {
            //Copy text prefix
            if (l_pArg->pPrefix)
            {
                if ((l_szReturn + l_pArg->szPrefix) < i_szBuffer)
                {
                    memcpy(o_pBuffer + l_szReturn, l_pArg->pPrefix, l_pArg->szPrefix * sizeof(tXCHAR));
                    l_szReturn += l_pArg->szPrefix;
                }
                else
                {
                    l_bError = TRUE;
                    break;
                }
            }

            //Retrieving width and precision
            l_iWidth     = l_pArg->iWidth;
            l_iPrecision = l_pArg->iPrecision;

            if (FORMATTER_ARG_WIDTH == l_iWidth)
            {
                COPY_ARG(l_iWidth, sizeof(tINT32), l_pP7Args->bSize);
                i_pValues += l_pP7Args->bSize; 
                l_pP7Args ++;

                if (0 > l_iWidth)
                { l_iWidth = 0; }
                else if (FORMATTER_MAX_WIDTH < l_iWidth)
                { l_iWidth = FORMATTER_MAX_WIDTH;}
            }
            if (FORMATTER_ARG_PRECISION == l_iPrecision)
            {
                COPY_ARG(l_iPrecision, sizeof(tINT32), l_pP7Args->bSize);
                i_pValues += l_pP7Args->bSize; 
                l_pP7Args ++;
                if (0 > l_iPrecision)
                { l_iPrecision = 0; }
                else if (FORMATTER_MAX_WIDTH < l_iPrecision) 
                { l_iPrecision = FORMATTER_MAX_WIDTH;}
            }

            if (eTypeIntDec == l_pArg->eType)
            {
                tBOOL l_bMinus = FALSE;
                if (P7TRACE_ARG_TYPE_INT32 == l_pP7Args->bType)
                {
                    tINT32 l_iVal = 0;
                    COPY_ARG(l_iVal, sizeof(tINT32), l_pP7Args->bSize);
                    l_uValue.iMax = l_iVal; 
                }
                else if (P7TRACE_ARG_TYPE_INT64 == l_pP7Args->bType)
                {
                    tINT64 l_iVal = 0ll;
                    COPY_ARG(l_iVal, sizeof(tINT64), l_pP7Args->bSize);
                    l_uValue.iMax = l_iVal;
                }
                else if (P7TRACE_ARG_TYPE_INT16 == l_pP7Args->bType)
                {
                    tINT16 l_iVal = 0;
                    COPY_ARG(l_iVal, sizeof(tINT16), l_pP7Args->bSize);
                    l_uValue.iMax = l_iVal; 
                }
                else if (P7TRACE_ARG_TYPE_INT8 == l_pP7Args->bType)
                {
                    tINT8 l_iVal = 0;
                    COPY_ARG(l_iVal, sizeof(tINT8), l_pP7Args->bSize);
                    l_uValue.iMax = l_iVal; 
                }
                else //if (P7TRACE_ARG_TYPE_INTMAX == l_pP7Args->bType)
                {
                    intmax_t l_iVal = 0ll;
                    memcpy(&l_iVal, i_pValues, min(g_pSize[l_pP7Args->bType], l_pP7Args->bSize));
                    l_uValue.iMax = l_iVal;
                }

                if (0 > l_uValue.iMax)
                {
                    l_bMinus = TRUE;
                    l_iWidth --;
                    l_uValue.iMax = -l_uValue.iMax;
                }

                if (l_pArg->bFlagSign) l_iWidth --;

                FORMAT_DIGIT(10ull, g_pHEX, FORMAT_SIGN, FORMAT_NO_PREFIX);
            }
            else if (eTypeUintDec == l_pArg->eType)
            {
                l_uValue.uMax = 0ull;
                COPY_ARG(l_uValue.uMax, g_pSize[l_pP7Args->bType], l_pP7Args->bSize);
                FORMAT_DIGIT(10ull, g_pHEX, FORMAT_NO_SIGN, FORMAT_NO_PREFIX);
            }
            else if (eTypeUintHex == l_pArg->eType)
            {
                l_uValue.uMax = 0ull;
                COPY_ARG(l_uValue.uMax, g_pSize[l_pP7Args->bType], l_pP7Args->bSize);
                if (l_pArg->bFlagGrid) {l_iWidth -=2;} //0x
                FORMAT_DIGIT(16ull, g_pHex, FORMAT_NO_SIGN, FORMAT_X_PREFIX);
            }
            else if (eTypeUintHEX == l_pArg->eType)
            {
                l_uValue.uMax = 0ull;
                COPY_ARG(l_uValue.uMax, g_pSize[l_pP7Args->bType], l_pP7Args->bSize);
                if (l_pArg->bFlagGrid) {l_iWidth -=2;} //0x
                FORMAT_DIGIT(16ull, g_pHEX, FORMAT_NO_SIGN, FORMAT_X_PREFIX);

                //Code snippet for test
                //l_pDigit  = m_pBuffer->pBuffer + m_pBuffer->szBuffer - 1;
                //l_pTail   = l_pDigit;
                //*l_pDigit = TM('0');
                //
                //if (l_uValue.uMax) l_pDigit++;
                //while(l_uValue.uMax)
                //{
                //    *(--l_pDigit) =*(g_pHEX + (size_t)(l_uValue.uMax % 16));
                //    l_uValue.uMax /= 16;
                //}
                //
                //l_szAdd       = (size_t)(l_pTail - l_pDigit + 1);
                //l_iPrecision -= (tINT32)l_szAdd;
                //l_iWidth     -= (tINT32)l_szAdd;
                //if (0 < l_iPrecision) l_iWidth -= l_iPrecision;
                //
                //if (0 < l_iWidth)     l_szAdd += l_iWidth;
                //if (0 < l_iPrecision) l_szAdd += l_iPrecision;
                //
                //if ((l_szReturn + l_szAdd + 1) >= i_szBuffer)
                //{
                //    l_bError = TRUE;
                //    break;
                //}
                //
                //l_pIter = o_pBuffer + l_szReturn;
                //
                //if (FALSE == l_pArg->bFlagLeftAlign)
                //{
                //    if (TM(' ') == l_pArg->cPadding)
                //    {
                //        while (0 < l_iWidth--) *l_pIter++ = l_pArg->cPadding; 
                //    }
                //
                //    //if (l_bMinus) {*l_pIter++ = TM('-'); l_iWidth --;}
                //    //else if (l_pArg->bFlagSign) {*l_pIter++ = TM('+'); l_iWidth --;}
                //    if (l_pArg->bFlagGrid) {*l_pIter++ = TM('0'); *l_pIter++ = TM('x');}
                //
                //    if (TM('0') == l_pArg->cPadding)
                //    {
                //        while (0 < l_iWidth--) *l_pIter++ = l_pArg->cPadding; 
                //    }
                //    while (0 < l_iPrecision--) *l_pIter++ = TM('0'); 
                //
                //    l_szAdd = (size_t)(l_pTail - l_pDigit + 1);
                //    memcpy(l_pIter, l_pDigit, sizeof(tXCHAR) * l_szAdd);
                //    l_pIter += l_szAdd;
                //}
                //else
                //{
                //    //if (l_bMinus) {*l_pIter++ = TM('-'); l_iWidth --;}
                //    //else if (l_pArg->bFlagSign) {*l_pIter++ = TM('+'); l_iWidth --;}
                //    if (l_pArg->bFlagGrid) {*l_pIter++ = TM('0'); *l_pIter++ = TM('x');}
                //
                //    while (0 < l_iPrecision--) *l_pIter++ = TM('0'); 
                //
                //    if (TM('0') == l_pArg->cPadding)
                //    {
                //        while (0 < l_iWidth--) *l_pIter++ = l_pArg->cPadding; 
                //    }
                //
                //    l_szAdd = (size_t)(l_pTail - l_pDigit + 1);
                //    memcpy(l_pIter, l_pDigit, sizeof(tXCHAR) * l_szAdd);
                //    l_pIter += l_szAdd;
                //
                //    if (TM(' ') == l_pArg->cPadding)
                //    {
                //        while (0 < l_iWidth--) *l_pIter++ = l_pArg->cPadding; 
                //    }
                //}
                //
                //l_szReturn = (size_t)(l_pIter - o_pBuffer);
            }
            else if (eTypeUintBin == l_pArg->eType)
            {
                l_uValue.uMax = 0ull;
                COPY_ARG(l_uValue.uMax, g_pSize[l_pP7Args->bType], l_pP7Args->bSize);
                if (l_pArg->bFlagGrid) {l_iWidth --;}//b
                FORMAT_DIGIT(2ull, g_pHEX, FORMAT_NO_SIGN, FORMAT_B_PREFIX);
            }
            else if (eTypeString == l_pArg->eType)
            {
                tINT32 l_iReturn = -1;
                if (P7TRACE_ARG_TYPE_USTR16 == l_pP7Args->bType)
                {
                #ifdef UTF8_ENCODING
                    l_iReturn = Convert_UTF16_To_UTF8((tWCHAR*)i_pValues, 
                                                      o_pBuffer + l_szReturn, 
                                                      (tUINT32)(i_szBuffer - l_szReturn)
                                                     );
                    //move var_arg pointer
                    while (*(tWCHAR*)i_pValues) i_pValues += sizeof(tWCHAR);
                    i_pValues += sizeof(tWCHAR);
                #else //UTF-16
                    COPY_STR(tWCHAR, tWCHAR);
                #endif                             
                }
                else if (P7TRACE_ARG_TYPE_STRA == l_pP7Args->bType)
                {
                    COPY_STR(tACHAR, tXCHAR);
                }
                else if (P7TRACE_ARG_TYPE_USTR8 == l_pP7Args->bType)
                {
                #ifdef UTF8_ENCODING
                    COPY_STR(tACHAR, tXCHAR);
                #else
                    l_iReturn = Convert_UTF8_To_UTF16((tACHAR*)i_pValues, 
                                                      o_pBuffer + l_szReturn, 
                                                      (tUINT32)(i_szBuffer - l_szReturn)
                                                     );
                    //move var_arg pointer
                    while (*(tACHAR*)i_pValues) i_pValues += sizeof(tACHAR);
                    i_pValues += sizeof(tACHAR);
                #endif                             
                }
                else if (P7TRACE_ARG_TYPE_USTR32 == l_pP7Args->bType)
                {
                #ifdef UTF8_ENCODING
                    l_iReturn = Convert_UTF32_To_UTF8((tUINT32*)i_pValues, 
                                                      o_pBuffer + l_szReturn, 
                                                      (tUINT32)(i_szBuffer - l_szReturn)
                                                     );
                #else
                    l_iReturn = Convert_UTF32_To_UTF16((tUINT32*)i_pValues, 
                                                       o_pBuffer + l_szReturn, 
                                                       (tUINT32)(i_szBuffer - l_szReturn)
                                                      );
                #endif                             
                    //move var_arg pointer
                    while (*(tUINT32*)i_pValues) i_pValues += sizeof(tUINT32);
                    i_pValues += sizeof(tUINT32);
                }

                if ( (l_iReturn >= 0) && ((size_t)(l_iReturn + 8) < (i_szBuffer - l_szReturn)) )
                { l_szReturn += (size_t)l_iReturn; }
                else { l_bError = TRUE; break; }
            }
            else if (eTypeChar == l_pArg->eType)
            {
                //7 bytes max for UTF-8 + zero
                if ((i_szBuffer - l_szReturn) < 7) {l_bError = TRUE; break;}

                if (P7TRACE_ARG_TYPE_CHAR == l_pP7Args->bType)
                {
                    tINT8 l_iVal = 0;
                    COPY_ARG(l_iVal, g_pSize[l_pP7Args->bType], l_pP7Args->bSize);

                    *(o_pBuffer + l_szReturn++) = l_iVal;
                }
                else if (P7TRACE_ARG_TYPE_CHAR16 == l_pP7Args->bType)
                {
                    tINT16 l_iVal = 0;
                    COPY_ARG(l_iVal, g_pSize[l_pP7Args->bType], l_pP7Args->bSize);

                #ifdef UTF8_ENCODING
                    tWCHAR l_pWstr[2] = {(tWCHAR)l_iVal, 0};
                    l_szReturn += (size_t)Convert_UTF16_To_UTF8((tWCHAR*)l_pWstr, 
                                                                o_pBuffer + l_szReturn, 
                                                                (tUINT32)(i_szBuffer - l_szReturn)
                                                               );
                #else
                    *(o_pBuffer + l_szReturn++) = l_iVal;
                #endif                             
                }
                else if (P7TRACE_ARG_TYPE_CHAR32 == l_pP7Args->bType)
                {
                    tUINT32 l_pVal[2] = {0, 0};
                    COPY_ARG(l_pVal[0], g_pSize[l_pP7Args->bType], l_pP7Args->bSize);
                #ifdef UTF8_ENCODING
                    l_szReturn += (size_t)Convert_UTF32_To_UTF8((tUINT32*)l_pVal,
                                                                o_pBuffer + l_szReturn, 
                                                                (tUINT32)(i_szBuffer - l_szReturn)
                                                               );
                #else
                    l_szReturn += (size_t)Convert_UTF32_To_UTF16((tUINT32*)l_pVal, 
                                                                o_pBuffer + l_szReturn, 
                                                                (tUINT32)(i_szBuffer - l_szReturn)
                                                               );
                #endif                             
                }
            }
            else if (eTypePointer == l_pArg->eType)
            {
                l_uValue.uMax = 0ull;
                COPY_ARG(l_uValue.uMax, l_pP7Args->bSize, l_pP7Args->bSize);
                l_iWidth -=2; //0x
                FORMAT_DIGIT(16ull, g_pHEX, FORMAT_NO_SIGN, FORMAT_X_PREFIX);
            }
            else if (eTypeDouble == l_pArg->eType)
            {
                tINT32 l_iCount;

                memcpy(&l_uValue.d64, i_pValues, l_pP7Args->bSize);

                if (    (FORMATTER_ARG_WIDTH == l_pArg->iWidth)
                     && (FORMATTER_ARG_PRECISION == l_pArg->iPrecision)
                   )
                {
                    l_iCount = PSPrint(o_pBuffer + l_szReturn, 
                                       i_szBuffer - l_szReturn, 
                                       l_pArg->pDouble, 
                                       l_iWidth, 
                                       l_iPrecision,
                                       l_uValue.d64
                                      );
                }
                else if (FORMATTER_ARG_PRECISION == l_pArg->iPrecision)
                {
                    l_iCount = PSPrint(o_pBuffer + l_szReturn, 
                                       i_szBuffer - l_szReturn, 
                                       l_pArg->pDouble, 
                                       l_iPrecision, 
                                       l_uValue.d64
                                      );
                }
                else if (FORMATTER_ARG_WIDTH == l_pArg->iPrecision)
                {
                    l_iCount = PSPrint(o_pBuffer + l_szReturn, 
                                       i_szBuffer - l_szReturn, 
                                       l_pArg->pDouble, 
                                       l_iWidth, 
                                       l_uValue.d64
                                      );
                }
                else
                {
                    l_iCount = PSPrint(o_pBuffer + l_szReturn, 
                                       i_szBuffer - l_szReturn, 
                                       l_pArg->pDouble, 
                                       l_uValue.d64
                                      );
                }

                if (0 < l_iCount)
                {
                    l_szReturn += (size_t)l_iCount;
                }
                else
                {
                    l_bError = TRUE;
                    break;
                }
            }
            else if (eTypeUintOct == l_pArg->eType)
            {
                l_uValue.uMax = 0ull;
                COPY_ARG(l_uValue.uMax, g_pSize[l_pP7Args->bType], l_pP7Args->bSize);
                if (l_pArg->bFlagGrid) {l_iWidth --;}//0
                FORMAT_DIGIT(8ull, g_pHEX, FORMAT_NO_SIGN, FORMAT_0_PREFIX);
            }

            l_pArg     = l_pArg->pNext; 
            i_pValues += l_pP7Args->bSize; //for strings it is 0
            l_pP7Args ++;
        } //while(l_pArg)

        if (l_szReturn < i_szBuffer)
        {
            o_pBuffer[l_szReturn] = 0;
        }

        return (!l_bError) ? (tINT32)l_szReturn : -1;
    }//Format

    ////////////////////////////////////////////////////////////////////////////
    //Print - test function, print format string back to buffer
    // tBOOL Print(tXCHAR *o_pBuffer, size_t i_szBuffer)
    // {
    //     size_t  l_szOffs   = 0;
    //     sArg   *l_pArg     = m_pArgHead;
    //     tBOOL   l_bReturn  = TRUE;
    //     size_t  l_szDouble = 0;
    // 
    //     while (l_pArg)
    //     {
    //         l_szDouble = (l_pArg->pDouble) ? PStrLen(l_pArg->pDouble) : 0; 
    // 
    //         if ((i_szBuffer - l_szOffs) < (l_pArg->szPrefix + l_szDouble + 64))
    //         {
    //             l_bReturn = FALSE;
    //             break;
    //         }
    // 
    //         if (l_pArg->pPrefix)
    //         {
    //             memcpy(o_pBuffer + l_szOffs, l_pArg->pPrefix, l_pArg->szPrefix * sizeof(tXCHAR));
    //             l_szOffs += l_pArg->szPrefix;
    //         }
    // 
    //         if (eTypeDouble == l_pArg->eType)
    //         {
    //             if (l_szDouble)
    //             {
    //                 PStrCpy(o_pBuffer + l_szOffs, i_szBuffer-l_szOffs, l_pArg->pDouble);
    //                 l_szOffs += l_szDouble;
    //             }
    //             else
    //             {
    //                 l_bReturn = FALSE;
    //                 break;
    //             }
    //         }
    //         else if (eTypeNone != l_pArg->eType)
    //         {
    //             o_pBuffer[l_szOffs++] = TM('%');
    // 
    //             if (l_pArg->bFlagSign)        { o_pBuffer[l_szOffs++] = TM('+'); }
    //             if (l_pArg->bFlagLeftAlign)   { o_pBuffer[l_szOffs++] = TM('-'); }
    //             if (l_pArg->bFlagBlankPrefix) { o_pBuffer[l_szOffs++] = TM(' '); }
    //             if (l_pArg->bFlagGrid)        { o_pBuffer[l_szOffs++] = TM('#'); }
    //             if (l_pArg->bFlagZeroPrefix)  { o_pBuffer[l_szOffs++] = TM('0'); }
    // 
    //             if (FORMATTER_NO_WIDTH == l_pArg->iWidth) {}
    //             else if (FORMATTER_ARG_WIDTH == l_pArg->iWidth) { o_pBuffer[l_szOffs++] = TM('*'); }
    //             else { l_szOffs += PSPrint(o_pBuffer + l_szOffs, i_szBuffer-l_szOffs, TM("%d"), l_pArg->iWidth);}
    // 
    //             if (FORMATTER_NO_PRECISION == l_pArg->iPrecision)
    //             {
    //             }
    //             else
    //             {
    //                 o_pBuffer[l_szOffs++] = TM('.'); 
    //                 if (FORMATTER_ARG_PRECISION == l_pArg->iPrecision) { o_pBuffer[l_szOffs++] = TM('*'); }
    //                 else if (FORMATTER_SKIP_PRECISION == l_pArg->iPrecision) { o_pBuffer[l_szOffs++] = TM('0'); }
    //                 else { l_szOffs += PSPrint(o_pBuffer + l_szOffs, i_szBuffer-l_szOffs, TM("%d"), l_pArg->iPrecision);}
    //             }
    // 
    //             if (eSizehh == l_pArg->eSize)
    //             {
    //                 o_pBuffer[l_szOffs++] = TM('h');
    //                 o_pBuffer[l_szOffs++] = TM('h');
    //             }
    //             else if (eSizell == l_pArg->eSize)
    //             {
    //                 o_pBuffer[l_szOffs++] = TM('l');
    //                 o_pBuffer[l_szOffs++] = TM('l');
    //             }
    //             else if (eSizeh == l_pArg->eSize)
    //             {
    //                 o_pBuffer[l_szOffs++] = TM('h');
    //             }
    //             else if (eSizel == l_pArg->eSize)
    //             {
    //                 o_pBuffer[l_szOffs++] = TM('l');
    //             }
    //             else if (eSizeI == l_pArg->eSize)
    //             {
    //                 o_pBuffer[l_szOffs++] = TM('I');
    //             }
    //             else if (eSizez == l_pArg->eSize)
    //             {
    //                 o_pBuffer[l_szOffs++] = TM('z');
    //             }
    //             else if (eSize32 == l_pArg->eSize)
    //             {
    //                 o_pBuffer[l_szOffs++] = TM('I');
    //                 o_pBuffer[l_szOffs++] = TM('3');
    //                 o_pBuffer[l_szOffs++] = TM('2');
    //             }
    //             else if (eSize64 == l_pArg->eSize)
    //             {
    //                 o_pBuffer[l_szOffs++] = TM('I');
    //                 o_pBuffer[l_szOffs++] = TM('6');
    //                 o_pBuffer[l_szOffs++] = TM('4');
    //             }
    //             else if (eSizet == l_pArg->eSize)
    //             {
    //                 o_pBuffer[l_szOffs++] = TM('t');
    //             }
    //             else if (eSizeL == l_pArg->eSize)
    //             {
    //                 o_pBuffer[l_szOffs++] = TM('L');
    //             }
    //             else if (eSizew == l_pArg->eSize)
    //             {
    //                 o_pBuffer[l_szOffs++] = TM('w');
    //             }
    // 
    //             if (eTypeStringH == l_pArg->eType)      {o_pBuffer[l_szOffs++] = TM('s');}
    //             else if (eTypeStringW == l_pArg->eType) {o_pBuffer[l_szOffs++] = TM('S');}
    //             else if (eTypeInt_d == l_pArg->eType)   {o_pBuffer[l_szOffs++] = TM('d');}
    //             else if (eTypeInt_u == l_pArg->eType)   {o_pBuffer[l_szOffs++] = TM('u');}
    //             else if (eTypeInt_i == l_pArg->eType)   {o_pBuffer[l_szOffs++] = TM('i');}
    //             else if (eTypeInt_x == l_pArg->eType)   {o_pBuffer[l_szOffs++] = TM('x');}
    //             else if (eTypeInt_X == l_pArg->eType)   {o_pBuffer[l_szOffs++] = TM('X');}
    //             else if (eTypeInt_o == l_pArg->eType)   {o_pBuffer[l_szOffs++] = TM('o');}
    //             else if (eTypeInt_b == l_pArg->eType)   {o_pBuffer[l_szOffs++] = TM('b');}
    //             else if (eTypeInt_p == l_pArg->eType)   {o_pBuffer[l_szOffs++] = TM('p');}
    //             else if (eTypeInt_n == l_pArg->eType)   {o_pBuffer[l_szOffs++] = TM('n');}
    //             else if (eTypeInt_c == l_pArg->eType)   {o_pBuffer[l_szOffs++] = TM('c');}
    //             else if (eTypeInt_C == l_pArg->eType)   {o_pBuffer[l_szOffs++] = TM('C');}
    //         }
    // 
    //         l_pArg = l_pArg->pNext;
    //     }
    // 
    //     if (l_bReturn)
    //     {
    //         o_pBuffer[l_szOffs] = 0;
    //     }
    // 
    //     return l_bReturn;
    // }

    ////////////////////////////////////////////////////////////////////////////
    //TestScan - test function
    /*
    static tBOOL TestScan()
    {
        tXCHAR l_pBuffer[8192];
        CFormatter   *l_pFormatter = NULL;
        const tXCHAR *l_pPattern   = NULL;
        const tXCHAR *l_pError     = NULL;

    #define COMPARE_PATTERN(i_pFormat, i_pPattern)\
        if (l_pFormatter)\
        {\
            delete l_pFormatter;\
        }\
        l_pFormatter = new CFormatter(TM(i_pFormat), NULL, 0, NULL);\
        l_pPattern   = TM(i_pPattern);\
        if (    (!l_pFormatter->Print(l_pBuffer, 8192))\
                || (0 != PStrCmp(l_pBuffer, l_pPattern))\
            )\
        {\
            l_pError = l_pPattern;\
            goto l_blExit;\
        }\

        COMPARE_PATTERN("%%1 %I32d %%2 %I64d %%3 %hhd %%4 %wd %%5 %Ld %%6 %lld %%7 %ld %%8 %Id %%9 %zd %%10 %td",
                        "%1 %I32d %2 %I64d %3 %hhd %4 %wd %5 %Ld %6 %lld %7 %ld %8 %Id %9 %zd %10 %td"
                        );
        COMPARE_PATTERN("%+- #0*lld", "%+- #0*lld");
        COMPARE_PATTERN("%+- #0123lld", "%+- #0123lld");
        COMPARE_PATTERN("%+.03f", "%+.03f");
        COMPARE_PATTERN("%.0d", "%.0d");
        COMPARE_PATTERN("Test1 %.*d Test2 %.*d", "Test1 %.*d Test2 %.*d");
        COMPARE_PATTERN("%s%S%d%u%i%x%X%o%c%C%b%p%n", "%s%S%d%u%i%x%X%o%c%C%b%p%n");
        COMPARE_PATTERN("Test %hc", "Test %hc");
        COMPARE_PATTERN("Test %wc", "Test %wc");
        COMPARE_PATTERN("Test %lc", "Test %lc");
        COMPARE_PATTERN("Test %0c", "Test %0c");
        COMPARE_PATTERN("Test %%", "Test %");
        COMPARE_PATTERN("%% Test", "% Test");
        COMPARE_PATTERN("%%%c Test", "%%c Test");
        COMPARE_PATTERN("%% Test", "% Test");
        COMPARE_PATTERN("Test %%", "Test %");
        COMPARE_PATTERN("Test %c Test", "Test %c Test");

    l_blExit:
        delete l_pFormatter;

        if (l_pError)
        {
            printf("ERROR");
        }

        return l_pError ? FALSE : TRUE;
    }
    */

private:
    ////////////////////////////////////////////////////////////////////////////
    //AddArg
    sArg *AddArg(const tXCHAR *i_pPrefix, size_t i_szPrefix)
    {
        sArg *l_pReturn = new sArg();
        if (l_pReturn)
        {
            if (i_szPrefix)
            {
                l_pReturn->pPrefix = (tXCHAR*)malloc(i_szPrefix * sizeof(tXCHAR));
                if (l_pReturn->pPrefix)
                {
                    const tXCHAR *l_pSrc  = i_pPrefix;
                    tXCHAR       *l_pDst  = l_pReturn->pPrefix;

                    l_pReturn->szPrefix = 0;

                    while (i_szPrefix--)
                    {
                        *l_pDst = *l_pSrc;
                        l_pDst ++;
                        l_pReturn->szPrefix++;

                        if (    (TM('%') == *l_pSrc)
                             && (TM('%') == *(l_pSrc + 1))
                           )
                        {
                            l_pSrc ++;
                            i_szPrefix--;
                        }

                        l_pSrc ++;
                    }
                }
            }
        }
        return l_pReturn;
    }

    ////////////////////////////////////////////////////////////////////////////
    //GetSize
    CFormatter::eSize GetSize(const tXCHAR *&i_pFormat)
    {
        struct sSize{ const tXCHAR *pPrefix; size_t szPrefix; CFormatter::eSize eSize;};
        const static sSize g_pSize[] = {{TM("I64"), 3, eSize64},
                                        {TM("I32"), 3, eSize32},
                                        {TM("ll"),  2, eSizell},
                                        {TM("hh"),  2, eSizehh},
                                        {TM("h"),   1, eSizeh },
                                        {TM("w"),   1, eSizew },
                                        {TM("l"),   1, eSizel },
                                        {TM("L"),   1, eSizeL },
                                        {TM("I"),   1, eSizeI },
                                        {TM("z"),   1, eSizez },
                                        {TM("j"),   1, eSizej },
                                        {TM("t"),   1, eSizet },
                                        {TM("?"),   0, eSize32}
                                       };

        const sSize *l_pIter = &g_pSize[0];

        while (l_pIter->szPrefix)
        {
            if (0 == PStrNCmp(i_pFormat, l_pIter->pPrefix, l_pIter->szPrefix))
            {
                i_pFormat += l_pIter->szPrefix;
                return l_pIter->eSize;
            }

            l_pIter ++;
        }

        return eSizeUnk;
    }
};

#endif //FORMATTER_H