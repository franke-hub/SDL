//----------------------------------------------------------------------------
//
//       Copyright (C) 2022-2023 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       http/Codec.h
//
// Purpose-
//       Encoders and decoders.
//
// Last change date-
//       2023/07/29
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_HTTP_CODEC_H_INCLUDED
#define _LIBPUB_HTTP_CODEC_H_INCLUDED

#include <functional>               // For std::function
#include <string>                   // For std::string, size_t

#include <pub/Ioda.h>               // For pub::Ioda

#include "pub/http/bits/devconfig.h" // For HTTP config controls

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
namespace http {
//----------------------------------------------------------------------------
//
// Class-
//       Codec
//
// Purpose-
//       Encoder/decoder base class.
//
//----------------------------------------------------------------------------
class Codec {                       // Encoder/decoder base class
//----------------------------------------------------------------------------
// Codec::Typedefs and enumerations
//----------------------------------------------------------------------------
public:
typedef std::string    string;      // Using std::string
typedef std::function<void(int)>    f_error; // Error handler

//----------------------------------------------------------------------------
// Codec::Attributes
//----------------------------------------------------------------------------
protected:
size_t                 row;         // Current decode/encode input line
size_t                 col;         // Current decode/encode input column
f_error                h_error;     // Error handler

//----------------------------------------------------------------------------
// Codec::Destructor, constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Codec( void ) = default;        // Destructor
   Codec( void );                   // Constructor

//----------------------------------------------------------------------------
// Codec::Accessor methods
//----------------------------------------------------------------------------
size_t get_row( void ) const        // Get line number
{  return row + 1; }

size_t get_col( void ) const        // Get column number
{  return col + 1; }

//----------------------------------------------------------------------------
// Codec::Methods
//----------------------------------------------------------------------------
virtual Ioda                        // Decoded I/O data area
   decode(                          // Decode
     const Ioda&       inp);        // This input data area

virtual string                      // Decoded string
   decode(                          // Decode
     const string&     inp);        // This string

virtual Ioda                        // Encoded I/O data area
   encode(                          // Encode
     const Ioda&       inp);        // This input data area

virtual string                      // Encoded string
   encode(                          // Encode
     const string&     inp);        // This string

void
   on_error(const f_error& f)       // Set error handler
{  h_error= f; }

protected:
int                                 // The next character
   read(                            // Read next character from
     IodaReader&       inp);        // This input IodaReader
}; // class Codec

//----------------------------------------------------------------------------
//
// Class-
//       Codec64
//
// Purpose-
//       Base64 encoder/decoder.
//
// Implementation notes-
//       May not be be used in static constructors.
//
//----------------------------------------------------------------------------
class Codec64 : public Codec {      // Base64 encoder/decoder
//----------------------------------------------------------------------------
// Codec64::Typedefs and enumerations
//----------------------------------------------------------------------------
public:
enum { PAD_CHAR= '=' };             // The ending pad character

enum ERROR_CODE                     // Error codes
{  EC_NO_ERROR                      // No error
,  EC_ENCODE                        // Invalid encoding character
,  EC_LENGTH                        // Overlength line detected
,  EC_TERMPAD                       // Data after PAD_CHAR
,  EC_TERMSEQ                       // Invalid termination sequence
}; // enum ERROR_CODE

//----------------------------------------------------------------------------
// Codec64::Attributes
//----------------------------------------------------------------------------
protected:
int                    options= 0;  // Encoding/decoding (internal) controls
int                    rfc= 0;      // The RFC protocol, default RFC 2045

int                    de_tab[256]; // The decoding table
int                    en_tab[64];  // The encoding table

//----------------------------------------------------------------------------
// Codec64::Destructor, constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Codec64( void ) = default;      // Destructor

   Codec64( void );                 // Default constructor
   Codec64(int rfc);                // Constructor, specifying RFC

//----------------------------------------------------------------------------
// Codec64::Methods
//----------------------------------------------------------------------------
virtual Ioda                        // Decoded I/O data area
   decode(                          // Decode
     const Ioda&       inp);        // This input data area

virtual string                      // Decoded string
   decode(                          // Decode
     const string&     inp)         // This string
{  return Codec::decode(inp); }

virtual Ioda                        // Encoded I/O data area
   encode(                          // Encode
     const Ioda&       inp);        // This input data area

virtual string                      // Encoded string
   encode(                          // Encode
     const string&     inp)         // This string
{  return Codec::encode(inp); }

protected:
int                                 // The next decode character
   d_read(                          // Read the next encoded character
     IodaReader&       inp);        // From this IodaReader
}; // class Codec64
}  // namespace http
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_HTTP_CODEC_H_INCLUDED
