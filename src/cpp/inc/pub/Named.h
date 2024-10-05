//----------------------------------------------------------------------------
//
//       Copyright (C) 2018-2024 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Named.h
//
// Purpose-
//       Define the the Named attribute and the NamedObject class.
//
// Last change date-
//       2024/09/26
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_NAMED_H_INCLUDED
#define _LIBPUB_NAMED_H_INCLUDED

#include <string>                   // For std::string
#include <pub/Object.h>             // For pub::Object.h

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
//----------------------------------------------------------------------------
//
// Class-
//       Named
//
// Purpose-
//       Define the Named attribute
//
//----------------------------------------------------------------------------
class Named {                       // The Named attribute
//----------------------------------------------------------------------------
// Named::Attributes
//----------------------------------------------------------------------------
protected:
const std::string      name;        // The name

//----------------------------------------------------------------------------
// Named::Constructors
//----------------------------------------------------------------------------
public:
   Named( void ) = default;         // Default constructor

   Named(                           // Constructor
     const std::string name)        // The associated name
:  name(name) {}

virtual
   ~Named( void ) = default;        // Destructor

//----------------------------------------------------------------------------
// Named::Accessors
//----------------------------------------------------------------------------
public:
std::string                         // The associated name
   get_name( void ) const           // Get associated name
{  return name; }
}; // class Named

//----------------------------------------------------------------------------
//
// Class-
//       NamedObject
//
// Purpose-
//       Define the NamedObject type
//
//----------------------------------------------------------------------------
class NamedObject : public Named, public Object { // The NamedObject type
//----------------------------------------------------------------------------
// NamedObject::Constructors
//----------------------------------------------------------------------------
public:
   NamedObject( void ) = default;   // Default constructor

   NamedObject(                     // Constructor
     const std::string name)        // The associated name
:  Named(name), Object() {}

virtual
   ~NamedObject( void ) = default;  // Destructor
}; // class Named
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_NAMED_H_INCLUDED
