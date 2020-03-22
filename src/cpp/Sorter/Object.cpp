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
//       Object.cpp
//
// Purpose-
//       Sortable object abstact base class.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "Object.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "OBJECT  " // Source file

#ifndef HCDM
#define HCDM                        // If defined, Hard Core Debug Mode
#endif

//----------------------------------------------------------------------------
//
// Method-
//       Object::~Object
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Object::~Object( void )          // Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       Object::Object
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Object::Object(                  // Constructor
     unsigned int      value)
:  value(value)
{
   valid= this;
}

//----------------------------------------------------------------------------
//
// Method-
//       Object::compare
//
// Purpose-
//       Compare objects
//
//----------------------------------------------------------------------------
int                                 // Comparitor (<0, =0, >0)
   Object::compare(                 // Compare this object
     Object*           source)      // To this Object
{
   #ifdef HCDM
     if( this == NULL || source == NULL )
     {
       fprintf(stderr, "Object(%p)::compare(%p) NULL pointer\n",
                       this, source);
       fflush(stderr);
       abort();
     }

     if( this->valid != this )
     {
       fprintf(stderr, "Object(%p)::compare(%p) invalid(%p)\n",
                       this, source, this);
       fflush(stderr);
       abort();
     }

     if( source->valid != source )
     {
       fprintf(stderr, "Object(%p)::compare(%p) invalid(%p)\n",
                       this, source, source);
       fflush(stderr);
       abort();
     }
   #endif

   return (signed)value - (signed)source->value;
}

