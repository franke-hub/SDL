//----------------------------------------------------------------------------
//
//       Copyright (C) 2022 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       T_Stream.cpp
//
// Purpose-
//       Test the Stream objects.
//
// Last change date-
//       2022/08/16
//
// Arguments-
//       With no arguments, --client --server defaulted
//       --bringup  Bringup test (Display object sizes)
//       --client   Basic test
//       --stress   Stress test
//
//       --server   Run server
//
//       --host n   Override connection host
//       --port n   Override default port number
//
//----------------------------------------------------------------------------
#include "T_Stream.hpp"             // Prerequisite includes, etc

//----------------------------------------------------------------------------
//
// Subroutine-
//       to_integer
//
// Purpose-
//       Convert string to integer, handling error cases
//
// Implementation note-
//       Leading or trailing blanks are NOT allowed.
//
//----------------------------------------------------------------------------
static int                          // The integer value
   to_integer(                      // Extract and verify integer value
     const char*       inp)         // From this string
{
   errno= 0;
   char* strend;                    // Ending character
   long value= strtol(inp, &strend, 0);
   if( strend == inp || *inp == ' ' || *strend != '\0' )
     errno= EINVAL;
   else if( value < INT_MIN || value > INT_MAX )
     errno= ERANGE;

   return value;
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
   int value= to_integer(optarg);
   if( errno ) {
     opt_help= true;
     if( errno == ERANGE )
       fprintf(stderr, "--%s, range error: '%s'\n"
              , OPTS[opt_index].name, optarg);
     else if( *optarg == '\0' )
       fprintf(stderr, "--%s, no value specified\n", OPTS[opt_index].name);
     else
       fprintf(stderr, "--%s, format error: '%s'\n"
              , OPTS[opt_index].name, optarg);
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
   fprintf(stderr, "%s <options> parameter ...\n", __FILE__ );
   fprintf(stderr, "Options:\n"
                   "  --help\tThis help message\n"
                   "  --hcdm\tHard Core Debug Mode\n"

                   "  --debug\targument\n"
                   "  --verbose\t{=n} Verbosity, default 0\n"
                   "  --bringup\tRun bringup test\n"
                   "  --client\tRun client basic test\n"
                   "  --stress\tRun client stress test\n"
                   "  --host\tSet host name\n"
                   "  --port\tSet port number\n"
                   "  --runtime\tSet test run time (seconds)\n"
                   "  --trace\tActivate internal trace\n"
                   "  --server\tRun server\n"
                   "  --verify\tVerify file data\n"
          );

   return 1;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       init
//
// Purpose-
//       Initialize
//
//----------------------------------------------------------------------------
static void make_dir(std::string path) // Insure directory exists
{
   struct stat info;
   int rc= stat(path.c_str(), &info);
   if( rc != 0 ) {
     rc= mkdir(path.c_str(), DIR_MODE);
     if( rc ) {
       fprintf(stderr, "Cannot create %s", path.c_str());
       exit(1);
     }
   }
}

static int                          // Return code, 0 expected
   init( void)                      // Initialize
{
   if( opt_trace ) {                // If --trace specified
     //-----------------------------------------------------------------------
     // If required, create memory-mapped trace subdirectory
     const char* env= getenv("HOME"); // Get HOME directory
     if( env == nullptr ) {
       fprintf(stderr, "No HOME directory\n");
       return 1;
     }
     std::string S= env;
     S += "/.config";
     make_dir(S);
     S += "/.trace";
     make_dir(S);

     //-----------------------------------------------------------------------
     // Create memory-mapped trace file
     S += "/trace.mem";
     int fd= open(S.c_str(), O_RDWR | O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
     if( fd < 0 ) {
       fprintf(stderr, "%4d open(%s) %s\n", __LINE__
                     , S.c_str(), strerror(errno));
       return 1;
     }

     int rc= ftruncate(fd, TRACE_SIZE); // (Expand to TRACE_SIZE)
     if( rc ) {
       fprintf(stderr, "%4d ftruncate(%s,%.8x) %s\n", __LINE__
                     , S.c_str(), TRACE_SIZE, strerror(errno));
       return 1;
     }

     trace_table= mmap(nullptr, TRACE_SIZE, PROT_RW, MAP_SHARED, fd, 0);
     if( trace_table == MAP_FAILED ) { // If no can do
       fprintf(stderr, "%4d mmap(%s,%.8x) %s\n", __LINE__
                     , S.c_str(), TRACE_SIZE, strerror(errno));
       trace_table= nullptr;
       return 1;
     }

     Trace::table= pub::Trace::make(trace_table, TRACE_SIZE);
     close(fd);                     // Descriptor not needed once mapped

     Trace::trace(".INI", 0, "TRACE STARTED") ;
   }

   return 0;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       term
//
// Purpose-
//       Terminate
//
//----------------------------------------------------------------------------
static void
   term( void )                     // Terminate
{
   Trace::trace(".INI", 0, "TRACE STOPPED") ;

   // Free the trace table (and disable tracing)
   if( trace_table ) {
     Trace::table= nullptr;
     munmap(trace_table, TRACE_SIZE);
     trace_table= nullptr;
   }

   // Terminate debugging
   opt_hcdm= false;
   Debug::set(nullptr);
   delete debug;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_bringup
//
// Purpose-
//       Bringup test
//
//----------------------------------------------------------------------------
static void
   size_of(                         // Display size of object
     const char*       name,        // Object name
     size_t            size)        // Object size
{ printf("%4zd = sizeof(%s)\n", size, name); }

static inline int                   // Error count
   test_bringup( void )             // Bringup test
{
   debugf("\ntest_bringup\n");
   int error_count= 0;

   size_of("Client",        sizeof(pub::http::Client));
   size_of("ClientAgent",   sizeof(pub::http::ClientAgent));
   size_of("Listen",        sizeof(pub::http::Listen));
   size_of("Options",       sizeof(pub::http::Options));
   size_of("Request",       sizeof(pub::http::Request));
   size_of("Response",      sizeof(pub::http::Response));
   size_of("Server",        sizeof(pub::http::Server));
   size_of("ServerAgent",   sizeof(pub::http::ServerAgent));
   size_of("Stream",        sizeof(pub::http::Stream));

   return error_count;
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
   while( (C= getopt_long(argc, argv, OSTR, OPTS, &opt_index)) != -1 ) {
     switch( C ) {
       case 0:
       {{{{
         switch( opt_index ) {
           case OPT_HELP:           // These options handled by getopt
           case OPT_HCDM:
           case OPT_BRINGUP:
           case OPT_CLIENT:
           case OPT_SERVER:
           case OPT_STRESS:
           case OPT_TRACE:
           case OPT_VERIFY:
             break;

           case OPT_DEBUG:
             opt_debug= optarg;
             break;

           case OPT_HOST:
             host= optarg;
             break;

           case OPT_PORT: {
             int opt_port= parm_int();
             if( opt_port > 0 && opt_port <= UINT16_MAX )
               port= std::to_string(opt_port);
             else
               fprintf(stderr, "Invalid port(%s)\n", optarg);
             }
             break;

           case OPT_RUNTIME:
             opt_runtime= atof(optarg);
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

   // Parameter verification
   if( !opt_bringup && !opt_client && !opt_server && !opt_stress ) {
     opt_client= true;
     opt_server= true;
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
extern int
   main(                            // Mainline code
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   int                 error_count= 0; // Error counter

   //-------------------------------------------------------------------------
   // Initialize
   //-------------------------------------------------------------------------
   host= Socket::gethostname();     // Use the default host name
   int rc= parm(argc, argv);        // Argument analysis
   if( rc ) return rc;              // Return if invalid
   init();                          // Initialize

   debug= new Debug("debug.out");   // Create Debug object
   Debug::set(debug);
   if( HCDM || true ) {             // If Hard Core INTENSIVE Debug Mode
     debug_set_mode(Debug::MODE_INTENSIVE);
     debugh("%4d %s HCDM: MODE_INTENSIVE\n", __LINE__, __FILE__);
   }
   debug_set_head(Debug::HEAD_THREAD);

   setlocale(LC_NUMERIC, "");       // For printf("%'d\n", 123456789);

   //-------------------------------------------------------------------------
   // TRY wrapper
   //-------------------------------------------------------------------------
   try {
     if( opt_bringup )
       error_count += test_bringup();

     ServerThread* server= nullptr;
     if( opt_server ) {
       server= new ServerThread();  // (Create, auto-start ServerThread)
       Thread::sleep(0.125);        // (ServerThread startup delay)
     }

     if( opt_client || opt_stress ) {
       ClientThread client;         // (The T_Stream pseudo ClientThread)

       if( opt_client ) {
         error_count += client.test_client();
         error_count += client.statistics();
       }
       if( opt_stress ) {
         error_count += client.test_stress();
         error_count += client.statistics();
       }

       client.join();

       if( server )
         server->stop();
     }

     if( server ) {
       server->join();
       delete server;
     }
   }

   //-------------------------------------------------------------------------
   // Handle exceptions
   //-------------------------------------------------------------------------
   catch(pub::Exception& X) {
     error_count++;
     debugf("%4d T_Stream: %s\n", __LINE__, ((string)X).c_str());
   } catch(std::exception& X) {
     error_count++;
     debugf("%4d T_Stream: std::exception(%s)\n", __LINE__, X.what());
   } catch(const char* X) {
     error_count++;
     debugf("%4d T_Stream: catch(\"%s\")\n", __LINE__, X);
   } catch(...) {
     error_count++;
     debugf("%4d T_Stream: catch(...)\n", __LINE__);
   }

//----------------------------------------------------------------------------
// Testing complete
//----------------------------------------------------------------------------
   Thread::sleep(0.5);              // (Delay to allow cleanup)
   if( error_count == 0 )
     debugf("NO errors detected\n");
   else if( error_count == 1 )
     debugf("1 error detected\n");
   else {
     debugf("%d errors detected\n", error_count);
     error_count= 1;
   }

   term();                          // Terminate
   return error_count;
}
