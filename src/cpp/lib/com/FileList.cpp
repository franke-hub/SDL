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
//       FileList.cpp
//
// Purpose-
//       Extract information about a list of files.
//
// Last change date-
//       2010/01/01
//
// Needs work-
//       TODO: Handle // and //computer_name/ prefix in Windows.
//       Note: Windows: dir \\computer_name always fails.
//
//----------------------------------------------------------------------------
#include <stdlib.h>
#include <string.h>
#include <ctype.h>                  // toupper

#include <com/define.h>             // TRUE, FALSE
#include <com/Debug.h>
#include <com/FileName.h>

#include "com/FileList.h"

#if    defined(_OS_BSD)
  #include <dirent.h>               // opendir, readdir, ...

#elif  defined(_OS_WIN)
  #include "winsim.h"               // opendir, readdir, ...

#else
#error "Unsupported  version"

#endif // _OS_VER

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard-Core Debug Mode
#endif

#include <com/ifmacro.h>

//----------------------------------------------------------------------------
//
// Struct-
//       FileListObject
//
// Purpose-
//       The hidden FileList object
//
//----------------------------------------------------------------------------
struct FileListObject               // The hidden FileList object
{
   DIR*                dirStream;   // The directory stream
   dirent*             dirEntry;    // The current directory entry
   const char*         qualifier;   // The qualifier name
}; // struct FileListObject

//----------------------------------------------------------------------------
//
// Subroutine-
//       ::osUpper
//
// Purpose-
//       For operating systems that do not support case, toupper
//
//----------------------------------------------------------------------------
static inline int                   // OS-dependent toupper
   osUpper(                         // OS-dependent toupper
     int               __char)      // -> Qualifier name
{
   #if defined(_OS_WIN) || defined(_OS_CYGWIN)
     __char= toupper(__char);
   #endif

   return __char;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       ::isWildMatch
//
// Purpose-
//       Determine filename match, with wildcards
//
//----------------------------------------------------------------------------
int                                 // TRUE if filename qualifies
   isWildMatch(                     // Does the current filename qualify?
     const char*       ptrQual,     // -> Qualifier name
     const char*       ptrName)     // -> True file name
{
   for(;;)
   {
     if( *ptrQual == '?' )          // Single character match
     {
       if( *ptrName == '\0' )
         return FALSE;
       ptrQual++;
       ptrName++;
       continue;
     }

     if( *ptrQual == '*' )          // Mutiple character match
     {
       while( *ptrQual == '*' )
         ptrQual++;

       if( *ptrQual == '\0' )
         return TRUE;

       for(;;)
       {
         while( *ptrName != *ptrQual )
         {
           if( *ptrName == '\0' )
             return FALSE;
           ptrName++;
         }

         if( isWildMatch(ptrQual, ptrName) )
           return TRUE;
         ptrName++;
       }
     }

     if( osUpper(*ptrQual) != osUpper(*ptrName) ) // If not character match
       return FALSE;

     if( *ptrQual == '\0' )
       break;

     ptrQual++;
     ptrName++;
   }

   return TRUE;
}

//----------------------------------------------------------------------------
//
// Method-
//       FileList::~FileList
//
// Purpose-
//       Destructor
//
//----------------------------------------------------------------------------
   FileList::~FileList(void)        // Default destructor
{
   IFHCDM( debugf("FileList(%p)::~FileList()\n", this); )

   reset();
}

//----------------------------------------------------------------------------
//
// Method-
//       FileList::FileList
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
   FileList::FileList( void )       // Default constructor
:  object(NULL)
{
   IFHCDM( debugf("FileList(%p)::FileList()\n", this); )

}

   FileList::FileList(              // Standard constructor
     const char*       filePath,    // File Path
     const char*       fileName)    // File Name (may contain wildcards)
:  object(NULL)
{
   IFHCDM( debugf("FileList(%p)::FileList(%s,%s)\n", this, filePath, fileName); )

   reset(filePath, fileName);
}

//----------------------------------------------------------------------------
//
// Method-
//       FileList::getCurrent
//
// Purpose-
//       Extract the current filename.
//
//----------------------------------------------------------------------------
const char*                         // The current name qualifier (if any)
   FileList::getCurrent(void)       // Extract the current name qualifier
{
   IFHCDM( debugf("FileList(%p)::getCurrent()...\n", this); )

   FileListObject* O= (FileListObject*)object; // -> FileListObject
   if( O == NULL )                  // If no current object
     return NULL;

   if( O->dirStream == NULL )       // If no current name
     return NULL;

   if( strcmp(O->dirEntry->d_name, ".") == 0
       || strcmp(O->dirEntry->d_name, "..") == 0 )
     return getNext();

   IFHCDM( debugf("%s= FileList(%p)::getCurrent()\n", O->dirEntry->d_name, this); )
   return O->dirEntry->d_name;      // Return the file name
}

//----------------------------------------------------------------------------
//
// Method-
//       FileList::getNext
//
// Purpose-
//       Set the next filename in the list.
//
//----------------------------------------------------------------------------
const char*                         // The next name qualifier (if any)
   FileList::getNext(void)          // Extract the next name qualifier
{
   const char*         ptrQual;     // -> Qualifier name
   const char*         ptrName;     // -> Qualifier name
   char*               ptrC;        // -> Character

   int                 i;

   IFHCDM( debugf("FileList(%p)::getNext()...\n", this); )

   FileListObject* O= (FileListObject*)object; // -> FileListObject
   if( O == NULL )                  // If no current object
     return NULL;

   if( O->dirStream == NULL )
     return NULL;

   ptrQual= O->qualifier;
   for(;;)                          // Read and qualify file name
   {
     O->dirEntry= readdir(O->dirStream); // Locate the next file
     if( O->dirEntry == NULL )      // If file not found
     {
       closedir(O->dirStream);      // Close the directory stream
       O->dirStream= NULL;          // Indicate closed
       return NULL;                 // File not found
     }

     ptrName= O->dirEntry->d_name;
     ptrC= O->dirEntry->d_name;
     for(i=0; ptrC[i] != '\0'; i++)
     {
       if( ptrC[i] == '/' )
         ptrName= &ptrC[i+1];
     }

     if( strcmp(ptrName, ".") != 0 && strcmp(ptrName, "..") != 0
         && isWildMatch(ptrQual, ptrName) ) // If filename match
       break;
   }

   IFHCDM( debugf("%s= FileList(%p)::getNext()\n", O->dirEntry->d_name, this); )
   return O->dirEntry->d_name;      // Return the file name
}

//----------------------------------------------------------------------------
//
// Method-
//       FileList::reset
//
// Purpose-
//       Reset the FileList object
//
//----------------------------------------------------------------------------
void
   FileList::reset( void )          // Reset the FileList object
{
   IFHCDM( debugf("FileList(%p)::reset()\n", this); )

   FileListObject* O= (FileListObject*)object; // -> FileListObject
   if( O == NULL )                  // If the FileListObject does not exist
     return;                        // Nothing more to do

   if( O->dirStream != NULL )       // If the stream is currently open
   {
     closedir(O->dirStream);        // Close the directory stream
     O->dirStream= NULL;            // Indicate closed
   }

   delete O;
   object= NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       FileList::reset
//
// Purpose-
//       Set the filepath and filename.  The filename may have wildcards.
//
//----------------------------------------------------------------------------
const char*                         // The first file name
   FileList::reset(                 // Set the file name
     const char*       filePath,    // File Path
     const char*       fileName)    // File Name (may contain wildcards)
{
   IFHCDM( debugf("FileList(%p)::reset(%s,%s)\n", this, filePath, fileName); )

   // Initialization
   FileListObject* O= (FileListObject*)object; // -> FileListObject
   if( O == NULL )                  // If the FileListObject does not exist
   {
     O= new FileListObject();       // Allocate the object
     if( O == NULL )
       return NULL;

     object= O;                     // Indicate created
     O->dirStream= NULL;            // Indicate stream closed
   }

   // Initialize for a new file name
   if( O->dirStream != NULL )       // If the handle is currently open
   {
     closedir(O->dirStream);        // Close the directory stream
     O->dirStream= NULL;            // Indicate closed
   }

   // Validate the name
   O->qualifier= fileName;
   O->dirStream= opendir(filePath);

   return getNext();                // Return the file name
}

