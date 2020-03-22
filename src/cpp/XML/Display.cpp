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
//       Display.cpp
//
// Purpose-
//       XML unit test.
//
// Last change date-
//       2018/01/01
//
// Parameters-
//       --stripText (Remove text before writing XML debug information)
//       [Name of input file.] Default is Display.inp
//
// Input-
//       Display.inp (Any XML file.)
//
// Output-
//       stdout: XML debug information.
//       Display.out (First XML statement in serialized form)
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
#include <com/Writer.h>
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
static const char*     SOURCE_FILE; // The source file name
static int             STRIP_TEXT= FALSE; // --stripText flag

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
//       deleteText
//
// Purpose-
//       Delete text from nodes and subnodes
//
//----------------------------------------------------------------------------
static void
   deleteText(                      // Delete text from
     XmlNode*          node)        // This node (and all child nodes)
{
   XmlNode* link= node->getChild();
   while( link != NULL )
   {
     XmlNode* next= link->getNext();
     if( link->getType() == XmlNode::TYPE_TEXT )
     {
       link->detach();              // Detach the text node
       delete link;                 // Delete the text node
     }

     link= next;
   }

   link= node->getChild();          // Delete text from child nodes
   while( link != NULL )
   {
     deleteText(link);
     link= link->getNext();
   }
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
   FileReader reader(SOURCE_FILE);  // The input file
   if( reader.getState() != reader.STATE_INPUT )
   {
     fprintf(stderr, "File(%s): NOT READABLE\n", SOURCE_FILE);
     exit(EXIT_FAILURE);
   }

   XmlParser parser;
   XmlNode* root= parser.parse(reader);
   if( root == NULL )
     throwf("No XML statement found");

   reader.close();

   if( STRIP_TEXT )
   {
     deleteText(root);
     root->debug();
     debugf("\n");

     return;
   }

   parser.setEntity("this", "that");
   parser.debug();
   debugf("\n");

   debugf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
     debugf("%s\n", parser.toString().c_str());
   debugf("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");

   FileWriter writer("Display.out");
   parser.output(writer);
   writer.printf("\n");
   writer.close();
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
           "Display: Display XML information\n"
           "\n"
           "Options:\n"
           "  --stripText\tRemove text nodes\n"
           "\n"
           "Parameters:\n"
           "  (NONE.)\n"
           "Input: File in XML format\n"
           "Output: Display.out (First XML statement)\n"
           "Output: stdout (XML debug information)\n"
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
   STRIP_TEXT= FALSE;

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

       else if( strcmp("--stripText", argp) == 0 )
         STRIP_TEXT= TRUE;

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
     SOURCE_FILE= "Display.inp";

   if( HELPI || ERROR )
   {
     if( ERROR )
       error("\n");

     info();
   }

   if( verify )
   {
     fprintf(stderr, "Source: '%s'\n", SOURCE_FILE);
     fprintf(stderr, "%10s --stripText\n", STRIP_TEXT ? "TRUE" : FALSE);
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

