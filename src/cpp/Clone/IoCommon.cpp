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
//       IoCommon.cpp
//
// Purpose-
//       Common I/O objects and subroutines used by RdClient and RdServer.
//
// Last change date-
//       2026/07/05
//
//----------------------------------------------------------------------------
#include <memory>                   // For std::make_unique, ...
#include <mutex>                    // For std::mutex, std::lock_guard
#include <cerrno>                   // For errno
#include <cstdarg>                  // For va_* functions
#include <cstdio>                   // For fprintf, ...
#include <cstring>                  // For strcmp, memcmp, ...

#include <dirent.h>                 // For directory management
#include <endian.h>                 // For be16toh, be32toh, ...
#include <utime.h>                  // For utimbuf
#include <sys/signal.h>             // For signal, ...
#include <sys/stat.h>               // For stat, struct stat, ...

#include <pub/Data.h>               // For pub::Path, pub::File
#include <pub/Latch.h>              // For pub::RecursiveLatch
#include <pub/List.h>               // For pub::List<>
#include <pub/memory.h>             // For pub::scoped_ptr
#include <pub/Signals.h>            // For pub::signals::Signal

#include "IoCommon.h"               // For I/O common objects and subroutines
#include "CommonThread.h"           // For CommonThread

#ifndef O_BINARY                    // Defined in CYGWIN, not in LINUX
  #define O_BINARY 0
#endif

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Objects
using PUB::List;                    // For convenience
using PUB::RecursiveLatch;          // For convenience
using PUB::signals::Signal;         // For convenience

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum                                // Generic enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose
}; // Generic enum

//----------------------------------------------------------------------------
// Typedefs
//----------------------------------------------------------------------------
typedef struct stat    stat_t;      // The struct stat type

//----------------------------------------------------------------------------
// External data areas
//----------------------------------------------------------------------------
pub::List<RdPath>      RdPath::stack; // The RdPath stack

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static constexpr const uint64_t
                       JULIAN_DAY1970= 2'440'588;
static constexpr const uint64_t
                       JULIAN_SEC1970= JULIAN_DAY1970 * 86'400;

static pub::signals::Connector
                       tree_check_handler; // The check_signal handler

namespace {                         // Anonymous namespace
static struct init_term {
   init_term( void )                // Initialize (the tree_list_handler)
{
tree_check_handler=                 // Connect the tree_check_handler
   handle_check_signal([](pub::signals::Event_t& E)
{
   CheckEvent* event= dynamic_cast<CheckEvent*>(&E);
   if( event ) {
     RdPath::debug_static(event->info);
   }
});
}

   ~init_term( void )               // Disconnect the tree_check_handler
{  tree_check_handler.disconnect(); }
} IT; // static struct init_term
}; // Anonymous namespace

//----------------------------------------------------------------------------
//
// Subroutine-
//       SNO (Should Not Occur)
//
// Purpose-
//       Handle "Should Not Occur" situation
//
//----------------------------------------------------------------------------
[[noreturn]]
static void
   SNO(int line)                    // (Should Not Occur)
{  throwf("%4d %s (Should Not Occur)\n", line, __FILE__); }

//============================================================================
//
// Method-
//       Backout::Backout
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Backout::Backout(                // Constructor
     RdFile*           file,        // The associated RdFile
     fd_t              fd)          // The associated File Descriptor
:  file(file), fd(fd)
{  if( HCDM )
     debugf("Backout(%p)::Backout(%s/%s,%d)\n", this
           , s2c(file->path->path_name), s2c(file->file_name), fd);
}

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
{  if( HCDM ) {
     if( file == nullptr || this->fd < 0 )
       debugf("Backout(%p)::~Backout(NOP)\n", this);
     else
       debugf("Backout(%p)::~Backout(%s) fd(%d)\n", this
             , s2c(file->get_full_name()), fd);
   }

   if( file == nullptr || fd < 0 )  // If file non-existent or not open
     return;

   RdFile* file= this->file;
   string full_name= file->get_full_name();
   this->file= nullptr;
   this->fd= -1;

   msglog("Backout(%s)\n", s2c(full_name));

   chmod(s2c(full_name), file->get_chmod()|S_IWUSR);
   if( rmfile(full_name) != 0 )     // Remove file failed
     msgioerr("%4d Backout: remove(%s) failure", __LINE__, s2c(full_name));
   else
     msgout("  %-10s %c %-32s %s\n"
           , "removed", 'F', s2c(file->file_name), "[Backout action]");
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
{  file= nullptr; }

//============================================================================
//
// Method-
//       HostDesc::HostDesc
//
// Purpose-
//       Constructors
//
//----------------------------------------------------------------------------
   HostDesc::HostDesc(              // Copy constructor
     const HostDesc&   host)        // Source HostDesc
{  operator=(host); }

   HostDesc::HostDesc(              // Constructor
     const PeerDesc&   peer)        // Source PeerDesc
{  operator=(peer); }

//----------------------------------------------------------------------------
//
// Method-
//       HostDesc::operator=
//
// Purpose-
//       Assignment operators
//
//----------------------------------------------------------------------------
HostDesc&                           // Resultant
   HostDesc::operator=(             // Assignment from
     const HostDesc&   host)        // Source HostDesc
{
   file_size= host.file_size;
   file_info= host.file_info;
   file_time= host.file_time;
   file_ksum= host.file_ksum;

   return *this;
}

HostDesc&                           // Resultant
   HostDesc::operator=(             // Assignment from
     const PeerDesc&   peer)        // Source PeerDesc
{
   file_size= peer_to_host(peer.file_size);
   file_info= peer_to_host(peer.file_info);
   file_time= peer_to_host(peer.file_time);
   file_ksum= peer_to_host(peer.file_ksum);

   return *this;
}

//============================================================================
//
// Method-
//       PeerDesc::PeerDesc
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
   PeerDesc::PeerDesc(              // Copy constructor
     const PeerDesc&   peer)        // Source PeerDesc
{  operator=(peer); }

   PeerDesc::PeerDesc(              // Constructor
     const HostDesc&   host)        // Source HostDesc
{  operator=(host); }

//----------------------------------------------------------------------------
//
// Method-
//       PeerDesc::operator=
//
// Purpose-
//       Assignment operators
//
//----------------------------------------------------------------------------
PeerDesc&                           // Resultant
   PeerDesc::operator=(             // Assignment from
     const PeerDesc&   peer)        // Source PeerDesc
{
   file_size= peer.file_size;
   file_info= peer.file_info;
   file_time= peer.file_time;
   file_ksum= peer.file_ksum;

   return *this;
}

PeerDesc&                           // Resultant
   PeerDesc::operator=(             // Assignment from
     const HostDesc&   host)        // Source HostDesc
{
   file_size= host_to_peer(host.file_size);
   file_info= host_to_peer(host.file_info);
   file_time= host_to_peer(host.file_time);
   file_ksum= host_to_peer(host.file_ksum);

   return *this;
}

//============================================================================
//
// Method-
//       RdFile::RdFile
//
// Purpose-
//       Constructors
//
//----------------------------------------------------------------------------
   RdFile::RdFile(                  // Constructor
     const RdPath*     path,        // The associated RdPath
     const string&     name)        // (Fully qualified) File name
:  File(name), path(path)           // (File(name) initializes st)
{
   init_desc_size();
   init_desc_info();
   init_desc_time();
   init_desc_ksum();                // (Must follow init_desc_size)
}

   RdFile::RdFile(                  // Constructor
     const RdPath*     path,        // The associated RdPath
     const stat_t&     st,          // Stat descriptor
     const string&     name)        // File name
:  File(st, name), path(path)
{
   init_desc_size();
   init_desc_info();
   init_desc_time();
   init_desc_ksum();                // (Must follow init_desc_size)
}

   RdFile::RdFile(                  // Constructor
     const RdPath*     path,        // The associated RdPath
     const PeerDesc&   peer,        // PeerDesc descriptor
     const string&     name)        // Peer name
:  File(name), path(path)
{
   desc.file_size= peer_to_host(peer.file_size);
   desc.file_info= peer_to_host(peer.file_info);
   desc.file_time= peer_to_host(peer.file_time);
   desc.file_ksum= peer_to_host(peer.file_ksum);
}

//----------------------------------------------------------------------------
// RdFile::debug | Debugging display
void
   RdFile::debug(                   // Debugging display
     const char*       info) const  // Heading information
{
   string link_text;
   if( link_name != "" )
     link_text= " -> " + link_name;
   if( *info == '\0' )              // If short version
     debugf(": RdFile(%p) path(%p) %c %s%s\n", this, path
           , get_file_type() , s2c(file_name), s2c(link_text));

   else
     debugf("RdFile(%p) debug(%s) %c I(0x%.8zX) T(%'12zd)"
            " S(%'12zd) K(0x%.8zx.%.8zx) %s%s\n"
           , this, info, get_file_type()
           , desc.file_info, desc.file_time, desc.file_size
           , desc.file_ksum>>32, desc.file_ksum & size_t(0x00000000'ffffffff)
           , s2c(file_name), s2c(link_text));
}

//----------------------------------------------------------------------------
// RdFile::compare_info | ACCESSOR: Compare desc.file_time, desc.file_info
bool                                // TRUE iff file_time or file_info differs
   RdFile::compare_info(            // Compare (this Client's) desc.file_info
     const RdFile*     that) const  // To this Server's desc.file_info
{
   if( this->desc.file_time != that->desc.file_time ) // If file times differ
     return true;

   return this->desc.file_info != that->desc.file_info;
}

//----------------------------------------------------------------------------
// RdFile::compare_time | ACCESSOR: Compare desc.file_time
int                                 // (<0, =0, >0)
   RdFile::compare_time(            // Compare (this Client's) desc.file_time
     const RdFile*     that) const  // To this Server's desc.file_time
{
   int64_t             this_time= this->desc.file_time; // This HostTime
   int64_t             that_time= that->desc.file_time; // That HostTime

   // Some filesystems are only accurate to two seconds
   this_time &= int64_t(0xFFFF'FFFF'FFFF'FFFE); // Truncate odd second out
   that_time &= int64_t(0xFFFF'FFFF'FFFF'FFFE); // Truncate odd second out

   if( this_time < that_time )
     return -1;
   else if( this_time > that_time )
     return 1;

   return 0;
}

//----------------------------------------------------------------------------
// RdFile::get_chmod | ACCESSOR: Get associated chmod permissions
int                                 // The associated chmod permissions
   RdFile::get_chmod( void ) const  // Get associated chmod permissions
{
   int minp= (int)desc.file_info;
   int mout= 0;
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

   if( env_hcdm > 9 )
     msglog("%.8x= RdFile::chmod(%.8x)\n", mout, minp);

   return mout;
}

//----------------------------------------------------------------------------
// RdFile::get_file_type | ACCESSOR: Get associated FILE_TYPE
int                                 // The associated FILE_TYPE
   RdFile::get_file_type( void ) const // Get associated FILE_TYPE
{
   int ft= FT_UNKNOWN;              // Default, unsupported type

   if( (desc.file_info&INFO_ISTYPE) == INFO_ISFILE ) // If regular file
    ft= FT_FILE;                    // Indicate regular file

   else if( (desc.file_info&INFO_ISTYPE) == INFO_ISPATH ) // If directory (Path)
     ft= FT_PATH;                   // Indicate directory

   else if( (desc.file_info&INFO_ISTYPE) == INFO_ISLINK ) // If soft link
     ft= FT_LINK;                   // Indicate link

   else if( (desc.file_info&INFO_ISTYPE) == INFO_ISPIPE ) // If FIFO (pipe)
     ft= FT_FIFO;                   // Indicate FIFO

   return ft;                       // Return, indicate file type
}

//----------------------------------------------------------------------------
// RdFile::get_full_name | ACCESSOR: Get associated fully qualified name
string
   RdFile::get_full_name( void ) const // Get fully qualified name
{  return pub::data::Name::get_full_name(path->path_name, this->file_name); }

//----------------------------------------------------------------------------
// RdFile::get_owner | ACCESSOR: Get associated CommonThread
const CommonThread*                 // The associated CommonThread
   RdFile::get_owner( void ) const  // Get associated CommonThread
{  return path->thread; }

//----------------------------------------------------------------------------
// RdFile::set_attr | ACCESSOR: Set file attributes (from HostDesc)
void
   RdFile::set_attr( void )         // Set permissions and change time
{
   string full_name= get_full_name(); // Get the file name
   const char* full_char= s2c(full_name);

   // Set the mode
   ::chmod(full_char, get_chmod()); // Set the file permissions

   // Set the last modification time
   struct utimbuf ubuff;            // The update time buffer
   ubuff.actime= 0;                 // (Ignore) access time
   ubuff.modtime= desc.file_time - JULIAN_SEC1970; // The new modification time
   utime(full_char, &ubuff);        // Update the modification time
}

//----------------------------------------------------------------------------
// RdFile::init_desc_info | Initialize HostDesc.file_info
void
   RdFile::init_desc_info( void )   // Convert st to HostDesc.file_info
{
   const mode_t minp= st.st_mode;   // The input mode
   Host_info_t  mout= 0;            // The output mode

   // Determine type of file
   if( S_ISREG(minp) ) {            // If regular file
     mout= INFO_ISFILE;
   } else if( S_ISLNK(minp) ) {     // If link
     mout= INFO_ISLINK;
   } else if( S_ISDIR(minp) ) {     // If path
     mout= INFO_ISPATH;
   } else if( S_ISFIFO(minp) ) {    // If pipe
     mout= INFO_ISPIPE;
   }

   // Set permissions
   if( (minp&S_IRUSR) != 0 ) mout |= INFO_RUSR;
   if( (minp&S_IWUSR) != 0 ) mout |= INFO_WUSR;
   if( (minp&S_IXUSR) != 0 ) mout |= INFO_XUSR;

   if( (minp&S_IRGRP) != 0 ) mout |= INFO_RGRP;
   if( (minp&S_IWGRP) != 0 ) mout |= INFO_WGRP;
   if( (minp&S_IXGRP) != 0 ) mout |= INFO_XGRP;

   if( (minp&S_IROTH) != 0 ) mout |= INFO_ROTH;
   if( (minp&S_IWOTH) != 0 ) mout |= INFO_WOTH;
   if( (minp&S_IXOTH) != 0 ) mout |= INFO_XOTH;

   // Set mode extensions
   if( (minp&S_ISUID) != 0 ) mout |= INFO_AUID; // Set user id on execution
   if( (minp&S_ISGID) != 0 ) mout |= INFO_AGID; // Set group id on excution
   if( (minp&S_ISVTX) != 0 ) mout |= INFO_AVTX; // Save swapped text after use
// if( (minp&S_ENFMT) != 0 ) mout |= INFO_AFMT; // (Broken: duplicates S_ISGID)
   desc.file_info= mout;
}

//----------------------------------------------------------------------------
// RdFile::init_desc_ksum | Initialize HostDesc.file_ksum
int                                 // Return code, 0 expected
   RdFile::init_desc_ksum( void )   // Compute file_ksum
{
   desc.file_ksum= 0;               // Default when checksum invalid

   // Only regular files get checksums, and then only if requested
   const CommonThread* owner= get_owner(); // (The owning CommonThread)
   if( !S_ISREG(st.st_mode)         // If not a regular file
       || (owner->getGVersionInfo().f[7] & VersionInfo::VIF7_KSUM) == 0 )
     return -1;

   //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   // Open the file
   string full_name= get_full_name();
   fd_t fd= open(s2c(full_name), O_RDONLY | O_BINARY);
   if( fd < 0 ) {                   // If open failed
     msgioerr("%4d RdFile.init_desc_ksum: open(%s) failure",
              __LINE__, s2c(full_name));
     return -2;
   }

   //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   // Read the file
   size_t size= desc.file_size;     // (The file size)
   if( size ) {                     // If the file contains data
     size_t DIM= (size + sizeof(PEER64_t)-1) / sizeof(PEER64_t);
     std::unique_ptr<PEER64_t[]> buffer( new PEER64_t[DIM+4] );
     buffer[size/sizeof(PEER64_t)]= 0; // Fill the last word

     size_t    L= read(fd, buffer.get(), size); // Read the entire file
     HOST64_t  ksum= 0;             // Runnning checksum
     if( L != size ) {              // If read error
       msgioerr("%4d RdFile.init_desc_ksum: read(%s) I/O error", __LINE__
               , s2c(full_name));
       close(fd);
       return -2;
     }

     for(size_t X= 0; X < DIM; ++X)
       ksum += peer_to_host(buffer[X]);

     if( close(fd) != 0 ) {         // If close failed
       msgioerr("%4d RdFile.init_desc_ksum: close(%s) failure", __LINE__
               , s2c(full_name));
       return -2;
     }

     //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
     // Checksum successfully computed
     desc.file_ksum= ksum;
   }

   return 0;
}

//----------------------------------------------------------------------------
// RdFile::init_desc_size | Initialize HostDesc.file_size
void
   RdFile::init_desc_size( void )   // Set file_size from st
{  desc.file_size= st.st_size; }

//----------------------------------------------------------------------------
// RdFile::init_desc_time | Initialize HostDesc.file_time
void
   RdFile::init_desc_time( void )   // Set file_time from st
{  desc.file_time= st.st_mtime + JULIAN_SEC1970; }

//----------------------------------------------------------------------------
// RdFile::display | Display RdFile (in the log)
void
   RdFile::display(                 // Display RdFile (in the log)
     const char*       info) const  // Heading information
{
   if( info[0] != '\0' )            // If info present
     msglog("%s ", info);           // Add informational header

   msglog("RdFile(%p) %c I(0x%.8zX) T(%'12zd) S(%'12zd) K(0x%.8zx.%.8zx) %s\n"
         , this, get_file_type()
         , desc.file_info, desc.file_time, desc.file_size
         , desc.file_ksum>>32, desc.file_ksum & size_t(0x00000000'ffffffff)
         , s2c(file_name));
}

//============================================================================
//
// Method-
//       RdPath::RdPath
//
// Purpose-
//       Constructors
//
//----------------------------------------------------------------------------
   RdPath::RdPath(                  // Constructor
     const CommonThread*
                       owner)       // Owning CommonThread
:  Path(nullptr), thread(owner)
{  }

   RdPath::RdPath(                  // Constructor
     const CommonThread*
                       owner,       // Owning CommonThread
     const string&     name)        // The (relative) Path name
:  Path(nullptr), thread(owner)
{  reset(name); }

//----------------------------------------------------------------------------
//
// Method-
//       RdPath::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   RdPath::debug(                   // Debugging display
     const char*       info) const  // Caller information
{  debugf("RdPath(%p)::debug(%s) %s\n", this, info, s2c(path_name));

   for(RdFile* file= get_head(); file; file= file->get_next()) {
     file->debug();

     if( this != file->path ) {     // Consistency check
       debugf("this(%p) != file->path(%p)\n", this, file->path);
     }
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       RdPath::debug_static
//
// Purpose-
//       Debugging stack display
//
//----------------------------------------------------------------------------
void
   RdPath::debug_static(            // Debugging display
     const char*       info)        // Caller information
{
   debugf("RdPath::debug_static(%s)\n", info);

   RdPath* path= stack.get_tail();
   while( path ) {
     debugf("RdPath(%p) '%s'\n", path, s2c(path->path_name));
     const RdFile* file= path->get_head();
     while( file ) {
       file->debug(info);
       file= file->get_next();
     }

     path= path->get_prev();
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       RdPath::get_head
//
// Purpose-
//       Get the first RdFile on the File list
//
//----------------------------------------------------------------------------
RdFile*                             // The first RdFile on the File list
   RdPath::get_head( void ) const   // Get first RdFile on the File list
{  return (RdFile*)list.get_head(); }

//----------------------------------------------------------------------------
//
// Method-
//       RdPath::fifo
//       RdPath::lifo
//
// Purpose-
//       Insert a RdFile to the File list tail
//       Insert a RdFile at the File list head
//
//----------------------------------------------------------------------------
void
   RdPath::fifo(                    // Append an RdFile to the File list
     RdFile*           file)        // The RdFile to insert
{  list.fifo(file); }

void
   RdPath::lifo(                    // Prepend an RdFile at the File list
     RdFile*           file)        // The RdFile to insert
{  list.lifo(file); }

//----------------------------------------------------------------------------
//
// Method-
//       RdPath::locate
//
// Purpose-
//       Locate an RdFile
//
//----------------------------------------------------------------------------
RdFile*                             // The associated RdFile
   RdPath::locate(                  // Locate the RdFile
     const string&     name) const  // With this name
{
   for(RdFile* file= get_head(); file; file= file->get_next()) {
     if( file->file_name == name )
       return const_cast<RdFile*>(file);
   }

   SNO(__LINE__);
   return nullptr;
}

//----------------------------------------------------------------------------
//
// Method-
//       RdPath::make_file
//
// Purpose-
//       Create and append a new RdFile
//
//----------------------------------------------------------------------------
pub::data::File*                    // The new RdFile*
   RdPath::make_file(               // Create a new File*
     const stat_t&     st,          // Struct stat descriptor
     const string&     name) const  // The File name
{  return new RdFile(this, st, name); }

pub::data::File*                    // The new RdFile*
   RdPath::make_file(               // Create a new File*
     const PeerDesc&   desc,        // PeerDesc descriptor
     const string&     name) const  // Peer name
{  return new RdFile(this, desc, name); }

//----------------------------------------------------------------------------
//
// Method-
//       RdPath::pop
//
// Purpose-
//       Remove RdPath from stack
//
//----------------------------------------------------------------------------
RdPath*
   RdPath::pop( void )              // Remove RdPath from stack
{  return stack.remq(); }

//----------------------------------------------------------------------------
//
// Method-
//       RdPath::push
//
// Purpose-
//       Add RdPath onto stack
//
//----------------------------------------------------------------------------
void
   RdPath::push(                    // Add RdPath onto stack
     RdPath*           path)        // The RdPath
{  stack.lifo(path); }

//----------------------------------------------------------------------------
//
// Method-
//       RdPath::remove
//
// Purpose-
//       Remove an RdFile from the File list
//
//----------------------------------------------------------------------------
void
   RdPath::remove(                  // Remove an RdFile from the File list
     RdFile*           file)        // The RdFile to remove
{  list.remove(file);  }

//----------------------------------------------------------------------------
//
// Method-
//       RdPath::reset
//
// Purpose-
//       Remove an RdFile from the File list
//
//----------------------------------------------------------------------------
void
   RdPath::reset(                   // Reset and load the directory
     const string&     name)        // Path name (Locally qualified)
{
   Path::reset(name);               // Reset and load the directory

   // Path::reset does not include file->link_name. Add that in now.
   for(RdFile* file= get_head(); file; file=file->get_next()) {
     if( file->get_file_type() == FT_LINK ) {
       char buffer[PATH_MAX];       // Link name buffer
       ssize_t size= rdlink(file->get_full_name(), buffer, sizeof(buffer));
       if( size < 0 )               // If cannnot read link
         throwf("%zd= rdlink(%s,%p,%zd)\n", size, s2c(file->get_full_name())
               , buffer, sizeof(buffer));

       file->link_name= buffer;
     }
   }
}
