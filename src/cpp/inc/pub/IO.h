//----------------------------------------------------------------------------
//
//       Copyright (c) 2026 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
// SPDX-License-Identifier: LGPL-3.0-only
//----------------------------------------------------------------------------
//
// Title-
//       IO.h
//
// Purpose-
//       Input/output subroutines using std::string file/path names
//
// Last change date-
//       2026/06/24
//
// Implementation notes-
//       The system-defined O_* and S_* macros are included.
//       Relative path names beginning with '/' become absolute path names
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_IO_H_INCLUDED
#define _LIBPUB_IO_H_INCLUDED

#include <stdexcept>                // For std::runtime_error
#include <string>                   // For std::string
#include <cstddef>                  // For size_t, ssize_t
#include <cstdio>                   // For FILE

#include <sys/fcntl.h>              // For O_* macros, ...
#include <sys/stat.h>               // For struct stat, S_* macros, ...

#include "pub/bits/pubconfig.h"     // For _LIBPUB_ macros

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
namespace io {
typedef int            fd_t;        // File Descriptor type
typedef std::string    string;      // For convenience (in namespace io)

//----------------------------------------------------------------------------
//
// Class-
//       pub::io::io_error
//
// Purpose-
//       Indicate I/O error
//
//----------------------------------------------------------------------------
class io_error : public std::runtime_error {
public:
   explicit io_error(const string& arg);
   explicit io_error(const char* arg);
}; // struct io_error

//----------------------------------------------------------------------------
//
// Subroutine-
//       pub::io::error_if
//
// Purpose-
//       Throw io_error exception if error condition
//
//----------------------------------------------------------------------------
_LIBPUB_PRINTF(2, 3)
void
   error_if(                        // Throw io_error if error
     int               cc,          // The condition to test (should be false)
     const char*       fmt,         // The printf format string
                       ...);        // The printf arguments

//----------------------------------------------------------------------------
//
// Subroutine-
//       pub::io::chmod
//
// Purpose-
//       Change file or path mode
//
//----------------------------------------------------------------------------
int                                 // Return code, 0 OK
   chmod(                           // Change file/path mode
     const string&     name,        // For this relative file/path name
     mode_t            mode);       // Into this file mode

//----------------------------------------------------------------------------
//
// Subroutine-
//       pub::io::fd_to_FILE        // Uses fdopen (Requires type)
//       pub::io::FILE_to_fd        // Uses fileno
//
// Purpose-
//       Convert file descriptor to FILE*
//       Convert FILE* to file descriptor
//
// Compatible fd_to_FILE types-
//       fd_to_FILE : open()
//             type : flags
//       ---------- : -----------------------------
//              "r" : O_RDONLY
//              "w" : O_WRONLY | O_CREAT | O_TRUNC
//              "a" : O_WRONLY | O_CREAT | O_APPEND
//             "r+" : O_RDWR
//             "w+" : O_RDWR   | O_CREAT | O_TRUNC
//             "a+" : O_RDWR   | O_CREAT | O_APPEND
//
//----------------------------------------------------------------------------
FILE*                               // The associated FILE*
   fd_to_FILE(                      // Convert file descriptor to FILE*
     fd_t              fd,          // The file descriptor to convert
     const char*       type);       // Type, must be compatible with open type

fd_t                                // The file descriptor
   FILE_to_fd(                      // Convert FILE* to file descriptor
     FILE*             file);       // The FILE* to convert

//----------------------------------------------------------------------------
//
// Subroutine-
//       pub::io::get_cwd           // (Uses getcwd) throws(io_error)
//       pub::io::set_cwd           // (Uses chdir)
//
// Purpose-
//       Get current working directory
//       Set current working directory
//
//----------------------------------------------------------------------------
std::string                         // The current working directory
   get_cwd( void );                 // Get current working directory

int                                 // Return code, 0 OK
   set_cwd(                         // Set current working directory
     const string&     path);       // To this relative path name

//----------------------------------------------------------------------------
//
// Subroutine-
//       pub::io::lstat
//       pub::io::stat
//
// Purpose-
//       Get file state without following links
//       Get file state, following links
//
//----------------------------------------------------------------------------
extern int                          // Return code, 0 OK
   lstat(                           // Get state information
     const string&     name,        // For this relative file/path name
     struct stat*      result);     // (OUTPUT) lstat information

extern int                          // Return code, 0 OK
   stat(                            // Get state information
     const string&     name,        // For this relative file/path name
     struct stat*      result);     // (OUTPUT) stat information

//----------------------------------------------------------------------------
//
// Subroutine-
//       pub::io::mkfifo            // (Uses mkfifo)
//       pub::io::rmfifo            // (Uses unlink)
//
// Purpose-
//       Create FIFO
//       Remove FIFO
//
// Implementation notes-
//       Subroutine rmfifo verifies that the name is a FIFO.
//
//----------------------------------------------------------------------------
int                                 // Return code, 0 OK
   mkfifo(                          // Create FIFO
     const string&     name,        // Creating this (relative) name
     mode_t            mode);

int                                 // Return code, 0 OK
   rmfifo(                          // Remove FIFO
     const string&     name);       // Having this (relative) name

//----------------------------------------------------------------------------
//
// Subroutine-
//       pub::io::mklink            // (Uses symlink)
//       pub::io::rmlink            // (Uses unlink)
//       pub::io::rdlink            // (Uses readlink)
//
// Purpose-
//       Create symbolic link
//       Remove symbolic link
//       Read symbolic link
//
// Implementation notes-
//       Subroutine rdlink adds a trailing '\0' if space is available, but
//         does not modify the return ssize_t value.
//       Subroutine rmlink verifies that the link name is a link.
//
//----------------------------------------------------------------------------
int                                 // Return code, 0 OK
   mklink(                          // Create symbolic link
     const string&     target,      // To this target name
     const string&     name);       // Creating this (relative) link name

int                                 // Return code, 0 OK
   rmlink(                          // Remove link
     const string&     name);       // Having this (relative) link name

ssize_t                             // Number of bytes read, >= 0 if OK
   rdlink(                          // Read symbolic link
     const string&     name,        // With this link name
     char*             addr,        // Into this buffer
     size_t            size);       // Of this size

//----------------------------------------------------------------------------
//
// Subroutine-
//       pub::io::mkpath            // (Uses mkdir)
//       pub::io::rmpath            // (Uses rmdir)
//       pub::io::rmfile            // (Uses unlink)
//
// Purpose-
//       Create directory
//       Remove directory
//       Remove file
//
//----------------------------------------------------------------------------
int                                 // Return code, 0 OK
   mkpath(                          // Create directory
     const string&     name,        // With this relative path name
     mode_t            mode);       // And this mode

int                                 // Return code, 0 OK
   rmpath(                          // Remove directory
     const string&     name);       // With this relative path name

int                                 // Return code, 0 OK
   rmfile(                          // Remove file
     const string&     name);       // With this relative file name

//----------------------------------------------------------------------------
//
// Subroutine-
//       pub::io::close(fd_t)
//       pub::io::open(string, int, ...)
//       pub::io::read(fd_t, void*, size_t)
//       pub::io::write(fd_t, const void*, size_t)
//
// Purpose-
//       Close file
//       Open file
//       Read from file
//       Write into file
//
//----------------------------------------------------------------------------
int                                 // Return code, 0 OK
   close(                           // Close file
     fd_t              fd);         // With this File Descriptor

fd_t                                // File descriptor, >=0 OK
   open(                            // Open file
     const string&     name,        // With this relative path name
     int               type);       // And these (O_*) flags

fd_t                                // File descriptor, >=0 OK
   open(                            // Open file
     const string&     name,        // With this relative path name
     int               type,        // And these (O_*) flags
     mode_t            mode);       // And this mode

ssize_t                             // Length read
   read(                            // Read from file
     fd_t              fd,          // File descriptor (from open)
     void*             addr,        // Buffer address
     size_t            size);       // Read length

ssize_t                             // Length written
   write(                           // Write into file
     fd_t              fd,          // File descriptor (from open)
     const void*       addr,        // Buffer address
     size_t            size);       // Write length
}  // namespace io
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_DATA_H_INCLUDED
