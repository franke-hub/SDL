//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       OS/WIN/Process.cpp
//
// Purpose-
//       Instantiate Process methods.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <windows.h>
#ifdef _OS_WIN
#include <direct.h>
#endif

#include <com/FileName.h>

#include "com/Process.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "PROCESS " // Source filename

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard-Core Debug Mode
#endif

#ifndef SCDM
#undef  SCDM                        // If defined, Soft-Core Debug Mode
#endif

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE  1
#endif

//----------------------------------------------------------------------------
//
// Struct-
//       Attribute
//
// Purpose-
//       Define the hidden Process attributes.
//
//----------------------------------------------------------------------------
struct Attribute                    // Hidden attributes
{
enum State                          // The State of the Process
{
   State_Initial= 0,                // Initial state
   State_Operating                  // Operating (Running) State
}                      fsm;         // The State of the Process
   int                 compCode;    // The completion code

   PROCESS_INFORMATION processInfo; // WIN Process information
};

//----------------------------------------------------------------------------
//
// Method-
//       ::allocateObject
//
// Purpose-
//       Allocate space for the hidden attributes.
//
//----------------------------------------------------------------------------
static Attribute*                   // -> Attribute
   allocateObject( void )           // Allocate the hidden attributes
{
   Attribute*          object;

   object= (Attribute*)malloc(sizeof(Attribute));
   if( object == NULL )
   {
     fprintf(stderr, "%s %d: Storage shortage\n", __SOURCE__, __LINE__);
     exit(EXIT_FAILURE);
   }
   memset(object, 0, sizeof(Attribute));

   #ifdef HCDM
     printf("%p= Process::allocateObject()\n", object);
   #endif

   return object;
}

//----------------------------------------------------------------------------
//
// Method-
//       ::displayAttribute
//
// Purpose-
//       Display a Attribute
//
//----------------------------------------------------------------------------
static void
   displayattribute(                // Display a Attribute
     Attribute&        object)      // The Attribute to display
{
   #ifdef HCDM
     printf("%s= Process::Attribute(%p)\n", "........", &object);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       Process::~Process
//
// Purpose-
//       Destructor
//
//----------------------------------------------------------------------------
   Process::~Process( void )        // Destructor
{
   #ifdef SCDM
     printf("%s= Process(%p)::~Process()\n", "........", this);
   #endif

   if( attr != NULL )
   {
     free(attr);
     attr= NULL;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Process::Process
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
   Process::Process( void )         // Constructor
:  attr(NULL)
{
   Attribute*          attr;        // -> Attribute

   #ifdef SCDM
     printf("%s= Process(%p)::Process()\n", "........", this);
   #endif

   attr= ::allocateObject();        // Allocate the hidden attribute

   attr->fsm= Attribute::State_Initial; // Set initial State

   this->attr= attr;                // Set the Attribute
}

//----------------------------------------------------------------------------
//
// Method-
//       Process::signal
//
// Purpose-
//       Signal the Process.
//
//----------------------------------------------------------------------------
void
   Process::signal(                 // Signal the Process
     int               sid)         // Using this signal identifier
{
   Attribute&          attr= *(Attribute*)(this->attr); // Get the attribute

   int                 rc;          // Called routine's return code

   #ifdef SCDM
     printf("%s= Process(%p)::signal(0x%.8x)\n", "........", this, sid);
   #endif

   if( attr.fsm != Attribute::State_Operating )
   {
     fprintf(stderr, "%s %d: Process(%p)::signal State(%d) error\n",
                     __SOURCE__, __LINE__,
                     this, attr.fsm);
     abort();
   }

   rc= ::TerminateProcess(attr.processInfo.hProcess, sid);
   if( rc == 0 )
   {
     fprintf(stderr, "%s %d: ", __SOURCE__, __LINE__);
     perror("System error");
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Process::start
//
// Purpose-
//       Start the Process.
//
//----------------------------------------------------------------------------
void
   Process::start(                  // Start the Process
     const char*       functionName,// Name of the function to execute
     const char*       parameterList) // Parameter list string
{
   Attribute&          attr= *(Attribute*)(this->attr); // Get the attribute
   SECURITY_ATTRIBUTES securityAttributes; // Security attributes
   STARTUPINFO         startupInfo; // Startup information

   char                functionBuff[FILENAME_MAX+1]; // Resolved function name
   char*               functionCopy;// Copy(functionName)
   FILE*               functionFile;// Temporary File*
   char*               parameterCopy; // Copy(parameterList)

   int                 rc;          // Called routine return code

   #ifdef SCDM
     printf("%s= Process(%p)::start(%s)\n", "........", this, functionName);
   #endif

   if( attr.fsm != Attribute::State_Initial )
   {
     fprintf(stderr, "%s %d: Process(%p)::start State(%d) error\n",
                     __SOURCE__, __LINE__,
                     this, attr.fsm);
     abort();
   }
   attr.fsm= Attribute::State_Operating;

   // Initialize the security attributes
   securityAttributes.nLength= sizeof(securityAttributes);
   securityAttributes.lpSecurityDescriptor= NULL;
   securityAttributes.bInheritHandle= TRUE;

   // Initialize the startup info
   memset(&startupInfo, 0, sizeof(startupInfo));

   // Copy the command name
   if( FileName::resolve(functionBuff,functionName) != NULL )
   {
     fprintf(stderr, "%s %d: Cannot Exec(%s)\n",
                     __SOURCE__, __LINE__, functionName);
     abort();
   }
   functionCopy= functionBuff;

   #ifdef _OS_CYGWIN
   {{{{
     int           i;

     if( functionCopy[0] == '/' && functionCopy[1] == '/' )
     {
       functionCopy[0]= functionCopy[2];
       functionCopy[1]= ':';

       strcpy(&functionCopy[2], &functionCopy[3]);
     }

     for(i=0; functionCopy[i] != '\0'; i++)
     {
       if( functionCopy[i] == '/' )
         functionCopy[i]= '\\';
     }
   }}}}
   #endif

   // If the function doesn't exist, try it with .exe
   functionFile= fopen(functionCopy, "rb");
   if( functionFile == NULL )
   {
     strcat(functionCopy, ".exe");
//// functionFile= fopen(functionCopy, "rb");
   }
   if( functionFile != NULL )
     fclose(functionFile);

   // Copy the command line
   parameterCopy= (char*)malloc(strlen(functionCopy)+1
                               +strlen(parameterList)+3);
   if( parameterCopy == NULL )
   {
     fprintf(stderr, "%s %d: ", __SOURCE__, __LINE__);
     perror("Storage shortage");
     abort();
   }
   sprintf(parameterCopy, "\"%s\"", functionCopy);
   strcat(parameterCopy, " ");
   strcat(parameterCopy, parameterList);

   #ifdef HCDM
     printf("%4d: %s: fName(%s) pList(%s)\n", __LINE__, __SOURCE__,
            functionCopy, parameterCopy);
   #endif

   // Start the Process
   rc= ::CreateProcess(             // Create the Process
           functionCopy,            // The function name
           parameterCopy,           // The parameter list
           &securityAttributes,     // Process security attributes
           &securityAttributes,     // Thread  security attributes
           TRUE,                    // Handles are inherited
           CREATE_DEFAULT_ERROR_MODE, // Creation flags
           NULL,                    // Copy the environment
           NULL,                    // Copy the current directory
           &startupInfo,            // Dummy STARTUPINFO
           &attr.processInfo);      // -> Process information

   if( !rc )                        // If startup failure
   {
     rc= ::GetLastError();
     fprintf(stderr, "%s %d: Process(%p)::start(%s): error(%d) ",
                     __SOURCE__, __LINE__, this, functionCopy, rc);
     errno= rc;
     perror("System error");
     abort();
   }

   free(parameterCopy);
}

//----------------------------------------------------------------------------
//
// Method-
//       Process::wait
//
// Purpose-
//       Wait for Process to complete
//
//----------------------------------------------------------------------------
int                                 // Return code
   Process::wait( void )            // Wait for Process to complete
{
   Attribute&          attr= *(Attribute*)(this->attr); // Get the attribute

   DWORD               waitCode;    // Completion code

   if( attr.fsm != Attribute::State_Operating )
   {
     fprintf(stderr, "%s %d: Process(%p)::wait State(%d) error\n",
                     __SOURCE__, __LINE__,
                     this, attr.fsm);
     abort();
   }

   for(;;)                          // Wait for Process completion
   {
     waitCode= WaitForSingleObject(attr.processInfo.hProcess, INFINITE);
     if (waitCode != WAIT_TIMEOUT)
       break;
   }
   attr.fsm= Attribute::State_Initial;

   return waitCode;
}

