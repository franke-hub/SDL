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
//       FSlist.cpp
//
// Purpose-
//       Display files in the current directory, long format.
//
// Last change date-
//       2020/10/03
//
//----------------------------------------------------------------------------
#if 0
#include <assert.h>                // Used in timeTest
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>

#include <com/Calendar.h>
#include <com/Clock.h>
#include <com/define.h>
#include <com/FileInfo.h>
#include <com/FileList.h>
#include <com/FileName.h>
#include <com/Julian.h>             // Used in timeTest
#include <com/List.h>

#if defined(_OS_WIN)
  #include <sys/utime.h>
#else
  #include <utime.h>
#endif

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
//
// Class-
//       CompFile
//
// Purpose-
//       Comparable filename link.
//
//----------------------------------------------------------------------------
class CompFile : public SORT_List<CompFile>::Link { // Comparable file descriptor
//----------------------------------------------------------------------------
// CompFile::Attributes
//----------------------------------------------------------------------------
public:
FileName               name;        // The file name
FileInfo               info;        // The file information

//----------------------------------------------------------------------------
// CompFile::Constructors
//----------------------------------------------------------------------------
virtual inline
   ~CompFile( void ) {}             // Destructor

inline
   CompFile( void )                 // Default constructor
:  name(), info()
{}

//----------------------------------------------------------------------------
// CompFile::Methods
//----------------------------------------------------------------------------
inline int
   compare(                         // Compare
     const CompFile*   that) const  // To that CompFile
{
   return name.compare(that->name);
}

inline void
   reset(                           // Reset name and info
     const char*       fileName)    // With this file name
{
   name.reset(fileName);
   info.reset(fileName);
}

inline void
   reset(                           // Reset name and info
     const char*       filePath,    // With this path
     const char*       fileName)    // And this name
{
   name.reset(filePath, fileName);
   info.reset(filePath, fileName);
}
}; // class CompFile

// Implement method compare
template <> int
   SORT_List<CompFile>::Link::compare(
     const SORT_List<void>::Link*
                       that) const
{
   const CompFile* comp= dynamic_cast<const CompFile*>(that);
   if( comp == NULL )
     throw "SortCastException";

   return ((CompFile*)this)->compare(comp);
}

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static SORT_List<CompFile>
                       direct;      // Directory content
static char*           initPath= NULL;// Initial path

// Test date array
#define DIM_DATE 16
static Calendar        testDate[DIM_DATE]=
{  Calendar(), Calendar(), Calendar(), Calendar()
,  Calendar(), Calendar(), Calendar(), Calendar()
,  Calendar(), Calendar(), Calendar(), Calendar()
,  Calendar(), Calendar(), Calendar(), Calendar()
}; // testDate[]

#define DIM_TIME 16
static time_t          testTime[DIM_TIME]=
{   946702800,  962424000,  946702800,  946702800
,  1320552001, 1320555601, 1320559201, 1320562801
,   946702800,  946702800,  946702800,  946702800
,   946702800,  946702800,  946702800,  946702800
}; // testTime[]

//----------------------------------------------------------------------------
//
// Subroutine-
//       getSecond
//
// Purpose-
//       Return a clock second.
//
//----------------------------------------------------------------------------
static int64_t                      // The clock second
   getSecond(                       // Get clock second
     const Clock&      clock)       // For this Clock
{
   double time= clock.getTime();
   int64_t second= (int64_t)time;   // Get current second
   if( time < double(second) )      // Time must be >= second
     --second;

   return second;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       getNanosecond
//
// Purpose-
//       Return a clock nanosecond.
//
//----------------------------------------------------------------------------
static unsigned long                // The clock nanosecond
   getNanosecond(                   // Get clock nanosecond
     const Clock&      clock)       // For this Clock
{
   int64_t second= getSecond(clock); // Get current second
   return (unsigned long)((clock.getTime() - double(second)) * 1000000000);
}

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
     uint64_t          value)       // This one
{
   char                temp[32];    // temporary string
   int                 L;           // Length of temporary
// int                 sign;        // Sign of result (Value is UNSIGNED)

   int                 m;           // Modulo remainder
   int                 comma;       // Comma indicator

// sign= 1;
// if( value < 0 )                  // Value is UNSIGNED (must be positive)
// {
//   sign= (-1);
//   value= -value;
// }

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
// if( sign < 0 )                   // Value is UNSIGNED (must be positive)
//   temp[L++]= '-';
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
//       display
//
// Purpose-
//       Display a struct tm.
//
//----------------------------------------------------------------------------
static void
   display(                         // Display
     struct tm&        tod)         // This struct tm
{
   printf("%.2d/%.2d/%.4d  %.2d:%.2d:%.2d%s\n"
         , tod.tm_mon
         , tod.tm_mday
         , tod.tm_year+1900
         , tod.tm_hour
         , tod.tm_min
         , tod.tm_sec
         , tod.tm_isdst ? " ISDST" : ""
         );
}

static void
   display(                         // Display
     Calendar&         cal)         // This Calendarm
{
   printf("%.2d/%.2d/%.4d  %.2d:%.2d:%.2d\n"
         , cal.getMonth()
         , cal.getDay()
         , (int)cal.getYear()
         , cal.getHour()
         , cal.getMinute()
         , cal.getSecond()
         );
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       makeTree
//
// Purpose-
//       Create calendar files.
//
//----------------------------------------------------------------------------
static void
   makeTree( void )                 // Create calendar files
{
   int                 i;

   for(i= 0; i<DIM_DATE; i++)
   {
     Calendar& c= testDate[i];
     char workName[256];
     struct tm mdy;

     mdy.tm_year= c.getYear()-1900;
     mdy.tm_mon=  c.getMonth()-1;
     mdy.tm_mday= c.getDay();
     mdy.tm_hour= c.getHour();
     mdy.tm_min=  c.getMinute();
     mdy.tm_sec=  c.getSecond();
     mdy.tm_isdst= FALSE;
     time_t dot= mktime(&mdy);
     time_t tod= dot;

     if( mdy.tm_isdst != FALSE )
     {
       printf("  *** REDO *** "
             "FILE[%.2d]=%.4d-%.2d-%.2d+%.2d-%.2d-%.2d ISDST\n", i,
              mdy.tm_year+1900, mdy.tm_mon+1, mdy.tm_mday,
              mdy.tm_hour, mdy.tm_min, mdy.tm_sec);

       mdy.tm_year= c.getYear()-1900;
       mdy.tm_mon=  c.getMonth()-1;
       mdy.tm_mday= c.getDay();
       mdy.tm_hour= c.getHour();
       mdy.tm_min=  c.getMinute();
       mdy.tm_sec=  c.getSecond();
       tod= mktime(&mdy);

//     printf("%ld:%ld %s\n", (long)dot, (long)tod, (tod == dot) ? "==" : "!=");
     }

     // Crosscheck
     struct tm crc= *localtime(&tod);
     if(    mdy.tm_year  != crc.tm_year
         || mdy.tm_mon   != crc.tm_mon
         || mdy.tm_mday  != crc.tm_mday
         || mdy.tm_hour  != crc.tm_hour
         || mdy.tm_min   != crc.tm_min
         || mdy.tm_sec   != crc.tm_sec
         || mdy.tm_isdst != crc.tm_isdst )
     {
       printf("MDY: "); display(mdy);
       printf("CRC: "); display(crc);

       fprintf(stderr, "%4d Should Not Occur\n", __LINE__);
       exit(EXIT_FAILURE);
     }

     sprintf(workName, "FILE[%.2d]=%.4d-%.2d-%.2d+%.2d-%.2d-%.2d", i,
             mdy.tm_year+1900, mdy.tm_mon+1, mdy.tm_mday,
             mdy.tm_hour, mdy.tm_min, mdy.tm_sec);

     FILE* file= fopen(workName, "rb");
     if( file == NULL )
     {
       char temp[32];
       number(temp, tod);
       file= fopen(workName, "wb");
       fprintf(file, "%s %s\n", temp, workName);
     }
     fclose(file);

     #if( FALSE )
       utimbuf uti;
       uti.actime= tod;
       uti.modtime= tod;
       utime(workName, &uti);
     #else
       {{{{
         FileInfo time(workName);
         Clock c(tod);

         time.setLastAccess(c);
         time.setLastModify(c);
       }}}}
     #endif

     #if( FALSE )
       struct stat s;
       if( stat(workName, &s) == 0 )
       {
         if( tod != s.st_atime )
           printf("%4d Expected(%ld) Got(%ld) %s\n", __LINE__,
                  long(tod), long(s.st_atime), workName);
         if( tod != s.st_mtime )
           printf("%4d Expected(%ld) Got(%ld) %s\n", __LINE__,
                  long(tod), long(s.st_mtime), workName);
       }
     #endif

     FileInfo info(NULL, workName);
     time_t got= (time_t)info.getLastModify().getTime();
     if( got != tod )
       printf("%4d Expected(%ld) Got(%ld) %s\n", __LINE__,
              long(tod), long(got), workName);

     char workArea[32];
     number(workArea, tod);
     if( mdy.tm_isdst != FALSE )
       printf("%s %s ISDST\n", workArea, workName);
     else
       printf("%s %s\n", workArea, workName);
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       showTime
//
// Purpose-
//       Convert the times to calendar entries
//
//----------------------------------------------------------------------------
static void
   showTime( void )                 // Display the directory tree
{
   int                 i;

   printf("showTime()\n");
   const char* workName= "Test.file";
   FILE* file= fopen(workName, "rb");
   if( file == NULL )
   {
     file= fopen(workName, "wb");
     fprintf(file, "%s\n", workName);
   }
   fclose(file);

   for(i= 0; i<DIM_TIME; i++)
   {
     time_t tod= testTime[i];
     struct tm lclTM= *localtime(&tod);

     char workArea[32];
     number(workArea, tod);
     printf("%s %4d-%.2d-%.2d %.2d:%.2d:%.2d.%.9d (%dX) TIME[%2d]\n",
            workArea, lclTM.tm_year+1900, lclTM.tm_mon+1, lclTM.tm_mday,
            lclTM.tm_hour, lclTM.tm_min, lclTM.tm_sec, 0,
            lclTM.tm_isdst, i);

     #if( FALSE )
       utimbuf uti;
       uti.actime= tod;
       uti.modtime= tod;
       utime(workName, &uti);
     #else
       {{{{
         FileInfo time(workName);
         Clock c(tod);

         time.setLastAccess(c);
         time.setLastModify(c);
       }}}}
     #endif

     #if( FALSE )
       struct stat s;
       if( stat(workName, &s) != 0 )
         printf("%4d Missing file(%s)\n", __LINE__, workName);
       else
       {
         if( tod != s.st_atime )
           printf("%4d Expected(%ld) Got(%ld) %s\n", __LINE__,
                  long(tod), long(s.st_atime), workName);
         if( tod != s.st_mtime )
           printf("%4d Expected(%ld) Got(%ld) %s\n", __LINE__,
                  long(tod), long(s.st_mtime), workName);
       }
     #endif

     FileInfo info(NULL, workName);
     time_t got= (long)info.getLastModify().getTime();
     if( got != tod )
       printf("%4d Expected(%ld) Got(%ld) %s\n", __LINE__,
              long(tod), long(got), workName);
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       showTree
//
// Purpose-
//       Display the directory tree (using local time.)
//
//----------------------------------------------------------------------------
static void
   showTree( void )                 // Display the directory tree
{
   CompFile*           compFile;    // Working CompFile*
   char                initArea[FILENAME_MAX+1]; // Initial path
   int                 L;           // Working length

   //-------------------------------------------------------------------------
   // Get the initial path
   //-------------------------------------------------------------------------
   if( initPath == NULL )
   {
     initPath= initArea;            // Use internal area
     if( getcwd(initPath, sizeof(initArea)) == NULL ) // Get current working directory
     {
       fprintf(stderr, "%4d ShouldNotOccur\n", __LINE__);
       exit(EXIT_FAILURE);
     }
   }
   L= strlen(initPath);
   if( L > 1 )
   {
     if( initPath[L-1] == '\\' || initPath[L-1] == '/' )
       initPath[L-1]= '\0';
   }

   //-------------------------------------------------------------------------
   // Show the tree
   //-------------------------------------------------------------------------
   FileList list(initPath);         // Get the list of files

   while( TRUE )
   {
     const char* fileName= list.getCurrent();
     if( fileName == NULL )
       break;

     compFile= new CompFile();
     compFile->reset(initPath, fileName);
     direct.fifo(compFile);

     list.getNext();
   }

   direct.sort();
   compFile= (CompFile*)direct.getHead();
   while( compFile != NULL )
   {
     Clock c(compFile->info.getLastModify());
     time_t tod= (long)c.getTime();

     #if( FALSE )
       struct stat s;

       if( strcmp(compFile->info.getFileName(), compFile->name.getFileName()) != 0 )
         printf("%4d SNO %s %s\n", __LINE__, compFile->info.getFileName(), compFile->name.getFileName());

       const char* fileName= compFile->name.getFileName();
       if( stat(fileName, &s) != 0 )
         printf("%4d Missing file(%s)\n", __LINE__, fileName);
       else
         if( tod != s.st_mtime )
           printf("%4d Expected(%ld) Got(%ld) %s\n", __LINE__,
                  long(tod), long(s.st_mtime), fileName);
     #endif

     struct tm* ptrTM= localtime(&tod);
     struct tm lclTM= *ptrTM;
     long tempZone= timezone;
     struct tm gmtTM= *gmtime(&tod);
     gmtTM.tm_isdst= 0;
     time_t gmt= mktime(&gmtTM);

     int diff= int(difftime(tod,gmt)) / 60;

     char workArea[32];
     number(workArea, tod);
     number(initArea, compFile->info.getFileSize());
     printf("%s %4d-%.2d-%.2d %.2d:%.2d:%.2d.%.9ld %5ld %.4d (%d%d) %s %s\n",
            workArea, lclTM.tm_year+1900, lclTM.tm_mon+1, lclTM.tm_mday,
            lclTM.tm_hour, lclTM.tm_min, lclTM.tm_sec, getNanosecond(c),
            tempZone, diff, lclTM.tm_isdst, gmtTM.tm_isdst,
            initArea, compFile->name.getNamePart());
#if 0
     number(workArea, gmt);
     printf("%s %4d-%.2d-%.2d %.2d:%.2d:%.2d.%.9ld %5ld %.4d (%d%d) %s %s\n",
            workArea, gmtTM.tm_year+1900, gmtTM.tm_mon+1, gmtTM.tm_mday,
            gmtTM.tm_hour, gmtTM.tm_min, gmtTM.tm_sec, getNanosecond(c),
            tempZone, diff, gmtTM.tm_isdst, gmtTM.tm_isdst,
            initArea, compFile->name.getNamePart());
#endif

     compFile= (CompFile*)compFile->getNext();
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       timeTest
//
// Purpose-
//       Test Calendar/Clock/Julian.
//
//----------------------------------------------------------------------------
static inline void                  // (May be unused)
   timeTest( void )                 // Display the directory tree
{
   double   tod= 86400;             // 01/02/1970
   Clock    clock= tod;             // 01/02/1970
   Julian   julian(clock);
   Calendar cal(julian);
   Calendar c01(clock);
   Calendar c02(cal);

   printf("CAL: "); display(cal);
   printf("C01: "); display(c01);
   printf("C02: "); display(c02);

   printf("Jsec: %20.6f\n", julian.getTime());
   printf("Jday: %20.6f\n", julian.getTime()/86400.0);
   printf("Csec: %20.6f\n", clock.getTime());

   assert( clock  == cal.toClock() );
   assert( julian == cal.toJulian() );

   FileInfo info("FILE[15]=2000-01-01+00-00-00");
   info.setLastAccess(clock);
   info.setLastModify(clock);

   exit(0);
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
   info( void )                     // Display program information
{
   fprintf(stderr, "FSlist <options>\n");
   fprintf(stderr,
           "Options:\n"
           "  -d:Path (Specifies initial directory)\n"
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

   int                 error;       // Error encountered indicator
   int                 verify;      // Verify indicator

   //-------------------------------------------------------------------------
   // Defaults
   //-------------------------------------------------------------------------
   error=  FALSE;                   // Default, no error
   verify= FALSE;                   // Default, no verify

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

       if (strcmp(argp, "v") == 0 )
         verify= TRUE;

       else if (memcmp(argp, "d:", 2) == 0 )
       {
         initPath= argp + 2;
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

   if( verify )                     // If verify
   {
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       initCalendar
//
// Purpose-
//       Initialize Calendar entries
//
//----------------------------------------------------------------------------
static void
   initCalendar( void )             // Initialize Calendar array
{
   int                 i;

   for(i= 0; i<DIM_DATE; i++)
     testDate[i].setYMD(2000,1,1);

   testDate[0].setYMDHMSN(2000, 1, 1,   0, 0, 0);
   testDate[1].setYMDHMSN(2000, 7, 1,   0, 0, 1);
   testDate[2].setYMDHMSN(2008, 9,10,  11,12, 2);
   testDate[3].setYMDHMSN(2009,10,11,  12,34, 3);

   testDate[4].setYMDHMSN(2011,11, 6,  00,00, 4);
   testDate[5].setYMDHMSN(2011,11, 6,  01,00, 5);
   testDate[6].setYMDHMSN(2011,11, 6,  02,00, 6);
   testDate[7].setYMDHMSN(2011,11, 6,  03,00, 7);

   testDate[8].setYMDHMSN(2011,11, 7,  00,00, 8);
   testDate[9].setYMDHMSN(2011,12, 1,  00,00, 9);
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
   //-------------------------------------------------------------------------
   // Parameter analysis
   //-------------------------------------------------------------------------
   parm(argc, argv);
   initCalendar();

   //-------------------------------------------------------------------------
   // Process functions
   //-------------------------------------------------------------------------
// timeTest();                      // Test calendar functions
   makeTree();                      // Create local directory files
   showTree();                      // Display the local directory
   showTime();                      // Display the sample times

   return 0;
}
#else
#include <stdio.h>
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
   (void)argc; (void)argv;
   fprintf(stderr, "FSlist is deprecated\n");
   return 1;
}
#endif
