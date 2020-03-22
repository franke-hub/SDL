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
//       Crypto.h
//
// Purpose-
//       Standard include file for Crypto routines.
//
// Last change date-
//       2007/01/01                 Version 2, Release 1
//
//----------------------------------------------------------------------------
#include <com/Random.h>             // For Random::get/set

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define INSTRUMENT_CTLFILE        0 // CTLFILE trace
#define INSTRUMENT_INPFILE        0 // INPFILE trace
#define INSTRUMENT_OUTFILE        0 // OUTFILE trace
#define INSTRUMENT_KEYWORD        0 // Master key trace
#define INSTRUMENT_KEYCODE        1 // Master key trace

#define CTLMINS                  24 // Minimum control file size
#define CTLSIZE          0x00100000 // Control buffer size
#define INPSIZE          0x00100000 // Input buffer size
#define OUTSIZE          0x00010000 // Output buffer size

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef INLINE
#define INLINE inline               // INLINE control, inline or <nothing>
#endif

#ifndef USE_MALLOC_BUFFER
#define USE_MALLOC_BUFFER           // If defined, use malloc'ed buffer
#endif

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define BITS_PER_BYTE 8             // Number of bits in a byte
#define BITS_PER_WORD (BITS_PER_BYTE * sizeof(Word))
#define BYTE_MASK 0x000000ff        // Byte mask
#define BYTES_PER_WORD 4            // Number of bytes in a word (for #if)

//----------------------------------------------------------------------------
// Enumerations and typedefs
//----------------------------------------------------------------------------
typedef uint32_t       Word;        // The unit of encryption

//----------------------------------------------------------------------------
// Union: WC: Word or character array
//----------------------------------------------------------------------------
union WC                            // Word or character array
{
   Word                w;           // Word value
   unsigned char       c[sizeof(Word)]; // Character array
};

//----------------------------------------------------------------------------
//
// Subroutine-
//      lrot
//
// Function-
//      Long Rotate Left.
//
//----------------------------------------------------------------------------
static inline Word                  // Resultant
   lrot(                            // Rotate left
     Word              source,      // Source Word
     int               shift)       // Shift amount
{
   Word                result;      // Resultant

   shift %= BITS_PER_WORD;          // Get shift amount

   result  = source >> (BITS_PER_WORD - shift);
   result |= source << shift;

   return result;
}

