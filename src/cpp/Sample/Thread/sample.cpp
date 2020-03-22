//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the MIT license.
//       (See accompanying file LICENSE.MIT or the original contained
//       within https://opensource.org/licenses/MIT)
//
//----------------------------------------------------------------------------
//
// Title-
//       sample.cpp
//
// Purpose-
//       Sample thread usage.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <com/Signal.h>
#include <pthread.h>                // Must precede other includes

#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <com/Debug.h>
#include <com/define.h>

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "SAMPLE  " // Source file, for debugging

#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef SCDM
#define SCDM                        // If defined, Soft Core Debug Mode
#endif

#ifndef IODM
#undef  IODM                        // If defined, Input/Output Debug Mode
#endif

//----------------------------------------------------------------------------
// Enumerations and typedefs
//----------------------------------------------------------------------------
typedef void*          (*THREADF)(void*); // Thread function

//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------
#define ELEMENTS(x) (sizeof(x) / sizeof(x[0]))

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
// Parameters
static char            ebParm[4096];// EB parameter list area
static int             port;        // Connection port number
static char            passwd[32];  // Connection password
static char            userid[32];  // Connection userid

// Switches and controls
static int             swDebug;     // Is this the debug version?
static Signal          signalHandler; // Signal handler

//----------------------------------------------------------------------------
//
// Subroutine-
//       init
//
// Purpose-
//       Initialization processing
//
//----------------------------------------------------------------------------
static void
   init( void )                     // Initialize
{
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       term
//
// Purpose-
//       Termination processing
//
//----------------------------------------------------------------------------
static void
   term( void )                     // Terminate
{
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       logf
//
// Purpose-
//       Write message to log.
//
//----------------------------------------------------------------------------
static inline void
   vlogf(                           // Write log message
     const char*     fmt,           // PRINTF format descriptor
     va_list         argptr)        // PRINTF arguments
{
   vfprintf(stderr, fmt, argptr);   // Copy to stderr (used as log)
   fflush(stderr);
}

static inline void
   logf(                            // Write log message
     const char*     fmt,           // PRINTF format descriptor
                     ...)           // PRINTF argruments
{
   va_list           argptr;        // Argument list pointer

   va_start(argptr, fmt);           // Initialize va_ functions
   vlogf(fmt, argptr);              // Write to log
   va_end(argptr);                  // Close va_ functions
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       logh
//
// Purpose-
//       Write message to log with heading.
//
//----------------------------------------------------------------------------
static inline void
   vlogh(                           // Write log message header
     int             lineno,        // Line number
     const char*     fmt,           // PRINTF format descriptor
     va_list         argptr)        // PRINTF arguments
{
   logf("%4d: %s ", lineno, __SOURCE__);
   vlogf(fmt, argptr);
}

static inline void
   logh(                            // Write log message header
     int             lineno,        // Line number
     const char*     fmt,           // PRINTF format descriptor
                     ...)           // PRINTF argruments
{
   va_list           argptr;        // Argument list pointer

   va_start(argptr, fmt);           // Initialize va_ functions
   vlogh(lineno, fmt, argptr);      // Write to log
   va_end(argptr);                  // Close va_ functions
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       shouldNotOccur
//
// Purpose-
//       Write message to log and exit.
//
//----------------------------------------------------------------------------
static void
   shouldNotOccur(                  // Write log message and exit
     int             lineno,        // Line number
     const char*     fmt,           // PRINTF format descriptor
                     ...)           // PRINTF argruments
{
   va_list           argptr;        // Argument list pointer

   va_start(argptr, fmt);           // Initialize va_ functions
   vlogh(lineno, fmt, argptr);      // Write to log
   va_end(argptr);                  // Close va_ functions

   exit(EXIT_FAILURE);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       info
//
// Purpose-
//       Informational exit
//
//----------------------------------------------------------------------------
static void
   info(                            // Informational exit
     const char*     sourceName)    // The source fileName
{
   fprintf(stderr, "%s function <options>\n", sourceName);
   fprintf(stderr, "\n");
   fprintf(stderr, "Options:\n");
   fprintf(stderr, "-port:number\tPort number (default 65025)\n");
   fprintf(stderr, "-user:userid\tConnection userid\n");
   fprintf(stderr, "-pass:passwd\tConnection password\n");
   exit(EXIT_FAILURE);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       parm
//
// Purpose-
//       Analyze parameters
//
//----------------------------------------------------------------------------
static void
   parm(                            // Parameter analysis
     int             argc,          // Argument count
     char*           argv[])        // Argument array
{
   int               error;         // TRUE if error encountered
   int               verify;        // TRUE if verify required

   int               i, j;

   //-------------------------------------------------------------------------
   // Set defaults
   //-------------------------------------------------------------------------
   error= FALSE;                    // Default, no error found
   verify= FALSE;

   ebParm[0]= '\0';                 // Set default parameters
   port= 65025;
   strcpy(userid, "userid");
   strcpy(passwd, "password");
   swDebug= FALSE;

   //-------------------------------------------------------------------------
   // Examine parameters
   //-------------------------------------------------------------------------
   for(j=1; j<argc; j++)            // Examine the parameter list
   {
     if( argv[j][0] == '-' )        // If this is a switch list
     {
       if( strcmp(argv[j], "-help") == 0 )
         error= TRUE;

       else if( memcmp(argv[j], "-port:", 6) == 0 )
         port= atol(argv[j]+6);

       else if( memcmp(argv[j], "-user:", 6) == 0 )
       {
         if( strlen(argv[j]+6) >= sizeof(userid) )
         {
           error= TRUE;
           fprintf(stderr, "Parameter too long: '%s'\n", argv[j]);
           continue;
         }

         strcpy(userid, argv[j]+6);
       }

       else if( memcmp(argv[j], "-pass:", 6) == 0 )
       {
         if( strlen(argv[j]+6) >= sizeof(passwd) )
         {
           error= TRUE;
           fprintf(stderr, "Parameter too long: '%s'\n", argv[j]);
           continue;
         }

         strcpy(passwd, argv[j]+6);
       }

       else if( strcmp(argv[j], "--") == 0 )
         break;

       else
       {
         for(i=1; argv[j][i] != '\0'; i++) // Examine the switch list
         {
           switch(argv[j][i])       // Examine the switch
           {
             case 'd':              // -d (debug)
               swDebug= TRUE;
               break;

             case 'h':              // -h (help)
               error= TRUE;
               break;

             case 'v':              // -v (verify)
               verify= TRUE;
               break;

             default:               // If invalid switch
               error= TRUE;
               fprintf(stderr, "Invalid switch '%c'\n", (int)argv[j][i]);
               break;
           }
         }
       }
     }
     else                           // Argument
     {
       error= TRUE;
       fprintf(stderr, "Invalid parameter: '%s'\n", argv[j]);
     }
   }

   //-------------------------------------------------------------------------
   // Load the EB parameters
   //-------------------------------------------------------------------------
   for(j=j+1; j<argc; j++)          // Examine the parameter list
   {
     if( ebParm[0] != '\0' )        // If there is a parameter there
       strcat(ebParm, " ");         // Add a blank delimiter

     if( strlen(ebParm) + strlen(argv[j]) >= sizeof(ebParm) )
     {
       error= TRUE;
       fprintf(stderr, "Too many EB parameters!\n");
       break;
     }

     strcat(ebParm, argv[j]);
   }

   //-------------------------------------------------------------------------
   // Check and validate
   //-------------------------------------------------------------------------
   if( error )
     info(argv[0]);

   if( verify )
   {
     fprintf(stderr, "-port: %d\n", port);
     fprintf(stderr, "-user: '%s'\n", userid);
     fprintf(stderr, "-pass: '%s'\n", passwd);
     fprintf(stderr, "-parm: '%s'\n", ebParm);
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       startThread
//
// Purpose-
//       Start a thread.
//
//----------------------------------------------------------------------------
static pthread_t                    // Thread identifier
   startThread(                     // Start a thread
     THREADF           function,    // The function
     void*             parameter)   // The thread's parameter
{
   pthread_attr_t      attrs;       // Thread attributes
   pthread_t           tid;         // Thread identifier

   int                 rc;

   rc= pthread_attr_init(&attrs);   // Initialize the attributes
   if( rc != 0 )
     shouldNotOccur(__LINE__, "%d= pthread_attr_init\n", rc);

   pthread_attr_setdetachstate(&attrs, PTHREAD_CREATE_JOINABLE);
   rc= pthread_create(&tid, &attrs, function, parameter);
   if( rc != 0 )
     shouldNotOccur(__LINE__, "%d= pthread_create\n", rc);

   pthread_attr_destroy(&attrs);    // Destroy the attributes
   return tid;                      // Return the thread identifier
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       waitThread
//
// Purpose-
//       Wait for the completion of a thread.
//
//----------------------------------------------------------------------------
static void*                        // Return code
   waitThread(                      // Wait for a thread to complete
     pthread_t         tid)         // Thread identifier
{
   void*               rc;          // Return code

   pthread_join(tid, &rc);
   return rc;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       sampleThread
//
// Purpose-
//       Sample thread.
//
//----------------------------------------------------------------------------
static void*                        // Return value
   sampleThread(                    // Sample thread
     void*             parm)        // Parameter
{
   printf("sampleThread(%s)\n", (const char*)parm);
   return (void*)"sampleThread";
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       sample
//
// Purpose-
//       Sample driver.
//
//----------------------------------------------------------------------------
static void
   sample( void )                   // Sample usage of threads
{
   pthread_t           one;         // Tid[1]
   pthread_t           two;         // Tid[2]

   one= startThread(&sampleThread, (void*)"Thread one parameter");
   two= startThread(&sampleThread, (void*)"Thread two parameter");
   printf("Thread one returns(%s)\n", (const char*)waitThread(one));
   printf("Thread two returns(%s)\n", (const char*)waitThread(two));
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
int                                 // Return code
   main(                            // Mainline code
     int             argc,          // Argument count
     char*           argv[])        // Argument array
{
   //-------------------------------------------------------------------------
   // Parameter validation
   //-------------------------------------------------------------------------
   init();
   parm(argc, argv);

   //-------------------------------------------------------------------------
   // Process the function
   //-------------------------------------------------------------------------
   sample();                        // Sample driver

   //-------------------------------------------------------------------------
   // Return to caller
   //-------------------------------------------------------------------------
   term();
   return 0;
}

