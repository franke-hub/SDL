//----------------------------------------------------------------------------
//
//       Copyright (c) 2010 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       TextSource.cpp
//
// Purpose-
//       TextSource implementation methods.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "Common.h"
#include "TextSource.h"

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
// Method-
//       TextSource::~TextSource
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   TextSource::~TextSource( void )  // Destructor
{
   close();
}

//----------------------------------------------------------------------------
//
// Method-
//       TextSource::TextSource
//
// Purpose-
//       Default constructor.
//
//----------------------------------------------------------------------------
   TextSource::TextSource( void )   // Default constructor
:  DataSource()
{
}

//----------------------------------------------------------------------------
//
// Method-
//       TextSource::TextSource
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   TextSource::TextSource(          // Constructor
     const char*       text)        // For this text
{
   open(text);
}

//----------------------------------------------------------------------------
//
// Method-
//       TextSource::close
//
// Purpose-
//       Close the TextSource.
//
//----------------------------------------------------------------------------
void
   TextSource::close( void )        // Close the TextSource
{
   reset();
}

//----------------------------------------------------------------------------
//
// Method-
//       TextSource::open
//
// Purpose-
//       Copy the text.
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   TextSource::open(                // Open a TextSource
     const char*       text)        // With this text
{
   close();

   if( text == NULL )
     return CC_ERR;

   origin= (unsigned char*)strdup(text);
   if( origin == NULL )
     return CC_ERR;

   length= strlen((char*)origin);
   setWidth();

   return 0;
}

