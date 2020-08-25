//----------------------------------------------------------------------------
//
//       Copyright (c) 2020 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Properties.cpp
//
// Purpose-
//       Properties implementation methods.
//
// Last change date-
//       2020/08/24
//
//----------------------------------------------------------------------------
#include <pub/Exception.h>
#include <pub/utility.h>

#include "pub/Properties.h"
using _PUB_NAMESPACE::utility::to_string;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef SCDM
#undef  SCDM                        // If defined, Soft Core Debug Mode
#endif

#include <pub/ifmacro.h>

namespace _PUB_NAMESPACE {
//----------------------------------------------------------------------------
//
// Method-
//       Properties::~Properties
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Properties::~Properties( void )  // Destructor
{  }

//----------------------------------------------------------------------------
//
// Method-
//       Properties::Properties
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Properties::Properties( void )   // Default constructor
:  map()
{  }

//----------------------------------------------------------------------------
//
// Method-
//       Properties::get_property
//
// Purpose-
//       Get Property value
//
//----------------------------------------------------------------------------
const char*                         // The Property value
   Properties::get_property(        // Get Property value
     const string&     name) const  // For this Property name
{
   MapIter_t mi= map.find(name.c_str());
   if( mi == map.end() )
     return nullptr;

   return mi->second.c_str();
}

const char*                         // The Property value
   Properties::get_property(        // Get Property value
     const string&     name,        // For this Property name
     const string&     value) const // And this default value
{
   const char* result= get_property(name);
   if( result == nullptr )
     result= value.c_str();

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Properties::insert
//
// Purpose-
//       Insert Property.
//
//----------------------------------------------------------------------------
void
   Properties::insert(              // Insert
     const string&     name,        // This Property name and
     const string&     value)       // This Property value
{
   const char* result= get_property(name); // Locate current property
   if( result != nullptr )          // If property already exists
     throw IndexException(to_string("Property exists: %s", name.c_str()));

   map[name]= value;                // Insert the name/value pair
}

//----------------------------------------------------------------------------
//
// Method-
//       Properties::remove
//
// Purpose-
//       Remove Property.
//
//----------------------------------------------------------------------------
void
   Properties::remove(              // Remove
     const string&     name)        // This Property name
{
   MapIter_t mi= map.find(name.c_str());
   if( mi == map.end() )
     throw IndexException(to_string("Missing property: %s", name.c_str()));
   else
     map.erase(mi);
}

//----------------------------------------------------------------------------
//
// Method-
//       Properties::operator=
//
// Purpose-
//       Get Property value
//
//----------------------------------------------------------------------------
const char*                         // The Property value
   Properties::operator[](          // Get Property value
     const string&     name) const  // For this Property name
{
   const char* result = get_property(name);
   if( result == nullptr )
     throw IndexException(to_string("Missing property: %s", name.c_str()));

   return result;
}
} // namespace _PUB_NAMESPACE
