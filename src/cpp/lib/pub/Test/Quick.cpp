//----------------------------------------------------------------------------
//
//       Copyright (C) 2018-2019 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Quick.cpp
//
// Purpose-
//       Quick verification tests.
//
// Last change date-
//       2019/01/01
//
//----------------------------------------------------------------------------
#include <chrono>
#include <ctype.h>                  // For isprint()
#include <getopt.h>                 // For getopt()
#include <iostream>
#include <new>
#include <random>
#include <string>
#include <thread>

#include <assert.h>
#include <endian.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "MUST.H"                   // Error counting assert
#include "pub/Debug.h"
#include "pub/config.h"             // For _PUB_NAMESPACE
#include "pub/Object.h"
#include "pub/Exception.h"
#include "pub/Latch.h"              // See test_Latch
#include "pub/memory.h"             // See test_atomic_shared_ptr, NOT CODED YET
#include "pub/Signals+Slots.h"      // See test_Signals
#include "pub/UTF8.h"               // See test_UTF8
#include "pub/utility.h"            // For _PUB_NAMESPACE::utility
using namespace _PUB_NAMESPACE;
using namespace _PUB_NAMESPACE::debugging;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef TRACE                       // If defined, use internal trace
#define TRACE
#endif

#define IFDEBUG(x) { if( opt_debug ) { x }}

#include "pub/ifmacro.h"            // Dependent macro

//----------------------------------------------------------------------------
// External data areas
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Options
//----------------------------------------------------------------------------
static int             opt_case= false; // (Only set if --all)
static int             opt_debug= 0; // --debug
static int             opt_dump= false; // --dump
static int             opt_help= false; // --help or error
static int             opt_index;   // Option index
static int             opt_latch= false; // --latch
static int             opt_must= false; // --must
static int             opt_signals= false; // --signals
static int             opt_trace= false; // --trace
static int             opt_verbose= false; // --verbose
static int             opt_utf8= false; // --utf8
static const char*     utf8_encode= nullptr; // --utf8.encode
static const char*     utf8_decode= nullptr; // --utf8.decode

static struct option   OPTS[]=      // Options
{  {"help",    no_argument,       &opt_help,    true}
,  {"dump",    no_argument,       &opt_dump,    true}
,  {"latch",   no_argument,       &opt_latch,   true}
,  {"must",    no_argument,       &opt_must,    true}
,  {"signals", no_argument,       &opt_signals, true}
,  {"trace",   no_argument,       &opt_trace,   true}
,  {"utf8",    no_argument,       &opt_utf8,    true}
,  {"verbose", no_argument,       &opt_verbose, true}

,  {"all",     optional_argument, nullptr,      0}
,  {"debug",   optional_argument, nullptr,      0}
,  {"utf8.encode", required_argument, nullptr,  0}
,  {"utf8.decode", required_argument, nullptr,  0}
,  {0, 0, 0, 0}                     // (End of option list)
};

enum OPT_INDEX
{  OPT_HELP
,  OPT_DUMP
,  OPT_LATCH
,  OPT_MUST
,  OPT_SIGNALS
,  OPT_TRACE
,  OPT_UTF8
,  OPT_VERBOSE
,  OPT_ALL
,  OPT_DEBUG
,  OPT_UTF8_ENCODE
,  OPT_UTF8_DECODE
};

//----------------------------------------------------------------------------
//
// Subroutine-
//       hcdmf
//
// Purpose-
//       Hard Core Debug Mode printf
//
//----------------------------------------------------------------------------
static void
   hcdmf(int line)
{  IFHCDM( debugf("%4d Quick (HCDM)\n", line); ) }

//----------------------------------------------------------------------------
//
// Subroutine-
//       hexchar
//
// Purpose-
//       Get value of HEX character
//
//----------------------------------------------------------------------------
static unsigned                     // HEX value (EXCEPTION if error)
   hexchar(                         // HEX character value
     int               C)           // For this HEX character
{
   if( C >= '0' && C <= '9' )
     return C - '0';

   if( C >= 'a' && C <= 'f' )
     return 10 + C - 'a';

   if( C >= 'A' && C <= 'F' )
     return 10 + C - 'A';

   throwf("Invalid HEX character(%c)", C);
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
   fprintf(stderr, "Quick [options]\n"
                   "Options:\n"
                   "  --help\tThis help message\n"
                   "  --all\t\tRun all regression tests\n"
                   "  --dump\tDump.h debug.out test\n"
                   "  --debug\t{=value}\n"
                   "  --latch\tLatch.h regression test\n"
                   "  --signals\tsignals::Signal.h regression test\n"
                   "  --trace\tTrace.h debug.out test\n"
                   "  --utf8\tUTF8.h regression test\n"
                   "  --utf8.encode\t=test test UTF8::encode()\n"
                   "  --utf8.decode\t=test test UTF8::decode()\n"
          );

   exit(EXIT_FAILURE);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       parm
//
// Purpose-
//       Parameter analysis example.
//
//----------------------------------------------------------------------------
static void
   parm(                            // Parameter analysis
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   int                 C;

   //-------------------------------------------------------------------------
   // Argument analysis
   //-------------------------------------------------------------------------
   opterr= 0;                       // Do not write error messages

   while( (C= getopt_long(argc, (char**)argv, ":", OPTS, &opt_index)) != -1 )
     switch( C )
     {
       case 0:
         switch( opt_index )
         {
           case OPT_ALL:
             opt_latch= true;
             opt_case= true;
             // opt_dump= true;     // Select separately (visual debugging)
             opt_signals= true;
             // opt_trace= true;    // Select separately (visual debugging)
             opt_utf8= true;
             break;

           case OPT_DEBUG:
             if( optarg )
               opt_debug= atoi(optarg);
             else
               opt_debug= -1;
             break;

           case OPT_UTF8_ENCODE:
             utf8_encode= optarg;
             break;

           case OPT_UTF8_DECODE:
             utf8_decode= optarg;
             break;

           default:
             break;
         }
         break;

       case ':':
         opt_help= true;
         if( optopt == 0 )
           fprintf(stderr, "Option requires an argument '%s'.\n",
                           argv[optind-1]);
         else
           fprintf(stderr, "Option requires an argument '-%c'.\n", optopt);
         break;

       case '?':
         opt_help= true;
         if( optopt == 0 )
           fprintf(stderr, "Unknown option '%s'.\n", argv[optind-1]);
         else if( isprint(optopt) )
           fprintf(stderr, "Unknown option '-%c'.\n", optopt);
         else
           fprintf(stderr, "Unknown option character '0x%x'.\n", optopt);
         break;

       default:
         fprintf(stderr, "%4d SNO ('%c',0x%x).\n", __LINE__, C, C);
         exit( EXIT_FAILURE );
     }

   if( opt_help )
     info();
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       scdmf
//
// Purpose-
//       Soft Core Debug Mode printf
//
//----------------------------------------------------------------------------
static void
   scdmf(int line)
{  IFDEBUG( debugf("%4d Quick (SCDM)\n", line); ) }

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_Case
//
// Purpose-
//       Test case example.
//
//----------------------------------------------------------------------------
static inline int
   test_Case( void )                  // Test case example
{
   debugf("\ntest_Case\n");

   int                 errorCount= 0; // Number of errors encountered

   return errorCount;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_dump
//
// Purpose-
//       Test utility::dump.h
//
//----------------------------------------------------------------------------
static inline int
   test_dump( void )                  // Test utility::dump.h
{
   debugf("\ntest_dump\n");

   int                 errorCount= 0; // Number of errors encountered

   using pub::utility::dump;
   alignas(16) char buffer[256];
   for(int i= 0; i<sizeof(buffer); i++)
     buffer[i]= "0123456789ABCDEF"[i%16];

   FILE* file= Debug::get()->get_FILE();
                        dump(file, buffer+3, 29);
   fprintf(file, "\n"); dump(file, buffer+3, 3);
   fprintf(file, "\n"); dump(file, buffer+14, 14);
   fprintf(file, "\n"); dump(file, buffer+1, 126);
   fprintf(file, "\n"); dump(file, buffer, 128);

   fprintf(file, "\n"); dump(file, buffer+3, 3, (void*)3);
   fprintf(file, "\n"); dump(file, buffer+14, 14, (void*)14);
   fprintf(file, "\n"); dump(file, buffer+1, 126, (void*)1);
   fprintf(file, "\n"); dump(file, buffer, 128, (void*)0);

   return errorCount;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_Latch
//
// Purpose-
//       (Minimally) test Latch.h
//
//----------------------------------------------------------------------------
static inline int
   test_Latch( void )                 // Test Latch.h
{
   debugf("\ntest_Latch\n");

   int                 errorCount= 0; // Number of errors encountered

   //-------------------------------------------------------------------------
   IFDEBUG( debugf("..Testing: Share/ExclusiveLatch\n"); )
   SharedLatch    shared;
   ExclusiveLatch exclusive(shared);

   {{{{ std::lock_guard<decltype(shared)> lock1(shared);
     errorCount += MUST_EQ(shared.count, 1);
     if( exclusive.try_lock() )
       errorCount += MUST_NOT(Obtain exclusive while shared);

     {{{{ std::lock_guard<decltype(shared)> lock2(shared);
       errorCount += MUST_EQ(shared.count, 2);
     }}}}
     errorCount += MUST_EQ(shared.count, 1);
   }}}}
   errorCount += MUST_EQ(shared.count, 0);

   if( exclusive.try_lock() )
   {
     errorCount += MUST_EQ(shared.count, 0x80000000);
     exclusive.unlock();;
     errorCount += MUST_EQ(shared.count, 0);
   } else {
     errorCount += MUST_NOT(Fail to obtain exclusive latch);
   }

   {{{{ std::lock_guard<decltype(exclusive)> lock(exclusive);
     errorCount += MUST_EQ(shared.count, 0x80000000);
   }}}}
   errorCount += MUST_EQ(shared.count, 0);

   //-------------------------------------------------------------------------
   IFDEBUG( debugf("..Testing: RecursiveLatch\n"); )
   std::thread::id not_thread;
   RecursiveLatch recursive;
   errorCount += MUST_EQ(recursive.latch, not_thread);
   errorCount += MUST_EQ(recursive.count, 0);

   {{{{ std::lock_guard<decltype(recursive)> lock1(recursive);
     errorCount += MUST_EQ(recursive.count, 1);

     {{{{ std::lock_guard<decltype(recursive)> lock2(recursive);
       errorCount += MUST_EQ(recursive.count, 2);
     }}}}

     errorCount += MUST_EQ(recursive.count, 1);
   }}}}
   errorCount += MUST_EQ(recursive.latch, not_thread);
   errorCount += MUST_EQ(recursive.count, 0);

   //-------------------------------------------------------------------------
   IFDEBUG( debugf("..Testing: NonRecursiveLatch\n"); )
   NonRecursiveLatch nonrecursive;
   errorCount += MUST_EQ(nonrecursive.latch, not_thread);

   {{{{ std::lock_guard<decltype(nonrecursive)> lock1(nonrecursive);
     try {
       {{{{ std::lock_guard<decltype(nonrecursive)> lock2(nonrecursive);
         errorCount += MUST_NOT(Recursively hold NonRecursiveLatch);
       }}}}
     } catch(Exception X) {
       IFDEBUG( debugf("....Expected: %s\n", ((std::string)X).c_str()); )
       errorCount += MUST_EQ(nonrecursive.latch, not_thread);
     }
   }}}}

   //-------------------------------------------------------------------------
   IFDEBUG( debugf("..Testing: NullLatch\n"); )
   NullLatch fake_latch;

   {{{{ std::lock_guard<decltype(fake_latch)> lock1(fake_latch);
        std::lock_guard<decltype(fake_latch)> lock2(fake_latch);
        std::lock_guard<decltype(fake_latch)> lock3(fake_latch);
        std::lock_guard<decltype(fake_latch)> lock4(fake_latch);
   }}}}

   return errorCount;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_Must
//
// Purpose-
//       Test case example, tests MUST functions.
//
//----------------------------------------------------------------------------
static inline int
   test_Must( void )                  // Test Case.h
{
   debugf("\ntest_Must\n");

   int                 errorCount= 0; // Number of errors encountered

   // This tests the MUST.H macros
   int one= 1;
   int two= 1;
   std::thread::id is_thread= std::this_thread::get_id();
   std::thread::id no_thread;

   errorCount += VERIFY(1 == 1);
   errorCount += VERIFY(1 == 2);
   errorCount += MUST_EQ(one, 1);
   errorCount += MUST_EQ(two, 2);
   errorCount += MUST_NOT(Sample error description);
   errorCount += MUST_EQ(is_thread, is_thread);
   errorCount += MUST_EQ((volatile std::thread::id&)no_thread, no_thread);
   errorCount += MUST_EQ((volatile std::thread::id&)is_thread, no_thread);
   errorCount += MUST_EQ(no_thread, is_thread);

   return errorCount;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_Signals
//
// Purpose-
//       Test signals/Signal.h
//
// Implementation note-
//       signals/Signal.h debugging functions normally commented out.
//       They must be enabled if needed.
//
//----------------------------------------------------------------------------
static inline int
   test_Signals( void )             // Test signals::Signal.h
{
   debugf("\ntest_Signals\n");

   int                 errorCount= 0; // Number of errors encountered

   static int          A_counter= 0; // Number of A clicks
   static int          B_counter= 0; // Number of B clicks

   using click_conn=   pub::signals::Connection<float,float>;
   using click_signal= pub::signals::Signal<float,float>;
// using click_slot=   pub::signals::Slot<float,float>; // Not needed or used

   struct gui_element{
     click_signal clicked;
     void mouse_down(float X,float Y) { clicked.emit(X, Y); }
   };

   struct Slot_A {
     void operator()(float X, float Y)
     { IFDEBUG( debugf("SA : A was clicked at %.0f,%.0f\n", X, Y); )
       A_counter++;
     };
   };

   struct Slot_B {
     void operator()(float X, float Y)
     { IFDEBUG( debugf("SB : B was clicked at %.0f,%.0f\n", X, Y); )
       B_counter++;
     };
   };

   hcdmf(__LINE__); gui_element A; // The "A" gui_element
   hcdmf(__LINE__); gui_element B; // The "B" gui_element
   hcdmf(__LINE__); click_conn connection_1, connection_2;

   connection_1= A.clicked.connect([](float X,float Y) {
           IFDEBUG(  debugf("l1 : A was clicked at %.0f,%.0f\n", X, Y); )
           A_counter++;
           });
   connection_2= B.clicked.connect([](float X,float Y) {
           IFDEBUG(  debugf("l2 : B was clicked at %.0f,%.0f\n", X, Y); )
           B_counter++;
           });

   // A has one connection, B has one connection
   IFHCDM( hcdmf(__LINE__); A.clicked.debug("A");      )
   IFHCDM( hcdmf(__LINE__); B.clicked.debug("B");      )
   IFHCDM( hcdmf(__LINE__); connection_1.debug("c_1"); )
   IFHCDM( hcdmf(__LINE__); connection_2.debug("c_2"); )
   scdmf(__LINE__); A.mouse_down(__LINE__, -1);
   scdmf(__LINE__); B.mouse_down(-1, __LINE__);
   errorCount += MUST_EQ(A_counter, 1);
   errorCount += MUST_EQ(B_counter, 1);
   errorCount += VERIFY(B_counter == 1);

   {
     scdmf(__LINE__);
     auto temporary= A.clicked.connect([](float X,float Y){
                              IFDEBUG(  debugf("tmp: A was clicked at %.0f,%.0f\n", X, Y); )
                                        A_counter++;
                                        });
     IFHCDM( hcdmf(__LINE__); A.clicked.debug("A"); )
     // A has two connections, B has one connection
     scdmf(__LINE__); A.mouse_down(1, 0);
     scdmf(__LINE__); B.mouse_down(0, 1);
     errorCount += MUST_EQ(A_counter, 3);
     errorCount += MUST_EQ(B_counter, 2);
   }

   // temporary out of scope: A has one connection, B has one connection
   // This overwrites the B connection with the A connection,
   // leaving connection_1 empty and connection_2 with the A connection.
   scdmf(__LINE__);
   connection_2 = std::move(connection_1);

   // A has one connection, B has no connections
   IFHCDM( hcdmf(__LINE__); A.clicked.debug("A"); )
   IFHCDM( hcdmf(__LINE__); B.clicked.debug("B"); )
   scdmf(__LINE__); A.mouse_down(2, 0);
   scdmf(__LINE__); B.mouse_down(0, 2);
   errorCount += MUST_EQ(A_counter, 4);
   errorCount += MUST_EQ(B_counter, 2);

   // Add a Slot_B connection to Slot_A.
   // A.mouse_down drives both l1 and SB connections
   click_conn more= A.clicked.connect(Slot_B());
   IFHCDM( hcdmf(__LINE__); A.clicked.debug("B"); )
   scdmf(__LINE__); A.mouse_down(3, 0);
   errorCount += MUST_EQ(A_counter, 5);
   errorCount += MUST_EQ(B_counter, 3);

   // B.mouse_down does nothing.
   scdmf(__LINE__); B.mouse_down(0, 3);
   errorCount += MUST_EQ(A_counter, 5);
   errorCount += MUST_EQ(B_counter, 3);

   // This is a usage error.
   // The connection is created but not saved, so it's immediately deleted
   // Thus it has no effect.
   B.clicked.connect(Slot_B());
   IFHCDM( hcdmf(__LINE__); B.clicked.debug("T"); )
   scdmf(__LINE__); A.mouse_down(4, 0); // Increments A_counter and B_counter
   errorCount += MUST_EQ(A_counter, 6);
   errorCount += MUST_EQ(B_counter, 4);

   // B.mouse_down still does nothing.
   scdmf(__LINE__); B.mouse_down(0, 4); // Might expect B_counter == 5
   errorCount += MUST_EQ(A_counter, 6);
   errorCount += MUST_EQ(B_counter, 4); // But connection does not exist

   //-------------------------------------------------------------------------
   // Test Signal::reset()
   A.clicked.reset();
   B.clicked.reset();

   scdmf(__LINE__); A.mouse_down(-5, 0);
   scdmf(__LINE__); B.mouse_down(0, -5);
   errorCount += MUST_EQ(A_counter, 6); // (Unchanged)
   errorCount += MUST_EQ(B_counter, 4); // (Unchanged)

   //-------------------------------------------------------------------------
   // Test Connection::reset()
   connection_1= A.clicked.connect(Slot_A());
   connection_1.reset();

   scdmf(__LINE__); A.mouse_down(-6, 0);
   scdmf(__LINE__); B.mouse_down(0, -6);
   errorCount += MUST_EQ(A_counter, 6); // (Unchanged)
   errorCount += MUST_EQ(B_counter, 4); // (Unchanged)

   //-------------------------------------------------------------------------
   // Test multiple connections
   A_counter= B_counter= 0;
   click_conn l_array[32];
   for(int i= 0; i<32; i++)
   {
     if( i & 1 )
       l_array[i]= B.clicked.connect(Slot_B());
     else
       l_array[i]= A.clicked.connect(Slot_A());
   }
   IFHCDM( hcdmf(__LINE__); A.clicked.debug("A"); )
   IFHCDM( hcdmf(__LINE__); B.clicked.debug("B"); )
   scdmf(__LINE__); A.mouse_down(16, 0);
   scdmf(__LINE__); B.mouse_down(0, 16);
   errorCount += MUST_EQ(A_counter, 16);
   errorCount += MUST_EQ(B_counter, 16);

   return errorCount;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_Trace
//
// Purpose-
//       Test Trace.h
//
//----------------------------------------------------------------------------
static inline int
   test_Trace( void )               // Test Trace.h
{
   debugf("\ntest_Trace\n");

   int                 errorCount= 0; // Number of errors encountered

#ifdef TRACE
   using _PUB_NAMESPACE::Trace;

   uint32_t trace_size= 0x00020000;
   struct Record : public Trace::Record {
   void set_ident(const char* id) { ident= *(const uint32_t*)id; }

     int32_t           ident;
     int32_t           offset;
   };

   Trace* trace= Trace::make(trace_size);
   Trace::trace= trace;

   for(int i= 0; i<trace_size+12; i++) // 32 wraps + extra
   {
     IFTRACE(
       Record* record= (Record*)trace->allocate(sizeof(Record));
       record->offset= htobe32((char*)record - (char*)trace);
       record->set_ident(".FOO");
     )
   }

   Trace* taken= Trace::take();
   if( taken != trace )
   {
     errorCount++;
     debugf("Trace::take() failure[1]\n");
   }
   if( Trace::trace )
   {
     errorCount++;
     debugf("Trace::take() failure[2]\n");
   }
   trace->dump();

   // This test is designed to only show interesting records.
   tracef("\nTest wrap clear\n");
   memset((char*)trace + sizeof(Trace), 0, trace_size - sizeof(Trace));
   uint32_t size= trace_size - 512;
   trace->allocate(size);
   trace->dump();

   delete trace;
   trace= nullptr;

   IFTRACE(
     errorCount++;
     debugf("%4d Should not occur\n", __LINE__);

     Record* record= (Record*)trace->allocate(sizeof(Record));
     record->offset= htobe32((char*)record - (char*)trace);
     record->set_ident(".ERR");
   )
#else
   errorCount++;
   debugf("TRACE not defined. Test aborted\n");
#endif

   return errorCount;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_UTF8
//
// Purpose-
//       Test UTF8.h
//
//----------------------------------------------------------------------------
static inline int
   test_UTF8( void )                // Test UTF8.h
{
   debugf("\ntest_UTF8\n");

   int                 errorCount= 0; // Number of errors encountered

   unsigned char       buffer[32];  // Working buffer
   unsigned            code;        // Working code point
   UTF8                utf8(buffer, sizeof(buffer));

   for(code= 0; code < 0x00110000; code++)
   {
     if( code >= 0x0000D800 && code <= 0x0000DFFF )
     {
       code= 0x0000DFFF;
       continue;
     }

     utf8.set_used(0);
     utf8.encode(code);
//// debugf("%4d HCDM %.6x %.2x %.2x %.2x %.2x\n", __LINE__, code,
////        buffer[0], buffer[1], buffer[2], buffer[3]);

     utf8.set_used(0);
     if( code != utf8.decode() )
     {
       debugf("%4d %.2x %.2x %.2x %.2x\n", __LINE__,
              buffer[0], buffer[1], buffer[2], buffer[3]);

       utf8.set_used(0);
       debugf("%4d code(0x%x) utf8(0x%x)\n", __LINE__, code, utf8.decode());
       return 1;
     }
   }

   return errorCount;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_UTF8_encode
//
// Purpose-
//       Test UTF8::encode (valuerange 00..10ffff)
//
//----------------------------------------------------------------------------
static inline int
   test_UTF8_encode(int argc, char** argv) // Test UTF8::encode
{
   debugf("\ntest_UTF8_encode(%s)\n", utf8_encode);

   int                 errorCount= 0; // Number of errors encountered

   unsigned char       buffer[64];    // The encode buffer
   memset(buffer, 0xff, sizeof(buffer));
   UTF8                utf8(buffer, sizeof(buffer)); // Encoder

   unsigned code= utility::atox(utf8_encode);
   utf8.encode(code);

   debugf("%.6x %.2x %.2x %.2x %.2x\n", code,
          buffer[0], buffer[1], buffer[2], buffer[3]);

   return errorCount;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_UTF8_decode
//
// Purpose-
//       Test UTF8::decode (ONE encoded character)
//
//----------------------------------------------------------------------------
static inline int
   test_UTF8_decode(int argc, char** argv) // Test UTF8::decode
{
   debugf("\ntest_UTF8_decode(%s)\n", utf8_decode);

   int                 errorCount= 0; // Number of errors encountered

   char                buffer[1024];  // The encode buffer
   unsigned            size= 0;       // Number of bytes used

   // Fill the buffer
   const char* C= utf8_decode;
   while( size < sizeof(buffer) )
   {
     while( isspace(*C) ) C++;
     if( *C == '\0' )
       break;

     unsigned V= hexchar(*C);
     C++;
     if( *C != '\0' && !isspace(*C) )
     {
       V <<= 4;
       V  += hexchar(*C);
       C++;
     }

     buffer[size++]= V;
   }

   UTF8                utf8(buffer, size); // Encoder

   try {
     for(;;)
     {
       unsigned code= utf8.decode();
       debugf("%.6x\n", code);
     }
   } catch( pub::UTF8::BufferEmpty& X ) {
     debugf("** DONE **\n");
   } catch( ... ) {
     errorCount++;
     throw;
   }

   return errorCount;
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
     int             argc,          // Argument count
     char*           argv[])        // Argument array
{
   unsigned errorCount= 0;

   try {
     parm(argc, argv);

     if( opt_case )    errorCount += test_Case();
     if( opt_dump )    errorCount += test_dump();
     if( opt_latch )   errorCount += test_Latch();
     if( opt_must )    errorCount += test_Must();
     if( opt_signals ) errorCount += test_Signals();
     if( opt_utf8 )    errorCount += test_UTF8();
     if( opt_trace )   errorCount += test_Trace();
     if( utf8_encode ) errorCount += test_UTF8_encode(argc, argv);
     if( utf8_decode ) errorCount += test_UTF8_decode(argc, argv);
   } catch(Exception& X) {
     errorCount++;
     debugf("%4d %s\n", __LINE__, std::string(X).c_str());
   } catch(std::exception& X) {
     errorCount++;
     debugf("%4d std::exception(%s)\n", __LINE__, X.what());
   } catch(...) {
     errorCount++;
     debugf("%4d catch(...)\n", __LINE__);
   }

   if( errorCount == 0 )
     debugf("NO errors detected\n");
   else if( errorCount == 1 )
     debugf("1 error detected\n");
   else {
     debugf("%d errors detected\n", errorCount);
     errorCount= 1;
   }

   return errorCount;
}
