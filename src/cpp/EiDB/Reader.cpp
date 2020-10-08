//----------------------------------------------------------------------------
//
//       Copyright (C) 2003 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Reader.cpp
//
// Purpose-
//       Reader methods.
//
// Last change date-
//       2003/06/22
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>

#include "Reader.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "READER  " // Source filename

//----------------------------------------------------------------------------
//
// Method-
//       Reader::~Reader
//
// Purpose-
//       Destructor
//
//----------------------------------------------------------------------------
   Reader::~Reader( void )          // Destructor
{
   close();
}

//----------------------------------------------------------------------------
//
// Method-
//       Reader::Reader
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
   Reader::Reader( void )           // Default Constructor
:  fileName(NULL)
,  fileHandle(NULL)
,  fileLine(0)
{
}

//----------------------------------------------------------------------------
//
// Method-
//       Reader::open
//
// Purpose-
//       Activate the Reader
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   Reader::open(                    // Open the Reader
     const char*     fileName,      // The name of the File to be read
     unsigned        mode)          // The associated mode (ignored)
{
   (void)mode;                      // (Ignored)

   if( fileHandle != NULL )
     close();

   fileHandle= fopen(fileName, "rb");
   this->fileName= fileName;
   fileLine= 0;

   return (fileHandle == NULL);
}

//----------------------------------------------------------------------------
//
// Method-
//       Reader::close
//
// Purpose-
//       Deactivate the Reader
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   Reader::close( void )            // Close the Reader
{
   int                 result;

   result= 0;
   if( fileHandle != NULL )
   {
     result= fclose(fileHandle);
     fileHandle= NULL;
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Reader::readLine
//
// Purpose-
//       Read a line.
//
//----------------------------------------------------------------------------
int                                 // The line delimiter
   Reader::readLine(                // Read a line
     char*           addr,          // Data address
     unsigned        size)          // Data length
{
   int               result;
   char*             string;
   int               length;

   if( fileHandle == NULL || size == 0 )
     return EOF;

   string= fgets(addr, size, fileHandle);
   if( string == NULL )
     return EOF;

   result= '\0';
   length= strlen(addr);
   if( length > 0 )
     result= addr[length-1] & 0x00ff;

   while( length > 0 )
   {
     if( addr[length-1] != '\r' && addr[length-1] != '\n' )
       break;

     length--;
     addr[length]= '\0';
   }

   fileLine++;
   return result;
}

