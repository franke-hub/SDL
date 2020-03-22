//----------------------------------------------------------------------------
//
//       Copyright (C) 2004 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Accumulator.cpp
//
// Purpose-
//       Exon/Intron DataBase Loader object methods.
//
// Last change date-
//       2004/08/08
//
// Desciption-
//       The Accumulator accumulates an Exon/Intron database line.  A database
//       item consists of a descriptor (label) and multiple lines of character
//       data.  A LabelAccumulator ignores the data and other Accumulators
//       ignore the label.  See the constructor for each Accumulator for a
//       description of what is accumulated.
//
//       This source contains code for all the Accumulator types.  All types
//       are compiled, the caller selects which type is used by constructing
//       the required type.
//
//----------------------------------------------------------------------------
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "Accumulator.h"
#include "common.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "DBLOADER" // Source file

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define LINE_SIZE        0x00100000 // Maximum size of a single line

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
// Local data areas
//----------------------------------------------------------------------------
static const char*     iValid= "actg.nyrmwskbdvh";
static const char*     eValid= "ACTG+NYRMWSKBDVH";

//----------------------------------------------------------------------------
//
// Method-
//       Accumulator::~Accumulator
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Accumulator::~Accumulator( void )  // Destructor
{
   if( text != NULL )
   {
     free(text);
     text= NULL;
   }

   close();
}

//----------------------------------------------------------------------------
//
// Method-
//       Accumulator::Accumulator
//
// Purpose-
//       Constructor.
//
// Desciption-
//       The Accumulator is the base class for other Accumulators.  This
//       consists of the base code used for reading the database.
//
//----------------------------------------------------------------------------
   Accumulator::Accumulator( void )   // Constructor
:  text(NULL)
,  handle(NULL)
,  fileName(NULL)
,  lineNumber(0)
,  warnings(0)
{
   text= (char*)malloc(LINE_SIZE);
   if( text == NULL )
   {
     fprintf(stderr, "No Storage(%u)\n", LINE_SIZE);
     throw "NoStorageException";
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Accumulator::getLineNumber
//
// Purpose-
//       Return the current line number.
//
//----------------------------------------------------------------------------
unsigned                            // Resultant
   Accumulator::getLineNumber( void ) const // Get the current line number
{
   return lineNumber;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Accumulator::getWarnings
//
// Purpose-
//       Return the number of warnings.
//
//----------------------------------------------------------------------------
unsigned                            // Resultant
   Accumulator::getWarnings( void ) const // Get the warning counter
{
   return warnings;
}

//----------------------------------------------------------------------------
//
// Method-
//       Accumulator::close
//
// Purpose-
//       Close the database.
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   Accumulator::close( void )       // Close the database
{
   int                 result;      // Resultant

   if( handle == NULL )
     return 0;

   result= fclose(handle);
   if( result != 0 )
   {
     fprintf(stderr, "File(%s), close fault\n", fileName);
     warnings++;
   }

   handle= NULL;

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Accumulator::open
//
// Purpose-
//       Open the database.
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   Accumulator::open(               // Open the database
     const char*       fileName)    // File Name
{
   close();
   this->fileName= fileName;
   lineNumber= 0;
   warnings=   0;

   handle= fopen(fileName, "rb");
   if( handle == NULL )
   {
     sprintf(text, "Error: fopen(%s) failed: ", fileName);
     perror(text);
     return (-3);
   }

   return 0;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Accumulator::read
//
// Purpose-
//       Read one line, discarding trailing '\n'
//
//----------------------------------------------------------------------------
char*                               // Resultant
   Accumulator::read(               // Read one line
     char*             line,        // -> Working line
     unsigned          size)        // Sizeof(*line)
{
   char*               C;           // -> String
   int                 L;           // Length

   C= fgets(line, size, handle);
   if( C != NULL )
   {
     L= strlen(line);
     while( L > 0 )
     {
       if( line[L-1] != '\r' && line[L-1] != '\n' )
         break;

       L--;
       line[L]= '\0';
     }

     lineNumber++;
   }

   return C;
}

//----------------------------------------------------------------------------
//
// Method-
//       DataAccumulator::~DataAccumulator
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   DataAccumulator::~DataAccumulator( void )  // Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       DataAccumulator::DataAccumulator
//
// Purpose-
//       Constructor.
//
// Desciption-
//       The DataAccumulator accumulates an Exon/Intron database line,
//       discarding the label portion.  Exons and Introns are included
//       in the resultant data.
//
//----------------------------------------------------------------------------
   DataAccumulator::DataAccumulator( void )  // Constructor
:  Accumulator()
{
}

//----------------------------------------------------------------------------
//
// Method-
//       DataAccumulator::load
//
// Purpose-
//       Accumulate the next database data item.
//
//----------------------------------------------------------------------------
char*                               // -> Accumulated line
   DataAccumulator::load( void )    // Load the Accumulator
{
   int                 wCount;      // Number of warnings for this line

   char*               ptrL;        // -> Last input line
   int                 LL;          // Line length
   char                string[128]; // Temporary input line
   int                 TL;          // Total Line length

   // If nothing to read
   if( handle == NULL )
     return NULL;

   wCount= 0;
   // Accumulate one line
   for(;;)
   {
     ptrL= read(text, LINE_SIZE);
     if( ptrL == NULL )
       return NULL;

     if( *ptrL != '>' )
     {
       if( wCount == 0 )
         fprintf(stderr, "Line %d: Missing '>'\n", lineNumber);
       wCount++;
       warnings++;
       continue;
     }

     // Accumulate the line
     text[0]= '\0';
     TL= 0;
     for(;;)
     {
       ptrL= read(string, sizeof(string));
       if( ptrL == NULL )
         break;

       LL= strlen(string);
       if( LL == 0 )
         break;

       if( (TL + LL) > LINE_SIZE )
       {
         if( wCount == 0 )
           fprintf(stderr, "Line %d: Too long\n", lineNumber);
         wCount++;
         warnings++;
         continue;
       }

       strcat(text, string);
       TL += LL;
     }

     if( TL == 0 )
     {
       if( wCount == 0 )
         fprintf(stderr, "Line %d: Empty\n", lineNumber);
       wCount++;
       warnings++;
       continue;
     }

     //-----------------------------------------------------------------------
     // We have loaded one line.
     //-----------------------------------------------------------------------
     break;
   }

   return text;
}

//----------------------------------------------------------------------------
//
// Method-
//       LabelAccumulator::~LabelAccumulator
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   LabelAccumulator::~LabelAccumulator( void )  // Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       LabelAccumulator::LabelAccumulator
//
// Purpose-
//       Constructor.
//
// Desciption-
//       The LabelAccumulator accumulates an Exon/Intron database line,
//       discarding the data portion.  It is the inverse of a DataAccumulator.
//
//----------------------------------------------------------------------------
   LabelAccumulator::LabelAccumulator( void )  // Constructor
:  Accumulator()
{
}

//----------------------------------------------------------------------------
//
// Method-
//       LabelAccumulator::load
//
// Purpose-
//       Accumulate the next database label item.
//
//----------------------------------------------------------------------------
char*                               // -> Accumulated line
   LabelAccumulator::load( void )   // Load the Accumulator
{
   int                 wCount;      // Number of warnings for this line

   char*               ptrL;        // -> Last input line
   int                 LL;          // Line length
   char                string[128]; // Temporary input line

   // If nothing to read
   if( handle == NULL )
     return NULL;

   wCount= 0;
   // Accumulate one line
   for(;;)
   {
     ptrL= read(text, LINE_SIZE);
     if( ptrL == NULL )
       return NULL;

     if( *ptrL != '>' )
     {
       if( wCount == 0 )
         fprintf(stderr, "Line %d: Missing '>'\n", lineNumber);
       wCount++;
       warnings++;
       continue;
     }

     // Ignore the data
     for(;;)
     {
       ptrL= read(string, sizeof(string));
       if( ptrL == NULL )
         break;

       LL= strlen(string);
       if( LL == 0 )
         break;
     }

     //-----------------------------------------------------------------------
     // We have loaded one line.
     //-----------------------------------------------------------------------
     break;
   }

   return text;
}

//----------------------------------------------------------------------------
//
// Method-
//       ExonAccumulator::~ExonAccumulator
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   ExonAccumulator::~ExonAccumulator( void )  // Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       ExonAccumulator::ExonAccumulator
//
// Purpose-
//       Constructor.
//
// Desciption-
//       The DataAccumulator accumulates an Exon/Intron database line,
//       discarding the label portion.  Only the Exon portion of the database
//       is included and it's accumulated as a single item.
//
//----------------------------------------------------------------------------
   ExonAccumulator::ExonAccumulator( void )  // Constructor
:  Accumulator()
{
}

//----------------------------------------------------------------------------
//
// Method-
//       ExonAccumulator::load
//
// Purpose-
//       Accumulate the next database data item.
//
//----------------------------------------------------------------------------
char*                               // -> Accumulated line
   ExonAccumulator::load( void )    // Load the Accumulator
{
   int                 wCount;      // Number of warnings for this line

   char*               ptrL;        // -> Last input line
   int                 LL;          // Line length
   char                string[128]; // Temporary input line
   int                 TL;          // Total Line length

   // If nothing to read
   if( handle == NULL )
     return NULL;

   wCount= 0;
   // Accumulate one line
   for(;;)
   {
     ptrL= read(text, LINE_SIZE);
     if( ptrL == NULL )
       return NULL;

     if( *ptrL != '>' )
     {
       if( wCount == 0 )
         fprintf(stderr, "Line %d: Missing '>'\n", lineNumber);
       wCount++;
       warnings++;
       continue;
     }

     // Accumulate the line
     TL= 0;
     for(;;)
     {
       ptrL= read(string, sizeof(string));
       if( ptrL == NULL )
         break;

       LL= strlen(ptrL);
       if( LL == 0 )
         break;

       if( (TL + LL) > LINE_SIZE )
       {
         if( wCount == 0 )
           fprintf(stderr, "Line %d: Too long\n", lineNumber);
         wCount++;
         warnings++;
         continue;
       }

       while( *ptrL != '\0' )
       {
         if( strchr(eValid, *ptrL) != NULL )
           text[TL++]= *ptrL;

         ptrL++;
       }
     }

     text[TL]= '\0';
     if( TL == 0 )
     {
       if( wCount == 0 )
         fprintf(stderr, "Line %d: Empty\n", lineNumber);
       wCount++;
       warnings++;
       continue;
     }

     //-----------------------------------------------------------------------
     // We have loaded one line.
     //-----------------------------------------------------------------------
     break;
   }

   return text;
}

//----------------------------------------------------------------------------
//
// Method-
//       IntronAccumulator::~IntronAccumulator
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   IntronAccumulator::~IntronAccumulator( void )  // Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       IntronAccumulator::IntronAccumulator
//
// Purpose-
//       Constructor.
//
// Desciption-
//       The DataAccumulator accumulates an Exon/Intron database line,
//       discarding the label portion.  Only the Intron portion of the
//       database is included and it's accumulated as a single item.
//
//----------------------------------------------------------------------------
   IntronAccumulator::IntronAccumulator( void )  // Constructor
:  Accumulator()
{
}

//----------------------------------------------------------------------------
//
// Method-
//       IntronAccumulator::load
//
// Purpose-
//       Accumulate the next database data item.
//
//----------------------------------------------------------------------------
char*                               // -> Accumulated line
   IntronAccumulator::load( void )  // Load the Accumulator
{
   int                 wCount;      // Number of warnings for this line

   char*               ptrL;        // -> Last input line
   int                 LL;          // Line length
   char                string[128]; // Temporary input line
   int                 TL;          // Total Line length

   // If nothing to read
   if( handle == NULL )
     return NULL;

   wCount= 0;
   // Accumulate one line
   for(;;)
   {
     ptrL= read(text, LINE_SIZE);
     if( ptrL == NULL )
       return NULL;

     if( *ptrL != '>' )
     {
       if( wCount == 0 )
         fprintf(stderr, "Line %d: Missing '>'\n", lineNumber);
       wCount++;
       warnings++;
       continue;
     }

     // Accumulate the line
     TL= 0;
     for(;;)
     {
       ptrL= read(string, sizeof(string));
       if( ptrL == NULL )
         break;

       LL= strlen(ptrL);
       if( LL == 0 )
         break;

       if( (TL + LL) > LINE_SIZE )
       {
         if( wCount == 0 )
           fprintf(stderr, "Line %d: Too long\n", lineNumber);
         wCount++;
         warnings++;
         continue;
       }

       while( *ptrL != '\0' )
       {
         if( strchr(iValid, *ptrL) != NULL )
           text[TL++]= *ptrL;

         ptrL++;
       }
     }

     text[TL]= '\0';
     if( TL == 0 )
     {
       if( wCount == 0 )
         fprintf(stderr, "Line %d: Empty\n", lineNumber);
       wCount++;
       warnings++;
       continue;
     }

     //-----------------------------------------------------------------------
     // We have loaded one line.
     //-----------------------------------------------------------------------
     break;
   }

   return text;
}

