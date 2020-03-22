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
//       JpegDecoder.h
//
// Purpose-
//       JPEG decoder class.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef JPEGDECODER_H_INCLUDED
#define JPEGDECODER_H_INCLUDED

#include "Decoder.h"

//----------------------------------------------------------------------------
//
// Class-
//       JpegDecoder
//
// Purpose-
//       JPEG Decoder object descriptor.
//
//----------------------------------------------------------------------------
class JpegDecoder : public Decoder { // JpegDecoder
//----------------------------------------------------------------------------
// JpegDecoder::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~JpegDecoder( void );            // Destructor
   JpegDecoder(                     // Constructor
     GUI::Window&      window);     // The associated Window

//----------------------------------------------------------------------------
// JpegDecoder::Methods
//----------------------------------------------------------------------------
public:
virtual int                         // Return code (0 OK)
   decode(                          // Decode a file
     const char*       name);       // File name

//----------------------------------------------------------------------------
// JpegDecoder::Attributes
//----------------------------------------------------------------------------
protected:
   // None defined
}; // class JpegDecoder

#endif // JPEGDECODER_H_INCLUDED
