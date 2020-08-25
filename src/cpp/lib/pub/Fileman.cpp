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
//       Fileman.cpp
//
// Purpose-
//       Fileman.h object methods
//
// Last change date-
//       2020/08/24
//
//----------------------------------------------------------------------------
#include <assert.h>
#include <dirent.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <pub/Debug.h>              // For pub::debugging::errorp
#include "pub/Fileman.h"            // The class objects

#include <pub/utility.h>            // For pub::utility::dump
#include <pub/Debug.h>              // For pub::debugging
#include "pub/Fileman.h"            // The class objects

using namespace _PUB_NAMESPACE::debugging;
using _PUB_NAMESPACE::debugging::errorp;

namespace _PUB_NAMESPACE::Fileman { // The Fileman namespace
//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  MIN_POOL_SIZE= 65536             // Minimum pool size
};

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
   char* text= pool->malloc(size);
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
   if( (last-text) < size )         // If file contains '\0' delimiter
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
//       File::compare
//
// Purpose-
//       Compare File objects (using associated name strings)
//
//----------------------------------------------------------------------------
int                                 // Resultant
   File::compare(                   // Compare this File's filename
     const SORT_List<void>::Link*
                       _that) const  // To that File's filename
{
   const File* that= dynamic_cast<const File*>(_that);
   return name.compare(that->name);
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
   DIR* dir= opendir(name.c_str()); // Open the directory stream
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
   list.sort();
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
     pub::utility::dump(Debug::get()->get_FILE(), data, used);
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
}  // namespace _PUB_NAMESPACE::Fileman
