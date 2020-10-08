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
//       ServerThread.cpp
//
// Purpose-
//       Implement ServerThread object methods
//
// Last change date-
//       2020/10/03
//
// Implementation notes-
//       This multi-threaded server DOES NOT change path or file permissions
//       during transfer. (A second thread might use the modified permissions,
//       making the change permanent.)
//
//----------------------------------------------------------------------------
#include <exception>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>               // For S_IREAD ...

#include <com/Atomic.h>
#include <com/Barrier.h>
#include <com/Debug.h>
#include <com/define.h>             // For NULL

#include "ocrw.h"
#include "RdCommon.h"

#include "ServerThread.h"

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
//
// Subroutine-
//       invalidRequest
//
// Function-
//       An invalid resquest was received from the client
//
//----------------------------------------------------------------------------
static void                         // (Does not return)
   invalidRequest(                  // Handle invalid request
     int               lineno,      // Calling line number
     int               op)          // The invalid request
{
   throwf("%4d ServerThread: Why did Client ask '%c' (%d)?", lineno, op, op);
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
{
   IFSCDM( debugf("%4d ServerThread(%p)::~ServerThread()\n", __LINE__, this); )
}

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
     const char*       path)        // Initial directory (in ListenThread)
:  CommonThread(socket)
,  path(path)
{
   IFSCDM(
     debugf("%4d ServerThread(%p)::ServerThread(%p,%s)\n", __LINE__, this,
            socket, path);
   )

   // Run the Thread
   start();
}

//----------------------------------------------------------------------------
//
// Method-
//       ServerThread::exchangeVersionID
//
// Function-
//       Exchange version identifiers.
//
//----------------------------------------------------------------------------
int                                 // TRUE if version identifiers match
   ServerThread::exchangeVersionID( void ) // Exchange version identifiers
{
   int                 L;           // Request length
   struct
   {
     VersionInfo       info;        // Input version info
     char              pad[16];     // Pad
   }                   inpVersion;  // Version info

   //-------------------------------------------------------------------------
   // Exchange version identifiers
   //-------------------------------------------------------------------------
   localVersionInformation();       // Initialize local version information
   if( sw_verify )                  // If verify switch
     lVersionInfo.f[7] |= VersionInfo::VIF7_KSUM;// Indicate checksum switch

   memset(&inpVersion, 0, sizeof(inpVersion));
   L= nRecvString(&inpVersion, sizeof(inpVersion));
   nSendString(&lVersionInfo, sizeof(lVersionInfo));
   if( strcmp(inpVersion.info.version, RD_VERSION) != 0 )
   {
     msgout("%4d Server: Version mismatch: Here(%s) Peer(%s)\n",
            __LINE__, RD_VERSION, inpVersion.info.version);
     return FALSE;
   }

   if( L != sizeof(VersionInfo) )
   {
     msgout("%4d Server: Version length: Got(%d) Expected(%ld)\n",
            __LINE__, L, (long)sizeof(VersionInfo));
     return FALSE;
   }
   memcpy(&rVersionInfo, &inpVersion.info, sizeof(rVersionInfo));
   globalVersionInformation();

   return TRUE;
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
long                                // Return code (always 0)
   ServerThread::run( void )        // Operate this ServerThread
{
   IFSCDM( debugf("%4d ServerThread(%p)::run()...\n", __LINE__, this); )

   // Connected message
   if( strlen(path) >= (MAX_DIRPATH-1) )
     throwf("Path(%s) name too long", path);

   const char* peerName= socket->getPeerName();
   if( peerName == NULL )
     peerName= Socket::addrToChar(socket->getPeerAddr());
   msgout("Server: Connected... Host(%s:%d)\n",
          peerName, socket->getPeerPort());

   // Handle client request messages
   msglog("ServerThread(%s)\n", path);
   fsm= FSM_READY;                  // Indicate operational
   try {
     serve();                       // Process server requests
     fsm= FSM_CLOSE;                // Normal termination
     msgout("Server: ...Completed Host(%s:%d)\n",
            peerName, socket->getPeerPort());
     sleep(1.5);                    // Allow time for message transfer
   } catch( const char* X ) {
     fprintf(stderr, "Server: exception(%s)\n", X);
              msglog("Server: exception(%s)\n", X);
   } catch( std::exception& X ) {
     fprintf(stderr, "Server: exception(%s)\n", X.what());
              msglog("Server: exception(%s)\n", X.what());
   } catch(...) {
     fprintf(stderr, "Server: exception(%s)\n", "...");
              msglog("Server: exception(%s)\n", "...");
   }

   // Thread termination
   term();                          // Indicate terminated

   IFSCDM( debugf("%4d ...ServerThread(%p)::run()\n", __LINE__, this); )
   return 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       ServerThread::serve
//
// Purpose-
//       Process server requests (initial directory)
//
//----------------------------------------------------------------------------
void
   ServerThread::serve( void )      // Process server requests
{
   int                 isValid;     // TRUE if version has been validated
   DirEntry            dirEntry(this); // Working DirEntry
   PeerRequest         query;       // Question from client
   PeerResponse        qresp;       // Reply to client

   isValid= FALSE;                  // Default, invalid
   while( fsm == FSM_READY )        // Process initial server requests
   {
     nRecv(&query, 1);              // Read question from client
     qresp.rc= RSP_YO;              // Default, operation accepted

     switch(query.oc)               // Process request
     {
       case REQ_GOTO:               // Goto subdirectory
         //-------------------------------------------------------------------
         // Install subdirectory
         //-------------------------------------------------------------------
         if( !isValid )
         {
           msgout("%4d Server not validated\n", __LINE__);
           qresp.rc= RSP_NO;
           nSend(&qresp, 1);
           break;
         }

         nRecvString(dirEntry.fileName, sizeof(dirEntry.fileName));
         dirEntry.list= new DirList(this, path, &dirEntry);

         // Install the subdirectory
         nSend(&qresp, 1);          // The operation is accepted
         serveDirectory(path, &dirEntry);
         isValid= FALSE;
         break;

       case REQ_VERSION:            // Exchange version identifiers
         isValid= exchangeVersionID();

         if( !isValid )
           qresp.rc= RSP_NO;
         nSend(&qresp, 1);
         break;

       case REQ_CWD:                // Retrieve CWD
         nSend(&qresp, 1);
         nSendString(path, strlen(path));
         break;

       case REQ_QUIT:               // Exit
         //-------------------------------------------------------------------
         // Exit
         //-------------------------------------------------------------------
         nSend(&qresp, 1);          // The operation is accepted
         return;

       default:                     // Error, invalid question
         invalidRequest(__LINE__, query.oc);
         break;
     }
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       ServerThread::serveDirectory
//
// Function-
//       Install a directory subtree.
//
//----------------------------------------------------------------------------
void
   ServerThread::serveDirectory(    // Serve directory subtree
     const char*       path,        // Path to current directory
     DirEntry*         dirEntry)    // The directory entry
{
   DirEntry*           ptrE;        // -> DirEntry
   DirList*            ptrL;        // -> DirList
   char                newPath[MAX_DIRFILE+1]; // Nested path name
   char                fileName[MAX_DIRNAME+1]; // Current file name

   PeerRequest         query;       // Question from client
   PeerResponse        qresp;       // Reply to client

   //-------------------------------------------------------------------------
   // Load the directory
   //-------------------------------------------------------------------------
   msglog("serveDirectory(%s,%s)..\n", path, dirEntry->fileName);
   makeFileName(newPath, path, dirEntry->fileName);

   //-------------------------------------------------------------------------
   // Reply with directory information
   //-------------------------------------------------------------------------
   ptrL= dirEntry->list;            // Get the associated DirList
   #ifdef USE_ASYNCHRONOUS_LOADER
     ptrL->start();                 // Start the loader process
     nSendDirectory(ptrL);
   #else
     nSendDirectory(ptrL);
     ptrL->runLoader();
   #endif

   //-------------------------------------------------------------------------
   // Install the directory
   //-------------------------------------------------------------------------
   while( fsm == FSM_READY )        // Process this directory
   {
     nRecv(&query, 1);              // Read question from client
     qresp.rc= RSP_YO;              // Default, operation accepted

     switch(query.oc)               // Process request
     {
       case REQ_FILE:               // Install file
         //-------------------------------------------------------------------
         // Install file
         //-------------------------------------------------------------------
         nRecvString(fileName, MAX_DIRNAME+1); // Read filename
         ptrE= ptrL->locate(fileName);
         if( verifyType(ptrE, FT_FILE) != 0 )
           break;

         #ifdef USE_CHECK_PERMISSIONS
           // Verify that we have permission to read this file
           if( (ptrE->fileInfo&INFO_RUSR) == 0 )
           {
             qresp.rc= RSP_NO;      // Reject, not permitted
             nSend(&qresp, 1);
             break;
           }
         #endif

         serveFile(newPath, ptrE);
         break;

       case REQ_GOTO:               // Goto subdirectory
         //-------------------------------------------------------------------
         // Install subdirectory
         //-------------------------------------------------------------------
         nRecvString(fileName, MAX_DIRNAME+1); // Read directory name
         ptrE= ptrL->locate(fileName);
         if( verifyType(ptrE, FT_PATH) != 0 )
           break;

         #ifdef USE_CHECK_PERMISSIONS
           // Verify that we have permission to read into this directory
           if( (ptrE->fileInfo&INFO_RUSR) == 0
               ||(ptrE->fileInfo&INFO_XUSR) == 0 )
           {
             qresp.rc= RSP_NO;      // Reject, not permitted
             nSend(&qresp, 1);
             break;
           }
         #endif

         // Install the new subdirectory
         nSend(&qresp, 1);          // The operation is accepted
         #ifdef USE_ASYNCHRONOUS_LOADER
           ptrL->wait();            // Make sure loading has completed
         #endif
         serveDirectory(newPath, ptrE);
         break;

       case REQ_QUIT:               // Exit
         //-------------------------------------------------------------------
         // Exit (back to previous directory)
         //-------------------------------------------------------------------
         nSend(&qresp, 1);          // The operation is accepted
         #ifdef USE_EARLY_CLEANUP   // If early cleanup
           delete ptrL;
           dirEntry->list= NULL;
         #endif
         msglog("..serveDirectory(%s)\n", newPath);
         return;

       default:                     // Error, invalid question
         invalidRequest(__LINE__, query.oc);
         break;
     }
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       ServerThread::serveFile
//
// Function-
//       Return a file to the client.
//
//----------------------------------------------------------------------------
void
   ServerThread::serveFile(         // Install a file
     const char*       path,        // Current Path
     DirEntry*         ptrE)        // -> DirEntry
{
   PeerResponse        qresp;       // Reply to client

   char                fileName[MAX_DIRFILE+1]; // Current file name
   int                 hand;        // Input (changed) file handle
   off64_t             left;        // Bytes of file left to send
   int                 rlen;        // Number of bytes read

   //-------------------------------------------------------------------------
   // Open the file
   //-------------------------------------------------------------------------
   msglog("serveFile(%s,%s)\n", path, ptrE->fileName);
   makeFileName(fileName, path, ptrE->fileName);
   hand= open64(fileName,O_RDONLY | O_RSHARE | O_BINARY);
   if( hand < 0 )                   // If open failed
   {
     msgerr("%4d Server: open64(%s) failure", __LINE__, fileName);

     qresp.rc= RSP_NO;              // Reject the request
     nSend(&qresp, 1);
     return;
   }

   //-------------------------------------------------------------------------
   // Accept the request
   //-------------------------------------------------------------------------
   qresp.rc= RSP_YO;                // Default, request accepted
   nSend(&qresp, 1);                // Accept the request

   //-------------------------------------------------------------------------
   // Send the file
   //-------------------------------------------------------------------------
   left= ptrE->fileSize;            // Entire file left to be sent
   while( left > 0 )                // More bytes need to be sent
   {
     rlen= read(hand, buffer, min(left,MAX_TRANSFER)); // Read the file
     if( rlen < 0 )
       throwf("%4d Server: read(%s) I/O error", __LINE__, fileName);

     if( rlen < 1 )
       throwf("%4d Server: read(%s) unexpected end of file",
              __LINE__, fileName);

     nSendStruct(buffer,rlen);      // Send some of the file
     left -= rlen;                  // Those sent aren't left to send
   }

   //-------------------------------------------------------------------------
   // Close the file
   //-------------------------------------------------------------------------
   if( close(hand) != 0 )            // Close data file failed
     throwf("%4d Server: close(%s) failure", __LINE__, fileName);
}

//----------------------------------------------------------------------------
//
// Method-
//       ServerThread::term
//
// Purpose-
//       Terminate this ServerThread.
//
//----------------------------------------------------------------------------
void
   ServerThread::term( void )       // Terminate this ServerThread
{
   IFSCDM( debugf("%4d ServerThread(%p)::term()\n", __LINE__, this); )

   if( fsm == FSM_READY )           // If forced termination
   {
     const char* peerName= socket->getPeerName();
     if( peerName == NULL )
       peerName= Socket::addrToChar(socket->getPeerAddr());
     msgout("Server: ...Cancelled Host(%s:%d)\n",
            peerName, socket->getPeerPort());
   }

   CommonThread::term();
}

//----------------------------------------------------------------------------
//
// Method-
//       ServerThread::verifyType
//
// Function-
//       Verify that an item is of the appropriate type
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   ServerThread::verifyType(        // Verify item type
     DirEntry*         ptrE,        // -> DirEntry
     int               type)        // Expected type
{
   PeerResponse        qresp;       // Reply to client

   if( ptrE != NULL && getFileType(ptrE->fileInfo) == type )
     return 0;

   qresp.rc= RSP_NO;
   nSend(&qresp, sizeof(qresp));
   return 1;
}

