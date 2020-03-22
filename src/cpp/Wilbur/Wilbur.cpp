//----------------------------------------------------------------------------
//
//       Copyright (c) 2010 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Wilbur.cpp
//
// Purpose-
//       Wilbur mainline.
//
// Last change date-
//       2010/01/01
//
// Controls-
//       If the first parameter is not a switch parameter, it specifies the
//       log file name (and sets intensive debug mode.)
//
//       (No other parameters are available.)
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <exception>

#include <com/Debug.h>
#include <com/Exception.h>
#include <com/Signal.h>

#include "Common.h"

//----------------------------------------------------------------------------
//
// Class-
//       MySignal
//
// Purpose-
//       Signal handler.
//
//----------------------------------------------------------------------------
class MySignal : public Signal {
public:
virtual int                         // Return code (0 iff handled)
   handle(                          // Signal handler
     SignalCode        signal)      // Signal code
{
   debugf("Signal(%d) %s received\n",
          signal, getSignalName(signal));

   int result= 1;                   // Default, failed to handle signal
   switch(signal)                   // Handle signal
   {
     case SC_CHILDSTOP:             // Ignored signals
     case SC_BGRDCONTROL:
     case SC_BGWRCONTROL:
     case SC_WINDOWSIZE:
       result= 0;
       break;

     default:
       Common* common= Common::get(); // Shutdown
       if( common != NULL )
       {
         common->shutdown();
         delete common;
       }
       break;
   }

   return result;
}
}; // class MySignal

//----------------------------------------------------------------------------
//
// Subroutine-
//       exit_handler
//
// Purpose-
//       Run exit handler.
//
//----------------------------------------------------------------------------
static void
   exit_handler( void )             // Exit handler
{
   // Note: the TraceLogger has been deleted at this point. Use printf.
   // printf("Wilbur: exit_handler\n");
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       setup
//
// Purpose-
//       Set up termination handlers.
//
//----------------------------------------------------------------------------
static void
   setup( void )                    // Set up termination handlers
{
   if( atexit(exit_handler) != 0 )  // If cannot register handler
     throw "atexit failure";
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
   Common*             common= NULL;// Wilbur common area
   MySignal            signal;      // Signal handler

   //-------------------------------------------------------------------------
   // Set logFile
   //-------------------------------------------------------------------------
   const char* logFile= NULL;       // Default log file name
   int arg1= 1;                     // Default first argument
   if( argc > arg1 && *argv[arg1] != '-' ) // If filename parameter
     logFile= argv[arg1++];         // Set the filename parameter

// logFile= "2>";                   // Extreme HCDM, logFile= stderr

   //-------------------------------------------------------------------------
   // Operate Wilbur
   //-------------------------------------------------------------------------
   try {
     printf("Starting Wilbur...\n");
     common= Common::activate(logFile); // Access Common area
     if( logFile != NULL )
       debugSetIntensiveMode();     // (Allow tail -f debug.out)

     setup();                       // Set up termination handlers

     logf("... Wilbur READY ...\n");
     common->finalize();
//   logf("...Wilbur complete\n");  // TraceLogger no longer exists

     if( FALSE ) {                  // *** FAILS TO RAISE SEGV ***
       printf("Should raise SIGSEGV\n");
       Common* common= NULL;
       common->shutdown();
       printf("ShouldNotOccur\n");
     }

     if( FALSE ) {                  // (Handled properly)
       printf("Should raise SIGABRT\n");
       abort();
     }

     if( FALSE ) {                  // (Handled properly)
       printf("Should throw(const char*)\n");
       throw "That's all, Folks";
     }
   } catch(const char* X) {
     printf("Exception(%s)\n", X);
   } catch(std::exception& X) {
     printf("catch(exception.what(%s))\n", X.what());
   } catch(...) {
     printf("Exception(...)\n");
   }

   return 0;
}

