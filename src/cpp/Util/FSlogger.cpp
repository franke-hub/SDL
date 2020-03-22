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
//       FSlogger.cpp
//
// Purpose-
//       Keep a file log, looking for differences.
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
#include <com/Checksum.h>
#include <com/FileInfo.h>
#include <com/FileList.h>
#include <com/FileName.h>
#include <com/Media.h>
#include <com/nativeio.h>
#include <com/params.h>

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "FSlogger" // Source file name

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
#define BUFSIZE               16384 // I/O buffer size

#define FILENAME     "FSlogger.out" // File name
#define VERSION_STRING  "TREE V1R1" // Version identifier

//----------------------------------------------------------------------------
// Record types
//----------------------------------------------------------------------------
#define ID_VERSION             0x01 // Version identifier
#define ID_PUSHDIR             0x02 // Push subdirectory
#define ID_POPDIR              0x03 // Pop subdirectory

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
// Class-
//       OpenMedia
//
// Purpose-
//       Extend FileMedia to remember the file name.
//
//----------------------------------------------------------------------------
class OpenMedia : public FileMedia {// OpenMedia object
//----------------------------------------------------------------------------
// OpenMedia::Attributes
//----------------------------------------------------------------------------
protected:
   const char*         name;        // The open name

//----------------------------------------------------------------------------
// OpenMedia::Methods
//----------------------------------------------------------------------------
public:
virtual int                         // Return code (0 OK)
   open(                            // Start using the OpenMedia
     const char*       name,        // The name
     const char*       mode)        // The mode
{
   int rc= FileMedia::open(name, mode);
   if( rc == 0 )
     this->name= name;

   return rc;
}

virtual int                         // Return code (0 OK)
   close( void )                    // Finish using the OpenMedia
{
   name= NULL;

   return FileMedia::close();
}

//----------------------------------------------------------------------------
// OpenMedia::Methods
//----------------------------------------------------------------------------
const char*
   getOpenName( void )
{
   return name;
}
}; // class OpenMedia

//----------------------------------------------------------------------------
//
// Structure-
//       LogRecord
//
// Purpose-
//       Describe a log record entry.
//
//----------------------------------------------------------------------------
struct LogRecord                    // Log record
{
   Checksum64          checksum;    // File checksum
   unsigned long       fileMode;    // File mode
   unsigned long       fileSize;    // Size of the file, in bytes
   time_t              fileTime;    // Time last modified
   char                fileName[1]; // FileName string (ends with '\0')
};

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

   Checksum64          checksum;    // File checksum
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

static DirArray*                    // -> DirArray
   load(                            // Load directory tree
     const char*       path,        // Path name
     const char*       name);       // Directory name qualifier

static DirArray*                    // -> DirArray
   get(                             // Load from external storage
     OpenMedia&        file);       // External file

int                                 // Return code (0 OK)
   put(                             // Store onto external storage
     OpenMedia&        file);       // External file

private:
static DirArray*                    // -> DirArray
   getTree(                         // Load Tree from external storage
     OpenMedia&        file,        // External file
     unsigned          size);       // Number of data bytes

int                                 // Return code (0 OK)
   putTree(                         // Store Tree onto external storage
     OpenMedia&        file);       // External file
}; // struct DirArray

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static char            buffer[BUFSIZE]; // I/O buffer
static char*           initPath= NULL; // Initial path
static DirArray*       ignoreFile= NULL; // The list of ignored files
static DirArray*       ignorePath= NULL; // The list of ignored directories
static int             sw_dir= FALSE; // Directories interesting?
static int             sw_status= FALSE; // Status change interesting?
static int             sw_verbose= FALSE; // VERBOSE mode

DirEntry*              DirEntry::freeList= NULL;// List of free DirEntry blocks

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
   fprintf(stderr, "FSlogger <options>\n");
   fprintf(stderr,
           "Options:\n"
           "  -d:Path (Specifies initial directory)\n"
           "  -if:File (Specifies file to be skipped)\n"
           "  -ip:Path (Specifies path to be skipped)\n"
           "  -dir (Directories are interesting)\n"
           "  -sts (Status changes are interesting)\n"
           "  -v   (Verbose)\n"
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
   sw_dir= FALSE;                   // Default, DIR uninteresting
   sw_status= FALSE;                // Default, STS uninteresting
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

       else if (swname("dir", argp))
         sw_dir= swatob("dir", argp);

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

       else if (swname("sts", argp))
         sw_status= swatob("sts", argp);

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
//       nameReader
//
// Purpose-
//       Read a '\0' delimited name from a file.
//
//----------------------------------------------------------------------------
static int                          // Name length
   nameReader(                      // Accumulate a name
     char*             name,        // Resultant
     OpenMedia&        file)        // File
{
   int                 i;
   unsigned            L;

   for(i=0; ; i++)
   {
     if( i >= FILENAME_MAX )
     {
       fprintf(stderr, "File(%s) invalid internal format\n",
                       file.getOpenName());
       exit(EXIT_FAILURE);
     }

     L= file.read(&name[i], 1);
     if( L != 1 )
     {
       fprintf(stderr, "File(%s) invalid internal format\n",
                       file.getOpenName());
       exit(EXIT_FAILURE);
     }

     if( name[i] == '\0' )
       break;
   }

   return i;
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
     fprintf(stderr, "No storage\n");
     exit(EXIT_FAILURE);
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Enumeration-
//       RcCheck
//
// Purpose-
//       Define the return codes for fsChecksum()
//
//----------------------------------------------------------------------------
enum RcCheck                        // Return codes for fsCheck
{
   Check_OK=                      0,// Checksum computed
   Check_NG=                      1 // Could not read file
};

//----------------------------------------------------------------------------
//
// Subroutine-
//       fsChecksum
//
// Purpose-
//       Compute a file's checksum.
//
//----------------------------------------------------------------------------
static RcCheck                      // Return code
   fsChecksum(                      // Compute file checksum
     char*             inpName,     // File name
     Checksum64&       checksum)    // Output checksum
{
   OpenMedia           file;        // File object

   DirArray*           ptrA;        // -> DirArray
   DirEntry*           ptrE;        // -> DirEntry
   int                 L;           // Number of bytes read
   int                 rc;

   //-------------------------------------------------------------------------
   // Initialize the Checksum
   //-------------------------------------------------------------------------
   checksum.reset();
   ptrA= ignoreFile;
   if( ptrA != NULL )
   {
     ptrE= ptrA->head;
     while( ptrE != NULL )
     {
       if( strcmp(inpName, ptrE->fileName) == 0 )
       {
         #ifdef HCDM
           debugf(">>F: %s !!IGNORED\n", inpName);
         #else
           if( sw_verbose )
             debugf(">>F: %s !!IGNORED\n", inpName);
         #endif

         return Check_OK;
       }
       ptrE= ptrE->next;
     }
   }

   if( sw_verbose )
     debugf(">>F: %s\n", inpName);

   //-------------------------------------------------------------------------
   // File initialization
   //-------------------------------------------------------------------------
   rc= file.open(inpName, file.MODE_READ);// Open for input
   if( rc != 0 )                    // If the input file doesn't exist
   {
     errorf("Err: %s (No access)\n", inpName);
     return Check_NG;
   }

   //-------------------------------------------------------------------------
   // Read the input file, computing the checksum
   //-------------------------------------------------------------------------
   for(;;)                          // Read the input file
   {
     L= file.read(buffer, BUFSIZE); // Read the buffer
     if( L < 0 )                    // If I/O error
     {
       errorf("Err: %s (Read fault)\n", inpName);
       file.close();                // Close the file
       return Check_NG;             // Function complete
     }

     if( L == 0 )                   // If end of file
       break;                       // Function complete

     checksum.accumulate(buffer, L);// Accumulate the checksum
   }

   file.close();                    // Close the file
   return Check_OK;                 // Function complete
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

   if( this == NULL )               // If release of empty array
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

   free(ptrA->name);                // Release the name
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
   DirEntry*           ptrE;

   debugf("DirArray(%p)::debugCoherency()\n", this);
   if( this == NULL )
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
//       DirArray::load
//
// Purpose-
//       Load a directory.
//
//----------------------------------------------------------------------------
DirArray*                           // -> Sorted DirArray list
   DirArray::load(                  // Load a directory
     const char*       dirName,     // (Full) directory name
     const char*       localName)   // Local directory name
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
         return NULL;
       }
       ptrE= ptrE->next;
     }
   }

   #ifdef HCDM
     debugf(">>D: %s\n", dirName);
   #endif

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
     return(NULL);

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
   // Load the subdirectories
   //-------------------------------------------------------------------------
   for(ptrE= ptrA->head; ptrE != NULL; ptrE= ptrE->next)
   {
     FileName::concat(FQN, dirName, ptrE->fileName);
     if( S_ISLNK(ptrE->st_mode) )   // If link
     {
       ptrE->checksum.reset();      // Reset the checksum
     }
     else if( S_ISDIR(ptrE->st_mode) ) // If subdirectory
     {
       ptrE->child= load(FQN, ptrE->fileName); // Load it
     }
     else if( S_ISREG(ptrE->st_mode) ) // If regular file
     {
       fsChecksum(FQN, ptrE->checksum); // Compute its checksum
     }
     else if( S_ISCHR(ptrE->st_mode)
              || S_ISBLK(ptrE->st_mode)
              || S_ISFIFO(ptrE->st_mode)
              || S_ISSOCK(ptrE->st_mode) ) // If special
     {
       ptrE->checksum.reset();      // Reset the checksum
     }
     else                           // What mode is it?
     {
       ptrE->checksum.reset();      // Reset the checksum
       fprintf(stderr, "File(%s) st_mode(%lx)\n",
                       FQN, (long)ptrE->st_mode);
     }
   }

   //-------------------------------------------------------------------------
   // Set the local name
   //-------------------------------------------------------------------------
   ptrA->name= (char*)zalloc(strlen(localName)+1);
   strcpy(ptrA->name, localName);

   //-------------------------------------------------------------------------
   // Return
   //-------------------------------------------------------------------------
   return(ptrA);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       DirArray::getTree
//
// Purpose-
//       Load a subdirectory from an external file
//
//----------------------------------------------------------------------------
DirArray*                           // -> Sorted DirArray list
   DirArray::getTree(               // Load a subdirectory
     OpenMedia&        file,        // External file
     unsigned          size)        // Number of data bytes
{
   LogRecord           logRecord;   // Log Record
   DirArray*           ptrA;        // Resultant
   DirArray*           subA;        // SubResultant
   DirEntry*           ptrE;        // -> DirEntry
   DirEntry*           head;        // -> Head DirEntry
   DirEntry*           tail;        // -> Tail DirEntry

   char                name[FILENAME_MAX+1];
   unsigned            count;       // Number of entries
   unsigned            L;           // Generic length
   unsigned            R;           // Remaining length
   int                 i;

   //-------------------------------------------------------------------------
   // Initialize
   //-------------------------------------------------------------------------
   count= 0;
   head= NULL;
   tail= NULL;
   R= size;

   //-------------------------------------------------------------------------
   // Read subdirectory name
   //-------------------------------------------------------------------------
   L= nameReader(name, file);
   if( L >= R )
   {
     fprintf(stderr, "%d: File(%s) invalid format\n",
                      __LINE__, file.getOpenName());
     return NULL;
   }
   R -= (L + 1);

   //-------------------------------------------------------------------------
   // Read the log records
   //-------------------------------------------------------------------------
   for(;;)
   {
     if( R <= sizeof(logRecord) )
     {
       fprintf(stderr, "%d: File(%s) invalid format\n",
                        __LINE__, file.getOpenName());
       return NULL;
     }

     L= file.read((char*)&logRecord, sizeof(logRecord)-1);
     if( L != sizeof(logRecord)-1 )
     {
       fprintf(stderr, "%d: File(%s) invalid format\n",
                        __LINE__, file.getOpenName());
       return NULL;
     }
     R -= L;

     count++;                       // Count this entry
     ptrE= DirEntry::allocate();    // Allocate a directory entry
     if( head == NULL )
       head= ptrE;
     else
       tail->next= ptrE;
     tail= ptrE;

     ptrE->checksum= logRecord.checksum;
     ptrE->st_mode=  logRecord.fileMode;
     ptrE->st_size=  logRecord.fileSize;
     ptrE->st_time=  logRecord.fileTime;
     L= nameReader(ptrE->fileName, file);
     if( L >= R )
     {
       fprintf(stderr, "%d: File(%s) invalid format\n",
                        __LINE__, file.getOpenName());
       return NULL;
     }
     R -= (L + 1);

     if( R == 0 )
       break;
   }

   //-------------------------------------------------------------------------
   // Allocate the DirArray
   //-------------------------------------------------------------------------
   ptrA= DirArray::allocate(count);
   memset(ptrA, 0, sizeof(*ptrA));
   ptrA->name= (char*)zalloc(strlen(name)+1);
   strcpy(ptrA->name, name);

   //-------------------------------------------------------------------------
   // Initialize the DirArray
   //-------------------------------------------------------------------------
   ptrA->count= count;
   ptrA->head=  head;
   ptrE= head;
   for(i=0; i<count; i++)
   {
     ptrA->list[i]= ptrE;
     ptrE= ptrE->next;
   }

   //-------------------------------------------------------------------------
   // Load any subdirectories
   //-------------------------------------------------------------------------
   for(;;)
   {
     buffer[0]= ID_POPDIR;
     file.read(buffer, 1);          // Either push or pop
     if( buffer[0] != ID_PUSHDIR )  // If not push
     {
       if( buffer[0] == ID_POPDIR ) // Should be pop
         break;                     // Normal completion

       fprintf(stderr, "%d: File(%s) invalid format\n",
                        __LINE__, file.getOpenName());
       return NULL;
     }

     L= file.read(buffer, 4);
     if( L != 4 )
     {
       fprintf(stderr, "%d: File(%s) invalid format\n",
                        __LINE__, file.getOpenName());
       return NULL;
     }

     R= *((uint32_t*)&buffer[0]);   // Size of subdirectory
     subA= getTree(file, R);        // Get subdirectory
     if( subA == NULL )             // If file format failure
       return NULL;                 // Propagate failure to higher node

     for(i=0; i<count; i++)
     {
       ptrE= ptrA->list[i];
       if( ptrE->child == NULL
           && strcmp(ptrE->fileName, subA->name) == 0 )
       {
         ptrE->child= subA;
         subA= NULL;
         break;
       }
     }

     if( subA != NULL )
     {
       fprintf(stderr, "%d: File(%s) invalid format\n",
                        __LINE__, file.getOpenName());
       return NULL;
     }
   }

   return ptrA;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       DirArray::get
//
// Purpose-
//       Load a directory from an external file
//
//----------------------------------------------------------------------------
DirArray*                           // -> Sorted DirArray list
   DirArray::get(                   // Load a directory
     OpenMedia&      file)          // External file
{
   unsigned          R;             // Remaining length

   //-------------------------------------------------------------------------
   // Verify version id
   //-------------------------------------------------------------------------
   file.read(buffer, 5);            // Read key, length
   if( buffer[0] != ID_VERSION )
   {
     fprintf(stderr, "%d: File(%s) invalid format\n",
                      __LINE__, file.getOpenName());
     return NULL;
   }

   if( *((uint32_t*)&buffer[1]) != strlen(VERSION_STRING)+1 )
   {
     fprintf(stderr, "File(%s) invalid version\n", file.getOpenName());
     return NULL;
   }

   file.read(buffer, strlen(VERSION_STRING)+1);
   if( strcmp(buffer, VERSION_STRING) != 0 )
   {
     fprintf(stderr, "File(%s) invalid version\n", file.getOpenName());
     return NULL;
   }

   //-------------------------------------------------------------------------
   // Begin subdirectory
   //-------------------------------------------------------------------------
   R= file.read(buffer, 5);         // Read header
   if( R != 5 )                     // If cannot read
   {
     fprintf(stderr, "File(%s) invalid format\n", file.getOpenName());
     return NULL;
   }

   if( buffer[0] != ID_PUSHDIR )
   {
     fprintf(stderr, "File(%s) invalid format\n", file.getOpenName());
     return NULL;
   }

   R= *((uint32_t*)&buffer[1]);

   //-------------------------------------------------------------------------
   // Load subtree
   //-------------------------------------------------------------------------
   return getTree(file, R);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       DirArray::putTree
//
// Purpose-
//       Store a directory onto an external file
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   DirArray::putTree(               // Store a subdirectory
     OpenMedia&        file)        // External file
{
   LogRecord           logRecord;   // Log Record
   DirEntry*           ptrE;        // -> DirEntry

   unsigned            L;           // Generic length

   //-------------------------------------------------------------------------
   // As a special case, empty subdirectories are not stored
   //-------------------------------------------------------------------------
   if( head == NULL )
     return 0;

   //-------------------------------------------------------------------------
   // Compute length of Tree record
   //-------------------------------------------------------------------------
   L= strlen(name)+1;               // Length of name + '\0'
   for(ptrE= head; ptrE != NULL; ptrE= ptrE->next)
   {
     L += sizeof(LogRecord);        // Each file takes a LogRecord
     L += strlen(ptrE->fileName);   // Each file name is saved
   }

   //-------------------------------------------------------------------------
   // Write the Tree record
   //-------------------------------------------------------------------------
   buffer[0]= ID_PUSHDIR;           // Push directory
   *((int32_t*)&buffer[1])= L;      // (length)
   file.write(buffer, 5);           // Write the header
   file.write(name, strlen(name)+1);// Write the Tree name

   //-------------------------------------------------------------------------
   // Write the LogRecord array
   //-------------------------------------------------------------------------
   for(ptrE= head; ptrE != NULL; ptrE= ptrE->next)
   {
     logRecord.checksum= ptrE->checksum; // Copy the checksum
     logRecord.fileMode= ptrE->st_mode;  // Copy the file mode
     logRecord.fileSize= ptrE->st_size;  // Copy the file size
     logRecord.fileTime= ptrE->st_time;  // Copy the file time

     file.write((char*)&logRecord, sizeof(logRecord)-1);
     file.write(ptrE->fileName, strlen(ptrE->fileName)+1);
   }

   //-------------------------------------------------------------------------
   // Write the SubDirectories
   //-------------------------------------------------------------------------
   for(ptrE= head; ptrE != NULL; ptrE= ptrE->next)
   {
     if( ptrE->child != NULL )
       ptrE->child->putTree(file);
   }

   //-------------------------------------------------------------------------
   // Write the Trailer
   //-------------------------------------------------------------------------
   buffer[0]= ID_POPDIR;
   file.write(buffer, 1);

   return 0;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       DirArray::put
//
// Purpose-
//       Store a directory onto an external file
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   DirArray::put(                   // Store a directory
     OpenMedia&        file)        // External file
{
   //-------------------------------------------------------------------------
   // Write the version info
   //-------------------------------------------------------------------------
   buffer[0]= ID_VERSION;
   *((int32_t*)&buffer[1])= strlen(VERSION_STRING)+1;
   file.write(buffer, 5);
   file.write(VERSION_STRING, strlen(VERSION_STRING)+1);

   //-------------------------------------------------------------------------
   // Write the subtree
   //-------------------------------------------------------------------------
   putTree(file);                   // Write the subtree

   return 0;                        // Hey, that was easy
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
// Subroutine-
//       ifFirst
//
// Function-
//       Print one-time message.
//
//----------------------------------------------------------------------------
static int                          // Constant TRUE
   ifFirst(                         // Print if first time
     int               firstTime,   // First time flag
     const char*       fmt,         // Format list
                       ...)         // Argument array
{
   va_list             argptr;      // Argument list pointer

   if( firstTime )
     return TRUE;

   va_start(argptr, fmt);           // Initialize va_ functions
   vdebugf(fmt, argptr);            // Write the message
   va_end(argptr);                  // Close va_ functions

   return TRUE;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       compareTree
//
// Function-
//       Compare a directory subtree.
//
//----------------------------------------------------------------------------
static void
   compareTree(                     // Compare directory subtree
     const char*       path,        // Current path
     DirArray*         oldTree,     // Old Tree
     DirArray*         newTree)     // New Tree
{
   DirEntry*           oldE;        // Pointer to file descriptor
   DirEntry*           newE;        // Pointer to file descriptor
   char                FQN[FILENAME_MAX+1+FILENAME_MAX+1];
   int                 once= FALSE; // One-time flag

   //-------------------------------------------------------------------------
   // Initialize
   //-------------------------------------------------------------------------
   if( oldTree == NULL || newTree == NULL )
   {
     if( oldTree == NULL && newTree == NULL )
     {
       debugf("D(%s) Non-existent!\n", path);
       return;
     }
     if( oldTree == NULL )
     {
       debugf("D(%s) Created\n", path);
       return;
     }

     debugf("D(%s) Deleted\n", path);
     return;
   }

   //-------------------------------------------------------------------------
   // File compare
   //-------------------------------------------------------------------------
   oldE= oldTree->head;
   newE= newTree->head;
   for(;;)
   {
     if( oldE == NULL || newE == NULL )
     {
       if( oldE == NULL && newE == NULL )
         break;

       if( oldE == NULL )
       {
         once= ifFirst(once, "\nD: %s\n", path);
         debugf("New %s(%s)\n", newE->getType(), newE->fileName);
         newE= newE->next;
         continue;
       }

       once= ifFirst(once, "\nD: %s\n", path);
       debugf("Del %s(%s)\n", oldE->getType(), oldE->fileName);
       oldE= oldE->next;
       continue;
     }

     if( strcmp(oldE->fileName, newE->fileName) > 0 )
     {
       once= ifFirst(once, "\nD: %s\n", path);
       debugf("New %s(%s)\n", newE->getType(), newE->fileName);
       newE= newE->next;
       continue;
     }

     if( strcmp(oldE->fileName, newE->fileName) < 0 )
     {
       once= ifFirst(once, "\nD: %s\n", path);
       debugf("Del %s(%s)\n", oldE->getType(), oldE->fileName);
       oldE= oldE->next;
       continue;
     }

     // File names are identical
     #if 0
       debugf("\n");
       debugf("OLD: [%.8x.%.8x] time(%10u) size(%10u) name(%s)\n",
              (uint32_t)(oldE->checksum.getValue()>>32),
              (uint32_t)(oldE->checksum.getValue()),
              oldE->st_time, oldE->st_size, oldE->fileName);
       debugf("NEW: [%.8x.%.8x] time(%10u) size(%10u) name(%s)\n",
              (uint32_t)(newE->checksum.getValue()>>32),
              (uint32_t)(newE->checksum.getValue()),
              newE->st_time, newE->st_size, newE->fileName);
     #endif

     if( oldE->checksum == newE->checksum ) // Checksum is unchanged
     {
       if( !sw_dir                  // If directories are not interesting
           && strcmp(oldE->getType(), "D") == 0
           && strcmp(newE->getType(), "D") == 0 )
         ;
       else if( strcmp(oldE->getType(), newE->getType()) != 0 )
       {
         // The file type has been converted
         once= ifFirst(once, "\nD: %s\n", path);
         debugf("Typ %s(%s)\n", oldE->getType(), oldE->fileName);
       }
       else if( sw_status )
       {
         if( oldE->st_time != newE->st_time )
         {
           // The file has been re-created with a new time
           once= ifFirst(once, "\nD: %s\n", path);
           debugf("Tod %s(%s)\n", oldE->getType(), oldE->fileName);
         }

         else if( oldE->st_size != newE->st_size )
         {
           // The file has had zeros added at the end
           once= ifFirst(once, "\nD: %s\n", path);
           debugf("Siz %s(%s)\n", oldE->getType(), oldE->fileName);
         }

         else if( oldE->st_mode != newE->st_mode )
         {
           // The file attributes have changed
           once= ifFirst(once, "\nD: %s\n", path);
           debugf("Atr %s(%s)\n", oldE->getType(), oldE->fileName);
         }
         else
         {
           // Files are really identical
         }
       }
     }
     else                           // Checksum differs
     {
       if( oldE->st_time != newE->st_time )
       {
         // The file has changed, but so has the time
         once= ifFirst(once, "\nD: %s\n", path);
         debugf("Chg %s(%s)\n", oldE->getType(), oldE->fileName);
       }

       else
       {
         // An attempt was made to disguise a changed file
         once= ifFirst(once, "\nD: %s\n", path);
         debugf("SUM %s(%s)\n", oldE->getType(), oldE->fileName);
       }
     }

     oldE= oldE->next;
     newE= newE->next;
   }

   //-------------------------------------------------------------------------
   // Compare subdirectories
   //-------------------------------------------------------------------------
   oldE= oldTree->head;
   newE= newTree->head;
   for(;;)
   {
     if( oldE == NULL || newE == NULL )
     {
       break;
     }

     if( oldE->child == NULL )
     {
       oldE= oldE->next;
       continue;
     }

     if( newE->child == NULL )
     {
       newE= newE->next;
       continue;
     }

     if( strcmp(oldE->fileName, newE->fileName) > 0 )
     {
       newE= newE->next;
       continue;
     }

     if( strcmp(oldE->fileName, newE->fileName) < 0 )
     {
       oldE= oldE->next;
       continue;
     }

     FileName::concat(FQN, path, oldE->fileName);
     compareTree(FQN, oldE->child, newE->child);

     oldE= oldE->next;
     newE= newE->next;
   }
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
   OpenMedia           file;        // File

   DirArray*           oldTree;     // -> Previous directory tree
   DirArray*           newTree;     // -> Current directory tree

   char                initArea[FILENAME_MAX+1]; // Initial path
   int                 L;           // Working length
   int                 rc;

   //-------------------------------------------------------------------------
   // Parameter analysis
   //-------------------------------------------------------------------------
   parm(argc, argv);

   //-------------------------------------------------------------------------
   // Get the initial path
   //-------------------------------------------------------------------------
   if( initPath == NULL )
   {
     getcwd(initArea, sizeof(initArea)); // Get current working directory
     initPath= initArea;            // Use internal area
   }
   L= strlen(initPath);
   if( L > 1 )
   {
     if( initPath[L-1] == '\\' || initPath[L-1] == '/' )
       initPath[L-1]= '\0';
   }
   tracef("Path: %s\n", initPath);

   //-------------------------------------------------------------------------
   // Check the tree
   //-------------------------------------------------------------------------
   oldTree= NULL;
   rc= file.open(FILENAME, file.MODE_READ); // Open the input file
   if( rc == 0 )
   {
     oldTree= DirArray::get(file);  // Load the prior Tree
     if( oldTree == NULL )          // If load failure
     {
       fprintf(stderr, "Delete file(%s) to retry\n", FILENAME);

       exit(EXIT_FAILURE);
     }

     file.close();
   }

   newTree= DirArray::load(initPath, initPath); // Load the Tree
   if( newTree != NULL )
   {
     compareTree(initPath, oldTree, newTree); // Compare the trees

     rc= file.open(FILENAME, file.MODE_WRITE);
     if( rc != 0 )
     {
       fprintf(stderr, "File(%s), open(WR) failure\n", FILENAME);
       exit(EXIT_FAILURE);
     }
     newTree->put(file);            // Store the new Tree
     file.close();
   }
   else
     fprintf(stderr, "Path(%s) has no readable files\n", initPath);

   newTree->release();              // Delete the new Tree
   oldTree->release();              // Delete the old Tree

   return 0;
}

