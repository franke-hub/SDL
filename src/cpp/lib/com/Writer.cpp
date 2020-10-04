//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2020 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Writer.cpp
//
// Purpose-
//       Writer object methods.
//
// Last change date-
//       2020/10/03
//
//----------------------------------------------------------------------------
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <com/define.h>
#include <com/Debug.h>
#include "com/Writer.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#define DEFAULT_SIZE 32768          // Default buffer size
#define MINIMUM_SIZE 128            // Minimum buffer size

#ifdef _OS_WIN
#define vsnprintf _vsnprintf
#endif

//----------------------------------------------------------------------------
//
// Method-
//       Writer::~Writer
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Writer::~Writer( void )          // Destructor
{
   #ifdef HCDM
     debugf("Writer(%p)::~Writer()\n", this);
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
//       Writer::Writer
//
// Purpose-
//       Default constructor.
//
//----------------------------------------------------------------------------
   Writer::Writer( void )           // Default constructor
:  buffer(NULL)
,  length(0)
,  size(0)
{
   #ifdef HCDM
     debugf("Writer(%p)::Writer()\n", this);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       Writer::Writer
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Writer::Writer(                  // Constructor
     Size_t            size)        // Initial length
:  buffer(NULL)
,  length(size)
,  size(0)
{
   #ifdef HCDM
     debugf("Writer(%p)::Writer(%lu)\n", this, size);
   #endif

   resize(size);
}

//----------------------------------------------------------------------------
//
// Method-
//       Writer::getState
//
// Purpose-
//       Return STATE_RESET when no getState() method available
//
//----------------------------------------------------------------------------
Writer::State                       // The current State
   Writer::getState( void ) const   // Get current State
{
   return STATE_RESET;
}

//----------------------------------------------------------------------------
//
// Method-
//       Writer::getAvail
//
// Purpose-
//       Get the number of available buffer bytes
//
//----------------------------------------------------------------------------
Writer::Size_t                      // The number of bytes available
   Writer::getAvail( void )         // Get number of bytes available
{
   #ifdef HCDM
     debugf("Writer(%p)::getAvail()\n", this);
   #endif

   Size_t              result= 0;   // Resultant

   if( getState() == STATE_OUTPUT )
   {
     if( size >= length )
       output();

     result= length - size;
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Writer::getLength
//
// Purpose-
//       Get the number of buffer bytes
//
//----------------------------------------------------------------------------
Writer::Size_t                      // The number of bytes
   Writer::getLength( void )        // Get number of bytes
{
   #ifdef HCDM
     debugf("Writer(%p)::getLength()\n", this);
   #endif

   return length;
}

//----------------------------------------------------------------------------
//
// Method-
//       Writer::reset
//
// Purpose-
//       Reset (deallocate) the buffer
//
//----------------------------------------------------------------------------
void
   Writer::reset( void )            // Reset the buffer
{
   #ifdef HCDM
     debugf("Writer(%p)::reset()\n", this);
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

   size= 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       Writer::resize
//
// Purpose-
//       Resize the buffer
//
//----------------------------------------------------------------------------
void
   Writer::resize(                  // Resize the buffer
     Size_t            size)        // The new length
{
   #ifdef HCDM
     debugf("Writer(%p)::resize(%lu)\n", this, size);
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
//       Writer::vprintf
//       Writer::printf
//
// Purpose-
//       Print into the Writer
//
//----------------------------------------------------------------------------
void
   Writer::vprintf(                 // Print into the Writer
     const char*       fmt,         // The format string
     va_list           argptr)      // VALIST
{
   #ifdef HCDM
     debugf("Writer(%p)::vprintf(%s,...)\n", this, fmt);
   #endif

   Size_t              L;           // Number of bytes transmitted

   if( getState() != STATE_OUTPUT )
   {
     debugf("Writer(%p)::printf(%s,...) state(%d)\n", this, fmt, getState());
     throw "InvalidStateException";
   }

   if( size >= length )
     output();

   L= vsnprintf(buffer+size, length-size, fmt, argptr);

   if( ssize_t(L) < 0 || L >= (length-size) )
   {
     output();
     L= vsnprintf(buffer+size, length-size, fmt, argptr);

     if( ssize_t(L) < 0 || L >= (length-size) )
     {
       debugf("Writer(%p)::printf(%s,...) buffer overflow\n", this, fmt);
       throw "OutputException";
     }
   }

   size += L;
}

void
   Writer::printf(                  // Print into the Writer
     const char*       fmt,         // The format string
                       ...)         // The PRINTF arguments
{
   #ifdef HCDM
     debugf("Writer(%p)::printf(%s,...)\n", this, fmt);
   #endif

   va_list             argptr;      // Argument list pointer

   va_start(argptr, fmt);           // Initialize va_ functions
   vprintf(fmt, argptr);
   va_end(argptr);                  // Close va_ functions
}

//----------------------------------------------------------------------------
//
// Method-
//       Writer::push
//
// Purpose-
//       Push into the Writer
//
//----------------------------------------------------------------------------
Writer::Byte*                       // The push buffer
   Writer::push(                    // Push into the Writer
     Size_t            L)           // For this length
{
   #ifdef HCDM
     debugf("Writer(%p)::push(%lu)\n", this, L);
   #endif

   Byte*               result= NULL;

   if( size >= length || L > (length-size) )
     output();

   if( L <= (length-size) )
   {
     result= buffer + size;
     size += L;
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Writer::put
//
// Purpose-
//       Put into the Writer
//
//----------------------------------------------------------------------------
void                                // The next character
   Writer::put(                     // Put into the Writer
     int               C)           // The next character
{
   #ifdef HCDM
     debugf("Writer(%p)::put()\n", this);
   #endif

   if( size >= length )
   {
     output();
     if( size >= length )
     {
       debugf("Writer(%p)::put() failure\n", this);
       throw "OutputException";
     }
   }

   buffer[size++]= C;
}

//----------------------------------------------------------------------------
//
// Method-
//       Writer::write
//
// Purpose-
//       Write into the Writer
//
//----------------------------------------------------------------------------
Writer::Size_t                      // The number of Bytes written
   Writer::write(                   // Write into the Writer
     const Byte*       A,           // From this address
     Size_t            L)           // For this length
{
   #ifdef HCDM
     debugf("Writer(%p)::write(%p,%lu)\n", this, A, L);
   #endif

   Size_t              result= 0;   // Resultant
   Size_t              xfer;        // Transfer length

   while( result < L )
   {
     if( size >= length )
     {
       output();
       if( size >= length )
         break;
     }

     xfer= L - result;
     if( xfer > (length - size) )
       xfer= length - size;

     memcpy(buffer + size, A + result, xfer);
     size += xfer;
     result += xfer;
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Writer::writeLine
//
// Purpose-
//       Write a NULL delimited output line, then '\n'.
//
//----------------------------------------------------------------------------
void
   Writer::writeLine(               // Write line into the Writer
     const Byte*       A)           // From this address
{
   #ifdef HCDM
     debugf("Writer(%p)::write(%s)\n", this, A);
   #endif

   while( (*A) != '\0' )
   {
     put(*A);
     A++;
   }
   put('\n');
}

//----------------------------------------------------------------------------
//
// Method-
//       MediaWriter::~MediaWriter
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   MediaWriter::~MediaWriter( void ) // Destructor
{
   #ifdef HCDM
     debugf("MediaWriter(%p)::~MediaWriter()\n", this);
   #endif

   if( state != STATE_RESET )
     close();

   media= NULL;
   state= STATE_RESET;
}

//----------------------------------------------------------------------------
//
// Method-
//       MediaWriter::MediaWriter
//
// Purpose-
//       Default constructor.
//
//----------------------------------------------------------------------------
   MediaWriter::MediaWriter( void ) // Default constructor
:  Writer()
,  media(NULL)
,  state(STATE_RESET)
{
   #ifdef HCDM
     debugf("MediaWriter(%p)::MediaWriter()\n", this);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       MediaWriter::MediaWriter
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   MediaWriter::MediaWriter(        // Constructor
     Size_t            size)        // Initial length
:  Writer()
,  media(NULL)
,  state(STATE_RESET)
{
   #ifdef HCDM
     debugf("MediaWriter(%p)::MediaWriter(%lu)\n", this, size);
   #endif

   resize(size);
}

//----------------------------------------------------------------------------
//
// Method-
//       MediaWriter::attach
//
// Purpose-
//       Attach Media
//
//----------------------------------------------------------------------------
void
   MediaWriter::attach(             // Attach Media
     Media&            media)       // The new Media
{
   #ifdef HCDM
     debugf("MediaWriter(%p)::attach(%p)\n", this, &media);
   #endif

   if( state != STATE_RESET )
   {
     debugf("MediaWriter(%p)::attach(%p) state(%d)\n", this, &media, state);
     throw "InvalidStateException";
   }

   MediaWriter::media= &media;
   if( MediaWriter::media == NULL )
   {
     debugf("MediaWriter(%p)::attach(%p) state(%d)\n", this, &media, state);
     throw "NullPointerException";
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       MediaWriter::detach
//
// Purpose-
//       Detach Media
//
//----------------------------------------------------------------------------
void
   MediaWriter::detach( void )      // Detach Media
{
   #ifdef HCDM
     debugf("MediaWriter(%p)::detach()\n", this);
   #endif

   if( state != STATE_RESET )
   {
     debugf("MediaWriter(%p)::detach() state(%d)\n", this, state);
     throw "InvalidStateException";
   }

   media= NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       MediaWriter::getState
//
// Purpose-
//       Get the current State
//
//----------------------------------------------------------------------------
MediaWriter::State                  // The current State
   MediaWriter::getState( void ) const // Get current State
{
   #ifdef HCDM
     debugf("MediaWriter(%p)::getState()\n", this);
   #endif

   State               result= state; // Resultant

   if( state != STATE_RESET )
   {
     if( media->getState() == STATE_EOM )
       result= STATE_EOM;
     if( media->getState() == STATE_ERROR )
       result= STATE_ERROR;
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       MediaWriter::open
//
// Purpose-
//       Open the MediaWriter
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   MediaWriter::open(               // Open the MediaWriter
     const char*       name,        // The MediaWriter name
     const char*       mode)        // The open mode
{
   #ifdef HCDM
     debugf("MediaWriter(%p)::open(%s,%s)\n", this, name, mode );
   #endif

   int                 result= 0;   // Resultant

   if( name == NULL )
     name= ">";

   if( mode == NULL )
     mode= MODE_WRITE;

   if( strcmp(mode, MODE_WRITE) != 0
       && strcmp(mode, MODE_INOUT) != 0
       && strcmp(mode, MODE_OUTIN) != 0 )
   {
     debugf("MediaWriter(%p)::open(%s,%s)\n",
            this, name, mode);
     throw "InvalidArgumentException";
   }

   if( state != STATE_RESET )
   {
     debugf("MediaWriter(%p)::open(%s,%s) state(%d)\n",
            this, name, mode, state);
     throw "InvalidStateException";
   }

   if( media == NULL )
   {
     debugf("MediaWriter(%p)::open(%s,%s) media(%p)\n",
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
       state= STATE_OUTPUT;
   }
   else
   {
     if( media->getState() == STATE_OUTPUT
         || media->getState() == STATE_INOUT
         || media->getState() == STATE_OUTIN
         || media->getState() == STATE_EOM )
       state= STATE_OUTPUT;
     else
     {
       debugf("MediaWriter(%p)::open(%s,%s) %d= media->getState()\n",
              this, name, mode, media->getState());
       throw "InvalidStateException";
     }
   }

   size= 0;

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       MediaWriter::close
//
// Purpose-
//       Close the MediaWriter
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   MediaWriter::close( void )       // Close the MediaWriter
{
   #ifdef HCDM
     debugf("MediaWriter(%p)::close()\n", this);
   #endif

   int                 result= 0;   // Resultant

   if( state != STATE_RESET )
   {
     flush();

     state= STATE_RESET;
     if( media->getState() != STATE_RESET )
       result= media->close();
   }

   size= 0;
   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       MediaWriter::flush
//
// Purpose-
//       Flush the MediaWriter
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   MediaWriter::flush( void )       // Flush the MediaWriter
{
   #ifdef HCDM
     debugf("MediaWriter(%p)::flush()\n", this);
   #endif

   int                 result= RC_USER; // Resultant

   if( state == STATE_OUTPUT )
   {
     while( size > 0 )
     {
       result= output();
       if( result != 0 )
       {
         debugf("Writer(%p)::flush() %d= output()\n", this, result);
         throw "OutputException";
       }
     }

     size= 0;
     result= media->flush();
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       MediaWriter::output
//
// Purpose-
//       Write onto the Media
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   MediaWriter::output( void )      // Write output
{
   #ifdef HCDM
     debugf("MediaWriter(%p)::output()\n", this);
   #endif

   int                 result= 0;   // Return code
   Size_t              L;           // Write length

   if( state != STATE_OUTPUT )
   {
     debugf("MediaWriter(%p)::output() state(%d)\n", this, state);
     throw "InvalidStateException";
   }

   if( size > 0 )
   {
     L= media->write(buffer, size);
     if( L < size && L > 0 )
       memcpy(buffer, buffer+L, size-L);

     if( L == 0 )
     {
       switch( media->getState() )
       {
         case Media::STATE_OUTPUT:
           result= RC_NULL;
           break;

         case Media::STATE_EOM:
           result= RC_EOM;
           break;

         default:
           result= RC_MEDIA_FAULT;
           break;
       }
     }

     size -= L;
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       FileWriter::~FileWriter
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   FileWriter::~FileWriter( void )  // Destructor
{
   #ifdef HCDM
     debugf("FileWriter(%p)::~FileWriter()\n", this);
   #endif

   if( getState() != STATE_RESET )
     close();
}

//----------------------------------------------------------------------------
//
// Method-
//       FileWriter::FileWriter
//
// Purpose-
//       Default constructor.
//
//----------------------------------------------------------------------------
   FileWriter::FileWriter( void )   // Default constructor
:  Writer()
,  media()
,  state(STATE_RESET)
{
   #ifdef HCDM
     debugf("FileWriter(%p)::FileWriter()\n", this);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       FileWriter::FileWriter
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   FileWriter::FileWriter(          // Constructor
     const char*       name)        // The Media name
:  Writer()
,  media()
,  state(STATE_RESET)
{
   #ifdef HCDM
     debugf("FileWriter(%p)::FileWriter(%s)\n", this, name);
   #endif

   open(name);
}

//----------------------------------------------------------------------------
//
// Method-
//       FileWriter::getState
//
// Purpose-
//       Get the current State
//
//----------------------------------------------------------------------------
FileWriter::State                   // The current State
   FileWriter::getState( void ) const // Get current State
{
   #ifdef HCDM
     debugf("FileWriter(%p)::getState()\n", this);
   #endif

   State               result= state; // Resultant

   if( state != STATE_RESET )
   {
     if( media.getState() == STATE_EOM )
       result= STATE_EOM;
     if( media.getState() == STATE_ERROR )
       result= STATE_ERROR;
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       FileWriter::open
//
// Purpose-
//       Open the FileWriter
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   FileWriter::open(                // Open the FileWriter
     const char*       name,        // The FileWriter name
     const char*       mode)        // The open mode
{
   #ifdef HCDM
     debugf("FileWriter(%p)::open(%s,%s)\n", this, name, mode);
   #endif

   int                 result= 0;   // Resultant

   if( name == NULL )
     name= ">";

   if( mode == NULL )
     mode= MODE_WRITE;

   if( strcmp(mode, MODE_WRITE) != 0
       && strcmp(mode, MODE_INOUT) != 0
       && strcmp(mode, MODE_OUTIN) != 0 )
   {
     debugf("FileWriter(%p)::open(%s,%s)\n",
            this, name, mode);
     throw "InvalidArgumentException";
   }

   if( state != STATE_RESET )
   {
     debugf("FileWriter(%p)::open(%s,%s) state(%d)\n",
            this, name, mode, state);
     throw "InvalidStateException";
   }

   if( buffer == NULL )
   {
     if( length == 0 )
       length= DEFAULT_SIZE;

     resize(length);
   }

   if( name == NULL )
     name= ">";

   if( media.getState() == STATE_RESET )
   {
     result= media.open(name, mode);
     if( result == 0 )
       state= STATE_OUTPUT;
   }
   else
   {
     if( media.getState() == STATE_OUTPUT
         || media.getState() == STATE_INOUT
         || media.getState() == STATE_OUTIN
         || media.getState() == STATE_EOM )
       state= STATE_OUTPUT;
     else
     {
       debugf("FileWriter(%p)::open(%s,%s) %d= media.getState()\n",
              this, name, mode, media.getState());
       throw "InvalidStateException";
     }
   }

   size= 0;

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       FileWriter::close
//
// Purpose-
//       Close the FileWriter
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   FileWriter::close( void )        // Close the FileWriter
{
   #ifdef HCDM
     debugf("FileWriter(%p)::close()\n", this);
   #endif

   int                 result= 0;   // Resultant

   if( state != STATE_RESET )
   {
     flush();

     state= STATE_RESET;
     if( media.getState() != STATE_RESET )
       result= media.close();
   }

   size= 0;
   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       FileWriter::flush
//
// Purpose-
//       Flush the FileWriter
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   FileWriter::flush( void )        // Flush the FileWriter
{
   #ifdef HCDM
     debugf("FileWriter(%p)::flush()\n", this);
   #endif

   int                 result= RC_USER; // Resultant

   if( state == STATE_OUTPUT )
   {
     while( size > 0 )
     {
       result= output();
       if( result != 0 )
       {
         debugf("Writer(%p)::flush() %d= output()\n", this, result);
         throw "OutputException";
       }
     }

     size= 0;
     result= media.flush();
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       FileWriter::output
//
// Purpose-
//       Write onto the Media
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   FileWriter::output( void )       // Write output
{
   #ifdef HCDM
     debugf("FileWriter(%p)::output()\n", this);
   #endif

   int                 result= 0;   // Return code
   Size_t              L;           // Write length

   if( state != STATE_OUTPUT )
   {
     debugf("FileWriter(%p)::output() state(%d)\n", this, state);
     throw "InvalidStateException";
   }

   if( size > 0 )
   {
     L= media.write(buffer, size);
     if( L < size && L > 0 )
       memcpy(buffer, buffer+L, size-L);

     if( L == 0 )
     {
       switch( media.getState() )
       {
         case Media::STATE_OUTPUT:
           result= RC_NULL;
           break;

         case Media::STATE_EOM:
           result= RC_EOM;
           break;

         default:
           result= RC_MEDIA_FAULT;
           break;
       }
     }

     size -= L;
   }

   return result;
}

