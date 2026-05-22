//----------------------------------------------------------------------------
//
//       Copyright (c) 2018-2026 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
// SPDX-License-Identifier: GPL-3.0-only
//----------------------------------------------------------------------------
//
// Title-
//       TestMisc.cpp
//
// Purpose-
//       Miscellaneous tests.
//
// Last change date-
//       2026/05/22
//
//----------------------------------------------------------------------------
#include <functional>               // For std::function
#include <string>                   // For std::string

#include <cassert>                  // For assert
#include <cstring>

#include <getopt.h>

#include <pub/Debug.h>
#include <pub/Exception.h>

// The tested includes
#include "pub/TEST.H"               // For VERIFY, ...
#include "pub/Hardware.h"           // For pub::Hardware
#include "pub/IO.h"                 // For pub::IO
#include "pub/Properties.h"         // For pub::Properties
#include "pub/Random.h"             // For pub::Random
#include "pub/Statistic.h"          // For pub::Statistic
#include "pub/System.h"             // For pub::System, tested
#include "pub/Tokenizer.h"          // For pub::Tokenizer
#include "pub/Wrapper.h"            // For pub::Wrapper

// Namespace accessors
#define PUB _LIBPUB_NAMESPACE
using namespace PUB::debugging;

// Class accessors
using Exception= PUB::Exception;
using IndexException= PUB::IndexException;
using PUB::Wrapper;                 // For pub::Wrapper class
using std::string;                  // For convenience

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum // Generic enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more
}; // Generic enum

//----------------------------------------------------------------------------
// Test_IO constant data
//----------------------------------------------------------------------------
const char             IO_data[]=   // Size includes embedded and trailing '\0'
   "This is the IO_data\n"
   "abc\0"
   "def\n";

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_Example
//
// Purpose-
//       Test (Example.h)
//
//----------------------------------------------------------------------------
static inline int                   // Number of errors encountered
   test_Example( void )             // Test Example.h
{
   int                 error_count= 0; // Number of errors encountered

   if( opt_verbose )
     debugf("\ntest_Example\n");

   return error_count;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_Hardware
//
// Purpose-
//       Test Hardware.h
//
// Implementation note-
//       Hardware.h is only (correctly) implemented for GNU x86
//
//----------------------------------------------------------------------------
#if defined(__GNUC__) && defined(_HW_X86)
static inline int                   // Number of errors encountered
   test_Hardware( void )            // Test Hardware.h
{
   int                 error_count= 0; // Number of errors encountered

   if( opt_verbose )
     debugf("\ntest_Hardware\n");

   intptr_t one= (intptr_t)pub::Hardware::getLR();
   intptr_t two= (intptr_t)pub::Hardware::getLR();
   error_count += VERIFY(two > one && (two-one) < 64);
   if( opt_verbose )
     debugf("one(0x%.16zx) two(0x%.16zx) getLR\n", one, two);

   one= (intptr_t)pub::Hardware::getSP();
   two= (intptr_t)pub::Hardware::getSP();
   error_count += VERIFY(two == one);
   if( opt_verbose )
     debugf("one(0x%.16zx) two(0x%.16zx) getSP\n", one, two);

   intptr_t max= 0;
   intptr_t min= 1'000'000'000'000;
   one= pub::Hardware::getTSC();
   intptr_t old= one;
   for(int i= 0; i<64; ++i) {
     two= pub::Hardware::getTSC();
     error_count += VERIFY(two >= one);
     intptr_t del= two - old;
     if( del < min )
       min= del;
     if( del > max )
       max= del;
     old= two;
   }
   error_count += VERIFY(two > one);
   if( opt_verbose )
     debugf("one(0x%.16zx) two(0x%.16zx) min(%zd) max(%zd) getTSC\n"
           , one, two, min, max);

   return error_count;
}
#else
static inline int                   // Number of errors encountered
   test_Hardware( void )            // Test Hardware.h
{
   if( opt_verbose )
     debugf("test_Hardware skipped: "
            "GNU compiler, x86 architecture required\n");

   return 0;
}
#endif

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_IO
//
// Purpose-
//       Test IO.h
//
//----------------------------------------------------------------------------
static inline int                   // Error count
   IO_test(                         // Test IO.h
     string            IO_path,     // For this path
     string            IO_file,     // And this file
     string            IO_link,     // And this link
     mode_t            path_mode,   // And this path mode
     mode_t            file_mode,   // And this file mode
     int               file_type)   // And this file open type
{
   int error_count= 0;

   using namespace PUB::io;

   char buffer[256];                // Big (enough) buffer
   string IO_full(IO_path + "/" + IO_file);
   mode_t mode_mask= ACCESSPERMS;
   mode_t path_mode_chmod= path_mode | S_IROTH | S_IXOTH;
   struct stat info;

   //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   // Verify open, close, read, write, and mkpath
   int rc= mkpath(IO_path, path_mode);
   error_count += VERIFY( rc == 0);
   error_if(rc != 0, "%d= mkpath(%s,%.4o)", rc, IO_path.c_str(), path_mode);

   fd_t fd= open(IO_full, file_type, file_mode);
   error_count += VERIFY( fd >= 0);

   FILE* file= fd_to_FILE(fd, "w");
   fd_t fd_dup= FILE_to_fd(file);
   error_count += VERIFY( fd == fd_dup);

   ssize_t size= write(fd, IO_data, sizeof(IO_data));
   error_count += VERIFY( size == sizeof(IO_data) );

   rc= close(fd);
   error_count += VERIFY( rc == 0);

   fd= open(IO_full, O_RDONLY);
   error_count += VERIFY( fd >= 0);

   size= read(fd, buffer, sizeof(buffer));
   error_count += VERIFY( size == sizeof(IO_data) );

   size= read(fd, buffer, sizeof(buffer));
   error_count += VERIFY( size == 0 );

   rc= close(fd);
   error_count += VERIFY( rc == 0);
   error_count += VERIFY( memcmp(IO_data, buffer, sizeof(IO_data)) == 0 );

   //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   // Verify path creation mode and chmod
   rc= stat(IO_path, &info);
   info.st_mode &= mode_mask;
   error_count += VERIFY( rc == 0);
   error_count += VERIFY( info.st_mode == path_mode );
   if( opt_verbose )
     debugf("Test_IO: %s: current_mode(%.4o) create_mode(%.4o)\n"
           , IO_path.c_str(), info.st_mode, path_mode);

   rc= chmod(IO_path, path_mode_chmod);
   error_count += VERIFY( rc == 0);
   rc= lstat(IO_path, &info);
   info.st_mode &= mode_mask;
   error_count += VERIFY( rc == 0);
   error_count += VERIFY( info.st_mode == path_mode_chmod );
   if( opt_verbose )
     debugf("Test_IO: %s: current_mode(%.4o) changed_mode(%.4o)\n"
           , IO_path.c_str(), info.st_mode, path_mode_chmod);

   //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   // Create, verify, and remove link
   rc= mklink(IO_full, IO_link);
   error_count += VERIFY( rc == 0);

   size= rdlink(IO_link, buffer, sizeof(buffer));
   error_count += VERIFY( (size_t)size == IO_full.size() );
   error_count += VERIFY( string(buffer) == IO_full );
   if( opt_verbose )
     debugf("Test_IO: '%s'= rdlink()\n", buffer);

   rc= rmlink(IO_link);
   error_count += VERIFY( rc == 0);

   //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   // Verify rmlink (correctly) fails if not removing a link
   rc= rmlink(IO_full);
   error_count += VERIFY( rc == -1);
   error_count += VERIFY( errno == EINVAL );
   if( opt_verbose )
     debugf("Test_IO: %d= rmlink(%s) %d:%s\n", rc, IO_full.c_str()
           , errno, strerror(errno));

   //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   // Verify rmfile (correctly) fails if removing a path
   rc= rmfile(IO_path);
   error_count += VERIFY( rc == -1);
   error_count += VERIFY( errno == EISDIR );
   if( opt_verbose )
     debugf("Test_IO: %d= rmfile(%s) %d:%s\n", rc, IO_path.c_str()
           , errno, strerror(errno));

   //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   // Verify get_cwd and set_cwd
   if( IO_path[0] != '/' ) {        // If relative IO_path
     string old_cwd= get_cwd();
     if( opt_verbose )
       debugf("\nTest_IO: old_cwd(%s)\n", old_cwd.c_str());

     rc= set_cwd(IO_path);
     error_count += VERIFY( rc == 0);
     if( opt_verbose )
       debugf("Test_IO: %s= set_cwd(%s)\n", get_cwd().c_str(), IO_path.c_str());

     rc= stat(IO_file, &info);
     info.st_mode &= mode_mask;
     error_count += VERIFY( rc == 0);
     error_count += VERIFY( info.st_mode == file_mode );
     if( opt_verbose )
       debugf("Test_IO: %s: current_mode(%.4o) create_mode(%.4o)\n"
             , IO_full.c_str(), info.st_mode, file_mode);

     rc= set_cwd("..");
     error_count += VERIFY( rc == 0);

     string new_cwd= get_cwd();
     error_count += VERIFY( new_cwd == old_cwd );
     if( opt_verbose )
       debugf("Test_IO: new_cwd(%s)\n", new_cwd.c_str());
   }

   //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   // Cleanup
   rc= rmfile(IO_full);
   error_count += VERIFY( rc == 0);

   rc= rmpath(IO_path);
   error_count += VERIFY( rc == 0);
   error_if(rc != 0, "Test_IO: %d= rmpath(%s)", rc, IO_path.c_str());

   return error_count;
}

static inline int                   // Error count
   test_IO( void )                  // Test IO.h
{
   if( opt_verbose )
     debugf("\ntest_IO\n");

   int error_count= 0;

   using namespace PUB::io;

   string IO_path("Test_IO");
   string IO_file("IO_file");
   string IO_full(IO_path + "/" + IO_file);
   string IO_link("IO_link");
   mode_t file_mode= S_IRWXU;
   int    file_type= O_WRONLY | O_CREAT | O_TRUNC;
   mode_t path_mode= S_IRWXU | S_IRGRP | S_IXGRP;

   try {
     //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
     // Test relative path
     error_count +=
     IO_test(IO_path, IO_file, IO_link, path_mode, file_mode, file_type);

     //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
     // Test absolute path
     IO_path= "/tmp/" + IO_path;
     if( opt_verbose )
       debugf("\n");
     error_count +=
     IO_test(IO_path, IO_file, IO_link, path_mode, file_mode, file_type);
   } catch(std::exception& X) {
     ++error_count;
     debugf("Test_IO catch(std::exception) what(%s)\n", X.what());

     rmfile(IO_full);
     rmpath(IO_path);
   }

   //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   // Test error_if subroutine
   if( opt_verbose )
     debugf("\n");
   bool caught= false;
   try {
     errno= EAGAIN;
     error_if(true, "Expected exception (%s)", "EAGAIN");
     error_count += VERIFY( "exception not thrown" == nullptr );
   } catch(io_error& X) {
     caught= true;
     if( opt_verbose )
       debugf("Test_IO: Expected io_error caught\n..what(%s)\n", X.what());
   }
   error_count += VERIFY( caught );

   if( opt_verbose )
     debug_flush();

   return error_count;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_Properties
//
// Purpose-
//       Test Properties.
//
//----------------------------------------------------------------------------
static inline int
   test_Properties( void )          // Test Properties
{
   int error_count= 0;

   if( opt_verbose )
     debugf("\ntest_Properties\n");

   using _LIBPUB_NAMESPACE::Properties;
   typedef Properties::MapIter_t MapIter_t; // Map iterator type
   Properties props;                // Our test object

   std::string s;

   s= "yY";
   props.insert("yY", "yar");
   if( opt_verbose ) {
     debugf("%s: %s\n", s.c_str(), props[ s ]);
     debugf("%s: %s\n", "yY"     , props["yY"]);
     debugf("%s: %s\n", "Yy"     , props.get_property("Yy"));
   }
   error_count += VERIFY(strcmp(props[ s ], "yar") == 0);
   error_count += VERIFY(strcmp(props["yY"], "yar") == 0);
   error_count += VERIFY(strcmp(props.get_property("Yy"), "yar") == 0);

   s= "Nn";
   props.insert("Nn", "nar");
   error_count += VERIFY(strcmp(props[ s ], "nar") == 0);
   error_count += VERIFY(strcmp(props["Nn"], "nar") == 0);
   error_count += VERIFY(strcmp(props.get_property("nN"), "nar") == 0);

   s= "W";
   props.insert(s, "wasp");
   error_count += VERIFY(strcmp(props[ s ], "wasp") == 0);
   error_count += VERIFY(strcmp(props["W"], "wasp") == 0);
   error_count += VERIFY(strcmp(props.get_property("w"), "wasp") == 0);

   error_count += VERIFY(strcmp(props.get_property("Foo", "bar"), "bar") == 0);
   props.insert("foo", "bart s");
   error_count += VERIFY(strcmp(props.get_property("Foo", "bar"), "bart s") == 0);
   props.remove("foo");
   error_count += VERIFY(props.get_property("foo") == nullptr);

   if( opt_verbose ) {
     debugf("\nProperties:\n");
     for(MapIter_t mi= props.begin(); mi != props.end(); mi++) {
       debugf("%s: '%s'\n", mi->first.c_str(), mi->second.c_str());
     }
   }

   //-------------------------------------------------------------------------
   // Verify IndexException raised where expected
   try {
     props.insert("Yy", "yard");
     errorf("%4d Missing IndexException\n", __LINE__);
     error_count++;
   } catch(IndexException& X) {
     if( opt_verbose)
       debugf("%4d Expected IndexException caught: %s\n", __LINE__,
             std::string(X).c_str());
   } catch(std::exception& X) {
     errorf("%4d Wrong exception type(%s)\n", __LINE__, X.what());
     error_count++;
   } catch(...) {
     errorf("%4d Wrong exception type(%s)\n", __LINE__, "...");
     error_count++;
   }

   try {
     props["foo"];
     errorf("%4d Missing IndexException\n", __LINE__);
     error_count++;
   } catch(IndexException& X) {
     if( opt_verbose)
       debugf("%4d Expected IndexException caught: %s\n", __LINE__,
             std::string(X).c_str());
   } catch(std::exception& X) {
     errorf("%4d Wrong exception type(%s)\n", __LINE__, X.what());
     error_count++;
   } catch(...) {
     errorf("%4d Wrong exception type(%s)\n", __LINE__, "...");
     error_count++;
   }

   try {
     props.remove("foo");
     errorf("%4d Missing IndexException\n", __LINE__);
     error_count++;
   } catch(IndexException& X) {
     if( opt_verbose)
       debugf("%4d Expected IndexException caught: %s\n", __LINE__,
             std::string(X).c_str());
   } catch(std::exception& X) {
     errorf("%4d Wrong exception type(%s)\n", __LINE__, X.what());
     error_count++;
   } catch(...) {
     errorf("%4d Wrong exception type(%s)\n", __LINE__, "...");
     error_count++;
   }

   //-------------------------------------------------------------------------
   // Verify Properties.reset() method
   props.reset();
   error_count += VERIFY(props.get_property("S") == nullptr);
   error_count += VERIFY(props.begin() == props.end());

   return error_count;
}

//----------------------------------------------------------------------------
//
// Macro-
//       verify_info
//
//----------------------------------------------------------------------------
#define verify_info debugf("\n%4d %s: ", __LINE__, __FILE__)

//----------------------------------------------------------------------------
//
// Subroutine-
//       bit_counter
//
// Purpose-
//       For each bit in a word, count its occurrence in a counter array.
//
//----------------------------------------------------------------------------
static inline void
   bit_counter(                     // Count bit occurrences.
     uint64_t          word,        // In this word
     uint64_t*         array)       // Counter array[64]
{
   uint64_t mask= 1;
   for(int i= 0; i<64; i++) {
     if( (word&mask) != 0 )
       array[i]++;

     mask <<= 1;
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       bit_checker (Used by Random::_self_test)
//
// Purpose-
//       Display the number of occurrences for each bit.
//
//----------------------------------------------------------------------------
static inline void
   bit_checker(                     // Check bit occurrences.
     const char*       type,        // The function used to set the bits
     size_t            count,       // The number of tests
     uint64_t*         array)       // Counter array[64]
{
   size_t minCount= count/2 - count/512;
   size_t maxCount= count/2 + count/512;
   debugf("bit_checker(%s) {%'zd; %'zd; %'zd}\n", type
         , minCount, count/2, maxCount);

   for(int i= 0; i<63; i++) {
     int x= 62 - i;
     size_t ones= array[x];
     debugf("[%2d] %'8zd of %'8zd ", x, ones, count);
     if( ones >= minCount && ones <= maxCount )
       debugf("OK\n");
     else
       debugf("!! NG !!\n");
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_Random
//
// Purpose-
//       Test Random.h
//
//----------------------------------------------------------------------------
static inline int                   // Number of errors found
   test_Random( void )              // Test Random.h
{
static constexpr int   DIM_ARRAY= 64;

static constexpr size_t ITERATIONS=      // Range:    10,000,000 and up
                                               size_t(50'000'000);

static constexpr size_t CHECK_ITERATIONS= // Range: 1,000,000,000 and up
                                             size_t(3'000'000'000);

   if( opt_verbose )
     debugf("\ntest_Random (long-running)\n");

   int error_count= 0;

   PUB::Random& RNG= PUB::Random::standard;

   //-----------------------------------------------------------------------
   // Quick test for duplicates
   uint64_t            array[DIM_ARRAY]; // Random value array
   size_t              count;     // Iteration/result counter

   for(int i=0; i<DIM_ARRAY; i++)
     array[i]= RNG.get64();

   for(int i=0; i<DIM_ARRAY; i++) {
     for(int j=i+1; j<DIM_ARRAY; j++) {
       if( array[i] == array[j] ) {
         debugf("Random::get64() repeats [%d]==[%d]\n", i, j);
         for(int x= 0; x<DIM_ARRAY; ++x) {
           debugf("[%'5d]: %'zu\n", x, array[x]);
         }
         return 1;                // Quick loop detected
       }
     }
   }

   //-----------------------------------------------------------------------
   // Test for duplicates. Duplicates will repeat sequence
   uint64_t checker= array[DIM_ARRAY-1];
   if( opt_verbose )
     debugf("Pass 1\n");
   for(uint64_t i= 1; i <= CHECK_ITERATIONS; ++i) {
     if( RNG.get64() == checker ) { // Oh no! Algorithm failure
       debugf("Random::get64() repeats: %'zd loops, value %'zu\n", i, checker);
       return ++error_count;        // Slow loop detected
     }
     if( opt_verbose && (i % 1'000'000'000) == 0 )
       printf("Iteration %'16zd of %'16zd\n", i, CHECK_ITERATIONS);
   }

   checker= RNG.get64();            // We might have skidded into a loop
   if( opt_verbose )
     debugf("Pass 2\n");
   for(uint64_t i= 1; i <= CHECK_ITERATIONS; ++i) {
     if( RNG.get64() == checker ) { // Oh no! Algorithm failure
       debugf("Random::get64() repeats: %'zd loops, value %'zu\n", i, checker);
       return ++error_count;        // Slow loop detected
     }
     if( opt_verbose && (i % 1'000'000'000) == 0 )
       printf("Iteration %'16zd of %'16zd\n", i, CHECK_ITERATIONS);
   }
   if( opt_verbose )
     debugf("No duplicate found in %'zu iterations\n", 2 * CHECK_ITERATIONS);

   //-----------------------------------------------------------------------
   // Distribution tests
   if( opt_verbose && DIM_ARRAY >= 64 ) {
     // Testing get
     int prior[64];               // Prior bit value
     int  cur0[64];               // Sequentially zero (current sequence)
     int  cur1[64];               // Sequentially ones (current sequence)
     int  max0[64];               // Sequentially zero (longest sequence)
     int  max1[64];               // Sequentially ones (longest sequence)
     int  seq0[64];               // Sequentially zero
     int  seq1[64];               // Sequentially ones

     for(int i=0; i<64; i++) {
       prior[i]= (-1);            // Neither one nor zero
       cur0[i]= 0;
       cur1[i]= 0;
       max0[i]= 0;
       max1[i]= 0;
       seq0[i]= 0;
       seq1[i]= 0;
     }

     for(int i=0; i<DIM_ARRAY; i++)   // For bitCounter
       array[i]= 0;

     for(count= 0; count<ITERATIONS; count++) {
       uint64_t temp= RNG.get64();

       uint64_t mask= 1;
       for(int i= 0; i<64; i++) {
         if( (temp & mask) == 0 ) { // If bitvalue zero
           if( prior[i] == 0 ) {
             cur0[i]++;
             if( cur0[i] > max0[i] )
               max0[i]= cur0[i];

             seq0[i]++;
           } else {
             prior[i]= 0;
             cur0[i]= 0;
             cur1[i]= 1;
           }
         } else {
           if( prior[i] == 1 ) {
             cur1[i]++;
             if( cur1[i] > max1[i] )
               max1[i]= cur1[i];

             seq1[i]++;
           } else {
             prior[i]= 1;
             cur0[i]= 1;
             cur1[i]= 0;
           }
         }

         mask <<= 1;
       }

       bit_counter(temp, array);
     }
     verify_info; bit_checker("get", count, array);

     // Testing randomize
     debugf("\n BIT         Seq0    :    Seq1 Max0 Max1 ITERATIONS(%'zd)\n"
           , ITERATIONS);
     for(int i= 0; i<63; i++) {
       int x= 62 - i;
       debugf("[%2d] %'12d %'12d %'4d %'4d\n", x
             , seq0[x], seq1[x], max0[x], max1[x]);
     }

     for(int i=0; i<64; i++)      // For bitCounter
       array[i]= 0;

     for(count= 0; count<ITERATIONS; count++) {
       RNG.randomize();
       uint64_t temp= RNG.get64();
       bit_counter(temp, array);
     }
     verify_info; bit_checker("randomize", count, array);
   }

   return error_count;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_Statistic
//
// Purpose-
//       Test Statistic.
//
//----------------------------------------------------------------------------
static inline int
   test_Statistic( void )           // Test Statistic
{
   int error_count= 0;

   if( opt_verbose )
     debugf("\ntest_Statistic\n");

   PUB::statistic::Active stat;
   error_count += VERIFY(stat.inc() == 1 );
   error_count += VERIFY(stat.inc() == 2 );
   error_count += VERIFY(stat.inc() == 3 );
   error_count += VERIFY(stat.inc() == 4 );
   error_count += VERIFY(stat.inc() == 5 );
   error_count += VERIFY(stat.dec() == 4 );
   error_count += VERIFY(stat.dec() == 3 );
   error_count += VERIFY(stat.dec() == 2 );
   error_count += VERIFY(stat.inc() == 3 );

   error_count += VERIFY(stat.counter.load() == 6 );
   error_count += VERIFY(stat.current.load() == 3 );
   error_count += VERIFY(stat.maximum.load() == 5 );
   error_count += VERIFY(stat.minimum.load() == 2 );

   if( opt_verbose ) {
     printf("stat: %ld  %ld,%ld,%ld\n", stat.counter.load()
           , stat.minimum.load() , stat.current.load(), stat.maximum.load());
   }

   return error_count;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_System
//
// Purpose-
//       Test System.h
//
//----------------------------------------------------------------------------
static inline int                   // Number of errors encountered
   test_System( void )              // Test System.h, System::log
{
   using namespace PUB::System;

   int                 error_count= 0; // Number of errors encountered

   if( opt_verbose )
     debugf("\ntest_System\n");

   //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   // Test System::log
   if( opt_verbose ) {
     set_log_level(LL_HCDM);           // (Log HCDM only)
     error_count += VERIFY(get_log_level() == LL_HCDM);

     log(LL_NONE,  "\n");           // (syslog not stderr)
     log(LL_NONE,  "%s LL_NONE  (-stderr)\n", __FILE__); // (syslog not stderr)
     log(LL_INFO,  "%s LL_INFO  (-stderr)\n", __FILE__); // (syslog not stderr)
     log(LL_ERROR, "%s LL_ERROR (-stderr)\n", __FILE__); // (syslog not stderr)
     log(LL_HCDM,  "%s LL_HCDM  (+stderr)\n", __FILE__); // (syslog and stderr)
     log(LL_ALL,   "%s LL_ALL   (+stderr)\n", __FILE__); // (syslog and stderr)
   }

   //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   // Test System::get_LR, System::get_SP, System::get_TSC
#if defined(__GNUC__) && defined(_HW_X86) // GNU compiler, x86 required
   intptr_t one= (intptr_t)get_LR();
   intptr_t two= (intptr_t)get_LR();
   error_count += VERIFY(two > one && (two-one) < 64);
   if( opt_verbose )
     debugf("one(0x%.16zx) two(0x%.16zx) get_LR\n", one, two);

   one= (intptr_t)get_SP();
   two= (intptr_t)get_SP();
   error_count += VERIFY(two == one);
   if( opt_verbose )
     debugf("one(0x%.16zx) two(0x%.16zx) get_SP\n", one, two);

   intptr_t max= 0;
   intptr_t min= 1'000'000'000'000;
   one= get_TSC();
   intptr_t old= one;
   for(int i= 0; i<64; ++i) {
     two= get_TSC();
     error_count += VERIFY(two >= one);
     intptr_t del= two - old;
     if( del < min )
       min= del;
     if( del > max )
       max= del;
     old= two;
   }

   error_count += VERIFY(two > one);
   if( opt_verbose )
     debugf("one(0x%.16zx) two(0x%.16zx) min(%zd) max(%zd) get_TSC\n"
           , one, two, min, max);
#else
   ++error_count;
   debugf("System::get_LR, System::get_SP, and System::get_TSC\n"
          "require GNU compiler and x86 hardware\n");
#endif

   return error_count;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       show_Tokenizer
//
// Purpose-
//       Display a Tokenizer.
//
//----------------------------------------------------------------------------
static inline void
   show_Tokenizer(                  // Display
     const PUB::Tokenizer&  tokenizer) // This Tokenizer
{
   typedef PUB::Tokenizer           Tokenizer;
   typedef PUB::Tokenizer::Iterator Iterator;

   Tokenizer izer(tokenizer);

   if( opt_verbose ) {
     printf("\n");
     for(Iterator it= izer.begin(); it != izer.end(); ++it)
       printf("'%s'\n", it().c_str());
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_Tokenizer
//
// Purpose-
//       Test Tokenizer.
//
//----------------------------------------------------------------------------
static inline int
   test_Tokenizer( void )           // Test Tokenizer
{
   int error_count= 0;

   if( opt_verbose )
     debugf("\ntest_Tokenizer\n");

   using _LIBPUB_NAMESPACE::Tokenizer;
   typedef _LIBPUB_NAMESPACE::Tokenizer::Iterator Iterator;
   Tokenizer izer(" a  b  c  def g "); // Our test object
   Iterator it= izer.begin();
   error_count += VERIFY( it != izer.end() );
   error_count += VERIFY( it() == "a" );
   error_count += VERIFY( (it++)() == "a" );
   error_count += VERIFY( (++it)() == "c" );
   error_count += VERIFY( (++it)() == "def" );
   error_count += VERIFY( it != izer.end() );
   error_count += VERIFY( (++it)() == "g" );
   error_count += VERIFY( ++it == izer.end() );
   error_count += VERIFY( ++it == izer.end() );
   error_count += VERIFY( it() == "" );
   show_Tokenizer(izer);

   izer.reset(" alpha ' beta gamma ' delta");
   it= izer.begin();
   error_count += VERIFY( it() == "alpha" );
   error_count += VERIFY( (++it)() == " beta gamma " );
   error_count += VERIFY( (++it)() == "delta" );
   error_count += VERIFY( (++it)   == izer.end() );
   show_Tokenizer(izer);

   it= izer.begin();
   it.set_quote(false);
   error_count += VERIFY( it() == "alpha" );
   error_count += VERIFY( (++it)() == "\'" );
   error_count += VERIFY( (++it)() == "beta" );
   error_count += VERIFY( (++it)() == "gamma" );
   error_count += VERIFY( (++it)() == "\'" );
   error_count += VERIFY( (++it)() == "delta" );
   error_count += VERIFY( (++it)   == izer.end() );
   show_Tokenizer(izer);

   Tokenizer date(" 08/09/2025 17:50 ", " /:=");
   it= date.begin();
   error_count += VERIFY( it() == "08" );
   error_count += VERIFY( (++it)() == "09" );
   error_count += VERIFY( (++it)() == "2025" );
   error_count += VERIFY( (++it)() == "17" );
   error_count += VERIFY( (++it)() == "50" );
   error_count += VERIFY( (++it)   == date.end() );
   show_Tokenizer(date);

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
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   //-------------------------------------------------------------------------
   // Initialize
   Wrapper  tc;                     // The test case wrapper
   Wrapper* tr= &tc;                // A test case wrapper pointer

   tc.on_main([tr](int, char*[])
   {
     int error_count= 0;

     setlocale(LC_NUMERIC, "");        // Allows printf("%'d\n", 123456789);
     error_count += test_Hardware();   // Test Hardware.h
     error_count += test_IO();         // Test IO.h
     error_count += test_Properties(); // Test Properties.h
     error_count += test_Random();     // Test Random.h
     error_count += test_Statistic();  // Test Statistic.h
     error_count += test_System();     // Test System.h
     error_count += test_Tokenizer();  // Test Tokenizer.h

     if( error_count || opt_verbose ) {
       debugf("\n");
       tr->report_errors(error_count);
     }
     return error_count != 0;
   });

   //-------------------------------------------------------------------------
   // Run the test
   return tc.run(argc, argv);
}
