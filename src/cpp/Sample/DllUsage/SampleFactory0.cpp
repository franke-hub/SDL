//----------------------------------------------------------------------------
//
//       Copyright (c) 2012 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       SampleFactory0.cpp
//
// Purpose-
//       SampleFactory object methods.
//
// Last change date-
//       2012/01/01
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <com/define.h>
#include <com/Debug.h>
#include "SampleFactory.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static int             constructed= FALSE; // TRUE when constructed
static int             objectConst= FALSE; // TRUE when constructed

//----------------------------------------------------------------------------
//
// Method-
//       SampleFactory::~SampleFactory
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   SampleFactory::~SampleFactory( void ) // Destructor
{
   #ifdef HCDM
     debugf("SampleFactory(%p)::~SampleFactory()\n", this);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       SampleFactory::SampleFactory
//
// Purpose-
//       Default constructor.
//
//----------------------------------------------------------------------------
   SampleFactory::SampleFactory( void ) // Default constructor
:  Factory()
{
   #ifdef HCDM
     debugf("SampleFactory(%p)::SampleFactory()\n", this);
   #endif

   constructed= TRUE;
}

//----------------------------------------------------------------------------
//
// Method-
//       SampleFactory::make
//
// Purpose-
//       Create a SampleFactory::Object.
//
//----------------------------------------------------------------------------
Interface*                          // Actually, a SampleFactory::Object*
   SampleFactory::make( void )      // Create an Interface Object
{
   return new Object();
}

//----------------------------------------------------------------------------
//
// Method-
//       SampleFactory::talk
//
// Purpose-
//       Logic verification method.
//
//----------------------------------------------------------------------------
void
   SampleFactory::talk(             // Logic verification method
     const char*       message) const // Test message
{
   printf("SampleFactory0(%p)::talk(%s) %d\n", this, message, constructed);
}

//----------------------------------------------------------------------------
//
// Method-
//       SampleFactory::Object::~Object
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   SampleFactory::Object::~Object( void ) // Destructor
{
   #ifdef HCDM
     debugf("SampleFactory::Object(%p)::~Object()\n", this);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       SampleFactory::Object::Object
//
// Purpose-
//       Default constructor.
//
//----------------------------------------------------------------------------
   SampleFactory::Object::Object( void ) // Default constructor
:  Interface()
{
   #ifdef HCDM
     debugf("SampleFactory::Object(%p)::Object()\n", this);
   #endif

   objectConst= TRUE;
}

//----------------------------------------------------------------------------
//
// Method-
//       SampleFactory::Object::doSomething
//
// Purpose-
//       Logic verification method.
//
//----------------------------------------------------------------------------
void
   SampleFactory::Object::doSomething(// Logic verification method
     const char*       message) const // Test message
{
   printf("SampleFactory0::Object(%p)::ding(%s) %d\n", this, message, objectConst);
}

