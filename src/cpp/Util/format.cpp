//----------------------------------------------------------------------------
//
//       Copyright (c) 2012-2026 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
// SPDX-License-Identifier: GPL-3.0-only
//-----------------------------------------------------------------------------
//
// Title-
//       format.cpp
//
// Purpose-
//       Format stdin, writing to stdout.
//
// Last change date-
//       2026/05/12
//
// Implementation notes-
//       if --mode:dos or --mode:unix is specified:
//           '\r' characters are ignored
//           '\0' and '\n' characters are treated as line terminators and
//               empty the output buffer, removing trailing blanks.
//       else
//           '\0', '\r', and '\n' characters are treated as characters, and
//               empty the output buffer.
//
// Options-
//       --hcdm        [Hard Core Debug Mode]
//       --verbose{:n} [Verbosity, higher is more verbose]
//
//       --fix:empty   [Remove completely blank lines]
//       --fix:bs      [Handle backspaces]
//
//       --mode:copy   [Copy without modification]
//                     (Conflicts with --fix:bs and --fix:empty)
//       --mode:dos    [End each line with "\r\n"]
//       --mode:unix   [(Default) End each line with "\n"]
//       --mode:none   [Copy without changing line endings]
//                     (Conflicts with --fix:empty)
//
//       --format.txt  [Write test file, not documented in info()]
//
//       {input file name}  (Default: STDIN)
//       {output file name} (Default: STDOUT)
//
//----------------------------------------------------------------------------
#include <cerrno>                   // For errno, strerrno
#include <cstdio>                   // For fprintf
#include <cstdlib>                  // For atoi, exit
#include <cstring>                  // For strcmp

#include <unistd.h>                 // For write, STDOUT_FILENO
#include <fcntl.h>                  // For open, close, O_*, S_*

#include <pub/Debug.h>              // For namespace pub::debugging

#define PUB _LIBPUB_NAMESPACE
using namespace PUB::debugging;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum                                // Generic enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose

,  BUFFER_SIZE= 16'777'216          // Allocated buffer size
}; // Generic enum

enum MODE                           // Output mode
{  MODE_COPY                        // Copy as-is, no conversion
,  MODE_NONE                        // No line ending conversion, --fix::bs OK
,  MODE_DOS                         // Convert to DOS format
,  MODE_UNIX                        // Convert to UNIX format
}; // enum MODE

//----------------------------------------------------------------------------
// Local data areas
//----------------------------------------------------------------------------
static int             mode= MODE_UNIX; // Conversion mode

static int             opt_hcdm= HCDM; // --hcdm
static int             opt_verbose= VERBOSE; // --verbose

static int             opt_empty= false; // --fix:empty
static int             opt_fixbs= false; // --fix:bs

static char*           buffer_area= nullptr;
static int             buffer_used= 0; // Current buffer index

static int             fd_inp= STDIN_FILENO;  // Input file descriptor
static int             fd_out= STDOUT_FILENO; // Output file descriptor

static const char*     fn_inp= "<1";
static const char*     fn_out= "2>";

//----------------------------------------------------------------------------
// Sample test file
//----------------------------------------------------------------------------
const char*            test_file=
    "Copyright: NONE (Test data)\n"
    "\n"
    "format.cpp test file\n"
    "    \n"
    "The prior line contained blanks\n"
    "This line\r has strange formatting\0donchano?\n"
    "This line containscontains\b\b\b\b\b\b\b\b backspace characers\n"
    "This line contains trailing blanks    \n"
    "Final line"                    // (With missing '\n')
    ;
const size_t           test_size= 235; // (Verify when test file changes)

//----------------------------------------------------------------------------
//
// Subroutine-
//       modeName
//
// Purpose-
//       Name associated with -mode parameter
//
//----------------------------------------------------------------------------
static const char*                  // -mode: parameter
   modeName( void )                 // Get -mode parameter name
{
   if( mode == MODE_COPY )
     return "COPY";
   if( mode == MODE_DOS )
     return "DOS";
   if( mode == MODE_UNIX )
     return "UNIX";
   return "NONE";
}

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
   fprintf(stderr, "format {options} {input-file {output-file}}\n"
           "  Copy input-file to output-file\n"
           "  (Defaults: STDIN, STDOUT)\n"
           "\n"
           "Options:\n"
           "  --help\tDisplay this help message\n"
           "  --hcdm\tHard Core Debug Mode\n"
           "  --verbose{:n}\tVerbosity, higher is more verbose\n"
           "\n"
           "  --fix:bs\tRemoves backspaces (removing characters)\n"
           "  --fix:empty\tRemoves empty lines\n"
           "\n"
           "  --mode:none\tDoes not modify line endings\n"
           "  \t\tConflicts with --fix:empty\n"
           "  --mode:copy\tDoes not modify anything\n"
           "  \t\tConflicts with --fix:bs or --fix:empty\n"
           "  --mode:dos\tEnds each line with \\r\\n, "
             "removing trailing blanks\n"
           "  --mode:unix\t(Default) Ends each line with \\n, "
             "removing trailing blanks\n"
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
   char*               argp;        // Argument pointer
   int                 argi;        // Argument index

   int                 opt_index= 0; // Curent parameter index
   int                 help= 0;     // Help needed?

   //-------------------------------------------------------------------------
   // Argument analysis
   //-------------------------------------------------------------------------
   for( argi=1; argi<argc; ++argi ) { // Analyze variable controls
     argp= argv[argi];              // Address the parameter
     if( memcmp(argp, "--", 2) == 0 ) { // If switch parameter
       argp += 2;
       if( strcmp(argp, "help") == 0 )
         help= 1;
       else if( strcmp(argp, "hcdm") == 0 )
         opt_hcdm= true;
       else if( memcmp(argp, "verbose", 7) == 0 ) {
         opt_verbose= true;
         if( strlen(argp) > 7 ) {     // If parameter specified
           if( strlen(argp) == 6 || argp[7] != ':' ) { // If malformed
             help= 2;
             fprintf(stderr, "Invalid parameter '%s'\n", argp);
             continue;
           } else {
             opt_verbose= atoi(argp+8);
           }
         }
       }
       else if( strcasecmp(argp, "fix:bs") == 0 )
         opt_fixbs= true;
       else if( strcasecmp(argp, "fix:empty") == 0 )
         opt_empty= true;
       else if( strcasecmp(argp, "mode:copy") == 0 )
         mode= MODE_COPY;
       else if( strcasecmp(argp, "mode:dos") == 0 )
         mode= MODE_DOS;
       else if( strcasecmp(argp, "mode:unix") == 0 )
         mode= MODE_UNIX;
       else if( strcasecmp(argp, "mode:none") == 0 )
         mode= MODE_NONE;
       else if( strcasecmp(argp, "format.txt") == 0 ) {
         // "Secret" command to create test file
         FILE* f= fopen("format.txt", "w");
         if( f ) {
           fwrite(test_file, test_size, 1, f);
           fclose(f);
         }
       } else {
         help= 2;
         fprintf(stderr, "ERROR: Invalid parameter '--%s'\n", argp);
       }
     } else {                       // If filename parameter
       if( opt_index == 0 ) {       // STDIN replacement
         ++opt_index;               // STDOUT is next
         int fd= open(argp, O_RDONLY);  // Open input file
         if( fd >= 0 ) {            // If open succeeded
           fd_inp= fd;
           fn_inp= argp;
         } else  {                  // If open failed
           help= 2;
           fprintf(stderr, "ERROR: Cannot open(%s) %d:%s\n"
                           , argp, errno, strerror(errno));
         }
       } else if( opt_index == 1 ) { // STDOUT replacement
         ++opt_index;               // STDOUT is next
         int    opts= O_WRONLY | O_CREAT | O_TRUNC;
         mode_t mode= S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
         int fd= open(argp, opts, mode);  // Open output file
         if( fd >= 0 ) {            // If open succeeded
           fd_out= fd;
           fn_out= argp;
         } else  {                  // If open failed
           help= 2;
           fprintf(stderr, "ERROR: Cannot open(%s) %d:%s\n"
                           , argp, errno, strerror(errno));
         }
       } else {                     // If too many parameters
         help= 2;
         fprintf(stderr, "ERROR: Too many parameters: '%s'\n", argp);
       }
     }
   }

   //-------------------------------------------------------------------------
   // Conflict analysis
   //-------------------------------------------------------------------------
   if( opt_fixbs && mode == MODE_COPY ) {
     help= 2;
     fprintf(stderr, "ERROR: --mode:copy conflicts with --fix:bs\n");
   }

   if( opt_empty && (mode == MODE_COPY || mode == MODE_NONE) ) {
     help= 2;
     if( mode == MODE_COPY )
       fprintf(stderr, "ERROR: --mode:%s conflicts with --fix:empty\n", "copy");
     if( mode == MODE_NONE )
       fprintf(stderr, "ERROR: --mode:%s conflicts with --fix:empty\n", "none");
   }

   //-------------------------------------------------------------------------
   // Completion analysis
   //-------------------------------------------------------------------------
   if( help ) {                     // If assistance required
     if( help > 1 )                 // If parameter error
       fprintf(stderr, "\n\n");
     info();
   }

   if( opt_verbose ) {
     fprintf(stderr, "%5s --hcdm\n",      opt_hcdm ? "TRUE" : "FALSE");
     fprintf(stderr, "%5d --verbose\n",   opt_verbose);

     fprintf(stderr, "%5s --fix:bs\n",    opt_fixbs ? "TRUE" : "FALSE");
     fprintf(stderr, "%5s --fix:empty\n", opt_empty ? "TRUE" : "FALSE");
     fprintf(stderr, "%5s --mode\n",      modeName());
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       addto_buffer
//
// Purpose-
//       Add character to output buffer
//
// Delimiter-
//       '-1' end of file detected with buffer present
//       '\0' end of string character detected
//       '\r' carriage return detected
//       '\n' line feed detected
//
//----------------------------------------------------------------------------
static void
   addto_buffer(                    // Add to the output buffer
     int               I)           // This input character
{
   if( buffer_used < BUFFER_SIZE ) { // If there's room
     buffer_area[buffer_used++]= I;
     return;
   }

   // There's no room at the buffer_area inn
   // We've got ourselves a 16M buffer here. That's a heckofa large line.
   // We don't expect this code to be used, and haven't tested it.

   // We could do something fancy like write the first half of the buffer and
   // move the remainder to the start, but this shouldn't be needed.
   // Maybe a later version of this code will use a 64M buffer. Who knows?

   // Simple error recovery procedure: Write an error message and exit.
   fprintf(stderr, "Input line too long (>16M). Copy aborted\n");
   exit(2);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       empty_buffer
//
// Purpose-
//       Physically write the output buffer
//
//----------------------------------------------------------------------------
static void
   empty_buffer( void )             // Write the output buffer
{
   if( opt_hcdm || opt_verbose )
     tracef("empty_buffer() buffer_used(%d)\n", buffer_used);

   if( buffer_used == 0 )           // (Do nothing if buffer is already empty)
     return;

   ssize_t size= write(fd_out, buffer_area, buffer_used);
   if( opt_hcdm || opt_verbose > 1 )
     tracef("%zd= write(%d,%p,%d)\n", size, fd_out, buffer_area, buffer_used);
   if( size == (ssize_t)buffer_used ) {
     buffer_used= 0;
     return;
   }

   // Error recovery procedure
   fprintf(stderr, "Write error %d:%s, {%zd != %zd}, Copy aborted\n"
                 , errno, strerror(errno), size, (size_t)buffer_used);
   exit(2);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       rd_char
//
// Purpose-
//       Get the next input character
//
//----------------------------------------------------------------------------
static int                          // Return code, character read or EOF
   rd_char( void )                  // Read the next input character
{
   if( opt_hcdm || opt_verbose > 1 )
     tracef("rd_char()\n");

   unsigned char buffer[8];         // The input buffer

   ssize_t size= read(fd_inp, buffer, 1); // Get next character
   if( opt_hcdm || opt_verbose > 1 ) {
     if( size == 1 )
       tracef("%zd= read(%d,%p,%d) '%c'\n", size, fd_inp, buffer, 1
             , buffer[0]);
     else if( size == 0 )
       tracef("%zd= read(%d,%p,%d) EOF\n", size, fd_inp, buffer, 1);
     else
       tracef("%zd= read(%d,%p,%d) ERR\n", size, fd_inp, buffer, 1);
   }
   if( size == 1 )
     return buffer[0];
   if( size == 0 )
     return EOF;

   // Error recovery procedure
   fprintf(stderr, "Read error %d:%s, Copy aborted\n", errno, strerror(errno));
   exit(2);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       write_buffer
//
// Purpose-
//       Write the output buffer
//
// Delimiter-
//       '-1' end of file detected with buffer present
//       '\0' end of string character detected
//       '\r' carriage return detected
//       '\n' line feed detected
//
//----------------------------------------------------------------------------
static void
   write_buffer(                    // Write the output buffer
     int               delim= -1)   // Delimiter
{
   if( opt_hcdm || opt_verbose )
     tracef("write_buffer(%d)\n", delim);

   if( mode == MODE_COPY || mode == MODE_NONE ) { // If not modifying line end
     if( delim >= 0 )
       addto_buffer(delim);
     empty_buffer();
     return;
   }

   // Mode == MODE_DOS || mode == MODE_UNIX
   if( delim == '\r' )              // Ignore carriage return
     return;

   // Remove trailing blanks
   while(buffer_used > 0 && buffer_area[buffer_used - 1] == ' ')
     --buffer_used;

   if( opt_empty && buffer_used == 0 ) // Handle ignored empty line
      return;

   // (The buffer area has spare space for the line termination sequence.)
   if( mode == MODE_DOS )
     buffer_area[buffer_used++]= '\r';
   buffer_area[buffer_used++]= '\n';
   empty_buffer();
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       inp2out
//
// Purpose-
//       Copy input file into output file.
//
//----------------------------------------------------------------------------
static void
   inp2out( void )                  // Copy stdin to stdout
{
   for(;;) {
     int I= rd_char();              // Get the next input character
     if( I < 0 )
       break;

     if( I == '\n' || I == '\r' || I == '\0' ) {
       write_buffer(I);             // Write the output buffer
       continue;
     }

     if( I == '\b' && opt_fixbs ) { // If fixable backspace
       if( buffer_used > 0 )
         --buffer_used;
       continue;
     }

     addto_buffer(I);               // Add character to buffer
   }

   // End of file
   if( buffer_used )                // If missing '\n' terminator
     write_buffer();                // We need to write the line
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
   //-------------------------------------------------------------------------
   // Parameter analysis
   //-------------------------------------------------------------------------
   parm(argc, argv);

   //-------------------------------------------------------------------------
   // Initialization
   //-------------------------------------------------------------------------
   buffer_area= (char*)malloc(BUFFER_SIZE + 8); // Allow space for end sequence

   //-------------------------------------------------------------------------
   // Copy input to output
   //-------------------------------------------------------------------------
   inp2out();

   //-------------------------------------------------------------------------
   // Termination
   //-------------------------------------------------------------------------
   free(buffer_area);

   if( fd_inp != STDIN_FILENO )
     close(fd_inp);
   if( fd_out != STDOUT_FILENO )
     close(fd_out);

   return 0;
}
