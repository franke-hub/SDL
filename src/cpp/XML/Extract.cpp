//----------------------------------------------------------------------------
//
//       Copyright (c) 2012-2018 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Extract.cpp
//
// Purpose-
//       Read the diagnostic log, extracting sample data items.
//
// Last change date-
//       2018/01/01
//
// Parameters-
//       Optional: Name of input file. Default is Extract.inp
//
// Input-
//       Extract.inp (XML file.)
//
//       Port opened at mm/dd/yyyy.
//       XML <msg> records
//       Port closed at mm/dd/yyyy.
//
// Output-
//       stdout: CSV: time, sensor, channel, ...
//
//----------------------------------------------------------------------------
#include <stdarg.h>                 // For error()
#include <stdio.h>                  // For printf
#include <stdlib.h>                 // For various
#include <string.h>                 // For strcmp

#include <string>
using std::string;

#include <com/Debug.h>
#include <com/Reader.h>
#include "com/XmlNode.h"
#include "com/XmlParser.h"

//----------------------------------------------------------------------------
// Constant for parameterization
//----------------------------------------------------------------------------
#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE  1
#endif

#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static char            inpBuff[65536]; // The input line buffer
static const char*     SOURCE_FILE; // The source file name

//----------------------------------------------------------------------------
//
// Subroutine-
//       error
//
// Purpose-
//       Write a message onto stderr
//
//----------------------------------------------------------------------------
static void
   error(                           // Write error message
     const char*       fmt,         // The PRINTF format string
                       ...)         // The remaining arguments
{
   va_list             argptr;      // Argument list pointer

   va_start(argptr, fmt);           // Initialize va_ functions
   vfprintf(stderr, fmt, argptr);
   va_end(argptr);                  // Close va_ functions
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       examine
//
// Purpose-
//       Read the log, extract node entries.
//
//----------------------------------------------------------------------------
static void
   examine( void )                  // Read the log
{
   FileReader          reader(SOURCE_FILE); // The input file
   int                 rc;

   if( reader.getState() != reader.STATE_INPUT )
   {
     fprintf(stderr, "File(%s): NOT READABLE\n", SOURCE_FILE);
     exit(EXIT_FAILURE);
   }

   for(;;)                          // Locate "Port opened" line
   {
     rc= reader.readLine(inpBuff, sizeof(inpBuff));
     if( rc == Reader::RC_EOF )
       break;

     if( memcmp(inpBuff, "Port opened", 11) == 0 )
       break;
   }

   if( rc == Reader::RC_EOF )
     throwf("File(%s), missing \"Port opened\" line\n", SOURCE_FILE);

   XmlParser parser;
   for(;;)
   {
//   XmlNode* root= XmlNode::parse(reader);
     XmlNode* root= parser.parse(reader);
     if( root == NULL )
       break;

     if( root->getName() != "msg" )
       throwf("Root name(%s) not 'msg'", root->getName().c_str());

     const XmlNode* time= root->getChild("time");
     if( time != NULL )
     {
       const XmlNode* channel[10];
       const XmlNode* sensor= root->getChild("sensor");

       int FOUND= FALSE;
       if( sensor != NULL )
       {
         for(int i= 0; i<10; i++)
         {
           channel[i]= NULL;
           char buffer[32];
           sprintf(buffer, "ch%d", i+1);
           const XmlNode* node= root->getChild(buffer);
           if( node != NULL )
           {
             channel[i]= node->getChild("watts");
             if( channel[i] != NULL )
               FOUND= TRUE;
           }
         }
       }

       if( FOUND )
       {
         printf("%s, %s",
//              time->getText().c_str(), sensor->getText().c_str());
                parser.getText(time).c_str(), parser.getText(sensor).c_str());
         for(int i= 0; i<10; i++)
         {
           if( channel[i] == NULL )
             printf(", N/A");
           else
//           printf(", %s", channel[i]->getText().c_str());
             printf(", %s", parser.getText(channel[i]).c_str());
         }
         printf("\n");
       }
     }

//   XmlNode::delTree(root);
   }

   reader.close();
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       info
//
// Purpose-
//       Parameter informational display.
//
//----------------------------------------------------------------------------
static void
   info( void )                     // Parameter analysis
{
   fprintf(stderr,
           "Extract: Extract log information\n"
           "\n"
           "Options:\n"
           "  (NONE.)\n"
           "\n"
           "Parameters:\n"
           "  (NONE.)\n"
           "Input: File in Extract.inp format\n"
           "Output: stdout (The CSV data entries)\n"
          );

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
   char*               argp= NULL;  // Argument parameter (NULL for glitch)
   int                 argi;        // Argument index

   int                 ERROR;       // Error encountered indicator
   int                 HELPI;       // Help encountered indicator
   int                 verify;      // Verification control

   //-------------------------------------------------------------------------
   // Defaults
   //-------------------------------------------------------------------------
   ERROR= FALSE;                    // Set defaults
   HELPI= FALSE;
   verify= FALSE;
   SOURCE_FILE= NULL;

   //-------------------------------------------------------------------------
   // Argument analysis
   //-------------------------------------------------------------------------
   for( argi=1; argi<argc; argi++ ) // Analyze variable controls
   {
     argp= argv[argi];              // Address the parameter

     if( *argp == '-' )             // If this parameter is in switch format
     {
       if( strcmp("-help", argp) == 0 // If help request
           || strcmp("--help", argp) == 0 )
         HELPI= TRUE;

       else if( strcmp("-verify", argp) == 0 )
         verify= TRUE;

       else                         // If invalid switch
       {
         ERROR= TRUE;
         error("Invalid parameter '%s'\n", argv[argi]);
       }
     }
     else                           // If filename parameter
     {
       if( SOURCE_FILE != NULL)
       {
         ERROR= TRUE;
         error("Unexpected file name '%s'\n", argv[argi]);
       }
       else
         SOURCE_FILE= argp;
     }
   }

   //-------------------------------------------------------------------------
   // Completion analysis
   //-------------------------------------------------------------------------
   if( SOURCE_FILE == NULL )
     SOURCE_FILE= "Extract.inp";

   if( HELPI || ERROR )
   {
     if( ERROR )
       error("\n");

     info();
   }

   if( verify )
   {
     fprintf(stderr, "Source: '%s'\n", SOURCE_FILE);
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
   parm(argc, argv);                // Parameter analysis

   try {
     examine();                     // Read the log
   } catch(const char* X) {
     error("Exception(%s)\n", X);
   } catch(string X) {
     error("Exception(%s)\n", X.c_str());
   } catch(...) {
     error("Unexpected exception(...)\n");
   }

   return EXIT_SUCCESS;
}

