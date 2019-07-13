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
#ifndef UTF_CONV_H
#define UTF_CONV_H

////////////////////////////////////////////////////////////////////////////////
//                     Basic UTF LE conversion functions
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
//Get_utf8_Length
static UNUSED_FUNC size_t Get_UTF8_Length(const char *i_pText)
{
    size_t        l_dwLength = 0;
    unsigned char l_bCh      = 0;

    if (NULL == i_pText)
    {
        return 0;
    }

    while ( 0 != (*i_pText))
    {
        l_bCh = *i_pText;
        if (0x7F >= l_bCh) //1 char
        {
            i_pText += 1;
        }
        else if (0xE0 > l_bCh) //11100000b = 2 chars
        {
            i_pText += 2;
        }
        else if (0xF0 > l_bCh) //11110000b  = 3 chars
        {
            i_pText += 3;
        }
        else if (0xF8 > l_bCh) //11111000b  = 4 chars
        {
            i_pText += 4;
        }
        else if (0xFC > l_bCh) //11111100b  = 5 chars
        {
            i_pText += 5;
        }
        else //6 chars
        {
            i_pText += 6;
        }

        l_dwLength ++;
    }

    return l_dwLength;
}//Get_utf8_Length


////////////////////////////////////////////////////////////////////////////////
//Get_UTF16_Length
static UNUSED_FUNC size_t Get_UTF16_Length(const tWCHAR *i_pText)
{
    size_t  l_dwLength = 0;
    tWCHAR  l_wCh      = 0;

    if (NULL == i_pText)
    {
        return 0;
    }

    while ( 0 != (*i_pText))
    {
        l_wCh = *i_pText;

        if (    (l_wCh >= 0xD800ul) //processing surrogate pairs
             && (l_wCh <= 0xDFFFul)
           )
        {
            i_pText += 2;
        }
        else
        {
            i_pText += 1;
        }

        l_dwLength ++;
    }

    return l_dwLength;
}//Get_UTF16_Length


////////////////////////////////////////////////////////////////////////////////
//Get_UTF8_Bytes
//return: count characters in UTF-16 string
static UNUSED_FUNC size_t Get_UTF8_Stat(const tWCHAR *i_pStr, 
                                        size_t       &o_rBytes, 
                                        size_t       &o_rChars)
{
    size_t  l_szReturn = 0;
    tUINT32 l_dwCh     = 0;
    
    if (NULL == i_pStr)
    {
        return -1;
    }

    o_rBytes = 0;
    o_rChars = 0;

    while (0ul != (*i_pStr)) 
    {
        l_dwCh = (tUINT16)(*i_pStr);

        if (    (l_dwCh >= 0xD800ul) //processing surrogate pairs
             && (l_dwCh <= 0xDFFFul)
           )
        {
            tUINT32 l_dwTrailing = (tUINT16)*(++i_pStr);
            l_szReturn ++;
            if (    (0xDC00ul <= l_dwTrailing)
                 && (0xDFFFul >= l_dwTrailing)
               )
            {
                l_dwCh = 0x10000ul + (((l_dwCh & 0x3FFul) << 10) | (l_dwTrailing & 0x3FFul));
            }
            else //unexpected 
            {
                l_dwCh = '?';
            }
        }

        if (0x80 > l_dwCh)
        {
            o_rBytes++;
            o_rChars++;
        }
        else if (0x800ul > l_dwCh)
        {
            o_rBytes += 2;
            o_rChars++;
        }
        else if (0x10000ul > l_dwCh)
        {
            o_rBytes += 3;
            o_rChars++;
        }
        else if (0x200000ul  > l_dwCh)
        {
            o_rBytes += 4;
            o_rChars++;
        }
        else
        {
            o_rBytes += 1;
            o_rChars++;
        }

        if (*i_pStr)
        {
            ++i_pStr;
            ++l_szReturn;
        }
        else
        {
            break;
        }
    }
    
    return l_szReturn;
}//Get_UTF8_Bytes


////////////////////////////////////////////////////////////////////////////////
//Convert_UTF8_To_UTF16
//Warning: this function do not produce UTF-16 surrogate pairs - function replace
//surrogate pairs by '*' char. Surrogate pairs will be implemented in next life :-)
//Works only with LE
static UNUSED_FUNC tINT32 Convert_UTF8_To_UTF16(const char *i_pSrc, 
                                                tWCHAR     *o_pDst, 
                                                tUINT32     i_dwMax_Len
                                               )
{
    tINT32        l_iLength = 0;
    unsigned char l_bCh     = 0;
    
    if (    (NULL == i_pSrc)
         || (NULL == o_pDst)  
         || (0    >= i_dwMax_Len)   
       )
    {
        return l_iLength;
    }

    i_dwMax_Len -= 1; //reserve space to trailing \0;
    
    while (    ( 0 != (*i_pSrc)) 
            && (i_dwMax_Len > (tUINT32)l_iLength)
          )
    {
        l_bCh = *i_pSrc;
        if (0x7F >= l_bCh) //1 char
        {
            *o_pDst = (tWCHAR)l_bCh;
        }
        else if (0xE0 > l_bCh) //11100000b = 2 chars
        {
            *o_pDst  = (tWCHAR)((l_bCh & 0x1F) << 6);
            *o_pDst |= (*(++i_pSrc) & 0x3F);
        }
        else if (0xF0 > l_bCh) //11110000b  = 3 chars
        {
            *o_pDst  = (tWCHAR)((l_bCh & 0xF) << 12);
            *o_pDst |= (*(++i_pSrc) & 0x3F) << 6;
            *o_pDst |= (*(++i_pSrc) & 0x3F);
        }
        else if (0xF8 > l_bCh) //11111000b  = 4 chars
        {
            *o_pDst = (tWCHAR)'*';
            i_pSrc += 3;
        }
        else if (0xFC > l_bCh) //11111100b  = 5 chars
        {
            *o_pDst = (tWCHAR)'*';
            i_pSrc += 4;
        }
        else //6 chars
        {
            *o_pDst = (tWCHAR)'*';
            i_pSrc += 5;
        }
        
        o_pDst ++;
        i_pSrc ++;
        l_iLength ++;
    }
    
    *o_pDst = 0;
    
    return l_iLength;
}//Convert_UTF8_To_UTF16


////////////////////////////////////////////////////////////////////////////////
//Convert_UTF32_To_UTF8 (LE)
//Return - count of the used bytes, except trailing 0, if destination buffer 
//size is not enough to store whole source string, result will be truncated
static UNUSED_FUNC tINT32 Convert_UTF32_To_UTF8(const tUINT32 *i_pSrc, 
                                                tACHAR        *o_pDst, 
                                                tUINT32        i_dwDst_Len
                                               )
{
    tINT32  l_iLength = i_dwDst_Len;
    tUINT32 l_dwCh    = 0;
    
    if (    (NULL == i_pSrc)
         || (NULL == o_pDst)  
         || (0    >= i_dwDst_Len)   
       )
    {
        return -1;
    }

    while (    ( 0ul != (*i_pSrc)) 
            && (2 <= l_iLength)
          )
    {
        l_dwCh = (*i_pSrc);

        if (0x80 > l_dwCh)
        {
            *o_pDst = (tACHAR)(l_dwCh & 0x7Ful);
            o_pDst ++;
            l_iLength--;
        }
        else if (0x800ul > l_dwCh)
        {
            if (3 <= l_iLength)
            {
                *o_pDst = (tACHAR)(0xC0ul | ((l_dwCh >> 6) & 0x1Ful));  o_pDst ++;
                *o_pDst = (tACHAR)(0x80ul | ((l_dwCh >> 0) & 0x3Ful));  o_pDst ++;
                l_iLength -= 2;
            }
            else
            {
                break;
            }
        }
        else if (0x10000ul > l_dwCh)
        {
            if (4 <= l_iLength)
            {
                *o_pDst = (tACHAR)(0xE0ul | ((l_dwCh >> 12) & 0x0Ful));  o_pDst ++;
                *o_pDst = (tACHAR)(0x80ul | ((l_dwCh >>  6) & 0x3Ful));  o_pDst ++;
                *o_pDst = (tACHAR)(0x80ul | ((l_dwCh >>  0) & 0x3Ful));  o_pDst ++;
                l_iLength -= 3;
            }
            else
            {
                break;
            }
        }
        else if (0x200000ul  > l_dwCh)
        {
            if (5 <= l_iLength)
            {
                *o_pDst = (tACHAR)(0xF0ul | ((l_dwCh >> 18) & 0x07ul)); o_pDst ++;
                *o_pDst = (tACHAR)(0x80ul | ((l_dwCh >> 12) & 0x3Ful)); o_pDst ++;
                *o_pDst = (tACHAR)(0x80ul | ((l_dwCh >>  6) & 0x3Ful)); o_pDst ++;
                *o_pDst = (tACHAR)(0x80ul | ((l_dwCh >>  0) & 0x3Ful)); o_pDst ++;
                l_iLength -= 4;
            }
            else
            {
                break;
            }
        }
        else
        {
            *o_pDst = '?';
            o_pDst ++;
            l_iLength -= 1;
        }

        if (*i_pSrc)
        {
            ++i_pSrc;
        }
        else
        {
            break;
        }
    }
    
    *o_pDst = 0;
    
    return i_dwDst_Len - l_iLength;
}//Convert_UTF32_To_UTF8


////////////////////////////////////////////////////////////////////////////////
//Convert_UTF32_To_UTF16 (LE)
//Return - count of the used bytes, except trailing 0, if destination buffer 
//size is not enough to store whole source string, result will be truncated
static UNUSED_FUNC tINT32 Convert_UTF32_To_UTF16(const tUINT32 *i_pSrc, 
                                                 tWCHAR        *o_pDst, 
                                                 tUINT32        i_dwDst_Len
                                                )
{
    tINT32  l_iLength = i_dwDst_Len;
    tUINT32 l_dwCh;
    
    if (    (NULL == i_pSrc)
         || (NULL == o_pDst)  
         || (0    >= i_dwDst_Len)   
       )
    {
        return -1;
    }

    while (    ( 0ul != (*i_pSrc)) 
            && (3 <= l_iLength)
          )
    {
        if (*i_pSrc < 0x10000u)
        {
            *o_pDst++ = (tWCHAR)*i_pSrc;
            l_iLength --;
        }
        else
        {
            l_dwCh = *i_pSrc - 0x10000u;
            *o_pDst++ = (tWCHAR)(0xD800 | ((l_dwCh >> 10) & 0xFFFF));
            *o_pDst++ = (tWCHAR)(0xDC00 | ((l_dwCh & 0x3FF) & 0xFFFF));
            l_iLength -= 2;
        }

        ++i_pSrc;
    }
    
    *o_pDst = 0;
    
    return i_dwDst_Len - l_iLength;
}//Convert_UTF32_To_UTF16


////////////////////////////////////////////////////////////////////////////////
//Convert_UTF16_To_UTF8 (LE)
//Return - count of the used bytes, except trailing 0, if destination buffer 
//size is not enough to store whole source string, result will be truncated
static UNUSED_FUNC tINT32 Convert_UTF16_To_UTF8(const tWCHAR *i_pSrc, 
                                                tACHAR       *o_pDst, 
                                                tUINT32       i_dwDst_Len
                                               )
{
    tINT32  l_iLength = i_dwDst_Len;
    tUINT32 l_dwCh    = 0;
    
    if (    (NULL == i_pSrc)
         || (NULL == o_pDst)  
         || (0    >= i_dwDst_Len)   
       )
    {
        return -1;
    }

    while (    ( 0ul != (*i_pSrc)) 
            && (2 <= l_iLength)
          )
    {
        l_dwCh = (tUINT16)(*i_pSrc);

        if (    (l_dwCh >= 0xD800ul) //processing surrogate pairs
             && (l_dwCh <= 0xDFFFul)
           )
        {
            tUINT32 l_dwTrailing = (tUINT16)*(++i_pSrc);
            if (    (0xDC00ul <= l_dwTrailing)
                 && (0xDFFFul >= l_dwTrailing)
               )
            {
                l_dwCh = 0x10000ul + (((l_dwCh & 0x3FFul) << 10) | (l_dwTrailing & 0x3FFul));
            }
            else //unexpected 
            {
                l_dwCh = '?';
            }
        }

        if (0x80 > l_dwCh)
        {
            *o_pDst = (tACHAR)(l_dwCh & 0x7Ful);
            o_pDst ++;
            l_iLength--;
        }
        else if (0x800ul > l_dwCh)
        {
            if (3 <= l_iLength)
            {
                *o_pDst = (tACHAR)(0xC0ul | ((l_dwCh >> 6) & 0x1Ful));  o_pDst ++;
                *o_pDst = (tACHAR)(0x80ul | ((l_dwCh >> 0) & 0x3Ful));  o_pDst ++;
                l_iLength -= 2;
            }
            else
            {
                break;
            }
        }
        else if (0x10000ul > l_dwCh)
        {
            if (4 <= l_iLength)
            {
                *o_pDst = (tACHAR)(0xE0ul | ((l_dwCh >> 12) & 0x0Ful));  o_pDst ++;
                *o_pDst = (tACHAR)(0x80ul | ((l_dwCh >>  6) & 0x3Ful));  o_pDst ++;
                *o_pDst = (tACHAR)(0x80ul | ((l_dwCh >>  0) & 0x3Ful));  o_pDst ++;
                l_iLength -= 3;
            }
            else
            {
                break;
            }
        }
        else if (0x200000ul  > l_dwCh)
        {
            if (5 <= l_iLength)
            {
                *o_pDst = (tACHAR)(0xF0ul | ((l_dwCh >> 18) & 0x07ul)); o_pDst ++;
                *o_pDst = (tACHAR)(0x80ul | ((l_dwCh >> 12) & 0x3Ful)); o_pDst ++;
                *o_pDst = (tACHAR)(0x80ul | ((l_dwCh >>  6) & 0x3Ful)); o_pDst ++;
                *o_pDst = (tACHAR)(0x80ul | ((l_dwCh >>  0) & 0x3Ful)); o_pDst ++;
                l_iLength -= 4;
            }
            else
            {
                break;
            }
        }
        else
        {
            *o_pDst = '?';
            o_pDst ++;
            l_iLength -= 1;
        }

        if (*i_pSrc)
        {
            ++i_pSrc;
        }
        else
        {
            break;
        }
    }
    
    *o_pDst = 0;
    
    return i_dwDst_Len - l_iLength;
}//Convert_UTF8_To_UTF16


#endif //UTF_CONV_H
