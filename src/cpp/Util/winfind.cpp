//----------------------------------------------------------------------------
//
//       Copyright (c) 2015 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       winfind.cpp
//
// Purpose-
//       Whence command for Windows (ONLY).
//
// Last change date-
//       2015/01/01
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include <com/istring.h>

//----------------------------------------------------------------------------
// Missing macro
//----------------------------------------------------------------------------
#ifndef S_ISREG
#define S_ISREG(m) (((m)&_S_IFMT) == _S_IFREG)
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
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
#ifdef _OS_WIN
   if( argc > 1 )
   {
       char* name= argv[1];         // Name to check
       int   L= strlen(name);       // Name length
       if( L == 0 )
         return 0;

       int M= L;
       while( M > 0 )
       {
         M--;
         if( name[M] == '.' )
             break;
       }

       if( M == 0 || stricmp(".exe", name+M) != 0 )
       {
         name= (char*)malloc(L+8);
         strcpy(name, argv[1]);
         strcat(name, ".exe");
       }
       else
         name= strdup(name);
       L= strlen(name) + 8;         // (Spare room at end);

       char* path= getenv("PATH");  // Get path environment variable
       if( path != NULL )
       {
         path= strdup(path);
         // printf("PATH(%s)\n", path);
         char* C= path;
         while( *C != '\0' )
         {
           char* D= strchr(C, ';'); // Windows path delimiter
           if( D == NULL )
             D= C + strlen(C) - 1;
           else
             *D= '\0';

           int size= strlen(C);
           if( size > 0 )
           {
             char* test= (char*)malloc(size+L);
             strcpy(test, C);
             if( C[size-1] != '\\' )
               strcat(test, "\\");
             strcat(test, name);
             // printf("TEST(%s)\n", test);

             struct stat s;
             int rc= stat(test, &s);
             if( rc == 0 )
             {
               if( S_ISREG(s.st_mode) )
               {
                 printf("%s\n", test);
                 free(test);
                 break;
               }
             }

             free(test);
           }

           C= D+1;
         }

         free(path);
         free(name);
       }
   }
#endif // _OS_WIN

   return 0;
}

