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
#ifndef IFILE_H
#define IFILE_H



////////////////////////////////////////////////////////////////////////////////
class IFile
{
public:
    enum eFlags
    {
        EOPEN                                                    = 0x00000001UL,
        ECREATE                                                  = 0x00000002UL,
        EACCESS_WRITE                                            = 0x00000004UL,
        EACCESS_READ                                             = 0x00000008UL,
        ESHARE_WRITE                                             = 0x00000010UL,
        ESHARE_READ                                              = 0x00000020UL
    };

public:
    virtual tBOOL   IsOpened()                                              = 0;
    virtual tBOOL   Open(const tXCHAR *i_pName, tUINT32 i_dwFlags)          = 0;
    virtual tBOOL   Close(tBOOL i_bFlush)                                   = 0;
    virtual tBOOL   Set_Position(tUINT64 i_qwOffset)                        = 0;
    virtual tUINT64 Get_Position()                                          = 0;
    virtual tUINT64 Get_Size()                                              = 0;
    virtual size_t  Write(const tUINT8 *i_pBuffer,                        
                          size_t        i_szBuffer,                       
                          tBOOL         i_bFlush                          
                         )                                                  = 0;
    virtual size_t  Read(tUINT8 *o_pBuffer, size_t i_szBuffer)              = 0;

    virtual tINT32  Add_Ref()                                               = 0;
    virtual tINT32  Release()                                               = 0;
};


#endif //IFILE_H
