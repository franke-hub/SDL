//----------------------------------------------------------------------------
//
//       Copyright (c) 2014-2023 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       RdCommon.h
//
// Purpose-
//       Common controls.
//
// Last change date-
//       2023/08/03
//
//----------------------------------------------------------------------------
#ifndef RDCOMMON_H_INCLUDED
#define RDCOMMON_H_INCLUDED

#include <new>                      // For std::size_t
#include <string>                   // For std::string

#include <com/define.h>
#include <com/Thread.h>

//----------------------------------------------------------------------------
// Constants for parameterization
//
// Implementation notes-
//       The USE_ASYNCHRONOUS_LOADER control surprisingly causes slightly
//       worse completion times whether used on the client or the server.
//
//       The USE_CHECK_PERMISSIONS control avoids some error messages but
//       prevents modifications of R/O emtpy file times. (Optional)
//
//       The USE_EARLY_CLEANUP control prevents the build up of DirEntry
//       and DirList objects. Use it.
//----------------------------------------------------------------------------
#define BRINGUP        FALSE        // TRUE if bringup

#undef  USE_ASYNCHRONOUS_LOADER     // If defined, use asynchronous loader
#define USE_CHECK_PERMISSIONS       // If defined, use server permission checks
#define USE_EARLY_CLEANUP           // If defined, use early DirList delete

//----------------------------------------------------------------------------
// RD_VERSION: Version identifier
//----------------------------------------------------------------------------
#define RD_VERSION     "3.20130101" // CLIENT/SERVER version id.

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum                                // Generic constants
{  MAX_SENDSIZE=       1500         // If > zero, the largest send size
,  MAX_TRANSFER=       0x00100000   // The size of the transfer buffer

#if defined(_OS_WIN)
,  SERVER_PORT=        0x0000fefc   // The "well-known" port number (DOS)
#else
,  SERVER_PORT=        0x0000fefe   // The "well-known" port number (BSD)
#endif

// Now obsolete, handled by FileName object
,  MAX_DIRNAME=        512          // The largest size of a fileName part
,  MAX_DIRPATH=        512          // The largest size of a pathName part
,  MAX_DIRFILE=        1024         // The largest concatenated fileName
}; // enum

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------
#ifndef URHERE
#define URHERE() fprintf(stderr, "<%s> %d: ", __FILE__, __LINE__)
#endif

#ifndef SIZEOF
#define SIZEOF(s_name, s_elem) (long)sizeof(((s_name*)0)->s_elem)
#endif

#define TF(bool) ((bool == TRUE) ? "TRUE" : "FALSE")

//----------------------------------------------------------------------------
// OS dependencies
//----------------------------------------------------------------------------
#if defined(_OS_WIN)
  #define sleep _sleep
#endif

//----------------------------------------------------------------------------
// Enumerations and typedefs
//----------------------------------------------------------------------------
enum HOST_INFO                      // Value masks for HostInfo
{  INFO_UNUSED_BITS=   0x0FF00888   // Unassigned bits
,  INFO_WININFO=       0xF0000600   // Windows compatability mask

   // Type
,  INFO_ISTYPE=        0xF0000000   // Mask for type
,  INFO_ISWHAT=        0x00000000   // TRUE iff unknown type
,  INFO_ISFILE=        0x10000000   // TRUE iff regular file
,  INFO_ISLINK=        0x20000000   // TRUE iff link
,  INFO_ISPATH=        0x30000000   // TRUE iff path
,  INFO_ISPIPE=        0x40000000   // TRUE iff pipe

   // Windows-only attributes
,  INFO_ATTR_A=        0x00080000   // TRUE iff Archived
,  INFO_ATTR_S=        0x00040000   // TRUE iff System
,  INFO_ATTR_H=        0x00020000   // TRUE iff Hidden
,  INFO_ATTR_R=        0x00010000   // TRUE iff Read-only

   // Extended BSD attributes
,  INFO_AUID=          0x00008000   // TRUE iff ISUID attribute
,  INFO_AGID=          0x00004000   // TRUE iff ISGID attribute
,  INFO_AVTX=          0x00002000   // TRUE iff ISVTX attribute
,  INFO_AFMT=          0x00001000   // TRUE iff ENFMT attribute

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
};

enum FILE_TYPE                      // File type short names
{  FT_UNKNOWN=         'U'          // Unsupported thing
,  FT_PATH=            'D'          // Directory
,  FT_LINK=            'L'          // Soft link
,  FT_FILE=            'F'          // Regular file
,  FT_FIFO=            'P'          // FIFO (pipe)
};

typedef uint16_t       HOST16;      // Host format, 16 bit integer
typedef uint32_t       HOST32;      // Host format, 32 bit integer
typedef uint64_t       HOST64;      // Host format, 64 bit integer

typedef uint16_t       PEER16;      // Network format, 16 bit integer
typedef uint32_t       PEER32;      // Network format, 32 bit integer
typedef uint64_t       PEER64;      // Network format, 64 bit integer

typedef HOST64         HostInfo;    // Host info
typedef HOST64         HostKsum;    // Host checksum
typedef HOST64         HostSize;    // Host file size
typedef HOST64         HostTime;    // Host file date (Julian Second)

typedef PEER64         PeerInfo;    // Peer info
typedef PEER64         PeerKsum;    // Peer checksum
typedef PEER64         PeerSize;    // Peer file size
typedef PEER64         PeerTime;    // Peer file date (Julian Second)

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class Buffer;
class CommonThread;
class DirEntry;
class DirList;
class Socket;

//----------------------------------------------------------------------------
// Global data areas
//----------------------------------------------------------------------------
extern Buffer*         mx_buffer;   // Global MAX_TRANSFER Buffer

extern int             hcdm;        // Hard Core Debug Mode
extern int             scdm;        // Soft Core Debug Mode
extern int             iodm;        // In/Output Debug Mode

extern int             port;        // Connection port number
extern int             sw_erase;    // Erase remote target if it does
                                    // not exist locally
extern int             sw_older;    // Update remote target even if
                                    // source is newer
extern int             sw_quiet;    // Quiet mode
extern int             sw_unsafe;   // Unsafe mode (allow path mismatch)
extern int             sw_verify;   // Verify mode

//----------------------------------------------------------------------------
//
// Subroutine-
//       getFileType
//
// Purpose-
//       Extract FileType from HostInfo
//
//----------------------------------------------------------------------------
FILE_TYPE                           // The FILE_TYPE
   getFileType(                     // Get the FILE_TYPE
     HostInfo          info);       // From this HostInfo

//----------------------------------------------------------------------------
//
// Subroutine-
//       hostToPeer
//
// Purpose-
//       Convert HOST (any endian) to PEER (big endian) format.
//
//----------------------------------------------------------------------------
extern PEER16                       // PEER format
   hostToPeer(                      // Convert HOST format to PEER format
     HOST16            host16);     // HOST format short value

extern PEER32                       // PEER format
   hostToPeer(                      // Convert HOST format to PEER format
     HOST32            host32);     // HOST format int value

extern PEER64                       // PEER format
   hostToPeer(                      // Convert HOST format to PEER format
     HOST64            host64);     // HOST format long value

#if defined(_OS_WIN)
//----------------------------------------------------------------------------
//
// Subroutine-
//       isLink
//
// Function-
//       Determine whether a file is a simulated link.
//
//----------------------------------------------------------------------------
extern int                          // TRUE if simulated link
   isLink(                          // Is this a simulated link?
     const char*       fileName,    // The file name
     int               mode);       // The file mode
#endif

//----------------------------------------------------------------------------
//
// Subroutine-
//       makeFileName
//
// Purpose-
//       Combine a path and filename into a fully qualified name.
//
//----------------------------------------------------------------------------
extern std::string                  // Resultant
   makeFileName(                    // Generate fully qualified name
     const char*       path,        // Source path name
     const char*       name);       // Source file name

//----------------------------------------------------------------------------
//
// Inline subroutine-
//       min
//
// Purpose-
//       Return integer minimum value.
//
//----------------------------------------------------------------------------
#undef min
static inline uint64_t              // The minimum
   min(                             // Get minimum
     uint64_t          a,           // Comparand
     uint64_t          b)           // Comparand
{
   uint64_t result= a;              // Resultant
   if( b < a )
     result= b;

   return result;
}

#if defined(_OS_WIN)
//----------------------------------------------------------------------------
//
// Subroutine-
//       mkdir
//
// Function-
//       Windows ignores the second parameter for mkdir.
//
//----------------------------------------------------------------------------
extern int                          // Return code
   mkdir(                           // Invoke system mkdir
     const char*       dirName,     // The directory name
     int               mode);       // The directory mode (ignored)
#endif

//----------------------------------------------------------------------------
//
// Subroutine-
//       msgdump
//
// Purpose-
//       Debugging dump to msglog.
//
//----------------------------------------------------------------------------
extern void
   msgdump(                         // Debugging dump to log
     const void*       addr,        // Dump address
     unsigned long     size);       // Dump length

//----------------------------------------------------------------------------
//
// Subroutine-
//       msgerr
//
// Purpose-
//       Write an error message followed by perror()
//
//----------------------------------------------------------------------------
extern void
   msgerr(                          // Write message, perror()
     const char*       fmt,         // The PRINTF format string
                       ...)         // The remaining arguments
   _ATTRIBUTE_PRINTF(1, 2);

//----------------------------------------------------------------------------
//
// Subroutine-
//       msglog
//
// Purpose-
//       Write a log message.
//
//----------------------------------------------------------------------------
extern void
   msglog(                          // Write message to stdlog
     const char*       fmt,         // The PRINTF format string
                       ...)         // The remaining arguments
   _ATTRIBUTE_PRINTF(1, 2);

//----------------------------------------------------------------------------
//
// Subroutine-
//       msgout
//
// Purpose-
//       Write an output message.
//
//----------------------------------------------------------------------------
extern void
   msgout(                          // Write message to stdout
     const char*       fmt,         // The PRINTF format string
                       ...)         // The remaining arguments
   _ATTRIBUTE_PRINTF(1, 2);

//----------------------------------------------------------------------------
//
// Subroutine-
//       peerToHost
//
// Purpose-
//       Convert PEER (big endian) to HOST (any endian) format.
//
//----------------------------------------------------------------------------
extern HOST16                       // HOST format
   peerToHost(                      // Convert PEER format to HOST format
     PEER16            peer16);     // PEER format short value

extern HOST32                       // HOST format
   peerToHost(                      // Convert PEER format to HOST format
     PEER32            peer32);     // PEER format int value

extern HOST64                       // HOST format
   peerToHost(                      // Convert PEER format to HOST format
     PEER64            peer64);     // PEER format long value

//----------------------------------------------------------------------------
//
// Subroutine-
//       rdinit
//
// Purpose-
//       Initialize.
//
//----------------------------------------------------------------------------
extern void
   rdinit( void );                  // Initialize

//----------------------------------------------------------------------------
//
// Subroutine-
//       rdterm
//
// Purpose-
//       Terminate.
//
//----------------------------------------------------------------------------
extern void
   rdterm( void );                  // Terminate

#if defined(_OS_WIN)
//----------------------------------------------------------------------------
//
// Subroutine-
//       readlink
//
// Function-
//       Simulated readlink.
//
//----------------------------------------------------------------------------
extern int                          // (Failure) return code
   readlink(                        // Return error
     const char*       fileName,    // The file name
     char*             linkName,    // The link name
     int               size);       // Sizeof(linkName)
#endif

//----------------------------------------------------------------------------
//
// Subroutine-
//       strCompare
//
// Purpose-
//       Compare strings accounting for case.
//
//----------------------------------------------------------------------------
extern int                          // Resultant (per strcmp())
   strCompare(                      // Compare strings
     const CommonThread*
                       thread,      // Owning CommonThread
     const char*       source,      // Source string
     const char*       target);     // Target string

#if defined(_OS_WIN)
//----------------------------------------------------------------------------
//
// Subroutine-
//       symlink
//
// Function-
//       Simulated symlink.
//
//----------------------------------------------------------------------------
extern int                          // Return code
   symlink(                         // Simulated symlink
     const char*       linkName,    // The link name
     const char*       fileName);   // The file name
#endif


//----------------------------------------------------------------------------
//
// Struct-
//       Block
//
// Purpose-
//       Describe a generic storage Block
//
//----------------------------------------------------------------------------
struct Block {                      // Generic storage Block
Block*                 next;        // The next Block
}; // struct Block


//----------------------------------------------------------------------------
//
// Class-
//       Backout
//
// Purpose-
//       Backout recovery.
//
//----------------------------------------------------------------------------
class Backout {                     // The Backout Object
//----------------------------------------------------------------------------
// Backout::Attributes
//----------------------------------------------------------------------------
protected:
const char*            path;        // The associated path
DirEntry*              dirEntry;    // The associated DirEntry
int                    handle;      // The associated handle

//----------------------------------------------------------------------------
// Backout::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Backout( void );                // Destructor (Performs backout)
   Backout(                         // Constructor
     const char*       path,        // The associated path
     DirEntry*         dirEntry,    // The associated DirEntry
     int               handle);     // The associated file handle

private:                            // Bitwise copy is prohibited
   Backout(const Backout&);         // Disallowed copy constructor
   Backout& operator=(const Backout&); // Disallowed assignment operator

//----------------------------------------------------------------------------
// Backout::Methods
//----------------------------------------------------------------------------
public:
virtual void
   reset( void );                   // Cancel backout operation
}; // class Backout


//----------------------------------------------------------------------------
//
// Class-
//       Buffer
//
// Purpose-
//       Buffer allocator.
//
//----------------------------------------------------------------------------
class Buffer {                      // Buffer allocator object
public: // ===================================================================
class Auto {                        // Auto allocate/delete
protected:
Buffer*                buffer;      // The associated Buffer
void*                  block;       // The allocated buffer block

public:
  ~Auto( void );                    // Destructor
  Auto(                             // Constructor
    Buffer*            buffer);     // Allocate from this buffer

inline void*                        // The associated buffer block
  get( void ) const                 // Get associated buffer block
{ return block; }
}; // class Buffer::Auto =====================================================

//----------------------------------------------------------------------------
// Buffer::Attributes
//----------------------------------------------------------------------------
protected:
unsigned int           size;        // The buffer size
void*                  head;        // The head of the allocated buffer list

// Statistics
unsigned               aCount;      // Number of calls to allocate
unsigned               rCount;      // Number of calls to release
unsigned               uCount;      // Unallocated (available) buffer count

//----------------------------------------------------------------------------
// Buffer::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Buffer( void );                 // Destructor
   Buffer(                          // Constructor
     unsigned int      size);       // The size of each Buffer

private:                            // Bitwise copy is prohibited
   Buffer(const Buffer&);           // Disallowed copy constructor
   Buffer& operator=(const Buffer&); // Disallowed assignment operator

//----------------------------------------------------------------------------
// Buffer::Methods
//----------------------------------------------------------------------------
public:
void*                               // The allocated buffer
   allocate( void );                // Allocate a buffer

void
   release(                         // Release a buffer
     void*             buffer);     // The allocated buffer

void
   status( void );                  // Display Buffer statistics
}; // class Buffer


//----------------------------------------------------------------------------
//
// Class-
//       DirEntry
//
// Purpose-
//       Describe a directory entry.
//
//----------------------------------------------------------------------------
class DirEntry {                    // Directory element
//----------------------------------------------------------------------------
// DirEntry::Attributes
//----------------------------------------------------------------------------
public:
DirEntry*              next;        // Next element on list
const CommonThread*    owner;       // Associated CommonThread
DirList*               list;        // Subdirectory list

HostInfo               fileInfo;    // Information about file
HostTime               fileTime;    // Time last modified
HostSize               fileSize;    // Number of bytes in file
HostKsum               fileKsum;    // File checksum

char                   fileName[MAX_DIRNAME+1]; // The file name
char                   linkName[MAX_DIRNAME+1]; // For links, the target name

//----------------------------------------------------------------------------
// DirEntry::Constructors
//----------------------------------------------------------------------------
public:
   ~DirEntry( void );              // Destructor
   DirEntry(                       // Constructor
     const CommonThread*
                       owner);     // Owning CommonThread

//----------------------------------------------------------------------------
// DirEntry::Operators
//----------------------------------------------------------------------------
public:
void*
   operator new(std::size_t size);

void
   operator delete(void* addr);

//----------------------------------------------------------------------------
// DirEntry::Methods
//----------------------------------------------------------------------------
public:
int                                 // Return code (0 OK)
   checksum(                        // Compute checksum
     const char*       path);       // The file path

int                                 // Return code (0 EQ)
   compareInfo(                     // Compare hostInfo
     const DirEntry*   client);     // Server (source) DirEntry

int                                 // Time difference (<0, =0, >0)
   compareTime(                     // Compare fileTime
     const DirEntry*   that);       // Server (source) DirEntry

int
   chmod( void ) const;             // Get chmod parameter

void
   display(                         // Display the DirEntry
     const char*       info= "") const; // Informational header

void
   fromFile(                        // Initialize from file
     const char*       pathName,    // The source path name
     const char*       fileName);   // The file name

int
   intoFile(                        // Set file attributes
     const char*       pathName) const; // The target path name
}; // class DirEntry


//----------------------------------------------------------------------------
//
// Class-
//       DirList
//
// Purpose-
//       A sorted list of DirEntry objects.
//
// Implementation notes-
//       Starting the DirList Thread loads all the associated DirEntry.list
//       elements.
//
//----------------------------------------------------------------------------
class DirList : public Thread {     // Directory element list
//----------------------------------------------------------------------------
// DirList::Attributes
//----------------------------------------------------------------------------
public:
HOST32                 count;       // Number of elements
DirEntry*              head;        // -> First DirEntry element

const CommonThread*    owner;       // Associated CommonThread
char*                  path;        // Subdirectory path

//----------------------------------------------------------------------------
// DirList::Constructors
//----------------------------------------------------------------------------
   ~DirList( void );                // Destructor
   DirList(                         // Constructor (populates DirEntry list)
     const CommonThread*
                       owner,       // Owning CommonThread
     const char*       path,        // Subdirectory path
     const DirEntry*   ptrE);       // Associated file

   DirList(                         // Constructor (empty DirEntry list)
     const CommonThread*
                       owner,       // Owning CommonThread
     const char*       path);       // Subdirectory path

//----------------------------------------------------------------------------
// DirList::Operators
//----------------------------------------------------------------------------
public:
void*
   operator new(std::size_t size);

void
   operator delete(void* addr);

//----------------------------------------------------------------------------
// DirList::Methods
//----------------------------------------------------------------------------
public:
void
   display(                         // Display this object
     const char*       info= "") const; // And this identifier

DirEntry*                           // The inserted element
   insert(                          // Insert DirEntry onto list
     DirEntry*         ptrE,        // Element to insert
     DirEntry*         prior);      // Prior DirEntry

DirEntry*                           // -> DirEntry
   locate(                          // Locate directory entry
     char*             fileName);   // Filename to locate

DirEntry*                           // The next element
   remove(                          // Remove DirEntry from list
     DirEntry*         ptrE,        // Element to remove
     DirEntry*         prior);      // Prior DirEntry

virtual void
   runLoader( void );               // Foreground subdirectory loader

protected:
virtual long                        // The Thread's return code
   run( void );                     // Background subdirectory loader
}; // class DirList


//----------------------------------------------------------------------------
//
// Class-
//       InputBuffer
//
// Purpose-
//       Describe an input Buffer.
//       In this implementation, the CommonThread Transfer Buffer is used.
//
//----------------------------------------------------------------------------
class InputBuffer {                 // Input Buffer
//----------------------------------------------------------------------------
// InputBuffer::Attributes
//----------------------------------------------------------------------------
private:
CommonThread*          owner;       // -> Owning CommonThread
unsigned               used;        // Number of bytes used
unsigned               size;        // Current buffer size

//----------------------------------------------------------------------------
// InputBuffer::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~InputBuffer( void );            // Destructor
   InputBuffer(                     // Constructor
     CommonThread*     owner);      // -> Owning CommonThread

//----------------------------------------------------------------------------
// InputBuffer::Methods
//----------------------------------------------------------------------------
public:
const char*                         // The current buffer address
   getDataAddr( void );             // The current buffer address

unsigned                            // The current buffer length
   getDataSize( void );             // The current buffer length

void
   use(                             // Update the used buffer length
     unsigned          used);       // Incremental used length

int                                 // The next character
   getChar( void );                 // Get the next character

void
   fill( void );                    // Fill the buffer
}; // class InputBuffer


//----------------------------------------------------------------------------
//
// Class-
//       OutputBuffer
//
// Purpose-
//       Describe an output Buffer.
//       In this implementation, the CommonThread Transfer Buffer is used.
//
//----------------------------------------------------------------------------
class OutputBuffer {                // Output Buffer
//----------------------------------------------------------------------------
// OutputBuffer::Attributes
//----------------------------------------------------------------------------
private:
CommonThread*          owner;       // -> Owning CommonThread
unsigned               used;        // Number of bytes used
unsigned               size;        // Current buffer size

//----------------------------------------------------------------------------
// OutputBuffer::Constructors
//----------------------------------------------------------------------------
public:
   ~OutputBuffer( void );           // Destructor
   OutputBuffer(                    // Constructor
     CommonThread*     owner);      // -> Owning CommonThread

//----------------------------------------------------------------------------
// OutputBuffer::Methods
//----------------------------------------------------------------------------
public:
char*                               // The current buffer address
   getDataAddr( void );             // The current buffer address

unsigned                            // The current buffer length
   getDataSize( void );             // The current buffer length

void
   use(                             // Update the used buffer length
     unsigned          used);       // Incremental used length

void
   putChar(                         // Put the next character
     int               data);       // The next character

void
   empty( void );                   // Empty the buffer
}; // class OutputBuffer


//----------------------------------------------------------------------------
//
// Struct-
//       PeerDesc
//
// Purpose-
//       Describe a file (network format).
//
//----------------------------------------------------------------------------
struct PeerDesc {                   // File descriptor
   PeerSize            fileSize;    // Number of bytes in file
   PeerInfo            fileInfo;    // Information about file
   PeerTime            fileTime;    // Time last modified
   PeerKsum            fileKsum;    // File checksum
}; // struct PeerDesc


//----------------------------------------------------------------------------
//
// Struct-
//       PeerName
//
// Purpose-
//       Describe a fixed length data area.
//
//----------------------------------------------------------------------------
struct PeerName {                   // File descriptor
   PEER16              size;        // Length of data
   char                name[1];     // The data
}; // struct PeerName


//----------------------------------------------------------------------------
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
   PeerName            name;        // Name (variable length)
}; // struct PeerPair


//----------------------------------------------------------------------------
//
// Struct-
//       PeerPath
//
// Purpose-
//       Describe a list of files.
//
//----------------------------------------------------------------------------
struct PeerPath {                   // Directory descriptor
   PEER32              count;       // Number of PeerPair elements
// PeerPair            pair;        // The first PeerPair follows
}; // struct PeerPath


//----------------------------------------------------------------------------
//
// Struct-
//       PeerRequest
//
// Purpose-
//       Request descriptor.
//
//----------------------------------------------------------------------------
enum
{  REQ_FILE=                    'F' // Read File (PeerName follows)
,  REQ_GOTO=                    'G' // Goto Path (PeerName follows)
,  REQ_QUIT=                    'Q' // Exit from Path
,  REQ_VERSION=                 'V' // Return VERSIONID name
,  REQ_CWD=                     'P' // Return current working directory
};

struct PeerRequest {                // Request descriptor
   char              oc;            // Order code
}; // struct PeerRequest


//----------------------------------------------------------------------------
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


//----------------------------------------------------------------------------
//
// Struct-
//       VersionInfo
//
// Purpose-
//       Describe version information.
//
//----------------------------------------------------------------------------
struct VersionInfo {                // Version information
enum VIF0                           // Flag byte [0] (Supported attributes)
{  VIF0_AWIN=          0x80         // Windows attributes supported
,  VIF0_ABSD=          0x40         // BSD attributes supported
,  VIF0_CASE=          0x01         // Names with differing case are unique
}; // enum VIF0

enum VIF1                           // Flag byte [1] (Operating system)
{  VIF1_OMIX=          0            // Mixed O/S (local and remote differ)
,  VIF1_OBSD=          1            // Pure BSD O/S
,  VIF1_OCYG=          2            // CYGWIN O/S
,  VIF1_OWIN=          4            // Windows O/S
}; // enum VIF1

enum VIF7                           // Flag byte [7] (Operational controls)
{  VIF7_KSUM=          0x01         // Get checksums for all files
}; // enum VIF7

   char                version[16]; // Version identifier
   char                f[8];        // Capability indicators
}; // struct VersionInfo


//----------------------------------------------------------------------------
//
// Struct-
//       VersionData (DOCUMENTATION ONLY -- NOT EXPLICITLY USED)
//
// Purpose-
//       Describe variable length version information.
//
//----------------------------------------------------------------------------
struct VersionData {                // Version information
   PeerName            cwd;         // Current Working Directory
}; // struct VersionData

#endif // RDCOMMON_H_INCLUDED
