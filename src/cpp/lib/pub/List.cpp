//----------------------------------------------------------------------------
//
//       Copyright (C) 2007-2025 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
// SPDX-License-Identifier: GPL-3.0-only
//----------------------------------------------------------------------------
//
// Title-
//       List.cpp
//
// Purpose-
//       List object methods.
//
// Last change date-
//       2025/09/23
//
//----------------------------------------------------------------------------
#include <stdexcept>                // For std::invalid_argument

#include <pub/Debug.h>              // For namespace pub::debugging
#include "pub/List.h"               // For pub::List, implemented
#include <pub/utility.h>            // For pub::utility::checkstop

#define PUB _LIBPUB_NAMESPACE
using namespace PUB::debugging;     // For debugging methods

namespace _LIBPUB_NAMESPACE {       // The pub namespace
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
//       __detail::__end
//
// Purpose-
//       This CONST dummy end-of-list pseudo-link is the oldest link on every
//       AI_list with an active iterator. Newer elements point to it, but
//       its value is never referenced. It is removed from the AI_list when
//       incrementing the begin() iterator to equal the end() iterator.
//
//----------------------------------------------------------------------------
const void*            __detail::__end= nullptr;

//----------------------------------------------------------------------------
//
// Method-
//       AI_list<void>::iterator_begin_error
//
// Purpose-
//       Checkstop: begin but pseudo-link present
//
//----------------------------------------------------------------------------
[[noreturn]]
void
   AI_list<void>::iterator_begin_error( void ) // Duplicate begin
{
   utility::checkstop(__LINE__, __FILE__, "begin invoked but already active");
}

//----------------------------------------------------------------------------
//
// Method-
//       AI_list<void>::iterator_increment_error
//
// Purpose-
//       Checkstop: operator++ but pseudo-link missing
//
//----------------------------------------------------------------------------
[[noreturn]]
void
   AI_list<void>::iterator_increment_error( void ) // Pseudo-link missing
{
   utility::checkstop(__LINE__, __FILE__, "AI_iter++ but pseudo-link missing");
}

//----------------------------------------------------------------------------
//
// Method-
//       AI_list<void>::verify_nullptr
//
// Purpose-
//       Invoked by destructor to verify that the list is empty.
//
// Implementation notes-
//       If the AI_list isn't empty when its destructor is invoked, an AI_iter
//       for the list exists. That iterator is still running under control of
//       some other thread, and it's going to access this list. This situation
//       must be prevented.
//
//----------------------------------------------------------------------------
void
   AI_list<void>::verify_nullptr(   // Insure that no link exists
     void*             link)        // The current _tail
{
   if( link == nullptr )
     return;

   utility::checkstop(__LINE__, __FILE__, "~AI_list invoked while active");
}

//----------------------------------------------------------------------------
//
// Method-
//       DHDL_list<void>::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   DHDL_list<void>::debug(          // Debugging display
     const char*       info) const  // Caller information
{
   debugf("DHDL_list<void>::debug(%s)\n", info);

   size_t index= 0;
   for(_Link* link= _head; link; link= link->_next) {
     debugf("[%2zd] prev(%p) <- this(%p) -> next(%p)\n", index++
           , link->_prev, link, link->_next);
   }
}

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
     _Link*            link,        // -> _Link to insert after
     _Link*            head,        // -> First _Link to insert
     _Link*            tail)        // -> Final _Link to insert
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
     // Inverted/Invalid argument checks
     if( link->_prev == nullptr && this->_head != link )
       throw std::invalid_argument("Inverted argument list");
     if( link->_next == nullptr && this->_tail != link )
       throw std::invalid_argument("Inconsistent List and Link arguments");

     // Insert the link
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
     _Link*            link) const  // -> _Link
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
     _Link*            link)        // -> _Link to insert
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
     _Link*            head,        // -> First _Link to remove
     _Link*            tail)        // -> Final _Link to remove
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
//       DHDL_list<void>::size
//
// Purpose-
//       Count the _Links
//
//----------------------------------------------------------------------------
size_t                              // The number of links
   DHDL_list<void>::size( void ) const // Get the _Link count
{
   size_t length= 0;
   for(_Link* link= _head; link; link= link->_next)
     ++length;

   return length;
}

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
     _Link*            link)        // -> _Link to insert
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
     _Link*            link,        // -> _Link to insert after
     _Link*            head,        // -> First _Link to insert
     _Link*            tail)        // -> Final _Link to insert
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
     _Link*            link) const  // -> _Link
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
     _Link*            link)        // -> _Link to insert
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
     _Link*            head,        // -> First _Link to remove
     _Link*            tail)        // -> Last _Link to remove
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
     _Link*            link)        // -> _Link to insert
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
     _Link*            link,        // -> _Link to insert after
     _Link*            tail,        // -> First _Link to insert
     _Link*            head)        // -> Final _Link to insert
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
     _Link*            link) const  // -> _Link
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
     _Link*            link)        // -> _Link to insert
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
     _Link*            head,        // -> First _Link to remove
     _Link*            tail)        // -> Last _Link to remove
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

//----------------------------------------------------------------------------
//
// Method-
//       SORT_list<void>::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   SORT_list<void>::debug(          // Debugging display
     const char*       info) const  // Caller information
{
   debugf("SORT_list<void>::debug(%s)\n", info);

   size_t index= 0;
   for(_Link* link= _head; link; link= link->_next) {
     debugf("[%2zd] prev(%p) <- this(%p) -> next(%p)\n", index++
           , link->_prev, link, link->_next);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       SORT_list<void>::fifo
//
// Purpose-
//       Insert a _Link onto the list with FIFO ordering.
//
//----------------------------------------------------------------------------
void
   SORT_list<void>::fifo(           // Insert _Link, FIFO order
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
//       SORT_list<void>::insert
//
// Purpose-
//       Insert a chain onto the list at the specified position.
//
//----------------------------------------------------------------------------
void
   SORT_list<void>::insert(         // Add to SORT_list<void> at position,
     _Link*            link,        // -> _Link to insert after
     _Link*            head,        // -> First _Link to insert
     _Link*            tail)        // -> Final _Link to insert
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
     // Inverted/Invalid argument checks
     if( link->_prev == nullptr && this->_head != link )
       throw std::invalid_argument("Inverted argument list");
     if( link->_next == nullptr && this->_tail != link )
       throw std::invalid_argument("Inconsistent List and Link arguments");

     // Insert the link
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
//       SORT_list<void>::is_coherent
//
// Purpose-
//       Check the object for coherency.
//
//----------------------------------------------------------------------------
bool                                // TRUE if object is coherent
   SORT_list<void>::is_coherent( void ) const // Coherency check
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
//       SORT_list<void>::is_on_list
//
// Purpose-
//       Check whether a _Link is on the list.
//
//----------------------------------------------------------------------------
bool                                // TRUE if _Link is in list
   SORT_list<void>::is_on_list(     // Is _Link contained?
     _Link*            link) const  // -> _Link
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
//       SORT_list<void>::lifo
//
// Purpose-
//       Insert a _Link onto the list with LIFO ordering.
//
//----------------------------------------------------------------------------
void
   SORT_list<void>::lifo(           // Insert _Link, LIFO order
     _Link*            link)        // -> _Link to insert
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
//       SORT_list<void>::remove
//
// Purpose-
//       Remove a chain from the list. The removed chain is not changed.
//
//----------------------------------------------------------------------------
void
   SORT_list<void>::remove(         // Remove from list
     _Link*            head,        // -> First _Link to remove
     _Link*            tail)        // -> Final _Link to remove
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
//       SORT_list<void>::remq
//
// Purpose-
//       Remove oldest _Link from the list.
//
//----------------------------------------------------------------------------
SORT_list<void>::_Link*             // -> Removed _Link
   SORT_list<void>::remq( void )    // Remove oldest _Link
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
//       SORT_list<void>::reset
//
// Purpose-
//       Reset (empty) the list.
//
//----------------------------------------------------------------------------
SORT_list<void>::_Link*             // The set of removed _Links
   SORT_list<void>::reset( void )   // Reset (empty) the List
{
   _Link* link= _head;              // Resultant

   _head= nullptr;
   _tail= nullptr;

   return link;
}

//----------------------------------------------------------------------------
//
// Method-
//       SORT_list<void>::size
//
// Purpose-
//       Count the _Links
//
//----------------------------------------------------------------------------
size_t                              // The number of links
   SORT_list<void>::size( void ) const // Get the _Link count
{
   size_t length= 0;
   for(_Link* link= _head; link; link= link->_next)
     ++length;

   return length;
}

//----------------------------------------------------------------------------
//
// Method-
//       SORT_list<void>::sort
//
// Purpose-
//       Sort the list (using Merge sort algorithm.)
//
//----------------------------------------------------------------------------
void
   SORT_list<void>::sort( void )    // Sort the list
{
   size_t length= size();           // Get the List length
   if( length < 2 )                 // Empty and single element lists
     return;                        // are already sorted

   // Merge sort
   for(size_t width= 1; width < length; width <<= 1) {
     _Link* next= _head;            // Working split position _Link

     // The first split/merge sets the initial head and tail
     _Link* L= sort_split(next, width); // The left _Link set
     _Link* R= sort_split(next, width); // The right _Link set
     auto [head, tail]= sort_merge(L, R); // Merge the sets

     // Go through the entire list at the current width
     while( next ) {                // While links remain in list
       L= sort_split(next, width); // The left _Link set
       R= sort_split(next, width); // The right _Link set
       auto [merged_head, merged_tail]= sort_merge(L, R); // Merge the sets

       tail->_next= merged_head;
       merged_head->_prev= tail;
       tail= merged_tail;
     }

     head->_prev= nullptr;
     _head= head;
     tail->_next= nullptr;
     _tail= tail;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       SORT_list<void>::sort_split
//
// Purpose-
//       Split the list into parts.
//
// Implementation notes-
//       Field head->_prev is unchanged, but next->_prev and tail->_next are
//       both set to nullptr
//
//----------------------------------------------------------------------------
SORT_list<void>::_Link*             // The set of removed _Links (left)
   SORT_list<void>::sort_split(     // Split the _Link set
     _Link*&           next,        // INP: The head element
                                    // OUT: The remaining _Links (remainder)
     size_t            N)           // The number of _Links to remove
{
   if( !next )                      // If nothing to split
     return nullptr;

   _Link* head= next;
   _Link* tail= next;

   for (size_t i= 1; i < N; ++i) {
     tail= tail->_next;
     if( tail == nullptr ) {        // Fewer than N _Links remain
       next= nullptr;
       return head;
     }
   }

   _Link* rest= tail->_next;        // Split after tail
   if( rest ) {
     rest->_prev= nullptr;
     tail->_next= nullptr;
   }

   next= rest;                      // Remainder after the split
   return head;                     // The run of length <= N
}

//----------------------------------------------------------------------------
//
// Method-
//       SORT_list<void>::sort_merge
//
// Purpose-
//       Merge the parts into one sorted _Link set.
//
//----------------------------------------------------------------------------
std::pair<SORT_list<void>::_Link*,SORT_list<void>::_Link*> // The combined set
   SORT_list<void>::sort_merge(     // Merge
     _Link*            L,           // The Left (lower) _Link set
     _Link*            R)           // The Right (remaining) _Link set
{
   enum                             // Current state
   { IN_L= -1                       // In LEFT state
   , INIT= 0                        // in INITIAL state
   , IN_R= +1                       // In RIGHT state
   } fsm= INIT;                     // Initial state

   _Link* head;
   _Link* tail;
   // Merge the first _Link
   if( R == nullptr || *L < *R ) {
     head= tail= L;
     L= L->_next;
     fsm= IN_L;
   } else {
     head= tail= R;
     R= R->_next;
     fsm= IN_R;
   }
   head->_prev= nullptr;

   // While both left and right links exist, merge
   // If we're not switching lists, we don't need to adjust any link
   while (L && R) {
     if( *L < *R ) {
       if( fsm != IN_L ) {
         tail->_next= L;
         L->_prev= tail;
         fsm= IN_L;
       }
       tail= L;
       L= L->_next;
     } else {
       if( fsm != IN_R ) {
         tail->_next= R;
         R->_prev= tail;
         fsm= IN_R;
       }
       tail= R;
       R= R->_next;
     }
   }

   // The remainder of the LEFT or RIGHT Link set remains
   // (For a list of size 2, neither Link set remains.)
   if( L ) {                        // If the LEFT Link set remains
     if( fsm == IN_R ) {            // It's possible that fsm == IN_L
       tail->_next= L;
       L->_prev= tail;
     }
     while( L ) {
       tail= L;
       L= L->_next;
     }
   } else if( R ) {                 // If the RIGHT Link set remains
     tail->_next= R;
     R->_prev= tail;
     while( R ) {
       tail= R;
       R= R->_next;
     }
   }

   return {head, tail};
}
} // namespace _LIBPUB_NAMESPACE
