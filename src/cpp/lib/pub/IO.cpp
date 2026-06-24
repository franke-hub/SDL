//----------------------------------------------------------------------------
//
//       Copyright (c) 2020-2026 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
// SPDX-License-Identifier: GPL-3.0-only
//----------------------------------------------------------------------------
//
// Title-
//       IO.cpp
//
// Purpose-
//       Input/output subroutines
//
// Last change date-
//       2026/06/24
//
//----------------------------------------------------------------------------
#include <string>                   // For std::string
#include <cerrno>                   // For errno
#include <climits>                  // For PATH_MAX
#include <cstddef>                  // For size_t, ssize_t
#include <cstring>                  // For strerror

#include <unistd.h>                 // For close, read, write, ...
#include <sys/fcntl.h>              // For open
#include <sys/stat.h>               // For struct stat, lstat, mkdir

#include <pub/IO.h>                 // For pub::io subroutines, implemented
#include <pub/utility.h>            // For pub::utility::to_stringv

#define PUB _LIBPUB_NAMESPACE

namespace _LIBPUB_NAMESPACE::io {   // The Data namespace
//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose
};

//----------------------------------------------------------------------------
//
// Subroutine-
//       is_fifo
//
// Purpose-
//       Verify that the specified name is for a fifo
//
//----------------------------------------------------------------------------
static bool                         // TRUE if name refers to a (relative) fifo
   is_fifo(                         // Verify that
     const string&     name)        // Name refers to a (relative) fifo
{
   struct stat info;
   int rc= PUB::io::lstat(name, &info); // Get state
   if( rc == 0 && ((info.st_mode & S_IFMT) == S_IFIFO) )
     return true;

   return false;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       is_link
//
// Purpose-
//       Verify that the specified name is for a link
//
//----------------------------------------------------------------------------
static bool                         // TRUE if name refers to a (relative) link
   is_link(                         // Verify that
     const string&     name)        // Name refers to a (relative) link
{
   struct stat info;
   int rc= PUB::io::lstat(name, &info); // Get link state
   if( rc == 0 && ((info.st_mode & S_IFMT) == S_IFLNK) )
     return true;

   return false;
}

//----------------------------------------------------------------------------
//
// Class-
//       pub::io::io_error::io_error
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
   io_error::io_error(const string& arg)
:  std::runtime_error(arg)
{  }

   io_error::io_error(const char* arg)
:  std::runtime_error(arg)
{  }

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
                       ...)         // The printf arguments
{
   if( cc == false )
     return;

   va_list             argptr;      // Argument list pointer

   va_start(argptr, fmt);           // Initialize va_ functions
   string what= utility::to_stringv(fmt, argptr);
   va_end(argptr);                  // Close va_ functions

   throw io_error(what + " " + std::to_string(errno) + ":"
                 + string(strerror(errno)));
}

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
     mode_t            mode)        // Into this file mode
{  return ::chmod(name.c_str(), mode); }

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
//----------------------------------------------------------------------------
FILE*                               // The associated FILE*
   fd_to_FILE(                      // Convert file descriptor to FILE*
     fd_t              fd,          // The file descriptor to convert
     const char*       type)        // Type, must be compatible with open type
{  return fdopen(fd, type); }

fd_t                                // The file descriptor
   FILE_to_fd(                      // Convert FILE* to file descriptor
     FILE*             file)        // The FILE* to convert
{  return fileno(file); }

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
string                              // The current working directory
   get_cwd( void )                  // Get current working directory
{
   char buffer[PATH_MAX + 8];       // Working buffer
   char* path= ::getcwd(buffer, sizeof(buffer));
   if( path )
     return path;

   string error("pub::io::get_cwd: ");
   error += std::to_string(errno);
   error += ":";
   error += strerror(errno);
   throw io_error(error);
}

int                                 // Return code, 0 OK
   set_cwd(                         // Set current working directory
     const string&     path)        // To this relative path name
{  return chdir(path.c_str()); }

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
int                                 // Return code, 0 OK
   lstat(                           // Get state information
     const string&     name,        // For this relative file/path name
     struct stat*      result)      // (OUTPUT) stat information
{  return ::lstat(name.c_str(), result); }

int                                 // Return code, 0 OK
   stat(                            // Get state information
     const string&     name,        // For this relative file/path name
     struct stat*      result)      // (OUTPUT) stat information
{  return ::stat(name.c_str(), result); }

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
//----------------------------------------------------------------------------
int                                 // Return code, 0 OK
   mkfifo(                          // Create FIFO
     const string&     name,        // Creating this (relative) name
     mode_t            mode)
{  return ::mkfifo(name.c_str(), mode); }

int                                 // Return code, 0 OK
   rmfifo(                          // Remove FIFO
     const string&     name)        // Having this (relative) name
{
   if( is_fifo(name) )
     return unlink(name.c_str());

   errno= EINVAL;
   return -1;
}

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
     const string&     name)        // Creating this (relative) link name
{  return symlink(target.c_str(), name.c_str()); }

int                                 // Return code, 0 OK
   rmlink(                          // Remove link
     const string&     name)        // Having this (relative) link name
{
   if( is_link(name) )
     return unlink(name.c_str());

   errno= EINVAL;
   return -1;
}

ssize_t                             // Number of bytes read, -1 if error
   rdlink(                          // Read symbolic link
     const string&     name,        // With this (relative) link name
     char*             addr,        // Into this buffer
     size_t            size)        // Of this size
{
   ssize_t ssize= readlink(name.c_str(), addr, size);
   if( ssize < 0 ) {
     if( size > 0 )
       addr[0]= '\0';
   } else if( (size_t)ssize < size ) {
     addr[ssize]= '\0';
   }
   return ssize;
}

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
     mode_t            mode)        // And this mode
{  return ::mkdir(name.c_str(), mode); }

int                                 // Return code, 0 OK
   rmpath(                          // Remove directory
     const string&     name)        // With this relative path name
{  return ::rmdir(name.c_str()); }

int                                 // Return code, 0 OK
   rmfile(                          // Remove file
     const string&     name)        // With this relative file name
{  return ::unlink(name.c_str()); }

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
     fd_t              fd)          // With this File Descriptor
{  return ::close(fd); }

fd_t                                // File descriptor, >=0 OK
   open(                            // Open file
     const string&     name,        // With this relative file name
     int               type)        // And these (O_*) flags
{  return ::open(name.c_str(), type); }

fd_t                                // File descriptor, >=0 OK
   open(                            // Open file
     const string&     name,        // With this relative file name
     int               type,        // And these (O_*) flags
     mode_t            mode)        // And this create mode
{  return ::open(name.c_str(), type, mode); }

ssize_t                             // Length read
   read(                            // Read from file
     fd_t              fd,          // File descriptor (from open)
     void*             addr,        // Buffer address
     size_t            size)        // Read length
{  return ::read(fd, addr, size); }

ssize_t                             // Length written
   write(                           // Write into file
     fd_t              fd,          // File descriptor (from open)
     const void*       addr,        // Buffer address
     size_t            size)        // Write length
{  return ::write(fd, addr, size); }
}  // namespace _LIBPUB_NAMESPACE::io
