//----------------------------------------------------------------------------
//
//       Copyright (c) 2014-2026 Frank Eskesen.
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
//       IoCommon.h
//
// Purpose-
//       Common I/O objects and subroutines used by RdClient and RdServer.
//
// Last change date-
//       2026/07/22
//
//----------------------------------------------------------------------------
#ifndef IOCOMMON_H_INCLUDED
#define IOCOMMON_H_INCLUDED

#include <cstdio>                   // For FILE*
#include <cstdlib>                  // For size_t

#include <limits.h>                 // For NAME_MAX, PATH_MAX
#include <time.h>                   // For timespec
#include <unistd.h>                 // For open, close, read, write

#include <pub/Data.h>               // For pub::data::File, pub::data::Path, ...
#include <pub/List.h>               // For pub::List
#include <pub/Thread.h>             // For pub::Thread

#include "RdCommon.h"               // For common objects and subroutines

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum                                // Generic enum
{  MAX_STRUCTSZ=       0x00007FFF   // Maximum structure size (32767)
,  MAX_TRANSFER=       0x00100000   // The size of the transfer buffer (1MB)

// The "well-known" port number
,  SERVER_PORT=        0x0000FEFE   // BSD: 65278

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// (Defined by /usr/include/limits.h)
// NAME_MAX=           255          // Chars in a file name (without nul)
// PATH_MAX=           4096         // Chars in a path name (including nul)
}; // Generic enum

static constexpr const char*        // CLIENT/SERVER interface version id.
                       RD_VERSION= "3.20130101";

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class  CommonThread;
struct PeerDesc;
struct RdFile;
struct RdPath;

//----------------------------------------------------------------------------
// Expose PUB types and methods
typedef PUB::Socket    Socket;      // Our Socket type

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Expose pub/IO.h types and methods
typedef PUB::io::fd_t  fd_t;        // File Descriptor type

#if 1
using PUB::io::chmod;               // Change mode (permission attributes)

using PUB::io::get_cwd;             // Get Current Working Directory
using PUB::io::set_cwd;             // Set Current Working Directory

using PUB::io::lstat;               // Get state, do not follow links
using PUB::io::stat;                // Get state, following links

using PUB::io::mkfifo;              // Create FIFO
using PUB::io::rmfifo;              // Remove FIFO

using PUB::io::mklink;              // Create link
using PUB::io::rmlink;              // Remove link
using PUB::io::rdlink;              // Read link

using PUB::io::mkpath;              // Create path (subdirectory)
using PUB::io::rmpath;              // Remove path
using PUB::io::rmfile;              // Remove file

// open, close, read, and write are defined in (the included) unistd.h
#endif

//----------------------------------------------------------------------------
enum HOST_INFO                      // Host_info fields and masks
{  INFO_UNUSED_BITS=   0x0FF00888   // Unassigned bits

   // Type
,  INFO_ISTYPE=        0xF0000000   // Mask for type
,  INFO_ISWHAT=        0x00000000   // TRUE iff unknown type
,  INFO_ISFILE=        0x10000000   // TRUE iff regular file
,  INFO_ISLINK=        0x20000000   // TRUE iff link
,  INFO_ISPATH=        0x30000000   // TRUE iff path
,  INFO_ISPIPE=        0x40000000   // TRUE iff pipe

   // Extended BSD attributes
,  INFO_AUID=          0x00008000   // TRUE iff ISUID attribute
,  INFO_AGID=          0x00004000   // TRUE iff ISGID attribute
,  INFO_AVTX=          0x00002000   // TRUE iff ISVTX attribute (Sticky)
,  INFO_AFMT=          0x00001000   // TRUE iff ENFMT attribute (Broken)

   // Permissions
,  INFO_RUSR=          0x00000400   // TRUE iff readable by user
,  INFO_WUSR=          0x00000200   // TRUE iff writable by user
,  INFO_XUSR=          0x00000100   // TRUE iff execable by user
,  INFO_RGRP=          0x00000040   // TRUE iff readable by group
,  INFO_WGRP=          0x00000020   // TRUE iff writable by group
,  INFO_XGRP=          0x00000010   // TRUE iff execable by group
,  INFO_ROTH=          0x00000004   // TRUE iff readable by other
,  INFO_WOTH=          0x00000002   // TRUE iff writable by other
,  INFO_XOTH=          0x00000001   // TRUE iff execable by other
,  INFO_RANY=          0x00000444   // TRUE if  readable by any
,  INFO_WANY=          0x00000222   // TRUE if  writable by any
,  INFO_XANY=          0x00000111   // TRUE if  execable by any
,  INFO_PERMITS=       0x000FF777   // Permissions mask
}; // enum HOST_INFO

enum FILE_TYPE                      // File type short names
{  FT_UNKNOWN=         'U'          // Unsupported thing
,  FT_FILE=            'F'          // Regular file
,  FT_PATH=            'D'          // Directory
,  FT_LINK=            'L'          // Soft link
,  FT_FIFO=            'P'          // FIFO (pipe)
}; // enum FILE_TYPE

typedef HOST64_t       Host_info_t;  // Host INFO (type and mode)
typedef HOST64_t       Host_ksum_t;  // Host checksum
typedef HOST64_t       Host_size_t;  // Host file size
typedef HOST64_t       Host_time_t;  // Host file time (Julian Second)

typedef PEER64_t       Peer_info_t;  // Peer INFO (type and mode)
typedef PEER64_t       Peer_ksum_t;  // Peer checksum
typedef PEER64_t       Peer_size_t;  // Peer file size
typedef PEER64_t       Peer_time_t;  // Peer file time (Julian Second)

//============================================================================
//
// Class-
//       Backout
//
// Purpose-
//       Backout recovery control.
//
//----------------------------------------------------------------------------
class Backout {                     // The Backout Object
//----------------------------------------------------------------------------
// Backout::Attributes
//----------------------------------------------------------------------------
protected:
RdFile*                file= nullptr; // The associated RdFile
fd_t                   fd= -1;      // The associated File Descriptor

//----------------------------------------------------------------------------
// Backout::Constructors/destructor/assignment operator
//----------------------------------------------------------------------------
public:
   Backout(                         // Constructor
     RdFile*           file,        // The associated RdFile
     fd_t              fd);         // The associated file descriptor

   Backout(const Backout&) = delete; // Disallowed copy constructor

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   ~Backout( void );                // Destructor (Performs backout)

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Backout&
   operator=(const Backout&) = delete; // Disallowed assignment operator

//----------------------------------------------------------------------------
// Backout::Methods
//----------------------------------------------------------------------------
void
   reset( void );                   // Cancel backout operation
}; // class Backout

//============================================================================
//
// Struct-
//       HostDesc
//
// Purpose-
//       Describe a file (HOST format).
//
//----------------------------------------------------------------------------
struct HostDesc {                   // File descriptor
   HostDesc( void ) = default;      // Default constructor
   HostDesc(const HostDesc&);       // Copy constructor
   HostDesc(const PeerDesc&);       // Copy from PeerDesc

HostDesc&
   operator=(const HostDesc&);      // Assignment operator
HostDesc&
   operator=(const PeerDesc&);      // Assignment operator

   Host_size_t         file_size;   // Number of bytes in file
   Host_info_t         file_info;   // Information about file
   Host_time_t         file_time;   // Time last modified
   Host_ksum_t         file_ksum;   // File checksum
}; // struct HostDesc

//============================================================================
//
// Struct-
//       PeerDesc
//
// Purpose-
//       Describe a file (network format).
//
//----------------------------------------------------------------------------
struct PeerDesc {                   // File descriptor
   PeerDesc( void ) = default;      // Default constructor
   PeerDesc(const PeerDesc&);       // Copy constructor
   PeerDesc(const HostDesc&);       // Copy from HostDesc

PeerDesc&
   operator=(const PeerDesc&);      // Assignment operator
PeerDesc&
   operator=(const HostDesc&);      // Assignment operator

   Peer_size_t         file_size;   // Number of bytes in file
   Peer_info_t         file_info;   // Information about file
   Peer_time_t         file_time;   // Time last modified
   Peer_ksum_t         file_ksum;   // File checksum
}; // struct PeerDesc

//============================================================================
//
// Struct-
//       PeerName (DOCUMENTATION ONLY -- NOT EXPLICITLY USED)
//
// Purpose-
//       Describe a Peer name string
//
// Implementation note-
//       See: CommonThread: rd_buff/data(string&) wr_buff/data(string&)
//
//----------------------------------------------------------------------------
struct PeerName {                   // Name descriptor
   PEER16_t            size;        // Length of name
   char                name[1];     // The name (without '\0' delimiter)
}; // struct PeerName

//============================================================================
//
// Struct-
//       PeerPair (DOCUMENTATION ONLY -- NOT EXPLICITLY USED)
//
// Purpose-
//       PeerDesc, PeerName pair.
//
//----------------------------------------------------------------------------
struct PeerPair {                   // File descriptor
   PeerDesc            desc;        // Descriptor
   PeerName            name;        // Name (string)
}; // struct PeerPair

//============================================================================
//
// Struct-
//       PeerPath (DOCUMENTATION ONLY -- NOT EXPLICITLY USED)
//
// Purpose-
//       Describe a list of files.
//
//----------------------------------------------------------------------------
struct PeerPath {                   // Directory descriptor
   PEER32_t            count;       // Number of PeerPair elements
// PeerPair            pair;        // The first PeerPair follows
}; // struct PeerPath

//============================================================================
//
// Struct-
//       PeerRequest
//
// Purpose-
//       Request descriptor.
//
//----------------------------------------------------------------------------
enum
{  REQ_FILE=                    'F' // Read File (string follows)
,  REQ_GOTO=                    'G' // Goto Path (string follows)
,  REQ_QUIT=                    'Q' // Exit from Path
,  REQ_VERSION=                 'V' // Return VERSIONID sequence
,  REQ_CWD=                     'P' // Return current working directory
};

struct PeerRequest {                // Request descriptor
   char              oc;            // Order code
}; // struct PeerRequest

//============================================================================
//
// Struct-
//       PeerResponse
//
// Purpose-
//       Response descriptor.
//
//----------------------------------------------------------------------------
enum
{  RSP_YO=             'Y'          // Operation accepted
,  RSP_NO=             'N'          // Operation failure
}; // enum

struct PeerResponse {               // Response descriptor
   char                rc;          // Response code
}; // struct PeerResponse

//============================================================================
//
// Struct-
//       RdFile
//
// Purpose-
//       Describe a directory entry.
//
//----------------------------------------------------------------------------
struct RdFile : public pub::data::File {
//----------------------------------------------------------------------------
// RdFile::Typedefs and enumerations
typedef pub::data::File   File;     // (Base File)
typedef pub::data::Path   Path;     // (Base Path)

//----------------------------------------------------------------------------
// RdFile::Attributes
// std::string         file_name;   // The file name (in pub::data::File)
// struct stat_t       st;          // The file info (in pub::data::File)

const RdPath*          path;        // The associated RdPath
HostDesc               desc;        // File descriptor (HOST format)
string                 link_name;   // The Link name (for links)

//----------------------------------------------------------------------------
// RdFile::Constructors/destructor
   RdFile(                          // Constructor
     const RdPath*     path,        // The associated RdPath
     const string&     name);       // (Fully qualified) file name

   RdFile(                          // Constructor
     const RdPath*     path,        // The associated RdPath
     const stat_t&     st,          // Stat descriptor
     const string&     name);       // File name

   RdFile(                          // Constructor
     const RdPath*     path,        // The associated RdPath
     const PeerDesc&   desc,        // PeerDesc descriptor
     const string&     name);       // File name

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
virtual
   ~RdFile( void ) = default;

//----------------------------------------------------------------------------
// RdFile::debug | Debugging display
virtual void
   debug(                           // Debugging display
     const char*       info= "") const override; // Caller information

//----------------------------------------------------------------------------
// RdFile::get_next | ACCESSOR: Get next RdFile* on list
// RdFile::get_prev | ACCESSOR: Get prior RdFile* on list
RdFile* get_next( void ) const { return (RdFile*)File::get_next(); }
RdFile* get_prev( void ) const { return (RdFile*)File::get_prev(); }

//----------------------------------------------------------------------------
// RdFile::compare_info | ACCESSOR: Compare desc.file_info
bool                                // TRUE iff !0
   compare_info(                    // Compare (this Client's) desc.file_info
     const RdFile*     server) const; // To this Server's desc.file_info

//----------------------------------------------------------------------------
// RdFile::compare_time | ACCESSOR: Compare desc.file_time
int                                 // (<0, =0, >0)
   compare_time(                    // Compare (this Client's) desc.file_time
     const RdFile*     server) const; // To this Server's desc.file_time

//----------------------------------------------------------------------------
// RdFile::display | Display this RdFile (in the log)
void
   display(                         // Display this RdFile
     const char*       info= "") const; // Heading information

//----------------------------------------------------------------------------
// RdFile::get_chmod | ACCESSOR: Get associated chmod permissions
int                                 // The associated chmod permissions
   get_chmod( void ) const;         // Get associated chmod permissions

//----------------------------------------------------------------------------
// RdFile::get_file_name | ACCESSOR: Get associated file_name (w/o path name)
// (Defined in base class)

//----------------------------------------------------------------------------
// RdFile::get_file_type | ACCESSOR: Get associated FILE_TYPE
int                                 // The associated FILE_TYPE
   get_file_type( void ) const;     // Get associated FILE_TYPE

//----------------------------------------------------------------------------
// RdFile::get_full_name | ACCESSOR: Get associated path_name + file_name
string
   get_full_name( void ) const;     // Get fully qualified name

//----------------------------------------------------------------------------
// RdFile::get_owner | ACCESSOR: Get associated CommonThread
const CommonThread*                 // The associated CommonThread
   get_owner( void ) const;         // Get associated CommonThread

//----------------------------------------------------------------------------
// RdFile::set_attr | ACCESSOR: Set file attributes (from HostDesc)
void
   set_attr( void );                // Set permissions and change time

//----------------------------------------------------------------------------
// RdFile::init_desc_* | Initialize the descriptor
void
   init_desc_info();                // Initialize descriptor info

int                                 // Return code, 0 expected
   init_desc_ksum();                // Initialize descriptor checksum

void
   init_desc_size();                // Initialize descriptor size

void
   init_desc_time();                // Initialize descriptor time
}; // struct RdFile

//============================================================================
//
// Struct-
//       RdPath
//
// Purpose-
//       Describe a directory entry.
//
//----------------------------------------------------------------------------
struct RdPath : public pub::data::Path, public pub::List<RdPath>::Link {
//----------------------------------------------------------------------------
// RdPath::Typedefs and enumerations
typedef pub::data::File   File;     // The base File
typedef pub::data::Path   Path;     // The base Path

//----------------------------------------------------------------------------
// RdPath::Attributes
// const std::string   path_name;   // The file name (in pub::data::Path)
// DHDL_sort<File>     list;        // The (sortable) list of Files
const CommonThread*    thread= nullptr; // The associated CommonThread

//----------------------------------------------------------------------------
// RdPath::Constructors/destructor
   RdPath(                          // Constructor
     const CommonThread*
                       owner);      // Owning CommonThread

   RdPath(                          // Constructor
     const CommonThread*
                       owner,       // Owning CommonThread
     const string&     name);       // The (relative) Path name

virtual
   ~RdPath( void ) = default;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void
   debug(                           // Debugging display
     const char*       info= "") const override; // Caller information

//----------------------------------------------------------------------------
// RdPath::Methods
RdFile*                             // The head RdFile
   get_head( void ) const;          // Get head RdFile

void
   fifo(                            // Append a File to the File list
     RdFile*           file);       // The RdFile to insert

void
   lifo(                            // Prepend a File to the File list
     RdFile*           file);       // The RdFile to insert

RdFile*                             // The associated RdFile
   locate(                          // Locate the RdFile
     const string&     name) const; // With this name

virtual File*                       // The new RdFile*
   make_file(                       // Create new RdFile*
     const stat_t&     st,          // Struct stat descriptor
     const string&     name) const override; // The File name

File*                               // The new RdFile*
   make_file(                       // Create a new RdFile*
     const PeerDesc&   desc,        // PeerDesc descriptor
     const string&     name) const; // The File name

RdFile*                             // The *NEXT* RdFile in the list
   remove_and_delete(               // Remove from the List and delete
     RdFile*           file);       // This RdFile

void
   reset(                           // Reset and load the directory
     const string&     name);       // Path name (Locally qualified)
}; // struct RdPath

//============================================================================
//
// Struct-
//       VersionInfo
//
// Purpose-
//       Describe version information.
//
//----------------------------------------------------------------------------
struct VersionInfo {                // Version information
char                   version[16]; // Version identifier
char                   f[8];        // Capability indicators

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Flag byte controls
enum VIF0                           // Flag byte [0] (Supported attributes)
{  VIF0_ABSD=          0x40         // BSD attributes supported
,  VIF0_CASE=          0x01         // Names with differing case are unique
}; // enum VIF0

enum VIF1                           // Flag byte [1] (Operating system)
{  VIF1_OBSD=          1            // Pure BSD O/S
,  VIF1_OCYG=          2            // CYGWIN O/S
}; // enum VIF1

enum VIF7                           // Flag byte [7] (Operational controls)
{  VIF7_KSUM=          0x01         // Get checksums for all files
}; // enum VIF7
}; // struct VersionInfo

//============================================================================
//
// (Static inline) subroutine-
//       get_file_type
//
// Purpose-
//       Extrace file type from file
//
//----------------------------------------------------------------------------
static inline int                   // The file type (character)
   get_file_type(                   // Get file type
     const RdFile*     file)        // From this RdFile
{  return file->get_file_type(); }
#endif // IOCOMMON_H_INCLUDED
