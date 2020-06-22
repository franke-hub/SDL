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
//       2020/06/22
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

using _PUB_NAMESPACE::debugging::errorp;

namespace _PUB_NAMESPACE::Fileman { // The Fileman namespace
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
}  // namespace _PUB_NAMESPACE::Fileman
