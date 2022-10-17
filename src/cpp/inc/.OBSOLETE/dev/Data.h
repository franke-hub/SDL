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
//       HTTP request/response data organizer.
//
// Last change date-
//       2022/09/11
//
// Implementation notes-
//       We don't pass-through string functions, but do provide get_string.
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_HTTP_DATA_H_INCLUDED
#define _LIBPUB_HTTP_DATA_H_INCLUDED

#include <string>                   // For std::string
#include <stdint.h>                 // For uint8_t

#include "pub/Buffer.h"             // For pub::Buffer
#include <pub/List.h>               // For pub::List

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
namespace http {
//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
struct Hunk;                        // (defined in this header file)

//----------------------------------------------------------------------------
//
// Class-
//       Data
//
// Purpose-
//       Request/response data organizer.
//
//----------------------------------------------------------------------------
class Data {                        // Request/response data organizer
//----------------------------------------------------------------------------
// Data::Attributes
//----------------------------------------------------------------------------
public:
static constexpr size_t npos= -1;   // No position, or length to end of string

protected:
List<Hunk>             list;        // Our list of Hunks
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
void append(const Buffer&);         // Append Buffer
void append(const Data&);           // Append Data
void append(const Hunk&);           // Append Hunk
void append(const std::string);     // Append std::string

void discard(size_t);               // Discard (beginning at origin)

void reset( void );                 // Reset the Data

size_t                              // The number of bytes stored
   store(                           // Store (copy) the Data
     BufferBorrow&     buff) const; // Into this BufferBorrow

size_t                              // The number of bytes stored
   store(                           // Store (copy) the Data
     void*             addr,        // Into this address
     size_t            size) const; // For this length
}; // class Data

//----------------------------------------------------------------------------
//
// Struct-
//       Hunk
//
// Purpose-
//       Data segment
//
//----------------------------------------------------------------------------
struct Hunk : public List<Hunk>::Link { // Data segment
char*                  addr;        // Data address
size_t                 size;        // Data length

   Hunk( void ) = default;          // Default constructor
   Hunk(                            // Constructor
     const void*       addr_,       // Source buffer*
     size_t            size_)       // Source buffer length
:  Link(), addr((char*)addr_), size(size_) {}

   ~Hunk( void ) = default;         // Destructor

//----------------------------------------------------------------------------
// Hunk::debug
//----------------------------------------------------------------------------
void debug(const char*) const;      // Debugging display
void debug( void ) const            // Debugging display
{  debug(""); }
}; // struct Hunk
}  // namespace http
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_HTTP_DATA_H_INCLUDED
