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
//       ServerThread.cpp
//
// Purpose-
//       Implement ServerThread object methods
//
// Last change date-
//       2026/07/21
//
// Implementation notes-
//       This multi-threaded server DOES NOT change path or file permissions
//       during transfer. (A second thread might use the modified permissions,
//       making such a change permanent.)
//
//----------------------------------------------------------------------------
#include <exception>                // For std::exception
#include <cstdlib>                  // For size_t
#include <cstring>                  // For memcpy, ...

#include <sys/stat.h>               // For S_IREAD ...

#include "IoCommon.h"               // For I/O common objects and subroutines
#include "ServerThread.h"           // For ServerThread, implemented

#ifndef O_BINARY                    // Defined in CYGWIN, not in LINUX
  #define O_BINARY 0
#endif

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum                                // Generic enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose
}; // Generic enum

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

[[noreturn]]
static void
   SNO(int line, int op)            // (Should Not Occur, invalid opcode )
{  throwf("%4d %s Client op(%c,%d) invalid\n", line, __FILE__, op, op); }

//----------------------------------------------------------------------------
//
// Method-
//       ServerThread::ServerThread
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   ServerThread::ServerThread(      // Constructor
     Socket*           socket,      // Associated Socket
     string            path)        // Initial directory (from ListenThread)
:  CommonThread(socket), init_path(path)
{  if( opt_hcdm )
     debugf("ServerThread(%p)::ServerThread(%p,%s)\n", this
           , socket, s2c(init_path));

   //-------------------------------------------------------------------------
   // Set transfer size -- optimization attempt (has no noticable effect)
   //-------------------------------------------------------------------------
   if( USE_RCVBUF_SIZE > 0 ) {      // If transfer size optimization
     int optval= USE_RCVBUF_SIZE;
     socket->set_option(SOL_SOCKET, SO_RCVBUF, &optval, sizeof(optval));
   }

   start();                         // Start the Thread, invoking run()
}

//----------------------------------------------------------------------------
//
// Method-
//       ServerThread::~ServerThread
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   ServerThread::~ServerThread( void ) // Destructor
{  if( opt_hcdm ) debugf("ServerThread(%p)::~ServerThread\n", this); }

//----------------------------------------------------------------------------
//
// Method-
//       ServerThread::exchange_versionID
//
// Function-
//       Exchange version identifiers, setting rVersionInfo
//
//----------------------------------------------------------------------------
int                                 // TRUE if version identifiers match
   ServerThread::exchange_versionID( void ) // Exchange version identifiers
{  if( opt_hcdm )
     debugf("ServerThread(%p)::exchange_versionID\n", this);

   set_localVersionInformation();   // Initialize local version information

   HOST16_t peer_size= 0;
   rd_buff(peer_size);
   rd_data(&rVersionInfo, sizeof(rVersionInfo));

   HOST16_t host_size= (HOST16_t)sizeof(lVersionInfo);
   wr_buff(host_size);
   wr_data(&lVersionInfo, sizeof(lVersionInfo));

   if( host_size != peer_size ) {
     msgout("Server: exchange size mismatch: Here(%d) Peer(%d)\n"
           , host_size, peer_size);
     return false;
   }

   if( strcmp(RD_VERSION, rVersionInfo.version) != 0 ) {
     msgout("Server: exchange version mismatch: Here(%s) Peer(%s)\n"
           , RD_VERSION, rVersionInfo.version );
     return false;
   }

   set_globalVersionInformation();
   return true;
}

//----------------------------------------------------------------------------
//
// Method-
//       ServerThread::run
//
// Purpose-
//       Operate the ServerThread.
//
//----------------------------------------------------------------------------
void
   ServerThread::run( void )        // Operate this ServerThread
{  if( opt_hcdm ) debugf("ServerThread(%p)::run...\n", this);

   // Connected message
   if( init_path.size() > (PATH_MAX-1) )
     throwf("Path(%s) name too long (%'zd > %'zd", s2c(init_path)
           , init_path.size(), size_t(PATH_MAX-1));

   string peer_name= socket->get_peer_name();
   msgout("Server: Connected... Host(%s:%d)\n"
         , s2c(peer_name), socket->get_peer_port());

   if( HCDM )
     opt_hcdm= true;
   if( VERBOSE > opt_verbose )
     opt_verbose= VERBOSE;

   // Handle client request messages
   msglog("ServerThread(%s)\n", s2c(init_path));
   fsm= FSM_READY;                  // Indicate operational
   try {
     bool validated= false;         // Default, not validated
     while( fsm == FSM_READY ) {    // Process initial server requests
       PeerRequest  query;          // Client command
       PeerResponse qresp;          // Reply to query
       rd_data(&query, 1);          // Read client command
       qresp.rc= RSP_YO;            // Default, operation accepted

       switch(query.oc) {           // Process request
         case REQ_GOTO: {{{{        // Goto subdirectory
           if( !validated ) {
             msgout("%4d Server: missing exchange_vertionID\n", __LINE__);
             say_no();
             break;
           }

           RdPath path(this);       // The initial RdPath
           path.path_name= init_path;
           string file_name;        // Get path file name
           rd_data(file_name);      // Get path file name
           RdFile file(&path, file_name); // The requested path file

           // Verify that we have permission to read into this directory
           if( (file.desc.file_info&INFO_RUSR) == 0
               || (file.desc.file_info&INFO_XUSR) == 0 ) {
             say_no();              // Reject, can't use directory
             break;
           }
           wr_data(&qresp, 1);      // Command accepted

           serve_path(&file);
           validated= false;
           break;
           }}}}

         case REQ_VERSION:          // Exchange version identifiers
           validated= exchange_versionID();
           if( !validated )
             qresp.rc= RSP_NO;
           wr_data(&qresp, 1);
           break;

         case REQ_CWD:              // Retrieve CWD
           wr_data(&qresp, 1);
           wr_data(init_path);
           break;

         case REQ_QUIT:             // Exit
           fsm= FSM_CLOSE;          // Normal termination
           wr_data(&qresp, 1);      // The operation is accepted
           sleep(0.5);              // Allow time for send completion
           break;

         default:                   // Error, invalid question
           SNO(__LINE__, query.oc);
           break;
       }
     }

     msgout("Server: ...Completed Host(%s:%d)\n"
           , s2c(peer_name), socket->get_peer_port());
   } catch( std::exception& X ) {
     msgerr("Server: exception(%s)\n", X.what());
   } catch( const char* X ) {
     msgerr("Server: const char*(%s)\n", X);
   } catch(...) {
     msgerr("Server: catch(...)\n");
   }

   // Thread termination
   if( fsm == FSM_READY ) {         // If forced termination
     msgout("Server: ...Cancelled Host(%s:%d)\n"
           , s2c(peer_name), socket->get_peer_port());
   }

   if( opt_hcdm ) debugf("...ServerThread(%p)::run\n", this);
}

//----------------------------------------------------------------------------
//
// Method-
//       ServerThread::say_no
//
// Purpose-
//       Send negative reponse
//
//----------------------------------------------------------------------------
void
   ServerThread::say_no( void )     // Send negative response
{  if( opt_hcdm ) debugf("ServerThread(%p)::say_no\n", this);

   PeerResponse qresp;              // Reply data block
   qresp.rc= RSP_NO;
   wr_data(&qresp, 1);
}

//----------------------------------------------------------------------------
//
// Method-
//       ServerThread::serve_file
//
// Function-
//       Return a file to the client.
//
//----------------------------------------------------------------------------
void
   ServerThread::serve_file(        // Install a file
     string            path_name,   // Current Path name
     RdFile*           file)        // -> RdFile
{  if( opt_hcdm )
     debugf("ServerThread(%p)::serve_file(%s/%s)\n", this
           , s2c(path_name), s2c(file->get_file_name()));

   msglog("serve_file(%s/%s)\n", s2c(path_name), s2c(file->get_file_name()));

   //-------------------------------------------------------------------------
   // Open the file
   //-------------------------------------------------------------------------
   string file_name= get_full_name(path_name, file->get_file_name());
   fd_t fd= open(s2c(file_name), O_RDONLY | O_BINARY);
   if( fd < 0 ) {                   // If open failed
     msgioerr("%4d Server: open(%s) failure", __LINE__, s2c(file_name));

     say_no();
     return;
   }

   //-------------------------------------------------------------------------
   // Accept the request
   //-------------------------------------------------------------------------
   PeerResponse qresp;              // Reply to client
   qresp.rc= RSP_YO;                // Default, request accepted
   wr_data(&qresp, 1);              // Accept the request

   //-------------------------------------------------------------------------
   // Send the file
   //-------------------------------------------------------------------------
   size_t size= file->desc.file_size; // Entire file left to be sent
   while( size > 0 ) {              // More bytes need to be sent
     size_t read_size= size;
     if( read_size > MAX_TRANSFER )
       read_size= MAX_TRANSFER;

     ssize_t L= read(fd, buffer, min(size, MAX_TRANSFER)); // Read file
     if( L < 1 )
       throwf("Server %'zd= read(%s) error: %d:%s"
             , L, s2c(file->get_file_name()), errno, strerror(errno));

     wr_data(buffer, L);            // Send some of the file
     size -= (size_t)L;             // Those sent aren't left to send
   }

   //-------------------------------------------------------------------------
   // Close the file
   //-------------------------------------------------------------------------
   if( close(fd) != 0 )             // Close data file failed
     throwf("%4d Server: close(%s) failure", __LINE__
           , s2c(file->get_file_name()));
}

//----------------------------------------------------------------------------
//
// Method-
//       ServerThread::serve_path
//
// Function-
//       Install a directory subtree.
//
//----------------------------------------------------------------------------
void
   ServerThread::serve_path(        // Serve directory subtree
     RdFile*           path_file)   // The directory RdFile
{  if( opt_hcdm )
     debugf("ServerThread(%p)::serve_path(%s)\n", this
           , s2c(path_file->get_full_name()) );

   //-------------------------------------------------------------------------
   // Validate parameters
   //-------------------------------------------------------------------------
   if( path_file->get_file_type() != FT_PATH )  {
     SNO(__LINE__);                 // Ask to serve path that's not a path
     return;
   }

   //-------------------------------------------------------------------------
   // Load the directory
   //-------------------------------------------------------------------------
   msglog("serve_path(%s)..\n", s2c(path_file->get_full_name()));

   string this_name= path_file->get_full_name();
   if( path_file->file_name == "." )
     this_name=path_file->path->path_name;

   RdPath this_path(this, this_name);
   RdPath::push(&this_path);

   //-------------------------------------------------------------------------
   // Reply with directory information
   //-------------------------------------------------------------------------
   wr_path(&this_path);

   //-------------------------------------------------------------------------
   // Install this subdirectory
   //-------------------------------------------------------------------------
   while( fsm == FSM_READY ) {      // Process this directory
     PeerRequest  query;            // Question from client
     PeerResponse qresp;            // Reply to client

     rd_data(&query, 1);            // Read client command
     qresp.rc= RSP_YO;              // Default, operation accepted

     switch(query.oc) {             // Process request
       case REQ_FILE: {{{{          // Install file
         //-------------------------------------------------------------------
         // Install file
         //-------------------------------------------------------------------
         string file_name;          // The file name
         rd_data(file_name);        // Get the file name
         RdFile* file= this_path.locate(file_name); // Locate the RdFile
         if( file->get_file_type() != FT_FILE )
           SNO(__LINE__);           // Shouldn't ask for non-regular file

         // Verify that we have permission to read this file
         if( (file->desc.file_info & INFO_RUSR) == 0 ) {
           say_no();
           break;
         }

         serve_file(this_name, file);
         break;
         }}}}

       case REQ_GOTO: {{{{          // Goto subdirectory
         //-------------------------------------------------------------------
         // Install subdirectory
         //-------------------------------------------------------------------
         string file_name;          // The directory name
         rd_data(file_name);        // Get directory name
         RdFile* file= this_path.locate(file_name);
         if( file == nullptr || file->get_file_type() != FT_PATH )  {
           SNO(__LINE__);           // Ask to install path, but it's not a path
           say_no();                // Reject, not a directory
           break;
         }

         // Verify that we have permission to read into this directory
         if( (file->desc.file_info&INFO_RUSR) == 0
             || (file->desc.file_info&INFO_XUSR) == 0 ) {
           say_no();                // Reject, can't use directory
           break;
         }

         // Install the new subdirectory
         wr_data(&qresp, 1);        // The operation is accepted
         serve_path(file);
         break;
         }}}}

       case REQ_QUIT:               // Exit
         //-------------------------------------------------------------------
         // Exit (back to previous directory)
         //-------------------------------------------------------------------
         wr_data(&qresp, 1);        // The operation is accepted
         msglog("..serve_path(%s)\n", s2c(this_name));
         RdPath::pop();
         return;

       default:                     // Error, invalid question
         SNO(__LINE__, query.oc);
         break;
     }
   }
}
