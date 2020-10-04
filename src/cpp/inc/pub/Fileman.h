//----------------------------------------------------------------------------
//
//       Copyright (C) 2020 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Fileman.h
//
// Purpose-
//       All the file management classes, conveniently packaged in one file.
//
// Last change date-
//       2020/10/03
//
//----------------------------------------------------------------------------
#ifndef _PUB_FILEMAN_H_INCLUDED
#define _PUB_FILEMAN_H_INCLUDED

#include <string>                   // For std::string
#include <sys/stat.h>               // For struct stat

#include <pub/List.h>               // For pub::DHDL_List, ...

namespace _PUB_NAMESPACE::Fileman { // The pub::Fileman namespace
//----------------------------------------------------------------------------
//
// Class-
//       File
//
// Purpose-
//       File name information
//
//----------------------------------------------------------------------------
class File : public SORT_List<File>::Link { // File name information
//----------------------------------------------------------------------------
// File::Attributes
//----------------------------------------------------------------------------
public:
typedef struct stat    stat_t;      // struct stat type

const std::string      name;        // The file name (Does not include Path)
const stat_t           st;          // The stat info

//----------------------------------------------------------------------------
// File::Constructors
//----------------------------------------------------------------------------
public:
   File(                            // Constructor
     const stat_t&     _st,         // Stat descriptor
     const std::string&_name)       // File name
:  name(_name), st(_st) {}

//----------------------------------------------------------------------------
// File::Destructor
//----------------------------------------------------------------------------
virtual ~File( void ) {}            // Destructor

//----------------------------------------------------------------------------
// File::Methods
//----------------------------------------------------------------------------
public:
virtual int                         // Result (<0, =0, >0)
   compare(                         // Compare this (using file name) to
     const SORT_List<void>::Link*
                       _that) const; // That (File*) Link
}; // class File

//----------------------------------------------------------------------------
//
// Class-
//       Path
//
// Purpose-
//       Path name information
//
//----------------------------------------------------------------------------
class Path {                        // Path name information
//----------------------------------------------------------------------------
// Path::Attributes
//----------------------------------------------------------------------------
public:
const std::string      name;        // The path name (Locally qualified)
SORT_List<File>        list;        // The (sorted) list of Files

//----------------------------------------------------------------------------
// Path::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Path( void );                   // Destructor

   Path(                            // Constructor
     const std::string&_name);      // Path name (Locally qualified)
}; // class Path

//----------------------------------------------------------------------------
//
// Class-
//       Line
//
// Purpose-
//       An immutable file line.
//
// Implementation notes-
//       The text is allocated from a Pool.
//       It is not allocated or released by the Line object.
//
//----------------------------------------------------------------------------
class Line : public DHDL_List<Line>::Link { // File line
//----------------------------------------------------------------------------
// Line::Attributes
//----------------------------------------------------------------------------
public:
const char*            text;        // The associated text

//----------------------------------------------------------------------------
// Line::Constructors
//----------------------------------------------------------------------------
public:
   Line(                            // Constructor
     const char*       _text)       // The associated text
:  text(_text) {}

//----------------------------------------------------------------------------
// Line::Methods
//----------------------------------------------------------------------------
public:
void
   debug( void ) const {}           // Debugging display (NOT IMPLEMENTED)
}; // class Line

//----------------------------------------------------------------------------
//
// Class-
//       Pool
//
// Purpose-
//       A storage Pool fragment
//
// Implementation notes-
//       Storage is allocated from a Pool.
//
//----------------------------------------------------------------------------
class Pool : public DHDL_List<Pool>::Link { // Storage Pool fragment
//----------------------------------------------------------------------------
// Pool::Attributes
//----------------------------------------------------------------------------
protected:
size_t                 used;        // Number of bytes used
size_t                 size;        // The total Pool size
char*                  data;        // The Pool data area

//----------------------------------------------------------------------------
// Pool::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Pool( void )                    // Destructor
{  delete [] data; }                // Delete the data

   Pool(                            // Constructor
     size_t            _size)       // The allocation size
:  used(0), size(_size), data(new char[size]) {}

//----------------------------------------------------------------------------
// Pool::Methods
//----------------------------------------------------------------------------
public:
virtual void
   debug( void ) const;             // Debugging display

virtual char*                       // Allocated storage, nullptr on failure
   malloc(                          // Allocate storage
     size_t            _size);      // The required length
}; // class Pool

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
DHDL_List<Line>        _line;       // The Line list
DHDL_List<Pool>        _pool;       // The Pool list

bool                   _changed;    // File is changed
bool                   _damaged;    // File is damaged

//----------------------------------------------------------------------------
// Data::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Data( void );                   // Destructor

   Data( void );                    // Default constructor

   Data(                            // Constructor
     const std::string&_path,       // The (locally qualified) path name
     const std::string&_file);      // The File information

//----------------------------------------------------------------------------
// Data::Accessors
//----------------------------------------------------------------------------
public:
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

std::string                         // The path/file name
   full( void )                     // Get path/file name
{  return _path + "/" + _file; }

DHDL_List<Line>&                    // The Line list
   line( void )                     // Get Line list
{  return _line; }

std::string                         // The file name
   file( void )                     // Get file name
{  return _file; }

//----------------------------------------------------------------------------
// Data::Methods
//----------------------------------------------------------------------------
public:
void
   close( void );                   // Close (empty) the Data

void
   debug( void ) const;             // Debugging trace

Line*                               // Line*, bad_alloc iff failure
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
   write( void )                    // Replace file
{  return write(_path, _file); }
}; // class Data
}  // namespace _PUB_NAMESPACE::Fileman
#endif // _PUB_FILEMAN_H_INCLUDED
