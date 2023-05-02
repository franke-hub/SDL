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
//       ParseINI.cpp
//
// Purpose-
//       File parameter controls.
//
// Last change date-
//       2020/10/03
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <com/sysmac.h>

#include "com/ParseINI.h"

//----------------------------------------------------------------------------
//
// Struct-
//       Parameter
//
// Purpose-
//       Define a parameter entry.
//
//----------------------------------------------------------------------------
struct Parameter {
   Parameter*          nextParm;    // -> Next Parameter
   char*               parmValue;   // This parameter's value
   char                parmName[1]; // This parameter's name
};

//----------------------------------------------------------------------------
//
// Struct-
//       Section
//
// Purpose-
//       Define a section entry.
//
//----------------------------------------------------------------------------
struct Section {
   Section*            nextSect;    // -> Next Section
   Parameter*          headParm;    // -> First Parameter
   char                sectName[1]; // This Section's name
};

//----------------------------------------------------------------------------
//
// Struct-
//       Object
//
// Purpose-
//       Define the ParseINI object.
//
//----------------------------------------------------------------------------
struct Object {
   Section*            headSect;    // The first Section entry
   Section*            tailSect;    // The last  Section entry
};

//----------------------------------------------------------------------------
//
// Subroutine-
//       error
//
// Purpose-
//       Write error message.
//
//----------------------------------------------------------------------------
static void
   error(                           // Handle error
     const char*       fileName,    // The parameter file name
     long              lineNumber,  // The line number
     const char*       message)     // The error message
{
   fprintf(stderr, "ParseINI File(%s) Line(%ld) %s\n", fileName, lineNumber,
                   message);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       skipBlanks
//
// Purpose-
//       Skip blanks in the parameter file.
//
//----------------------------------------------------------------------------
static int                          // The next non-blank character
   skipBlanks(                      // Skip blanks
     FILE*             handle)      // The file handle
{
   int                 c;           // The current character

   c= ' ';                          // Default, blank character
   while( c == ' ' )                // Skip blanks
   {
     c= fgetc(handle);
   }

   return c;                        // Return the non-blank
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       skipToEndOfLine
//
// Purpose-
//       Skip to end of line (or end of file).
//
//----------------------------------------------------------------------------
static int                          // The delimiting character
   skipToEndOfLine(                 // Skip to end of line
     FILE*             handle)      // The file handle
{
   int                 c;           // The current character

   c= ' ';                          // Default, blank character
   while( c != '\n' && c != EOF )   // Skip to end of line
   {
     c= fgetc(handle);
   }

   return c;                        // Return the delimiter
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       ParseINI::construct
//
// Purpose-
//       In-situ constructor.
//
//----------------------------------------------------------------------------
void
   ParseINI::construct( void )      // Construct the ParseINI object
{
   if( object != NULL )             // If already constructed
     return;                        // Once is enough

   object= (void*)malloc(sizeof(Object)); // Allocate the object
   if( object == NULL )             // If allocation failed
     return;                        // Return, no object

   memset(object, 0, sizeof(Object)); // Initialize the object
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       ParseINI::destroy
//
// Purpose-
//       In-situ destructor.
//
//----------------------------------------------------------------------------
void
   ParseINI::destroy( void )        // Destroy the ParseINI object
{
   Object*             O;           // -> Object
   Section*            ptrSect;     // -> Section
   Parameter*          ptrParm;     // -> Parameter

   O= (Object*)object;              // -> Object
   if( O == NULL )                  // If already destroyed
     return;                        // Once is enough

   while( O->headSect != NULL )     // Delete the sections
   {
     ptrSect= O->headSect;          // Address the top Section
     while( ptrSect->headParm != NULL ) // Delete the parameters
     {
       ptrParm= ptrSect->headParm;  // Address the top element
       ptrSect->headParm= ptrParm->nextParm; // Remove it
       free(ptrParm);               // Delete it
     }

     O->headSect= ptrSect->nextSect;// Remove the Section
     free(ptrSect);                 // Delete the Section
   }

   free(O);                         // Delete the object
   object= NULL;                    // Indicate deleted
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       ParseINI::open
//
// Purpose-
//       Open the parameter file.
//
//----------------------------------------------------------------------------
void
   ParseINI::open(                  // Open the parameter file
     const char*       parmFile)    // The parameter file name
{
   Object*             O;           // -> Object
   Section*            ptrSect;     // -> Section
   Parameter*          ptrPrev;     // -> Previous Parameter
   Parameter*          ptrParm;     // -> Parameter
   FILE*               handle;      // The file handle
   char*               p;           // Generic char pointer
   int                 c;           // The next character
   int                 nSize;       // Sizeof (parmName)
   int                 vSize;       // Sizeof (parmValu)
   int                 pSize;       // Sizeof (Parameter)
   long                lineNumber= 0; // The current file line number

   char                parmName[MAXSIZE+1]; // Parameter name work area
   char                parmValu[MAXSIZE+1]; // Parameter value work area

   O= (Object*)object;              // -> Object
   if( O == NULL )                  // If object not properly constructed
   {
     construct();                   // Construct it now
     O= (Object*)object;            // -> Object
   }

   if( O == NULL )                  // If object not properly constructed
   {
     error(parmFile, 0, "No storage available");
     return;                        // Nothing to open
   }

   if( O->headSect != NULL )        // If some file already opened
     return;                        // Only one file is allowed

   handle= fopen(parmFile, "rb");   // Open the parameter file
   if( handle == NULL )             // If open failure
     return;                        // No parameters exist

   ptrSect= (Section*)malloc(sizeof(Section));
   if( ptrSect == NULL )            // If allocation failed
   {
     error(parmFile, lineNumber, "No storage");
     fclose(handle);                // Close the file
     return;
   }
   ptrSect->nextSect= NULL;         // Initialize the NULL section
   ptrSect->headParm= NULL;
   ptrSect->sectName[0]= '\0';

   O->headSect= ptrSect;
   O->tailSect= ptrSect;

   //-------------------------------------------------------------------------
   // Handle new file line
   //-------------------------------------------------------------------------
newLine:                            // Process a new line
   lineNumber++;                    // Increment the line number
   c= skipBlanks(handle);           // Skip over blanks
   while( c == '\r' )               // Ignore carriage returns
     c= skipBlanks(handle);
   if( c == '\n' )
     goto newLine;

   if( c == ';' )                   // If comment
   {
     c= skipToEndOfLine(handle);    // Ignore the line
     if( c == EOF )
         goto endOfFile;
     goto newLine;
   }

   if( c == EOF )                   // If end of file
     goto endOfFile;

   //-------------------------------------------------------------------------
   // Handle parameter/section name
   //-------------------------------------------------------------------------
   nSize= 0;                        // Begin parameter name
   if( c == '[' )                   // If section name
   {
     for(;;)
     {
       c= fgetc(handle);
       if( c == ']' )               // If delimiter
           break;

       parmName[nSize]= c;
       if( nSize < MAXSIZE )
           nSize++;

       if( c == '\n' || c == '\r' || c == EOF )
       {
           error(parmFile, lineNumber, "No closing brace");
           if( c != '\n' )
               skipToEndOfLine(handle); // Ignore the line
           goto newLine;
       }
     }

     skipToEndOfLine(handle);       // Skip remainder of line
     parmName[nSize]= '\0';         // Set string delimiter

     ptrSect= O->headSect;          // Look for duplicate section
     while( ptrSect != NULL )
     {
       if( strcmp(parmName, ptrSect->sectName) == 0 )
       {
           error(parmFile, lineNumber, "Duplicate section");
           goto newLine;            // ptrSect changed
       }
       ptrSect= ptrSect->nextSect;
     }

     pSize= sizeof(Section) + nSize + 1;
     ptrSect= (Section*)malloc(pSize);
     if( ptrSect == NULL )
     {
       error(parmFile, lineNumber, "No storage");
       goto endOfFile;
     }

     ptrSect->headParm= NULL;
     strcpy(ptrSect->sectName, parmName);

     ptrSect->nextSect= O->headSect;
     O->headSect= ptrSect;
     goto newLine;
   }
   //-------------------------------------------------------------------------
   // Handle quoted parameter name
   //-------------------------------------------------------------------------
   else if( c == '\"' )             // If quoted name
   {
     for(;;)
     {
       c= fgetc(handle);
       if( c == '\"' )              // If delimiter
       {
           c= skipBlanks(handle);
           break;
       }

       parmName[nSize]= c;
       if( nSize < MAXSIZE )
           nSize++;

       if( c == '\n' || c == '\r' || c == EOF )
       {
           error(parmFile, lineNumber, "No closing quote");
           if( c != '\n' )
               skipToEndOfLine(handle); // Ignore the line
           goto newLine;
       }
     }
   }

   //-------------------------------------------------------------------------
   // Handle parameter name
   //-------------------------------------------------------------------------
   else                             // Standard name
   {
     for(;;)
     {
       parmName[nSize]= c;
       if( nSize < MAXSIZE )
           nSize++;

       c= fgetc(handle);
       if( c == ' ' )               // If blank (must be delimiter)
       {
           c= skipBlanks(handle);
           if( c == ';'
               || c == '='
               || c == '\n'
               || c == '\r'
               || c == EOF )
               break;
           error(parmFile, lineNumber, "Malformed name");
           if( c != '\n' )
               skipToEndOfLine(handle); // Ignore the line
           goto newLine;
       }

       if( c == ';'
           || c == '='
           || c == '\n'
           || c == '\r'
           || c == EOF )            // If delimiter
           break;
     }
   }
   parmName[nSize]= '\0';           // Set string delimiter
   if( nSize == 0 )                 // If empty name
   {
     if( c != '\n' )
         skipToEndOfLine(handle);   // Ignore the line
     goto newLine;
   }

   //-------------------------------------------------------------------------
   // Handle parameter value
   //-------------------------------------------------------------------------
   vSize= 0;                        // Begin parameter value
   if( c == '=' )                   // If standard delimiter
     c= skipBlanks(handle);         // Skip leading blanks in value

   if( c == ';' )                   // If comment delimiter
   {
     skipToEndOfLine(handle);       // Ignore the remainder of the line
     goto parmValue;                // Process parameter value
   }

   if( c == '\"' )                  // If quoted value
   {
     for(;;)                        // Extract the parameter value
     {
       c= fgetc(handle);
       if( c == '\"' )              // If delimiter
           break;

       parmValu[vSize]= c;
       if( vSize < MAXSIZE )
           vSize++;
       if( c == '\n' || c == '\r' || c == EOF )
       {
           error(parmFile, lineNumber, "No closing quote");
           if( c != '\n' )
               skipToEndOfLine(handle); // Ignore the line
           goto newLine;
       }
     }

     c= skipBlanks(handle);
   }
   else                             // If standard value
   {
     while( c != ' '
         && c != ';'
         && c != '\n'
         && c != '\r'
         && c != EOF )              // Extract the parameter value
     {
       parmValu[vSize]= c;
       if( vSize < MAXSIZE)
         vSize++;

       c= fgetc(handle);
     }

     if( c == ' ' )
       c= skipBlanks(handle);
   }

   if( c == ';' )                   // If comment delimiter
     c= skipToEndOfLine(handle);    // Ignore the remainder of the line

   if( c != '\n' && c != '\r' && c != EOF )
   {
     error(parmFile, lineNumber, "Malformed value");
     skipToEndOfLine(handle);       // Ignore the line
     goto newLine;
   }

   //-------------------------------------------------------------------------
   // Look for duplicate parameter name
   //-------------------------------------------------------------------------
parmValue:
   parmValu[vSize]= '\0';           // Set string delimiter
   ptrPrev= NULL;
   ptrParm= ptrSect->headParm;

   while( ptrParm != NULL )         // Look for duplicate
   {
     if( strcmp(parmName, ptrParm->parmName) == 0 )
     {
       if( ptrPrev == NULL )        // Delete the (previous) duplicate
           ptrSect->headParm= ptrParm->nextParm;
       else
           ptrPrev->nextParm= ptrParm->nextParm;
       free(ptrParm);
       break;
     }

     ptrPrev= ptrParm;
     ptrParm= ptrParm->nextParm;
   }

   //-------------------------------------------------------------------------
   // Allocate and initialize the Parameter object
   //-------------------------------------------------------------------------
   pSize= sizeof(Parameter) + nSize + vSize + 1;
   ptrParm= (Parameter*)malloc(pSize);
   if( ptrParm == NULL )
   {
     error(parmFile, lineNumber, "No storage");
     if( c == EOF )
       goto endOfFile;
     goto newLine;
   }

   strcpy(ptrParm->parmName, parmName);
   p= ptrParm->parmName + nSize + 1;
   strcpy(p, parmValu);
   ptrParm->parmValue= p;

   ptrParm->nextParm= ptrSect->headParm;
   ptrSect->headParm= ptrParm;

   goto newLine;

   //-------------------------------------------------------------------------
   // End Of File
   //-------------------------------------------------------------------------
endOfFile:
   fclose(handle);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       ParseINI::close
//
// Purpose-
//       Close the parameter file.
//
//----------------------------------------------------------------------------
void
   ParseINI::close( void )          // Close the parameter file
{
   destroy();                       // Delete the object
}

#if 0
//----------------------------------------------------------------------------
//
// Subroutine-
//       ParseINI::debug
//
// Purpose-
//       List all defined elements.
//
//----------------------------------------------------------------------------
void
   ParseINI::debug( void ) const    // Bringup debugging
{
   Object*             O;           // -> Object
   Section*            ptrSect;     // -> Section
   Parameter*          ptrParm;     // -> Parameter

   O= (Object*)object;              // -> Object
   if( O == NULL )                  // If object not properly constructed
       return;                      // No value is possible

   ptrSect= O->headSect;            // Default, NULL section
   while( ptrSect != NULL )
   {
     printf("\n");
     printf("[%s]\n", ptrSect->sectName);

     ptrParm= ptrSect->headParm;
     while( ptrParm != NULL )
     {
       printf("'%s' = '%s'\n", ptrParm->parmName, ptrParm->parmValue);
       ptrParm= ptrParm->nextParm;
     }

     ptrSect= ptrSect->nextSect;
   }
}
#endif

//----------------------------------------------------------------------------
//
// Subroutine-
//       ParseINI::getValue
//
// Purpose-
//       Extract a parameter value.
//
//----------------------------------------------------------------------------
const char*                         // The parameter value
   ParseINI::getValue(              // Extract parameter value
     const char*       sectName,    // The section name (NULL allowed)
     const char*       parmName)    // The parameter's name
{
   Object*             O;           // -> Object
   Section*            ptrSect;     // -> Section
   Parameter*          ptrParm;     // -> Parameter

   O= (Object*)object;              // -> Object
   if( O == NULL )                  // If object not properly constructed
     return NULL;                   // No value is possible

   if( sectName == NULL )           // If NULL section
     sectName= "";                  // Use null name

   ptrSect= O->headSect;            // Default, NULL section
   while( ptrSect != NULL )
   {
     if( strcmp(sectName, ptrSect->sectName) == 0 )
       break;
     ptrSect= ptrSect->nextSect;
   }

   if( ptrSect != NULL )            // If section was found
   {
     ptrParm= ptrSect->headParm;
     while( ptrParm != NULL )       // Look for parameter name
     {
       if( strcmp(parmName, ptrParm->parmName) == 0 )
         return ptrParm->parmValue;

       ptrParm= ptrParm->nextParm;
     }
   }

#if 0
   printf("NULL= getValue('%s','%s')\n", sectName, parmName);
#endif

   return NULL;
}

