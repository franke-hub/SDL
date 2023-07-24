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
//       Sample.cpp
//
// Purpose-
//       Sample implentation source file (for a local library object)
//
// Last change date-
//       2023/07/23
//
// Implementation note-
//       The basic source file template is given to the public domain.
//       You can freely use it without attribution of any kind.
//
//       The source file's "look and feel" is explicitly not copyrighted.
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
// External data areas
//----------------------------------------------------------------------------
Sample                 Sample::global; // The global Sample

//----------------------------------------------------------------------------
//
// Method-
//       Sample::Sample
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
   Sample::Sample( void )           // Constructor
:  name("sample")                   // Yes, they're all the same
{  if( HCDM ) debugf("%4d Sample(%p)::Sample\n", __LINE__, this); }

//----------------------------------------------------------------------------
//
// Method-
//       Sample::~Sample
//
// Purpose-
//       Destructor
//
//----------------------------------------------------------------------------
   Sample::~Sample( void )          // Destructor
{  if( HCDM ) debugf("%4d Sample(%p)::~Sample\n", __LINE__, this); }

//----------------------------------------------------------------------------
//
// Method-
//       Sample::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   Sample::debug(                       // Debugging display
     const char*       info) const      // Informational message
{
   if( USE_DEBUG ) {
     debugf("Sample(%p)::debug(%s)\n", this, info);
     debugf("name(%s)\n", name.c_str());
     debugf("global.name(%s)\n", global.name.c_str());
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Sample::start
//
// Purpose-
//       Start the Sample
//
//----------------------------------------------------------------------------
void
   Sample::start( void )            // Start this Sample
{  run(); }                         // To start it is to run it
