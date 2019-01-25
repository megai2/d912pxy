////////////////////////////////////////////////////////////////////////////////
//                                                                             /
// 2012-2017 (c) Baical                                                        /
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
#ifndef PSYSTEM_H
#define PSYSTEM_H

class CSys
{
public:
    ///////////////////////////////////////////////////////////////////////////////
    //Get_Host_Name
    static tBOOL Get_Host_Name(tWCHAR *o_pName, size_t i_szName)
    {
        if (    (NULL == o_pName)
             || (32   >= i_szName)
           )
        {
            return FALSE;
        }

        DWORD l_dwMax_Len = (DWORD)i_szName;
        return GetComputerNameW((wchar_t*)o_pName, &l_dwMax_Len); 
    }//Get_Host_Name


    ///////////////////////////////////////////////////////////////////////////////
    //Get_Host_Name
    static tBOOL Get_Host_Name(tACHAR *o_pName, size_t i_szName)
    {
        const size_t l_szName = 256;
        DWORD        l_dwSize = (DWORD)l_szName;
        wchar_t      l_pName[l_szName];

        if (GetComputerNameW((wchar_t*)l_pName, &l_dwSize))
        {
            Convert_UTF16_To_UTF8(l_pName, o_pName, (tUINT32)i_szName);
        }
        else
        {
            strcpy_s(o_pName, i_szName, "Unknown:Error");
        }
        return TRUE;
    }//Get_Host_Name



    ///////////////////////////////////////////////////////////////////////////////
    //Get_DateTime
    static tBOOL Get_DateTime(tUINT32 &o_rYear,
                              tUINT32 &o_rMonth,
                              tUINT32 &o_rDay,
                              tUINT32 &o_rHour,
                              tUINT32 &o_rMinute,
                              tUINT32 &o_rSecond,
                              tUINT32 &o_rmSec
                             )
    {
        SYSTEMTIME l_sTitem = {0};
        GetLocalTime(&l_sTitem);

        o_rYear   = (tUINT32)l_sTitem.wYear;
        o_rMonth  = (tUINT32)l_sTitem.wMonth;
        o_rDay    = (tUINT32)l_sTitem.wDay;
        o_rHour   = (tUINT32)l_sTitem.wHour;
        o_rMinute = (tUINT32)l_sTitem.wMinute;
        o_rSecond = (tUINT32)l_sTitem.wSecond;
        o_rmSec   = (tUINT32)l_sTitem.wMilliseconds;

        return TRUE;
    }//Get_Host_Name

};


#endif //PSYSTEM_H
