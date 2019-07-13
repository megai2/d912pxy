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
#ifndef COMMONCLIENT_H
#define COMMONCLIENT_H

// Linux:
//#include <unistd.h>   
//#include <iostream>
//#include <pthread.h>  
//#include <time.h>
//#include <sys/time.h> 
//#include <errno.h>


////////////////////////////////////////////////////////////////////////////////
//Independent
#include "Common.h"

#include "UDP_NB.h"

#include "PSystem.h"

#include "P7_Client.h"
//because USER_PACKET_CHANNEL_ID_MAX_SIZE defined in "P7_Client.h".

#include "CRC32.h" //used only in TPackets

#include "TPackets.h"
#include "PacketsPool.h"
#include "Client.h"

#include "P7_Telemetry.h"
#include "P7_Trace.h"
#include "P7_Extensions.h"


////////////////////////////////////////////////////////////////////////////////
//CUintList
class CUintList
    : public CListPool<tUINT32>
{
protected:
    virtual tBOOL Data_Release(tUINT32 i_pData)
    {
        UNUSED_ARG(i_pData);
        return TRUE;
    }
};


#endif //COMMONCLIENT_H
