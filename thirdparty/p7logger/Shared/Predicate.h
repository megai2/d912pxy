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
#pragma once

#define PR_USE_REGEXP

#if defined(PR_USE_REGEXP)
    #pragma warning(disable : 4995 )
    #include <QRegExp>
    #include <QString>
#endif


////////////////////////////////////////////////////////////////////////////////
enum ePr_VType
{
    EPR_VTYPE_UNK     = -1,
    EPR_VTYPE_INT     =  0,
    EPR_VTYPE_FLOAT       ,
    EPR_VTYPE_DATE        ,
    EPR_VTYPE_WTEXT       ,
    EPR_VTYPE_ENUM         //user enumes +1, +2, +3, ... etc
};


////////////////////////////////////////////////////////////////////////////////
enum ePr_Op
{
    EPR_OP_UNK            = -1, // 
    EPR_OP_EQUAL          =  0, //=
    EPR_OP_NOT_EQUAL          , //!=
    EPR_OP_GREATER            , //>
    EPR_OP_GREATER_OR_EQUAL   , //>=
    EPR_OP_LESS               , //<
    EPR_OP_LESS_OR_EQUAL      , //<=
    EPR_OP_LEFT_INCLUDE       , //<<
    EPR_OP_RIGHT_INCLUDE      , //>>
    EPR_OP_LEFT_NOT_INCLUDE   , //<!
    EPR_OP_RIGHT_NOT_INCLUDE  , //!>

#if defined(PR_USE_REGEXP)
    EPR_OP_REG_EXP            , //r=
    EPR_OP_WILD_CARD            //w=
#endif
};


////////////////////////////////////////////////////////////////////////////////
struct sPr_Op_Text
{
    const tXCHAR  *pText;
    tUINT32        dwLen;
    ePr_Op         eType;
    ePr_Op         eRevers;
    tBOOL          bDigit;
};

static const sPr_Op_Text g_pOperators[] = 
{
    { TM("<="), 2, EPR_OP_LESS_OR_EQUAL,    EPR_OP_GREATER_OR_EQUAL,  TRUE },
    { TM("<<"), 2, EPR_OP_LEFT_INCLUDE,     EPR_OP_RIGHT_INCLUDE,     FALSE},
    { TM(">>"), 2, EPR_OP_RIGHT_INCLUDE,    EPR_OP_LEFT_INCLUDE,      FALSE},
    { TM("<!"), 2, EPR_OP_LEFT_NOT_INCLUDE, EPR_OP_RIGHT_NOT_INCLUDE, FALSE},
    { TM("!>"), 2, EPR_OP_RIGHT_NOT_INCLUDE,EPR_OP_LEFT_NOT_INCLUDE,  FALSE},
    { TM("!="), 2, EPR_OP_NOT_EQUAL,        EPR_OP_NOT_EQUAL,         TRUE },
    { TM(">="), 2, EPR_OP_GREATER_OR_EQUAL, EPR_OP_LESS_OR_EQUAL,     TRUE },
    { TM("="),  1, EPR_OP_EQUAL,            EPR_OP_EQUAL,             TRUE },
    { TM(">"),  1, EPR_OP_GREATER,          EPR_OP_LESS,              TRUE },
    { TM("<"),  1, EPR_OP_LESS,             EPR_OP_GREATER,           TRUE },

#if defined(PR_USE_REGEXP)
    { TM("r="), 2, EPR_OP_REG_EXP,          EPR_OP_REG_EXP,           FALSE},
    { TM("w="), 2, EPR_OP_WILD_CARD,        EPR_OP_WILD_CARD,         FALSE},
#endif
};


////////////////////////////////////////////////////////////////////////////////
struct sPr_Var
{
    tXCHAR    *m_pName;
    ePr_VType  m_eType;
    tUINT32    m_dwLength; //length of the text if it is
    tBOOL      m_bUsed;

    union 
    {
        tXCHAR  *m_pValue;
        tUINT64  m_qwValue;
        double   m_dbValue;
    };

    sPr_Var(tXCHAR *i_pName, ePr_VType i_eType)
        : m_pName(NULL)
        , m_eType(i_eType)
        , m_dwLength(0)
        , m_bUsed(FALSE)
        , m_qwValue(0) //the largest value in union
    {
        tUINT32 l_dwLen = (i_pName) ? PStrLen(i_pName) : 0;

        if (l_dwLen)
        {
            m_pName = new tXCHAR[l_dwLen + 1];
            if (m_pName)
            {
                PStrCpy(m_pName, l_dwLen + 1, i_pName);
            }
        }
    }

    ~sPr_Var()
    {
        if (m_pName)
        {
            delete [] m_pName;
            m_pName = NULL;
        }
    }
};


////////////////////////////////////////////////////////////////////////////////
struct sPr_Enum_Item
{
    tXCHAR *m_pName;
    int     m_iValue;

    sPr_Enum_Item(tXCHAR *i_pName, int i_iValue)
        : m_pName(NULL)
        , m_iValue(i_iValue)
    {
        tUINT32 l_dwLen = (i_pName) ? PStrLen(i_pName) : 0;

        if (l_dwLen)
        {
            m_pName = new tXCHAR[l_dwLen + 1];
            if (m_pName)
            {
                PStrCpy(m_pName, l_dwLen + 1, i_pName);
            }
        }
    }

    ~sPr_Enum_Item()
    {
        if (m_pName)
        {
            delete [] m_pName;
            m_pName = NULL;
        }
    }
};

////////////////////////////////////////////////////////////////////////////////
struct sPr_Enum
{
    tUINT32                m_dwID;
    CBList<sPr_Enum_Item*> m_cItems;

    sPr_Enum(tUINT32 i_dwID) : m_dwID(i_dwID) {}
    ~sPr_Enum()
    {
        m_cItems.Clear(TRUE);
    }
};


////////////////////////////////////////////////////////////////////////////////
struct sPr_Group;

struct sPr_Node
{
    union 
    {
        sPr_Group *m_pGroup;
        tXCHAR    *m_pValue;
        tUINT64    m_qwValue;
        double     m_dbValue;
    };

    tBOOL    m_bLink; //if TRUE we use m_pGroup, otherwise - values
    ePr_Op   m_eOperator;
    tUINT32  m_dwLength; //length of the text if it is
    sPr_Var *m_pVariable;
#if defined(PR_USE_REGEXP)
    QRegExp *m_pRE;   
#endif

    sPr_Node();
    ~sPr_Node();
};


////////////////////////////////////////////////////////////////////////////////
struct sPr_OrGroup
{
    CBList<sPr_Node*> m_cNodes;

    ~sPr_OrGroup()
    {
        m_cNodes.Clear(TRUE);
    }
};


////////////////////////////////////////////////////////////////////////////////
struct sPr_Group
{
    CBList<sPr_OrGroup*> m_cOrGroups;

    ~sPr_Group()
    {
        m_cOrGroups.Clear(TRUE);
    }
};



////////////////////////////////////////////////////////////////////////////////
//Normal class usage
//1. Create child class with yours structure
//2. Implement abstract function Set_Data()
//3. Set variables list using Set_Variables()
//4. If you are using enums - use Set_Enum()
//5. Set predicate string -  Set_Predicate()
//6. Use Compare() to compare you structure using predicate
//
//See usage example at the end of this header file
class CPredicate
{
protected:
    tXCHAR           *m_pPr;
    tUINT32           m_dwPr_Max;
    tUINT32           m_dwPr_Len;

    CBList<sPr_Var*>  m_cVars;
    CBList<sPr_Enum*> m_cEnums;
    sPr_Group         m_cGroup;
    tXCHAR            m_cError[512];
public:
    CPredicate();
    virtual ~CPredicate();

    ////////////////////////////////////////////////////////////////////////////
    //Predicate (i_pPr) consist of next part
    //Variables - %MyVariable%
    //Constants - "0x20" possible values Int, Float, Date, String, Enum. Date 
    //            has simple format "DD.MM.YYYY HH:MM:SS.MS"
    //Atoms     - atom is working block, you compare variable and constant. It
    //            is not possible to compare 2 variables, only variable and 
    //            constant, next operators are available: "<=", "<<", ">>", "<!"
    //            "!>", "!=", ">=", "=", ">", "<". Operators "<<", ">>" used 
    //            only for text ("T1 << T2" - T1 contains T2)
    //Grouping   - "(" and  ")"
    //Operators - "&", "|"
    //Examples:
    //$Var1$ >="0x10" & "Text" >> $Var2$
    //$Var1$ >="500" & ("Text" >> $Var2$ | "Text1" >> $Var2$)
    virtual tBOOL Set_Predicate(const tXCHAR *i_pPr);
    tXCHAR *Get_Error();


    ////////////////////////////////////////////////////////////////////////////
    //Set_Variables()
    // i_dwCount   - count of the variables, each variable has 2 values
    //   tXCHAR   - name of the variable, allowed any characters
    //   ePr_VType - type of the variable
    // ...         - variables (2 value: name & type)
    tBOOL Set_Variables(tUINT32 i_dwCount, ...);

    ////////////////////////////////////////////////////////////////////////////
    //Set_Enum() 
    // i_dwEnum_ID - ID of the enum, EPR_VTYPE_ENUM, EPR_VTYPE_ENUM + 1, ... etc
    // i_dwCount   - count of the enum values. Each value has 2 fields
    //   tXCHAR   - text value representation
    //   int       - int value representation
    // ...         - enum values (2 fields: text & int)
    tBOOL Set_Enum(tUINT32 i_dwEnum_ID, tUINT32 i_dwCount, ...);


    tBOOL Check(const void *i_pData);
protected:
    tBOOL Check(sPr_Group *i_pGroup);

    tBOOL Build(sPr_Group *i_pGroup, tUINT32 *io_pOffset);
    tBOOL Add_Node(CBList<sPr_Node*> *i_pNodes, tUINT32 *io_pOffset);
    tBOOL Fill_Node(sPr_Node *i_pNode,
                    tXCHAR  *i_pVariable,
                    tXCHAR  *i_pConst
                   );

    tBOOL Is_True(sPr_Node *i_pNode);

    tBOOL Is_Sub(tXCHAR   *i_pString, 
                 tUINT32   i_dwLenght, 
                 tXCHAR   *i_pSearch,
                 tUINT32   i_dwSub_Len
                );

    //re-implement this function, you should get yours input structure and 
    //every field put to the m_cVars list.
    virtual tBOOL   Fill_Variables(const void *i_pData) = 0;
};


////////////////////////////////////////////////////////////////////////////////
//                          Predicate Example
////////////////////////////////////////////////////////////////////////////////
//
//
// enum eTest1
// {
//     ETEST1_ENUM0 = 0,
//     ETEST1_ENUM1 = 1,
//     ETEST1_ENUM2 = 2,
//     ETEST1_ENUM3 = 3
// };
//
// struct sTest1
// {
//     tUINT32    m_dwInt;
//     eTest1   m_eEnum;
//     tXCHAR *m_pText;
// };
// 
////////////////////////////////////////////////////////////////////////////////
// class CMy_Pred:
//     public CPredicate
// {
// public:
//     CMy_Pred():
//         CPredicate()
//     {
//        Set_Variables(3, 
//                      TM("Int"),  EPR_VTYPE_INT,        //Index = 0;
//                      TM("Enum"), EPR_VTYPE_ENUM + 0,   //Index = 1;
//                      TM("Text"), EPR_VTYPE_WTEXT        //Index = 2;
//                     );
// 
//        Set_Enum(EPR_VTYPE_ENUM + 0, 
//                 4,
//                 TM("ENUM0"),   0, //ETEST1_ENUM0 = 0,
//                 TM("ENUM1"),   1, //ETEST1_ENUM1 = 1,
//                 TM("ENUM2"),   2, //ETEST1_ENUM2 = 2,
//                 TM("ENUM3"),   3  //ETEST1_ENUM3 = 3
//                );
//     }
// 
// protected:
//     BOOL Fill_Variables(void *i_pData)
//     {
//         sPr_Var *l_pVar = m_cVars[0];//Index = 0, first declared variable "Int" 
//         if (l_pVar->m_bUsed)
//         {
//             l_pVar->m_qwValue = ((sTest1*)i_pData)->m_dwInt;
//         }
// 
//         l_pVar = m_cVars[1]; //Index = 1, second declared variable "Enum" 
//         if (l_pVar->m_bUsed)
//         {
//             l_pVar->m_qwValue = ((sTest1*)i_pData)->m_eEnum;
//         }
// 
//         l_pVar = m_cVars[2]; //Index = 2, third declared variable "Text" 
//         if (l_pVar->m_bUsed)
//         {
//             l_pVar->m_pValue = ((sTest1*)i_pData)->m_pText;
//             
//             //Very important to set Text length !!!!
//             if (l_pVar->m_pValue)
//             {
//                 l_pVar->m_dwLength = PStrLen(l_pVar->m_pValue);
//             }
//             else
//             {
//                 l_pVar->m_dwLength = 0;
//             }
//         }
//         return TRUE;
//     }
// };
//
////////////////////////////////////////////////////////////////////////////////
// int wmain(int i_iArgC, tXCHAR* i_pArgV[])
// {
//     CMy_Pred l_cPred;
//     sTest1   l_sTest = {10, ETEST1_ENUM2, TM("My Name")};
// 
//     if (FALSE == l_cPred.Set_Predicate(TM("$Enum$ <= \"ENUM2\" & ($Int$ >= \"0x9\" | $Text$ >> \"Andrey is My Name\")")))
//     {
//         wprintf(l_cPred.Get_Error());
//     }
// 
//     if (l_cPred.Check(&l_sTest))
//     {
//         wprintf(TM("TRUE\n"));
//     }
//     else
//     {
//         wprintf(TM("FALSE\n"));
//     }
// 
//     return 0;
// }
