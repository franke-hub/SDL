//----------------------------------------------------------------------------
//
//       Copyright (c) 2018 Frank Eskesen.
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
//       2018/01/01
//
// Implementation notes-
//       The List destructor does not examine existing Link elements.
//
// Usage notes-
//       class _Link_ : public List<_Link_>::Link { /* Your code here */ };
//       List<_Link_>  list;        // A List of _Link_ elements
//
//----------------------------------------------------------------------------
#ifndef OBJ_LIST_H_INCLUDED
#define OBJ_LIST_H_INCLUDED

#include "Object.h"                 // For _OBJ_NAMESPACE, ...

namespace _OBJ_NAMESPACE {
//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
template<class T> class List;       // General purpose typed List

//----------------------------------------------------------------------------
//
// Class-
//       List
//
// Purpose-
//       List is a doubly headed, doubly linked general purpose List.
//
// Implementation notes-
//       The List is not thread safe. Method usage must be serialized.
//       The FIFO, LIFO, INSERT, and REMOVE methods run in constant time.
//
//----------------------------------------------------------------------------
template<> class List<void> {       // List base
public:
class Link {                        // List<void>::Link
friend class List<void>;            // (Allow set_ method access)

//----------------------------------------------------------------------------
// List::Link::Attributes
//----------------------------------------------------------------------------
protected:
Link*                  next;        // -> Forward Link
Link*                  prev;        // -> Reverse Link

//----------------------------------------------------------------------------
// List::Link::Metods
//----------------------------------------------------------------------------
inline void
   set_next(                        // Set next Link
     Link*             link)        // -> Next Link
{  next= link; }

inline void
   set_prev(                        // Set prior Link
     Link*             link)        // -> Prior Link
{  prev= link; }

public:
inline Link*                        // -> Next Link
   get_next( void ) const           // Get next Link
{  return next; }

inline Link*                        // -> Prior Link
   get_prev( void ) const           // Get prior Link
{  return prev; }
}; // class List<void>::Link

//----------------------------------------------------------------------------
// List::Attributes
//----------------------------------------------------------------------------
protected:
Link*                  head= nullptr; // -> Head Link
Link*                  tail= nullptr; // -> Tail Link

//----------------------------------------------------------------------------
// List::Constructors
//----------------------------------------------------------------------------
public:
   List( void )                     // Default constructor
{  if( false ) debugf("List(%p)::List\n", this); }

   ~List( void )                    // Destructor
{  if( false ) debugf("List(%p)::~List\n", this); }

// Disallowed: Copy constructor, assignment operator
   List(const List&) = delete;
List& operator=(const List&) = delete;

//----------------------------------------------------------------------------
// List::Accessors
//----------------------------------------------------------------------------
public:
inline Link*                        // -> Oldest Link on List
   get_head( void ) const           // Get head link
{  return head; }

inline Link*                        // -> Newest Link on List
   get_tail( void ) const           // Get tail link
{  return tail; }

//----------------------------------------------------------------------------
//
// Method-
//       List::fifo
//
// Purpose-
//       Insert a link onto the list with FIFO ordering.
//
//----------------------------------------------------------------------------
void
   fifo(                            // Insert (FIFO order)
     Link*             link)        // -> Link to insert
{
   link->set_next(nullptr);         // Set next link pointer
   link->set_prev(tail);            // Set prior link pointer

   if( head == nullptr )            // If the list is empty
     head= link;                    // Add link to empty list
   else                             // If the list is not empty
     tail->set_next(link);          // Add link to list

   tail= link;                      // Set new list tail link
}

//----------------------------------------------------------------------------
//
// Method-
//       List::insert
//
// Purpose-
//       Insert a chain of elements onto the list at the specified position.
//
//----------------------------------------------------------------------------
void
   insert(                          // Insert at position,
     Link*             after,       // -> Link to insert after
     Link*             first,       // -> First Link to insert
     Link*             final)       // -> Final Link to insert
{
   if( after == nullptr )           // If insert at head
   {
     first->set_prev(nullptr);
     if( head == nullptr )          // If the list is empty
     {
       final->set_next(nullptr);
       head= first;
       tail= final;
     }
     else                           // If the list is populated
     {
       final->set_next(head);
       head->set_prev(final);
       head= first;
     }
   }
   else                             // If insert into List
   {
     Link* next= after->get_next(); // Address the next Link
     final->set_next(next);         // Set the forward link pointer
     first->set_prev(after);        // Set the reverse link pointer

     after->set_next(first);        // Insert onto the forward list
     if( next == nullptr )          // Insert onto the reverse list
       tail= final;
     else
       next->set_prev(final);
   }
}

void
   insert(                          // Insert at position,
     Link*             after,       // -> Link to insert after
     Link*             link)        // -> The link to insert
{  insert(after, link, link); }

//----------------------------------------------------------------------------
//
// Method-
//       List::is_coherent
//
// Purpose-
//       List coherency check.
//
//----------------------------------------------------------------------------
bool                                // TRUE if the object is coherent
   is_coherent( void ) const        // Coherency check
{
   Link*               link;        // Pointer to current link
   Link*               prev;        // Pointer to prior   link

   link= head;
   prev= nullptr;
   while( link )
   {
     if( link->get_prev() != prev )
       return false;

     if( link->get_next() == nullptr )
       break;

     prev= link;
     link= link->get_next();
   }

   if( tail != link )
     return false;

   return true;
}

//----------------------------------------------------------------------------
//
// Method-
//       List::is_on_list
//
// Purpose-
//       Test whether Link is present in this List.
//
//----------------------------------------------------------------------------
int                                 // TRUE if link is contained
   is_on_list(                      // Is Link contained?
     Link*             link) const  // -> Link
{
   if( link != nullptr )            // If a Link was specified
   {
     Link* next= head;
     while( next != nullptr )
     {
       if( next == link )
         return true;

       next= next->get_next();
     }
   }

   return false;
}

//----------------------------------------------------------------------------
//
// Method-
//       List::lifo
//
// Purpose-
//       Insert a link onto the list with LIFO ordering.
//
//----------------------------------------------------------------------------
void
   lifo(                            // Insert (LIFO order)
     Link*             link)        // -> Link to insert
{
   link->set_next(head);            // Set next link pointer
   link->set_prev(nullptr);         // Set prior link pointer

   if( head == nullptr )            // If the list is empty
     tail= link;                    // Add link to empty list
   else                             // If the list is not empty
     head->set_prev(link);          // Add link to list

   head= link;                      // Set new list head link
}

//----------------------------------------------------------------------------
//
// Method-
//       List::remove
//
// Purpose-
//       Remove a chain of elements from the list.
//
//----------------------------------------------------------------------------
void
   remove(                          // Remove from list
     Link*             first,       // -> First Link to remove
     Link*             final)       // -> Final Link to remove
{
   Link* prev= first->get_prev();   // Link prior to first
   Link* next= final->get_next();   // Link after final

   if( prev == nullptr )
   {
     head= next;
     if( next != nullptr )
       next->set_prev(nullptr);
   }
   else
   {
     prev->set_next(next);
//// first->set_prev(nullptr);      // Not necessary (DO NOT REMOVE COMMENT)
   }

   if( next == nullptr )
   {
     tail= prev;
     if( prev != nullptr )
       prev->set_next(nullptr);
   }
   else
   {
     next->set_prev(prev);
//// final->set_next(nullptr);      // Not necessary (DO NOT REMOVE COMMENT)
   }
}

void
   remove(                          // Remove from List
     Link*             link)        // This Link
{  remove(link, link); }

//----------------------------------------------------------------------------
//
// Method-
//       List::remq
//
// Purpose-
//       Remove the oldest link from the list.
//
//----------------------------------------------------------------------------
Link*                               // -> Removed Link
   remq( void )                     // Remove oldest Link
{
   Link* link= head;                // Address the first Link
   if( link != nullptr )            // If the list is not empty
   {
     head= link->get_next();        // Remove link from list
     if( head != nullptr )          // If the list is not empty
       head->set_prev(nullptr);     // Set first link backchain pointer
     else                           // The list is empty
       tail= nullptr;               // No tail link exists
   }

   return link;                     // Return the oldest link
}

//----------------------------------------------------------------------------
//
// Method-
//       List::reset
//
// Purpose-
//       Remove ALL Links from the List.
//
//----------------------------------------------------------------------------
Link*                               // The set of removed Links
   reset( void )                    // Reset (empty) the list
{
   Link* link= head;                // Resultant

   head= nullptr;
   tail= nullptr;

   return link;
}
}; // class List

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
class List : public List<void> {    // List<T>
public:
class Link : public List<void>::Link { // List<T>::Link
public:
inline T*                           // -> Next Link
   get_next( void ) const           // Get next Link
{  return static_cast<T*>(next); }

inline T*                           // -> Prior Link
   get_prev( void ) const           // Get prior Link
{  return static_cast<T*>(prev); }
}; // class List<void>::Link

//----------------------------------------------------------------------------
// List::Accessors
//----------------------------------------------------------------------------
public:
T*                                  // -> Oldest T* on List
   get_head( void ) const           // Get head link
{  return static_cast<T*>(head); }

T*                                  // -> Newest T* on List
   get_tail( void ) const           // Get tail link
{  return static_cast<T*>(tail); }

T*                                  // Removed T*
   remq( void )                     // Remove oldest link
{  return static_cast<T*>(List<void>::remq()); }

T*                                  // -> The set of removed Links
   reset( void )                    // Reset (empty) the List
{  return static_cast<T*>(List<void>::reset()); }
}; // class List<T>
}  // namespace _OBJ_NAMESPACE

#endif // OBJ_LIST_H_INCLUDED
