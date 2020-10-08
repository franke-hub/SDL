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
//       Loader.cpp
//
// Purpose-
//       Loader object methods.
//
// Last change date-
//       2012/01/01
//
//----------------------------------------------------------------------------
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <com/Debug.h>

#include "Loader.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#include <com/ifmacro.h>

//----------------------------------------------------------------------------
//
// Method-
//       Loader::~Loader
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Loader::~Loader( void )          // Destructor
{
   IFHCDM( debugf("Loader(%p)::~Loader()\n", this); )

   makef= NULL;
   takef= NULL;

   #ifdef _OS_BSD
     if( handle != NULL )
     {
////   debugf("%4d dlclose(%s)\n", __LINE__, name.c_str()); // Info only
       dlclose(handle);
       handle= NULL;
     }
   #endif

   #ifdef _OS_WIN
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       Loader::Loader
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Loader::Loader(                  // Constructor
     const char*       name)        // Name of library
:  name(name)
,  handle(NULL)
,  makef(NULL), takef(NULL)
{
   IFHCDM( debugf("Loader(%p)::Loader(%s)\n", this, name); )

   #ifdef _OS_BSD
//// debugf("%4d dlopen(%s)\n", __LINE__, name); // Can hang
     handle= dlopen(name, RTLD_LAZY);
//// debugf("%4d ..DONE\n", __LINE__);
     if( handle == NULL )
       throwf("%4d Loader,dlopen(%s): %s", __LINE__, name, dlerror());

     // Per dlsym documentation
     // makef= (Makef)dlsym(handle, "DLL_make");
     *(void**)(&makef)= dlsym(handle, "DLL_make");
     if( makef == NULL)
       throwf("%4d Loader,dlsym(%s),DLL_make: %s", __LINE__, name, dlerror());

     // Per dlsym documentation
     // takef= (Takef)dlsym(handle, "DLL_take");
     *(void**)(&takef)= dlsym(handle, "DLL_take");
     if( takef == NULL)
       throwf("%4d Loader,dlsym(%s),DLL_take: %s", __LINE__, name, dlerror());
   #endif

   #ifdef _OS_WIN
   #endif

#if 0
   IFHCDM(
     debugf("%4d Loader(%s) @x%.8lx @x%.8lx @x%.8lx\n", __LINE__, name,
            (long)handle, (long)makef, (long)takef);
   )
#endif
}

//----------------------------------------------------------------------------
//
// Method-
//       Loader::debug
//
// Purpose-
//       Debugging display.
//
//----------------------------------------------------------------------------
void
   Loader::debug( void )            // Debugging display
{
   debugf("%8lx handle\n", (long)handle);
   debugf("%8lx makef\n", (long)makef);
   debugf("%8lx takef\n", (long)takef);
}

//----------------------------------------------------------------------------
//
// Method-
//       Loader::make
//
// Purpose-
//       Create an Interface Object.
//
//----------------------------------------------------------------------------
Interface*                          // Resultant Interface Object
   Loader::make( void )             // Create an Interface Object
{
   return (*makef)();
}

//----------------------------------------------------------------------------
//
// Method-
//       Loader::take
//
// Purpose-
//       Recycle an Interface Object.
//
//----------------------------------------------------------------------------
void
   Loader::take(                    // Recycle
     Interface*        object)      // This Interface Object
{
   (*takef)(object);
}

