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
//http://www.codeproject.com/Articles/207464/Exception-Handling-in-Visual-Cplusplus

#ifndef PCRASH_DUMP_H
#define PCRASH_DUMP_H

#include "GTypes.h"
#include "Length.h"
#include "PString.h"
#include "WString.h"
#include "AList.h"
#include "PAtomic.h"
#include "ISignal.h"

#define CH_DESC_FILE_EXT                                  TM("dsc")

struct sCdContext
{
    char   *pBuffer;
    size_t  szBuffer;
    size_t  szPathMax;
    char   *pDescFileName;
};


////////////////////////////////////////////////////////////////////////////////
//CdCreateDumpTxt
static __attribute__ ((unused)) void CdCreateDumpTxt(sCdContext *i_pCdContext, const char *i_pText, tUINT64 i_qwCode)
{   
    UNUSED_ARG(i_pCdContext);
    UNUSED_ARG(i_pText);
    UNUSED_ARG(i_qwCode);
}//CdCreateDumpTxt


////////////////////////////////////////////////////////////////////////////////
//CdWrite
static __attribute__ ((unused)) void CdWrite(void *i_pCdInfo, eCrashCode i_eCode, const void *i_pContext)
{
    UNUSED_ARG(i_pCdInfo);
    UNUSED_ARG(i_eCode);
    UNUSED_ARG(i_pContext);

    //sCdContext *l_pCrush = (sCdContext*)i_pCdInfo;
    //
    //if ( IBreakdownNotify::eException == i_eCode )
    //{
    //    if (i_pContext)
    //    {
    //        _EXCEPTION_POINTERS l_sException = *(_EXCEPTION_POINTERS*)(i_pContext);
    //
    //        char l_pText[128];
    //        sprintf_s(l_pText, 
    //                  _countof(l_pText), 
    //                  "Exception 0x%08X", 
    //                  l_sException.ExceptionRecord->ExceptionCode
    //                 );
    //
    //        tUINT64 l_qwCode = CdCreateDumpBin(&l_sException);
    //        CdCreateDumpTxt(l_pCrush, l_pText, l_qwCode);
    //
    //    }
    //}
    //else if (IBreakdownNotify::ePureCall == i_eCode)
    //{
    //    CdCreateDumpTxt(l_pCrush, (const char*)i_pContext, i_eCode);
    //}
    //else if (IBreakdownNotify::eMemAlloc == i_eCode)
    //{
    //    CdCreateDumpTxt(l_pCrush, (const char*)i_pContext, i_eCode);
    //}
    //else if (IBreakdownNotify::eInvalidParameter == i_eCode)
    //{
    //    CdCreateDumpTxt(l_pCrush, (const char*)i_pContext, i_eCode);
    //}
    //else if ( IBreakdownNotify::eSignal >= i_eCode)
    //{
    //    CdCreateDumpTxt(l_pCrush, (const char*)i_pContext, i_eCode);
    //}
}//CdWrite


////////////////////////////////////////////////////////////////////////////////
//CdCleanUp
static __attribute__ ((unused)) void CdCleanUp(const tXCHAR *i_pDir, unsigned int i_dwMax_Count)
{
    CBList<CWString*> l_cFiles;
    CWString          l_cDir;

    l_cDir.Set(i_pDir);
    CFSYS::Enumerate_Files(&l_cFiles, &l_cDir, TM("*.") CH_DESC_FILE_EXT, 0);

    //sort files - first is oldest, last is newest !
    pAList_Cell l_pStart   = NULL;
    tUINT32     l_dwDirLen = l_cDir.Length();

    while ((l_pStart = l_cFiles.Get_Next(l_pStart)))
    {
        pAList_Cell l_pMin  = l_pStart;
        pAList_Cell l_pIter = l_pStart;

        while ((l_pIter = l_cFiles.Get_Next(l_pIter)))
        {
            CWString *l_pPathM = l_cFiles.Get_Data(l_pMin);
            CWString *l_pPathI = l_cFiles.Get_Data(l_pIter);
            tXCHAR   *l_pNameM = l_pPathM->Get() + l_dwDirLen;
            tXCHAR   *l_pNameI = l_pPathI->Get() + l_dwDirLen;

            if (0 < PStrICmp(l_pNameM, l_pNameI))
            {
                l_pMin = l_pIter;
            }
        }

        if (l_pMin != l_pStart)
        {
            l_cFiles.Extract(l_pMin);
            l_cFiles.Put_After(l_cFiles.Get_Prev(l_pStart), l_pMin);
            l_pStart = l_pMin;
        }
    } //while (l_pStart = m_cFiles.Get_Next(l_pStart))

    //delete oldest files
    while (l_cFiles.Count() > i_dwMax_Count)
    {
        l_pStart = l_cFiles.Get_First();
        if (l_pStart)
        {
            CWString *l_pPath = l_cFiles.Get_Data(l_pStart);
            
            //delete description file
            CFSYS::Delete_File(l_pPath->Get());

            l_cFiles.Del(l_pStart, TRUE);
        }
    }//while (l_cFiles.Count() > i_dwMax_Count)
}//CdCleanUp


////////////////////////////////////////////////////////////////////////////////
//CdAlloc
static __attribute__ ((unused)) void *CdAlloc(const tXCHAR *i_pFolder)
{
    sCdContext  *l_pCrush    = (sCdContext*)malloc(sizeof(sCdContext));
    tBOOL        l_bError    = FALSE;

    if ( !l_pCrush )
    {
        return NULL;
    }


    ////////////////////////////////////////////////////////////////////////////
    //initialize parameters
    memset(l_pCrush, 0, sizeof(sCdContext));

    l_pCrush->szBuffer      = 32768;
    l_pCrush->pBuffer       = (char*)malloc(l_pCrush->szBuffer * sizeof(char));
    l_pCrush->szPathMax     = 16384;
    l_pCrush->pDescFileName = (tXCHAR*)malloc(l_pCrush->szPathMax * sizeof(tXCHAR));

    if (    (NULL == l_pCrush->pBuffer)
         || (NULL == l_pCrush->pDescFileName)
       )
    {
        l_bError = TRUE;
        goto l_lblExit;
    }

    CdCleanUp(i_pFolder, 5);

       
l_lblExit:
    if (    (l_bError)
         && (l_pCrush)
       )
    {
        free(l_pCrush);
        l_pCrush = NULL;
    }


    return l_pCrush;
}//CH_Install



////////////////////////////////////////////////////////////////////////////////
//CdFree
static __attribute__ ((unused)) void CdFree(void *i_pCdInfo)
{
    sCdContext *l_pCrush = (sCdContext*)i_pCdInfo;

    if ( !l_pCrush )
    {
        return;
    }

    if (l_pCrush->pBuffer)
    {
        free(l_pCrush->pBuffer);
        l_pCrush->pBuffer = NULL;
    }

    if (l_pCrush->pDescFileName)
    {
        free(l_pCrush->pDescFileName);
        l_pCrush->pDescFileName = NULL;
    }
}//CdFree

#endif //PCRASH_DUMP_H