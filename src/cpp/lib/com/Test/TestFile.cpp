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
//       TestFile.cpp
//
// Purpose-
//       Test file handling objects: FileList, FileName and FileInfo
//
// Last change date-
//       2020/06/13
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <exception>

#include <com/Debug.h>
#include "com/FileInfo.h"
#include "com/FileList.h"
#include "com/FileName.h"

//----------------------------------------------------------------------------
//
// Subroutine-
//       checkName
//
// Purpose-
//       Check a file name
//
//----------------------------------------------------------------------------
static void
   checkName(                       // Check FileName
     const char*       fileDesc)    // The file name
{
   printf("checkName(%s)\n", fileDesc);

   char                buffer[FILENAME_MAX+1+FILENAME_MAX+1];

   FileInfo info(fileDesc);
   printf(">>exists(%s)\n", info.exists() ? "True" : "False");
   printf(">>isFile(%s)\n", info.isFile() ? "True" : "False");
   printf(">>isLink(%s)\n", info.isLink() ? "True" : "False");
   printf(">>attrib(%s%s%s%s)\n",
          info.isPath()       ? "d" : "-",
          info.isReadable()   ? "r" : "-",
          info.isWritable()   ? "w" : "-",
          info.isExecutable() ? "x" : "-");

   FileName name(fileDesc);
   printf(">>FileDesc(%s)\n", name.getFileName());
   printf(">>PathOnly(%s) static\n", name.getPathOnly(buffer, name.getFileName()));
   printf(">>PathOnly(%s)\n", name.getPathOnly());
   printf(">>NamePart(%s)\n", name.getNamePart());
   printf(">>NameOnly(%s) static\n", name.getNameOnly(buffer, name.getFileName()));
   printf(">>NameOnly(%s)\n", name.getNameOnly());
   printf(">>Extension(%s)\n", name.getExtension());

   const char* R= name.resolve();   // Must insure sequence of calls
   const char* T= name.getTemporary();
   printf(">>resolve(%s) T(%s)\n", R, T);
   printf(">>FileDesc(%s)\n", name.getFileName());
   printf(">>PathOnly(%s) static\n", name.getPathOnly(buffer, name.getFileName()));
   printf(">>PathOnly(%s)\n", name.getPathOnly());
   printf(">>NamePart(%s)\n", name.getNamePart());
   printf(">>NameOnly(%s) static\n", name.getNameOnly(buffer, name.getFileName()));
   printf(">>NameOnly(%s)\n", name.getNameOnly());
   printf(">>Extension(%s)\n", name.getExtension());
   if( info.isPath() )
   {
     const char* pathName= name.getFileName();
     for(FileList list(pathName);;list.getNext())
     {
       const char* fileName= list.getCurrent();
       if( fileName == NULL )
         break;

       printf(">>contains(%s)\n", fileName);

       if( FileName::concat(buffer, sizeof(buffer), pathName, fileName) == NULL )
         strcpy(buffer, "<LENGTH_ERROR");
       FileInfo ofni(buffer);
       if( !ofni.exists() && !ofni.isLink() )
       {
         printf("..but !FileInfo(%s).exists()\n", buffer);
         throw "ShouldNotOccur";
       }
     }
   }

   printf(">>%s= appendPath(\"foo\")\n", name.appendPath("foo"));
   printf(">>%s= append(\".bar\")\n", name.append(".bar"));
   printf("\n");
}

static void
   checkName(                       // Check FileName
     const char*     pathName,      // The path name
     const char*     fileName)      // The file name
{
   printf("checkName(%s,%s)\n", pathName, fileName);

   FileName target(pathName, fileName);
   checkName(target.getFileName());
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       checkPath
//
// Purpose-
//       Check a path
//
//----------------------------------------------------------------------------
static void
   checkPath(                       // Check FileList
     const char*       pathName,    // The path name
     const char*       fileName)    // The file name
{
   printf("checkPath(%s,%s)\n", pathName, fileName);

   for(FileList list(pathName, fileName);;list.getNext())
   {
     const char* fileName= list.getCurrent();
     if( fileName == NULL )
       break;

     checkName(pathName, fileName);
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testCompare
//
// Purpose-
//       Test filename comparison
//
//----------------------------------------------------------------------------
static int
   testCompare( void )              // Test file name compare
{
   FileName nameA("/path/a");
   FileName nameB("/path/b");

   if( FileName::compare("/path/a", "/path/b") >= 0 )
   {
     fprintf(stderr, "%d compare(/path/a, /path/b)\n", __LINE__);
     return 2;
   }

   if( FileName::compare("/path/b", "/path/a") <= 0 )
   {
     fprintf(stderr, "%d compare(/path/b, /path/a)\n", __LINE__);
     return 2;
   }

   if( FileName::compare("/path/a", "/path/a") != 0 )
   {
     fprintf(stderr, "%d compare(/path/a, /path/a)\n", __LINE__);
     return 2;
   }

   if( nameA.compare("/path/b") >= 0 )
   {
     fprintf(stderr, "%d compare(nameA, /path/b)\n", __LINE__);
     return 2;
   }

   if( nameB.compare("/path/a") <= 0 )
   {
     fprintf(stderr, "%d compare(nameB, /path/a)\n", __LINE__);
     return 2;
   }

   if( nameA.compare("/path/a") != 0 )
   {
     fprintf(stderr, "%d compare(nameA, /path/a)\n", __LINE__);
     return 2;
   }

   if( nameA.compare(nameB) >= 0 )
   {
     fprintf(stderr, "%d compare(nameA, nameB)\n", __LINE__);
     return 2;
   }

   if( nameB.compare(nameA) <= 0 )
   {
     fprintf(stderr, "%d compare(nameB, nameA)\n", __LINE__);
     return 2;
   }

   if( nameA.compare(nameA) != 0 )
   {
     fprintf(stderr, "%d compare(nameA, nameA)\n", __LINE__);
     return 2;
   }

   #if defined(_OS_WIN) || defined(_OS_CYGWIN)
     if( FileName::compare("/path/a", "/path/B") >= 0 )
     {
       fprintf(stderr, "%d compare(/path/a, /path/B)\n", __LINE__);
       return 2;
     }

     if( FileName::compare("/path/B", "/path/a") <= 0 )
     {
       fprintf(stderr, "%d compare(/path/B, /path/a)\n", __LINE__);
       return 2;
     }

     if( FileName::compare("/path/a", "/path/A") != 0 )
     {
       fprintf(stderr, "%d compare(/path/a, /path/A)\n", __LINE__);
       return 2;
     }
   #endif

   return 0;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testLimits
//
// Purpose-
//       Test exact value of file name size limits
//
//----------------------------------------------------------------------------
static int
   testLimits( void )               // Test file name size limits
{
   char                result[FILENAME_MAX+1+FILENAME_MAX+1];
   char                both[FILENAME_MAX+1+FILENAME_MAX+1];
   char                name[FILENAME_MAX+2];
   char                path[FILENAME_MAX+2];

   const char*         CC;

   path[0]= '\0';
   while( strlen(path) < (FILENAME_MAX-2) )
     strcat(path, "p");
   strcat(path, FileName::getPathSeparator());

   name[0]= '\0';
   while( strlen(name) < (FILENAME_MAX-1) )
     strcat(name, "n");

   if( (CC= FileName::concat(both, sizeof(both), path, name)) == NULL )
   {
     fprintf(stderr, "%d concat(FILENAME_MAX)\n", __LINE__);
     return 2;
   }

   if( (CC= FileName::getNameOnly(result,both)) == NULL )
   {
     fprintf(stderr, "%d getNameOnly(FILENAME_MAX)\n", __LINE__);
     return 2;
   }

   if( (CC= FileName::getPathOnly(result,both)) == NULL )
   {
     fprintf(stderr, "%d getPathOnly(FILENAME_MAX)\n", __LINE__);
     return 2;
   }

   if( (CC= FileName::getExtension(result,both)) == NULL )
   {
     fprintf(stderr, "%d getPathOnly(FILENAME_MAX)\n", __LINE__);
     return 2;
   }

   //--------------------------------------------------------------------------
   // Now make the path and name too large
   path[strlen(path)-1]= 'p';
   strcat(path, "/");
   strcat(name, "n");

   if( (CC= FileName::concat(both, sizeof(both)-1, path, name)) == NULL )
   {
     fprintf(stderr, "%d concat(FILENAME_MAX*2)\n", __LINE__);
     return 2;
   }

   if( (CC= FileName::getNameOnly(result,both)) != NULL )
   {
     printf("%ld path(%s)\n", (long)strlen(path), path);
     printf("%ld name(%s)\n", (long)strlen(name), name);
     printf("%ld both(%s)\n", (long)strlen(both), both);
     printf("%ld sult(%s)\n", (long)strlen(result), result);
     fprintf(stderr, "%d getNameOnly(FILENAME_MAX+1)\n", __LINE__);
     return 2;
   }

   if( (CC= FileName::getPathOnly(result,both)) != NULL )
   {
     fprintf(stderr, "%d getPathOnly(FILENAME_MAX+1)\n", __LINE__);
     return 2;
   }

   if( (CC= FileName::concat(both, sizeof(both)-2, path, name)) != NULL )
   {
     fprintf(stderr, "%d concat(FILENAME_MAX*2+1)\n", __LINE__);
     return 2;
   }

   //--------------------------------------------------------------------------
   // Test resolve limits
   if( (CC= FileName::resolve(name, "/")) != NULL )
   {
     fprintf(stderr, "%d %s= resolve(/) %s\n", __LINE__, CC, result);
     return 2;
   }
   while( strlen(name) < (FILENAME_MAX-1) )
     strcat(name, "n");

   if( (CC= FileName::resolve(result,name)) != NULL )
   {
     fprintf(stderr, "%d %s= resolve(FILENAME_MAX) %s\n", __LINE__, CC, result);
     return 2;
   }

   strcat(name, "n");
   if( (CC= FileName::resolve(result,name)) == NULL )
   {
     fprintf(stderr, "%d %s= resolve(FILENAME_MAX+1) %s\n", __LINE__, "(null)", result);
     return 2;
   }

   return 0;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       tryBlock
//
// Purpose-
//       Mainline code.
//
//----------------------------------------------------------------------------
static int
   tryBlock(                        // Mainline code
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   int                 result= 0;   // Resultant

   int                 i;

   result= testCompare();
   if( result != 0 )
     return result;

   result= testLimits();
   if( result != 0 )
     return result;

   for(i= 1; i<argc; i++)
   {
     if( strcmp(argv[i], "-") == 0 )
       break;

     checkName(argv[i]);
   }

   const char* pathName= NULL;
   for(++i; i<argc; i++)
   {
     if( pathName == NULL )
       pathName= argv[i];
     else
     {
       checkPath(pathName, argv[i]);
       pathName= NULL;
     }
   }

   return result;
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
extern int
   main(                            // Mainline code
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   int                 result= 0;

   try {
     result= tryBlock(argc, argv);
   } catch(const char* x) {
     fprintf(stderr, "catch(const char*(%s))\n", x);
     result= 2;
   } catch(std::exception& x) {
     fprintf(stderr, "catch(exception.what(%s))\n", x.what());
     result= 2;
   } catch(...) {
     fprintf(stderr, "catch(...)\n");
     result= 2;
   }

   printf("Result(%d)\n", result);

   return result;
}

