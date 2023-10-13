//----------------------------------------------------------------------------
//
//       Copyright (C) 2022-2023 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       TestIoda.cpp
//
// Purpose-
//       TestIoda.h
//
// Last change date-
//       2023/09/25
//
//----------------------------------------------------------------------------
#include <cassert>                  // For assert
#include <cstdint>                  // For size_t
#include <cstdlib>                  // For rand, srand
#include <ctime>                    // For time
#include <locale.h>                 // For setlocale

#include <pub/TEST.H>               // For VERIFY macros
#include <pub/Debug.h>              // For pub::Debug, namespace pub::debugging
#include <pub/Exception.h>          // For pub::Exception
#include <pub/Reporter.h>           // For pub::Reporter
#include <pub/utility.h>            // For pub::utilities
#include <pub/Wrapper.h>            // For pub::Wrapper

#include "pub/Ioda.h"               // For pub::Ioda, ... (Tested)

#define PUB _LIBPUB_NAMESPACE
using namespace PUB;
using namespace PUB::debugging;
using PUB::utility::visify;

using std::string;

typedef Ioda::Mesg        Mesg;
typedef Ioda::Page        Page;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose
}; // enum

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
// Extended options
static int             opt_dirty= false; // --dirty
static int             opt_size= false;  // --size
static int             opt_unit= true;   // (Always TRUE)
static struct option   opts[]=      // The getopt_long parameter: longopts
{  {"dirty",  no_argument,        &opt_dirty,       true} // --dirty
,  {"size",   no_argument,        &opt_size,        true} // --size
,  {0, 0, 0, 0}                     // (End of option list)
};

//----------------------------------------------------------------------------
//
// Subroutine-
//       size_of
//
// Purpose-
//       Display size of something
//
//----------------------------------------------------------------------------
#define SIZEOF(x) size_of(sizeof(x), #x)

static inline void
   size_of(                         // Display size of something
     size_t            size,        // The size
     const char*       name)        // The something's name
{  debugf("%8zd= sizeof(%s)\n", size, name); }

//----------------------------------------------------------------------------
//
// Subroutine-
//       equals
//
// Purpose-
//       Compare (write) Ioda data
//
//----------------------------------------------------------------------------
static inline void
   SNO(int line)                    // Should Not Occur
{  debugf("%4d %s SHOULD NOT OCCUR\n", line, __FILE__);
   throw std::runtime_error("should not occur");
}

static bool                         // TRUE if identical data
   equals(                          // Do Iodas contain identical data?
     const Ioda&       lhs,         // Left Hand Side Ioda
     const Ioda&       rhs)         // Right Hand Side Ioda
{
   if( lhs.get_used() != rhs.get_used() ) // If differing sizes
     return false;                  // (They can't be equal)

   if( lhs.get_used() == 0 )        // If both are read Iodas
     return true;                   // (Read Iodas are identical)

   // Get the associated msghdr iovec areas so we can compare data
   Mesg lhs_mesg;
   Mesg rhs_mesg;
   lhs.set_wr_mesg(lhs_mesg);
   rhs.set_wr_mesg(rhs_mesg);

   struct iovec* lhs_iov= lhs_mesg.msg_iov;
   struct iovec* rhs_iov= rhs_mesg.msg_iov;
   size_t lhs_len= lhs_mesg.msg_iovlen;
   size_t rhs_len= rhs_mesg.msg_iovlen;
   size_t lhs_lix= 0;               // Current index
   size_t rhs_lix= 0;
   const char* lhs_addr= nullptr;   // Current address
   const char* rhs_addr= nullptr;
   unsigned lhs_size= 0;            // Current remaining length
   unsigned rhs_size= 0;

   // Compare data areas
   for(;;) {
     if( lhs_size == 0 ) {
       if( lhs_lix >= lhs_len ) {   // If lhs EOF
         if( rhs_size != 0 || rhs_lix < rhs_len ) // If not rhs EOF
           SNO(__LINE__);             // (Should Not Occur)
         return true;
       }
       lhs_addr= (char*)lhs_iov[lhs_lix].iov_base;
       lhs_size= (unsigned)lhs_iov[lhs_lix++].iov_len;
       continue;
     }
     if( rhs_size == 0 ) {
       if( rhs_lix >= rhs_len )     // If rhs EOF
         SNO(__LINE__);             // (Should Not Occur) (lhs_size != 0)
       rhs_addr= (char*)rhs_iov[rhs_lix].iov_base;
       rhs_size= (unsigned)rhs_iov[rhs_lix++].iov_len;
       continue;
     }

     unsigned size= std::min(lhs_size, rhs_size);
     if( memcmp(lhs_addr, rhs_addr, size) != 0 )
       return false;

     lhs_addr += size;
     rhs_addr += size;
     lhs_size -= size;
     rhs_size -= size;
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_case
//
// Purpose-
//       Cut/paste sample test.
//
//----------------------------------------------------------------------------
static inline int
   test_case( void )                // Cut/paste source
{
   if( opt_verbose )
     debugf("\ntest_case:\n");
   int error_count= 0;

   return error_count;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_dirty
//
// Purpose-
//       The world-famous quick and dirty test.
//
//----------------------------------------------------------------------------
static inline int
   test_dirty( void )
{
   if( opt_verbose )
     debugf("\ntest_dirty:\n");
   int error_count= 0;

   return error_count;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_size
//
// Purpose-
//       Display class and structure size
//
//----------------------------------------------------------------------------
static inline int
   test_size( void )                // Test object sizes
{
   if( opt_verbose )
     debugf("\ntest_sizes:\n");
   int error_count= 0;

   SIZEOF(Ioda);
   SIZEOF(Ioda::Page);
   SIZEOF(Ioda::Mesg);
   SIZEOF(IodaReader);

   return error_count;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       unit_exceptions
//
// Purpose-
//       Ioda unit test: exception generation
//
//----------------------------------------------------------------------------
static inline int
   unit_exceptions( void )
{
   int error_count= 0;

   size_t read_size= 2'000;
   Ioda read(read_size);                       // An input Ioda
   Ioda into("This is an output Ioda");        // An output Ioda
   size_t into_used= into.get_used();
   error_count += VERIFY( read.get_size() == read_size );
   error_count += VERIFY( read.get_used() ==         0 );
   error_count += VERIFY( into.get_size() ==         0 );
   error_count += VERIFY( into.get_used() == into_used );

   // Test operator+=(Ioda&&) exceptions - - - - - - - - - - - - - - - - - - -
   bool
   exceptional= false;
   try {
     into += std::move(into);       // Cannot += from self
   } catch(std::runtime_error& X) {
     exceptional= true;
     if( opt_verbose )
       debugf("expected exception: %s\n", X.what());
   }
   error_count += VERIFY( exceptional == true );

   try {
     into += std::move(read);       // Cannot += from input Ioda
   } catch(std::runtime_error& X) {
     exceptional= true;
     if( opt_verbose )
       debugf("expected exception: %s\n", X.what());
   }
   error_count += VERIFY( exceptional == true );

   exceptional= false;
   try {
     read += std::move(into);       // Cannot += into input Ioda
   } catch(std::runtime_error& X) {
     exceptional= true;
     if( opt_verbose )
       debugf("expected exception: %s\n", X.what());
   }
   error_count += VERIFY( exceptional == true );

   // Test method append exceptions- - - - - - - - - - - - - - - - - - - - - -
   exceptional= false;
   try {
     into.append(into);             // Cannot append from self
   } catch(std::runtime_error& X) {
     exceptional= true;
     if( opt_verbose )
       debugf("expected exception: %s\n", X.what());
   }
   error_count += VERIFY( exceptional == true );

   try {
     into.append(read);             // Cannot append from input Ioda
   } catch(std::runtime_error& X) {
     exceptional= true;
     if( opt_verbose )
       debugf("expected exception: %s\n", X.what());
   }
   error_count += VERIFY( exceptional == true );

   try {
     read.append(into);             // Cannot append into input Ioda
   } catch(std::runtime_error& X) {
     exceptional= true;
     if( opt_verbose )
       debugf("expected exception: %s\n", X.what());
   }
   error_count += VERIFY( exceptional == true );

   // Test method copy exceptions- - - - - - - - - - - - - - - - - - - - - - -
   exceptional= false;
   try {
     into.copy(into);               // Cannot copy from self
   } catch(std::runtime_error& X) {
     exceptional= true;
     if( opt_verbose )
       debugf("expected exception: %s\n", X.what());
   }
   error_count += VERIFY( exceptional == true );

   try {
     into.copy(read);               // Cannot copy from input Ioda
   } catch(std::runtime_error& X) {
     exceptional= true;
     if( opt_verbose )
       debugf("expected exception: %s\n", X.what());
   }
   error_count += VERIFY( exceptional == true );

   try {
     read.copy(into);               // Cannot copy into input Ioda
   } catch(std::runtime_error& X) {
     exceptional= true;
     if( opt_verbose )
       debugf("expected exception: %s\n", X.what());
   }
   error_count += VERIFY( exceptional == true );

   // Test method set_used - - - - - - - - - - - - - - - - - - - - - - - - - -
   exceptional= false;
   try {
     read.set_used(read_size + 1);  // More used data than buffer size
   } catch(std::runtime_error& X) {
     exceptional= true;
     if( opt_verbose )
       debugf("expected exception: %s\n", X.what());
   }
   error_count += VERIFY( exceptional == true );

   try {
     into.set_used(1);              // Method set_used requires an input Ioda
   } catch(std::runtime_error& X) {
     exceptional= true;
     if( opt_verbose )
       debugf("expected exception: %s\n", X.what());
   }
   error_count += VERIFY( exceptional == true );

   try {
     read.set_used(0);              // Nothing used
   } catch(std::runtime_error& X) {
     exceptional= true;
     if( opt_verbose )
       debugf("expected exception: %s\n", X.what());
   }
   error_count += VERIFY( exceptional == true );

   // Test output method exceptions- - - - - - - - - - - - - - - - - - - - - -
   exceptional= false;
   try {
     read.put('x');                 // Cannot put(char) into input Ioda
   } catch(std::runtime_error& X) {
     exceptional= true;
     if( opt_verbose )
       debugf("expected exception: %s\n", X.what());
   }
   error_count += VERIFY( exceptional == true );

   exceptional= false;
   try {                            // Ioda::put(string) converts to write
     read.put("put string");        // Cannot write into input Ioda
   } catch(std::runtime_error& X) {
     exceptional= true;
     if( opt_verbose )
       debugf("expected exception: %s\n", X.what());
   }
   error_count += VERIFY( exceptional == true );

   // Size checks verify that exceptions didn't change anything
   error_count += VERIFY( read.get_size() == read_size );
   error_count += VERIFY( read.get_used() == 0 );
   error_count += VERIFY( into.get_size() == 0 );
   error_count += VERIFY( into.get_used() == into_used );

   return error_count;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_unit
//
// Purpose-
//       Ioda unit test
//
//----------------------------------------------------------------------------
static inline int
   test_unit( void )
{
   if( opt_verbose )
     debugf("\ntest_unit:\n");
   int error_count= 0;

   constexpr int LINES= 500;
   string line= "The quick brown fox jumps over the lazy dog.\r\n\r\n";
   assert( line.size() == 48 );     // (Total size 24,000)

   //-------------------------------------------------------------------------
   if( opt_verbose )
     debugf("\nIoda::put(string)\n");
   Ioda from;
   error_count += VERIFY( from.get_size() == 0 );
   error_count += VERIFY( from.get_used() == 0 );

   for(int i= 0; i<LINES; ++i)
     from.put(line);
   error_count += VERIFY( from.get_used() == 24'000 );
   if( opt_verbose )
     from.debug("from 24,000; size 0");

   string full= (string)from;
   error_count += VERIFY( full.size() == 24'000 ); // (Total size 24,000)

   //-------------------------------------------------------------------------
   if( opt_verbose )
     debugf("\noperator+=(Ioda&&)\n");
   Ioda into;
   into += std::move(from);
   error_count += VERIFY( from.get_size() ==      0 );
   error_count += VERIFY( from.get_used() ==      0 );
   error_count += VERIFY( into.get_size() ==      0 );
   error_count += VERIFY( into.get_used() == 24'000 );

   if( opt_verbose ) {
     debugf("\n");
     from.debug("empty from 0");
     debugf("\n");
     into.debug("into 24,000");
   }

   //-------------------------------------------------------------------------
   if( opt_verbose )
     debugf("\nIoda::get_mesg\n");
   Mesg mesg;
   Ioda read;
   read.set_rd_mesg(mesg, 20'000);
   error_count += VERIFY( read.get_size() == 20'000 );
   error_count += VERIFY( read.get_used() ==      0 );
   error_count += VERIFY( mesg.size()     == 20'000 );
   if( opt_verbose && false )
     read.debug("meaningless data"); // Read Ioda content is meaningless

   read.set_used(5'000);
   error_count += VERIFY( mesg.size() == 20'000 );
   error_count += VERIFY( read.get_size() ==      0 );
   error_count += VERIFY( read.get_used() ==  5'000 );
   read.set_wr_mesg(mesg);
   error_count += VERIFY( mesg.size()     ==  5'000 );
   if( opt_verbose && false )
     read.debug("meaningless data"); // Read Ioda content is meaningless

   // (Tests depends upon Ioda's PAGE_SIZE == 4096. It's defined in Ioda.cpp)
   into.set_wr_mesg(mesg, 6'000);
   error_count += VERIFY( mesg.size() == 6'000 );
   string head4096((char*)mesg.msg_iov[0].iov_base, mesg.msg_iov[0].iov_len);
   string tail1904((char*)mesg.msg_iov[1].iov_base, mesg.msg_iov[1].iov_len);
   error_count += VERIFY( head4096 == full.substr(    0, 4'096));
   error_count += VERIFY( tail1904 == full.substr(4'096, 1'904));
   if( opt_verbose )
     mesg.debug("wr_mesg 0x1770");

   //-------------------------------------------------------------------------
   if( opt_verbose )
     debugf("\nIoda::split/discard\n");

   enum { SIZES_DIM= 25 };
   static const size_t sizes[SIZES_DIM]=
      { 0x00000 //  0
      , 0x00001 //  1
      , 0x00002 //  2
      , 0x00003 //  3
      , 0x00004 //  4
      , 0x00ffe //  5
      , 0x00fff //  6
      , 0x01000 //  7 4,096
      , 0x01001 //  8
      , 0x01002 //  9
      , 0x01ffd // 10 8,090
      , 0x01ffe // 11
      , 0x02000 // 12 8,092
      , 0x02001 // 13
      , 0x02002 // 14
      , 0x04ffe // 15 20,478
      , 0x04fff // 16
      , 0x05000 // 17 20,480
      , 0x05001 // 18
      , 0x05002 // 19
      , 0x05003 // 20
      , 0x05004 // 21
      ,  23'999 // 22 0x05dbf
      ,  24'000 // 23 0x05dc0
      ,  24'001 // 24 0x05dc1
      };        // 25 SIZES_DIM
   for(size_t sx= 0; sx < SIZES_DIM; ++sx) {
     size_t size= sizes[sx];
     { if( opt_verbose > 1 )
         tracef("[%2zd] Split size:(0x%.6zx) %'6zd\n", sx, size, size);
       Ioda tail; tail.put(full);
       Ioda head; tail.split(head, size);
       error_count += VERIFY( ((string)head + (string)tail) == full);
       if( error_count ) break;

       head.reset();                // Test discard
       head.put(full);
       head.discard(size);
       error_count += VERIFY( equals(head, tail) );
     }
   }

   for(size_t sx= 0; sx < 64; ++sx) {
     size_t size= rand() % 24'100;  // (Can be larger than Ioda.used)
     { if( opt_verbose > 1 )
         tracef("[%2zd]  Rand size:(0x%.6zx) %'6zd\n", sx, size, size);
       Ioda tail; tail.put(full);
       Ioda head; tail.split(head, size);
       error_count += VERIFY( ((string)head + (string)tail) == full);
       if( error_count ) break;

       head.reset();                // Test discard
       head.put(full);
       head.discard(size);
       error_count += VERIFY( equals(head, tail) );
     }
   }

   //-------------------------------------------------------------------------
   if( opt_verbose )
     debugf("\nIodaReader\n");
   IodaReader reader(into);
   int L= (int)line.size();
   line= line.substr(0, L-4);

   for(int i= 0; i<LINES; ++i) {
     string S= reader.get_line();
     error_count += VERIFY( S == line );
//   if( error_count ) debugf("%4d [%d] '%s'\n", __LINE__, i, S.c_str());
     S= reader.get_line();
     error_count += VERIFY( S == "" );
     if( error_count )
       break;
   }

   for(int i= 0; i<LINES; ++i) {
     int x= rand() % LINES;
     x *= L;
     reader.set_offset(x);
     string S= reader.get_line();
     error_count += VERIFY( S == line );
     S= reader.get_line();
     error_count += VERIFY( S == "" );
     if( error_count )
       break;
   }

   reader.set_offset(0);            // Test get_token
   error_count += VERIFY( reader.get_token(" ") == "The" );
   error_count += VERIFY( reader.get_token(" ") == "quick" );
   error_count += VERIFY( reader.get_token(" ") == "brown" );
   error_count += VERIFY( reader.get_token(" ") == "fox" );
   error_count += VERIFY( reader.get_token(" ") == "jumps" );
   error_count += VERIFY( reader.get_token("\r\n") == "over the lazy dog." );
   error_count += VERIFY( reader.get_token("\r\n") == "" );
   error_count += VERIFY( reader.get_token(" ") == "The" );
   error_count += VERIFY( reader.get_token("s") == "quick brown fox jump" );
   error_count += VERIFY( reader.get_token("\r\n") == " over the lazy dog." );

   //-------------------------------------------------------------------------
   if( opt_verbose )
     debugf("\nIoda::exception generation\n");
   error_count += unit_exceptions();

   //-------------------------------------------------------------------------
   if( opt_verbose )
     debugf("\nDestructors\n");

   return error_count;
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
   Wrapper  tc= opts;               // The test case wrapper
   Wrapper* tr= &tc;                // A test case wrapper pointer

   setlocale(LC_NUMERIC, "");       // Enables printf("%'d\n", 123'456'789)

   tc.on_info([]() {
     fprintf(stderr, "  --dirty\tRun dirty test\n");
     fprintf(stderr, "  --size\tRun object size test\n");
   });

   tc.on_init([](int, char**) {
     srand(time(nullptr));          // Initialize the random number generator
     return 0;
   });

   tc.on_main([tr](int, char**) {
     int error_count= 0;            // Error count

     try {
       if( opt_verbose )
         debugf("%s: %s %s\n", __FILE__, __DATE__, __TIME__);

       if( opt_size  ) error_count= test_size();
       if( opt_unit  ) error_count= test_unit();
//     if( opt_dirty ) error_count += test_dirty();

       // Statistics (if opt_verbose && compiled into Ioda.cpp)
       if( opt_verbose ) {
         Reporter::get()->report([](Reporter::Record& record) {
           debugf("%s\n", record.h_report().c_str());
         }); // reporter.report
       }
     } catch(const char* x) {
       debugf("FAILED: Exception: const char*(%s)\n", x);
       ++error_count;
     } catch(std::exception& x) {
       debugf("FAILED: Exception: exception(%s)\n", x.what());
       ++error_count;
     } catch(...) {
       debugf("FAILED: Exception: ...\n");
       ++error_count;
     }

     if( opt_verbose || error_count ) {
       debugf("\n");
       tr->report_errors(error_count);
     }
     return int(error_count != 0);
   });

   //-----------------------------------------------------------------------
   // Run the tests
   opt_hcdm= HCDM;
   opt_verbose= VERBOSE;
   return tc.run(argc, argv);
}
