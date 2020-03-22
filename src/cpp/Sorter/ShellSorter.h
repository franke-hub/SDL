//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the MIT license.
//       (See accompanying file LICENSE.MIT or the original contained
//       within https://opensource.org/licenses/MIT)
//
//----------------------------------------------------------------------------
//
// Title-
//       ShellSorter.h
//
// Purpose-
//       Shell sort object.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef SHELLSORTER_H_INCLUDED
#define SHELLSORTER_H_INCLUDED

#ifndef SORTER_H_INCLUDED
#include "Sorter.h"
#endif

//----------------------------------------------------------------------------
//
// Class-
//       ShellSorter
//
// Purpose-
//       Shell sorter.
//
//----------------------------------------------------------------------------
class ShellSorter : public Sorter { // Shell sorter
//----------------------------------------------------------------------------
// ShellSorter::Constructors
//----------------------------------------------------------------------------
public:
   ~ShellSorter( void );            // Destructor
   ShellSorter( void );             // Constructor

//----------------------------------------------------------------------------
// ShellSorter::Methods
//----------------------------------------------------------------------------
public:
virtual const char*                 // The class name
   getClassName( void ) const;      // Get class name

virtual void
   sort(                            // Sort the objects
     unsigned          count,       // Number of elements
     Object**          array);      // Object array

//----------------------------------------------------------------------------
// ShellSorter::Attributes
//----------------------------------------------------------------------------
protected:
   // None defined
}; // class ShellSorter

#endif // SHELLSORTER_H_INCLUDED
