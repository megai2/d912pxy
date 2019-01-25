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
////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                            P7.Telemetry                                    //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//          Documentation is located in <P7>/Documentation/P7.pdf             //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
#ifndef P7_TELEMETRY_H
#define P7_TELEMETRY_H

#include "P7_Client.h"
#include "P7_Cproxy.h"

#define TELEMETRY_DEFAULT_SHARED_NAME                         TM("P7.Telemetry")

//__declspec(novtable)
class IP7_Telemetry
{
public:
    ////////////////////////////////////////////////////////////////////////////
    //Add_Ref - increase object's reference count
    //          See documentation for details.
    virtual tINT32 Add_Ref()                                                = 0;

    ////////////////////////////////////////////////////////////////////////////
    //Release - decrease object's reference count. If reference count less or
    //          equal to 0 - object will be destroyed
    //          See documentation for details.
    virtual tINT32 Release()                                                = 0;

    ////////////////////////////////////////////////////////////////////////////
    //Create - creates  new  telemetry  counter,  max  amount  of  counters  per 
    //         telemetry instance - 256
    //         See documentation for details.
    virtual tBOOL Create(const tXCHAR  *i_pName, 
                         tINT64         i_llMin,
                         tINT64         i_llMax,
                         tINT64         i_llAlarm,
                         tUINT8         i_bOn,
                         tUINT8        *o_pID 
                        )                                                   = 0;

    ////////////////////////////////////////////////////////////////////////////
    //Add - add counter sample 
    //      See documentation for details.
    virtual tBOOL Add(tUINT8 i_bID, tINT64 i_llValue)                       = 0;

    ////////////////////////////////////////////////////////////////////////////
    //Find - find counter ID by name (case sensitive)
    //       See documentation for details.
    virtual tBOOL Find(const tXCHAR *i_pName, tUINT8 *o_pID)                = 0;

    ////////////////////////////////////////////////////////////////////////////
    //Share  - function to share current P7.Telemetry object in address space of
    //         the current process, to get shared instance use function
    //         P7_Get_Shared_Telemetry(tXCHAR *i_pName)
    //         See documentation for details.
    virtual tBOOL Share(const tXCHAR *i_pName)                              = 0;
};


////////////////////////////////////////////////////////////////////////////////
//P7_Create_Telemetry - function creates new instance of IP7_Telemetry object
//                      See documentation for details.
extern "C" P7_EXPORT IP7_Telemetry* __cdecl P7_Create_Telemetry(IP7_Client             *i_pClient,
                                                                const tXCHAR           *i_pName,
                                                                const stTelemetry_Conf *i_pConf = NULL
                                                               );


////////////////////////////////////////////////////////////////////////////////
//This functions allows you to get P7 telemetry instance if it  was  created  by 
//someone else inside current process. If  no  instance  was  registered  inside
//current process - function will return NULL. Do not forget to call Release  on 
//interface when you finish your work
//See documentation for details.
extern "C" P7_EXPORT IP7_Telemetry* __cdecl P7_Get_Shared_Telemetry(const tXCHAR *i_pName);


#endif //P7_TELEMETRY_H
