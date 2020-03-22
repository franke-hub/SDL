//----------------------------------------------------------------------------
//
//       Copyright (c) 2010-2014 Frank Eskesen.
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
//       2014/01/01
//
//----------------------------------------------------------------------------
#include "Common.h"
#include "Properties.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef SCDM
#undef  SCDM                        // If defined, Soft Core Debug Mode
#endif

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
{
}

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
:  Interface()
,  propertyMap()
{
}

//----------------------------------------------------------------------------
//
// Method-
//       Properties::begin/end
//
// Purpose-
//       Get propertyMap iterators
//
//----------------------------------------------------------------------------
Properties::Iterator                // The Properties iterator
   Properties::begin( void ) const  // Get Properties iterator
{
   return propertyMap.begin();
}

Properties::Iterator                // The Properties iterator
   Properties::end( void ) const    // Get Properties iterator
{
   return propertyMap.end();
}

//----------------------------------------------------------------------------
//
// Method-
//       Properties::delProperty
//
// Purpose-
//       Delete Property.
//
//----------------------------------------------------------------------------
std::string                         // The Property value
   Properties::delProperty(         // Delete Property
     const std::string&name)        // For this Property name
{
   std::string         result;      // Resultant (default "")

   Property::iterator mi= propertyMap.find(istring(name.c_str()));
   if( mi != propertyMap.end() )
   {
     result= mi->second;
     propertyMap.erase(mi);
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Properties::getProperty
//
// Purpose-
//       Get Property value
//
//----------------------------------------------------------------------------
const char*                         // The Property value
   Properties::getProperty(         // Get Property value
     const std::string&name) const  // For this Property name
{
   Iterator mi= propertyMap.find(istring(name.c_str()));
   if( mi == propertyMap.end() )
     return NULL;

   return mi->second.c_str();
}

const char*                         // The Property value
   Properties::getProperty(         // Get Property value
     const std::string&name,        // For this Property name
     const std::string&value) const // And this default value
{
   Iterator mi= propertyMap.find(istring(name.c_str()));
   if( mi == propertyMap.end() )
     return value.c_str();

   return mi->second.c_str();
}

//----------------------------------------------------------------------------
//
// Method-
//       Properties::setProperty
//
// Purpose-
//       Set Property value
//
//----------------------------------------------------------------------------
void
   Properties::setProperty(         // Set Property value
     const std::string&name,        // Associate this Property name
     const std::string&value)       // With this Property value
{
   propertyMap[istring(name.c_str())]= value;
}

//----------------------------------------------------------------------------
//
// Method-
//       Properties::reset
//
// Purpose-
//       Reset Properties
//
//----------------------------------------------------------------------------
void
   Properties::reset( void )        // Reset Properties
{
   propertyMap.clear();
}

