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
//       TextBuffer.cpp
//
// Purpose-
//       TextBuffer implementation methods.
//
// Last change date-
//       2010/01/01
//
// Implementation notes-
//       Method toChar returns the remainder of the text buffer, which the
//       caller may modify. Since there is only one buffer, the caller cannot
//       modify the text buffer and use the get method to retrieve data.
//
//----------------------------------------------------------------------------
#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "Common.h"
#include "TextBuffer.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef SCDM
#undef  SCDM                        // If defined, Soft Core Debug Mode
#endif

//----------------------------------------------------------------------------
//
// Method-
//       TextBuffer::~TextBuffer
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   TextBuffer::~TextBuffer( void )  // Destructor
{
   reset();
}

//----------------------------------------------------------------------------
//
// Method-
//       TextBuffer::TextBuffer
//
// Purpose-
//       Default constructor.
//
//----------------------------------------------------------------------------
   TextBuffer::TextBuffer( void )   // Default constructor
:  inp(-1)
,  out(0)
,  textSize(sizeof buff)
,  textBuff(buff)
{
}

//----------------------------------------------------------------------------
//
// Method-
//       TextBuffer::TextBuffer
//
// Purpose-
//       Copy constructor.
//
//----------------------------------------------------------------------------
   TextBuffer::TextBuffer(          // Copy constructor
     const TextBuffer& source)      // Source TextBuffer
:  inp(-1)
,  out(0)
,  textSize(sizeof buff)
,  textBuff(buff)
{
   put(source);
}

//----------------------------------------------------------------------------
//
// Method-
//       TextBuffer::operator=
//
// Purpose-
//       Assignment operator.
//
//----------------------------------------------------------------------------
TextBuffer&                         // (*this)
   TextBuffer::operator=(           // Assignment operator
     const TextBuffer& source)      // Source TextBuffer
{
   if( &source != this )
   {
     reset();
     put(source);
   }

   return (*this);
}

//----------------------------------------------------------------------------
//
// Method-
//       TextBuffer::operator[]
//
// Purpose-
//       Get character from the TextBuffer.
//
//----------------------------------------------------------------------------
int                                 // The character (EOF iff invalid index)
   TextBuffer::operator[](          // Get TextBuffer character
     int               X) const     // At this position
{
   int                 result= (-1);// Resultant

   if( X >= 0 && X < out )
     result= textBuff[X];

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       TextBuffer::get
//
// Purpose-
//       Get next character from the TextBuffer.
//
//----------------------------------------------------------------------------
int                                 // The next character
   TextBuffer::get( void )          // Get next character
{
   int                 result= (-1);// Resultant

   if( inp < 0 )
   {
     inp= 0;
     if( out >= textSize )
       expand(out+8);

     textBuff[out]= '\0';
   }

   if( inp < out )
     result= textBuff[inp++];

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       TextBuffer::put(int)
//
// Purpose-
//       Put character into the TextBuffer.
//
//----------------------------------------------------------------------------
void
   TextBuffer::put(                 // Put character into the TextBuffer
     int               C)           // The character
{
   if( inp >= 0 )
     throw "TextBuffer::put Exception";

   if( out >= textSize )
     expand(out+4096);

   textBuff[out++]= C;
}

//----------------------------------------------------------------------------
//
// Method-
//       TextBuffer::put(const char*)
//
// Purpose-
//       Put string into the TextBuffer.
//
//----------------------------------------------------------------------------
void
   TextBuffer::put(                 // Put string into the TextBuffer
     const char*       string)      // The string
{
   if( inp >= 0 )
     throw "TextBuffer::put Exception";

   int m= strlen(string);
   for(int i= 0; i<m; i++)
   {
     if( out >= textSize )
       expand(out+4096);

     textBuff[out++]= string[i];
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       TextBuffer::put(const char*, unsigned)
//
// Purpose-
//       Put text into the TextBuffer.
//
//----------------------------------------------------------------------------
void
   TextBuffer::put(                 // Put text into the TextBuffer
     const char*       text,        // The text
     unsigned          size)        // The text length
{
   if( inp >= 0 )
     throw "TextBuffer::put Exception";

   for(unsigned i= 0; i<size; i++)
   {
     if( out >= textSize )
       expand(out+4096);

     textBuff[out++]= text[i];
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       TextBuffer::put(const std::string&)
//
// Purpose-
//       Put string into the TextBuffer.
//
//----------------------------------------------------------------------------
void
   TextBuffer::put(                 // Put string into the TextBuffer
     const std::string&string)      // The string
{
   if( inp >= 0 )
     throw "TextBuffer::put Exception";

   int m= string.size();
   for(int i= 0; i<m; i++)
   {
     if( out >= textSize )
       expand(out+4096);

     textBuff[out++]= string[i];
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       TextBuffer::put(const TextBuffer&)
//
// Purpose-
//       Put TextBuffer into the TextBuffer.
//
//----------------------------------------------------------------------------
void
   TextBuffer::put(                 // Put TextBuffer into this TextBuffer
     const TextBuffer& source)      // Source TextBuffer
{
   if( inp >= 0 )
     throw "TextBuffer::put Exception";

   for(int i= 0; i<source.out; i++)
   {
     if( out >= textSize )
       expand(out+4096);

     textBuff[out++]= source.textBuff[i];
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       TextBuffer::reset
//
// Purpose-
//       Reset the TextBuffer.
//
//----------------------------------------------------------------------------
void
   TextBuffer::reset( void )        // Reset the TextBuffer
{
   if( textBuff != buff )
     free(textBuff);

   inp= (-1);
   out= 0;
   textSize= sizeof buff;
   textBuff= buff;
}

//----------------------------------------------------------------------------
//
// Method-
//       TextBuffer::size
//
// Purpose-
//       Get TextBuffer length.
//
//----------------------------------------------------------------------------
int                                 // The TextBuffer length
   TextBuffer::size( void ) const   // Get TextBuffer length
{
   return out;
}

//----------------------------------------------------------------------------
//
// Method-
//       TextBuffer::skip
//
// Purpose-
//       Skip characters.
//
//----------------------------------------------------------------------------
int                                 // The last skipped character
   TextBuffer::skip(                // Skip characters
     int               count)       // Number of characters to skip
{
   int                 result= (-1);// Resultant

   for(int i= 0; i<count; i++)
     result= get();

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       TextBuffer::toChar
//
// Purpose-
//       Convert to char*
//
//----------------------------------------------------------------------------
char*                               // Resultant
   TextBuffer::toChar( void )       // Convert to char*
{
   if( inp < 0 )
   {
     inp= 0;
     if( out >= textSize )
       expand(out+8);

     textBuff[out]= '\0';
   }

   return textBuff+inp;
}

//----------------------------------------------------------------------------
//
// Method-
//       TextBuffer::toString
//
// Purpose-
//       Convert to string.
//
//----------------------------------------------------------------------------
std::string                         // Resultant
   TextBuffer::toString( void ) const // Convert to string
{
   std::string result(textBuff,out);// Resultant
   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       TextBuffer::expand
//
// Purpose-
//       Expand the TextBuffer.
//
//----------------------------------------------------------------------------
void
   TextBuffer::expand(              // Expand the TextBuffer
     int               size)        // To this size
{
   void* text= malloc(size);        // Allocate a new buffer
   if( text == NULL )
     throw "TextBuffer::expand Exception";

   memcpy(text, textBuff, out);
   if( textBuff != buff )
     free(textBuff);
   textBuff= (char*)text;
   textSize= size;
}

