//----------------------------------------------------------------------------
//
//       Copyright (c) 2010-2020 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       FileData.cpp
//
// Purpose-
//       FileData object methods.
//
// Last change date-
//       2020/10/03
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <com/define.h>
#include <com/Debug.h>
#include <com/FileInfo.h>
#include <com/Unconditional.h>

#include "com/FileData.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef SIZE_MAX // (For _OS_WIN
#define SIZE_MAX 0xffffffff
#endif

#define INVALID_SIZE SIZE_MAX
#define MAXIMUM_COMP 0x00100000

#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

//----------------------------------------------------------------------------
//
// Subroutine-
//       ::getSize
//
// Function-
//       Get the size of a file
//
//----------------------------------------------------------------------------
static int                          // Return code (0 OK)
   getSize(                         // Get the size of the file
     const char*       fileName,    // With this name
     unsigned long&    size)        // Resultant
{
   int                 result= (-1);// Resultant
   size= INVALID_SIZE;              // Default, invalid size

   if( fileName != NULL )
   {
     FileInfo info(fileName);

     uint64_t actual= info.getFileSize();
     if( actual < 0x0000000100000000LL )
     {
       size= int(actual);
       result= 0;
     }
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       FileData::~FileData
//
// Function-
//       Destructor
//
//----------------------------------------------------------------------------
   FileData::~FileData( void )      // Destructor
{
   reset();
}

//----------------------------------------------------------------------------
//
// Method-
//       FileData::FileData
//
// Function-
//       Constructor
//
//----------------------------------------------------------------------------
   FileData::FileData( void )       // Default constructor
:  name(NULL)
,  addr(NULL)
,  size(INVALID_SIZE)
{
}

   FileData::FileData(              // Constructor
     const char*       fileName)    // File name
:  name(NULL)
,  addr(NULL)
,  size(INVALID_SIZE)
{
   name= Unconditional::strdup(fileName);
   ::getSize(name, size);
}

//----------------------------------------------------------------------------
//
// Method-
//       FileData::getFileAddr
//
// Function-
//       Access the file data, read-only.
//
//----------------------------------------------------------------------------
const void*                         // The file data address
   FileData::getFileAddr( void )    // Get file data address
{
   if( addr == NULL )
   {
     if( ::getSize(name, size) == 0 )
     {
       if( size == 0 )
         addr= malloc(1);
       else
       {
         addr= malloc(size+1);
         if( addr != NULL )
         {
           FILE* file= fopen(name, "rb");
           if( file == NULL )
           {
             free(addr);
             addr= NULL;
           }
           else
           {
             int rc= fread(addr, size, 1, file);
             if( rc != 1 )
             {
               free(addr);
               addr= NULL;
             }
           }

           fclose(file);
         }
       }

       if( addr != NULL )           // Make text files into C strings
       {
         char* CV= (char*)addr;
         CV[size]= '\0';
       }
     }
   }

   return addr;
}

//----------------------------------------------------------------------------
//
// Method-
//       FileData::compare
//
// Function-
//       Compare two files
//
//----------------------------------------------------------------------------
int                                 // Resultant (<0, 0, >0)
   FileData::compare(               // Compare with
     FileData&         comprahend)  // Comprahend
{
   if( size != comprahend.size )
     return (size - comprahend.size);

   if( size <= MAXIMUM_COMP )
   {
     if( getFileAddr() == NULL || comprahend.getFileAddr() == NULL )
       return (+1);

     if( size != comprahend.size )  // The size might have changed!
       return( size - comprahend.size );

     if( size == 0 )
       return 0;

     return memcmp(addr, comprahend.addr, size);
   }

   int result= 0;                   // Default, result == 0
   void* cAddr= NULL;               // Comprahend read address
   FILE* cFile= NULL;               // Comprahend FILE*
   void* tAddr= NULL;               // This read address
   FILE* tFile= NULL;               // This FILE*
   try {
     tAddr= Unconditional::malloc(size);
     cAddr= Unconditional::malloc(size);

     cFile= fopen(comprahend.name, "rb");
     if( cFile == NULL )
       throw "fopen";

     tFile= fopen(name, "rb");
     if( tFile == NULL )
       throw "fopen";

     size_t length= size;
     while( result == 0 && length > 0 )
     {
       size_t tSize= fread(tAddr, 1, length, tFile);
       size_t cSize= fread(cAddr, 1, length, cFile);
       if( tSize != length || cSize != length )
         throw "fread";

       if( tSize != cSize )
         result= (tSize - cSize);
       else
         result= memcmp(tAddr, cAddr, tSize);

       length -= tSize;
     }
   #ifdef HCDM
   } catch(const char* x) {
     fprintf(stderr, "FileData::compare EX(%s)\n", x);
     result= (+1);
   #endif
   } catch(...) {
     result= (+1);
   }

   if( cFile != NULL )
     fclose(cFile);

   if( tFile != NULL )
     fclose(tFile);

   if( cAddr != NULL )
     free(cAddr);

   if( tAddr != NULL )
     free(tAddr);

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       FileData::reset
//
// Function-
//       Reset the FileData object
//
//----------------------------------------------------------------------------
void
   FileData::reset( void )          // Reset the FileData object
{
   if( addr != NULL )
   {
     free(addr);
     addr= NULL;
   }

   if( name != NULL )
   {
     free(name);
     name= NULL;
   }
}

void
   FileData::reset(                 // Reset the FileData obejct
     const char*       fileName)    // Using this FileName
{
   reset();
   name= Unconditional::strdup(fileName);
}

