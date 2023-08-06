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
//       Ioda.h
//
// Purpose-
//       I/O Data Area.
//
// Last change date-
//       2022/07/30
//
// Implementation notes-
//       The I/O data area contains a scatter/gather I/O area used both as an
//       I/O buffer and to pass data between components, minimizing overhead
//       for these operations.
//
//       In order to avoid accidental copying, the Ioda copy constructor, copy
//       assignment operator and copy append operator (+=) have been deleted.
//       Use the copy (assignment) method where copying rather than moving is
//       intended.
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_IODA_H_INCLUDED
#define _LIBPUB_IODA_H_INCLUDED

#include <string>                   // For std::string
#include <stdint.h>                 // For uint32_t
#include <stdio.h>                  // For EOF
#include <sys/socket.h>             // For struct msghdr

#include <pub/List.h>               // For pub::List

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
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
enum {
// LOG2_SIZE= 12                    // Log2(PAGE_SIZE)
// PAGE_SIZE= 4096                  // The (constant) data size
}; // PAGE_SIZE constants

char*                  data;        // Data address
size_t                 used;        // Number of bytes used

//----------------------------------------------------------------------------
// Page::debug
//----------------------------------------------------------------------------
void debug(const char* info= "") const; // Debugging display
}; // struct Ioda::Page

//============================================================================
// Ioda::Typedefs, enumerations, and constants
//----------------------------------------------------------------------------
typedef std::string    string;

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
Ioda&
   operator=(const Ioda&) = delete; // (Copy) assignment
Ioda&
   operator=(Ioda&&);               // (Move) assignment

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Ioda&
   operator+=(const Ioda&) = delete; // Append Ioda (copy)
Ioda&
   operator+=(Ioda&&);              // Append Ioda (move)
Ioda&
   operator+=(const string& S)      // Append std::string
{  write(S.c_str(), S.size()); return *this; }

explicit
   operator string( void ) const;   // (std::string) cast operator

//----------------------------------------------------------------------------
// Ioda::Accessor methods
//----------------------------------------------------------------------------
void debug(const char* info="") const; // Debugging display

size_t
   get_used( void ) const           // Get used data length
{  return used; }

// set_used converts a read Ioda into an write Ioda, truncating it.
// (Write Ioda::size == 0; Ioda::used == total used data length)
void
   set_used(size_t);                // Set the used data length

void
   set_rd_mesg(                     // Set read Mesg
     Mesg&             msg,         // (Resultant)
     size_t            len);        // Of this (maximum) length

void
   set_wr_mesg(                     // Set write Mesg
     Mesg&             msg,         // (Resultant)
     size_t            len= 0,      // Of this (maximum) length
     size_t            off=0) const; // Starting at this offset

//----------------------------------------------------------------------------
// Ioda::I/O methods
//----------------------------------------------------------------------------
void
   put(int);                        // Write character

void
   put(const string& S)             // Write string
{  write(S.c_str(), S.size()); }

void
   write(const void*, size_t);      // Write buffer

//----------------------------------------------------------------------------
// Ioda::Methods
//----------------------------------------------------------------------------
void
   copy(const Ioda&);               // Copy Ioda, replacing any content.

void
   discard(size_t);                 // Discard leading data

void
   move(Ioda&&);                    // Move Ioda, replacing any content.

void
   reset( void );                   // Reset (empty) the Ioda

void                                // Reset the Ioda, converting it an input
   reset(size_t);                   // buffer of the specified size

void                                // (*this contains the trailing data)
   split(                           // Split leading data
     Ioda&           result,        // The leading data resultant
     size_t          size);         // Split at length
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
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_IODA_H_INCLUDED
