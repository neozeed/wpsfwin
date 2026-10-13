//=====================================================================
// LIST.HPP
//    LIST Class declaration.
// 
// Date Created :  10/28/94
// Last Update  :  10/28/94
// Written By   :  Mario E. De Armas 
// Compiler Used:  MS Visual C++ 1.5
// 
// Copyright IBM Corporation, 1994. All rights reserved.
// 
// 
// FILE LOG:
// DATE        | ACTION/CHANGE                           | PROGRAMMER 
// 10/28/94    | Creation.                               | MDA
//=====================================================================

#ifndef __LIST_PROTOTYPED__

#define __LIST_PROTOTYPED__


//---------------------------------------------------------------------
// LOCAL STRUCTURES
//---------------------------------------------------------------------
typedef struct _LIST_NODE {      
   DWORD_PTR                m_dwValue;
   struct _LIST_NODE    *m_pNextNode;   
   struct _LIST_NODE    *m_pPrevNode;   
} LIST_NODE;

                        
//---------------------------------------------------------------------
// CLASS DECLARATION
//---------------------------------------------------------------------
class LIST {
   private:                   
      int         m_nCount;
      LIST_NODE   *m_pNodeHead;
      LIST_NODE   *m_pNodeTail;
      
   protected:
      // Memory allocation/deallocation helper functions.
      LIST_NODE   *NewNode(DWORD_PTR dwValue);
      void        FreeNode(LIST_NODE *pNode);
      
      // Remove specific node in list.
      void        RemoveNode(LIST_NODE *pNode);
      
      // Search helper functions (linear search algorithms).
      LIST_NODE   *FindByValue(DWORD_PTR dwValue);
      LIST_NODE   *FindByPosition(int nPosition);

   public:         
      // Constructor and destructor.
      LIST(void);
      ~LIST(void);
   
      // Count of elements.
      int         GetCount(void);
      BOOL        IsEmpty(void);

      // Peek at head or tail.
      LIST_NODE   *GetHead(void);
      LIST_NODE   *GetTail(void);
      
      // Operations.
      // Get head or tail and remove it.
      LIST_NODE   *RemoveHead(void);
      LIST_NODE   *RemoveTail(void);
      
      // Add before head or after tail.
      BOOL        AddHead(DWORD_PTR dwValue);
      BOOL        AddTail(DWORD_PTR dwValue);
      
      // Remove one or all elements.
      void        Remove(DWORD_PTR dwValue);
      void        RemoveAll(void);
      
      // Iteration.
      /*
      DWORD_PTR       GetHeadPosition(void);
      DWORD_PTR       GetTailPosition(void);
      DWORD_PTR       GetNext(DWORD_PTR dwValue);
      DWORD_PTR       GetPrev(DWORD_PTR dwValue);
      */
      
      // Getting/Modifying an element at a given position.
      DWORD_PTR       GetAt(int nPosition);
      void        SetAt(int nPosition,DWORD_PTR dwValue);
      void        RemoveAt(int nPosition);
      
      // Inserting before or after a given position.
      /*
      void        InsertBefore(int nPosition,DWORD_PTR dwNewValue);
      void        InsertAfter(int nPosition,DWORD_PTR dwNewValue);
      */
      
      // Helper function (linear search algorithm).
      int         Find(DWORD_PTR dwValue);
      DWORD_PTR       FindIndex(int nPosition);
};


#endif 

// EOF LIST.HPP

