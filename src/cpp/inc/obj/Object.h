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
//       Object.h
//
// Purpose-
//       Garbage collected Object, including associated helper objects.
//
// Last change date-
//       2018/01/01
//
// Usage notes-
//       Object.h includes define.h, built_in.h, Exception.h, and Ref.h.
//       Use $include <obj/Object.h> to include these files to avoid
//       circular dependencies.
//
//       Objects are deleted when they are no longer referenced.
//       Objects created in the runtime stack MUST NOT be Pointer referenced;
//       That is, no generic Ref should ever refer to a stack Object.
//
// Implementation note-
//       Garbage counting is performed using reference counting. The reference
//       counter is a 31 bit value, thus limiting any single Object to 2G
//       references to it. (The implementation checks for overflow.)
//
//----------------------------------------------------------------------------
#ifndef OBJ_OBJECT_H_INCLUDED
#define OBJ_OBJECT_H_INCLUDED

#include <atomic>                   // For std::atomic
#include <stdint.h>                 // For int32_t (Precise size required)
#include <string>                   // For std::string

#include "define.h"                 // For _OBJ_NAMESPACE (and more)

namespace _OBJ_NAMESPACE {
//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class Object;                       // The Object base class
template<class T> class Ref_t;      // Typed reference
typedef Ref_t<Object>  Ref;         // Generic reference to Object

//----------------------------------------------------------------------------
//
// Class-
//       Object
//
// Purpose-
//       Object base class.
//
// Implementation notes-
//       Method getClassName() only returns valid results AFTER Object
//       construction completes. It demangles typeid(*this).name() so it is
//       valid for all Objects. Subclass specialization is NOT required.
//
//----------------------------------------------------------------------------
class Object {                      // Object base class
friend class Ref_t<Object>;         // (Only updater of Object::references)

//----------------------------------------------------------------------------
// Object::Attributes
//----------------------------------------------------------------------------
private:
std::atomic<int32_t>   references;  // Number of references to this Object

//----------------------------------------------------------------------------
// Object::Destructor/Constructor/Assignment
//----------------------------------------------------------------------------
public:
virtual
   ~Object( void );                 // Destructor

inline
   Object( void )                   // Default constructor
:  references(0) { }

public:                             // Bitwise copy/assignment allowed
inline
   Object(                          // Copy constructor
     const Object&     source)      // Source Object&
:  references(0) { }

//----------------------------------------------------------------------------
// Object::Operators
//----------------------------------------------------------------------------
inline Object&                      // (Always *this)
   operator=(                       // Assignment operator
     const Object&     source)
{  return *this; }                  // There is nothing to copy

//----------------------------------------------------------------------------
// Object::Accessors
//----------------------------------------------------------------------------
public:
std::string
   get_class_name( void ) const;    // Get the class name

//----------------------------------------------------------------------------
// Object::Methods
//----------------------------------------------------------------------------
public:
virtual int                         // Result (<0,=0,>0)
   compare(                         // Compare to
     const Object&     object) const; // This Object&

virtual size_t                      // A hash code value for this Object
   hashf( void ) const;             // Create hash code value for Object

virtual const std::string           // A String representation of this Object
   string( void ) const;            // Represent this Object as a String
}; // class Object
}  // namespace _OBJ_NAMESPACE

//----------------------------------------------------------------------------
//
// Description-
//       Helper files and built-ins
//
// Purpose-
//       Guaranteed to be included by Object.h
//
//----------------------------------------------------------------------------
#include "built_in.h"               // The Basic built-in functions
#include "Exception.h"              // The Exception Object, for implementation
#include "Ref.h"                    // The Ref Object
#endif // OBJ_OBJECT_H_INCLUDED
