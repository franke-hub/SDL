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
//       FSread.cpp
//
// Purpose-
//       File system read test
//
// Last change date-
//       2007/01/01
//
// Controls-
//       /bnnnn   = Buffer size nnnn (maximum is 10000)
//       /p       = Print the data
//       /f       = Use fopen, fread
//       /t       = Text mode
//       /v       = Verify parameters
//       /m0      = No data movement
//       /m1      = Data movement via inline code
//       /m2      = Data movement uses memccpy, memset
//       filedesc = The file to be read
//
//----------------------------------------------------------------------------
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <com/nativeio.h>
#include <com/Interval.h>
#include <com/syslib.h>

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define BUFSIZE               20000 // I/O buffer (maximum size)
#define LINSIZE                  80 // Line size
#define inpfile          argv[argc] // Input file name

#define DOSEOF                   26 // DOS end of file character
#define CODING_ERROR              0 // When asserted, always false

//----------------------------------------------------------------------------
// Internal structures
//----------------------------------------------------------------------------
struct line
{
   struct line*        next;        // Chain pointer
   char                data[LINSIZE]; // The physical data
};

//----------------------------------------------------------------------------
// Static areas
//----------------------------------------------------------------------------
static const char*     msg_fread[4]= {
   "I/O via open/read (binary mode)",
   "I/O via fopen/fread (binary mode)",
   "I/O via open/read (text mode)",
   "I/O via fopen/fread (text mode)"
   };

static const char*     msg_movet[3]= {
   "No data movement",
   "Data movement via inline code",
   "Data movement uses memccpy, memset"
   };

//----------------------------------------------------------------------------
// General work areas
//----------------------------------------------------------------------------
static char            buffer[BUFSIZE]; // I/O buffer
static int             bufsize;     // I/O Buffer size
static int             sw_fread;    // When on, indicates fread
static int             sw_print;    // When on, print file contents
static int             sw_movet;    // Move type: 0, 1 or 2
static int             sw_tmode;    // When on, indicates text mode
static int             sw_verif;    // Verify parameters

//------------------------------------ For MOVE_xx routines
struct line*           ptrline;     // Line pointer
char*                  ptrdata;     // Data pointer
int                    lendata;     // Data length remaining
int                    skipping_newline; // True when cr/nl across buffer

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
   int                 c;           // The next print character
   int                 i;

   if (!sw_print)                   // If not printing
     return;                        // Exit, no function required

   for(i=0; i<lenline; i++)         // Display the buffer line
   {
     c= ptrline[i];                 // Get next character

     if (c == '\r')                 // If special character '\r'
       printf("\\r");               // Represent the character
     else if (c == '\n')            // If special character '\n'
       printf("\\n\n");             // Represent the characacter
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
       printf("\\x1a\n");           // Represent the character
     else                           // If standard character
       printf("%c",c);              // Print the character
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Move_01
//
// Function-
//       Data movement via inline code.
//
//----------------------------------------------------------------------------
static void
   move_01(                         // Data movement via C code
     char*             ptrbuff,     // Pointer to this buffer
     int               lenbuff)     // Length of this buffer
{
   char                c;           // The current character
   int                 i, k;

   while (lenbuff > 0)              // Format the buffer
   {
     c= *ptrbuff;                   // Get next character
     ptrbuff++;
     lenbuff--;

     if (c == '\n')
     {
       k= lendata;
       for (i=0; i<k; i++)
       {
         *ptrdata= ' ';
         ptrdata++;
       }

       if (sw_print)                // If printing
         print_line(ptrline->data, LINSIZE);

       ptrline= ptrline->next;
       ptrdata= &ptrline->data[0];
       lendata= LINSIZE;
       continue;
     }

     if (c == '\r')
       continue;

     *ptrdata= c;                   // Copy the character
     ptrdata++;
     lendata--;
     if (lendata < 0)               // If line overflow
     {
       if (sw_print)                // If printing
         print_line(ptrline->data, LINSIZE);

       ptrline= ptrline->next;
       ptrdata= &ptrline->data[0];
       lendata= LINSIZE;
     }
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Move_02
//
// Function-
//       Data movement via library subroutines.
//
//----------------------------------------------------------------------------
static void
   move_02(                         // Data movement via C library
     char*             ptrbuff,     // Pointer to this buffer
     int               lenbuff)     // Length of this buffer
{
   char*               p;           // General pointer value
   long                k;

   while (lenbuff > 0)              // Format the buffer
   {
     if (*ptrbuff == '\r')          // If this is a carriage return
     {
       ptrbuff++;                   // Skip it
       lenbuff--;
       if (lenbuff == 0)            // If now at end of buffer
         continue;                  // Exit, get next buffer
     }

     k= lendata;                    // Default, length is line size
     if (lenbuff < k)               // If buffer length is small
       k= lenbuff;                  // Use remaining buffer length

     p= (char*)memccpy(ptrdata, ptrbuff, '\n', k);// Move until end of line
     if (p == NULL)                 // If no delimiter found
       if (k == lenbuff)            // If end of buffer
       {
         ptrdata += k;              // Update buffer pointer
         lendata -= k;              // Update buffer index
         lenbuff= 0;
         continue;                  // End of buffer
       }

       else                         // If line overflow
       {
         if (sw_print)              // If printing
           print_line(ptrline->data, LINSIZE);

         ptrline= ptrline->next;
         ptrdata= &ptrline->data[0];
         lendata= LINSIZE;

         ptrbuff += k;              // Adjust pointers
         lenbuff -= k;
         continue;                  // End of line
       }

     k= p - ptrdata;                // Calculate move length
     ptrbuff += k;                  // Adjust pointers
     lenbuff -= k;
     k--;                           // We don't want the '\n' char
     ptrdata += k;
     lendata -= k;

     memset(ptrdata, ' ', lendata); // Clear the buffer
     if (*(ptrdata-1) == '\r')      // If the last character is '\r'
       *(ptrdata-1) = ' ';          // Delete it

     if (sw_print)                  // If printing
       print_line(ptrline->data, LINSIZE);

     ptrline= ptrline->next;
     ptrdata= &ptrline->data[0];
     lendata= LINSIZE;
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
int
   main(                            // FSREAD testcase
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   struct line         lines[2];    // Data buffers
   Interval            timer;       // Interval timer

   char*               ptrargv;     // Current argument pointer

   FILE*               inpp= NULL;  // File pointer
   int                 inph= (-1);  // File handle

   char*               ptrbuff;     // Buffer pointer
   int                 lenbuff;     // Sizeof(buffer) this operation

   int                 ccount= 0;   // Character count

   int                 i;

   //-------------------------------------------------------------------------
   // Test control initialization
   //-------------------------------------------------------------------------
   bufsize= 2048;                   // Default, buffer(2048)
   sw_tmode= 0;                     // Default, binary (not text)
   sw_fread= 0;                     // Default, read (not fread)
   sw_print= 0;                     // Default, no printing
   sw_movet= 0;                     // Default, pure speed test
   sw_verif= 0;                     // Default, no validation

   if (argc == 1)                   // If no filename present
   {
     printf("No filename specified.\n");
     return 1;
   }

   argc--;                          // Last argument is the filename
   for(i=1; i<argc; i++)            // Get switches
   {
     ptrargv= argv[i];              // Address this pointer
     if (*ptrargv != '/')           // If not a switch parameter
       printf ("Invalid parameter %i, '%s' ignored\n",
                i, ptrargv);
     else                           // If a switch parameter
     {
       ptrargv++;                   // Skip past the switch
       if (*ptrargv == 'f')         // If FREAD switch
         sw_fread= 1;               // Indicate FREAD

       else if (*ptrargv == 'p')    // If PRINT switch
         sw_print= 1;               // Indicate PRINT

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

       else if (*ptrargv == 'm')    // If move mode switch
       {
         ptrargv++;                 // Skip past mode switch
         if (*ptrargv == '0')       // If move mode 0
           sw_movet= 0;             // Indicate move mode 0
         else if (*ptrargv == '1')  // If move mode 1
           sw_movet= 1;             // Indicate move mode 1
         else if (*ptrargv == '2')  // If move mode 2
           sw_movet= 2;             // Indicate move mode 2
         else                       // If invalid move mode
         {
           sw_movet= 0;             // Restore the default
           printf("Invalid move mode '%s'\n", argv[i]);
         }
       }

       else                         // If invalid switch
         printf ("Invalid parameter %i, '%s' ignored\n",
                  i, argv[i]);
     }
   }

   if (sw_verif)                    // If parameter verification
   {
     printf ("Buffer size: %d\n", bufsize);
     printf("%s\n", msg_fread[sw_tmode*2+sw_fread]);// I/O controls
     printf("%s\n", msg_movet[sw_movet]);// Move type indicator
     printf("Filename: '%s'\n",inpfile);
   }

   //-------------------------------------------------------------------------
   // Data initialization
   //-------------------------------------------------------------------------
   (&lines[0])->next= &lines[1];    // Initialize chain pointers
   (&lines[1])->next= &lines[0];

   ptrline= &lines[0];              // Address the first line
   ptrdata= &ptrline->data[0];      // Address the first data area
   lendata= LINSIZE;                // Data length is entire line
   skipping_newline= 0;             // We are not skipping newline

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
       return 1;
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
       return 1;
     }
   }

   //-------------------------------------------------------------------------
   // Read the input file
   //-------------------------------------------------------------------------
   timer.start();                   // Set start time
   while(1)                         // Read the input file
   {
     ptrbuff= &buffer[0];           // Address the data buffer
     if (sw_fread)                  // If fread mode
     {
       lenbuff= fread(ptrbuff, 1, bufsize, inpp);// Read a buffer image
       if (ferror(inpp))            // If read error
       {
         printf ("Error reading input file '%s'\n",inpfile);
         return 2;
       }
     }

     else                           // If read mode
       lenbuff= read(inph, ptrbuff, bufsize); // Read a buffer image

     if (lenbuff == 0)              // If zero length buffer
       break;                       // Exit, end of file

     ccount += lenbuff;             // Increment the data count
     if (sw_movet == 0)             // If inpfile instrumentation
       print_line(ptrbuff, lenbuff);// Print out the buffer

     else if (sw_movet == 1)        // If data move within code
       move_01(ptrbuff, lenbuff);   // Move the data

     else if (sw_movet == 2)        // If data move via MEMCCPY
       move_02(ptrbuff, lenbuff);   // Move the data

     else                           // If internal error
       assert(CODING_ERROR);        // Should not occur
   }

   //-------------------------------------------------------------------------
   // End of file
   //-------------------------------------------------------------------------
   timer.stop();                    // Elapsed time (in ms.)
   printf("Elapsed time: %.3f seconds\n", timer.toDouble());

   if (sw_print)                    // If printing
     printf("Bytecount: %d\n",ccount);

   return 0;
}

