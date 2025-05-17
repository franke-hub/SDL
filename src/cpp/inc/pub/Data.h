//----------------------------------------------------------------------------
//
//       Copyright (c) 2020-2025 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
// SPDX-License-Identifier: LGPL-3.0-only
//----------------------------------------------------------------------------
//
// Title-
//       Data.h
//
// Purpose-
//       File management classes, conveniently packaged in one file.
//
// Last change date-
//       2025/01/25
//
// Implementation note-
//       Derived from Fileman.h
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_DATA_H_INCLUDED
#define _LIBPUB_DATA_H_INCLUDED

#include <string>                   // For std::string

#include <sys/stat.h>               // For struct stat

#include "pub/bits/pubconfig.h"     // For _LIBPUB_ macros
#include <pub/List.h>               // For pub::DHDL_list, ...

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
namespace data {
//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class Line;
class Pool;

//----------------------------------------------------------------------------
//
// Class-
//       Data
//
// Purpose-
//       File data container.
//
//----------------------------------------------------------------------------
class Data {                        // File data container
//----------------------------------------------------------------------------
// Data::Attributes
//----------------------------------------------------------------------------
protected:
std::string            _path;       // The (locally qualified) path name
std::string            _file;       // The file name
DHDL_list<Line>        _line;       // The Line list
DHDL_list<Pool>        _pool;       // The Pool list

bool                   _changed;    // File is changed
bool                   _damaged;    // File is damaged

//----------------------------------------------------------------------------
// Data::Constructors/destructor
//----------------------------------------------------------------------------
public:
   Data( void );                    // Default constructor

   Data(                            // Constructor
     const std::string&_path,       // The (locally qualified) path name
     const std::string&_file);      // The File information

   ~Data( void );                   // Destructor

//----------------------------------------------------------------------------
// Data::debug | Display debugging information
//----------------------------------------------------------------------------
void
   debug(const char*   info= "") const; // Display debugging information

//----------------------------------------------------------------------------
// Data::Accessors
//----------------------------------------------------------------------------
void
   change(                          // Set changed state
     bool              state = true) // To this state
{  _changed= state; }

bool                                // The changed state
   changed( void )                  // Get changed state
{  return _changed; }

bool                                // The damaged state
   damaged( void )                  // Get damaged state
{  return _damaged; }

std::string                         // The file name (without the path)
   file( void )                     // Get file name (without the path)
{  return _file; }

std::string                         // The path/file name
   full( void )                     // Get path/file name
{  return _path + "/" + _file; }

DHDL_list<Line>&                    // The Line list
   line( void )                     // Get Line list
{  return _line; }

std::string                         // The path name (without the file)
   path( void )                     // Get path name (without the file)
{  return _path; }

//----------------------------------------------------------------------------
// Data::Methods
//----------------------------------------------------------------------------
void
   close( void );                   // Close (empty) the Data

Line*                               // Line*, throws std::bad_alloc iff failure
   get_line(                        // Allocate a new line Line and
     const std::string&_string);    // Initialize it with this string

int                                 // Return code, 0 OK
   open(                            // (Re)load data
     const std::string&_path,       // The path name
     const std::string&_file);      // The file name

int                                 // Return code, 0 OK
   write(                           // Write data
     const std::string&path,        // The (locally qualified) path name
     const std::string&file) const; // The file name

int                                 // Return code, 0 OK
   write( void ) const              // Replace file
{  return write(_path, _file); }
}; // class Data

//----------------------------------------------------------------------------
//
// Struct-
//       File
//
// Purpose-
//       File information
//
//----------------------------------------------------------------------------
struct File : public List<File>::Link { // File information
//----------------------------------------------------------------------------
// File::Typedefs and enumerations
//----------------------------------------------------------------------------
typedef struct stat    stat_t;      // struct stat type

//----------------------------------------------------------------------------
// File::Attributes
//----------------------------------------------------------------------------
const std::string      name;        // The file name (Does not include Path)
const stat_t           st;          // The lstat info

//----------------------------------------------------------------------------
// File::Constructor/Destructor
//----------------------------------------------------------------------------
   File(                            // Constructor
     const stat_t&     _st,         // Stat descriptor
     const std::string&_name)       // File name
:  name(_name), st(_st) {}

   ~File( void ) = default;         // Destructor
}; // struct File

//----------------------------------------------------------------------------
//
// Struct-
//       Line
//
// Purpose-
//       An immutable file line.
//
// Implementation notes-
//       Text is allocated from a Pool but is neither allocated nor released
//       by the Line object.
//
//----------------------------------------------------------------------------
struct Line : public DHDL_list<Line>::Link { // File line
//----------------------------------------------------------------------------
// Line::Attributes
//----------------------------------------------------------------------------
const char*            text;        // The associated text

//----------------------------------------------------------------------------
// Line::Constructors/destructor
//----------------------------------------------------------------------------
   Line(                            // Constructor
     const char*       _text)       // The associated text
:  text(_text) {}

   ~Line( void ) = default;
}; // struct Line

//----------------------------------------------------------------------------
//
// Struct-
//       Name
//
// Purpose-
//       File name information
//
//----------------------------------------------------------------------------
struct Name {                       // File name information
//----------------------------------------------------------------------------
// Name::Attributes
//----------------------------------------------------------------------------
typedef struct stat    stat_t;      // struct stat type

stat_t                 st;          // The lstat info
std::string            name;        // The (locally qualified) file name
std::string            file_name;   // The file name (without path_name)
std::string            path_name;   // The path name (without file_name)

//----------------------------------------------------------------------------
// Name::Constructor/Destructor
//----------------------------------------------------------------------------
   Name(                            // Constructor
     std::string       full_name);  // The file name

   ~Name( void ) = default;         // Destructor

//----------------------------------------------------------------------------
//
// Method-
//       Name::get_file_name
//       Name::get_path_name
//
// Purpose-
//       Get file name part of (locally qualified) file name.
//       Get path name part of (locally qualified) file name.
//
//----------------------------------------------------------------------------
static std::string                  // The file part of (relative) full_name
   get_file_name(                   // Get file part of
     std::string       full_name);  // This relative full name

static std::string                  // The path part of (relative) full_name
   get_path_name(                   // Get path part of
     std::string       full_name);  // This relative full name

//----------------------------------------------------------------------------
//
// Method-
//       Name::reset
//
// Purpose-
//       Reset the file name
//
//----------------------------------------------------------------------------
void
   reset(                           // Reset the file name
     std::string       full_name);  // The file name

//----------------------------------------------------------------------------
//
// Method-
//       Name::resolve
//
// Purpose-
//       Resolve links, converting file_part, path_part, and name.
//
// Implementation note-
//       File path components of "/../" are explicitly allowed, and
//       a beginning path component of "/.." is also allowed.
//
//----------------------------------------------------------------------------
std::string                         // The invalid path ("" if succesful)
   resolve( void );                 // Resolve links
}; // struct Name

//----------------------------------------------------------------------------
//
// Struct-
//       Path
//
// Purpose-
//       Path name information
//
//----------------------------------------------------------------------------
struct Path {                       // Path name information
//----------------------------------------------------------------------------
// Path::Attributes
//----------------------------------------------------------------------------
const std::string      name;        // The path name (Locally qualified)
List<File>             list;        // The (sorted) list of Files

//----------------------------------------------------------------------------
// Path::Constructors
//----------------------------------------------------------------------------
   Path(                            // Constructor
     const std::string&_name);      // Path name (Locally qualified)

   ~Path( void );                   // Destructor
}; // class Struct

//----------------------------------------------------------------------------
//
// Class-
//       Pool
//
// Purpose-
//       A storage Pool fragment
//
// Implementation notes-
//       Storage is allocated from a Pool and never released until the Pool
//       is deleted.
//
//----------------------------------------------------------------------------
class Pool : public DHDL_list<Pool>::Link { // Storage Pool fragment
//----------------------------------------------------------------------------
// Pool::Attributes
//----------------------------------------------------------------------------
protected:
size_t                 used;        // Number of bytes used
size_t                 size;        // The total Pool size
char*                  data;        // The Pool data area

//----------------------------------------------------------------------------
// Pool::Constructor/destructor
//----------------------------------------------------------------------------
public:
   Pool(                            // Constructor
     size_t            _size);      // The allocation size

   ~Pool( void );                   // Destructor

//----------------------------------------------------------------------------
// Pool::debug | Display debugging information
//----------------------------------------------------------------------------
void
   debug(const char*   info= "") const; // Display debugging information

//----------------------------------------------------------------------------
// Pool::malloc | Allocate text from Pool
//----------------------------------------------------------------------------
char*                               // Allocated storage (or nullptr)
   malloc(                          // Allocate storage
     size_t            _size);      // The required length
}; // class Pool
}  // namespace fileman
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_DATA_H_INCLUDED
