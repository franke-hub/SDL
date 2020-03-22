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
//       ObjectList.h
//
// Purpose-
//       Describe the ObjectList and ObjectList::Link objects.
//
// Last change date-
//       2012/01/01
//
//----------------------------------------------------------------------------
#ifndef OBJECTLIST_H_INCLUDED
#define OBJECTLIST_H_INCLUDED

#include "com/Object.h"

//----------------------------------------------------------------------------
//
// Class-
//       ObjectList
//
// Purpose-
//       The generic ListBase is optimized for FIFO or LIFO operation, and also
//       for insert or removal of element chains.
//
//----------------------------------------------------------------------------
class ObjectList : public Object {  // Generic Linked List
//----------------------------------------------------------------------------
//
// Class-
//       ObjectList::Link
//
// Purpose-
//       The ObjectList::Link object defines ObjectList elements
//       This class is for internal use only.
//
//----------------------------------------------------------------------------
class Link {                        // An ObjectList link
//----------------------------------------------------------------------------
// ObjectList::Link::Attributes
//----------------------------------------------------------------------------
protected:
Link*                  next;        // Next  Link
Link*                  prev;        // Prior Link
Ref<Object>            object;      // Ref<Object> (generic)

//----------------------------------------------------------------------------
// ObjectList::Link::Constructors
//----------------------------------------------------------------------------
public:
inline
   ~Link() {}                       // Default destructor

inline
   Link(                            // Default constructor
     Object*           O)           // Associated Object
:  next(NULL), prev(NULL), object(O) {}

private:                            // Bitwise copy prohibited
   Link(const Link&);
Link&
   operator=(const Link&);

//----------------------------------------------------------------------------
// ObjectList::Link::Accessors
//----------------------------------------------------------------------------
public:
inline Link*                        // -> Link
   getNext( void ) const            // Get next Link
{
   return next;
}

inline Object*                      // The associated Object
   getObject( void ) const          // Get associated Object
{
   return object.get();
}

inline Link*                        // -> Prior Link
   getPrev( void ) const            // Get prior Link
{
   return prev;
}

inline void
   setNext(                         // Set next Link
     Link*             link)        // -> Next Link
{
   next= link;
}

inline void
   setPrev(                         // Set prior Link
     Link*             link)        // -> Prior Link
{
   prev= link;
}
}; // class ObjectList::Link

//----------------------------------------------------------------------------
// ObjectList::Attributes
//----------------------------------------------------------------------------
protected:
Link*                  head;        // -> Head Link
Link*                  tail;        // -> Tail Link

//----------------------------------------------------------------------------
// ObjectList::Constructors
//----------------------------------------------------------------------------
public:
inline
   ~ObjectList( void )              // Default destructor
{  reset();                         // Reset the list
}

inline
   ObjectList( void )               // Default constructor
:  Object(), head(NULL), tail(NULL) {}

private:                            // Bitwise copy prohibited
   ObjectList(const ObjectList&);
ObjectList&
   operator=(const ObjectList&);

//----------------------------------------------------------------------------
// ObjectList::Accessors
//----------------------------------------------------------------------------
public:
inline Object*                      // -> First Object on ObjectList (the oldest)
   getHead( void ) const            // Get head Object
{  Object* result= NULL;

   Link* link= head;
   if( link != NULL)
     result= link->getObject();

   return result;
}

inline Object*                      // -> Last Object on ObjectList (the newest)
   getTail( void ) const            // Get tail Object
{  Object* result= NULL;

   Link* link= tail;
   if( link != NULL)
     result= link->getObject();

   return result;
}

int                                 // TRUE if the object is coherent
   isCoherent( void ) const;        // Coherency check

int                                 // TRUE if Object is on the list
   isOnList(                        // Is Object on list?
     Object*           link) const; // -> Object

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
   fifo(                            // Insert (FIFO order)
     Object*           object);     // -> Object to insert

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
   lifo(                            // Insert (LIFO order)
     Object*           object);     // -> Object to insert

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
   remq( void );                    // Remove oldest Object

//----------------------------------------------------------------------------
//
// Method-
//       ObjectList::reset
//
// Purpose-
//       Reset (empty) the ObjectList.
//
//----------------------------------------------------------------------------
void
   reset( void );                   // Reset the ObjectList

//----------------------------------------------------------------------------
//
// Method-
//       ObjectList::insert
//       ObjectList::remove
//
// Purpose-
//       These interfaces have no meaning unless Links are exposed
//
//----------------------------------------------------------------------------
private:
void
   insert(                          // Add to ObjectList at position,
     Link*             link,        // -> Link to insert after
     Link*             head,        // -> First Link to insert
     Link*             tail);       // -> Final Link to insert

void
   remove(                          // Remove from list
     Link*             head,        // -> First Link to remove
     Link*             tail);       // -> Final Link to remove
}; // class ObjectList

#endif // OBJECTLIST_H_INCLUDED
