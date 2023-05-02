//----------------------------------------------------------------------------
//
//       Copyright (c) 2010-2018 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       winsim.cpp
//
// Purpose-
//       Implement methods in winsim.h
//
// Last change date-
//       2018/01/01 Additional debugging features.
//
// Needs work-
//       TODO: Handle // and //computer_name/ prefix in Windows.
//       Note: Windows: dir \\computer_name always fails.
//
//----------------------------------------------------------------------------
#ifdef _OS_WIN
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include <com/Debug.h>
#include <com/FileName.h>

#include "winsim.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#include <com/ifmacro.h>            // After HCDM

//----------------------------------------------------------------------------
//
// Subroutine-
//       dirent.h::closedir
//
// Purpose-
//       WINDOWS closedir method.
//
//----------------------------------------------------------------------------
extern int                          // Return code (0 OK)
   closedir(                        // Close directory handle
     DIR*              handle)      // The handle
{
   IFHCDM( debugf("closedir(%p)\n", handle); )

   int result= _findclose(handle->__handle);
   IFHCDM( debugf("%d= _findclose(%p)\n", result, handle->__handle); )
   free(handle);
   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       dirent.h::opendir
//
// Purpose-
//       WINDOWS opendir method.
//
//----------------------------------------------------------------------------
extern DIR*                         // -> DIR (or NULL)
   opendir(                         // Open directory handle
     const char*       pathName)    // The path name
{
   DIR*                result= NULL;// Resultant
   char                workName[FILENAME_MAX+8];

   const int length= strlen(pathName);
   if( length == 0 || pathName[length-1] == '\\' || pathName[length-1] == '/' )
   {
     if( FileName::concat(workName, sizeof(workName), pathName, "*") == NULL )
       return NULL;
   }
   else if( FileName::concat(workName, sizeof(workName), pathName, "\\*") == NULL )
     return NULL;                   // Invalid pathName
   pathName= workName;

   struct _finddata_t ffBlock;      // FileFinding Block
   intptr_t handle= _findfirst(pathName, &ffBlock);
   IFHCDM( debugf("%p= _findfirst(%s)\n", handle, pathName); )
   if( handle >= 0 )
   {
     result= (DIR*)malloc(sizeof(DIR));
     IFHCDM( debugf("%p= malloc(%d)\n", result, sizeof(DIR)); )
     if( result != NULL )
     {
       result->__flag= 0;
       result->__handle= handle;
       strcpy(result->__dirent.d_name, ffBlock.name);
     }
   }

   IFHCDM( debugf("%p= opendir(%s)\n", result, pathName); )

   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       dirent.h::readdir
//
// Purpose-
//       WINDOWS readdir method.
//
//----------------------------------------------------------------------------
extern dirent*                      // -> dirent (or NULL)
   readdir(                         // Read directory
     DIR*              handle)      // The directory handle
{
   dirent*             result= NULL;// Resultant

   if( handle->__flag >= 0 )
   {
     result= &handle->__dirent;

     if( handle->__flag == 0 )
       handle->__flag= 1;
     else
     {
       struct _finddata_t ffBlock;    // FileFinding Block
       int rc= _findnext(handle->__handle, &ffBlock); // Return block
       IFHCDM( debugf("%d= _findnext(%p)\n", rc, handle->__handle); )
       if( rc != 0 )                  // If file not found
       {
         result= NULL;
         handle->__flag= (-1);
       }
       else
         strcpy(result->d_name, ffBlock.name);
     }
   }

   IFHCDM( debugf("%p= readdir(%p) %s\n", result, handle,
                  result ? result->d_name : "N/A");
   )

   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       unistd.h::readlink
//
// Purpose-
//       WINDOWS readlink method.
//
//----------------------------------------------------------------------------
extern int                          // Length read, -1 if error
   readlink(                        // Read a link
     const char*       linkName,    // The name of the link
     char*             buffer,      // Result buffer
     size_t            length)      // Result buffer length
{
   IFHCDM( debugf("%d ShouldNotOccur readlink(%s)\n", __LINE__, linkName); )

   return (-1);                     // HOW DID WE GET HERE?
}

//----------------------------------------------------------------------------
// End of File (_OS_WIN only)
#endif // _OS_WIN

