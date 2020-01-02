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

////////////////////////////////////////////////////////////////////////////////
///////////////////////////IP7_Telemetry////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
class IP7_Telemetry
   : public IP7C_Channel
{
public:
    ////////////////////////////////////////////////////////////////////////////
    //Create - creates  new  telemetry  counter,  max  amount  of  counters  per 
    //         telemetry instance - 256
    //         See documentation for details.
    virtual tBOOL Create(const tXCHAR *i_pName, 
                         tDOUBLE       i_dbMin,
                         tDOUBLE       i_dbAlarmMin,
                         tDOUBLE       i_dbMax,
                         tDOUBLE       i_dbAlarmMax,
                         tBOOL         i_bOn,
                         tUINT16      *o_pID 
                        )                                                   = 0;

    ////////////////////////////////////////////////////////////////////////////
    //Add - add counter sample 
    //      See documentation for details.
    virtual tBOOL Add(tUINT16 i_bID, tDOUBLE i_llValue)                     = 0;

    ////////////////////////////////////////////////////////////////////////////
    //Add - add counter sample 
    //      See documentation for details.
    //      UINT64 value type
    virtual tBOOL         AddU64(tUINT16 i_bID, tUINT64 i_llValue)          = 0;

    ////////////////////////////////////////////////////////////////////////////
    //Find - find counter ID by name (case sensitive)
    //       See documentation for details.
    virtual tBOOL Find(const tXCHAR *i_pName, tUINT16 *o_pID)               = 0;

    ////////////////////////////////////////////////////////////////////////////
    //Set_Enable - enable or disable counter
    virtual tBOOL Set_Enable(tUINT16 i_wID, tBOOL i_bEnable)                = 0;

    ////////////////////////////////////////////////////////////////////////////
    //Get_Enable - get enable flag
    virtual tBOOL Get_Enable(tUINT16 i_wID)                                 = 0;

    ////////////////////////////////////////////////////////////////////////////
    //Get_Min - get min value
    virtual tDOUBLE Get_Min(tUINT16 i_wID)                                  = 0;

    ////////////////////////////////////////////////////////////////////////////
    //Get_Max - get max value
    virtual tDOUBLE Get_Max(tUINT16 i_wID)                                  = 0;

    ////////////////////////////////////////////////////////////////////////////
    //Get_Name - get name
    virtual const tXCHAR *Get_Name(tUINT16 i_wID)                           = 0;

    ////////////////////////////////////////////////////////////////////////////
    //Get_Count - get amount of counters
    virtual tUINT16 Get_Count()                                             = 0;

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
