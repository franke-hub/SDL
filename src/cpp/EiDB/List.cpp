//----------------------------------------------------------------------------
//
//       Copyright (C) 2003 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       List.cpp
//
// Purpose-
//       Generic List of (void*) Items.
//
// Last change date-
//       2003/06/17
//
//----------------------------------------------------------------------------
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "List.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "LIST    " // Source file

#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef SCDM
#undef  SCDM                        // If defined, Soft Core Debug Mode
#endif

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define CHUNK_SIZE             4096 // Number of Items allocated in a chunk

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE  1
#endif

//----------------------------------------------------------------------------
//
// Struct-
//       Item
//
// Purpose-
//       Describe an Item pointer.
//
//----------------------------------------------------------------------------
struct Item {                       // Item pointer
   Item*               next;        // Next Item on List
   void*               item;        // -> item
}; // struct Item

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static Item*           head= NULL;  // -> First free Item

//----------------------------------------------------------------------------
//
// Subroutine-
//       releaseItem
//
// Purpose-
//       Release an Item
//
//----------------------------------------------------------------------------
void
   releaseItem(                     // Release an Item
     Item*             ptrI)        // Item to release
{
   ptrI->next= head;
   head= ptrI;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       allocateItem
//
// Purpose-
//       Allocate an Item
//
// Notes-
//       Items are allocated from a free list, which is expanded in chunks.
//       Once a chunk of Items are allocated, they are never returned to
//       the operating system.
//
//----------------------------------------------------------------------------
Item*                               // -> Item
   allocateItem( void )             // Allocate an Item
{
   Item*               ptrI;        // Working Item pointer

   int                 i;

   ptrI= head;
   if( ptrI == NULL )
   {
     ptrI= (Item*)malloc(sizeof(Item)*CHUNK_SIZE);
     #ifdef HCDM
       printf("%p= allocateItem() chunk\n", ptrI);
     #endif
     if( ptrI == NULL )
     {
       fprintf(stderr, "No Storage(%zu)\n", sizeof(Item)*CHUNK_SIZE);
       throw "NoStorageException";
     }

     for(i= 1; i<CHUNK_SIZE; i++)
     {
       releaseItem(ptrI);
       ptrI++;
     }
   }
   else
     head= ptrI->next;

   memset(ptrI, 0, sizeof(*ptrI));
   #ifdef SCDM
     printf("%p= allocateItem()\n", ptrI);
   #endif
   return ptrI;
}

//----------------------------------------------------------------------------
//
// Method-
//       List::~List
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   List::~List( void )              // Destructor
{
   #ifdef SCDM
     printf("List(%p)::~List()\n", this);
   #endif

   empty();
}

//----------------------------------------------------------------------------
//
// Method-
//       List::List
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   List::List( void )               // Constructor
:  count(0)
,  head(NULL)
,  index(0)
,  tail(NULL)
{
   #ifdef SCDM
     printf("List(%p)::List()\n", this);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       List::debug
//
// Purpose-
//       Debugging List display.
//
//----------------------------------------------------------------------------
void
   List::debug( void ) const        // Debugging display
{
   unsigned            index;       // Working Item index
   Item*               ptrI;        // Working Item pointer

   printf("List(%p)::debug(), count(%u) head(%p) index(%u) tail(%p)\n",
          this, count, head, this->index, tail);

   index= 0;
   for(ptrI= (Item*)head; ptrI != NULL; ptrI= ptrI->next)
     printf("[%3d] %p->%p\n", index++, ptrI, ptrI->item);
}

//----------------------------------------------------------------------------
//
// Method-
//       List::getItem
//
// Purpose-
//       Get an Item from the List.
//
//----------------------------------------------------------------------------
void*                               // Return Item
   List::getItem(                   // Get Item from the List
     unsigned          inpIndex)    // Line index
{
   unsigned            index= inpIndex; // Working line index
   Item*               ptrI;        // Working Item pointer

   if( index < this->index )
   {
     this->index= 0;
     tail= head;
   }

   ptrI= (Item*)tail;
   index -= this->index;
   while( ptrI != NULL )
   {
     if( index == 0 )
     {
       #ifdef SCDM
         #ifdef HCDM
           debug();
         #endif
         printf("%p= List(%p)::getItem(%d)\n", ptrI->item, this, inpIndex);
       #endif

       return ptrI->item;
     }

     tail= ptrI= ptrI->next;
     this->index++;
     index--;
   }

   if( inpIndex >= count )
   {
     #ifdef SCDM
       #ifdef HCDM
         debug();
       #endif
       printf("<<null>>= List(%p)::getItem(%d)\n", this, inpIndex);
     #endif

     return NULL;
   }

   printf("ABORT= List(%p)::getItem(%d)\n", this, inpIndex);
   debug();
   throw "RangeException";
   return NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       List::append
//
// Purpose-
//       Add a line into the database.
//
//----------------------------------------------------------------------------
unsigned                            // Item index
   List::append(                    // Add Item onto List
     void*             item)        // Item to add
{
   Item*               ptrI;        // Working Item pointer
   Item*               newI;        // Working Item pointer

   newI= allocateItem();
   newI->item= item;

   ptrI= (Item*)tail;
   if( ptrI == NULL )
   {
     ptrI= (Item*)head;
     if( ptrI == NULL )
     {
       head= newI;
       tail= newI;
       count= 1;
       index= 0;

       #ifdef SCDM
         #ifdef HCDM
           debug();
         #endif
         printf("%8d= List(%p)::append(%p)\n", 1, this, item);
       #endif
       return 1;
     }
     index= 0;
   }

   while( ptrI->next != NULL )
   {
     index++;
     tail= ptrI= ptrI->next;
   }

   count++;
   ptrI->next= newI;

   #ifdef SCDM
     #ifdef HCDM
       debug();
     #endif
     printf("%8d= List(%p)::append(%p)\n", count, this, item);
   #endif
   return count;
}

//----------------------------------------------------------------------------
//
// Method-
//       List::empty
//
// Purpose-
//       Empty the List.
//
//----------------------------------------------------------------------------
void
   List::empty( void )              // Empty the database
{
   Item*               ptrI;        // Working Item pointer

   #ifdef SCDM
     printf("List(%p)::empty()\n", this);
   #endif

   while( head != NULL )
   {
     ptrI= (Item*)head;
     head= ptrI->next;

     releaseItem(ptrI);
   }

   head= tail= NULL;
   count= 0;
   index= 0;
}

