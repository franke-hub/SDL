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
//       Properties.h
//
// Purpose-
//       Name/value string pair map.
//
// Last change date-
//       2020/06/22
//
// Implementation notes-
//       Property names are case insensitive.
//
//----------------------------------------------------------------------------
#ifndef _PUB_PROPERTIES_H_INCLUDED
#define _PUB_PROPERTIES_H_INCLUDED

#include <map>
#include <string>

#include "config.h"
#include "utility.h"

namespace _PUB_NAMESPACE {
//----------------------------------------------------------------------------
//
// Class-
//       Properties
//
// Purpose-
//       Name/value string pair map.
//
//----------------------------------------------------------------------------
class Properties {                  // Name/value string pair map
//----------------------------------------------------------------------------
// Properties::Attributes
//----------------------------------------------------------------------------
public:
typedef std::string    string;      // Using std::string

typedef std::map<string, string, utility::op_lt_istr>
                       Map_t;       // The Properties Map type
typedef Map_t::const_iterator
                       MapIter_t;   // The Properties Map iterator type

protected:
Map_t                  map;         // The Property map

//----------------------------------------------------------------------------
// Properties::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Properties( void );             // Destructor
   Properties( void );              // Constructor

   Properties(const Properties&) = delete; // DISALLOWED copy constructor
   Properties& operator=(const Properties&) = delete; // DISALLOWED assignment operator

//----------------------------------------------------------------------------
// Properties::Methods
//----------------------------------------------------------------------------
public:
inline MapIter_t                    // The begin iterator
   begin( void ) const              // Get begin iterator
{  return map.begin(); }

inline MapIter_t                    // The end iterator
   end( void ) const                // Get end iterator
{  return map.end(); }

virtual const char*                 // The Property value
   getProperty(                     // Get Property value
     const string&     name) const; // For this Property name

virtual const char*                 // The Property value
   getProperty(                     // Get Property value
     const string&     name,        // For this Property name
     const string&     value) const; // And this default value

virtual void
   insert(                          // Insert
     const string&     name,        // Property name
     const string&     value);      // Property value

virtual void
   remove(                          // Remove
     const string&     name);       // This Property name

inline void
   reset( void )                    // Reset Properties
{  map.clear(); }                   // (Clear the map)

virtual const char*                 // The Property value
   operator[](                      // Get Property value
     const string&     name) const; // For this Property name
}; // class Properties
}  // namespace _PUB_NAMESPACE
#endif // _PUB_PROPERTIES_H_INCLUDED
