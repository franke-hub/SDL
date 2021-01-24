//----------------------------------------------------------------------------
//
//       Copyright (c) 2010 Frank Eskesen.
//
//       This file is free content, distributed under the MIT license.
//       (See accompanying file LICENSE.MIT or the original contained
//       within https://opensource.org/licenses/MIT)
//
//----------------------------------------------------------------------------
//
// Title-
//       UniCode.h
//
// Purpose-
//       Graphical User Interface: UniCode utility class
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
#ifndef GUI_UNICODE_H_INCLUDED
#define GUI_UNICODE_H_INCLUDED

#ifndef GUI_TYPES_H_INCLUDED
#include "Types.h"
#endif

#include "namespace.gui"
//----------------------------------------------------------------------------
//
// Class-
//       UniCode
//
// Purpose-
//       UniCode utility class.
//
// Usage notes-
//       The conversion methods do not consider code point rendering,
//       they only consider transforms to and from UTF encodings. It
//       may take more than one code point to render a visible glyph.
//
// CodePoint ranges-
//       0x000000 .. 0x00D7FF (Valid)
//       0x00D800 .. 0x00DBFF (Reserved for UTF16 conversion (word1))
//       0x00DC00 .. 0x00DFFF (Reserved for UTF16 conversion (word2))
//       0x00E000 .. 0x10FFFF (Valid)
//
//       fromUTF* Returns 0x00FFFD (Replacement) for an out of range sequence.
//                The return length is one.
//       getUTF*  Returns 0x00FFFD (Replacement) for an out of range sequence.
//                The in/out offset is incremented by one.
//       intoUTF* Returns 0 (No length) for an out of range sequence.
//
//----------------------------------------------------------------------------
class UniCode {                     // The extended character set
//----------------------------------------------------------------------------
// UniCode::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
typedef uint32_t       CodePoint;   // A UniCode code point (character)

//----------------------------------------------------------------------------
// UniCode::Constructors
//----------------------------------------------------------------------------
public:
   ~UniCode( void );                // (UNUSED) Destructor
   UniCode( void );                 // (UNUSED) Contructor

//----------------------------------------------------------------------------
//
// Public method-
//       UniCode::fromUTF8
//
// Purpose-
//       Convert (NUL terminated) UTF-8 sequence into CodePoint
//
//----------------------------------------------------------------------------
public:
static CodePoint                    // The CodePoint
   fromUTF8(                        // Convert into CodePoint
     const UTF8_t*     source,      // Source string sequence
     unsigned&         length);     // (OUTPUT) Length used

//----------------------------------------------------------------------------
//
// Public method-
//       UniCode::fromUTF16
//
// Purpose-
//       Convert (Big-Endian, NUL terminated) UTF-16 sequence into CodePoint
//
//----------------------------------------------------------------------------
public:
static CodePoint                    // The CodePoint
   fromUTF16(                       // Convert into CodePoint
     const UTF16_t*    source,      // Source string sequence
     unsigned&         length);     // (OUTPUT) Length used

//----------------------------------------------------------------------------
//
// Public method-
//       UniCode::fromUTF16LE
//
// Purpose-
//       Convert (Little-Endian, NUL terminated) UTF-16 sequence into CodePoint
//
//----------------------------------------------------------------------------
public:
static CodePoint                    // The CodePoint
   fromUTF16LE(                     // Convert into CodePoint
     const UTF16_t*    source,      // Source string sequence
     unsigned&         length);     // (OUTPUT) Length used

//----------------------------------------------------------------------------
//
// Public method-
//       UniCode::intoUTF8
//
// Purpose-
//       Convert CodePoint into UTF-8 sequence.
//
//----------------------------------------------------------------------------
public:
static unsigned                     // Number of UTF-8 words (0, 1, 2, 3, or 4)
   intoUTF8(                        // Convert into UTF-8 string sequence
     CodePoint         code,        // This CodePoint
     UTF8_t*           result);     // Result string (length >= 4)

//----------------------------------------------------------------------------
//
// Public method-
//       UniCode::intoUTF16
//
// Purpose-
//       Convert CodePoint into (Big-Endian) UTF-16 sequence.
//
//----------------------------------------------------------------------------
public:
static unsigned                     // Number of UTF-16 words (0, 1, or 2)
   intoUTF16(                       // Convert into UTF-16 string sequence
     CodePoint         code,        // This CodePoint
     UTF16_t*          result);     // Result string (length >= 2)

//----------------------------------------------------------------------------
//
// Public method-
//       UniCode::intoUTF16LE
//
// Purpose-
//       Convert CodePoint into (Little-Endian) UTF-16 sequence.
//
//----------------------------------------------------------------------------
public:
static unsigned                     // Number of UTF-16 words (0, 1, or 2)
   intoUTF16LE(                     // Convert into UTF-16 string sequence
     CodePoint         code,        // This CodePoint
     UTF16_t*          result);     // Result string (length >= 2)

//----------------------------------------------------------------------------
//
// Public method-
//       UniCode::getUTF8
//
// Purpose-
//       Convert (NUL terminated) UTF-8 sequence into CodePoint
//
//----------------------------------------------------------------------------
public:
static CodePoint                    // The next CodePoint
   getUTF8(                         // Get next CodePoint
     const UTF8_t*     source,      // From this UTF-8 string
     unsigned&         offset);     // (IN/OUT) Source string offset

//----------------------------------------------------------------------------
//
// Public method-
//       UniCode::getUTF16
//
// Purpose-
//       Convert (Big-Endian, NUL terminated) UTF-16 sequence into CodePoint
//
//----------------------------------------------------------------------------
static CodePoint                    // The next CodePoint
   getUTF16(                        // Get next CodePoint
     const UTF16_t*    source,      // From this UTF-16 string
     unsigned&         offset);     // (IN/OUT) Source string unit offset

//----------------------------------------------------------------------------
//
// Public method-
//       UniCode::getUTF16LE
//
// Purpose-
//       Convert (Little-Endian, NUL terminated) UTF-16 sequence into CodePoint
//
//----------------------------------------------------------------------------
static CodePoint                    // The next CodePoint
   getUTF16LE(                      // Get next CodePoint
     const UTF16_t*    source,      // From this UTF-16 string
     unsigned&         offset);     // (IN/OUT) Source string unit offset

//----------------------------------------------------------------------------
//
// Public method-
//       UniCode::putUTF8
//
// Purpose-
//       Convert CodePoint into UTF-8 sequence.
//
//----------------------------------------------------------------------------
public:
static unsigned                     // Number of UTF-8 words (0, 1, 2, 3, or 4)
   putUTF8(                         // Convert into UTF-8 string sequence
     CodePoint         code,        // This CodePoint
     UTF8_t*           result,      // Result string (length >= 4)
     unsigned&         offset);     // (IN/OUT) Result string unit offset

//----------------------------------------------------------------------------
//
// Public method-
//       UniCode::putUTF16
//
// Purpose-
//       Convert CodePoint into (Big-Endian) UTF-16 sequence.
//
//----------------------------------------------------------------------------
public:
static unsigned                     // Number of UTF-16 words (0, 1, or 2)
   putUTF16(                        // Convert into UTF-16 string sequence
     CodePoint         code,        // This CodePoint
     UTF16_t*          result,      // Result string (length >= 2)
     unsigned&         offset);     // (IN/OUT) Result string unit offset

//----------------------------------------------------------------------------
//
// Public method-
//       UniCode::putUTF16LE
//
// Purpose-
//       Convert CodePoint into (Little-Endian) UTF-16 sequence.
//
//----------------------------------------------------------------------------
public:
static unsigned                     // Number of UTF-16 words (0, 1, or 2)
   putUTF16LE(                      // Convert into UTF-16 string sequence
     CodePoint         code,        // This CodePoint
     UTF16_t*          result,      // Result string (length >= 2)
     unsigned&         offset);     // (IN/OUT) Result string unit offset

//----------------------------------------------------------------------------
//
// Public method-
//       UniCode::copy8to16
//
// Purpose-
//       Copy (NUL terminated) UTF-8 sequence into (Big-Endian) UTF-16
//
//----------------------------------------------------------------------------
public:
static unsigned                     // Number of UTF-16 words
   copy8to16(                       // Copy UTF-8 into UTF-16
     const UTF8_t*     source,      // Source UTF-8 string
     UTF16_t*          result);     // Result UTF-16 string

//----------------------------------------------------------------------------
//
// Public method-
//       UniCode::copy8to16LE
//
// Purpose-
//       Copy (NUL terminated) UTF-8 sequence into (Little-Endian) UTF-16
//
//----------------------------------------------------------------------------
public:
static unsigned                     // Number of UTF-16 words
   copy8to16LE(                     // Copy UTF-8 into UTF-16
     const UTF8_t*     source,      // Source UTF-8 string
     UTF16_t*          result);     // Result UTF-16 string

//----------------------------------------------------------------------------
//
// Public method-
//       UniCode::copy16to8
//
// Purpose-
//       Copy (Big-Endian, NUL terminated) UTF-16 sequence into  UTF-8
//
//----------------------------------------------------------------------------
public:
static unsigned                     // Number of UTF-8 words
   copy16to8(                       // Copy UTF-16 into UTF-8
     const UTF16_t*    source,      // Source UTF-16 string
     UTF8_t*           result);     // Result UTF-8 string

//----------------------------------------------------------------------------
//
// Public method-
//       UniCode::copy16to8LE
//
// Purpose-
//       Copy (Little-Endian, NUL terminated) UTF-16 sequence into  UTF-8
//
//----------------------------------------------------------------------------
public:
static unsigned                     // Number of UTF-8 words
   copy16to8LE(                     // Copy UTF-16 into UTF-8
     const UTF16_t*    source,      // Source UTF-16 string
     UTF8_t*           result);     // Result UTF-8 string
}; // class UniCode
#include "namespace.end"

#endif // GUI_UNICODE_H_INCLUDED
