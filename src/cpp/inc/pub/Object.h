//----------------------------------------------------------------------------
//
//       Copyright (c) 2018-2024 Frank Eskesen.
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
//       The base object class.
//
// Last change date-
//       2024/09/26
//
// Implementation notes-
//       All pub library header files are guaranteed to #include "config.h"
//       either directly or indirectly.
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_OBJECT_H_INCLUDED
#define _LIBPUB_OBJECT_H_INCLUDED

#include <ostream>                  // For std::ostream
#include <string>                   // For std::string

#include "config.h"                 // For _LIBPUB_ macros

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
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
// Object::Constructors/destructor
//----------------------------------------------------------------------------
public:
   Object( void ) = default;        // Default constructor
   Object(const Object&) = default; // Copy constructor

virtual
   ~Object( void ) = default;       // Destructor

//----------------------------------------------------------------------------
// Object::operators
//----------------------------------------------------------------------------
Object&
   operator=(const Object&)         // Assignment operator
{  return *this; }

virtual                             // A String representation of this Object
   explicit operator std::string( void ) const; // (explicit) Cast operator

//----------------------------------------------------------------------------
// Object::Accessors
//----------------------------------------------------------------------------
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

inline std::string                  // The String representation of this Object
   to_string( void ) const          // Get String representation of this Object
{  return operator std::string(); }
}; // class Object
_LIBPUB_END_NAMESPACE

//----------------------------------------------------------------------------
//
// Description-
//       Global operators
//
// Purpose-
//       Global comparison operators, global cout << operator.
//
//----------------------------------------------------------------------------
#define _PUB _LIBPUB_NAMESPACE      // (Temporary)
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
#endif // _LIBPUB_OBJECT_H_INCLUDED
