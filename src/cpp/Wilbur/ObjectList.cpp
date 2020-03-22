//----------------------------------------------------------------------------
//
//       Copyright (c) 2012 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       ObjectList.cpp
//
// Purpose-
//       ObjectList object methods.
//
// Last change date-
//       2012/01/01
//
//----------------------------------------------------------------------------
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//nclude <com/DebugObject.h>        // For debugging
#include "ObjectList.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define MAX_COHERENT     1000000000 // Maximum coherent element count

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE  1
#endif

//----------------------------------------------------------------------------
//
// Method-
//       ObjectList::fifo
//
// Purpose-
//       Insert a link onto the list with FIFO ordering.
//
//----------------------------------------------------------------------------
void
   ObjectList::fifo(                // Insert (FIFO order)
     Object*           object)      // -> Object to insert
{
   Link* link= new Link(object);    // Create insert Link
   Link* head= this->head;          // Our head link

   link->setNext(NULL);             // Set next link pointer
   link->setPrev(tail);             // Set prior link pointer

   if( head == NULL )               // If the list is empty
     this->head= link;              // Add link to empty list
   else                             // If the list is not empty
     tail->setNext(link);           // Add link to list

   tail= link;                      // Set new list tail link
}

//----------------------------------------------------------------------------
//
// Method-
//       ObjectList::insert
//
// Purpose-
//       Insert a chain of elements onto the list
//
// Implementation note-
//       Implementation useless unless Links are exposed.
//
//----------------------------------------------------------------------------
void
   ObjectList::insert(              // Add to ObjectList at position,
     Link*             link,        // -> Link to insert after
     Link*             head,        // -> First Link to insert
     Link*             tail)        // -> Final Link to insert
{
   if( link == NULL )               // If insert at head
   {
     head->setPrev(NULL);
     if( this->head == NULL )       // If the list is empty
     {
       tail->setNext(NULL);
       this->tail= tail;
       this->head= head;
     }
     else                           // If the list is populated
     {
       tail->setNext(this->head);
       this->head->setPrev(tail);
       this->head= head;
     }
   }
   else                             // If the list is not empty
   {
     Link* next= link->getNext();   // Address the next Link
     tail->setNext(next);           // Set the forward link pointer
     head->setPrev(link);           // Set the reverse link pointer

     link->setNext(head);           // Insert onto the forward list
     if( next == NULL )             // Insert onto the reverse list
       this->tail= tail;
     else
       next->setPrev(tail);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       ObjectList::isCoherent
//
// Purpose-
//       Check the object for coherency.
//
//----------------------------------------------------------------------------
int                                 // TRUE if object is coherent
   ObjectList::isCoherent( void ) const // Coherency check
{
   Link*               link;        // Pointer to current link
   Link*               prev;        // Pointer to prior   link

   if( head == NULL )               // If the list is empty
   {
     if( tail != NULL )             // If the tail is not NULL
       return FALSE;

     return TRUE;
   }

   link= head;
   prev= NULL;
   for(int count= 0;;count++)
   {
     if( count > MAX_COHERENT )
       return FALSE;

     if( link->getPrev() != prev )
       return FALSE;

     if( link->getNext() == NULL )
       break;
     else if( link == tail )
       return FALSE;

     prev= link;
     link= link->getNext();
   }

   if( tail != link )
     return FALSE;

   return TRUE;
}

//----------------------------------------------------------------------------
//
// Method-
//       ObjectList::isOnList
//
// Purpose-
//       Check whether an Object is on the list.
//
//----------------------------------------------------------------------------
int                                 // TRUE if link is in list
   ObjectList::isOnList(            // Is Object contained?
     Object*           object) const // -> Object
{
   Link* link= head;
   while( link != NULL )
   {
     if( link->getObject() == object )
       return TRUE;

     link= link->getNext();
   }

   return FALSE;
}

//----------------------------------------------------------------------------
//
// Method-
//       ObjectList::lifo
//
// Purpose-
//       Insert a link onto the list with LIFO ordering.
//
//----------------------------------------------------------------------------
void
   ObjectList::lifo(                // Insert (LIFO order)
     Object*           object)      // -> Object to insert
{
   Link* link= new Link(object);    // Create insert Link
   Link* head= this->head;          // Our head link

   link->setNext(head);             // Set next link pointer
   link->setPrev(NULL);             // Set prior link pointer

   if( head == NULL )               // If the list is empty
     tail= link;                    // Add link to empty list
   else                             // If the list is not empty
     head->setPrev(link);           // Add link to list

   this->head= link;                // Set new list head link
}

//----------------------------------------------------------------------------
//
// Method-
//       ObjectList::remove
//
// Purpose-
//       Remove a chain from the list.
//
// Implementation note-
//       Implementation useless unless Links are exposed.
//
//----------------------------------------------------------------------------
void
   ObjectList::remove(              // Remove from list
     Link*             head,        // -> First Link to remove
     Link*             tail)        // -> Final Link to remove
{
   Link* prev= head->getPrev();
   Link* next= tail->getNext();

   if( prev == NULL )
   {
     this->head= next;
     if( next != NULL )
       next->setPrev(NULL);
   }
   else
   {
     prev->setNext(next);
     head->setPrev(NULL);           // Remove dangling reference
   }

   if( next == NULL )
   {
     this->tail= prev;
     if( prev != NULL )
       prev->setNext(NULL);
   }
   else
   {
     next->setPrev(prev);
     tail->setNext(NULL);           // Remove dangling reference
   }

   // Can't delete links; they're the remove reference
}

//----------------------------------------------------------------------------
//
// Method-
//       ObjectList::remq
//
// Purpose-
//       Remove the oldest link from the list.
//
//----------------------------------------------------------------------------
Object*                             // -> Removed Object
   ObjectList::remq( void )         // Remove oldest link
{
   Object* result= NULL;

   Link* link= head;                // Address the first Link
   if( link != NULL )               // If the list is not empty
   {
     result= link->getObject();     // Get object (before link disappears)

     Link* head= link->getNext();   // Remove link from list
     this->head= head;              // :
     if( head != NULL )             // If the list is not empty
       head->setPrev(NULL);         // Set first link backchain pointer
     else                           // The list is empty
       tail= NULL;                  // No tail link exists

//   link->setNext(NULL);           // Remove dangling reference
     delete link;
   }

   return result;                   // Return the oldest object
}

//----------------------------------------------------------------------------
//
// Method-
//       ObjectList::reset
//
// Purpose-
//       Reset the List.
//
// Implementation note-
//       Note that all references to an ObjectList_Link must be removed for
//       a Link to be physically deleted.
//
//----------------------------------------------------------------------------
void
   ObjectList::reset( void )        // Reset (empty) the list
{
   Link* link= head;
   while( link != NULL )            // Reset each node on list
   {
     head= link->getNext();         // Remove the link from the head
//   link->reset();                 // Reset this link
     delete link;                   // Delete this link

     link= head;
   }

   tail= NULL;
}

