//----------------------------------------------------------------------------
//
//       Copyright (c) 2014-2020 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Zz64Archive.h
//
// Purpose-
//       Define and implement the Zz64Archive object.
//
// Last change date-
//       2020/10/02
//
// Implementation notes-
//       Included from Archive.cpp (NOT)
//       ** NOT IMPLEMENTED **
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Class-
//       Zz64Archive
//
// Purpose-
//       ZIP (64) Archive
//
//----------------------------------------------------------------------------
class Zz64Archive : public Archive { // ZIP64 Archive
protected: // INTERNAL STRUCTURES

protected: // ATTRIBUTES

public: // CONSTRUCTORS
virtual
   ~Zz64Archive( void );            // Destructor
   Zz64Archive(                     // Constructor
     DataSource*       file);       // DataSource

static Zz64Archive*                 // The Zz54Archive
   make(                            // Create Archive
     DataSource*       file);       // From this DataSource

public: // METHODS
virtual const char*                 // The item name (NULL if missing)
   index(                           // Select item number
     unsigned int      index);      // The item index

virtual const char*                 // The next item name
   next( void );                    // Skip to the next item
}; // class Zz64Archive

//----------------------------------------------------------------------------
//
// Method-
//       Zz64Archive::~Zz64Archive
//
// Purpose-
//       Destructor
//
//----------------------------------------------------------------------------
   Zz64Archive::~Zz64Archive( void )// Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       Zz64Archive::Zz64Archive
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
   Zz64Archive::Zz64Archive(        // Constructor
     DataSource*       file)        // DataSource
:  Archive()
{
}

//----------------------------------------------------------------------------
//
// Method-
//       Zz64Archive::make
//
// Purpose-
//       Validation constructor
//
//----------------------------------------------------------------------------
Zz64Archive*                        // Resultant Zz64Achive
   Zz64Archive::make(               // Create Archive from
     DataSource*       file)        // This DataSource
{
   return NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       Zz64Archive::index
//
// Function-
//       Select an item by index
//
//----------------------------------------------------------------------------
const char*                         // The item name
   Zz64Archive::index(              // Select
     unsigned int      index)       // This item index
{
   return NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       Zz64Archive::next
//
// Function-
//       Select the next item
//
//----------------------------------------------------------------------------
const char*                         // The item name
   Zz64Archive::next( void )        // Select the next item
{
   return NULL;
}
