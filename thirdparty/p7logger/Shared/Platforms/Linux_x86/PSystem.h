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

#include <unistd.h>

class CSys
{
public:
    ///////////////////////////////////////////////////////////////////////////////
    //Get_Host_Name
    static tBOOL Get_Host_Name(tWCHAR *o_pName, size_t i_szName)
    {
        const size_t l_szName = 256;
        char         l_pName[l_szName];

        if (0 == gethostname(l_pName, l_szName))
        {
            //If the null-terminated hostname is too large to fit, then the name
            //is truncated, and no error is returned (but see NOTES below).
            //POSIX.1-2001 says that if such truncation occurs, then it is
            //unspecified whether the returned buffer includes a terminating
            //null byte.
            l_pName[l_szName-1] = 0;
        }
        else
        {
            strcpy(l_pName, "Unknown:Error");
        }

        Convert_UTF8_To_UTF16(l_pName, o_pName, i_szName);
        return TRUE;
    }//Get_Host_Name

    ///////////////////////////////////////////////////////////////////////////////
    //Get_Host_Name
    static tBOOL Get_Host_Name(tACHAR *o_pName, size_t i_szName)
    {
        if (0 == gethostname(o_pName, i_szName))
        {
            //If the null-terminated hostname is too large to fit, then the name
            //is truncated, and no error is returned (but see NOTES below).
            //POSIX.1-2001 says that if such truncation occurs, then it is
            //unspecified whether the returned buffer includes a terminating
            //null byte.
            o_pName[i_szName-1] = 0;
        }
        else
        {
            strcpy(o_pName, "Unknown:Error");
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
        time_t l_sTime = time(NULL);
        struct tm l_sTM = *localtime(&l_sTime);

        o_rYear   = (uint32_t)(l_sTM.tm_year + 1900);
        o_rMonth  = (uint32_t)(l_sTM.tm_mon + 1);
        o_rDay    = (uint32_t)l_sTM.tm_mday;
        o_rHour   = (uint32_t)l_sTM.tm_hour;
        o_rMinute = (uint32_t)l_sTM.tm_min;
        o_rSecond = (uint32_t)l_sTM.tm_sec;
        o_rmSec   = 0;

        return TRUE;
    }//Get_Host_Name

};


#endif //PSYSTEM_H
