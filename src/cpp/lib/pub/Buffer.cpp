//----------------------------------------------------------------------------
//
//       Copyright (C) 2022 Frank Eskesen.
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
//       Implement Buffer.h
//
// Last change date-
//       2022/09/24
//
//----------------------------------------------------------------------------
#include <new>                      // For std::bad_alloc
#include <cstdlib>                  // For malloc, size_t
#include <cstring>                  // For memcpy, memset, ...
#include <stdexcept>                // For std::runtime_error, ...
#include <string>                   // For std::string

#include <assert.h>                 // For assert
#include <stdio.h>                  // For fprintf, EOF
#include <stdint.h>                 // For integer types

#include "pub/Buffer.h"             // For pub::Buffer, implemented
#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/Exception.h>          // For pub::Exception
#include <pub/List.h>               // For pub::List
#include <pub/utility.h>            // For utility functions

#define PUB _LIBPUB_NAMESPACE
using namespace PUB;
using namespace PUB::debugging;
using namespace PUB::utility;
using std::string;

namespace _LIBPUB_NAMESPACE {       // Implementation namespace
//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 2                       // Verbosity, higher is more verbose
}; // enum

//----------------------------------------------------------------------------
// External data areas
//----------------------------------------------------------------------------
int                    BufferBorrow::check_write= true; // Overflow exception?

//----------------------------------------------------------------------------
//
// Method-
//       Buffer constructors
//
// Purpose-
//       Offline debugging
//
//----------------------------------------------------------------------------
#if 0
   Buffer::Buffer()
{  debugf("Buffer(%p)::Buffer\n", this); }

   explicit Buffer::Buffer(size_t size)
{  debugf("Buffer(%p)::Buffer(%zd)\n", this, size);
   resize(size);
}

   Buffer::Buffer(const Buffer& src)
{  debugf("Buffer(%p)::Buffer(& %p)\n", this, &src);
   operator=(src);
}

   Buffer::Buffer(Buffer&& src)
{  debugf("Buffer(%p)::Buffer(&& %p)\n", this, &src);
   operator=(src);
}

   Buffer::~Buffer()
{  debugf("Buffer(%p)::~Buffer\n", this); }

   BufferReader::BufferReader(const Buffer& src)
{  debugf("BufferReader(%p)::BufferReader(%p)\n", this, &src);
   addr= src.addr; used=src.used;
}

   BufferReader::~BufferReader()
{  debugf("BufferReader(%p)::~BufferReader\n", this); }

   BufferWriter::BufferWriter(const Buffer& src)
{  debugf("BufferWriter(%p)::BufferWriter(%p)\n", this, &src);
   addr= src.addr; size= src.size;
}

   BufferWriter::~BufferWriter()
{  debugf("BufferWriter(%p)::~BufferWriter\n", this); }
#endif

//----------------------------------------------------------------------------
//
// Method-
//       BufferData::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   BufferData::debug(const char* info) const // Debugging display
{  debugf("Buffer(%p)::debug(%s) {%p.%.4zd} {%4zd,%4zd}\n", this, info
         , addr, size, used, seen);
   if( addr && VERBOSE > 1 )
     dump(addr, used);
}

//----------------------------------------------------------------------------
//
// Method-
//       Buffer::operator=
//
// Purpose-
//       Copy operator=
//       Move operator=
//
//----------------------------------------------------------------------------
Buffer&                             // (Always *this)
   Buffer::operator=(               // Copy operator
     const Buffer&     src)         // Source Buffer
{  if( HCDM && VERBOSE > 1 )
      debugf("Buffer(%p)::operator=(& %p)\n", this, &src);

   reset();

   append(src);
   used= src.used;
   seen= src.seen;

   return *this;
}

Buffer&                             // (Always *this)
   Buffer::operator=(               // Move operator
     Buffer&&          src)         // Source Buffer
{  if( HCDM && VERBOSE > 1 )
      debugf("Buffer(%p)::operator=(&& %p)\n", this, &src);

   delete[] addr;

   addr= src.addr;
   size= src.size;
   used= src.used;
   seen= src.seen;

   src.addr= nullptr;
   src.size= 0;
   src.used= 0;
   src.seen= 0;

   return *this;
}

//----------------------------------------------------------------------------
//
// Method-
//       Buffer::operator std::string
//
// Purpose-
//       Convert the (entire) Buffer content into a std::string
//
//----------------------------------------------------------------------------
   Buffer::operator std::string( void ) const  // Convert to std::string
{
   if( used == 0 )                  // If the Buffer's empty
     return "";

   std::string out(addr, used);     // The resultant string
   return out;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Buffer::append
//
// Purpose-
//       Append to Buffer
//
//----------------------------------------------------------------------------
void
   Buffer::append(                 // Append (entire) Buffer to Buffer
     const Buffer&     S)          // The append Buffer
{
   resize(used + S.used);

   memcpy(addr+used, S.addr, S.used);
   used += S.used;
}

void
   Buffer::append(                 // Append C-string to Buffer
     const char*       S)          // The append string
{
   size_t append_size= strlen(S);
   if( (used + append_size) > size )
     resize(used + append_size);

   memcpy(addr+used, S, append_size);
   used += append_size;
}

void
   Buffer::append(                 // Append std::string to Buffer
     const string&     S)          // The append string
{
   size_t append_size= S.size();
   if( (used + append_size) > size )
     resize(used + append_size);

   memcpy(addr+used, S.c_str(), append_size);
   used += append_size;
}

//----------------------------------------------------------------------------
//
// Method-
//       Buffer::get
//
// Purpose-
//       Read next character, -1 at end of file.
//
//----------------------------------------------------------------------------
int                                 // The next character
   Buffer::get( void )              // Read next character
{  if( HCDM && VERBOSE > 1 ) {
     int C= seen < size ? addr[seen] : 0;
     string S= utility::visify(C);
     debugf("'%s'= Buffer(%p)::get[%zd]\n", S.c_str(), this, seen);
   }

   if( seen < used )
     return addr[seen++];

   return EOF;
}

//----------------------------------------------------------------------------
//
// Method-
//       Buffer::peek
//
// Purpose-
//       Peek at next character
//
//----------------------------------------------------------------------------
int                                 // The next character, 0 if none
   Buffer::peek( void ) const       // Peek at next character
{  if( HCDM && VERBOSE > 1) {
     int C= seen < size ? addr[seen] : 0;
     string S= utility::visify(C);
     debugf("'%s'= Buffer(%p)::peek[%zd]\n", S.c_str(), this, seen);
   }

   if( seen < used )
     return addr[seen];

   return EOF;
}

//----------------------------------------------------------------------------
//
// Method-
//       Buffer::put
//
// Purpose-
//       Write character.
//
//----------------------------------------------------------------------------
void
   Buffer::put(                    // Put character into Buffer
     int               C)          // The output character
{  if( HCDM && VERBOSE > 1 )
     debugf("Buffer(%p)::put(%c) [%4zd]\n", this, C, used);

   if( used >= size ) {
     if( size == 0 )
       resize(256);
     else
       resize(used + 1);
   }

   addr[used++]= C;
}

//----------------------------------------------------------------------------
//
// Method-
//       Buffer::read
//
// Purpose-
//       Read from Buffer
//
// Implementation note-
//       ** WARNING** NOT TESTED.
//
//----------------------------------------------------------------------------
ssize_t                             // The number of bytes read
   Buffer::read(                    // Read
     void*             addr,        // At this address
     size_t            size)        // For this length
{  if( HCDM && VERBOSE > 1 )
     debugf("Buffer(%p)::read(%p,%zd)\n", this, addr, size);

   size_t avail= used - seen;       // Number of available characters
   if( avail ) {                    // If some data available
     if( avail > size )             // If we can fully satisfy the read
       avail= size;                 // (More data remains available)
     memcpy(addr, this->addr + seen, avail);
     seen += avail;
     return avail;
   }

   return EOF;
}

//----------------------------------------------------------------------------
//
// Method-
//       Buffer::read_token
//
// Purpose-
//       Read next token
//
// Implementation notes-
//       ** WARNING** NOT TESTED.
//       ** WARNING** QUOTES DO NOT GET SPECIAL HANDLING (YET).
//
//       Leading/trailing whitespace IS NOT ignored
//
//----------------------------------------------------------------------------
std::string                         // The next token, or ""
   Buffer::read_token(              // Get next token
     const char*       delim)       // Allowed delimiters
{  if( HCDM && VERBOSE > 0 )
     debugf("Buffer(%p)::read_token(%s) [%zd]\n", this, visify(delim).c_str()
           , seen);

   size_t origin= seen;             // String origin
   for(;;) {                        // Read next token
     int C= get();                  // The next character
     for(int i= 0; delim[i]; i++) {
       if( C == delim[i] ) {
         std::string S(addr + origin, seen - origin - 1);

         if( C == '\r' ) {          // (Possible "\r\n" delimiter);
           if( peek() == '\n' && delim[i+1] == '\n' )
             get();
         }

         return S;
       }

       if( C == '\r' || C == '\n' || C == EOF ) { // If unexpected terminator
         if( seen > 0 )             // (Backspace)
           --seen;
         std::string S(addr + origin, seen - origin);
         return S;
       }
     }
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Buffer::resize
//
// Purpose-
//       Resize the Buffer
//
//----------------------------------------------------------------------------
void
   Buffer::resize(                  // Resize the Buffer
     size_t            size)        // The updated size
{  if( HCDM && VERBOSE > 1 ) debugf("Buffer(%p)::resize(%zd)\n", this, size);

   if( size <= this->size )         // Ignore if new size <= old size
     return;

   // Update the request size in order to minimize resize requests
   if( this->size ) {               // If not the first request
     if( size < 256 )
       size= 256;
     else if( size < 4096 )
       size= 4096;
     else if( size < 8192 )
       size= 8192;
     else if( size < 16384 )
       size= 16384;
     else {
       size += 16383;               // Round up to or past next 16K boundary
       size &= ~(16383);            // Truncate at 16K boundary
     }
   }

   // Replace the buffer
   char* new_addr= new char[size];
   memcpy(new_addr, addr, used);   // (realloc)
   memset(new_addr+used, 0, size-used);
   delete[] addr;
   addr= new_addr;
   this->size= size;
}

//----------------------------------------------------------------------------
//
// Method-
//       Buffer::write
//
// Purpose-
//       Write into Buffer
//
// Implementation note-
//       ** WARNING** NOT TESTED.
//
//----------------------------------------------------------------------------
ssize_t                             // The number of bytes writter
   Buffer::write(                   // Write
     const void*       addr,        // From this address
     size_t            size)        // For this length
{  if( HCDM && VERBOSE > 1 )
     debugf("Buffer(%p)::write(%p,%zd)\n", this, addr, size);

   resize(used + size);             // Make room
   memcpy(this->addr + used, addr, size);
   used += size;

   return size;                     // Indicates success
}

//----------------------------------------------------------------------------
//
// Method-
//       BufferBorrow::put
//
// Purpose-
//       Write character.
//
//----------------------------------------------------------------------------
int                                // The number of characters written
   BufferBorrow::put(              // Put character into BufferBorrow
     int               C)          // The output character
{  if( HCDM && VERBOSE > 1 )
     debugf("Buffer(%p)::put(%c) [%4zd]\n", this, C, used);

   if( used >= size )
     return 0;

   addr[used++]= C;
   return 1;
}

//----------------------------------------------------------------------------
//
// Method-
//       BufferBorrow::write
//
// Purpose-
//       Write into Buffer
//
// Implementation note-
//       ** WARNING** NOT TESTED.
//
//----------------------------------------------------------------------------
ssize_t                             // The number of bytes written
   BufferBorrow::write(             // Write
     const void*       addr,        // From this address
     size_t            size)        // For this length
{  if( HCDM && VERBOSE > 1 )
     debugf("BufferBorrow(%p)::write(%p,%zd)\n", this, addr, size);

   size_t remain= this->size - used; // Number of remaining characters
   if( remain ) {                   // If some space remains available
     if( remain > size )            // If we can fully satisfy the write
       remain= size;                // (More space remains available)
     memcpy(this->addr + used, addr, remain);
     used += remain;
   }

   if( check_write && remain != size )
     throw std::runtime_error("BufferBorrow incomplete write");

   return remain;
}
}  // namespace _LIBPUB_NAMESPACE
