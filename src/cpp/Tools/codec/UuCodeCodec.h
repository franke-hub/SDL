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
//       UuCodeCodec.h
//
// Purpose-
//       UU encoding/decoding object.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef UUCODECODEC_H_INCLUDED
#define UUCODECODEC_H_INCLUDED

#ifndef CODEC_H_INCLUDED
#include "Codec.h"
#endif

//----------------------------------------------------------------------------
//
// Class-
//       UuCodeCodec
//
// Purpose-
//       Encode/decode.
//
//----------------------------------------------------------------------------
class UuCodeCodec : public Codec    // UuCodeCodec
{
//----------------------------------------------------------------------------
// UuCodeCodec::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~UuCodeCodec( void );            // Destructor

   UuCodeCodec( void );             // Constructor

private:                            // Bitwise copy is prohibited
   UuCodeCodec(const UuCodeCodec&); // Disallowed copy constructor
UuCodeCodec&
   operator=(                       // Disallowed assignment operator
     const             UuCodeCodec&);

//----------------------------------------------------------------------------
// UuCodeCodec::Methods
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
// UuCodeCodec::Attributes
//----------------------------------------------------------------------------
public:
   // None defined
}; // class UuCodeCodec

#endif // UUCODECODEC_H_INCLUDED
