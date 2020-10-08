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
//       decodeyn.cpp
//
// Purpose-
//       Decode a yEnc encoded file.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <com/AutoPointer.h>
#include <com/Buffer.h>
#include <com/istring.h>
#include <com/params.h>
#include <com/Parser.h>
#include <com/Reader.h>
#include <com/Writer.h>

#include "YncodeCodec.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define LINE_SIZE             32768 // Allocated line size

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef SHOULD_NOT_OCCUR
#define SHOULD_NOT_OCCUR FALSE      // Always FALSE
#endif

#define REQUIRED TRUE               // Always TRUE

//----------------------------------------------------------------------------
//
// Struct-
//       Yline
//
// Purpose-
//       Describe an =y control line.
//
//----------------------------------------------------------------------------
struct Yline {                      // =y Control line descriptor
   enum VP_TYPE                     //
   { TYPE_0                         // Definition
   , TYPE_PTR                       // Pointer
   , TYPE_DEC                       // Decimal value
   , TYPE_HEX                       // Hexidecimal value
   };                               // Type

   const char*         name;        // The name of the control
   unsigned char       type;        // The type of the control
   unsigned char       isReq;       // TRUE iff required
   unsigned char       isSet;       // TRUE iff set
   union
   {
     const char*         p;         // Pointer
     uint32_t            v;         // Value
   }                   data;        // Value or Pointer
}; // struct Yline
#define YLINE(name, type, required) {name, Yline::TYPE_ ## type, required, false, .data= {.p= nullptr}}

//----------------------------------------------------------------------------
// Local data areas
//----------------------------------------------------------------------------
static Yline           yBEGIN[]=    // =ybegin line
{  YLINE("=ybegin ", 0, REQUIRED)
,  YLINE("line",   DEC, REQUIRED)
,  YLINE("size",   DEC, REQUIRED)
,  YLINE("name",   PTR, REQUIRED)
,  YLINE("part",   DEC, !REQUIRED)
,  YLINE("total",  DEC, !REQUIRED)
,  YLINE(NULL, 0, !REQUIRED)
};

static Yline            yEND[]=     // =yend line
{  YLINE("=yend ",   0, REQUIRED)
,  YLINE("size",   DEC, REQUIRED)
,  YLINE("crc32",  HEX, !REQUIRED)
,  YLINE("part",   DEC, !REQUIRED)
,  YLINE("pcrc32", HEX, !REQUIRED)
,  YLINE(NULL, 0, !REQUIRED)
};

static Yline            yPART[]=    // =ypart line
{  YLINE("=ypart ",  0, REQUIRED)
,  YLINE("begin",  DEC, REQUIRED)
,  YLINE("end",    DEC, REQUIRED)
,  YLINE(NULL, 0, !REQUIRED)
};

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
   fprintf(stderr, "decodeyn filename <input-filename\n");
   fprintf(stderr, "\n");
   fprintf(stderr, "filename\n");
   fprintf(stderr, "  The output file name\n");
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
       if( count > 1 )
       {
         error= TRUE;
         fprintf(stderr, "Extra filename(%s) specified\n", argp);
       }
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
//       parseDec
//
// Purpose-
//       Parse long decimal value, updating text pointer.
//
//----------------------------------------------------------------------------
static uint32_t                     // The long decimal value
   parseDec(                        // Parse decimal value
     const char*&      text)        // In this string
{
   Parser parser(text);
   int32_t result= parser.toDec32();
   text= parser.getString();
   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       parseHex
//
// Purpose-
//       Parse long hexadecimal value, updating text pointer.
//
//----------------------------------------------------------------------------
static uint32_t                     // The long hexadecimal value
   parseHex(                        // Parse hexadecimal value
     const char*&      text)        // In this string
{
   Parser parser(text);
   int32_t result= parser.toHex32();
   text= parser.getString();
   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       skipBlank
//
// Purpose-
//       Find next non-whitespace in string
//
//----------------------------------------------------------------------------
static const char*                  // Next non-whitespace character
   skipBlank(                       // Find next non-whitespace character
     const char*       text)        // In this string
{
   Parser parser(text);
   return parser.skipSpace();
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       strimem
//
// Purpose-
//       Compare a string to memory, ignoring case.
//
//----------------------------------------------------------------------------
static inline int                   // Resultant
   strimem(                         // STRING :: Memory
     const char*       str,         // The string
     const char*       mem)         // The memory
{
   return memicmp(str, mem, strlen(str));
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       parseYL
//
// Purpose-
//       Parse a Yline array
//
//----------------------------------------------------------------------------
static int                          // Return code, 0 OK
   parseYL(                         // Parse Yline array
     const char*       inpLine,     // The input line
     Yline*            yLINE)       // The Yline array
{
   int                 result;      // Resultant
   const char*         C= inpLine;  // Working character pointer
   int                 found;       // TRUE iff found

   int                 i;

   for(i= 0; yLINE[i].name != NULL; i++)
   {
     yLINE[i].isSet= FALSE;
     yLINE[i].data.p= NULL;
     yLINE[i].data.v= 0;
   }

   if( strimem(yLINE[0].name, C) != 0 )
     return 1;

   C= inpLine + strlen(yLINE[0].name);
   while( *C != '\0' )
   {
     C= skipBlank(C);
     if( *C == '\0' )
       break;

     found= FALSE;
     for(i= 1; yLINE[i].name != NULL; i++)
     {
       if( strimem(yLINE[i].name, C) == 0 )
       {
         found= TRUE;
         break;
       }
     }
     if( !found || *(C + strlen(yLINE[i].name)) != '=' )
     {
       yLINE[0].isSet= TRUE;
       fprintf(stderr, "%s: invalid item '%s' in '%s'\n",
               yLINE[0].name, C, inpLine);
       return 1;
     }

     yLINE[i].isSet= TRUE;
     C += strlen(yLINE[i].name) + 1;
     if( yLINE[i].type == Yline::TYPE_PTR )
     {
       yLINE[i].data.p= C;
       break;
     }

     else if( yLINE[i].type == Yline::TYPE_DEC )
       yLINE[i].data.v= parseDec(C);

     else if( yLINE[i].type == Yline::TYPE_HEX )
       yLINE[i].data.v= parseHex(C);

     else
       assert( SHOULD_NOT_OCCUR );

     if( *C != ' ' && *C != '\0' )
     {
       yLINE[0].isSet= TRUE;
       fprintf(stderr, "%s: invalid '%s=' value in '%s'\n",
               yLINE[0].name, yLINE[i].name, inpLine);
       return 1;
     }
   }

   result= 0;
   for(i= 1; yLINE[i].name != NULL; i++)
   {
     if( !yLINE[i].isSet && yLINE[i].isReq )
     {
       yLINE[0].isSet= TRUE;
       fprintf(stderr, "%s: missing '%s=' in '%s'\n",
               yLINE[0].name, yLINE[i].name, inpLine);
       result= 1;
     }
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       ylGET
//
// Purpose-
//       Extract a YL element.
//
//----------------------------------------------------------------------------
static const Yline*                 // The element
   ylGET(                           // Extract Yline element
     const char*       name,        // The value name
     Yline*            yLINE)       // The Yline array
{
   int                 i;

   for(i= 0; yLINE[i].name != NULL; i++)
   {
     if( stricmp(name, yLINE[i].name) == 0 )
       return &yLINE[i];
   }

   assert( SHOULD_NOT_OCCUR );
   return NULL;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       decodeyn
//
// Purpose-
//       yEnc decode STDIN into a file.
//
//----------------------------------------------------------------------------
static int                          // Return code
   decodeyn(                        // yEnc decode a file
     const char*       fileName)    // -> Target filename
{
   int                 result;      // Resultant

   YncodeCodec         codec;       // Codec object
   FileReader          inp;         // Source file (stdin)
   FileWriter          out;         // Target file
   TempBuffer          temp;        // Extracted file
   const Yline*        yLINE;       // -> yLINE

   int                 rc;

   //-------------------------------------------------------------------------
   // Open the files
   //-------------------------------------------------------------------------
   rc= inp.open(NULL);              // (stdin)
   if( rc != 0 )
   {
     fprintf(stderr, "File(<stdin) ");
     perror("Open failure");
     return -1;
   }

   rc= out.open(fileName);          // Open the file
   if( rc != 0 )
   {
     fprintf(stderr, "File(%s) ", fileName);
     perror("Open failure");
     return -1;
   }

   rc= temp.open(fileName, Media::MODE_WRITE);
   if( rc != 0 )
   {
     fprintf(stderr, "%4d: File(%s) TEMP open[WR] failure(%d)\n",
                     __LINE__, fileName, rc);
     return -1;
   }

   //-------------------------------------------------------------------------
   // Extract the header/trailer
   //-------------------------------------------------------------------------
   {{{{
     AutoPointer ptr(LINE_SIZE);
     char* inpLine= (char*)ptr.get();

     // Find and parse start delimiter
     result= 0;
     for(;;)
     {
       rc= inp.readLine(inpLine, LINE_SIZE);
       if( rc < 0 )                 // End of file or error
       {
         if( rc == Reader::RC_SKIP )// If overlength, ignore
           continue;

         fprintf(stderr, "File(%s): rc(%d)", fileName, rc);
         perror("Read error");
         result= (-2);
         break;
       }

       if( parseYL(inpLine, yBEGIN) == 0 )
       {
         if( ylGET("total", yBEGIN)->isSet && !ylGET("part", yBEGIN)->isSet )
         {
           fprintf(stderr, "%s: 'total=' without 'part=' in '%s'\n",
                   yBEGIN[0].name, inpLine);
           continue;
         }
         break;
       }
     }

     // Parse the part line
     if( result == 0 && ylGET("part", yBEGIN)->isSet )
     {
       rc= inp.readLine(inpLine, LINE_SIZE);
       if( rc < 0 )                 // End of file or error
         result= (-2);

       else if ( parseYL(inpLine, yPART) != 0 )
         result= (-2);
     }

     // Copy into temporary
     while( result == 0 )
     {
       rc= inp.readLine(inpLine, LINE_SIZE);
       if( rc < 0 )
       {
         result= (-1);
         fprintf(stderr, "File(<): rc(%d)", rc);
         perror("Read error");
         break;
       }

       // Handle empty line
       if( inpLine[0] == '\0' )
         continue;

       // Handle "=yend" line
       if( memicmp("=yend ", inpLine, 6) == 0 )
         break;

       // Load the data line
       temp.printf("%s\n", inpLine);
     }

     // Parse the end delimiter
     if( result == 0 )
     {
       if( parseYL(inpLine, yEND) != 0 )
         result= (-3);
     }

     temp.close();
   }}}}

   //-------------------------------------------------------------------------
   // Decode the file
   //-------------------------------------------------------------------------
   if( result == 0 )
   {
     rc= temp.open(fileName, Media::MODE_READ);
     if( rc != 0 )
     {
       fprintf(stderr, "%4d: File(%s) TEMP open[RD] failure(%d)\n",
                       __LINE__, fileName, rc);
       result= (-1);
     }

     else
       result= codec.decode(temp, out);
   }
   out.close();

   // Validate the result
   if( result == 0 )
   {
     yLINE= ylGET("size", yEND);
     if( ylGET("size", yBEGIN)->data.v != yLINE->data.v )
     {
       result= (-4);
       fprintf(stderr, "Inconsistent ybegin(%u)/yend(%u) 'size='\n",
               (uint32_t)ylGET("size", yBEGIN)->data.v, (uint32_t)yLINE->data.v);
     }
     if( yLINE->data.v != codec.getSize() )
     {
       result= (-4);
       fprintf(stderr, "Invalid size: expected(%u) got(%u)\n",
               (uint32_t)yLINE->data.v, (uint32_t)codec.getSize());
     }

     yLINE= ylGET("crc32", yEND);
     if( yLINE->isSet
         && yLINE->data.v != codec.getSum() )
     {
       result= (-4);
       fprintf(stderr, "Invalid CRC32: expected(%.8x) got(%.8x)\n",
               (uint32_t)yLINE->data.v, (uint32_t)codec.getSum());
     }

     yLINE= ylGET("pcrc32", yEND);
     if( yLINE->isSet
         && yLINE->data.v != codec.getSum() )
     {
       fprintf(stderr, "Invalid PCRC32: expected(%.8x) got(%.8x)\n",
               (uint32_t)yLINE->data.v, (uint32_t)codec.getSum());
     }
   }

   return result;
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

     try {
       rc= decodeyn(argv[i]);       // Decode into file
       if( rc != 0 )                // If failure
       {
         fprintf(stderr, "Decode failed(%d): %s\n", rc, argv[i]);
         returncd= 1;               // Indicate it
       }
     } catch(const char* X) {
       fprintf(stderr, "Exception: %s\n", X);
     } catch(...) {
       fprintf(stderr, "Exception: ...\n");
     }
   }

   //-------------------------------------------------------------------------
   // Return
   //-------------------------------------------------------------------------
   return returncd;
}

