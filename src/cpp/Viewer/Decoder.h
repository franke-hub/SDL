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
//       Decoder.h
//
// Purpose-
//       Decoder base class.
//
// Last change date-
//       2021/01/28
//
//----------------------------------------------------------------------------
#ifndef DECODER_H_INCLUDED
#define DECODER_H_INCLUDED

#include <stdint.h>                 // For uint32_t

//----------------------------------------------------------------------------
//
// Class-
//       Decoder
//
// Purpose-
//       Decoder object descriptor.
//
//----------------------------------------------------------------------------
class Decoder {                     // Decoder
//----------------------------------------------------------------------------
// Decoder::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Decoder( void );                // Destructor
   Decoder( void );                 // Constructor

private:                            // Bitwise copy is prohibited
   Decoder(const Decoder&);         // Disallowed copy constructor
   Decoder& operator=(const Decoder&); // Disallowed assignment operator

//----------------------------------------------------------------------------
// Decoder::Methods
//----------------------------------------------------------------------------
public:
virtual int                         // Return code (0 OK)
   decode(                          // Decode a file
     const char*       name);       // File name

//----------------------------------------------------------------------------
// Decoder::Attributes
//----------------------------------------------------------------------------
uint32_t*              buffer= nullptr; // Image buffer (0x00rrggbb)
uint32_t               width= 0;    // Buffer width
uint32_t               height= 0;   // Buffer height
}; // class Decoder
#endif // DECODER_H_INCLUDED
