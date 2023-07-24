//----------------------------------------------------------------------------
//
//       Copyright (C) 2020-2023 Frank Eskesen.
//
//       This file is free content, distributed under creative commons CC0,
//       explicitly released into the Public Domain.
//       (See accompanying html file LICENSE.ZERO or the original contained
//       within https://creativecommons.org/publicdomain/zero/1.0/legalcode)
//
//----------------------------------------------------------------------------
//
// Title-
//       S_main.cpp
//
// Purpose-
//       Sample mainline source file
//
// Last change date-
//       2023/07/23
//
// Implementation note-
//       The basic source file template is given to the public domain.
//       You can freely use it without attribution of any kind.
//
//       The mainline file's "look and feel" is explicitly not copyrighted.
//
//----------------------------------------------------------------------------
#include <pub/config.h>             // For _PUB_NAMESPACE macro
#include <pub/Debug.h>              // For namespace debugging
#include "Sample.h"                 // Object declarations

#define PUB _PUB_NAMESPACE          // (More useful if used more than once)
using namespace PUB::debugging;     // For debugging subroutines

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
// Usage: if( HCDM ) { ... }, if( VERBOSE > 1 ) { ... }
// Compiler verfies compilation; optimization elides unreachable statements.
enum
{  HCDM= true                       // Hard Core Debug Mode?
,  VERBOSE= 1                       // Verbosity, higher is more verbose

,  USE_DEBUG= true                  // Activate debug method?
};

//----------------------------------------------------------------------------
//
// Subroutine-
//       main
//
// Purpose-
//       Mainline code
//
//----------------------------------------------------------------------------
int
   main(int, char**)                // Mainline code
//   int               argc,        // Argument count (Unused)
//   char*             argv[])      // Argument array (Unused)
{
   Sample sample;
   sample.on_run([&sample]() {
     if( VERBOSE > 0 ) debugf("%4d Sample(%p)::on_run\n", __LINE__, &sample);

     sample.debug("runner invoked");
   });

   sample.start();
   return 0;
}
