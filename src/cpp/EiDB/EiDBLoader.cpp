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
//       EiDBLoader.cpp
//
// Purpose-
//       Exon/Intron DataBase Loader object methods.
//
// Last change date-
//       2003/03/16
//
// Description-
//       This routine is the glue code which loads an in-storage Exon/Intron
//       database.  It uses the EiDB (database) object, an Accumulator object
//       and an Extractor object to control its operation.
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "Accumulator.h"
#include "common.h"
#include "EiDB.h"
#include "Extractor.h"

#include "EiDBLoader.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "DBLOADER" // Source file

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define MAX_WARNING              10 // Maximum number of printed warnings

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE  1
#endif

//----------------------------------------------------------------------------
//
// Method-
//       EiDBLoader::~EiDBLoader
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   EiDBLoader::~EiDBLoader( void )  // Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       EiDBLoader::EiDBLoader
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   EiDBLoader::EiDBLoader( void )   // Constructor
:  ignoreFirst(FALSE)
,  ignoreLast(FALSE)
,  ignoreOnly(FALSE)
,  maxSize(0)
,  minSize(0)
{
}

//----------------------------------------------------------------------------
//
// Method-
//       EiDBLoader::setIgnoreFirst
//
// Purpose-
//       Set the ignoreFirst control
//
//----------------------------------------------------------------------------
void
   EiDBLoader::setIgnoreFirst(      // Set ignore first item control
     int               mode)        // To this value
{
   ignoreFirst= mode;
}

//----------------------------------------------------------------------------
//
// Method-
//       EiDBLoader::setIgnoreLast
//
// Purpose-
//       Set the ignoreLast control
//
//----------------------------------------------------------------------------
void
   EiDBLoader::setIgnoreLast(       // Set ignore last item control
     int               mode)        // To this value
{
   ignoreLast= mode;
}

//----------------------------------------------------------------------------
//
// Method-
//       EiDBLoader::setIgnoreOnly
//
// Purpose-
//       Set the ignoreOnly control
//
//----------------------------------------------------------------------------
void
   EiDBLoader::setIgnoreOnly(       // Set ignore only item control
     int               mode)        // To this value
{
   ignoreOnly= mode;
}

//----------------------------------------------------------------------------
//
// Method-
//       EiDBLoader::setMaxSize
//
// Purpose-
//       Set maximum column size.
//
//----------------------------------------------------------------------------
void
   EiDBLoader::setMaxSize(          // Set maximum column size
     int               size)        // To this value
{
   maxSize= size;
}

//----------------------------------------------------------------------------
//
// Method-
//       EiDBLoader::setMinSize
//
// Purpose-
//       Set minimum column size.
//
//----------------------------------------------------------------------------
void
   EiDBLoader::setMinSize(          // Set minimum column size
     int               size)        // To this value
{
   minSize= size;
}

//----------------------------------------------------------------------------
//
// Method-
//       EiDBLoader::load
//
// Purpose-
//       Load the database.
//
// Return code-
//       >0 Database loaded with errors
//       =0 Database loaded, no errors
//       <0 Database not loaded due to errors
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   EiDBLoader::load(                // Load the database
     EiDB&             eidb,        // The database to load
     Accumulator&      accumulator, // The Accumulator
     Extractor&        extractor,   // The Extractor
     LOADMODE          loadMode)    // Load Mode
{
   int                 result;      // Resultant
   int                 warnings;    // Number of warnings

   int                 is_valid;    // TRUE if valid exon
   unsigned            lineNo;      // Current line number
   char*               ptrL;        // -> Line
   unsigned            size;        // strlen(ptrL)

   // Initialize
   if( loadMode != MODE_LEFTRIGHT && loadMode != MODE_RIGHTLEFT)
     return (-1);                   // Parameter error

   extractor.setIgnoreFirst(ignoreFirst); // Propagate ignores
   extractor.setIgnoreLast(ignoreLast);
   extractor.setIgnoreOnly(ignoreOnly);

   // Load the database
   warnings= 0;
   result= 0;
   for(;;)
   {
     ptrL= accumulator.load();
     if( ptrL == NULL )
       break;

     lineNo= accumulator.getLineNumber();
     extractor.load(ptrL);
     for(;;)
     {
       // Extract Items
       ptrL= extractor.next(lineNo);
       if( ptrL == NULL )
         break;

       // Insert the Exon into the database
       size= strlen(ptrL);
       is_valid= TRUE;
       if( maxSize > 0 && size > maxSize )
         is_valid= FALSE;
       if( minSize > 0 && size < minSize )
         is_valid= FALSE;

       if( is_valid )
       {
         // Insert the line into the database
         if( loadMode == MODE_RIGHTLEFT )
           strrev(ptrL);
         if( signed(eidb.putLine(ptrL)) < 0 )
         {
           if( warnings < MAX_WARNING )
             fprintf(stderr, "Line %d: No storage\n", lineNo);

           warnings++;
           result= 1;
         }
       }
     }
   }

   // File has been read
   accumulator.close();

   warnings += accumulator.getWarnings();
   warnings += extractor.getWarnings();
   fprintf(stderr, "Load complete, %d warnings\n", warnings);
   return result;
}

