//----------------------------------------------------------------------------
//
//       Copyright (C) 2023 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Main.cpp
//
// Purpose-
//       RFC7541 unit, example, and regression tests.
//
// Last change date-
//       2023/10/19
//
//----------------------------------------------------------------------------
#include <cstdint>                  // For uint32_t, uint16_t, ...

#include <locale.h>                 // For setlocale
#include <stdexcept>                // For std::runtime_error
#include <string>                   // For std::string

#include <assert.h>                 // For assert
#include <string.h>                 // For

// The tested includes
#include <pub/TEST.H>               // For VERIFY, ...
#include <pub/Debug.h>              // For debugging
#include <pub/Interval.h>           // For pub::Interval
#include <pub/Ioda.h>               // For pub::Ioda, pub::IodaReader
#include <pub/utility.h>            // For pub::utility::visify
#include <pub/Wrapper.h>            // For pub::Wrapper

#include "RFC7541.h"                // For class RFC7541, tested

// Namespace accessors
#define PUB _LIBPUB_NAMESPACE
using namespace PUB;                // For pub objects
using namespace PUB::debugging;     // For debugging subroutines
using PUB::utility::visify;         // For pub::utility::visify method
using namespace std;

// RFC7541 types
typedef RFC7541::connection_error   connection_error;
typedef RFC7541::octet              octet;
typedef RFC7541::Huff               Huff;
typedef RFC7541::Entry              Entry;
typedef RFC7541::Integer            Integer;
typedef RFC7541::Pack               Pack;
typedef RFC7541::Property           Property;
typedef RFC7541::Properties         Properties;
typedef RFC7541::Value_t            Value_t;

// RFC7541 ENCODE_TYPE parameters
enum ENCODE_TYPE
{  ET_INDEX=           RFC7541::ET_INDEX
,  ET_INSERT_NOINDEX=  RFC7541::ET_INSERT_NOINDEX
,  ET_INSERT=          RFC7541::ET_INSERT
,  ET_RESIZE=          RFC7541::ET_RESIZE
,  ET_NEVER_NOINDEX=   RFC7541::ET_NEVER_NOINDEX
,  ET_NEVER=           RFC7541::ET_NEVER
,  ET_CONST_NOINDEX=   RFC7541::ET_CONST_NOINDEX
,  ET_CONST=           RFC7541::ET_CONST
}; // ENCODE_TYPE

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose

,  DYNAMIC_ENTRY_0= Pack::DYNAMIC_ENTRY_0
,  JUST_CHECKING= false             // Check one timing operation?
}; // enum

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static int             error_count= 0; // Error counter
static void*           table= nullptr; // The Trace table

//----------------------------------------------------------------------------
// Extended options
//----------------------------------------------------------------------------
extern int             opt_hcdm;    // Hard Core Debug Mode?
extern int             opt_verbose; // Debugging verbosity

static int             opt_debug=    0; // --debug
static int             opt_dirty=    0; // --dirty
static int             opt_timing=   0; // --timing
static int             opt_trace=    0; // --trace
static struct option   opts[]=      // The getopt_long parameter: longopts
{  {"debug",           no_argument,       &opt_debug,       true}
,  {"dirty",           no_argument,       &opt_dirty,       true}
,  {"timing",          no_argument,       &opt_timing,      true}
,  {"trace",           optional_argument, &opt_trace, 0x00400000}
,  {0, 0, 0, 0}                     // (End of option list)
};

static const char*     ostr="";     // The getopt_long parameter: optstring

// Ignore error messages for character arrays containing any value > 127
#pragma GCC diagnostic ignored "-Wnarrowing"

//----------------------------------------------------------------------------
//
// Subroutine-
//       torf
//
// Purpose-
//       Convert condition to "true" or "false" string
//
//----------------------------------------------------------------------------
static inline const char*
   torf(bool condition)
{  return condition ? "true" : "false"; }

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_dirty
//
// Purpose-
//       The world-famous "quick and dirty test."
//
//----------------------------------------------------------------------------
static inline int
   test_dirty( void )               // Quick and dirty test dujour
{
   if( opt_verbose )
     debugf("\ntest_dirty:\n");
   int error_count= 0;

   // Info: Tested string_decode, now fixed and also now private.

   return error_count;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       time_Huff
//
// Purpose-
//       Test RFC7541: Huffman encoding/decoding timing tests
//
//----------------------------------------------------------------------------
static inline int
   time_Huff( void )                // Test RFC7541: Huff timing tests
{
   // debugf("\nRFC 7541 Huff encode/decode timing test:\n");
   debugf("RFC 7541 Huff timing test skipped: it's NOT CODED YET.\n");
   error_count= 0;

   return error_count;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       time_pack
//
// Purpose-
//       Test RFC7541: HPACK timing tests
//
//----------------------------------------------------------------------------
static inline int
   time_pack( void )                // Test RFC7541: HPACK timing tests
{
   debugf("\nRFC 7541 HPACK timing tests:\n");

   Integer    integer;
   Ioda       writer;
   IodaReader reader(writer);

   int        error_count= 0;

   //-------------------------------------------------------------------------
   // Integer encoding/decoding timing tests
   debugf("\nRFC7541::Integer encode/decode timing test:\n");

   Interval   interval;
   size_t     ITERATIONS=  100;
   uint32_t   QUESTIONS= 100'000;
   for(size_t i= 0; i<ITERATIONS; ++i) {
     for(uint32_t question= 0; question < QUESTIONS; ++question) {
       writer.reset();
       reader.reset();
       integer.encode(writer, question, 0x50, 4);
       integer.encode(writer, question, 0xA0, 4);
       uint32_t answer= integer.decode(reader, 4);
       error_count += VERIFY( answer == question );
       if( error_count ) {
         debugf("Q(%d) A(%d)\n", question, answer);
         break;
       }

       answer= integer.decode(reader, 4);
       error_count += VERIFY( answer == question );
       if( error_count ) {
         debugf("Q(%d) A(%d)\n", question, answer);
         break;
       }
       error_count += VERIFY( reader.get() == EOF );
     }
   }
   interval.stop();
   double operations= (double)ITERATIONS * (double)QUESTIONS * 2.0;
   debugf("%'16.3f seconds, %'12.0f Integer encode/decode operations\n"
         , (double)interval, operations);
   debugf("%'16.3f operations/second\n", operations / (double)interval);

   //-------------------------------------------------------------------------
   // Pack encoding/decoding timing tests
   debugf("\nRFC7541::Pack encode/decode timing test:\n");
   Pack out_pack(512), inp_pack(512);

   ITERATIONS= 1'000'000;
   interval.start();
   for(size_t iteration= 0; iteration <= ITERATIONS; ++iteration) {
     writer.reset(); reader.reset();
     Properties out_prop, inp_prop;

     char buffer[64];
     sprintf(buffer, "N_%.14zd", iteration);
     string name= buffer;
     sprintf(buffer, "V_%.14zd", iteration);
     string value= buffer;
     out_prop.append(name, value);
     sprintf(buffer, "V_%.14zd", iteration+1);
     value= buffer;
     out_prop.append(name, value);

     out_pack.encode(writer, out_prop);
     inp_prop= inp_pack.decode(reader);
     error_count += VERIFY( inp_prop == out_prop );
     error_count += VERIFY( reader.get_length() == 0 );
     if( JUST_CHECKING && iteration == 1026 ) {
       reader.dump("just checking");
       inp_pack.debug("just checking");
     }
   }
   interval.stop();
   operations= (double)ITERATIONS * 2.0;
   debugf("%'16.3f seconds, %'12.0f Pack encode/decode operations\n"
         , (double)interval, operations);
   debugf("%'16.3f operations/second\n", operations / (double)interval);

   return error_count;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       unit_Huff
//
// Purpose-
//       Test RFC7541: Huffman encoding/decoding
//
//----------------------------------------------------------------------------
static inline int
   unit_Huff( void )                // Test RFC7541: Huffman encoding/decoding
{
   if( opt_verbose )
     debugf("\nunit_Huff:\n");
   int error_count= 0;

   // Static tests
   VERIFY( RFC7541::Huff::encoded_length("") == 0 );
   const Huff nul;                  // Empty Huff
   VERIFY( nul == nul );            // Self comparison
   Huff one;
   VERIFY( nul == one );            // Empty to empty comparison

   // Dynamic tests
   char buffer[256];                // The string test buffer
   for(int i= 0; i<256; ++i)
     buffer[i]= i;
   buffer[0]= 255;
   buffer[255]= 0;

   for(size_t L= 1; L<=256; ++L) {
     std::string sample(buffer, L);

     if( opt_verbose )
       debugf("\nsample '%s'\n", pub::utility::visify(sample).c_str());
     one= sample;

     std::string check= one.decode();
     error_count += VERIFY( check == sample );
     error_count += VERIFY(Huff::encoded_length(sample) == one.get_size());
     if( opt_verbose)
       debugf("decode '%s'\n", pub::utility::visify(check).c_str());
     if( error_count ) {
       if( !opt_verbose ) {
         debugf("sample '%s'\n", pub::utility::visify(sample).c_str());
         debugf("decode '%s'\n", pub::utility::visify(check).c_str());
       }
       one.debug("encode/decode error");
       break;
     }

     // Copy test
     Huff two= one;
     error_count += VERIFY( two.decode() == sample );
     if( error_count ) {
       one.debug("one");
       two.debug("two");
       break;
     }

     // Accessor method tests
     const octet* one_octet= one.get_addr();
     size_t one_size=  one.get_size();
     const octet* two_octet= two.get_addr();
     size_t two_size=  two.get_size();
     VERIFY( one_octet != two_octet );
     VERIFY( one_size  == two_size  );
     VERIFY( memcmp(one_octet, two_octet, two_size) == 0 );

     // String constructor test
     Huff str(sample);
     error_count += VERIFY( str.decode() == sample );
     if( error_count ) {
       one.debug("one");
       two.debug("str");
       break;
     }

     // Comparison operator test
     VERIFY( one == two );
     str= "Strawberry";
     VERIFY( two != str );
     VERIFY( one != nul );

     // Move constructor/assignment test
     Huff h03= std::move(two);
     VERIFY( one == h03 );
     VERIFY( two == nul );

     two= std::move(h03);
     VERIFY( one == two );
     VERIFY( h03 == nul );

     if( error_count )
       break;
   }

   return error_count;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       unit_pack
//
// Purpose-
//       Test RFC7541: HPACK unit test
//
//----------------------------------------------------------------------------
static inline int
   pack_verify(                     // Verify question == answer
     int               line,        // Caller's line number
     IodaReader&       reader,      // Associated Reader
     Value_t           question,
     Value_t           answer)
{
   int error_count= VERIFY( answer == question );
   if( error_count ) {
     debugf("%4d A(%d) Q(%d)\n", line, answer, question);
     reader.dump("A != Q");
   }

   return error_count;
}

static inline int
   unit_pack( void )                // Test RFC7541::Integer encoding/decoding
{
   if( opt_verbose )
     debugf("\nunit_pack:\n");
   int error_count= 0;

   // Input/output objects
   Ioda       writer;
   IodaReader reader(writer);

   // Test objects
   Integer    integer;
   Properties inp_prop;
   Properties out_prop;
   Pack       inp_pack;
   Pack       out_pack;

   //-------------------------------------------------------------------------
   // Integer unit tests (Find out the question being answered!)
   Value_t question= 42;
   integer.encode(writer, question, 0xA0, 4); // Bringup test, the question
   if( opt_verbose )
     writer.debug("Integer.encode");

   Value_t answer= integer.decode(reader, 4);
   error_count += VERIFY( answer == question );
   if( opt_verbose )
     reader.dump("Integer.decode");

   error_count += VERIFY( reader.get() == EOF );

   for(question= 0; question < 60'000; question += 11) {
     writer.reset();
     reader.reset();
     integer.encode(writer, question); // (Default: 0x80, 7)
     integer.encode(writer, question, 0x00, 7);
     integer.encode(writer, question, 0xC0, 6);
     integer.encode(writer, question, 0xA0, 5);
     integer.encode(writer, question, 0x50, 4);
     integer.encode(writer, question, 0xA8, 3);

     error_count += VERIFY( (reader.peek() & 0x80) == 0x80 );
     answer= integer.decode(reader);
     if( pack_verify(__LINE__, reader, question, answer) ) // 0x80, 7
       return 1;

     error_count += VERIFY( (reader.peek() & 0x80) == 0x00 );
     answer= integer.decode(reader);
     if( pack_verify(__LINE__, reader, question, answer) ) // 0x00, 7
       return 1;

     error_count += VERIFY( (reader.peek() & 0xC0) == 0xC0 );
     answer= integer.decode(reader, 6);
     if( pack_verify(__LINE__, reader, question, answer) ) // 0xC0, 6
       return 1;

     error_count += VERIFY( (reader.peek() & 0xE0) == 0xA0 );
     answer= integer.decode(reader, 5);
     if( pack_verify(__LINE__, reader, question, answer) ) // 0xA0, 5
       return 1;

     error_count += VERIFY( (reader.peek() & 0xF0) == 0x50 );
     answer= integer.decode(reader, 4);
     if( pack_verify(__LINE__, reader, question, answer) ) // 0x50, 4
       return 1;

     error_count += VERIFY( (reader.peek() & 0xF8) == 0xA8 );
     answer= integer.decode(reader, 3);
     if( pack_verify(__LINE__, reader, question, answer) ) // 0xA8, 3
       return 1;

     error_count += VERIFY( reader.peek() == EOF );
     error_count += VERIFY( reader.get() == EOF );
   }

   //-------------------------------------------------------------------------
   // HPACK ET_RESIZE encoding/decoding tests
   out_pack.reset(); inp_pack.reset();
   writer.reset(); reader.reset();
   if( opt_verbose ) {
     inp_pack.hcdm= true;
     inp_pack.verbose= 1;
   }

   out_pack.resize(writer, 0);
   out_pack.resize(writer, 256);
   inp_pack.decode(reader);
   if( opt_verbose ) {
     inp_pack.debug("ET_RESIZE 0, 256");
     writer.debug("ET_RESIZE 0, 256");
   }

   // Verify reorganization of values
   writer.reset(); reader.reset();
   out_prop.reset(); inp_prop.reset();
   out_prop.append("N123456789ABCD00", "V123456789ABCD09", ET_INSERT_NOINDEX);
   out_prop.append("N123456789ABCD00", "V123456789ABCD08", ET_INSERT_NOINDEX);
   out_prop.append("N123456789ABCD00", "V123456789ABCD07", ET_INSERT_NOINDEX);
   out_prop.append("N123456789ABCD00", "V123456789ABCD06", ET_INSERT_NOINDEX);
   out_prop.append("N123456789ABCD01", "V123456789ABCD05", ET_INSERT_NOINDEX);
   out_prop.append("N123456789ABCD01", "V123456789ABCD04", ET_INSERT_NOINDEX);
   out_prop.append("N123456789ABCD01", "V123456789ABCD03", ET_INSERT_NOINDEX);
   out_prop.append("N123456789ABCD01", "V123456789ABCD02", ET_INSERT_NOINDEX);
   out_prop.append("N123456789ABCD02", "V123456789ABCD01", ET_INSERT_NOINDEX);
   out_prop.append("N123456789ABCD02", "V123456789ABCD00", ET_INSERT_NOINDEX);
   out_pack.encode(writer, out_prop);
   inp_pack.decode(reader);
   if( opt_verbose ) {
     inp_pack.debug("ET_REORG 256");
     if( opt_verbose > 1 )
       out_pack.debug("ET_REORG 256");
   }

   writer.reset(); reader.reset();
   out_pack.resize(writer, 512);
   inp_pack.decode(reader);
   if( opt_verbose ) {
     inp_pack.debug("inp_pack ET_REORG 512");
//   if( opt_verbose > 1 )
       out_pack.debug("out_pack ET_REORG 512");
   }

   writer.reset(); reader.reset();
   out_prop.reset(); inp_prop.reset();
   out_pack.resize(writer, 31);     // Too small to contain any entries

   // (HPACK tables that are too small to contain entries are still usable)
   out_prop.append("N123456789ABCD02", "V123456789ABCDXX");
   out_pack.encode(writer, out_prop);
   inp_prop= inp_pack.decode(reader);
   error_count += VERIFY( inp_prop == out_prop );
   if( opt_verbose ) {
     inp_pack.debug("inp_pack ET_RESIZE 31");
     inp_prop.debug("inp_prop ET_RESIZE 31");
   }

   // ENCODE/DECODE: Too many resize operations
   out_pack.reset(); inp_pack.reset(); // ENCODE: Too many resize operations
   writer.reset(); reader.reset();
   bool caught= false;
   try {
     out_pack.resize(writer, 0);
     out_pack.resize(writer, 128);
     out_pack.resize(writer, 64);
   } catch(connection_error& X) {
     caught= true;
     if( opt_verbose )
       debugf("(Expected) connection_error(%s) caught\n", X.what());
   }
   error_count += VERIFY( caught );

   out_pack.reset(); inp_pack.reset(); // DECODE: Too many resize operations
   writer.reset(); reader.reset();
   caught= false;
   try {
     char buffer[5]= {0x20,0x3F,0x21,0x3F,0x22};
     writer.write(buffer, 5);       // resize(0); resize(64); resize(65);
     inp_pack.decode(reader);
   } catch(connection_error& X) {
     caught= true;
     if( opt_verbose )
       debugf("(Expected) connection_error(%s) caught\n", X.what());
   }
   error_count += VERIFY( caught );

   // ENCODE/DECODE: Resize not first operation
   out_pack.reset(); inp_pack.reset(); // ENCODE: Resize not first operation
   writer.reset(); reader.reset();
   caught= false;
   try {
     writer.put(0x84);              // ':method': 'GET'
     out_pack.resize(writer, 64);
   } catch(connection_error& X) {
     caught= true;
     if( opt_verbose )
       debugf("(Expected) connection_error(%s) caught\n", X.what());
   }
   error_count += VERIFY( caught );

   out_pack.reset(); inp_pack.reset(); // DECODE: Resize not first operation
   writer.reset(); reader.reset();
   caught= false;
   try {
     char buffer[3]= {0x84,0x3F,0x21};
     writer.write(buffer, 3);       // ':method': 'GET'; resize(64)
     inp_pack.decode(reader);
   } catch(connection_error& X) {
     caught= true;
     if( opt_verbose )
       debugf("(Expected) connection_error(%s) caught\n", X.what());
   }
   error_count += VERIFY( caught );

   // ENCODE/DECODE: Second resize <= first
   out_pack.reset(); inp_pack.reset(); // Second resize <= first
   writer.reset(); reader.reset();
   caught= false;
   try {
     out_pack.resize(writer, 64);
     out_pack.resize(writer, 0);
   } catch(connection_error& X) {
     caught= true;
     if( opt_verbose )
       debugf("(Expected) connection_error(%s) caught\n", X.what());
   }
   error_count += VERIFY( caught );

   out_pack.reset(); inp_pack.reset(); // Second resize <= first
   writer.reset(); reader.reset();
   caught= false;
   try {
     char buffer[3]= {0x3F,0x21,0x20};
     writer.write(buffer, 3);       // resize(64); resize(0)
     inp_pack.decode(reader);
   } catch(connection_error& X) {
     caught= true;
     if( opt_verbose )
       debugf("(Expected) connection_error(%s) caught\n", X.what());
   }
   error_count += VERIFY( caught );

   inp_pack.hcdm= false;
   inp_pack.verbose= 0;

   //-------------------------------------------------------------------------
   // HPACK encoding/decoding tests
   out_pack.reset(); inp_pack.reset();
   out_pack.resize(256); inp_pack.resize(256);
   out_pack.resize(512); inp_pack.resize(512);

   size_t ITERATIONS= 100'000;
   size_t DISPLAY_MAX= 1056;
   size_t DISPLAY_MIN= 1024;
   for(size_t iteration= 1; iteration <= ITERATIONS; ++iteration) {
     if( iteration >= DISPLAY_MAX ) {
       inp_pack.hcdm= false;
       inp_pack.verbose= 0;
     } else if( opt_verbose && iteration == DISPLAY_MIN ) {
       inp_pack.hcdm= true;
       inp_pack.verbose= 1;
     }
     writer.reset(); reader.reset();
     out_prop.reset(); inp_prop.reset();
     char buffer[64];
     sprintf(buffer, "N_%.14zd", iteration);
     string name= buffer;
     sprintf(buffer, "V_%.14zd", iteration);
     string value= buffer;
     out_prop.append(name, value);

     if( opt_verbose && iteration >= DISPLAY_MIN && iteration < DISPLAY_MAX )
       debugf("\nENCODE\n");
     out_pack.encode(writer, out_prop);
     if( opt_verbose && iteration >= DISPLAY_MIN && iteration < DISPLAY_MAX )
       out_pack.debug("pack_encode");

     if( opt_verbose && iteration >= DISPLAY_MIN && iteration < DISPLAY_MAX )
       debugf("\nDECODE\n");
     inp_prop= inp_pack.decode(reader);
     if( opt_verbose && iteration >= DISPLAY_MIN && iteration < DISPLAY_MAX )
       inp_pack.debug("pack_decode");

     error_count += VERIFY( inp_prop == out_prop );
     error_count += VERIFY( reader.get_length() == 0 );
   }
   inp_pack.hcdm= false;
   inp_pack.verbose= 0;

   return error_count;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_time
//
// Purpose-
//       RFC7541 timing tests
//
//----------------------------------------------------------------------------
static inline int
   test_time( void )                // RFC7541 timing tests
{
   int error_count= 0;

   error_count += time_Huff();
   error_count += time_pack();

   return error_count;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_unit
//
// Purpose-
//       RFC7541 Unit tests
//
//----------------------------------------------------------------------------
static inline int
   test_unit( void )                // RFC7541 Unit tests
{
   int error_count= 0;

   error_count += unit_Huff();
   error_count += unit_pack();

   return error_count;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       exam_7541
//
// Purpose-
//       Test RFC7541 examples
//
//----------------------------------------------------------------------------
#include "Main7541.hpp"

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
   Wrapper  tc(opts, ostr);         // The test case wrapper
   Wrapper* tr= &tc;                // A test case wrapper pointer

   tc.on_parm([tr](std::string P, const char* V)
   {
     if( opt_verbose > 1 )
       debugf("on_parm(%s,%s)\n", P.c_str(), V);

     if( false )                    // Select parameters in else if() {...}
       ;

     else if( P == "trace" ) {
       if( V )
         opt_trace= tr->ptoi(V);
     }

     else if( P[0] == '-' ) {       // If switch option
       debugf("%4d Should not occur %c,%d\n", __LINE__, P[1], P[1]);
     }

     else
       debugf("Unexpected parameter '%s'='%s'\n", P.c_str(), V);

     return 0;
   });

   tc.on_info([](void)
   {
     fprintf(stderr,
            "  --debug\tRun debugging displays instead of tests\n"
            "  --dirty\tRun \"quick and dirty\" test\n"
            "  --timing\tRun timing tests\n"
            "  --trace\t{=size} Create internal trace file './trace.mem'\n"
            );
   });

   tc.on_init([tr](int, char**)
   {
     setlocale(LC_NUMERIC, "");     // Allow printf("%'d\n", 123456780)

     if( opt_trace )
       table= tr->init_trace("./trace.mem", opt_trace);

     return 0;
   });

   tc.on_term([tr](void)
   {
     if( table )
       tr->term_trace(table, opt_trace);
   });

   tc.on_main([tr](int, char**)
   {
     error_count= 0;

     if( opt_verbose ) {
       debugf("\nOptions:\n");
       debugf("%5s hcdm\n", torf(opt_hcdm));
       debugf("%5d verbose\n", opt_verbose);

       debugf("%5s debug\n", torf(opt_debug));
       debugf("%5s timing\n", torf(opt_timing));
       debugf("%5s trace: %#x\n", torf(opt_trace), opt_trace);
     }

     if( opt_dirty ) {              // Quick and dirty test (** ONLY **)
       error_count += test_dirty();
     } else if( opt_debug ) {       // Table display (** ONLY **)
       RFC7541::debug("TABLES");
     } else {
       error_count += test_unit();
       // error_count += VERIFY( bool("FORCED ERROR") != true );
       if( error_count == 0 ) {
         if( opt_timing )
           error_count += test_time();
         error_count += exam_7541();
       }
     }

     if( opt_verbose ) {
       debugf("\n");
       tr->report_errors(error_count);
     }
     return int(error_count != 0);
   });

   //-------------------------------------------------------------------------
   // Run the test
   try {
     opt_hcdm= HCDM;
     opt_verbose= VERBOSE;

     return tc.run(argc, argv);
   } catch(std::exception& x) {
     debugf("std::exception(%s)\n", x.what());
   } catch(const char* x) {
     debugf("Exception(const char*(%s))\n", x);
   } catch(...) {
     debugf("Exception ...\n");
   }

   return 2;
}
