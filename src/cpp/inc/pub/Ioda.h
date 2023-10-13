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
//       2022/09/27
//
// Implementation notes-
//       The I/O data area contains a scatter/gather I/O area used both as an
//       I/O buffer and to pass data between components, minimizing overhead
//       for these operations.
//
// Implementation notes (copying)-
//       In order to avoid accidental copying, the Ioda copy constructor, copy
//       assignment operator and copy append operator (+=) have been deleted.
//
//       The (**HIGH-OVERHEAD**) std::string cast alternative can be used as
//       a direct replacement for these methods. The append and copy methods
//       are high (but lower) overhead indirect method replacements.
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
//       For an Ioda::Buffer: Ioda::size != 0; Ioda::used == 0)
//         Ioda::size is the maximum size of the input buffer.
//       For an Ioda::Writer: Ioda::size == 0; Ioda::used >= 0)
//         Ioda::used is the size of the output buffer.
//         An output Ioda may use append methods; an Ioda::Buffer cannot.
//       The default constructor creates a zero length Ioda::Writer.
//       Ioda::set_used(size) truncates an Ioda::Buffer, converting it into
//          an Ioda::Writer.
//       Ioda::reset(size) resets any Ioda into an Ioda::Buffer.
//       Ioda::reset() resets any Ioda into the (default) empty Ioda::Writer.
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
public:
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - -- - - - - - - - -
// Ioda::Mesg, struct msghdr wrapper with storage allocation control
//----------------------------------------------------------------------------
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
}; // struct Ioda::Mesg - - - - - - - - - - - - -- - - - - - - - - - - - - - -

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - -- - - - - - - - -
// Ioda::Page, address of I/O data page
//----------------------------------------------------------------------------
struct Page : public List<Page>::Link { // Ioda page list link
char*                  data;        // Data address
size_t                 used;        // Number of bytes used

//----------------------------------------------------------------------------
// Page::debug
//----------------------------------------------------------------------------
void debug(const char* info= "") const; // Debugging display
}; // struct Ioda::Page - - - - - - - - - - - - -- - - - - - - - - - - - - - -

//============================================================================
// Ioda::Typedefs, enumerations, and constants
//----------------------------------------------------------------------------
typedef std::string    string;      // Import std::string
typedef Ioda           Buffer;      // Ioda used as an input Buffer
typedef IodaReader     Reader;      // IodaReader type
typedef Ioda           Writer;      // Ioda used as an output Writer

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

   Ioda(const Ioda& copy) = delete; // *NO* copy(const Ioda&) constructor
   explicit
   Ioda(const std::string& copy);   // Ioda(const Ioda&) alternative

   Ioda(Ioda&&);                    // Move constructor

   explicit
   Ioda(size_t);                    // Construct as input buffer

   ~Ioda( void );                   // Destructor

//----------------------------------------------------------------------------
// Ioda::Operators
//----------------------------------------------------------------------------
Ioda&
   operator=(const Ioda&) = delete; // *NO* copy(const Ioda&) assignment
Ioda&
   operator=(const std::string& copy); // (std::string)assignment alternative,
                                    // **HIGH-OVERHEAD** if (string) cast used

Ioda&
   operator=(Ioda&&);               // Move assignment

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Ioda&
   operator+=(const Ioda&) = delete; // *NO* const Ioda& append
Ioda&
   operator+=(const string& S)      // operator+=(const Ioda&) alternative
{  put(S); return *this; }          // **HIGH-OVERHEAD** if (string) cast used

Ioda&
   operator+=(Ioda&& from);         // Move append Ioda

//************************ HIGH-OVERHEAD OPERATIONS **************************
bool
   operator==(const Ioda& S)        // Equality comparison
{  return (string)*this == (string)S; } // **HIGH-OVERHEAD** casts

bool
   operator!=(const Ioda& S)        // Inequality comparison
{  return (string)*this != (string)S; } // **HIGH-OVERHEAD** casts

explicit
   operator string( void ) const;   // **HIGH-OVERHEAD** operation

//----------------------------------------------------------------------------
// Ioda::Accessor methods
//----------------------------------------------------------------------------
void debug(const char* info="") const; // Debugging display
void dump(const char* info="") const; // Debugging dump

bool
   is_buffer( void ) const
{  return size != 0; }

bool
   is_reader( void ) const
{  return false; }

bool
   is_writer( void ) const
{  return size == 0; }

size_t
   get_size( void ) const           // Get maximum input data length
{  return size; }

size_t
   get_used( void ) const           // Get current output data length
{  return used; }

// Convert a read Ioda into an write Ioda, truncating it.
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
void                                // ** DATA COPYING REQUIRED **
   append(const Ioda&);             // Append Ioda

void                                // ** DATA COPYING REQUIRED **
   copy(const Ioda&);               // Copy Ioda, replacing any content.

void
   discard(size_t);                 // Discard leading data

void
   move(Ioda&&);                    // Move Ioda, replacing any content.

void
   reset( void );                   // Reset (empty) the Ioda

void                                // Reset the Ioda, converting it an input
   reset(size_t);                   // Ioda of the specified size

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
// Implementation notes-
//       Ioda::used is the IodaReader's data length. (Since Ioda::used == 0
//       in a read Ioda, the associated Ioda should be a write Ioda.)
//
//----------------------------------------------------------------------------
class IodaReader {                  // Ioda data reader
//----------------------------------------------------------------------------
// IodaReader::Typedefs and enumerations
//----------------------------------------------------------------------------
public:
typedef std::string    string;
typedef IodaReader     Reader;      // IodaReader type (alias)

//----------------------------------------------------------------------------
// IodaReader::Attributes
//----------------------------------------------------------------------------
protected:
const Ioda::Writer&    ioda;        // The associated (const) Ioda Writer
size_t                 offset= 0;   // The current offset

// operator[] cache
mutable Ioda::Page*    ix_page= nullptr; // The associated Page
mutable size_t         ix_off0= 0;  // The page origin's index

//----------------------------------------------------------------------------
// IodaReader::Constructors/Destructor
//----------------------------------------------------------------------------
public:
   IodaReader(const Ioda::Writer&); // Constructor

   IodaReader( void ) = delete;     // *NO* default constructor
   IodaReader(const IodaReader&) = delete; // *NO* copy constructor
   IodaReader(IodaReader&&) = delete; // *NO* move constructor

   ~IodaReader( void );             // Destructor

//----------------------------------------------------------------------------
// IodaReader::Operators
//----------------------------------------------------------------------------
IodaReader& operator=(const IodaReader&) = delete; // *NO* copy assignment
IodaReader& operator=(const IodaReader&&) = delete; // *NO* move assignment

int
   operator[](size_t x) const       // Get character at offset
{  return index(x); }

//----------------------------------------------------------------------------
// IodaReader::Accessor methods
//----------------------------------------------------------------------------
void debug(const char* info="") const; // Debugging display

void dump(const char* info="") const; // Debugging dump

int
   index(size_t) const;             // Get character at offset

bool
   is_buffer( void ) const
{  return false; }

// If Ioda is_buffer() is true, reader.get_used() returns zero, the same as
// an empty Ioda::Writer.
bool
   is_reader( void ) const
{  return true; }

bool
   is_writer( void ) const
{  return false; }

size_t
   get_length( void ) const         // Get remaining length
{  return ioda.get_used() - offset; }

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
