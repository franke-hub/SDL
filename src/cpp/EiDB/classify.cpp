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
//       classify.cpp
//
// Purpose-
//       Exon/Intron DataBase classifier.
//
// Last change date-
//       2004/08/08
//
// Description-
//       This routine examines an Exon/Intron database file, looking for
//       group identifier matches.
//
//       A list of groups is read, the database is loaded and examined.
//       If a line matches any of the groups specified in the group list,
//       it is written into the "matching" output file.  If no match is
//       found, it is written into the "missed" output file.  Thus, each
//       input line is written into one of the output files.
//
//----------------------------------------------------------------------------
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "common.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "CLASSIFY" // Source file

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
// Struct-
//       Grouping
//
// Function-
//       Define a grouping.
//
//----------------------------------------------------------------------------
struct Grouping {                   // Grouping
   Grouping*           next;        // Chain pointer
   int                 refCount;    // Number of references
   char*               text;        // Descriptor
}; // struct Grouping

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static Grouping*       headGroup= NULL; // Grouping header

static char*           nameGroup;   // Filename: Group list
static char*           nameInput;   // Filename: Input file
static char*           nameFound;   // Filename: Elements found in group
static char*           nameMissd;   // Filename: Elements missed in group

static FILE*           fileGroup;   // File: Group list
static FILE*           fileInput;   // File: Input file
static FILE*           fileFound;   // File: Elements found in group
static FILE*           fileMissd;   // File: Elements missed in group

static char            sw_verbose;  // TRUE iff -v   option specified

//----------------------------------------------------------------------------
//
// Subroutine-
//       getLine
//
// Purpose-
//       Get next line, discarding trailing '\n'
//
//----------------------------------------------------------------------------
static char*                        // Resultant
   getLine(                         // Get next line
     FILE*             file,        // -> FILE
     int               size,        // Sizeof(*line)
     char*             line)        // -> Input line accumulator
{
   char*               C;           // -> String
   int                 L;           // Length

   C= fgets(line, size, file);
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
   }

   return C;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       info
//
// Function-
//       Display parameter information.
//
//----------------------------------------------------------------------------
static void
   info( void )
{
   fprintf(stderr,"Usage: %s ", __SOURCE__);
   fprintf(stderr,"groupFile masterFile foundFile missedFile");
   fprintf(stderr,"\n");
   fprintf(stderr,"Exon/Interon database classifier\n");
   fprintf(stderr,"\n");
   fprintf(stderr,"INP: groupFile\n"
                  "\tThe file containing the list of classifier groups.\n");
   fprintf(stderr,"INP: masterFile\n"
                  "\tThe name of the EIDB database file\n");
   fprintf(stderr,"OUT: foundFile\n"
                  "\tThe subset of the EIDB database file in the group list\n");
   fprintf(stderr,"OUT: missedFile\n"
                  "\tThe remainder of the EIDB database file\n");

   exit(EXIT_FAILURE);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       parm
//
// Function-
//       Parameter analysis routine.
//
//----------------------------------------------------------------------------
static void
   parm(                            // Analyze parameters
     int               argc,        // Argument count
     char*             argv[])      // Argument vector
{
   int                 error;       // Error switch
   int                 i, j, k;     // General index variables

   //-------------------------------------------------------------------------
   // Set defaults
   //-------------------------------------------------------------------------
   sw_verbose= FALSE;               // Default switch settings

   //-------------------------------------------------------------------------
   // Examine parameters
   //-------------------------------------------------------------------------
   error= FALSE;                    // Default, no error found
   k= 0;                            // No filenames specified
   for(j=1; j<argc; j++)            // Examine the parameter list
   {
     if( argv[j][0] == '-' )        // If this is a switch list
     {
       if( strcmp("-help", argv[j]) == 0 )
         error= TRUE;

       else                         // Switch list
       {
         for(i=1; argv[j][i] != '\0'; i++) // Examine the switch list
         {
           switch(argv[j][i])       // Examine the switch
           {
             case 'v':              // -v (verbose)
               sw_verbose= TRUE;
               break;

             default:               // If invalid switch
               error= TRUE;
               fprintf(stderr, "Invalid switch '%c'\n", (int)argv[j][i]);
               break;
           }
         }
       }
       continue;
     }

     //-----------------------------------------------------------------------
     // Process a flat (non-switch) parameter
     //-----------------------------------------------------------------------
     switch(k)
     {
       case 0:
         nameGroup= argv[j];
         break;

       case 1:
         nameInput= argv[j];
         break;

       case 2:
         nameFound= argv[j];
         break;

       case 3:
         nameMissd= argv[j];
         break;

       default:
         error= TRUE;
         fprintf(stderr, "Unexpected parameter: %s\n", argv[j]);
         break;
     }

     k++;
   }

   //-------------------------------------------------------------------------
   // Validate the parameters
   //-------------------------------------------------------------------------
   if( k < 4 )                      // If a fileName was not specified
   {
     error= TRUE;
     fprintf(stderr, "Missing filename.\n");
   }

   if( error )                      // If an error was detected
   {
     info();                        // Tell how this works
     exit(EXIT_FAILURE);            // And exit, function aborted
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       init
//
// Function-
//       Initialize.
//
//----------------------------------------------------------------------------
static void
   init( void )                     // Initialize
{
   fileGroup= fopen(nameGroup, "rb");
   if( fileGroup == NULL )
   {
     fprintf(stderr, "File(%s): Error: ", nameGroup);
     perror("open error: ");
     exit(EXIT_FAILURE);
   }

   fileInput= fopen(nameInput, "rb");
   if( fileGroup == NULL )
   {
     fprintf(stderr, "File(%s): Error: ", nameInput);
     perror("open error: ");
     exit(EXIT_FAILURE);
   }

   fileFound= fopen(nameFound, "rb");
   if( fileFound != NULL )
   {
     fprintf(stderr, "File(%s): Error: file exists\n", nameFound);
     exit(EXIT_FAILURE);
   }

   fileMissd= fopen(nameMissd, "rb");
   if( fileFound != NULL )
   {
     fprintf(stderr, "File(%s): Error: file exists\n", nameMissd);
     exit(EXIT_FAILURE);
   }

   fileFound= fopen(nameFound, "wb");
   if( fileFound == NULL )
   {
     fprintf(stderr, "File(%s): Error: ", nameFound);
     perror("open error: ");
     exit(EXIT_FAILURE);
   }

   fileMissd= fopen(nameMissd, "wb");
   if( fileMissd == NULL )
   {
     fprintf(stderr, "File(%s): Error: ", nameMissd);
     perror("open error: ");
     exit(EXIT_FAILURE);
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       term
//
// Function-
//       Terminate.
//
//----------------------------------------------------------------------------
static void
   term( void )                     // Initialize
{
   Grouping*           ptrG;        // -> Grouping

   fclose(fileGroup);
   fclose(fileInput);
   fclose(fileFound);
   fclose(fileMissd);

   ptrG= headGroup;
   while( ptrG != NULL )
   {
     if( ptrG->refCount != 1 )
     {
       if( ptrG->refCount == 0 )
         fprintf(stderr, "Group(%s) not found\n", ptrG->text);
       else
         fprintf(stderr, "Group(%s) found %d times\n",
                         ptrG->text, ptrG->refCount);
     }

     ptrG= ptrG->next;
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       loadGroup
//
// Function-
//       Load the group list.
//
//----------------------------------------------------------------------------
static void
   loadGroup( void )                // Load the group list
{
   char                string[4096];// Working string
   char*               ptrL;        // -> Line (string)
   Grouping*           ptrG;        // -> Grouping
   int                 L;           // Line length

   for(;;)
   {
     ptrL= getLine(fileGroup, sizeof(string), string);
     if( ptrL == NULL )
       break;

     while( *ptrL == ' ' )
       ptrL++;

     L= strlen(ptrL);
     while( L > 0 && ptrL[L-1] == ' ' )
     {
       ptrL[L-1]= '\0';
       L--;
     }

     if( L == 0 )
       continue;

     ptrG= new Grouping();
     ptrG->next= headGroup;
     ptrG->refCount= 0;
     ptrG->text= new char[L+1];
     strcpy(ptrG->text, ptrL);

     headGroup= ptrG;
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       parse
//
// Function-
//       Parse the input file.
//
//----------------------------------------------------------------------------
static void
   parse( void )                    // Parse the input file
{
   char                string[4096];// Working string
   char                compare[4096]; // Working string
   char*               ptrL;        // -> Line (string)
   Grouping*           ptrG;        // -> Grouping
   int                 L;           // Line length
   unsigned            lineno;      // Line number
   unsigned            found;       // TRUE iff match found

   int                 i;

   lineno= 0;
   for(;;)
   {
     ptrL= getLine(fileInput, sizeof(string), string);
     if( ptrL == NULL )
       break;

     // Verify format of first line
     lineno++;
     if( *ptrL != '>' )
     {
       fprintf(stderr, "Line(%d): invalid format\n", lineno);
       continue;
     }

     while( *ptrL != '_' && *ptrL != '\0' )
       ptrL++;

     if( *ptrL == '\0' )
     {
       fprintf(stderr, "Line(%d): invalid format\n", lineno);
       continue;
     }
     ptrL++;

     i= 0;
     while( *ptrL != ' ' && *ptrL != '\0' )
     {
       compare[i++]= *ptrL;
       ptrL++;
     }
     if( *ptrL == '\0' )
     {
       fprintf(stderr, "Line(%d): invalid format\n", lineno);
       continue;
     }

     compare[i++]= '\0';
     found= FALSE;
     ptrG= headGroup;
     while( ptrG != NULL )
     {
       if( strcmp(compare, ptrG->text) == 0 )
       {
         found= TRUE;
         ptrG->refCount++;
         break;
       }

       ptrG= ptrG->next;
     }

     // Copy group to target
     if( found )
       fprintf(fileFound, "%s\n", string);
     else
       fprintf(fileMissd, "%s\n", string);
     for(;;)
     {
       ptrL= getLine(fileInput, sizeof(string), string);
       if( ptrL == NULL )
         break;

       lineno++;
       if( found )
         fprintf(fileFound, "%s\n", string);
       else
         fprintf(fileMissd, "%s\n", string);
       L= strlen(ptrL);
       while( L > 0 && ptrL[L-1] == ' ' )
       {
         ptrL[L-1]= '\0';
         L--;
       }

       if( L == 0 )
         break;
     }

     if( ptrL == NULL )
       break;
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       main
//
// Purpose-
//       Mainline code.
//
//----------------------------------------------------------------------------
extern int                          // Return code
   main(                            // Mainline code
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   //-------------------------------------------------------------------------
   // Initialization
   //-------------------------------------------------------------------------
   parm(argc, argv);                // Parameter analysis

   //-------------------------------------------------------------------------
   // Scan the data table
   //-------------------------------------------------------------------------
   init();                          // Initialize
   loadGroup();                     // Load the group list
   parse();                         // Parse the master file
   term();                          // Terminate

   //-------------------------------------------------------------------------
   // Function complete
   //-------------------------------------------------------------------------
   return 0;
}

