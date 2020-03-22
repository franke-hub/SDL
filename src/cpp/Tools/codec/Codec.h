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
//       Codec.h
//
// Purpose-
//       Encoding/decoding object.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef CODEC_H_INCLUDED
#define CODEC_H_INCLUDED

#ifndef ECODE_H_INCLUDED
#include "com/Ecode.h"
#endif

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class Reader;
class Writer;

//----------------------------------------------------------------------------
//
// Class-
//       Codec
//
// Purpose-
//       Encode/decode.
//
//----------------------------------------------------------------------------
class Codec : public Ecode          // Codec
{
//----------------------------------------------------------------------------
// Codec::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
enum ReturnCode                     // Generic Return Codes
{  RC_OK= 0                         // No error
,  RC_NG                            // Generic error
,  RC_SZ                            // Overlength line
,  RC_EF                            // End of file
};

enum DecodeCode                     // Decode status code
{  DC_OK= 0                         // No error
,  DC_NOH                           // No header detected
,  DC_ICS                           // Invalid Code Sequence
,  DC_ERR                           // Internal/System error
};

enum EncodeCode                     // Encode status code
{  EC_OK= 0                         // No error
,  EC_RDR                           // Error reading file
,  EC_WTR                           // Error writing file
};

//----------------------------------------------------------------------------
// Codec::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Codec( void );                  // Destructor

   Codec( void );                   // Constructor

private:                            // Bitwise copy is prohibited
   Codec(const Codec&);             // Disallowed copy constructor
   Codec& operator=(const Codec&);  // Disallowed assignment operator

//----------------------------------------------------------------------------
// Codec::Methods
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
// Codec::Attributes
//----------------------------------------------------------------------------
public:
   // None defined
}; // class Codec

#endif // CODEC_H_INCLUDED
