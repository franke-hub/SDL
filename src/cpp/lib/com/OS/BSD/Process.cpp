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
//       OS/BSD/Process.cpp
//
// Purpose-
//       BSD Instantiate Process methods.
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

#if defined(_OS_BSD)
#include <sys/wait.h>               // for waitpid
#include <sys/signal.h>             // for kill
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
#define SCDM                        // If defined, Soft-Core Debug Mode
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

   pid_t               pid;         // Process identifier
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
     printf("%p= ::allocateObject()\n", object);
   #endif

   return object;
}

//----------------------------------------------------------------------------
//
// Method-
//       ::parse
//
// Purpose-
//       Parse a string into components.
//
//----------------------------------------------------------------------------
static int                          // Number of components
   parse(                           // Parse a string
     char*             string,      // The parameter string
     char**            vector)      // -> Parameter vector (may be NULL)
{
   char*               delimiter;   // Closing delimiter
   unsigned            argx;        // Argument index

   #ifdef HCDM
     printf(">>parse(%s, %p)\n", string, vector);
   #endif

   argx= 0;                         // Initial argument index
   for(;;)                          // Parse the string
   {
     while( (*string) == ' ' || (*string) == '\t' ) // Skip leading white space
       string++;

     if( (*string) == '\0' )        // If end of string
       break;

     delimiter= NULL;               // Default, not quoted
     if( (*string) == '\'' || (*string) == '\"' )
     {
       delimiter= string;           // Save the delimiter
       string++;
     }

     if( vector != NULL )           // If the parameter vector is specified
       vector[argx]= string;
     argx++;

     if( delimiter != NULL )        // If quoted string
     {
       while( (*string) != (*delimiter) )
       {
         if( (*string) == '\\' )    // If escape
           string++;                // Skip the escape

         if( (*string) == '\0' )    // If invalid string
           return (-1);             // Error, invalid string

         if( (*string) == '\r' || (*string) == '\n' ) // If invalid character
           return (-1);             // Error, invalid string

         string++;                  // Skip the character
       }

       if( vector != NULL )
         *string= '\0';

       string++;
       if( (*string) != ' ' && (*string) != '\t' && (*string) != '\0' )
         return (-1);               // Error, invalid string

       continue;
     }

     while( (*string) != ' ' && (*string) != '\t' && (*string) != '\0' )
     {
       if( (*string) == '\r' || (*string) == '\n' ) // If invalid character
         return (-1);               // Error, invalid string

       if( (*string) == '\\' )      // If escape
       {
         string++;                  // Skip the escape
         if( (*string) == '\0' )    // If invalid string
           return (-1);             // Error, invalid string
         if( (*string) == '\r' || (*string) == '\n' ) // If invalid character
           return (-1);             // Error, invalid string

         string++;                  // Skip the escaped character
         continue;
       }

       if( (*string) == '\'' || (*string) == '\"' )
         return (-1);               // Error, invalid string

       string++;                    // Skip the character
     }

     if( (*string) == '\0' )        // If end of string
       break;

     if( vector != NULL )
       *string= '\0';

     string++;
   }

   if( vector != NULL )             // If the parameter vector is specified
     vector[argx]= NULL;            // Ending argument delimiter
   return argx;
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
   #ifdef HCDM
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

   #ifdef HCDM
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

   int                 rc;          // Called routine return code

   #ifdef HCDM
     printf("%s= Process(%p)::signal(%d)\n", "........", this, sid);
   #endif

   rc= kill(attr.pid, sid);
   if( rc != 0 )
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
   char                dummyList;   // Empty parameter list

   char                buffer[FILENAME_MAX+1];
   char*               functionCopy;// Copy(functionName)
   char*               parameterCopy; // Copy(parameterList)

   char**              vector;      // -> Parameter list

   int                 rc;          // Called routine return code

   #ifdef HCDM
     printf("%s= Process(%p)::start(%s,%s)\n", "........", this,
            functionName, parameterList);
   #endif

   if( attr.fsm != Attribute::State_Initial )
   {
     fprintf(stderr, "%s %d: Process(%p)::start State(%d) error\n",
                     __SOURCE__, __LINE__,
                     this, attr.fsm);
     abort();
   }
   attr.fsm= Attribute::State_Operating;

   rc= fork();                      // fork() with exec required
   #ifdef HCDM
     printf("%s %d: %d= fork()\n", __SOURCE__, __LINE__, rc);
   #endif
   if( rc < 0 )                     // If no child created
   {
     fprintf(stderr, "%s %d: ", __SOURCE__, __LINE__);
     perror("System error");
     abort();
   }

   if( rc != 0 )                    // If this is the parent
   {
     attr.pid= rc;                  // Save the process identifier
     return;
   }

   // This is the child process -- run it
   if( parameterList == NULL )
   {
     dummyList= '\0';
     parameterCopy= &dummyList;
   }
   else
   {
     parameterCopy= (char*)malloc(strlen(parameterList)+1);
     if( parameterCopy == NULL )
     {
       fprintf(stderr, "%s %d: ", __SOURCE__, __LINE__);
       perror("Storage shortage");
       abort();
     }
     strcpy(parameterCopy, parameterList);
   }
   #ifdef HCDM
     printf("%s %d: parameterCopy(%s)\n", __SOURCE__, __LINE__, parameterCopy);
   #endif

   #ifdef HCDM
     printf("%s %d: child\n", __SOURCE__, __LINE__);
   #endif

   if( FileName::resolve(buffer, functionName) != NULL )
   {
     fprintf(stderr, "%s %d: Cannot Exec(%s)\n",
                     __SOURCE__, __LINE__, functionName);
     abort();
   }
   functionCopy= buffer;

   #ifdef HCDM
     printf("%s %d: functionCopy(%s)\n", __SOURCE__, __LINE__, functionCopy);
   #endif
   rc= parse(parameterCopy, NULL);  // Count the parameters
   if( rc < 0 )                     // If invalid parameter list
   {
     fprintf(stderr, "%s %d: Invalid parameter list(%s)\n",
                     __SOURCE__, __LINE__, parameterList);
     abort();
   }

   vector= (char**)malloc(sizeof(char*)*(rc+2)); // Allocate the parameter list
   if( vector == NULL )
   {
     fprintf(stderr, "%s %d: ", __SOURCE__, __LINE__);
     perror("Storage shortage");
     abort();
   }

   #ifdef HCDM
     printf("%s %d: before execvp(%s) '%s'\n", __SOURCE__, __LINE__,
            functionCopy, parameterCopy);
   #endif
   vector[0]= functionCopy;
   parse(parameterCopy, &vector[1]);
   execvp(functionCopy, vector);

   // The process should not return
   fprintf(stderr, "%s %d: %s(%s)\n", __SOURCE__, __LINE__,
                   functionCopy, parameterList);
   perror("System error");
   abort();
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

   #ifdef HCDM
     printf("%s= Process(%p)::wait()\n", "........", this);
   #endif

   if( attr.fsm != Attribute::State_Operating )
   {
     fprintf(stderr, "%s %d: Process(%p)::wait State(%d) error\n",
                     __SOURCE__, __LINE__,
                     this, attr.fsm);
     abort();
   }

   waitpid(attr.pid, &attr.compCode, 0);
   attr.fsm= Attribute::State_Initial;

   return attr.compCode;
}

