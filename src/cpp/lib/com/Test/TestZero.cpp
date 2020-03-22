//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       TestZero.cpp
//
// Purpose-
//       Test the Zeroed object.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <stdlib.h>
#include <string.h>

#include <com/Debug.h>
#include <com/Verify.h>

#include "com/Zeroed.h"

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
// Class-
//       IsZeroed
//
// Purpose-
//       Zeroed object superclass.
//
//----------------------------------------------------------------------------
class IsZeroed : public Zeroed {    // The IsZeroed test Object
//----------------------------------------------------------------------------
// IsZeroed::Typedefs and enumerations
//----------------------------------------------------------------------------
protected:
enum
{  DIM_ARRAY= 4096                  // The array size
}; // enum

//----------------------------------------------------------------------------
// IsZeroed::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~IsZeroed( void )                // Destructor
{
   #ifdef SCDM
     debugf("IsZeroed(%p)::~IsZeroed()\n", this);
   #endif
}

   IsZeroed( void )                 // Default constructor
{
   #ifdef SCDM
     debugf("IsZeroed(%p)::IsZeroed()\n", this);
   #endif
}

//----------------------------------------------------------------------------
// IsZeroed::Methods
//----------------------------------------------------------------------------
public:
int                                 // TRUE iff no errors found
   isValid( void ) const            // Is the object really zeroed?
{
   int                 resultant= TRUE;
   int                 i;

   #ifdef SCDM
     debugf("IsZeroed(%p)::isValid()\n", this);
   #endif

   for(i= 0; i<DIM_ARRAY; i++)
   {
     if( array[i] != 0 )
     {
       debugf("Element %d non-zero", i);
       resultant= FALSE;
       break;
     }
   }

   return resultant;
}

//----------------------------------------------------------------------------
// IsZeroed::Attributes
//----------------------------------------------------------------------------
protected:
   int                 array[DIM_ARRAY]; // The Zeroed data
}; // class IsZeroed

//----------------------------------------------------------------------------
//
// Subroutine-
//       testZeroed
//
// Purpose-
//       Test the Zeroed object function.
//
//----------------------------------------------------------------------------
void
   testZeroed( void )               // Test Zeroed object functions
{
   debugf("\n");
   verify_info(); debugf("testZeroed()\n");

   IsZeroed*           object;
   IsZeroed            staticObject;
   Zeroed*             zeroed;

   // Allocate and release a Zeroed object
   zeroed= new Zeroed();
   delete zeroed;

   // Allocate, test and release an IsZeroed object
   object= new IsZeroed();
   verify( object->isValid() );
   delete object;

   // Test the IsZeroed static object
   object= new(&staticObject) IsZeroed();
   verify( object->isValid() );
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       main
//
// Purpose-
//       Mainline code.
//
//----------------------------------------------------------------------------
extern int
   main(                            // Mainline code
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   testZeroed();
   verify_exit();
}

