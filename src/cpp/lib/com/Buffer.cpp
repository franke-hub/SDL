//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2020 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Buffer.cpp
//
// Purpose-
//       Buffer object methods.
//
// Last change date-
//       2020/10/02
//
//----------------------------------------------------------------------------
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <com/define.h>
#include <com/Debug.h>
#include "com/Buffer.h"

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
//       Buffer::~Buffer
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Buffer::~Buffer( void )          // Destructor
{
   #ifdef HCDM
     debugf("Buffer(%p)::~Buffer()\n", this);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       Buffer::Buffer
//
// Purpose-
//       Default constructor.
//
//----------------------------------------------------------------------------
   Buffer::Buffer( void )           // Default constructor
:  Reader(), Writer()
{
   #ifdef HCDM
     debugf("Buffer(%p)::Buffer()\n", this);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       Buffer::Buffer
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Buffer::Buffer(                  // Constructor
     Size_t            size)        // Initial length
:  Reader(size), Writer(size)
{
   #ifdef HCDM
     debugf("Buffer(%p)::Buffer(%lu)\n", this, size);
   #endif

   reset();
}

//----------------------------------------------------------------------------
//
// Method-
//       Buffer::getState
//
// Purpose-
//       Return STATE_RESET when no getState() method available
//
//----------------------------------------------------------------------------
Buffer::State                       // The current State
   Buffer::getState( void ) const   // Get current State
{
   return STATE_RESET;
}

//----------------------------------------------------------------------------
//
// Method-
//       Buffer::getAvail
//
// Purpose-
//       Get the number of available buffer bytes
//
//----------------------------------------------------------------------------
Buffer::Size_t                      // The number of bytes available
   Buffer::getAvail( void )         // Get number of bytes available
{
   #ifdef HCDM
     debugf("Buffer(%p)::getAvail()\n", this);
   #endif

   Size_t              result= 0;   // Resultant

   if( getState() == STATE_INPUT )
     result= Reader::getAvail();
   else if( getState() == STATE_OUTPUT )
     result= Writer::getAvail();

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Buffer::getLength
//
// Purpose-
//       Get the number of buffer bytes
//
//----------------------------------------------------------------------------
Buffer::Size_t                      // The number of bytes
   Buffer::getLength( void )        // Get number of bytes
{
   #ifdef HCDM
     debugf("Buffer(%p)::getLength()\n", this);
   #endif

   Size_t              result= 0;   // Resultant

   if( getState() == STATE_INPUT )
     result= Reader::getLength();
   else if( getState() == STATE_OUTPUT )
     result= Writer::getLength();

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Buffer::reset
//
// Purpose-
//       Reset the buffer
//
//----------------------------------------------------------------------------
void
   Buffer::reset( void )            // Reset the buffer
{
   #ifdef HCDM
     debugf("Buffer(%p)::reset()\n", this);
   #endif

   Reader::reset();
   Writer::reset();
}

//----------------------------------------------------------------------------
//
// Method-
//       Buffer::resize
//
// Purpose-
//       Resize the buffer
//
//----------------------------------------------------------------------------
void
   Buffer::resize(                  // Resize the buffer
     Size_t            size)        // The new length
{
   #ifdef HCDM
     debugf("Buffer(%p)::resize(%lu)\n", this, size);
   #endif

   Reader::resize(size);
   Writer::resize(size);
   reset();
}

//----------------------------------------------------------------------------
//
// Method-
//       MediaBuffer::~MediaBuffer
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   MediaBuffer::~MediaBuffer( void ) // Destructor
{
   #ifdef HCDM
     debugf("MediaBuffer(%p)::~MediaBuffer()\n", this);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       MediaBuffer::MediaBuffer
//
// Purpose-
//       Default constructor.
//
//----------------------------------------------------------------------------
   MediaBuffer::MediaBuffer( void ) // Default constructor
:  Buffer()
,  media()
,  state(STATE_RESET)
{
   #ifdef HCDM
     debugf("MediaBuffer(%p)::MediaBuffer()\n", this);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       MediaBuffer::MediaBuffer
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   MediaBuffer::MediaBuffer(        // Constructor
     Size_t            size)        // Initial length
:  Buffer(size)
,  media()
,  state(STATE_RESET)
{
   #ifdef HCDM
     debugf("MediaBuffer(%p)::MediaBuffer(%lu)\n", this, size);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       MediaBuffer::attach
//
// Purpose-
//       Attach Media
//
//----------------------------------------------------------------------------
void
   MediaBuffer::attach(             // Attach Media
     Media&            media)       // The new Media
{
   #ifdef HCDM
     debugf("MediaBuffer(%p)::attach(%p)\n", this, &media);
   #endif

   if( state != STATE_RESET )
   {
     debugf("MediaBuffer(%p)::attach(%p) state(%d)\n", this, &media, state);
     throw "InvalidStateException";
   }

   MediaBuffer::media= &media;
   if( MediaBuffer::media == NULL )
   {
     debugf("MediaBuffer(%p)::attach(%p) state(%d)\n", this, &media, state);
     throw "NullPointerException";
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       MediaBuffer::detach
//
// Purpose-
//       Detach Media
//
//----------------------------------------------------------------------------
void
   MediaBuffer::detach( void )      // Detach Media
{
   #ifdef HCDM
     debugf("MediaBuffer(%p)::detach()\n", this);
   #endif

   if( state != STATE_RESET )
   {
     debugf("MediaBuffer(%p)::detach() state(%d)\n", this, state);
     throw "InvalidStateException";
   }

   media= NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       MediaBuffer::getState
//
// Purpose-
//       Get the current State
//
//----------------------------------------------------------------------------
MediaBuffer::State                  // The current State
   MediaBuffer::getState( void ) const // Get current State
{
   #ifdef HCDM
     debugf("MediaBuffer(%p)::getAvail()\n", this);
   #endif

   State               result= state; // Resultant

   if( state != STATE_RESET )
     result= media->getState();

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       MediaBuffer::open
//
// Purpose-
//       Open the MediaBuffer
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   MediaBuffer::open(               // Open the MediaBuffer
     const char*       name,        // The Media name
     const char*       mode)        // The Media name
{
   #ifdef HCDM
     debugf("MediaBuffer(%p)::open(%s,%s)\n", this, name, mode);
   #endif

   int                 result= 0;   // Resultant
   Size_t              size;        // Working size

   if( name == NULL || mode == NULL )
   {
     debugf("MediaBuffer(%p)::open(%s,%s)\n",
            this, name, mode);
     throw "NullPointerException";
   }

   if( state != STATE_RESET )
   {
     debugf("MediaBuffer(%p)::open(%s,%s) state(%d)\n",
            this, name, mode, state);
     throw "InvalidStateException";
   }

   if( media == NULL )
   {
     debugf("MediaBuffer(%p)::open(%s,%s) media(%p)\n",
            this, name, mode, media);
     throw "NullPointerException";
   }

   if( strcmp(mode, MODE_READ) == 0 )
   {
     if( media->getState() == STATE_RESET )
     {
       size= Reader::length;
       if( size == 0 )
         size= DEFAULT_SIZE;
       Reader::resize(size);

       result= media->open(name, MODE_READ);
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
         debugf("MediaBuffer(%p)::open(%s,%s) %d= media->getState()\n",
                this, name, mode, media->getState());
         throw "InvalidStateException";
       }
     }

     Reader::used= Reader::size= 0;
   }
   else if( strcmp(mode, MODE_WRITE) == 0 )
   {
     if( media->getState() == STATE_RESET )
     {
       size= Writer::length;
       if( size == 0 )
         size= DEFAULT_SIZE;
       Writer::resize(size);

       result= media->open(name, MODE_WRITE);
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
         debugf("MediaBuffer(%p)::open(%s,%s) %d= media->getState()\n",
                this, name, mode, media->getState());
         throw "InvalidStateException";
       }
     }

     Writer::size= 0;
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       MediaBuffer::close
//
// Purpose-
//       Close the MediaBuffer
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   MediaBuffer::close( void )       // Close the MediaBuffer
{
   #ifdef HCDM
     debugf("MediaBuffer(%p)::close()\n", this);
   #endif

   int                 result= RC_USER; // Resultant

   if( state != STATE_RESET )
   {
     if( media->getState() != STATE_RESET )
     {
       flush();
       result= media->close();
     }
     state= STATE_RESET;

     Reader::reset();
     Writer::reset();
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       MediaBuffer::flush
//
// Purpose-
//       Flush the MediaBuffer
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   MediaBuffer::flush( void )       // Flush the MediaBuffer
{
   #ifdef HCDM
     debugf("MediaBuffer(%p)::flush()\n", this);
   #endif

   int                 result= RC_USER; // Resultant

   if( state == STATE_INPUT )
   {
     Reader::used= Reader::size= 0;
     result= media->flush();
   }
   else if( state == STATE_OUTPUT )
   {
     while( Writer::size > 0 )
     {
       result= output();
       if( result != 0 )
       {
         debugf("MediaBuffer(%p)::flush() %d= output()\n", this, result);
         throw "OutputException";
       }
     }

     Writer::size= 0;
     result= media->flush();
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       MediaBuffer::input
//
// Purpose-
//       Read from the Media
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   MediaBuffer::input( void )       // Read input
{
   #ifdef HCDM
     debugf("MediaBuffer(%p)::input()\n", this);
   #endif

   int                 result= 0;   // Return code
   Byte*               buffer= Reader::buffer; // Using Reader::buffer
   Size_t              size= Reader::size; // Using Reader::size
   Size_t              used= Reader::used; // Using Reader::used
   Size_t              L;           // Read length

   if( state != STATE_INPUT )
   {
     debugf("MediaBuffer(%p)::input() state(%d)\n", this, state);
     throw "InvalidStateException";
   }

   // Preserve existing data
   if( used < size && used > 0 )
     memcpy(buffer, buffer+used, size-used);

   size -= used;
   Reader::used= 0;

   // Fill with new data
   if( size < Reader::length )
   {
     L= media->read(buffer+size, (Reader::length - size));
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
   Reader::size= size;

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       MediaBuffer::output
//
// Purpose-
//       Write onto the Media
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   MediaBuffer::output( void )      // Write output
{
   #ifdef HCDM
     debugf("MediaBuffer(%p)::output()\n", this);
   #endif

   int                 result= 0;   // Return code
   Byte*               buffer= Writer::buffer; // Using Writer::buffer
   Size_t              size= Writer::size; // Using Writer::size
   Size_t              L;           // Write length

   if( state != STATE_OUTPUT )
   {
     debugf("MediaBuffer(%p)::output() state(%d)\n", this, state);
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
   Writer::size= size;

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       FileBuffer::~FileBuffer
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   FileBuffer::~FileBuffer( void )  // Destructor
{
   #ifdef HCDM
     debugf("FileBuffer(%p)::~FileBuffer()\n", this);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       FileBuffer::FileBuffer
//
// Purpose-
//       Default constructor.
//
//----------------------------------------------------------------------------
   FileBuffer::FileBuffer( void )   // Default constructor
:  Buffer()
,  media()
,  state(STATE_RESET)
{
   #ifdef HCDM
     debugf("FileBuffer(%p)::FileBuffer()\n", this);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       FileBuffer::getState
//
// Purpose-
//       Get the current State
//
//----------------------------------------------------------------------------
FileBuffer::State                   // The current State
   FileBuffer::getState( void ) const // Get current State
{
   #ifdef HCDM
     debugf("FileBuffer(%p)::getAvail()\n", this);
   #endif

   State               result= state; // Resultant

   if( state != STATE_RESET )
     result= media.getState();

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       FileBuffer::open
//
// Purpose-
//       Open the FileBuffer
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   FileBuffer::open(                // Open the FileBuffer
     const char*       name,        // The Media name
     const char*       mode)        // The Media name
{
   #ifdef HCDM
     debugf("FileBuffer(%p)::open(%s,%s)\n", this, name, mode);
   #endif

   int                 result= 0;   // Resultant
   Size_t              size;        // Working size

   if( name == NULL || mode == NULL )
   {
     debugf("FileBuffer(%p)::open(%s,%s)\n",
            this, name, mode);
     throw "NullPointerException";
   }

   if( state != STATE_RESET )
   {
     debugf("FileBuffer(%p)::open(%s,%s) state(%d)\n",
            this, name, mode, state);
     throw "InvalidStateException";
   }

   if( strcmp(mode, MODE_READ) == 0 )
   {
     if( media.getState() == STATE_RESET )
     {
       size= Reader::length;
       if( size == 0 )
         size= DEFAULT_SIZE;
       Reader::resize(size);

       result= media.open(name, MODE_READ);
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
         debugf("FileBuffer(%p)::open(%s,%s) %d= media.getState()\n",
                this, name, mode, media.getState());
         throw "InvalidStateException";
       }
     }

     Reader::used= Reader::size= 0;
   }
   else if( strcmp(mode, MODE_WRITE) == 0 )
   {
     if( media.getState() == STATE_RESET )
     {
       size= Writer::length;
       if( size == 0 )
         size= DEFAULT_SIZE;
       Writer::resize(size);

       result= media.open(name, MODE_WRITE);
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
         debugf("FileBuffer(%p)::open(%s,%s) %d= media.getState()\n",
                this, name, mode, media.getState());
         throw "InvalidStateException";
       }
     }

     Writer::size= 0;
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       FileBuffer::close
//
// Purpose-
//       Close the FileBuffer
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   FileBuffer::close( void )        // Close the FileBuffer
{
   #ifdef HCDM
     debugf("FileBuffer(%p)::close()\n", this);
   #endif

   int                 result= RC_USER; // Resultant

   if( state != STATE_RESET )
   {
     if( media.getState() != STATE_RESET )
     {
       flush();
       result= media.close();
     }
     state= STATE_RESET;

     Reader::reset();
     Writer::reset();
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       FileBuffer::flush
//
// Purpose-
//       Flush the FileBuffer
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   FileBuffer::flush( void )        // Flush the FileBuffer
{
   #ifdef HCDM
     debugf("FileBuffer(%p)::flush()\n", this);
   #endif

   int                 result= RC_USER; // Resultant

   if( state == STATE_INPUT )
   {
     Reader::used= Reader::size= 0;
     result= media.flush();
   }
   else if( state == STATE_OUTPUT )
   {
     while( Writer::size > 0 )
     {
       result= output();
       if( result != 0 )
       {
         debugf("MediaBuffer(%p)::flush() %d= output()\n", this, result);
         throw "OutputException";
       }
     }

     Writer::size= 0;
     result= media.flush();
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       FileBuffer::input
//
// Purpose-
//       Read from the Media
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   FileBuffer::input( void )        // Read input
{
   #ifdef HCDM
     debugf("FileBuffer(%p)::input()\n", this);
   #endif

   int                 result= 0;   // Return code
   Byte*               buffer= Reader::buffer; // Using Reader::buffer
   Size_t              size= Reader::size; // Using Reader::size
   Size_t              used= Reader::used; // Using Reader::used
   Size_t              L;           // Read length

   if( state != STATE_INPUT )
   {
     debugf("FileBuffer(%p)::input() state(%d)\n", this, state);
     throw "InvalidStateException";
   }

   // Preserve existing data
   if( used < size && used > 0 )
     memcpy(buffer, buffer+used, size-used);

   size -= used;
   Reader::used= 0;

   // Fill with new data
   if( size < Reader::length )
   {
     L= media.read(buffer+size, (Reader::length - size));
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
   Reader::size= size;

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       FileBuffer::output
//
// Purpose-
//       Write onto the Media
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   FileBuffer::output( void )       // Write output
{
   #ifdef HCDM
     debugf("FileBuffer(%p)::output()\n", this);
   #endif

   int                 result= 0;   // Return code
   Byte*               buffer= Writer::buffer; // Using Reader::buffer
   Size_t              size= Writer::size; // Using Reader::size
   Size_t              L;           // Write length

   if( state != STATE_OUTPUT )
   {
     debugf("FileBuffer(%p)::output() state(%d)\n", this, state);
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
   Writer::size= size;

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       TempBuffer::~TempBuffer
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   TempBuffer::~TempBuffer( void )  // Destructor
{
   #ifdef HCDM
     debugf("TempBuffer(%p)::~TempBuffer()\n", this);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       TempBuffer::TempBuffer
//
// Purpose-
//       Default constructor.
//
//----------------------------------------------------------------------------
   TempBuffer::TempBuffer( void )   // Default constructor
:  Buffer()
,  media()
,  state(STATE_RESET)
{
   #ifdef HCDM
     debugf("TempBuffer(%p)::TempBuffer()\n", this);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       TempBuffer::getState
//
// Purpose-
//       Get the current State
//
//----------------------------------------------------------------------------
TempBuffer::State                   // The current State
   TempBuffer::getState( void ) const // Get current State
{
   #ifdef HCDM
     debugf("TempBuffer(%p)::getAvail()\n", this);
   #endif

   State               result= state; // Resultant

   if( state != STATE_RESET )
     result= media.getState();

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       TempBuffer::open
//
// Purpose-
//       Open the TempBuffer
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   TempBuffer::open(                // Open the TempBuffer
     const char*       name,        // The Media name
     const char*       mode)        // The Media name
{
   #ifdef HCDM
     debugf("TempBuffer(%p)::open(%s,%s)\n", this, name, mode);
   #endif

   int                 result= 0;   // Resultant
   Size_t              size;        // Working size

   if( name == NULL || mode == NULL )
   {
     debugf("TempBuffer(%p)::open(%s,%s)\n",
            this, name, mode);
     throw "NullPointerException";
   }

   if( state != STATE_RESET )
   {
     debugf("TempBuffer(%p)::open(%s,%s) state(%d)\n",
            this, name, mode, state);
     throw "InvalidStateException";
   }

   if( strcmp(mode, MODE_READ) == 0 )
   {
     if( media.getState() == STATE_RESET )
     {
       size= Reader::length;
       if( size == 0 )
         size= DEFAULT_SIZE;
       Reader::resize(size);

       result= media.open(name, MODE_READ);
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
         debugf("TempBuffer(%p)::open(%s,%s) %d= media.getState()\n",
                this, name, mode, media.getState());
         throw "InvalidStateException";
       }
     }

     Reader::used= Reader::size= 0;
   }
   else if( strcmp(mode, MODE_WRITE) == 0 )
   {
     if( media.getState() == STATE_RESET )
     {
       size= Writer::length;
       if( size == 0 )
         size= DEFAULT_SIZE;
       Writer::resize(size);

       result= media.open(name, MODE_WRITE);
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
         debugf("TempBuffer(%p)::open(%s,%s) %d= media.getState()\n",
                this, name, mode, media.getState());
         throw "InvalidStateException";
       }
     }

     Writer::size= 0;
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       TempBuffer::close
//
// Purpose-
//       Close the TempBuffer
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   TempBuffer::close( void )        // Close the TempBuffer
{
   #ifdef HCDM
     debugf("TempBuffer(%p)::close()\n", this);
   #endif

   int                 result= RC_USER; // Resultant

   if( state != STATE_RESET )
   {
     if( media.getState() != STATE_RESET )
     {
       flush();
       result= media.close();
     }
     state= STATE_RESET;

     Reader::reset();
     Writer::reset();
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       TempBuffer::flush
//
// Purpose-
//       Flush the TempBuffer
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   TempBuffer::flush( void )        // Flush the TempBuffer
{
   #ifdef HCDM
     debugf("TempBuffer(%p)::flush()\n", this);
   #endif

   int                 result= RC_USER; // Resultant

   if( state == STATE_INPUT )
   {
     Reader::used= Reader::size= 0;
     result= media.flush();
   }
   else if( state == STATE_OUTPUT )
   {
     while( Writer::size > 0 )
     {
       result= output();
       if( result != 0 )
       {
         debugf("MediaBuffer(%p)::flush() %d= output()\n", this, result);
         throw "OutputException";
       }
     }

     Writer::size= 0;
     result= media.flush();
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       TempBuffer::truncate
//
// Purpose-
//       Truncate the TempBuffer
//
//----------------------------------------------------------------------------
void
   TempBuffer::truncate( void )     // Truncate the TempBuffer
{
   #ifdef HCDM
     debugf("TempBuffer(%p)::truncate()\n", this);
   #endif

   media.truncate();
}

//----------------------------------------------------------------------------
//
// Method-
//       TempBuffer::input
//
// Purpose-
//       Read from the Media
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   TempBuffer::input( void )        // Read input
{
   #ifdef HCDM
     debugf("TempBuffer(%p)::input()\n", this);
   #endif

   int                 result= 0;   // Return code
   Byte*               buffer= Reader::buffer; // Using Reader::buffer
   Size_t              size= Reader::size; // Using Reader::size
   Size_t              used= Reader::used; // Using Reader::used
   Size_t              L;           // Read length

   if( state != STATE_INPUT )
   {
     debugf("TempBuffer(%p)::input() state(%d)\n", this, state);
     throw "InvalidStateException";
   }

   // Preserve existing data
   if( used < size && used > 0 )
     memcpy(buffer, buffer+used, size-used);

   size -= used;
   Reader::used= 0;

   // Fill with new data
   if( size < Reader::length )
   {
     L= media.read(buffer+size, (Reader::length - size));
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
   Reader::size= size;

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       TempBuffer::output
//
// Purpose-
//       Write onto the Media
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   TempBuffer::output( void )       // Write output
{
   #ifdef HCDM
     debugf("TempBuffer(%p)::output()\n", this);
   #endif

   int                 result= 0;   // Return code
   Byte*               buffer= Writer::buffer; // Using Writer::buffer
   Size_t              size= Writer::size; // Using Writer::size
   Size_t              L;           // Write length

   if( state != STATE_OUTPUT )
   {
     debugf("TempBuffer(%p)::output() state(%d)\n", this, state);
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
   Writer::size= size;

   return result;
}

