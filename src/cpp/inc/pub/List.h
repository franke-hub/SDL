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
//       2020/08/24
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
//       AU_List
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
class Link {                        // AU_List<void>::Link
protected:
Link*                  prev;        // -> Prior Link

public:
inline Link*                        // -> Prior Link
   get_prev( void ) const           // Get prior Link
{  return prev; }

inline void
   set_prev(                        // Set prior Link
     Link*             link)        // -> Prior Link
{  prev= link; }
}; // class AU_List<void>::Link

//----------------------------------------------------------------------------
// AU_List::Attributes
//----------------------------------------------------------------------------
protected:
std::atomic<Link*>     tail;        // -> Tail Link

//----------------------------------------------------------------------------
// AU_List::Constructors
//----------------------------------------------------------------------------
public:
   ~AU_List( void );                // Destructor
   AU_List( void );                 // Constructor

   AU_List(const AU_List&) = delete; // Disallowed copy constructor
AU_List& operator=(const AU_List&) = delete; // Disallowed assignment operator

//----------------------------------------------------------------------------
// AU_List::Accessors
//----------------------------------------------------------------------------
public:
//----------------------------------------------------------------------------
//
// Method-
//       AU_List::get_tail
//
// Purpose-
//       Get the tail Link.
//
// Implementation notes-
//       Only the consumer thread can safely use this method.
//
//----------------------------------------------------------------------------
Link*                               // -> The tail Link
   get_tail( void ) const           // Get tail Link
{  return tail.load(); }

//----------------------------------------------------------------------------
//
// Method-
//       AU_List::fifo
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
//       AU_List::is_coherent
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
//       AU_List::is_on_list
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
//       AU_List::remove
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
//       AU_List::remq
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
//       AU_List::reset
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
//       AU_List::swap
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
//             item->set_prev(nullptr);     // The fake item ends the AU_List
//                                          // (becoming its new head.)
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
}; // class AU_List<void>

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
class Link : public AU_List<void>::Link { // AU_List<T>::Link
public:
inline T*                           // -> Prior Link
   get_prev( void ) const           // Get prior Link
{  return static_cast<T*>(prev); }
}; // class AU_List<T>::Link

public:
T*                                  // -> Tail Link
   get_tail( void ) const           // Get tail link
{  return static_cast<T*>(AU_List<void>::get_tail()); }

T*                                  // -> Prior tail
   fifo(                            // Insert (fifo order)
     T*                link)        // -> Link to insert
{  return static_cast<T*>(AU_List<void>::fifo(link)); }

T*                                  // Removed link (if on list)
   remove(                          // Remove Link
     T*                link)        // The link to remove
{  return static_cast<T*>(AU_List<void>::remove(link)); }

T*                                  // Removed Link
   remq( void )                     // Remove head Link
{  return static_cast<T*>(AU_List<void>::remq()); }

T*                                  // -> The set of removed Links
   reset( void )                    // Reset (empty) the List
{  return static_cast<T*>(AU_List<void>::reset()); }

T*                                  // (See above)
   swap(                            // Replace List
     T*                list)        // With this one
{  return static_cast<T*>(AU_List<void>::swap(list)); }
}; // class AU_List<T>

//----------------------------------------------------------------------------
//
// Class-
//       AU_FIFO
//
// Purpose-
//       A helper class used to improve AU_List::remq performance.
//       (See sample code in AU_List<void>::swap() above.)
//
//----------------------------------------------------------------------------
template<class T>                   // T is an AU_List::Link
class AU_FIFO {                     // AU_FIFO helper class
protected:
T*                     head;        // The current head

public:
//----------------------------------------------------------------------------
// AU_FIFO::Constructor
//----------------------------------------------------------------------------
   AU_FIFO(                         // Constructor
     T*                tail)        // The list
:  head(nullptr)
{  reset(tail); }                   // Initial conversion

//----------------------------------------------------------------------------
// AU_FIFO::remq: Obtain next element
//----------------------------------------------------------------------------
T*                                  // The head Link
   remq( void )                     // Get head Link
{
   T* result= head;

   if( result != nullptr )
     head= result->get_prev();

   return result;
}

//----------------------------------------------------------------------------
// AU_FIFO::reset: (Re-)initialize the list
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
}; // class AU_FIFO

//----------------------------------------------------------------------------
//
// Class-
//       DHDL_List
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
class Link {                        // DHDL_List<void>::Link
protected:
Link*                  next;        // -> Forward Link
Link*                  prev;        // -> Reverse Link

public:
inline Link*                        // -> Next Link
   get_next( void ) const           // Get next Link
{  return next; }

inline Link*                        // -> Prior Link
   get_prev( void ) const           // Get prior Link
{  return prev; }

inline void
   set_next(                        // Set next Link
     Link*             link)        // -> Next Link
{  next= link; }

inline void
   set_prev(                        // Set prior Link
     Link*             link)        // -> Prior Link
{  prev= link; }
}; // class DHDL_List<void>::Link

//----------------------------------------------------------------------------
// DHDL_List::Attributes
//----------------------------------------------------------------------------
protected:
Link*                  head;        // -> Head Link
Link*                  tail;        // -> Tail Link

//----------------------------------------------------------------------------
// DHDL_List::Constructors
//----------------------------------------------------------------------------
public:
   ~DHDL_List( void );              // Default destructor
   DHDL_List( void );               // Default constructor

private:                            // Bitwise copy prohibited
   DHDL_List(const DHDL_List&);
DHDL_List&
   operator=(const DHDL_List&);

//----------------------------------------------------------------------------
// DHDL_List::Accessors
//----------------------------------------------------------------------------
public:
inline Link*                        // -> Head Link on List
   get_head( void ) const           // Get head link
{  return head; }

inline Link*                        // -> Tail Link on List
   get_tail( void ) const           // Get tail link
{  return tail; }

//----------------------------------------------------------------------------
//
// Method-
//       DHDL_List::fifo
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
//       DHDL_List::insert
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
//       DHDL_List::is_coherent
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
//       DHDL_List::is_on_list
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
//       DHDL_List::lifo
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
//       DHDL_List::remove
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
//       DHDL_List::remq
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
//       DHDL_List::reset
//
// Purpose-
//       Remove ALL Links from the List.
//
//----------------------------------------------------------------------------
Link*                               // The set of removed Links
   reset( void );                   // Reset (empty) the list
}; // class DHDL_List

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
class Link : public DHDL_List<void>::Link { // DHDL_List<T>::Link
public:
inline T*                           // -> Next Link
   get_next( void ) const           // Get next Link
{  return static_cast<T*>(next); }

inline T*                           // -> Prior Link
   get_prev( void ) const           // Get prior Link
{  return static_cast<T*>(prev); }
}; // class DHDL_List<void>::Link

public:
T*                                  // -> Head T* on List
   get_head( void ) const           // Get head link
{  return static_cast<T*>(head); }

T*                                  // -> Tail T* on List
   get_tail( void ) const           // Get tail link
{  return static_cast<T*>(tail); }

T*                                  // Removed T*
   remq( void )                     // Remove head link
{  return static_cast<T*>(DHDL_List<void>::remq()); }

T*                                  // -> The set of removed Links
   reset( void )                    // Reset (empty) the List
{  return static_cast<T*>(DHDL_List<void>::reset()); }
}; // class DHDL_List<T>

//----------------------------------------------------------------------------
//
// Class-
//       DHSL_List
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
class Link {                        // DHSL_List<void>::Link
protected:
Link*                  next;        // -> Forward Link

public:
inline Link*                        // -> Next Link
   get_next( void ) const           // Get next Link
{  return next; }

inline void
   set_next(                        // Set next Link
     Link*             link)        // -> Next Link
{  next= link; }
}; // class DHSL_List<void>::Link

//----------------------------------------------------------------------------
// DHSL_List::Attributes
//----------------------------------------------------------------------------
protected:
Link*                  head;        // -> Head Link
Link*                  tail;        // -> Tail Link

//----------------------------------------------------------------------------
// DHSL_List::Constructors
//----------------------------------------------------------------------------
public:
   ~DHSL_List( void );              // Default destructor
   DHSL_List( void );               // Default constructor

private:                            // Bitwise copy prohibited
   DHSL_List(const DHSL_List&);
DHSL_List&
   operator=(const DHSL_List&);

//----------------------------------------------------------------------------
// DHSL_List::Accessors
//----------------------------------------------------------------------------
public:
inline Link*                        // -> Head Link on List
   get_head( void ) const           // Get head Link
{  return head; }

inline Link*                        // -> Tail Link on List
   get_tail( void ) const           // Get tail Link
{  return tail; }

//----------------------------------------------------------------------------
//
// Method-
//       DHSL_List::fifo
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
//       DHSL_List::insert
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
//       DHSL_List::is_coherent
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
//       DHSL_List::is_on_list
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
//       DHSL_List::lifo
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
//       DHSL_List::remove
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
//       DHSL_List::remq
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
//       DHSL_List::reset
//
// Purpose-
//       Remove ALL Links from the List.
//
//----------------------------------------------------------------------------
Link*                               // The set of removed Links
   reset( void );                   // Reset (empty) the List
}; // class DHSL_List<void>

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
class DHSL_List : public DHSL_List<void> {// DHSL_List<T>
public:
class Link : public DHSL_List<void>::Link { // DHSL_List<T>::Link
public:
inline T*                           // -> Next Link
   get_next( void ) const           // Get next Link
{  return static_cast<T*>(next); }
}; // class DHSL_List<T>::Link

public:
T*                                  // -> Head T* on List
   get_head( void ) const           // Get head Link
{  return static_cast<T*>(head); }

T*                                  // -> Tail T* on List
   get_tail( void ) const           // Get tail Link
{  return static_cast<T*>(tail); }

T*                                  // Removed T*
   remq( void )                     // Remove head Link
{  return static_cast<T*>(DHSL_List<void>::remq()); }

T*                                  // -> The set of removed Links
   reset( void )                    // Reset (empty) the List
{  return static_cast<T*>(DHSL_List<void>::reset()); }
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
class NODE_List : public DHDL_List<T> { // Node_List<T>
public:
class Link : public DHDL_List<T>::Link { // NODE_List<T>::Link
protected:
NODE_List<T>*          parent;      // The parent Link

public:
inline NODE_List<T>*                // The parent List
   get_parent(void) const           // Get parent List
{  return parent; }

inline void
   set_parent(                      // Set parent Link
     NODE_List<T>*     list)        // To this Link
{  parent= list; }
}; // class NODE_List<T>::Link

public:
void
   fifo(                            // Insert (FIFO order)
     Link*             link)        // -> Link to insert
{
   DHDL_List<T>::fifo(link);
   link->set_parent(this);
}

void
   insert(                          // Insert at position,
     Link*             link,        // -> Link to insert after
     Link*             head,        // -> First Link to insert
     Link*             tail)        // -> Final Link to insert
{
   DHDL_List<T>::insert(link, head, tail);
   for(;;) {
     head->set_parent(this);
     if( head == tail )
       break;
     head= head->get_next();
   }
}

void
   lifo(                            // Insert (LIFO order)
     Link*             link)        // -> Link to insert
{
   DHDL_List<T>::lifo(link);
   link->set_parent(this);
}

void
   remove(                          // Remove from list
     Link*             head,        // -> First Link to remove
     Link*             tail)        // -> Final Link to remove
{
   DHDL_List<T>::remove(head, tail);

   for(;;) {
     head->set_parent(nullptr);
     if( head == tail )             // (No error checking!)
       break;
     head= head->get_next();
   }
}

T*                                  // Removed T*
   remq( void )                     // Remove head link
{
   T* link= DHDL_List<T>::remq();
   if( link )
     link->set_parent(nullptr);
   return link;
}

T*                                  // -> The set of removed Links
   reset( void )                    // Reset (empty) the List
{
   T* link= DHDL_List<T>::reset();
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
//       SHSL_List
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
class Link {                        // SHSL_List<void>::Link
protected:
Link*                  next;        // -> Next Link

public:
inline Link*                        // -> Next Link
   get_next( void ) const           // Get next Link
{  return next; }

inline void
   set_next(                        // Set next Link
     Link*             link)        // -> Next Link
{  next= link; }
}; // class SHSL_List<void>::Link

//----------------------------------------------------------------------------
// SHSL_List::Attributes
//----------------------------------------------------------------------------
protected:
Link*                  head;        // -> Head Link

//----------------------------------------------------------------------------
// SHSL_List::Constructors
//----------------------------------------------------------------------------
public:
   ~SHSL_List( void );              // Default destructor
   SHSL_List( void );               // Default constructor

private:                            // Bitwise copy prohibited
   SHSL_List(const SHSL_List&);
SHSL_List&
   operator=(const SHSL_List&);

//----------------------------------------------------------------------------
// SHSL_List::Accessors
//----------------------------------------------------------------------------
public:
inline Link*                        // -> Head Link on List
   get_head( void ) const           // Get head Link
{  return head; }

//----------------------------------------------------------------------------
//
// Method-
//       SHSL_List::is_coherent
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
//       SHSL_List::is_on_list
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
//       SHSL_List::fifo
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
//       SHSL_List::insert
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
//       SHSL_List::lifo
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
//       SHSL_List::remove
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
//       SHSL_List::remq
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
//       SHSL_List::reset
//
// Purpose-
//       Remove ALL Links from the List.
//
//----------------------------------------------------------------------------
Link*                               // The set of removed Links
   reset( void );                   // Reset (empty) the List
}; // class SHSL_List<void>

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
class Link : public SHSL_List<void>::Link { // SHSL_List<T>::Link
public:
inline T*                           // -> Next Link
   get_next( void ) const           // Get next Link
{  return static_cast<T*>(next); }
}; // class SHSL_List<T>::Link

public:
T*                                  // -> Head T* on List
   get_head( void ) const           // Get head link
{  return static_cast<T*>(head); }

T*                                  // Removed T*
   remq( void )                     // Remove head link
{  return static_cast<T*>(SHSL_List<void>::remq()); }

T*                                  // -> The set of removed Links
   reset( void )                    // Reset (empty) the List
{  return static_cast<T*>(SHSL_List<void>::reset()); }
}; // class DHSL_List<T>

//----------------------------------------------------------------------------
//
// Class-
//       SORT_List<void>
//
// Purpose-
//       The SORT_List is a sortable DHDL_List.
//
// Implementation notes-
//       See DHDL_List for general implementation notes.
//
//       A SORT_List is in sorted order (from lowest to highest) only after
//       the sort method is invoked. If Links are added to the List, the
//       List remains potentially out of sort order until the sort method
//       is invoked (again.)
//
//       The DHDL_List base is private, making the SORT_List appear to be
//       a separate class. This prevents DHDL_List::Link objects from being
//       added to a SORT_List at compile time, which is useful.
//
//----------------------------------------------------------------------------
template<> class SORT_List<void> : private DHDL_List<void> { // SORT_List
typedef DHDL_List<void>::Link DHDL_Link; // Internal shorthand
public:
class Link : private DHDL_Link {    // SORT_List<void>::Link
public:
virtual int                         // Result (<0, =0, >0)
   compare(                         // Compare to
     const Link*       that) const = 0; // This Link

inline Link*                        // -> Next Link
   get_next( void ) const           // Get next Link
{  return static_cast<Link*>(next); }

inline Link*                        // -> Prior Link
   get_prev( void ) const           // Get prior Link
{  return static_cast<Link*>(prev); }

inline void
   set_next(                        // Set next Link
     Link*             link)        // -> Next Link
{  next= link; }

inline void
   set_prev(                        // Set prior Link
     Link*             link)        // -> Prior Link
{  prev= link; }
}; // class SORT_List<void>::Link

//----------------------------------------------------------------------------
// SORT_List::Constructors
//----------------------------------------------------------------------------
public:
inline
   ~SORT_List( void ) {}            // Default destructor
inline
   SORT_List( void )                // Default constructor
:  DHDL_List<void>() {}             // (Construct DHDL_List base)

//----------------------------------------------------------------------------
// SORT_List::Accessors
//----------------------------------------------------------------------------
public:
Link*                               // -> Head Link on List
   get_head( void ) const           // Get head link
{  return (Link*)head; }

Link*                               // -> Tail Link on List
   get_tail( void ) const           // Get tail link
{  return (Link*)tail; }

//----------------------------------------------------------------------------
// SORT_List::Methods
//----------------------------------------------------------------------------
inline void
   fifo(                            // Insert (FIFO order)
     Link*             link)        // -> Link to insert
{  DHDL_List<void>::fifo((DHDL_Link*)link); }

inline void
   insert(                          // Insert at position,
     Link*             link,        // -> Link to insert after
     Link*             head,        // -> First Link to insert
     Link*             tail)        // -> Final Link to insert
{  DHDL_List<void>::insert((DHDL_Link*)link, (DHDL_Link*)head,
                           (DHDL_Link*)tail);
}

int                                 // TRUE if the object is coherent
   is_coherent( void ) const        // Coherency check
{  return DHDL_List<void>::is_coherent(); }

int                                 // TRUE if link is contained
   is_on_list(                      // Is Link contained?
     Link*             link) const  // -> Link
{  return DHDL_List<void>::is_on_list((DHDL_Link*)link); }

inline void
   lifo(                            // Insert (LIFO order)
     Link*             link)        // -> Link to insert
{  DHDL_List<void>::lifo((DHDL_Link*)link); }

inline void
   remove(                          // Remove from list
     Link*             head,        // -> First Link to remove
     Link*             tail)        // -> Final Link to remove
{  DHDL_List<void>::remove((DHDL_Link*)head, (DHDL_Link*)tail); }

inline Link*                        // -> Removed Link
   remq( void )                     // Remove head (lowest valued) Link
{  return (Link*)DHDL_List<void>::remq(); }

inline Link*                        // -> The set of removed Links
   reset( void )                    // Reset (empty) the List
{  return (Link*)DHDL_List<void>::reset(); }

//----------------------------------------------------------------------------
//
// Method-
//       SORT_List::sort
//
// Purpose-
//       Sort the list. After this operation, the list is sorted.
//       Sort runs in polynomial time, currently implemented as a bubble sort.
//
//----------------------------------------------------------------------------
public:
void
   sort( void );                    // Sort the list
}; // class SORT_List

//----------------------------------------------------------------------------
//
// Class-
//       SORT_List<T>
//
// Purpose-
//       Typed SORT_List object, where T is of class SORT_List<T>::Link.
//
//----------------------------------------------------------------------------
template<class T>
class SORT_List : public SORT_List<void> { // SORT_List<T>
public:
class Link : public SORT_List<void>::Link { // SORT_List<T>::Link
public:
//----------------------------------------------------------------------------
// This method MUST BE supplied. There is no default implementation.
//
// Code format:
//     template<>
//     SORT_List<T>::Link::compare(const SORT_List<void>::Link* that) const
//     {
//       // In this implementation,
//       // use: static_cast<const T*>(this)->  (to refer to this object)
//       // and: static_cast<const T*>(that)->  (to refer to that object)
//
//       // Note: even though you can static_cast<const T*>(this), this method
//       // is not declared within or even considered part of class T itself.
//       // (It's actually a member of class T's base class, so you can
//       // override this method in class T. You'll still need to declare an
//       // implementation here.)
//       return 0; // Will suffice for the default in that case
//     }
//
// To override the base class implementation, in class T code:
//     virtual int compare(const SORT_List<void>::Link* that) const
//     { implementation; }
//
//----------------------------------------------------------------------------
virtual int                         // Result (<0, =0, >0)
   compare(                         // Compare to
     const SORT_List<void>::Link*
                       that) const; // This (class T) Link

inline T*                           // -> Next Link
   get_next( void ) const           // Get next Link
{  return static_cast<T*>(SORT_List<void>::Link::get_next()); } // (No direct access to next)

inline T*                           // -> Prior Link
   get_prev( void ) const           // Get prior Link
{  return static_cast<T*>(SORT_List<void>::Link::get_prev()); } // (No direct access to prev)
}; // class SORT_List<void>::Link

public:
T*                                  // -> Head T* on List
   get_head( void ) const           // Get head link
{  return static_cast<T*>(SORT_List<void>::get_head()); } // (No direct access to head)

T*                                  // -> Tail T* on List
   get_tail( void ) const           // Get tail link
{  return static_cast<T*>(SORT_List<void>::get_tail()); } // (No direct access to tail)

T*                                  // Removed T*
   remq( void )                     // Remove head (lowest valued) link
{  return static_cast<T*>(SORT_List<void>::remq()); }

T*                                  // -> The set of removed Links
   reset( void )                    // Reset (empty) the List
{  return static_cast<T*>(SORT_List<void>::reset()); }
}; // class SORT_List<T>

//----------------------------------------------------------------------------
//
// Class-
//       List<T>
//
// Purpose-
//       Typed List object, where T is of class List<T>::Link.
//
// Implementation notes-
//       This is a DHDL_List. See the associated notes.
//
//----------------------------------------------------------------------------
template<class T>
class List : public DHDL_List<T> {  // List<T>, equivalent to DHDL_List<T>
}; // class List<T>
}  // namespace _PUB_NAMESPACE
#endif // _PUB_LIST_H_INCLUDED
