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
//       2022/11/17
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

// Signal handlers
typedef void (*sig_handler_t)(int);
static sig_handler_t   sys1_handler= nullptr; // System SIGINT  signal handler
static sig_handler_t   sys2_handler= nullptr; // System SIGSEGV signal handler
static sig_handler_t   usr1_handler= nullptr; // System SIGUSR1 signal handler
static sig_handler_t   usr2_handler= nullptr; // System SIGUSR2 signal handler

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
static void
   info( void)                      // Parameter description
{
   fprintf(stderr, "%s <options> parameter ...\n", __FILE__ );
   fprintf(stderr, "Options:\n"
                   "  --help\tThis help message\n"
                   "  --hcdm\tHard Core Debug Mode\n"
                   "  --iodm\tI/O Debug Mode\n"

                   "  --debug\targument\n"
                   "  --verbose\t{=n} Verbosity, default 0\n"
                   "  --bringup\tRun bringup test\n"
                   "  --client\tRun client basic test\n"
                   "  --stress\t{=n} Run client stress test\n"
                   "  --host\tSet host name\n"
                   "  --port\tSet port number\n"
                   "  --runtime\tSet test run time (seconds)\n"
                   "  --ssl\tUse SSL sockets\n"
                   "  --trace\tActivate internal trace\n"
                   "  --server\tRun server\n"
                   "  --verify\tVerify file data\n"
                   "  --worker\tUse server threads\n"
          );

   exit(1);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       sig_handler
//
// Purpose-
//       Handle signals.
//
//----------------------------------------------------------------------------
static void
   sig_handler(                     // Handle signals
     int               id)          // The signal identifier
{
   static int recursion= 0;         // Signal recursion depth
   if( recursion ) {                // If signal recursion
     fprintf(stderr, "sig_handler(%d) recursion\n", id);
     fflush(stderr);
     exit(EXIT_FAILURE);
   }

   // Handle signal
   recursion++;                     // Disallow recursion
   const char* text= "SIG????";
   if( id == SIGINT ) text= "SIGINT";
   else if( id == SIGSEGV ) text= "SIGSEGV";
   else if( id == SIGUSR1 ) text= "SIGUSR1";
   else if( id == SIGUSR2 ) text= "SIGUSR2";
   errorf("\n\nsig_handler(%d) %s\n\n", id, text);

   if( Trace::table ) {
     Trace::trace(".SIG", __LINE__, text);
     Trace::table->flag[Trace::X_HALT]= true;
   }

   switch(id) {                     // Handle the signal
     case SIGINT:                   // (Console CTRL-C)
       // TODO: NOT CODED YET
       break;

     case SIGSEGV:                  // (Program fault)
       debug_set_mode(Debug::MODE_INTENSIVE);
       debug_backtrace();           // Attempt diagnosis (recursion aborts)
       debugf("..terminated..\n");
       exit(EXIT_FAILURE);
       break;

     default:                       // (SIGUSR1 || SIGUSR2)
       break;                       // (No configured action)
   }

   recursion--;
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
static int                          // Return code, 0 expected
   init( void)                      // Initialize
{
   if( HCDM )
     opt_hcdm= true;

   if( opt_hcdm && opt_verbose < 1 )
     opt_verbose= 1;

   if( USE_SIGNAL ) {
     sys1_handler= signal(SIGINT,  sig_handler);
     sys2_handler= signal(SIGSEGV, sig_handler);
     usr1_handler= signal(SIGUSR1, sig_handler);
     usr2_handler= signal(SIGUSR2, sig_handler);
   }

   Debug* extant= Debug::show();
   debug= new Debug("debug.out");   // Create Debug object
   if( extant )
     debug->set_file_mode("ab");
   Debug::set(debug);
   if( opt_hcdm || USE_INTENSIVE ) { // If Hard Core INTENSIVE Debug Mode
     debug_set_mode(Debug::MODE_INTENSIVE);
     debugh("HCDM: MODE_INTENSIVE\n");
   }
   debug_set_head(Debug::HEAD_THREAD);

   if( opt_trace ) {                // If --trace specified
     //-----------------------------------------------------------------------
     // Create memory-mapped trace file
     string S= "./trace.mem";
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

     Trace::table= PUB::Trace::make(trace_table, TRACE_SIZE);
     close(fd);                     // Descriptor not needed once mapped

     Trace::trace(".INI", 0, "TRACE STARTED") ;
   }

   client_agent= new ClientAgent();
   listen_agent= new ListenAgent();

   setlocale(LC_NUMERIC, "");       // For printf("%'d\n", 123456789);

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
   // Remove client/server agent
   delete client_agent;
   delete listen_agent;
   client_agent= nullptr;
   listen_agent= nullptr;

   //-------------------------------------------------------------------------
   // Restore system signal handlers
   if( sys1_handler ) signal(SIGINT,  sys1_handler);
   if( sys2_handler ) signal(SIGSEGV, sys2_handler);
   if( usr1_handler ) signal(SIGUSR1, usr1_handler);
   if( usr2_handler ) signal(SIGUSR2, usr2_handler);
   sys1_handler= sys2_handler= usr1_handler= usr2_handler= nullptr;

   // Release the trace table (which disables tracing)
   Trace::trace(".XIT", 0, "TRACE STOPPED") ;
   if( trace_table ) {
     Trace::table= nullptr;
     munmap(trace_table, TRACE_SIZE);
     trace_table= nullptr;
   }

   // Terminate debugging
#if 0  // TODO: REMOVE (when thread/worker tracing removed)
   opt_hcdm= false;
   Debug::set(nullptr);
   delete debug;
#endif
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

   size_of("Client",        sizeof(PUB::http::Client));
   size_of("ClientAgent",   sizeof(PUB::http::ClientAgent));
   size_of("Ioda",          sizeof(PUB::http::Ioda));
   size_of("Ioda::Mesg",    sizeof(PUB::http::Ioda::Mesg));
   size_of("Ioda::Page",    sizeof(PUB::http::Ioda::Page));
   size_of("Listen",        sizeof(PUB::http::Listen));
   size_of("ListenAgent",   sizeof(PUB::http::ListenAgent));
   size_of("Options",       sizeof(PUB::http::Options));
   size_of("Request",       sizeof(PUB::http::Request));
   size_of("Response",      sizeof(PUB::http::Response));
   size_of("Server",        sizeof(PUB::http::Server));
   size_of("Stream",        sizeof(PUB::http::Stream));

   if( false ) {                    // Bringup internal tests
     printf("\npage200(\"BODY\")\n%s", page200("BODY").c_str());
     printf("\npage403(\"/FILE\")\n%s", page403("/FILE").c_str());
     printf("\npage404(\"/FILE\")\n%s", page404("/FILE").c_str());
     printf("\npage405(\"METH\")\n%s", page405("METH").c_str());
     printf("\npage500(\"OOPS\")\n%s", page500("OOPS").c_str());
   }

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
static void                         // Exit if error detected
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
           case OPT_IODM:
           case OPT_BRINGUP:
           case OPT_CLIENT:
           case OPT_SERVER:
           case OPT_TRACE:
           case OPT_VERIFY:
           case OPT_WORKER:

           case OPT_NO_WORKER:
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

           case OPT_STREAM:
           case OPT_STRESS:
             if( optarg ) {
               opt_stress= parm_int();
               if( opt_stress < 0 )
                 opt_stress= 0;
             }
             break;

           case OPT_MAJOR:
             if( optarg )
               opt_major= parm_int();
             break;

           case OPT_MINOR:
             if( optarg )
               opt_minor= parm_int();
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
           fprintf(stderr, "%4d Option has no argument '%s'.\n"
                         , __LINE__, argv[optind-1]);
         else
           fprintf(stderr, "%4d Option requires an argument '%s'.\n"
                         , __LINE__, argv[optind-1]);
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
   if( opt_help )
     info();
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
   //-------------------------------------------------------------------------
   // Initialize
   //-------------------------------------------------------------------------
   host= Socket::gethostname();     // Use the default host name
   parm(argc, argv);                // Argument analysis (Exit if error)
   init();                          // Initialize

   if( opt_verbose ) {
     debugf("%s %s %s (Compiled)\n", __FILE__, __DATE__, __TIME__);

     time_t tod;
     time(&tod);
     struct tm* info= localtime(&tod);
     char buff[32];
     strftime(buff, sizeof(buff), "%b %e %Y %R:%S", info);
     debugf("%s %s (Started)\n", __FILE__, buff);

     debugf("\n");
     debugf("Settings:\n");
     debugf("%5.1f: runtime\n",  opt_runtime);
     debugf("%5s: server: %s\n", torf(bool(opt_server)), host.c_str());
     debugf("%5s: hcdm\n",   torf(opt_hcdm));
     debugf("%5s: iodm\n",   torf(opt_iodm));
     debugf("%5d: verbose\n",opt_verbose);

     debugf("%5s: client\n", torf(opt_client));
     debugf("%5s: ssl\n",    torf(opt_ssl));
     if( opt_stress )
       debugf("%5s: stress: %d\n", torf(opt_stress), opt_stress);
     else
       debugf("%5s: stress\n", torf(opt_stress));
     debugf("%5s: worker\n", torf(opt_worker));

     // Debugging, experimentation
     debugf("\n");
     debugf("%5d: MAX_REQUEST_COUNT\n", MAX_REQUEST_COUNT);
     debugf("%5s: Protocol (unencrypted)\n", "HTTP1");
     debugf("%5d: --major\n", opt_major);
     debugf("%5d: --minor\n", opt_minor);
     debugf("\n\n");
   }

   //-------------------------------------------------------------------------
   // Run the tests (with try wrapper)
   //-------------------------------------------------------------------------
   try {
     if( opt_bringup )
       error_count += test_bringup();

     ServerThread* server= nullptr;
     if( opt_server ) {
       server= new ServerThread();
       server->ready.wait();
     }

     if( opt_client || opt_stress ) {
       if( opt_client ) {
         error_count= 0;
         ClientThread::test_client();
         ClientThread::statistics();
       }
       if( opt_stress ) {
         error_count= 0;
         ClientThread::test_stress();
         ClientThread::statistics();
       }
     }

     if( server ) {
       server->stop();
       server->ended.wait();
       delete server;
     }
   } catch(PUB::Exception& X) { // - - - - - - - - - - - - - - - - - - - - - -
     ++error_count;
     debugf("%4d T_Stream: %s\n", __LINE__, ((string)X).c_str());
   } catch(std::exception& X) {
     ++error_count;
     debugf("%4d T_Stream: std::exception(%s)\n", __LINE__, X.what());
   } catch(const char* X) {
     ++error_count;
     debugf("%4d T_Stream: catch(\"%s\")\n", __LINE__, X);
   } catch(...) {
     ++error_count;
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
     debugf("%zd errors detected\n", error_count.load());
     error_count= 1;
   }

   term();                          // Terminate
   return error_count.load() != 0;
}
