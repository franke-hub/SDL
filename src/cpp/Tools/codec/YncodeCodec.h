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
//       YncodeCodec.h
//
// Purpose-
//       yEnc encoding/decoding object.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef YNCODECODEC_H_INCLUDED
#define YNCODECODEC_H_INCLUDED

#ifndef CODEC_H_INCLUDED
#include "Codec.h"
#endif

#include <com/CRC32.h>

//----------------------------------------------------------------------------
//
// Class-
//       YncodeCodec
//
// Purpose-
//       Encode/decode.
//
//----------------------------------------------------------------------------
class YncodeCodec : public Codec    // YncodeCodec
{
//----------------------------------------------------------------------------
// YncodeCodec::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~YncodeCodec( void );            // Destructor

   YncodeCodec( void );             // Constructor

private:                            // Bitwise copy is prohibited
   YncodeCodec(const   YncodeCodec&); // Disallowed copy constructor
YncodeCodec&
   operator=(                       // Disallowed assignment operator
     const             YncodeCodec&);

//----------------------------------------------------------------------------
// YncodeCodec::Methods
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

virtual uint64_t                    // The checksize
   getSize( void ) const;           // Get checksize

virtual uint32_t                    // The checksum
   getSum( void ) const;            // Get checksum

//----------------------------------------------------------------------------
// YncodeCodec::Attributes
//----------------------------------------------------------------------------
protected:
   CRC32               checksum;    // Checksum
   uint64_t            size;        // Checksize
}; // class YncodeCodec

#endif // YNCODECODEC_H_INCLUDED
