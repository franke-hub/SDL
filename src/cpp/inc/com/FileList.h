//----------------------------------------------------------------------------
//
//       Copyright (c) 2010 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       FileList.h
//
// Purpose-
//       Extract information about a list of files.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
#ifndef FILELIST_H_INCLUDED
#define FILELIST_H_INCLUDED

//----------------------------------------------------------------------------
//
// Class-
//       FileList
//
// Purpose-
//       Extract information about a list of files.
//
// Notes-
//       The special file names "." and ".." are NOT returned.
//
// Usage examples-
//       for(FileList fileList(pathName);;fileList.getNext())
//       {
//         const char* fileName= fileList.getCurrent();
//         if( fileName == NULL )
//           break;
//
//         FileInfo fileInfo(pathName, fileName); // ALWAYS exists
//         : // Work with name
//       }
//
//----------------------------------------------------------------------------
class FileList {
//----------------------------------------------------------------------------
// FileList::Attributes
//----------------------------------------------------------------------------
private:
void*                  object;      // The (hidden) object

//----------------------------------------------------------------------------
// FileList::Constructors
//----------------------------------------------------------------------------
public:
   ~FileList( void );               // Default destructor
   FileList( void );                // Default constructor

   FileList(                        // Constructor
     const char*       filePath,    // File Path
     const char*       fileName= "*"); // File Name (may contain wildcards)

private:
   FileList(const FileList&);       // Bitwise copy not allowed
   FileList& operator=(const FileList&);// Bitwise assignment not allowed

//----------------------------------------------------------------------------
// FileList::methods
//----------------------------------------------------------------------------
public:
const char*                         // The current name qualifier (if any)
   getCurrent( void );              // Extract the current file name

const char*                         // The next existing file name (if any)
   getNext( void );                 // Extract the next file name

void
   reset( void );                   // RESET the FileList (delete object)

const char*                         // The first file name, if it exists.
   reset(                           // RESET the FileList (rebuild object)
     const char*       filePath,    // File Path
     const char*       fileName= "*"); // File Name (may contain wildcards)
}; // class FileList

#endif // FILELIST_H_INCLUDED
