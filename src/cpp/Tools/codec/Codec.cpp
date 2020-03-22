//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//-----------------------------------------------------------------------------
//
// Title-
//       Codec.cpp
//
// Purpose-
//       Instantiate Codec Object.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <stdio.h>                  // For EOF

#include <com/Reader.h>
#include <com/Writer.h>
#include "Codec.h"

//----------------------------------------------------------------------------
//
// Method-
//       Codec::~Codec
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Codec::~Codec( void )            // Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       Codec::Codec
//
// Purpose-
//       Default constructor.
//
//----------------------------------------------------------------------------
   Codec::Codec( void )             // Default constructor
:  Ecode()
{
}

//----------------------------------------------------------------------------
//
// Method-
//       Codec::decode
//
// Purpose-
//       Default (NULL) encoding.
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   Codec::decode(                   // Decode a file
     Reader&           inp,         // Input file
     Writer&           out)         // Output file
{
   int                 C;           // The current character

   for(;;)                          // Copy the file
   {
     C= inp.get();
     if( C == EOF )
       break;

     out.put(C);
   }

   return EC_OK;
}

//----------------------------------------------------------------------------
//
// Method-
//       Codec::encode
//
// Purpose-
//       Default (NULL) decoding.
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   Codec::encode(                   // Encode a file
     Reader&           inp,         // Input file
     Writer&           out)         // Output file
{
   return decode(inp,out);
}

