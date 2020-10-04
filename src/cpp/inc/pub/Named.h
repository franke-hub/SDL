//----------------------------------------------------------------------------
//
//       Copyright (C) 2018-2020 Frank Eskesen.
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
//       2020/10/03
//
//----------------------------------------------------------------------------
#ifndef _PUB_NAMED_H_INCLUDED
#define _PUB_NAMED_H_INCLUDED

#include "Object.h"
#include <string>

namespace _PUB_NAMESPACE {
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
virtual
   ~Named( void ) {}                // Destructor

   Named(                           // Constructor
     const char*       name)        // The associated name
:  name(name) {}

   Named(                           // Constructor
     const std::string name)        // The associated name
:  name(name) {}

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
virtual
   ~NamedObject( void ) {}          // Destructor

   NamedObject(                     // Constructor
     const char*       name)        // The associated name
:  Named(name), Object() {}

   NamedObject(                     // Constructor
     const std::string name)        // The associated name
:  Named(name), Object() {}
}; // class Named
}  // namespace _PUB_NAMESPACE
#endif // _PUB_NAMED_H_INCLUDED
