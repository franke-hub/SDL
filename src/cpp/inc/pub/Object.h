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
//       ~/pub/Object.h
//
// Purpose-
//       A standard _PUB_OBJECT::Object
//
// Last change date-
//       2018/01/01
//
// Implementation notes-
//       This header is guaranteed to #include "config.h"
//
//----------------------------------------------------------------------------
#ifndef _PUB_OBJECT_H_INCLUDED
#define _PUB_OBJECT_H_INCLUDED

#include <ostream>                  // For std::ostream
#include <string>                   // For std::string

#include "config.h"                 // For _PUB_NAMESPACE, ...

namespace _PUB_NAMESPACE {
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
//----------------------------------------------------------------------------
// Object::Attributes
//----------------------------------------------------------------------------
private:
// No attributes

//----------------------------------------------------------------------------
// Object::Destructor/Constructor/Assignment
//----------------------------------------------------------------------------
public:
virtual
   ~Object( void );                 // Destructor {}
   Object( void ) {}                // Default constructor

   Object(const Object& source) {}  // Default copy constructor
Object& operator=(const Object&) { return *this; } // Default assignment operator

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

virtual                             // A String representation of this Object
   explicit operator std::string( void ) const; // (explicit) Cast operator

inline std::string                  // The String representation of this Object
   to_string( void ) const          // Get String representation of this Object
{  return operator std::string(); }
}; // class Object
}  // namespace _PUB_NAMESPACE

//----------------------------------------------------------------------------
//
// Description-
//       Global operators
//
// Purpose-
//       Global comparison operators, global cout << operator.
//
//----------------------------------------------------------------------------
#define _PUB _PUB_NAMESPACE         // (Temporary)
inline bool                         // Resultant
   operator==(                      // Compare (L::R) for equality
     _PUB::Object&     L,           // Left parameter
     _PUB::Object&     R)           // Right paramaeter
{  return (L.compare(R) == 0); }

inline bool                         // Resultant
   operator!=(                      // Compare (L::R) for inequality
     _PUB::Object&     L,           // Left parameter
     _PUB::Object&     R)           // Right paramaeter
{  return (L.compare(R) != 0); }

inline bool                         // Resultant
   operator<=(                      // Compare (L::R) for lesser || equality
     _PUB::Object&     L,           // Left parameter
     _PUB::Object&     R)           // Right paramaeter
{  return (L.compare(R) <= 0);
}

inline bool                         // Resultant
   operator>=(                      // Compare (L::R) for greater || equality
     _PUB::Object&     L,           // Left parameter
     _PUB::Object&     R)           // Right paramaeter
{  return (L.compare(R) >= 0); }

inline bool                         // Resultant
   operator<(                       // Compare (L::R) for lesser
     _PUB::Object&     L,           // Left parameter
     _PUB::Object&     R)           // Right paramaeter
{  return (L.compare(R) < 0); }

inline bool                         // Resultant
   operator>(                       // Compare (L::R) for greater
     _PUB::Object&      L,          // Left parameter
     _PUB::Object&      R)          // Right paramaeter
{  return (L.compare(R) > 0); }

inline std::ostream&                // (stream)
   operator<<(                      // Append to output stream
     std::ostream&     stream,      // (This stream)
     const _PUB::Object&object)     // (This _PUB::Object)
{  return stream << (std::string)object; }
#undef _PUB
#endif // _PUB_OBJECT_H_INCLUDED
