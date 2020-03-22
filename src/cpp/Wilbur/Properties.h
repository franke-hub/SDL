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
//       Properties.h
//
// Purpose-
//       Name/value string pair map.
//
// Last change date-
//       2014/01/01
//
// Implementation notes-
//       The Property name is case insensitive.
//
//----------------------------------------------------------------------------
#ifndef PROPERTIES_H_INCLUDED
#define PROPERTIES_H_INCLUDED

#include <map>
#include <com/istring.h>

#ifndef INTERFACE_H_INCLUDED
#include "Interface.h"
#endif

//----------------------------------------------------------------------------
//
// Class-
//       Properties
//
// Purpose-
//       Name/value string pair map.
//
//----------------------------------------------------------------------------
class Properties : public Interface { // Name/value string pair map
//----------------------------------------------------------------------------
// Properties::Attributes
//----------------------------------------------------------------------------
public:
typedef std::map<istring, std::string> Property;
typedef Property::const_iterator       Iterator;

protected:
Property               propertyMap; // Property map

//----------------------------------------------------------------------------
// Properties::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Properties( void );             // Destructor
   Properties( void );              // Constructor

protected:                          // Disallowed:
   Properties(const Properties&);   // Copy constructor
Properties& operator=(const Properties&); // Assignment operator

//----------------------------------------------------------------------------
// Properties::Methods
//----------------------------------------------------------------------------
public:
virtual Iterator                    // The begin iterator
   begin( void ) const;             // Get begin iterator

virtual Iterator                    // The end iterator
   end( void ) const;               // Get end iterator

virtual std::string                 // The Property value
   delProperty(                     // Delete Property value
     const std::string&name);       // For this Property name

virtual const char*                 // The Property value
   getProperty(                     // Get Property value
     const std::string&name) const; // For this Property name

virtual const char*                 // The Property value
   getProperty(                     // Get Property value
     const std::string&name,        // For this Property name
     const std::string&value) const;// And this default value

virtual void
   setProperty(                     // Set Property
     const std::string&name,        // Property name
     const std::string&value);      // Property value

virtual void
   reset( void );                   // Reset Properties
}; // class Properties

#endif // PROPERTIES_H_INCLUDED
