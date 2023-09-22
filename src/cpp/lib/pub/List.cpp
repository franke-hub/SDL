//----------------------------------------------------------------------------
//
//       Copyright (C) 2007-2023 Frank Eskesen.
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
//       List object methods.
//
// Last change date-
//       2023/09/21
//
//----------------------------------------------------------------------------
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <pub/Debug.h>              // For namespace pub::debugging
#include "pub/List.h"               // For pub::List, implemented

using namespace _LIBPUB_NAMESPACE::debugging; // Debugging functions

namespace _LIBPUB_NAMESPACE {       // The fileman namespace
//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{ HCDM= false                       // Hard Core Debug Mode?
, MAX_COHERENT= __detail::MAX_COHERENT // Maximum coherent List element count
}; // generic enum

//----------------------------------------------------------------------------
//
// Global-
//       __detail::_PREV_link __end
//
// Purpose-
//       This dummy end-of-list pseudo-link is the oldest link on every
//       AI_list with an active iterator. Newer elements point to it, but
//       its value is never referenced. It is removed from the AI_list when
//       incrementing the begin() iterator to equal the end() iterator.
//
//----------------------------------------------------------------------------
const void*            __detail::__end= nullptr;

//----------------------------------------------------------------------------
//
// Method-
//       DHDL_list<void>::fifo
//
// Purpose-
//       Insert a _Link onto the list with FIFO ordering.
//
//----------------------------------------------------------------------------
void
   DHDL_list<void>::fifo(           // Insert _Link, FIFO order
     _Link*            link)        // -> _Link to insert
{
   link->_next= nullptr;            // Set next _Link pointer
   link->_prev= _tail;              // Set prior _Link pointer

   if( _head == nullptr )           // If the list is empty
     _head= link;                   // Add _Link to empty list
   else                             // If the list is not empty
     _tail->_next= link;            // Add _Link to list

   _tail= link;                     // Set new list tail _Link
}

//----------------------------------------------------------------------------
//
// Method-
//       DHDL_list<void>::insert
//
// Purpose-
//       Insert a chain onto the list at the specified position.
//
//----------------------------------------------------------------------------
void
   DHDL_list<void>::insert(         // Add to DHDL_list<void> at position,
     _Link*             link,       // -> _Link to insert after
     _Link*             head,       // -> First _Link to insert
     _Link*             tail)       // -> Final _Link to insert
{
   if( link == nullptr )            // If insert at head
   {
     head->_prev= nullptr;
     if( this->_head == nullptr )   // If the list is empty
     {
       tail->_next= nullptr;
       this->_tail= tail;
       this->_head= head;
     }
     else                           // If the list is populated
     {
       tail->_next= this->_head;
       this->_head->_prev= tail;
       this->_head= head;
     }
   }
   else                             // If the list is not empty
   {
     _Link* next= link->_next;      // Address the next _Link
     tail->_next= next;             // Set the forward _Link pointer
     head->_prev= link;             // Set the reverse _Link pointer

     link->_next= head;             // Insert onto the forward list
     if( next == nullptr )          // Insert onto the reverse list
       this->_tail= tail;
     else
       next->_prev= tail;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       DHDL_list<void>::is_coherent
//
// Purpose-
//       Check the object for coherency.
//
//----------------------------------------------------------------------------
bool                                // TRUE if object is coherent
   DHDL_list<void>::is_coherent( void ) const // Coherency check
{
   if( _head == nullptr )           // If the list is empty
   {
     if( _tail != nullptr )         // If _tail is not nullptr
       return false;

     return true;                   // _head == _tail == nullptr
   }

   _Link* link= _head;              // Pointer to current _Link
   _Link* prev= nullptr;            // Pointer to prior   _Link
   for(int count= 0;;count++)
   {
     if( link->_prev != prev )
       return false;

     if( link->_next == nullptr )
       break;

     if( link == _tail )
       return false;

     prev= link;
     link= link->_next;

     if( count > MAX_COHERENT )
       return false;
   }

   if( _tail != link )
     return false;

   return true;
}

//----------------------------------------------------------------------------
//
// Method-
//       DHDL_list<void>::is_on_list
//
// Purpose-
//       Check whether a _Link is on the list.
//
//----------------------------------------------------------------------------
bool                                // TRUE if _Link is in list
   DHDL_list<void>::is_on_list(     // Is _Link contained?
     _Link*             link) const // -> _Link
{
   if( link != nullptr )            // If a _Link was specified
   {
     _Link* next= _head;
     while( next != nullptr )
     {
       if( next == link )
         return true;

       next= next->_next;
     }
   }

   return false;
}

//----------------------------------------------------------------------------
//
// Method-
//       DHDL_list<void>::lifo
//
// Purpose-
//       Insert a _Link onto the list with LIFO ordering.
//
//----------------------------------------------------------------------------
void
   DHDL_list<void>::lifo(           // Insert _Link, LIFO order
     _Link*             link)       // -> _Link to insert
{
   link->_next= _head;              // Set next _Link pointer
   link->_prev= nullptr;            // Set prior _Link pointer

   if( _head == nullptr )           // If the list is empty
     _tail= link;                   // Add _Link to empty list
   else                             // If the list is not empty
     _head->_prev= link;            // Add _Link to list

   _head= link;                     // Set new list head _Link
}

//----------------------------------------------------------------------------
//
// Method-
//       DHDL_list<void>::remove
//
// Purpose-
//       Remove a chain from the list. The removed chain is not changed.
//
//----------------------------------------------------------------------------
void
   DHDL_list<void>::remove(         // Remove from list
     _Link*             head,       // -> First _Link to remove
     _Link*             tail)       // -> Final _Link to remove
{
   _Link* prev= head->_prev;        // _Link prior to head
   _Link* next= tail->_next;        // _Link after tail

   if( prev == nullptr )
   {
     this->_head= next;
     if( next != nullptr )
       next->_prev= nullptr;
   }
   else
   {
     prev->_next= next;
//// head->_prev= nullptr;          // Not necessary (DO NOT REMOVE COMMENT)
   }

   if( next == nullptr )
   {
     this->_tail= prev;
     if( prev != nullptr )
       prev->_next= nullptr;
   }
   else
   {
     next->_prev= prev;
//// tail->_next= nullptr;          // Not necessary (DO NOT REMOVE COMMENT)
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       DHDL_list<void>::remq
//
// Purpose-
//       Remove oldest _Link from the list.
//
//----------------------------------------------------------------------------
DHDL_list<void>::_Link*             // -> Removed _Link
   DHDL_list<void>::remq( void )    // Remove oldest _Link
{
   _Link* link= _head;              // Address the first _Link
   if( link != nullptr )            // If the list is not empty
   {
     _head= link->_next;            // Remove _Link from list
     if( _head != nullptr )         // If the list is not empty
       _head->_prev= nullptr;       // Set first _Link backchain pointer
     else                           // The list is empty
       _tail= nullptr;              // No _tail _Link exists
   }

   return link;                     // Return the oldest _Link
}

//----------------------------------------------------------------------------
//
// Method-
//       DHDL_list<void>::reset
//
// Purpose-
//       Reset (empty) the list.
//
//----------------------------------------------------------------------------
DHDL_list<void>::_Link*             // The set of removed _Links
   DHDL_list<void>::reset( void )   // Reset (empty) the List
{
   _Link* link= _head;              // Resultant

   _head= nullptr;
   _tail= nullptr;

   return link;
}

//----------------------------------------------------------------------------
//
// Method-
//       DHDL_list<void>::sort
//
// Purpose-
//       Sort the list.
//
// Implementation notes-
//       TODO: Use a better algorithm than bubble sort.
//
//----------------------------------------------------------------------------
#if USE_BASE_SORT
void
   DHDL_list<void>::sort(           // Sort the list
     const _Comparator cmp)         // Using this Comparitor
{
   _Link* head= reset();            // The original head of the list

   while( head )                    // Sort the list
   {
     _Link* low= head;
     _Link* next= low->_next;
     while( next != nullptr )
     {
       if( cmp(next, low) )
         low= next;

       next= next->_next;
     }

     if( low == head )
       head= head->_next;
     else
     {
       if( low->_next != nullptr )
         low->_next->_prev= low->_prev;
       low->_prev->_next= low->_next;
     }

     fifo(low);
   }
}
#endif

//----------------------------------------------------------------------------
//
// Method-
//       DHSL_list<void>::fifo
//
// Purpose-
//       Insert a _Link onto the list with FIFO ordering.
//
//----------------------------------------------------------------------------
void
   DHSL_list<void>::fifo(           // Insert _Link, FIFO order
     _Link*             link)       // -> _Link to insert
{
   link->_next= nullptr;            // Set _Link chain pointer

   if( _head != nullptr )           // If adding to existing list
     _tail->_next= link;            // Insert the _Link onto the list
   else                             // If adding to empty list
     _head= link;                   // Add _Link to list

   _tail= link;
}

//----------------------------------------------------------------------------
//
// Method-
//       DHSL_list<void>::insert
//
// Purpose-
//       Insert a chain onto the list at the specified position.
//
//----------------------------------------------------------------------------
void
   DHSL_list<void>::insert(         // Add to list at position,
     _Link*             link,       // -> _Link to insert after
     _Link*             head,       // -> First _Link to insert
     _Link*             tail)       // -> Final _Link to insert
{
   if( link == nullptr )            // If insert at head
   {
     if( this->_head == nullptr )   // If the list is empty
     {
       tail->_next= nullptr;
       this->_head= head;
       this->_tail= tail;
     }
     else                           // If the list is populated
     {
       tail->_next= this->_head;
       this->_head= head;
     }
   }
   else
   {
     tail->_next= link->_next;      // Set _Link chain pointers
     link->_next= head;             // Insert onto the forward list

     if( tail->_next == nullptr )   // If insert after last _Link
       this->_tail= tail;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       DHSL_list::is_coherent
//
// Purpose-
//       Check the object for coherency.
//
//----------------------------------------------------------------------------
bool                                // TRUE if object is coherent
   DHSL_list<void>::is_coherent( void ) const // Coherency check
{
   _Link* prev= _head;
   if( prev != nullptr )
   {
     for(int count= 0;;count++)
     {
       _Link* link= prev->_next;
       if( link == nullptr )
         break;
       if( prev == _tail || count > MAX_COHERENT )
         return false;

       prev= link;
     }
   }

   return (prev == _tail);
}

//----------------------------------------------------------------------------
//
// Method-
//       DHSL_list<void>::is_on_list
//
// Purpose-
//       Check whether a _Link is on the list.
//
//----------------------------------------------------------------------------
bool                                // TRUE if _Link is in list
   DHSL_list<void>::is_on_list(     // Is _Link contained?
     _Link*             link) const // -> _Link
{
   if( link != nullptr )            // If a _Link was specified
   {
     _Link* next= _head;
     while( next != nullptr )
     {
       if( next == link )
         return true;

       next= next->_next;
     }
   }

   return false;
}

//----------------------------------------------------------------------------
//
// Method-
//       DHSL_list<void>::lifo
//
// Purpose-
//       Add to list in LIFO order.
//
//----------------------------------------------------------------------------
void
   DHSL_list<void>::lifo(           // Insert _Link, LIFO order
     _Link*             link)       // -> _Link to insert
{
   link->_next= _head;              // Set _Link chain pointer

   if( _head == nullptr )           // If adding to empty list
     _tail= link;                   // Add to empty list

   _head= link;                     // Add _Link to list
}

//----------------------------------------------------------------------------
//
// Method-
//       DHSL_list<void>::remove
//
// Purpose-
//       Remove a chain of elements from the DHSL_list.
//
//----------------------------------------------------------------------------
void
   DHSL_list<void>::remove(         // Remove specific _Links
     _Link*             head,       // -> First _Link to remove
     _Link*             tail)       // -> Last _Link to remove
{
   _Link* link= this->_head;        // Address oldest _Link
   if( link == nullptr )            // If the list is empty
     return;                        // Exit, function complete

   if( link == head )               // If removing head element
   {
     this->_head= tail->_next;
     if( this->_head == nullptr )   // If the list is now empty
       this->_tail= nullptr;        // Make it completely empty
     return;
   }

   for(;;)                          // Search for prior element
   {
     if( link->_next == head )
       break;

     link= link->_next;
     if( link == nullptr )
       return;
   }

   link->_next= tail->_next;        // Remove from within list
   if( link->_next == nullptr )
     this->_tail= link;
}

//----------------------------------------------------------------------------
//
// Method-
//       DHSL_list<void>::remq
//
// Purpose-
//       Remove oldest _Link from a DHSL_list.
//
//----------------------------------------------------------------------------
DHSL_list<void>::_Link*             // -> Removed _Link
   DHSL_list<void>::remq( void )    // Remove oldest _Link
{
   _Link* link= _head;              // Address oldest _Link
   if( link != nullptr )            // If the list is empty
   {
     _head= link->_next;            // Remove _Link from list
     if( _head == nullptr )         // If the list is now empty
       _tail= nullptr;              // Make it completely empty
   }

   return link;                     // Return the oldest _Link
}

//----------------------------------------------------------------------------
//
// Method-
//       DHSL_list<void>::reset
//
// Purpose-
//       Reset (empty) the List.
//
//----------------------------------------------------------------------------
DHSL_list<void>::_Link*             // The set of removed _Links
   DHSL_list<void>::reset( void )   // Reset (empty) the List
{
   _Link* link= _head;

   _head= nullptr;
   _tail= nullptr;

   return link;
}

//----------------------------------------------------------------------------
//
// Method-
//       SHSL_list<void>::fifo
//
// Purpose-
//       Insert a _Link onto the list with FIFO ordering.
//
//----------------------------------------------------------------------------
void
   SHSL_list<void>::fifo(           // Insert _Link, FIFO order
     _Link*             link)       // -> _Link to insert
{
   link->_prev= nullptr;            // Set _Link chain pointer

   _Link* last= _tail;              // Address first _Link on list
   if( last == nullptr )            // If adding to empty list
   {
     _tail= link;                   // Add _Link to list
     return;                        // Exit, function complete
   }

   while( last->_prev != nullptr )  // Find the (tail) end of the list
     last= last->_prev;

   last->_prev= link;               // Add _Link to list
}

//----------------------------------------------------------------------------
//
// Method-
//       SHSL_list<void>::insert
//
// Purpose-
//       Insert a chain onto the list at the specified position.
//
//----------------------------------------------------------------------------
void
   SHSL_list<void>::insert(         // Add to list at position,
     _Link*             link,       // -> _Link to insert after
     _Link*             tail,       // -> First _Link to insert
     _Link*             head)       // -> Final _Link to insert
{
   if( link ) {
     head->_prev= link->_prev;
     link->_prev= tail;
   } else {
     head->_prev= this->_tail;
     this->_tail= tail;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       SHSL_list<void>::is_coherent
//
// Purpose-
//       Check whether this list is coherent.
//
//----------------------------------------------------------------------------
bool                                // TRUE if the object is coherent
   SHSL_list<void>::is_coherent( void ) const // Is this list coherent?
{
   _Link* link= _tail;
   for(int count= 0;;count++)
   {
     if( link == nullptr )
       break;
     if( count > MAX_COHERENT )
       return false;

     link= link->_prev;
   }

   return true;
}

//----------------------------------------------------------------------------
//
// Method-
//       SHSL_list<void>::is_on_list
//
// Purpose-
//       Check whether a _Link is on the list.
//
//----------------------------------------------------------------------------
bool                                // TRUE if _Link is in list
   SHSL_list<void>::is_on_list(     // Is _Link contained?
     _Link*             link) const // -> _Link
{
   if( link != nullptr )            // If a _Link was specified
   {
     _Link* next= _tail;
     while( next != nullptr )
     {
       if( next == link )
         return true;

       next= next->_prev;
     }
   }

   return false;
}

//----------------------------------------------------------------------------
//
// Method-
//       SHSL_list<void>::lifo
//
// Purpose-
//       Insert a _Link onto the list with LIFO ordering.
//
//----------------------------------------------------------------------------
void
   SHSL_list<void>::lifo(           // Insert _Link, LIFO order
     _Link*             link)       // -> _Link to insert
{
   link->_prev= _tail;              // Set _Link chain pointer
   _tail= link;                     // Add _Link to list
}

//----------------------------------------------------------------------------
//
// Method-
//       SHSL_list<void>::remove
//
// Purpose-
//       Remove a chain of elements from the SHSL_list
//
//----------------------------------------------------------------------------
void
   SHSL_list<void>::remove(         // Remove a chain of elements
     _Link*             head,       // -> First _Link to remove
     _Link*             tail)       // -> Last _Link to remove
{
   _Link* link= this->_tail;        // Address tail _Link
   if( link == nullptr )            // If the list is empty
     return;                        // Exit, function complete

   if( link == head )               // If removing tail element
   {
     this->_tail= tail->_prev;
     return;
   }

   for(;;)                          // Search for prior element
   {
     if( link->_prev == head )
       break;

     link= link->_prev;
     if( link == nullptr )          // IGNORE: Error if head not on List
       return;
   }

   link->_prev= tail->_prev;        // Remove from within list
}

//----------------------------------------------------------------------------
//
// Method-
//       SHSL_list<void>::remq
//
// Purpose-
//       Remove tail _Link from the list.
//
//----------------------------------------------------------------------------
SHSL_list<void>::_Link*             // -> Removed _Link
   SHSL_list<void>::remq( void )    // Remove tail _Link
{
   _Link* link= _tail;              // Address tail _Link
   if( link != nullptr )            // If the list is not empty
     _tail= link->_prev;            // Remove _Link from list

   return link;                     // Return the tail _Link
}

//----------------------------------------------------------------------------
//
// Method-
//       SHSL_list<void>::reset
//
// Purpose-
//       Reset (empty) the list.
//
//----------------------------------------------------------------------------
SHSL_list<void>::_Link*             // The set of removed _Links
   SHSL_list<void>::reset( void )   // Reset (empty) the list
{
   _Link* link= _tail;

   _tail= nullptr;

   return link;
}
} // namespace _LIBPUB_NAMESPACE
