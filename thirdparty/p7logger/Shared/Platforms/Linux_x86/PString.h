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
#ifndef PSTRING_H
#define PSTRING_H

#if !defined(PRINTF)
    #if defined(UTF8_ENCODING)
        #define PRINTF(...) printf(__VA_ARGS__)
    #else
        #define PRINTF(...) wprintf(__VA_ARGS__)
    #endif
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "UTF.h"

////////////////////////////////////////////////////////////////////////////////
//PStrLen
static __attribute__ ((unused)) tUINT32 PStrLen(const tXCHAR *i_pText) 
{
    return strlen(i_pText);
}//PStrLen

////////////////////////////////////////////////////////////////////////////////
//PStrCpy
static __attribute__ ((unused)) tXCHAR* PStrCpy(tXCHAR       *i_pDst,
                                                size_t        i_szDst,
                                                const tXCHAR *i_pSrc)
{
    UNUSED_ARG(i_szDst);
    return strcpy(i_pDst, i_pSrc);
}//PStrLen

////////////////////////////////////////////////////////////////////////////////
//PWStrCpy
static inline tWCHAR *PWStrCpy(tWCHAR       *i_pDst,
                               size_t        i_szDst,
                               const tWCHAR *i_pSrc
                              )
{
    while (    (*i_pSrc)
            && (1 < i_szDst)
          )
    {
        *i_pDst = *i_pSrc;
        i_pDst++; i_pSrc++;
        i_szDst--;
    }

    *i_pDst = 0;

    return i_pDst;
}//PStrLen


////////////////////////////////////////////////////////////////////////////////
//PStrCpy
static __attribute__ ((unused)) tXCHAR* PStrNCpy(tXCHAR       *i_pDst,
                                                 size_t        i_szDst,
                                                 const tXCHAR *i_pSrc,
                                                 size_t        i_pSrcMaxCount
                                                )
{
    UNUSED_ARG(i_szDst);
    strncpy((char*)i_pDst, (char*)i_pSrc, i_pSrcMaxCount);
    return i_pDst;
}//PStrLen


////////////////////////////////////////////////////////////////////////////////
//PStrNCmp
static __attribute__ ((unused)) tINT32 PStrNCmp(const tXCHAR *i_pS1,
                                                const tXCHAR *i_pS2,
                                                size_t        i_szLen
                                               )
{
    return strncmp(i_pS1, i_pS2, i_szLen);
}//PStrNCmp


////////////////////////////////////////////////////////////////////////////////
//PStrNCmp
static __attribute__ ((unused)) tINT32 PStrNiCmp(const tXCHAR *i_pS1,
                                                 const tXCHAR *i_pS2,
                                                 size_t        i_szLen
                                                )
{
    return strncasecmp(i_pS1, i_pS2, i_szLen);
}//PStrNCmp


////////////////////////////////////////////////////////////////////////////////
//PStrNCmp
static __attribute__ ((unused)) tINT32 PStrICmp(const tXCHAR *i_pS1,
                                                const tXCHAR *i_pS2
                                               )
{
    return strcasecmp(i_pS1, i_pS2);
}//PStrNCmp


////////////////////////////////////////////////////////////////////////////////
//PStrCmp
static __attribute__ ((unused)) tINT32 PStrCmp(const tXCHAR *i_pS1, 
                                               const tXCHAR *i_pS2
                                              )
{
    return strcmp(i_pS1, i_pS2);
}//PStrCmp

////////////////////////////////////////////////////////////////////////////////
//PStrToInt
static __attribute__ ((unused)) tINT32 PStrToInt(const tXCHAR *i_pS1)
{
    return atoi(i_pS1);
}//PStrToInt


////////////////////////////////////////////////////////////////////////////////
//PUStrLen
static __attribute__ ((unused)) tUINT32 PUStrLen(const tXCHAR *i_pText)
{
    return Get_UTF8_Length(i_pText);
}//PUStrLen



////////////////////////////////////////////////////////////////////////////////
//PUStrCpy
static __attribute__ ((unused)) void PUStrCpy(tWCHAR       *i_pDst,
                                              tUINT32       i_dwMax_Len,
                                              const tXCHAR *i_pSrc)
{
    Convert_UTF8_To_UTF16(i_pSrc, i_pDst, i_dwMax_Len);
    //wcscpy_s(i_pDst, i_dwMax_Len, i_pSrc);
}//PUStrCpy


///////////////////////////////////////////////////////////////////////////////
//PSPrint
static __attribute__ ((unused)) tINT32 PSPrint(tXCHAR       *o_pBuffer,
                                               size_t        i_szBuffer,
                                               const tXCHAR *i_pFormat,
                                               ...
                                              )
{
    va_list l_pVA;
    int     l_iReturn = 0;

    va_start(l_pVA, i_pFormat);

    l_iReturn = vsnprintf((char*)o_pBuffer,
                          i_szBuffer,
                          (char*)i_pFormat,
                          l_pVA);

    va_end(l_pVA);

    return (tINT32)l_iReturn;
}//PSPrint


///////////////////////////////////////////////////////////////////////////////
//PStrDub
static __attribute__ ((unused)) tXCHAR *PStrDub(const tXCHAR *i_pStr)
{
    return strdup(i_pStr);
}//PStrDub


///////////////////////////////////////////////////////////////////////////////
//PStrFreeDub
static __attribute__ ((unused)) void PStrFreeDub(tXCHAR *i_pStr)
{
    free(i_pStr);
}//PStrFreeDub


///////////////////////////////////////////////////////////////////////////////
//PStrChr
static const __attribute__ ((unused)) tXCHAR *PStrChr(const tXCHAR * i_pStr, tXCHAR i_cCh)
{
    return strchr(i_pStr, i_cCh);
}//PStrChr

///////////////////////////////////////////////////////////////////////////////
//PStrChr
static __attribute__ ((unused)) tXCHAR *PStrChr(tXCHAR * i_pStr, tXCHAR i_cCh)
{
    return strchr(i_pStr, i_cCh);
}//PStrChr


///////////////////////////////////////////////////////////////////////////////
//PStrrChr
static const __attribute__ ((unused)) tXCHAR *PStrrChr(const tXCHAR * i_pStr, const tXCHAR i_cCh)
{
    return strrchr(i_pStr, i_cCh);
}//PStrrChr

///////////////////////////////////////////////////////////////////////////////
//PStrrChr
static __attribute__ ((unused)) tXCHAR *PStrrChr(tXCHAR * i_pStr, tXCHAR i_cCh)
{
    return strrchr(i_pStr, i_cCh);
}//PStrrChr


///////////////////////////////////////////////////////////////////////////////
//PVsnprintf
static __attribute__ ((unused)) int PVsnprintf(tXCHAR *o_pBuffer,
                                               size_t  i_szBuffer, 
                                               const tXCHAR *i_pFormat, 
                                               va_list i_pArgs)
{
    return vsnprintf(o_pBuffer, i_szBuffer, i_pFormat, i_pArgs);
}//PVsnprintf



///////////////////////////////////////////////////////////////////////////////
//PStrScan
#define PStrScan(i_pBuffer, i_pFormat, ...) sscanf(i_pBuffer,  i_pFormat, __VA_ARGS__)


#endif //PSTRING_H
