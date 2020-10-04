//----------------------------------------------------------------------------
//
//       Copyright (c) 2010-2020 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       FileInfo.cpp
//
// Purpose-
//       Extract information about a particular file.
//
// Last change date-
//       2020/10/03
//
//----------------------------------------------------------------------------
#define __STDC_FORMAT_MACROS        // For linux inttypes.h
#include <inttypes.h>               // For PRId64

#include <stdio.h>                  // For FILENAME_MAX
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include <com/Clock.h>
#include <com/Debug.h>
#include <com/define.h>
#include <com/FileName.h>
#include <com/Julian.h>

#include "com/FileInfo.h"

#if   defined(_OS_BSD)
  #include <utime.h>                // For utime

#elif defined(_OS_WIN)
  #include <windows.h>              // For S_xxxxx
  #include <winsim.h>               // For S_xxxxx
  #include <io.h>                   // For chmod
  #include <sys/utime.h>            // For utime

#else
#error "Unsupported  version"

#endif // _OS_VER

#ifdef _OS_CYGWIN
  #define lstat64 lstat             // In CYGWIN, lstat == lstat64
  #define stat64  stat              // In CYGWIN, stat == stat64
#endif

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#define HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef MAX_SYMLINK
#define MAX_SYMLINK 128             // The maximum link recursion count
#endif

#define NANOSECONDS_PER_SECOND 1000000000

//----------------------------------------------------------------------------
//
// Enum-
//       winCONST
//
// Purpose-
//       Define WINDOWS constants and types.
//
//----------------------------------------------------------------------------
#ifdef _OS_WIN
enum winCONST                       // Windows constants
{  FILETIMES_PER_SECOND= 10000000   // (FILETIME intervals)/second
,  NANOSECONDS_PER_FILETIME= 100    // Nanoseconds/(FILETIME interval)
}; // enum winCONST

#ifndef INVALID_FILE_ATTRIBUTES
  #define INVALID_FILE_ATTRIBUTES (-1)
#endif

#define lstat64 winSTAT
#define stat64  winSTAT

struct timespec {
   time_t              tv_sec;      // Seconds
   long                tv_nsec;     // Nanoseconds
}; // struct timespec

struct winSTAT {
   uint64_t            st_size;     // File size, in bytes
   uint32_t            st_mode;     // File mode

   struct timespec     st_atim;     // Access time
   struct timespec     st_ctim;     // Create time
   struct timespec     st_mtim;     // Modify time
}; // struct

#define utimbuf inpTIME
#define utime   winTIME

struct inpTIME {
   time_t              actime;      // Access time
   time_t              modtime;     // Modification time
}; // struct inpTIME
#endif

//----------------------------------------------------------------------------
//
// Subroutine-
//       winINT64
//
// Purpose-
//       Create int64_t from highpart, lowpart.
//
//----------------------------------------------------------------------------
#ifdef _OS_WIN
int64_t                             // Resultant
   winINT64(                        // Create int64_t
     DWORD             high,        // High part
     DWORD             low)         // Low part
{
#if( TRUE )
   int64_t result= high;            // Set resultant
   result <<= 32;
   result |= low;
   return result;
#else
   ULARGE_INTEGER foo;              // Microsoft approved method
   foo.u.LowPart= low;
   foo.u.HighPart= high;
   return foo.QuadPart;
#endif
}
#endif

//----------------------------------------------------------------------------
//
// Class-
//       WinHAND
//
// Purpose-
//       Create a Windows handle (deleted by destructor.)
//
//----------------------------------------------------------------------------
#ifdef _OS_WIN
class WinHAND {                     // Windows handle
protected:
unsigned int           readOnly;    // TRUE iff read/only upgrade
HANDLE                 handle;      // Resultant handle
char*                  fileName;    // The file name

public:
inline
   ~WinHAND( void )                 // Destructor
{
   if( readOnly )                   // Upgrade from read only?
   {
     DWORD attributes= GetFileAttributes(fileName);
     if( attributes != INVALID_FILE_ATTRIBUTES )
       SetFileAttributes(fileName, attributes | FILE_ATTRIBUTE_READONLY);

     readOnly= FALSE;
   }

   if( intptr_t(handle) != HFILE_ERROR )
   {
     CloseHandle(handle);
     handle= (HANDLE)HFILE_ERROR;
   }

   if( fileName != NULL )
   {
     free(fileName);
     fileName= NULL;
   }
}

inline
   WinHAND(                         // Constructor
     const char*       name,        // File name
     unsigned int      access = GENERIC_READ) // File access mode
:  readOnly(FALSE)
,  handle((HANDLE)HFILE_ERROR)
,  fileName(NULL)
{
   fileName= strdup(name);          // Save the file name
   if( fileName == NULL )           // Storage shortage
     return;

   DWORD share= FILE_SHARE_READ | FILE_SHARE_WRITE;

   handle= (HANDLE)CreateFile(name, access,  share, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
   if( intptr_t(handle) == HFILE_ERROR )
   {
     DWORD attributes= GetFileAttributes(name);
     if( attributes == INVALID_FILE_ATTRIBUTES )
       return;

     if( (attributes & FILE_ATTRIBUTE_READONLY) != 0 )
     {
       if( SetFileAttributes(fileName, attributes & (~(FILE_ATTRIBUTE_READONLY))) )
         readOnly= TRUE;
     }

     handle= (HANDLE)CreateFile(name, access,  share, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
   }
}

inline HANDLE                       // The HANDLE
   getHandle( void ) const          // Get HANDLE
{
   return handle;
}
}; // class WinHAND
#endif

//----------------------------------------------------------------------------
//
// Subroutine-
//       winSTAT
//
// Purpose-
//       Windows STAT alternative.
//
// Implementation notes-
//       CYGWIN hides the status information in extended attributes.
//       We do not match this functionality.
//
//----------------------------------------------------------------------------
#ifdef _OS_WIN
struct timespec                     // Resultant
   filetimeToTimespec(              // Convert FILETIME to timespec
     const FILETIME&   filetime)    // The FILETIME
{
   struct timespec     result;      // Resultant

   Julian j= Julian::UTC1601;       // FILETIME time origin
   int64_t delta= winINT64(filetime.dwHighDateTime, filetime.dwLowDateTime);
   long  nanos= int(delta % FILETIMES_PER_SECOND) * NANOSECONDS_PER_FILETIME;
   delta /= FILETIMES_PER_SECOND;   // Number of seconds
   Julian i(double(delta) + double(nanos)/NANOSECONDS_PER_SECOND);
   j += i;

   Clock c(j);
   result.tv_sec=  c.getTime();
   result.tv_nsec= (c.getTime() - result.tv_sec) * NANOSECONDS_PER_SECOND;
   return result;
}

FILETIME                            // Resultant
   timeToFILETIME(                  // Convert time_t to FILETIME
     time_t            time)        // The time_t
{
   FILETIME            result;      // Resultant

   Clock c(time);
   Julian j(c);
   j -= Julian::UTC1601;            // Subtract Microsoft origin

   int64_t interval= j.getTime();   // The number of seconds
   uint32_t nanosec=  (j.getTime() - interval) * NANOSECONDS_PER_SECOND;
   interval *= FILETIMES_PER_SECOND;
   interval += (nanosec / NANOSECONDS_PER_FILETIME);

   result.dwLowDateTime= DWORD(interval);
   result.dwHighDateTime= DWORD(interval>>32);

   return result;
}

int                                 // Return code (0 OK)
   winSTAT(                         // Windows STAT function
     const char*       fileName,    // File name
     struct winSTAT*   result)      // Resultant
{
   BY_HANDLE_FILE_INFORMATION info;

   WinHAND hand(fileName);
   if( intptr_t(hand.getHandle()) != HFILE_ERROR ) // If handle valid
   {
     int rc= GetFileInformationByHandle(hand.getHandle(), &info);
     if( rc == 0 )                    // If some other problem
       return (-1);

     result->st_size= winINT64(info.nFileSizeHigh, info.nFileSizeLow);

     result->st_atim= filetimeToTimespec(info.ftLastAccessTime);
     result->st_ctim= filetimeToTimespec(info.ftCreationTime);
     result->st_mtim= filetimeToTimespec(info.ftLastWriteTime);
   }

   struct stat s;
   int rc= stat(fileName, &s);
   if( rc == 0 )
     result->st_mode= s.st_mode;

   return rc;
}
#endif

//----------------------------------------------------------------------------
//
// Struct/Subroutine-
//       winTIME
//
// Purpose-
//       Windows UTIME alternative.
//
//----------------------------------------------------------------------------
#ifdef _OS_WIN
int                                 // Return code (0 OK)
   winTIME(                         // Windows UTIME function
     const char*       fileName,    // File name
     const utimbuf*    buff)        // Update times
{
   FILETIME access= timeToFILETIME(buff->actime);
   FILETIME modify= timeToFILETIME(buff->modtime);

   int result= (-1);
   WinHAND hand(fileName, GENERIC_WRITE);
   if( SetFileTime(hand.getHandle(), NULL, &access, &modify) )
     result= 0;

   return result;
}
#endif

//----------------------------------------------------------------------------
//
// Subroutine-
//       fromTimespec
//
// Purpose-
//       Create Clock from struct timespec
//
//----------------------------------------------------------------------------
static inline Clock                 // Resultant
   fromTimespec(                    // Create Clock
     struct timespec&  spec)        // From this timespec
{
   Clock result(spec.tv_sec + double(spec.tv_nsec)/NANOSECONDS_PER_SECOND);
   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       FileInfo::~FileInfo
//
// Purpose-
//       Destructor
//
//----------------------------------------------------------------------------
   FileInfo::~FileInfo( void )      // Default destructor
{
   reset();
}

//----------------------------------------------------------------------------
//
// Method-
//       FileInfo::FileInfo
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
   FileInfo::FileInfo( void )       // Default constructor
:  fileName(NULL)
{
   reset();
}

   FileInfo::FileInfo(              // Construct, setFileName
     const char*       fileName)    // The absolute path/file name
:  fileName(NULL)
{
   reset(fileName);
}

   FileInfo::FileInfo(              // Construct, setFileName
     const char*       filePath,    // The absolute path (NULL for current)
     const char*       fileName)    // The relative file name
:  fileName(NULL)
{
   reset(filePath, fileName);
}

//----------------------------------------------------------------------------
//
// Method-
//       FileInfo::Accessors
//
// Purpose-
//       Accessor functions
//
//----------------------------------------------------------------------------
const char*                         // The fully qualified file name
   FileInfo::getFileName( void ) const // Get fully qualified file name
{
   return fileName;
}

const Clock&                        // Last access time
   FileInfo::getLastAccess( void ) const // Get last access time
{
   return lastAccess;
}

const Clock&                        // Last create time
   FileInfo::getLastCreate( void ) const // Get last create time
{
   return lastCreate;
}

const Clock&                        // Last modify time
   FileInfo::getLastModify( void ) const // Get last modify time
{
   return lastModify;
}

FileInfo::FileSize                  // Number of bytes in file
   FileInfo::getFileSize( void ) const // Get number of bytes in file
{
   return fileSize;
}

unsigned int                        // Permissions
   FileInfo::getPermissions( void ) const // Get permissions
{
   return mode & MODE_PERM;
}

int                                 // TRUE if file exists
   FileInfo::exists( void ) const   // Does file exist?
{
   return _exists;
}

int                                 // TRUE if associated file executable
   FileInfo::isExecutable( void ) const // Is this file executable?
{
   return (mode&S_IXUSR) != 0;
}

int                                 // TRUE if associated file exists
   FileInfo::isFile( void ) const   // Is this a file?
{
   return S_ISREG(mode);
}

int                                 // TRUE if this is (also) a link
   FileInfo::isLink( void ) const   // Is this a link?
{
   return _isLink;
}

int                                 // TRUE if file is a path (not a file)
   FileInfo::isPath( void ) const   // Is this a path?
{
   return S_ISDIR(mode);
}

int                                 // TRUE if file is a pipe (not a file)
   FileInfo::isPipe( void ) const   // Is this a pipe?
{
   return S_ISFIFO(mode);
}

int                                 // TRUE if file is readable
   FileInfo::isReadable( void ) const // Is this file readable?
{
   return (mode&S_IRUSR) != 0;
}

int                                 // TRUE if file is writable
   FileInfo::isWritable( void ) const // Is this file writable?
{
   return (mode&S_IWUSR) != 0;
}

int                                 // Return code (0 OK)
   FileInfo::setLastAccess(         // Set last access time
     const Clock&      access)      // To this time
{
   int result= (-1);                // Resultant

   if( _exists )
   {
     #ifdef _OS_WIN
       if( S_ISDIR(mode) )
         return 0;
     #endif

     utimbuf uti;
     uti.actime=  access.getTime();
     uti.modtime= lastModify.getTime();

     result= utime(fileName, &uti);
     if( result == 0 )
       lastAccess= access;
   }

   return result;
}

int                                 // Return code (0 OK)
   FileInfo::setLastModify(         // Set last modify time
     const Clock&      modify)      // To this time
{
   int result= (-1);                // Resultant

   if( _exists )
   {
     #if defined(_OS_WIN) || defined(_OS_CYGWIN)
       if( S_ISDIR(mode) )          // Directory time is not supported
         return 0;
     #endif

     utimbuf uti;
     uti.actime=  lastAccess.getTime();
     uti.modtime= modify.getTime();

     result= utime(fileName, &uti);
     if( result == 0 )
       lastModify= modify;
   }

   return result;
}

int                                 // Return code (0 OK)
   FileInfo::setPermissions(        // Set permissions
     unsigned int      permit)      // The permissions
{
   int result= (-1);                // Default (FAILURE)
   if( fileName != NULL )
   {
     permit &= MODE_PERM;
     result= chmod(fileName, permit);
     if( result == 0 )
     {
       mode &= ~(MODE_PERM);
       mode |= permit;
     }
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       FileInfo::debug
//
// Purpose-
//       Write debugging information
//
//----------------------------------------------------------------------------
void
   FileInfo::debug( void ) const    // Write debugging information
{
   debugf("FileInfo(%p)::debug()\n", this);
   debugf(" fileName(%s)\n", fileName);
   debugf(" fileSize(%" PRId64 ")\n", fileSize);
   debugf(" lastAccess(%f)\n", lastAccess.getTime());
   debugf(" lastCreate(%f)\n", lastCreate.getTime());
   debugf(" lastModify(%f)\n", lastModify.getTime());
   debugf(" _exists(%s)\n", _exists ? "TRUE" : "FALSE");
   debugf(" _isLink(%s)\n", _isLink ? "TRUE" : "FALSE");
   debugf(" mode(%.8x)\n", mode);
}

//----------------------------------------------------------------------------
//
// Method-
//       FileInfo::reset
//
// Purpose-
//       RESET the FileInfo
//
//----------------------------------------------------------------------------
void
   FileInfo::reset( void )          // RESET the FileInfo
{
   if( fileName != NULL )
     free(fileName);

   memset((char*)this, 0, sizeof(*this));
}

void
   FileInfo::reset(                 // RESET the FileInfo
     const char*       fileDesc)    // The absolute file name
{
   struct stat64       s;           // File status
   int                 rc;

   reset();
   fileName= strdup(fileDesc);
   if( fileName == NULL )
     return;

   rc= lstat64(fileName, &s);       // Get status, do not resolve link
   if( rc != 0 )                    // If failure
     return;                        // File non-existent

   if( S_ISLNK(s.st_mode) )         // If symbolic link
   {
     _isLink= TRUE;                 // Indicate Link
     rc= stat64(fileName, &s);      // Get status, resolve link
     if( rc != 0 )                  // If failure
       return;                      // File non-existent
   }

   fileSize= s.st_size;             // Set file size
   lastAccess= fromTimespec(s.st_atim); // Set dates
   lastCreate= fromTimespec(s.st_ctim);
   lastModify= fromTimespec(s.st_mtim);

   _exists= TRUE;                   // The object exists
   mode= s.st_mode;                 // Set the mode
}

void
   FileInfo::reset(                 // RESET the FileInfo
     const char*       filePath,    // The absolute file path (NULL for current)
     const char*       fileName)    // The relative file name
{
   reset();

   try {
     FileName name(filePath, fileName);
     reset(name.getFileName());
   } catch(...) {
   }
}

