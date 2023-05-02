//----------------------------------------------------------------------------
//
//       Copyright (c) 2010 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       UniCode.cpp
//
// Purpose-
//       Graphical User Interface: UniCode implementation
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "gui/Object.h"
#include "gui/UniCode.h"
using namespace GUI;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#define REPLACEMENT_CHAR 0x0000FFFD // Error replacement character

//----------------------------------------------------------------------------
//
// Subroutine-
//       fetchBE
//
// Purpose-
//       Fetch a Big-Endian UTF16 word.
//
//----------------------------------------------------------------------------
static inline UTF16_t               // Resultant
   fetchBE(                         // Fetch UTF16 word (Big-Endian)
     const UTF16_t*    buffer,      // UTF16 string origin
     unsigned          offset)      // UTF16 string offset
{
   const unsigned char* text= (const unsigned char*)buffer;
   offset *= 2;

   UTF16_t result= text[offset] << 8 | text[offset+1];
   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       fetchLE
//
// Purpose-
//       Fetch a Little-Endian UTF16 word.
//
//----------------------------------------------------------------------------
static inline UTF16_t               // Resultant
   fetchLE(                         // Fetch UTF16 word (Little-Endian)
     const UTF16_t*    buffer,      // UTF16 string origin
     unsigned          offset)      // UTF16 string offset
{
   const unsigned char* text= (const unsigned char*)buffer;
   offset *= 2;

   UTF16_t result= text[offset+1] << 8 | text[offset];
   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       storeBE
//
// Purpose-
//       Store a Big-Endian UTF16 word.
//
//----------------------------------------------------------------------------
static inline void
   storeBE(                         // Store UTF16 word (Big-Endian)
     const UTF16_t     data,        // The UTF16 word
     UTF16_t*          buffer,      // UTF16 buffer origin
     unsigned          offset)      // UTF16 string offset
{
   unsigned char* text= (unsigned char*)buffer;
   offset *= 2;

   text[offset]= data >> 8;
   text[offset+1]= data;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       storeLE
//
// Purpose-
//       Store a Little-Endian UTF16 word.
//
//----------------------------------------------------------------------------
static inline void
   storeLE(                         // Store UTF16 word (Little-Endian)
     const UTF16_t     data,        // The UTF16 word
     UTF16_t*          buffer,      // UTF16 buffer origin
     unsigned          offset)      // UTF16 string offset
{
   unsigned char* text= (unsigned char*)buffer;
   offset *= 2;

   text[offset+1]= data >> 8;
   text[offset]= data;
}

//----------------------------------------------------------------------------
//
// Method-
//       UniCode::~UniCode
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   UniCode::~UniCode( void )        // Destructor
{
   #ifdef HCDM
     Logger::log("%4d: UniCode(%p)::~UniCode()\n", __LINE__, this);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       UniCode::UniCode
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   UniCode::UniCode( void )         // Constructor
{
   #ifdef HCDM
     Logger::log("%4d: UniCode(%p)::UniCode()\n", __LINE__, this);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       UniCode::fromUTF8
//
// Purpose-
//       Convert (NUL terminated) UTF-8 sequence into CodePoint
//
//----------------------------------------------------------------------------
UniCode::CodePoint                  // The CodePoint
   UniCode::fromUTF8(               // Convert into CodePoint
     const UTF8_t*     source,      // Source string (NUL terminated)
     unsigned&         length)      // (OUTPUT) Length used
{
   length= 0;                       // Resultant
   if( *source == '\0' )            // If End-Of-String
     return 0;                      // All done

   CodePoint result= source[length++] & 0x000000ff; // Get first character
   if( result >= 0x00000080 )
   {
     if( (result & 0x000000e0) == 0x000000c0 ) // Two character sequence
     {
       if( source[length] == '\0' )
         return REPLACEMENT_CHAR;

       CodePoint combo1= source[length++] & 0x000000ff;
       if( (combo1 & 0x000000c0 ) != 0x00000080 )
         return REPLACEMENT_CHAR;

       result= (result & 0x0000001f) << 6
             | (combo1 & 0x0000003f);
       if( result < 0x00000080 )
         return REPLACEMENT_CHAR;
     }
     else if( (result & 0x000000f0) == 0x000000e0 ) // Three char sequence
     {
       if( source[length] == '\0'
           || source[length+1] == '\0' )
         return REPLACEMENT_CHAR;

       CodePoint combo1= source[length++] & 0x000000ff;
       CodePoint combo2= source[length++] & 0x000000ff;
       if( (combo1 & 0x000000c0 ) != 0x00000080
           || (combo2 & 0x000000c0 ) != 0x00000080 )
         return REPLACEMENT_CHAR;

       result= (result & 0x0000000f) << 12
             | (combo1 & 0x0000003f) << 6
             | (combo2 & 0x0000003f);
       if( result < 0x00000800 )
         return REPLACEMENT_CHAR;
       if( result >= 0x0000d800 && result <= 0x0000dfff )
         return REPLACEMENT_CHAR;
     }
     else if( (result & 0x000000f8) == 0x000000f0 ) // Four char sequence
     {
       if( source[length] == '\0'
           || source[length+1] == '\0'
           || source[length+2] == '\0' )
         return REPLACEMENT_CHAR;

       CodePoint combo1= source[length++] & 0x000000ff;
       CodePoint combo2= source[length++] & 0x000000ff;
       CodePoint combo3= source[length++] & 0x000000ff;
       if( (combo1 & 0x000000c0 ) != 0x00000080
           || (combo2 & 0x000000c0 ) != 0x00000080
           || (combo3 & 0x000000c0 ) != 0x00000080 )
         return REPLACEMENT_CHAR;

       result= (result & 0x00000007) << 18
             | (combo1 & 0x0000003f) << 12
             | (combo2 & 0x0000003f) << 6
             | (combo3 & 0x0000003f);
       if( result < 0x00010000 || result > 0x0010ffff )
         return REPLACEMENT_CHAR;

     }
     else                           // Invalid sequence
       return REPLACEMENT_CHAR;     // Skip single character
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       UniCode::fromUTF16
//
// Purpose-
//       Convert (Big Endian, NUL terminated) UTF-16 sequence into CodePoint
//
//----------------------------------------------------------------------------
UniCode::CodePoint                  // The CodePoint
   UniCode::fromUTF16(              // Convert into CodePoint
     const UTF16_t*    source,      // Source string (NUL terminated)
     unsigned&         length)      // (OUTPUT) Length used
{
   length= 0;                       // Resultant
   CodePoint result= fetchBE(source, 0); // Resultant
   if( result == 0 )                // If End-Of-String
     return 0;                      // All done

   length++;                        // Not end of string, update length
   if( result >= 0x0000d800 && result <= 0x0000dfff ) // Two-byte encoding
   {
     CodePoint combo1= fetchBE(source, 1);
     if( result > 0x0000dbff || combo1 < 0x0000dc00 || combo1 > 0x0000dfff )
       return REPLACEMENT_CHAR;

     length++;
     result= (0x00010000 + ((result - 0x0000d800) << 10))
           | (combo1 & 0x000003ff);
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       UniCode::fromUTF16LE
//
// Purpose-
//       Convert (Little Endian, NUL terminated) UTF-16 sequence into CodePoint
//
//----------------------------------------------------------------------------
UniCode::CodePoint                  // The CodePoint
   UniCode::fromUTF16LE(            // Convert into CodePoint
     const UTF16_t*    source,      // Source string (NUL terminated)
     unsigned&         length)      // (OUTPUT) Length used
{
   length= 0;                       // Resultant
   CodePoint result= fetchLE(source, 0); // Resultant
   if( result == 0 )                // If End-Of-String
     return 0;                      // All done

   length++;                        // Not end of string, update length
   if( result >= 0x0000d800 && result <= 0x0000dfff ) // Two-byte encoding
   {
     CodePoint combo1= fetchLE(source, 1);
     if( result > 0x0000dbff || combo1 < 0x0000dc00 || combo1 > 0x0000dfff )
       return REPLACEMENT_CHAR;

     length++;
     result= (0x00010000 + ((result - 0x0000d800) << 10))
           |  (combo1 & 0x000003ff);
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       UniCode::intoUTF8
//
// Purpose-
//       Convert CodePoint into UTF-8 string
//
//----------------------------------------------------------------------------
unsigned                            // Output length (0, 1, 2, 3, or 4)
   UniCode::intoUTF8(               // Convert into UTF-8 string
     CodePoint         cp,          // This CodePoint
     UTF8_t*           result)      // (OUTPUT) UTF-8 string (length >= 4)
{
   unsigned outX= 0;                // Output index

   if( cp < 0x00000080 )
     result[outX++]= char(cp);
   else if( cp < 0x00000800 )
   {
     result[outX++]= char(0x000000c0 |  (cp >>  6));
     result[outX++]= char(0x00000080 |  (cp        & 0x0000003f));
   }
   else if( cp < 0x00010000 )
   {
     if( cp < 0x0000d800 || cp > 0x0000dfff )
     {
       result[outX++]= char(0x000000e0 |  (cp >> 12));
       result[outX++]= char(0x00000080 | ((cp >>  6) & 0x0000003f));
       result[outX++]= char(0x00000080 |  (cp        & 0x0000003f));
     }
   }
   else if( cp < 0x00110000 )
   {
     result[outX++]= char(0x000000f0 |  (cp >> 18));
     result[outX++]= char(0x00000080 | ((cp >> 12) & 0x0000003f));
     result[outX++]= char(0x00000080 | ((cp >>  6) & 0x0000003f));
     result[outX++]= char(0x00000080 |  (cp        & 0x0000003f));
   }

   return outX;
}

//----------------------------------------------------------------------------
//
// Method-
//       UniCode::intoUTF16
//
// Purpose-
//       Convert CodePoint into (Big-Endian) UTF-16  string
//
//----------------------------------------------------------------------------
unsigned                            // Output length (0, 1, or 2)
   UniCode::intoUTF16(              // Convert into UTF-16 string
     CodePoint         cp,          // This CodePoint
     UTF16_t*          result)      // (OUTPUT) UTF-16 string (length >= 2)
{
   unsigned outX= 0;                // Output index

   if( cp >= 0x00010000 )
   {
     if( cp <= 0x0010ffff )
     {
       CodePoint out1= 0x0000d800 + ((cp - 0x0010000) >> 10);
       CodePoint out2= 0x0000dc00 +  (cp & 0x00003ff);
       storeBE(out1, result, outX++);
       storeBE(out2, result, outX++);
     }
   }
   else if( cp < 0x0000d800 || cp > 0x0000dfff )
     storeBE(cp, result, outX++);

   return outX;
}

//----------------------------------------------------------------------------
//
// Method-
//       UniCode::intoUTF16LE
//
// Purpose-
//       Convert CodePoint into (Little-Endian) UTF-16  string
//
//----------------------------------------------------------------------------
unsigned                            // Output length (0, 1, or 2)
   UniCode::intoUTF16LE(            // Convert into UTF-16 string
     CodePoint         cp,          // This CodePoint
     UTF16_t*          result)      // (OUTPUT) UTF-16 string (length >= 2)
{
   unsigned outX= 0;                // Output index

   if( cp >= 0x00010000 )
   {
     if( cp <= 0x0010ffff )
     {
       CodePoint out1= 0x0000d800 + ((cp - 0x0010000) >> 10);
       CodePoint out2= 0x0000dc00 +  (cp & 0x00003ff);
       storeLE(out1, result, outX++);
       storeLE(out2, result, outX++);
     }
   }
   else if( cp < 0x0000d800 || cp > 0x0000dfff )
     storeLE(cp, result, outX++);

   return outX;
}

//----------------------------------------------------------------------------
//
// Method-
//       UniCode::getUTF8           (Source UTF-8 string)
//       UniCode::getUTF16          (Source UTF-16 string, Big-Endian)
//       UniCode::getUTF16LE        (Source UTF-16 string, Little-Endian)
//
// Purpose-
//       Get next CodePoint from UTF string.
//
//----------------------------------------------------------------------------
UniCode::CodePoint                  // The UniCode CodePoint
   UniCode::getUTF8(                // Get UniCode CodePoint
     const UTF8_t*     source,      // From this UTF-8 string
     unsigned&         offset)      // At this offset
{
   unsigned length= 0;
   CodePoint result= fromUTF8(source+offset, length);
   offset += length;
   return result;
}

UniCode::CodePoint                  // The UniCode CodePoint
   UniCode::getUTF16(               // Get UniCode CodePoint
     const UTF16_t*    source,      // From this UTF-16 string
     unsigned&         offset)      // At this unit offset
{
   unsigned length= 0;
   CodePoint result= fromUTF16(source+offset, length);
   offset += length;
   return result;
}

UniCode::CodePoint                  // The UniCode CodePoint
   UniCode::getUTF16LE(             // Get UniCode CodePoint
     const UTF16_t*    source,      // From this UTF-16 string
     unsigned&         offset)      // At this unit offset
{
   unsigned length= 0;
   CodePoint result= fromUTF16LE(source+offset, length);
   offset += length;
   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       UniCode::putUTF8           (Target UTF-8 string)
//       UniCode::putUTF16          (Target UTF-16 string, Big-Endian)
//       UniCode::putUTF16LE        (Target UTF-16 string, Little-Endian)
//
// Purpose-
//       Get next CodePoint from UTF string.
//
//----------------------------------------------------------------------------
unsigned                            // Number of UTF-8 words (0, 1, 2, 3, or 4)
   UniCode::putUTF8(                // Put UniCode CodePoint
     CodePoint         source,      // The UniCode CodePoint
     UTF8_t*           target,      // Into this UTF-8 string
     unsigned&         offset)      // At this offset
{
   unsigned length= intoUTF8(source, target+offset);
   offset += length;
   return length;
}

unsigned                            // Number of UTF-16 words (0, 1, or 2)
   UniCode::putUTF16(               // Put UniCode CodePoint
     CodePoint         source,      // The UniCode CodePoint
     UTF16_t*          target,      // Into this UTF-16 string
     unsigned&         offset)      // At this offset
{
   unsigned length= intoUTF16(source, target+offset);
   offset += length;
   return length;
}

unsigned                            // Number of UTF-16 words (0, 1, or 2)
   UniCode::putUTF16LE(             // Put UniCode CodePoint
     CodePoint         source,      // The UniCode CodePoint
     UTF16_t*          target,      // Into this UTF-16 string
     unsigned&         offset)      // At this offset
{
   unsigned length= intoUTF16LE(source, target+offset);
   offset += length;
   return length;
}

//----------------------------------------------------------------------------
//
// Method-
//       UniCode::copy8to16
//       UniCode::copy8to16LE
//       UniCode::copy16to8
//       UniCode::copy16to8LE
//
// Purpose-
//       Copy utilities.
//
//----------------------------------------------------------------------------
unsigned                            // Number of UTF-16 words
   UniCode::copy8to16(              // Copy UTF-8 into UTF-16
     const UTF8_t*     source,      // Source UTF-8 string
     UTF16_t*          result)      // Result UTF-16 string
{
   unsigned count= 0;               // Number of output words
   unsigned offset= 0;              // Number of input words

   for(;;)
   {
     CodePoint cp= getUTF8(source, offset);
     if( cp == 0 )
       break;

     unsigned length= intoUTF16(cp, result);
     result += length;
     count  += length;
   }
   intoUTF16(0, result);

   return count;
}

unsigned                            // Number of UTF-16 words
   UniCode::copy8to16LE(            // Copy UTF-8 into UTF-16
     const UTF8_t*     source,      // Source UTF-8 string
     UTF16_t*          result)      // Result UTF-16 string
{
   unsigned count= 0;               // Number of output words
   unsigned offset= 0;              // Number of input words

   for(;;)
   {
     CodePoint cp= getUTF8(source, offset);
     if( cp == 0 )
       break;

     unsigned length= intoUTF16LE(cp, result);
     result += length;
     count  += length;
   }
   intoUTF16LE(0, result);

   return count;
}

unsigned                            // Number of UTF-8 words
   UniCode::copy16to8(              // Copy UTF-16 into UTF-8
     const UTF16_t*    source,      // Source UTF-16 string
     UTF8_t*           result)      // Result UTF-8 string
{
   unsigned count= 0;               // Number of output words
   unsigned offset= 0;              // Number of input words

   for(;;)
   {
     CodePoint cp= getUTF16(source, offset);
     if( cp == 0 )
       break;

     unsigned length= intoUTF8(cp, result);
     result += length;
     count  += length;
   }
   intoUTF8(0, result);

   return count;
}

unsigned                            // Number of UTF-8 words
   UniCode::copy16to8LE(            // Copy UTF-16 into UTF-8
     const UTF16_t*    source,      // Source UTF-16 string
     UTF8_t*           result)      // Result UTF-8 string
{
   unsigned count= 0;               // Number of output words
   unsigned offset= 0;              // Number of input words

   for(;;)
   {
     CodePoint cp= getUTF16LE(source, offset);
     if( cp == 0 )
       break;

     unsigned length= intoUTF8(cp, result);
     result += length;
     count  += length;
   }
   intoUTF8(0, result);

   return count;
}

