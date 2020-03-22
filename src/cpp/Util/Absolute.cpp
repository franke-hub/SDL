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
//       Absolute.cpp
//
// Purpose-
//       Determine the absolute path to a file, removing all links.
//
// Last change date-
//       2007/01/01
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
#define __SOURCE__       "ABSOLUTE" // Source file

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

//----------------------------------------------------------------------------
// Versioning
//----------------------------------------------------------------------------
#if    defined(_OS_WIN)

#elif  defined(_OS_BSD)

#else
#error "Operating system not supported"
#endif

//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------
#define elements(array) sizeof(array)/sizeof(array[0])

//----------------------------------------------------------------------------
//
// Subroutine-
//       absolute
//
// Purpose-
//       Remove links from filename.
//
//----------------------------------------------------------------------------
#ifndef _OS_WIN
static char*                        // Resultant, NULL if error
   absolute(                        // Remove links from filename
     char*           result,        // Result fileName
     const char*     input)         // Source fileName
{
   char              source[DIM];   // Working source filename
   char              target[DIM];   // Working target filename
   char              linkName[DIM]; // Link name

   unsigned          recursion;     // Recursion counter
   struct stat       statBuff;      // Stat buffer
   unsigned          ndxSource;     // Working source index
   unsigned          ndxTarget;     // Working target index
   unsigned          endTarget;     // Last valid target index
   unsigned          lowTarget;     // Root target index

   int               rc;

   result[0]= '\0';
   source[0]= '\0';
   if( *input != '/' )              // If not absolute path
   {
     getcwd(source, sizeof(source));
     strcat(source, "/");
   }

   if( strlen(source) + strlen(input) >= sizeof(source) )
     return NULL;

   strcat(source, input);           // Set initial filename
   for(recursion=0; recursion<512; recursion++) // Remove links
   {
     #ifdef HCDM
       debugf("Source(%s)\n", source);
     #endif
     ndxSource= 0;
     ndxTarget= 0;

     #ifdef _OS_CYGWIN
       //---------------------------------------------------------------------
       // Handle CYGWIN drive specifier
       //---------------------------------------------------------------------
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
             return result;
           }
         }
       }
     #endif

     if( source[ndxSource] != '/' ) // If malformed name
       return NULL;

     target[ndxTarget++]= source[ndxSource++];
     lowTarget= ndxTarget;
     for(;;)                        // Look for first link in name
     {
       if( source[ndxSource] == '\0' ) // No links in name
       {
         if( ndxTarget >= MAX )
           return NULL;

         target[ndxTarget]= '\0';
         strcpy(result, target);
         return result;
       }

       endTarget= ndxTarget;        // Beginning of name
       while( source[ndxSource] != '/' && source[ndxSource] != '\0' )
       {
         if( ndxTarget >= DIM )
           return NULL;

         target[ndxTarget++]= source[ndxSource++];
       }
       target[ndxTarget]= '\0';     // The target is now a filename
       rc= lstat(target, &statBuff); // Get file information
       #ifdef HCDM
         debugf("%d= lstat(%s)\n", rc, target);
       #endif
       if( rc != 0 )                // If failure
       {
         if( ndxTarget < MAX )
         {
           strcpy(result, target);
           if( source[ndxSource] == '\0' )
             return result;
         }

         return NULL;
       }
       if( S_ISLNK(statBuff.st_mode) ) // If link
         break;

       //---------------------------------------------------------------------
       // Handle special file;
       //   prefix/./suffix => prefix/suffix
       //   prefix1/prefix2/../suffix => prefix1/suffix
       //   /../suffix => /suffix
       //---------------------------------------------------------------------
       if( target[endTarget] == '.' ) // If possible special file
       {
         if( target[endTarget+1] == '\0' )
           ndxTarget= endTarget-1;
         else if( target[endTarget+1] == '.' &&  target[endTarget+2] == '\0' )
         {
           ndxTarget= endTarget-1;
           if( endTarget > lowTarget )
           {
             ndxTarget--;
             while( target[ndxTarget] != '/' )
               ndxTarget--;
           }
         }
         #ifdef HCDM
           target[ndxTarget]= '\0';
           debugf(". target(%s)\n", target);
         #endif
       }

       //---------------------------------------------------------------------
       // Not a special file, continue
       //---------------------------------------------------------------------
       if( source[ndxSource] != '\0' )
         target[ndxTarget++]= source[ndxSource++]; // Copy the '/'
     }

     //-----------------------------------------------------------------------
     // The name is a link name
     //-----------------------------------------------------------------------
     memset(linkName, 0, sizeof(linkName));
     rc= readlink(target, linkName, sizeof(linkName));
     #ifdef HCDM
       debugf("%d= readlink(%s)='%s'\n", rc, target, linkName);
     #endif
     if( rc < 0 )                   // If failure
     {
       if( ndxTarget < MAX )
         strcpy(result, target);
       return NULL;
     }

     if( linkName[0] == '/' )       // If restart
       strcpy(target, linkName);
     else
     {
       while( target[ndxTarget] != '/' )
         ndxTarget--;
       target[ndxTarget+1]= '\0';

       if( (strlen(target)+strlen(linkName)) >= DIM )
         return NULL;
       strcat(target, linkName);
     }

     if( (strlen(target)+strlen(&source[ndxSource])) >= DIM )
       return NULL;

     strcat(target, &source[ndxSource]);
     #ifdef HCDM
       debugf(">>'%s'\n=>'%s'\n", source, target);
     #endif
     strcpy(source, target);
   }

   #ifdef HCDM
     debugf(">>'%s' has too many links\n", input);
   #endif
   return NULL;
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
   char*             result;        // Resultant
   char              string[DIM];   // (Overly large) string

   int               i;

   //-------------------------------------------------------------------------
   // Initialize
   //-------------------------------------------------------------------------
   for(i= 1; i<argc; i++)
   {
     result= absolute(string, argv[i]);
     if( result == NULL )
       printf("'%s' => (FAILURE):%s\n", argv[i], string);
     else
       printf("'%s' => '%s'\n", argv[i], result);
   }
#endif

   //-------------------------------------------------------------------------
   // Terminate
   //-------------------------------------------------------------------------
   return 0;
}

