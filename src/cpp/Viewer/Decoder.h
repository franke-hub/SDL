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
//       Decoder.h
//
// Purpose-
//       Decoder base class.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef DECODER_H_INCLUDED
#define DECODER_H_INCLUDED

#include <gui/Window.h>

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
   Decoder(                         // Constructor
     GUI::Window&      window);     // The associated Window

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
protected:
   GUI::Window&        window;      // The output Window
}; // class Decoder

#endif // DECODER_H_INCLUDED
