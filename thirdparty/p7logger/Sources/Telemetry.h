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
// This file provide trace functionality                                       /
////////////////////////////////////////////////////////////////////////////////
#ifndef TELEMETRY_H
#define TELEMETRY_H


////////////////////////////////////////////////////////////////////////////////
//CP7Tel_Counter
class CP7Tel_Counter
{
private:
    tBOOL           m_bInitialized;

public:
    tXCHAR            *m_pName;
    tBOOL              m_bDelivered;
    sP7Tel_Counter_v2 *m_pHeader;
    tUINT16            m_wSeqN; 
    tUINT32            m_uHash;
    CP7Tel_Counter    *pTreeNext;

    CP7Tel_Counter(tUINT16       i_wID,
                   tUINT8        i_bOn,
                   tDOUBLE       i_dbMin,
                   tDOUBLE       i_dbAlarmMin,
                   tDOUBLE       i_dbMax,
                   tDOUBLE       i_dbAlarmMax,
                   const tXCHAR *i_pName,
                   tUINT32       i_uHash
                   );
    ~CP7Tel_Counter();

    static tUINT32  Get_Hash(const tXCHAR *i_pName);

    tBOOL           Has_Name(const tXCHAR *i_pName);
    tBOOL           Is_Initialized();
    void            Enable(tUINT16 i_wOn);
};//CP7Tel_Counter


////////////////////////////////////////////////////////////////////////////////
//sCounterMap
struct sCounterMap
{
    CP7Tel_Counter *pCounter;

    sCounterMap(CP7Tel_Counter *i_pCounter)
        : pCounter(i_pCounter)
    {
    }

    ~sCounterMap()
    {
        pCounter = NULL;
    }
};//sCounterMap


////////////////////////////////////////////////////////////////////////////////
//CCounters_Tree
class CCounters_Tree:
    public CRBTree<sCounterMap*, tUINT32>
{
public:
    CCounters_Tree()
        : CRBTree<sCounterMap*, tUINT32>(16, TRUE)
    {
    }

protected:
    //////////////////////////////////////////////////////////////////////////// 
    //Return
    //TRUE  - if (i_pKey < i_pData::key)
    //FALSE - otherwise
    tBOOL Is_Key_Less(tUINT32 i_uKey, sCounterMap *i_pData) 
    {
        return (i_uKey < i_pData->pCounter->m_uHash) ? TRUE : FALSE;
    }

    //////////////////////////////////////////////////////////////////////////// 
    //Return
    //TRUE  - if (i_pKey == i_pData::key)
    //FALSE - otherwise
    tBOOL Is_Qual(tUINT32 i_uKey, sCounterMap *i_pData) 
    {
        return (i_uKey == i_pData->pCounter->m_uHash) ? TRUE : FALSE;
    }
};//CCounters_Tree



////////////////////////////////////////////////////////////////////////////////
//CP7Telemetry
class CP7Telemetry
   : public IP7_Telemetry
{
    //put volatile variables at the top, to obtain 32 bit alignment. 
    //Project has 8 bytes alignment by default
    tINT32 volatile         m_lReference;
                           
    IP7_Client             *m_pClient; 
    tUINT32                 m_dwChannel_ID;
                           
    tBOOL                   m_bInitialized;

    tBOOL                   m_bActive;
                           
    tLOCK                   m_sCS; 
                           
    sP7Tel_Info             m_sHeader_Info;
    sP7Tel_Value_v2         m_sValue;
                           
    CCounters_Tree          m_cCounters_Map;
    CBList<CP7Tel_Counter*> m_pCounters;
                           
    tBOOL                   m_bIsHeaderDelivered;
    sP7C_Status             m_sStatus;
                           
    sP7C_Data_Chunk        *m_pChunks;
    tUINT32                 m_dwChunks_Max_Count; 
                           
    tBOOL                   m_bIs_Channel;
                           
    CShared::hShared        m_hShared;
                           
    stTelemetry_Conf        m_sConf;

    CMEvent                 m_cOnConnect_Event;
    tBOOL                   m_bOnConnect_Thread;
    CThShell::tTHREAD       m_hOnConnect_Thread;


public:
    CP7Telemetry(IP7_Client *i_pClient, const tXCHAR *i_pName, const stTelemetry_Conf *i_pConf); 
    virtual ~CP7Telemetry();

    tBOOL               Is_Initialized();

    //IP7C_Channel--------------------------------------------------------------
    IP7C_Channel::eType Get_Type() { return IP7C_Channel::eTelemetry; }
    void                On_Init(sP7C_Channel_Info *i_pInfo);
    void                On_Receive(tUINT32 i_dwChannel, tUINT8 *i_pBuffer, tUINT32 i_dwSize, tBOOL i_bBigEndian);
    void                On_Status(tUINT32 i_dwChannel, const sP7C_Status *i_pStatus);
    void                On_Flush(tUINT32 i_dwChannel, tBOOL *io_pCrash);

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

    //IP7_Telemetry-------------------------------------------------------------
    virtual tBOOL         Create(const tXCHAR *i_pName, 
                                 tDOUBLE       i_dbMin,
                                 tDOUBLE       i_dbAlarmMin,
                                 tDOUBLE       i_dbMax,
                                 tDOUBLE       i_dbAlarmMax,
                                 tBOOL         i_bOn,
                                 tUINT16      *o_pID 
                                );
                          
    virtual tBOOL         Find(const tXCHAR *i_pName, tUINT16 *o_pID);
    virtual tBOOL         Add(tUINT16 i_bID, tDOUBLE i_llValue);
    virtual tBOOL         AddU64(tUINT16 i_bID, tUINT64 i_llValue);
    virtual tBOOL         Set_Enable(tUINT16 i_wID, tBOOL i_bEnable);
    virtual tBOOL         Get_Enable(tUINT16 i_wID);
    virtual tDOUBLE       Get_Min(tUINT16 i_wID);
    virtual tDOUBLE       Get_Max(tUINT16 i_wID);
    virtual const tXCHAR *Get_Name(tUINT16 i_wID);
    virtual tUINT16       Get_Count();

    virtual tBOOL         Share(const tXCHAR *i_pName);



private:
    void  Flush();

    static THSHELL_RET_TYPE THSHELL_CALL_TYPE Static_OnConnect_Routine(void *i_pContext)
    {
        CP7Telemetry *l_pRoutine = static_cast<CP7Telemetry *>(i_pContext);
        if (l_pRoutine)
        {
            l_pRoutine->OnConnect_Routine();
        }

        CThShell::Cleanup();
        return THSHELL_RET_OK;
    } 

    void OnConnect_Routine();
};//CP7Telemetry

#endif //TELEMETRY_H
