//----------------------------------------------------------------------------
//
//       Copyright (c) 2010-2020 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       DataSource.cpp
//
// Purpose-
//       DataSource implementation methods.
//
// Last change date-
//       2020/10/02
//
//----------------------------------------------------------------------------
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <com/Debug.h>

#include "com/DataSource.h"

//----------------------------------------------------------------------------
//
// Method-
//       DataSource::~DataSource
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   DataSource::~DataSource( void )  // Destructor
{
   reset();
}

//----------------------------------------------------------------------------
//
// Method-
//       DataSource::DataSource
//
// Purpose-
//       Default constructor.
//
//----------------------------------------------------------------------------
   DataSource::DataSource( void )   // Constructor
:  origin(NULL)
,  offset(0)
,  length(0)
,  width(0)
,  name()
,  line(0)
,  column(0)
{
}

//----------------------------------------------------------------------------
//
// Method-
//       DataSource::DataSource
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   DataSource::DataSource(          // Constructor
     const char*       name,        // Source name
     void*             origin,      // Source address
     size_t            length)      // Source length
:  origin(NULL)
,  offset(0)
,  length(0)
,  width(0)
,  name()
,  line(0)
,  column(0)
{
   if( name == NULL )               // Set the name
     name= "";
   this->name= name;

   if( origin != NULL && length > 0 && (length+4) > length )
   {
     this->origin= (unsigned char*)malloc(length + 4);
     if( this->origin != NULL )
     {
       this->length= length;
       memcpy(this->origin, origin, length);
       memset(this->origin+length, 0, 4); // Clear ending "character"

       setWidth();
     }
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       DataSource::DataSource
//
// Purpose-
//       Copy constructor.
//
//----------------------------------------------------------------------------
   DataSource::DataSource(          // Copy constructor
     const DataSource& source)      // Source DataSource
:  origin(NULL)
,  offset(0)
,  length(0)
,  width(0)
,  name()
,  line(0)
,  column(0)
{
   operator=(source);               // Use assignment operator to copy
}

//----------------------------------------------------------------------------
//
// Method-
//       DataSource::operator=
//
// Purpose-
//       Assignment operator
//
//----------------------------------------------------------------------------
DataSource&                         // Always *this
   DataSource::operator=(           // Assign
     const DataSource& source)      // From this source
{
   reset();                         // Reset the DataSource

   origin= (unsigned char*)malloc(source.length + 4);
   if( origin != NULL )
   {
     name=   source.name;
     length= source.length;
     width=  source.width;

     memcpy(origin, source.origin, length);
     memset(origin+length, 0, 4); // Clear ending "character"
   }

   return *this;
}

//----------------------------------------------------------------------------
//
// Method-
//       DataSource::setOffset
//
// Purpose-
//       Set Data offset
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   DataSource::setOffset(           // Set Data offset
     size_t            offset)      // To this value
{
   if( offset > length )
     return CC_ERR;

   this->offset= offset;
   return 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       DataSource::clone
//
// Purpose-
//       Clone this DataSource.
//
//----------------------------------------------------------------------------
DataSource*                         // Resultant DataSource
   DataSource::clone(               // Clone this DataSource
     const char*       name) const  // With this (relative) name
{
   return new DataSource(name, origin, length);
}

//----------------------------------------------------------------------------
//
// Method-
//       DataSource::get
//
// Purpose-
//       Get the next character.
//
//----------------------------------------------------------------------------
int                                 // The next data "character"
   DataSource::get( void )          // Get next data "character"
{
   int width= this->width;          // The current data width
   if( width <= 0 )
   {
     if( width == 0 )
       width= 1;
     else
       width= (-width);
   }

   unsigned char buffer[4];         // Input buffer
   if( width > 4 )                  // (Should not occur)
     width= 4;

   int L= read(buffer, width);      // Read next character
   if( L == 0 )
     return CC_EOF;

   if( L != width )
     throwf("DataSource::get %d= read(%d)", L, width);

   int result= CC_ERR;              // If invalid width
   switch( this->width )
   {
     case -4:
       result= invert32(*((uint32_t*)buffer));
       break;

     case -2:
       result= invert16(*((uint16_t*)buffer));
       break;

     case 0:
     case 1:
       result= (int)*(buffer);
       break;

     case 2:
       result= (int)*((uint16_t*)buffer);
       break;

     case 4:
       result= (int)*((uint32_t*)buffer);
       break;

     default:
       break;
   }

   column++;
   if( result == '\n' || result == '\r' || result == '\0' )
   {
     column= 0;
     if( result != '\r' )
       line++;
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       DataSource::getLine
//
// Purpose-
//       Get the next data line (ignoring '\r')
//
// Implementation notes-
//       This method reads the entire line, discarding extra characters.
//
//----------------------------------------------------------------------------
int                                 // The line delimiter
   DataSource::getLine(             // Get next data line
     void*             target,      // Data address
     unsigned          length)      // Data length (in bytes)
{
   int                 C;           // Current character
   unsigned            offset;      // Current offset

   switch( width )
   {
     case -4:
     case 4:
       length /= 4;
       break;

     case -2:
     case 2:
       length /= 2;
       break;

     default:
       break;
   }

   for(offset= 0;; offset++)
   {
     do
     {
       C= get();
     } while( C == '\r' );

     if( C <= 0 || C == '\n' )
       break;

     if( offset < length )
     {
       switch( width )
       {
         case -4:
         case 4:
           *(((uint32_t*)target)+offset)= C;
           break;

         case -2:
         case 2:
           *(((uint16_t*)target)+offset)= C;
           break;

         case 0:
         case 1:
         default:
           *(((char*)target)+offset)= C;
           break;
       }
     }
   }

   if( offset >= length )
     C= CC_LTL;
   else
   {
     switch( width )
     {
       case -4:
       case 4:
         *(((uint32_t*)target)+offset)= 0;
         break;

       case -2:
       case 2:
         *(((uint16_t*)target)+offset)= 0;
         break;

       case 0:
       case 1:
       default:
         *(((char*)target)+offset)= '\0';
         break;
     }
   }

   return C;
}

//----------------------------------------------------------------------------
//
// Method-
//       DataSource::read
//
// Purpose-
//       Read from DataSource.
//
//----------------------------------------------------------------------------
unsigned int                        // The number of bytes read
   DataSource::read(                // Read from DataSource
     void*             addr,        // Data address
     unsigned int      size)        // Data length (in bytes)
{
   if( origin == NULL )             // Disallow read when no data
     return 0;

   if( (offset + size) > length )   // Disallow read past EOF
     size= length - offset;

   if( size > 0 )
   {
     memcpy(addr, origin + offset, size);
     offset += size;
   }

   return size;
}

//----------------------------------------------------------------------------
//
// Method-
//       DataSource::reset
//
// Purpose-
//       Reset this DataSource.
//
//----------------------------------------------------------------------------
void
   DataSource::reset( void )        // Reset this DataSource
{
   if( origin != NULL )
   {
     free(origin);
     origin= NULL;
   }

   offset= 0;
   length= 0;
   width= 0;

   name= "";
   line= 0;
   column= 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       DataSource::setWidth
//
// Purpose-
//       Set the data width from the file header.
//
// Header-
//       0xFFFE0000 UTF-32, little endian
//       0x0000FEFF UTF-32, big endian
//       0xFFFE     UTF-16, little endian
//       0xFEFF     UTF-16, big endian
//
//----------------------------------------------------------------------------
void
   DataSource::setWidth(            // Set data width
     const unsigned char*
                       origin,      // From this origin
     const unsigned    length)      // And this length
{
   width= 1;
   if( length >= 4 && (length&3) == 0 ) // If possibly UTF-32
   {
     if( *(origin) == '\0' && *(origin+1) == '\0'
         && *(origin+2) == 0xFE && *(origin+3) == 0xFF )
     {
       width= 4;
       if( *((uint32_t*)origin) == 0xFFFE0000 )
         width= -4;

       return;
     }

     if( *(origin) == 0xFF && *(origin+1) == 0xFE
         && *(origin+2) == '\0' && *(origin+3) == '\0' )
     {
       width= -4;
       if( *((uint32_t*)origin) == 0x0000FEFF )
         width= 4;

       return;
     }
   }

   if( length >= 2 && (length&1) == 0 ) // If possibly UTF-16
   {
     if( *(origin) == 0xFE && *(origin+1) == 0xFF )
     {
       width= 2;
       if( *((uint16_t*)origin) == 0xFFFE )
         width= -2;

       return;
     }

     if( *(origin) == 0xFF && *(origin+1) == 0xFE )
     {
       width= -2;
       if( *((uint16_t*)origin) == 0xFEFF )
         width= 2;

       return;
     }
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       DataSource::invert16
//
// Purpose-
//       Invert UTF-16 character
//
// Implementation notes-
//       0xabcd => 0x0000cdab
//
//----------------------------------------------------------------------------
int                                 // The inverted data "character"
   DataSource::invert16(            // Invert
     unsigned int      C)           // This UTF-16 character
{
   int result= ((C&0x0000FF00)>>8) | ((C&0x000000FF)<<8);

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       DataSource::invert32
//
// Purpose-
//       Invert UTF-32 character
//
// Implementation notes-
//       0xabcdefhi => 0xhiefcdab
//
//----------------------------------------------------------------------------
int                                 // The inverted data "character"
   DataSource::invert32(            // Invert
     unsigned int      C)           // This UTF-32 character
{
   int result= ((C&0xFF000000)>>24) | ((C&0x00FF0000)>> 8)
             | ((C&0x0000FF00)<< 8) | ((C&0x000000FF)<<24);

   return result;
}

