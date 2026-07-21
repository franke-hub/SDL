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
//       ClientThread.cpp
//
// Purpose-
//       Implement ClientThread object methods
//
// Last change date-
//       2026/07/21
//
//----------------------------------------------------------------------------
#include <exception>                // For std::exception
#include <cstring>                  // For strcasecmp

#include <sys/stat.h>               // For S_IREAD, ...

#include <pub/IO.h>                 // For namespace pub::io

#include "IoCommon.h"               // For I/O common objects and subroutines
#include "ClientThread.h"           // For ClientThread, implemented

#ifndef O_BINARY                    // Defined in CYGWIN, not in LINUX
  #define O_BINARY 0
#endif

#define IO PUB::io

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum                                // Generic enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // VERBOSITY, higher is more verbose
}; // generic enum

enum AC                             // Action codes
{  AC_NOP=                        0 // No action
,  AC_GETSERVER=                  1 // Next SERVER, Keep CLIENT
,  AC_GETCLIENT=                  2 // Keep SERVER, Next CLIENT
,  AC_BOTH=                       3 // Get next (CLIENT and SERVER)
}; // enum AC

enum RC                             // Return codes
{  RC_NORM= 0                       // Normal (No error)
,  RC_ERROR= 1                      // Error
}; // enum RC

//----------------------------------------------------------------------------
// Constant data areas
//----------------------------------------------------------------------------
static constexpr const char*
                       const_file_name= "!const"; // The const file name

//----------------------------------------------------------------------------
//
// Subroutine-
//       attempted_const_modify
//
// Function-
//       Attempt to modify constant file
//
//----------------------------------------------------------------------------
[[noreturn]]
static void
   attempted_const_modify(          // Attempt to modify
     const string&     name)        // This const file
{  throwf("ERROR: Attempt to modify(%s)\n(This must be done manually.)\n"
          , s2c(name));
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       case_comp
//
// Function-
//       Case insensitive string compare
//
//----------------------------------------------------------------------------
static int                          // Case insensitive comparison
   case_comp(                       // Case insensitive string compare
     string            lhs,         // Left Hand Side
     string            rhs)         // Right Hand Side
{
   const char* cc_lhs= s2c(lhs);
   const char* cc_rhs= s2c(rhs);
   return strcasecmp(cc_lhs, cc_rhs);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       invalid_response
//
// Function-
//       An invalid response was received from the server
//
//----------------------------------------------------------------------------
[[noreturn]]
static void                         // (Does not return)
   invalid_response(                // Handle invalid response
     int               lineno,      // Calling line number
     const char*       op_name,     // Failing operation name
     int               op_resp)     // The invalid response
{  fprintf(stderr, "%4d ClientThread: Why did Server reply '%c' (%d) to %s?\n"
                 , lineno, op_resp, op_resp, op_name);
   ClientThread::stop();
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       print_action
//
// Function-
//       Print action taken for an item.
//
//----------------------------------------------------------------------------
static void
   print_action(                    // Action was taken
     const char*       action,      // This action was taken
     const RdFile*     file,        // This file was acted upon
     const char*       reason)      // This is the reason why
{
   if( opt_quiet )                  // If silent running
     msglog("  %-10s %c %-32s %s\n"
           , action, get_file_type(file), s2c(file->file_name), reason);
   else
     msgout("  %-10s %c %-32s %s\n"
           , action, get_file_type(file), s2c(file->file_name), reason);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       print_path
//
// Function-
//       Print path name (once per path)
//
//----------------------------------------------------------------------------
static void
   print_path(
     int&              once,        // INP/OUT: Is this the first print_path?
     const string&     path)        // The path name to print
{
   if( once == false                // Is the first print_path for this path?
       && opt_quiet != true )       // and we are not running silent
     msgout("\n%s\n", s2c(path));   // Display the current directory

   once= true;
}

//----------------------------------------------------------------------------
//
// Method-
//       ClientThread::ClientThread
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   ClientThread::ClientThread(      // Constructor
     Socket*           socket,      // Associated Socket
     const string&     path)        // Initial directory
:  CommonThread(socket), init_path(path)
{  if( opt_hcdm )
     debugf("ClientThread(%p)::ClientThread(%p,%s)\n", this
           , socket, s2c(init_path));

   //-------------------------------------------------------------------------
   // Set transfer size -- optimization attempt (has no noticable effect)
   //-------------------------------------------------------------------------
   if( USE_RCVBUF_SIZE > 0 ) {      // If transfer size optimization
     int optval= USE_RCVBUF_SIZE;
     socket->set_option(SOL_SOCKET, SO_RCVBUF, &optval, sizeof(optval));
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       ClientThread::~ClientThread
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   ClientThread::~ClientThread( void ) // Destructor
{  if( opt_hcdm ) debugf("ClientThread(%p)~\n", this); }

//----------------------------------------------------------------------------
//
// Method-
//       ClientThread::exchange_versionID
//
// Function-
//       Exchange version identifiers.
//
//----------------------------------------------------------------------------
int                                 // TRUE if version identifiers match
   ClientThread::exchange_versionID( void ) // Exchange version identifiers
{  if( opt_hcdm )
     debugf("ClientThread(%p)::exchange_versionID\n", this);

   set_localVersionInformation();   // Initialize local version information

   PeerRequest query;
   query.oc= REQ_VERSION;
   wr_data(&query, sizeof(query));

   HOST16_t host_size= (HOST16_t)sizeof(lVersionInfo);
   wr_buff(host_size);
   wr_buff(&lVersionInfo, host_size);
   wr_buff();

   HOST16_t peer_size= 0;
   rd_buff(peer_size);
   if( host_size != peer_size ) {
     msgout("Client: exchange size mismatch: Here(%d) Peer(%d)\n"
           , host_size, peer_size);
     return false;
   }
   rd_buff(&rVersionInfo, peer_size);

   PeerResponse qresp;
   rd_data(&qresp, sizeof(qresp));

   if( strcmp(RD_VERSION, rVersionInfo.version) != 0 ) {
     msgout("Client: exchange version mismatch: Here(%s) Peer(%s)\n"
           , RD_VERSION, rVersionInfo.version);
     return false;
   }

   if( qresp.rc != RSP_YO )
     invalid_response(__LINE__, "VERSION", qresp.rc);

   set_globalVersionInformation();

   //-------------------------------------------------------------------------
   // Verify the Current Working Directory and OS
   //-------------------------------------------------------------------------
   if( !opt_unsafe ) {              // Verify CWD?
     query.oc= REQ_CWD;
     wr_data(&query, sizeof(query));
     rd_data(&qresp, sizeof(qresp));
     if( qresp.rc != RSP_YO )
       invalid_response(__LINE__, "GETCWD", qresp.rc);

     string client_cwd= get_cwd();
     string server_cwd;
     rd_data(server_cwd);
     string client_name= get_file_name(client_cwd);
     string server_name= get_file_name(server_cwd);
     if( client_name != server_name ) {
       msgout("Error: CWD file name mismatch: Here(%s) Peer(%s)\n"
              "Use -U for unsafe operation\n"
             , s2c(client_name), s2c(server_name));
       return false;
     }
   }

   return true;
}

//----------------------------------------------------------------------------
//
// Method-
//       ClientThread::install_attr
//
// Function-
//       Install attributes (change last modification time and permissions)
//
//----------------------------------------------------------------------------
bool                                // TRUE iff attributes were updated
   ClientThread::install_attr(      // Install attributes
     RdFile*           client)      // -> Client RdFile
{  if( opt_hcdm )
     debugf("ClientThread(%p)::install_attr(%s)\n", this
           , s2c(client->file_name));

   //-------------------------------------------------------------------------
   // Verify update attributes allowed
   //-------------------------------------------------------------------------
   if( BRINGUP_MODE || opt_keep )
     return false;

   //-------------------------------------------------------------------------
   // Update the attributes, already copied into client
   //-------------------------------------------------------------------------
   client->set_attr();              // Update the item's attributes
   return true;                     // Attributes updated
}

//----------------------------------------------------------------------------
//
// Method-
//       ClientThread::install_item
//
// Function-
//       Install one fifo, file, link, or path.
//
//----------------------------------------------------------------------------
int                                 // Return code, 0 expected
   ClientThread::install_item(      // Install something
     RdFile*           client,      // -> Client RdFile
     RdFile*           server)      // -> Server RdFile
{  if( opt_hcdm )
     debugf("ClientThread(%p)::install_item(%s)\n", this
           , s2c(client->file_name));

   //-------------------------------------------------------------------------
   // Update the client info
   //-------------------------------------------------------------------------
   client->desc= server->desc;
   client->file_name= server->file_name;
   client->link_name= server->link_name;

   string full_name= client->get_full_name();

   //-------------------------------------------------------------------------
   // Diagnostics
   //-------------------------------------------------------------------------
   msglog("\n");
   msglog("install_item: %s\n-----------\n", s2c(server->file_name));
   client->display("CLIENT:");
   server->display("SERVER:");

   if( BRINGUP_MODE ) {
     print_action("ignored", client, "[BRINGUP (won't install)]");
     return(RC_ERROR);
   }

   if( opt_keep ) {
     print_action("skipped", client, "[Install disallowed: -K]");
     return(RC_ERROR);
   }

   //-------------------------------------------------------------------------
   // Install the item
   //-------------------------------------------------------------------------
   int result= RC_NORM;             // Default, successful
   switch(get_file_type(server)) {  // Process by item type
     case FT_PATH: {{{{             // If it's a directory
       //---------------------------------------------------------------------
       // Install a directory
       //---------------------------------------------------------------------
       if( mkpath(full_name, S_IRWXU) == 0 ) { // Create writeable
         int rc= update_path(client);
         if( rc == RC_NORM )
           install_attr(client);
         else
           result= RC_ERROR;
       } else {                     // If mkpath failure
         msgioerr("%4d ClientThread: mkpath(%s) failure", __LINE__
                 , s2c(full_name));
         result= RC_ERROR;
       }
       break;
     }}}}

     case FT_LINK:                  // If it's a link
       //---------------------------------------------------------------------
       // Install a soft link
       //---------------------------------------------------------------------
       if( mklink(server->link_name, full_name) != 0 ) { // If symlink failure
         print_action("skipped", server, "[Cannot create link]");
         result= RC_ERROR;
         break;
       }
       client->link_name= server->link_name;
       break;

     case FT_FILE: {{{{             // If it's a file
       //---------------------------------------------------------------------
       // Request the file
       //---------------------------------------------------------------------
       PeerRequest  query;          // Server request
       PeerResponse qresp;          // Server response
       query.oc= REQ_FILE;          // Request the file
       wr_data(&query, 1);
       wr_data(server->file_name);
       rd_data(&qresp, 1);          // Get the reply
       if( qresp.rc != RSP_YO ) {   // If operation rejected
         if( qresp.rc != RSP_NO )   // If operation garbled
           invalid_response(__LINE__, "FILE", qresp.rc);

         print_action("skipped", client, "[Disallowed by SERVER]");
         result= RC_ERROR;
         break;
       }

       //---------------------------------------------------------------------
       // Open the file
       //---------------------------------------------------------------------
       fd_t fd= open(s2c(full_name), O_WRONLY|O_BINARY|O_TRUNC|O_CREAT
                    , S_IRUSR|S_IWUSR);
       if( fd < 0 ) {               // Open failed
         msgioerr("%4d ClientThread: open(%s) failure", __LINE__
                 , s2c(full_name));
         print_action("cancelled", client, "[Open failure]");
         stop();
         break;
       }

       //---------------------------------------------------------------------
       // Receive the file (using server attributes!)
       //---------------------------------------------------------------------
       Backout backout(client, fd); //
       size_t left= server->desc.file_size; // Entire file left to be sent
       while(left > 0 ) {           // More bytes need to be received
         size_t rd_size= min(left, MAX_TRANSFER);
         rd_data(buffer, rd_size);  // Read from server
         size_t wr_size= write(fd, buffer, rd_size); // Write some of the file
         if( wr_size != rd_size )   // Wrong amount written
           throwf("%4d ClientThread: %'zd=write(%s,%'zd) error",
                  __LINE__, wr_size, s2c(full_name), rd_size);

         left -= rd_size;           // Those read aren't left to read
       }                            // Done reading more bytes

       //---------------------------------------------------------------------
       // Close the file
       //---------------------------------------------------------------------
       int rc= close(fd);           // Close the file
       if( rc == 0 ) {              // If closed OK
         install_attr(client);      // Install attributes
         backout.reset();           // Cancel backout
       } else {                     // If close failure
         // Backout will remove the file
         msgioerr("%4d ClientThread: close(%s) failure", __LINE__
                 , s2c(full_name));
         result= RC_ERROR;
       }
       break;
     }}}}

     case FT_FIFO: {{{{             // If it's a pipe
       //---------------------------------------------------------------------
       // Install a pipe
       //---------------------------------------------------------------------
       client->desc.file_info= server->desc.file_info;
       int rc= mkfifo(full_name, client->get_chmod());
       if( rc == RC_NORM ) {        // If pipe created
         update_attr(client, server); // Update attributes
       } else {
         msgioerr("%4d ClientThread: mkfifo(%s) failure", __LINE__
                 , s2c(full_name));
         result= RC_ERROR;
       }
       break;
     }}}}

     default:                       // If it's of unknown type
       //---------------------------------------------------------------------
       // Install an item of unknown type
       //---------------------------------------------------------------------
       print_action("ignored", client, "[What kind of thing is it?]");
       result= RC_ERROR;
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       ClientThread::remove_item
//
// Function-
//       Delete a Client file, link or path.
//
// Implementation notes-
//       For a path, call remove_path first. (If that was done here, it would
//       add an extra stack level per subdirectory.)
//
//----------------------------------------------------------------------------
int                                 // Return code (0 expected)
   ClientThread::remove_item(       // Remove something
     RdFile*           client)      // Current client RdFile
{  if( opt_hcdm )
     debugf("ClientThread(%p)::remove_item(%s)\n", this
           , s2c(client->get_full_name()));

   //-------------------------------------------------------------------------
   // Get the fully qualified file name
   //-------------------------------------------------------------------------
   string full_name= client->get_full_name();

   //-------------------------------------------------------------------------
   // Verify that we're not trying to remove a "!const" file
   //-------------------------------------------------------------------------
   if( client->file_name == const_file_name )
     attempted_const_modify(full_name);

   //-------------------------------------------------------------------------
   // Diagnostics
   //-------------------------------------------------------------------------
   msglog("\n");
   msglog("remove_item: %s\n-----------\n", s2c(full_name));
   client->display("CLIENT:");

   if( BRINGUP_MODE ) {
     print_action("kept", client, "[BRINGUP (won't remove)]");
     return(RC_ERROR);
   }

   if( opt_keep ) {
     print_action("kept", client, "[Remove disallowed: -K]");
     return(RC_ERROR);
   }

   //-------------------------------------------------------------------------
   // Remove the item
   //-------------------------------------------------------------------------
   switch(get_file_type(client)) {  // Process by item type
     case FT_PATH:                  // If it's a directory
       //---------------------------------------------------------------------
       // Remove a directory
       //---------------------------------------------------------------------
       if( rmpath(full_name) != 0 ) { // If remove directory fails
         msgioerr("%4d ClientThread: rmpath(%s) failure", __LINE__
                 , s2c(full_name));
         return(RC_ERROR);
       }
       break;

     case FT_LINK:                  // If it's a link
       //---------------------------------------------------------------------
       // Remove a soft link
       //---------------------------------------------------------------------
       if( rmlink(full_name) != 0 ) { // Remove link failed
         msgioerr("%4d ClientThread: rmlink(%s) failure", __LINE__
                 , s2c(full_name));
         return(RC_ERROR);
       }
       break;

     case FT_FILE:                  // If it's a file
     case FT_FIFO:                  // If it's a pipe
       //---------------------------------------------------------------------
       // Remove a file or pipe
       //---------------------------------------------------------------------
       chmod(full_name, client->get_chmod()|S_IWUSR); // Make writable
       if( rmfile(full_name) != 0 ) { // Remove file failed
         msgioerr("%4d ClientThread: rmfile(%s) failure", __LINE__
                 , s2c(full_name));
         return(RC_ERROR);
       }
       break;

     default:                       // If it's of unknown type
       //---------------------------------------------------------------------
       // Remove an item of unknown type
       //---------------------------------------------------------------------
       print_action("ignored", client, "[What kind of thing is it?]");
       return(RC_ERROR);
   }

   return(RC_NORM);
}

//----------------------------------------------------------------------------
//
// Method-
//       ClientThread::remove_path
//
// Function-
//       Remove all subtree content
//
//----------------------------------------------------------------------------
void
   ClientThread::remove_path(       // Remove all subtree content
     RdFile*           client)      // -> Client Path RdFile descriptor
{  if( opt_hcdm )
     debugf("ClientThread(%p)::remove_path(%s)\n", this
           , s2c(client->get_full_name()));

   //-------------------------------------------------------------------------
   // Get the fully qualified file name
   //-------------------------------------------------------------------------
   string full_name= client->get_full_name();
   const char* full_char= s2c(full_name);

   //-------------------------------------------------------------------------
   // Diagnostics
   //-------------------------------------------------------------------------
   msglog("\n");
   msglog("removePath: %s\n-----------\n", full_char);
   client->display("CLIENT:");

   if( BRINGUP_MODE ) {
     print_action("kept", client, "[BRINGUP (won't rmpath)]");
     return;
   }

   if( opt_keep ) {
     print_action("kept", client, "[Remove path disallowed: -K]");
     return;
   }

   //-------------------------------------------------------------------------
   // Switch into new subdirectory
   //-------------------------------------------------------------------------
   if( (client->desc.file_info&INFO_RUSR) == 0 // Don't have permission to read
       || (client->desc.file_info&INFO_WUSR) == 0 // or can't write in it
       || (client->desc.file_info&INFO_XUSR) == 0 ) { // or can't change to it
     int rc= chmod(full_char, client->get_chmod()|(S_IRUSR|S_IWUSR|S_IXUSR));
     if( rc != 0 )                  // Couldn't give self permissions
       throwf("%4d ClientThread: chmod(%s) failure", __LINE__, full_char);
   }

   //-------------------------------------------------------------------------
   // Delete all items in the subdirectory, recursively
   //-------------------------------------------------------------------------
   RdPath  path= RdPath(this, full_name); // Read/sort the delete directory
   RdFile* file= path.get_head();   // Address the first element
   while( file != nullptr ) {       // For each item in the directory
     if( get_file_type(file) == FT_PATH ) // If a subdirectory
       remove_path(file);           // Remove it first

     remove_item(file);
     file= file->get_next();
   }

   //-------------------------------------------------------------------------
   // Restore permissions
   //-------------------------------------------------------------------------
   if( (client->desc.file_info&INFO_RUSR) == 0 // Didn't have permission to read
       || (client->desc.file_info&INFO_WUSR) == 0 // or to write in it
       || (client->desc.file_info&INFO_XUSR) == 0 ) { // or to change to it
     int rc= chmod(full_name, client->get_chmod()); // Restore permissions
     if( rc != 0 )
       throwf("%4d ClientThread: chmod(%s) restore failure", __LINE__
             , s2c(full_name));
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       ClientThread::run
//
// Purpose-
//       Operate the ClientThread.
//
//----------------------------------------------------------------------------
void
   ClientThread::run( void )        // Operate this ClientThread
{  if( opt_hcdm ) debugf("ClientThread(%p)::run...\n", this);

   // Thread initialization
   msgout("Client: Started...\n");
   fsm= FSM_READY;                  // Indicate operational

   if( HCDM )
     opt_hcdm= true;
   if( VERBOSE > opt_verbose )
     opt_verbose= VERBOSE;

   // Pseudo-thread operation
   try {
     if( exchange_versionID() ) {   // If version is valid
       RdPath path(this);
       path.path_name= ".";

       RdFile file(&path, init_path);
       update_path(&file);          // Install initial subdirectory
     }

     PeerRequest  query;            // RdServer request
     PeerResponse qresp;            // RdServer response
     query.oc= REQ_QUIT;
     wr_data(&query, sizeof(query));
     rd_data(&qresp, sizeof(qresp));

     // Normal termination
     msgout("...Client: Complete\n");
     fsm= FSM_CLOSE;
   } catch( std::exception& X ) {
     msgerr("Client: exception(%s)\n", X.what());
   } catch( const char* X ) {
     msgerr("Client: exception(%s)\n", X);
   } catch(...) {
     msgerr("Client: exception(%s)\n", "...");
   }

   // Thread termination
   if( fsm == FSM_READY )           // If forced termination
     msgout("Client: Terminated\n");

   if( opt_hcdm ) debugf("...ClientThread(%p)::run()\n", this);
}

//----------------------------------------------------------------------------
//
// Method-
//       ClientThread::stop
//
// Purpose-
//       Stop the client (Early termination)
//
//----------------------------------------------------------------------------
[[noreturn]]
void
   ClientThread::stop( void )       // Stop the Client (Early termination)
{  if( opt_hcdm ) debugf("ClientThread::stop\n");

   fprintf(stderr, "(Cannot continue)\n");
   rdterm();                        // Terminate and
   exit(1);                         // Exit
}

//----------------------------------------------------------------------------
//
// Method-
//       ClientThread::update_attr
//
// Function-
//       Update item attributes with server attributes.
//
// Implementation notes-
//       An error message is written when attribute updating fails.
//
//----------------------------------------------------------------------------
bool                                // TRUE iff attributes were updated
   ClientThread::update_attr(       // Update attributes
     RdFile*           client,      // -> Client RdFile
     RdFile*           server)      // -> Server RdFile
{  if( opt_hcdm )
     debugf("ClientThread(%p)::update_attr(%s,%s)\n", this
           , s2c(client->file_name), s2c(server->file_name));

   //-------------------------------------------------------------------------
   // Diagnostics
   //-------------------------------------------------------------------------
   msglog("\n");
   msglog("update_attr: %s\n-----------\n", s2c(client->file_name));

   client->display("CLIENT:");
   server->display("SERVER:");

   int client_ft= get_file_type(client);
   int server_ft= get_file_type(server);
   if( client_ft != server_ft ) {      // (Occurs only if code logic error)
     msgerr("-----: Why do file types differ? client(%c) server(%c)\n"
           , client_ft, server_ft);
     return false;
   }

   if( client->compare_info(server) ) // If attributes unchanged
     return false;                  // (Nothing to update)

   if( BRINGUP_MODE ) {
     print_action("ignored", client, "[BRINGUP]");
     return false;
   }

   if( opt_keep ) {
     print_action("skipped", client, "[Update attr disallowed: -K]");
     return false;
   }

   //-------------------------------------------------------------------------
   // We don't update Link attributes!
   //-------------------------------------------------------------------------
   if( server_ft == FT_LINK )
     return false;

   //-------------------------------------------------------------------------
   // Update the attributes
   //-------------------------------------------------------------------------
   client->desc.file_size= server->desc.file_size;
   client->desc.file_time= server->desc.file_time;
   client->desc.file_info= server->desc.file_info;
   client->desc.file_ksum= server->desc.file_ksum;
   client->set_attr();
   return true;
}

//----------------------------------------------------------------------------
//
// Method-
//       ClientThread::update_item
//
// Function-
//       Update a file, link or directory.
//
//----------------------------------------------------------------------------
int                                 // Return code (RC_NORM expected)
   ClientThread::update_item(       // Update something
     RdFile*           client,      // -> Client RdFile
     RdFile*           server)      // -> Server RdFile
{  if( opt_hcdm )
     debugf("ClientThread(%p)::update_item(%s,%s)\n", this
           , s2c(client->file_name), s2c(server->file_name));

   //-------------------------------------------------------------------------
   // Diagnostics
   //-------------------------------------------------------------------------
   msglog("\n");
   msglog("update_item: %s\n-----------\n", s2c(server->file_name));
   client->display("CLIENT:");
   server->display("SERVER:");

   if( BRINGUP_MODE ) {
     print_action("kept", client, "[BRINGUP (won't update)]");
     return(RC_ERROR);
   }

   if( opt_keep ) {
     print_action("skipped", client, "[Update disallowed: -K]");
     return(RC_ERROR);
   }

   //-------------------------------------------------------------------------
   // Update the item
   //-------------------------------------------------------------------------
   int returncd= RC_NORM;           // Default, normal return code
   switch(get_file_type(client)) {  // Process by item type
     case FT_FIFO:                  // If pipe
       ;                            // No function required
       break;

     case FT_PATH:                  // If directory
       returncd= update_path(client);
       if( returncd == RC_NORM )
         install_attr(client);
       break;

     case FT_LINK:                  // If soft link
       returncd= remove_item(client);
       if( returncd == RC_NORM )
         returncd= install_item(client, server);
       break;

     case FT_FILE:                  // If file
       returncd= remove_item(client);
       if( returncd == RC_NORM )
         returncd= install_item(client, server);
       break;

     default:                       // If unknown type
       returncd= RC_ERROR;          // Cannot update it
       break;
   }

   return(returncd);
}

//----------------------------------------------------------------------------
//
// Method-
//       ClientThread::update_path
//
// Function-
//       Update new and changed files, links and directories within a
//       a directory subtree.
//
//----------------------------------------------------------------------------
int                                 // Return code (RC_NORM expected)
   ClientThread::update_path(       // Update path
     const RdFile*     client)      // The Client path RdFile
{  if( opt_hcdm )
     debugf("ClientThread(%p)::update_path(%s)\n", this
           , s2c(client->get_full_name()));

   //-------------------------------------------------------------------------
   // Initialization
   //-------------------------------------------------------------------------
   int once= false;                 // Subroutine print_path once control
   string file_name= client->get_file_name();
   string full_name= client->get_full_name();
   if( file_name == "." )
     full_name= client->path->path_name;

   msglog("ClientThread: update_path(%s)\n", s2c(full_name));

   RdPath* client_path= new RdPath(this, full_name);
   std::unique_ptr<RdPath> unique_client(client_path);
   RdFile* client_file= client_path->get_head();
   RdPath::push(client_path);

   PeerRequest  query;              // Server request
   PeerResponse qresp;              // Reply from server
   query.oc= REQ_GOTO;              // Request the subdirectory
   wr_data(&query, 1);
   wr_data(file_name);
   rd_data(&qresp, 1);
   if( qresp.rc != RSP_YO ) {
     if( qresp.rc != RSP_NO )
       invalid_response(__LINE__, "GOTO", qresp.rc);

     print_path(once, full_name);
     print_action("skipped", client, "[Disallowed by SERVER]");
     return RC_ERROR;
   }

   //-------------------------------------------------------------------------
   // Load the remote directory contents
   //-------------------------------------------------------------------------
   RdPath* server_path= rd_path(client);
   std::unique_ptr<RdPath> unique_server(server_path);
   RdFile* server_file= server_path->get_head();

   //-------------------------------------------------------------------------
   // Diagnostics
   //-------------------------------------------------------------------------
   if( opt_hcdm && opt_verbose > 2 ) {
     debugf("\nupdate_path\n");
     client_path->debug("client");
     debugf("\n");
     server_path->debug("server");
   }

   //-------------------------------------------------------------------------
   // Install/remove/update items in this subdirectory
   //-------------------------------------------------------------------------
   for(;;) {                        // Process this directory
     // Logging diagnostics
     msglog("\n");
     if( client_file == nullptr )
       msglog("CLIENT: nullptr\n");
     else
       client_file->display("CLIENT:");

     if( server_file == nullptr )
       msglog("SERVER: nullptr\n");
     else
       server_file->display("SERVER:");

     if( client_file == nullptr && server_file == nullptr ) // If complete
       break;

     //-----------------------------------------------------------------------
     // Determine relative positions of items
     //-----------------------------------------------------------------------
     int ac= AC_NOP;                // Default, no postprocessing
     int cc= 0;                     // Name compare code
     if( client_file == nullptr )   // If target end of file
       cc= (+1);                    // Target name > Source name
     else if( server_file == nullptr ) // If source end of file
       cc= (-1);                    // Target name < Source name
     else                           // If within both directories
       cc= client_file->file_name.compare(server_file->file_name);

     //-----------------------------------------------------------------------
     // An item exists remotely but not locally.  Install it.
     //-----------------------------------------------------------------------
     if( cc > 0 ) {                 // If we are missing an item
       msglog("ACTION: install\n");

       ac= AC_GETSERVER;
       RdFile* insert_file= new RdFile(client_path, server_file->file_name);
       print_path(once, full_name);
       int rc= install_item(insert_file, server_file);
       if( rc == RC_NORM ) {
         print_action("installed", insert_file, "");
         client_path->lifo(insert_file); // Insert BEFORE the client_file
       } else {
         delete insert_file;        // Discard allocated element
       }
       goto deferred_action;
     }

     //-----------------------------------------------------------------------
     // Disallow possible "!const" file modification
     //-----------------------------------------------------------------------
     if( compare(client_file->file_name, const_file_name) == 0 ) {
       bool files_differ= false;    // Default: Files do not differ

       if( server_file == nullptr ) // (If Client file would be deleted)
         files_differ= true;        // Server does not contain file

       if( get_file_type(server_file) != get_file_type(client_file)
           || server_file->file_name != client_file->file_name )
         files_differ= true;        // Types or exact names differ

       if( server_file->desc.file_size != client_file->desc.file_size
           || server_file->desc.file_ksum != client_file->desc.file_ksum
           || server_file->desc.file_time != client_file->desc.file_time )
         files_differ= true;        // Contents or times differ

       if( client_file->desc.file_info != server_file->desc.file_info )
         files_differ= true;        // Attributes differ

       if( files_differ )
         attempted_const_modify(full_name);
     }

     //-----------------------------------------------------------------------
     // An item exists locally but not remotely. Remove it.
     //-----------------------------------------------------------------------
     if( cc < 0 ) {
       msglog("ACTION: remove\n");
       if( opt_erase ) {
         print_path(once, full_name);
         if( get_file_type(client_file) == FT_PATH ) // If a path
           remove_path(client_file); // Remove the subtree
         int rc= remove_item(client_file); // Remove the item itself
         if( rc == RC_NORM )
           print_action("removed", client_file, "");
         else
           print_action("kept", client_file, "[unable to remove]");
       } else {                     // If removal not allowed
         print_path(once, full_name); // Informational
         print_action("kept", client_file, "[-E parameter not specified]");
       }

       // Remove the entry from the list, then delete it
       client_file= client_path->remove_and_delete(client_file);
       continue;
     }

     //-----------------------------------------------------------------------
     // Check for ambiguous update.
     //-----------------------------------------------------------------------
     msglog("ACTION: equals\n");
     if( (gVersionInfo.f[0]&VersionInfo::VIF0_CASE) == 0 // If case insensitive
         && (lVersionInfo.f[0]&VersionInfo::VIF0_CASE) !=
            (rVersionInfo.f[0]&VersionInfo::VIF0_CASE) ) { // And different
       if( client_file->file_name != server_file->file_name ) { // If inexact
         // Remove any duplicate ambiguous entries
         if( (lVersionInfo.f[0]&VersionInfo::VIF0_CASE) != 0 ) {
           // Local machine is case sensitive, remove duplicate entries
           while( client_file->get_next() != nullptr
                  && case_comp(client_file->file_name,
                               client_file->get_next()->file_name) == 0 ) {
             print_path(once, full_name); // Informational
             print_action("skipped", client_file, "[ambiguous]");
             msglog("Skipped ambiguous Client file(%s)\n"
                   , s2c(client_file->file_name));

             // Remove case insensitive entry from the list and delete it
             client_file= client_path->remove_and_delete(client_file);
           }
         } else {
           // Remote machine is case sensitive, remove duplicate entries
           while( server_file->get_next() != nullptr
                  && case_comp(server_file->file_name,
                               server_file->get_next()->file_name) == 0 ) {
             print_path(once, full_name); // Informational
             print_action("skipped", server_file, "[ambiguous]");

             // Remove case insensitive entry from the list and delete it
             server_file= server_path->remove_and_delete(server_file);
             msglog("Skipped ambiguous Server file(%s)\n"
                   , s2c(server_file->file_name));
           }
         }
       }
     }

     //-----------------------------------------------------------------------
     // An identically named item is of differing type,
     // or the file names are identical except for case.
     //
     // The item must be removed before it can be installed.
     //-----------------------------------------------------------------------
     if( get_file_type(server_file) != get_file_type(client_file)
         || server_file->file_name != client_file->file_name ) {
       msglog("UPDATE: type or exact name mismatch\n");

       print_path(once, full_name); // Informational
       if( !opt_erase ) {           // If erasure not allowed
         print_action("kept", client_file, "[-E parameter not specified]");
         if( get_file_type(server_file) != get_file_type(client_file) )
           print_action("remote", server_file, "[type differs]");
         else
           print_action("remote", server_file, "[name differs]");

         client_file->desc.file_info &= ~(INFO_ISTYPE); // Prevent subdirectory scan
       } else {                     // If erasure allowed
         if( get_file_type(client_file) == FT_PATH ) // If a path
           remove_path(client_file); // Remove the subtree

         int rc= remove_item(client_file); // Remove the item itself
         if( rc == RC_NORM )
           print_action("removed", client_file, "");

         // Set updated attributes
         rc= install_item(client_file, server_file);
         if( rc == RC_NORM ) {
           print_action("installed", server_file, "");
         } else {
           // Remove the entry from the list, then delete it
           client_file= client_path->remove_and_delete(client_file);
           continue;
         }
       }

       ac= AC_BOTH;
       goto deferred_action;
     }

     //-----------------------------------------------------------------------
     // An identically named and typed item exists.
     //-----------------------------------------------------------------------
     msglog("UPDATE: name and type identical\n");
     ac= AC_BOTH;
     switch(get_file_type(client_file)) {
       case FT_PATH: {{{{           // If directory
         int rc= update_path(client_file);
         if( rc == RC_NORM )
           install_attr(client_file);
         break;
       }}}}

       case FT_LINK: {{{{           // If soft link
         if( client_file->link_name != server_file->link_name ) {
           print_path(once, full_name);
           int rc= update_item(client_file, server_file);
           if( rc == RC_NORM )
             print_action("updated", client_file, "");
         }
         break;
       }}}}

       case FT_FILE: {{{{           // If file
         //-------------------------------------------------------------------
         // Identical file names exist at both sites
         //-------------------------------------------------------------------
         if( server_file->desc.file_size == client_file->desc.file_size
             && server_file->desc.file_ksum == client_file->desc.file_ksum
             && server_file->compare_time(client_file) == 0 ) {
           if( client_file->compare_info(server_file) != 0 ) {
             print_path(once, full_name);
             update_attr(client_file, server_file);
             if( !opt_keep )
               print_action("attributes", client_file, "");
           }
           break;
         }

         //-------------------------------------------------------------------
         // Check whether the file is newer here (and we care)
         //-------------------------------------------------------------------
         if( server_file->compare_time(client_file) < 0 && opt_older == false ) {
           print_path(once, full_name);
           print_action("kept", client_file, "[-O parameter not specified]");
           break;
         }

         //-------------------------------------------------------------------
         // The file needs to be replaced.
         //-------------------------------------------------------------------
         print_path(once, full_name);
         if( update_item(client_file, server_file) == RC_NORM )
           print_action("updated", server_file, "");
         break;
       }}}}

       case FT_FIFO:                // If pipe
         if( (server_file->desc.file_info&INFO_PERMITS)
             != (client_file->desc.file_info&INFO_PERMITS)
             ||server_file->desc.file_time != client_file->desc.file_time ) {
           print_path(once, full_name);
           update_attr(client_file, server_file);
           if( !opt_keep )
             print_action("attributes", client_file, "");
         }
         break;

       default:                     // If unknown type
         break;
     }

     //-----------------------------------------------------------------------
     // Process deferred action code.
     //-----------------------------------------------------------------------
deferred_action:
     switch(ac) {                   // Process action code
       case AC_NOP:                 // No action
         break;

       case AC_GETSERVER:           // Get next SERVER item
         server_file= server_file->get_next();
         break;

       case AC_GETCLIENT:           // Get next CLIENT item
         client_file= client_file->get_next();
         break;

       case AC_BOTH:                // Get next item
         // If the local machine is case sensitive and the remote is not,
         // we must skip duplicate local items
         if( (lVersionInfo.f[0]&VersionInfo::VIF0_CASE) != 0
             && (rVersionInfo.f[0]&VersionInfo::VIF0_CASE) == 0 ) {
           while( client_file->get_next() != nullptr
               && case_comp(client_file->file_name,
                            client_file->get_next()->file_name) == 0 ) {
             client_file= client_file->get_next();
             print_path(once, full_name);
             print_action("skipped", client_file, "[ambiguous]");
           }
         }

         // If the remote machine is case sensitive and the local is not,
         // we must skip duplicate remote items
         if( (lVersionInfo.f[0]&VersionInfo::VIF0_CASE) == 0
             && (rVersionInfo.f[0]&VersionInfo::VIF0_CASE) != 0 ) {
           while( server_file->get_next() != nullptr
               && case_comp(server_file->file_name,
                            server_file->get_next()->file_name) == 0 ) {
             server_file= server_file->get_next();
             print_path(once, full_name);
             print_action("skipped", server_file, "[ambiguous]");
           }
         }

         client_file= client_file->get_next();
         server_file= server_file->get_next();
         break;

       default:                     // If unknown code
         throwf("%4d ClientThread: Action code(%d)", __LINE__, ac);
         break;
     }
   }

   //-------------------------------------------------------------------------
   // Complete current directory processing
   //-------------------------------------------------------------------------
   query.oc= REQ_QUIT;
   wr_data(&query, 1);
   rd_data(&qresp, 1);
   if( qresp.rc != RSP_YO )
     invalid_response(__LINE__, "QUIT", qresp.rc);

   RdPath::pop();
   msglog("%4d ClientThread: update_path(%s) complete\n", __LINE__
         , s2c(full_name));
   return RC_NORM;
}
