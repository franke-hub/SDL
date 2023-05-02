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
//       Reader.cpp
//
// Purpose-
//       Reader object methods.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <com/define.h>
#include <com/Debug.h>
#include "com/Reader.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#define DEFAULT_SIZE 32768          // Default buffer size
#define MINIMUM_SIZE 128            // Minimum buffer size

//----------------------------------------------------------------------------
//
// Method-
//       Reader::~Reader
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Reader::~Reader( void )          // Destructor
{
   #ifdef HCDM
     debugf("Reader(%p)::~Reader()\n", this);
   #endif

   if( buffer != NULL )
   {
     delete[] buffer;
     buffer= NULL;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Reader::Reader
//
// Purpose-
//       Default constructor.
//
//----------------------------------------------------------------------------
   Reader::Reader( void )           // Default constructor
:  buffer(NULL)
,  length(0)
,  size(0)
,  used(0)
{
   #ifdef HCDM
     debugf("Reader(%p)::Reader()\n", this);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       Reader::Reader
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Reader::Reader(                  // Constructor
     Size_t            size)        // Initial length
:  buffer(NULL)
,  length(size)
,  size(0)
,  used(0)
{
   #ifdef HCDM
     debugf("Reader(%p)::Reader(%lu)\n", this, size);
   #endif

   resize(size);
}

//----------------------------------------------------------------------------
//
// Method-
//       Reader::getState
//
// Purpose-
//       Return STATE_RESET when no getState() method available
//
//----------------------------------------------------------------------------
Reader::State                       // The current State
   Reader::getState( void ) const   // Get current State
{
   return STATE_RESET;
}

//----------------------------------------------------------------------------
//
// Method-
//       Reader::getAvail
//
// Purpose-
//       Get the number of available buffer bytes
//
//----------------------------------------------------------------------------
Reader::Size_t                      // The number of bytes available
   Reader::getAvail( void )         // Get number of bytes available
{
   #ifdef HCDM
     debugf("Reader(%p)::getAvail()\n", this);
   #endif

   Size_t              result= 0;   // Resultant

   if( getState() == STATE_INPUT )
   {
     if( used >= size )
       input();

     result= size - used;
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Reader::getLength
//
// Purpose-
//       Get the number of buffer bytes
//
//----------------------------------------------------------------------------
Reader::Size_t                      // The number of bytes
   Reader::getLength( void )        // Get number of bytes
{
   #ifdef HCDM
     debugf("Reader(%p)::getLength()\n", this);
   #endif

   return length;
}

//----------------------------------------------------------------------------
//
// Method-
//       Reader::reset
//
// Purpose-
//       Reset (deallocate) the buffer
//
//----------------------------------------------------------------------------
void
   Reader::reset( void )            // Reset the buffer
{
   #ifdef HCDM
     debugf("Reader(%p)::reset()\n", this);
   #endif

   if( getState() != STATE_RESET )
   {
     debugf("Writer(%p)::reset() state(%d)\n", this, getState());
     throw "InvalidStateException";
   }

   if( buffer != NULL )
   {
     delete[] buffer;
     buffer= NULL;
   }

   used= size= 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       Reader::resize
//
// Purpose-
//       Resize the buffer
//
//----------------------------------------------------------------------------
void
   Reader::resize(                  // Resize the buffer
     Size_t            size)        // The new length
{
   #ifdef HCDM
     debugf("Reader(%p)::resize(%lu)\n", this, size);
   #endif

   reset();

   if( size < MINIMUM_SIZE )
     size= MINIMUM_SIZE;

   length= size;
   buffer= new Byte[size];
}

//----------------------------------------------------------------------------
//
// Method-
//       Reader::fill
//
// Purpose-
//       Fill the Reader
//
//----------------------------------------------------------------------------
void
   Reader::fill( void )             // Fill the Reader
{
   #ifdef HCDM
     debugf("Reader(%p)::fill()\n", this);
   #endif

   while( size < length )
   {
     if( input() != 0 )
       break;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Reader::get
//
// Purpose-
//       Get from the Reader
//
//----------------------------------------------------------------------------
int                                 // The next character
   Reader::get( void )              // Get from the Reader
{
   #ifdef HCDM
     debugf("Reader(%p)::get()\n", this);
   #endif

   int                 result;      // Resultant

   if( used >= size )
   {
     result= input();
     if( used < size )
       result= (unsigned char)buffer[used++];
   }
   else
     result= (unsigned char)buffer[used++];

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Reader::prior
//
// Purpose-
//       Re-get from the Reader
//
//----------------------------------------------------------------------------
int                                 // The last character
   Reader::prior( void ) const      // Re-get from the Reader
{
   #ifdef HCDM
     debugf("Reader(%p)::prior()\n", this);
   #endif

   int                 result;      // Resultant

   if( used == 0 )
   {
     if( getState() == STATE_RESET )
       result= RC_USER;
     else
       result= RC_EOF;
   }
   else
     result= buffer[used-1];

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Reader::pull
//
// Purpose-
//       Pull from the Reader
//
//----------------------------------------------------------------------------
const Reader::Byte*                 // The pull buffer
   Reader::pull(                    // Pull from the Reader
     Size_t            L)           // For this length
{
   #ifdef HCDM
     debugf("Reader(%p)::pull(%lu)\n", this, L);
   #endif

   Byte*               result= NULL;

   if( used >= size || L > (size-used) )
     input();

   if( L <= (size-used) )
   {
     result= buffer + used;
     used += L;
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Reader::read
//
// Purpose-
//       Read from the Reader
//
//----------------------------------------------------------------------------
Reader::Size_t                      // The number of Bytes read
   Reader::read(                    // Read from the Reader
     Byte*             A,           // Into this address
     Size_t            L)           // For this length
{
   #ifdef HCDM
     debugf("Reader(%p)::read(%p,%lu)\n", this, A, L);
   #endif

   Size_t              result= 0;   // Resultant
   Size_t              xfer;        // Transfer length

   while( result < L )
   {
     if( used >= size )
     {
       input();
       if( used >= size )
         break;
     }

     xfer= L - result;
     if( xfer > (size - used) )
       xfer= size - used;

     memcpy(A + result, buffer + used, xfer);
     used += xfer;
     result += xfer;
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Reader::findBlank
//
// Purpose-
//       Continue reading until a blank is found
//
//----------------------------------------------------------------------------
int                                 // The next blank character
   Reader::findBlank( void )        // Read to the next blank character
{
   int                 result;      // Resultant

   do
   {
     result= get();
   }
   while( result > 0 && !isspace(result) );

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Reader::skipBlank
//
// Purpose-
//       Continue reading until a non-blank is found
//
//----------------------------------------------------------------------------
int                                 // The next non-blank character
   Reader::skipBlank( void )        // Read to the next non-blank character
{
   int                 result;      // Resultant

   do
   {
     result= get();
   }
   while( result > 0 && isspace(result) );

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Reader::readLine
//
// Purpose-
//       Read the next line.
//
//----------------------------------------------------------------------------
int                                 // The line delimiter, normally '\n'
   Reader::readLine(                // Read the next line
     Byte*             A,           // Data address
     Size_t            L)           // Data length
{
   int                 result;      // Resultant
   Size_t              X= 0;        // Current offset

   do
   {
     result= get();
     while( result == '\r' )
       result= get();

     if( result == '\n' || result <= 0 )
       break;

     if( X < L )
       A[X++]= result;
   }
   while( TRUE );

   if( X >= L )
   {
     result= RC_SKIP;
     X= L-1;
   }
   A[X]= '\0';

   //-------------------------------------------------------------------------
   // As a special case, if the RC_EOF delimiter is found and some data
   // are present, the normal '\n' delimiter is returned.
   //-------------------------------------------------------------------------
   if( result == RC_EOF && X > 0 )
     result= '\n';

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Reader::skipLine
//
// Purpose-
//       Skip the next line.
//
//----------------------------------------------------------------------------
int                                 // The line delimiter
   Reader::skipLine( void )         // Skip the next line
{
   int                 result;      // Resultant

   do
   {
     result= get();
     while( result == '\r' )
       result= get();
   }
   while( result != '\n' && result > 0 );

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       MediaReader::~MediaReader
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   MediaReader::~MediaReader( void ) // Destructor
{
   #ifdef HCDM
     debugf("MediaReader(%p)::~MediaReader()\n", this);
   #endif

   if( state != STATE_RESET )
     close();

   media= NULL;
   state= STATE_RESET;
}

//----------------------------------------------------------------------------
//
// Method-
//       MediaReader::MediaReader
//
// Purpose-
//       Default constructor.
//
//----------------------------------------------------------------------------
   MediaReader::MediaReader( void ) // Default constructor
:  Reader()
,  media(NULL)
,  state(STATE_RESET)
{
   #ifdef HCDM
     debugf("MediaReader(%p)::MediaReader()\n", this);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       MediaReader::MediaReader
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   MediaReader::MediaReader(        // Constructor
     Size_t            size)        // Initial length
:  Reader()
,  media(NULL)
,  state(STATE_RESET)
{
   #ifdef HCDM
     debugf("MediaReader(%p)::MediaReader(%lu)\n", this, size);
   #endif

   resize(size);
}

//----------------------------------------------------------------------------
//
// Method-
//       MediaReader::attach
//
// Purpose-
//       Attach Media
//
//----------------------------------------------------------------------------
void
   MediaReader::attach(             // Attach Media
     Media&            media)       // The new Media
{
   #ifdef HCDM
     debugf("MediaReader(%p)::attach(%p)\n", this, &media);
   #endif

   if( state != STATE_RESET )
   {
     debugf("MediaReader(%p)::attach(%p) state(%d)\n", this, &media, state);
     throw "InvalidStateException";
   }

   MediaReader::media= &media;
   if( MediaReader::media == NULL )
   {
     debugf("MediaReader(%p)::attach(%p) state(%d)\n", this, &media, state);
     throw "NullPointerException";
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       MediaReader::detach
//
// Purpose-
//       Detach Media
//
//----------------------------------------------------------------------------
void
   MediaReader::detach( void )      // Detach Media
{
   #ifdef HCDM
     debugf("MediaReader(%p)::detach()\n", this);
   #endif

   if( state != STATE_RESET )
   {
     debugf("MediaReader(%p)::detach() state(%d)\n", this, state);
     throw "InvalidStateException";
   }

   media= NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       MediaReader::getState
//
// Purpose-
//       Get the current State
//
//----------------------------------------------------------------------------
MediaReader::State                  // The current State
   MediaReader::getState( void ) const // Get current State
{
   #ifdef HCDM
     debugf("MediaReader(%p)::getState()\n", this);
   #endif

   State               result= state; // Resultant

   if( state != STATE_RESET )
   {
     if( media->getState() == STATE_EOF )
       result= STATE_EOF;
     if( media->getState() == STATE_ERROR )
       result= STATE_ERROR;
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       MediaReader::open
//
// Purpose-
//       Open the MediaReader
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   MediaReader::open(               // Open the MediaReader
     const char*       name,        // The MediaReader name
     const char*       mode)        // The open mode
{
   #ifdef HCDM
     debugf("MediaReader(%p)::open(%s,%s)\n", this, name, mode);
   #endif

   int                 result= 0;   // Resultant

   if( name == NULL )
     name= "<";

   if( mode == NULL )
     mode= MODE_READ;

   if( strcmp(mode, MODE_READ) != 0
       && strcmp(mode, MODE_INOUT) != 0
       && strcmp(mode, MODE_OUTIN) != 0 )
   {
     debugf("MediaReader(%p)::open(%s,%s)\n",
            this, name, mode);
     throw "InvalidArgumentException";
   }

   if( state != STATE_RESET )
   {
     debugf("MediaReader(%p)::open(%s,%s) state(%d)\n",
            this, name, mode, state);
     throw "InvalidStateException";
   }

   if( media == NULL )
   {
     debugf("MediaReader(%p)::open(%s,%s) media(%p)\n",
            this, name, mode, media);
     throw "NullPointerException";
   }

   if( buffer == NULL )
   {
     if( length == 0 )
       length= DEFAULT_SIZE;

     resize(length);
   }

   if( media->getState() == STATE_RESET )
   {
     result= media->open(name, mode);
     if( result == 0 )
       state= STATE_INPUT;
   }
   else
   {
     if( media->getState() == STATE_INPUT
         || media->getState() == STATE_INOUT
         || media->getState() == STATE_OUTIN
         || media->getState() == STATE_EOF )
       state= STATE_INPUT;
     else
     {
       debugf("MediaReader(%p)::open(%s,%s) %d= media->getState()\n",
              this, name, mode, media->getState());
       throw "InvalidStateException";
     }
   }

   used= size= 0;

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       MediaReader::close
//
// Purpose-
//       Close the MediaReader
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   MediaReader::close( void )       // Close the MediaReader
{
   #ifdef HCDM
     debugf("MediaReader(%p)::close()\n", this);
   #endif

   int                 result= 0;   // Resultant

   if( state != STATE_RESET )
   {
     flush();

     state= STATE_RESET;
     if( media->getState() != STATE_RESET )
       result= media->close();
   }

   used= size= 0;
   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       MediaReader::flush
//
// Purpose-
//       Flush the MediaReader
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   MediaReader::flush( void )       // Flush the MediaReader
{
   #ifdef HCDM
     debugf("MediaReader(%p)::flush()\n", this);
   #endif

   int                 result= RC_USER; // Resultant

   if( state == STATE_INPUT )
   {
     used= size= 0;
     result= media->flush();
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       MediaReader::input
//
// Purpose-
//       Read from the Media
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   MediaReader::input( void )       // Read input
{
   #ifdef HCDM
     debugf("MediaReader(%p)::input()\n", this);
   #endif

   int                 result= 0;   // Return code
   Size_t              L;           // Read length

   if( state != STATE_INPUT )
   {
     debugf("MediaReader(%p)::input() state(%d)\n", this, state);
     throw "InvalidStateException";
   }

   // Preserve existing data
   if( used < size && used > 0 )
     memcpy(buffer, buffer+used, size-used);

   size -= used;
   used= 0;

   // Fill with new data
   if( size < length )
   {
     L= media->read(buffer+size, (length - size));
     if( L == 0 )
     {
       switch( media->getState() )
       {
         case Media::STATE_INPUT:
           result= RC_NULL;
           break;

         case Media::STATE_EOF:
           result= RC_EOF;
           break;

         default:
           result= RC_MEDIA_FAULT;
           break;
       }
     }

     size += L;
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       FileReader::~FileReader
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   FileReader::~FileReader( void )  // Destructor
{
   #ifdef HCDM
     debugf("FileReader(%p)::~FileReader()\n", this);
   #endif

   if( getState() != STATE_RESET )
     close();
}

//----------------------------------------------------------------------------
//
// Method-
//       FileReader::FileReader
//
// Purpose-
//       Default constructor.
//
//----------------------------------------------------------------------------
   FileReader::FileReader( void )   // Default constructor
:  Reader()
,  media()
,  state(STATE_RESET)
{
   #ifdef HCDM
     debugf("FileReader(%p)::FileReader()\n", this);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       FileReader::FileReader
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   FileReader::FileReader(          // Constructor
     const char*       name)        // The Media name
:  Reader()
,  media()
,  state(STATE_RESET)
{
   #ifdef HCDM
     debugf("FileReader(%p)::FileReader(%s)\n", this, name);
   #endif

   open(name);
}

//----------------------------------------------------------------------------
//
// Method-
//       FileReader::getState
//
// Purpose-
//       Get the current State
//
//----------------------------------------------------------------------------
FileReader::State                   // The current State
   FileReader::getState( void ) const // Get current State
{
   #ifdef HCDM
     debugf("FileReader(%p)::getState()\n", this);
   #endif

   State               result= state; // Resultant

   if( state != STATE_RESET )
   {
     if( media.getState() == STATE_EOF )
       result= STATE_EOF;
     if( media.getState() == STATE_ERROR )
       result= STATE_ERROR;
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       FileReader::open
//
// Purpose-
//       Open the FileReader
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   FileReader::open(                // Open the FileReader
     const char*       name,        // The FileReader name
     const char*       mode)        // The open mode
{
   #ifdef HCDM
     debugf("FileReader(%p)::open(%s,%s)\n", this, name, mode);
   #endif

   int                 result= 0;   // Resultant

   if( name == NULL )
     name= "<";

   if( mode == NULL )
     mode= MODE_READ;

   if( strcmp(mode, MODE_READ) != 0
       && strcmp(mode, MODE_INOUT) != 0
       && strcmp(mode, MODE_OUTIN) != 0 )
   {
     debugf("FileReader(%p)::open(%s,%s)\n",
            this, name, mode);
     throw "InvalidArgumentException";
   }

   if( state != STATE_RESET )
   {
     debugf("FileReader(%p)::open(%s,%s) state(%d)\n",
            this, name, mode, state);
     throw "InvalidStateException";
   }

   if( buffer == NULL )
   {
     if( length == 0 )
       length= DEFAULT_SIZE;

     resize(length);
   }

   if( media.getState() == STATE_RESET )
   {
     result= media.open(name, mode);
     if( result == 0 )
       state= STATE_INPUT;
   }
   else
   {
     if( media.getState() == STATE_INPUT
         || media.getState() == STATE_INOUT
         || media.getState() == STATE_OUTIN
         || media.getState() == STATE_EOF )
       state= STATE_INPUT;
     else
     {
       debugf("FileReader(%p)::open(%s,%s) %d= media.getState()\n",
              this, name, mode, media.getState());
       throw "InvalidStateException";
     }
   }

   used= size= 0;

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       FileReader::close
//
// Purpose-
//       Close the FileReader
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   FileReader::close( void )        // Close the FileReader
{
   #ifdef HCDM
     debugf("FileReader(%p)::close()\n", this);
   #endif

   int                 result= 0;   // Resultant

   if( state != STATE_RESET )
   {
     flush();

     state= STATE_RESET;
     if( media.getState() != STATE_RESET )
       result= media.close();
   }

   used= size= 0;
   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       FileReader::flush
//
// Purpose-
//       Flush the FileReader
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   FileReader::flush( void )        // Flush the FileReader
{
   #ifdef HCDM
     debugf("FileReader(%p)::flush()\n", this);
   #endif

   int                 result= RC_USER; // Resultant

   if( state == STATE_INPUT )
   {
     used= size= 0;
     result= media.flush();
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       FileReader::input
//
// Purpose-
//       Read from the Media
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   FileReader::input( void )        // Read input
{
   #ifdef HCDM
     debugf("FileReader(%p)::input()\n", this);
   #endif

   int                 result= 0;   // Return code
   Size_t              L;           // Read length

   if( state != STATE_INPUT )
   {
     debugf("FileReader(%p)::input() state(%d)\n", this, state);
     throw "InvalidStateException";
   }

   // Preserve existing data
   if( used < size && used > 0 )
     memcpy(buffer, buffer+used, size-used);

   size -= used;
   used= 0;

   // Fill with new data
   if( size < length )
   {
     L= media.read(buffer+size, (length - size));
     if( L == 0 )
     {
       switch( media.getState() )
       {
         case Media::STATE_INPUT:
           result= RC_NULL;
           break;

         case Media::STATE_EOF:
           result= RC_EOF;
           break;

         default:
           result= RC_MEDIA_FAULT;
           break;
       }
     }

     size += L;
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       LineReader::~LineReader
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   LineReader::~LineReader( void )  // Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       LineReader::LineReader
//
// Purpose-
//       Default constructor.
//
//----------------------------------------------------------------------------
   LineReader::LineReader( void )   // Default constructor
:  FileReader()
,  line(0)
,  column(0)
{
}

//----------------------------------------------------------------------------
//
// Method-
//       LineReader::LineReader
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   LineReader::LineReader(          // Constructor
     const char*       name)        // The Media name
:  FileReader()
,  line(0)
,  column(0)
{
   open(name);
}

//----------------------------------------------------------------------------
//
// Method-
//       LineReader::getColumn
//
// Purpose-
//       Return the current column
//
//----------------------------------------------------------------------------
unsigned long                       // The current column
   LineReader::getColumn( void ) const // Get current column
{
   return column;
}

//----------------------------------------------------------------------------
//
// Method-
//       LineReader::getLine
//
// Purpose-
//       Return the current line
//
//----------------------------------------------------------------------------
unsigned long                       // The current line
   LineReader::getLine( void ) const// Get current line
{
   return line;
}

//----------------------------------------------------------------------------
//
// Method-
//       LineReader::open
//
// Purpose-
//       Open the LineReader
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   LineReader::open(                // Open the LineReader
     const char*       name,        // The LineReader name
     const char*       mode)        // The open mode
{
   int                 result;      // Resultant

   result= FileReader::open(name, mode);
   if( result == 0 )
     line= column= 0;

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       LineReader::close
//
// Purpose-
//       Close the LineReader
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   LineReader::close( void )        // Close the LineReader
{
   int                 result= 0;   // Resultant

   result= FileReader::close();
   line= column= 0;

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       LineReader::get
//
// Purpose-
//       Get from the LineReader
//
//----------------------------------------------------------------------------
int                                 // The next character
   LineReader::get( void )          // Get from the LineReader
{
   int                 result;      // Resultant

   if( line == 0 )
     line= 1;

   result= FileReader::get();
   while( result == '\r' )
   {
     column= 0;
     result= FileReader::get();
   }

   column++;
   if( result == '\n' )
   {
     line++;
     column= 0;
   }

   return result;
}

