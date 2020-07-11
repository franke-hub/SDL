//----------------------------------------------------------------------------
//
//       Copyright (C) 2020 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Shm_demo.cpp
//
// Purpose-
//       Sample program: POSIX shared memory example.
//
// Last change date-
//       2020/07/11
//
//----------------------------------------------------------------------------
#include <ctype.h>                  // For isprint()
#include <errno.h>                  // For errno
#include <fcntl.h>                  // For O_* constants
#include <getopt.h>                 // For getopt_long()
#include <limits.h>                 // For INT_MAX, INT_MIN
#include <stdarg.h>                 // For va_list
#include <stdio.h>                  // For printf
#include <stdlib.h>                 // For various
#include <string.h>                 // For strcmp
#include <unistd.h>                 // For ftruncate
#include <sys/mman.h>               // For mmap, shm_open, ...
#include <sys/stat.h>               // For mode constants
#include <sys/types.h>              // For type definitions

#include <com/Random.h>             // For Random

//----------------------------------------------------------------------------
// Options
//----------------------------------------------------------------------------
static int             opt_help= false; // --help (or error)
static int             opt_hcdm= false; // Hard Core Debug Mode
static int             opt_index;   // Option index

static int             opt_clean= false; // Remove files
static const char*     opt_file= nullptr; // Test file name
static const char*     opt_name= nullptr; // Test storage name
static int             opt_redo= false; // Re-verify storage
static int             opt_verbose= -1; // Verbosity

static const char*     OSTR= ":";   // The getopt_long optstring parameter
static struct option   OPTS[]=      // The getopt_long longopts parameter
{  {"help",    no_argument,       &opt_help,    true} // --help
,  {"hcdm",    no_argument,       &opt_hcdm,    true} // --hcdm

,  {"clean",   no_argument,       &opt_clean,   true} // --clean
,  {"file",    required_argument, nullptr,      0} // Shared memory file
,  {"name",    required_argument, nullptr,      0} // Shared memory name
,  {"redo",    no_argument,       &opt_redo,    true} // --redo (Re-verify)
,  {"verbose", optional_argument, &opt_verbose, 0} // --verbose
,  {0, 0, 0, 0}                     // (End of option list)
};

enum OPT_INDEX                      // Must match OPTS[]
{  OPT_HELP
,  OPT_HCDM

,  OPT_CLEAN
,  OPT_FILE
,  OPT_NAME
,  OPT_REDO
,  OPT_VERBOSE
}; // enum OPT_INDEX

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static const int       MODE_RW= (S_IRUSR | S_IWUSR);
static const int       PROT_RO= PROT_READ;
static const int       PROT_RW= (PROT_READ | PROT_WRITE);
static const uint64_t  SEED= uint64_t(0x0123456789ABCDEF); // Good as any
static const size_t    SIZE= size_t(0x01000000); // (16M)

//----------------------------------------------------------------------------
//
// Subroutine-
//       init
//
// Purpose-
//       Initialize
//
//----------------------------------------------------------------------------
static int                          // Return code (0 OK)
   init(                            // Initialize
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{  return 0; } // Placeholder

//----------------------------------------------------------------------------
//
// Subroutine-
//       term
//
// Purpose-
//       terminate
//
//----------------------------------------------------------------------------
static void
   term( void )                     // Terminate
{  } // Placeholder

//----------------------------------------------------------------------------
//
// Subroutine-
//       random_gen
//
// Purpose-
//       Generate a fixed sequence of random numbers.
//
//----------------------------------------------------------------------------
static void
   random_gen(                      // Generate a fixed random number sequence
     uint64_t*         addr,        // The storage area
     size_t            size)        // The storage size, in words
{
   Random random;                   // (Simple) Random number generator
   random.setSeed(SEED);            // Constant seed

   for(size_t i= 0; i<size; i++) {  // Verify the storage area
     addr[i]= random.get();         // Set next random value
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       random_ver
//
// Purpose-
//       Verify a fixed sequence of random numbers.
//
//----------------------------------------------------------------------------
static int                          // Return code, 0 OK
   random_ver(                      // Verify a fixed random number sequence
     uint64_t*         addr,        // The storage area
     size_t            size)        // The storage size, in words
{
   Random random;                   // (Simple) Random number generator
   random.setSeed(SEED);            // Constant seed

   for(size_t i= 0; i<size; i++) {  // Verify the storage area
     uint64_t word= random.get();   // The next expected value
     if( addr[i] != word ) {        // If verification failure
       fprintf(stderr, "%4d random_ver(%p,%.8zx)[%.8zx] failed: "
                       "Expected(%.16lx) Got(%.16lx)\n", __LINE__
                     , addr, size, i, addr[i], word);
       return 1;
     }
   }

   return 0;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_memgen
//
// Purpose-
//       Generate shared memory
//
//----------------------------------------------------------------------------
static int                          // Error counter
   test_memgen(                     // Generate shared memory
     int               fd)          // With this file descriptor
{
   int error_count= 0;

   printf("test_memgen()\n");

   int
   rc= ftruncate(fd, SIZE);         // (Expand to SIZE)
   if( rc ) {
     perror("ftruncate failed");
     return 1;
   }

   uint64_t* addr= (uint64_t*)mmap(nullptr, SIZE, PROT_RW, MAP_SHARED, fd, 0);
   printf("%4d %p= mmap(%p,%.8zx,%x,%x,%d,%d)\n", __LINE__
          , addr, nullptr, SIZE, PROT_RW, MAP_SHARED, fd, 0);
   if( addr == MAP_FAILED ) {       // If no can do
     perror("failed");
     return 1;
   }

   random_gen(addr, SIZE / sizeof(uint64_t));

   rc= munmap(addr, SIZE);
   if( rc ) {
     error_count++;
     fprintf(stderr, "%4d munmap(%p.%.8zx) ", __LINE__, addr, SIZE);
     perror("failed");
   }

   return error_count;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_memuse
//
// Purpose-
//       Verify shared memory
//
//----------------------------------------------------------------------------
static int                          // Error counter
   test_memuse(                     // Verify shared memory
     int               fd)          // With this file descriptor
{
   int error_count= 0;

   printf("test_memuse()\n");

   uint64_t* addr= (uint64_t*)mmap(nullptr, SIZE, PROT_RO, MAP_SHARED, fd, 0);
   printf("%4d %p= mmap(%p,%.8zx,%x,%x,%d,%d)\n", __LINE__
          , addr, nullptr, SIZE, PROT_RW, MAP_SHARED, fd, 0);
   if( addr == MAP_FAILED ) {       // If no can do
     perror("failed");
     return 1;
   }

   error_count += random_ver(addr, SIZE / sizeof(uint64_t));

   int
   rc= munmap(addr, SIZE);
   if( rc ) {
     error_count++;
     fprintf(stderr, "%4d munmap(%p.%.8zx) ", __LINE__, addr, SIZE);
     perror("failed");
   }

   return error_count;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_clean
//
// Purpose-
//       Remove files.
//
//----------------------------------------------------------------------------
static int                          // Error counter
   test_clean( void )               // Remove files
{
   int error_count= 0;

   if( opt_file ) {
     printf("\nfile_clean(%s)\n", opt_file);

     int rc= unlink(opt_file);
     if( rc ) {
       perror("unlink");
       error_count++;
     }
   }

   if( opt_name ) {
     printf("\nname_clean(%s)\n", opt_name);

     int rc= shm_unlink(opt_name);
     if( rc ) {
       perror("shm_unlink");
       error_count++;
     }
   }

   return error_count;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_file
//
// Purpose-
//       Test persistent file
//
//----------------------------------------------------------------------------
static int                          // Error counter
   test_file( void )                // Test persistent file
{
   int error_count= 0;

   printf("\ntest_file(%s)\n", opt_file);

   int fd= open(opt_file, O_RDWR | O_CREAT, MODE_RW);
   if( fd < 0 ) {
     perror("open");
     return 1;
   }

   error_count += test_memgen(fd);
   error_count += test_memuse(fd);

   if( close(fd) ) {
     error_count++;
     perror("close");
   }

   return error_count;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_name
//
// Purpose-
//       Test named shared storage
//
//----------------------------------------------------------------------------
static int                          // Error counter
   test_name( void )                // Test named shared storage
{
   int error_count= 0;

   printf("\ntest_name(%s)\n", opt_name);

   int fd= shm_open(opt_name, O_RDWR | O_CREAT, MODE_RW);
   if( fd < 0 ) {
     perror("shm_open");
     return 1;
   }

   error_count += test_memgen(fd);
   error_count += test_memuse(fd);

   if( close(fd) ) {
     error_count++;
     perror("close");
   }

   return error_count;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_redo
//
// Purpose-
//       Test named shared storage
//
//----------------------------------------------------------------------------
static int                          // Error counter
   test_redo( void )                // Test named shared storage
{
   int error_count= 0;

   if( opt_file ) {
     printf("\nfile_redo(%s)\n", opt_file);

     int fd= open(opt_file, O_RDONLY, MODE_RW);
     if( fd < 0 ) {
       perror("open");
       error_count++;
     } else {
       error_count += test_memuse(fd);

       if( close(fd) ) {
         error_count++;
         perror("close");
       }
     }
   }

   if( opt_name ) {
     printf("\nname_redo(%s)\n", opt_name);

     int fd= shm_open(opt_name, O_RDONLY, MODE_RW);
     if( fd < 0 ) {
       perror("shm_open");
       error_count++;
     } else {
       error_count += test_memuse(fd);

       if( close(fd) ) {
         error_count++;
         perror("close");
       }
     }
   }

   return error_count;
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
   char* strend;                    // Ending character
   long value= strtol(optarg, &strend, 0);
   if( value < INT_MIN || value > INT_MAX ) // Error checking
     errno= ERANGE;
   else if( strend == optarg || *strend != '\0' ) // If invalid string
     errno= EINVAL;

   if( errno ) {
     opt_help= true;
     if( errno == ERANGE )
       fprintf(stderr, "--%s, range error: '%s'\n", OPTS[opt_index].name, optarg);
     else if( strend == optarg )
       fprintf(stderr, "--%s, no value specified\n", OPTS[opt_index].name);
     else
       fprintf(stderr, "--%s, format error: '%s'\n", OPTS[opt_index].name, optarg);
   }

   return value;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       info
//
// Purpose-
//       Parameter description.
//
//----------------------------------------------------------------------------
static int                          // Return code (Always 1)
   info( void)                      // Parameter description
{
   fprintf(stderr, "%s <options> parameter ...\n"
                   "Options:\n"
                   "  --help\tThis help message\n"
                   "  --hcdm\tHard Core Debug Mode\n"

                   "  --clean\tRemove the mapped files (only)\n"
                   "  --file\tPersistent file name\n"
                   "  --name\tShared storage area name\n"
                   "  --redo\tRedo the verification (only)\n"
                   "  --verbose\t{=n} Verbosity, default 0\n"
                   , __FILE__
          );

   return 1;
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
static int                          // Return code (0 if OK)
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
           case OPT_HELP:           // These options are set by getopt
           case OPT_HCDM:
           case OPT_CLEAN:
           case OPT_REDO:
             break;

           case OPT_FILE:
             opt_file= optarg;      // Use this file name
             break;

           case OPT_NAME:
             opt_name= optarg;      // Use this shared storage name
             break;

           case OPT_VERBOSE:
             if( optarg )
               opt_verbose= parm_int();
             break;

           default:
             fprintf(stderr, "%4d Unexpected opt_index(%d)\n", __LINE__,
                             opt_index);
             break;
         }
         break;
       }}}}

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
                           (optopt & 0x00ff));
         break;

       default:
         fprintf(stderr, "%4d ShouldNotOccur ('%c',0x%x).\n", __LINE__,
                         C, (C & 0x00ff));
         break;
     }
   }

   // Verify parameters
   if( opt_file == nullptr && opt_name == nullptr ) {
     opt_help= true;
     fprintf(stderr, "Nothing to do. Specify --name and/or --file\n");
   }

   // Return sequence
   int rc= 0;
   if( opt_help )
     rc= info();
   return rc;
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
   // Initialize
   //-------------------------------------------------------------------------
   int rc= parm(argc, argv);        // Argument analysis
   if( rc ) return rc;              // Return if invalid

   rc= init(argc, argv);            // Initialize
   if( rc ) return rc;              // Return if invalid

   printf("%s: %s %s\n", __FILE__, __DATE__, __TIME__); // Compile time message
   if( opt_verbose >= 0 ) {
     printf("--hcdm(%d) --clean(%d) --redo(%d) --verbose(%d)\n"
            "--file(%s) --name(%s)\n"
            , opt_hcdm, opt_clean, opt_redo, opt_verbose, opt_file, opt_name);
   }

   //-------------------------------------------------------------------------
   // Mainline code: Run demos
   //-------------------------------------------------------------------------
   int error_count= 0;

   if( opt_redo )
     error_count += test_redo();
   if( opt_clean )
     error_count += test_clean();

   if( !opt_redo && !opt_clean ) {
     if( opt_file )
       error_count += test_file();
     if( opt_name )
       error_count += test_name();
   }

   //-------------------------------------------------------------------------
   // Terminate
   //-------------------------------------------------------------------------
   term();                          // Termination cleanup

   printf("\n");
   printf("%d error%s found\n", error_count, error_count == 1 ? "" : "s");
   if( error_count )
     rc= 1;

   return rc;
}
