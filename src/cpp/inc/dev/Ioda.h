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
//       http/Ioda.h
//
// Purpose-
//       HTTP I/O data area.
//
// Last change date-
//       2022/10/19
//
// Implementation notes-
//       The I/O data area contains a scatter/gather I/O area used both as an
//       I/O buffer and for passing data between components.
//       It's intended to minimize overhead for these operations.
//
//       The uint32_t contains the combined length operator[] cache values.
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_HTTP_IODA_H_INCLUDED
#define _LIBPUB_HTTP_IODA_H_INCLUDED

#include <string>                   // For std::string
#include <stdint.h>                 // For uint32_t
#include <stdio.h>                  // For EOF
#include <sys/socket.h>             // For struct msghdr

#include <pub/List.h>               // For pub::List
#include "dev/bits/devconfig.h"     // For HTTP config controls

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
namespace http {
//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class IodaReader;

//============================================================================
//
// Class-
//       Ioda
//
// Purpose-
//       Input/Output Data Area.
//
// Implementation notes-
//       Ioda::Mesg is the struct msghdr to be used with recvmsg and sendmsg.
//       It handles all association storage allocation and release.
//
// Struct Mesg-
//       struct msghdr {
//           void         *msg_name;       // optional address
//           socklen_t     msg_namelen;    // size of address
//           struct iovec *msg_iov;        // scatter/gather array
//           size_t        msg_iovlen;     // # elements in msg_iov
//           void         *msg_control;    // ancillary data, see below
//           size_t        msg_controllen; // ancillary data buffer len
//           int           msg_flags;      // flags on received message
//       };
//
//       struct iovec {                    // Scatter/gather array items
//           void  *iov_base;              // Starting address
//           size_t iov_len;               // Number of bytes to transfer
//       };
//
//----------------------------------------------------------------------------
class Ioda {                        // Input/Output Data Area
friend class IodaReader;
//============================================================================
// Ioda::Mesg, struct msghdr wrapper with storage allocation control
//----------------------------------------------------------------------------
public:
struct Mesg : public msghdr {       // Wrapper for struct msghdr
   Mesg( void );                    // Default constructor
   Mesg(const Mesg&) = delete;;     // Copy constructor
   Mesg(Mesg&&);                    // Move constructor

   ~Mesg( void );                   // Destructor

//----------------------------------------------------------------------------
// Mesg::debug
//----------------------------------------------------------------------------
void debug(const char* info= "") const; // Debugging display

//----------------------------------------------------------------------------
// Mesg::Methods
//----------------------------------------------------------------------------
size_t size( void ) const;          // Get total data length
}; // struct Ioda::Mesg

//============================================================================
// Ioda::Page, address of I/O data page
//----------------------------------------------------------------------------
struct Page : public List<Page>::Link { // Ioda page list link
typedef uint32_t       Size;        // (Limited) size type

char*                  data;        // Data address
Size                   used;        // Number of bytes used

//----------------------------------------------------------------------------
// Page::debug
//----------------------------------------------------------------------------
void debug(const char* info= "") const; // Debugging display
}; // struct Ioda::Page

//============================================================================
// Ioda::Typedefs, enumerations, and constants
//----------------------------------------------------------------------------
typedef uint32_t       Size;        // (Limited) size type
typedef std::string    string;

static constexpr Size  LOG2_SIZE= 12;   // Log2(PAGE_SIZE)
static constexpr Size  PAGE_SIZE= 4096; // The Iota::Page data size

//----------------------------------------------------------------------------
// Ioda::Attributes
//----------------------------------------------------------------------------
protected:
List<Page>             list;        // Our list of Pages
size_t                 size= 0;     // The combined (available) size
size_t                 used= 0;     // The combined (used) size

//----------------------------------------------------------------------------
// Ioda::Contructors/Destructor
//----------------------------------------------------------------------------
public:
   Ioda( void );                    // Default constructor
   Ioda(const Ioda&) = delete;      // Copy constructor
   Ioda(Ioda&&);                    // Move constructor
   explicit Ioda(size_t);           // Construct with initial buffer

   ~Ioda( void );                   // Destructor

//----------------------------------------------------------------------------
// Ioda::Operators
//----------------------------------------------------------------------------
Ioda& operator=(const Ioda&) = delete; // (Copy) assignment
Ioda& operator=(Ioda&&);            // (Move) assignment

Ioda& operator+=(const Ioda&) = delete; // Append Ioda (copy)
Ioda& operator+=(Ioda&&);           // Append Ioda (move)
Ioda& operator+=(const string& S)   // Append std::string
{  write(S.c_str(), S.size()); return *this; }

explicit operator string( void ) const; // (Cast) std::string operator

//----------------------------------------------------------------------------
// Ioda::Accessor methods
//----------------------------------------------------------------------------
void debug(const char* info="") const; // Debugging display

void
   get_rd_mesg(                     // Get read Mesg
     Mesg&             msg,         // (Resultant)
     size_t            len);        // Of this (maximum) length

void
   get_wr_mesg(                     // Get write Mesg
     Mesg&             msg,         // (Resultant)
     size_t            len= 0,      // Of this (maximum) length
     size_t            off=0) const; // Starting at this offset

size_t get_used( void ) const       // Get used data length
{  return used; }

void set_used(size_t);              // Set the used data length

//----------------------------------------------------------------------------
// Ioda::I/O methods
//----------------------------------------------------------------------------
void put(int);                      // Write character

void put(const string& S)           // Write string
{  write(S.c_str(), S.size()); }

void reset( void );                 // Reset (empty) the Ioda
void reset(size_t);                 // Reset the Ioda as input buffer

void write(const void*, size_t);    // Write buffer

//----------------------------------------------------------------------------
// Ioda::Methods
//----------------------------------------------------------------------------
void discard(size_t offset)         // Discard leading data
{  Ioda ignore; split(ignore, offset); } // TODO: code separately
void split(Ioda& out, size_t offset); // Split leading data
}; // class Ioda

//============================================================================
//
// Class-
//       IodaReader
//
// Purpose-
//       Ioda data reader.
//
//----------------------------------------------------------------------------
class IodaReader {                  // Ioda data reader
//----------------------------------------------------------------------------
// IodaReader::Typedefs and enumerations
//----------------------------------------------------------------------------
public:
typedef Ioda::Size     Size;        // (Limited) data length
typedef std::string    string;

//----------------------------------------------------------------------------
// IodaReader::Attributes
//----------------------------------------------------------------------------
protected:
const Ioda&            ioda;        // The associated (const) Ioda
size_t                 offset= 0;   // The current offset

// operator[] cache
mutable Ioda::Page*    ix_page= nullptr; // The associated Page
mutable size_t         ix_off0= 0;  // The page origin's index

//----------------------------------------------------------------------------
// IodaReader::Constructors/Destructor
//----------------------------------------------------------------------------
public:
   IodaReader(const Ioda&);         // Constructor
   ~IodaReader( void );             // Destructor

//----------------------------------------------------------------------------
// IodaReader::Operators
//----------------------------------------------------------------------------
int
   operator[](size_t x) const       // Get character at offset
{  return index(x); }

//----------------------------------------------------------------------------
// IodaReader::Accessor methods
//----------------------------------------------------------------------------
int
   index(size_t) const;             // Get character at offset

size_t
   get_offset( void ) const         // Get offset
{  return offset; }

void
   set_offset(size_t o)             // Set offset
{  offset= o; }

//----------------------------------------------------------------------------
// IodaReader::Methods
//----------------------------------------------------------------------------
int
   bksp( void );                    // Get the previous character

int
   get( void );                     // Get the next character

string
   get_line( void );                // Get the next line

string
   get_token(string delim);         // Get the next token

int
   peek( void ) const;              // Examine the next character

void
   reset( void )                    // Reset the IodaReader for re-use
{  offset= 0; ix_page= nullptr; ix_off0= 0; }
}; // class IodaReader
}  // namespace http
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_HTTP_IODA_H_INCLUDED
