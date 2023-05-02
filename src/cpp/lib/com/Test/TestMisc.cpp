//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2020 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       TestMisc.cpp
//
// Purpose-
//       Miscellaneous tests.
//
// Last change date-
//       2020/10/03
//
//----------------------------------------------------------------------------
#include <new>                      // For new(void*) {in-place operator}
#include <iostream>                 // For cout <<, endl

#include <math.h>                   // For fabs()
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <com/Debug.h>
#include <com/Random.h>

#include "com/Clock.h"
#include "com/Exception.h"
#include "com/Handler.h"
#include "com/istring.h"
#include "com/MinMax.h"
#include "com/Normalizer.h"
#include "com/Signal.h"
#include "com/Thread.h"
#include "com/Trace.h"
#include "com/Verify.h"

// clude "TestMisc.err"             // Work in progress that doesn't work

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#define HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef SCDM
#define SCDM                        // If defined, Soft Core Debug Mode
#endif

#define DIM_ARRAY 32
#define EPSILON 0.00001
#define ITERATIONS 100000

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static const Exception staticException("StaticException");
static int             globalEvent;
static int             globalError;

//----------------------------------------------------------------------------
// External references
//----------------------------------------------------------------------------
static Random&         RNG= Random::standard; // Our random number generator

//----------------------------------------------------------------------------
//
// Class-
//       Record
//
// Purpose-
//       Define a Trace record.
//
//----------------------------------------------------------------------------
struct Record : public Trace::Record// A trace record
{
   // Header area
   uint32_t            traceType;   // Trace type
   void*               thread;      // -> Current thread
   uint64_t            timeStamp;   // Local epoch time stamp

   // User area
   uint32_t            userData[4]; // User data
}; // struct Record

//----------------------------------------------------------------------------
//
// Class-
//       TraceArea
//
// Purpose-
//       Define a Trace area
//
//----------------------------------------------------------------------------
struct TraceArea                    // A trace area
{
   Trace               trace;       // The trace header
   char                area[32][4097]; // Trace records
}; // struct TraceArea

//----------------------------------------------------------------------------
//
// Class-
//       MyHandler
//
// Purpose-
//       Define a handler.
//
//----------------------------------------------------------------------------
class MyHandler : public Handler
{
public:
   ~MyHandler( void ) {}
   MyHandler( void )
:  Handler() {}

virtual void
   handleError( void )              // Error handler
{
   #ifdef SCDM
     debugf("MyHandler::handleError(%d)\n", getIdent());
   #endif

   globalError= getIdent();
}

virtual void
   handleEvent( void )              // Event handler
{
   #ifdef SCDM
     debugf("MyHandler::handleEvent(%d)\n", getIdent());
   #endif

   globalEvent= getIdent();
}
}; // class MyHandler

//----------------------------------------------------------------------------
//
// Class-
//       MySignal
//
// Purpose-
//       Define a Signal handler.
//
//----------------------------------------------------------------------------
class MySignal : public Signal
{
public:
   ~MySignal( void ) {}
   MySignal( void )
:  Signal((uint64_t)1<<SC_USER2) {}

virtual int
   handle(                          // Signal handler
     SignalCode        signal)      // Signal code
{
   #ifdef SCDM
     debugf("MySignal::handle(%d)\n", signal);
   #endif

   if( signal == SC_USER2 ) {
     globalEvent= 12345;          // Verify signal handler called
     return 0;
   }

   return 1;
}
}; // class MySignal

//----------------------------------------------------------------------------
//
// Subroutine-
//       testBacktrace
//
// Purpose-
//       Test Exception::backtrace (Just adds another layer to the trace.)
//
//----------------------------------------------------------------------------
static void
   testBacktrace( void )            // Test Exception::backtrace
{
   debugf("\n");
   debugf("Exception::backtrace() test\n");

   Exception::backtrace();
}


//----------------------------------------------------------------------------
//
// Subroutine-
//       testVerify
//
// Purpose-
//       Test the VerifyEC object -- must be first
//
//----------------------------------------------------------------------------
static int                          // Number of VerifyEC object errors
   testVerify( void )               // Test VerifyEC object
{
   int                 errorCount= 0; // Number of VerifyEC object errors

   int                 rc;          // Return code

   verify_info(); debugf("testVerify()\n");
   VerifyEC::message(__FILE__, __LINE__, "%s", "testVerify()\n");

   debugf("\n");
   verify("This is not really an error" == NULL);
   VerifyEC::_verify_(FALSE, __FILE__, __LINE__, "%s %s %s %s %s %s",
                     "This", "is", "not", "really", "an", "error");

   if( !verify( error_count() == 2 ) )
   {
     errorCount++;
     verify_info(); debugf("Error: error_count(%d)\n", error_count());
   }

   rc= VerifyEC::exit("VerifyEC self-test with 2 errors");
   if( rc != 1 )
   {
     errorCount++;
     VerifyEC::_verify_(FALSE, __FILE__, __LINE__, "Exit code(%d)", rc);
   }

   debugf("\n");
   verify("This is not really an error");

   if( !verify( error_count() == 1 ) )
   {
     errorCount++;
     verify_info(); debugf("Error: error_count(%d)\n", error_count());
   }

   rc= VerifyEC::exit("VerifyEC self-test with 1 error");
   if( rc != 1 )
   {
     errorCount++;
     VerifyEC::_verify_(FALSE, __FILE__, __LINE__, "Exit code(%d)", rc);
   }

   debugf("\n");
   rc= VerifyEC::exit("VerifyEC self-test with NO errors");
   if( rc != 0 )
   {
     errorCount++;
     VerifyEC::_verify_(FALSE, __FILE__, __LINE__, "Exit code(%d)", rc);
   }

   return errorCount;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testClock
//
// Purpose-
//       Test the Clock object.
//
//----------------------------------------------------------------------------
static void
   testClock( void )                // Test Clock object
{
   debugf("\n");
   verify_info(); debugf("testClock()\n");

   Clock               diff;
   Clock               now;
   Clock               then;
   unsigned long       count;

   //-------------------------------------------------------------------------
   // Compute the granule, the smallest clock quanta
   //-------------------------------------------------------------------------
   then= Clock::current();
   do
   {
     now= Clock::current();
   } while( now == then );
   diff= now - then;
   verify_info(); debugf("Granule(%f)\n", (double)diff);

   //-------------------------------------------------------------------------
   // Determine rate at which the clock may be called
   //-------------------------------------------------------------------------
   then= Clock::current();
   for(count= 0;;count++)
   {
     now= Clock::current();
     diff= now - then;
     if( (double)diff >= 3.3 )
       break;

     #ifdef SCDM
       debugf("%16.4f (%5.4f)\r", (double)now, (double)diff);
     #endif
   }
   #ifdef SCDM
     debugf("\n\n");
   #endif

   debugf("%16.4f Stop\n", (double)now);
   debugf("%16.4f Start\n", (double)then);
   debugf("%16.4f Interval\n", (double)diff);
   debugf("\n");
   verify_info(); debugf("%12ld Iterations (%.2f per second)\n",
          count, (double)count/(double)diff);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       subException
//
// Purpose-
//       Throw an Exception to test the Exception object.
//
//----------------------------------------------------------------------------
extern "C" void subException( void ); // (Not very far) Forward reference
extern "C" void
   subException( void )             // Test Exception in subroutine call
{
   throw Exception("SubException");
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testException
//
// Purpose-
//       Test the Exception object.
//
//----------------------------------------------------------------------------
static void
   testException( void )            // Test Exception
{
   debugf("\n");
   verify_info(); debugf("testException()\n");
   testBacktrace();

   Exception         autoException("AutomaticException");

   try
   {
     throw autoException;
     verify("AutoException not thrown" == NULL );
   }
   catch(Exception& e)
   {
     verify( strcmp((const char*)e, "AutomaticException") == 0);
   }
   catch(...)
   {
     verify( "ShouldNotOccur" == NULL);
   }

   try
   {
     throw staticException;
     verify("StaticException not thrown" == NULL );
   }
   catch(Exception& e)
   {
     verify( strcmp(e.what(), "StaticException") == 0);
   }
   catch(...)
   {
     verify( "ShouldNotOccur" == NULL);
   }

   try
   {
     throw Exception("NewException");
     verify("NewException not thrown" == NULL );
   }
   catch(Exception& e)
   {
     verify( strcmp(e.what(), "NewException") == 0);
   }
   catch(...)
   {
     verify( "ShouldNotOccur" == NULL);
   }

   try
   {
     throw NoStorageException();
     verify("NoStorageException not thrown" == NULL );
   }
   catch(Exception& e)
   {
     verify( strcmp(e.what(), "NoStorageException") == 0);
   }
   catch(...)
   {
     verify( "ShouldNotOccur" == NULL);
   }

   try
   {
     subException();
     verify("SubException not thrown" == NULL );
   }
   catch(Exception& e)
   {
     verify( strcmp(e.what(), "SubException") == 0);
   }
   catch(...)
   {
     printf("SHOULD NOT OCCUR\n");
     verify( "ShouldNotOccur" == NULL);
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testHandler
//
// Purpose-
//       Test the Handler object.
//
//----------------------------------------------------------------------------
static void
   testHandler( void )              // Test Handler object
{
   debugf("\n");
   verify_info(); debugf("testHandler()\n");

   MyHandler           handler;     // My handler
   Handler             reference;   // Reference to handler

   globalEvent= (-1);
   globalError= (-1);
   handler.error(123);
   verify( globalError == 123 );    // verify handler.handleError() called

   reference.setHandler(&handler);
   reference.event(321);
   verify( globalEvent == 321 );    // verify handler.handleEvent() called
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testMath
//
// Purpose-
//       Test the MinMax and Normalizer objects
//
//----------------------------------------------------------------------------
static void
   testMath( void )                 // Test math objects
{
   debugf("\n");
   verify_info(); debugf("testMath()\n");

   MinMax              minmax;
   Normalizer          normal;
   double              array[DIM_ARRAY];
   double              normalized;
   double              restored;

   int                 i;

   for(i=0; i<DIM_ARRAY; i++)
   {
     array[i]= double(RNG.get()&0x000000007fffffffLL)/732.0;
     minmax.sample(array[i]);
   }
   normal.initialize(0.0, 1.0, minmax.getMinimum(), minmax.getMaximum());

   for(i= 0; i<DIM_ARRAY; i++)
   {
     normalized= normal.normalize(array[i]);
     restored= normal.restore(normalized);

     tracef("%f= normal.normalize(%f)\n", normalized, array[i]);
     tracef("%f= normal.restore(%f)\n", restored, normalized);
     tracef("\n");

     verify( normalized >= 0.0 && normalized <= 1.0 );
     verify( fabs(restored - array[i]) < EPSILON );
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testSignal
//
// Purpose-
//       Test the Signal object
//
//----------------------------------------------------------------------------
static void
   testSignal( void )               // Test Signal object
{
   debugf("\n");
   verify_info(); debugf("testSignal()\n");

   globalEvent= (-1);
   MySignal mySignal;               // Signal handler
   mySignal.generate(mySignal.SC_USER2);
   Thread::sleep(1.5);
   verify( globalEvent == 12345 );  // Verify signal handler
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testString
//
// Purpose-
//       Test the istring object
//
//----------------------------------------------------------------------------
static void
   testString( void )               // Test istring object
{
   debugf("\n");
   verify_info(); debugf("testString()\n");

   istring h1= "hello";
   istring h2= "hElLo";
   istring h3= "HellO";
   istring h4= "HELLO";

   istring w1= "world";
   istring w2= "WoRlD";
   istring w3= "wORLd";
   istring w4= "WORLD";

   std::cout << "This is h1 '" << h1 << "'" << std::endl;
   std::cout << "This is h2 '" << h2 << "'" << std::endl;
   std::cout << "This is h3 '" << h3 << "'" << std::endl;
   std::cout << "This is h4 '" << h4 << "'" << std::endl;

   std::cout << "This is w1 '" << w1 << "'" << std::endl;
   std::cout << "This is w2 '" << w2 << "'" << std::endl;
   std::cout << "This is w3 '" << w3 << "'" << std::endl;
   std::cout << "This is w4 '" << w4 << "'" << std::endl;

   verify( h1 == h4.c_str() );
   verify( h1 == "HeLLo" );
   verify( h1 == "hello" );
   verify( h1 != "hallo" );

   verify( h1.c_str() == h4 );
   verify( "HeLLo" == h1 );
   verify( "hello" == h1 );
   verify( "hallo" != h1 );
   verify( "HeLLoWorld" == h1 + w4 );
   verify( "HellOwOrlD" == (h1 + w4) );

   verify( h1 == h1 );
   verify( h1 == h2 );
   verify( h1 == h3 );
   verify( h1 == h4 );

   verify( h2 == h1 );
   verify( h2 == h2 );
   verify( h2 == h3 );
   verify( h2 == h4 );

   verify( h3 == h1 );
   verify( h3 == h2 );
   verify( h3 == h3 );
   verify( h3 == h4 );

   verify( h4 == h1 );
   verify( h4 == h2 );
   verify( h4 == h3 );
   verify( h4 == h4 );

   verify( w1 == w1 );
   verify( w1 == w2 );
   verify( w1 == w3 );
   verify( w1 == w4 );

   verify( w2 == w1 );
   verify( w2 == w2 );
   verify( w2 == w3 );
   verify( w2 == w4 );

   verify( w3 == w1 );
   verify( w3 == w2 );
   verify( w3 == w3 );
   verify( w3 == w4 );

   verify( w4 == w1 );
   verify( w4 == w2 );
   verify( w4 == w3 );
   verify( w4 == w4 );

   verify( h1 != w1 );
   verify( h1 != w2 );
   verify( h1 != w3 );
   verify( h1 != w4 );

   verify( w1 != h1 );
   verify( w1 != h2 );
   verify( w1 != h3 );
   verify( w1 != h4 );

   verify( strcmp(h1.c_str(), h1.c_str()) == 0 );
   verify( strcmp(h1.c_str(), h2.c_str()) != 0 );
   verify( strcmp(h1.c_str(), h3.c_str()) != 0 );
   verify( strcmp(h1.c_str(), h4.c_str()) != 0 );

   verify( strcmp(w1.c_str(), w1.c_str()) == 0 );
   verify( strcmp(w1.c_str(), w2.c_str()) != 0 );
   verify( strcmp(w1.c_str(), w3.c_str()) != 0 );
   verify( strcmp(w1.c_str(), w4.c_str()) != 0 );

   std::string s1= "hello";
   std::string s4= "WORLD";

   std::string ss;
   istring     ii;

   ii= s1.c_str();                 // Must convert to assign
   ii += " ";                      // (and also separate out operations)
   ii += s4.c_str();
   verify( strcmp(ii.c_str(), "hello WORLD") == 0 );

   ii= h1 + " " + w4;              // Can combine istrings
   verify( strcmp(ii.c_str(), "hello WORLD") == 0 );

   ss= s1 + " " + s4;              // Can combine strings
   verify( strcmp(ss.c_str(), "hello WORLD") == 0 );

   ss= h1.c_str();                 // Must convert to assign
   ss += " ";                      // (and also separate out operations)
   ss += w4.c_str();
   verify( strcmp(ss.c_str(), "hello WORLD") == 0 );

   #if 0 // Compile errors!
     ss= i1;                        // A istring is NOT a string
     ii= s1;                        // And vice-versa
     ss= h1.c_str() + " " + w2.c_str(); // DISALLOWED
     ii= s1.c_str() + " " + s2.c_str(); // DISALLOWED
     ii= h1.c_str() + " " + h2.c_str(); // DISALLOWED

     ii= h1 + " " + h2.c_str();     // ALLOWED
     ii= h1.c_str() + " " + h2;     // DISALLOWED
     ss= s1 + " " + h2.c_str();     // ALLOWED
     ss= s1.c_str() + " " + h2;     // DISALLOWED
   #endif

   // Seeing is believing, so just to be surely sure...
   std::cout << "ii '" << ii << "'" << std::endl;
   std::cout << "ss '" << ss << "'" << std::endl;

   std::cout << "ii '" << ii << "', " // We can mix/match with cout <<
             << "ss '" << ss << "'" << std::endl;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testTrace
//
// Purpose-
//       Test the Trace objects
//
//----------------------------------------------------------------------------
static void
   testTrace( void )                // Test Trace objects
{
   debugf("\n");
   verify_info(); debugf("testTrace()\n");

   TraceArea           area;
   Record*             record;

   long                i;

   new(&area.trace) Trace(sizeof(area));
   for(i=0; i<0x00010000; i++)
   {
     record= (Record*)area.trace.allocate(sizeof(record));

     record->traceType= *((uint32_t*)(".TST"));
     record->thread=    (void*)(size_t)i;
     record->timeStamp= i;
     record->userData[0]= i;
     record->userData[1]= i;
     record->userData[2]= i;
     record->userData[3]= i;
   }

   verify_info(); debugf("Trace area"); debugf("\n");
   dump(&area, sizeof(area));       // Appears only in trace
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
   main(int, char**)                // Mainline code
//   int               argc,        // Argument count
//   char*             argv[])      // Argument array
{
   //-------------------------------------------------------------------------
   // Initialization
   //-------------------------------------------------------------------------
   #ifdef HCDM
     debugSetIntensiveMode();       // Hard Core Debug Mode
     verify_info(); debugf("HCDM\n");
   #endif

   //-------------------------------------------------------------------------
   // TRY wrapper
   //-------------------------------------------------------------------------
   try {

   //-------------------------------------------------------------------------
   // Prerequisite tests
   //-------------------------------------------------------------------------
   if( 1 )                          // These objects have passed all tests
   {
     if( testVerify() != 0 )
     {
       debugf("Verify errors preclude further testing\n");
       return 1;
     }
   }

   //-------------------------------------------------------------------------
   // Object tests
   //-------------------------------------------------------------------------
   if( 1 )                          // These objects have passed all tests
   {
     testClock();
     testException();
     testHandler();
     testSignal();
     testString();
     testMath();
     testTrace();
   }

   //-------------------------------------------------------------------------
   // CATCH wrapper
   //-------------------------------------------------------------------------
   } catch(const char* X) {
     error_found();
     verify_info(); debugf("EXCEPTION(const char*((%s))\n", X);
   }
   catch(...)
   {
     verify("EXCEPTION(...)");
   }

//----------------------------------------------------------------------------
// Testing complete
//----------------------------------------------------------------------------
   verify_exit();
}

