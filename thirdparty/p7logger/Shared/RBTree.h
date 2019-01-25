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
//******************************************************************************
// This header file contains red black tree pooled implementation              *
// - http://www.rsdn.ru/article/alg/binstree.xml                               *
// - http://algolist.manual.ru/ds/rbtree.php                                   *
// - http://rain.ifmo.ru/cat/view.php/vis/trees/red-black-2002                 *
//******************************************************************************
#ifndef RBTREE_H
#define RBTREE_H


#define RBT_NODE_RED                                                         (1)
#define RBT_NODE_BLACK                                                       (0)

template <typename tData_Type, typename tKey_Type>
class CRBTree
{
protected:
    struct sNode
    {
        sNode         *pLeft;    
        sNode         *pRight;   
        sNode         *pParent;  
        tData_Type     tData;    
        tACHAR         bColor;   
    };

    struct tPool_Segment
    {
        sNode         *m_pNodes;
        tUINT32        m_dwCount;
        tPool_Segment *m_pNext;
    };

    tPool_Segment     *m_pPool_First;
    sNode             *m_pPool_Node_First;
    //count of cell allocated for every pool segment
    tUINT32            m_dwPool_Size; 
                      
    tBOOL              m_bData_Free;
                      
    sNode              m_sSentinel;// = { m_pNIL, m_pNIL, 0, RBT_NODE_BLACK, 0};
    sNode             *m_pRoot;    // = m_pNIL;      
    sNode             *m_pNIL;     // &m_sSentinel    
public:
    ////////////////////////////////////////////////////////////////////////////
    //CRBTree
    CRBTree(tUINT32 i_dwPool_Size = 128, tBOOL i_bData_Free = TRUE)
    {
        m_pNIL  = &m_sSentinel;
        m_pRoot = m_pNIL;    

        m_sSentinel.bColor  = RBT_NODE_BLACK;
        m_sSentinel.pParent = NULL;
        m_sSentinel.tData   = NULL;
        m_sSentinel.pLeft   = m_pNIL;
        m_sSentinel.pRight  = m_pNIL;

        m_dwPool_Size       = i_dwPool_Size;
        m_pPool_First       = NULL;
        m_pPool_Node_First  = NULL;

        m_bData_Free        = i_bData_Free;
    }//CRBTree


    ////////////////////////////////////////////////////////////////////////////
    //~CRBTree
    virtual ~CRBTree()
    {
        //delete all tree
        Clear();

        tPool_Segment *l_pPool_Tmp = NULL;
        while (m_pPool_First)
        {
            l_pPool_Tmp = m_pPool_First;
            m_pPool_First = m_pPool_First->m_pNext;

            Pool_Segment_Free(l_pPool_Tmp);
        }
    }//~CRBTree


   ////////////////////////////////////////////////////////////////////////////
   //Push
   tData_Type Push(tData_Type i_pData, tKey_Type i_pKey) 
    {
        sNode *l_pCurrent = m_pRoot; 
        sNode *l_pParent  = NULL; 
        sNode *l_pNode    = NULL;


        //find where node belongs
        while (m_pNIL != l_pCurrent)
        {
            if (Is_Qual(i_pKey, l_pCurrent->tData))
            {
                return (l_pCurrent->tData);
            }
            else
            {
                l_pParent = l_pCurrent;
                l_pCurrent = Is_Key_Less(i_pKey, l_pCurrent->tData) ? l_pCurrent->pLeft : l_pCurrent->pRight;
            }
        }

        l_pNode = Node_Allocate();

        if (NULL == l_pNode) 
        {
            return NULL;
        }

        l_pNode->tData   = i_pData;
        l_pNode->pParent = l_pParent;
        l_pNode->pLeft   = m_pNIL;
        l_pNode->pRight  = m_pNIL;
        l_pNode->bColor  = RBT_NODE_RED;

        /* insert node in tree */
        if(l_pParent)
        {
            if(Is_Key_Less(i_pKey, l_pParent->tData))
            {
                l_pParent->pLeft = l_pNode;
            }
            else
            {
                l_pParent->pRight = l_pNode;
            }
        }
        else 
        {
            m_pRoot = l_pNode;
        }

        Fix_Push(l_pNode);
        return l_pNode->tData;
    }//Push


    ////////////////////////////////////////////////////////////////////////////
    //Del
    void Del(sNode *i_pNode) 
    {
        sNode *l_pX = NULL;
        sNode *l_pY = NULL;

        if (    (NULL == i_pNode)
             || (m_pNIL  == i_pNode)
           )
        {
            return;
        }
		
        if (m_bData_Free)
        {
            Data_Release(i_pNode->tData);
        }

        if (    (m_pNIL == i_pNode->pLeft)
             || (m_pNIL == i_pNode->pRight) 
           )
        {
            //y has a m_pNIL node as a child
            l_pY = i_pNode;
        } 
        else 
        {
            //find tree successor with a m_pNIL node as a child
            l_pY = i_pNode->pRight;
            while (m_pNIL != l_pY->pLeft)
            {
                l_pY = l_pY->pLeft;
            }
        }

        //x is y's only child
        if (m_pNIL != l_pY->pLeft)
        {
            l_pX = l_pY->pLeft;
        }
        else
        {
            l_pX = l_pY->pRight;
        }

        /* remove y from the parent chain */
        l_pX->pParent = l_pY->pParent;
        if (l_pY->pParent)
        {
            if (l_pY == l_pY->pParent->pLeft)
            {
                l_pY->pParent->pLeft = l_pX;
            }
            else
            {
                l_pY->pParent->pRight = l_pX;
            }
        }
        else
        {
            m_pRoot = l_pX;
        }

        if (l_pY != i_pNode) 
        {
            i_pNode->tData = l_pY->tData;
        }

        if (l_pY->bColor == RBT_NODE_BLACK)
        {
            Fix_Del (l_pX);
        }

        Node_Free(l_pY);
    }//Del


    ////////////////////////////////////////////////////////////////////////////
    //Find
    tData_Type Find(tKey_Type i_pKey) 
    {
        sNode *l_pCurrent = m_pRoot;
        while (m_pNIL != l_pCurrent)
        {
            if(Is_Qual(i_pKey, l_pCurrent->tData))
            {
                return (l_pCurrent->tData);
            }
            else
            {
                l_pCurrent = Is_Key_Less (i_pKey, l_pCurrent->tData) 
                                          ? l_pCurrent->pLeft
                                          : l_pCurrent->pRight;
            }
        }

        return NULL;
    }//Find


    ////////////////////////////////////////////////////////////////////////////
    //Clear
    void Clear()
    {
        if (m_pNIL == m_pRoot)
        {
            return;
        }

        Clear(m_pRoot);
        m_pRoot = m_pNIL;
    }//Clear


    ////////////////////////////////////////////////////////////////////////////
    //Get_Depth
    void Get_Depth(tUINT32 *o_pMin, tUINT32 *o_pMax)
    {
        if (    (NULL == o_pMin)
             || (NULL == o_pMax)
           )
        {
            return;
        }

        *o_pMin = ((tUINT32)~((tUINT32)0)); //max possible value
        *o_pMax = 0;

        if (m_pNIL != m_pRoot)
        {
            Get_Depth(m_pRoot, o_pMin, o_pMax, 0);
        }
    }//Get_Depth

private:
    ////////////////////////////////////////////////////////////////////////////
    //Pool_Segment_Create
    tBOOL Pool_Segment_Create()
    {
        tBOOL l_bReturn = FALSE;

        tPool_Segment *l_pPool = (tPool_Segment*)MemAlloc(sizeof(tPool_Segment));
        
        if (l_pPool)
        {
            memset(l_pPool, 0, sizeof(tPool_Segment));
            l_pPool->m_dwCount = m_dwPool_Size;
            l_pPool->m_pNodes  = (sNode*)MemAlloc(sizeof(sNode) * l_pPool->m_dwCount);
            
            if (l_pPool->m_pNodes)
            {
                sNode *l_pNode = NULL;

                memset(l_pPool->m_pNodes, 
                       0, 
                       sizeof(tPool_Segment) * l_pPool->m_dwCount
                      );

                l_pNode = l_pPool->m_pNodes;

                for (tUINT32 l_dwIDX = 1; l_dwIDX < l_pPool->m_dwCount; l_dwIDX++)
                {
                    l_pNode->pRight = (l_pNode + 1);
                    l_pNode ++;
                }

                l_bReturn          = TRUE;
                l_pPool->m_pNext   = m_pPool_First;
                m_pPool_First      = l_pPool;
                l_pNode->pRight    = m_pPool_Node_First;
                m_pPool_Node_First = l_pPool->m_pNodes;
            }
        }

        if (FALSE == l_bReturn)
        {
            Pool_Segment_Free(l_pPool);
        }

        return l_bReturn;
    }//Pool_Segment_Create


    ////////////////////////////////////////////////////////////////////////////
    //Pool_Segment_Free
    void Pool_Segment_Free(tPool_Segment *io_pPool)
    {
        if (io_pPool)
        {
            if (io_pPool->m_pNodes)
            {
                MemFree(io_pPool->m_pNodes);
                io_pPool->m_pNodes = NULL;
            }
            MemFree(io_pPool);
        }
    }//Pool_Segment_Free


    ////////////////////////////////////////////////////////////////////////////
    //Clear
    void Clear(sNode *i_pNode)
    {
        if (m_pNIL != i_pNode->pLeft)
        {
            Clear(i_pNode->pLeft);
        }

        if (m_pNIL != i_pNode->pRight)
        {
            Clear(i_pNode->pRight);
        }

        if (m_bData_Free)
        {
            Data_Release(i_pNode->tData);
        }

        if (i_pNode->pParent) 
        {
            if (i_pNode == i_pNode->pParent->pRight)
            {
                i_pNode->pParent->pRight = m_pNIL;
            }
            else
            {
                i_pNode->pParent->pLeft = m_pNIL;
            }
        }

        Node_Free(i_pNode);
    }//Clear


    ////////////////////////////////////////////////////////////////////////////
    //Get_Depth
    void Get_Depth(sNode *i_pNode, 
                   tUINT32 *o_pMin, 
                   tUINT32 *o_pMax, 
                   tUINT32  i_dwCurrent
                  )
    {
        i_dwCurrent ++;

        if (    (m_pNIL == i_pNode->pLeft)
             && (m_pNIL == i_pNode->pRight)
           )
        {
            if (i_dwCurrent < (*o_pMin))
            {
                *o_pMin = i_dwCurrent;
            }

            if (i_dwCurrent > (*o_pMax))
            {
                *o_pMax = i_dwCurrent;
            }
        }
        else
        {
            if (m_pNIL != i_pNode->pLeft)
            {
                Get_Depth(i_pNode->pLeft, o_pMin, o_pMax, i_dwCurrent);
            }

            if (m_pNIL != i_pNode->pRight)
            {
                Get_Depth(i_pNode->pRight, o_pMin, o_pMax, i_dwCurrent);
            }
        }
    }//Get_Depth


    ////////////////////////////////////////////////////////////////////////////
    //Rotate_Left
    void Rotate_Left(sNode *i_pNode) 
    {
        sNode *l_pNodeR = i_pNode->pRight;

        i_pNode->pRight = l_pNodeR->pLeft;
        if (m_pNIL != l_pNodeR->pLeft) 
        {
            l_pNodeR->pLeft->pParent = i_pNode;
        }

        if (m_pNIL != l_pNodeR) 
        {
            l_pNodeR->pParent = i_pNode->pParent;
        }

        if (i_pNode->pParent) 
        {
            if (i_pNode == i_pNode->pParent->pLeft)
            {
                i_pNode->pParent->pLeft = l_pNodeR;
            }
            else
            {
                i_pNode->pParent->pRight = l_pNodeR;
            }
        } 
        else 
        {
            m_pRoot = l_pNodeR;
        }

        l_pNodeR->pLeft = i_pNode;
        if (m_pNIL != i_pNode) 
        {
            i_pNode->pParent = l_pNodeR;
        }
    }//Rotate_Left


    ////////////////////////////////////////////////////////////////////////////
    //Rotate_Right
    void Rotate_Right(sNode *i_pNode) 
    {
        sNode *l_pNodeL = i_pNode->pLeft;

        i_pNode->pLeft = l_pNodeL->pRight;
        if (m_pNIL != l_pNodeL->pRight)
        {
            l_pNodeL->pRight->pParent = i_pNode;
        }

        if (m_pNIL != l_pNodeL) 
        {
            l_pNodeL->pParent = i_pNode->pParent;
        }

        if (i_pNode->pParent) 
        {
            if (i_pNode == i_pNode->pParent->pRight)
            {
                i_pNode->pParent->pRight = l_pNodeL;
            }
            else
            {
                i_pNode->pParent->pLeft = l_pNodeL;
            }
        }
        else 
        {
            m_pRoot = l_pNodeL;
        }

        l_pNodeL->pRight = i_pNode;

        if (m_pNIL != i_pNode)
        {
            i_pNode->pParent = l_pNodeL;
        }
    }//Rotate_Right


    ////////////////////////////////////////////////////////////////////////////
    //Fix_Push
    void Fix_Push(sNode *i_pNode) 
    {
        while (    (i_pNode != m_pRoot)
                && (RBT_NODE_RED == i_pNode->pParent->bColor) 
              )
        {
            // we have a violation
            if (i_pNode->pParent == i_pNode->pParent->pParent->pLeft) 
            {
                sNode *l_pUncle = i_pNode->pParent->pParent->pRight;
                if (RBT_NODE_RED == l_pUncle->bColor) 
                {
                    i_pNode->pParent->bColor          = RBT_NODE_BLACK;
                    l_pUncle->bColor                  = RBT_NODE_BLACK;
                    i_pNode->pParent->pParent->bColor = RBT_NODE_RED;
                    i_pNode                           = i_pNode->pParent->pParent;
                } 
                else //(RBT_NODE_BLACK == l_pUncle->bColor) 
                {
                    if (i_pNode == i_pNode->pParent->pRight) 
                    {
                        /* make x a left child */
                        i_pNode = i_pNode->pParent;
                        Rotate_Left(i_pNode);
                    }

                    /* recolor and rotate */
                    i_pNode->pParent->bColor          = RBT_NODE_BLACK;
                    i_pNode->pParent->pParent->bColor = RBT_NODE_RED;
                    Rotate_Right(i_pNode->pParent->pParent);
                }
            }
            else //(i_pNode->pParent == i_pNode->pParent->pParent->pRight) 
            {
                sNode *l_pUncle = i_pNode->pParent->pParent->pLeft;
                if (RBT_NODE_RED == l_pUncle->bColor) 
                {
                    i_pNode->pParent->bColor          = RBT_NODE_BLACK;
                    l_pUncle->bColor                  = RBT_NODE_BLACK;
                    i_pNode->pParent->pParent->bColor = RBT_NODE_RED;
                    i_pNode                           = i_pNode->pParent->pParent;
                } 
                else //(RBT_NODE_BLACK == l_pUncle->bColor) 
                {
                    if (i_pNode == i_pNode->pParent->pLeft) 
                    {
                        i_pNode = i_pNode->pParent;
                        Rotate_Right(i_pNode);
                    }

                    i_pNode->pParent->bColor          = RBT_NODE_BLACK;
                    i_pNode->pParent->pParent->bColor = RBT_NODE_RED;
                    Rotate_Left(i_pNode->pParent->pParent);
                }
            }
        }

        m_pRoot->bColor = RBT_NODE_BLACK;
    }//Fix_Push


    ////////////////////////////////////////////////////////////////////////////
    //Fix_Del
    void Fix_Del(sNode *i_pNode) 
    {
        while (    (i_pNode != m_pRoot)
                && (RBT_NODE_BLACK == i_pNode->bColor)
              )
        {
            if (i_pNode == i_pNode->pParent->pLeft) 
            {
                sNode *l_pBrother = i_pNode->pParent->pRight;
                if (RBT_NODE_RED == l_pBrother->bColor)
                {
                    l_pBrother->bColor       = RBT_NODE_BLACK;
                    i_pNode->pParent->bColor = RBT_NODE_RED;
                    Rotate_Left(i_pNode->pParent);
                    l_pBrother = i_pNode->pParent->pRight;
                }

                if (    (RBT_NODE_BLACK == l_pBrother->pLeft->bColor) 
                     && (RBT_NODE_BLACK == l_pBrother->pRight->bColor) 
                   )
                {
                    l_pBrother->bColor = RBT_NODE_RED;
                    i_pNode            = i_pNode->pParent;
                } 
                else 
                {
                    if (RBT_NODE_BLACK == l_pBrother->pRight->bColor)
                    {
                        l_pBrother->pLeft->bColor = RBT_NODE_BLACK;
                        l_pBrother->bColor        = RBT_NODE_RED;
                        Rotate_Right (l_pBrother);
                        l_pBrother = i_pNode->pParent->pRight;
                    }
                    l_pBrother->bColor = i_pNode->pParent->bColor;
                    i_pNode->pParent->bColor   = RBT_NODE_BLACK;
                    l_pBrother->pRight->bColor = RBT_NODE_BLACK;
                    Rotate_Left (i_pNode->pParent);
                    i_pNode = m_pRoot;
                }
            } 
            else
            {
                sNode *l_pBrother = i_pNode->pParent->pLeft;
                if (RBT_NODE_RED == l_pBrother->bColor) 
                {
                    l_pBrother->bColor       = RBT_NODE_BLACK;
                    i_pNode->pParent->bColor = RBT_NODE_RED;
                    Rotate_Right (i_pNode->pParent);
                    l_pBrother = i_pNode->pParent->pLeft;
                }
                if (    (RBT_NODE_BLACK == l_pBrother->pRight->bColor)
                     && (RBT_NODE_BLACK == l_pBrother->pLeft->bColor) 
                   )
                {
                    l_pBrother->bColor = RBT_NODE_RED;
                    i_pNode            = i_pNode->pParent;
                } 
                else
                {
                    if (RBT_NODE_BLACK == l_pBrother->pLeft->bColor)
                    {
                        l_pBrother->pRight->bColor = RBT_NODE_BLACK;
                        l_pBrother->bColor         = RBT_NODE_RED;
                        Rotate_Left (l_pBrother);
                        l_pBrother = i_pNode->pParent->pLeft;
                    }
                    l_pBrother->bColor        = i_pNode->pParent->bColor;
                    i_pNode->pParent->bColor  = RBT_NODE_BLACK;
                    l_pBrother->pLeft->bColor = RBT_NODE_BLACK;
                    Rotate_Right (i_pNode->pParent);
                    i_pNode = m_pRoot;
                }
            }
        }

        i_pNode->bColor = RBT_NODE_BLACK;
    }//Fix_Del

protected:
    ////////////////////////////////////////////////////////////////////////////
    // Data_Release
    // override this function in derived class to implement specific data 
    // destructor(structures as example)
    virtual tBOOL Data_Release(tData_Type i_pData)
    {
        if (NULL == i_pData)
        {
            return FALSE;
        }

        delete i_pData;
        return TRUE;
    }// Data_Release


    ////////////////////////////////////////////////////////////////////////////
    //MemAlloc
    //override this function in derived class to implement own memory allocation
    //mechanism, it will be used everywhere in this class
    virtual void *MemAlloc(size_t i_szSize)
    {
        return new tUINT8[i_szSize];
    }//MemAlloc


    ////////////////////////////////////////////////////////////////////////////
    //
    //override this function in derived class to implement own memory deallocation
    //mechanism, it will be used everywhere in this class
    virtual void MemFree(void * i_pMemory)
    {
        delete [] ((tUINT8*)i_pMemory);
    }


    ////////////////////////////////////////////////////////////////////////////
    //Node_Allocate
    virtual sNode *Node_Allocate()
    {
        sNode *l_pReturn = NULL;
        if (NULL == m_pPool_Node_First)
        {
            Pool_Segment_Create();
        }

        l_pReturn = m_pPool_Node_First;

        if (m_pPool_Node_First)
        {
            m_pPool_Node_First = m_pPool_Node_First->pRight;
        }

        return l_pReturn;
    }//Node_Allocate


    ////////////////////////////////////////////////////////////////////////////
    //Node_Free
    virtual void Node_Free(sNode *i_pNode)
    {
        memset(i_pNode, 0, sizeof(sNode));
        i_pNode->pRight    = m_pPool_Node_First;
        m_pPool_Node_First = i_pNode;
    }//Node_Free


    ////////////////////////////////////////////////////////////////////////////
    //        This is abstract functions, you should to implement them        //
    ////////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////// 
    //Return
    //TRUE  - if (i_pKey < i_pData::key)
    //FALSE - otherwise
    virtual tBOOL Is_Key_Less(tKey_Type i_pKey, tData_Type i_pData)         = 0; 

    //////////////////////////////////////////////////////////////////////////// 
    //Return
    //TRUE  - if (i_pKey == i_pData::key)
    //FALSE - otherwise
    virtual tBOOL Is_Qual(tKey_Type i_pKey, tData_Type i_pData)             = 0; 
};




////////////////////////////////////////////////////////////////////////////////
//                                 TEST                                       //
////////////////////////////////////////////////////////////////////////////////

// #include <Windows.h>
// #include <stdio.h>
// #include <conio.h>
// 
// #include "RBTree.h"
// 
// #define _CRTDBG_MAP_ALLOC
// #include <crtdbg.h>
// 
// struct sInt
// {
//     int val;
// };
// 
// 
// class CIntMap:
//     public CRBTree<sInt*, int>
// {
// public:
//     CIntMap():
//         CRBTree(1024, FALSE)
//     {
// 
//     }
// protected:
//     //////////////////////////////////////////////////////////////////////////// 
//     //Return
//     //TRUE  - if (i_pKey < i_pData::key)
//     //FALSE - otherwise
//     tBOOL Is_Key_Less(int i_iKey, sInt *i_pData)
//     {
//         return (i_iKey < i_pData->val);
//     }
// 
//     //////////////////////////////////////////////////////////////////////////// 
//     //Return
//     //TRUE  - if (i_pKey == i_pData::key)
//     //FALSE - otherwise
//     virtual tBOOL Is_Qual(int i_iKey, sInt *i_pData)
//     {
//         return (i_iKey == i_pData->val);
//     }
// 
// };
// 
// 
// int wmain(int argc, wchar_t* argv[])
// {
//     CIntMap *l_pMap = new CIntMap();
//     tUINT32    l_dwMax = 1000000;
//     sInt    *l_pInt = new sInt[l_dwMax];
//     tUINT32    l_dwMiss = 0;
// 
// 
//     for (tUINT32 l_dwIteration = 0; l_dwIteration < 1000; l_dwIteration++)
//     {
//         l_dwMiss = 0;
//         srand(l_dwIteration);
// 
//         for (tUINT32 l_dwIDX = 0; l_dwIDX < l_dwMax; l_dwIDX++)
//         {
//             l_pInt[l_dwIDX].val = rand()*rand();
// 
//             if (&l_pInt[l_dwIDX] != l_pMap->Push(&l_pInt[l_dwIDX], l_pInt[l_dwIDX].val))
//             {
//                 l_dwMiss ++;
//             }
//         }
// 
//         tUINT32 l_dwDMin = 0;
//         tUINT32 l_dwDMax = 0;
//         l_pMap->Get_Depth(&l_dwDMin, &l_dwDMax);
// 
//         printf("Iteration[%d] Miss count = %d, Depth = %d/%d\n", 
//                l_dwIteration,
//                l_dwMiss, 
//                l_dwDMin, 
//                l_dwDMax
//               );
// 
//         if ((l_dwDMin * 2) < l_dwDMax)
//         {
//             printf("Depth error = %d/%d\n", l_dwDMin, l_dwDMax);
//         }
// 
//         l_pMap->Clear();
// 
//         if ((_kbhit()) && (27 == _getwch()))
//         {
//             wprintf(L"... exiting\n");
//             break;
//         }
//     }
// 
// 
//     delete l_pMap;
// 
//     delete [] l_pInt;
// 
//     //#ifndef NDEBUG
//     //
//     //int flag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG); // Get current flag
//     //
//     //flag |= _CRTDBG_LEAK_CHECK_DF; // Turn on leak-checking bit
//     //
//     //_CrtSetDbgFlag(flag); // Set flag to the new value
//     //
//     //#endif 
// 
//     _CrtDumpMemoryLeaks();
// 
//     return 0;
// }


#endif //RBTREE_H
