//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
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
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <stdlib.h>

#include "Decoder.h"
using namespace GUI;

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
   Decoder::Decoder(                // Constructor
     GUI::Window&      window)      // The associated Window
:  window(window)
{
   window.setAttribute(Object::VISIBLE, FALSE);
}

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
   return (-1);                     // Need derived class
}

