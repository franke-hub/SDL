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
//       Dirty.cpp
//
// Purpose-
//       Some Quick and Dirty tests.
//
// Last change date-
//       2020/10/16
//
//----------------------------------------------------------------------------
#include <chrono>                   // We include everything and then some
#include <functional>
#include <iostream>
#include <list>
#include <memory>
#include <mutex>
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
#include <xcb/xcb.h>
#include <xcb/xproto.h>
#include <X11/Xlib.h>
#include <X11/Xlib-xcb.h>

#include "pub/Allocator.h"
#include "pub/Debug.h"
#include "pub/Object.h"
#include "pub/Latch.h"
#include "pub/Exception.h"
#include "pub/Latch.h"
#include "pub/memory.h"
#include "pub/Trace.h"
#include "pub/Worker.h"
#include "pub/utility.h"

#include "Xcb/Signal.h"

using namespace pub;
using namespace pub::utility;
using namespace pub::debugging;

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_Dirty
//
// Purpose-
//       The proverbial Quick and Dirty test.
//
//----------------------------------------------------------------------------
static inline int                   // Number of errors encountered
   test_Dirty( void )               // Today's Quick and Dirty test
{
   int errorCount= 0;               // Number of errors encountered

   debugf("\ntest_Dirty\n");

   return errorCount;
}

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
   int errorCount= 0;               // Number of errors encountered

   debugf("\ntest_Example\n");

   return errorCount;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_Malloc
//
// Purpose-
//       Test minimum allocation difference.
//
//----------------------------------------------------------------------------
static inline int                   // Number of errors encountered
   test_Malloc( void )              // Test malloc allocation granule
{
   int errorCount= 0;               // Number of errors encountered

   debugf("\ntest_Malloc\n");

enum { SIZE= 64 };                  // Size of allocated area

   size_t min_diff= 4096;
   char* old= (char*)malloc(SIZE);
   for(unsigned i= 0; i<8192; i++) {
     char* next= (char*)malloc(SIZE);
     if( i < 8 ) debugf("%p= malloc(0x%.4x)\n", next, int(SIZE));
     size_t diff= next-old;
     if( old > next )
       diff= old-next;
     if( diff < min_diff )
       min_diff= diff;
     old= next;
   }

   debugf("%8zd malloc overhead\n", min_diff - SIZE);

   return errorCount;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_Sizes
//
// Purpose-
//       Test sizeof selected objects
//
//----------------------------------------------------------------------------
struct Foo { char foo[17]; };
struct Bar { char bar[29]; };

struct Op_v {
void operator()(void) { printf("Op_v"); }
};

struct Op_i {
int operator()(void) { printf("Op_i"); return 27; }
};

struct Op_0 {
int operator()(const Foo& a) { printf("Op_0(%s)", a.foo); return 32; }
};

struct Op_1 {
int operator()(Foo& a) { strcpy(a.foo, "Op_1"); return 41; }
};

struct Op_2 {
int operator()(Foo& a, Bar& b) { strcpy(a.foo, "Op_2"); strcpy(b.bar, "Op_2"); return 42; }
};

static inline int                   // Number of errors encountered
   test_Sizes( void )               // Test sizeof selected objects
{
   int errorCount= 0;               // Number of errors encountered

   typedef std::function<void(void)>      Fn_v; // The function signatures
// typedef std::function<int(void)>       Fn_i;
// typedef std::function<int(const Foo&)> Fn_0;
// typedef std::function<int(Foo&)>       Fn_1;
//         std::function<int(Foo&,Bar&)>  Fn_2;

   debugf("\ntest_Sizes\n");

   // Pointer types
   debugf("%8zd sizeof(void*)\n",      sizeof(void*));
   debugf("%8zd sizeof(shared_ptr)\n", sizeof(std::shared_ptr<Foo>));
   debugf("%8zd sizeof(unique_ptr)\n", sizeof(std::unique_ptr<Foo>));
   debugf("%8zd sizeof(weak_ptr)\n",   sizeof(std::weak_ptr<Foo>));

   // Operator/Function types (Duplicates commented out)
   debugf("\n");
   debugf("%8zd sizeof(Op_v)\n",       sizeof(Op_v));
// debugf("%8zd sizeof(Op_i)\n",       sizeof(Op_i));
// debugf("%8zd sizeof(Op_0)\n",       sizeof(Op_0));
// debugf("%8zd sizeof(Op_1)\n",       sizeof(Op_1));
// debugf("%8zd sizeof(Op_2)\n",       sizeof(Op_2));
// debugf("\n");
   debugf("%8zd sizeof(Fn_v)\n",       sizeof(Fn_v));
// debugf("%8zd sizeof(Fn_i)\n",       sizeof(Fn_i));
// debugf("%8zd sizeof(Fn_0)\n",       sizeof(Fn_0));
// debugf("%8zd sizeof(Fn_1)\n",       sizeof(Fn_1));
// debugf("%8zd sizeof(Fn_2)\n",       sizeof(Fn_2));

   // Library objects
   debugf("\n");
   debugf("%8zd sizeof(pub::Latch)\n", sizeof(pub::Latch));

   // System objects
   debugf("\n");
   debugf("%8zd sizeof(std::mutex)\n", sizeof(std::mutex));

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
     debugf("Dirty.cpp\n");

     errorCount += test_Dirty();
     errorCount += test_Malloc();
     errorCount += test_Sizes();
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

   debugf("\n");
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

