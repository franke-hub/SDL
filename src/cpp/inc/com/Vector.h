//----------------------------------------------------------------------------
//
//       Copyright (c) 2014 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Vector.h
//
// Purpose-
//       The Vector<T> Object is an extendable array of T Object references
//
// Last change date-
//       2014/01/01
//
//----------------------------------------------------------------------------
#ifndef VECTOR_H_INCLUDED
#define VECTOR_H_INCLUDED

#ifndef OBJECT_H_INCLUDED
#include <com/Object.h>
#endif

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
template<class T> class Vector;

//----------------------------------------------------------------------------
//
// Class-
//       Vector<Object>
//
// Purpose-
//       Also used as the Vector<T> base class
//
// Implementation notes-
//       The clone method copies the Object reference array but does not
//       clone the array elements. Until either is changed, a clone and its
//       original refer to the same objects.
//
//----------------------------------------------------------------------------
template<> class Vector<Object> : public Cloneable { // Extendable Array
//----------------------------------------------------------------------------
// Vector<Object>::Attributes
//----------------------------------------------------------------------------
protected:
unsigned               count;       // Number of refs
unsigned               used;        // Number of used references
Ref<Object>*           refs;        // The Object reference array

//----------------------------------------------------------------------------
// Vector<Object>::Constructor/Destructor/Assignment
//----------------------------------------------------------------------------
public:
virtual
   ~Vector<Object>( void );         // Destructor
   Vector<Object>( void );          // Default constructor
   Vector<Object>(                  // Copy constructor
     const Vector<Object>&
                       source);     // Source Vector<Object>

   Vector<Object>(                  // Constructor
     unsigned          available);  // Initial number of elements

//----------------------------------------------------------------------------
// Vector<Object>::Operators
//----------------------------------------------------------------------------
public:
Vector<Object>&                     // Updated reference
   operator=(                       // Assignment operator
     const Vector<Object>&
                       source);     // Source Vector<Object>

Object*                             // -> Object
   operator[](                      // Operator []
     unsigned          index) const;// Index

//----------------------------------------------------------------------------
// Vector<Object>::exceptions
//----------------------------------------------------------------------------
protected:
void
   indexException(                  // Throw indexException
     unsigned          index) const; // For this index

//----------------------------------------------------------------------------
// Vector<Object>::Object methods
//----------------------------------------------------------------------------
public:
virtual int                         // Result (<0,=0,>0)
   compare(                         // Compare to
     const Object&     object) const; // This Object

//----------------------------------------------------------------------------
// Vector<Object>::Cloneable
//----------------------------------------------------------------------------
public:
virtual Cloneable*                  // A clone of this Object
   clone( void ) const              // Create a copy of this Object
{  return new Vector<Object>(*this); } // Create clone using copy constructor

//----------------------------------------------------------------------------
// Vector<Object>::Methods
//----------------------------------------------------------------------------
public:
unsigned                            // The new size
   insert(                          // Insert a new element
     Object*           object);     // -> Object

unsigned                            // Number of used elements
   size( void ) const               // Get element count
{  return used; }
}; // class Vector<Object>

//----------------------------------------------------------------------------
//
// Class-
//       Vector<T>
//
// Purpose-
//       The typed Vector class
//
//----------------------------------------------------------------------------
template<class T> class Vector : public Vector<Object> { // Extendable Array
//----------------------------------------------------------------------------
// Vector<T>::Constructor/Destructor/Assignment
//----------------------------------------------------------------------------
public:
virtual inline
   ~Vector<T>( void ) {}            // Destructor

inline
   Vector<T>( void )                // Default constructor
:  Vector<Object>() {}

inline
   Vector<T>(                       // Copy constructor
     const Vector<T>&  source)      // Source Vector<T>
:  Vector<Object>(source) {}

inline
   Vector<T>(                       // Constructor
     unsigned          available)   // Initial number of elements
:  Vector<Object>(available) {}

inline Vector<T>&                   // Always *this
   operator=(                       // Assignment operator
     const Vector<T>&  source)
{  return Vector<Object>::operator=(source); }

//----------------------------------------------------------------------------
// Vector<T>::Cloneable
//----------------------------------------------------------------------------
public:
virtual Cloneable*                  // A clone of this Object, a Vector<T>
   clone( void ) const              // Create a copy of this Object
{  return new Vector<T>(*this); }   // Create clone using copy constructor

//----------------------------------------------------------------------------
// Vector<T>::Methods
//----------------------------------------------------------------------------
public:
virtual T*                          // -> T
   operator[](                      // Operator []
     unsigned          index) const // Index
{  return (T*)Vector<Object>::operator[](index); }
}; // class Vector<T>

#endif // VECTOR_H_INCLUDED
