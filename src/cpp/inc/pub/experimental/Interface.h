//----------------------------------------------------------------------------
//
//       Copyright (c) 2026 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
// SPDX-License-Identifier: LGPL-3.0-only
//----------------------------------------------------------------------------
//
// Title-
//       Interface.h
//
// Purpose-
//       Interface types.
//
// Last change date-
//       2026/01/19
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_INTERFACE_H_INCLUDED
#define _LIBPUB_INTERFACE_H_INCLUDED

#include "pub/bits/pubconfig.h"     // For _LIBPUB_ macros

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
//----------------------------------------------------------------------------
//
// Interface class-
//       IF_Debug
//
// Purpose-
//       The Debug interface.
//
//----------------------------------------------------------------------------
class IF_Debug {                    // The Debug interface
public:
virtual void
   debug(const char* info= nullptr) const; // Informational debug message
}; // class IF_Debug

//----------------------------------------------------------------------------
//
// Class-
//       IF_Object
//
// Purpose-
//       The Object interface
//
//----------------------------------------------------------------------------
class IF_Object {                   // The Object Interface
//----------------------------------------------------------------------------
// IF_Object::Typedefs and enumerations
//----------------------------------------------------------------------------
public:
typedef size_t         hash_t;      // The hash type

//----------------------------------------------------------------------------
// IF_Object::Attributes
//----------------------------------------------------------------------------
private:
// No attributes

//----------------------------------------------------------------------------
// IF_Object::Constructors/destructor
//----------------------------------------------------------------------------
public:
   IF_Object( void ) = default;     // Default constructor
   IF_Object(const IF_Object&) = default; // Copy constructor

virtual
   ~IF_Object( void ) = default;    // Destructor

//----------------------------------------------------------------------------
// IF_Object::operators
//----------------------------------------------------------------------------
IF_Object&
   operator=(const IF_Object&)      // Assignment operator
{  return *this; }

virtual                             // The IF_Object's String representation
   explicit operator std::string( void ) const; // (explicit) cast operator

//----------------------------------------------------------------------------
// IF_Object::Accessors
//----------------------------------------------------------------------------
std::string
   get_class_name( void ) const;    // Get the class name

//----------------------------------------------------------------------------
// IF_Object::Methods
//----------------------------------------------------------------------------
public:
virtual int                         // Result (<0,=0,>0)
   compare(                         // Compare to
     const IF_Object&  IF_Object) const; // This IF_Object&

virtual hash_t                      // The IF_Object's hash code
   hashf( void ) const;             // Get IF_Object's hash code

std::string                         // The IF_Object's String representation
   to_string( void ) const          // Get IF_Object's String representation
{  return operator std::string(); } // (Note: virtual cast operator
}; // class IF_Object

//----------------------------------------------------------------------------
//
// Interface class-
//       IF_Worker
//
// Purpose-
//       The Worker interface.
//
//----------------------------------------------------------------------------
class IF_Worker {                   // The Worker interface
public:
virtual void
   work( void );                    // The Worker method
}; // class IF_Worker
_LIBPUB_END_NAMESPACE

//----------------------------------------------------------------------------
//
// Description-
//       Global IF_Object operators
//
// Purpose-
//       Global comparison operators, global cout << operator.
//
//----------------------------------------------------------------------------
#define _PUB _LIBPUB_NAMESPACE      // (Temporary)
inline bool                         // Resultant
   operator==(                      // Compare (L::R) for equality
     const _PUB::IF_Object& L,      // Left parameter
     const _PUB::IF_Object& R)      // Right paramaeter
{  return (L.compare(R) == 0); }

inline bool                         // Resultant
   operator!=(                      // Compare (L::R) for inequality
     const _PUB::IF_Object& L,      // Left parameter
     const _PUB::IF_Object& R)      // Right paramaeter
{  return (L.compare(R) != 0); }

inline bool                         // Resultant
   operator<=(                      // Compare (L::R) for lesser || equality
     const _PUB::IF_Object& L,      // Left parameter
     const _PUB::IF_Object& R)      // Right paramaeter
{  return (L.compare(R) <= 0);
}

inline bool                         // Resultant
   operator>=(                      // Compare (L::R) for greater || equality
     const _PUB::IF_Object& L,      // Left parameter
     const _PUB::IF_Object& R)      // Right paramaeter
{  return (L.compare(R) >= 0); }

inline bool                         // Resultant
   operator<(                       // Compare (L::R) for lesser
     const _PUB::IF_Object& L,      // Left parameter
     const _PUB::IF_Object& R)      // Right paramaeter
{  return (L.compare(R) < 0); }

inline bool                         // Resultant
   operator>(                       // Compare (L::R) for greater
     const _PUB::IF_Object& L,      // Left parameter
     const _PUB::IF_Object& R)      // Right paramaeter
{  return (L.compare(R) > 0); }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
inline std::ostream&                // (stream)
   operator<<(                      // Append to output stream
     std::ostream&     stream,      // (This stream)
     const _PUB::IF_Object& object) // (This _PUB::IF_Object)
{  return stream << (std::string)object; }
#undef _PUB                         // (Temporary)
#endif // _LIBPUB_INTERFACE_H_INCLUDED
