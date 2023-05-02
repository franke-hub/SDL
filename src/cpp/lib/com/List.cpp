//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2020 Frank Eskesen.
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
//       2020/10/03
//
//----------------------------------------------------------------------------
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <com/Atomic.h>
#include "com/List.h"

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

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE  1
#endif

//----------------------------------------------------------------------------
//
// Method-
//       AU_List<void>::~AU_List
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   AU_List<void>::~AU_List( void )  // Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       AU_List<void>::AU_List
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   AU_List<void>::AU_List( void )   // Default constructor
:  tail(NULL)
{
}

//----------------------------------------------------------------------------
//
// Method-
//       AU_List<void>::isCoherent
//
// Purpose-
//       Check the object for coherency.
//
//----------------------------------------------------------------------------
int                                 // TRUE if object is coherent
   AU_List<void>::isCoherent( void ) const // Coherency check
{
   Link* link= const_cast<Link*>(tail); // Address newest Link
   for(int count= 0; count < MAX_COHERENT; count++)
   {
     if( link == NULL )
       return TRUE;

     link= link->getPrev();
   }

   return FALSE;
}

//----------------------------------------------------------------------------
//
// Method-
//       AU_List<void>::isOnList
//
// Purpose-
//       Check whether a Link is on the List.
//
//----------------------------------------------------------------------------
int                                 // TRUE if Link is on List
   AU_List<void>::isOnList(         // Is Link contained?
     Link*             link) const  // -> Link
{
   if( link != NULL )               // If a link was specified
   {
     Link* prev= const_cast<Link*>(tail); // Remove volatile attribute
     while( prev != NULL )
     {
       if( prev == link )
         return TRUE;

       prev= prev->getPrev();
     }
   }

   return FALSE;
}

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
   Link*               oldv;        // Used in CSP()
   Link*               newv;        // Used in CSP()
   int                 rc;          // Called routine return code

   newv= link;                      // The tail-to-be
   do                               // Add link to list
   {
     oldv= const_cast<Link*>(tail); // Current tail
     link->setPrev(oldv);           // Set the link chain pointer
     rc= csp((ATOMICP**)&tail, oldv, newv); // Update the list
   } while( rc != 0 );              // Retry if required

   return oldv;                     // Return the previous tail
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
   int                 rc;          // Called routine return code

   for(;;)                          // Handle first element or empty list
   {
     link= const_cast<Link*>(tail); // Address newest Link
     if( link == NULL )             // If the List was empty
       return NULL;                 // Exit, link not found

     if( link != item )             // If not the first Link
       break;

     rc= csp((ATOMICP**)&tail, link, link->getPrev()); // Remove it from the List
     if( rc == 0 )                  // If it was removed
       return link;                 // Exit, the first element was removed
   }

   // We have more than one Link on the List
   do                               // Find the prior link
   {
     prev= link;                    // Save the prior Link pointer
     link= link->getPrev();         // Address the next older Link
     if( link == item )             // If we found the link
     {
       prev->setPrev(link->getPrev()); // Remove the found link
       break;
     }
   } while( link != NULL );         // Until we run out of links

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
//----------------------------------------------------------------------------
AU_List<void>::Link*                // -> Oldest Link
   AU_List<void>::remq( void )      // Remove oldest Link
{
   Link*               link;        // -> Link
   Link*               prev;        // -> Prior Link
   int                 rc;          // Called routine return code

   for(;;)                          // Handle List with zero or one elements
   {
     link= const_cast<Link*>(tail); // Address newest Link
     if( link == NULL )             // If the List was empty
       return NULL;                 // Exit, function complete

     if( link->getPrev() != NULL )  // If this is not the only Link
       break;

     rc= csp((ATOMICP**)&tail, link, NULL); // Remove it from the List
     if( rc == 0 )                 // If it was removed
       return link;                // Exit, the ONLY element was removed
   }

   // We have more than one Link on the List
   do                               // Find the oldest Link
   {
     prev= link;                    // Save the prior Link pointer
     link= link->getPrev();         // Address the next older Link
   } while( link->getPrev() != NULL ); // Until we find the oldest

   prev->setPrev(NULL);             // Remove the oldest link from the list
   return link;                     // Return the oldest link
}

AU_List<void>::Link*                // (See List.h)
   AU_List<void>::remq(             // Remove oldest Link
     Link*             last)        // Which might be this one
{
   Link*               link;        // -> Link
   Link*               prev;        // -> Prior Link
   int                 rc;          // Called routine return code

   for(;;)                          // Handle list with zero or one elements
   {
     link= const_cast<Link*>(tail); // Address newest Link
     if( link == NULL )             // If the List was empty
       return NULL;                 // Exit, no head element

     if( link->getPrev() != NULL )  // If this is not the only Link
       break;

     rc= csp((ATOMICP**)&tail, link, NULL); // Remove it from the List
     if( rc == 0 )                  // If it was removed
     {
       if( link == last )           // When this is true (the special case)
         link= NULL;                // The resultant is NULL

       return link;                 // Exit, the ONLY element was removed
     }
   }

   // We have more than one Link on the List
   do                               // Find the oldest Link
   {
     prev= link;                    // Save the prior Link pointer
     link= link->getPrev();         // Address the next older Link
   } while( link->getPrev() != NULL ); // Until we find the oldest

   prev->setPrev(NULL);             // Remove the oldest link from the list
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
   int                 rc;          // Called routine return code

   do                               // Remove all Links from List
   {
     link= const_cast<Link*>(tail); // Address newest Link
     if( link == NULL )             // If the List was empty
       break;                       // Nothing to reset

     rc= csp((ATOMICP**)&tail, link, NULL); // Remove the entire Link set
   } while( rc != 0 );              // If removal failed, try again

   return link;                     // Return the oldest link
}

//----------------------------------------------------------------------------
//
// Method-
//       DHDL_List<void>::~DHDL_List
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   DHDL_List<void>::~DHDL_List( void ) // Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       DHDL_List<void>::DHDL_List
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   DHDL_List<void>::DHDL_List( void ) // Default constructor
:  head(NULL), tail(NULL)
{
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
   link->setNext(NULL);             // Set next link pointer
   link->setPrev(tail);             // Set prior link pointer

   if( head == NULL )               // If the list is empty
     head= link;                    // Add link to empty list
   else                             // If the list is not empty
     tail->setNext(link);           // Add link to list

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
//       DHDL_List<void>::isCoherent
//
// Purpose-
//       Check the object for coherency.
//
//----------------------------------------------------------------------------
int                                 // TRUE if object is coherent
   DHDL_List<void>::isCoherent( void ) const // Coherency check
{
   Link*               link;        // Pointer to current link
   Link*               prev;        // Pointer to prior   link

   if( head == NULL )               // If the list is empty
   {
     if( tail != NULL )             // If the tail is not NULL
       return FALSE;

     return TRUE;                   // head == tail == NULL
   }

   link= head;
   prev= NULL;
   for(int count= 0;;count++)
   {
     if( link->getPrev() != prev )
       return FALSE;

     if( link->getNext() == NULL )
       break;

     if( link == tail )
       return FALSE;

     prev= link;
     link= link->getNext();

     if( count > MAX_COHERENT )
       return FALSE;
   }

   if( tail != link )
     return FALSE;

   return TRUE;
}

//----------------------------------------------------------------------------
//
// Method-
//       DHDL_List<void>::isOnList
//
// Purpose-
//       Check whether a Link is on the list.
//
//----------------------------------------------------------------------------
int                                 // TRUE if link is in list
   DHDL_List<void>::isOnList(       // Is Link contained?
     Link*             link) const  // -> Link
{
   if( link != NULL )               // If a Link was specified
   {
     Link* next= head;
     while( next != NULL )
     {
       if( next == link )
         return TRUE;

       next= next->getNext();
     }
   }

   return FALSE;
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
   link->setNext(head);             // Set next link pointer
   link->setPrev(NULL);             // Set prior link pointer

   if( head == NULL )               // If the list is empty
     tail= link;                    // Add link to empty list
   else                             // If the list is not empty
     head->setPrev(link);           // Add link to list

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
   Link* prev= head->getPrev();     // Link prior to head
   Link* next= tail->getNext();     // Link after tail

   if( prev == NULL )
   {
     this->head= next;
     if( next != NULL )
       next->setPrev(NULL);
   }
   else
   {
     prev->setNext(next);
//// head->setPrev(NULL);           // Not necessary (DO NOT CHANGE)
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
//// tail->setNext(NULL);           // Not necessary (DO NOT CHANGE)
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
   if( link != NULL )               // If the list is not empty
   {
     head= link->getNext();         // Remove link from list
     if( head != NULL )             // If the list is not empty
       head->setPrev(NULL);         // Set first link backchain pointer
     else                           // The list is empty
       tail= NULL;                  // No tail link exists
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

   head= NULL;
   tail= NULL;

   return link;
}

//----------------------------------------------------------------------------
//
// Method-
//       DHSL_List<void>::~DHSL_List
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   DHSL_List<void>::~DHSL_List( void ) // Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       DHSL_List<void>::DHSL_List
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   DHSL_List<void>::DHSL_List( void ) // Constructor
:  head(NULL), tail(NULL)
{
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
   link->setNext(NULL);             // Set link chain pointer

   if( head != NULL )               // If adding to existing list
     tail->setNext(link);           // Insert the link onto the list
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
   if( link == NULL )               // If insert at head
   {
     if( this->head == NULL )       // If the list is empty
     {
       tail->setNext(NULL);
       this->head= head;
       this->tail= tail;
     }
     else                           // If the list is populated
     {
       tail->setNext(this->head);
       this->head= head;
     }
   }
   else
   {
     tail->setNext(link->getNext());// Set link chain pointers
     link->setNext(head);           // Insert onto the forward list

     if( tail->getNext() == NULL )  // If insert after last link
       this->tail= tail;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       DHSL_List::isCoherent
//
// Purpose-
//       Check the object for coherency.
//
//----------------------------------------------------------------------------
int                                 // TRUE if object is coherent
   DHSL_List<void>::isCoherent( void ) const // Coherency check
{
   Link* prev= head;
   if( prev != NULL )
   {
     for(int count= 0;;count++)
     {
       Link* link= prev->getNext();
       if( link == NULL )
         break;
       if( prev == tail || count > MAX_COHERENT )
         return FALSE;

       prev= link;
     }
   }

   return (prev == tail);
}

//----------------------------------------------------------------------------
//
// Method-
//       DHSL_List<void>::isOnList
//
// Purpose-
//       Check whether a link is on the list.
//
//----------------------------------------------------------------------------
int                                 // TRUE if link is in list
   DHSL_List<void>::isOnList(       // Is Link contained?
     Link*             link) const  // -> Link
{
   if( link != NULL )               // If a link was specified
   {
     Link* next= head;
     while( next != NULL )
     {
       if( next == link )
         return TRUE;

       next= next->getNext();
     }
   }

   return FALSE;
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
   link->setNext(head);             // Set link chain pointer

   if( head == NULL )               // If adding to empty list
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
   if( link == NULL )               // If the list is empty
     return;                        // Exit, function complete

   if( link == head )               // If removing head element
   {
     this->head= tail->getNext();
     if( this->head == NULL )       // If the list is now empty
       this->tail= NULL;            // Make it completely empty
     return;
   }

   for(;;)                          // Search for prior element
   {
     if( link->getNext() == head )
       break;

     link= link->getNext();
     if( link == NULL )
       return;
   }

   link->setNext(tail->getNext());  // Remove from within list
   if( link->getNext() == NULL )
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
   if( link != NULL )               // If the list is empty
   {
     head= link->getNext();         // Remove link from list
     if( head == NULL )             // If the list is now empty
       tail= NULL;                  // Make it completely empty
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

   head= NULL;
   tail= NULL;

   return link;
}

//----------------------------------------------------------------------------
//
// Method-
//       SHSL_List<void>::~SHSL_List
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   SHSL_List<void>::~SHSL_List( void ) // Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       SHSL_List<void>::SHSL_List
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   SHSL_List<void>::SHSL_List( void ) // Constructor
:  head(NULL)
{
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
   link->setNext(NULL);             // Set link chain pointer

   Link* last= head;                // Address first link on list
   if( last == NULL )               // If adding to empty list
   {
     head= link;                    // Add link to list
     return;                        // Exit, function complete
   }

   while( last->getNext() != NULL ) // Find the end of the list
     last= last->getNext();

   last->setNext(link);             // Add link to list
}

//----------------------------------------------------------------------------
//
// Method-
//       SHSL_List<void>::isCoherent
//
// Purpose-
//       Check whether this list is coherent.
//
//----------------------------------------------------------------------------
int                                 // TRUE if the object is coherent
   SHSL_List<void>::isCoherent( void ) const // Is this list coherent?
{
   Link* link= head;
   for(int count= 0;;count++)
   {
     if( link == NULL )
       break;
     if( count > MAX_COHERENT )
       return FALSE;

     link= link->getNext();
   }

   return TRUE;                     // Checking is not possible
}

//----------------------------------------------------------------------------
//
// Method-
//       SHSL_List<void>::isOnList
//
// Purpose-
//       Check whether a link is on the list.
//
//----------------------------------------------------------------------------
int                                 // TRUE if link is in list
   SHSL_List<void>::isOnList(       // Is Link contained?
     Link*             link) const  // -> Link
{
   if( link != NULL )               // If a link was specified
   {
     Link* next= head;
     while( next != NULL )
     {
       if( next == link )
         return TRUE;

       next= next->getNext();
     }
   }

   return FALSE;
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
   link->setNext(head);             // Set link chain pointer
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
   Link* link= this->head;          // Address oldest link
   if( link == NULL )               // If the list is empty
     return;                        // Exit, function complete

   if( link == head )               // If removing head element
   {
     this->head= tail->getNext();
     return;
   }

   for(;;)                          // Search for prior element
   {
     if( link->getNext() == head )
       break;

     link= link->getNext();
     if( link == NULL )
       return;
   }

   link->setNext(tail->getNext());  // Remove from within list
}

//----------------------------------------------------------------------------
//
// Method-
//       SHSL_List<void>::remq
//
// Purpose-
//       Remove oldest link from the list.
//
//----------------------------------------------------------------------------
SHSL_List<void>::Link*              // -> Removed Link
   SHSL_List<void>::remq( void )    // Remove oldest link
{
   Link* link= head;                // Address oldest link
   if( link != NULL )               // If the list is not empty
     head= link->getNext();         // Remove link from list

   return link;                     // Return the oldest link
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

   head= NULL;

   return link;
}

//----------------------------------------------------------------------------
//
// Method-
//       SORT_List<void>::Link::compare
//
// Purpose-
//       Implements pure virtual method.
//
//----------------------------------------------------------------------------
int                                 // Result (<0, =0, >0)
   SORT_List<void>::Link::compare(  // Compare Link values
     const Link*           ) const  // -> Other Link
{
   return 0;                        // Default implementation
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
   Link* head= reset();             // The original head of the list

   while( head != NULL )            // Sort the list
   {
     Link* low= head;
     Link* next= low->getNext();
     while( next != NULL )
     {
       if( low->compare(next) > 0 )
         low= next;

       next= next->getNext();
     }

     if( low == head )
       head= head->getNext();
     else
     {
       if( low->getNext() != NULL )
         low->getNext()->setPrev(low->getPrev());
       low->getPrev()->setNext(low->getNext());
     }

     fifo(low);
   }
}

