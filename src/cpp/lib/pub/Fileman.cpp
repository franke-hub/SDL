//----------------------------------------------------------------------------
//
//       Copyright (c) 2020-2023 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Fileman.cpp
//
// Purpose-
//       Fileman.h object methods
//
// Last change date-
//       2023/11/21
//
// Implementation note-
//       TODO: Deprecate, rename to Data.h; rename line=>get_list; etc.
//
//----------------------------------------------------------------------------
#include <assert.h>                 // For assert
#include <dirent.h>                 // For struct dirent
#include <errno.h>                  // For errno
#include <limits.h>                 // For PATH_MAX, SYMLINK_MAX, SYMLOOP_MAX
#include <stdarg.h>                 // For va_* functions
#include <stdio.h>                  // For fprintf, ...
#include <string.h>                 // For strcpy, ...
#include <unistd.h>                 // For getcwd, ...
#include <sys/stat.h>               // For struct stat, lstat
#include <sys/types.h>              // For system types

#include <pub/Debug.h>              // For pub::debugging
#include "pub/Fileman.h"            // For pub::fileman, implemented
#include <pub/List.h>               // For pub::List
#include <pub/utility.h>            // For pub::utility::dump

using namespace _LIBPUB_NAMESPACE::debugging;

namespace _LIBPUB_NAMESPACE::fileman { // The fileman namespace
//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
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

//----------------------------------------------------------------------------
//
// Method-
//       Data::~Data
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Data::~Data( void )              // Destructor
{
   close();                         // Delete _line and _pool data
}

//----------------------------------------------------------------------------
//
// Method-
//       Data::Data
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Data::Data( void )               // Default constructor
:  _path(), _file(), _line(), _pool(), _changed(false), _damaged(true)
{  }                                // (In closed state)

   Data::Data(                      // Constructor
     const std::string&_path,       // The Path name
     const std::string&_file)       // The File name
:  _path(), _file(), _line(), _pool()
{
   open(_path, _file);              // Load the data
}

//----------------------------------------------------------------------------
//
// Method-
//       Data::close
//
// Purpose-
//       Close (empty) the data.
//
//----------------------------------------------------------------------------
void
   Data::close( void )              // Delete all data
{
   for(;;)                          // Delete the _line List
   {
     Line* line= _line.remq();
     if( line == nullptr )
       break;

     delete line;
   }

   for(;;)                          // Delete the _pool List
   {
     Pool* pool= _pool.remq();
     if( pool == nullptr )
       break;

     delete pool;
   }

   _path= "";
   _file= "";
   _changed= false;
   _damaged= true;
}

//----------------------------------------------------------------------------
//
// Method-
//       Data::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   Data::debug( void ) const        // Debugging display
{
   size_t index= 0;
   for(Line* line= _line.get_head(); line; line= line->get_next()) {
     printf("[%4zd] '%s'\n", ++index, line->text);
   }
   if( index )
     printf("\n");

   for(Pool* pool= _pool.get_head(); pool != nullptr; pool= pool->get_next())
     pool->debug();
}

//----------------------------------------------------------------------------
//
// Method-
//       Data::get_line
//
// Purpose-
//       Allocate a new Line*
//
//----------------------------------------------------------------------------
Line*                               // The allocated Line*
   Data::get_line(                  // Allocate a new Line
     const std::string&string)      // Containing this string
{
   size_t size= string.length() + 1;

   Pool* pool= _pool.get_head();    // Get last inserted Pool
   char* text= nullptr;
   if( pool )                       // (If empty pool)
     text= pool->malloc(size);
   if( text == nullptr ) {          // If a new Pool is needed
     if( size > MIN_POOL_SIZE )
       pool= new Pool(size);
     else
       pool= new Pool(MIN_POOL_SIZE);

     _pool.lifo(pool);              // Insert Pool onto Pool List
     text= pool->malloc(size);      // Allocate from new Pool
     assert( text != nullptr );
   }
   strcpy(text, string.c_str());    // Copy the text

   Line* line= new Line(text);      // Allocate a new Line
   return line;
}

//----------------------------------------------------------------------------
//
// Method-
//       Data::open
//
// Purpose-
//       Load data.
//
//----------------------------------------------------------------------------
int                                 // Return code, 0 OK
   Data::open(                      // (Re)load data
     const std::string&_path,       // The Path name
     const std::string&_file)       // The File name
{
   close();                         // Delete any existing data

   this->_path= _path;              // Update path name
   this->_file= _file;              // Update file name
   _changed= false;                 // Not changed
   _damaged= false;                 // Not damaged

   struct stat st;                  // File stats
   std::string _full= full();       // The fully qualified name
   int rc= stat(_full.c_str(), &st); // Get file information
   if( rc != 0 )                    // If failure
   {
     errorp("%4d: Data: stat(%s) failure: %d", __LINE__, _full.c_str(), rc);
     return rc;
   }

   size_t size= st.st_size;         // The size of the file

   // Allocate the input data area Pool
   Pool* pool= new Pool(size + 1);  // We'll add '\0' to the end
   _pool.lifo(pool);
   char* text= pool->malloc(size + 1); // Allocate entire Pool
   assert( text != nullptr );       // Should work, tests edge case

   // Load the file
   FILE* f= fopen(_full.c_str(), "rb");
   size_t L= fread(text, 1, size+1, f);
   if( L != size )
   {
     _damaged= true;
     fprintf(stderr, "%4d Data: File(%s) read failure %ld\n", __LINE__,
                     _full.c_str(), (long)L);
     memset(text, 0, size);
   }
   text[size]= '\0';                // Add '\0' delimiter
   fclose(f);

   // Insure that the file does not contain a '\0' delimiter
   char* last= strchr(text, '\0');  // Locate first '\0' delimiter
   if( size_t(last-text) < size )   // If file contains '\0' delimiter
   {
     _damaged= true;
     fprintf(stderr, "%4d Data: File(%s) contains '\\0' delimiter\n", __LINE__,
                     _full.c_str());
   }

   // Parse the text into lines (Performance critical path)
   char* used= text;
   while( used < last )
   {
     char* from= used;              // Starting character
     char* next= strchr(used, '\n'); // Get next line delimiter
     if( next )
     {
       *next= '\0';                 // Replace with string delimiter
       used= next + 1;              // Next line origin
       while( next > from )
       {
         next--;
         if( *next != '\r' )
           break;

         _changed= true;            // Write will change file format
         *next= '\0';
       }

       _line.fifo(new Line(from));
     } else {                       // Last line missing '\n'
       _changed= true;              // Write will change file format
       fprintf(stderr, "%4d Data: File(%s) last line missing '\\n'\n", __LINE__,
                       _full.c_str());
       _line.fifo(new Line(from));
       break;
     }
   }

   if( _damaged )
     return -1;
   return 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       Data::write
//
// Purpose-
//       Write data.
//
//----------------------------------------------------------------------------
int                                 // Return code, 0 OK
   Data::write(                     // Write data
     const std::string&path,        // The (locally qualified) path name
     const std::string&file) const  // The file name
{
   int                 rc= -1;      // Return code, default ERROR

   std::string _full= path + "/" + file; // Locally qualified name
   if( _damaged )                   // If damaged file
     fprintf(stderr, "*WARNING* writing damaged file(%s)\n", _full.c_str());

   FILE* f= fopen(_full.c_str(), "wb"); // Open the file
   if( f ) {                        // If open succeeded
     for(Line* line= _line.get_head(); line; line= line->get_next())
       fprintf(f, "%s\n", line->text);

     rc= fclose(f);
     if( rc )
       errorp("%4d: Data: close('%s') failure", __LINE__, _full.c_str());
   } else {                         // If open failure
     errorp("%4d: Data: open('%s') failure", __LINE__, _full.c_str());
   }

   return rc;
}

//----------------------------------------------------------------------------
//
// Method-
//       Name::Name
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Name::Name(                      // Constructor
     std::string       full_name)   // The file name
{  reset(full_name); }

//----------------------------------------------------------------------------
//
// Method-
//       Name::get_file_name
//
// Purpose-
//       Get file name part of (relative) full_name
//
//----------------------------------------------------------------------------
std::string                         // The file name of (relative) full_name
   Name::get_file_name(             // Get file part of
     std::string       full_name)   // This relative full name
{
   ssize_t X= full_name.length() - 1; // Last character in _name
   while( X >= 0 && full_name[X] != '/' ) // Find last '/' in name
     X--;

   return full_name.substr(X + 1);
}

//----------------------------------------------------------------------------
//
// Method-
//       Name::get_path_name
//
// Purpose-
//       Get path part of (relative) full_name
//
//----------------------------------------------------------------------------
std::string                         // The path name of (relative) full_name
   Name::get_path_name(             // Get path part of
     std::string       full_name)   // This relative full name
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
//       Name::reset
//
// Purpose-
//       Reset the file name
//
//----------------------------------------------------------------------------
void
   Name::reset(                     // Reset the file name
     std::string       full_name)   // The file name
{
   path_name= get_path_name(full_name);
   file_name= get_file_name(full_name);

   name= path_name + "/" + file_name;

   // Get file/link status
   memset(&st, 0, sizeof(st));      // In case of failure
   lstat(name.c_str(), &st);
}

//----------------------------------------------------------------------------
//
// Method-
//       Name::resolve
//
// Purpose-
//       Resolve (remove) links in file_name, link_name, and name
//
//----------------------------------------------------------------------------
std::string                         // The invalid path ("" if none)
   Name::resolve( void )            // Resolve links in name
{
   // Resolve current working directory
   std::string full_name= path_name + "/" + file_name;
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
       std::string cwd= CWD;
       if( cwd == "/" )
         cwd= "";
       full_name= cwd + "/" + path_name + "/" + file_name;
     }
   }

   // Resolve MAX_SYMLOOP (ugly)
   unsigned MAX_SYMLOOP= MIN_SYMLOOP; // Minimum maximum symbolic link count
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
//   debugf("\nVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV\n");
//   debugf("     '          1         2         3         4         5         6         7'\n");
//   debugf("%4zd '01234567890123456789012345678901234567890123456789012345678901234567890'\n", X);
//   debugf("%4d '%s'\n", __LINE__, full_name.c_str());
//   if( X < full_name.length() && full_name[X] != '/' ) return "INTERNAL ERROR";

     X++;                           // (Skip '/' character)
     size_t L= full_name.length();
     if( X > L )
       break;

     std::string init_part= full_name;
     std::string last_part= "";
     const char* O= full_name.c_str();
     const char* C= strchr(O + X, '/');
     if( C == nullptr )
       X= L;
     else
       X= C - O;
     init_part= full_name.substr(0, X); // DOES NOT include trailing '/'
     last_part= full_name.substr(X);    // INCLUDES leading '/' (or "")

//   debugf("%4d '%s'[%zd]'%s'\n", __LINE__, init_part.c_str(), X, last_part.c_str());
     std::string file_part= get_file_name(init_part);

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
       if( full_name.substr(0, 3) == "/.." )
         return full_name + " (Name /..)";

       init_part= get_path_name(init_part);
       init_part= get_path_name(init_part);
       X= init_part.length();
       full_name= init_part + last_part;
       continue;
     }

     // Handle path component
     struct stat info;
     int rc= lstat(init_part.c_str(), &info);
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
       rc= readlink(init_part.c_str(), buffer, PATH_MAX);
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

   name= full_name;
   file_name= get_file_name(name);
   path_name= get_path_name(name);

   // Get file status
   memset(&st, 0, sizeof(st));      // In case of failure (nonexistent file)
   lstat(name.c_str(), &st);
   return "";
}

//----------------------------------------------------------------------------
//
// Method-
//       Path::~Path
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Path::~Path( void )              // Destructor
{
   for(;;)                          // Delete the list
   {
     File* file= list.remq();
     if( file == nullptr )
       break;

     delete file;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Path::Path
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Path::Path(                      // Constructor
     const std::string&_path)       // The Path name
:  name(_path), list()
{
   //-------------------------------------------------------------------------
   // Read the directory
   //-------------------------------------------------------------------------
   std::string S= name.c_str();
   if( S == "" ) S= "/";            // (Empty path name for "/")
   DIR* dir= opendir(S.c_str());    // Open the directory stream
   if( dir == NULL )                // Stream not opened
   {
     errorp("%4d: Path: opendir('%s') failure", __LINE__, _path.c_str());
     return;
   }

   int count= 0;                    // Number of File*
   for(;;)                          // For each directory entry
   {
     struct dirent* ent= readdir(dir); // Read the directory entry
     if( ent == NULL )
       break;

     std::string file(ent->d_name); // The file name
     if( file == "." || file == ".." ) // If pseudo entry
       continue;                    // Ignore it
     std::string full= _path + "/" + file; // The fully qualified name

     struct stat s;                 // File stats
     int rc= lstat(full.c_str(), &s);   // Load the file information
     if( rc != 0 )                  // If failure
     {
       errorp("%4d: Path: lstat(%s) failure: %d", __LINE__, full.c_str(), rc);
       continue;
     }

     list.fifo(new File(s, file));
     count++;
   }

   int rc= closedir(dir);           // Done reading the directory
   if( rc != 0 )                    // If error encountered
     errorp("%4d: Path: closedir('%s') failure", __LINE__, _path.c_str());

   //-------------------------------------------------------------------------
   // Sort the list
   //-------------------------------------------------------------------------
#if USE_BASE_SORT
   typedef const List<void>::_Link  Link;
   list.sort([](Link* lhs, Link* rhs)
            { return ((File*)lhs)->name < ((File*)rhs)->name; }
   );
#else
   list.sort([](File* lhs, File* rhs)
            { return lhs->name < rhs->name; }
   );
#endif
}

//----------------------------------------------------------------------------
//
// Method-
//       Pool::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   Pool::debug( void ) const        // Debugging display
{
   if( HCDM ) {
     tracef("%p Pool used(%zd) size(%zd) data(%p)\n", this, used, size, data);
     utility::dump(Debug::get()->get_FILE(), data, used);
     tracef("\n");
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Pool::malloc
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
}  // namespace _LIBPUB_NAMESPACE::fileman
