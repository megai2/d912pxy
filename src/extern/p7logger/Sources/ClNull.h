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
////////////////////////////////////////////////////////////////////////////////
// This header file provide functionality to deliver data to NULL              /
////////////////////////////////////////////////////////////////////////////////

#ifndef CLNULL_H
#define CLNULL_H

class CClNull:
    public CClient
{
public:
    CClNull(tXCHAR **i_pArgs,
            tINT32   i_iCount
           );
    virtual ~CClNull();

private:
    eClient_Status Init_Base(tXCHAR **i_pArgs, tINT32 i_iCount);

public:
    eClient_Status Get_Status()
    {
        return ECLIENT_STATUS_OK;
    }

    eClient_Status Sent(tUINT32            i_dwChannel_ID,
                        sP7C_Data_Chunk   *i_pChunks, 
                        tUINT32            i_dwCount,
                        tUINT32            i_dwSize
                       );

    tBOOL Get_Status(sP7C_Status *o_pStatus);
    tBOOL Get_Info(sP7C_Info *o_pInfo);
    tBOOL Flush();
};


#endif //CLNULL_H
