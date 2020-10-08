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
//       ShellSorter.cpp
//
// Purpose-
//       Shell sorter.
//
// Last change date-
//       2007/01/01
//
// Notes-
//       An OK but stack intensive sorter.
//       Because of memory usage, not good for very large arrays
//
//----------------------------------------------------------------------------
#include <stdio.h>

#include "ShellSorter.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "SHELL   " // Source file

//----------------------------------------------------------------------------
//
// Method-
//       ShellSorter::~ShellSorter
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   ShellSorter::~ShellSorter( void ) // Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       ShellSorter::ShellSorter
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   ShellSorter::ShellSorter( void )   // Constructor
:  Sorter()
{
}

//----------------------------------------------------------------------------
//
// Method-
//       ShellSorter::getClassName
//
// Purpose-
//       Get the class name.
//
//----------------------------------------------------------------------------
const char*                         // The class name
   ShellSorter::getClassName( void ) const
{
   return "ShellSorter";
}

//----------------------------------------------------------------------------
//
// Method-
//       ShellSorter::sort
//
// Purpose-
//       Sort an array.
//
//----------------------------------------------------------------------------
void
   ShellSorter::sort(               // Sort an Object array
     unsigned          count,       // Number of elements
     Object**          array)       // Object array
{
   Object*             object;      // Temporary

   unsigned increment= 3;
   while( increment > 0 )
   {
     for(unsigned i= 0; i<count; i++)
     {
       unsigned j= i;
       object= array[i];
       while( j >= increment && array[j-increment]->compare(object) > 0 )
       {
         array[j]= array[j-increment];
         j -= increment;
       }
       array[j]= object;
     }

     increment /= 2;
   }
}

