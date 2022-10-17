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
//       Data.cpp
//
// Purpose-
//       Implement http/Data.h
//
// Last change date-
//       2022/09/24
//
// Implementation notes-
//       TODO: optimization, but DEFER until usage code closer to completion.
//       ~Hunk shouldn't delete data. This should be done in ~Data instead.
//       This allows Hunk to be used as a plain structure.
//       Add an array of Hunks in Data, and make that array the linked
//       object, or HunkArrayLink. Have one HunkArray defined in Data, which
//       handles the normal use case. An allocator can allocate and release
//       the Hunk data areas using a set of fixed Hunk sizes. Each Hunk can
//       accept more data up to its size, avoiding lots of allocations for
//       small Hunk additions. Add append(void*, size_t), which would be the
//       *only* functional append function. All others call that Data append.
//
//       Data::reset function needed?
//
//       Data::cache offset: hunk + index?
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

#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/Exception.h>          // For pub::Exception
#include <pub/List.h>               // For pub::List
#include <pub/utility.h>            // For pub::to_string

#include "pub/http/Data.h"          // For pub::http::Data, implemented

using namespace _LIBPUB_NAMESPACE;
using namespace _LIBPUB_NAMESPACE::debugging;
using _LIBPUB_NAMESPACE::utility::to_string;
using _LIBPUB_NAMESPACE::utility::visify;
using std::string;

namespace _LIBPUB_NAMESPACE::http { // Implementation namespace
//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 2                       // Verbosity, higher is more verbose

,  USE_VERIFY= true                 // Use internal consistency checking?
}; // enum

//----------------------------------------------------------------------------
//
// Method-
//       Data::Data
//       Data::~Data
//
// Purpose-
//       Constructors
//       Destructor
//
//----------------------------------------------------------------------------
   Data::Data( void )             // Default constructor
:  list()
{  if( HCDM )
     debugh("Data(%p)::Data\n", this);
}

   Data::Data(                    // Copy constructor
     const Data&       from)      // (The copy source)
:  list()
{  if( HCDM )
     debugh("Data(%p)::Data(%p)\n", this, &from);

   append(from);
}

   Data::~Data( void )            // Destructor
{  if( HCDM )
     debugh("Data(%p)::~Data\n", this);

   reset();                       // Delete Hunks
}

//----------------------------------------------------------------------------
//
// Method-
//       Data::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   Data::debug(const char* info) const // Debugging display
{  debugf("Data(%p)::debug(%s) size(%zd)\n", this, info, size);
   size_t index= 0;
   size_t total= 0;
   for(Hunk* hunk= list.get_head(); hunk; hunk= hunk->get_next()) {
     if( hunk->size > 16 ) {
       string S((char*)hunk->addr, 16);
       S= visify(S);
       debugf("..[%2zd] %p {%p,%4zd} '%s'...\n", index++, hunk
              , hunk->addr, hunk->size, S.c_str());
     } else {
       string S((char*)hunk->addr, hunk->size);
       S= visify(S);
       debugf("..[%2zd] %p {%p,%4zd} '%s'\n", index++, hunk
              , hunk->addr, hunk->size, S.c_str());
     }

     total += hunk->size;
   }
   debugf("..[%2zd] %8zd Total\n", index, total);
   if( total != size )
     debugf("..Total(%zd) != size(%zd) ****** WARNING ******\n", total, size);
}

//----------------------------------------------------------------------------
// Data::Accessor methods
//----------------------------------------------------------------------------
std::string                         // The data
   Data::get_string( void ) const   // Get data string
{  if( HCDM )
     debugh("Data(%p)::get_string\n", this);

   string output;                   // The output string

   const Hunk* hunk= list.get_head();
   while( hunk ) {
     string T(hunk->addr, hunk->size);
     output += T;
     hunk= hunk->get_next();
   }

   return output;
}

//----------------------------------------------------------------------------
// Data::Operators
//----------------------------------------------------------------------------
const char*                         // The character (nullptr if out of range)
   Data::operator[](size_t index) const // Address character[index]
{  if( HCDM && VERBOSE > 1 )
     debugh("Data(%p)[%zd]\n", this, index);

   const Hunk* hunk= list.get_head();
   while( index ) {
     if( hunk == nullptr )
       return nullptr;

     if( index < hunk->size )       // If character in hunk
       break;

     index -= hunk->size;
     hunk= hunk->get_next();
   }

   // Return the character's address
   return ((const char*)hunk->addr) + index;
}

//----------------------------------------------------------------------------
//
// Method-
//       Data::append
//
// Purpose-
//       Append Data
//
//----------------------------------------------------------------------------
void
   Data::append(const void* addr, size_t size) // Append data area
{  if( HCDM )
     debugh("Data(%p)::append(%p,%zd)\n", this, addr, size);

   Hunk* hunk= new Hunk();          // Create a new Hunk
   char* copy= (char*)malloc(size); // Allocate a copy area
   if( copy == nullptr ) {          // If not allocated
     delete hunk;                   // (delete the allocated hunk)
     throw std::bad_alloc();        // Allocation (of copy) failed
   }

   memcpy(copy, addr, size);      // Copy the data
   hunk->addr= copy;              // Initialize the hunk
   hunk->size= size;
   if( HCDM ) debugh("Hunk(%p) {%p,%zd}\n", hunk, copy, size);

   list.fifo(hunk);
   this->size += hunk->size;
}

void
   Data::append(const Buffer& copy) // Append Buffer
{  const void*  addr= const_cast<Buffer&>(copy)._addr();
   const size_t size= const_cast<Buffer&>(copy)._used();

   if( HCDM )
     debugh("Data(%p)::append(Buffer{%p,%zd})\n", this, addr, size);

   append(addr, size);
}

void
   Data::append(const Data& copy)   // Append Data
{  if( HCDM )
     debugh("Data(%p)::append(Data:%p)\n", this, &copy);

   for(Hunk* hunk= copy.list.get_head(); hunk; hunk= hunk->get_next())
     append(hunk->addr, hunk->size);
}

void
   Data::append(const Hunk& copy)   // Append Hunk
{  if( HCDM )
     debugh("Data(%p)::append(Hunk:%p)\n", this, &copy);

   append(copy.addr, copy.size);
}

void
   Data::append(const std::string copy) // Append std::string
{  if( HCDM )
     debugh("Data(%p)::append(String:%s)\n", this, copy.c_str());

   append(copy.c_str(), copy.size());
}

//----------------------------------------------------------------------------
//
// Method-
//       Data::discard
//
// Purpose-
//       Discard beginning Data
//
// Purpose-
//       Discard beginning Data
//
//----------------------------------------------------------------------------
void
   Data::discard(                   // Discard
     size_t            count)       // This many bytes
{  if( HCDM )
     debugh("Data(%p)::discard(%zd)\n", this, count);

   for(Hunk* hunk= list.remq(); hunk && count != 0; hunk= list.remq()) {
     if( hunk->size > count ) {     // If partial hunk removal
       hunk->size -= count;
       size -= count;
       memmove(hunk->addr, hunk->addr+count, hunk->size);
       list.lifo(hunk);
       return;
     }

     count -= hunk->size;
     size  -= hunk->size;
     if( HCDM ) debugh("~Hunk(%p) {%p,%zd}\n", hunk, hunk->addr, hunk->size);
     free(hunk->addr);
     delete hunk;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Data::reset
//
// Purpose-
//       Discard all Data
//
//----------------------------------------------------------------------------
void
   Data::reset( void )              // Reset the Data
{  if( HCDM )
     debugh("Data(%p)::reset\n", this);

   for(Hunk* hunk= list.remq(); hunk; hunk= list.remq()) {
     if( HCDM ) debugh("~Hunk(%p) {%p,%zd}\n", hunk, hunk->addr, hunk->size);
     free(hunk->addr);
     delete hunk;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Data::store
//
// Purpose-
//       Store Data
//
//----------------------------------------------------------------------------
size_t                              // The number of bytes stored
   Data::store(                     // Store (copy) the Data
     BufferBorrow&     buff) const  // Into this BufferBorrow
{
   buff._used()= store(buff._addr(), buff._size());
   if( USE_VERIFY && get_size() > buff._used() ) // TODO: ??possible??
     throw std::runtime_error("Data::store(BufferBorrow&) overflow");

   return buff._used();
}

size_t                              // The number of bytes stored
   Data::store(                     // Store (copy) the Data
     void*             addr_,       // Into this address
     size_t            size) const  // For this length
{  size_t offset= 0;                // TODO: remove offset code

   if( HCDM )
     debugh("Data(%p)::store(%p,%zd)\n", this, addr_, size);
//debug("store"); // SHCDM (Super HCDM)

   char* addr= (char*)addr_;        // (So addr supports arithmetic ops)

   const Hunk* hunk= list.get_head();
   while( offset ) {
     if( hunk == nullptr )          // If store complete, end of data
       break;

//debugf("%4d offset(%zd) ", __LINE__, offset); hunk->debug(); // SHCDM (Super HCDM)
     if( offset < hunk->size )      // Use the first copy hunk
       break;

     offset -= hunk->size;
     hunk= hunk->get_next();
   }

   // Position in current hunk
   if( hunk == nullptr ) {
//debugf("%4d offset(%zd) addr(%p) size(%zd)\n", __LINE__, offset, addr, size); // SHCDM (Super HCDM)
     if( size )
       *addr= '\0';
     return 0;
   }

//debugf("%4d offset(%zd) ", __LINE__, offset); hunk->debug(); // SHCDM (Super HCDM)
   const char* from_addr= (const char*)hunk->addr;
   size_t from_size= hunk->size;
   if( offset ) {
     from_addr += offset;
     from_size -= offset;
   }

   // Store data
   size_t length= 0;                // Number of bytes stored
   while( size >= from_size ) {     // Store Hunk by Hunk
//debugf("%4d addr(%p) size(%zd) len(%zd) fr_addr(%p) fr_size(%zd)\n", __LINE__
//      , addr, size, length, from_addr, from_size); // SHCDM (Super HCDM)
     memcpy(addr, from_addr, from_size); // Copy remaining hunk data
     length += from_size;           // Update copied length
     addr += from_size;
     size -= from_size;

     hunk= hunk->get_next();        // Get next hunk
     if( hunk == nullptr ) {        // If store complete, end of data
//debugf("%4d addr(%p) size(%zd) len(%zd) fr_addr(*) fr_size(*)\n", __LINE__
//      , addr, size, length); // SHCDM (Super HCDM)
       if( size )                   // If output buffer isn't full
         *addr= '\0';               // Set trailing delimiter
       return length;
     }

     from_addr= (const char*)hunk->addr;
     from_size= hunk->size;
   }

   // Store complete, buffer full or nearly full
   if( size ) {
//debugf("%4d addr(%p) size(%zd) len(%zd) fr_addr(%p) fr_size(%zd)\n", __LINE__
//      , addr, size, length, from_addr, from_size); // SHCDM (Super HCDM)
     memcpy(addr, from_addr, size); // Copy remaining partial hunk
     length += size;                // Update copied length
     addr += size;
   }

//debugf("%4d len(%zd) output buffer full\n", __LINE__, length); // SHCDM (Super HCDM)
   return length;                   // (Output buffer full, NO delimiter)
}

//----------------------------------------------------------------------------
//
// Method-
//       Hunk::~Hunk
//       Hunk::Hunk
//
// Purpose-
//       Destructor
//       Constructors
//
//----------------------------------------------------------------------------
#if 0 // Unused
   Hunk::~Hunk( void )            // Destructor
{  if( HCDM )
     debugh("Hunk(%p)::~Hunk\n", this);

   free(addr);
   addr= nullptr;
}

   Hunk::Hunk( void )             // Default constructor
{  if( HCDM )
     debugh("Hunk(%p)::Hunk\n", this);
}

   Hunk::Hunk(                      // Constructor
     const void*       addr_,       // Source data*
     size_t            size_)       // Source data length
:  Link(), addr(malloc(size_)), size(size_)
{  if( HCDM )
     debugh("Hunk(%p)::Hunk {%p,%zd}\n", this, addr, size);

   if( addr == nullptr )
     throw std::bad_alloc();

   if( addr_ != nullptr )
     memcpy(addr, addr_, size_);
}
#endif

//----------------------------------------------------------------------------
//
// Method-
//       Hunk::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   Hunk::debug(const char* info) const // Debugging display
{  debugf("Hunk(%p)::debug(%s) {%p.%4ld}\n", this, info, addr, size);
}

#if 0 // TODO: REMOVE
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
{
   addr= new char[src.size];        // (Duplicated) Buffer address
   memcpy(addr, src.addr, src.size); // Duplicate the data
   size= src.size;
   used= src.used;
   offset= src.offset;

   return *this;
}

Buffer&                             // (Always *this)
   Buffer::operator=(               // Move operator
     Buffer&&          src)         // Source Buffer
{
   addr= src.addr;
   size= src.size;
   used= src.used;
   offset= src.offset;

   src.addr= nullptr;
   src.size= 0;
   src.used= 0;
   src.offset= 0;

   return *this;
}

//----------------------------------------------------------------------------
//
// Method-
//       Buffer::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   Buffer::debug(const char* info) const // Debugging display
{  debugf("Buffer(%p)::debug(%s) {%p.%4ld} {%4ld,%4ld}\n", this, info
         , addr, size, used, offset);
   if( addr )
     debugf("{{\n%s\n}}\n", addr);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Buffer::append
//
// Purpose-
//       Append data to Buffer
//
//----------------------------------------------------------------------------
void
   Buffer::append(                 // Append string to Buffer
     const char*       S)          // The append string
{  size_t append_size= strlen(S);
   if( (used + append_size) > size )
     resize(used + append_size);

   memcpy(addr+used, S, append_size);
   used += append_size;
}

void
   Buffer::append(                 // Append string to Buffer
     const string&     S)          // The append string
{  size_t append_size= S.size();
   if( (used + append_size) > size )
     resize(used + append_size);

   memcpy(addr+used, S.c_str(), append_size);
   used += append_size;
}

void
   Buffer::append(                 // Append string to Buffer
     int               C)          // The append character
{
   if( used >= size )
     resize(used + 8);

   addr[used++]= C;
}

//----------------------------------------------------------------------------
//
// Method-
//       Buffer::fetch
//
// Purpose-
//       Fetch Data, loading (replacing) the Buffer
//
//----------------------------------------------------------------------------
void
   Buffer::fetch(                   // Fetch
     const Data&       data,        // From this Data
     size_t            offset)      // At this Data offset
{  if( HCDM ) debugh("Buffer(%p)::fetch(%p,%zd)\n", this, &data, offset);

   this->offset= 0;
   used= data.store(addr, size, offset);
}

void
   Buffer::fetch(                   // Fetch
     const Hunk&       hunk)        // From this Hunk
{  if( HCDM )
     debugh("Buffer(%p)::fetch({%p,%zd})\n", this, hunk.addr, hunk.size);

   this->offset= 0;
   used= hunk.size;
   if( used > size )
     resize(used);
   memcpy(addr, hunk.addr, used);
}

//----------------------------------------------------------------------------
//
// Method-
//       Buffer::peek_char
//
// Purpose-
//       Peek at next character
//
//----------------------------------------------------------------------------
int                                 // The next character, 0 if none
   Buffer::peek_char( void ) const  // Peek at next character
{  if( HCDM && VERBOSE > 1) {
     int C= offset < size ? addr[offset] : 0;
     string S= utility::visify(C);
     debugh("Buffer(%p)::peek_char[%zd] '%s'\n", this, offset, S.c_str());
   }

   if( offset < used )
     return addr[offset];

   return 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       Buffer::read_char
//
// Purpose-
//       Read next character, -1 at end of file.
//
//----------------------------------------------------------------------------
int                                 // The next character
   Buffer::read_char( void )        // Read next character
{  if( HCDM && VERBOSE > 1 ) {
     int C= offset < size ? addr[offset] : 0;
     string S= utility::visify(C);
     debugh("Buffer(%p)::read_char[%zd] '%s'\n", this, offset, S.c_str());
   }

   if( offset < used )
     return addr[offset++];

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
//       Leading/trailing whitespace IS NOT ignored
//
//----------------------------------------------------------------------------
const char*                         // The next token, or ""
   Buffer::read_token(              // Get next token
     const char*       delim)       // Allowed delimiters
{  if( HCDM && VERBOSE > 0 )
     debugh("Buffer(%p)::read_token(%s) [%zd]\n", this, visify(delim).c_str()
           , offset);

   size_t origin= offset;           // String origin
   for(;;) {                        // Read next token
     int C= read_char();            // The next character
     for(int i= 0; delim[i]; i++) {
       if( C == delim[i] ) {
         addr[offset - 1]= '\0';
         if( C == '\r' ) {          // (Delimiter "\r\n");
           if( peek_char() == '\n' )
             read_char();
         }
         return addr + origin;
       }

       if( C == '\r' ) {            // If '\r' found, not expected
         if( peek_char() == '\n' )  // If '\n' is next
           C= read_char();
         return "";
       }

       if( C == '\n')               // If '\n' found, not expected
         return "";

       if( C <= 0 )                 // If EOF encountered
         return "";
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
{  if( HCDM && VERBOSE > 1 ) debugh("Buffer(%p)::resize(%zd)\n", this, size);

   if( size < this->size )          // (Should not occur)
     return;                        // (But ignorable)

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
   delete addr;
   addr= new_addr;
   this->size= size;
}

//----------------------------------------------------------------------------
//
// Method-
//       Buffer::to_string
//
// Purpose-
//       Convert the (entire) Buffer content into a std::string
//
//----------------------------------------------------------------------------
std::string                         // The resultant string
   Buffer::to_string( void ) const  // Convert to std::string
{
   if( used == 0 )                  // If the Buffer's empty
     return "";

   std::string out(addr, used);     // The resultant string
   return out;
}
#endif
}  // namespace _LIBPUB_NAMESPACE::http
