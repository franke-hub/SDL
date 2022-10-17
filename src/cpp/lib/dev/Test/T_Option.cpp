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
//       T_Option.cpp
//
// Purpose-
//       Test http::Options.h
//
// Last change date-
//       2022/09/02
//
//----------------------------------------------------------------------------
#include <string>                   // For std::string

#include <pub/Debug.h>              // For debugging classes and functions
#include <pub/TEST.H>               // For VERIFY macro

#include "pub/http/Options.h"       // For pub::http::Options, tested

#define PUB _LIBPUB_NAMESPACE
using namespace PUB;                // For pub classes
using namespace PUB::debugging;     // For pub debugging functions
using namespace PUB::http;          // For pub::http classes
using namespace std;                // For std classes

typedef Options::const_iterator const_iterator;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 1                       // Verbosity: Higher is more verbose
};

//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------
#define TRY_CATCH(THIS) \
{{{{ bool caught= false; \
     try { THIS; } \
     catch(std::exception& X) { caught= true; debugf("catch(%s)\n", #THIS); } \
     VERIFY( caught ); \
}}}}

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
   main( void )                     // Mainline code
//   int               argc,        // Argument count **Unused**
//   char*             argv[])      // Argument array **Unused**
{
   int                 error_count= 0; // Error counter

   //-------------------------------------------------------------------------
   // Run tests
   //-------------------------------------------------------------------------
   Options opts;
   error_count += VERIFY( !opts.insert("This", "The THIS value") );
   error_count += VERIFY( !opts.insert("That", "The THAT value") );
   error_count += VERIFY( !opts.insert("Other", "The OTHER value") );
   error_count += VERIFY(  opts.insert("This", "The THIS replacement value") );

   opts.begin()++;
   opts.end()++;

   debugf("\niterator++\n");
   for(const_iterator it= opts.begin(); it != opts.end(); it++) {
     debugf("%s: '%s'\n", it->first.c_str(), it->second.c_str());
   }

   debugf("\n++iterator\n");
   for(const_iterator it= opts.begin(); it != opts.end(); ++it) {
     debugf("%s: '%s'\n", it->first.c_str(), it->second.c_str());
   }

   debugf("\nlocate, operator[]\n");
   debugf("this: '%s'\n", opts.locate("this"));
   debugf("THIS: '%s'\n", opts.locate("THIS"));
   debugf("that: '%s'\n", opts.locate("that"));
   debugf("nada: '%s'\n", opts["nada"].c_str());
   debugf("other: '%s'\n", opts.locate("other"));
   error_count += VERIFY(opts["this"] == "The THIS replacement value" );
   error_count += VERIFY(opts["THIS"] == "The THIS replacement value" );
   error_count += VERIFY(opts["that"] == "The THAT value" );
   error_count += VERIFY(opts["other"] == "The OTHER value" );
   error_count += VERIFY(opts["nada"] == "");

   debugf("\nremove\n");
   error_count += VERIFY(  opts.remove("nada") );
   error_count += VERIFY( !opts.remove("nada") );
   debugf("nada: '%s'\n", opts.locate("nada"));

   debugf("\nExceptions, end() handling:\n");
   const_iterator xx= opts.end();
   xx++; xx++; xx++; xx++;
   error_count += VERIFY( xx == opts.end() );
// try {
// } catch(std::exception& X) {
// }
   TRY_CATCH( ++xx )
   TRY_CATCH( ++opts.end() )

   //-------------------------------------------------------------------------
   // Testing complete
   //-------------------------------------------------------------------------
   if( error_count == 0 )
     debugf("NO errors detected\n");
   else if( error_count == 1 )
     debugf("1 error detected\n");
   else {
     debugf("%d errors detected\n", error_count);
     error_count= 1;
   }

   return error_count;
}
