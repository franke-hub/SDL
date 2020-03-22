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
//       Extractor.cpp
//
// Purpose-
//       Extractor object methods.
//
// Last change date-
//       2003/03/16
//
// Desciption-
//       The Extractor is a partner item to the Accumulator.
//
//       The Extractor extracts items of a particular type from an Accumulator
//       line.  A line is loaded into the Extractor and then its parts are
//       extracted using the rules associated with the paraticular Extractor
//       type.
//
//       This source contains code for all the Extractor types.  All types
//       are compiled, the caller selects which type is used by constructing
//       the required type.
//
//----------------------------------------------------------------------------
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "Extractor.h"
#include "common.h"
#include "Wildstr.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "EXTRACT " // Source file

#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

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
// Local data areas
//----------------------------------------------------------------------------
static const char*     iList= "actg.nyrmwskbdvh";
static const char*     eList= "ACTG.NYRMWSKBDVH";

//----------------------------------------------------------------------------
//
// Method-
//       Extractor::~Extractor
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Extractor::~Extractor( void )    // Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       Extractor::Extractor
//
// Purpose-
//       Constructor.
//
// Desciption-
//       The base Extractor contains the base Extractor functionality, and
//       also functions as a simple Extractor.
//
//       This Extractor returns the entire gene, ignoring the "ignoreFirst",
//       "ignoreLast" and "ignoreOnly" controls.
//       (If it didn't, it would never return anything if that flag was set.)
//
//----------------------------------------------------------------------------
   Extractor::Extractor( void )     // Constructor
:  gene(NULL)
,  geneIsFirst(FALSE)
,  ignoreFirst(FALSE)
,  ignoreLast(FALSE)
,  ignoreOnly(FALSE)
,  genewarns(0)
,  warnings(0)
{
}

//----------------------------------------------------------------------------
//
// Method-
//       Extractor::getWarnings
//
// Purpose-
//       Get the warning counter
//
//----------------------------------------------------------------------------
unsigned                            // The warning counter
   Extractor::getWarnings( void ) const // Get warning counter
{
   return warnings;
}

//----------------------------------------------------------------------------
//
// Method-
//       Extractor::setIgnoreFirst
//
// Purpose-
//       Set the ignoreFirst control
//
//----------------------------------------------------------------------------
void
   Extractor::setIgnoreFirst(       // Set ignore first item control
     int               mode)        // To this value
{
   ignoreFirst= mode;
}

//----------------------------------------------------------------------------
//
// Method-
//       Extractor::setIgnoreLast
//
// Purpose-
//       Set the ignoreLast control
//
//----------------------------------------------------------------------------
void
   Extractor::setIgnoreLast(        // Set ignore last item control
     int               mode)        // To this value
{
   ignoreLast= mode;
}

//----------------------------------------------------------------------------
//
// Method-
//       Extractor::setIgnoreOnly
//
// Purpose-
//       Set the ignoreOnly control
//
//----------------------------------------------------------------------------
void
   Extractor::setIgnoreOnly(        // Set ignore only item control
     int               mode)        // To this value
{
   ignoreOnly= mode;
}

//----------------------------------------------------------------------------
//
// Method-
//       Extractor::next
//
// Purpose-
//       Get the next Exon or intron, handling ignore controls.
//
//----------------------------------------------------------------------------
char*                               // Next Exon sequence
   Extractor::next(                 // Get next Exon/Intron
     unsigned          lineNo)      // Input line number (for messages)
{
   char*               result;      // Resultant

   // Debugging
   #if defined(HCDM)
     printf("%4d: Extractor.next(%d) isFirst(%d) ignore(%d,%d,%d) %s\n",
            __LINE__, lineNo, geneIsFirst,
            ignoreFirst, ignoreLast, ignoreOnly, gene);
   #endif

   result= getNext(lineNo);         // Get the nominal resultant

   //-------------------------------------------------------------------------
   // Account for ignore controls
   //   ignoreFirst: Ignore the first gene iff there is no preceding intron
   //   ignoreLast:  Ignore the last  gene iff there is no following intron
   //   ignoreOnly:  Invert the action of ignoreFirst and ignoreLast
   //-------------------------------------------------------------------------
   if( ignoreFirst && geneIsFirst && genewarns < 2 )
   {
     geneIsFirst= FALSE;
     if( !ignoreOnly )
       result= getNext(lineNo);
   }
   else
   {
     geneIsFirst= FALSE;

     if( ignoreOnly )
     {
       while( gene != NULL && result != NULL )
         result= getNext(lineNo);

       if( !ignoreLast )
         result= NULL;
     }

     if( ignoreLast && gene == NULL )
     {
       if( !ignoreOnly )
         result= NULL;
     }
   }

   #if defined(HCDM)
     printf("%4d: %s= Extractor.next()\n", __LINE__, result);
   #endif

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Extractor::load
//
// Purpose-
//       Load a new sequence
//
//----------------------------------------------------------------------------
void
   Extractor::load(                 // Load a new Exon/Intron sequence
     char*             gene)        // The new Exon/Intron sequence
{
   genewarns= 0;
   geneIsFirst= TRUE;
   this->gene= gene;
}

//----------------------------------------------------------------------------
//
// Method-
//       Extractor::getNext
//
// Purpose-
//       Extract the next item from the sequence.
//       (Here we return the entire sequence, ignoring ignore controls.)
//
//----------------------------------------------------------------------------
char*                               // Next item in sequence
   Extractor::getNext(              // Extract the next Item
     unsigned          lineNo)      // Input line number (for messages)
{
   char*               result;      // Resultant

   result= gene;
   gene= NULL;
   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       iValid
//
// Purpose-
//       Test for valid intron character.
//
//----------------------------------------------------------------------------
static inline int                   // TRUE iff valid
   iValid(                          // Test for valid intron character
     int               c)           // The character
{
   if( c == '\0' )
     return FALSE;

   return (strchr(iList,c) != NULL);
}

//----------------------------------------------------------------------------
//
// Method-
//       AtgExtractor::~AtgExtractor
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   AtgExtractor::~AtgExtractor( void )  // Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       AtgExtractor::AtgExtractor
//
// Purpose-
//       Constructor.
//
// Desciption-
//       This Extractor works like an ExonExtractor except that it begins
//       at the first "ATG" sequence.
//
//----------------------------------------------------------------------------
   AtgExtractor::AtgExtractor(      // Constructor
     int               sw_wild)     // TRUE iff wildcharacter matching applies
:  Extractor()
,  sw_wild(sw_wild)
{
}

//----------------------------------------------------------------------------
//
// Method-
//       AtgExtractor::getNext
//
// Purpose-
//       Extract the next Exon sequence.
//
//----------------------------------------------------------------------------
char*                               // Next Exon sequence
   AtgExtractor::getNext(           // Extract the next Exon
     unsigned          lineNo)      // Input line number (for messages)
{
   char*               result;      // Resultant
   char*               ptrC;        // -> Char (string)

   // Debugging
   #ifdef HCDM
     printf("%4d: AtgExtractor.getNext(%d)\n", __LINE__, lineNo);
   #endif

   // Handle empty condition
   if( gene == NULL )
     return NULL;

   // Locate beginning ATG sequence, if required
   if( geneIsFirst == TRUE )
   {
     if( sw_wild )
       gene= wildstr(gene, "ATG");
     else
       gene= strstr(gene, "ATG");
     #ifdef HCDM
       if( gene == NULL )
         printf("%4d: 'ATG' not found\n", __LINE__);
     #endif
   }

   // Strip leading delimiters
   if( gene == NULL )
     return NULL;

   result= gene;
   while( iValid(*result) )
   {
     geneIsFirst= FALSE;
     result++;
   }

   #ifdef HCDM
     printf("%4d: left: '%s'\n", __LINE__, result);
   #endif
   if( *result == '\0' )
     return NULL;

   // Find trailing delimiter
   ptrC= result;
   while( *ptrC == 'A' ||  *ptrC == 'C' || *ptrC == 'G' ||  *ptrC == 'T'
       || getWild(*ptrC) != NULL )
     ptrC++;

   // Validate trailing delimiter
   if( *ptrC != '\0' && !iValid(*ptrC) )
   {
     if( genewarns == 0 && warnings < MAX_WARNING )
       fprintf(stderr, "Line %u: Invalid char(%c)\n", lineNo, *ptrC);

     genewarns++;
     warnings++;
     *ptrC= '\0';
     ptrC++;

     while( *ptrC != 'A' &&  *ptrC != 'C' && *ptrC != 'G' &&  *ptrC != 'T'
         && *ptrC != '\0' && getWild(*ptrC) == NULL )
       ptrC++;
   }

   if( *ptrC == '\0' )
     gene= NULL;
   else
   {
     gene= ptrC + 1;
     *ptrC= '\0';
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       ExonExtractor::~ExonExtractor
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   ExonExtractor::~ExonExtractor( void )  // Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       ExonExtractor::ExonExtractor
//
// Purpose-
//       Constructor.
//
// Desciption-
//       This Extractor returns the next Exon in the sequence.
//
//----------------------------------------------------------------------------
   ExonExtractor::ExonExtractor( void )   // Constructor
:  Extractor()
{
}

//----------------------------------------------------------------------------
//
// Method-
//       ExonExtractor::getNext
//
// Purpose-
//       Extract the next Exon sequence.
//
//----------------------------------------------------------------------------
char*                               // Next Exon sequence
   ExonExtractor::getNext(          // Extract the next Exon
     unsigned          lineNo)      // Input line number (for messages)
{
   char*               result;      // Resultant
   char*               ptrC;        // -> Char (string)

   // Debugging
   #ifdef HCDM
     printf("%4d: ExonExtractor.getNext(%d) %s\n", __LINE__, lineNo, gene);
   #endif

   // Strip leading delimiters
   if( gene == NULL )
     return NULL;

   result= gene;
   while( iValid(*result) )
   {
     geneIsFirst= FALSE;
     result++;
   }

   if( *result == '\0' )
     return NULL;

   // Find trailing delimiter
   ptrC= result;
   while( *ptrC == 'A' ||  *ptrC == 'C' || *ptrC == 'G' ||  *ptrC == 'T'
       || getWild(*ptrC) != NULL )
     ptrC++;

   // Validate trailing delimiter
   if( *ptrC != '\0' && !iValid(*ptrC) )
   {
     if( genewarns == 0 && warnings < MAX_WARNING )
       fprintf(stderr, "Line %u: Invalid char(%c)\n", lineNo, *ptrC);

     genewarns++;
     warnings++;
     *ptrC= '\0';
     ptrC++;

     while( *ptrC != 'A' &&  *ptrC != 'C' && *ptrC != 'G' &&  *ptrC != 'T'
         && *ptrC != '\0' && getWild(*ptrC) == NULL )
       ptrC++;
   }

   if( *ptrC == '\0' )
     gene= NULL;
   else
   {
     gene= ptrC + 1;
     *ptrC= '\0';
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       eValid
//
// Purpose-
//       Test for valid exon character.
//
//----------------------------------------------------------------------------
static inline int                   // TRUE iff valid
   eValid(                          // Test for valid exon character
     int               c)           // The character
{
   if( c == '\0' )
     return FALSE;

   return strchr(eList,c) != NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       IntronExtractor::~IntronExtractor
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   IntronExtractor::~IntronExtractor( void )  // Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       IntronExtractor::IntronExtractor
//
// Purpose-
//       Constructor.
//
// Desciption-
//       This Extractor returns the next Intron in the sequence.
//
//----------------------------------------------------------------------------
   IntronExtractor::IntronExtractor( void )   // Constructor
:  Extractor()
{
}

//----------------------------------------------------------------------------
//
// Method-
//       IntronExtractor::getNext
//
// Purpose-
//       Extract the next Intron sequence.
//
//----------------------------------------------------------------------------
char*                               // Next Intron sequence
   IntronExtractor::getNext(        // Extract the next Intron
     unsigned          lineNo)      // Input line number (for messages)
{
   char*               result;      // Resultant
   char*               ptrC;        // -> Char (string)

   // Debugging
   #ifdef HCDM
     printf("%4d: IntronExtractor.getNext(%d)\n", __LINE__, lineNo);
   #endif

   // Strip leading delimiters
   if( gene == NULL )
     return NULL;

   result= gene;
   while( eValid(*result) )
   {
     geneIsFirst= FALSE;
     result++;
   }

   if( *result == '\0' )
     return NULL;

   // Find trailing delimiter
   ptrC= result;
   while( *ptrC == 'a' ||  *ptrC == 'c' || *ptrC == 'g' || *ptrC == 't'
       || getWild(*ptrC) != NULL )
     ptrC++;

   // Validate trailing delimiter
   if( *ptrC != '\0' && !eValid(*ptrC) )
   {
     if( genewarns == 0 && warnings < MAX_WARNING )
       fprintf(stderr, "Line %u: Invalid char(%c)\n", lineNo, *ptrC);

     genewarns++;
     warnings++;
     *ptrC= '\0';
     ptrC++;

     while( *ptrC != 'a' &&  *ptrC != 'c' && *ptrC != 'g' &&  *ptrC != 't'
         && *ptrC != '\0' && getWild(*ptrC) == NULL )
       ptrC++;
   }

   if( *ptrC == '\0' )
     gene= NULL;
   else
   {
     gene= ptrC + 1;
     *ptrC= '\0';
   }

   return result;
}

