//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the MIT license.
//       (See accompanying file LICENSE.MIT or the original contained
//       within https://opensource.org/licenses/MIT)
//
//----------------------------------------------------------------------------
//
// Title-
//       Object.h
//
// Purpose-
//       Sortable Object.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef OBJECT_H_INCLUDED
#define OBJECT_H_INCLUDED

//----------------------------------------------------------------------------
//
// Class-
//       Object
//
// Purpose-
//       Object base class.
//
//----------------------------------------------------------------------------
class Object {                      // Object base class
//----------------------------------------------------------------------------
// Object::Constructors
//----------------------------------------------------------------------------
public:
   ~Object( void );                 // Destructor
   Object(                          // Constructor
     unsigned          value);      // Number

//----------------------------------------------------------------------------
// Object::Methods
//----------------------------------------------------------------------------
public:
virtual int                         // Comparitor (<0, =0, >0)
   compare(                         // Compare this object
     Object*           source);     // To this Object

//----------------------------------------------------------------------------
// Object::Attributes
//----------------------------------------------------------------------------
protected:
   Object*             valid;       // Validator
   unsigned const      value;       // Value
}; // class Object

#endif // OBJECT_H_INCLUDED
