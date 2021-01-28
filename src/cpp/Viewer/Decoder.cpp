//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2021 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Decoder.cpp
//
// Purpose-
//       Decoder implementation.
//
// Last change date-
//       2021/01/28
//
//----------------------------------------------------------------------------
#include <stdlib.h>

#include "Decoder.h"

//----------------------------------------------------------------------------
//
// Method-
//       Decoder::~Decoder
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Decoder::~Decoder( void )        // Destructor
{
   if( buffer ) {
     free(buffer);
     buffer= nullptr;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Decoder::Decoder
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Decoder::Decoder( void )         // Constructor
{  }

//----------------------------------------------------------------------------
//
// Method-
//       Decoder::decode
//
// Purpose-
//       Decode a file.
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   Decoder::decode(                 // Decode a file
     const char*       name)        // File name
{
   (void)name; return (-1);         // Need derived class
}

