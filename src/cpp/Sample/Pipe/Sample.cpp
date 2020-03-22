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
//       Sample.cpp
//
// Purpose-
//       Sample process usage.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>

#undef SIGHUP
#undef SIGINT
#undef SIGQUIT
#undef SIGILL
#undef SIGTRAP
#undef SIGABRT
#undef SIGEMT
#undef SIGFPE
#undef SIGKILL
#undef SIGBUS
#undef SIGSEGV
#undef SIGSYS
#undef SIGPIPE
#undef SIGALRM
#undef SIGTERM
#undef SIGURG
#undef SIGSTOP
#undef SIGTSTP
#undef SIGCONT
#undef SIGCHLD
#undef SIGTTIN
#undef SIGTTOU
#undef SIGIO
#undef SIGXCPU
#undef SIGXFSZ
#undef SIGMSG
#undef SIGWINCH
#undef SIGPWR
#undef SIGUSR1
#undef SIGUSR2
#undef SIGPROF
#undef SIGDANGER
#undef SIGVTALRM
#undef SIGMIGRATE
#undef SIGPRE
#undef SIGVIRT
#undef SIGGRANT
#undef SIGRETRACT
#undef SIGSOUND
#undef SIGSAK

#include <com/Debug.h>
#include <com/Signal.h>

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
// Constants for parameterization
//----------------------------------------------------------------------------
#define PIPE_STDINP 0               // Pipe standard input index
#define PIPE_STDOUT 1               // Pipe standard output index
#define PIPE_COUNT  2               // Number of pipe indexes

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
static Signal          sigHand;     // Signal handler

// Switches and controls
static int             swDebug;     // Is this the debug version?
static int             swSystem;    // Is this the SYSTEM version?

//----------------------------------------------------------------------------
// Constant data areas
//----------------------------------------------------------------------------
static const char*     const vmstatParm[]= // vmstat process execvp string
   { "vmstat"
   , "10"
   , "5"
   , NULL
   };

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
   fprintf(stdout, "This goes to stdout\n");
   fprintf(stderr, "This goes to stderr\n");
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
   fprintf(stderr, "-system\tUse system\n");
   fprintf(stderr, "-v\tVerify parameters\n");
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

   swDebug= FALSE;
   swSystem= FALSE;

   //-------------------------------------------------------------------------
   // Examine parameters
   //-------------------------------------------------------------------------
   for(j=1; j<argc; j++)            // Examine the parameter list
   {
     if( argv[j][0] == '-' )        // If this is a switch list
     {
       if( strcmp(argv[j], "-help") == 0 )
         error= TRUE;

       else if( strcmp(argv[j], "-system") == 0 )
         swSystem= TRUE;

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
   // Check and validate
   //-------------------------------------------------------------------------
   if( error )
     info(argv[0]);

   if( verify )
   {
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       viafork
//
// Purpose-
//       Start a process using FORK.
//
//----------------------------------------------------------------------------
static const char*                  // Return message (NULL OK)
   viafork(                         // Start a process with piped output
     const char*       const plist[], // Parameter list, plist[0] is program
     pid_t&            processID,   // (OUT) Process identifier
     int&              pipeHand)    // (OUT) Pipe file handle
{
   const char*         result;      // Resultant

   FILE*               fileHand;    // The file handle of interest
   int                 handle;      // The pipe handle of interest
   int                 xfer[PIPE_COUNT]; // Transfer pipe
   char                string[128]; // Working string
   struct tm           tm;          // Time format field
   time_t              tod;         // Time of day
   int                 rc;

   // Flush stdout, stderr (so child processes won't duplicate output)
   fflush(stdout);
   fflush(stderr);

   // Create a transfer pipe
   rc= pipe(xfer);                  // Create the pipes
   if( rc != 0 )
   {
     result= "pipe() failure";
     fprintf(stderr, "Unable to create pipe ");
     perror(result);
     return result;
   }

   // Run the child process
   processID= fork();               // Create a child process
   if( processID == 0 )             // If this is the child
   {
     // Transfer the pipe into stderr, stdout
     close(xfer[PIPE_STDINP]);      // We won't be needing this
     handle= xfer[PIPE_STDOUT];

     fclose(stderr);
     fclose(stdout);
     dup2(handle, fileno(stderr));
     dup2(handle, fileno(stdout));
     close(handle);

     // At this point stdout isn't really stdout, it's closed.
     // Printing something DISAPPEARS!
     printf("Are you there?\n");
     fprintf(stderr, "I guess not!\n");
     fflush(stdout);
     fflush(stderr);

     // But if we write to fileno(stdout), it's visible
     tod= time(NULL);
     tm= *localtime(&tod);
     sprintf(string, "WRITE(fileno(stdout): %s", asctime(&tm));
     write(fileno(stdout), string, strlen(string));

     // And if we take pains to have a FILE*, that's visible too
     handle= dup(fileno(stdout));
     fileHand= fdopen(handle, "w");
     fprintf(fileHand, "FPRINTF(fdopen(dup(fileno(stdout))): %s",
                       asctime(&tm));
     fclose(fileHand);

     // EXECVP corrects stderr and stdout so they go to the pipe
     // so the called program doesn't need to know.
     execvp(plist[0], (char**)(plist)); // Transfer control

     sprintf(string, "%4d Return from execvp()\n", __LINE__);
     write(fileno(stdout), string, strlen(string));
     exit(EXIT_FAILURE);            // (Should not occur)
   }

   // Run the parent process
   close(xfer[PIPE_STDOUT]);        // We won't be needing this
   pipeHand= xfer[PIPE_STDINP];

   result= NULL;
   if( (int)processID < 0 )
   {
     result= "fork() failure";
     fprintf(stderr, "Unable to start %s: ", plist[0]);
     perror(result);

     // Clean up for caller
     close(xfer[PIPE_STDINP]);      // We won't be needing this now, either
     pipeHand= (-1);
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       viasystem
//
// Purpose-
//       Start a process using SYSTEM.
//
//----------------------------------------------------------------------------
static const char*                  // Return message (NULL OK)
   viasystem(                       // Start a process with piped output
     const char*       const plist[], // Parameter list, plist[0] is program
     pid_t&            processID,   // (OUT) Process identifier
     int&              pipeHand)    // (OUT) Pipe file handle
{
   const char*         result;      // Resultant

   FILE*               fileHand;    // The file handle of interest
   int                 handle;      // The pipe handle of interest
   int                 oldERR;      // Old stderr
   int                 oldOUT;      // Old stdout
   int                 xfer[PIPE_COUNT]; // Transfer pipe
   char                string[128]; // Working string
   struct tm           tm;          // Time format field
   time_t              tod;         // Time of day

   int                 i;
   int                 rc;

   // WARNING
   fprintf(stderr, "-system does not work properly\n");

   // Flush stdout, stderr (so child processes won't duplicate output)
   fflush(stdout);
   fflush(stderr);

   // Create a transfer pipe
   rc= pipe(xfer);                  // Create the pipes
   if( rc != 0 )
   {
     result= "pipe() failure";
     fprintf(stderr, "Unable to create pipe ");
     perror(result);
     return result;
   }

   // Switch the output files
   handle= xfer[PIPE_STDOUT];

   oldERR= dup(fileno(stderr));
   oldOUT= dup(fileno(stdout));
// fclose(stderr);
// fclose(stdout);
// rc= fileno(stdout);            // For debugging
   dup2(handle, fileno(stderr));
   dup2(handle, fileno(stdout));
   rc= fileno(stdout);            // For debugging
   close(handle);

   // At this point stdout isn't really stdout, it's the pipe.
   printf("Are you there?\n");
   fprintf(stderr, "I guess not!\n");
   fflush(stdout);
   fflush(stderr);

   // But if we write to fileno(stdout), it's visible
   tod= time(NULL);
   tm= *localtime(&tod);
   sprintf(string, "WRITE(fileno(stdout): %s", asctime(&tm));
   write(fileno(stdout), string, strlen(string));

   // And if we take pains to have a FILE*, that's visible too
   handle= dup(fileno(stdout));
   fileHand= fdopen(handle, "w");
   fprintf(fileHand, "FPRINTF(fdopen(dup(fileno(stdout))): %s",
                     asctime(&tm));
   fclose(fileHand);
   close(handle);

   // Run the program
   memset(string, '\0', sizeof(string)); // For debugging
   strcpy(string, plist[0]);
   for(i= 1; plist[i] != NULL; i++)
   {
     strcat(string, " ");
     strcat(string, plist[i]);
   }

   fflush(stdout);
   fflush(stderr);
   system(string);                  // Transfer control

   // Return
   dup2(oldERR, fileno(stderr));
   dup2(oldOUT, fileno(stdout));
   close(oldERR);
   close(oldOUT);

   pipeHand= xfer[PIPE_STDINP];
   processID= 0;
   result= NULL;

   return result;
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
   char                buffer[8];   // Transfer buffer
   int                 L;           // Transfer length
   int                 status;      // Process completion status
   const char*         string;      // Working string
   pid_t               vmstat;      // VMSTAT process identifier
   int                 vmhand;      // VMSTAT file handle

   if( swSystem )
   {
     string= viasystem(vmstatParm, vmstat, vmhand);
     if( string != NULL )
     {
       fprintf(stderr, "viasystem failure: %s\n", string);
       return;
     }
   }
   else
   {
     string= viafork(vmstatParm, vmstat, vmhand);
     if( string != NULL )
     {
       fprintf(stderr, "viafork failure: %s\n", string);
       return;
     }
   }

   // Copy the piped output
   printf("\n");
   printf("Beginning piped output:\n");
   for(;;)
   {
     L= read(vmhand, buffer, 1);
     if( L <= 0 )
     {
       if( L < 0 )
       {
         fprintf(stderr, "Pipe I/O error ");
         perror("read()");
       }
       break;
     }

     if( buffer[0] == '\\' )
       putchar('\\');
     if( buffer[0] == '\r' )
     {
       printf("\\r");
       continue;
     }
     if( buffer[0] == '\n' )
       printf("\\n");
     putchar(buffer[0]);
     fflush(stdout);
   }
   close(vmhand);

   if( (int)vmstat > 0 )
     waitpid(vmstat, &status, 0);
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
   sample();                        // Sample driver
   sample();                        // Sample driver
   sample();                        // Sample driver

   //-------------------------------------------------------------------------
   // Return to caller
   //-------------------------------------------------------------------------
   term();
   return 0;
}

