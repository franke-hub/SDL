//----------------------------------------------------------------------------
//
//       Copyright (c) 2014-2020 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Archive.cpp
//
// Purpose-
//       Implement the Archive object.
//
// Last change date-
//       2020/10/02
//
//----------------------------------------------------------------------------
#define _FILE_OFFSET_BITS 64
#define __STDC_FORMAT_MACROS        // For linux inttypes.h

#include <assert.h>
#include <inttypes.h>               // For PRId64
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <zlib.h>
#include <sys/stat.h>

#include <com/AutoPointer.h>
#include <com/Calendar.h>           // For FAT_DATE_TIME
#include <com/Clock.h>              // For FAT_DATE_TIME
#include <com/CRC32.h>
#include <com/Debug.h>
#include <com/FileInfo.h>           // For PathArchive
#include <com/FileList.h>           // For PathArchive
#include <com/FileName.h>
#include <com/FileSource.h>
#include <com/istring.h>
#include <com/List.h>

#include "com/Archive.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#define HCDM                        // If defined, Hard-Core Debug Mode
#endif

#include <com/ifmacro.h>

enum COMP_MODE                      // Compression mode
{  COMP_NONE                        // No compression
,  COMP_ZLIB= Z_DEFLATED            // ZLIB compression
}; // enum COMP_MODE

//----------------------------------------------------------------------------
// Class definition/implementation includes
//----------------------------------------------------------------------------
#include "UtilArchive.h"            // Must be first: defines utilities

#include "BzipArchive.h"            // BZIP2 (single file)
#include "DiskArchive.h"            // (Disk resident, TAR format)
#include "GzipArchive.h"            // GZIP  (single file)
#include "Zz32Archive.h"            // ZIP32 (multi-file)
// clude "Zz64Archive.h"            // ZIP64 (multi-file) ** NOT IMPLEMENTED **
#include "_tbzArchive.h"            // .tar.bz archive
#include "_tgzArchive.h"            // .tar.gz archive

//----------------------------------------------------------------------------
//
// Method-
//       Archive::~Archive
//
// Purpose-
//       Destructor
//
// Implementation notes-
//       The source Archive is deleted if present.
//       IGNORED fields: name, origin.
//
//----------------------------------------------------------------------------
   Archive::~Archive( void )        // Destructor
{
   if( file != NULL )
   {
     delete file;
     file= NULL;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Archive::Archive
//
// Purpose-
//       Default constructor
//
//----------------------------------------------------------------------------
   Archive::Archive( void )         // Default constructor
:  DataSource()
,  file(NULL)
,  mode(0)
,  time(0)
,  object(0)
{
}

//----------------------------------------------------------------------------
//
// Method-
//       Archive::make
//
// Function-
//       Create an Archive
//
// Implementation note-
//       First attempt to select using the file name.
//       If this fails, try each known method using magic number matching.
//
//----------------------------------------------------------------------------
Archive*                            // Resultant Archive
   Archive::make(                   // Create an Archive
     DataSource*       file)        // From this DataSource
{
   if( file == NULL )
     return NULL;

   const char* full= file->getName().c_str(); // Get file name
   const char* name= FileName::getExtension(full); // Get name extension

   // TBZ format is only valid by name
   int isTAR= FALSE;
   if( stricmp(name, ".tbz") == 0 || stricmp(name, ".tbz2") == 0 )
     isTAR= TRUE;
   else
   {
     unsigned L= strlen(full);
     if( L > 8 )
     {
       L -= 8;
       full += L;
       if( stricmp(full, ".tar.bz2") == 0 )
         isTAR= TRUE;
       else if( stricmp(full+1, ".tar.bz") == 0 )
         isTAR= TRUE;
     }
   }
   if( isTAR )
   {
     Archive* archive= _tbzArchive::make(file);
     if( archive != NULL )
       return archive;
   }

   // TGZ format is only valid by name
   isTAR= FALSE;
   if( stricmp(name, ".tgz") == 0 )
     isTAR= TRUE;
   else
   {
     unsigned L= strlen(full);
     if( L > 7 )
     {
       L -= 7;
       full += L;
       if( stricmp(full, ".tar.gz") == 0 )
         isTAR= TRUE;
     }
   }
   if( isTAR )
   {
     Archive* archive= _tgzArchive::make(file);
     if( archive != NULL )
       return archive;
   }

   // If BZIP file name extention (BZIP processed only by name)
   if( stricmp(name, ".bz") == 0 || stricmp(name, ".bz2") == 0 )
   {
     Archive* archive= BzipArchive::make(file);
     if( archive != NULL )
       return archive;
   }

   // If GZIP file name extension (GZIP processed only by name)
   if( stricmp(name, ".gz") == 0 )
   {
     Archive* archive= GzipArchive::make(file);
     if( archive != NULL )
       return archive;
   }

   if( stricmp(name, ".tar") == 0 ) // If TAR file name extension
   {
     Archive* archive= DiskArchive::make(file);
     if( archive != NULL )
       return archive;
   }

   if( stricmp(name, ".zip") == 0 ) // If ZIP file name extension
   {
     Archive* archive= Zz32Archive::make(file);
     if( archive != NULL )
       return archive;

//   // Zz64Archive NOT IMPLEMENTED
//   archive= Zz64Archive::make(file);
//   if( archive != NULL )
//     return archive;
   }

   // Select using trial and error for magic number matching
   if( stricmp(name, ".zip") != 0 ) // If not ZIP file name extension
   {
     Archive* archive= Zz32Archive::make(file); // Most likely
     if( archive != NULL )
       return archive;

//   // Zz64Archive NOT IMPLEMENTED
//   archive= Zz64Archive::make(file);
//   if( archive != NULL )
//     return archive;
   }

   if( stricmp(name, ".tar") != 0 ) // If not TAR file name extension
   {
     Archive* archive= DiskArchive::make(file);
     if( archive != NULL )
       return archive;
   }

   return NULL;                     // No match found
}

Archive*                            // Resultant Archive
   Archive::make(                   // Create an Archive
     const char*       name)        // For this file name
{
   Archive*            archive= NULL; // The resultant Archive
   DataSource*         file= NULL;  // The source Archive

   try {
     file= new FileSource(name);

     archive= make(file);
     if( archive == NULL )
       delete file;
   } catch(...) {
     if( file != NULL )
       delete file;

     archive= NULL;
   }

   return archive;
}

//----------------------------------------------------------------------------
//
// Method-
//       Archive::setOffset
//
// Function-
//       setOffset within current item.
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   Archive::setOffset(              // Set position
     size_t            offset)      // Offset
{
   if( offset > length )
     return (-1);

   this->offset= offset;
   return 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       Archive::take
//
// Function-
//       Return DataSource and delete this Archive.
//
//----------------------------------------------------------------------------
DataSource*                         // The DataSource
   Archive::take( void )            // Take back control of DataSource
{
   DataSource* result= file;        // The DataSource

   file= NULL;                      // Do not delete the DataSource
   delete this;                     // Delete this Archive

   return result;                   // Return the DataSource
}

//----------------------------------------------------------------------------
//
// Method-
//       Archive::index
//
// Function-
//       Select an item by index
//
//----------------------------------------------------------------------------
const char*                         // The item name (ALWAYS NULL)
   Archive::index(                  // Select
     unsigned int      index)       // This item index
{
   (void)index;                     // Unused in base class
   return NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       Archive::next
//
// Function-
//       Select the next item
//
//----------------------------------------------------------------------------
const char*                         // The item name (ALWAYS NULL)
   Archive::next( void )            // Select the next item
{
   return NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       Archive::read
//
// Function-
//       Read from current item.
//
//----------------------------------------------------------------------------
unsigned int                        // The number of bytes read
   Archive::read(                   // Read from current item
     void*             addr,        // Into this buffer address
     unsigned int      size)        // For this length
{
   if( origin == NULL )
     return 0;

   if( (offset + size) > length )
     size= length - offset;

   if( size > 0 )
   {
     memcpy(addr, origin + offset, size);
     offset += size;
   }

   return size;
}

