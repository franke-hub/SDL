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
//       Media.cpp
//
// Purpose-
//       Media object methods.
//
// Last change date-
//       2020/10/03
//
//----------------------------------------------------------------------------
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <com/define.h>
#include <com/Debug.h>
#include <com/istring.h>
#include <com/Socket.h>
#include <com/Software.h>

#include "com/Media.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#define STORAGE_SIZE 32768          // Storage data size

//----------------------------------------------------------------------------
// External data areas
//----------------------------------------------------------------------------
const char*              MediaType::MODE_READ=   "r";
const char*              MediaType::MODE_WRITE=  "w";
const char*              MediaType::MODE_CREATE= "c";
const char*              MediaType::MODE_APPEND= "a";
const char*              MediaType::MODE_INOUT=  "r+";
const char*              MediaType::MODE_OUTIN=  "w+";
const char*              MediaType::MODE_CREATE_IN= "c+";
const char*              MediaType::MODE_APPEND_IN= "a+";

//----------------------------------------------------------------------------
//
// Struct-
//       TempStorage
//
// Purpose-
//       Describe the TempMedia storage list.
//
//----------------------------------------------------------------------------
struct TempStorage {                // TempMedia data storage
   TempStorage*        next;        // The next TempStorage
   Media::Size_t       size;        // The number of data bytes written

   Media::Byte         data[STORAGE_SIZE]; // The data

//----------------------------------------------------------------------------
// Constructors
//----------------------------------------------------------------------------
   ~TempStorage( void ) {}
   TempStorage( void ) : next(NULL), size(0) {}
}; // struct TempStorage

//----------------------------------------------------------------------------
//
// Subroutine-
//       isInputState
//
// Purpose-
//       Determine whether the state allows input.
//
//----------------------------------------------------------------------------
inline int                          // TRUE iff input state
   isInputState(                    // Check state
     Media::State      state)       // For this State
{
   if( state == Media::STATE_INPUT
       || state == Media::STATE_INOUT
       || state == Media::STATE_OUTIN
       || state == Media::STATE_EOF )
     return TRUE;

   return FALSE;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       isOutputState
//
// Purpose-
//       Determine whether the state allows output.
//
//----------------------------------------------------------------------------
inline int                          // TRUE iff output state
   isOutputState(                   // Check state
     Media::State      state)       // For this State
{
   if( state == Media::STATE_OUTPUT
       || state == Media::STATE_INOUT
       || state == Media::STATE_OUTIN
       || state == Media::STATE_EOM )
     return TRUE;

   return FALSE;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       getSystemHandle
//
// Purpose-
//       Convert name to system handle.
//
//----------------------------------------------------------------------------
inline void*                        // The system handle, or NULL
   getSystemHandle(                 // Get handle
     const char*       name)        // For this name
{
   if( strcmp(name, "<") == 0
       || strcmp(name, "<0" ) == 0
       || stricmp(name, "<stdin" ) == 0 )
     return stdin;

   if( strcmp(name, ">") == 0
       || strcmp(name, ">1" ) == 0
       || stricmp(name, ">stdout" ) == 0 )
     return stdout;

   if( strcmp(name, ">2" ) == 0
       || stricmp(name, ">stderr" ) == 0 )
     return stderr;

   return NULL;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       isSystemHandle
//
// Purpose-
//       Determine whether the handle is a system handle.
//
//----------------------------------------------------------------------------
inline int                          // TRUE iff system handle
   isSystemHandle(                  // Check handle
     void*             handle)      // For this handle
{
   if( handle == stdin
       || handle == stdout
       || handle == stderr )
     return TRUE;

   return FALSE;
}

//----------------------------------------------------------------------------
//
// Method-
//       Media::Pure virtual functions
//
// Purpose-
//       Write a message and throw an exception.
//
//----------------------------------------------------------------------------
Media::State                        // The Media State
   Media::getState( void ) const    // Get Media State
{
   debugf("Media(%p)::getState() (pure virtual)\n", this);
   throw "InvalidObjectException";
}

int                                 // Return code (0 OK)
   Media::open(                     // Open the Media
     const char*       name,        // The Media name
     const char*       mode)        // The Media mode
{
   debugf("Media(%p)::open(%s,%s) (pure virtual)\n", this, name, mode);
   throw "InvalidObjectException";
}

int                                 // Return code (0 OK)
   Media::close( void )             // Close the Media
{
   debugf("Media(%p)::close() (pure virtual)\n", this);
   throw "InvalidObjectException";
}

int                                 // Return code (0 OK)
   Media::flush( void )             // Flush the Media
{
   debugf("Media(%p)::flush() (pure virtual)\n", this);
   throw "InvalidObjectException";
}

Media::Size_t                       // The number of bytes read
   Media::read(                     // Read from the Media
     Byte*             addr,        // Data address
     Size_t            size)        // Data length
{
   debugf("Media(%p)::read(%p,%ld) (pure virtual)\n", this, addr, size);
   throw "InvalidObjectException";
}

Media::Size_t                       // The number of bytes written
   Media::write(                    // Write onto the Media
     const Byte*       addr,        // Data address
     Size_t            size)        // Data length
{
   debugf("Media(%p)::write(%p,%ld) (pure virtual)\n", this, addr, size);
   throw "InvalidObjectException";
}

//----------------------------------------------------------------------------
//
// Method-
//       FileMedia::~FileMedia
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   FileMedia::~FileMedia( void )    // Destructor
{
   #ifdef HCDM
     debugf("FileMedia(%p)::~FileMedia()\n", this);
   #endif

   if( state != STATE_RESET )
     close();
}

//----------------------------------------------------------------------------
//
// Method-
//       FileMedia::FileMedia
//
// Purpose-
//       Default constructor.
//
//----------------------------------------------------------------------------
   FileMedia::FileMedia( void )     // Default constructor
:  state(STATE_RESET)
,  openState(STATE_RESET)
,  handle(NULL)
{
   #ifdef HCDM
     debugf("FileMedia(%p)::FileMedia()\n", this);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       FileMedia::getState
//
// Purpose-
//       Return the current State
//
//----------------------------------------------------------------------------
Media::State                        // The current State
   FileMedia::getState( void ) const// Get current State
{
   if( handle != NULL )
   {
     if( ferror((FILE*)handle) )
       return STATE_ERROR;

     if( isInputState(state) && feof((FILE*)handle) )
       return STATE_EOF;
   }

   return state;
}

//----------------------------------------------------------------------------
//
// Method-
//       FileMedia::open
//
// Purpose-
//       Open the FileMedia
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   FileMedia::open(                 // Open the FileMedia
     const char*       name,        // The name
     const char*       mode)        // The mode
{
   #ifdef HCDM
     debugf("FileMedia(%p)::open(%s,%s)\n", this, name, mode);
   #endif

   int                 result= RC_NORMAL; // Resultant

   if( state != STATE_RESET )
   {
     debugf("FileMedia(%p)::open(%s,%s) state(%d)\n", this, name, mode, state);
     throw "MediaStateException";
   }

   if( mode == NULL )
     mode= "(NULL)";

   if( name == NULL )
   {
     name= "(NULL)";
     if( strcmp(mode, MODE_READ) == 0 )
       name= "<";
     else if( strcmp(mode, MODE_WRITE) == 0 )
       name= ">";
   }

   handle= NULL;
   if( strcmp(mode, MODE_READ) == 0 )
   {
     if( name[0] == '<' )
       handle= getSystemHandle(name);

     if( handle == NULL )
       handle= fopen(name, "rb");

     if( handle == NULL )
       result= RC_SYSTEM;
     else
       state= STATE_INPUT;
   }

   else if( strcmp(mode, MODE_WRITE) == 0 )
   {
     if( name[0] == '>' )
       handle= getSystemHandle(name);

     if( handle == NULL )
       handle= fopen(name, "wb");

     if( handle == NULL )
       result= RC_SYSTEM;
     else
       state= STATE_OUTPUT;
   }

   else if( strcmp(mode, MODE_CREATE) == 0 )
   {
     handle= fopen(name, "rb");
     if( handle != NULL )
     {
       fclose((FILE*)handle);
       result= RC_CREATE;
     }
     else
     {
       handle= fopen(name, "wb");
       if( handle == NULL )
         result= RC_SYSTEM;
       else
         state= STATE_OUTPUT;
     }
   }

   else if( strcmp(mode, MODE_APPEND) == 0 )
   {
     handle= fopen(name, "ab");
     if( handle == NULL )
       result= RC_SYSTEM;
     else
       state= STATE_OUTPUT;
   }

   else if( strcmp(mode, MODE_INOUT) == 0 )
   {
     handle= fopen(name, "rb+");
     if( handle == NULL )
       result= RC_SYSTEM;
     else
       state= STATE_INOUT;
   }

   else if( strcmp(mode, MODE_OUTIN) == 0 )
   {
     handle= fopen(name,"wb+");
     if( handle == NULL )
       result= RC_SYSTEM;
     else
       state= STATE_OUTIN;
   }

   else if( strcmp(mode, MODE_CREATE_IN) == 0 )
   {
     handle= fopen(name, "rb");
     if( handle != NULL )
     {
       fclose((FILE*)handle);
       result= RC_CREATE;
     }
     else
     {
       handle= fopen(name, "wb+");
       if( handle == NULL )
         result= RC_SYSTEM;
       else
         state= STATE_OUTIN;
     }
   }

   else if( strcmp(mode, MODE_APPEND_IN) == 0 )
   {
     handle= fopen(name, "ab+");
     if( handle == NULL )
       result= RC_SYSTEM;
     else
       state= STATE_OUTIN;
   }

   else
   {
     debugf("FileMedia(%p)::open(%s,%s)\n", this, name, mode);
     throw "InvalidArgumentException";
   }

   if( result == RC_NORMAL )
     openState= state;

   #ifdef SCDM
     debugf("%p= ::fopen(%s,%s)\n", handle, name, mode);
   #endif

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       FileMedia::close
//
// Purpose-
//       Close the FileMedia
//
//----------------------------------------------------------------------------
int                                // Return code (0 OK)
   FileMedia::close( void )        // Close the FileMedia
{
   #ifdef HCDM
     debugf("FileMedia(%p)::close()\n", this);
   #endif

   int                 result;     // Resultant

   if( state == STATE_RESET )
   {
     debugf("FileMedia(%p)::close() state(%d)\n", this, state);
     throw "MediaStateException";
   }

   result= 0;
   if( !isSystemHandle(handle) )
     result= fclose((FILE*)handle);

   handle= NULL;
   openState= state= STATE_RESET;
   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       FileMedia::flush
//
// Purpose-
//       Flush the FileMedia
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   FileMedia::flush( void )         // Flush the FileMedia
{
   #ifdef HCDM
     debugf("FileMedia(%p)::flush()\n", this);
   #endif

   return fflush((FILE*)handle);
}

//----------------------------------------------------------------------------
//
// Method-
//       FileMedia::read
//
// Purpose-
//       Read from the FileMedia
//
//----------------------------------------------------------------------------
Media::Size_t                       // The number of Bytes read
   FileMedia::read(                 // Read from the FileMedia
     Byte*             addr,        // Into this address
     Size_t            size)        // For this length
{
   #ifdef HCDM
     debugf("FileMedia(%p)::read(%p,%lu)\n", this, addr, size);
   #endif

   return fread(addr, 1, size, (FILE*)handle);
}

//----------------------------------------------------------------------------
//
// Method-
//       FileMedia::write
//
// Purpose-
//       Write into the FileMedia
//
//----------------------------------------------------------------------------
Media::Size_t                       // The number of Bytes written
   FileMedia::write(                // Write into the FileMedia
     const Byte*       addr,        // From this address
     Size_t            size)        // For this length
{
   #ifdef HCDM
     debugf("FileMedia(%p)::write(%p,%lu)\n", this, addr, size);
   #endif

   return fwrite(addr, 1, size, (FILE*)handle);
}

//----------------------------------------------------------------------------
//
// Method-
//       SockMedia::~SockMedia
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   SockMedia::~SockMedia( void )    // Destructor
{
   #ifdef HCDM
     debugf("SockMedia(%p)::~SockMedia()\n", this);
   #endif

   if( state != STATE_RESET )
     close();
}

//----------------------------------------------------------------------------
//
// Method-
//       SockMedia::SockMedia
//
// Purpose-
//       Constructors.
//
//----------------------------------------------------------------------------
   SockMedia::SockMedia( void )     // Default constructor
:  state(STATE_RESET)
,  socket(NULL)
{
   #ifdef HCDM
     debugf("SockMedia(%p)::SockMedia()\n", this);
   #endif
}

   SockMedia::SockMedia(            // Socket constructor
     Socket*           sock)        // -> Socket
:  state(STATE_RESET)
,  socket(sock)
{
   #ifdef HCDM
     debugf("SockMedia(%p)::SockMedia()\n", this);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       SockMedia::getSocket
//
// Purpose-
//       Return the current Socket
//
//----------------------------------------------------------------------------
Socket*                             // The Socket*
   SockMedia::getSocket( void ) const // Get Socket*
{
   return socket;
}

//----------------------------------------------------------------------------
//
// Method-
//       SockMedia::setSocket
//
// Purpose-
//       Set the Socket
//
//----------------------------------------------------------------------------
void
   SockMedia::setSocket(            // Set the Socket
     Socket*           socket)      // -> Socket
{
   if( state != STATE_RESET )
   {
     debugf("SockMedia(%p)::setSocket(%p) state(%d)\n", this, socket, state);
     throw "MediaStateException";
   }

   this->socket= socket;
}

//----------------------------------------------------------------------------
//
// Method-
//       SockMedia::getState
//
// Purpose-
//       Return the current State
//
//----------------------------------------------------------------------------
Media::State                        // The current State
   SockMedia::getState( void ) const// Get current State
{
   return state;
}

//----------------------------------------------------------------------------
//
// Method-
//       SockMedia::open
//
// Purpose-
//       Open the SockMedia
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   SockMedia::open(                 // Open the SockMedia
     const char*       name,        // The name
     const char*       mode)        // The mode
{
   #ifdef HCDM
     debugf("SockMedia(%p)::open(%s,%s)\n", this, name, mode);
   #endif

   int                 result= RC_NORMAL; // Resultant

   if( state != STATE_RESET )
   {
     debugf("SockMedia(%p)::open(%s,%s) state(%d)\n", this, name, mode, state);
     throw "MediaStateException";
   }

   if( socket == NULL )
   {
     debugf("SockMedia(%p)::open(%s,%s) NullSocket\n", this, name, mode);
     throw "MediaSocketException";
   }

   if( mode == NULL )
     mode= MODE_OUTIN;

   if( name == NULL )
     name= "(NULL)";

   if( strcmp(mode, MODE_READ) == 0 )
     state= STATE_INPUT;

   else if( strcmp(mode, MODE_WRITE) == 0 )
     state= STATE_OUTPUT;

   else if( strcmp(mode, MODE_CREATE) == 0 )
     result= RC_CREATE;

   else if( strcmp(mode, MODE_APPEND) == 0 )
     state= STATE_OUTPUT;

   else if( strcmp(mode, MODE_INOUT) == 0 )
     state= STATE_INOUT;

   else if( strcmp(mode, MODE_OUTIN) == 0 )
     state= STATE_OUTIN;

   else if( strcmp(mode, MODE_CREATE_IN) == 0 )
     result= RC_CREATE;

   else if( strcmp(mode, MODE_APPEND_IN) == 0 )
     state= STATE_OUTIN;

   else
   {
     debugf("SockMedia(%p)::open(%s,%s)\n", this, name, mode);
     throw "InvalidArgumentException";
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       SockMedia::close
//
// Purpose-
//       Close the SockMedia
//
//----------------------------------------------------------------------------
int                                // Return code (0 OK)
   SockMedia::close( void )        // Close the SockMedia
{
   #ifdef HCDM
     debugf("SockMedia(%p)::close()\n", this);
   #endif

   int                 result;     // Resultant

   if( state == STATE_RESET )
   {
     debugf("SockMedia(%p)::close() state(%d)\n", this, state);
     throw "MediaStateException";
   }

   if( socket == NULL )
   {
     debugf("SockMedia(%p)::close() NullSocket\n", this);
     throw "MediaSocketException";
   }

   result= 0;
   socket->close();
   socket= NULL;
   state= STATE_RESET;
   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       SockMedia::flush
//
// Purpose-
//       Flush the SockMedia
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   SockMedia::flush( void )         // Flush the SockMedia
{
   #ifdef HCDM
     debugf("SockMedia(%p)::flush()\n", this);
   #endif

   return Software::EC_PERM;
}

//----------------------------------------------------------------------------
//
// Method-
//       SockMedia::read
//
// Purpose-
//       Read from the SockMedia
//
//----------------------------------------------------------------------------
Media::Size_t                       // The number of Bytes read
   SockMedia::read(                 // Read from the SockMedia
     Byte*             addr,        // Into this address
     Size_t            size)        // For this length
{
   #ifdef HCDM
     debugf("SockMedia(%p)::read(%p,%lu)\n", this, addr, size);
   #endif

   if( state == STATE_RESET
       || socket == NULL )
   {
     debugf("SockMedia(%p)::read() MediaSocketException\n", this);
     throw "MediaSocketException";
   }

   return socket->recv(addr, size);
}

//----------------------------------------------------------------------------
//
// Method-
//       SockMedia::write
//
// Purpose-
//       Write into the SockMedia
//
//----------------------------------------------------------------------------
Media::Size_t                       // The number of Bytes written
   SockMedia::write(                // Write into the SockMedia
     const Byte*       addr,        // From this address
     Size_t            size)        // For this length
{
   #ifdef HCDM
     debugf("SockMedia(%p)::write(%p,%lu)\n", this, addr, size);
   #endif

   if( state == STATE_RESET
       || socket == NULL )
   {
     debugf("SockMedia(%p)::write() MediaSocketException\n", this);
     throw "MediaSocketException";
   }

   return socket->send(addr, size);
}

//----------------------------------------------------------------------------
//
// Method-
//       TempMedia::~TempMedia
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   TempMedia::~TempMedia( void )    // Destructor
{
   #ifdef HCDM
     debugf("TempMedia(%p)::~TempMedia()\n", this);
   #endif

   truncate();                      // Delete all associated storage
}

//----------------------------------------------------------------------------
//
// Method-
//       TempMedia::TempMedia
//
// Purpose-
//       Default constructor.
//
//----------------------------------------------------------------------------
   TempMedia::TempMedia( void )     // Default constructor
:  state(STATE_RESET)
,  openState(STATE_RESET)
,  head(NULL)
,  tail(NULL)
,  size(0)
,  busy(NULL)
,  used(0)
{
   #ifdef HCDM
     debugf("TempMedia(%p)::TempMedia()\n", this);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       TempMedia::truncate
//
// Purpose-
//       Delete all associated storage
//
//----------------------------------------------------------------------------
void
   TempMedia::truncate( void )      // Delete storage
{
   TempStorage*        temp;        // Working TempStorage*

   while( head != NULL )
   {
     temp= (TempStorage*)head;
     head= temp->next;
     delete temp;
   }

   busy= tail= NULL;
   size= used= 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       TempMedia::getState
//
// Purpose-
//       Return the current State
//
//----------------------------------------------------------------------------
Media::State                        // The current State
   TempMedia::getState( void ) const// Get current State
{
   return state;
}

//----------------------------------------------------------------------------
//
// Method-
//       TempMedia::open
//
// Purpose-
//       Open the TempMedia
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   TempMedia::open(                 // Open the TempMedia
     const char*       name,        // The name
     const char*       mode)        // The mode
{
   #ifdef HCDM
     debugf("TempMedia(%p)::open(%s,%s)\n", this, name, mode);
   #endif

   int                 result= RC_NORMAL; // Resultant

   if( state != STATE_RESET )
   {
     debugf("TempMedia(%p)::open(%s,%s) state(%d)\n", this, name, mode, state);
     throw "MediaStateException";
   }

   if( mode == NULL )
     mode= "(NULL)";

   if( strcmp(mode, MODE_READ) == 0 )
     state= STATE_INPUT;

   else if( strcmp(mode, MODE_WRITE) == 0 )
   {
     truncate();
     state= STATE_OUTPUT;
   }

   else if( strcmp(mode, MODE_CREATE) == 0 )
   {
     if( head != NULL )
       result= RC_CREATE;
     else
       state= STATE_OUTPUT;
   }

   else if( strcmp(mode, MODE_APPEND) == 0 )
     state= STATE_OUTPUT;

   else if( strcmp(mode, MODE_INOUT) == 0 )
     state= STATE_INOUT;

   else if( strcmp(mode, MODE_OUTIN) == 0 )
   {
     truncate();
     state= STATE_OUTIN;
   }

   else if( strcmp(mode, MODE_CREATE_IN) == 0 )
   {
     if( head != NULL )
       result= RC_CREATE;
     else
       state= STATE_OUTIN;
   }

   else if( strcmp(mode, MODE_APPEND_IN) == 0 )
     state= STATE_OUTIN;

   else
   {
     debugf("TempMedia(%p)::open(%s,%s)\n", this, name, mode);
     throw "InvalidArgumentException";
   }

   if( result == RC_NORMAL )
   {
     openState= state;
     if( tail != NULL )
       size= ((TempStorage*)tail)->size;

     busy= head;
     used= 0;
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       TempMedia::close
//
// Purpose-
//       Close the TempMedia
//
//----------------------------------------------------------------------------
int                                // Return code (0 OK)
   TempMedia::close( void )        // Close the TempMedia
{
   #ifdef HCDM
     debugf("TempMedia(%p)::close()\n", this);
   #endif

   switch( openState )
   {
     case STATE_OUTPUT:
     case STATE_INOUT:
     case STATE_OUTIN:
       flush();
       break;

     case STATE_INPUT:
       break;

     default:
       debugf("TempMedia(%p)::close() state(%d)\n", this, state);
       throw "MediaStateException";
   }

   openState= state= STATE_RESET;
   return 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       TempMedia::flush
//
// Purpose-
//       Flush the TempMedia
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   TempMedia::flush( void )         // Flush the TempMedia
{
   #ifdef HCDM
     debugf("TempMedia(%p)::flush()\n", this);
   #endif

   switch( openState )
   {
     case STATE_INPUT:
       break;

     case STATE_OUTPUT:
     case STATE_INOUT:
     case STATE_OUTIN:
       if( tail != NULL )
         ((TempStorage*)tail)->size= size;
       break;

     default:
       debugf("TempMedia(%p)::flush()\n", this);
       throw "InvalidArgumentException";
   }

   return 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       TempMedia::read
//
// Purpose-
//       Read from the TempMedia
//
//----------------------------------------------------------------------------
Media::Size_t                       // The number of Bytes read
   TempMedia::read(                 // Read from the TempMedia
     Byte*             addr,        // Into this address
     Size_t            size)        // For this length
{
   #ifdef HCDM
     debugf("TempMedia(%p)::read(%p,%lu)\n", this, addr, size);
   #endif

   Size_t              result= 0;
   TempStorage*        temp= (TempStorage*)busy;
   Size_t              xfer;        // Transfer length

   switch( state )
   {
     case STATE_EOF:
       if( temp == NULL )
       {
         if( head == NULL )
           break;

         busy= temp= (TempStorage*)head;
         used= 0;
       }
       else if( used >= temp->size && temp->next == NULL )
         break;

       state= openState;
       [[ fallthrough ]]
       ;;

     case STATE_INPUT:
     case STATE_INOUT:
     case STATE_OUTIN:
       while( result < size )
       {
         if( temp == NULL )
         {
           if( head == NULL )
           {
             state= STATE_EOF;
             break;
           }

           busy= temp= (TempStorage*)head;
           used= 0;
         }

         if( used >= temp->size )
         {
           if( temp->next == NULL )
           {
             if( result == 0 )
               state= STATE_EOF;

             break;
           }

           busy= temp= temp->next;
           used= 0;
           continue;
         }

         xfer= temp->size - used;
         if( xfer > (size - result) )
           xfer= size - result;

         memcpy(addr + result, temp->data + used, xfer);
         result += xfer;
         used += xfer;
       }
       break;

     default:
       debugf("TempMedia(%p)::read(%p,%lu) state(%d)\n",
              this, addr, size, state);
       throw "MediaStateException";
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       TempMedia::write
//
// Purpose-
//       Write into the TempMedia
//
//----------------------------------------------------------------------------
Media::Size_t                       // The number of Bytes written
   TempMedia::write(                // Write into the TempMedia
     const Byte*       addr,        // From this address
     Size_t            size)        // For this length
{
   #ifdef HCDM
     debugf("TempMedia(%p)::write(%p,%lu)\n", this, addr, size);
   #endif

   Size_t              result= 0;
   TempStorage*        temp= (TempStorage*)tail;
   Size_t              xfer;        // Transfer length

   switch( state )
   {
     case STATE_OUTPUT:
     case STATE_INOUT:
     case STATE_OUTIN:
       if( tail == NULL )
       {
         head= tail= busy= temp= new TempStorage();
         TempMedia::size= used= 0;
       }

       while( result < size )
       {
         xfer= STORAGE_SIZE - TempMedia::size;
         if( xfer == 0 )
         {
           temp->size= TempMedia::size;
           temp= new TempStorage();
           ((TempStorage*)tail)->next= temp;
           TempMedia::tail= temp;
           TempMedia::size= 0;

           xfer= STORAGE_SIZE;
         }

         if( xfer > (size - result) )
           xfer= size - result;

         memcpy(temp->data + TempMedia::size, addr + result, xfer);
         result += xfer;
         TempMedia::size += xfer;
       }
       break;

     default:
       debugf("TempMedia(%p)::write(%p,%lu) state(%d)\n",
              this, addr, size, state);
       throw "MediaStateException";
   }

   return result;
}

