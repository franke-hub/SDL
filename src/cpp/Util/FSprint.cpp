//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       FSprint
//
// Purpose-
//       File system diagnostic file print utility
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <com/nativeio.h>

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define BUFSIZE               10000 // I/O buffer (maximum size)
#define LINSIZE                  80 // Line size

#define DOSEOF                   26 // DOS end of file character
#define ESCAPE                   27 // Escape character
#define CODING_ERROR              0 // When asserted, always false
#define SWITCH                  '-' // Switch is "-"

//----------------------------------------------------------------------------
// Static areas
//----------------------------------------------------------------------------
static const char*   msg_fread[4]= {
   "open/read (binary mode)",
   "fopen/fread (binary mode)",
   "open/read (text mode)",
   "fopen/fread (text mode)"
   };

//----------------------------------------------------------------------------
// General work areas
//----------------------------------------------------------------------------
static int             bufsize;     // I/O Buffer size
static char*           inpfile;     // Input file name
static long            lcount= 1;   // Linenumber count
static int             sw_fread;    // When on, indicates fread
static int             sw_number;   // When on, indicates linenumber
static int             sw_tmode;    // When on, indicates text mode
static int             sw_verif;    // Verify parameters

//----------------------------------------------------------------------------
//
// Subroutine-
//       info
//
// Function-
//       Print a description of what this program does.
//
//----------------------------------------------------------------------------
static void
   info( void )                     // Print program information
{
   printf("FSprint <options> fileDesc\n");
   printf("Options:\n"
          "  -bSize Use buffer size= Size\n"
          "  -f Use fread() rather than read()\n"
          "  -n Display line number\n"
          "  -t Use text mode\n"
          "  -v Verify parameters\n"
         );
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Print_line
//
// Function-
//       Print a line of data.
//
//----------------------------------------------------------------------------
static void
   print_line(                      // Print a line of data
     char*             ptrline,     // Pointer to line
     int               lenline)     // Length of line
{
   int                 i;           // General loop index
   int                 c;           // The next print character

   for(i=0; i<lenline; i++)         // Display the buffer line
   {
     c= ptrline[i];                 // Get next character

     if (c == '\\')                 // If special character '\\'
       printf("\\\\");              // Represent the character
     else if (c == '\0')            // If special character '\0'
       printf("\\000");             // Represent the character
     else if (c == '\r')            // If special character '\r'
       printf("\\r");               // Represent the character
     else if (c == '\n')            // If special character '\n'
     {
       printf("\\n\n");             // Represent the characacter
       if (sw_number)
         printf("%6ld ", lcount++);
     }
     else if (c == '\a')            // If special character '\a'
       printf("\\a");               // Represent the character
     else if (c == '\b')            // If special character '\b'
       printf("\\b");               // Represent the character
     else if (c == '\f')            // If special character '\f'
       printf("\\f");               // Represent the character
     else if (c == '\t')            // If special character '\t'
       printf("\\t");               // Represent the character
     else if (c == '\v')            // If special character '\v'
       printf("\\v");               // Represent the character
     else if (c == DOSEOF)          // If special character DOSEOF
       printf("\\032");             // Represent the character
     else if (c == ESCAPE)          // If special character ESCAPE
       printf("\\033");             // Represent the character
     else                           // If standard character
       printf("%c",c);              // Print the character
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       main
//
// Function-
//       Mainline code.
//
//----------------------------------------------------------------------------
int                                 // Return code
   main(                            // Mainline code
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   char*               ptrargv;     // Current argument pointer
   int                 i;           // General loop index

   FILE*               inpp= NULL;  // File pointer
   int                 inph= (-1);  // File handle

   char                buffer[BUFSIZE]; // I/O buffer
   char*               ptrbuf;      // Buffer pointer
   int                 lenbuf;      // Sizeof(buffer) this operation

   int                 ccount= 0;   // Character count

   //-------------------------------------------------------------------------
   // Test control initialization
   //-------------------------------------------------------------------------
   bufsize= 2048;                   // Default, buffer(2048)
   inpfile= NULL;                   // Default, no filename

   sw_fread= 0;                     // Default, read (not fread)
   sw_number= 0;                    // Default, no line number
   sw_tmode= 0;                     // Default, binary (not text)
   sw_verif= 0;                     // Default, no validation

   for(i=1; i<argc; i++)            // Get switches
   {
     ptrargv= argv[i];              // Address this pointer
     if (*ptrargv == SWITCH)        // If this is a switch parameter
     {
       ptrargv++;                   // Skip past the switch
       if (*ptrargv == 'f')         // If FREAD switch
         sw_fread= 1;               // Indicate FREAD

       else if (*ptrargv == 'n')    // If NUMBER switch
         sw_number= 1;              // Indicate NUMBER

       else if (*ptrargv == 't')    // If TMODE switch
         sw_tmode= 1;               // Indicate TMODE

       else if (*ptrargv == 'v')    // If VERIFY switch
         sw_verif= 1;               // Indicate VERIFY

       else if (*ptrargv == 'b')    // If buffer size switch
       {
         ptrargv++;                 // Skip past buffer size switch
         bufsize= atoi(ptrargv);    // Convert the buffer size
         if (bufsize <= 0 || bufsize > BUFSIZE) // If invalid buffer size
         {
           bufsize= 2048;           // Restore the default
           printf("Invalid buffer size '%s'\n", argv[i]);
         }
       }

       else                         // If invalid switch
         printf ("Invalid parameter '%s' ignored\n", ptrargv);
     }

     else                           // If not a switch parameter
     {
       if (inpfile == NULL)         // If first file parameter
         inpfile= ptrargv;          // Set the file parameter
       else                         // If unexpected parameter
       {
         printf ("Unexpected parameter '%s'\n", ptrargv);
         info();
         return(1);
       }
     }
   }

   if (inpfile == NULL)             // If no filename present
   {
     printf("No filename specified.\n");
     info();
     return(1);
   }

   if (sw_verif)                    // If parameter verification
   {
     printf("Filename: '%s'\n",inpfile);
     printf(" I/O via: %s\n", msg_fread[sw_tmode*2+sw_fread]);
     printf("Buffsize: %d\n", bufsize);
     printf("  Number: %s\n", sw_number ? "TRUE" : "FALSE");
   }

   //-------------------------------------------------------------------------
   // File initialization
   //-------------------------------------------------------------------------
   if (sw_fread)                    // If fread mode
   {
     if (sw_tmode)                  // If text mode
       inpp= fopen(inpfile, "r");   // Open the input file
     else                           // If binary mode
       inpp= fopen(inpfile, "rb");  // Open the input file

     if (inpp == NULL)              // If we cannot open the input file
     {
       printf ("Error, cannot open input file '%s'\n",inpfile);
       return(2);
     }
   }

   else                             // If read mode
   {
     if (sw_tmode)                  // If text mode
       inph=  open(inpfile,         // Open the input file
                   O_RDONLY,
                   0);
     else                           // If binary mode
       inph=  open(inpfile,         // Open the input file
                   O_RDONLY|O_BINARY,
                   0);

     if (inph == (-1))              // If we cannot open the input file
     {
       printf ("Error, cannot open input file '%s'\n",inpfile);
       return(2);
     }
   }

   //-------------------------------------------------------------------------
   // Read the input file
   //-------------------------------------------------------------------------
   if (sw_number)
     printf("%6ld ", lcount);
   while(1)                         // Read the input file
   {
     ptrbuf= &buffer[0];            // Address the data buffer

     if (sw_fread)                  // If fread mode
     {
       lenbuf= fread(ptrbuf, 1, bufsize, inpp); // Read a buffer image
       if (ferror(inpp))            // If read error
       {
         printf ("Error reading input file '%s'\n",inpfile);
         return(3);
       }
     }

     else                           // If read mode
       lenbuf=  read(inph, ptrbuf, bufsize); // Read a buffer image

     if (lenbuf == 0)               // If zero length buffer
       break;                       // Exit, end of file

     ccount += lenbuf;              // Increment the data count
     print_line(buffer, lenbuf);    // Print out the buffer
   }

   //-------------------------------------------------------------------------
   // End of file
   //-------------------------------------------------------------------------
// printf("Bytecount: %d\n",ccount);
   return(0);
}

