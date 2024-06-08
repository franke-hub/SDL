//----------------------------------------------------------------------------
//
//       Copyright (c) 2021-2024 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Utf.i
//
// Purpose-
//       UTF inline implementations.
//
// Last change date-
//       2024/06/07
//
// Implementation notes-
//       *ONLY* included from ../Utf.h
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_BITS_UTF_I_INCLUDED
#define _LIBPUB_BITS_UTF_I_INCLUDED

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
//----------------------------------------------------------------------------
// Utf::is_combining, is codepoint a combining character?
//   (Combining character ranges valid for UTF version 15.1)
//----------------------------------------------------------------------------
bool                                // TRUE iff codepoint is combining
   Utf::is_combining(               // Is codepoint a combining character?
     utf32_t           code)        // The codepoint to be tested
{
   // This coding sequence [rather than 'if( code>0x0300 && code<0x0370 )]'
   // returns false at the lowest codepoint, avoiding additional tests.
   if( code < 0x0000'0300 )
     return false;

   if( code < 0x0000'0370 )         // Range 0x0000'0300..0x0000'036F
     return true;                   // (Combining)

   if( code < 0x0000'1AB0 )         // Range 0x0000'0370..0x0000'1ACF
     return false;

   if( code < 0x0000'1B00 )         // Range 0x0000'1AB0..0x0000'1AFF
     return true;                   // (Combining, some unassigned)

   if( code < 0x0000'1DC0 )         // Range 0x0000'1B00..0x0000'1DBF
     return false;

   if( code < 0x0000'1E00 )         // Range 0x0000'1DC0..0x0000'1DFF
     return true;                   // (Combining)

   if( code < 0x0000'20D0 )         // Range 0x0000'1E00..0x0000'20CF
     return false;

   if( code < 0x0000'2100 )         // Range 0x0000'20D0..0x0000'20FF
     return true;                   // (Combining, some unassigned)

   // Range 0x00'D800..0x00'DFFF is not Unicode (and also not combining.)
   if( code < 0x0000'FE20 )         // Range 0x0000'2100..0x0000'FE1F
     return false;

   if( code < 0x0000'FE30 )         // Range 0x0000'FE20..0x0000'FE2F
     return true;                   // (Combining)

   // Range > 0x10'FFFF is not Unicode (and also not combining.)
   return false;                    // Range 0x0000'FE30..0x0010'FFFF
}

//----------------------------------------------------------------------------
// Utf::is_unicode, is codepoint in allowed unicode range?
//----------------------------------------------------------------------------
bool                                // TRUE iff codepoint is valid
   Utf::is_unicode(                 // Is code in allowed unicode range?
     utf32_t           code)        // The source codepoint
{
   if( code  < 0x0000'D800 )        // All codepoints < 0x0000'D800
     return true;                   // are allowed

   if( code <= 0x0000'DFFF )        // 0xD800..DFFF: UTF16 surrogate pair range
     return false;                  // (Disallowed)

   if( code <= 0x0010'FFFF )        // If inside valid unicode range
     return true;                   // (Range 0x0000'E000 .. 0x0010'FFFF)

   return false;                    // (>0x0010'FFFF) is outside Unicode range
}

//----------------------------------------------------------------------------
// Utf::strlen, return U-string Length (Length does not include terminator)
//----------------------------------------------------------------------------
Utf::Length                         // The Length, in native units
   Utf::strlen(                     // Get Length of
     const utf16_t*    str)         // This U16-string
{
   Length length= 0;
   while( *str != 0 ) {
     ++str;
     ++length;
   }

   return length;
}

Utf::Length                         // The Length, in native units
   Utf::strlen(                     // Get Length of
     const utf32_t*    str)         // This U32-string
{
   Length length= 0;
   while( *str != 0 ) {
     ++str;
     ++length;
   }

   return length;
}
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_BITS_UTF_I_INCLUDED
