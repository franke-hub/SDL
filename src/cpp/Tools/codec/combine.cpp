//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//-----------------------------------------------------------------------------
//
// Title-
//       combine.cpp
//
// Purpose-
//       Combine a list of uuencoded files into a single uuencoded file.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <com/define.h>
#include <com/params.h>

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define INP_SIZE               8192 // Input buffer size

//----------------------------------------------------------------------------
//
// Struct-
//       Segment
//
// Purpose-
//       Describe a file segment.
//
//----------------------------------------------------------------------------
struct Segment {                    // File Segment
//----------------------------------------------------------------------------
// Segment: Enumerations and typedefs
//----------------------------------------------------------------------------
enum {                              // Constants for parameterization
   SIZE=                      16384 // The data size of each segment
}; // enum

//----------------------------------------------------------------------------
// Segment: Attributes
//----------------------------------------------------------------------------
   Segment*            next;        // Chain pointer
   unsigned            used;        // Number of bytes used
   char                data[SIZE];  // Segment data
}; // struct Segment

//----------------------------------------------------------------------------
//
// Struct-
//       Content
//
// Purpose-
//       Describe a file's content.
//
//----------------------------------------------------------------------------
struct Content {                    // File content
   Content*            next;        // Chain pointer
   char*               name;        // The file's name
   Segment*            head;        // First file segment
   Segment*            tail;        // Final file segment
}; // struct Content

//----------------------------------------------------------------------------
// Encoding table
//----------------------------------------------------------------------------
#define PAD_CHAR '='                // Pad character

//----------------------------------------------------------------------------
// Local data areas
//----------------------------------------------------------------------------
static char            inpBuff[INP_SIZE]; // Input buffer
static Content*        outs= NULL;    // The sorted list of output files

//----------------------------------------------------------------------------
//
// Subroutine-
//       info
//
// Purpose-
//       Parameter fault exit.
//
//----------------------------------------------------------------------------
static void
   info( void )                     // Parameter fault exit
{
   fprintf(stderr, "combine filename ... >output-filename\n");
   fprintf(stderr, "\n");
   fprintf(stderr, "filename ...\n");
   fprintf(stderr, "  The list of files to combine\n");
   exit(EXIT_FAILURE);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       parm
//
// Purpose-
//       Parameter analysis
//
//----------------------------------------------------------------------------
static void
   parm(                            // Parameter analysis
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   char*               argp;        // Argument pointer
   int                 argi;        // Argument index

   int                 count;       // The number of files to compare
   int                 error;       // Error encountered indicator
// int                 verify;      // Verification control

   //-------------------------------------------------------------------------
   // Defaults
   //-------------------------------------------------------------------------
   count= 0;                        // No files found yet
   error= FALSE;                    // Default, no errors found
// verify= 0;                       // Default, no verification

   //-------------------------------------------------------------------------
   // Argument analysis
   //-------------------------------------------------------------------------
   for( argi=1; argi<argc; argi++ ) // Analyze variable controls
   {
     argp= argv[argi];              // Address the parameter

     if( *argp == '-' )             // If this parameter is in switch format
     {
       argp++;                      // Skip over the switch char

//     if( swname("verify", argp) ) // If verify switch
//       verify= swatob("verify", argp); // Get switch value
//
//     else                         // If invalid switch
       {
         error= TRUE;
         fprintf(stderr, "Invalid parameter '%s'\n",
                         argv[argi]);
       }
     }
     else                           // If filename parameter
     {
       count++;
     }
   }

   //-------------------------------------------------------------------------
   // Completion analysis
   //-------------------------------------------------------------------------
   if( count < 1 )                  // If too few files specified
   {
     error= TRUE;
     fprintf(stderr, "No filename specified\n");
   }

   if( error )                      // If error encountered
     info();
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       rdline
//
// Purpose-
//       Read line from file, exit if failure.
//
//----------------------------------------------------------------------------
static char*                        // Resultant, NULL if EOF or error
   rdline(                          // Read line from file
     FILE*             H,           // File handle
     const char*       fileName)    // File name, for messages
{
   char*               result;      // Resultant
   int                 L;

   errno= 0;                        // In case of error
   result= fgets(inpBuff, sizeof(inpBuff), H);
   if( result != NULL )
   {
     L= strlen(result);
     while( L>0 && (result[L-1] == '\r' || result[L-1] == '\n') )
     {
       result[L-1]= '\0';
       L--;
     }
   }
   else
   {
     if( errno != 0 )
     {
       fprintf(stderr, "File(%s), ", fileName);
       perror("I/O error");
     }
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       rdchar
//
// Purpose-
//       Read character from input line.
//
//----------------------------------------------------------------------------
inline int                          // Resultant character
   rdchar(                          // Read line from file
     char*&            C)           // Input character
{
   int                 result;      // Resultant

   result= 0;                       // Default, NULL
   if( *C != '\0' )                 // If not end of line
   {
     result= *C;                    // Return the current character
     C++;                           // Update position
   }

   result &= 0x00ff;                // Insure positive value
   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       wrline
//
// Purpose-
//       Write a line into the output buffer.
//
//----------------------------------------------------------------------------
static void
   wrline(                          // Write line into buffer
     Content*          content,     // The output buffer
     const char*       line)        // The output line
{
   Segment*            segment;     // The current Segment
   const unsigned      L= strlen(line); // strlen(line)

   segment= content->tail;          // Get current Segment
   if( segment == NULL              // If no Segment
       || L >= Segment::SIZE-segment->used-3)
   {
     segment= (Segment*)malloc(sizeof(Segment));
     assert( segment != NULL );
     segment->next= NULL;
     segment->used= 0;

     if( content->head == NULL )
       content->head= segment;
     else
       content->tail->next= segment;

     content->tail= segment;
   }

   strcpy(&(segment->data[segment->used]), line); // Copy in the line
   segment->used += L;

   #ifndef _OS_CYGWIN
     segment->data[segment->used++]= '\r';
   #endif
   segment->data[segment->used++]= '\n';
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       combine
//
// Purpose-
//       Add a file to the list of output files.
//
//----------------------------------------------------------------------------
int                                 // Return code
   combine(                         // Add a file to the list of output files
     const char*       fileName)    // -> FileName to combine
{
   char*               C;           // -> Buffer
   FILE*               inp;         // -> FILE to decode
   Content*            content;     // -> Content descriptor
   Content*            ptrContent;  // -> Content descriptor

   //-------------------------------------------------------------------------
   // Open the file
   //-------------------------------------------------------------------------
   inp= fopen(fileName, "rb");      // Open the file
   if( inp == NULL )
   {
     fprintf(stderr, "File(%s) ", fileName);
     perror("open failed");
     return (-1);
   }

   //-------------------------------------------------------------------------
   // Find the beginning of the encoded data
   //-------------------------------------------------------------------------
   for(;;)                          // Find the start delimiter
   {
     C= rdline(inp, fileName);      // Read a file line
     if( C == NULL )                // End of file or error
     {
       fclose(inp);
       return (-1);
     }

     if( *C == '-' )                // If delimiter line
       break;
   }

   // TODO: Logic is needed here to handle different kinds of encodings.

   //-------------------------------------------------------------------------
   // Allocate a content descriptor
   //-------------------------------------------------------------------------
   content= (Content*)malloc(sizeof(Content));
   assert( content != NULL );
   memset(content, 0, sizeof(Content));

   content->name= (char*)malloc(strlen(fileName)+1);
   assert( content->name != NULL );
   strcpy(content->name, fileName);

   //-------------------------------------------------------------------------
   // Extract
   //-------------------------------------------------------------------------
   for(;;)                          // Extract the data
   {
     C= rdline(inp, fileName);      // Read a file line
     if( C == NULL )                // End of file or error
       break;

     if( *C == '-' || *C == PAD_CHAR ) // If end of valid data
       break;

     if( *C == '\0' )               // Ignore empty lines
       continue;

     wrline(content, C);            // Write the output line
   }

   //-------------------------------------------------------------------------
   // Add the Content to the sorted content list
   //-------------------------------------------------------------------------
   if( outs == NULL )
     outs= content;
   else
   {
     if( strcmp(outs->name,content->name) > 0 )
     {
       content->next= outs;
       outs= content;
     }
     else
     {
       ptrContent= outs;
       while( ptrContent->next != NULL
           && strcmp(ptrContent->next->name,content->name) < 0 )
       {
         ptrContent= ptrContent->next;
       }

       content->next= ptrContent->next;
       ptrContent->next= content;
     }
   }

   //-------------------------------------------------------------------------
   // Return to caller
   //-------------------------------------------------------------------------
   fclose(inp);
   return 0;                        // Decode complete
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       writer
//
// Purpose-
//       Write the combined and sorted output files.
//
//----------------------------------------------------------------------------
void                                // Return code
   writer( void )                   // Write the output files
{
   Content*            content;     // -> Content descriptor
   Segment*            segment;     // -> Segment descriptor

   content= outs;
   printf("---- INIT; name=%s\n", outs->name);
   while( content != NULL )
   {
     printf("---- INIT; name=%s\n", content->name); // DEBUGGING

     segment= content->head;
     while( segment != NULL )
     {
       segment->data[segment->used]= '\0';
       printf("%s", segment->data);

       segment= segment->next;
     }

     printf("---- TERM; name=%s\n", content->name); // DEBUGGING
     content= content->next;
   }
   printf("---- TERM; name=%s\n", outs->name);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       main
//
// Purpose-
//       Mainline code
//
//----------------------------------------------------------------------------
int                                 // Main return code
   main(                            // Mainline code
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   int                 rc;          // Called routine's return code
   int                 returncd;    // This routine's return code

   int                 i;

   //-------------------------------------------------------------------------
   // Parameter analysis
   //-------------------------------------------------------------------------
   parm(argc, argv);

   //-------------------------------------------------------------------------
   // Initialize
   //-------------------------------------------------------------------------
   returncd= 0;
   for(i=1; i<argc; i++)
   {
     if( argv[i][0] == '-' )        // If this parameter is in switch format
       continue;

     rc= combine(argv[i]);          // Copy the file to the content buffer
     if( rc != 0 )                  // If failure
       returncd= 1;                 // Indicate it
   }

   //-------------------------------------------------------------------------
   // Write the output files
   //-------------------------------------------------------------------------
   writer();

   //-------------------------------------------------------------------------
   // Return
   //-------------------------------------------------------------------------
   return returncd;
}

