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
//       CommonThread.cpp
//
// Purpose-
//       Implement CommonThread object methods
//
// Last change date-
//       2026/07/22
//
//----------------------------------------------------------------------------
#include <new>                      // For std::bad_alloc

#include <pub/Latch.h>              // For PUB::Latch
#include <pub/Signals.h>            // For pub::signals::Signal

#include "CommonThread.h"           // For CommonThread, implemented
#include "RdCommon.h"               // For common objects and subroutines

using PUB::Latch;                   // For convenience
using PUB::signals::Signal;         // For convenience
using std::bad_alloc;               // For convenience

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum                                // Generic enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose
}; // Generic enum

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static Latch           mutex;       // Exclusion mutex

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

//----------------------------------------------------------------------------
//
// Subroutine-
//       mode_name
//
// Purpose-
//       Get mode name from MODE type
//
//----------------------------------------------------------------------------
static const char*                  // The mode name
   mode_name(                       // Get mode name from
     int               type)        // This mode type
{
   const char* MODE= "ERROR";
   switch( type ) {
     case CommonThread::MODE_RESET:
       MODE= "RESET";
       break;

     case CommonThread::MODE_WR:
       MODE= "WR";
       break;

     case CommonThread::MODE_RD:
       MODE= "RD";
       break;

     default:
       break;
   }

   return MODE;
}

//----------------------------------------------------------------------------
//
// Method-
//       CommonThread::CommonThread
//       CommonThread::~CommonThread
//
// Purpose-
//       Constructor
//       Destructor
//
//----------------------------------------------------------------------------
   CommonThread::CommonThread(      // Constructor
     Socket*           socket)      // Associated Socket
:  Thread(), fsm(FSM_RESET), socket(socket)
{  if( HCDM ) debugf("CommonThread(%p)::CommonThread(%p)\n", this, socket);

   buffer= (char*)malloc(MAX_TRANSFER);
   if( buffer == nullptr )
     throw bad_alloc();

tree_check_handler=                 // Connect the tree_check_handler
   handle_check_signal([this](pub::signals::Event_t& E)
{
   CheckEvent* event= dynamic_cast<CheckEvent*>(&E);
   if( event ) {
     debugf("RdPath::debug_stack(%s)\n", event->info);

     RdPath* path= stack.get_tail();
     while( path ) {
       debugf("\nRdPath(%p) '%s'\n", path, s2c(path->path_name));
       const RdFile* file= path->get_head();
       while( file ) {
         file->debug(event->info);
         file= file->get_next();
       }

       path= path->get_prev();
     }
   }
});
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   CommonThread::~CommonThread( void ) // Destructor
{  if( HCDM ) debugf("CommonThread(%p)::~CommonThread()\n", this);

   tree_check_handler.disconnect(); // Disconnect the tree check handler

   if( socket ) {
     socket->close();
     socket= nullptr;
   }

   free(buffer);
   buffer= nullptr;
}

//----------------------------------------------------------------------------
//
// Method-
//       CommonThread::compare
//
// Purpose-
//       Compare file name strings, accounting for case
//
//----------------------------------------------------------------------------
int                                 // Result: <0, =0, >0
   CommonThread::compare(           // Compare strings
     const string&     lhs,         // Left hand side
     const string&     rhs)         // Right hand side
{
   if( gVersionInfo.f[0] & VersionInfo::VIF0_CASE ) // If case-sensitive
     return lhs.compare(rhs);

   return strcasecmp(s2c(lhs), s2c(rhs));
}

//----------------------------------------------------------------------------
//
// Method-
//       CommonThread::set_globalVersionInformation
//
// Purpose-
//       Combine the local and remote capabilities vectors.
//
// Implementation notes-
//       global= (local[*] & remote[*]) | local[VIF7_KSUM] | remote[VIF7_KSUM]
//
//----------------------------------------------------------------------------
void
   CommonThread::set_globalVersionInformation( void ) // Initialize global info
{
   gVersionInfo= lVersionInfo;
   for(size_t i=0; i<sizeof(gVersionInfo.f); i++)
     gVersionInfo.f[i]= lVersionInfo.f[i] & rVersionInfo.f[i];

   // If either side uses checksums, both do
   if( (lVersionInfo.f[7] & VersionInfo::VIF7_KSUM) != 0 ||
       (rVersionInfo.f[7] & VersionInfo::VIF7_KSUM) != 0 )
     gVersionInfo.f[7] |= VersionInfo::VIF7_KSUM;
}

//----------------------------------------------------------------------------
//
// Method-
//       CommonThread::set_localVersionInformation
//
// Purpose-
//       Set the local capability vector.
//
//----------------------------------------------------------------------------
void
   CommonThread::set_localVersionInformation( void ) // Initialize local info
{
   memset(&lVersionInfo, 0, sizeof(lVersionInfo));
   strcpy(lVersionInfo.version, RD_VERSION);
   #if defined(_OS_CYGWIN)          // For Cygwin
     lVersionInfo.f[0] |= VersionInfo::VIF0_ABSD; // BSD attributes
     lVersionInfo.f[1] |= VersionInfo::VIF1_OCYG; // CYG operating system

   #elif defined(_OS_BSD)           // For standard BSD
     lVersionInfo.f[0] |= VersionInfo::VIF0_ABSD; // BSD attributes
     lVersionInfo.f[0] |= VersionInfo::VIF0_CASE; // Case sensitivity applies
     lVersionInfo.f[1] |= VersionInfo::VIF1_OBSD; // BSD operating system

   #else
     static_assert(false, "OS not supported");
   #endif

   // Operational controls
   if( opt_verify )
     lVersionInfo.f[7] |= VersionInfo::VIF7_KSUM; // Verify checksum
}

//----------------------------------------------------------------------------
//
// Method-
//       CommonThread::rd_buff(size_t)
//
// Purpose-
//       Fill the read buffer
//
// Implementation notes-
//       MAX_TRANSFER size enforced
//
//----------------------------------------------------------------------------
void
   CommonThread::rd_buff(           // Fill the read buffer
     size_t            size)        // To this minimum length
{  if( HCDM )
     debugf("CommonThread(%p)::rd_buff(%'zd)\n", this, size);

   if( size > MAX_TRANSFER ) SNO(__LINE__); // Disallow buffer overfill

   rd_mode();                       // Set read mode

   size_t left= buff_size - buff_used;
   if( left >= size )               // If buffer available
     return;

   // Normalize the buffer (setting buff_used == 0)
   if( buff_used ) {
     memmove(buffer, buffer + buff_used, left);
     buff_size= left;
     buff_used= 0;
   }

   // Fill the (normalized) buffer (at least) to the required size
   while( buff_size < size ) {
     left= MAX_TRANSFER - buff_size;
     size_t L= rd_recv(buffer + buff_size, left);
     buff_size += L;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       CommonThread::rd_buff(void*, size_t)
//
// Purpose-
//       Read buffer data area
//
// Implementation notes-
//       MAX_TRANSFER does not apply. Any size may be used.
//       (The buffer is only used for the initial transfer)
//
//----------------------------------------------------------------------------
void
   CommonThread::rd_buff(           // Read buffer data area
     void*             v_addr,      // Input data buffer address
     size_t            i_size)      // Input data buffer length
{  if( HCDM )
     debugf("CommonThread(%p)::rd_buff(%p,%'zd)\n", this, v_addr, i_size);

   rd_mode();                       // Set read mode

   char*  addr= (char*)v_addr;
   size_t size= i_size;
   size_t left= buff_size - buff_used;
   if( left == 0 && size < MAX_TRANSFER ) {
     buff_used= 0;
     buff_size= rd_recv(buffer, MAX_TRANSFER);
     left= buff_size - buff_used;
   }

   if( left >= size ) {             // If sufficient fill available
     memcpy(addr, buffer + buff_used, size);
     buff_used += size;
     if( env_iodm ) {
       msglog("\n");
       msglog("rd_buff(%p,%'zd)\n", addr, size);
       msgdump(addr, min(size, env_iodm));
     }
     return;
   }

   // If there's any fill remaining, use it
   if( left ) {                     // If any fill remains
     memcpy(addr, buffer + buff_used, left);
     addr += left;
     size -= left;
     buff_size= buff_used= 0;       // (Normalize the buffer)
   }

   // Read any remainder directly into the input data area
   while( size ) {
     size_t L= rd_recv(addr, size);
     addr += L;
     size -= L;
   }

   if( env_iodm ) {
     msglog("\n");
     msglog("rd_buff(%p,%'zd)\n", v_addr, i_size);
     msgdump(v_addr, min(i_size, env_iodm));
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       CommonThread::rd_buff(HOST16_t)
//
// Purpose-
//       Read and convert a (PEER16_t) size
//
//----------------------------------------------------------------------------
HOST16_t                            // The resultant HOST16_t
   CommonThread::rd_buff(           // Read (PEER16_t) size; convert it into
     HOST16_t&         host)        // (OUT) This HOST16_t size
{  if( HCDM )
     debugf("CommonThread(%p)::rd_buff(HOST16_t(%'d))\n", this, host);

   PEER16_t peer;

   rd_buff(sizeof(peer));           // Fill the buffer
   rd_buff(&peer, sizeof(peer));    // Read the peer data
   host= peer_to_host(peer);

   if( HCDM )
     debugf("CommonThread(%p)::rd_buff(HOST16_t(%d)\n", this, host);

   return host;
}

//----------------------------------------------------------------------------
//
// Method-
//       CommonThread::rd_buff(string&)
//
// Purpose-
//       Read and convert a {(PEER16_t) size, char[size]}
//
// Implementation notes-
//       Intended for file name strings, maximum length: NAME_MAX
//
//----------------------------------------------------------------------------
string                              // The resultant string
   CommonThread::rd_buff(           // Read {(PEER16_t) size, char[size]}
     string&           name)        // (OUT) This (file name) string
{
   HOST16_t name_size;              // The (file name) string length
   char name_buff[NAME_MAX + 1];    // The (file name) string buffer

   rd_buff(sizeof(name_size));      // Prepare the buffer
   rd_buff(name_size);              // Read the name size
   if( name_size > NAME_MAX ) {
     debugf("Name too large(%d > %d)\n", name_size, NAME_MAX);
     SNO(__LINE__);
   }

   rd_buff(name_buff, name_size);   // Read into name buffer
   name= string(name_buff, name_size); // The (OUTPUT) name string

   if( HCDM )
     debugf("CommonThread(%p)::rd_buff(string(%s))\n", this, s2c(name));

   return name;
}

//----------------------------------------------------------------------------
//
// Method-
//       CommonThread::rd_data(void*, size_t)
//
// Purpose-
//       Read data area
//
//----------------------------------------------------------------------------
void
   CommonThread::rd_data(           // Read data area
     void*             addr,        // Input data buffer address
     size_t            size)        // Input data buffer length
{  if( HCDM )
     debugf("CommonThread(%p)::rd_data(%p,%'zd)\n", this, addr, size);

   rd_buff(addr, size);
}

//----------------------------------------------------------------------------
//
// Method-
//       CommonThread::rd_data(string&)
//
// Purpose-
//       Read and convert a {(PEER16_t) size, char[size]}
//
//----------------------------------------------------------------------------
string
   CommonThread::rd_data(           // Read {(PEER16_t) size, char[size]}
     string&           name)        // (OUT) This (file name) string
{
   rd_buff(name);

   if( HCDM )
     debugf("CommonThread(%p)::rd_data(string(%s))\n", this, s2c(name));

   return name;
}

//----------------------------------------------------------------------------
//
// Method-
//       CommonThread::rd_mode
//
// Purpose-
//       Go into READ mode
//
// Implementation notes-
//       Automatic conversion from MODE_WR into MODE_RD allowed and expected.
//
//----------------------------------------------------------------------------
void
   CommonThread::rd_mode( void )    // Go into input mode
{  if( HCDM ) debugf("rd_mode(%s)\n", mode_name(mode));

   if( mode == MODE_RD )            // If already in READ mode
     return;

   wr_buff();                       // Empty the write buffer, allowing change
   mode= MODE_RD;
}

//----------------------------------------------------------------------------
//
// Method-
//       CommonThread::rd_path
//
// Purpose-
//       Read server path
//
//----------------------------------------------------------------------------
RdPath*                             // The new RdPath
   CommonThread::rd_path(           // Get server RdPath
     const RdFile*     file)        // The client path file
{  if( HCDM )
     debugf("CommonThread(%p)::rd_path(RdFile({%s,%s}))\n", this
           , s2c(file->path->path_name), s2c(file->file_name));

   RdPath* path= new RdPath(file->path->thread);

   // Read server path count
   PEER32_t peer_count;             // The (32 bit) file count
   rd_buff(sizeof(peer_count));
   rd_buff(&peer_count, sizeof(peer_count));
   HOST32_t host_count= peer_to_host(peer_count);

   for(HOST32_t X= 0; X < host_count; ++X) { // Load the Files
     PeerDesc desc;                 // The Peer descriptor
     rd_buff(&desc, sizeof(desc));

     string name;                   // The Peer file name
     rd_buff(name);                 // Read the file name

     RdFile* file= new RdFile(path, desc, name);
     if( file->get_file_type() == FT_LINK ) {
       rd_buff(file->link_name);
     }

     path->fifo(file);
   }
   path->list.sort();

   return path;
}

//----------------------------------------------------------------------------
//
// Method-
//       CommonThread::rd_recv(void*, size_t)
//
// Purpose-
//       Socket receive from network.
//
//----------------------------------------------------------------------------
size_t                              // Number of bytes received
   CommonThread::rd_recv(           // Socket receive operation
     void*             addr,        // Data address
     size_t            size)        // Data length
{
   ssize_t L= socket->recv(addr, size); // Socket receive
   if( env_iodm ) {
     msglog("\n");
     msglog("%'zd= rd_recv(%p,%'zd)\n", L, addr, size);
     msgdump(addr, min(size_t(L), env_iodm));
     if( HCDM ) {
       debugf("\n");
       debugf("%'zd= rd_recv(%p,%'zd)\n", L, addr, size);
       dump(addr, min(size_t(L), env_iodm));
     }
   } else if( HCDM ) {
     debugf("%'zd= rd_recv(%p,%'zd)\n", L, addr, size);
     dump(addr, min(size_t(L), 32));
   }

   if( L < 1 ) {
     if( L == 0 || errno == ECONNABORTED ) {
       fprintf(stderr, "Connection aborted (recv)\n");
       rdterm();
       exit(1);
     }
     throwf("%4d ERROR: %'zd= rd_recv %d:%s", __LINE__, L
           , errno, strerror(errno));
   }

   return L;
}

//----------------------------------------------------------------------------
//
// Method-
//       CommonThread::wr_buff( void )
//
// Purpose-
//       Empty the write buffer
//
// Implementation notes-
//       Normalizes the buffer, making it convertible to MODE_RD
//
//----------------------------------------------------------------------------
void
   CommonThread::wr_buff( void )    // Empty the write buffer
{  if( HCDM ) debugf("wr_buff()\n");

   wr_mode();                       // Go into WRITE mode

   const char* addr= buffer + buff_used;
   size_t size= buff_size - buff_used;
   while( size ) {                  // While data remains
     size_t L= wr_send(addr, size); // Transmit what we can
     addr += L;
     size -= L;
   }

   buff_used= buff_size= 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       CommonThread::wr_buff(const void*,size_t)
//
// Purpose-
//       Write into write buffer
//
// Implementation notes-
//       MAX_TRANSFER does not apply. Any size may be used.
//
//----------------------------------------------------------------------------
void
   CommonThread::wr_buff(           // Append into buffer
     const void*       v_addr,      // Data address
     size_t            size)        // Data length
{  if( env_iodm ) {
     msglog("\n");
     msglog("wr_buff(%p,%'zd)\n", v_addr, size);
     msgdump(v_addr, min(size, env_iodm));
     if( HCDM ) {
       debugf("\n");
       debugf("wr_buff(%p,%'zd)\n", v_addr, size);
       dump(v_addr, min(size, env_iodm));
     }
   } else if( HCDM ) {
     debugf("wr_buff(%p,%'zd)\n", v_addr, size);
     dump(v_addr, min(size, 32));
   }

   if( size == 0 ) SNO(__LINE__);   // MUST NOT have zero length write

   wr_mode();

   size_t left= MAX_TRANSFER - buff_size;
   if( left >= size ) {             // If buffer space available
     memcpy(buffer + buff_size, v_addr, size);
     buff_size += size;
     return;
   }

   wr_buff();                       // Empty the write  buffer
   const char* addr= (const char*)v_addr; // Data address
   while( size ) {                  // Write the data
     size_t L= wr_send(addr, size);
     addr += L;
     size -= L;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       CommonThread::wr_buff(const HOST16_t&)
//
// Purpose-
//       Append string size into write buffer
//
//----------------------------------------------------------------------------
void
   CommonThread::wr_buff(           // Append into buffer
     const HOST16_t&   host)        // This file size string
{  if( HCDM ) debugf("wr_buff(HOST16_t(%d))\n", host);

   PEER16_t peer= host_to_peer(host);
   wr_buff(&peer, sizeof(peer));
}

//----------------------------------------------------------------------------
//
// Method-
//       CommonThread::wr_buff(const string&)
//
// Purpose-
//       Append string into write buffer
//
//----------------------------------------------------------------------------
void
   CommonThread::wr_buff(           // Append into buffer
     const string&     name)        // This file name string
{  if( HCDM )
     debugf("wr_buff(string(%s)) size(%zd)\n", s2c(name), name.size());

   if( name.size() > NAME_MAX ) SNO(__LINE__);

   HOST16_t host_size= (HOST16_t)name.size();
   wr_buff(host_size);
   wr_buff(s2c(name), host_size);
}

//----------------------------------------------------------------------------
//
// Method-
//       CommonThread::wr_data(const void*,size_t)
//
// Purpose-
//       Unconditionally write data
//
// Implementation notes-
//       Empties the write buffer first (resetting the mode)
//
//----------------------------------------------------------------------------
void
   CommonThread::wr_data(           // Transmit data
     const void*       v_addr,      // Data address
     size_t            size)        // Data length
{  if( env_iodm ) {
     msglog("\n");
     msglog("wr_data(%p,%'zd)\n", v_addr, size);
     msgdump(v_addr, min(size, env_iodm));
     if( HCDM ) {
       debugf("\n");
       debugf("wr_data(%p,%'zd)\n", v_addr, size);
       dump(v_addr, min(size, env_iodm));
     }
   } else if( HCDM ) {
     debugf("wr_buff(%p,%'zd)\n", v_addr, size);
     dump(v_addr, min(size, 32));
   }

   if( size <= 256 && buff_used != buff_size ) { // If small buffer append
     wr_buff(v_addr, size);
     wr_buff();
   } else {
     wr_buff();                     // Empty the buffer
     const char* addr= (const char*)v_addr;
     while( size ) {                // While data remains
       size_t L= wr_send(addr, size); // Transmit what we can
       addr += L;
       size -= L;
     }
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       CommonThread::wr_data(const string&)
//
// Purpose-
//       Write string
//
//----------------------------------------------------------------------------
void
   CommonThread::wr_data(           // Append into buffer
     const string&     name)        // This file name string
{  if( HCDM )
     debugf("wr_data(string(%s)) size(%zd)\n", s2c(name), name.size());

   wr_buff(name);                   // Add the string to the buffer, then
   wr_buff();                       // Empty the buffer
}

//----------------------------------------------------------------------------
//
// Method-
//       CommonThread::wr_mode
//
// Purpose-
//       Go into WRITE mode (i.e. using buffer in WRITE mode)
//
// Implementation notes-
//       Cannot convert an active MODE_RD into MODE_WR.
//
//----------------------------------------------------------------------------
void
   CommonThread::wr_mode( void )    // Go into WRITE mode
{  if( HCDM )
     debugf("wr_mode(%s) {%zd,%zd}\n", mode_name(mode), buff_used, buff_size);

   if( mode == MODE_WR )            // If already in WRITE mode
     return;

   if( buff_used == buff_size ) {
     buff_used= buff_size= 0;
     mode= MODE_WR;
     return;
   }

   // SHOULD NOT OCCUR: Now in MODE_RD.
   debugf("mode(%s) buffer(%p) buff_used(%'zd) buff_size(%'zd)\n"
         , mode_name(mode), buffer, buff_used, buff_size);
   dump(buffer, buff_size);
   dump(buffer + buff_used, buff_size - buff_used);
   SNO(__LINE__);                   // Debugging required if you hit this
}

//----------------------------------------------------------------------------
//
// Method-
//       CommonThread::wr_path
//
// Purpose-
//       Send a sorted directory on network.
//
//----------------------------------------------------------------------------
void
   CommonThread::wr_path(           // Send a sorted directory
     const RdPath*     path)        // -> RdPath
{  if( HCDM ) debugf("wr_path\n");

   msglog("wr_path\n");

   size_t count= path->list.size(); // Count the files
   if( count > UINT32_MAX ) SNO(__LINE__); // (New version needed if occurs)
   HOST32_t host_count= (HOST32_t)count;
   PEER32_t peer_count= host_to_peer(host_count);
   wr_buff(&peer_count, sizeof(peer_count));

   // Write the path information
   const RdFile* file= const_cast<RdPath*>(path)->get_head();
   while( file ) {
     --count;                       // (For consistency check)
     PeerDesc peerDesc(file->desc);
     wr_buff(&peerDesc, sizeof(peerDesc));

     wr_buff(file->file_name);

     // For Links, write the link name string
     if( file->get_file_type() == FT_LINK )
       wr_buff(file->link_name);

     file= const_cast<const RdFile*>(file->get_next());
   }
   if( count ) SNO(__LINE__);       // File count inconsistent (assert)

   wr_buff();                       // Empty the write buffer
}

//----------------------------------------------------------------------------
//
// Method-
//       CommonThread::wr_send(const void*, size_t)
//
// Purpose-
//       Socket send to peer.
//
//----------------------------------------------------------------------------
size_t                              // Number of bytes sent
   CommonThread::wr_send(           // Socket send operation
     const void*       addr,        // Data address
     size_t            size)        // Data length
{
   ssize_t L= socket->send(addr, size); // Socket send
   if( env_iodm ) {
     msglog("\n");
     msglog("%'zd= wr_send(%p,%'zd)\n", L, addr, size);
     msgdump(addr, min(size_t(L), env_iodm));
     if( HCDM ) {
       debugf("\n");
       debugf("%'zd= wr_send(%p,%'zd)\n", L, addr, size);
       dump(addr, min(size_t(L), env_iodm));
     }
   } else if( HCDM ) {
     debugf("%'zd= wr_send(%p,%'zd)\n", L, addr, size);
     dump(addr, min(size_t(L), 32));
   }

   if( L < 1 ) {
     fprintf(stderr, "%4d ERROR: %'zd= wr_send %d:%s\nConnection aborted\n"
                   , __LINE__, L, errno, strerror(errno));
     throw "disconnected";
   }

   return L;
}
