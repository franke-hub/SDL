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
//       Base64Codec.h
//
// Purpose-
//       Base64 encoding/decoding object.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef BASE64CODEC_H_INCLUDED
#define BASE64CODEC_H_INCLUDED

#ifndef CODEC_H_INCLUDED
#include "Codec.h"
#endif

//----------------------------------------------------------------------------
//
// Class-
//       Base64Codec
//
// Purpose-
//       Encode/decode.
//
//----------------------------------------------------------------------------
class Base64Codec : public Codec    // Base64Codec
{
//----------------------------------------------------------------------------
// Base64Codec::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Base64Codec( void );            // Destructor

   Base64Codec( void );             // Constructor

private:                            // Bitwise copy is prohibited
   Base64Codec(const   Base64Codec&); // Disallowed copy constructor
Base64Codec&
   operator=(                       // Disallowed assignment operator
     const             Base64Codec&);

//----------------------------------------------------------------------------
// Base64Codec::Methods
//----------------------------------------------------------------------------
public:
virtual int                         // Return code (0 OK)
   decode(                          // Decode a file
     Reader&           inp,         // Input file
     Writer&           out);        // Output file

virtual int                         // Return code (0 OK)
   encode(                          // Encode a file
     Reader&           inp,         // Input file
     Writer&           out);        // Output file

//----------------------------------------------------------------------------
// Base64Codec::Attributes
//----------------------------------------------------------------------------
public:
   // None defined
}; // class Base64Codec

#endif // BASE64CODEC_H_INCLUDED
