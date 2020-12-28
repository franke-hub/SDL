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
//       List.h
//
// Purpose-
//       Describe the List objects.
//
// Last change date-
//       2020/12/28
//
// Implementation notes-
//       For all List classes, the is_coherent and is_on_list methods run in
//       linear time. Method is_coherent examines the entire List. Method
//       is_on_list traverses the List until either the Link is found or the
//       entire List examined.
//
//       In lieu of searching Lists for duplicated Links, is_coherent reports
//       FALSE should a List contains more than an implementation defined
//       (Currently 1G) Link count. Other methods assume that the List is
//       coherent and either ignore or do not check for usage errors.
//
//       By convention, Lists are ordered from head to tail. The head Link
//       is the Link that will be removed by REMQ and before which LIFO Links
//       are inserted. The tail Link is the insert point after which FIFO
//       Links are inserted.
//
//       Unlike std::list, pub::List::Link objects must be user-allocated and
//       user-released. None of the current List types use std::shared_ptr for
//       Link pointers, therefore you probably shouldn't either.
//       List destructors don't do anything. They neither remove anything nor
//       delete anything.
//
// List types-
//       AU_List<T>:    Atomic Update Linked List, the only thread-safe List.
//       DHDL_List<T>:  Doubly Headed Doubly Linked List.
//       DHSL_List<T>:  Doubly Headed Singly Linked List.
//       NODE_List<T>:  Doubly Headed Doubly Linked List, with parent link.
//       SHSL_List<T>:  Single Headed Singly Linked List.
//       SORT_List<T>:  privately derived from DHDL_List<T>, implementing all
//                      methods. A user-defined compare method is added to the
//                      Link class, and a sort method is added to SORT_List.
//       List<T>:       Equivalent to DHDL_List<T>.
//
//       In each case, the associated Link class is defined within the List
//       class. All Link classes must be derived from that List::Link class.
//       An example follows:
//
// Example declaration and usage-
//       class My_Link : public List<My_List>::Link {
//       public:
//         My_Link(...) : List<My_List>::Link(), ... { ... } // Constructor
//         // Remainder of implementation of My_Link
//       }; // class My_Link, the elements to be put on a class My_List
//
//       List<My_Link> my_list1;    // A List containing My_Link elements
//       List<My_Link> my_list2;    // Another List contining My_Link Links
//
//       My_Link* link1= new My_Link(); // Create a new My_Link
//       my_list1.fifo(link1);         // Insert it (FIFO) onto my_list1
//       My_link* link2= my_list1.remq(); // Then remove it, emptying my_list1
//       assert( link1 == link2 );     // The link added is the link removed
//       my_list2.lifo(link2);         // Insert it (LIFO) onto my_list2
//       // my_list1 is empty, my_list2 only contains link2 (which == link1)
//
//----------------------------------------------------------------------------
#ifndef _PUB_LIST_H_INCLUDED
#define _PUB_LIST_H_INCLUDED

#include <atomic>                   // For std::atomic
#include "config.h"                 // For _PUB_NAMESPACE

namespace _PUB_NAMESPACE {
//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
template<class T> class AU_FIFO;    // AU_List helper class
template<class T> class AU_List;    // Atomic update List
template<class T> class DHDL_List;  // DHDL List
template<class T> class DHSL_List;  // DHSL List
template<class T> class NODE_List;  // NODE List
template<class T> class SHSL_List;  // SHSL List
template<class T> class SORT_List;  // SORT List
template<class T> class List;       // List, equivalent to DHDL_List

//----------------------------------------------------------------------------
//
// Class-
//       AU_List<>
//
// Purpose-
//       The Atomic Update List is a thread-safe FIFO insertion List.
//
// Implementation notes-
//       The FIFO and RESET methods run in constant time.
//       The REMQ method runs in linear time.
//       The SWAP method runs in constant time. AU_FIFO construction and reset
//       run in linear time, inverting the list order in one pass through it.
//
//       The AU_List is a Singly Headed Singly Linked List maintained in
//       reverse Link sequence: It begins with the tail Link and continues via
//       the prev Link chain ending with the head Link.
//
//       The Atomic Update List is optimized for sequential FIFO insertion
//       onto a single list by multiple threads. Insertion, via the FIFO
//       method, is thread-safe and may be used concurrently with any other
//       method.
//
//       Methods REMQ, SWAP, is_coherent and is_on_list must be run within a
//       single-threaded consumer. While SWAP could operate concurrently with
//       other threads, consumer thread logic is required to maintain global
//       list ordering.
//
//       Methods LIFO, INSERT, and REMOVE (available on other List types)
//       cannot be provided: No known thread-safe lock-free implementations
//       exist. While method REMQ is provided, a more efficient method using
//       a helper class (AU_FIFO) exists. See the example usage code below.
//
//----------------------------------------------------------------------------
template<> class AU_List<void> {    // AU_List base
public:
class Link {                        // AU_List<void>::Link -------------------
friend class AU_List;
protected:
Link*                  prev;        // -> Prior Link
}; // class AU_List<void>::Link ----------------------------------------------

//----------------------------------------------------------------------------
// AU_List<>::Attributes
//----------------------------------------------------------------------------
protected:
std::atomic<Link*>     tail= nullptr; // -> Tail Link

//----------------------------------------------------------------------------
// AU_List<>::Constructors/Destructor
//----------------------------------------------------------------------------
   AU_List( void ) {}               // Constructor
   ~AU_List( void ) {}              // Destructor

   AU_List(const AU_List&) = delete; // Disallowed copy constructor
AU_List& operator=(const AU_List&) = delete; // Disallowed assignment operator

//----------------------------------------------------------------------------
//
// Method-
//       AU_List<>::fifo
//
// Purpose-
//       Insert a link onto the list with FIFO ordering. (Thread-safe)
//
//----------------------------------------------------------------------------
Link*                               // -> Prior tail
   fifo(                            // Insert (FIFO order)
     Link*             link);       // -> Link to insert

//----------------------------------------------------------------------------
//
// Method-
//       AU_List<>::get_tail
//
// Purpose-
//       Get the tail link. (Implemented in AU_List<T>, not here.)
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Method-
//       AU_List<>::is_coherent
//
// Purpose-
//       Coherency check.
//
// Implementation notes-
//       Only the consumer thread can safely use this method.
//
//----------------------------------------------------------------------------
int                                 // TRUE if the object is coherent
   is_coherent( void ) const;       // Coherency check

//----------------------------------------------------------------------------
//
// Method-
//       AU_List<>::is_on_list
//
// Purpose-
//       Test whether Link is present in this List.
//
// Implementation notes-
//       Only the consumer thread can safely use this method.
//
//----------------------------------------------------------------------------
int                                 // TRUE if link is contained
   is_on_list(                      // Is link contained?
     Link*             link) const; // -> Link

//----------------------------------------------------------------------------
//
// Method-
//       AU_List<>::remove
//
// Purpose-
//       Remove a link from the list.
//
// Implementation notes-
//       Only the consumer thread can safely use this method.
//
//----------------------------------------------------------------------------
Link*                               // -> Removed Link
   remove(                          // Remove link
     Link*             link);       // The link to remove

//----------------------------------------------------------------------------
//
// Method-
//       AU_List<>::remq
//
// Purpose-
//       Remove the head link from the list.
//
// Implementation notes-
//       Only the consumer thread can safely use this method.
//       This method is only suitable for small lists. See the swap method
//       and the AU_FIFO helper class for more general usage.
//
//----------------------------------------------------------------------------
Link*                               // -> Removed (logical head) Link
   remq( void );                    // Remove head link

//----------------------------------------------------------------------------
//
// Method-
//       AU_List<>::reset
//
// Purpose-
//       Remove ALL Links from the List.
//
// Implementation notes-
//       Only the consumer thread can safely use this method.
//       The links are (reverse) ordered from tail to head.
//
//----------------------------------------------------------------------------
Link*                               // -> The set of removed Links
   reset( void );                   // Reset (empty) the List

//----------------------------------------------------------------------------
//
// Method-
//       AU_List<>::swap
//
// Purpose-
//       Remove ALL Links from the List, replacing the List
//
// Implementation notes-
//       Consumers use this method to replace the current List with a dummy
//       Link and then process all the removed List elements. When this
//       processing completes, the process is repeated using the same dummy
//       Link. If no new elements were added during processing, swap returns
//       nullptr. If new elements were added, swap returns the new List with
//       the dummy element as the head (last) element.
//
//       If (atomically) the current tail == nullptr or the replacement tail,
//       the List remains or is emptied and nullptr is returned.
//
//       While multiple consumer threads could use this method, global FIFO
//       ordering would then be lost.
//
//       A single consumer thread can maintain global FIFO ordering.
//       Method swqp returns links ordered from tail to head. The AU_FIFO
//       helper class efficiently inverts that LIFO ordering into FIFO order.
//       Sample code:
//         AU_List<Item>       itemList;    // The Work item list
//         :
//         // List handling logic (single-threaded)
//         // (List handling logic only needs to be driven when an item is
//         // added to an empty list, i.e. itemList.fifo() returns nullptr.
//         // This logic handles spurious activation also.)
//         Item                only;        // Our fake work Item
//         Item*               fake= &only; // (and a pointer to it)
//
//         Item* list= itemList.swap(fake); // Replace List with fake element
//         AU_FIFO<Item> fifo(list);        // Initialize the FIFO
//         for(;;) {                        // Drain the itemList
//           Item* item= fifo.remq();       // Get head link
//           if( item == nullptr ) {        // If none remain
//             list= itemList.swap(fake);   // Get new list
//             if( list == nullptr )        // If none left
//               return;                    // (or break)
//
//             fifo.reset(list);            // Re-initialize the AU_FIFO
//             item= fifo.remq();           // Remove head link (the fake item)
//             assert( item == fake );      // Verify what we think we know
//
//             item= fifo.remq();           // Remove next head link
//             assert( item != nullptr );   // Which cannot be a nullptr
//           }
//
//           // Process the removed Item (*item)
//         }
//
//----------------------------------------------------------------------------
Link*                               // -> The set of removed Links
   swap(                            // Swap (empty) the List
     Link*             tail);       // Replacing it with this link
}; // class AU_List<>

//----------------------------------------------------------------------------
//
// Class-
//       AU_List<T>
//
// Purpose-
//       Typed AU_List object, where T is of class AU_List<T>::Link.
//
//----------------------------------------------------------------------------
template<class T>
class AU_List : public AU_List<void> { // AU_List<T>
public:
typedef AU_List<void>  B_List;      // The base List type
typedef AU_List<void>::Link B_Link; // The base Link type

class Link : public B_Link {        // AU_List<T>::Link ----------------------
friend class AU_FIFO<T>;            // (For set_prev() access)
public:
T*                                  // -> Prior Link
   get_prev( void ) const           // Get prior Link
{  return static_cast<T*>(prev); }

protected:
void
   set_prev(                        // Set prior Link
     T*                link)        // To this link
{  prev= link; }
}; // class AU_List<T>::Link -------------------------------------------------

//----------------------------------------------------------------------------
// AU_List<T>::Constructor/Destructor
//----------------------------------------------------------------------------
   AU_List( void ) {}
   ~AU_List( void ) {}

//----------------------------------------------------------------------------
// AU_List<T>::Methods
//----------------------------------------------------------------------------
T*                                  // -> Prior tail
   fifo(                            // Insert (fifo order)
     T*                link)        // -> Link to insert
{  return static_cast<T*>(B_List::fifo(link)); }

T*                                  // -> Tail Link
   get_tail( void ) const           // Get tail link
{  return static_cast<T*>(tail.load()); }

int                                 // TRUE if the object is coherent
   is_coherent( void ) const        // Coherency check
{  return B_List::is_coherent(); }

int                                 // TRUE if link is contained
   is_on_list(                      // Is link contained?
     T*                link)        // -> Link
{  return B_List::is_on_list(link); }

T*                                  // Removed link (if on list)
   remove(                          // Remove Link
     T*                link)        // The link to remove
{  return static_cast<T*>(B_List::remove(link)); }

T*                                  // Removed Link
   remq( void )                     // Remove head Link
{  return static_cast<T*>(B_List::remq()); }

T*                                  // -> The set of removed Links
   reset( void )                    // Reset (empty) the List
{  return static_cast<T*>(B_List::reset()); }

T*                                  // (See above)
   swap(                            // Swap (empty) the List
     Link*             tail)        // Replacing it with this link
{  return static_cast<T*>(B_List::swap(tail)); }
}; // class AU_List<T>

//----------------------------------------------------------------------------
//
// Class-
//       AU_FIFO<T>
//
// Purpose-
//       A helper class used to improve AU_List::remq performance.
//       (See sample code in AU_List<void>::swap() above.)
//
//----------------------------------------------------------------------------
template<class T>                   // T is an AU_List::Link
class AU_FIFO {                     // AU_FIFO helper class
protected:
T*                     head= nullptr; // The current head

public:
//----------------------------------------------------------------------------
// AU_FIFO<T>::Constructor/Destructor
//----------------------------------------------------------------------------
   AU_FIFO(                         // Constructor
     T*                tail)        // The list
{  reset(tail); }                   // Initial conversion

   ~AU_FIFO( void ) {}              // Does nothing

//----------------------------------------------------------------------------
// AU_FIFO<T>::remq: Obtain next element
//----------------------------------------------------------------------------
T*                                  // The head Link
   remq( void )                     // Get head Link
{
   T* result= head;

   if( result != nullptr ) {
     head= result->get_prev();
     result->set_prev(nullptr);     // (set_prev() is protected)
   }

   return result;
}

//----------------------------------------------------------------------------
// AU_FIFO<T>::reset: (Re-)initialize the list
//----------------------------------------------------------------------------
void
   reset(                           // Re-initialize the list
     T*                tail)        // The list
{
   while( tail ) {
     T* prev= tail->get_prev();
     tail->set_prev(head);
     head= tail;
     tail= prev;
   }
}
}; // class AU_FIFO<T>

//----------------------------------------------------------------------------
//
// Class-
//       DHDL_List<>
//
// Purpose-
//       The Doubly Headed, Doubly Linked List is a general purpose List.
//
// Implementation notes-
//       The DHDL_List is not thread safe. Method usage must be serialized.
//       The FIFO, LIFO, INSERT, and REMOVE methods run in constant time.
//
//----------------------------------------------------------------------------
template<> class DHDL_List<void> {  // DHDL_List base
public:
class Link {                        // DHDL_List<void>::Link -----------------
friend class DHDL_List;
protected:
Link*                  next;        // -> Forward Link
Link*                  prev;        // -> Reverse Link
}; // class DHDL_List<void>::Link --------------------------------------------

//----------------------------------------------------------------------------
// DHDL_List<>::Attributes
//----------------------------------------------------------------------------
protected:
Link*                  head= nullptr; // -> Head Link
Link*                  tail= nullptr; // -> Tail Link

//----------------------------------------------------------------------------
// DHDL_List<>::Constructors/Destructor
//----------------------------------------------------------------------------
   DHDL_List( void ) {}             // Default constructor
   ~DHDL_List( void ) {}            // Default destructor

   DHDL_List(const DHDL_List&) = delete; // Disallowed copy constructor
DHDL_List&
   operator=(const DHDL_List&) = delete; // Disallowed assignment operator

//----------------------------------------------------------------------------
//
// Method-
//       DHDL_List<>::fifo
//
// Purpose-
//       Insert a link onto the list with FIFO ordering.
//
//----------------------------------------------------------------------------
void
   fifo(                            // Insert (FIFO order)
     Link*             link);       // -> Link to insert

//----------------------------------------------------------------------------
//
// Method-
//       DHDL_List<>::get_head
//       DHDL_List<>::get_tail
//
// Purpose-
//       Get the head link. (Implemented in DHDL_List<T>, not here,)
//       Get the tail link. (Implemented in DHDL_List<T>, not here.)
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Method-
//       DHDL_List<>::insert
//
// Purpose-
//       Insert a chain of elements onto the list at the specified position.
//
//----------------------------------------------------------------------------
void
   insert(                          // Insert at position,
     Link*             link,        // -> Link to insert after
     Link*             head,        // -> First Link to insert
     Link*             tail);       // -> Final Link to insert

//----------------------------------------------------------------------------
//
// Method-
//       DHDL_List<>::is_coherent
//
// Purpose-
//       List coherency check.
//
//----------------------------------------------------------------------------
int                                 // TRUE if the object is coherent
   is_coherent( void ) const;       // Coherency check

//----------------------------------------------------------------------------
//
// Method-
//       DHDL_List<>::is_on_list
//
// Purpose-
//       Test whether Link is present in this List.
//
//----------------------------------------------------------------------------
int                                 // TRUE if link is contained
   is_on_list(                      // Is Link contained?
     Link*             link) const; // -> Link

//----------------------------------------------------------------------------
//
// Method-
//       DHDL_List<>::lifo
//
// Purpose-
//       Insert a link onto the list with LIFO ordering.
//
//----------------------------------------------------------------------------
void
   lifo(                            // Insert (LIFO order)
     Link*             link);       // -> Link to insert

//----------------------------------------------------------------------------
//
// Method-
//       DHDL_List<>::remove
//
// Purpose-
//       Remove a chain of elements from the list.
//
//----------------------------------------------------------------------------
void
   remove(                          // Remove from list
     Link*             head,        // -> First Link to remove
     Link*             tail);       // -> Final Link to remove

//----------------------------------------------------------------------------
//
// Method-
//       DHDL_List<>::remq
//
// Purpose-
//       Remove the head Link from the List.
//
//----------------------------------------------------------------------------
Link*                               // -> Removed Link
   remq( void );                    // Remove head Link

//----------------------------------------------------------------------------
//
// Method-
//       DHDL_List<>::reset
//
// Purpose-
//       Remove ALL Links from the List.
//
//----------------------------------------------------------------------------
Link*                               // The set of removed Links
   reset( void );                   // Reset (empty) the list
}; // class DHDL_List<>

//----------------------------------------------------------------------------
//
// Class-
//       DHDL_List<T>
//
// Purpose-
//       Typed DHDL_List object, where T is of class DHDL_List<T>::Link.
//
//----------------------------------------------------------------------------
template<class T>
class DHDL_List : public DHDL_List<void> { // DHDL_List<T>
public:
typedef DHDL_List<void> B_List;     // The base List type
typedef DHDL_List<void>::Link B_Link; // The base Link type

class Link : public B_Link {        // DHDL_List<T>::Link --------------------
public:
T*                                  // -> Next Link
   get_next( void ) const           // Get next Link
{  return static_cast<T*>(next); }

T*                                  // -> Prior Link
   get_prev( void ) const           // Get prior Link
{  return static_cast<T*>(prev); }
}; // class DHDL_List<T>::Link -----------------------------------------------

//----------------------------------------------------------------------------
// DHDL_List<T>::Constructor/Destructor
//----------------------------------------------------------------------------
   DHDL_List( void ) {}
   ~DHDL_List( void ) {}

//----------------------------------------------------------------------------
// DHDL_List<T>::Methods
//----------------------------------------------------------------------------
void
   fifo(                            // Insert (FIFO order)
     T*                link)        // -> Link to insert
{  B_List::fifo(link); }

T*                                  // -> Head T* on List
   get_head( void ) const           // Get head link
{  return static_cast<T*>(head); }

T*                                  // -> Tail T* on List
   get_tail( void ) const           // Get tail link
{  return static_cast<T*>(tail); }

void
   insert(                          // Insert at position,
     T*                link,        // -> Link to insert after
     T*                head,        // -> First Link to insert
     T*                tail)        // -> Final Link to insert
{  B_List::insert(link, head, tail); }

int                                 // TRUE if the object is coherent
   is_coherent( void ) const        // Coherency check
{  return B_List::is_coherent(); }

int                                 // TRUE if link is contained
   is_on_list(                      // Is Link contained?
     T*                link) const  // -> Link
{  return B_List::is_on_list(link); }

void
   lifo(                            // Insert (LIFO order)
     T*                link)        // -> Link to insert
{  B_List::lifo(link); }

void
   remove(                          // Remove from list
     T*                head,        // -> First Link to remove
     T*                tail)        // -> Final Link to remove
{  B_List::remove(head, tail); }

T*                                  // Removed T*
   remq( void )                     // Remove head link
{  return static_cast<T*>(B_List::remq()); }

T*                                  // -> The set of removed Links
   reset( void )                    // Reset (empty) the List
{  return static_cast<T*>(B_List::reset()); }
}; // class DHDL_List<T>

//----------------------------------------------------------------------------
//
// Class-
//       DHSL_List<>
//
// Purpose-
//       The Doubly Headed, Singly Linked List.
//
// Implementation notes-
//       The DHSL_List is not thread safe. Method usage must be serialized.
//
//       The FIFO, LIFO, REMQ, and RESET methods run in constant time.
//       The INSERT and REMOVE methods run in linear time.
//
//----------------------------------------------------------------------------
template<> class DHSL_List<void> {  // DHSL_List base
public:
class Link {                        // DHSL_List<void>::Link -----------------
friend class DHSL_List;
protected:
Link*                  next;        // -> Forward Link
}; // class DHSL_List<void>::Link --------------------------------------------

//----------------------------------------------------------------------------
// DHSL_List<>::Attributes
//----------------------------------------------------------------------------
protected:
Link*                  head= nullptr; // -> Head Link
Link*                  tail= nullptr; // -> Tail Link

//----------------------------------------------------------------------------
// DHSL_List<>::Constructors/Destructor
//----------------------------------------------------------------------------
   ~DHSL_List( void ) {}            // Default destructor
   DHSL_List( void ) {}             // Default constructor

   DHSL_List(const DHSL_List&) = delete; // Disallowed copy constructor
DHSL_List&
   operator=(const DHSL_List&) = delete; // Disallowed assignment operator

//----------------------------------------------------------------------------
//
// Method-
//       DHSL_List<>::fifo
//
// Purpose-
//       Insert a Link onto the List with FIFO ordering.
//
//----------------------------------------------------------------------------
void
   fifo(                            // Insert (FIFO order)
     Link*             link);       // -> Link to insert

//----------------------------------------------------------------------------
//
// Method-
//       DHSL_List<>::get_head
//
// Purpose-
//       Get the head link. (Implemented in DHSL_List<T>, not here,)
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Method-
//       DHSL_List<>::insert
//
// Purpose-
//       Insert a chain of elements onto the list at the specified position.
//
//----------------------------------------------------------------------------
void
   insert(                          // Insert at position,
     Link*             link,        // -> Link to insert after
     Link*             head,        // -> First Link to insert
     Link*             tail);       // -> Final Link to insert

//----------------------------------------------------------------------------
//
// Method-
//       DHSL_List<>::is_coherent
//
// Purpose-
//       List coherency check.
//
//----------------------------------------------------------------------------
int                                 // TRUE if the object is coherent
   is_coherent( void ) const;       // Coherency check

//----------------------------------------------------------------------------
//
// Method-
//       DHSL_List<>::is_on_list
//
// Purpose-
//       Test whether Link is present in this List.
//
//----------------------------------------------------------------------------
int                                 // TRUE if Link is contained
   is_on_list(                      // Is Link contained?
     Link*             link) const; // -> Link

//----------------------------------------------------------------------------
//
// Method-
//       DHSL_List<>::lifo
//
// Purpose-
//       Insert a Link onto the List with LIFO ordering.
//
//----------------------------------------------------------------------------
void
   lifo(                            // Insert (LIFO order)
     Link*             link);       // -> Link to insert

//----------------------------------------------------------------------------
//
// Method-
//       DHSL_List<>::remove
//
// Purpose-
//       Remove a chain of elements from the List.
//       This is an expensive operation for a DHSL_List.
//
//----------------------------------------------------------------------------
void
   remove(                          // Remove from DHSL_List
     Link*             head,        // -> First Link to remove
     Link*             tail);       // -> Final Link to remove

//----------------------------------------------------------------------------
//
// Method-
//       DHSL_List<>::remq
//
// Purpose-
//       Remove the head Link from the list.
//
//----------------------------------------------------------------------------
Link*                               // -> Removed Link
   remq( void );                    // Remove head Link

//----------------------------------------------------------------------------
//
// Method-
//       DHSL_List<>::reset
//
// Purpose-
//       Remove ALL Links from the List.
//
//----------------------------------------------------------------------------
Link*                               // The set of removed Links
   reset( void );                   // Reset (empty) the List
}; // class DHSL_List<>

//----------------------------------------------------------------------------
//
// Class-
//       DHSL_List<T>
//
// Purpose-
//       Typed DHSL_List object, where T is of class DHSL_List<T>::Link.
//
//----------------------------------------------------------------------------
template<class T>
class DHSL_List : public DHSL_List<void> { // DHSL_List<T>
public:
typedef DHSL_List<void> B_List;     // The base List type
typedef DHSL_List<void>::Link B_Link; // The base Link type

class Link : public B_Link {        // DHSL_List<T>::Link --------------------
public:
T*                                  // -> Next Link
   get_next( void ) const           // Get next Link
{  return static_cast<T*>(next); }
}; // class DHSL_List<T>::Link -----------------------------------------------

//----------------------------------------------------------------------------
// DHSL_List<T>::Constructor/Destructor
//----------------------------------------------------------------------------
   DHSL_List( void ) {}
   ~DHSL_List( void ) {}

//----------------------------------------------------------------------------
// DHSL_List<T>::Methods
//----------------------------------------------------------------------------
void
   fifo(                            // Insert (FIFO order)
     T*                link)        // -> Link to insert
{  B_List::fifo(link); }

T*                                  // -> Head T* on List
   get_head( void ) const           // Get head Link
{  return static_cast<T*>(head); }

T*                                  // -> Tail T* on List
   get_tail( void ) const           // Get tail Link
{  return static_cast<T*>(tail); }

void
   insert(                          // Insert at position,
     T*                link,        // -> Link to insert after
     T*                head,        // -> First Link to insert
     T*                tail)        // -> Final Link to insert
{  B_List::insert(link, head, tail); }

int                                 // TRUE if the object is coherent
   is_coherent( void ) const        // Coherency check
{  return B_List::is_coherent(); }

int                                 // TRUE if Link is contained
   is_on_list(                      // Is Link contained?
     T*                link) const  // -> Link
{  return B_List::is_on_list(link); }

void
   lifo(                            // Insert (LIFO order)
     T*                link)        // -> Link to insert
{  B_List::lifo(link); }

void
   remove(                          // Remove from DHSL_List
     T*                head,        // -> First Link to remove
     T*                tail)        // -> Final Link to remove
{  B_List::remove(head, tail); }

T*                                  // Removed T*
   remq( void )                     // Remove head Link
{  return static_cast<T*>(B_List::remq()); }

T*                                  // -> The set of removed Links
   reset( void )                    // Reset (empty) the List
{  return static_cast<T*>(B_List::reset()); }
}; // class DHSL_List<T>

//----------------------------------------------------------------------------
//
// Class-
//       NODE_List<T>
//
// Purpose-
//       Typed NODE_List object, where T is of class NODE_List<T>::Link.
//
// Implementation note-
//       This is a DHDL_List<T>; Links also contain a pointer to parent List
//
//----------------------------------------------------------------------------
template<class T>
class NODE_List : public DHDL_List<void> { // NODE_List<T>
public:
typedef DHDL_List<void> B_List;     // The base List type
typedef DHDL_List<void>::Link B_Link; // The base Link type

class Link : public B_Link {        // NODE_List<T>::Link --------------------
protected:
NODE_List<T>*          parent= nullptr; // The parent Link

public:
NODE_List<T>*                       // The parent List
   get_parent(void) const           // Get parent List
{  return parent; }

void
   set_parent(                      // Set parent Link
     NODE_List<T>*     list)        // To this Link
{  parent= list; }

T*                                  // -> Next Link
   get_next( void ) const           // Get next Link
{  return static_cast<T*>(next); }

T*                                  // -> Prior Link
   get_prev( void ) const           // Get prior Link
{  return static_cast<T*>(prev); }
}; // class NODE_List<T>::Link -----------------------------------------------

void
   fifo(                            // Insert (FIFO order)
     T*                link)        // -> Link to insert
{
   B_List::fifo(link);
   link->set_parent(this);
}

T*                                  // -> Head T* on List
   get_head( void ) const           // Get head link
{  return static_cast<T*>(head); }

T*                                  // -> Tail T* on List
   get_tail( void ) const           // Get tail link
{  return static_cast<T*>(tail); }

void
   insert(                          // Insert at position,
     T*                link,        // -> Link to insert after
     T*                head,        // -> First Link to insert
     T*                tail)        // -> Final Link to insert
{
   B_List::insert(link, head, tail);
   for(;;) {
     head->set_parent(this);
     if( head == tail )
       break;
     head= head->get_next();
   }
}

int                                 // TRUE if the object is coherent
   is_coherent( void ) const        // Coherency check
{  return B_List::is_coherent(); }

int                                 // TRUE if link is contained
   is_on_list(                      // Is Link contained?
     T*                link) const  // -> Link
{  return B_List::is_on_list(link); }

void
   lifo(                            // Insert (LIFO order)
     T*                link)        // -> Link to insert
{
   B_List::lifo(link);
   link->set_parent(this);
}

void
   remove(                          // Remove from list
     T*                head,        // -> First Link to remove
     T*                tail)        // -> Final Link to remove
{
   B_List::remove(head, tail);

   for(;;) {
     head->set_parent(nullptr);
     if( head == tail )
       break;
     head= head->get_next();
   }
}

T*                                  // Removed T*
   remq( void )                     // Remove head link
{
   T* link= static_cast<T*>(B_List::remq());
   if( link )
     link->set_parent(nullptr);
   return link;
}

T*                                  // -> The set of removed Links
   reset( void )                    // Reset (empty) the List
{
   T* link= static_cast<T*>(B_List::reset());
   T* next= link;
   while( next != nullptr ) {
     next->set_parent(nullptr);
     next= next->get_next();
   }

   return link;
}
}; // class NODE_List<T>

//----------------------------------------------------------------------------
//
// Class-
//       SHSL_List<>
//
// Purpose-
//       The Singly Headed, Singly Linked List.
//
// Implemenation notes-
//       The SHDL_List is not thread safe. Method usage must be serialized.
//       The SHSL_List is optimized for LIFO operation. If you think of
//       this List as a Stack, LIFO == PUSH and REMQ == PULL.
//
//       The INSERT, LIFO and REMQ methods run in constant time.
//       The FIFO and REMOVE methods run in linear time.
//
//----------------------------------------------------------------------------
template<> class SHSL_List<void> {  // SHSL_List base
public:
class Link {                        // SHSL_List<void>::Link -----------------
friend class SHSL_List;
protected:
Link*                  next;        // -> Next Link
}; // class SHSL_List<void>::Link --------------------------------------------

//----------------------------------------------------------------------------
// SHSL_List<>::Attributes
//----------------------------------------------------------------------------
protected:
Link*                  head= nullptr; // -> Head Link

//----------------------------------------------------------------------------
// SHSL_List<>::Constructors/Destructor
//----------------------------------------------------------------------------
   SHSL_List( void ) {}             // Default constructor
   ~SHSL_List( void ) {}            // Default destructor

   SHSL_List(const SHSL_List&) = delete; // Disallowed copy constructor
SHSL_List&
   operator=(const SHSL_List&) = delete; // Disallowed assignment operator

//----------------------------------------------------------------------------
//
// Method-
//       SHSL_List<>::fifo
//
// Purpose-
//       Insert a Link onto the List with FIFO ordering.
//
// Implementation notes-
//       This examines all existing Link elements and takes linear time.
//
//----------------------------------------------------------------------------
void
   fifo(                            // Insert (FIFO order)
     Link*             link);       // -> Link to insert

//----------------------------------------------------------------------------
//
// Method-
//       SHSL_List<>::get_head
//
// Purpose-
//       Get the head link. (Implemented in SHSL_List<T>, not here,)
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Method-
//       SHSL_List<>::insert
//
// Purpose-
//       Insert a chain of elements onto the list at the specified position.
//
//----------------------------------------------------------------------------
void
   insert(                          // Insert at position,
     Link*             link,        // -> Link to insert after
     Link*             head,        // -> First Link to insert
     Link*             tail);       // -> Final Link to insert

//----------------------------------------------------------------------------
//
// Method-
//       SHSL_List<>::is_coherent
//
// Purpose-
//       List coherency check.
//
//----------------------------------------------------------------------------
int                                 // TRUE if the object is coherent
   is_coherent( void ) const;       // Coherency check

//----------------------------------------------------------------------------
//
// Method-
//       SHSL_List<>::is_on_list
//
// Purpose-
//       Test whether Link is present in this List.
//
//----------------------------------------------------------------------------
int                                 // TRUE if Link is contained
   is_on_list(                      // Is Link contained?
     Link*             link) const; // -> Link

//----------------------------------------------------------------------------
//
// Method-
//       SHSL_List<>::lifo
//
// Purpose-
//       Insert a Link onto the List with LIFO ordering.
//
//----------------------------------------------------------------------------
void
   lifo(                            // Insert (LIFO order)
     Link*             link);       // -> Link to insert

//----------------------------------------------------------------------------
//
// Method-
//       SHSL_List<>::remove
//
// Purpose-
//       Remove a chain of elements from the List.
//
// Implementation notes-
//       This examines existing Link elements, taking linear time.
//
//----------------------------------------------------------------------------
void
   remove(                          // Remove from List
     Link*             head,        // -> First Link to remove
     Link*             tail);       // -> Final Link to remove

//----------------------------------------------------------------------------
//
// Method-
//       SHSL_List<>::remq
//
// Purpose-
//       Remove the head Link from the list.
//
// Implementation notes-
//       REMQ is logically consistent with the LIFO and FIFO methods.
//       The list is ordered from head to tail, so FIFO scans the list,
//       inserting at the end of it, the tail.
//
//----------------------------------------------------------------------------
Link*                               // -> Removed Link
   remq( void );                    // Remove head Link

//----------------------------------------------------------------------------
//
// Method-
//       SHSL_List<>::reset
//
// Purpose-
//       Remove ALL Links from the List.
//
//----------------------------------------------------------------------------
Link*                               // The set of removed Links
   reset( void );                   // Reset (empty) the List
}; // class SHSL_List<>

//----------------------------------------------------------------------------
//
// Class-
//       SHSL_List<T>
//
// Purpose-
//       Typed SHSL_List object, where T is of class SHSL_List<T>::Link.
//
//----------------------------------------------------------------------------
template<class T>
class SHSL_List : public SHSL_List<void> { // SHSL_List<T>
public:
typedef SHSL_List<void> B_List;     // The base List type
typedef SHSL_List<void>::Link B_Link; // The base Link type

class Link : public B_Link {        // SHSL_List<T>::Link --------------------
public:
T*                                  // -> Next Link
   get_next( void ) const           // Get next Link
{  return static_cast<T*>(next); }
}; // class SHSL_List<T>::Link -----------------------------------------------

//----------------------------------------------------------------------------
// SHSL_List<T>::Constructor/Destructor
//----------------------------------------------------------------------------
   SHSL_List( void ) {}
   ~SHSL_List( void ) {}

//----------------------------------------------------------------------------
// SHSL_List<T>::Methods
//----------------------------------------------------------------------------
void
   fifo(                            // Insert (FIFO order)
     T*                link)        // -> Link to insert
{  B_List::fifo(link); }

T*                                  // -> Head T* on List
   get_head( void ) const           // Get head link
{  return static_cast<T*>(head); }

void
   insert(                          // Insert at position,
     T*                link,        // -> Link to insert after
     T*                head,        // -> First Link to insert
     T*                tail)        // -> Final Link to insert
{  B_List::insert(link, head, tail); }

int                                 // TRUE if the object is coherent
   is_coherent( void ) const        // Coherency check
{  return B_List::is_coherent(); }

int                                 // TRUE if Link is contained
   is_on_list(                      // Is Link contained?
     T*                link) const  // -> Link
{  return B_List::is_on_list(link); }

void
   lifo(                            // Insert (LIFO order)
     T*                link)        // -> Link to insert
{  B_List::lifo(link); }

void
   remove(                          // Remove from List
     T*                head,        // -> First Link to remove
     T*                tail)        // -> Final Link to remove
{  B_List::remove(head, tail); }

T*                                  // Removed T*
   remq( void )                     // Remove head link
{  return static_cast<T*>(B_List::remq()); }

T*                                  // -> The set of removed Links
   reset( void )                    // Reset (empty) the List
{  return static_cast<T*>(B_List::reset()); }
}; // class SHSL_List<T>

//----------------------------------------------------------------------------
//
// Class-
//       SORT_List<>
//
// Purpose-
//       Extend DHDL_List<> adding sort functionality
//
// Implementation note-
//       The Link<T>::compare signature MUST MATCH Link<void>::compare
//
//----------------------------------------------------------------------------
template<> class SORT_List<void> : public DHDL_List<void> { // SORT_List base
public:
typedef DHDL_List<void> B_List;     // The base List type
typedef DHDL_List<void>::Link B_Link; // The base Link type

class Link : public B_Link {        // SORT_List<void>:Link ------------------
friend class SORT_List;
public:
virtual int                         // Result (<0, =0, >0)
   compare(const Link*) const       // Compare to
{  return 0; }

protected:
Link* get_next() { return static_cast<Link*>(next); }
Link* get_prev() { return static_cast<Link*>(prev); }
void  set_next(Link* link) { next= link; }
void  set_prev(Link* link) { prev= link; }
}; // class SORT_List<void>::Link --------------------------------------------

//----------------------------------------------------------------------------
//
// Method-
//       SORT_List<>::sort
//
// Purpose-
//       Sort the list. After this operation, the list is sorted.
//       Sort runs in polynomial time, currently implemented as a bubble sort.
//
//----------------------------------------------------------------------------
void
   sort( void );                    // Sort the list
}; // class SORT_List<>

//----------------------------------------------------------------------------
//
// Class-
//       SORT_List<T>
//
// Purpose-
//       The SORT_List is a sortable DHDL_List.
//
// Implementation notes-
//       A SORT_List is in sorted order (from lowest to highest) only after
//       the sort method is invoked. If Links are added to the List, the
//       List remains potentially out of sort order until the sort method
//       is invoked (again.)
//
// Implementation notes-
//       The Link<T>::compare method MUST BE supplied. There is no default.
//
// Code format:
//       template<>
//       int SORT_List<T>::Link::compare(const B_Link* that_) const overrider
//       { /* Implementation */ }
//       // T* that= static_cast<T*>(that_);
//
// To override the base class implementation, in class T code:
//     virtual int compare(const SORT_List<void>::Link* that) const
//     { implementation; }
//
//----------------------------------------------------------------------------
template<class T>
class SORT_List : public SORT_List<void> { // SORT_List<T>
public:
typedef SORT_List<void> B_List;     // The base List type
typedef SORT_List<void>::Link B_Link; // The base Link type

class Link : public B_Link {        // SORT_List<T>::Link --------------------
public:
// The compare method is user-supplied. See implementation notes above.
virtual int                         // Result (<0, =0, >0)
   compare(                         // Compare to
     const B_Link*     that) const override; // This Link

T*                                  // -> Next Link
   get_next( void ) const           // Get next Link
{  return static_cast<T*>(next); }

T*                                  // -> Prior Link
   get_prev( void ) const           // Get prior Link
{  return static_cast<T*>(prev); }
}; // class SORT_List<T>::Link -----------------------------------------------

//----------------------------------------------------------------------------
// SORT_List<T>::Constructor/Destructor
//----------------------------------------------------------------------------
   SORT_List( void ) {}
   ~SORT_List( void ) {}

//----------------------------------------------------------------------------
// SORT_List<T>::Methods
//----------------------------------------------------------------------------
void
   fifo(                            // Insert (FIFO order)
     T*                link)        // -> Link to insert
{  B_List::fifo(link); }

T*                                  // -> Head T* on List
   get_head( void ) const           // Get head link
{  return static_cast<T*>(head); }

T*                                  // -> Tail T* on List
   get_tail( void ) const           // Get tail link
{  return static_cast<T*>(tail); }

void
   insert(                          // Insert at position,
     T*                link,        // -> Link to insert after
     T*                head,        // -> First Link to insert
     T*                tail)        // -> Final Link to insert
{  B_List::insert(link, head, tail); }

int                                 // TRUE if the object is coherent
   is_coherent( void ) const        // Coherency check
{  return B_List::is_coherent(); }

int                                 // TRUE if link is contained
   is_on_list(                      // Is Link contained?
     T*                link) const  // -> Link
{  return B_List::is_on_list(link); }

void
   lifo(                            // Insert (LIFO order)
     T*                link)        // -> Link to insert
{  B_List::lifo(link); }

void
   remove(                          // Remove from list
     T*                head,        // -> First Link to remove
     T*                tail)        // -> Final Link to remove
{  B_List::remove(head, tail); }

T*                                  // Removed T*
   remq( void )                     // Remove head link
{  return static_cast<T*>(B_List::remq()); }

T*                                  // -> The set of removed Links
   reset( void )                    // Reset (empty) the List
{  return static_cast<T*>(B_List::reset()); }

void
   sort( void )                     // Sort the List
{  B_List::sort(); }
}; // class SORT_List<T>

//----------------------------------------------------------------------------
//
// Class-
//       List<T>
//
// Purpose-
//       Typed List object, where T is of class List<T>::Link.
//
//----------------------------------------------------------------------------
template<class T>
class List : public DHDL_List<T> {  // List<T>, equivalent to DHDL_List<T>
}; // class List<T>
}  // namespace _PUB_NAMESPACE
#endif // _PUB_LIST_H_INCLUDED
