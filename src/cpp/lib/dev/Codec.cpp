//----------------------------------------------------------------------------
//
//       Copyright (C) 2022 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Codec.cpp
//
// Purpose-
//       Implement http/Codec.h
//
// Last change date-
//       2022/09/11
//
//----------------------------------------------------------------------------
#include <stdexcept>                // For std::runtime_error
#include <ctype.h>                  // For isspace
#include <stdio.h>                  // For EOF

#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/utility.h>            // For namespace pub::utility
#include "pub/http/Codec.h"         // For pub::http::Codec, implemented

using namespace _LIBPUB_NAMESPACE::debugging;
using namespace _LIBPUB_NAMESPACE::utility;

namespace _LIBPUB_NAMESPACE::http {  // Implementation namespace
//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 1                       // Verbosity, range 0..5
}; // enum

enum OPTIONS64                      // Codec64 options
{  O64_ENCODE= 0x00000001           // Encoding error reported
,  O64_LENGTH= 0x00000002           // Length error reported
};

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static constexpr char  rfc2045[65]= "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                    "abcdefghijklmnopqrstuvwxyz"
                                    "0123456789+/";

//----------------------------------------------------------------------------
//
// Method-
//       Codec::Codec
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
   Codec::Codec( void )             // Constructor
:  row(-1), col(-1), h_error([](int) {})
{  }

//----------------------------------------------------------------------------
//
// Method-
//       Codec:decode(std::string)
//
// Purpose-
//       Decode a string.
//
//----------------------------------------------------------------------------
std::string                         // The decoded string
   Codec::decode(                   // Decode
     const string&     S)           // This input string
{  if( HCDM )
     debugf("Codec(%p)::decode((string)%s)\n", this, visify(S).c_str());

   Buffer inp(S.length());
   inp.append(S);
   return (string)decode(inp);
}

//----------------------------------------------------------------------------
//
// Method-
//       Codec:encode(std::string)
//
// Purpose-
//       Encode a string.
//
//----------------------------------------------------------------------------
std::string                         // The encoded string
   Codec::encode(                   // Encode
     const string&     S)           // This input string
{  if( HCDM )
     debugf("Codec(%p)::encode((string)%s)\n", this, visify(S).c_str());

   Buffer inp(S.length());
   inp.append(S);
   return (string)encode(inp);
}

//----------------------------------------------------------------------------
//
// Method-
//       Codec:read
//
// Purpose-
//       Read the next input character, tracking row and column
//
//----------------------------------------------------------------------------
int                                 // The next input character
   Codec::read(                     // Read the next character from
     BufferReader&     rbuff)       // This input Buffer
{
   int C= rbuff.get();

   if( C == EOF )
     return EOF;

   if( C == '\n' ) {
     col= 0;
     ++row;
   } else if( C == '\r' ) {
     col= 0;
   } else {                       // (This includes TAB, VTAB, and FF)
     ++col;
   }

   return C;
}

//============================================================================
//
// Method-
//       Codec64::Codec64
//
// Purpose-
//       Constructors
//
//----------------------------------------------------------------------------
   Codec64::Codec64(                // RFC constructor
     int               rfc)         // Specifying this RFC
:  Codec(), rfc(rfc)
{  if( HCDM ) debugf("Codec64(%p)::Codec64(%d)\n", this, rfc);

   const char* rfc_tab= nullptr;
   switch(rfc) {                    // TODO: Add differing RFC support
     case 0:                        // (The default RFC)
       this->rfc= 2045;
       _LIBPUB_FALLTHROUGH;

     case 2045:
       rfc_tab= rfc2045;
       break;

     default:
       debugf("RFC(%d) not supported (yet)\n", rfc);
       throw std::runtime_error("Codec64: unsupported RFC");
   }

   for(unsigned i= 0; i<256; ++i)
     de_tab[i]= -1;

   for(unsigned i=0; i<64; ++i)
     en_tab[i]= rfc_tab[i];

   for(unsigned i=0; i<64; ++i)
     de_tab[en_tab[i]]= i;

   if( false ) {                    // Bringup, TODO: REMOVE
     char buff[4]= {' ', ' ', ' ', '\0'};
     debugf("de_tab:\n");
     for(int i= 0; i<256; ++i) {
       buff[2]= (char)i;
       debugf("[%3d] %2d%s\n", i, de_tab[i], de_tab[i] >= 0 ? buff+1 : buff+3);
     }

     debugf("en_tab:\n");
     for(int i= 0; i<64; ++i) {
       debugf("[%3d] '%c'\n", i, en_tab[i]);
     }
   }
}

   Codec64::Codec64( void )         // Default constructor
:  Codec64(0) {}

//----------------------------------------------------------------------------
//
// Method-
//       Codec64::decode
//
// Purpose-
//       Base64 decoder
//
// Implentation note-
//       RFC2045 decoder does not require terminating PAD_CHARs.
//
//----------------------------------------------------------------------------
Buffer                              // Decoded Buffer
   Codec64::decode(                 // Decode
     const Buffer&     ibuff)       // This input Buffer
{  if( HCDM ) debugf("Codec64::decode(Buffer)\n");

   int                 iset[4];     // The next 4 character input set
   Buffer              obuff;       // The output Buffer
   BufferReader        rbuff(ibuff); // The BufferReader
   uint32_t            oword;       // Working output word
   bool                tchar= false; // Encountered terminating character

   // Initialize
   row= 0;                          // Current input line (-1)
   col= 0;                          // Current input column (-1)
   options &= 0xffff0000;           // Clear error reporting options

   // DECODE -----------------------------------------------------------------
   for(;;) {                        // Decode the data
     iset[0]= decode_read(rbuff);   // Load the next 4 character input set
     if( iset[0] == EOF )           // If end of file and empty input set
       break;
     if( tchar ) {                  // If character after PAD_CHAR
       h_error((int)EC_TERMPAD);         // Report once, ignore remaining text
       break;
     }

     iset[1]= decode_read(rbuff);
     if( iset[1] == EOF || iset[1] == PAD_CHAR ) {
       // At least two characters are required in a terminating input set
       h_error(EC_TERMSEQ);         // Report once, ignore remaining text
       break;
     }

     iset[2]= decode_read(rbuff);
     if( iset[2] == EOF || iset[2] == PAD_CHAR ) {
       tchar= true;
       if( iset[2] == PAD_CHAR ) {
         iset[3]= decode_read(rbuff);
         if( iset[3] != PAD_CHAR )  // Sequence xx=x is invalid. (xx== needed)
           h_error(EC_TERMSEQ);
       }
       oword=                de_tab[iset[0]];
       oword= (oword << 6) | de_tab[iset[1]];
       if( oword & 0x000f )         // (Cannot specify unused bits)
         h_error(EC_TERMSEQ);
       obuff.put(oword >> 4);
       continue;                    // (Check for characters after end)
     }

     iset[3]= decode_read(rbuff);
     if( iset[3] == EOF || iset[3] == PAD_CHAR ) {
       tchar= true;
       oword=                de_tab[iset[0]];
       oword= (oword << 6) | de_tab[iset[1]];
       oword= (oword << 6) | de_tab[iset[2]];
       if( oword & 0x0003 )         // (Cannot specify unused bits)
         h_error(EC_TERMSEQ);

       obuff.put((oword >> 10));
       obuff.put((oword >>  2) & 0x00ff);
       continue;                    // (Check for characters after end)
     }

     oword=                de_tab[iset[0]];
     oword= (oword << 6) | de_tab[iset[1]];
     oword= (oword << 6) | de_tab[iset[2]];
     oword= (oword << 6) | de_tab[iset[3]];
     obuff.put(oword >> 16 );
     oword &= 0x0000ffff;
     obuff.put(oword >>  8 );
     obuff.put(oword  & 0x00ff);
   }

   return obuff;
}

//----------------------------------------------------------------------------
//
// Method-
//       Codec64::encode
//
// Purpose-
//       Base64 encoder
//
//----------------------------------------------------------------------------
Buffer                              // Encoded Buffer
   Codec64::encode(                 // Encode
     const Buffer&     ibuff)       // This input Buffer
{  if( HCDM ) debugf("Codec64::encode(Buffer)\n");

   Buffer              obuff;       // The output Buffer
   BufferReader        rbuff(ibuff); // The BufferReader
   uint32_t            oword;       // Working output word
   int                 iset[3];     // The next 3 character input set

   // Initialize
   row= 0;                          // Current input line (-1)
   col= 0;                          // Current input column (-1)
   options &= 0xffff0000;           // Clear error reporting options

   int out_col= 0;                  // Current output line (-1)
   int out_row= 0;                  // Current output column (-1)

   // ENCODE -----------------------------------------------------------------
   for(;;) {                        // Encode the data
     // TODO: Handle termination sequence according to RFC
     iset[0]= read(rbuff);
     if( iset[0] == EOF )
       break;

     iset[1]= read(rbuff);
     if( iset[1] == EOF ) {
       oword= iset[0];
       obuff.put(en_tab[(oword >>  2) & 0x003F]);
       obuff.put(en_tab[(oword <<  4) & 0x003F]);
       obuff.put((int)PAD_CHAR);
       obuff.put((int)PAD_CHAR);
       out_col += 4;
       break;
     }

     iset[2]= read(rbuff);
     if( iset[2] == EOF ) {
       oword= (iset[0] << 8) | iset[1];
       obuff.put(en_tab[(oword >> 10) & 0x003F]);
       obuff.put(en_tab[(oword >>  4) & 0x003F]);
       obuff.put(en_tab[(oword <<  2) & 0x003F]);
       obuff.put((int)PAD_CHAR);
       out_col += 4;
       break;
     }

     oword= (iset[0] << 16) | (iset[1] << 8) | iset[2];
     obuff.put(en_tab[(oword >> 18) & 0x003F]);
     obuff.put(en_tab[(oword >> 12) & 0x003F]);
     obuff.put(en_tab[(oword >>  6) & 0x003F]);
     obuff.put(en_tab[(oword >>  0) & 0x003F]);
     out_col += 4;
     if( out_col >= 76 ) {
       out_col= 0;
       ++out_row;
       obuff.append("\r\n");
     }
   }

   if( out_col ) {
     out_col= 0;
     ++out_row;
     obuff.append("\r\n");
   }

   col= out_col;
   row= out_row;

   return obuff;
}

//----------------------------------------------------------------------------
//
// Method-
//       Codec64:decode_read
//
// Purpose-
//       Read the next character to decode, tracking row and column
//
//----------------------------------------------------------------------------
int                                 // The next character to decode
   Codec64::decode_read(            // Read the next decode character from
     BufferReader&     rbuff)       // This input Buffer
{
   for(;;) {                        // Read the next valid character
     int C= Codec::read(rbuff);

     // Check for overlength line
     if( col > 76 ) {
       if( (options & O64_LENGTH) == 0 ) { // If not already reported
         options |= O64_LENGTH;     // (Only report once)
         h_error(EC_LENGTH);
       }
     }

     if( de_tab[C] >= 0 || C == PAD_CHAR || C == EOF )
       return C;

     // Line breaks or other characters not in the Base64 Alphabet must be
     // ignored by the decoding software. (i.e. this implementation)
     if( C != '\r' && C != '\n' ) { // If unexpected character
       if( (options & O64_ENCODE) == 0 ) { // If not already reported
         options |= O64_ENCODE;     // (Only report once)
         h_error(EC_ENCODE);
       }
     }
   }
}
}  // namespace _LIBPUB_NAMESPACE::http
