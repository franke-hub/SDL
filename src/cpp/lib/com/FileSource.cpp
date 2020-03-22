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
//       FileSource.cpp
//
// Purpose-
//       FileSource implementation methods.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <com/FileName.h>

#ifdef _OS_WIN
  #include <com/nativeio.h>
#else
  #include <sys/mman.h>
#endif

#include "com/FileSource.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef SCDM
#undef  SCDM                        // If defined, Soft Core Debug Mode
#endif

//----------------------------------------------------------------------------
//
// Method-
//       FileSource::~FileSource
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   FileSource::~FileSource( void )  // Destructor
{
   close();
}

//----------------------------------------------------------------------------
//
// Method-
//       FileSource::FileSource
//
// Purpose-
//       Default constructor.
//
//----------------------------------------------------------------------------
   FileSource::FileSource( void )   // Default constructor
:  DataSource()
,  handle(-1)
{
}

//----------------------------------------------------------------------------
//
// Method-
//       FileSource::FileSource
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   FileSource::FileSource(          // Constructor
     const char*       fileName)    // For this file name
:  DataSource()
,  handle(-1)
{
   open(fileName);
}

//----------------------------------------------------------------------------
//
// Method-
//       FileSource::clone
//
// Purpose-
//       Clone this DataSource.
//
//----------------------------------------------------------------------------
DataSource*                         // Resultant DataSource
   FileSource::clone(               // Clone this DataSource
     const char*       name) const  // With this (relative) name
{
   FileSource*         result= NULL;// Resultant

   char                newName[FILENAME_MAX+1]; // The new file name
   char                oldPath[FILENAME_MAX+1]; // The old file path

   FileName            oldName(this->name.c_str());

   if( oldName.resolve() != NULL )
     return NULL;

   if( oldName.getPathOnly(oldPath) == NULL )
     return NULL;

   if( FileName::concat(newName, oldPath, name) == NULL )
     return NULL;

   result= new FileSource(newName);
   if( result->handle < 0 )
   {
     delete result;
     result= NULL;
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       FileSource::close
//
// Purpose-
//       Unmap the file.
//
//----------------------------------------------------------------------------
void
   FileSource::close( void )        // Close the file
{
   #ifdef _OS_WIN
     if( origin != NULL )
       free(origin);

   #else
     if( origin != NULL )
       ::munmap(origin, length);
     if( handle >= 0 )
       ::close(handle);
   #endif

   handle= (-1);
   origin= NULL;
   reset();
}

//----------------------------------------------------------------------------
//
// Method-
//       FileSource::open
//
// Purpose-
//       Map the file.
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   FileSource::open(                // Open a file
     const char*       fileName)    // With this file name
{
   close();
   name= fileName;

   struct stat         stat_buf;
   if( ::stat(fileName, &stat_buf ) != 0 )
     return (-1);

   if( stat_buf.st_size == 0 )
     return (-1);

   handle= ::open(fileName, O_RDONLY);
   if( handle < 0 )
     return (-1);

   #ifdef _OS_WIN
     length= stat_buf.st_size;
     origin= (unsigned char*)malloc((int)length);
     if( origin != NULL )
     {
       int L= ::read(handle, origin, (int)length);
       if( L != length )
       {
         free(origin);
         origin= NULL;
       }
     }

     ::close(handle);
     if( origin == NULL )
     {
       length= 0;
       handle= (-1);
       return (-1);
     }

   #else
     length= stat_buf.st_size;
     origin= (unsigned char*)::mmap(0, length, PROT_READ, MAP_SHARED, handle, 0);
     if( origin == MAP_FAILED )
     {
       origin= NULL;
       close();
       return (-1);
     }
   #endif

   setWidth();
   return 0;
}

