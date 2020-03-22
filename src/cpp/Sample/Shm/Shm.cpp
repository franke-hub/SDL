//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the MIT license.
//       (See accompanying file LICENSE.MIT or the original contained
//       within https://opensource.org/licenses/MIT)
//
//-----------------------------------------------------------------------------
//
// Title-
//       Shm.cpp
//
// Purpose-
//       Shared Memory sample.
//
// Last change date-
//       2007/01/01
//
// Uses-
//       1) Shared memory.
//       2) Semaphores.
//       3) Threads.
//
//----------------------------------------------------------------------------
#include <pthread.h>                // Must be first

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/signal.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <com/Atomic.h>
#include <com/Debug.h>
#include <com/define.h>
#include <com/params.h>

#include "Shm.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "SHM     " // Source filename, for debugging

#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

//----------------------------------------------------------------------------
// Typedefs
//----------------------------------------------------------------------------
typedef void*          (*THREADF)(void*); // Thread function

//----------------------------------------------------------------------------
//
// enum-
//       SemInit
//
// Purpose-
//       Specify initial state for a semaphore.
//
//----------------------------------------------------------------------------
enum SemInit                        // Semaphore initial sate
{
   SEM_WAIT= 0,                     // WAITING
   SEM_POST= 1                      // POSTED
}; // enum SemInit

//----------------------------------------------------------------------------
//
// enum-
//       SemNum
//
// Purpose-
//       Specify semaphore numbers
//
//----------------------------------------------------------------------------
enum SemNum                         // Semaphore numbers
{
   SEM_STARTUP=                   0,// StartupThread
   SEM_TERMINATOR,                  // Termination
   SEM_COUNT                        // The number of semaphores
}; // enum SemNum

//----------------------------------------------------------------------------
//
// enum-
//       CommandType
//
// Purpose-
//       Specify the command type
//
//----------------------------------------------------------------------------
enum CommandType
{
   TypeUnspecified=               0,// Unspecified
   TypeBoot,                        // -boot
   TypeInit,                        // -init
   TypeTerm,                        // -term
   TypeWait,                        // -wait
   TypeCommand                      // name="command" depend ...
}; // enum CommandType

//----------------------------------------------------------------------------
// Local data areas
//----------------------------------------------------------------------------
static const char*     fileName;    // The specified filename
static key_t           fileToken;   // The specified token

static int             alwaysFalse= FALSE; // Constant, avoids compiler message
static CommandType     cmdType;     // The type of the command
static unsigned int    cmdUsed;     // The number of used bytes in cmdBuff
static char            cmdBuff[Command::CMD_SIZE]; // The command buffer
static char*           ptrName;     // -> Command name
static char*           ptrCmd;      // -> Command
static char*           ptrDeps;     // -> Dependencies

// Data areas initialized by init()
static int             semSegment= (-1); // Semaphore identifier
static int             ssrSegment= (-1); // SharedStorageRegion segment id
static SharedStorageRegion*
                       ssr= NULL;   // -> SharedStorageRegion

//----------------------------------------------------------------------------
//
// Subroutine-
//       debugCommand
//
// Purpose-
//       DEBUGGING, REMOVE.
//
//----------------------------------------------------------------------------
static inline void
   debugCommand(                    // Debugging
     int               line,        // Line number
     Command*          ptrCommand)  // -> Command
{
   char*               ptrC;

   debugf("%4d: debugCommand(%p)\n", line, ptrCommand);
   debugf("..pid(%d)\n", ptrCommand->pid);
   debugf("..fsm(%d)\n", ptrCommand->fsm);
   debugf(".. cc(%d)\n", ptrCommand->compCode);
   debugf(".. cc(%d)\n", ptrCommand->compCode);
   debugf("..name(%d) code(%d) deps(%d)\n",
          ptrCommand->name, ptrCommand->code, ptrCommand->deps);

   ptrC= (char*)ssr + ptrCommand->name; debugf("..name(%s)\n", ptrC);
   ptrC= (char*)ssr + ptrCommand->code; debugf("..code(%s)\n", ptrC);
   if( ptrCommand->deps != 0 )
     ptrC= (char*)ssr + ptrCommand->deps; debugf("..deps(%s)\n", ptrC);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       shouldNotOccur
//
// Purpose-
//       Handle a "should not occur" error.
//
//----------------------------------------------------------------------------
static volatile void
   shouldNotOccur( void )           // This error should not occur
{
   if( cmdType == TypeBoot          // If -boot command
       || (ssr != NULL && ssr->fsm == SharedStorageRegion::FSM_BOOT ) )
   {
     for(;;)                        // 888 LEDs
     {
       system("/usr/lib/methods/showled 0x888");
       sleep(1);
       system("/usr/lib/methods/showled 0xfff");
       sleep(1);

       if( alwaysFalse ) break;     // Does not occur, prevents compiler error
     }
   }

   exit(EXIT_FAILURE);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       internalError
//
// Purpose-
//       Handle an internal logic error.
//
//----------------------------------------------------------------------------
static volatile void
   internalError(                   // Internal logic error
     int               line)        // Failing line number
{
   fprintf(stderr, "%s %4d: Internal logic error\n", __SOURCE__, line);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       externalError
//
// Purpose-
//       Handle an external system error.
//
//----------------------------------------------------------------------------
static volatile void
   externalError(                   // External system error
     int               line)        // Failing line number
{
   fprintf(stderr, "%s %4d: ", __SOURCE__, line);
   perror("System error");
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       commandToOffset
//
// Purpose-
//       Convert an Command* to an Offset
//
//----------------------------------------------------------------------------
static Offset                       // Resultant
   commandToOffset(                 // Convert Command* to Offset
     Command*          ptrCommand)  // The Command* to convert
{
   Offset              result;      // Resultant

   result= (char*)ptrCommand - (char*)ssr;
   assert( result < sizeof(SharedStorageRegion) );
   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       offsetToCommand
//
// Purpose-
//       Convert an Offset to a Command*
//
//----------------------------------------------------------------------------
static Command*                     // Resultant
   offsetToCommand(                 // Convert offset to Command*
     Offset            offset)      // The offset to convert
{
   Command*            result;      // Resultant

   assert( offset < sizeof(SharedStorageRegion) );
   result= (Command*)((char*)ssr + offset);
   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       seminit
//
// Purpose-
//       Set the initial semaphore state.
//
//----------------------------------------------------------------------------
static int                          // IGNORED (For debugging)
   seminit(                         // Set initial state
     SemNum            number,      // For this semaphore
     SemInit           state)       // To this state
{
   int                 rc;

   rc= semctl(semSegment, number, SETVAL, state); // Set initial state
   return rc;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       semcall
//
// Purpose-
//       Build a semaphore-related operation.
//
//----------------------------------------------------------------------------
static void
   semcall(                         // Build semaphore operation
     SemNum            number,      // For this semaphore
     int               opcode)      // Using this operation
{
   sembuf              sb;          // Semaphore Buffer

   int                 rc;

   sb.sem_num= number;
   sb.sem_op=  opcode;
   sb.sem_flg= 0;
   rc= semop(semSegment, &sb, 1);
   if( rc == (-1) )
   {
     fprintf(stderr, "%d= semop(%d,%p,1) num(%d) op(%d) flg(0x%x)\n",
                     rc, semSegment, &sb, sb.sem_num, sb.sem_op, sb.sem_flg);
     externalError(__LINE__);
     shouldNotOccur();
   }

// rc= semctl(semSegment, number, GETVAL, 0);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       semWait
//
// Purpose-
//       Wait for a semaphore to be posted.
//
//----------------------------------------------------------------------------
static void
   semWait(                         // Wait
     SemNum            number)      // For this semaphore
{
   semcall(number, -1);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       semPost
//
// Purpose-
//       Post a semaphore.
//
//----------------------------------------------------------------------------
static void
   semPost(                         // Post
     SemNum            number)      // This semaphore
{
   semcall(number, +1);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       allocSSR
//
// Purpose-
//       Allocate the SharedStorageRegion
//
//----------------------------------------------------------------------------
static void*                        // -> SharedStorageRegion
   allocSSR( void )                 // Allocate the SharedStorageRegion
{
   Command*          oldCommand;    // -> Command
   Command*          ptrCommand;    // -> Command
   unsigned          protect;       // Protection flags
   unsigned          remLength;     // Remaining length

   protect= S_IRUSR                 // Allow read access
            | S_IWUSR               // Allow write access
            | IPC_CREAT             // Create the shared segment
            | IPC_EXCL;             // (Exclusively)

   //-------------------------------------------------------------------------
   // Allocate and initialize the semaphores
   //-------------------------------------------------------------------------
   semSegment= semget(fileToken, SEM_COUNT, protect);
   if( semSegment == (-1) )         // If failure
     return NULL;                   // Cannot allocate semaphores

   seminit(SEM_STARTUP, SEM_WAIT);  // STARTUP not posted
   seminit(SEM_TERMINATOR, SEM_WAIT); // TERMINATOR not posted

   //-------------------------------------------------------------------------
   // Allocate the shared segment
   //-------------------------------------------------------------------------
   ssrSegment= shmget(fileToken, sizeof(SharedStorageRegion), protect);
   if( ssrSegment == (-1) )         // If failure
   {
     semctl(semSegment, 0, IPC_RMID); // Delete the semaphores
     return NULL;                   // Cannot allocate segment
   }

   ssr= (SharedStorageRegion*)shmat(ssrSegment, 0, 0); // Attach the segment
   if( (intptr_t)ssr == (-1) )      // If failure
   {
     semctl(semSegment, 0, IPC_RMID);
     shmctl(ssrSegment, IPC_RMID, NULL);
     return NULL;                   // Cannot allocate segment
   }

   //-------------------------------------------------------------------------
   // Initialize the shared segment
   //-------------------------------------------------------------------------
   memset(ssr, 0, sizeof(SharedStorageRegion)); // Zero the region
   memcpy(ssr->ident, __SOURCE__, sizeof(ssr->ident));
   ssr->tokenid=    fileToken;
   ssr->waitforPid= (-1);
   ssr->size=       sizeof(SharedStorageRegion);

   remLength= sizeof(ssr->pool);    // Remaining length
   ptrCommand= (Command*)(ssr->pool); // The free storage pool
   ssr->freeList= commandToOffset(ptrCommand); // The pool offset
   while( remLength >= sizeof(Command) ) // Initialize the free pool
   {
     oldCommand= ptrCommand;
     ptrCommand++;
     oldCommand->next= commandToOffset(ptrCommand);
     remLength -= sizeof(Command);
   }
   oldCommand->next= 0;

   ssr->versionid= SharedStorageRegion::VERSIONID; // Must be last
   return ssr;                      // Segment allocated
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       locateSSR
//
// Purpose-
//       Locate the SharedStorageRegion
//
//----------------------------------------------------------------------------
static void*                        // -> SharedStorageRegion
   locateSSR( void )                // Locate the SharedStorageRegion
{
   unsigned          protect;       // Protection flags

   protect= S_IRUSR                 // Allow read access
            | S_IWUSR;              // Allow write access

   //-------------------------------------------------------------------------
   // Attach the semaphores
   //-------------------------------------------------------------------------
   semSegment= semget(fileToken, SEM_COUNT, protect);
   if( semSegment == (-1) )         // If failure
     return NULL;                   // Cannot allocate semaphores

   //-------------------------------------------------------------------------
   // Attach the shared segment
   //-------------------------------------------------------------------------
   ssrSegment= shmget(fileToken, sizeof(SharedStorageRegion), protect);
   if( ssrSegment == (-1) )         // If failure
     return NULL;                   // Cannot locate segment

   ssr= (SharedStorageRegion*)shmat(ssrSegment, 0, 0); // Attach the segment
   if( (intptr_t)ssr == (-1) )      // If failure
     return NULL;                   // Cannot allocate segment

   //-------------------------------------------------------------------------
   // Verify that we attached the *same* shared segment
   //-------------------------------------------------------------------------
   if( memcmp(ssr->ident, __SOURCE__, sizeof(ssr->ident)) != 0
       || ssr->versionid != SharedStorageRegion::VERSIONID
       || ssr->tokenid != fileToken
       || ssr->size != sizeof(SharedStorageRegion) )
   {
     fprintf(stderr, "Shared segment mismatch\n");
     shmdt(ssr);
     return NULL;
   }

   return ssr;                      // Segment allocated
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       deleteSSR
//
// Purpose-
//       Release the SharedStorageRegion
//
//----------------------------------------------------------------------------
static void
   deleteSSR( void )                // Delete the SharedStorageRegion
{
   locateSSR();                     // Locate the SSR

   if( ssr != NULL )                // If found
   {
     ssr->fsm= SharedStorageRegion::FSM_TERMINATED; // Indicate terminated
     shmdt(ssr);                    // Detach the segment
   }

   if( semSegment != (-1) )         // If semaphores found
     semctl(semSegment, 0, IPC_RMID); // Delete the semaphores

   if( ssrSegment != (-1) )         // If segment found
     shmctl(ssrSegment, IPC_RMID, NULL);
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
   {
     externalError(__LINE__);
     shouldNotOccur();
   }
   pthread_attr_setdetachstate(&attrs, PTHREAD_CREATE_JOINABLE);

   rc= pthread_create(&tid, &attrs, function, parameter);
   if( rc != 0 )
   {
     externalError(__LINE__);
     shouldNotOccur();
   }

   pthread_attr_destroy(&attrs);    // Destroy the attributes
   return tid;                      // Return the thread identifier
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       info
//
// Purpose-
//       Parameter fault exit.
//
//----------------------------------------------------------------------------
static void
   info( void )                     // Parameter fault exit
{
   fprintf(stderr, "schedule filename|token \\\n");
   fprintf(stderr, "    {-boot|-init|-term|-wait| \\\n");
   fprintf(stderr, "    name=\"command\" {dependent-name ...} }\n");
   fprintf(stderr, "\n");
   fprintf(stderr, "-boot\n");
   fprintf(stderr, "  Begin a schedule group.\n");
   fprintf(stderr, "  The schedule group filename MUST be unique.\n");
   fprintf(stderr, "\n");
   fprintf(stderr, "-init\n");
   fprintf(stderr, "  Begin a schedule group.\n");
   fprintf(stderr, "  If the schedule group filename is not unique,\n");
   fprintf(stderr, "  wait for it to complete.\n");
   fprintf(stderr, "\n");
   fprintf(stderr, "-term\n");
   fprintf(stderr, "  Wait for all dependent commands to be scheduled,\n");
   fprintf(stderr, "  then terminate the schedule group.\n");
   fprintf(stderr, "\n");
   fprintf(stderr, "-wait\n");
   fprintf(stderr, "  Wait for all dependent commands to complete,\n");
   fprintf(stderr, "  then terminate the schedule group.\n");
   fprintf(stderr, "\n");
   fprintf(stderr, "name=\"command\" {dependent-name ...}\n");
   fprintf(stderr, "  Wait for all named dependent commands to complete,\n");
   fprintf(stderr, "  then drive the named command.\n");
   fprintf(stderr, "  Use the special name '.' for unnamed commands.\n");
   exit(EXIT_FAILURE);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       parm
//
// Purpose-
//       Parameter analysis
//
//----------------------------------------------------------------------------
static void
   parm(                            // Parameter analysis
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   char*               ptrC;        // Generic pointer to character
   const char*         argp;        // Argument pointer
   int                 argi;        // Argument index
   int                 args;        // Argument string length

   int                 error;       // Error encountered indicator
   int                 verify;      // Verification control

   //-------------------------------------------------------------------------
   // Defaults
   //-------------------------------------------------------------------------
   error= FALSE;                    // Default, no errors found
   verify= 0;                       // Default, no verification

   cmdType= TypeUnspecified;        // Default, no command type
   cmdUsed= 0;                      // No bytes used
   ptrName= NULL;                   // No command name
   ptrCmd=  NULL;                   // No command
   ptrDeps= NULL;                   // No dependencies
   memset(cmdBuff, 0, sizeof(cmdBuff)); // Zero the command buffer

   //-------------------------------------------------------------------------
   // Argument analysis
   //-------------------------------------------------------------------------
   for( argi=1; argi<argc; argi++ ) // Analyze variable controls
   {
     argp= argv[argi];              // Address the parameter

     if( *argp == '-' )             // If this parameter is in switch format
     {
       argp++;                      // Skip over the switch char

       if( swname("verify", argp) ) // If verify switch
         verify= swatob("verify", argp); // Get switch value

       else
       {
         if( cmdType != TypeUnspecified ) // If multiple types
         {
           error= TRUE;             // Invalid command format
           fprintf(stderr, "Duplicate or misplaced control '%s'\n", argv[argi]);
         }
         else
         {
           if( swname("init", argp) )
             cmdType= TypeInit;

           else if( swname("boot", argp) )
             cmdType= TypeBoot;

           else if( swname("term", argp) )
             cmdType= TypeTerm;

           else if( swname("wait", argp) )
             cmdType= TypeWait;

           else
           {
             error= TRUE;           // Invalid command format
             fprintf(stderr, "Invalid  control '%s'\n", argv[argi]);
           }
         }
       }
     }
     else                           // If not a switch
     {
       if( fileName == NULL )       // If filename not specified yet
         fileName= argp;
       else
       {
         if( cmdType != TypeUnspecified
             && cmdType != TypeCommand )
         {
           error= TRUE;             // Invalid command format
           fprintf(stderr, "'%s' not expected\n", argv[argi]);
         }
         else
         {
           args= strlen(argp);      // The length of the command
           if( cmdUsed + args >= sizeof(cmdBuff) - 1 )
           {
             error= TRUE;
             fprintf(stderr, "Command too large, '%s'\n", argp);
             argp= "";
             cmdUsed= 0;
           }

           cmdType= TypeCommand;    // This is a command
           if( ptrName == NULL )    // If this is the name specifier
           {
             ptrName= cmdBuff;
             strcpy(cmdBuff, argp);
             cmdUsed= args+1;

             ptrC= strstr(ptrName, "=");
             if( ptrC != NULL )     // If "name=" format
             {
               *ptrC= '\0';         // Delimit the name
               ptrC++;              // Skip past the (overwritten) '='
               if( *ptrC != '\0' )  // If "name=command" format
               {
                 ptrCmd= ptrC;      // Command specifier specified
                 if( *ptrC == '\'' ||  *ptrC == '\"' ) // If quoted
                 {
                   while( *(ptrC+1) != '\0' ) // Find the last character
                   {
                     ptrC++;
                   }
                   ptrCmd++;        // Skip past the quote start
                   *ptrC= '\0';     // Remove the trailing quote
                 }
               }
             }
           }
           else if( ptrCmd == NULL ) // If this is the command specifier
           {
             ptrCmd= &cmdBuff[cmdUsed];
             strcpy(ptrCmd, argp);
             cmdUsed += args+1;
           }
           else if( ptrDeps == NULL ) // If this is the first dependency
           {
             ptrDeps= &cmdBuff[cmdUsed];
             strcpy(ptrDeps, argp);
             cmdUsed += args;
           }
           else                       // If this is another dependency
           {
             strcat(ptrDeps, " ");
             strcat(ptrDeps, argp);
             cmdUsed += args;
           }
         }
       }
     }
   }

   //-------------------------------------------------------------------------
   // Completion analysis
   //-------------------------------------------------------------------------
   if( fileName == NULL )           // If no file specified
   {
     error= TRUE;
     fprintf(stderr, "No filename specified\n");
   }
   else
   {
     fileToken= ftok(fileName, 0xfe010510);
     if( fileToken == (-1) )
     {
       error= TRUE;
       fprintf(stderr, "File(%s) non-existant\n", fileName);
     }
   }

   if( !error                       // If no error encountered already
       && cmdType == TypeCommand    // And this is a command
       && ptrCmd == NULL )          // But no command was specified
   {
     error= TRUE;
     fprintf(stderr, "No command specified\n");
   }

   if( error )                      // If error encountered
     info();

   if( verify )
   {
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       init
//
// Purpose-
//       Initialize.
//
//----------------------------------------------------------------------------
static void
   init( void )                     // Initialize
{
   switch( cmdType )
   {
     case TypeBoot:
     case TypeInit:
       allocSSR();
       if( ssr == NULL )
         break;

       if( cmdType == TypeBoot )
         ssr->fsm= SharedStorageRegion::FSM_BOOT;
       else
         ssr->fsm= SharedStorageRegion::FSM_INIT;
       break;

     case TypeTerm:
     case TypeWait:
     case TypeCommand:
       locateSSR();
       break;

     default:
       internalError(__LINE__);
       shouldNotOccur();
       break;
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       commandState
//
// Purpose-
//       Examine a single dependency.
//
//----------------------------------------------------------------------------
static Command::FSM                 // The state of the named command
   commandState(                    // Examine a dependency
     char*             dependency)  // The dependency to examine
{
   Command*            ptrCommand;  // -> Command
   Offset              offset;      // Command offset
   char*               ptrC;        // -> Generic character

   offset= ssr->activeList;         // -> Dependency list
   while( offset != 0 )             // Search for dependency
   {
     ptrCommand= offsetToCommand(offset);

     ptrC= (char*)ssr + ptrCommand->name;
     if( strcmp(ptrC, dependency) == 0 )
       return Command::FSM(ptrCommand->fsm);

     offset= ptrCommand->next;
   }

   return Command::FSM_COMPLETE;    // No such dependency
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       commandReady
//
// Purpose-
//       Examine a command's dependencies.
//
//----------------------------------------------------------------------------
static int                          // TRUE if schedulable
   commandReady(                    // Examine a command's dependencies
     Command*          inpCommand)  // The command to examine
{
   char                dependent[Command::CMD_SIZE]; // Dependent process name
   char*               ptrDep;      // -> Dependent

   ptrDep= NULL;                    // Default, no dependencies
   if( inpCommand->deps != 0 )      // If dependencies exist
     ptrDep= (char*)ssr + inpCommand->deps; // Address the dependency list

   while( ptrDep != NULL )          // Examine dependencies
   {
     strcpy(dependent, ptrDep);     // Copy the dependency list
     if( strstr(dependent, " ") != NULL )
     {
       *strstr(dependent, " ")= '\0';
       ptrDep= strstr(ptrDep, " ");
       ptrDep++;
     }
     else
       ptrDep= NULL;

     if( commandState(dependent) != Command::FSM_COMPLETE )
       return FALSE;
   }

   return TRUE;                     // All dependencies are satisfied
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       commandDriver
//
// Purpose-
//       Drive a command.
//
//----------------------------------------------------------------------------
static pid_t                        // The process identifier
   commandDriver(                   // Drive a command
     Command*          inpCommand)  // The command to drive
{
   char*               ptrC;        // -> Generic character
   pid_t               pid;         // Scheduled process id

   //-------------------------------------------------------------------------
   // Drive the command
   //-------------------------------------------------------------------------
   ptrC= (char*)ssr + inpCommand->code;

   inpCommand->fsm= Command::FSM_ACTIVE;
   pid= fork();
   if( pid != 0 )
   {
     inpCommand->pid= pid;
     if( pid < 0 )
       inpCommand->fsm= Command::FSM_WAITING;
     return pid;
   }

   ptrC= (char*)ssr + inpCommand->code;
   execl("/usr/bin/ksh", "-c", ptrC, NULL);

   fprintf(stderr, "Return from execl: %s\n", ptrC);
   exit(EXIT_FAILURE);
   return pid;                      // (Prevents compiler message)
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       schedule
//
// Purpose-
//       Examine a commands dependencies and schedule it if possible.
//
//----------------------------------------------------------------------------
static int                          // TRUE if scheduled
   schedule(                        // Schedule a command
     Command*          inpCommand)  // The command to schedule
{
   //-------------------------------------------------------------------------
   // Check dependencies
   //-------------------------------------------------------------------------
   if( !commandReady(inpCommand) )  // If not ready to go
     return FALSE;                  // Return, not ready

   //-------------------------------------------------------------------------
   // Drive the command
   //-------------------------------------------------------------------------
   commandDriver(inpCommand);
   return TRUE;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       specialProc
//
// Purpose-
//       Drive the "special" process.
//
//----------------------------------------------------------------------------
static pid_t                        // The "special" process identifier
   specialProc( void )              // Drive the "special" process
{
   pid_t               pid;         // Process identifier

   pid= fork();                     // Create the special process
   if( pid < 0 )                    // If failure
   {
     fprintf(stderr, "Unable to create subprocess\n");
     externalError(__LINE__);
     shouldNotOccur();
   }

   if( pid != 0 )                   // If this is the parent
     return pid;                    // Return the process identifier

   //-------------------------------------------------------------------------
   // Not a lot of function here
   //-------------------------------------------------------------------------
   sleep(600);                      // Or until a signal is received
   exit(EXIT_SUCCESS);
   return 0;                        // (Prevents compiler message)
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       startupThread
//
// Purpose-
//       Accept and schedule new commands.
//
//----------------------------------------------------------------------------
static void*                        // Return value (always NULL)
   startupThread(                   // Startup control thread
     void*             parm)        // Parameter
{
#if 0
   Offset              oldValue, newValue; // Used in csw
   pid_t               controlPid;  // Special process identifier

   Offset              offset;      // Command offset
   Command*            ptrCommand;  // -> Command

   int                 rc;

   for(;;)                          // Process commands
   {
     if( ssr->fsm == SharedStorageRegion::FSM_TERM
         || ssr->fsm == SharedStorageRegion::FSM_WAIT )
       break;

     semWait(SEM_STARTUP);          // Wait for semaphore

     if( ssr->fsm == SharedStorageRegion::FSM_TERM
         || ssr->fsm == SharedStorageRegion::FSM_WAIT )
       break;

     for(;;)                        // Remove the unseen commands
     {
       offset= ssr->unseenList;     // Unseen list
       if( offset == 0 )            // If no unseen commands
         break;                     // Nothing to do

       ptrCommand= offsetToCommand(offset);
       oldValue= offset;            // Old value
       newValue= 0;                 // New value
       rc= csw((int*)&ssr->unseenList, oldValue, newValue);
       if( rc == 0 )
         break;
     }

     if( offset == 0 )              // If no unseen commands
       continue;                    // Nothing to do

     for(;;)                        // Pass the commands to waitfor
     {
       rc= csw((int*)&commands, 0, offset);
       if( rc == 0 )
       {
         controlPid= specialPid;
         specialPid= (-1);
         kill(controlPid, SIGKILL);
         break;
       }

       sleep(1);                    // waitfor needs time to think
       if( ssr->fsm == SharedStorageRegion::FSM_TERM
           || ssr->fsm == SharedStorageRegion::FSM_WAIT )
         break;
     }
   }

   while( specialPid < 0 )
     sleep(1);
   terminated= TRUE;
   controlPid= specialPid;
   specialPid= (-1);
   kill(controlPid, SIGKILL);

#endif
   return NULL;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       waitforThread
//
// Purpose-
//       Wait for the completion of processes
//
//----------------------------------------------------------------------------
static void*                        // Return value (always NULL)
   waitforThread(                   // Waitfor control thread
     void*             parm)        // Parameter
{
   Offset              offset;      // Command offset
   pid_t               controlPid;  // Special process identifier
   pid_t               pid;         // Process identifier
   Command*            ptrCommand;  // -> Command
   int                 pendInit;    // TRUE if jobs need to start
   int                 pendTerm;    // TRUE if jobs need to complete
   int                 status;      // Completion status

   int                 rc;

   controlPid= specialProc();       // Drive the "special" process
   ssr->waitforPid= controlPid;
   for(;;)                          // Process commands
   {
     pid= wait(&status);            // Wait for any child to complete

     //-----------------------------------------------------------------------
     // Redrive the specialProc, if required.
     //-----------------------------------------------------------------------
     if( pid == controlPid )
     {
       controlPid= specialProc(); // Redrive the special process
       ssr->waitforPid= controlPid;
     }

     //-----------------------------------------------------------------------
     // Look for completed processes
     //-----------------------------------------------------------------------
     offset= ssr->activeList;       // The list of active processes
     while( offset != 0 )
     {
       ptrCommand= offsetToCommand(offset);
       if( ptrCommand->fsm == Command::FSM_ACTIVE )
       {
         if( pid == ptrCommand->pid )
         {
           ptrCommand->fsm= Command::FSM_COMPLETE;
           ptrCommand->compCode= status;
           break;
         }
       }

       offset= ptrCommand->next;
     }

     //-----------------------------------------------------------------------
     // Look for schedulable commands
     //-----------------------------------------------------------------------
     pendInit= FALSE;               // Default, nothing needs scheduling
     pendTerm= FALSE;               // Default, nothing outstanding
     offset= ssr->activeList;       // The list of active processes
     while( offset != 0 )
     {
       ptrCommand= offsetToCommand(offset);
       if( ptrCommand->fsm == Command::FSM_RESET
           || ptrCommand->fsm == Command::FSM_WAITING )
       {
         pendTerm= TRUE;            // This job hasn't completed yet
         rc= schedule(ptrCommand);
         if( !rc )
           pendInit= TRUE;          // This job hasn't started yet
       }
       else if( ptrCommand->fsm == Command::FSM_ACTIVE )
         pendTerm= TRUE;            // This job hasn't completed yet

       offset= ptrCommand->next;
     }

     //-----------------------------------------------------------------------
     // Status analysis
     //-----------------------------------------------------------------------
     if( pendInit )                 // If any job hasn't started yet
       continue;
     if( ssr->fsm == SharedStorageRegion::FSM_TERM ) // If waiting for schedule
       break;

     if( pendTerm )                 // If any job hasn't terminated yet
       continue;
     if( ssr->fsm == SharedStorageRegion::FSM_WAIT ) // If waiting for jobs
       break;
   }

   ssr->waitforPid= (-1);
   kill(controlPid, SIGKILL);
   return NULL;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       initProcess
//
// Purpose-
//       The initProcess runs the startupThread and the waitforThread.
//
//----------------------------------------------------------------------------
static void
   initProcess( void )              // Child of -boot or -init
{
   pthread_t           startupTid;  // Thread identifier
   pthread_t           waitforTid;  // Thread identifier

   waitforTid= startThread(&waitforThread, NULL);
   startupTid= startThread(&startupThread, NULL);

   pthread_join(startupTid, 0);
   pthread_join(waitforTid, 0);

   // InitProcess termination
   ssr->fsm= SharedStorageRegion::FSM_TERMINATED;
   semPost(SEM_TERMINATOR);
   exit(EXIT_SUCCESS);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       main
//
// Purpose-
//       Mainline code
//
//----------------------------------------------------------------------------
int                                 // Main return code
   main(                            // Mainline code
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   Offset              offset;      // Command offset
   Command*            ptrCommand;  // -> Command
   pid_t               pid;         // Process identifier

   int                 rc;

   //-------------------------------------------------------------------------
   // Parameter analysis
   //-------------------------------------------------------------------------
   parm(argc, argv);

   //-------------------------------------------------------------------------
   // Initialize
   //-------------------------------------------------------------------------
   init();

   //-------------------------------------------------------------------------
   // Handle the request
   //-------------------------------------------------------------------------
   if( ssr == NULL )                // If startup failure
   {
     if( cmdType == TypeBoot || cmdType == TypeInit )
       fprintf(stderr, "No shared storage\n");
     else
       fprintf(stderr, "Not initialized\n");
     shouldNotOccur();
   }

   switch( cmdType )
   {
     case TypeBoot:
     case TypeInit:
       pid= fork();                 // Create a child process
       if( pid < 0 )                // If startup failure
       {
         fprintf(stderr, "Unable to create child process\n");

         deleteSSR();               // Delete the SharedStorageRegion
         shouldNotOccur();          // This condition should not occur
       }

       if( pid != 0 )               // If this is the parent
         break;                     // Nothing more to do

       initProcess();               // Drive the init process
       break;

     case TypeTerm:
     case TypeWait:
       if( cmdType == TypeWait )
         ssr->fsm= SharedStorageRegion::FSM_WAIT; // Indicate waiting
       else
         ssr->fsm= SharedStorageRegion::FSM_TERM; // Indicate terminated

       semPost(SEM_STARTUP);           // Notify the startup task
       semWait(SEM_TERMINATOR);        // Wait for initProc completion
       deleteSSR();
       break;

     case TypeCommand:
       for(;;)                      // Allocate a command buffer
       {
         offset= ssr->freeList;
         if( offset == 0 )
         {
           fprintf(stderr, "Not enough shared storage\n");
           shouldNotOccur();
         }

         ptrCommand= offsetToCommand(offset);
         rc= csw((int*)&ssr->freeList, offset, ptrCommand->next);
         if( rc == 0 )
           break;
       }

       ptrCommand->fsm= Command::FSM_WAITING;
       memcpy(ptrCommand->command, cmdBuff, sizeof(cmdBuff));

       offset= commandToOffset(ptrCommand); // Base offset
       ptrCommand->name= offset + (intptr_t)((Command*)NULL)->command +
                         (ptrName - cmdBuff);
       ptrCommand->code= offset + (intptr_t)((Command*)NULL)->command +
                         (ptrCmd  - cmdBuff);
       ptrCommand->deps= 0;         // Default, no dependencies
       if( ptrDeps != NULL )
         ptrCommand->deps= offset + (intptr_t)((Command*)NULL)->command +
                           (ptrDeps - cmdBuff);

       for(;;)                      // Enqueue the command
       {
         ptrCommand->next= ssr->activeList;
         rc= csw((int*)&ssr->activeList, ptrCommand->next, offset);
         if( rc == 0 )
           break;
       }

       for(;;)                      // Drive the WaitforThread
       {
         pid= ssr->waitforPid;      // The waiting process identifier
         rc= csw((int*)&ssr->waitforPid, pid, (-1));
         if( rc == 0 )
           break;
       }
       if( pid != (-1) )            // If we are the thread driver
         kill(pid, SIGKILL);        // Drive the WaitforThread

       break;

     default:
       internalError(__LINE__);
       shouldNotOccur();
       break;
   }

   return 0;
}

