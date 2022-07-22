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
//       http/Data.h
//
// Purpose-
//       HTTP request/response data buffer.
//
// Last change date-
//       2022/02/16
//
// Implementation notes-
//       We don't pass-through string functions, but do provide get_string.
//
//----------------------------------------------------------------------------
#ifndef _PUB_HTTP_DATA_H_INCLUDED
#define _PUB_HTTP_DATA_H_INCLUDED

#include <string>                   // For std::string
#include <stdint.h>                 // For uint8_t

#include <pub/List.h>               // For pub::List

namespace pub::http {
//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
struct Hunk;                        // pub::http::Hunk, defined here

//----------------------------------------------------------------------------
//
// Class-
//       Data
//
// Purpose-
//       Request/response data buffer.
//
//----------------------------------------------------------------------------
class Data {                        // Request/response data buffer
//----------------------------------------------------------------------------
// Data::Attributes
//----------------------------------------------------------------------------
public:
static constexpr size_t npos= -1;   // No position, or length to end of string

protected:
pub::List<Hunk>        list;        // Our list of Hunks
size_t                 size= 0;     // The combined Hunk size

//----------------------------------------------------------------------------
// Data::Destructor/constructors
//----------------------------------------------------------------------------
public:
   ~Data( void );                   // Destructor

   Data( void );                    // Default constructor
   Data(                            // Copy constructor
     const Data&       data);       // Source Data object

//----------------------------------------------------------------------------
// Data::debug
//----------------------------------------------------------------------------
void debug(const char*) const;      // Debugging display
void debug( void ) const            // Debugging display
{  debug(""); }

//----------------------------------------------------------------------------
// Data::Accessor methods
//----------------------------------------------------------------------------
size_t
   get_size( void ) const           // Get total data length
{  return size; }

std::string                         // The data
   get_string( void ) const;        // Get data string

//----------------------------------------------------------------------------
// Data::Operators
//----------------------------------------------------------------------------
Data& operator+=(const Data& data)  // Append Data
{  append(data); return *this; }

Data& operator+=(const Hunk& hunk)  // Append Hunk
{  append(hunk); return *this; }

Data& operator+=(const std::string& string) // Append std::string
{  append(string); return *this; }

// (Returns nullptr if out of range)
const char* operator[](size_t) const; // Address (single) character

//----------------------------------------------------------------------------
// Data::Methods
//----------------------------------------------------------------------------
void append(const void*, size_t);   // Append buffer
void append(const Data&);           // Append Data
void append(const Hunk&);           // Append Hunk
void append(const std::string);     // Append std::string

void discard(size_t);               // Discard (beginning at origin)

size_t                              // The number of bytes stored
   store(                           // Store (copy) the buffer
     void*             addr,        // Into this address
     size_t            size,        // For this length
     size_t            offset= 0) const; // Starting at this offset

void reset( void );                 // Reset the Data
}; // class Data

//----------------------------------------------------------------------------
//
// Struct-
//       Hunk
//
// Purpose-
//       Data buffer segment
//
//----------------------------------------------------------------------------
struct Hunk : public pub::List<Hunk>::Link { // Data buffer segment
char*                  addr;        // Data address
size_t                 size;        // Data length

   ~Hunk( void ) = default;         // Destructor

   Hunk( void ) = default;          // Default constructor
   Hunk(                            // Constructor
     const void*       addr_,       // Source buffer*
     size_t            size_)       // Source buffer length
:  Link(), addr((char*)addr_), size(size_) {}

//----------------------------------------------------------------------------
// Hunk::debug
//----------------------------------------------------------------------------
void debug(const char*) const;      // Debugging display
void debug( void ) const            // Debugging display
{  debug(""); }
}; // struct Hunk

//----------------------------------------------------------------------------
//
// Struct-
//       Buffer
//
// Purpose-
//       Data accumulator buffer.
//
// Implementation notes-
//       The maximum length is predefined, built into Buffer logic.
//       Method append throws std::length_error on buffer overflow.
//
//----------------------------------------------------------------------------
struct Buffer {                     // Data accumulator Buffer

char* const            addr= nullptr; // The accumulator buffer address
const size_t           size= 0;     // The accumulator buffer length
size_t                 length= 0;   // Append offset
size_t                 offset= 0;   // Reader offset

   ~Buffer( void );                 // Destructor
   Buffer(size_t size= 0);          // Constructor

//----------------------------------------------------------------------------
// Buffer::debug
//----------------------------------------------------------------------------
void debug(const char*) const;      // Debugging display
void debug( void ) const            // Debugging display
{  debug(""); }

//----------------------------------------------------------------------------
// Buffer::Methods
//----------------------------------------------------------------------------
void append(const char*);          // Append string to Buffer
void append(const std::string&);   // Append string to Buffer
void append(int C);                // Append character to Buffer

void
   fetch(                           // Fetch
     const Data&       data,        // From this Data
     size_t            offset= 0);  // At this Data offset
void fetch(const Hunk&);            // Fetch (from Hunk)

int                                 // The next character
   peek_char( void ) const;         // Examine current character

int                                 // The next character
   read_char( void );               // Read next character

// Implementation note: read_token modifies the accumulator buffer. The buffer
// MUST be initialized again before it can be reused.
const char*                         // The next token, or ""
   read_token(                      // Get next token
     const char*       delim);      // Allowed delimiters

void
   reset( void )                    // Reset the buffer for re-use
{  length= 0; offset= 0; }
}; // struct Buffer
}  // namespace pub::http
#endif // _PUB_HTTP_DATA_H_INCLUDED
