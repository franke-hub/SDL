//----------------------------------------------------------------------------
//
//       Copyright (c) 2020-2021 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Fileman.cpp
//
// Purpose-
//       Display directory and subdirectory.
//
// Last change date-
//       2021/01/04
//
// Flags-
//       -d (directory)
//          Only display directory entries.
//
//       -f (files)
//          Only display files.
//
//       -l (Links)
//          Only display links.
//
//       -n (Names)
//          Display directory, file, and link names.
//
//       --print0 (Use '\0' delimiter)
//          Use '\0' rather than '\n' delimiter.
//
//       --verbose (Vebose)
//          Run noisily.
//
//----------------------------------------------------------------------------
#include <assert.h>                 // For debugging
#include <getopt.h>                 // For getopt_long()
#include <limits.h>                 // For INT_MAX, INT_MIN
#include <stdio.h>                  // For printf, fprintf, ...
#include <string.h>                 // For memset
#include <stdlib.h>                 // For exit
#include <unistd.h>                 // For readlink
#include <pub/utility.h>            // For pub::utility::atoi

#include "pub/Fileman.h"

using pub::fileman::Path;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// Month number to name array
//----------------------------------------------------------------------------
static const char*     month[]=     // Month of year
{  "Jan"
,  "Feb"
,  "Mar"
,  "Apr"
,  "May"
,  "Jun"
,  "Jul"
,  "Aug"
,  "Sep"
,  "Oct"
,  "Nov"
,  "Dec"
};

//----------------------------------------------------------------------------
// Options
//----------------------------------------------------------------------------
static int             opt_dirs= false; // Display directory names
static int             opt_file= false; // Display file names
static int             opt_help= false; // Display usage information
static int             opt_index= 0; // The current option index
static int             opt_link= false; // Display link names
static int             opt_name= false; // Display names (omit attributes)
static int             opt_print0= false; // Use '\0' rather than '\n' delimiter
static int             opt_verbose= false; // Verbose mode

static const char*     OSTR= ":dfln"; // The getopt_long optstring parameter
static struct option   OPTS[]=      // The getopt_long longopts parameter
{  {"help",    no_argument,       &opt_help,    true}

,  {"print0",  no_argument,       &opt_print0,  true}
,  {"verbose", no_argument,       &opt_verbose, true}
,  {0, 0, 0, 0}                     // (End of option list)
};

enum OPT_INDEX
{  OPT_PRINT0= 1
,  OPT_VERBOSE= 2
,  OPT_SIZE= 3
};

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
   fprintf(stderr,"\n");
   fprintf(stderr,"Fileman <options> directory ...\n");
   fprintf(stderr,"Recursively list directory content.\n");
   fprintf(stderr,"This can be especially useful when used in conjunction\n"
                  "with xargs, as in:\n"
                  "\tFileman -d | xargs -0 chmod a+rx\n"
                  "\tFileman -f | xargs -0 chmod a+r\n"
                  );

   fprintf(stderr,"\n");
   fprintf(stderr,"Options:\n");
   fprintf(stderr,"-d (Directory) Display directory names\n");
   fprintf(stderr,"-f (File) Display file names\n");
   fprintf(stderr,"-l (Link) Display link names\n");
   fprintf(stderr,"-n Display directory, file, and link names\n");
   fprintf(stderr,"--print0 Use \\0 rather than \\n delimiter\n");
   fprintf(stderr,"--verbose Display debugging information\n");

   exit(1);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       parm_int
//
// Purpose-
//       Convert parameter to integer, handling error cases
//
// Implementation note-
//       optarg: The argument string
//       opt_index: The argument index
//
//----------------------------------------------------------------------------
static int                          // The integer value
   parm_int( void )                 // Extract and verify integer value
{
   errno= 0;
   int value= pub::utility::atoi(optarg);
   if( errno ) {
     opt_help= true;
     if( errno == ERANGE )
       fprintf(stderr, "--%s, range error: '%s'\n", OPTS[opt_index].name, optarg);
     else if( *optarg == '\0' )
       fprintf(stderr, "--%s, no value specified\n", OPTS[opt_index].name);
     else
       fprintf(stderr, "--%s, format error: '%s'\n", OPTS[opt_index].name, optarg);
   }

   return value;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       parm
//
// Purpose-
//       Parameter analysis.
//
//----------------------------------------------------------------------------
static void
   parm(                            // Parameter analysis
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   //-------------------------------------------------------------------------
   // Parameter analysis
   //-------------------------------------------------------------------------
   int C;                           // The option character
   while( (C= getopt_long(argc, argv, OSTR, OPTS, &opt_index)) != -1 )
   {
     switch( C )
     {
       case 0:
       {{{{
         switch( opt_index )
         {
           case OPT_VERBOSE:
             if( optarg )
               opt_verbose= parm_int();
             break;

           default:
             break;
         }
         break;
       }}}}

       case 'd':
         opt_dirs= true;
         opt_name= true;
         break;

       case 'f':
         opt_file= true;
         opt_name= true;
         break;

       case 'l':
         opt_link= true;
         opt_name= true;
         break;

       case 'n':
         opt_dirs= true;
         opt_file= true;
         opt_link= true;
         opt_name= true;
         break;

       case ':':
         opt_help= true;
         if( optopt == 0 )
           fprintf(stderr, "%4d Option requires an argument '%s'.\n", __LINE__,
                           argv[optind-1]);
         else
           fprintf(stderr, "%4d Option requires an argument '-%c'.\n", __LINE__,
                           optopt);
         break;

       case '?':
         opt_help= true;
         if( optopt == 0 )
           fprintf(stderr, "%4d Unknown option '%s'.\n", __LINE__,
                           argv[optind-1]);
         else if( isprint(optopt) )
           fprintf(stderr, "%4d Unknown option '-%c'.\n", __LINE__, optopt);
         else
           fprintf(stderr, "%4d Unknown option character '0x%x'.\n", __LINE__,
                           optopt);
         break;

       default:
         fprintf(stderr, "%4d ShouldNotOccur ('%c',0x%x).\n", __LINE__, C, C);
         break;
     }
   }

   if( opt_help )
     info();

   if( ! (opt_dirs | opt_file | opt_link) )  {
     opt_dirs= true;
     opt_file= true;
     opt_link= true;
   }
}

//----------------------------------------------------------------------------
//
// Class-
//       File
//
// Purpose-
//       File name information, adding print methods
//
//----------------------------------------------------------------------------
class File : public pub::fileman::File { // File name information
public:
using pub::fileman::File::File;     // Use ::pub::fileman::File constructor

//----------------------------------------------------------------------------
// File::Methods
//----------------------------------------------------------------------------
public:
void
   print(                           // Format and print directory element
     std::string       path);       // Associated path name

void
   print_date( void );              // Display date

void
   print_mode( void );              // Display mode

void
   print_name(                      // Display name
     std::string       path);       // Associated path name

void
   print_size( void );              // Display size
}; // class File

//----------------------------------------------------------------------------
//
// Method-
//       File::print
//
// Purpose-
//       Format and print (to stdout)
//
//----------------------------------------------------------------------------
void
   File::print(                     // Format and print
     std::string       path)        // Using this path name
{
   print_mode();
   print_date();
   print_size();
   print_name(path);
}

//----------------------------------------------------------------------------
//
// Method-
//       File::print_date
//
// Purpose-
//       Format and print date (to stdout)
//
//----------------------------------------------------------------------------
void
   File::print_date( void )         // Format and print date
{
   struct tm* T= localtime(&st.st_mtime);
   printf("%s %2d %4d %.2d:%.2d ", month[T->tm_mon]
          , T->tm_mday, T->tm_year+1900, T->tm_hour, T->tm_min);
}

//----------------------------------------------------------------------------
//
// Method-
//       File::print_mode
//
// Purpose-
//       Format and print mode (to stdout)
//
//----------------------------------------------------------------------------
void
   File::print_mode( void )         // Format and print mode
{
   char buffer[16];                 // Working buffer

   memset(buffer, 0, sizeof(buffer));
   memset(buffer, '-', 10);

   if( S_ISDIR(st.st_mode) )
     buffer[0]= 'd';
   else if( S_ISLNK(st.st_mode) )
     buffer[0]= 'l';

   if( st.st_mode & S_IRUSR )
     buffer[1]= 'r';
   if( st.st_mode & S_IWUSR )
     buffer[2]= 'w';
   if( st.st_mode & S_IXUSR )
     buffer[3]= 'x';

   if( st.st_mode & S_IRGRP )
     buffer[4]= 'r';
   if( st.st_mode & S_IWGRP )
     buffer[5]= 'w';
   if( st.st_mode & S_IXGRP )
     buffer[6]= 'x';

   if( st.st_mode & S_IROTH )
     buffer[7]= 'r';
   if( st.st_mode & S_IWOTH )
     buffer[8]= 'w';
   if( st.st_mode & S_IXOTH )
     buffer[9]= 'x';

   buffer[10]= '.';
   printf("%s ", buffer);
}

//----------------------------------------------------------------------------
//
// Method-
//       File::print_name
//
// Purpose-
//       Format and print name (to stdout)
//
//----------------------------------------------------------------------------
void
   File::print_name(                // Format and print name
     std::string       path)        // Using this path name
{
   std::string full= path + "/" + name;
   printf("%s", full.c_str());
   if( S_ISLNK(st.st_mode) )
   {
     char buffer[FILENAME_MAX];
     ssize_t rc= readlink(full.c_str(), buffer, sizeof(buffer)-1);
     if( rc < 0 )
       printf(" -> ?????");
     else {
       buffer[rc]= '\0';
       printf(" -> %s", buffer);
     }
   }
   printf("\n");
}

//----------------------------------------------------------------------------
//
// Method-
//       File::print_size
//
// Purpose-
//       Format and print size (to stdout)
//
//----------------------------------------------------------------------------
void
   File::print_size( void )         // Format and print size
{
   double size= st.st_size;
   if( size < 1000.0 )
     printf("%4.0f ", size);
   else if( size <            10000.0 )
     printf("%3.1fK ", size /          1000.0);
   else if( size <          1000000.0 )
     printf("%3.0fK ", size /          1000.0);
   else if( size <         10000000.0 )
     printf("%3.1fM ", size /       1000000.0);
   else if( size <       1000000000.0 )
     printf("%3.0fM ", size /       1000000.0);
   else if( size <      10000000000.0 )
     printf("%3.1fG ", size /    1000000000.0);
   else if( size <    1000000000000.0 )
     printf("%3.0fG ", size /    1000000000.0);
   else if( size <   10000000000000.0 )
     printf("%3.1fT ", size / 1000000000000.0);
   else if( size < 1000000000000000.0 )
     printf("%3.0fT ", size / 1000000000000.0);
   else
     printf("HUGE ");
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       listDirectory
//
// Function-
//       List the contents of a directory and its subdirectories.
//
//----------------------------------------------------------------------------
static void
   listDirectory(                   // List the contents of a directory
     std::string       name)        // Path to directory
{
   Path                path(name);  // The directory content

   //-------------------------------------------------------------------------
   // Debugging
   //-------------------------------------------------------------------------
   if( opt_verbose > 1 )
     fprintf(stderr, "D: %s\n", name.c_str());

   //-------------------------------------------------------------------------
   // List items in this directory
   //-------------------------------------------------------------------------
   for(File* file= static_cast<File*>(path.list.get_head()); file != nullptr;
       file= static_cast<File*>(file->get_next())) {
     if( opt_verbose > 1 )
       fprintf(stderr, "F: %.8x %10ld %s/%s\n", file->st.st_mode,
               file->st.st_size, name.c_str(), file->name.c_str());
     bool valid= false;             // Default, not valid
     if( S_ISDIR(file->st.st_mode) ) {
       if( opt_dirs ) valid= true;
     } else if( S_ISLNK(file->st.st_mode) ) {
       if( opt_link ) valid= true;
     } else if( S_ISREG(file->st.st_mode) ) {
       if( opt_file ) valid= true;
     }

     if( valid )                    // If valid
     {
       if( opt_name )               // If only name required
         printf("%s/%s%c", name.c_str(), file->name.c_str(),
                opt_print0 ? '\0' : '\n');
       else
         file->print(name);         // Display complete information
     }
   }

   //-------------------------------------------------------------------------
   // Process subdirectories
   //-------------------------------------------------------------------------
   for(File* file= static_cast<File*>(path.list.get_head()); file != nullptr;
       file= static_cast<File*>(file->get_next())) {
     if( S_ISDIR(file->st.st_mode) ) // If this is a directory
       listDirectory(name + "/" + file->name); // Process the subdirectory
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
extern int                          // Return code
   main(                            // Mainline code
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   int                 argi;        // Argument index

   //-------------------------------------------------------------------------
   // Parameter analysis
   //-------------------------------------------------------------------------
   parm(argc, argv);

   //-------------------------------------------------------------------------
   // List specified directories
   //-------------------------------------------------------------------------
   if( optind >= argc )
     listDirectory(".");
   else
   {
     for(argi= optind; argi<argc; argi++)
       listDirectory(argv[argi]);
   }

   //-------------------------------------------------------------------------
   // Function complete
   //-------------------------------------------------------------------------
   return(0);
}

