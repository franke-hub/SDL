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
//       FSlist.cpp
//
// Purpose-
//       Display a file tree.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

#include <com/Debug.h>
#include <com/FileInfo.h>
#include <com/FileList.h>
#include <com/FileName.h>
#include <com/nativeio.h>
#include <com/params.h>

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "FSlist  " // Source file name

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#define HCDM                        // If defined, Hard-Core Debug Mode
#endif

#ifndef SCDM
#define SCDM                        // If defined, Soft-Core Debug Mode
#endif

//----------------------------------------------------------------------------
// Operating system dependencies
//----------------------------------------------------------------------------
#if defined(_OS_WIN)
  #define S_IFIFO 0

  #define lstat _stat
  #ifndef stat
    #define stat _stat
  #endif

  #define S_ISDIR(mode) (((mode)&(S_IFMT)) == (S_IFDIR))
  #define S_ISREG(mode) (((mode)&(S_IFMT)) == (S_IFREG))
  #define S_ISCHR(mode) (((mode)&(S_IFMT)) == (S_IFCHR))
  #define S_ISBLK(mode) 0
  #define S_ISFIFO(mode) 0
  #define S_ISLNK(mode) 0
  #define S_ISSOCK(mode) 0
#endif

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
struct DirArray;

//----------------------------------------------------------------------------
//
// Struct-
//       DirEntry
//
// Purpose-
//       Describe a directory entry.
//
//----------------------------------------------------------------------------
struct DirEntry                     // Directory element
{
   DirEntry*           next;        // Next element on list
   DirArray*           child;       // -> Subdirectory

   unsigned long       st_mode;     // File mode
   unsigned long       st_size;     // File size
   unsigned long       st_time;     // File modification time
   char                fileName[FILENAME_MAX+1]; // The file name

//----------------------------------------------------------------------------
// DirEntry::Methods
//----------------------------------------------------------------------------
static DirEntry*                    // -> DirEntry
   allocate( void );                // Allocate a DirEntry

void
   release( void );                 // Delete this DirEntry

void
   debugCoherency( void );          // Diagnostic display

const char*                         // The type of file {"D", "F", "L", "P"}
   getType( void ) const;           // Get type of file

void
   show( void ) const;              // Display the entry

//----------------------------------------------------------------------------
// DirEntry::Attributes
//----------------------------------------------------------------------------
static DirEntry*     freeList;      // List of free DirEntry blocks
}; // struct DirEntry

//----------------------------------------------------------------------------
//
// Struct-
//       DirArray
//
// Purpose-
//       An array of DirEntry elements.
//
//----------------------------------------------------------------------------
struct DirArray                     // Directory element array
{
   DirArray*           next;        // Next DirArray element
   unsigned int        count;       // Number of elements

   char*               name;        // Local name of directory
   DirEntry*           head;        // -> First DirEntry element
   DirEntry*           list[1];     // Element pointer array

//----------------------------------------------------------------------------
// DirArray::Methods
//----------------------------------------------------------------------------
static DirArray*                    // -> DirArray
   allocate(                        // Allocate a DirArray
     int               elements);   // Number of array elements

void
   release( void );                 // Delete this DirArray

void
   debugCoherency( void );          // Diagnostic display

static void
   show(                            // Show directory tree
     const char*       path);       // Path name
}; // struct DirArray

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static char*           initPath= NULL;// Initial path
static DirArray*       ignoreFile= NULL; // The list of ignored files
static DirArray*       ignorePath= NULL; // The list of ignored directories
static int             sw_verbose= FALSE; // VERBOSE mode

DirEntry*              DirEntry::freeList= NULL;// List of free DirEntry blocks

//----------------------------------------------------------------------------
//
// Subroutine-
//       number
//
// Purpose-
//       Display a number.
//       +1,234,567,890
//
//----------------------------------------------------------------------------
static char*                        // Resultant
   number(                          // Format a number
     char*             result,      // Resultant
     long              value)       // This one
{
   char                temp[32];    // temporary string
   int                 L;           // Length of temporary
   int                 sign;        // Sign of result

   int                 m;           // Modulo remainder
   int                 comma;       // Comma indicator

   sign= 1;
   if( value < 0 )
   {
     sign= (-1);
     value= -value;
   }

   L= 0;
   comma= 3;
   while( value > 0 )
   {
     if( comma == 0 )
     {
       temp[L++]= ',';
       comma= 3;
     }
     comma--;
     m= value % 10;
     value= value / 10;
     temp[L++]= '0' + m;
   }
   if( L == 0 )
     temp[L++]= '0';
   if( sign < 0 )
     temp[L++]= '-';
   while( L < 14 )
     temp[L++]= ' ';
   while( L > 0 )
   {
     result[14-L]= temp[L-1];
     L--;
   }
   result[14]= '\0';

   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       info
//
// Purpose-
//       Print a description of what this program does, then exit.
//
//----------------------------------------------------------------------------
static void
   info( void )                     // Accumulate a name
{
   fprintf(stderr, "FSlist <options>\n");
   fprintf(stderr,
           "Options:\n"
           "  -d:Path (Specifies initial directory)\n"
           "  -if:File (Specifies file to be skipped)\n"
           "  -ip:Path (Specifies path to be skipped)\n"
           "  -v  (Verify)\n"
          );

   exit(EXIT_FAILURE);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       parm
//
// Purpose-
//       Parameter analysis.
//
//----------------------------------------------------------------------------
static void
   parm(                            // Parameter analysis
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   char*               argp;        // Argument pointer
   int                 argi;        // Argument index

   DirArray*           ptrA;        // -> DirArray
   DirEntry*           ptrE;        // -> DirEntry
   int                 error;       // Error encountered indicator

   //-------------------------------------------------------------------------
   // Defaults
   //-------------------------------------------------------------------------
   error=  FALSE;                   // Default, no error
   sw_verbose= FALSE;               // Default, not verbose

   //-------------------------------------------------------------------------
   // Argument analysis
   //-------------------------------------------------------------------------
   if (argc > 1 && *argv[1] == '?') // If query request
     info();                        // Display options

   for (argi=1; argi<argc; argi++)  // Analyze variable controls
   {
     argp= argv[argi];              // Address the parameter

     if (*argp == '-')              // If this parameter is in switch format
     {
       argp++;                      // Skip over the switch char

       if (swname("v", argp))
         sw_verbose= swatob("v", argp);

       else if (swname("d:", argp))
       {
         initPath= argp + 2;
       }
       else if (swname("if:", argp))
       {
         if( ignoreFile == NULL )
           ignoreFile= DirArray::allocate(1);
         ptrE= DirEntry::allocate();
         strcpy(ptrE->fileName, argp+3);
         ptrE->next= ignoreFile->head;
         ignoreFile->head= ptrE;
       }
       else if (swname("ip:", argp))
       {
         if( ignorePath == NULL )
           ignorePath= DirArray::allocate(1);
         ptrE= DirEntry::allocate();
         strcpy(ptrE->fileName, argp+3);
         ptrE->next= ignorePath->head;
         ignorePath->head= ptrE;
       }
       else
       {
         error= TRUE;
         fprintf(stderr, "Invalid parameter '%s'\n", argv[argi]);
       }
     }
     else
     {
       error= TRUE;
       fprintf(stderr, "Unexpected parameter: '%s'\n", argp);
     }
   }

   //-------------------------------------------------------------------------
   // Completion analysis
   //-------------------------------------------------------------------------
   if( error )                      // If error encountered
     info();

   if( sw_verbose )                 // If verbose mode
   {
     ptrA= ignorePath;
     if( ptrA != NULL )
     {
       debugf("Ignored Paths:\n");
       ptrE= ptrA->head;
       while( ptrE != NULL )
       {
         debugf(">>%s\n", ptrE->fileName);
         ptrE= ptrE->next;
       }
     }
     ptrA= ignoreFile;
     if( ptrA != NULL )
     {
       debugf("Ignored Files:\n");
       ptrE= ptrA->head;
       while( ptrE != NULL )
       {
         debugf(">>%s\n", ptrE->fileName);
         ptrE= ptrE->next;
       }
     }
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       zalloc
//
// Purpose-
//       Allocate storage, exit if not available.
//
//----------------------------------------------------------------------------
static void*                        // -> Allocated storage
   zalloc(                          // Allocate storage
     unsigned          size)        // Required length
{
   void*               result;      // Resultant

   result= malloc(size);            // Allocate the storage
   if( result == NULL )
   {
     fprintf(stderr, "No storage(%u)\n", size);
     exit(EXIT_FAILURE);
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       DirArray::allocate
//
// Purpose-
//       Allocate a queue array block.
//
//----------------------------------------------------------------------------
DirArray*                           // -> DirArray
   DirArray::allocate(              // Allocate a DirArray
     int               elements)    // Number of array elements
{
   DirArray*           ptrA;        // Pointer to queue array block
   int                 s;           // Size of the array

   s= sizeof(DirArray) + (elements * sizeof(DirEntry*)); // Sizeof header +
                                    // (element count * sizeof pointer)

   ptrA= (DirArray*)zalloc(s);      // Allocate a new array
   ptrA->next= NULL;                // Set the chain pointer
   ptrA->head= NULL;                // Set the list vectors
   ptrA->count= elements;           // Set the element count
   return(ptrA);                    // Return pointer to element
}

//----------------------------------------------------------------------------
//
// Method-
//       DirArray::release
//
// Purpose-
//       Release this DirArray block.
//
//----------------------------------------------------------------------------
void
   DirArray::release( void )        // Delete this DirArray
{
   DirArray*           ptrA= this;  // Pointer to this DirArray
   DirEntry*           newE;        // Pointer to queue element
   DirEntry*           oldE;        // Pointer to queue element

   if( ptrA == NULL )               // If release of empty array
     return;                        // No problem

   newE= head;                      // Address the first element
   while(newE != NULL)              // Release subdirectory entries
   {
     if( newE->child != NULL )      // If this is a subdirectory
       newE->child->release();      // Release the subdirectory

     newE= newE->next;              // Address the next element
   }

   newE= head;                      // Address the first element
   while(newE != NULL)              // Release directory entries
   {
     oldE= newE;                    // Save the element pointer
     newE= oldE->next;              // Address the next element
     oldE->release();               // Release the old element
   }

   free(ptrA);                      // Release the array
}

//----------------------------------------------------------------------------
//
// Method-
//       DirArray::debugCoherency
//
// Purpose-
//       Diagnostic display.
//
//----------------------------------------------------------------------------
void
   DirArray::debugCoherency( void ) // Diagnostic display
{
   DirArray*           ptrA= this;
   DirEntry*           ptrE;

   debugf("DirArray(%p)::debugCoherency()\n", this);
   if( ptrA == NULL )
     return;

   for(ptrE= head; ptrE != NULL; ptrE= ptrE->next)
   {
     ptrE->debugCoherency();
   }

   for(ptrE= head; ptrE != NULL; ptrE= ptrE->next)
   {
     if( ptrE->child != NULL )
       ptrE->child->debugCoherency();
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       DirArray::show
//
// Purpose-
//       Show a directory.
//
//----------------------------------------------------------------------------
void
   DirArray::show(                  // Load a directory
     const char*       dirName)     // For this path name
{
   FileList            dir(dirName, "*");
   struct stat         s;           // File stats

   char                FQN[FILENAME_MAX+1+FILENAME_MAX+1];
   const char*         fileName;    // -> Current fileName

   DirArray*           ptrA;        // Pointer to queue array block
   DirEntry*           ptrE;        // Pointer to queue entry block
   DirArray            genA;        // Generated  queue array block

   int                 count;       // Number of directory entries

   int                 rc;
   int                 i, j;

   //-------------------------------------------------------------------------
   // Welcome to directory
   //-------------------------------------------------------------------------
   ptrA= ignorePath;
   if( ptrA != NULL )
   {
     ptrE= ptrA->head;
     while( ptrE != NULL )
     {
       if( strcmp(ptrE->fileName, dirName) == 0 )
       {
         #ifdef HCDM
           debugf(">>D: %s !!IGNORED\n", dirName);
         #endif
         return;
       }
       ptrE= ptrE->next;
     }
   }

   debugf("\n");
   debugf("%s\n", dirName);

   //-------------------------------------------------------------------------
   // Read the directory
   //-------------------------------------------------------------------------
   count= 0;                        // No entries located yet
   genA.head= NULL;                 // Start with an empty list
   fileName= dir.getCurrent();
   for(;;)                          // For each directory entry
   {
     if( fileName == NULL )         // If end of list
       break;

     if( strcmp(fileName,".") != 0
         && strcmp(fileName,"..") != 0 )
     {
       ptrE= DirEntry::allocate();  // Allocate a DirEntry

       FileName::concat(FQN, dirName, fileName);
       rc= lstat(FQN, &s);          // Read file status
       if( rc != 0 )                // If can't read object status
       {
         fprintf(stderr, "%s %d: File(%s) lstat failure\n",
                         __SOURCE__, __LINE__, FQN);
         exit(EXIT_FAILURE);
       }
       if( strlen(fileName) >= sizeof(ptrE->fileName) )
       {
         fprintf(stderr, "%s %d: File(%s/%s) too long\n",
                         __SOURCE__, __LINE__, dirName, fileName);
         exit(EXIT_FAILURE);
       }

       ptrE->st_mode= s.st_mode;    // Set the fileInfo
       ptrE->st_time= s.st_mtime;   // Set the modification time
       ptrE->st_size= s.st_size;    // Set the file size
       strcpy(ptrE->fileName, fileName); // Set the file name

       //-----------------------------------------------------------------------
       // Install the entry
       //-----------------------------------------------------------------------
       ptrE->next= genA.head;       // Set the chain pointer
       genA.head= ptrE;             // Install the entry
       count++;                     // Count the entry
     }

     fileName= dir.getNext();
   }

   if( count == 0 )                 // If a NULL directory
     return;

   //-------------------------------------------------------------------------
   // Allocate a queue array list
   //-------------------------------------------------------------------------
   ptrA= DirArray::allocate(count); // Allocate a DirArray

   //-------------------------------------------------------------------------
   // Populate the list
   //-------------------------------------------------------------------------
   ptrE= genA.head;                 // Address the first entry
   for(i=0; i<count; i++)           // Populate the list
   {
     ptrA->list[i]= ptrE;
     ptrE= ptrE->next;
   }

   //-------------------------------------------------------------------------
   // Sort the directory
   //-------------------------------------------------------------------------
   for(i=0; i<count; i++)           // Sort the directory
   {
     for(j=i+1; j<count; j++)
     {
       if( strcmp(ptrA->list[i]->fileName,
                  ptrA->list[j]->fileName) > 0 ) // If out of sequence
       {
         ptrE= ptrA->list[i];       // Switch the pointers
         ptrA->list[i]= ptrA->list[j];
         ptrA->list[j]= ptrE;
       }
     }
   }

   //-------------------------------------------------------------------------
   // Sort the list
   //-------------------------------------------------------------------------
   ptrA->head= ptrA->list[0];       // Set the list vector

   for(i=1; i<count; i++)           // Create the sorted list
     ptrA->list[i-1]->next= ptrA->list[i];

   ptrA->list[count-1]->next= NULL;


   //-------------------------------------------------------------------------
   // Show the list
   //-------------------------------------------------------------------------
   for(ptrE= ptrA->head; ptrE != NULL; ptrE= ptrE->next)
   {
     FileName::concat(FQN, dirName, ptrE->fileName);
     ptrE->show();               // Display
   }

   //-------------------------------------------------------------------------
   // Show the subdirectories
   //-------------------------------------------------------------------------
   for(ptrE= ptrA->head; ptrE != NULL; ptrE= ptrE->next)
   {
     FileName::concat(FQN, dirName, ptrE->fileName);
     if( S_ISDIR(ptrE->st_mode) )// If subdirectory
     {
       show(FQN);                // Show it
     }
   }

   //-------------------------------------------------------------------------
   // Return
   //-------------------------------------------------------------------------
   ptrA->release();
}

//----------------------------------------------------------------------------
//
// Method-
//       DirEntry::allocate
//
// Purpose-
//       Allocate a DirEntry block.
//
//----------------------------------------------------------------------------
DirEntry*                           // -> DirEntry
   DirEntry::allocate( void )       // Allocate a DirEntry
{
   DirEntry*           ptrE;        // Pointer to queue entry block

   ptrE= freeList;                  // Examine the recyle queue
   if( ptrE != NULL )               // If an element exists
   {
     freeList= ptrE->next;          // Remove it from the queue
     return(ptrE);                  // Return, function complete
   }

   ptrE= (DirEntry*)zalloc(sizeof(DirEntry));// Allocate a new element
   memset(ptrE, 0, sizeof(*ptrE));  // Clear the entry
   return(ptrE);                    // Return pointer to element
}

//----------------------------------------------------------------------------
//
// Method-
//       DirEntry::release
//
// Purpose-
//       Release a DirEntry block.
//
//----------------------------------------------------------------------------
void
   DirEntry::release( void )        // Delete this DirEntry
{
   next= freeList;                  // Set the element chain pointer
   freeList= this;                  // Add the element to the queue
}

//----------------------------------------------------------------------------
//
// Method-
//       DirEntry::getType
//
// Purpose-
//       Return the type of the file
//
//----------------------------------------------------------------------------
const char*                         // The type of file {"D", "F", "L", "P"}
   DirEntry::getType( void ) const  // Get type of file
{
   if( S_ISREG(st_mode)  ) return "F";
   if( S_ISCHR(st_mode)  ) return "F";
   if( S_ISDIR(st_mode)  ) return "D";
   if( S_ISLNK(st_mode)  ) return "L";
   if( S_ISFIFO(st_mode) ) return "P";

   return "U";
}

//----------------------------------------------------------------------------
//
// Method-
//       DirEntry::debugCoherency
//
// Purpose-
//       Diagnostic display.
//
//----------------------------------------------------------------------------
void
   DirEntry::debugCoherency( void ) // Diagnostic display
{
   debugf("DirEntry(%p)::debugCoherency()\n", this);
   debugf("..Child(%p) Time(%10lu) Size(%10lu) Name(%s)\n",
          child, st_time, st_size, fileName);
}

//----------------------------------------------------------------------------
//
// Method-
//       DirEntry::show
//
// Purpose-
//       Element display.
//
//----------------------------------------------------------------------------
void
   DirEntry::show( void ) const     // Display
{
   char                time[32];
   char                size[32];

   number(time, st_time);
   number(size, st_size);

// debugf("%s %10u %10u %s\n", getType(), st_time, st_size, fileName);
   debugf("%s %s %s %s\n", getType(), time, size, fileName);
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
   char                initArea[FILENAME_MAX+1]; // Initial path
   int                 L;           // Working length

   //-------------------------------------------------------------------------
   // Parameter analysis
   //-------------------------------------------------------------------------
   parm(argc, argv);

   //-------------------------------------------------------------------------
   // Get the initial path
   //-------------------------------------------------------------------------
   if( initPath == NULL )
   {
     initPath= initArea;            // Use internal area
     getcwd(initPath, sizeof(initArea)); // Get current working directory
   }
   L= strlen(initPath);
   if( L > 1 )
   {
     if( initPath[L-1] == '\\' || initPath[L-1] == '/' )
       initPath[L-1]= '\0';
   }
   tracef("Path: %s\n", initPath);

   //-------------------------------------------------------------------------
   // Show the tree
   //-------------------------------------------------------------------------
   DirArray::show(initPath);        // Show the Tree

   return 0;
}

