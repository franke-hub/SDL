//----------------------------------------------------------------------------
//
//       Copyright (c) 2010 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       winsim.h
//
// Purpose-
//       In _OS_WIN, define simulated unix functions. [Library internal only!]
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
#ifndef WINSIM_H_INCLUDED
#define WINSIM_H_INCLUDED
#ifdef _OS_WIN

#include <stdio.h>                  // for FILENAME_MAX

//----------------------------------------------------------------------------
//
// Package-
//       dirent.h
//
// Structs-
//       dirent   (readdir resultant)
//       DIR      (directory handle)
//
// Subroutines-
//       closedir (close directory handle)
//       opendir  (open directory handle)
//       readdir  (read from directory)
//
//----------------------------------------------------------------------------
struct dirent {                     // readdir resultant
   char                d_name[FILENAME_MAX+1]; // Interesting part of resultant
}; // struct dirent

struct DIR {                        // Open directory handle
   int                 __flag;      // Status flags
   intptr_t            __handle;    // The _OS_WIN handle
   struct dirent       __dirent;    // (Resultant dirent)
}; // struct DIR

int                                 // Return code (0 OK)
   closedir(                        // Close directory
     DIR*              __handle);   // The directory handle

DIR*                                // -> DIR
   opendir(                         // Open directory
     const char*       __path);     // The path name

struct dirent*                      // -> dirent
   readdir(                         // Read (next) directory entry
     DIR*              __handle);   // The directory handle

//----------------------------------------------------------------------------
//
// Package-
//       sys/stat.h
//
// Macros-
//       S_ISDIR
//       S_ISFIFO
//       S_ISLNK
//       S_ISREG
//
//       S_IRUSR
//       S_IWUSR
//       S_IXUSR
//
//----------------------------------------------------------------------------
#ifndef S_ISDIR                     // Is regular file?
#define S_ISDIR(x) ((x&_S_IFMT) == _S_IFDIR)
#endif

#ifndef S_ISFIFO                    // Is pipe?
#define S_ISFIFO(x) ((x&_S_IFMT) == _S_IFIFO)
#endif

#ifndef S_ISLNK
#define S_ISLNK(x) FALSE            // Windows does not support links
#endif

#ifndef S_ISREG                     // Is regular file?
#define S_ISREG(x) ((x&_S_IFMT) == _S_IFREG)
#endif

#define S_IRUSR S_IREAD
#define S_IWUSR S_IWRITE
#define S_IXUSR S_IEXEC

//----------------------------------------------------------------------------
//
// Package-
//       unistd.h
//
// Subroutines-
//       readlink (Read link)
//
//----------------------------------------------------------------------------
int                                 // Read length (-1 iff error)
   readlink(                        // Read symbolic link
     const char*       __name,      // The link name
     char*             __result,    // Resultant buffer address
     int               __length);   // Resultant buffer length

#endif // _OS_WIN
#endif // WINSIM_H_INCLUDED
