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
//       2023/04/29
//
// Implementation notes-
//       Usability study.
//
//----------------------------------------------------------------------------
#include <cassert>                  // For assert
#include <cstdint>                  // For size_t
#include <cstdlib>                  // For rand
#include <locale.h>                 // For setlocale

#include <pub/TEST.H>               // For VERIFY macros
#include <pub/Debug.h>              // For pub::Debug, namespace pub::debugging
#include <pub/Exception.h>          // For pub::Exception
#include <pub/utility.h>            // For pub::utilities
#include <pub/Wrapper.h>            // For pub::Wrapper

#include "pub/http/Ioda.h"          // For pub::http::Ioda, ... (Tested)

#define PUB _LIBPUB_NAMESPACE
using namespace PUB;
using namespace PUB::debugging;
using namespace PUB::http;
using PUB::utility::visify;

using std::string;

typedef PUB::http::Ioda   Ioda;
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
   for(int i= 0; i<LINES; ++i)
     from.put(line);
   if( opt_verbose )
     from.debug("from 24,000");
   string full= (string)from;
   assert( full.size() == 24'000 ); // (Total size 24,000)

   //-------------------------------------------------------------------------
   if( opt_verbose )
     debugf("\noperator +=\n");
   Ioda into;
   if( opt_verbose )
     into.debug("into");
   into += std::move(from);
   if( opt_verbose ) {
     from.debug("from 0");
     into.debug("into 24,000");
   }

   //-------------------------------------------------------------------------
   if( opt_verbose )
     debugf("\nIoda::get_mesg\n");
   Ioda read; Mesg mesg;
   read.get_rd_mesg(mesg, 20'000);
   if( opt_verbose )
     mesg.debug("rd_mesg 0x4e20");

   read.set_used(5'000);
   if( opt_verbose )
     debugf("..set_used should have deleted 3 rd_mesg buffers\n");
   read.get_wr_mesg(mesg);
   if( opt_verbose )
     mesg.debug("wr_mesg 0x1338");

   into.get_wr_mesg(mesg, 6'000);
   if( opt_verbose )
     mesg.debug("wr_mesg 0x1770");

enum { SIZES_DIM= 25 };
static const size_t sizes[SIZES_DIM]=
   { 0x00000 //  0
   , 0x00001 //  1
   , 0x00002 //  2
   , 0x00003 //  3
   , 0x00004 //  4
   , 0x00ffd //  5
   , 0x00ffe //  6
   , 0x00fff //  7
   , 0x01000 //  8
   , 0x01001 //  9
   , 0x01002 // 10
   , 0x01003 // 11
   , 0x01004 // 12
   , 0x01fff // 13 8191
   , 0x04ffd // 14
   , 0x04ffe // 15
   , 0x04fff // 16
   , 0x05000 // 17
   , 0x05001 // 18
   , 0x05002 // 19
   , 0x05003 // 20
   , 0x05004 // 21
   ,  23'999 // 22 0x05dbf
   ,  24'000 // 23 0x05dc0
   ,  24'001 // 24 0x05dc1
   };        // 25 SIZES_DIM

   //-------------------------------------------------------------------------
   if( opt_verbose )
     debugf("\nIoda::split\n");
   for(size_t sx= 0; sx < SIZES_DIM; ++sx) {
     size_t size= sizes[sx];
     if( opt_verbose > 1 )
       tracef("\nSIZE:(0x%.6zx) %'6zd\n", size, size);
     size_t page= size_t(-1);
     for(size_t px= 0; px < 8; ++px) {
       page= px * 4096;
       if( size < 4096 ) break;
       if( page >= (size-1) ) break;
       if( page > 0 )
       { Ioda tail; tail.put(full);
         if( opt_verbose > 1 )
           tracef("split(0x%.6zx) page-1\n", page-1);
         Ioda head; tail.split(head, page - 1);
         error_count += VERIFY( ((string)head + (string)tail) == full);
         if( error_count ) break;
       }
       if( page >= (size-0) ) break;
       { Ioda tail; tail.put(full);
         if( opt_verbose > 1 )
           tracef("split(0x%.6zx) page-0\n", page-0);
         Ioda head; tail.split(head, page - 0);
         error_count += VERIFY( ((string)head + (string)tail) == full);
         if( error_count ) break;
       }
       if( (page+1) == (size-1) ) break;
       { Ioda tail; tail.put(full);
         if( opt_verbose > 1 )
           tracef("split(0x%.6zx) page+1\n", page+1);
         Ioda head; tail.split(head, page + 1);
         error_count += VERIFY( ((string)head + (string)tail) == full);
         if( error_count ) break;
       }
     }

     if( opt_verbose > 1 )
       tracef(" page(0x%.6zx)\n", page);
     if( size <= 4095 && size > 1 )
     { Ioda tail; tail.put(full);
       if( opt_verbose > 1 )
         tracef("split(0x%.6zx) page+0\n", size_t(0));
       Ioda head; tail.split(head, 0);
       error_count += VERIFY( ((string)head + (string)tail) == full);
       if( error_count ) break;
     }
     if( size <= 4095 && size > 2 )
     { Ioda tail; tail.put(full);
       if( opt_verbose > 1 )
         tracef("split(0x%.6zx) page+1\n", size_t(1));
       Ioda head; tail.split(head, 1);
       error_count += VERIFY( ((string)head + (string)tail) == full);
       if( error_count ) break;
     }
     if( page > 0 && page == (size - 1) )
     { Ioda tail; tail.put(full);
       if( opt_verbose > 1 )
         tracef("split(0x%.6zx) page-1\n", page - 1);
       Ioda head; tail.split(head, page - 1);
       error_count += VERIFY( ((string)head + (string)tail) == full);
       if( error_count ) break;
     }
     if( size > 0 )
     { Ioda tail; tail.put(full);
       if( opt_verbose > 1 )
         tracef("split(0x%.6zx) SIZE-1\n", size - 1);
       Ioda head; tail.split(head, size - 1);
       error_count += VERIFY( ((string)head + (string)tail) == full);
       if( error_count ) break;
     }
     { Ioda tail; tail.put(full);
       if( opt_verbose > 1 )
         tracef("split(0x%.6zx) SIZE\n", size + 0);
       Ioda head; tail.split(head, size + 0);
       error_count += VERIFY( ((string)head + (string)tail) == full);
       if( error_count ) break;
     }
     { Ioda tail; tail.put(full);
       if( opt_verbose > 1 )
         tracef("split(0x%.6zx) SIZE+1\n", size + 1);
       Ioda head; tail.split(head, size + 1);
       error_count += VERIFY( ((string)head + (string)tail) == full);
       if( error_count ) break;
     }
     if( opt_verbose > 1 && size == 8191 ) {
       Ioda tail; tail.put(full);
       Ioda head; tail.split(head, size);
       tracef("\n\nVIEW %zd\nhead %zd\n{{{%s}}}\n\ntail %zd\n{{{%s}}}\n", size
             , ((string)head).size(), visify((string)head).c_str()
             , ((string)tail).size(), visify((string)tail).c_str());
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

   tc.on_info([]()
   {
     fprintf(stderr, "  --dirty\tRun dirty test\n");
     fprintf(stderr, "  --size\tRun object size test\n");
   });

   tc.on_main([tr](int, char**)
   {
     int error_count= 0;            // Error count

     try {
       if( opt_verbose )
         debugf("%s: %s %s\n", __FILE__, __DATE__, __TIME__);

       if( opt_size  ) error_count= test_size();
       if( opt_unit  ) error_count= test_unit();
       if( opt_dirty ) error_count += test_dirty();
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
