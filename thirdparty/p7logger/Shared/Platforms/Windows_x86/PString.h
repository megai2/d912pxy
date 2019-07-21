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
#pragma once

#include <strsafe.h>

#define PRINTF(...)     wprintf(__VA_ARGS__)

///////////////////////////////////////////////////////////////////////////////
//PStrLen
static inline tUINT32 PStrLen(const tXCHAR *i_pText)
{
    return (tUINT32)wcslen(i_pText);
}//PStrLen

///////////////////////////////////////////////////////////////////////////////
//PUStrLen
static inline tUINT32 PUStrLen(const tXCHAR *i_pText)
{
    return (tUINT32)wcslen(i_pText);
}//PUStrLen


////////////////////////////////////////////////////////////////////////////////
//PStrCpy
static inline tXCHAR* PStrCpy(tXCHAR       *i_pDst,
                              size_t        i_szDst,
                              const tXCHAR *i_pSrc
                              )
{
    wcscpy_s((wchar_t*)i_pDst, i_szDst, (wchar_t*)i_pSrc);
    return i_pDst;
}//PStrLen


////////////////////////////////////////////////////////////////////////////////
//PWStrCpy
static inline tXCHAR* PWStrCpy(tWCHAR       *i_pDst,
                               size_t        i_szDst,
                               const tWCHAR *i_pSrc
                              )
{
    wcscpy_s((wchar_t*)i_pDst, i_szDst, (wchar_t*)i_pSrc);
    return i_pDst;
}//PWStrCpy


////////////////////////////////////////////////////////////////////////////////
//PStrCpy
static inline tXCHAR* PStrNCpy(tXCHAR       *i_pDst,
                              size_t        i_szDst,
                              const tXCHAR *i_pSrc,
                              size_t        i_pSrcMaxCount
                              )
{
    wcsncpy_s((wchar_t*)i_pDst, i_szDst, (wchar_t*)i_pSrc, i_pSrcMaxCount);
    return i_pDst;
}//PStrLen


///////////////////////////////////////////////////////////////////////////////
//PStrNCmp
static inline tINT32 PStrNCmp(const tXCHAR *i_pS1, const tXCHAR *i_pS2, size_t i_szLen)
{
    return wcsncmp(i_pS1, i_pS2, i_szLen);
}//PStrNCmp


////////////////////////////////////////////////////////////////////////////////
//PStrNCmp
static inline tINT32 PStrNiCmp(const tXCHAR *i_pS1,
                        const tXCHAR *i_pS2,
                        size_t        i_szLen
                       )
{
    return _wcsnicmp(i_pS1, i_pS2, i_szLen);
}//PStrNCmp


////////////////////////////////////////////////////////////////////////////////
//PStrNCmp
static inline tINT32 PStrICmp(const tXCHAR *i_pS1, const tXCHAR *i_pS2)
{
    return _wcsicmp(i_pS1, i_pS2);
}//PStrNCmp


////////////////////////////////////////////////////////////////////////////////
//PStrCmp
static inline tINT32 PStrCmp(const tXCHAR *i_pS1, const tXCHAR *i_pS2)
{
    return wcscmp(i_pS1, i_pS2);
}//PStrCmp


///////////////////////////////////////////////////////////////////////////////
//PStrToInt
static inline tINT32 PStrToInt(const tXCHAR *i_pS1)
{
    return _wtoi(i_pS1);
}//PStrToInt


///////////////////////////////////////////////////////////////////////////////
//PStrCpy
static inline void PUStrCpy(tWCHAR *i_pDst, tUINT32 i_dwMax_Len, const tXCHAR *i_pSrc)
{
    wcscpy_s((wchar_t *)i_pDst, i_dwMax_Len, i_pSrc);
}//PStrCpy


///////////////////////////////////////////////////////////////////////////////
//PSPrint
static inline tINT32 PSPrint(tXCHAR       *o_pBuffer, 
                             size_t        i_szBuffer, 
                             const tXCHAR *i_pFormat, 
                             ...
                             )
{
    va_list l_pVA;
    int     l_iReturn = 0;

    va_start(l_pVA, i_pFormat);
    l_iReturn = _vsnwprintf_s(o_pBuffer, i_szBuffer, _TRUNCATE, i_pFormat, l_pVA);
    va_end(l_pVA); 

    return (tINT32)l_iReturn;
}//PSPrint


///////////////////////////////////////////////////////////////////////////////
//PSPrint
static inline tINT32 PSPrint(tACHAR       *o_pBuffer, 
                             size_t        i_szBuffer, 
                             const tACHAR *i_pFormat, 
                             ...
                             )
{
    va_list l_pVA;
    int     l_iReturn = 0;

    va_start(l_pVA, i_pFormat);
    l_iReturn = _vsnprintf_s(o_pBuffer, i_szBuffer, _TRUNCATE, i_pFormat, l_pVA);
    va_end(l_pVA); 

    return (tINT32)l_iReturn;
}//PSPrint


///////////////////////////////////////////////////////////////////////////////
//PVsnprintf
static inline int PVsnprintf(tXCHAR *o_pBuffer, size_t i_szBuffer, const tXCHAR *i_pFormat, va_list i_pArgs)
{
    //size_t l_szRem = 0;
    //StringCbVPrintfExW(o_pBuffer,
    //                   i_szBuffer,
    //                   NULL,
    //                   &l_szRem,
    //                   STRSAFE_IGNORE_NULLS,
    //                   i_pFormat,
    //                   i_pArgs
    //                  );
    //return (int)i_szBuffer - (int)l_szRem;
    return _vsnwprintf_s(o_pBuffer, i_szBuffer, _TRUNCATE, i_pFormat, i_pArgs);
}//PVsnprintf


///////////////////////////////////////////////////////////////////////////////
//PStrDub
static inline tXCHAR *PStrDub(const tXCHAR *i_pStr)
{
    return _wcsdup(i_pStr);
}//PStrDub


///////////////////////////////////////////////////////////////////////////////
//PStrFreeDub
static inline void PStrFreeDub(tXCHAR *i_pStr)
{
    free(i_pStr);
}//PStrFreeDub


///////////////////////////////////////////////////////////////////////////////
//PStrChr
static inline const tXCHAR *PStrChr(const tXCHAR * i_pStr, tXCHAR i_cCh)
{
    return wcschr(i_pStr, i_cCh);
}//PStrChr

///////////////////////////////////////////////////////////////////////////////
//PStrChr
static inline tXCHAR *PStrChr(tXCHAR * i_pStr, tXCHAR i_cCh)
{
    return wcschr(i_pStr, i_cCh);
}//PStrChr


///////////////////////////////////////////////////////////////////////////////
//PStrrChr
static inline tXCHAR *PStrrChr(tXCHAR * i_pStr, tXCHAR i_cCh)
{
    return wcsrchr(i_pStr, i_cCh);
}//PStrrChr

///////////////////////////////////////////////////////////////////////////////
//PStrrChr
static inline const tXCHAR *PStrrChr(const tXCHAR * i_pStr, const tXCHAR i_cCh)
{
    return wcsrchr(i_pStr, i_cCh);
}//PStrrChr


///////////////////////////////////////////////////////////////////////////////
//PStrScan
#define PStrScan(i_pBuffer, i_pFormat, ...)   swscanf_s(i_pBuffer,  i_pFormat, __VA_ARGS__)
