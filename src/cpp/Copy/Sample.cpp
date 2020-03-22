//----------------------------------------------------------------------------
//
//       Copyright (C) 2020 Frank Eskesen.
//
//       This file is free content, distributed under the "un-license,"
//       explicitly released into the Public Domain.
//       (See accompanying file LICENSE.UNLICENSE or the original
//       contained within http://unlicense.org)
//
//----------------------------------------------------------------------------
//
// Title-
//       Sample.cpp
//
// Purpose-
//       Sample source file.
//
// Last change date-
//       2020/01/29
//
// Implementation note-
//       The basic source file template is given to the public domain.
//       You can freely use it without attribution of any kind.
//
//       The source file's "look and feel" is explicitly not copyrighted.
//
//----------------------------------------------------------------------------
#include <pub/Debug.h>              // For debugging
#include "Sample.h"                 // Object declarations

using namespace _PUB_NAMESPACE::debugging; // For debugging

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#define HCDM                        // If defined, Hard Core Debug Mode
#endif

#include <pub/ifmacro.h>

namespace _PUB_NAMESPACE {
//----------------------------------------------------------------------------
// External data areas
//----------------------------------------------------------------------------
Sample                 Sample::global; // The global Sample

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
{  IFHCDM( debugf("%4d Sample(%p)::~Sample\n", __LINE__, this); ) }

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
{  IFHCDM( debugf("%4d Sample(%p)::Sample\n", __LINE__, this); ) }

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
   Sample::debug( void ) const      // Debugging display
{
IFHCDM(
   debugf("Sample(%p)::debug\n", this);
   debugf("name(%s)\n", name.c_str());
)
}

//----------------------------------------------------------------------------
//
// Method-
//       Sample::run
//
// Purpose-
//       Implement the default run method.
//
//----------------------------------------------------------------------------
void
   Sample::run( void )              // Run this Sample
{  IFHCDM( debugf("%4d Sample(%p)::run\n", __LINE__, this); ) }

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
} // namespace _PUB_NAMESPACE

//----------------------------------------------------------------------------
//
// Subroutine-
//       main
//
// Purpose-
//       Mainline code
//
// Implementation note-
//       We don't normally include main in an object implementation, but this
//       is only a simple sample.
//
//----------------------------------------------------------------------------
int                                 // void also allowed
   main(                            // Mainline code
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   pub::Sample sample;
   sample.debug();
   return 0;
}
