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
//       FSdups
//
// Purpose-
//       File system search for duplicates
//
// Last change date-
//       2010/02/01
//
// Usage-
//       FSdups
//
//----------------------------------------------------------------------------
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <exception>
#include <iostream>
#include <string>
typedef std::string string;

#include <com/define.h>
#include <com/FileData.h>
#include <com/FileInfo.h>
#include <com/FileList.h>
#include <com/FileName.h>
#include <com/List.h>

#ifdef _OS_BSD
  #include <netinet/in.h>          // For ntohl, htonl
#endif

#ifdef _OS_WIN
  #include <winsock2.h>            // For ntohl, htonl
#endif

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // Hard Core Debug Mode
#endif

#ifndef SCDM
#undef  SCDM                       // Soft Core Debug Mode
#endif

//----------------------------------------------------------------------------
//
// Struct-
//       Record
//
// Purpose-
//       Define a database record
//
//----------------------------------------------------------------------------
struct Record {                     // Database record
unsigned long          fileSize;    // The size of the file (in bytes)
char                   fileName[4096]; // The absolute name of the file

unsigned long                       // The file size
   getFileSize( void )              // Get file size
{
   return ntohl(fileSize);
}

void
   setFileSize(                     // Set file size
     unsigned long     fileSize)    // To this
{
   this->fileSize= htonl(fileSize);
}
}; // struct Record

class RecordLink : public AU_List<RecordLink>::Link {
public:
Record*                record;     // -> Record

   ~RecordLink( void );            // Destructor
   RecordLink( void );             // Constructor
}; // class RecordLink

class RecordList : public AU_List<RecordLink> {
public:
   ~RecordList( void );            // Destructor
   RecordList( void );             // Constructor
}; // class RecordList

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static int             argc;       // Argument count
static char**          argv;       // Argument array

RecordList             list;       // The list of Records
unsigned long          count;      // Number of records
Record**               array;      // Sorted record array

//----------------------------------------------------------------------------
//
// Subroutine-
//       outHCDM
//
// Function-
//       Write HCDM operator output message
//
//----------------------------------------------------------------------------
static inline void
   outHCDM(                         // Write operator message
     const char*       fmt,         // The PRINTF format string
                       ...)         // The remaining arguments
{
#if defined(HCDM)
   va_list             argptr;      // Argument list pointer

   va_start(argptr, fmt);           // Initialize va_ functions
   vprintf(fmt, argptr);
   va_end(argptr);                  // Close va_ functions
#else                               // Parameter unused without HCDM defined
   (void)fmt;
#endif
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       outSCDM
//
// Function-
//       Write SCDM operator output message
//
//----------------------------------------------------------------------------
static inline void
   outSCDM(                         // Write operator message
     const char*       fmt,         // The PRINTF format string
                       ...)         // The remaining arguments
{
#if defined(SCDM)
   va_list             argptr;      // Argument list pointer

   va_start(argptr, fmt);           // Initialize va_ functions
   vprintf(fmt, argptr);
   va_end(argptr);                  // Close va_ functions
#else                               // Parameter unused without HCDM defined
   (void)fmt;
#endif
}

//----------------------------------------------------------------------------
//
// Method-
//       RecordLink::~RecordLink
//
// Function-
//       Destructor
//
//----------------------------------------------------------------------------
   RecordLink::~RecordLink( void )  // Destructor
{
   outSCDM("RecordLink(%p)::~RecordLink()\n", this);

   if( record != NULL )
     free(record);

   record= NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       RecordLink::RecordLink
//
// Function-
//       Constructor
//
//----------------------------------------------------------------------------
   RecordLink::RecordLink( void )   // Constructor
:  record(NULL)
{
   outSCDM("RecordLink(%p)::RecordLink()\n", this);
}

//----------------------------------------------------------------------------
//
// Method-
//       RecordList::~RecordList
//
// Function-
//       Destructor
//
//----------------------------------------------------------------------------
   RecordList::~RecordList( void )  // Destructor
{
   outSCDM("RecordList(%p)::RecordList()\n", this);

   for(;;)
   {
     RecordLink* link= remq();
     if( link == NULL )
       break;

     delete link;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       RecordList::RecordList
//
// Function-
//       Constructor
//
//----------------------------------------------------------------------------
   RecordList::RecordList( void )   // Constructor
{
   outSCDM("RecordList(%p)::RecordList()\n", this);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       usage
//
// Function-
//       write usage information and exit
//
//----------------------------------------------------------------------------
static void
   usage(                           // Write usage information and exit
      int              argc,        // Argument count
      char*            argv[])      // Argument array
{
   (void)argc;                      // (Parameter unused)
   printf("Usage: %s\n"
          "Search filesystem (from current directory) looking for duplicates\n"
          "Duplicate file names are written to stdout\n"
          "\n"
          "Options:\n"
          "  <None available>\n"
          , argv[0]);

   exit(EXIT_FAILURE);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       parm
//
// Function-
//       Parameter analysis
//
//----------------------------------------------------------------------------
static void
   parm(                            // Parameter analysis
      int              argc,        // Argument count
      char*            argv[])      // Argument array
{
   int                 argx;        // Argument index

   for(argx= 1; argx<argc; argx++)
   {
     if( *argv[argx] != '-' )       // If not a switch
       break;                       // List of types found

     usage(argc, argv);
   }

   if( argx != argc )
     usage(argc, argv);

   ::argc= argc-argx;
   ::argv= &argv[argx];
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       dbLoad
//
// Function-
//       Load the database
//
//----------------------------------------------------------------------------
static void
   dbLoad(                          // Load the database
     const char*       path)        // Starting from this directory
{
   outSCDM("dbLoad(%s)\n", path);    // Say hello

   FileList fileList(path, "*");    // The (relative) directory
   for(;;fileList.getNext())        // Load database files
   {
     const char* name= fileList.getCurrent();
     if( name == NULL )
       break;

     if( strcmp(name, ".") == 0 || strcmp(name, "..") == 0 )
       continue;

     FileInfo fileInfo(path, name);
     if( !fileInfo.isLink() )       // If this is not a link
     {
       if( fileInfo.isFile()        // If this is a readable file
         && fileInfo.isReadable() )
       {
         Record record;
         record.setFileSize(fileInfo.getFileSize());
         FileName fileName(path, name);
         fileName.resolve();
         strcpy(record.fileName, fileName.getFileName());
         outHCDM("%8ld %s\n", record.getFileSize(), record.fileName);

         // Write the database record
         RecordLink* link= new RecordLink();
         size_t length= sizeof(record.fileSize) + strlen(record.fileName) + 1;
         link->record= (Record*)malloc(length);
         if( link->record == NULL )
           throw "Storage shortage";

         memcpy(link->record, &record, length);
         list.fifo(link);
       }
       else if( fileInfo.isPath()   // If this is a readable directory
         && fileInfo.isReadable() )
       {
         Record record;
         FileName fileName(path, name);
         fileName.resolve();
         strcpy(record.fileName, fileName.getFileName());
         dbLoad(record.fileName);
       }
     }
   }

   outHCDM("\n");
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       dbLoad
//
// Function-
//       Load the database
//
//----------------------------------------------------------------------------
static void
   dbLoad( void )                   // Load the database
{
   dbLoad(".");                     // Begin with the current directory

   RecordLink*         link;        // -> RecordLink
   unsigned long       i, j;

   ::count= 0;
   link= list.getTail();
   while( link != NULL )
   {
     ::count++;
     link= link->getPrev();
   }

   if( ::count > 0 )
   {
     array= (Record**)malloc(::count * sizeof(Record*));
     if( array == NULL )
       throw "Storage shortage";

     // Populate the Record* array
     link= list.getTail();
     for(i= 0; i < ::count; i++)
     {
       array[i]= link->record;
       link= link->getPrev();
     }

     // Sort the Record* array
     for(i= 0; i < ::count; i++)
     {
       for(j= i+1;j < ::count; j++)
       {
         if( array[i]->getFileSize() > array[j]->getFileSize() )
         {
           Record* swap= array[i];
           array[i]= array[j];
           array[j]= swap;
         }
         else if( array[i]->getFileSize() == array[j]->getFileSize() )
         {
           if( strcmp(array[i]->fileName, array[j]->fileName) > 0 )
           {
             Record* swap= array[i];
             array[i]= array[j];
             array[j]= swap;
           }
         }
       }
     }

     // Display the sorted Record* array
     #ifdef HCDM
       for(i= 0; i < ::count; i++)
       {
         Record* record= array[i];
         outHCDM("%8ld %s\n", record->getFileSize(), record->fileName);
       }
     #endif
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       dbScan
//
// Function-
//       Scan the database
//
//----------------------------------------------------------------------------
static void
   dbScan( void )                   // Scan the database
{
   outSCDM("dbScan()\n");

   if( array == NULL )
     return;

   for(unsigned long i= 0; i < ::count; i++)
   {
     Record* iRec= array[i];
     if( iRec == NULL )
       continue;

     outHCDM("%8ld %s\n", iRec->getFileSize(), iRec->fileName);
     FileData iFile(iRec->fileName);

     for(unsigned long j= i+1; j < ::count; j++)
     {
       Record* jRec= array[j];
       if( jRec == NULL )
         continue;

       outHCDM("..%8ld %s\n", jRec->getFileSize(), jRec->fileName);
       if( iRec->getFileSize() != jRec->getFileSize() )
         break;

       // Compare the files
       FileData jFile(jRec->fileName);
       if( iFile == jFile )
       {
         printf("%s == %s\n", iRec->fileName, jRec->fileName);
         array[j]= NULL;
       }
     }
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       main
//
// Function-
//       Mainline code.
//
//----------------------------------------------------------------------------
int
   main(                            // FSDUMP utility
      int              argc,        // Argument count
      char*            argv[])      // Argument array
{
   //-------------------------------------------------------------------------
   // Argument anlaysis
   //-------------------------------------------------------------------------
   parm(argc, argv);                // Parameter analysis

   //-------------------------------------------------------------------------
   // Operate, handling exceptions
   //-------------------------------------------------------------------------
   try {
     dbLoad();                      // Load the database
     dbScan();                      // Scan the database
   } catch(std::exception& x) {     // STL exception
     std::cerr << "STL exception: " << x.what() << std::endl;
   } catch(const char* x) {         // CHAR* exception
     std::cerr << "USR exception: " << x << std::endl;
   } catch(...) {                   // Other exception
     std::cerr << "SYSTEM exception" << std::endl;
   }

   return 0;
}

