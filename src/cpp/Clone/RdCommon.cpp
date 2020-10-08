//----------------------------------------------------------------------------
//
//       Copyright (c) 2014-2020 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       RdCommon.cpp
//
// Purpose-
//       Common routines used by RdClient and RdServer.
//
// Last change date-
//       2020/10/03
//
// Environment variables-
//       LOG_HCDM=n    Hard Core Debug Mode verbosity
//       LOG_SCDM=n    Soft Core Debug Mode verbosity
//       LOG_IODM=n    In/Output Debug Mode size
//       LOG_FILE=name Log file name (rdist.log)
//
//----------------------------------------------------------------------------
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>               // For S_IREAD ...
#include <sys/types.h>

#include <com/Atomic.h>
#include <com/Clock.h>
#include <com/Debug.h>
#include <com/FileInfo.h>
#include <com/istring.h>
#include <com/Julian.h>
#include <com/Network.h>
#include <com/RecursiveBarrier.h>
#include <com/Signal.h>
#include <com/Software.h>
#include <com/Thread.h>
#include <com/Unconditional.h>

#if defined(_OS_WIN) 
  #define _WINSOCK2API_  /* Prevent inclusion of winsock2.h by windows.h */
  #define _WINSOCKAPI_   /* Prevent inclusion of winsock.h by windows.h */
  #include <windows.h>

  #ifndef WINSOCK_API_LINKAGE
  #ifdef DECLSPEC_IMPORT
  #define WINSOCK_API_LINKAGE DECLSPEC_IMPORT
  #else
  #define WINSOCK_API_LINKAGE
  #endif
  #endif

  #define WSAAPI                  FAR PASCAL
  typedef unsigned char   u_char;
  typedef unsigned short  u_short;
  typedef unsigned int    u_int;
  typedef unsigned long   u_long;

  extern "C" WINSOCK_API_LINKAGE int WSAAPI WSAGetLastError( void );
  extern "C" WINSOCK_API_LINKAGE u_long WSAAPI ntohl( u_long netlong );
  extern "C" WINSOCK_API_LINKAGE u_short WSAAPI ntohs( u_short netshort );
  extern "C" WINSOCK_API_LINKAGE u_long WSAAPI htonl( u_long hostlong );
  extern "C" WINSOCK_API_LINKAGE u_short WSAAPI htons( u_short hostshort );
#else
  #include <dirent.h>

  extern "C" uint32_t ntohl(uint32_t);
  extern "C" uint16_t ntohs(uint16_t);
  extern "C" uint32_t htonl(uint32_t);
  extern "C" uint16_t htons(uint16_t);
#endif

#include "RdCommon.h"
#include "CommonThread.h"
#include "ocrw.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define DEFAULT_HOST    "127.0.0.1" // The default host name
#define LOG_FILENAME    "rdist.log" // The logfile name

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef SCDM
#undef  SCDM                        // If defined, Soft Core Debug Mode
#endif

//----------------------------------------------------------------------------
// Dependent macros
//----------------------------------------------------------------------------
#include <com/ifmacro.h>

//----------------------------------------------------------------------------
// OS dependencies
//----------------------------------------------------------------------------
#if defined(_OS_WIN) || defined(_OS_CYGWIN)
  #ifndef INVALID_FILE_ATTRIBUTES
    #define INVALID_FILE_ATTRIBUTES (-1)
  #endif

  #define A_Axxx (FILE_ATTRIBUTE_ARCHIVE)
  #define A_xSxx (FILE_ATTRIBUTE_SYSTEM)
  #define A_xxHx (FILE_ATTRIBUTE_HIDDEN)
  #define A_xxxR (FILE_ATTRIBUTE_READONLY)
  #define A_ASHR (A_Axxx | A_xSxx | A_xxHx | A_xxxR)
#endif

//----------------------------------------------------------------------------
// Global data areas
//----------------------------------------------------------------------------
Buffer*                mx_buffer= NULL; // Global MAX_TRANSFER Buffer

int                    hcdm;        // Hard Core Debug Mode
int                    scdm;        // Soft Core Debug Mode
int                    iodm;        // In/Output Debug Mode

int                    port= SERVER_PORT; // The server port
int                    sw_erase= FALSE; // Erase remote target if it does
                                    // not exist locally
int                    sw_older= FALSE; // Update remote target even if
                                    // source is newer
int                    sw_quiet= FALSE; // Quiet mode
int                    sw_unsafe= FALSE; // Unsafe mode
int                    sw_verify= FALSE; // Verify mode

//----------------------------------------------------------------------------
// Local data areas
//----------------------------------------------------------------------------
static Backout*        backout= NULL; // The active Backout object
static RecursiveBarrier
                       barrier= RECURSIVEBARRIER_INIT; // Single thread latch
static Signal*         mySignal= NULL; // Signal handler
static FILE*           stdlog= NULL; // Log file handle
static Block*          dirEntry= NULL; // The head dirEntry Block
static Block*          dirList= NULL;  // The head dirList  Block


//----------------------------------------------------------------------------
//
// Subroutine-
//       abortHandler
//
// Function-
//       Backout function.
//
//----------------------------------------------------------------------------
static void
   abortHandler( void )             // Handle backout
{
   msglog("abortHandler()\n");

   if( backout != NULL )            // If a Backout object exists
     backout->~Backout();           // Invoke its destructor
}

//----------------------------------------------------------------------------
//
// Class-
//       Direct
//
// Purpose-
//       Describe a directory.
//
//----------------------------------------------------------------------------
class Direct {                      // Directory
//----------------------------------------------------------------------------
// Direct::Attributes
//----------------------------------------------------------------------------
public:
#ifdef _OS_WIN
   long                handle;      // FileFinding Handle
   struct _finddata_t  block;       // FileFinding Block

#else
   DIR*                stream;      // The directory stream
#endif

//----------------------------------------------------------------------------
// Direct::Constructors
//----------------------------------------------------------------------------
public:
   ~Direct( void );                 // Destructor
   Direct(                          // Constructor
     const char*         pathName); // Path

//----------------------------------------------------------------------------
// Direct::Methods
//----------------------------------------------------------------------------
public:
const char*                         // The next file name
   getNext( void );                 // Get next file name
}; // class Direct

//----------------------------------------------------------------------------
//
// Class-
//       MySignal
//
// Purpose-
//       My signal handler.
//
//----------------------------------------------------------------------------
class MySignal : public Signal {
public:
virtual int                         // Return code (0 iff handled)
   handle(                          // Signal handler
     SignalCode        signal)      // Signal code
{
   msglog("Signal(%d) '%s' received\n", signal, getSignalName(signal));
   if( signal == SC_PIPE )          // This can occur naturally
     return 0;                      // So silently ignore it

   fprintf(stderr, "Signal(%d) '%s' received\n", signal, getSignalName(signal));

   // We use kill -31 to display current status information
   if( signal == SC_USER2 )
   {
     Block* block;
     int eCount= 0;
     for(block= dirEntry; block != NULL; block= block->next)
       eCount++;

     int lCount= 0;
     for(block= dirList; block != NULL; block= block->next)
       lCount++;

     fprintf(stderr, "Memory: dirEntry(%d) dirList(%d)\n", eCount, lCount);
     mx_buffer->status();
     CommonThread::status();
     return 0;
   }

   // All other handled signals terminate all threads
   abortHandler();                  // Backout the current operation
   CommonThread::notifyAll(CommonThread::NFC_FINAL);
   return 0;                        // The signal is handled
}
}; // class MySignal

//----------------------------------------------------------------------------
//
// Method-
//       Direct::~Direct
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
#ifdef _OS_WIN
   Direct::~Direct( void )          // Destructor
{
   if( handle >= 0 )                // If the handle is currently open
   {
     _findclose(handle);            // Close the file handle
     handle= (-1);                  // Indicate closed
   }
}

#else
   Direct::~Direct( void )          // Destructor
{
   if( stream != NULL )             // If the stream is currently open
   {
     closedir(stream);              // Close the directory stream
     stream= NULL;                  // Indicate closed
   }
}
#endif

//----------------------------------------------------------------------------
//
// Method-
//       Direct::Direct
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
#ifdef _OS_WIN
   Direct::Direct(                  // Constructor
     const char*       pathName)    // Directory path name
:  handle(-1)
{
   char                FQN[MAX_DIRFILE+1];
   char*               ptrFQN;

   ptrFQN= makeFileName(FQN, pathName, "*");
   handle= _findfirst(ptrFQN, &block);
}

#else
   Direct::Direct(                  // Constructor
     const char*       pathName)    // Directory path name
:  stream(NULL)
{
   stream= opendir(pathName);
}
#endif

//----------------------------------------------------------------------------
//
// Method-
//       Direct::getNext
//
// Purpose-
//       Get next file name.
//
//----------------------------------------------------------------------------
#ifdef _OS_WIN
const char*                         // The next file name
   Direct::getNext( void )          // Get next file name
{
   int                 rc;

   if( handle < 0 )
     return NULL;

   rc= _findnext(handle, &block);   // Get next entry
   if( rc != 0 )                    // If file not found
   {
     _findclose(handle);            // Close the file handle
     handle= (-1);                  // Indicate invalid handle
     return NULL;                   // File not found
   }

   return block.name;               // Return the file name
}

#else
const char*                         // The next file name
   Direct::getNext( void )          // Get next file name
{
   dirent*             direct;      // -> dirent

   if( stream == NULL )             // If no current stream
     return NULL;

   direct= readdir(stream);         // Locate the next file
   if( direct == NULL )             // If file not found
   {
     closedir(stream);              // Close the directory stream
     stream= NULL;                  // Indicate closed
     return NULL;                   // File not found
   }

   return direct->d_name;
}
#endif

//----------------------------------------------------------------------------
//
// Method-
//       Backout::~Backout
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Backout::~Backout( void )        // Destructor
{
   IFHCDM(
     if( dirEntry == NULL )
       debugf("Backout(%p)::~Backout(RESET) [%p]\n", this, backout);
     else
       debugf("Backout(%p)::~Backout(%s/%s) [%p]\n", this,
              path, dirEntry->fileName, backout);
   )

   DirEntry* dirEntry= this->dirEntry; // Prevent recursion
   this->dirEntry= NULL;
   backout= NULL;

   if( dirEntry != NULL && handle >= 0 ) // If backout is required
   {
     char fileName[MAX_DIRFILE+1];  // Fully qualified file name
     makeFileName(fileName, path, dirEntry->fileName);
     msglog("Backout(%s)\n", fileName);
     msgout("  %-10s %c %-32s %s\n",
            "removed", 'F', dirEntry->fileName, "[Backout action]");

     #if defined(_OS_WIN) || defined(_OS_CYGWIN)
       chmod(fileName, dirEntry->chmod()|S_IWUSR);
     #endif

     if( remove(fileName) != 0 ) // Remove file failed
       msgerr("%4d Backout: remove(%s) failure", __LINE__, fileName);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Backout::Backout
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Backout::Backout(                // Constructor
     const char*       path,        // The associated path
     DirEntry*         dirEntry,    // The associated DirEntry
     int               handle)      // The associated file handle
:  path(path)
,  dirEntry(dirEntry)
,  handle(handle)
{
   IFHCDM(
     debugf("Backout(%p)::Backout(%s/%s) [%p]\n", this,
            path, dirEntry->fileName, backout);
   )

   backout= this;
}

//----------------------------------------------------------------------------
//
// Method-
//       Backout::reset
//
// Purpose-
//       Cancel the backout operation.
//
//----------------------------------------------------------------------------
void
   Backout::reset( void )           // Cancel backout operation
{
   dirEntry= NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       Buffer::~Buffer
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Buffer::~Buffer( void )          // Destructor
{
   Block*              block;       // -> Block

   IFHCDM(
     debugf("Buffer(%p)::~Buffer()\n", this);
     status();
   )

   while( head != NULL )
   {
     block= (Block*)head;
     head= block->next;

     free(block);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Buffer::Buffer
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Buffer::Buffer(                  // Constructor
     unsigned int      size)        // The size of each Buffer
:  size(size)
,  head(NULL)
,  aCount(0)
,  rCount(0)
,  uCount(0)
{
   IFHCDM( debugf("Buffer(%p)::Buffer()\n", this); )
}

//----------------------------------------------------------------------------
//
// Method-
//       Buffer::allocate
//
// Purpose-
//       Allocate a Buffer block.
//
//----------------------------------------------------------------------------
void*                               // The allocated block
   Buffer::allocate( void )         // Allocate a block
{
   AutoRecursiveBarrier lock(barrier); // Thread latch

   Block* block= (Block*)head;
   if( block != NULL )
   {
     head= block->next;
     uCount--;
   }
   else
     block= (Block*)must_malloc(size); // Allocate a new block

   aCount++;
   IFHCDM( debugf("%p= Buffer(%p)::allocate()\n", block, this); )
   return block;
}

//----------------------------------------------------------------------------
//
// Method-
//       Buffer::release
//
// Purpose-
//       Release a Buffer block.
//
//----------------------------------------------------------------------------
void
   Buffer::release(                 // Release a block
     void*             object)      // The block to release
{
   IFHCDM( debugf("Buffer(%p)::release(%p)\n", this, object); )

   AutoRecursiveBarrier lock(barrier); // Thread latch

   Block* block= (Block*)object;    // Convert to Block
   block->next= (Block*)head;
   head= block;

   uCount++;
   rCount++;
}

//----------------------------------------------------------------------------
//
// Method-
//       Buffer::status
//
// Purpose-
//       Display the Buffer status
//
//----------------------------------------------------------------------------
void
   Buffer::status( void )           // Display Buffer status
{
   AutoRecursiveBarrier lock(barrier); // Thread latch

   fprintf(stderr, "Buffer: %p Avail(%d) Alloc(%d) Release(%d)\n",
                   this, uCount, aCount, rCount);

   IFHCDM(
     int vCount= 0;                 // Verification counter
     Block* block= (Block*)head;
     while( block != NULL )
     {
       vCount++;
       block= block->next;
     }

     if( uCount != vCount )
       throwf("%4d Buffer::status(%d != %d)", __LINE__, uCount, vCount);
   )
}

//----------------------------------------------------------------------------
//
// Method-
//       Buffer::Auto::~Auto
//
// Purpose-
//       Destructor (release buffer)
//
//----------------------------------------------------------------------------
   Buffer::Auto::~Auto( void )      // Destructor
{
   buffer->release(block);          // Release the associated block
}

//----------------------------------------------------------------------------
//
// Method-
//       Buffer::Auto::Auto
//
// Purpose-
//       Constructor (allocate buffer)
//
//----------------------------------------------------------------------------
   Buffer::Auto::Auto(              // Constructor
     Buffer*           buffer)      // Associated allocator
:  buffer(buffer)
,  block(buffer->allocate())
{
}

//----------------------------------------------------------------------------
//
// Method-
//       DirEntry::~DirEntry
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   DirEntry::~DirEntry( void )      // Destructor
{
   IFHCDM( debugf("DirEntry(%p)::~DirEntry()\n", this); )

   if( list != NULL )
   {
     delete list;
     list= NULL;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       DirEntry::DirEntry
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   DirEntry::DirEntry(              // Constructor
     const CommonThread*
                       owner)       // Owner CommonThread
:  next(NULL)
,  owner(owner)
,  list(NULL)
{
   IFHCDM( debugf("DirEntry(%p)::DirEntry()\n", this); )

   fileName[0]= '\0';
   linkName[0]= '\0';
}

//----------------------------------------------------------------------------
//
// Method-
//       DirEntry::operator delete
//
// Purpose-
//       DirEntry deallocator
//
//----------------------------------------------------------------------------
void
   DirEntry::operator delete(       // DirEntry deallocator
     void*             addr)        // The allocated block
{
   IFHCDM( debugf("DirEntry.delete(%p)\n", addr); )

   AutoRecursiveBarrier lock(barrier); // Get the Barrier latch

   ((Block*)addr)->next= dirEntry;
   dirEntry= (Block*)addr;
}

//----------------------------------------------------------------------------
//
// Method-
//       DirEntry::operator new
//
// Purpose-
//       DirEntry allocator
//
//----------------------------------------------------------------------------
void*                               // Resultant
   DirEntry::operator new(          // DirEntry allocator
     std::size_t       size)        // The required size
{
   Block*              result;      // Resultant

   {{{{
     AutoRecursiveBarrier lock(barrier); // Get the Barrier latch

     result= dirEntry;
     if( result == NULL )             // If no storage available
     {
       result= (Block*)must_malloc(size * 32);
       for(int i= 0; i<31; i++)
       {
         result->next= dirEntry;
         dirEntry= result;
         result= (Block*)(((char*)result) + size);
       }
     }
     else
       dirEntry= result->next;
   }}}}

   IFHCDM( debugf("%p= DirEntry.new(%zd)\n", result, size); )
   memset(result, 0, size);

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       DirEntry::checksum
//
// Purpose-
//       Compute file checksum.
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   DirEntry::checksum(              // Compute file checksum
     const char*       path)        // The file path
{
   HOST64              ksum;        // Resultant checksum

   PEER64*             word;        // Checksum word
   char                fileName[MAX_DIRFILE+1]; // Current file name
   int                 hand;        // File handle
   unsigned            left;        // File size remaining
   unsigned            size;        // Transfer length
   unsigned            L;           // Number of bytes read

   //-------------------------------------------------------------------------
   // Set initial checksum
   //-------------------------------------------------------------------------
   fileKsum= 0;                     // Default/error checksum
   if( getFileType(fileInfo) != FT_FILE ) // If not a file
     return (-1);                   // No checksum

   //-------------------------------------------------------------------------
   // Get the complete file name
   //-------------------------------------------------------------------------
   makeFileName(fileName, path, this->fileName);

   //-------------------------------------------------------------------------
   // Open the file
   //-------------------------------------------------------------------------
   hand= open64(fileName, O_RDONLY | O_RSHARE | O_BINARY);
   if( hand < 0 )                   // If open failed
   {
     msgerr("%4d DirEntry.checksum: open64(%s) failure",
            __LINE__, fileName);
     return (-1);
   }

   //-------------------------------------------------------------------------
   // Read the file
   //-------------------------------------------------------------------------
   Buffer::Auto temporary(mx_buffer);
   char* buffer= (char*)temporary.get();

   left= fileSize;
   word= (PEER64*)buffer;
   ksum= 0;
   while( left > 0 )
   {
     size= (unsigned)min(left, MAX_TRANSFER);
     word[(size-1)/sizeof(PEER64)]= 0;

     L= read(hand, buffer, size);
     if( L != size )
     {
       msgerr("%4d DirEntry.checksum: read(%s) I/O error", __LINE__, fileName);
       close(hand);
       return (-1);
     }

     for(size_t i= 0; i<(size+(sizeof(PEER64)-1))/sizeof(PEER64); i++)
       ksum += hostToPeer(word[i]);

     left -= size;
   }

   //-------------------------------------------------------------------------
   // Close the file
   //-------------------------------------------------------------------------
   if( close(hand) != 0 )           // Close data file failed
   {
     msgerr("%4d DirEntry.checksum: close(%s) failure", __LINE__, fileName);
     return(-1);
   }

   //-------------------------------------------------------------------------
   // Set the checksum
   //-------------------------------------------------------------------------
   fileKsum= ksum;
   return 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       DirEntry::chmod
//
// Purpose-
//       Convert info to chmod parameter.
//
//----------------------------------------------------------------------------
int                                 // chmod parameter
   DirEntry::chmod( void ) const    // Get chmod parameter
{
   int                 minp;        // Input info
   int                 mout;        // Output mode

   // Convert the mode
   minp= (int)fileInfo;
   mout= 0;
   #if defined(_OS_WIN)
     if( (minp&INFO_RANY) != 0 ) mout |= S_IRUSR;
     if( (minp&INFO_WANY) != 0 ) mout |= S_IWUSR;
     if( (minp&INFO_XANY) != 0 ) mout |= S_IXUSR;
   #endif

   #if defined(_OS_BSD)
     if( (minp&INFO_RUSR) != 0 ) mout |= S_IRUSR;
     if( (minp&INFO_WUSR) != 0 ) mout |= S_IWUSR;
     if( (minp&INFO_XUSR) != 0 ) mout |= S_IXUSR;

     if( (minp&INFO_RGRP) != 0 ) mout |= S_IRGRP;
     if( (minp&INFO_WGRP) != 0 ) mout |= S_IWGRP;
     if( (minp&INFO_XGRP) != 0 ) mout |= S_IXGRP;

     if( (minp&INFO_ROTH) != 0 ) mout |= S_IROTH;
     if( (minp&INFO_WOTH) != 0 ) mout |= S_IWOTH;
     if( (minp&INFO_XOTH) != 0 ) mout |= S_IXOTH;

     if( (minp&INFO_AUID) != 0 ) mout |= S_ISUID;
     if( (minp&INFO_AGID) != 0 ) mout |= S_ISGID;
     if( (minp&INFO_AVTX) != 0 ) mout |= S_ISVTX;
//// if( (minp&INFO_AFMT) != 0 ) mout |= S_ENFMT;
   #endif

   if( hcdm > 9 )
     msglog("%.8x= DirEntry::chmod(%.8x)\n", mout, (int)fileInfo);

   return mout;
}

//----------------------------------------------------------------------------
//
// Method-
//       DirEntry::compareInfo
//
// Purpose-
//       Compare HostInfo (to see if update is required)
//
// Implementation notes-
//       Called using client DirEntry, comparing with server DirEntry.
//
//----------------------------------------------------------------------------
int                                 // Return code (0 EQ)
   DirEntry::compareInfo(           // Compare hostInfo
     const DirEntry*   server)      // Server (source) DirEntry
{
   DirEntry*           client= this;// Client (target) DirEntry
   HostInfo            cInfo;       // Client HostInfo
   HostInfo            sInfo;       // Server HostInfo

   // For directories, the time attribute is meaningful if both the
   // local and remote are running pure BSD
   if( owner->getGVersionInfo().f[1] == VersionInfo::VIF1_OBSD )
   {
     if( client->fileTime != server->fileTime )
       return TRUE;
   }

   // Extract the attributes
   cInfo= client->fileInfo;
   sInfo= server->fileInfo;

   if( owner->getGVersionInfo().f[1] == VersionInfo::VIF1_OMIX ) // If mixed OS
   {
     if( owner->getLVersionInfo().f[1] == VersionInfo::VIF1_OWIN
         || owner->getRVersionInfo().f[1] == VersionInfo::VIF1_OWIN ) // If any Windows
     {
       cInfo &= INFO_WININFO;
       sInfo &= INFO_WININFO;
       if( getFileType(client->fileInfo) == FT_PATH )
         cInfo |= INFO_XUSR;
       if( getFileType(server->fileInfo) == FT_PATH )
         sInfo |= INFO_XUSR;
     }
   }

   return cInfo != sInfo;
}

//----------------------------------------------------------------------------
//
// Method-
//       DirEntry::compareTime
//
// Purpose-
//       Compare HostInfo (to see if update is required)
//
//----------------------------------------------------------------------------
int                                 // Return (<0, =0, >0)
   DirEntry::compareTime(           // Compare fileTime
     const DirEntry*   that)        // With that DirEntry
{
   int64_t             thisTime= this->fileTime; // This HostTime
   int64_t             thatTime= that->fileTime; // That HostTime

   thisTime &= int64_t(0xfffffffffffffffe); // Truncate odd second out
   thatTime &= int64_t(0xfffffffffffffffe); // Truncate odd second out

   thisTime -= thatTime;
   if( thisTime < 0 )
     return -1;
   else if( thisTime > 0 )
     return 1;

   return 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       DirEntry::display
//
// Purpose-
//       Display this entry.
//
//----------------------------------------------------------------------------
void
   DirEntry::display(               // Display this entry
     const char*       info) const  // With this header
{
   if( info[0] != '\0' )            // If info present
     msglog("%s ", info);           // Add informational header

   msglog("%p %c I(0x%.8lX) T(%12ld) S(%12ld) K(0x%.8lx.%.8lx) %s\n",
          this,
          getFileType(fileInfo),
          (long)fileInfo,
          (long)fileTime,
          (long)fileSize,
          (long)(fileKsum>>32), (long)(fileKsum),
          fileName);
}

//----------------------------------------------------------------------------
//
// Method-
//       DirEntry::fromFile
//
// Purpose-
//       Initialize the entry.
//
//----------------------------------------------------------------------------
void
   DirEntry::fromFile(              // Initialize from file
     const char*       ptrPath,     // The file path
     const char*       ptrName)     // The file name
{
   FileInfo info(ptrPath, ptrName);
   if( info.exists() == FALSE && info.isLink() == FALSE )
     throwf("%4d RdCommon: File(%s)", __LINE__, info.getFileName());

   if( strlen(ptrName) >= sizeof(this->fileName) )
     throwf("%4d RdCommon: File(%s) name too large", __LINE__, ptrName);

   strcpy(this->fileName, ptrName);

   // Convert the mode
   int minp= info.getPermissions();
   int mout= 0;

   if( info.isLink() ) mout |= INFO_ISLINK;
   else if( info.isFile() ) mout |= INFO_ISFILE;
   else if( info.isPath() ) mout |= INFO_ISPATH;
   else if( info.isPipe() ) mout |= INFO_ISPIPE;

   if( (minp&S_IRUSR) != 0 ) mout |= INFO_RUSR;
   if( (minp&S_IWUSR) != 0 ) mout |= INFO_WUSR;
   if( (minp&S_IXUSR) != 0 ) mout |= INFO_XUSR;

   // Extended attributes
   #if defined(_OS_BSD)
     if( (minp&S_IRGRP) != 0 ) mout |= INFO_RGRP;
     if( (minp&S_IWGRP) != 0 ) mout |= INFO_WGRP;
     if( (minp&S_IXGRP) != 0 ) mout |= INFO_XGRP;

     if( (minp&S_IROTH) != 0 ) mout |= INFO_ROTH;
     if( (minp&S_IWOTH) != 0 ) mout |= INFO_WOTH;
     if( (minp&S_IXOTH) != 0 ) mout |= INFO_XOTH;

     struct stat         s;         // File status
     if( lstat(info.getFileName(), &s) == 0 ) // Read LINK status
     {
       minp= s.st_mode;
       if( (minp&S_ISUID) != 0 ) mout |= INFO_AUID;
       if( (minp&S_ISGID) != 0 ) mout |= INFO_AGID;
       if( (minp&S_ISVTX) != 0 ) mout |= INFO_AVTX;
////// if( (minp&S_ENFMT) != 0 ) mout |= INFO_AFMT;
     }
   #endif

   #if defined(_OS_WIN)
     minp= GetFileAttributes(info.getFileName()); // Extract the file attributes
     if( minp != INVALID_FILE_ATTRIBUTES) // If got attributes
     {
       if( (minp&A_Axxx) != 0 ) mout |= INFO_ATTR_A;
       if( (minp&A_xSxx) != 0 ) mout |= INFO_ATTR_S;
       if( (minp&A_xxHx) != 0 ) mout |= INFO_ATTR_H;
       if( (minp&A_xxxR) != 0 ) mout |= INFO_ATTR_R;
     }
   #endif

   fileInfo= mout;                  // Set the file information

   // Get the size and time
   fileSize= info.getFileSize();    // Set the file size
   fileTime= (HOST64)info.getLastModify().toJulian().getTime(); // Set the file time

   //-------------------------------------------------------------------------
   // On Windows, odd file times are sometimes rounded up by one
   //   so we *always* round up odd filetimes by one.
   //-------------------------------------------------------------------------
   // if( (fileTime&1) != 0 )       // IS THIS STILL TRUE?
   //   fileTime++;

   // Initialize the link information
   linkName[0]= '\0';
   if( getFileType(fileInfo) == FT_LINK )
   {
     int rc= readlink(info.getFileName(), linkName, sizeof(linkName)); // Read link name
     if( rc < 0 )                   // If readlink failure
       throwf("%4d RdCommon: errno(%d) %d=readlink(%s) failure",
              __LINE__, errno, rc, info.getFileName());

     if( size_t(rc) >= sizeof(linkName) ) // If link name too large
       throwf("%4d RdCommon: fileName(%s) link name too large",
              __LINE__, info.getFileName());

     linkName[rc]= '\0';            // Set the string delimiter
   }

   //-------------------------------------------------------------------------
   // If either side wants checksum information, set the checksum
   //-------------------------------------------------------------------------
   fileKsum= 0;                     // Default checksum
   if( getFileType(fileInfo) == FT_FILE )
     if( (owner->getLVersionInfo().f[7]&VersionInfo::VIF7_KSUM) != 0
         || (owner->getRVersionInfo().f[7]&VersionInfo::VIF7_KSUM) != 0 )
       checksum(ptrPath);
}

//----------------------------------------------------------------------------
//
// Method-
//       DirEntry::intoFile
//
// Purpose-
//       Update the file attributes.
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   DirEntry::intoFile(              // Update file attributes
     const char*       ptrPath) const // The file path
{
   //-------------------------------------------------------------------------
   // Update the modification time
   //-------------------------------------------------------------------------
   FileInfo info(ptrPath, fileName);

   Julian j(fileTime);
   if( j < Julian::UTC2000 )        // Disallow fileTime < 2000/01/01
   {
     Julian now;
     j= now;
   }

   Clock  c(j);
   if( info.setLastModify(c) )
     msgerr("%4d RdCommon: setLastModify(%s)", __LINE__, fileName);

   //-------------------------------------------------------------------------
   // Update the mode
   //-------------------------------------------------------------------------
   int mout= chmod();
   int rc= ::chmod(info.getFileName(), mout); // Update mode
   if( rc != 0 )                    // If can't change permissions
     msgerr("%4d RdCommon: errno(%d) %d=chmod('%s',%x)",
            __LINE__, errno, rc, info.getFileName(), mout);

   if( hcdm > 9 )
     msglog("%d= ::chmod(%s,%.8x)\n", rc, info.getFileName(), mout);

   #if defined(_OS_WIN)
     int minp= fileInfo;
     mout= 0;
     if( (minp&INFO_ATTR_A) != 0 ) mout |= (A_Axxx);
     if( (minp&INFO_ATTR_S) != 0 ) mout |= (A_xSxx);
     if( (minp&INFO_ATTR_H) != 0 ) mout |= (A_xxHx);
     if( (minp&INFO_ATTR_R) != 0 ) mout |= (A_xxxR);
     if( (minp&INFO_WUSR)   == 0 ) mout |= (A_xxxR);

     if( SetFileAttributes(info.getFileName(), mout) == FALSE )
       msgerr("%4d RdCommon: error(%d) %d=SetFileAttributes(%s,%x)",
              __LINE__, GetLastError(), FALSE, info.getFileName(), mout);
   #endif

   return 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       DirList::~DirList
//
// Purpose-
//       Release this DirList
//
//----------------------------------------------------------------------------
   DirList::~DirList( void )        // Destructor
{
   IFHCDM( debugf("DirList(%p)::~DirList()\n", this); )

   wait();                          // Wait for Thread completion

   while( head != NULL)             // Release directory entries
   {
     DirEntry* ptrE= head;          // Save the element pointer
     head= ptrE->next;              // Address the next element
     delete ptrE;                   // Release the current element
   }

   if( path != NULL)                // Delete copied path
   {
     free(path);
     path= NULL;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       DirList::DirList
//
// Purpose-
//       Constructors
//
//----------------------------------------------------------------------------
   DirList::DirList(                // Constructor
     const CommonThread*
                       owner,       // The owner CommonThread
     const char*       path,        // Subdirectory path
     const DirEntry*   inpE)        // Associated file
:  Thread()
,  count(0)
,  head(NULL)
,  owner(owner)
,  path(NULL)
{
   const char*         ptrName;     // -> Current fileName
   DirEntry**          ptrSort;     // -> Directory entry list for sorting

   DirEntry*           ptrE;        // -> DirEntry

   int                 count;       // Number of directory entries
   int                 size;        // Size of fileName

   IFHCDM( debugf("DirList(%p)::DirList(%s,%s)\n", this, path, inpE->fileName); )
   ELHCDM( msglog("DirList(%p)::DirList(%s,%s)\n", this, path, inpE->fileName); )

   //-------------------------------------------------------------------------
   // Read the directory
   //-------------------------------------------------------------------------
   char   fullName[MAX_DIRFILE+1];  // The fully qualified name
   makeFileName(fullName, path, inpE->fileName);
   Direct dir(fullName);            // Directory object
   this->path= must_strdup(fullName); // Associated path

   count= 0;                        // No entries located yet
   for(;;)                          // For each directory entry
   {
     ptrName= dir.getNext();        // Get next file name
     if( ptrName == NULL )          // If end of list
       break;

     if( strcmp(ptrName, ".") != 0
         && strcmp(ptrName, "..") != 0 )
     {
       // Unless both sides are identical O/S types, we also skip .lnk files
       if( owner->getGVersionInfo().f[1] == VersionInfo::VIF1_OMIX )
       {
         size= strlen(ptrName);
         if( size > 4 && stricmp(".lnk", ptrName + size - 4) == 0 )
           continue;
       }

       ptrE= new DirEntry(owner);   // Allocate a DirEntry
       ptrE->fromFile(this->path, ptrName); // Read file status

       //-----------------------------------------------------------------------
       // Install the entry
       //-----------------------------------------------------------------------
       ptrE->next= this->head;      // Set the chain pointer
       this->head= ptrE;            // Install the entry
       count++;                     // Count the entry
     }
   }
   this->count= count;

   //-------------------------------------------------------------------------
   // Sort the file names
   //-------------------------------------------------------------------------
   if( count > 0 )
   {
     ptrSort= (DirEntry**)must_malloc(count*sizeof(DirEntry*));

     ptrE= this->head;              // Populate the sort array
     for(int i=0; i<count; i++)
     {
       ptrSort[i]= ptrE;
       ptrE= ptrE->next;
     }

     for(int i=0; i<count; i++)     // Sort by name
     {
       for(int j=i+1; j<count; j++)
       {
         if( strCompare(owner,ptrSort[i]->fileName,ptrSort[j]->fileName) > 0 )
         {
           ptrE= ptrSort[i];
           ptrSort[i]= ptrSort[j];
           ptrSort[j]= ptrE;
         }
       }
     }

     this->head= ptrSort[0];        // Create the sorted list
     for(int i=1; i<count; i++)
       ptrSort[i-1]->next= ptrSort[i];
     ptrSort[count-1]->next= NULL;

     free(ptrSort);
   }

   if( hcdm > 8 )
   {
     msglog("..DirList(%p)::DirList(%s,%s)\n", this, path, inpE->fileName);
     this->display("DirList.DirList");
   }

   setPriority(-1);                 // Run loader at lower priority
}

   DirList::DirList(                // Constructor
     const CommonThread*
                       owner,       // The owner CommonThread
     const char*       path)        // Subdirectory path
:  Thread()
,  count(0)
,  head(NULL)
,  owner(owner)
,  path(must_strdup(path))
{
   IFHCDM( debugf("DirList(%p)::DirList(%s)\n", this, path); )

   setPriority(-1);                 // Run loader at lower priority
}

//----------------------------------------------------------------------------
//
// Method-
//       DirList::operator delete
//
// Purpose-
//       DirList deallocator
//
//----------------------------------------------------------------------------
void
   DirList::operator delete(        // DirList deallocator
     void*             addr)        // The allocated block
{
   IFHCDM( debugf("DirList.delete(%p)\n", addr); )

   AutoRecursiveBarrier lock(barrier); // Get the Barrier latch

   ((Block*)addr)->next= dirList;
   dirList= (Block*)addr;
}

//----------------------------------------------------------------------------
//
// Method-
//       DirList::operator new
//
// Purpose-
//       DirList allocator
//
//----------------------------------------------------------------------------
void*                               // Resultant
   DirList::operator new(           // DirList allocator
     std::size_t       size)        // The required size
{
   Block*              result;      // Resultant

   {{{{
     AutoRecursiveBarrier lock(barrier); // Get the Barrier latch

     result= dirList;
     if( result == NULL )             // If no storage available
     {
       result= (Block*)must_malloc(size * 32);
       for(int i= 0; i<31; i++)
       {
         result->next= dirList;
         dirList= result;
         result= (Block*)(((char*)result) + size);
       }
     }
     else
       dirList= result->next;
   }}}}

   IFHCDM( debugf("%p= DirList.new(%zd)\n", result, size); )
   memset(result, 0, size);

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       DirList::display
//
// Purpose-
//       Display this object.
//
//----------------------------------------------------------------------------
void
   DirList::display(                // Display this object
     const char*       text) const  // With this identifier
{
   DirEntry*           ptrE;        // -> Entry

   AutoRecursiveBarrier lock(barrier); // For multi-thread neatness
   msglog("DirList(%p)::display(%s) count(%d)\n", this, text, count);

   ptrE= head;
   for(unsigned i= 0; i<count; i++)
   {
     if( ptrE == NULL )
       throwf("%4d RdCommon: Should not occur", __LINE__);

     msglog("[%5d] ", i);           // Matches "CLIENT:", "SERVER:"
     ptrE->display();

     ptrE= ptrE->next;
   }

   if( ptrE != NULL )
     throwf("%4d RdCommon: Should not occur", __LINE__);
}

//----------------------------------------------------------------------------
//
// Method-
//       DirList::insert
//
// Purpose-
//       Insert a DirEntry onto the DirList.
//
//----------------------------------------------------------------------------
DirEntry*                           // The inserted element
   DirList::insert(                 // Insert DirEntry
     DirEntry*         ptrE,        // Element to insert
     DirEntry*         prior)       // Prior DirEntry
{
   if( prior == NULL )
   {
     ptrE->next= head;
     head= ptrE;
   }
   else
   {
     ptrE->next= prior->next;
     prior->next= ptrE;
   }

   count++;
   return ptrE;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       DirList::locate
//
// Function-
//       Locate a directory entry.
//
//----------------------------------------------------------------------------
DirEntry*                           // -> DirEntry
   DirList::locate(                 // Locate directory entry
     char*             fileName)    // Filename to locate
{
   DirEntry*           ptrE;        // -> DirEntry
   int                 rc;

   ptrE= head;
   while( ptrE != NULL )            // Simple search of directory
   {
     rc= strcmp(fileName, ptrE->fileName); // Check it out
     if( rc == 0 )                  
       return ptrE;                 

     ptrE= ptrE->next;
   }

   msgout("%4d RdCommon: locate(%s) NOT FOUND\n", __LINE__, fileName);
   return NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       DirList::remove
//
// Purpose-
//       Remove a DirEntry from the DirList, then release it.
//
//----------------------------------------------------------------------------
DirEntry*                           // The next element
   DirList::remove(                 // Remove DirEntry
     DirEntry*         ptrE,        // Element to remove
     DirEntry*         prior)       // Prior DirEntry
{
   DirEntry*           result;      // Resultant

   if( prior == NULL )
     head= ptrE->next;
   else
     prior->next= ptrE->next;

   count--;
   result= ptrE->next;
   delete ptrE;
   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       DirList::runLoader
//
// Purpose-
//       Background subdirectory element loader
//
//----------------------------------------------------------------------------
void
   DirList::runLoader( void )       // Subdirectory element loader
{
   IFHCDM( debugf("DirList(%p)::run(%s)..\n", this, path); )
   ELHCDM( msglog("DirList(%p)::run(%s)..\n", this, path); )

   // Load all subdirectory entries
   DirEntry* ptrE= head;
   while( ptrE != NULL )
   {
     if( (ptrE->fileInfo&INFO_ISTYPE) == INFO_ISPATH )
     {
       assert( ptrE->list == NULL ); // TODO: REMOVE
       ptrE->list= new DirList(owner, path, ptrE);
     }

     ptrE= ptrE->next;
   }

   IFHCDM( debugf("..DirList(%p)::run(%s)\n", this, path); )
   ELHCDM( msglog("..DirList(%p)::run(%s)\n", this, path); )
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       DirList::run
//
// Purpose-
//       Background subdirectory element loader
//
//----------------------------------------------------------------------------
long                                // Return code
   DirList::run( void )             // Subdirectory element loader
{
   try {
     runLoader();
   } catch( const char* X ) {
     msgout("DirList.run exception(%s)\n", X);
     throw X;
   } catch( ... ) {
     throwf("DirList.run exception(...)\n");
   }

   return 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       InputBuffer::~InputBuffer
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   InputBuffer::~InputBuffer( void ) // Destructor
{
   if( used != size )
     fprintf(stderr, "%4d ~InputBuffer(), unused(%d)", __LINE__, size-used);
}

//----------------------------------------------------------------------------
//
// Method-
//       InputBuffer::InputBuffer
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   InputBuffer::InputBuffer(        // Constructor
     CommonThread*     owner)       // -> CommonThread
:  owner(owner)
,  used(0)
,  size(0)
{
}

//----------------------------------------------------------------------------
//
// Method-
//       InputBuffer::fill
//
// Purpose-
//       Fill the buffer, if required.
//
//----------------------------------------------------------------------------
void
   InputBuffer::fill( void )        // Fill the buffer
{
   if( iodm && hcdm > 1 )
     msglog("InputBuffer::fill used(%u) size(%u)\n", used, size);

   char* buffer= owner->getBuffer();
   if( used >= size )               // If buffer is empty
   {
     used= 0;
     size= owner->nRecv(buffer, MAX_TRANSFER);
     if( iodm && hcdm > 1 )
       msglog("..InputBuffer::fill used(%u) size(%u)\n", used, size);
     return;
   }

   if( used > 0 )                   // If buffer is used
   {
     for(unsigned i=0; i<(size-used); i++) // Move to buffer origin
       buffer[i]= buffer[i+used];

     size -= used;
     used= 0;
   }

   if( size < MAX_TRANSFER )        // If room available in buffer
     size += owner->nRecv(buffer + size, MAX_TRANSFER-size);

   if( iodm && hcdm > 1 )
     msglog("..InputBuffer::fill used(%u) size(%u)\n", used, size);
}

//----------------------------------------------------------------------------
//
// Method-
//       InputBuffer::getChar
//
// Purpose-
//       Get the next data buffer byte.
//
//----------------------------------------------------------------------------
int                                 // The next data buffer byte
   InputBuffer::getChar( void )     // Get next data buffer byte
{
   int                 resultant;   // Resultant character

   fill();                          // Fill the buffer

   resultant= *(owner->getBuffer() + used) & 0x000000ff;
   used++;
   return resultant;
}

//----------------------------------------------------------------------------
//
// Method-
//       InputBuffer::getDataAddr
//
// Purpose-
//       Return the current data address.
//
//----------------------------------------------------------------------------
const char*                         // The current buffer address
   InputBuffer::getDataAddr( void ) // Get current buffer address
{
   return owner->getBuffer() + used;
}

//----------------------------------------------------------------------------
//
// Method-
//       InputBuffer::getDataSize
//
// Purpose-
//       Return the current data length.
//
//----------------------------------------------------------------------------
unsigned                            // The current buffer length
   InputBuffer::getDataSize( void ) // Get current buffer length
{
   return size - used;
}

//----------------------------------------------------------------------------
//
// Method-
//       InputBuffer::use
//
// Purpose-
//       Update the used buffer length.
//
//----------------------------------------------------------------------------
void
   InputBuffer::use(                // Update the used buffer length
     unsigned          used)        // Incremental used length
{
   if( (this->used + used) > size )
     throwf("%4d RdCommon: used(%u) ->used(%u) size(%u)",
            __LINE__, used, this->used, size);

   this->used += used;
   if( iodm && hcdm > 1 )
     msglog("InputBuffer::use(%4u) used(%4u) size(%4u)\n",
            used, this->used, size);
}

//----------------------------------------------------------------------------
//
// Method-
//       OutputBuffer::~OutputBuffer
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   OutputBuffer::~OutputBuffer( void ) // Destructor
{
   if( used != 0 )
     fprintf(stderr, "%4d ~OutputBuffer(), missing empty()", __LINE__);
}

//----------------------------------------------------------------------------
//
// Method-
//       OutputBuffer::OutputBuffer
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   OutputBuffer::OutputBuffer(      // Constructor
     CommonThread*     owner)       // -> Owning CommonThread
:  owner(owner)
,  used(0)
,  size(MAX_TRANSFER)
{
}

//----------------------------------------------------------------------------
//
// Method-
//       OutputBuffer::empty
//
// Purpose-
//       Empty the buffer, if required.
//
//----------------------------------------------------------------------------
void
   OutputBuffer::empty( void )      // Empty the buffer
{
   if( used > 0 )                   // If buffer contains data
   {
     owner->nSendStruct(owner->getBuffer(), used);
     used= 0;
   }

   if( iodm && hcdm > 1 )
     msglog("OutputBuffer::empty used(%u) size(%u)\n",
            used, size);
}

//----------------------------------------------------------------------------
//
// Method-
//       OutputBuffer::getDataAddr
//
// Purpose-
//       Return the current data address.
//
//----------------------------------------------------------------------------
char*                               // The current buffer address
   OutputBuffer::getDataAddr( void )// Get current buffer address
{
   if( used >= size )               // If the buffer is full
     empty();                       // Empty the buffer

   return owner->getBuffer() + used;
}

//----------------------------------------------------------------------------
//
// Method-
//       OutputBuffer::getDataSize
//
// Purpose-
//       Return the current data length.
//
//----------------------------------------------------------------------------
unsigned                            // The current buffer length
   OutputBuffer::getDataSize( void )// Get current buffer length
{
   if( used >= size )               // If the buffer is full
     empty();                       // Empty the buffer

   return size - used;
}

//----------------------------------------------------------------------------
//
// Method-
//       OutputBuffer::putChar
//
// Purpose-
//       Put the next data buffer byte.
//
//----------------------------------------------------------------------------
void
   OutputBuffer::putChar(           // Put the next character
     int               data)        // The next character
{
   if( used >= size )               // If the buffer is full
     empty();                       // Empty the buffer

   *(owner->getBuffer() + used)= data;
   used++;
}

//----------------------------------------------------------------------------
//
// Method-
//       OutputBuffer::use
//
// Purpose-
//       Update the used buffer length.
//
//----------------------------------------------------------------------------
void
   OutputBuffer::use(               // Update the used buffer length
     unsigned          used)        // Incremental used length
{
   if( (this->used + used) > size )
     throwf("%4d RdCommon: used(%u) ->used(%u) size(%u)",
            __LINE__, used, this->used, size);

   this->used += used;
   if( this->used >= size )         // If the buffer is full
     empty();                       // Empty it

   if( iodm && hcdm > 1 )
     msglog("OutputBuffer::use(%4u) used(%4u) size(%4u)\n",
            used, this->used, size);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       getFileType
//
// Purpose-
//       Determine type of item.
//
//----------------------------------------------------------------------------
FILE_TYPE                           // The FileType
   getFileType(                     // Get the FileType
     HostInfo          info)        // From this FileInfo
{
   FILE_TYPE           returncd;    // This routine's return code

   //-------------------------------------------------------------------------
   // Determine file type
   //-------------------------------------------------------------------------
   returncd= FT_UNKNOWN;            // Default, unsupported type

   if( (info&INFO_ISTYPE) == INFO_ISFILE ) // If regular file
     returncd= FT_FILE;             // Indicate regular file

   else if( (info&INFO_ISTYPE) == INFO_ISPATH ) // If nested Dorectory entry
     returncd= FT_PATH;             // Indicate directory

   else if( (info&INFO_ISTYPE) == INFO_ISLINK ) // If soft link
     returncd= FT_LINK;             // Indicate link

   else if( (info&INFO_ISTYPE) == INFO_ISPIPE ) // If FIFO (pipe)
     returncd= FT_FIFO;             // Indicate FIFO

   return(returncd);                // Return, indicate file type
}

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
     HOST16            host16)      // HOST format short value
{
   return htons(host16);
}

extern PEER32                       // PEER format
   hostToPeer(                      // Convert HOST format to PEER format
     HOST32            host32)      // HOST format int value
{
   return htonl(host32);
}

extern PEER64                       // PEER format
   hostToPeer(                      // Convert HOST format to PEER format
     HOST64            host64)      // HOST format 64 bit value
{
   return Network::hton64(host64);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       makeFileName
//
// Purpose-
//       Combine a path and filename into a fully qualified name.
//
//----------------------------------------------------------------------------
extern char*                        // Resultant
   makeFileName(                    // Generate fully qualified name
     char*             resultant,   // -> Resultant fully qualified name
     const char*       path,        // Source path name
     const char*       name)        // Source file name
{
   //-------------------------------------------------------------------------
   // Diagnostics
   //-------------------------------------------------------------------------
   if( (strlen(path) + strlen(name) + 1) >= MAX_DIRFILE )
     throwf("%4d RdCommon makeFileName(%s,%s) too large",
            __LINE__, path, name);

   if( strcmp(name,".") == 0 )
     strcpy(resultant, path);
   else
     sprintf(resultant, "%s/%s", path, name);

   return resultant;
}

#ifdef _OS_WIN
//----------------------------------------------------------------------------
//
// Subroutine-
//       mkdir
//
// Purpose-
//       Windows ignores the second parameter for mkdir.
//
//----------------------------------------------------------------------------
extern int                          // Return code
   mkdir(                           // Invoke system mkdir
     const char*       dirName,     // The directory name
     int               mode)        // The directory mode (ignored)
{
   return ::mkdir(dirName);         // System mkdir
}
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
     unsigned long     size)        // Dump length
{
   if( stdlog == NULL )
     return;

   AutoRecursiveBarrier lock(barrier);
   Debug::get()->dump(stdlog, addr, size);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       msgerr
//
// Purpose-
//       Write message to stderr + perror()
//
//----------------------------------------------------------------------------
void
   msgerr(                          // Write message to stderr
     const char*       fmt,         // The PRINTF format string
                       ...)         // The remaining arguments
{
   va_list             argptr;      // Argument list pointer
   char                string[512]; // Message assembly area

   AutoRecursiveBarrier lock(barrier);
   va_start(argptr, fmt);           // Initialize va_ functions
   vsprintf(string, fmt, argptr);   // Print the message
   va_end(argptr);                  // Close va_ functions

   #if( TRUE )                      // Date/time
   {{
     char   append[32];
     time_t tod= time(NULL);
     append[0]= ' ';
     strcpy(append+1, ctime(&tod));
     if( strchr(append, '\n') == NULL )
       strcat(append, "\n");
     strcat(string, append);
   }}
   #endif

   #if( TRUE )                      // Errno (perror follows)
   {{
     char append[32];
     sprintf(append, "errno(%d)", errno);
     strcat(string, append);
   }}
   #endif

   perror(string);                  // Write the message in one line
   fflush(stderr);      

   if( stdlog != NULL )             // Write all messages to stdlog
   {
     fprintf(stdlog, "%s: %s\n", string, Software::getSystemEI());
     fflush(stdlog);
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       msglog
//
// Purpose-
//       Write message to stdlog.
//
//----------------------------------------------------------------------------
void
   msglog(                          // Write message to stdlog
     const char*       fmt,         // The PRINTF format string
                       ...)         // The remaining arguments
{
   va_list             argptr;      // Argument list pointer

   if( stdlog == NULL )
     return;

   AutoRecursiveBarrier lock(barrier);
   va_start(argptr, fmt);           // Initialize va_ functions
   vfprintf(stdlog, fmt, argptr);   // Print the message
   va_end(argptr);                  // Close va_ functions

   fflush(stdlog);                  // Flush stdlog
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       msgout
//
// Purpose-
//       Write message to stdout, stderr and stdlog
//
//----------------------------------------------------------------------------
void
   msgout(                          // Write message to stdout
     const char*       fmt,         // The PRINTF format string
                       ...)         // The remaining arguments
{
   va_list             argptr;      // Argument list pointer

   AutoRecursiveBarrier lock(barrier);
   va_start(argptr, fmt);           // Initialize va_ functions
   vfprintf(stdout, fmt, argptr);   // Print the message on stdout
   va_end(argptr);

   fflush(stdout);                  // Flush stdout

   if( stdlog != NULL )             // Write all messages to stdlog
   {
     va_start(argptr, fmt);         // Initialize va_ functions
     vfprintf(stdlog, fmt, argptr);
     va_end(argptr);                // Close va_ functions

     fflush(stdlog);
   }
}

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
     PEER16            peer16)      // PEER format short value
{
   return ntohs(peer16);
}

extern HOST32                       // HOST format
   peerToHost(                      // Convert PEER format to HOST format
     PEER32            peer32)      // PEER format int value
{
   return ntohl(peer32);
}

extern HOST64                       // HOST format
   peerToHost(                      // Convert PEER format to HOST format
     PEER64            peer64)      // PEER format 64 bit value
{
   return Network::ntoh64(peer64);
}

#ifdef _OS_WIN
//----------------------------------------------------------------------------
//
// Subroutine-
//       readlink
//
// Purpose-
//       Simulate readlink (by failing.)
//
//----------------------------------------------------------------------------
extern int                          // (Failure) return code
   readlink(                        // Return error
     const char*       fileName,    // The file name
     char*             linkName,    // The link name
     int               size)        // Sizeof(linkName)
{
   assert( FALSE );                 // Should not occur
   return (-1);
}
#endif

//----------------------------------------------------------------------------
//
// Subroutine-
//       rdinit
//
// Purpose-
//       Initialize.
//
//----------------------------------------------------------------------------
void
   rdinit( void )                   // Initialize
{
   const char*         string;      // GETENV resultant
   const char*         fileName;    // Log file name

   // Set Signal handler
   mySignal= new MySignal();
   mySignal->enable(Signal::SC_USER2); // Enable kill -31

   // Set Exit handler
   atexit(abortHandler);            // Backout current operation

   // Set MAX_TRANSFER Buffer allocator
   mx_buffer= new Buffer(MAX_TRANSFER); // MAX_TRANSFER buffer allocator

   // Clear errno
   errno= 0;

   // Extract log controls
   hcdm= scdm= iodm= 0;             // Default controls
   stdlog= NULL;
   fileName= NULL;

   string= getenv("LOG_HCDM");
   if( string != NULL )
   {
     hcdm= atol(string);
     fileName= LOG_FILENAME;
   }

   string= getenv("LOG_SCDM");
   if( string != NULL )
   {
     scdm= atol(string);
     fileName= LOG_FILENAME;
   }

   string= getenv("LOG_IODM");
   if( string != NULL )
   {
     iodm= atol(string);
     fileName= LOG_FILENAME;
   }

   string= getenv("LOG_FILE");
   if( string != NULL )
     fileName= string;

   if( fileName != NULL )
   {
     stdlog= fopen(fileName, "w");
     if( stdlog == NULL )
       msgerr("File(%s): Open failure", fileName);
   }

   string= "Undefined";
   #ifdef _OS_WIN
     string= "Windows";
   #endif
   #ifdef _OS_BSD
     string= "BSD";
   #endif
   #ifdef _OS_CYGWIN
     string= "CYGWIN";
   #endif

   msglog("rdinit() %s %s %s\n", string, __DATE__, __TIME__);

   if( hcdm )
     msgout("Started in HCDM(%d)...\n", hcdm);

   if( scdm )
     msgout("Started in SCDM(%d)...\n", scdm);

   if( iodm )
     msgout("Started in IODM(%d)...\n", iodm);

   #if( BRINGUP )
     msgout("%s Started in TEST MODE...\n", __FILE__);
   #endif

   assert(sizeof(PeerDesc) == ( SIZEOF(PeerDesc,fileSize)
                              + SIZEOF(PeerDesc,fileInfo)
                              + SIZEOF(PeerDesc,fileTime)
                              + SIZEOF(PeerDesc,fileKsum)) );

   if( hcdm > 8 )
   {
     msglog("\n");
     msglog("%10ld = sizeof(PeerDesc.fileSize)\n", SIZEOF(PeerDesc,fileSize));
     msglog("%10ld = sizeof(PeerDesc.fileInfo)\n", SIZEOF(PeerDesc,fileInfo));
     msglog("%10ld = sizeof(PeerDesc.fileTime)\n", SIZEOF(PeerDesc,fileTime));
     msglog("%10ld = sizeof(PeerDesc.fileKsum)\n", SIZEOF(PeerDesc,fileKsum));
     msglog("%10ld = sizeof(PeerDesc)\n", (long)sizeof(PeerDesc));
     msglog("%10ld = sizeof(PeerName)\n", (long)sizeof(PeerName));
//// msglog("%10ld = sizeof(PeerPair)\n", (long)sizeof(PeerPair));
     msglog("%10ld = sizeof(PeerPath)\n", (long)sizeof(PeerPath));
     msglog("%10ld = sizeof(PeerRequest)\n", (long)sizeof(PeerRequest));
     msglog("%10ld = sizeof(PeerResponse)\n", (long)sizeof(PeerResponse));
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       rdterm
//
// Purpose-
//       Terminate.
//
//----------------------------------------------------------------------------
void
   rdterm( void )                   // Terminate
{
   msglog("rdterm()\n");

   //-------------------------------------------------------------------------
   // Terminate signal handling
   //-------------------------------------------------------------------------
   delete mySignal;
   mySignal= NULL;

   //-------------------------------------------------------------------------
   // Delete CommonThread array
   //-------------------------------------------------------------------------
   CommonThread::notifyAll(CommonThread::NFC_FINAL);

   CommonThread::threadCount= 0;
   if( CommonThread::threadArray != NULL )
   {
     free(CommonThread::threadArray);
     CommonThread::threadArray= NULL;
   }

   //-------------------------------------------------------------------------
   // Delete MAX_TRANSFER buffer allocator
   //-------------------------------------------------------------------------
   delete mx_buffer;
   mx_buffer= NULL;

   //-------------------------------------------------------------------------
   // Close logging
   //-------------------------------------------------------------------------
   fflush(stdout);
   fflush(stderr);
   fflush(stdlog);
   fflush(NULL);
   if( stdlog != NULL )
   {
     fclose(stdlog);
     stdlog= NULL;
   }
}

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
     const char*       target)      // Target string
{
   if( (thread->getGVersionInfo().f[0]&VersionInfo::VIF0_CASE) != 0 )
     return strcmp(source, target);

   return stricmp(source, target); 
}

#ifdef _OS_WIN
//----------------------------------------------------------------------------
//
// Subroutine-
//       symlink
//
// Purpose-
//       Simulate symlink function.
//
//----------------------------------------------------------------------------
extern int                          // Return code (0 OK)
   symlink(                         // Return error
     const char*       linkName,    // The link name
     const char*       fileName)    // The file name
{
   errno= EMLINK;                   // Too many links!
   return (-1);
}
#endif // _OS_WIN

