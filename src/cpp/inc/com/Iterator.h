//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Iterator.h
//
// Purpose-
//       Iterate through elements in some sort of list.
//
// Last change date-
//       2007/01/01
//
// Usage-
//       Iterator<Elem,List> myIter; // A List contains many Elems
//       List                myList; // The container
//       Elem&               myElem; // The element
//
//       for(myIter.begin(myList); myIter.isValid(); myIter.next())
//       {
//         myElem= myIter.current();
//         :
//       }
//
//----------------------------------------------------------------------------
#ifndef ITERATOR_H_INCLUDED
#define ITERATOR_H_INCLUDED

//----------------------------------------------------------------------------
//
// Class-
//       Iterator<Element, Container>
//
// Purpose-
//       An Iterator of class<Container> returns <Element> elements
//
//----------------------------------------------------------------------------
template<class Element, class Container>
class Iterator {                    // Iterator descriptor
//----------------------------------------------------------------------------
// Iterator<Element,Container>::Attributes
//----------------------------------------------------------------------------
Container*             container;   // The class we are iterating
Element*               element;     // The current instance
unsigned long          position;    // Position tag

//----------------------------------------------------------------------------
// Iterator::Constructors
//----------------------------------------------------------------------------
public:
   ~Iterator( void );               // Destructor
   Iterator( void );                // Constructor
   Iterator(                        // Constructor
     const Container&  source);     // Using <Container>

private:                            // Bitwise copy is prohibited
   Iterator(const Iterator&);       // Disallowed copy constructor
Iterator&
   operator=(const Iterator&);      // Disallowed assignment operator

//----------------------------------------------------------------------------
//
// Method-
//       Iterator<Element,Container>::current
//
// Purpose-
//       Retrieve the current element.
//
//----------------------------------------------------------------------------
public:
Element*                            // The current element
   current( void ) const;           // Retrieve the current element

//----------------------------------------------------------------------------
//
// Method-
//       Iterator<Element,Container>::begin
//
// Purpose-
//       Reset the Iterator, beginning at the first element.
//
//----------------------------------------------------------------------------
public:
void
   begin(                           // Reset the Iterator
     const Container&  source);     // Using <Container>

//----------------------------------------------------------------------------
//
// Method-
//       Iterator<Element,Container>::isValid
//
// Purpose-
//       Determine whether the Iterator has been expended.
//
//----------------------------------------------------------------------------
public:
int                                 // TRUE if current() is valid
   isValid( void ) const;           // Is the Iterator finished?

//----------------------------------------------------------------------------
//
// Method-
//       Iterator<Element,Container>::next
//
// Purpose-
//       Position the Iterator at the next element.
//
//----------------------------------------------------------------------------
public:
void
   next( void );                    // Position at the next element
}; // class Iterator

#endif // ITERATOR_H_INCLUDED
