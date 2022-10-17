//----------------------------------------------------------------------------
//
//       Copyright (C) 2022 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Buffer.h
//
// Purpose-
//       Storage BufferData, Buffer, BufferReader, and BufferBorrow
//
// Last change date-
//       2022/09/23
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_BUFFER_H_INCLUDED
#define _LIBPUB_BUFFER_H_INCLUDED

#include <string>                   // For std::string, size_t, ...
#include <stdint.h>                 // For uint8_t
#include <stdio.h>                  // For EOF
#include <string.h>                 // For strlen

#include <pub/bits/pubconfig.h>     // For _LIBPUB_ macros

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class Buffer;
class BufferBorrow;
class BufferReader;

//----------------------------------------------------------------------------
//
// Struct-
//       BufferData
//
// Purpose-
//       Buffer data area.
//
//----------------------------------------------------------------------------
struct BufferData {                 // Buffer data area
friend class Buffer;
friend class BufferBorrow;
friend class BufferReader;

char*                  addr= nullptr; // The accumulator buffer address
size_t                 size= 0;     // The accumulator buffer size
size_t                 used= 0;     // Append offset, number of chars written
size_t                 seen= 0;     // Reader offset, number of chars read

void
   debug(                           // Debugging display
     const char*       info="") const; // Caller information
}; // struct BufferData

//----------------------------------------------------------------------------
//
// Class-
//       Buffer
//
// Purpose-
//       BufferData read/write view.
//
//----------------------------------------------------------------------------
class Buffer : protected BufferData  { // BufferData read/write view
friend class BufferBorrow;
friend class BufferReader;

//----------------------------------------------------------------------------
// Buffer::Constructors/Destructor
//----------------------------------------------------------------------------
public:
   Buffer( void ) = default;        // Default constructor

   explicit Buffer(size_t size)     // Initial size constructor
{  resize(size); }

   Buffer(const Buffer& src)        // Copy constructor
{  operator=(src); }

   Buffer(Buffer&& src)             // Move constructor
{  operator=(src); }

   ~Buffer( void )                  // Destructor
{  delete[] addr; }

//----------------------------------------------------------------------------
// Buffer::operators
//----------------------------------------------------------------------------
Buffer&
   operator=(const Buffer& src);    // Copy operator

Buffer&
   operator=(Buffer&&);             // Move operator

   explicit operator std::string( void ) const; // Cast to std::string

//----------------------------------------------------------------------------
// Buffer::Accessor methods, intended for library use
//----------------------------------------------------------------------------
char*&  _addr(void) { return addr; }
size_t& _size(void) { return size; }
size_t& _used(void) { return used; }
size_t& _seen(void) { return seen; }

//----------------------------------------------------------------------------
// Buffer::Methods
//----------------------------------------------------------------------------
void append(const Buffer&);         // Append Buffer to Buffer
void append(const char*);           // Append C-string to Buffer
void append(const std::string&);    // Append std::string to Buffer

void
   debug(                           // Debugging display
     const char*       info="") const // Caller information
{  BufferData::debug(info); }

int                                 // The next character
   get( void );                     // Get next character

int                                 // The next character
   peek( void ) const;              // Examine the next character

void
   put(int);                        // Put next character

ssize_t                             // The number of characters read
   read(void*, size_t);             // Read from Buffer (address, length)

std::string                         // The next token
   read_token(const char*);         // Read the next token (delimiters)

void
   reset( void )                    // Reset the buffer, emptying it
{  used= 0; seen= 0; }

ssize_t                             // Number of characters written
   write(const void*, size_t);      // Write into Buffer (address, length)

protected:
void
   resize(size_t);                  // Resize (expand) the buffer
}; // class Buffer

//----------------------------------------------------------------------------
//
// Class-
//       BufferReader
//
// Purpose-
//       Reader view of a const Buffer reference
//
// Implementation note-
//       The source const Buffer& MUST remain constant while any associated
//       BufferReader exists.
//
//----------------------------------------------------------------------------
class BufferReader : protected BufferData {
friend class BufferBorrow;

//----------------------------------------------------------------------------
// BufferReader::Constructors/Destructor
//----------------------------------------------------------------------------
protected:
   explicit
   BufferReader( void ) = default;  // Default constructor (for BufferBorrow)

   explicit
   BufferReader(Buffer& src)        // Reference constructor (for BufferBorrow)
{  addr= src.addr; used= src.used; seen= src.seen; }

public:
   BufferReader(const Buffer& src)  // Reference constructor
{  addr= src.addr; used= src.used; }

   ~BufferReader( void ) = default; // Destructor

//----------------------------------------------------------------------------
// BufferReader::operators
//----------------------------------------------------------------------------
   explicit operator std::string( void ) const // Cast to std::string
{  return ((Buffer*)this)->operator std::string(); }

//----------------------------------------------------------------------------
// BufferReader::Accessor methods, intended for library use
//----------------------------------------------------------------------------
char*&  _addr(void) { return addr; }
size_t& _size(void) { return size; }
size_t& _used(void) { return used; }
size_t& _seen(void) { return seen; }

//----------------------------------------------------------------------------
// BufferReader::Methods
//----------------------------------------------------------------------------
void
   debug(                           // Debugging display
     const char*       info="") const // Caller information
{  BufferData::debug(info); }

int                                 // The next character
   get( void )                      // Get the next character
{  return ((Buffer*)this)->get(); }

int                                 // The next character
   peek( void ) const               // Examine the next character
{  return ((Buffer*)this)->peek(); }

ssize_t                             // The number of characters read
   read(void* addr, size_t size)    // Read from Buffer
{  return ((Buffer*)this)->read(addr, size); }

std::string                         // The next token
   read_token(const char* delim)    // Read the next token (token delimiters)
{  return ((Buffer*)this)->read_token(delim); }

void
   reset( void )                    // Reset the BufferReader for re-use
{  seen= 0; }
}; // class BufferReader

//----------------------------------------------------------------------------
//
// Class-
//       BufferBorrow
//
// Purpose-
//       BufferData writer/reader view.
//
// Implementation note-
//       The source Buffer& is "borrowed" and must only be used by this
//       BufferBorrow. The source may also be any other static buffer.
//
//----------------------------------------------------------------------------
class BufferBorrow : public BufferReader { // BufferData writer/reader view
static int             check_write; // If true, check for write overflow

//----------------------------------------------------------------------------
// BufferBorrow::Constructor/Destructor
//----------------------------------------------------------------------------
public:
   BufferBorrow(Buffer& src)        // Reference constructor
:  BufferReader(src)
{  addr= src.addr; size= src.size; used= src.used; }

   BufferBorrow(void* addr, size_t size) // Static BufferBorrow constructor
:  BufferReader()
{  this->addr= (char*)addr; this->size= size; }

   ~BufferBorrow( void ) = default; // Destructor

//----------------------------------------------------------------------------
// BufferBorrow::operators
//----------------------------------------------------------------------------
   explicit operator std::string( void ) const // Cast to std::string
{  return ((Buffer*)this)->operator std::string(); }

//----------------------------------------------------------------------------
// BufferBorrow::Methods
//----------------------------------------------------------------------------
void append(const Buffer& S_)       // Append Buffer to Buffer
{  Buffer& S= const_cast<Buffer&>(S_); write(S._addr(), S._size()); }

void append(const char* S)          // Append C-string to Buffer
{  write(S, strlen(S)); }

void append(const std::string& S)   // Append std::string to Buffer
{  write(S.c_str(), S.size()); }

int                                 // The number of characters written
   put(int C);                      // Write one character

void
   reset( void )                    // Reset the BufferReader for re-use
{  ((Buffer*)this)->reset(); }

ssize_t                             // Number of characters written
   write(const void* addr, size_t size); // Write into BufferBorrow
}; // class BufferBorrow
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_BUFFER_H_INCLUDED
