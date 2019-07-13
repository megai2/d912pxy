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
#ifndef ICONSOLE_H
#define ICONSOLE_H

#define CON_RIGHT_ALT_PRESSED     0x0001 // the right alt key is pressed.
#define CON_LEFT_ALT_PRESSED      0x0002 // the left alt key is pressed.
#define CON_RIGHT_CTRL_PRESSED    0x0004 // the right ctrl key is pressed.
#define CON_LEFT_CTRL_PRESSED     0x0008 // the left ctrl key is pressed.
#define CON_SHIFT_PRESSED         0x0010 // the shift key is pressed.
#define CON_NUMLOCK_ON            0x0020 // the numlock light is on.
#define CON_SCROLLLOCK_ON         0x0040 // the scrolllock light is on.
#define CON_CAPSLOCK_ON           0x0080 // the capslock light is on.
#define CON_ENHANCED_KEY          0x0100 // the key is enhanced.

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class IConsole
{
public:
    enum eColor
    {
        eBlack = 0, 
        eBlue, 
        eGreen, 
        eCyan,
        eRed,
        eMagenta,
        eBrown,
        eLightGray,
        eDarkGray,
        eLightBlue,
        eLightGreen,
        eLightCyan,
        eLightRed,
        eLightMagenta,
        eYellow,
        eWhite,
        eColorsMax
    };

    enum eEvent
    {
        eSize = 0,
        eMouse,
        eKey,
        eFocus,
        eEventsMax
    };

    enum eKey
    {
        eKeyEsc       = 27,
        eKeyEnter     = 13,
        eKeySpace     = 32,

        eKeyExtra     = 0x100,
        eKeyArrowLeft,
        eKeyArrowRight,
        eKeyArrowDown,
        eKeyArrowUp,
        eKeyPageUp,
        eKeyPageDown,
        eKeyHome,
        eKeyEnd,
        eKeyF1,
        eKeyF2,
        eKeyF3,
        eKeyF4,
        eKeyF5,
        eKeyF6,
        eKeyF7,
        eKeyF8,
        eKeyF9,
        eKeyF10,
        eKeyF11,
        eKeyF12,
        eKeyDel,
        eKeyInsert,
        eKeyTab,
        eKeyBackSpace,

        eKeyMax
    };

    struct sSize
    {
        tUINT32 uW;
        tUINT32 uH;
    };
    
    struct sMouse
    {
        tUINT32 uX;
        tUINT32 uY;
        tUINT32 uButton;
    };

    struct sKey
    {
        tBOOL   bDown;
        tUINT16 wKey;
        tUINT16 wCount;
        tUINT32 uControls;
    };

    struct sFocus
    {
        tBOOL bFocus;
    };

    struct sInput 
    {
        IConsole::eEvent eEvent;
        union 
        {
            sSize  stSize;
            sMouse stMouse;
            sKey   stKey;
            sFocus stFocus;
        };
    };



    virtual tBOOL   SetCursor(tUINT32 i_uX, tUINT32 i_uY) = 0;
    virtual tUINT32 GetCursorX() = 0;
    virtual tUINT32 GetCursorY() = 0;
    virtual tUINT32 GetWidth() = 0;
    virtual tUINT32 GetHeight() = 0;
    virtual tBOOL   SetTextAttr(IConsole::eColor i_eFg, IConsole::eColor i_eBg) = 0;
    virtual tBOOL   GetInput(IConsole::sInput &o_rInput) = 0;
    virtual tBOOL   Clear() = 0;
    virtual tBOOL   Clear(tUINT32 i_uX, tUINT32 i_uY, tUINT32 i_uW, tUINT32 i_uH, IConsole::eColor i_eBg) = 0;
    virtual size_t  WriteTextB(const tXCHAR *i_pText, size_t i_szText) = 0;
    virtual size_t  WriteTextB(IConsole::eColor i_eFg, 
                               IConsole::eColor i_eBg, 
                               size_t           i_szLenMin, 
                               size_t           i_szLenMax, 
                               size_t           i_szOffset,
                               tXCHAR          *i_pText, 
                               size_t           i_szText,
                               size_t           i_szTextMax
                              ) = 0;

    virtual size_t  WriteTextO(IConsole::eColor i_eFg, 
                               IConsole::eColor i_eBg, 
                               size_t           i_szLenMin, 
                               size_t           i_szLenMax, 
                               size_t           i_szOffset,
                               const tXCHAR    *i_pFormat, 
                               ...
                              )
    {
        va_list l_pVA;
        size_t  l_szReturn = 0;

        va_start(l_pVA, i_pFormat);
        l_szReturn = WriteTextF(i_eFg, i_eBg, i_szLenMin, i_szLenMax, i_szOffset, i_pFormat, l_pVA);
        va_end(l_pVA);
        return l_szReturn;
    }

    virtual size_t  WriteText(IConsole::eColor i_eFg, 
                              IConsole::eColor i_eBg, 
                              size_t           i_szLenMin, 
                              size_t           i_szLenMax, 
                              const tXCHAR    *i_pFormat, 
                              ...
                             )
    {
        va_list l_pVA;
        size_t  l_szReturn = 0;

        va_start(l_pVA, i_pFormat);
        l_szReturn = WriteTextF(i_eFg, i_eBg, i_szLenMin, i_szLenMax, 0, i_pFormat, l_pVA);
        va_end(l_pVA);
        return l_szReturn;
    }


    virtual size_t  WriteTextF(IConsole::eColor i_eFg, 
                               IConsole::eColor i_eBg, 
                               size_t           i_szLenMin, 
                               size_t           i_szLenMax, 
                               size_t           i_szOffset,
                               const tXCHAR    *i_pFormat, 
                               va_list          i_pVA
                              ) = 0;

    virtual tBOOL   Present() = 0;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CConsoleWnd
{
    struct stHelp
    {
        tXCHAR          *pString;
        IConsole::eColor eBgColor;
        IConsole::eColor eFgColor;

        stHelp(const tXCHAR *i_pString, IConsole::eColor i_eBgColor, IConsole::eColor i_eFgColor)
        {
            pString  = PStrDub(i_pString);
            eBgColor = i_eBgColor;
            eFgColor = i_eFgColor;
        }

        ~stHelp()
        {
            if (pString)
            {
                free(pString);
                pString = NULL;
            }
        }
    };

public:
    struct stColumn
    {
        tUINT32 uId;
        tBOOL   bVisible;
        tXCHAR *pCaption;
        size_t  szCaption;

        stColumn(tUINT32 i_uId, tBOOL i_bVisible, const tXCHAR *i_pCaption)
            : pCaption(NULL)
        {
            uId      = i_uId;
            bVisible = i_bVisible;
            SetCaption(i_pCaption);
        }

        ~stColumn()
        {
            if (pCaption)
            {
                PStrFreeDub(pCaption);
                pCaption = NULL;
            }
            uId       = 0;
            bVisible  = FALSE;
            szCaption = 0;
        }

        void SetCaption(const tXCHAR *i_pCaption)
        {
            if (pCaption)
            {
                PStrFreeDub(pCaption);
            }

            pCaption = PStrDub(i_pCaption);
            szCaption = PStrLen(pCaption);
        }
    };

protected: 
    tUINT32          m_uX;
    tUINT32          m_uY;
    tUINT32          m_uW;
    tUINT32          m_uH;
    size_t           m_szLinesPerItem;
    size_t           m_szItemsPerWnd;
    tUINT64          m_qwItemsCount;
    IConsole        *m_pConsole;
    IConsole::eColor m_eWindowBgColor;
    
    IConsole::eColor m_eItemBgColor;
    IConsole::eColor m_eItemFgColor;
    
    IConsole::eColor m_eItemSlFgColor;
    IConsole::eColor m_eItemSlBgColor;

    IConsole::eColor m_eItemClFgColor;
    IConsole::eColor m_eItemClBgColor;

    tBOOL            m_bError;
    tXCHAR          *m_pCaption;
    size_t           m_szOffsetX;
    size_t           m_szOffsetY;
    tUINT64          m_qwOffsetWnd;
    tBOOL            m_bHelp;
    CBList<stHelp*>  m_cHelp;
    CBList<stColumn*>m_cColumns;
    tBOOL            m_bColumns;
    tBOOL            m_bUpdateColumns;
    tXCHAR          *m_pColumnsText;
    size_t           m_szColumnsTextMax;
    size_t           m_szColumnsText;

public:
    struct sCfg
    {
        tUINT32          uX;
        tUINT32          uY;
        tUINT32          uW;
        tUINT32          uH;
        size_t           uLinesPerItem;
        size_t           szItemsCount;
        IConsole        *pConsole;
        IConsole::eColor eWindowBgColor;
        IConsole::eColor eItemBgColor;
        IConsole::eColor eItemFgColor;
        IConsole::eColor eItemSlFgColor;
        IConsole::eColor eItemSlBgColor;
        IConsole::eColor eItemClFgColor;
        IConsole::eColor eItemClBgColor;
        const tXCHAR    *pCaption;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CConsoleWnd()    
        : m_uX(0)
        , m_uY(0)
        , m_uW(0)
        , m_uH(0)
        , m_szLinesPerItem(0)
        , m_szItemsPerWnd(0)
        , m_qwItemsCount(0ull)
        , m_pConsole(NULL)
        , m_eWindowBgColor(IConsole::eBlack)
        , m_eItemBgColor(IConsole::eBlack)
        , m_eItemFgColor(IConsole::eGreen)
        , m_eItemSlFgColor(IConsole::eBlue)
        , m_eItemSlBgColor(IConsole::eBlack)
        , m_eItemClFgColor(IConsole::eWhite)
        , m_eItemClBgColor(IConsole::eDarkGray)
        , m_bError(FALSE)
        , m_pCaption(NULL)
        , m_szOffsetX(0)
        , m_szOffsetY(0)
        , m_qwOffsetWnd(0)
        , m_bHelp(FALSE)
        , m_bColumns(FALSE)
        , m_bUpdateColumns(FALSE)
        , m_pColumnsText(NULL)
        , m_szColumnsTextMax(0)
        , m_szColumnsText(0)
    {
        CConsoleWnd::PushHelpString(TM(" Navigation keys mapping:"), IConsole::eBlue, IConsole::eWhite);
        CConsoleWnd::PushHelpString(TM(" * Arrow Up, Down, Page Up, Page down - move cursor"), IConsole::eBlack, IConsole::eGreen);
        CConsoleWnd::PushHelpString(TM(" * Arrow Left, Right - horizontal scroll"), IConsole::eBlack, IConsole::eGreen);
        CConsoleWnd::PushHelpString(TM(" * Home - goto top"), IConsole::eBlack, IConsole::eGreen);
        CConsoleWnd::PushHelpString(TM(" * End - goto bottom"), IConsole::eBlack, IConsole::eGreen);
        CConsoleWnd::PushHelpString(TM(" * Esc - exit to upper level or quit"), IConsole::eBlack, IConsole::eGreen);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual tBOOL Init(const CConsoleWnd::sCfg &i_rConfig)    
    {
        m_pConsole       = i_rConfig.pConsole;
        m_eWindowBgColor = i_rConfig.eWindowBgColor;
        m_eItemBgColor   = i_rConfig.eItemBgColor;
        m_eItemFgColor   = i_rConfig.eItemFgColor;
        m_eItemSlFgColor = i_rConfig.eItemSlFgColor;
        m_eItemSlBgColor = i_rConfig.eItemSlBgColor;
        m_eItemClFgColor = i_rConfig.eItemClFgColor;
        m_eItemClBgColor = i_rConfig.eItemClBgColor;
        m_szLinesPerItem = i_rConfig.uLinesPerItem;
        m_qwItemsCount   = i_rConfig.szItemsCount;

        SetCaption(i_rConfig.pCaption);

        m_pConsole->SetCursor(i_rConfig.uX, i_rConfig.uY);
        m_pConsole->Clear(i_rConfig.uX, i_rConfig.uY, i_rConfig.uW, i_rConfig.uH, m_eWindowBgColor);

        return Resize(i_rConfig.uX, i_rConfig.uY, i_rConfig.uW, i_rConfig.uH);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void SetContext(void *i_pContext)
    {
        UNUSED_ARG(i_pContext);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual ~CConsoleWnd()
    {
        m_cHelp.Clear(TRUE);
        m_cColumns.Clear(TRUE);

        if (m_pCaption)
        {
            PStrFreeDub(m_pCaption);
            m_pCaption = NULL;
        }

        if (m_pColumnsText)
        {
            free(m_pColumnsText);
            m_pColumnsText = NULL;
        }
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void SetCaption(const tXCHAR *i_pCaption)
    {
        if (m_pCaption)
        {
            PStrFreeDub(m_pCaption);
            m_pCaption = NULL;
        }

        m_pCaption = i_pCaption ? PStrDub(i_pCaption) : PStrDub(TM("?"));
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void PushHelpString(const tXCHAR *i_pString, IConsole::eColor i_eBgColor, IConsole::eColor i_eFgColor)
    {
        if (i_pString)
        {
            m_cHelp.Add_After(m_cHelp.Get_Last(), new stHelp(i_pString, i_eBgColor, i_eFgColor));
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void PushColumn(tUINT32 i_uId, tBOOL i_bVisible, const tXCHAR *i_pCaption)
    {
        if (i_pCaption)
        {
            m_cColumns.Add_After(m_cColumns.Get_Last(), new stColumn(i_uId, i_bVisible, i_pCaption));
            m_bUpdateColumns = TRUE;
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void EnableColumns(tBOOL i_bEnable)
    {
        m_bUpdateColumns = TRUE;
        m_bColumns       = i_bEnable;
        UpdateItemsPerWindow();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual CConsoleWnd::stColumn *GetColumn(tUINT32 i_uIndex)
    {
        return m_cColumns[i_uIndex];
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void EnableColumn(tUINT32 i_uIndex, tBOOL i_bEnable)
    {
        stColumn *l_pCol = m_cColumns[i_uIndex];

        if (!l_pCol)
        {
            return;
        }

        l_pCol->bVisible = i_bEnable;
        m_bUpdateColumns = TRUE;

        if (m_bColumns)
        {
            Redraw();
        }
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void SetColumnCaption(tUINT32 i_uIndex, const tXCHAR *i_pCation)
    {
        stColumn *l_pCol = m_cColumns[i_uIndex];

        if (!l_pCol)
        {
            return;
        }

        l_pCol->SetCaption(i_pCation);
        m_bUpdateColumns = TRUE;
        
        if (m_bColumns)
        {
            Redraw();
        }
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void SetItemsCount(tUINT64 i_qwItems)
    {
        m_qwItemsCount = i_qwItems;

        UpdateItemsPerWindow();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void Redraw()
    {
        if (m_bHelp)
        {
            return;
        }

        tUINT32 l_uiOffset = 1;

        if (m_bColumns)
        {
            l_uiOffset = 2;
        }

        UpdateHeader();
        m_pConsole->SetCursor(m_uX, m_uY + l_uiOffset);
        size_t l_szCount = Print() + l_uiOffset;

        if (l_szCount < (size_t)m_uH)
        {
            m_pConsole->Clear(m_uX, m_uY + (tUINT32)l_szCount, m_uW, m_uH - (tUINT32)l_szCount, m_eWindowBgColor);
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual tBOOL Resize(tUINT32 i_uX, tUINT32 i_uY, tUINT32 i_uW, tUINT32 i_uH)
    {
        m_uX = i_uX;
        m_uY = i_uY;
        m_uW = i_uW;
        m_uH = i_uH;

        UpdateItemsPerWindow();
        return TRUE;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual tBOOL ProcessKey(tUINT16 i_wChar)
    {
        if (m_bError)
        {
            return FALSE;
        }

        tBOOL   l_bReturn  = FALSE;
        size_t  l_szCount  = (size_t)m_uH;
        tUINT32 l_uiOffset = 1;

        if (m_bColumns)
        {
            l_uiOffset = 2;
        }

        if (m_bHelp)
        {
            if ((tUINT16)IConsole::eKeyEsc == i_wChar)
            {
                m_bHelp = FALSE;
                l_bReturn = TRUE;
            }

        }
        else if (    ((tUINT16)IConsole::eKeyF1 == i_wChar)
                  || ((tUINT16)IConsole::eKeyF2 == i_wChar)
                )
        {
            m_bHelp = TRUE;

            l_szCount = m_cHelp.Count() - l_uiOffset;
            m_pConsole->SetCursor(m_uX, m_uY);
            
            pAList_Cell l_pEl = NULL;
            while (NULL != (l_pEl = m_cHelp.Get_Next(l_pEl)))
            {
                stHelp *l_pHelp = m_cHelp.Get_Data(l_pEl);
                if (l_pHelp)
                {
                    m_pConsole->WriteText(l_pHelp->eFgColor, l_pHelp->eBgColor, m_uW, m_uW, l_pHelp->pString);
                }
                else
                {
                    l_szCount --;
                }
            }
        }
        else if ((tUINT16)IConsole::eKeyArrowLeft == i_wChar)
        {
            if (m_szOffsetX)
            {
                m_szOffsetX--;
                l_bReturn = TRUE;
                m_bUpdateColumns = TRUE;
            }
        }
        else if ((tUINT16)IConsole::eKeyArrowRight == i_wChar)
        {
            m_szOffsetX++;
            m_bUpdateColumns = TRUE;
            l_bReturn = TRUE;
        }
        else if ((tUINT16)IConsole::eKeyArrowUp == i_wChar)
        {
            if (m_szOffsetY)
            {
                m_szOffsetY--;    
                l_bReturn = TRUE;
            }
            else if (m_qwOffsetWnd)
            {
                m_qwOffsetWnd--;
                l_bReturn = TRUE;
            }
        }
        else if ((tUINT16)IConsole::eKeyArrowDown == i_wChar)
        {
            if ((m_szOffsetY + 1) < m_szItemsPerWnd)
            {
                m_szOffsetY++;    
                l_bReturn = TRUE;
            }
            else if ((m_qwOffsetWnd + (tUINT64)m_szItemsPerWnd + 1ull) <= m_qwItemsCount)
            {
                m_qwOffsetWnd++;
                l_bReturn = TRUE;
            }
        }
        else if ((tUINT16)IConsole::eKeyPageUp == i_wChar)
        {
            if (m_qwOffsetWnd > (tUINT64)m_szItemsPerWnd)
            {
                m_qwOffsetWnd -= (tUINT64)m_szItemsPerWnd;
                l_bReturn = TRUE;
            }
            else if (m_qwOffsetWnd)
            {
                m_qwOffsetWnd = 0;
                m_szOffsetY   = 0;    
                l_bReturn     = TRUE;
            }
            else if (m_szOffsetY)
            {
                m_szOffsetY = 0;    
                l_bReturn   = TRUE;
            }
        }
        else if ((tUINT16)IConsole::eKeyPageDown == i_wChar)
        {
            if ((m_qwOffsetWnd + 2ull*(tUINT64)m_szItemsPerWnd) <= m_qwItemsCount)
            {
                m_qwOffsetWnd += (tUINT64)m_szItemsPerWnd;
                l_bReturn = TRUE;
            }
            else if ((tUINT64)m_szItemsPerWnd <= m_qwItemsCount) 
            {
                m_qwOffsetWnd = m_qwItemsCount - (tUINT64)m_szItemsPerWnd;
                m_szOffsetY   = m_szItemsPerWnd - 1;    
                l_bReturn = TRUE;
            }
            else if (m_qwItemsCount)
            {
                m_szOffsetY = (size_t)m_qwItemsCount - 1;    
                l_bReturn = TRUE;
            }
        }
        else if ((tUINT16)IConsole::eKeyHome == i_wChar)
        {
            m_qwOffsetWnd = 0;
            m_szOffsetY   = 0;    
            l_bReturn = TRUE;
        }
        else if ((tUINT16)IConsole::eKeyEnd == i_wChar)
        {
            if ((tUINT64)m_szItemsPerWnd <= m_qwItemsCount) 
            {
                m_qwOffsetWnd = m_qwItemsCount - (tUINT64)m_szItemsPerWnd;
                m_szOffsetY   = m_szItemsPerWnd - 1;    
                l_bReturn = TRUE;
            }
            else if (m_qwItemsCount)
            {
                m_szOffsetY = (size_t)m_qwItemsCount - 1;    
                l_bReturn = TRUE;
            }
        }

        if (l_bReturn)
        {
            UpdateHeader();
            m_pConsole->SetCursor(m_uX, m_uY + l_uiOffset);
            l_szCount = Print();
        }

        l_szCount += l_uiOffset; 
        
        if (l_szCount < (size_t)m_uH)
        {
            m_pConsole->Clear(m_uX, m_uY + (tUINT32)l_szCount, m_uW, m_uH - (tUINT32)l_szCount, m_eWindowBgColor);
        }

        return l_bReturn;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual size_t Print() = 0;

protected: 
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void UpdateItemsPerWindow()
    {
        tUINT32 l_uiH = m_uH - 1;

        if (!m_szLinesPerItem)
        {
            return;
        }

        if (    (m_bColumns)
             && (l_uiH)
           )
        {
            l_uiH --;
        }

        m_szItemsPerWnd = l_uiH / m_szLinesPerItem;
        if (m_szItemsPerWnd > m_qwItemsCount)
        {
            m_szItemsPerWnd = (size_t)m_qwItemsCount;
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void UpdateHeader()
    {
        tUINT32 l_uPos = 100;
        tUINT64 l_qwOffsY = (tUINT64)m_szOffsetY;

        if (    (l_qwOffsY)
             || (1ull == m_qwItemsCount)
           )
        {
            l_qwOffsY++;
        }

        if (m_qwItemsCount)
        {
            l_uPos = (tUINT32)((100ull * (m_qwOffsetWnd + l_qwOffsY)) / m_qwItemsCount);
        }

        m_pConsole->SetCursor(m_uX, m_uY);
        m_pConsole->WriteText(IConsole::eBlack, 
                              IConsole::eDarkGray, 
                              m_uW, 
                              m_uW, 
                              TM("{%s} % 3u%% [Help = F1/F2]"),
                              m_pCaption,
                              l_uPos
                             );

        if (m_bColumns)
        {
            if (m_bUpdateColumns)
            {
                pAList_Cell l_pEl   = NULL;
                size_t      l_szLen = 0;

                while ((l_pEl = m_cColumns.Get_Next(l_pEl))) { l_szLen += m_cColumns.Get_Data(l_pEl)->szCaption;}

                if (l_szLen >= m_szColumnsTextMax)
                {
                    if (m_pColumnsText) free(m_pColumnsText);
                    m_szColumnsTextMax = l_szLen + 16;
                    m_pColumnsText = (tXCHAR*)malloc(m_szColumnsTextMax * sizeof(tXCHAR));
                }


                m_szColumnsText = 0;
                l_pEl = NULL;
                while ((l_pEl = m_cColumns.Get_Next(l_pEl)))
                {
                    stColumn *l_pCol = m_cColumns.Get_Data(l_pEl);

                    if (    (l_pCol)
                         && (l_pCol->bVisible)
                       )
                    {
                        PStrCpy(m_pColumnsText + m_szColumnsText,  m_szColumnsTextMax - m_szColumnsText, l_pCol->pCaption);
                        m_szColumnsText += l_pCol->szCaption;
                    }
                }

                m_bUpdateColumns = FALSE;

                if (!m_szColumnsText)
                {
                    m_pColumnsText[0] = TM(' ');
                    m_pColumnsText[1] = 0;
                    m_szColumnsText   = 1;
                }
            }

            m_pConsole->WriteTextB(m_eItemClFgColor, 
                                   m_eItemClBgColor, 
                                   m_uW,
                                   m_uW,
                                   m_szOffsetX,
                                   m_pColumnsText,
                                   m_szColumnsText,
                                   m_szColumnsTextMax
                                 );

        }
    }
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CConsoleInput
{
protected:
    tXCHAR          *m_pText;
    size_t           m_szText;
    size_t           m_szCursor;
    tXCHAR          *m_pCaption;
    tUINT32          m_uX;
    tUINT32          m_uY;
    tUINT32          m_uW;
    tUINT32          m_uH;

    IConsole        *m_pConsole;
    IConsole::eColor m_eWndFgColor;
    IConsole::eColor m_eWndBgColor;

    IConsole::eColor m_eInpFgColor;
    IConsole::eColor m_eInpBgColor;

    IConsole::eColor m_eCurFgColor;
    IConsole::eColor m_eCurBgColor;


public:
    struct sCfg
    {
        tUINT32          uX;
        tUINT32          uY;
        tUINT32          uW;
        tUINT32          uH;

        IConsole        *pConsole;
        IConsole::eColor eWndFgColor;
        IConsole::eColor eWndBgColor;

        IConsole::eColor eInpFgColor;
        IConsole::eColor eInpBgColor;

        IConsole::eColor eCurFgColor;
        IConsole::eColor eCurBgColor;

        const tXCHAR    *pCaption;

        sCfg(tUINT32       i_uX,
             tUINT32       i_uY,
             tUINT32       i_uW,
             tUINT32       i_uH,
             IConsole     *i_pConsole,
             const tXCHAR *i_pCaption
            )
            : uX(i_uX)
            , uY(i_uY)
            , uW(i_uW)
            , uH(i_uH)
            , pConsole(i_pConsole)
            , eWndFgColor(IConsole::eWhite)
            , eWndBgColor(IConsole::eDarkGray)
            , eInpFgColor(IConsole::eGreen)
            , eInpBgColor(IConsole::eLightGray)
            , eCurFgColor(IConsole::eWhite)
            , eCurBgColor(IConsole::eBlue)
            , pCaption(i_pCaption)
        {
        }
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CConsoleInput(CConsoleInput::sCfg &i_rConfig)
    {
        UNUSED_ARG(i_rConfig);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual ~CConsoleInput()
    {

    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void Redraw()
    {
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void SetCaption(const tXCHAR *i_pCaption)
    {
        if (m_pCaption)
        {
            PStrFreeDub(m_pCaption);
            m_pCaption = NULL;
        }

        m_pCaption = i_pCaption ? PStrDub(i_pCaption) : PStrDub(TM("?"));
    }

    virtual tBOOL ProcessKey(tUINT16 i_wChar)
    {
        UNUSED_ARG(i_wChar);
        return FALSE;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual tBOOL Apply(const tXCHAR *i_pText) = 0;
};

#endif //ICONSOLE_H
