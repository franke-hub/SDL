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
//       FSfind.cpp
//
// Purpose-
//       Test buffer copy mechanisms.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <com/nativeio.h>
#include <com/syslib.h>
#include <com/Interval.h>

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define DATSIZE                2048 // Data buffer size
#define BUFSIZE               20000 // I/O buffer (maximum size)
#define DOSEOF                   26 // DOS end of file character
#define CODING_ERROR              0 // When asserted, always false

//----------------------------------------------------------------------------
// Constants
//----------------------------------------------------------------------------
static const char*    msg_fread[4]= {
   "open/read (binary mode)",
   "fopen/fread (binary mode)",
   "open/read (text mode)",
   "fopen/fread (text mode)"
   };

static const char*     msg_movet[3]= {
   "None",
   "via inline code",
   "via memccpy"
   };

//----------------------------------------------------------------------------
// General work areas
//----------------------------------------------------------------------------
static int             inpused;     // Input buffer size used
static char            inpbuff[BUFSIZE]; // Input buffer
static char            outbuff[DATSIZE]; // Output buffer

static char*           ptrname;     // -> File name

static int             sw_fread;    // When on, indicates fread
static int             sw_print;    // When on, print file contents
static int             sw_ttype;    // Test type: 0, 1 or 2
static int             sw_tmode;    // When on, indicates text mode
static int             sw_verif;    // Verify parameters

// File pointer: Only one of these are used
static FILE*           inpp;        // File pointer
static int             inph;        // File handle

//----------------------------------------------------------------------------
//
// Subroutine-
//       fillBuffer
//
// Function-
//       Fill the input buffer.
//
//----------------------------------------------------------------------------
static int                          // Buffer length (0 if EOF)
   fillBuffer(void)                 // Fill the input buffer
{
   int                 inpsize;     // Sizeof(buffer) this operation

   if (sw_fread)                    // If fread mode
   {
     inpsize= fread(inpbuff, 1, inpused, inpp); // Read a buffer image
     if (ferror(inpp))              // If read error
     {
       printf ("Error reading input file '%s'\n", ptrname);
       return EXIT_FAILURE;
     }
   }

   else                             // If read mode
     inpsize= read(inph, inpbuff, inpused); // Read a buffer image

   return inpsize;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       printLine
//
// Function-
//       Print a line of data.
//
//----------------------------------------------------------------------------
static void
   printLine(                       // Print a line of data
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
//       test00
//
// Function-
//       Basic test.
//
//----------------------------------------------------------------------------
static void
   test00(void)                     // Data movement via C code
{
   int                 inpsize;     // Sizeof(buffer) this operation

   for(;;)
   {
     inpsize= fillBuffer();         // Fill the buffer
     if (inpsize == 0)
       break;

     printLine(inpbuff, inpsize);
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test01
//
// Function-
//       Data movement via inline code.
//
//----------------------------------------------------------------------------
static void
   test01(void)                     // Data movement via C code
{
   char*               inpaddr;     // Buffer pointer
   int                 inpsize;     // Sizeof(buffer) this operation

   char*               outaddr;     // -> Output buffer
   int                 outsize;     // Data length remaining

   char                c;           // The current character

   outaddr=  outbuff;               // Set data buffer origin
   outsize=  DATSIZE;               // Data length is entire line

   for(;;)                          // Format the buffer
   {
     inpsize= fillBuffer();         // Fill the buffer
     if (inpsize == 0)
       break;
     inpaddr= inpbuff;

     while(inpsize > 0)             // Format the buffer
     {
       c= *inpaddr;                 // Get next character
       inpaddr++;
       inpsize--;

       if (c == '\n')
       {
         if (sw_print)              // If printing
         {
           printLine(outbuff, DATSIZE-outsize);
           printf("\n");
         }

         outaddr= outbuff;
         outsize= DATSIZE;
         continue;
       }

       if (c == '\r')
         continue;

       *outaddr= c;                 // Copy the character
       outaddr++;
       outsize--;
       if (outsize < 0)             // If line overflow
       {
         if (sw_print)              // If printing
           printLine(outbuff, DATSIZE);

         outaddr= outbuff;
         outsize= DATSIZE;
       }
     }
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test02
//
// Function-
//       Data movement via library subroutines.
//
//----------------------------------------------------------------------------
static void
   test02(void)                     // Data movement via C library
{
   char*               inpaddr;     // Buffer pointer
   int                 inpsize;     // Sizeof(buffer) this operation

   char*               outaddr;     // -> Output buffer
   int                 outsize;     // Data length remaining

   char*               p;           // General pointer value
   int                 k;

   outaddr=  outbuff;               // Set data buffer origin
   outsize=  DATSIZE;               // Data length is entire line

   for(;;)                          // Format the buffer
   {
     inpsize= fillBuffer();         // Fill the buffer
     if (inpsize == 0)
       break;
     inpaddr= inpbuff;

     while (inpsize > 0)            // Format the buffer
     {
       if (*inpaddr == '\r')        // If this is a carriage return
       {
         inpaddr++;                 // Skip it
         inpsize--;
         if (inpsize == 0)          // If now at end of buffer
           break;                   // Exit, get next buffer
       }

       k= outsize;                  // Default, length is line size
       if (inpsize < k)             // If buffer length is small
         k= inpsize;                // Use remaining buffer length

       p= (char*)memccpy(outaddr, inpaddr, '\n', k);// Move until end of line
       if (p == NULL) {             // If no delimiter found
         if (k == inpsize)          // If end of buffer
         {
           outaddr += k;            // Update buffer pointer
           outsize -= k;            // Update buffer index
           inpsize= 0;
           break;                   // End of buffer
         }

         else                       // If line overflow
         {
           if (sw_print)            // If printing
           {
             printLine(outbuff, DATSIZE-inpsize);
             printf("\n");
           }

           outaddr= outbuff;
           outsize= DATSIZE;

           inpaddr += k;            // Adjust pointers
           inpsize -= k;
           continue;                // End of line
         }
       }

       k= p - outaddr;              // Calculate move length
       inpaddr += k;                // Adjust pointers
       inpsize -= k;
       k--;                         // We don't want the '\n' char
       outaddr += k;
       outsize -= k;

       if (*(outaddr-1) == '\r')    // If the last character is '\r'
         *(outaddr-1) = ' ';        // Delete it

       if (sw_print)                // If printing
       {
         printLine(outbuff, DATSIZE-outsize);
         printf("\n");
       }

       outaddr= outbuff;
       outsize= DATSIZE;
     }
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
int                                 // Return code (0 OK)
   main(                            // Mainline code
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   Interval            timer;       // Interval timer

   char*               ptrargv;     // Current argument pointer
   char*               ptrfind;     // -> Find string

   int                 errors;      // Error counter

   int                 i;

   //-------------------------------------------------------------------------
   // Parameter validation
   //-------------------------------------------------------------------------
   inpused=  2048;                  // Default, buffer(2048)
   sw_tmode= 0;                     // Default, binary (not text)
   sw_fread= 0;                     // Default, read (not fread)
   sw_print= 0;                     // Default, no printing
   sw_ttype= 0;                     // Default, pure speed test
   sw_verif= 0;                     // Default, no validation

   ptrfind= NULL;
   ptrname= NULL;

   errors= 0;
   for(i=1; i<argc; i++)            // Get switches
   {
     ptrargv= argv[i];              // Address this pointer
     if (*ptrargv != '-')           // If not a switch parameter
     {
       if (ptrfind == NULL)
         ptrfind= ptrargv;
       else if (ptrname == NULL)
         ptrname= ptrargv;
       else
       {
         errors++;
         printf ("Too many parameters, '%s' invalid\n",
                  ptrargv);
       }
     }
     else                           // If a switch parameter
     {
       ptrargv++;                   // Address the switch name
       if (*ptrargv == 'f')         // If FREAD switch
         sw_fread= 1;               // Indicate FREAD

       else if (*ptrargv == 't')    // If test type switch
       {
         ptrargv++;                 // Skip past mode switch
         if (*ptrargv == '0')       // If move mode 0
           sw_ttype= 0;             // Indicate move mode 0
         else if (*ptrargv == '1')  // If move mode 1
           sw_ttype= 1;             // Indicate move mode 1
         else if (*ptrargv == '2')  // If move mode 2
           sw_ttype= 2;             // Indicate move mode 2
         else                       // If invalid move mode
         {
           errors++;
           sw_ttype= 0;             // Restore the default
           printf("Invalid test type '%s'\n", argv[i]);
         }
       }

       else if (*ptrargv == 'p')    // If PRINT switch
         sw_print= 1;               // Indicate PRINT

       else if (*ptrargv == 't')    // If TMODE switch
         sw_tmode= 1;               // Indicate TMODE

       else if (*ptrargv == 'v')    // If VERIFY switch
         sw_verif= 1;               // Indicate VERIFY
       else                         // If INVALID switch
       {
         errors++;
         ptrargv++;                 // Skip past the switch
         printf ("Invalid parameter %i, '%s' ignored\n",
                  i, argv[i]);
       }
     }
   }

   if (errors)
     return EXIT_FAILURE;
   if (ptrname == NULL)
   {
     printf("No filename specified\n");
     return EXIT_FAILURE;
   }

   if (sw_verif)                    // If parameter verification
   {
     printf(" Buffer size: %d\n", inpused);
     printf("   Data Move: %s\n", msg_movet[sw_ttype]);
     printf("    Filename: '%s'\n", ptrname);
     printf("         I/O: %s\n", msg_fread[sw_tmode*2+sw_fread]);
     printf("    Printing: %s\n", sw_print ? "On" : "Off");
//   printf("Elapsed time: %.3f seconds\n", timer.toDouble());
   }

   //-------------------------------------------------------------------------
   // File initialization
   //-------------------------------------------------------------------------
   inpp= NULL;                      // Avoids compiler warning
   if (sw_fread)                    // If fread mode
   {
     if (sw_tmode)                  // If text mode
       inpp= fopen(ptrname, "r");   // Open the input file
     else                           // If binary mode
       inpp= fopen(ptrname, "rb");  // Open the input file

     if (inpp == NULL)              // If we cannot open the input file
     {
       printf ("Error, cannot open input file '%s'\n", ptrname);
       return EXIT_FAILURE;
     }
   }

   else                             // If read mode
   {
     if (sw_tmode)                  // If text mode
       inph=  open(ptrname,         // Open the input file
                   O_RDONLY,
                   0);
     else                           // If binary mode
       inph=  open(ptrname,         // Open the input file
                   O_RDONLY|O_BINARY,
                   0);

     if (inph == (-1))              // If we cannot open the input file
     {
       printf ("Error, cannot open input file '%s'\n", ptrname);
       return EXIT_FAILURE;
     }
   }

   //-------------------------------------------------------------------------
   // Read the input file
   //-------------------------------------------------------------------------
   timer.start();                   // Set start time
   if (sw_ttype == 0)               // If instrumentation only
     test00();                      // Print out the buffer

   else if (sw_ttype == 1)          // If data move within code
     test01();                      // Move the data

   else if (sw_ttype == 2)          // If data move via MEMCCPY
     test02();                      // Move the data

   else                             // If internal error
     assert(CODING_ERROR);          // Should not occur

   //-------------------------------------------------------------------------
   // End of file
   //-------------------------------------------------------------------------
   timer.stop();                    // Elapsed time (in ms.)
   printf("Elapsed time: %.3f seconds\n", timer.toDouble());

   return EXIT_SUCCESS;
}

