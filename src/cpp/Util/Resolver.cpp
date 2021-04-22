//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2021 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Resolver.cpp
//
// Purpose-
//       Determine the absolute path to a file, resolving links one by one.
//
// Last change date-
//       2021/04/15
//
//----------------------------------------------------------------------------
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

#include <com/Debug.h>

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "RESOLVER" // Source file

#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE  1
#endif

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define DIM                    2048 // Working name size
#define MAX                     512 // Maximum resultant size

#define ERR_RETRY                 1 // Continue, name partially resolved
#define ERR_TOOBIG             (-1) // Overflow, name too large
#define ERR_DIRECTORY          (-2) // Path name is not a directory
#define ERR_MALFORMED          (-3) // Name is improperly formed
#define ERR_SYSTEM             (-4) // A system error occurred

//----------------------------------------------------------------------------
// Versioning
//----------------------------------------------------------------------------
#if    defined(_OS_WIN)

#elif  defined(_OS_BSD)

#else
#error  "Operating system not supported"
#endif

//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------
#define elements(array) sizeof(array)/sizeof(array[0])

//----------------------------------------------------------------------------
//
// Subroutine-
//       resolver
//
// Purpose-
//       Remove one link from a filename.
//
//----------------------------------------------------------------------------
#ifndef _OS_WIN
static int                          // Return code, <= 0 if complete
   resolver(                        // Remove links from filename
     char*           result,        // Result fileName
     char*           source)        // Source fileName
{
   char              target[DIM];   // Working target filename
   char              linkName[DIM]; // Link name

   struct stat       statBuff;      // Stat buffer
   unsigned          ndxSource;     // Working source index
   unsigned          ndxTarget;     // Working target index
   unsigned          endTarget;     // Last valid target index
   unsigned          lowTarget;     // Root target index

   int               rc;

   result[0]= '\0';
   if( *source != '/' )             // If not absolute path
   {
     if( getcwd(result, MAX) == nullptr )
       return ERR_SYSTEM;
     strcat(result, "/");
     if( strlen(result) + strlen(source) >= MAX )
       return ERR_TOOBIG;

     strcat(result, source);
     return ERR_RETRY;
   }

   #ifdef HCDM
     debugf("Source(%s)\n", source);
   #endif
   ndxSource= 0;
   ndxTarget= 0;

   #ifdef _OS_CYGWIN
     //-----------------------------------------------------------------------
     // Handle CYGWIN drive specifier
     //-----------------------------------------------------------------------
     if( source[0] == '/' && source[1] == '/' )
     {
       target[ndxTarget++]= '/';
       target[ndxTarget++]= '/';
       for(ndxSource= 2; ndxSource<MAX; ndxSource++)
       {
         if( source[ndxSource] == '/' )
           break;

         target[ndxTarget++]= source[ndxSource];
         if( source[ndxSource] == '\0' )
         {
           strcpy(result, target);
           return 0;
         }
       }
     }
   #endif

   if( source[ndxSource] != '/' )   // If malformed name
   {
     sprintf(result, "%d: (Internal logic error)", __LINE__);
     return ERR_MALFORMED;
   }

   target[ndxTarget++]= source[ndxSource++];
   lowTarget= ndxTarget;
   for(;;)                          // Look for first link in name
   {
     if( source[ndxSource] == '\0' )// No links in name
     {
       if( ndxTarget >= MAX )
       {
         strcpy(result, "(Source name too big)");
         return ERR_TOOBIG;
       }

       target[ndxTarget]= '\0';
       strcpy(result, target);
       return 0;
     }

     endTarget= ndxTarget;          // Beginning of name
     while( source[ndxSource] != '/' && source[ndxSource] != '\0' )
     {
       if( ndxTarget >= DIM )
       {
         strcpy(result, "(Source name too big)");
         return ERR_TOOBIG;
       }

       target[ndxTarget++]= source[ndxSource++];
     }
     target[ndxTarget]= '\0';       // The target is now a filename
     rc= lstat(target, &statBuff);  // Get file information
     #ifdef HCDM
       debugf("%d= lstat(%s)\n", rc, target);
     #endif
     if( rc != 0 )                  // If failure
     {
       if( source[ndxSource] == '\0' )
         return 0;

       source[ndxSource]= '\0';
       strcpy(result, "(Not a directory)");
       return ERR_DIRECTORY;
     }
     if( S_ISLNK(statBuff.st_mode) )// If link
       break;

     //-----------------------------------------------------------------------
     // Handle special file;
     //   prefix/./suffix => prefix/suffix
     //   prefix1/prefix2/../suffix => prefix1/suffix
     //   /../suffix => /suffix
     //-----------------------------------------------------------------------
     if( target[endTarget] == '.' ) // If possible special file
     {
       if( target[endTarget+1] == '\0' )
       {
         ndxTarget= endTarget-1;
         target[ndxTarget]= '\0';

         if( strlen(target) + strlen(&source[ndxSource]) >= MAX )
         {
           strcpy(result, "(Source name too big)");
           return ERR_TOOBIG;
         }
         strcat(target, &source[ndxSource]);
         strcpy(result, target);
         return ERR_RETRY;
       }

       if( target[endTarget+1] == '.' &&  target[endTarget+2] == '\0' )
       {
         ndxTarget= endTarget-1;
         if( endTarget > lowTarget )
         {
           ndxTarget--;
           while( target[ndxTarget] != '/' )
             ndxTarget--;
         }
         target[ndxTarget]= '\0';

         if( strlen(target) + strlen(&source[ndxSource]) >= MAX )
         {
           strcpy(result, "(Source name too big)");
           return ERR_TOOBIG;
         }
         strcat(target, &source[ndxSource]);
         strcpy(result, target);
         return ERR_RETRY;
       }
     }

     //-----------------------------------------------------------------------
     // Not a special file, continue
     //-----------------------------------------------------------------------
     if( source[ndxSource] != '\0' )
       target[ndxTarget++]= source[ndxSource++]; // Copy the '/'
   }

   //-------------------------------------------------------------------------
   // The name is a link name
   //-------------------------------------------------------------------------
   memset(linkName, 0, sizeof(linkName));
   rc= readlink(target, linkName, sizeof(linkName));
   #ifdef HCDM
     debugf("%d= readlink(%s)='%s'\n", rc, target, linkName);
   #endif
   if( rc < 0 )                     // If failure
   {
     source[ndxSource]= '\0';
     strcpy(result, "-> (unreadable link)");
     return ERR_SYSTEM;
   }

   printf("..%s -> %s\n", target, linkName);
   if( linkName[0] == '/' )         // If restart
     strcpy(target, linkName);
   else
   {
     while( target[ndxTarget] != '/' )
       ndxTarget--;
     target[ndxTarget+1]= '\0';

     if( (strlen(target)+strlen(linkName)) >= MAX )
     {
       strcat(&source[ndxSource], "->");
       strcpy(result, linkName);
       return ERR_TOOBIG;
     }

     strcat(target, linkName);
   }

   if( (strlen(target)+strlen(&source[ndxSource])) >= MAX )
   {
     strcat(&source[ndxSource], "->");
     strcpy(result, linkName);
     return ERR_TOOBIG;
   }

   strcat(target, &source[ndxSource]);
   strcpy(result, target);
   return ERR_RETRY;
}
#endif

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
     int             argc,          // Argument count
     char*           argv[])        // Argument array
{
#ifdef _OS_WIN
   printf("Windows not supported\n");
#else
   char              oldName[DIM];  // (Overly large) string
   char              newName[DIM];  // (Overly large) string
   unsigned          recursion;     // Recursion counter

   int               i;
   int               rc;

   //-------------------------------------------------------------------------
   // Initialize
   //-------------------------------------------------------------------------
   for(i= 1; i<argc; i++)
   {
     if( i > 1 )
       printf("\n");

     strcpy(oldName, argv[i]);
     rc= 0;
     for(recursion=0; recursion<512; recursion++) // Remove links
     {
       printf("::%s\n", oldName);
       rc= resolver(newName, oldName);
       if( rc <= 0 )
         break;

       strcpy(oldName, newName);
     }
     if( rc < 0 )
       printf("::%s => %s\n", oldName, newName);

     if( rc > 0 )
       printf("(Recursive)\n");
   }
#endif

   //-------------------------------------------------------------------------
   // Terminate
   //-------------------------------------------------------------------------
   return 0;
}

