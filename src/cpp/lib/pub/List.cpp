//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2020 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
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
//       2020/12/28
//
// Implementation notes-
//       See Dispatch.cpp, Task::drain for sample AU_FIFO usage.
//
//----------------------------------------------------------------------------
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "pub/List.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define MAX_COHERENT     1000000000 // Maximum coherent element count

#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef SCDM
#undef  SCDM                        // If defined, Soft Core Debug Mode
#endif

namespace _PUB_NAMESPACE {
//----------------------------------------------------------------------------
//
// Method-
//       AU_List<void>::fifo
//
// Purpose-
//       Add to list in FIFO order.
//
//----------------------------------------------------------------------------
AU_List<void>::Link*                // -> Prior tail
   AU_List<void>::fifo(             // Insert FIFO
     Link*             link)        // -> Link to insert
{
   Link*               prev;        // Prior tail value

   prev= tail.load();               // The current tail
   link->prev= prev;
   while( !tail.compare_exchange_weak(prev, link) ) // Add link to list
     link->prev= prev;

   return prev;                     // Return the previous tail
}

//----------------------------------------------------------------------------
//
// Method-
//       AU_List<void>::is_coherent
//
// Purpose-
//       Check the object for coherency.
//
//----------------------------------------------------------------------------
int                                 // TRUE if object is coherent
   AU_List<void>::is_coherent( void ) const // Coherency check
{
   Link* link= tail.load();         // Address newest Link
   for(int count= 0; count < MAX_COHERENT; count++)
   {
     if( link == nullptr )
       return true;

     link= link->prev;
   }

   return false;
}

//----------------------------------------------------------------------------
//
// Method-
//       AU_List<void>::is_on_list
//
// Purpose-
//       Check whether a Link is on the List.
//
//----------------------------------------------------------------------------
int                                 // TRUE if Link is on List
   AU_List<void>::is_on_list(       // Is Link contained?
     Link*             link) const  // -> Link
{
   if( link != nullptr )            // If a link was specified
   {
     Link* prev= tail.load();       // Get newest link
     while( prev != nullptr )
     {
       if( prev == link )
         return true;

       prev= prev->prev;
     }
   }

   return false;
}

//----------------------------------------------------------------------------
//
// Method-
//       AU_List<void>::remove
//
// Purpose-
//       Remove Link from List.
//
//----------------------------------------------------------------------------
AU_List<void>::Link*                // The removed link (if on list)
   AU_List<void>::remove(           // Remove Link
     Link*             item)        // The link to remove
{
   Link*               link;        // -> Link
   Link*               prev;        // -> Prior Link

   for(;;)                          // Handle first element or empty list
   {
     link= tail.load();             // Get newest Link
     if( link == nullptr )          // If the List was empty
       return nullptr;              // Exit, link not found

     if( link != item )             // If not the first Link
       break;

     if( tail.compare_exchange_strong(link, link->prev) )
       return link;
   }

   // The remove link is not the first (newest) one on the List
   do                               // Find the prior link
   {
     prev= link;                    // Save the prior Link pointer
     link= link->prev;              // Address the next older Link
     if( link == item )             // If we found the link
     {
       prev->prev= link->prev;      // Remove the found link
       break;
     }
   } while( link != nullptr );      // Until we run out of links

   return link;                     // Return the removed link, if present
}

//----------------------------------------------------------------------------
//
// Method-
//       AU_List<void>::remq
//
// Purpose-
//       Remove oldest Link from List.
//
// Implementation note-
//       The link to be removed is the last one on the list. The entire list
//       needs to be examined whenever an element is removed. Consider using
//       the swap method and an AU_FIFO to reduce this overhead.
//       See: Dispatch.cpp, Task::drain for sample usage.
//
//----------------------------------------------------------------------------
AU_List<void>::Link*                // -> Oldest Link
   AU_List<void>::remq( void )      // Remove oldest Link
{
   Link*               link;        // -> Link
   Link*               prev;        // -> Prior Link

   for(;;)                          // Handle List with zero or one elements
   {
     link= tail.load();             // Address newest Link
     if( link == nullptr )          // If the List was empty
       return nullptr;              // Exit, function complete

     if( link->prev != nullptr )    // If this is not the only Link
       break;

     if( tail.compare_exchange_strong(link, nullptr) )
       return link;
   }

   // We have more than one Link on the List
   // Since we OWN the list, we can just find and remove the oldest link
   do                               // Find the oldest Link
   {
     prev= link;                    // Save the prior Link pointer
     link= link->prev;              // Address the next older Link
   } while( link->prev != nullptr ); // Until we find the oldest

   prev->prev= nullptr;             // Remove the oldest link from the list
   return link;                     // Return the oldest link
}

//----------------------------------------------------------------------------
//
// Method-
//       AU_List<void>::reset
//
// Purpose-
//       Reset (empty) the AU_List.
//
//----------------------------------------------------------------------------
AU_List<void>::Link*                // -> Removed (newest) Link
   AU_List<void>::reset( void )     // Reset (empty) the List
{
   Link*               link;        // -> Link

   link= tail.load();               // Get the newest link
   while( ! tail.compare_exchange_weak(link, nullptr) )
     ;

   return link;                     // Return the newest link
}

//----------------------------------------------------------------------------
//
// Method-
//       AU_List<void>::swap
//
// Purpose-
//       (Atomically and optionally) replace the List with a dummy Link.
//
//----------------------------------------------------------------------------
AU_List<void>::Link*                // -> Removed (newest) Link
   AU_List<void>::swap(             // Swap (empty) the List
     Link*             replace)     // With this replacement Link
{
   Link* link= tail.load();         // Get the current tail
   if( link == nullptr )            // If the List is currently empty
     return nullptr;                // Do not replace it

   // Attempt replacement with empty List
   while( link == replace ) {
     if( tail.compare_exchange_weak(link, nullptr) )
       return nullptr;
   }

   // Replace the List
   replace->prev= nullptr;
   while( ! tail.compare_exchange_weak(link, replace) )
     ;

   return link;                     // Return the newest link
}

//----------------------------------------------------------------------------
//
// Method-
//       DHDL_List<void>::fifo
//
// Purpose-
//       Insert a link onto the list with FIFO ordering.
//
//----------------------------------------------------------------------------
void
   DHDL_List<void>::fifo(           // Insert link, FIFO order
     Link*             link)        // -> Link to insert
{
   link->next= nullptr;             // Set next link pointer
   link->prev= tail;                // Set prior link pointer

   if( head == nullptr )            // If the list is empty
     head= link;                    // Add link to empty list
   else                             // If the list is not empty
     tail->next= link;              // Add link to list

   tail= link;                      // Set new list tail link
}

//----------------------------------------------------------------------------
//
// Method-
//       DHDL_List<void>::insert
//
// Purpose-
//       Insert a chain onto the list at the specified position.
//
//----------------------------------------------------------------------------
void
   DHDL_List<void>::insert(         // Add to DHDL_List<void> at position,
     Link*             link,        // -> Link to insert after
     Link*             head,        // -> First Link to insert
     Link*             tail)        // -> Final Link to insert
{
   if( link == nullptr )            // If insert at head
   {
     head->prev= nullptr;
     if( this->head == nullptr )    // If the list is empty
     {
       tail->next= nullptr;
       this->tail= tail;
       this->head= head;
     }
     else                           // If the list is populated
     {
       tail->next= this->head;
       this->head->prev= tail;
       this->head= head;
     }
   }
   else                             // If the list is not empty
   {
     Link* next= link->next;        // Address the next Link
     tail->next= next;              // Set the forward link pointer
     head->prev= link;              // Set the reverse link pointer

     link->next= head;              // Insert onto the forward list
     if( next == nullptr )          // Insert onto the reverse list
       this->tail= tail;
     else
       next->prev= tail;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       DHDL_List<void>::is_coherent
//
// Purpose-
//       Check the object for coherency.
//
//----------------------------------------------------------------------------
int                                 // TRUE if object is coherent
   DHDL_List<void>::is_coherent( void ) const // Coherency check
{
   Link*               link;        // Pointer to current link
   Link*               prev;        // Pointer to prior   link

   if( head == nullptr )            // If the list is empty
   {
     if( tail != nullptr )          // If the tail is not nullptr
       return false;

     return true;                   // head == tail == nullptr
   }

   link= head;
   prev= nullptr;
   for(int count= 0;;count++)
   {
     if( link->prev != prev )
       return false;

     if( link->next == nullptr )
       break;

     if( link == tail )
       return false;

     prev= link;
     link= link->next;

     if( count > MAX_COHERENT )
       return false;
   }

   if( tail != link )
     return false;

   return true;
}

//----------------------------------------------------------------------------
//
// Method-
//       DHDL_List<void>::is_on_list
//
// Purpose-
//       Check whether a Link is on the list.
//
//----------------------------------------------------------------------------
int                                 // TRUE if link is in list
   DHDL_List<void>::is_on_list(     // Is Link contained?
     Link*             link) const  // -> Link
{
   if( link != nullptr )            // If a Link was specified
   {
     Link* next= head;
     while( next != nullptr )
     {
       if( next == link )
         return true;

       next= next->next;
     }
   }

   return false;
}

//----------------------------------------------------------------------------
//
// Method-
//       DHDL_List<void>::lifo
//
// Purpose-
//       Insert a link onto the list with LIFO ordering.
//
//----------------------------------------------------------------------------
void
   DHDL_List<void>::lifo(           // Insert link, LIFO order
     Link*             link)        // -> Link to insert
{
   link->next= head;                // Set next link pointer
   link->prev= nullptr;             // Set prior link pointer

   if( head == nullptr )            // If the list is empty
     tail= link;                    // Add link to empty list
   else                             // If the list is not empty
     head->prev= link;              // Add link to list

   head= link;                      // Set new list head link
}

//----------------------------------------------------------------------------
//
// Method-
//       DHDL_List<void>::remove
//
// Purpose-
//       Remove a chain from the list. The removed chain is not changed.
//
//----------------------------------------------------------------------------
void
   DHDL_List<void>::remove(         // Remove from list
     Link*             head,        // -> First Link to remove
     Link*             tail)        // -> Final Link to remove
{
   Link* prev= head->prev;          // Link prior to head
   Link* next= tail->next;          // Link after tail

   if( prev == nullptr )
   {
     this->head= next;
     if( next != nullptr )
       next->prev= nullptr;
   }
   else
   {
     prev->next= next;
//// head->prev= nullptr;           // Not necessary (DO NOT REMOVE COMMENT)
   }

   if( next == nullptr )
   {
     this->tail= prev;
     if( prev != nullptr )
       prev->next= nullptr;
   }
   else
   {
     next->prev= prev;
//// tail->next= nullptr;           // Not necessary (DO NOT REMOVE COMMENT)
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       DHDL_List<void>::remq
//
// Purpose-
//       Remove oldest link from the list.
//
//----------------------------------------------------------------------------
DHDL_List<void>::Link*              // -> Removed Link
   DHDL_List<void>::remq( void )    // Remove oldest link
{
   Link* link= head;                // Address the first Link
   if( link != nullptr )            // If the list is not empty
   {
     head= link->next;              // Remove link from list
     if( head != nullptr )          // If the list is not empty
       head->prev= nullptr;         // Set first link backchain pointer
     else                           // The list is empty
       tail= nullptr;               // No tail link exists
   }

   return link;                     // Return the oldest link
}

//----------------------------------------------------------------------------
//
// Method-
//       DHDL_List<void>::reset
//
// Purpose-
//       Reset (empty) the list.
//
//----------------------------------------------------------------------------
DHDL_List<void>::Link*              // The set of removed Links
   DHDL_List<void>::reset( void )   // Reset (empty) the List
{
   Link* link= head;                // Resultant

   head= nullptr;
   tail= nullptr;

   return link;
}

//----------------------------------------------------------------------------
//
// Method-
//       DHSL_List<void>::fifo
//
// Purpose-
//       Insert a link onto the list with FIFO ordering.
//
//----------------------------------------------------------------------------
void
   DHSL_List<void>::fifo(           // Insert link, FIFO order
     Link*             link)        // -> Link to insert
{
   link->next= nullptr;             // Set link chain pointer

   if( head != nullptr )            // If adding to existing list
     tail->next= link;              // Insert the link onto the list
   else                             // If adding to empty list
     head= link;                    // Add link to list

   tail= link;
}

//----------------------------------------------------------------------------
//
// Method-
//       DHSL_List<void>::insert
//
// Purpose-
//       Insert a chain onto the list at the specified position.
//
//----------------------------------------------------------------------------
void
   DHSL_List<void>::insert(         // Add to list at position,
     Link*             link,        // -> Link to insert after
     Link*             head,        // -> First Link to insert
     Link*             tail)        // -> Final Link to insert
{
   if( link == nullptr )            // If insert at head
   {
     if( this->head == nullptr )    // If the list is empty
     {
       tail->next= nullptr;
       this->head= head;
       this->tail= tail;
     }
     else                           // If the list is populated
     {
       tail->next= this->head;
       this->head= head;
     }
   }
   else
   {
     tail->next= link->next;        // Set link chain pointers
     link->next= head;              // Insert onto the forward list

     if( tail->next == nullptr )    // If insert after last link
       this->tail= tail;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       DHSL_List::is_coherent
//
// Purpose-
//       Check the object for coherency.
//
//----------------------------------------------------------------------------
int                                 // TRUE if object is coherent
   DHSL_List<void>::is_coherent( void ) const // Coherency check
{
   Link* prev= head;
   if( prev != nullptr )
   {
     for(int count= 0;;count++)
     {
       Link* link= prev->next;
       if( link == nullptr )
         break;
       if( prev == tail || count > MAX_COHERENT )
         return false;

       prev= link;
     }
   }

   return (prev == tail);
}

//----------------------------------------------------------------------------
//
// Method-
//       DHSL_List<void>::is_on_list
//
// Purpose-
//       Check whether a link is on the list.
//
//----------------------------------------------------------------------------
int                                 // TRUE if link is in list
   DHSL_List<void>::is_on_list(     // Is Link contained?
     Link*             link) const  // -> Link
{
   if( link != nullptr )            // If a link was specified
   {
     Link* next= head;
     while( next != nullptr )
     {
       if( next == link )
         return true;

       next= next->next;
     }
   }

   return false;
}

//----------------------------------------------------------------------------
//
// Method-
//       DHSL_List<void>::lifo
//
// Purpose-
//       Add to list in LIFO order.
//
//----------------------------------------------------------------------------
void
   DHSL_List<void>::lifo(           // Insert link, LIFO order
     Link*             link)        // -> Link to insert
{
   link->next= head;                // Set link chain pointer

   if( head == nullptr )            // If adding to empty list
     tail= link;                    // Add to empty list

   head= link;                      // Add link to list
}

//----------------------------------------------------------------------------
//
// Method-
//       DHSL_List<void>::remove
//
// Purpose-
//       Remove a chain of elements from the DHSL_List.
//
//----------------------------------------------------------------------------
void
   DHSL_List<void>::remove(         // Remove specific links
     Link*             head,        // -> First Link to remove
     Link*             tail)        // -> Last Link to remove
{
   Link* link= this->head;          // Address oldest link
   if( link == nullptr )            // If the list is empty
     return;                        // Exit, function complete

   if( link == head )               // If removing head element
   {
     this->head= tail->next;
     if( this->head == nullptr )    // If the list is now empty
       this->tail= nullptr;         // Make it completely empty
     return;
   }

   for(;;)                          // Search for prior element
   {
     if( link->next == head )
       break;

     link= link->next;
     if( link == nullptr )
       return;
   }

   link->next= tail->next;          // Remove from within list
   if( link->next == nullptr )
     this->tail= link;
}

//----------------------------------------------------------------------------
//
// Method-
//       DHSL_List<void>::remq
//
// Purpose-
//       Remove oldest link from a DHSL_List.
//
//----------------------------------------------------------------------------
DHSL_List<void>::Link*              // -> Removed Link
   DHSL_List<void>::remq( void )    // Remove oldest link
{
   Link* link= head;                // Address oldest link
   if( link != nullptr )            // If the list is empty
   {
     head= link->next;              // Remove link from list
     if( head == nullptr )          // If the list is now empty
       tail= nullptr;               // Make it completely empty
   }

   return link;                     // Return the oldest link
}

//----------------------------------------------------------------------------
//
// Method-
//       DHSL_List<void>::reset
//
// Purpose-
//       Reset (empty) the List.
//
//----------------------------------------------------------------------------
DHSL_List<void>::Link*              // The set of removed links
   DHSL_List<void>::reset( void )   // Reset (empty) the List
{
   Link* link= head;

   head= nullptr;
   tail= nullptr;

   return link;
}

//----------------------------------------------------------------------------
//
// Method-
//       SHSL_List<void>::fifo
//
// Purpose-
//       Insert a link onto the list with FIFO ordering.
//
//----------------------------------------------------------------------------
void
   SHSL_List<void>::fifo(           // Insert link, FIFO order
     Link*             link)        // -> Link to insert
{
   link->next= nullptr;             // Set link chain pointer

   Link* last= head;                // Address first link on list
   if( last == nullptr )            // If adding to empty list
   {
     head= link;                    // Add link to list
     return;                        // Exit, function complete
   }

   while( last->next != nullptr )   // Find the (tail) end of the list
     last= last->next;

   last->next= link;                // Add link to list
}

//----------------------------------------------------------------------------
//
// Method-
//       SHSL_List<void>::insert
//
// Purpose-
//       Insert a chain onto the list at the specified position.
//
//----------------------------------------------------------------------------
void
   SHSL_List<void>::insert(         // Add to list at position,
     Link*             link,        // -> Link to insert after
     Link*             head,        // -> First Link to insert
     Link*             tail)        // -> Final Link to insert
{
   if( link ) {
     tail->next= link->next;
     link->next= head;
   } else {
     tail->next= this->head;
     this->head= head;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       SHSL_List<void>::is_coherent
//
// Purpose-
//       Check whether this list is coherent.
//
//----------------------------------------------------------------------------
int                                 // TRUE if the object is coherent
   SHSL_List<void>::is_coherent( void ) const // Is this list coherent?
{
   Link* link= head;
   for(int count= 0;;count++)
   {
     if( link == nullptr )
       break;
     if( count > MAX_COHERENT )
       return false;

     link= link->next;
   }

   return true;
}

//----------------------------------------------------------------------------
//
// Method-
//       SHSL_List<void>::is_on_list
//
// Purpose-
//       Check whether a link is on the list.
//
//----------------------------------------------------------------------------
int                                 // TRUE if link is in list
   SHSL_List<void>::is_on_list(     // Is Link contained?
     Link*             link) const  // -> Link
{
   if( link != nullptr )            // If a link was specified
   {
     Link* next= head;
     while( next != nullptr )
     {
       if( next == link )
         return true;

       next= next->next;
     }
   }

   return false;
}

//----------------------------------------------------------------------------
//
// Method-
//       SHSL_List<void>::lifo
//
// Purpose-
//       Insert a link onto the list with LIFO ordering.
//
//----------------------------------------------------------------------------
void
   SHSL_List<void>::lifo(           // Insert link, LIFO order
     Link*             link)        // -> Link to insert
{
   link->next= head;                // Set link chain pointer
   head= link;                      // Add link to list
}

//----------------------------------------------------------------------------
//
// Method-
//       SHSL_List<void>::remove
//
// Purpose-
//       Remove a chain of elements from the SHSL_List
//
//----------------------------------------------------------------------------
void
   SHSL_List<void>::remove(         // Remove a chain of elements
     Link*             head,        // -> First Link to remove
     Link*             tail)        // -> Last Link to remove
{
   Link* link= this->head;          // Address head link
   if( link == nullptr )            // If the list is empty
     return;                        // Exit, function complete

   if( link == head )               // If removing head element
   {
     this->head= tail->next;
     return;
   }

   for(;;)                          // Search for prior element
   {
     if( link->next == head )
       break;

     link= link->next;
     if( link == nullptr )          // IGNORE: Error if head not on List
       return;
   }

   link->next= tail->next;          // Remove from within list
}

//----------------------------------------------------------------------------
//
// Method-
//       SHSL_List<void>::remq
//
// Purpose-
//       Remove head link from the list.
//
//----------------------------------------------------------------------------
SHSL_List<void>::Link*              // -> Removed Link
   SHSL_List<void>::remq( void )    // Remove head link
{
   Link* link= head;                // Address head link
   if( link != nullptr )            // If the list is not empty
     head= link->next;              // Remove link from list

   return link;                     // Return the head link
}

//----------------------------------------------------------------------------
//
// Method-
//       SHSL_List<void>::reset
//
// Purpose-
//       Reset (empty) the list.
//
//----------------------------------------------------------------------------
SHSL_List<void>::Link*              // The set of removed Links
   SHSL_List<void>::reset( void )   // Reset (empty) the list
{
   Link* link= head;

   head= nullptr;

   return link;
}

//----------------------------------------------------------------------------
//
// Method-
//       SORT_List<void>::sort
//
// Purpose-
//       Sort the list.
//
//----------------------------------------------------------------------------
void
   SORT_List<void>::sort( void )    // Sort the list
{
   Link* head= static_cast<Link*>(reset()); // The original head of the list

   while( head != nullptr )         // Sort the list
   {
     Link* low= head;
     Link* next= low->get_next();
     while( next != nullptr )
     {
       if( low->compare(next) > 0 )
         low= next;

       next= next->get_next();
     }

     if( low == head )
       head= head->get_next();
     else
     {
       if( low->get_next() != nullptr )
         low->get_next()->set_prev(low->get_prev());
       low->get_prev()->set_next(low->get_next());
     }

     fifo(low);
   }
}
}  // namespace _PUB_NAMESPACE
