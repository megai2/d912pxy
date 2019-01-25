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
// This file provide trace functionality                                       /
////////////////////////////////////////////////////////////////////////////////
#ifndef TELEMETRY_H
#define TELEMETRY_H


////////////////////////////////////////////////////////////////////////////////
//CP7Tel_Counter
class CP7Tel_Counter
{
private:
    tBOOL          m_bInitialized;

public:
    sP7Tel_Counter m_sCounter;
    tUINT8         m_bSeqN; 

    CP7Tel_Counter(tUINT8        i_bID,
                   tUINT8        i_bOn,
                   tINT64        i_llMin,
                   tINT64        i_llMax,
                   tINT64        i_llAlarm,
                   const tXCHAR *i_pName
                   );
    ~CP7Tel_Counter();

    tBOOL           Has_Name(const tWCHAR *i_pName);
    tBOOL           Is_Initialized();
    void            Enable(tUINT8 i_bOn);
};//CP7Tel_Counter


////////////////////////////////////////////////////////////////////////////////
//sCounterMap
struct sCounterMap
{
    tUINT8   bID;
    tXCHAR  *pName;

    sCounterMap(const tXCHAR *i_pName, tUINT8 i_bID)
        : bID(i_bID)
        , pName(PStrDub(i_pName))
    {
    }

    ~sCounterMap()
    {
        if (pName)
        {
            PStrFreeDub(pName);
            pName = NULL;
        }
    }
};//sCounterMap


////////////////////////////////////////////////////////////////////////////////
//CCounters_Tree
class CCounters_Tree:
    public CRBTree<sCounterMap*, const tXCHAR*>
{
public:
    CCounters_Tree()
        : CRBTree<sCounterMap*, const tXCHAR*>(16, TRUE)
    {
    }

protected:
    //////////////////////////////////////////////////////////////////////////// 
    //Return
    //TRUE  - if (i_pKey < i_pData::key)
    //FALSE - otherwise
    tBOOL Is_Key_Less(const tXCHAR *i_pKey, sCounterMap *i_pData) 
    {
        return (0 < PStrCmp(i_pKey, i_pData->pName)) ? TRUE : FALSE;
    }

    //////////////////////////////////////////////////////////////////////////// 
    //Return
    //TRUE  - if (i_pKey == i_pData::key)
    //FALSE - otherwise
    tBOOL Is_Qual(const tXCHAR *i_pKey, sCounterMap *i_pData) 
    {
        return (0 == PStrCmp(i_pKey, i_pData->pName)) ? TRUE : FALSE;
    }
};//CCounters_Tree



////////////////////////////////////////////////////////////////////////////////
//CP7Telemetry
class CP7Telemetry
   : public IP7C_Channel
   , public IP7_Telemetry
{
    //put volatile variables at the top, to obtain 32 bit alignment. 
    //Project has 8 bytes alignment by default
    tINT32 volatile     m_lReference;

    IP7_Client         *m_pClient; 
    tUINT32             m_dwChannel_ID;

    tBOOL               m_bInitialized;

    tLOCK               m_sCS; 

    sP7Tel_Info         m_sHeader_Info;
    sP7Tel_Value        m_sValue;

    CCounters_Tree      m_cCounters_Map;
    CP7Tel_Counter     *m_pCounters[P7TELEMETRY_COUNTERS_MAX_COUNT + 1];
    tUINT32             m_dwUsed;

    tUINT32             m_dwResets_Channel; 
    tUINT32             m_dwResets_Counters; 
    sP7C_Status         m_sStatus;

    sP7C_Data_Chunk    *m_pChunks;
    tUINT32             m_dwChunks_Max_Count;

    tBOOL               m_bIs_Channel;

    CShared::hShared    m_hShared;

    stTelemetry_Conf    m_sConf;

public:
    CP7Telemetry(IP7_Client *i_pClient, const tXCHAR *i_pName, const stTelemetry_Conf *i_pConf); 
    virtual ~CP7Telemetry();

    tBOOL          Is_Initialized();

    void           On_Init(sP7C_Channel_Info *i_pInfo);
    void           On_Receive(tUINT32 i_dwChannel, tUINT8 *i_pBuffer, tUINT32 i_dwSize);
    void           On_Status(tUINT32 i_dwChannel, const sP7C_Status *i_pStatus);
    void           On_Flush(tUINT32 i_dwChannel, tBOOL *io_pCrash);

    virtual tBOOL  Create(const tXCHAR  *i_pName, 
                          tINT64         i_llMin,
                          tINT64         i_llMax,
                          tINT64         i_llAlarm,
                          tUINT8         i_bOn,
                          tUINT8        *o_pID 
                         );

    virtual tBOOL  Find(const tXCHAR *i_pName, tUINT8 *o_pID);

    virtual tBOOL  Add(tUINT8 i_bID, tINT64 i_llValue);

    virtual tBOOL  Share(const tXCHAR *i_pName);

    virtual tINT32 Add_Ref()
    {
        return ATOMIC_INC(&m_lReference);
    }

    virtual tINT32 Release()
    {
        tINT32 l_lResult = ATOMIC_DEC(&m_lReference);
        if ( 0 >= l_lResult )
        {
            delete this;
        }

        return l_lResult;
    }


private:
    void  Flush();
};//CP7Telemetry

#endif //TELEMETRY_H
