//----------------------------------------------------------------------------
//
//       Copyright (c) 2010-2020 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       DataSource.h
//
// Purpose-
//       Defines an input data source.
//
// Last change date-
//       2020/10/02
//
//----------------------------------------------------------------------------
#ifndef DATASOURCE_H_INCLUDED
#define DATASOURCE_H_INCLUDED

#include <string>                   // For std::string
#include <stddef.h>                 // For size_t
#include <stdint.h>                 // For integer types

//----------------------------------------------------------------------------
//
// Class-
//       DataSource
//
// Purpose-
//       Define input data source.
//
// Implementation notes-
//       Data width:
//            0 (Treated as 1)
//           -4 (UTF-32, endian reversal required)
//           -2 (UTF-16, endian reversal required)
//            1 (UTF-8)
//            2 (UTF-16, HOST endian)
//            4 (UTF-32, HOST endian)
//
//       Implementations may not know the data length. In this case
//       getLength() returns 0. (Same as an empty DataSource.)
//
//       Not all implementations support the clone() method. In this case
//       clone() returns NULL;
//
//----------------------------------------------------------------------------
class DataSource {
//----------------------------------------------------------------------------
// DataSource::Attributes
//----------------------------------------------------------------------------
protected:
unsigned char*         origin;      // Data origin
size_t                 offset;      // Data offset
size_t                 length;      // Data length
int                    width;       // Data width

std::string            name;        // Source name
unsigned long          line;        // Line number
unsigned int           column;      // Column number

//----------------------------------------------------------------------------
// DataSource::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
enum CC                             // Character code
{  CC_EOF= (-1)                     // (End of file)
,  CC_LTL= (-2)                     // (Error, getLine(), line too long)
,  CC_ERR= (-3)                     // (Error, generic)
}; // enum CC

//----------------------------------------------------------------------------
// DataSource::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~DataSource( void );             // Destructor

   DataSource( void );              // Default constructor

   DataSource(                      // Constructor
     const char*       name,        // Source name
     void*             origin,      // Source address
     size_t            length);     // Source length

   DataSource(                      // Copy constructor
     const DataSource& source);     // Source DataSource

//----------------------------------------------------------------------------
// DataSource::Operators
//----------------------------------------------------------------------------
public:
DataSource&                         // (Always *this)
   operator=(                       // Assignment operator
     const DataSource& source);     // Source DataSource

//----------------------------------------------------------------------------
// DataSource::Accessors
//----------------------------------------------------------------------------
public:
inline const char*                  // C-String
   getCName( void ) const           // Get (c-string) name
{
   return name.c_str();
}

inline size_t                       // The data length
   getLength( void ) const          // Get data length
{
   return length;
}

inline std::string                  // The source name
   getName( void ) const            // Get source name
{
   return name;
}

inline size_t                       // The data offset (in bytes)
   getOffset( void ) const          // Get data offset
{
   return offset;
}

inline unsigned int                 // The text width (if known)
   getWidth( void ) const           // Get text width
{
   if( width < 0 )
     return -width;

   return width;
}

inline unsigned int                 // The column number
   getColumn( void ) const          // Get column number (1 origin)
{
   return column + 1;
}

inline unsigned long                // The line number
   getLine( void ) const            // Get line number (1 origin)
{
   return line + 1;
}

virtual int                         // Return code (0 OK)
   setOffset(                       // Set data offset
     size_t            offset);     // To this value

public:
//----------------------------------------------------------------------------
//
// Method-
//       clone
//
// Purpose-
//       Duplicate this DataSource
//
// Implementation notes-
//       Implementations that do not support this method return NULL.
//       Derived classes may return a simple DataSource base object.
//
//----------------------------------------------------------------------------
virtual DataSource*                 // -> DataSource (or NULL)
   clone(                           // Clone this DataSource
     const char*       name) const; // With this (relative) name

//----------------------------------------------------------------------------
//
// Method-
//       get
//
// Purpose-
//       Get the next data character, accounting for width.
//
//----------------------------------------------------------------------------
virtual int                         // The next data "character"
   get( void );                     // Get next data "character"

//----------------------------------------------------------------------------
//
// Method-
//       getLine
//
// Purpose-
//       Get the next line of data characters, accounting for width.
//
// Implementation notes-
//       The entire line is read. Extra characters are discarded.
//
//----------------------------------------------------------------------------
virtual int                         // The line delimiter
   getLine(                         // Get next data line (ignoring '\r')
     void*             addr,        // Data address
     unsigned          size);       // Data length (in bytes)

//----------------------------------------------------------------------------
//
// Method-
//       read
//
// Purpose-
//       Read bytes from the data source, ignoring width.
//
// Implementation notes-
//       When mixing the get/getLine and read methods, the read method
//       should read a multiple of width bytes.
//
//----------------------------------------------------------------------------
virtual unsigned int                // The number of bytes read
   read(                            // Read
     void*             addr,        // Data address
     unsigned int      size);       // Data length (in bytes)

//----------------------------------------------------------------------------
//
// Method-
//       reset
//
// Purpose-
//       Reset (close) the DataSource
//
// Implementation notes-
//       This empties the DataSource.
//
//----------------------------------------------------------------------------
virtual void
   reset( void );                   // Reset (close) the DataSource

//----------------------------------------------------------------------------
// DataSource::Static Methods
//----------------------------------------------------------------------------
public:
static int                          // The inverted data "character"
   invert16(                        // Invert endian
     unsigned int      C);          // Of this UTF-16 data character

static int                          // The inverted data "character"
   invert32(                        // Invert endian
     unsigned int      C);          // Of this UTF-32 data character

protected:
virtual void
   setWidth(                        // Set the data width
     const unsigned char*
                       origin,      // From this origin
     unsigned int      length);     // And this length

inline void
   setWidth(                        // Set the data width
     const unsigned char*
                       origin,      // From this origin
     size_t            length)      // And this length
{
   unsigned int size= 4;
   size += (length&3);
   if( size > length )
     size= length;

   setWidth(origin, size);
}

inline void
   setWidth( void )                 // Set the data width
{
   unsigned int length= 4;
   length += (this->length&3);

   if( length > this->length )
     length= this->length;

   setWidth(origin, length);
}
}; // class DataSource

#endif // DATASOURCE_H_INCLUDED
