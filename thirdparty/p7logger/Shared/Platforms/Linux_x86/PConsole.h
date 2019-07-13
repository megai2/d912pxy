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
#ifndef PCONSOLE_H
#define PCONSOLE_H

#include <sys/ioctl.h>
#include <stdio.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include "IConsole.h"
#include "RBTree.h"
#include "AList.h"

////////////////////////////////////////////////////////////////////////////////
//Is_Key_Hit
static __attribute__ ((unused)) tBOOL Is_Key_Hit(void)
{
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if(ch != EOF)
    {
    ungetc(ch, stdin);
    return TRUE;
    }

    return FALSE;
}//Is_Key_Hit


////////////////////////////////////////////////////////////////////////////////
//Get_Char
static __attribute__ ((unused)) tXCHAR Get_Char()
{
    return getchar();
}//Get_Char


////////////////////////////////////////////////////////////////////////////////
//SetEchoOff
static __attribute__ ((unused)) void Set_EchoOff(tBOOL i_bOff)
{
    struct termios termInfo;

    if (-1 == tcgetattr(0,&termInfo))
    {
        return;
    }

    if (i_bOff)
    {
        termInfo.c_lflag &= ~ECHO; //turn off ECHO
    }
    else
    {
        termInfo.c_lflag |= ECHO; //turn on ECHO
    }
    tcsetattr(0, 0, &termInfo);
}//Get_Char


////////////////////////////////////////////////////////////////////////////////
struct stKeyMap
{
    tUINT16 wKey;
    tUINT64 qwHash;
    stKeyMap(tUINT16 i_wKey,
             tUINT8  i_pv7,
             tUINT8  i_pv6,
             tUINT8  i_pv5,
             tUINT8  i_pv4,
             tUINT8  i_pv3,
             tUINT8  i_pv2,
             tUINT8  i_pv1,
             tUINT8  i_pv0
            )
        : wKey(i_wKey), qwHash(0)
    {
        qwHash =   ((tUINT64)i_pv7 << 56)
                 | ((tUINT64)i_pv6 << 48)
                 | ((tUINT64)i_pv5 << 40)
                 | ((tUINT64)i_pv4 << 32)
                 | ((tUINT64)i_pv3 << 24)
                 | ((tUINT64)i_pv2 << 16)
                 | ((tUINT64)i_pv1 << 8)
                 | (tUINT64)i_pv0;
    }

    stKeyMap(tUINT16 i_wKey,
             tUINT8  i_pv4,
             tUINT8  i_pv3,
             tUINT8  i_pv2,
             tUINT8  i_pv1,
             tUINT8  i_pv0
            )
        : wKey(i_wKey), qwHash(0)
    {
        qwHash =   ((tUINT64)i_pv4 << 32)
                 | ((tUINT64)i_pv3 << 24)
                 | ((tUINT64)i_pv2 << 16)
                 | ((tUINT64)i_pv1 << 8)
                 | (tUINT64)i_pv0;
    }

    stKeyMap(tUINT16 i_wKey,
             tUINT8  i_pv3,
             tUINT8  i_pv2,
             tUINT8  i_pv1,
             tUINT8  i_pv0
            )
        : wKey(i_wKey), qwHash(0)
    {
        qwHash =   ((tUINT64)i_pv3 << 24)
                 | ((tUINT64)i_pv2 << 16)
                 | ((tUINT64)i_pv1 << 8)
                 | (tUINT64)i_pv0;
    }

    stKeyMap(tUINT16 i_wKey,
             tUINT8  i_pv2,
             tUINT8  i_pv1,
             tUINT8  i_pv0
            )
        : wKey(i_wKey), qwHash(0)
    {
        qwHash =   ((tUINT64)i_pv2 << 16)
                 | ((tUINT64)i_pv1 << 8)
                 | (tUINT64)i_pv0;
    }

    stKeyMap(tUINT16 i_wKey,
             tUINT8  i_pv1,
             tUINT8  i_pv0
            )
        : wKey(i_wKey), qwHash(0)
    {
        qwHash =   ((tUINT64)i_pv1 << 8)
                 | (tUINT64)i_pv0;
    }
};

////////////////////////////////////////////////////////////////////////////////
//CKeysTree
class CKeysList
    : public CListPool<char>
{
protected:
    virtual tBOOL Data_Release(char i_pData)
    {
        UNUSED_ARG(i_pData);
        return TRUE;
    }
};

////////////////////////////////////////////////////////////////////////////////
//CKeysTree
class CKeysTree:
    public CRBTree<stKeyMap*, tUINT64>
{
public:
    ////////////////////////////////////////////////////////////////////////////
    //~CKeysTree
    virtual ~CKeysTree()
    {
        Clear();
    }//~CRBTree

protected:
    //////////////////////////////////////////////////////////////////////////// 
    virtual tBOOL Data_Release(stKeyMap* i_pData)
    {
        delete i_pData;
        return TRUE; //memory used from pool, not necessary to return
    }// Data_Release

    //////////////////////////////////////////////////////////////////////////// 
    //Return
    //TRUE  - if (i_pKey < i_pData::key)
    //FALSE - otherwise
    tBOOL Is_Key_Less(tUINT64 i_pKey, stKeyMap *i_pData)
    {
        return (i_pKey < i_pData->qwHash);
    }

    //////////////////////////////////////////////////////////////////////////// 
    //Return
    //TRUE  - if (i_pKey == i_pData::key)
    //FALSE - otherwise
    tBOOL Is_Qual(tUINT64 i_pKey, stKeyMap *i_pData)
    {
        return (i_pKey == i_pData->qwHash);
    }
};//CDesc_Tree


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CConsole
    : public IConsole
{
    tUINT32   m_pCl[IConsole::eColorsMax];
    size_t    m_szText;
    tXCHAR   *m_pText;
    tXCHAR   *m_pEmpty;
    size_t    m_szEmpty;
    CKeysTree m_pKeys;
    CKeysList m_cKeysList;
public:
    CConsole()
    {
        m_szText = 32768;
        m_pText  = (tXCHAR*)malloc(m_szText * sizeof(tXCHAR));

        m_szEmpty = 0;
        m_pEmpty  = NULL;
        AllocEmptyString(1024);

        Set_EchoOff(TRUE);

        //https://en.wikipedia.org/wiki/ANSI_escape_code
        m_pCl[IConsole::eBlack]        = 0;
        m_pCl[IConsole::eBlue]         = 12;
        m_pCl[IConsole::eGreen]        = 40;
        m_pCl[IConsole::eCyan]         = 51;
        m_pCl[IConsole::eRed]          = 124;
        m_pCl[IConsole::eMagenta]      = 129;
        m_pCl[IConsole::eBrown]        = 94;
        m_pCl[IConsole::eLightGray]    = 252;
        m_pCl[IConsole::eDarkGray]     = 244;
        m_pCl[IConsole::eLightBlue]    = 33;
        m_pCl[IConsole::eLightGreen]   = 48;
        m_pCl[IConsole::eLightCyan]    = 195;
        m_pCl[IConsole::eLightRed]     = 196;
        m_pCl[IConsole::eLightMagenta] = 201;
        m_pCl[IConsole::eYellow]       = 11;
        m_pCl[IConsole::eWhite]        = 15;

        stKeyMap *l_pKey = new stKeyMap(eKeyArrowLeft, 27,91,68);
        m_pKeys.Push(l_pKey, l_pKey->qwHash);

        l_pKey = new stKeyMap(eKeyArrowRight, 27,91,67);
        m_pKeys.Push(l_pKey, l_pKey->qwHash);

        l_pKey = new stKeyMap(eKeyArrowDown, 27,91,66);
        m_pKeys.Push(l_pKey, l_pKey->qwHash);

        l_pKey = new stKeyMap(eKeyArrowUp, 27,91,65);
        m_pKeys.Push(l_pKey, l_pKey->qwHash);

        l_pKey = new stKeyMap(eKeyPageUp, 27,91,53,126);
        m_pKeys.Push(l_pKey, l_pKey->qwHash);

        l_pKey = new stKeyMap(eKeyPageDown, 27,91,54,126);
        m_pKeys.Push(l_pKey, l_pKey->qwHash);

        l_pKey = new stKeyMap(eKeyHome, 27,79,72);
        m_pKeys.Push(l_pKey, l_pKey->qwHash);

        l_pKey = new stKeyMap(eKeyEnd, 27,79,70);
        m_pKeys.Push(l_pKey, l_pKey->qwHash);

        l_pKey = new stKeyMap(eKeyDel, 27,91,51,126);
        m_pKeys.Push(l_pKey, l_pKey->qwHash);

        l_pKey = new stKeyMap(eKeyInsert, 27,91,50,126);
        m_pKeys.Push(l_pKey, l_pKey->qwHash);

        l_pKey = new stKeyMap(eKeyF1, 27,79,80);
        m_pKeys.Push(l_pKey, l_pKey->qwHash);

        l_pKey = new stKeyMap(eKeyF2, 27,79,81);
        m_pKeys.Push(l_pKey, l_pKey->qwHash);

        l_pKey = new stKeyMap(eKeyF3, 27,79,82);
        m_pKeys.Push(l_pKey, l_pKey->qwHash);

        l_pKey = new stKeyMap(eKeyF4, 27,79,83);
        m_pKeys.Push(l_pKey, l_pKey->qwHash);

        l_pKey = new stKeyMap(eKeyF5, 27,91,49,53,126);
        m_pKeys.Push(l_pKey, l_pKey->qwHash);

        l_pKey = new stKeyMap(eKeyF6, 27,91,49,55,126);
        m_pKeys.Push(l_pKey, l_pKey->qwHash);

        l_pKey = new stKeyMap(eKeyF7, 27,91,49,56,126);
        m_pKeys.Push(l_pKey, l_pKey->qwHash);

        l_pKey = new stKeyMap(eKeyF8, 27,91,49,57,126);
        m_pKeys.Push(l_pKey, l_pKey->qwHash);

        l_pKey = new stKeyMap(eKeyF9, 27,91,50,48,126);
        m_pKeys.Push(l_pKey, l_pKey->qwHash);

        l_pKey = new stKeyMap(eKeyF10, 27,91,50,49,126);
        m_pKeys.Push(l_pKey, l_pKey->qwHash);

        l_pKey = new stKeyMap(eKeyF11, 27,91,50,51,126);
        m_pKeys.Push(l_pKey, l_pKey->qwHash);

        l_pKey = new stKeyMap(eKeyF12, 27,91,50,52,126);
        m_pKeys.Push(l_pKey, l_pKey->qwHash);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual ~CConsole() 
    {
        if (m_pText)
        {
            free(m_pText);
            m_pText = NULL;
        }

        if (m_pEmpty)
        {
            free(m_pEmpty);
            m_pEmpty = NULL;
        }

        m_pKeys.Clear();
        m_cKeysList.Clear(TRUE);
        Set_EchoOff(FALSE);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    tBOOL SetCursor(tUINT32 i_uX, tUINT32 i_uY)
    {
        printf("\033[%d;%dH", i_uY+1, i_uX+1);
        return TRUE;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    tUINT32 GetCursorX()
    {
        return 0;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    tBOOL GetInput(IConsole::sInput &o_rInput)
    {
        while (Is_Key_Hit())
        {
            m_cKeysList.Add_After(m_cKeysList.Get_Last(), Get_Char());
        }

        if (!m_cKeysList.Count())
        {
            return FALSE;
        }

        char  l_cVal    = m_cKeysList.Get_Data(m_cKeysList.Get_First());
        tBOOL l_bReturn = FALSE;

        o_rInput.eEvent          = IConsole::eKey;
        o_rInput.stKey.bDown     = TRUE;
        o_rInput.stKey.uControls = 0;
        o_rInput.stKey.wCount    = 1;
        o_rInput.stKey.wKey      = l_cVal;

        if (    (27 == l_cVal) //escape sequence
             && (m_cKeysList.Count())
           )
        {
            tUINT64     l_qwHash  = 0;
            pAList_Cell l_pEl     = m_cKeysList.Get_First();
            stKeyMap   *l_pKey    = NULL;
            tUINT32     l_uiCount = 0;

            while (    (l_pEl)
                    && (8 > l_uiCount)
                  )
            {
                l_uiCount++;

                l_qwHash = (l_qwHash << 8) | m_cKeysList.Get_Data(l_pEl);
                l_pKey = m_pKeys.Find(l_qwHash);
                if (l_pKey)
                {
                    o_rInput.stKey.wKey = l_pKey->wKey;
                    l_bReturn = TRUE;

                    while (l_pEl) //clear esc sequence used to identefy key
                    {
                        pAList_Cell l_pPrevEl = m_cKeysList.Get_Prev(l_pEl);
                        m_cKeysList.Del(l_pEl, TRUE);
                        l_pEl = l_pPrevEl;
                    }

                    break;
                }
                else
                {
                    l_pEl = m_cKeysList.Get_Next(l_pEl);
                }
            }
        }

        if (!l_bReturn) //if it is not esc sequence - get first char and return it
        {
            if (10 == l_cVal)
            {
                o_rInput.stKey.wKey = (tUINT16)IConsole::eKeyEnter;
            }
            else if (9 == l_cVal)
            {
                o_rInput.stKey.wKey = (tUINT16)IConsole::eKeyTab;
            }
            else if (8 == l_cVal)
            {
                o_rInput.stKey.wKey = (tUINT16)IConsole::eKeyBackSpace;
            }

            l_bReturn = TRUE;
            m_cKeysList.Del(m_cKeysList.Get_First(), TRUE);
        }

        return l_bReturn;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    tUINT32 GetCursorY()
    {
        return 0;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    tUINT32 GetWidth()
    {
        struct winsize w;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
        return w.ws_col;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    tUINT32 GetHeight()
    {
        struct winsize w;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
        return w.ws_row;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    tBOOL SetTextAttr(IConsole::eColor i_eFg, IConsole::eColor i_eBg)
    {
        if (    (i_eFg < 0)
             || (i_eFg >= IConsole::eColorsMax)
             || (i_eBg < 0)
             || (i_eBg >= IConsole::eColorsMax)
           )
        {
            return FALSE;
        }

        printf("\x1b[38;5;%dm\x1b[48;5;%dm", m_pCl[i_eFg], m_pCl[i_eBg]);
        return TRUE;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    tBOOL Clear() 
    {
        printf("\033[H\033[J");
        return TRUE;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    tBOOL Clear(tUINT32 i_uX, tUINT32 i_uY, tUINT32 i_uW, tUINT32 i_uH, IConsole::eColor i_eBg)
    {
        tUINT32 l_uiW = GetWidth();
        tUINT32 l_uiH = GetHeight();

        if (    (i_uX > l_uiW)
             || (i_uY > l_uiH)
             || (!i_uW)
             || (!i_uH)
             || (i_uW >= m_szEmpty)
           )
        {
            return FALSE;
        }

        if ((i_uX + i_uW) > l_uiW)
        {
            i_uW = l_uiW - i_uX;
        }

        if ((i_uY + i_uH) > l_uiH)
        {
            i_uH = l_uiH - i_uY;
        }

        if (i_uW > m_szEmpty)
        {
            if (!AllocEmptyString((size_t)i_uW))
            {
                return FALSE;
            }
        }

        if (!SetTextAttr(IConsole::eBlack, i_eBg))
        {
            return FALSE;
        }

        SetCursor(i_uX, i_uY);

        tUINT32 l_uiXOld = i_uX;
        tUINT32 l_uiYOld = i_uY;
        while (i_uH--)
        {
            SetCursor(i_uX, i_uY++);
            m_pEmpty[i_uW] = 0;
            WriteTextB(m_pEmpty, i_uW);
            m_pEmpty[i_uW] = ' ';
        }

        SetCursor(l_uiXOld, l_uiYOld);

        return TRUE;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    size_t WriteTextB(const tXCHAR *i_pText, size_t i_szText)
    {
        UNUSED_ARG(i_szText);
        return (size_t)printf("%s", i_pText);
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    size_t WriteTextB(IConsole::eColor i_eFg, 
                      IConsole::eColor i_eBg, 
                      size_t           i_szLenMin, 
                      size_t           i_szLenMax, 
                      size_t           i_szOffset,
                      tXCHAR          *i_pText, 
                      size_t           i_szText,    
                      size_t           i_szTextMax
                     )
    {
        if (    (i_szLenMin > i_szLenMax)
             || (!i_pText)
             || (8 >= i_szTextMax)
             || (i_szText >= i_szTextMax)
           )
        {
            return 0;
        }

        if (!SetTextAttr(i_eFg, i_eBg))
        {
            return 0;
        }

        tXCHAR *l_pText = i_pText; 

        if (!i_szText)
        {
            i_szText = PStrLen(i_pText);
        }

        //if there is offset, recalculate all inputs
        if (i_szOffset)
        {
            if (i_szOffset < i_szText) //set sext offset and recalculate all
            {
                l_pText      = l_pText + i_szOffset;
                i_szText    -= i_szOffset;
                i_szTextMax -= i_szOffset;
                i_szOffset   = 0;
                *l_pText = TM('<');
            }
            else //if (i_szOffset >= i_szText)
            {
                l_pText    = m_pText;
                *l_pText   = TM('<');
                i_szText   = 1;
                i_szTextMax= m_szText;
            }
        }


        if (i_szText > i_szLenMax)
        {
            i_szText = i_szLenMax;
            l_pText[i_szText-1] = TM('>');
        }
        else if (i_szText < i_szLenMin)
        {
            if (i_szTextMax < (i_szLenMin + 1))
            {
                PStrNCpy(m_pText, m_szText, l_pText, i_szText);
                l_pText     = m_pText;
                i_szTextMax = m_szText;
            }

            tXCHAR *l_pIter = l_pText + i_szText;

            while (i_szText < i_szLenMin)
            {
                *l_pIter = TM(' ');
                l_pIter ++;
                i_szText++;
            }
        }

        if (i_szText < i_szTextMax)
        {
            l_pText[i_szText] = 0;
        }
        else
        {
            l_pText[i_szText-1] = 0;
        }

        return (size_t)printf("%s", l_pText);
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    size_t WriteTextF(IConsole::eColor i_eFg, 
                      IConsole::eColor i_eBg, 
                      size_t           i_szLenMin, 
                      size_t           i_szLenMax, 
                      size_t           i_szOffset,
                      const tXCHAR    *i_pFormat, 
                      va_list          i_pVA
                     )
    {
        int l_iReturn = PVsnprintf(m_pText, m_szText, i_pFormat, i_pVA);

        if (0 > l_iReturn)
        {
            return 0;
        }

        return WriteTextB(i_eFg, i_eBg, i_szLenMin, i_szLenMax, i_szOffset, m_pText, (size_t)l_iReturn, m_szText);
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    tBOOL Present()
    {
        return TRUE;
    }

protected:
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    tBOOL AllocEmptyString(size_t i_szEmpty)
    {
        if (m_pEmpty)
        {
            free(m_pEmpty);
        }

        if (i_szEmpty > 4096)
        {
            i_szEmpty = 4096;
        }

        m_szEmpty = i_szEmpty;
        m_pEmpty  = (tXCHAR*)malloc(m_szEmpty * sizeof(tXCHAR));

        if (!m_pEmpty)
        {
            return FALSE;
        }

        tXCHAR *l_pEmpty = m_pEmpty;
        size_t  l_szIterator = m_szEmpty;

        while (l_szIterator--)
        {
            *l_pEmpty = TM(' ');
            l_pEmpty++;
        }

        return TRUE;
    }
};



#endif //PCONSOLE_H
