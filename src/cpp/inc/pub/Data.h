//----------------------------------------------------------------------------
//
//       Copyright (c) 2020-2026 Frank Eskesen.
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
//       2026/07/01
//
// Implementation note-
//       Derived from Fileman.h
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_DATA_H_INCLUDED
#define _LIBPUB_DATA_H_INCLUDED

#include <string>                   // For std::string

#include <sys/stat.h>               // For struct stat

#include "pub/List.h"               // For pub::DHDL_list, ...

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
namespace data {
//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class Line;
struct Path;
class Pool;

//============================================================================
//
// Class-
//       pub::data::Data
//
// Purpose-
//       File data container.
//
//----------------------------------------------------------------------------
class Data {                        // File data container
//----------------------------------------------------------------------------
// pub::data::Data::Typedefs and enumerations
//----------------------------------------------------------------------------
public:
typedef std::string    string;      // For convenience

//----------------------------------------------------------------------------
// Data::Attributes
//----------------------------------------------------------------------------
protected:
string                 path_name;   // The (locally qualified) path name
string                 file_name;   // The file name
DHDL_list<Line>        line_list;   // The Line list
DHDL_list<Pool>        pool_list;   // The Pool list

bool                   is_changed= false; // File is changed
bool                   is_damaged= false; // File is damaged

//----------------------------------------------------------------------------
// pub::data::Data::Constructors/destructor
//----------------------------------------------------------------------------
public:
   Data( void );                    // Default constructor

   Data(                            // Constructor
     const string&     path,        // The (locally qualified) path name
     const string&     file);       // The File information

   ~Data( void );                   // Destructor

//----------------------------------------------------------------------------
// pub::data::Data::debug | Display debugging information
//----------------------------------------------------------------------------
void
   debug(const char*   info= "") const; // Display debugging information

//----------------------------------------------------------------------------
// pub::data::Data::Accessors
//----------------------------------------------------------------------------
void
   change(                          // Set changed state
     bool              state = true) // To this state
{  is_changed= state; }

bool                                // The changed state
   changed( void ) const            // Get changed state
{  return is_changed; }

bool                                // The damaged state
   damaged( void ) const            // Get damaged state
{  return is_damaged; }

string                              // The file name (without the path)
   file( void ) const               // Get file name (without the path)
{  return file_name; }

string                              // The (relative) fully qualified name
   full( void ) const;              // Get (relative) fully qualified name

DHDL_list<Line>&                    // The Line list
   line( void )                     // Get Line list
{  return line_list; }

string                              // The path name (without the file)
   path( void ) const               // Get path name (without the file)
{  return path_name; }

//----------------------------------------------------------------------------
// pub::data::Data::Methods
//----------------------------------------------------------------------------
void
   close( void );                   // Close (empty) the Data

Line*                               // Line*, throws std::bad_alloc iff failure
   get_line(                        // Allocate a new line Line and
     const string&     _string);    // Initialize it with this string

int                                 // Return code, 0 OK
   open(                            // (Re)load data
     const string&     path,        // The path name
     const string&     file);       // The file name

int                                 // Return code, 0 OK
   write(                           // Write data
     const string&     path,        // The (locally qualified) path name
     const string&     file) const; // The file name

int                                 // Return code, 0 OK
   write( void ) const              // Replace file
{  return write(path_name, file_name); }
}; // class Data

//============================================================================
//
// Struct-
//       pub::data::File
//
// Purpose-
//       File information
//
//----------------------------------------------------------------------------
struct File : public DHDL_sort<File>::Link { // File information
//----------------------------------------------------------------------------
// pub::data::File::Typedefs and enumerations
//----------------------------------------------------------------------------
typedef struct stat    stat_t;      // struct stat type
typedef std::string    string;      // For convenience

//----------------------------------------------------------------------------
// pub::data::File::Attributes
//----------------------------------------------------------------------------
string                 file_name;   // The file name (Does not include Path)
stat_t                 st;          // The lstat info

//----------------------------------------------------------------------------
// pub::data::File::Constructor/destructor
//----------------------------------------------------------------------------
   File(                            // Constructor
     const string&     name)        // File name
:  file_name(name), st(make_st(name))
{  }

   File(                            // Constructor
     const stat_t&     _st,         // Stat descriptor
     const string&     name)        // File name
:  file_name(name), st(_st)
{  }

virtual
   ~File( void ) = default;         // Destructor

//----------------------------------------------------------------------------
// pub::data::File::make_st (Constructor helper)
//----------------------------------------------------------------------------
static stat_t                       // The stat_t
   make_st(                         // Create a stat_t
     string            name);       // From this file name string

//----------------------------------------------------------------------------
// pub::data::File::debug | Debugging display
//----------------------------------------------------------------------------
virtual void
   debug(const char*   info= "") const;   // Debugging display

//----------------------------------------------------------------------------
//
// Method-
//       pub::data::File::get_file_name
//
// Purpose-
//       Get file name. (Does not include path name)
//
//----------------------------------------------------------------------------
string
   get_file_name( void ) const      // The file part of this file_name
{  return file_name; }

//----------------------------------------------------------------------------
// pub::data::File::operator <
//----------------------------------------------------------------------------
protected:
virtual bool operator<(const Base& _that) const override
{  const File* that= static_cast<const File*>(&_that);
   return file_name < that->file_name;
}
}; // struct File

//============================================================================
//
// Struct-
//       pub::data::Line
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
// pub::data::Line::Attributes
//----------------------------------------------------------------------------
const char*            text;        // The associated text

//----------------------------------------------------------------------------
// pub::data::Line::Constructors/destructor
//----------------------------------------------------------------------------
   Line(                            // Constructor
     const char*       text)        // The associated text
:  text(text)
{  }

   ~Line( void ) = default;
}; // struct Line

//============================================================================
//
// Struct-
//       pub::data::Name
//
// Purpose-
//       File name information
//
//----------------------------------------------------------------------------
struct Name {                       // File name information
//----------------------------------------------------------------------------
// pub::data::Name::Typedefs and enumerations
//----------------------------------------------------------------------------
typedef struct stat    stat_t;      // struct stat type
typedef std::string    string;      // For convenience

//----------------------------------------------------------------------------
// pub::data::Name::Attributes
//----------------------------------------------------------------------------
stat_t                 st;          // The lstat info
string                 full_name;   // The (locally qualified) full name
string                 file_name;   // The file name (without path_name)
string                 path_name;   // The path name (without file_name)

//----------------------------------------------------------------------------
// pub::data::Name::Constructor/destructor
//----------------------------------------------------------------------------
   Name(                            // Constructor
     const string&     full);       // The (locally qualified) full name

   Name(                            // Constructor
     const string&     path,        // The (locally qualified) path name
     const string&     file);       // The file name

   ~Name( void ) = default;         // Destructor

//----------------------------------------------------------------------------
//
// Method-
//       pub::data::Name::get_extension
//       pub::data::Name::get_file_name
//       pub::data::Name::get_full_name
//       pub::data::Name::get_path_name
//
// Purpose-
//       Get extension part of (locally qualified) file name.
//       Get file name part of (locally qualified) file name.
//       Get full name from path name and file name.
//       Get path name part of (locally qualified) file name.
//
//----------------------------------------------------------------------------
static string                       // The extension part of file_name
   get_extension(                   // Get file part of
     const string&     file_name);  // This relative file name

string
   get_extension( void ) const      // The extension part of this file_name
{  return get_extension(file_name); }

static string                       // The file part of (relative) full_name
   get_file_name(                   // Get file part of
     const string&     full_name);  // This relative full name

string
   get_file_name( void ) const      // The file part of this file_name
{  return file_name; }

static string                       // The (relative) fully qualified name
   get_full_name(                   // Get (relative) fully qualified name
     const string&     path,        // From this relative path name and
     const string&     file);       // This file name

string
   get_full_name( void ) const      // The (relative) fully qualified name
{  return full_name; }

static string                       // The path part of (relative) full_name
   get_path_name(                   // Get path part of
     const string&     full_name);  // This relative full name

string
   get_path_name( void ) const      // The file part of this file_name
{  return path_name; }

//----------------------------------------------------------------------------
//
// Method-
//       pub::data::Name::reset
//
// Purpose-
//       Reset the file name
//
//----------------------------------------------------------------------------
void
   reset(                           // Reset the file name
     const string&     full_name);  // The file name

void
   reset(                           // Reset the file name
     const string&     path_name,   // The (locally qualified) path name
     const string&     file_name);  // The file name

//----------------------------------------------------------------------------
//
// Method-
//       pub::data::Name::resolve
//
// Purpose-
//       Resolve links, converting, path_part, and file_part of name.
//
// Implementation note-
//       File path components of "/../" are explicitly allowed, and
//       a beginning path component of "/.." is also allowed.
//
//----------------------------------------------------------------------------
string                              // The invalid path ("" if succesful)
   resolve( void );                 // Resolve links
}; // struct Name

//============================================================================
//
// Struct-
//       pub::data::Path
//
// Purpose-
//       Path name information
//
//----------------------------------------------------------------------------
struct Path {                       // Path name information
//----------------------------------------------------------------------------
// pub::data::Path::Typedefs and enumerations
//----------------------------------------------------------------------------
typedef struct stat    stat_t;      // struct stat type
typedef std::string    string;      // For convenience

//----------------------------------------------------------------------------
// pub::data::Path::Attributes
//----------------------------------------------------------------------------
string                 path_name;   // The path name (Locally qualified)
DHDL_sort<File>        list;        // The (sortable) list of Files

//----------------------------------------------------------------------------
// pub::data::Path::Constructors/destructor
//----------------------------------------------------------------------------
   Path(                            // Constructor
     const char*       name= nullptr); // (Optional) path name

   Path(                            // Constructor
     const string&     name);       // Path name (Locally qualified)

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
virtual
   ~Path( void );                   // Destructor (Resets the List)

//----------------------------------------------------------------------------
// pub::data::Path::debug | Debugging display
//----------------------------------------------------------------------------
virtual void
   debug(const char*   info= "") const; // Debugging display

//----------------------------------------------------------------------------
// pub::data::Path::insert | Add file to List
//----------------------------------------------------------------------------
void
   insert(                          // Insert onto the List
     File*             file);       // This File

//----------------------------------------------------------------------------
// pub::data::Path::reset | Reset: Reload the directory
//----------------------------------------------------------------------------
void
   reset(                           // Reset and load the directory
     const char*       name= nullptr); // Path name (Locally qualified)

void
   reset(                           // Reset and load the directory
     const string&     name);       // Path name (Locally qualified)

//----------------------------------------------------------------------------
// pub::data::Path::make_file | Create a new File
//----------------------------------------------------------------------------
virtual File*                       // The created File
   make_file(                       // Create a new File
     const stat_t&     st,          // File information
     const string&     file_name) const; // The File name
}; // struct Path

//============================================================================
//
// Class-
//       pub::data::Pool
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
// pub::data::Pool::Attributes
//----------------------------------------------------------------------------
protected:
size_t                 used;        // Number of bytes used
size_t                 size;        // The total Pool size
char*                  data;        // The Pool data area

//----------------------------------------------------------------------------
// pub::data::Pool::Constructor/destructor
//----------------------------------------------------------------------------
public:
   Pool(                            // Constructor
     size_t            _size);      // The allocation size

   ~Pool( void );                   // destructor

//----------------------------------------------------------------------------
// pub::data::Pool::debug | Display debugging information
//----------------------------------------------------------------------------
void
   debug(const char*   info= "") const; // Display debugging information

//----------------------------------------------------------------------------
// pub::data::Pool::malloc | Allocate text from Pool
//----------------------------------------------------------------------------
char*                               // Allocated storage (or nullptr)
   malloc(                          // Allocate storage
     size_t            _size);      // The required length
}; // class Pool
}  // namespace data
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_DATA_H_INCLUDED
