//----------------------------------------------------------------------------
//
//       Copyright (c) 2019-2024 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Include.cpp
//
// Purpose-
//       Include test
//
// Last change date-
//       2024/10/10
//
//----------------------------------------------------------------------------
#include <assert.h>                 // For assert()

#include "Command.h"                // (Compile test only)
#include "Service.h"                // (Compile test only)

#include "pub_types.h"              // Test include
#include "std_types.h"              // Test include

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose
}; // (generic) enum

//----------------------------------------------------------------------------
//
// Class-
//       Include_task
//
// Purpose-
//       Test pub::Dispatch inclusion
//
//----------------------------------------------------------------------------
class Include_task : public Task {  // A pub::dispatch::Task
public:
virtual void
   work(Item* item)
{  item->post(123456); }
}; // class Include_task

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
   main(int, char**)                // Mainline code
//   int               argc,        // Argument count (UNUSED)
//   char*             argv[])      // Argument array (UNUSED)
{
   //-------------------------------------------------------------------------
   // Test pub_types.h
   //-------------------------------------------------------------------------
   {{{{                             // Test pub::debugging subroutines
     debugf("\n");
     debugf("pub::debugging subroutines compile OK\n");
   }}}}

   {{{{                             // Test pub::dispatch objects
     debugf("\n");
     Include_task task;
     Wait wait;
     Item item(&wait);
     task.enqueue(&item);
     int32_t rc= wait.wait();
     if( rc != 123456 )
       debugf("pub::dispatch INVALID RESULT(%d)\n", rc);
     debugf("pub::dispatch objects compile OK\n");
   }}}}

   {{{{                             // Test pub::Object
     debugf("\n");
     Object object;                 // (Don't have to use it.)
     debugf("pub::Object compiles OK\n");
   }}}}

   {{{{                             // Test pub::utility subroutines
     debugf("\n");
     nop();                         // utility::nop
     string S= to_string("pub::utility subroutines compile %s", "OK");
     string T= visify(S);
     debugf("%s\n", T.c_str());
   }}}}

   //-------------------------------------------------------------------------
   // Test std_types.h
   //-------------------------------------------------------------------------
   {{{{                             // Test std_type.h
     debugf("\n");
     debugf("shared_ptr<Object> sp1(new Object());\n");
     shared_ptr<Object> sp1(new Object());
     if( !sp1 )
       throw bad_alloc();
     debugf("'%s'= sp1->to_string();\n\n", sp1->to_string().c_str());

     debugf("shared_ptr<Object> sp2= make_shared<Object>();\n");
     shared_ptr<Object> sp2= make_shared<Object>();
     if( !sp2 )
       throw bad_alloc();
     debugf("'%s'= sp2->to_string();\n", sp2->to_string().c_str());

     debugf("std_type.h compiles OK\n");
   }}}}

   return 0;
}
