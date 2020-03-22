//----------------------------------------------------------------------------
//
//       Copyright (c) 2010-2014 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       FileInfo.h
//
// Purpose-
//       Extract information about a particular file.
//
// Last change date-
//       2014/01/01
//
// Usage notes-
//       For links, the is and get accessor methods return the status of the
//       link, not the link target. The set accessor methods update the link
//       target, not the link itself.
//
//----------------------------------------------------------------------------
#ifndef FILEINFO_H_INCLUDED
#define FILEINFO_H_INCLUDED

#include <stdint.h>

#ifndef CLOCK_H_INCLUDED
#include "Clock.h"
#endif

//----------------------------------------------------------------------------
//
// Class-
//       FileInfo
//
// Purpose-
//       Get information about a file.
//
//----------------------------------------------------------------------------
class FileInfo {
//----------------------------------------------------------------------------
// FileInfo::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
typedef uint64_t       FileSize;    // File size, in bytes

enum MODE                           // Defined mode bits
{  MODE_PERM= 0x000001FF            // (Permission bits)
,  MODE_RUSR= 0x00000100            // Read by user
,  MODE_WUSR= 0x00000080            // Write by user
,  MODE_XUSR= 0x00000040            // Exec by user
,  MODE_RGRP= 0x00000020            // Read by group
,  MODE_WGRP= 0x00000010            // Write by group
,  MODE_XGRP= 0x00000008            // Exec by group
,  MODE_ROTH= 0x00000004            // Read by other
,  MODE_WOTH= 0x00000002            // Write by other
,  MODE_XOTH= 0x00000001            // Exec by other
}; // MODE

//----------------------------------------------------------------------------
// FileInfo::Attributes
//----------------------------------------------------------------------------
protected:
char*                  fileName;    // The fully qualified file name
FileSize               fileSize;    // Number of bytes in file
Clock                  lastAccess;  // Last access time
Clock                  lastCreate;  // Last create time
Clock                  lastModify;  // Last modify time

unsigned               _exists  : 1;// TRUE if file or path exists
unsigned               _isLink  : 1;// TRUE if link
unsigned                        : 6;// Reserved for alignment
unsigned                        : 8;// Reserved for alignment
unsigned                        : 8;// Reserved for alignment
unsigned                        : 8;// Reserved for alignment

uint32_t               mode;        // Mode

//----------------------------------------------------------------------------
// FileInfo::Constructors
//----------------------------------------------------------------------------
public:
   ~FileInfo( void );               // Default destructor
   FileInfo( void );                // Default constructor

   FileInfo(                        // Construct, setFileName
     const char*       fileName);   // The absolute file name

   FileInfo(                        // Construct, setFileName
     const char*       filePath,    // The absolute file path (NULL for current)
     const char*       fileName);   // The relative file name

private:
   FileInfo(const FileInfo&);       // Bitwise copy not allowed
   FileInfo& operator=(const FileInfo&); // Bitwise assignment not allowed

//----------------------------------------------------------------------------
// FileInfo::methods
//----------------------------------------------------------------------------
public:
void
   debug( void ) const;             // Display debugging information

const char*                         // The fully qualified file name
   getFileName( void ) const;       // Get fully qualified file name

FileSize                            // Number of bytes in file
   getFileSize( void ) const;       // Get number of bytes in file

const Clock&                        // Last access time
   getLastAccess( void ) const;     // Get last access time

const Clock&                        // Last create time
   getLastCreate( void ) const;     // Get last create time

const Clock&                        // Last modify time
   getLastModify( void ) const;     // Get last modify time

unsigned int                        // The MODE permissions
   getPermissions( void ) const;    // Get MODE permissions

int                                 // TRUE if file exists
   exists( void ) const;            // Does file exist?

int                                 // TRUE if exists and is executable
   isExecutable( void ) const;      // Is this executable?

int                                 // TRUE if exists and is a file
   isFile( void ) const;            // Is this a file?

int                                 // TRUE if this is a link
   isLink( void ) const;            // Is this a link?

int                                 // TRUE if exists and is a path
   isPath( void ) const;            // Is this a path?

int                                 // TRUE if exists and is a pipe
   isPipe( void ) const;            // Is this a pipe?

int                                 // TRUE if exists and is readable
   isReadable( void ) const;        // Is this readable?

int                                 // TRUE if exists and is writeable
   isWritable( void ) const;        // Is this writeable?

void
   reset( void );                   // RESET the FileInfo

void
   reset(                           // RESET the FileInfo
     const char*       fileName);   // The file name

void
   reset(                           // RESET the FileInfo
     const char*       filePath,    // File Path
     const char*       fileName);   // File Name

int                                 // Return code (0 OK)
   setLastAccess(                   // Set last access time
     const Clock&      access);     // New last access time

int                                 // Return code (0 OK)
   setLastModify(                   // Set last modify time
     const Clock&      modify);     // New last modify time

int                                 // Return code (0 OK)
   setPermissions(                  // Set MODE permissions
     unsigned int      permit);     // New permissions
}; // class FileInfo

#endif // FILEINFO_H_INCLUDED
