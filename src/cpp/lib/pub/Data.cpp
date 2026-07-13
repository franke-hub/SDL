//----------------------------------------------------------------------------
//
//       Copyright (c) 2020-2026 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
// SPDX-License-Identifier: GPL-3.0-only
//----------------------------------------------------------------------------
//
// Title-
//       Data.cpp
//
// Purpose-
//       Data.h object methods
//
// Last change date-
//       2026/07/13
//
// Implementation note-
//       Derived from Fileman.cpp
//
//----------------------------------------------------------------------------
#include <cassert>                  // For assert
#include <cerrno>                   // For errno
#include <climits>                  // For PATH_MAX, SYMLINK_MAX, SYMLOOP_MAX
#include <cstdarg>                  // For va_* functions
#include <cstdio>                   // For fprintf, ...
#include <cstring>                  // For strcpy, ...

#include <dirent.h>                 // For struct dirent
#include <unistd.h>                 // For getcwd, ...
#include <sys/stat.h>               // For struct stat, lstat

#include <pub/Debug.h>              // For pub::debugging
#include "pub/Data.h"               // For namespace pub::data, implemented
#include "pub/List.h"               // For pub::List
#include <pub/utility.h>            // For pub::utility::dump
#include <pub/utility.i>            // For pub::utility subroutines

#define PUB _LIBPUB_NAMESPACE
using namespace PUB::debugging;     // For debugging subroutines
using pub::s2c;                     // For string to const char* (utility.i)

using std::string;                  // For convenience

namespace _LIBPUB_NAMESPACE::data { // The Data namespace
//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose

,  MIN_POOL_SIZE= 65536             // Minimum pool size
};
#define MIN_SYMLOOP 256             // Minimum maximum symlink loop count

//----------------------------------------------------------------------------
//
// Subroutine-
//       errorp
//
// Purpose-
//       Write I/O error message, preserving errno.
//
//----------------------------------------------------------------------------
_LIBPUB_PRINTF(1, 2)
static void
   errorp(                          // Write I/O error message
     const char*       fmt,         // Printf-style format string
                       ...)         // The remaining arguments
{
   va_list    argptr;               // The argument list
   int ERRNO= errno;                // Preserve errno

   fflush(NULL);                    // Flush everything first

   char buffer[4096];               // Big enough buffer
   va_start(argptr, fmt);           // Initialize va_ functions
   vsnprintf(buffer, sizeof(buffer), fmt, argptr); // Format the message
   va_end(argptr);                  // Close va_ functions
   perror(buffer);                  // Write the message in one operation

   fflush(stderr);                  // Insure messsage written
   errno= ERRNO;                    // Restore errno
}

//============================================================================
//
// Method-
//       pub::data::Data::Data
//       pub::data::Data::~Data
//
// Purpose-
//       Constructor.
//       Destructor.
//
//----------------------------------------------------------------------------
   Data::Data( void )               // Default constructor
:  path_name(), file_name(), line_list(), pool_list()
,  is_changed(false), is_damaged(true)
{  }                                // (In closed state)

   Data::Data(                      // Constructor
     const string&     path,        // The Path name
     const string&     file)        // The File name
:  path_name(), file_name(), line_list(), pool_list()
{  open(path, file); }              // Load the data

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   Data::~Data( void )              // Destructor
{  close(); }                       // Delete line_list and pool_list data

//----------------------------------------------------------------------------
//
// Method-
//       pub::data::Data::close
//
// Purpose-
//       Close (empty) the data.
//
//----------------------------------------------------------------------------
void
   Data::close( void )              // Delete all data
{
   for(;;) {                        // Delete all line_list Lines
     Line* line= line_list.remq();
     if( line == nullptr )
       break;

     delete line;
   }

   for(;;) {                        // Delete all pool_list Pools
     Pool* pool= pool_list.remq();
     if( pool == nullptr )
       break;

     delete pool;
   }

   path_name= "";
   file_name= "";
   is_changed= false;
   is_damaged= true;
}

//----------------------------------------------------------------------------
//
// Method-
//       pub::data::Data::full
//
// Purpose-
//       Get path + file name
//
//----------------------------------------------------------------------------
string                              // The path/file name
   Data::full( void ) const         // Get path/file name
{  return Name::get_full_name(path_name, file_name); }

//----------------------------------------------------------------------------
//
// Method-
//       pub::data::Data::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   Data::debug(const char* info) const // Display debugging information
{
   printf("Data::debug(%s)\n", info);

   size_t index= 0;
   for(Line* line= line_list.get_head(); line; line= line->get_next())
     printf("[%4zd] '%s'\n", ++index, line->text);

   if( index )
     printf("\n");

   for(Pool* pool= pool_list.get_head(); pool != nullptr; pool= pool->get_next())
     pool->debug(info);
}

//----------------------------------------------------------------------------
//
// Method-
//       pub::data::Data::get_line
//
// Purpose-
//       Allocate a new Line*
//
//----------------------------------------------------------------------------
Line*                               // The allocated Line*
   Data::get_line(                  // Allocate a new Line
     const string&     string)      // Containing this string
{
   size_t size= string.length() + 1;

   Pool* pool= pool_list.get_head(); // Get last inserted Pool
   char* text= nullptr;
   if( pool )                       // (If empty pool)
     text= pool->malloc(size);
   if( text == nullptr ) {          // If a new Pool is needed
     if( size > MIN_POOL_SIZE )
       pool= new Pool(size + 5);
     else
       pool= new Pool(MIN_POOL_SIZE);

     pool_list.lifo(pool);          // Insert Pool onto Pool List
     text= pool->malloc(size);      // Allocate from new Pool
     assert( text != nullptr );
   }
   strcpy(text, s2c(string));       // Copy the text

   Line* line= new Line(text);      // Allocate a new Line
   return line;
}

//----------------------------------------------------------------------------
//
// Method-
//       pub::data::Data::open
//
// Purpose-
//       Load data.
//
//----------------------------------------------------------------------------
int                                 // Return code, 0 OK
   Data::open(                      // (Re)load data
     const string&     path,        // The Path name
     const string&     file)        // The File name
{
   close();                         // Delete any existing data

   this->path_name= path;           // Update path name
   this->file_name= file;           // Update file name
   is_changed= false;               // Not changed
   is_damaged= false;               // Not damaged

   struct stat st;                  // File stats
   string      fqname= full();      // The fully qualified name
   int rc= stat(s2c(fqname), &st); // Get file information
   if( rc != 0 ) {                  // If failure
     // errorp("%4d: Data: stat(%s) failure: %d", __LINE__, s2c(fqname), rc);
     return rc;
   }

   size_t size= st.st_size;         // The size of the file

   // Allocate the input data area Pool
   Pool* pool= new Pool(size + 1);  // We'll add '\0' to the end
   pool_list.lifo(pool);
   char* text= pool->malloc(size + 1); // Allocate entire Pool
   assert( text != nullptr );       // Should work, tests edge case

   // Load the file
   FILE* f= fopen(s2c(fqname), "rb");
   size_t L= fread(text, 1, size+1, f);
   if( L != size ) {
     is_damaged= true;
     fprintf(stderr, "%4d Data: File(%s) read failure %ld\n", __LINE__,
                     s2c(fqname), (long)L);
     memset(text, 0, size);
   }
   text[size]= '\0';                // Add '\0' delimiter
   fclose(f);

   // Insure that the file does not contain a '\0' delimiter
   char* last= strchr(text, '\0');  // Locate first '\0' delimiter
   if( size_t(last-text) < size ) { // If file contains '\0' delimiter
     is_damaged= true;
     fprintf(stderr, "%4d Data: File(%s) contains '\\0' delimiter\n", __LINE__,
                     s2c(fqname));
   }

   // Parse the text into lines (Performance critical path)
   char* used= text;
   while( used < last ) {
     char* from= used;              // Starting character
     char* next= strchr(used, '\n'); // Get next line delimiter
     if( next ) {
       *next= '\0';                 // Replace with string delimiter
       used= next + 1;              // Next line origin
       while( next > from ) {
         next--;
         if( *next != '\r' )
           break;

         is_changed= true;          // Write will change file format
         *next= '\0';
       }

       line_list.fifo(new Line(from));
     } else {                       // Last line missing '\n'
       is_changed= true;            // Write will change file format
       fprintf(stderr, "%4d Data: File(%s) last line missing '\\n'\n", __LINE__,
                       s2c(fqname));
       line_list.fifo(new Line(from));
       break;
     }
   }

   if( is_damaged )
     return -1;
   return 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       pub::data::Data::write
//
// Purpose-
//       Write data.
//
//----------------------------------------------------------------------------
int                                 // Return code, 0 OK
   Data::write(                     // Write data
     const string&     path,        // The (locally qualified) path name
     const string&     file) const  // The file name
{
   int                 rc= -1;      // Return code, default ERROR

   string fqname= Name::get_full_name(path, file);
   if( is_damaged )                 // If damaged file
     fprintf(stderr, "*WARNING* writing damaged file(%s)\n", s2c(fqname));

   FILE* f= fopen(s2c(fqname), "wb"); // Open the file
   if( f ) {                        // If open succeeded
     for(Line* line= line_list.get_head(); line; line= line->get_next())
       fprintf(f, "%s\n", line->text);

     rc= fclose(f);
     if( rc )
       errorp("%4d: Data: close('%s') failure", __LINE__, s2c(fqname));
   } else {                         // If open failure
     errorp("%4d: Data: open('%s') failure", __LINE__, s2c(fqname));
   }

   return rc;
}

//============================================================================
//
// Method-
//       pub::data::File::make_st
//
// Purpose-
//       Create stat_t from file name (Constructor helper)
//
//----------------------------------------------------------------------------
File::stat_t                        // The resultant stat_t
   File::make_st(                   // Create a stat_t
     string            name)        // From this file name string
{  stat_t info{}; lstat(s2c(name), &info); return info; }

//----------------------------------------------------------------------------
//
// Method-
//       pub::data::File::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   File::debug(const char*   info) const // Debugging display
{  debugf("File(%p)::debug(%s) '%s'\n", this, info, s2c(file_name)); }

//============================================================================
//
// Method-
//       pub::data::Name::Name
//
// Purpose-
//       Constructors.
//
//----------------------------------------------------------------------------
   Name::Name(                      // Constructor
     const string&     full_name)   // The file name
{  reset(full_name); }

   Name::Name(                      // Constructor
     const string&     path_name,   // The (locally qualified) path name
     const string&     file_name)   // The file name
{  reset(path_name, file_name); }

//----------------------------------------------------------------------------
//
// Method-
//       pub::data::Name::get_extension
//
// Purpose-
//       Get the file extension part of a file_name
//
//----------------------------------------------------------------------------
string                              // The file name extension, "" if none
   Name::get_extension(             // Get file name extension for
     const string&     file_name)   // This file name
{
   ssize_t X= file_name.length() - 1; // Last character in file_name
   while( X >= 0 && file_name[X] != '/' ) { // Find last '.' in name
     if( file_name[X] == '.' )      // If extension separator
       return file_name.substr(X + 1);

     --X;
   }

   return "";                       // (No extension)
}

//----------------------------------------------------------------------------
//
// Method-
//       pub::data::Name::get_file_name
//
// Purpose-
//       Get file name part of (relative) full_name
//
//----------------------------------------------------------------------------
string                              // The file name of (relative) full_name
   Name::get_file_name(             // Get file part of
     const string&     full_name)   // This relative full name
{
   ssize_t X= full_name.length() - 1; // Last character in _name
   while( X >= 0 && full_name[X] != '/' ) // Find last '/' in name
     X--;

   return full_name.substr(X + 1);
}

//----------------------------------------------------------------------------
//
// Method-
//       pub::data::Name::get_full_name
//
// Purpose-
//       Combine relative path name and file name into fully qualified name
//
//----------------------------------------------------------------------------
string                              // The (relative) fully qualified name
   Name::get_full_name(             // Get (relative) fully qualified name
     const string&     path,        // Using this relative path name and
     const string&     file)        // This file name
{
   string full= path;

   if( full == "" )                 // If empty path
     full= ".";                     // (Use default relative path)

   if( full[full.size() - 1] == '/' )
     return full + file;

   return full + "/" + file;
}

//----------------------------------------------------------------------------
//
// Method-
//       pub::data::Name::get_path_name
//
// Purpose-
//       Get path part of (relative) full_name
//
//----------------------------------------------------------------------------
string                              // The path name of (relative) full_name
   Name::get_path_name(             // Get path part of
     const string&     full_name)   // This relative full name
{
   ssize_t X= full_name.length() - 1; // Last character in _name
   while( X >= 0 && full_name[X] != '/' ) // Find last '/' in name
     X--;

   if( X > 0 )
     return full_name.substr(0, X);
   if( X == 0 )
     return "";
   return ".";
}

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
   Name::reset(                     // Reset the Name with
     const string&     full)         // This (relative) full name
{
   full_name= full;
   path_name= get_path_name(full);
   file_name= get_file_name(full);

   // Get file/link status
   memset(&st, 0, sizeof(st));      // In case of failure
   lstat(s2c(full_name), &st);
}

void
   Name::reset(                     // Reset the Name with
     const string&     path,        // This (relative) path name and
     const string&     file)        // This file name
{
   path_name= path;
   file_name= file;
   full_name= get_full_name(path, file);

   // Get file/link status
   memset(&st, 0, sizeof(st));      // In case of failure
   lstat(s2c(full_name), &st);
}

//----------------------------------------------------------------------------
//
// Method-
//       pub::data::Name::resolve
//
// Purpose-
//       Resolve (remove) links in file_name, link_name, and name
//
//----------------------------------------------------------------------------
string                              // The invalid path ("" if none)
   Name::resolve( void )            // Resolve links in name
{
   // Resolve current working directory
   string full_name= this->full_name;
   if( full_name[0] != '/' ) {
     if( full_name[0] == '~' && full_name[1] == '/' ) {
       const char* HOME= getenv("HOME"); // Get $HOME
       if( HOME == nullptr ) return "Missing $HOME";
       full_name= HOME + full_name.substr(1);
     } else {
       char buffer[PATH_MAX + 8];
       buffer[0]= '\0';
       const char* CWD= getcwd(buffer, PATH_MAX);
       if( CWD == nullptr ) return "CWD too large";
       string cwd= CWD;
       if( cwd == "/" )
         cwd= "";
       full_name= cwd + "/" + path_name + "/" + file_name;
     }
   }

   // Determine the minimum maximum symbolic loop count
   unsigned MAX_SYMLOOP= MIN_SYMLOOP;
   #if defined( SYMLOOP_MAX ) && SYMLOOP_MAX > MIN_SYMLOOP
     MAX_SYMLOOP= SYMLOOP_MAX;
   #elif defined( _SC_SYMLOOP_MAX )
     long symloop_max= sysconf(_SC_SYMLOOP_MAX);
     if( symloop_max > MAX_SYMLOOP )
       MAX_SYMLOOP= symloop_max;
   #endif
   #if defined( _POSIX_SYMLOOP_MAX ) && _POSIX_SYMLOOP_MAX > MIN_SYMLOOP
     if( _POSIX_SYMLOOP_MAX > MAX_SYMLOOP )
       MAX_SYMLOOP= _POSIX_SYMLOOP_MAX;
   #endif

   // TODO: Handle UTF8 <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
   size_t X= 0;                     // Full name character offset
   unsigned sym_count= 0;           // Symbolic link count

   for(;;) {                        // Resolve symbolic links
     if( HCDM ) {
       debugf("\nVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV\n");
       debugf("     '          1         2         3         4         5"
                     "         6         7'\n");
       debugf("%4zd '012345678901234567890123456789012345678901234567890"
                     "12345678901234567890'\n", X);
       debugf("%4d '%s'\n", __LINE__, s2c(full_name));
       if( X < full_name.length() && full_name[X] != '/' )
         return "INTERNAL ERROR";
     }

     X++;                           // (Skip '/' character)
     size_t L= full_name.length();
     if( X > L )
       break;

     string init_part= full_name;
     string last_part= "";
     const char* O= s2c(full_name);
     const char* C= strchr(O + X, '/');
     if( C == nullptr )
       X= L;
     else
       X= C - O;
     init_part= full_name.substr(0, X); // DOES NOT include trailing '/'
     last_part= full_name.substr(X);    // INCLUDES leading '/' (or "")

     if( HCDM )
       debugf("%4d '%s'[%zd]'%s'\n", __LINE__, s2c(init_part), X
             , s2c(last_part));
     string file_part= get_file_name(init_part);

     // Handle special case file_part names: "", "." and ".."
     if( file_part == "" )
       return init_part + " (Empty file name)";

     if( file_part == "." ) {
       init_part= get_path_name(init_part);
       full_name= init_part + last_part;
       X= init_part.length();
       continue;
     }

     if( file_part == ".." ) {
       if( full_name.size() >= 4 && full_name.substr(0,4) == "/../" ) {
         init_part= "";
       } else {
         init_part= get_path_name(init_part);
         init_part= get_path_name(init_part);
       }

       X= init_part.length();
       if( HCDM ) {
         debugf("'%s'+'%s' init_part + last_part\n"
               , s2c(init_part), s2c(last_part));
       }
       full_name= init_part + last_part;
       continue;
     }

     // Handle path component
     struct stat info;
     int rc= lstat(s2c(init_part), &info);
     if( rc ) {
       if( last_part == "" )
         break;
       return init_part + " (Invalid path)";
     }

     if( S_ISLNK(info.st_mode) ) {
       if( sym_count++ > MAX_SYMLOOP )
         return init_part + " (MAX_SYMLOOP)";

       char buffer[PATH_MAX + 8];
       buffer[0]= '\0';
       rc= readlink(s2c(init_part), buffer, PATH_MAX);
       if( rc < 0 || size_t(rc) >= PATH_MAX )
         return init_part + " (readlink failure)";
       buffer[rc]= '\0';
       if( buffer[0] == '/' ) {
         init_part= buffer;
         X= 0;
       } else {
         init_part= get_path_name(init_part);
         X= init_part.length();
         init_part= init_part + "/" + buffer;
       }
       full_name= init_part + last_part;
       continue;
     }
   }

   file_name= get_file_name(full_name);
   path_name= get_path_name(full_name);

   // Get file status
   memset(&st, 0, sizeof(st));      // In case of failure (nonexistent file)
   lstat(s2c(full_name), &st);
   return "";
}

//============================================================================
//
// Method-
//       pub::data::Path::Path
//       pub::data::Path::~Path
//
// Purpose-
//       Constructors.
//       Destructor.
//
//----------------------------------------------------------------------------
   Path::Path(                      // Constructor
     const char*       name)        // The Path name
:  list()
{  reset(name); }

   Path::Path(                      // Constructor
     const string&     name)        // The Path name
:  list()
{  reset(name); }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   Path::~Path( void )              // Destructor
{  reset(); }

//----------------------------------------------------------------------------
//
// Method-
//       pub::data::Path::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   Path::debug(const char* info) const // Display debugging information
{
   debugf("Path(%p)::debug(%s) '%s'\n", this, info, s2c(path_name));

   for(File* file= list.get_head(); file; file= file->get_next()) {
     file->debug(info);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       pub::data::Path::insert
//
// Purpose-
//       Add a file to the List
//
//----------------------------------------------------------------------------
void
   Path::insert(                    // Insert onto the List
     File*             file)        // This File
{  list.fifo(file); }

//----------------------------------------------------------------------------
//
// Method-
//       pub::data::Path::make_file
//
// Purpose-
//       Create and add a file to the List
//
//----------------------------------------------------------------------------
File*                               // The new File
   Path::make_file(                 // Create a new  File
     const stat_t&     st,          // File information
     const string&     file_name) const // The File name
{  return new File(st, file_name); }

//----------------------------------------------------------------------------
//
// Method-
//       pub::data::Path::reset
//
// Purpose-
//       Reset and optionally reload the List
//
//----------------------------------------------------------------------------
void
   Path::reset(                     // Reset and optionally reload the List
     const char*       name)        // The new Path name
{
   for(;;) {                        // Delete the list
     File* file= list.remq();
     if( file == nullptr )
       break;

     delete file;
   }

   if( name == nullptr ) {          // If reset only
     path_name= "";
     return;
   }

   //-------------------------------------------------------------------------
   // Load the directory list
   //-------------------------------------------------------------------------
   path_name= name;                 // Set the path_name
   string S= path_name;
   if( S == "" ) S= ".";            // (Empty path name for relative path ".")
   DIR* dir= opendir(s2c(S));       // Open the directory stream
   if( dir == NULL ) {              // Stream not opened
     errorp("%4d: Path: opendir('%s') failure", __LINE__, name);
     return;
   }

   for(;;) {                        // For each directory entry
     struct dirent* ent= readdir(dir); // Read the directory entry
     if( ent == NULL )
       break;

     string file_name(ent->d_name);  // The file name
     if( file_name == "." || file_name == ".." ) // If pseudo entry
       continue;                    // Ignore it
     string full_name= Name::get_full_name(path_name, file_name);

     struct stat s;                 // File stats
     int rc= lstat(s2c(full_name), &s); // Load the file information
     if( rc != 0 ) {                // If failure
       errorp("%4d: Path: lstat(%s) failure: %d", __LINE__, s2c(full_name), rc);
       continue;
     }

     File* file= make_file(s, file_name);
     insert(file);
   }

   int rc= closedir(dir);           // Done reading the directory
   if( rc != 0 )                    // If error encountered
     errorp("%4d: Path: closedir('%s') failure", __LINE__, name);

   //-------------------------------------------------------------------------
   // Sort the list
   //-------------------------------------------------------------------------
   list.sort();
}

void
   Path::reset(                     // Reset and reload the List
     const string&     name)        // The new Path name
{  reset(s2c(name)); }

//============================================================================
//
// Method-
//       pub::data::Pool::Pool
//       pub::data::Pool::~Pool
//
// Purpose-
//       Constructor
//       Destructor
//
//----------------------------------------------------------------------------
   Pool::Pool(                      // Constructor
     size_t            _size)       // The allocation size
:  used(0), size(_size), data(new char[size])
{  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   Pool::~Pool( void )              // Destructor
{  delete [] data; }                // Delete the data

//----------------------------------------------------------------------------
//
// Method-
//       pub::data::Pool::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   Pool::debug(const char*info) const // Display debugging information
{
   printf("Pool::debug(%s)\n", info);

   if( HCDM ) {
     tracef("%p Pool used(%zd) size(%zd) data(%p)\n", this, used, size, data);
     utility::dump(Debug::get()->get_FILE(), data, used);
     tracef("\n");
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       pub::data::Pool::malloc
//
// Purpose-
//       Allocate storage from Pool fragment
//
//----------------------------------------------------------------------------
char*                               // The allocated storage, nullptr if none
   Pool::malloc(                    // Get allocated storage
     size_t            size)        // The required length
{
   if( used + size > this->size )   // If storage not available
     return nullptr;

   char* result= data + used;       // The allocated storage
   used += size;                    // Indicate allocated
   return result;
}
}  // namespace _LIBPUB_NAMESPACE::data
