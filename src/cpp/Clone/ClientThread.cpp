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
//       ClientThread.cpp
//
// Purpose-
//       Implement ClientThread object methods
//
// Last change date-
//       2020/10/03
//
//----------------------------------------------------------------------------
#include <exception>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>               // For S_IREAD ...

#include <com/Debug.h>
#include <com/define.h>             // For NULL
#include <com/istring.h>            // For stricmp

#include "ocrw.h"
#include "RdCommon.h"

#include "ClientThread.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

//----------------------------------------------------------------------------
// Dependent macros
//----------------------------------------------------------------------------
#include <com/ifmacro.h>

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define AC_NOP                    0 // No action
#define AC_GETSERVER              1 // Next SERVER, Keep CLIENT
#define AC_GETCLIENT              2 // Keep SERVER, Next CLIENT
#define AC_BOTH                   3 // Get next (CLIENT and SERVER)

// Standard return codes
#define RC_NORM                   0 // Normal (No error)
#define RC_ERROR                  1 // Error

//----------------------------------------------------------------------------
// Constant data areas
//----------------------------------------------------------------------------
static const char*     constFile= "!const"; // The const file name
//----------------------------------------------------------------------------
// OS dependencies
//----------------------------------------------------------------------------
#if defined(_OS_WIN)
  #define S_IRWXU (S_IREAD | S_IWRITE | S_IEXEC)
#endif

//----------------------------------------------------------------------------
//
// Subroutine-
//       constModify
//
// Function-
//       Attempt to modify constant file
//
//----------------------------------------------------------------------------
static void
   constModify(                     // Attempt to modify constant file
     const char*       path)        // Current Path
{
   throwf("ERROR: Attempt to modify(%s/%s)\n"
          "(This must be done manually.)",
          path, constFile);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       getNamePart
//
// Function-
//       Extract the path name part: return foo.bar for /home/user/tmp/foo.bar
//
//----------------------------------------------------------------------------
static const char*                  // The name part of path
   getNamePart(                     // Get name part of path
     const char*       path)        // For this directory name
{
   int x= strlen(path);
   while( x > 0 )
   {
     x--;
     if( path[x] == '/' )
       return path + x + 1;
   }

   return path;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       invalidResponse
//
// Function-
//       An invalid response was received from the server
//
//----------------------------------------------------------------------------
static void                         // (Does not return)
   invalidResponse(                 // Handle invalid response
     int               lineno,      // Calling line number
     const char*       opCode,      // Failing operation name
     int               opResp)      // The invalid response
{
   throwf("%4d ClientThread: Why did Server reply '%c' (%d) to %s?",
          lineno, opResp, opResp, opCode);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       normalizeDirectory
//
// Function-
//       Normalize a directory name.
//
//----------------------------------------------------------------------------
static void
   normalizeDirectory(              // Normalize a directory name
     VersionInfo*      info,        // Associated version identifier
     char*             path)        // Path to current directory (MODIFIED)
{
   switch(info->f[1])
   {
     case VersionInfo::VIF1_OBSD:   // (Normalization not required)
       break;

     case VersionInfo::VIF1_OCYG:   // Remove "/cygdrive/*" header
       if( memcmp(path, "/cygdrive/", 10) == 0 )
       {
         if( path[10] != '\0' && path[11] == '/' )
           strcpy(path, path+11);
       }
       break;

     case VersionInfo::VIF1_OWIN:   // Remove "*:" header, convert '\\' to '/'
       {{{{
       if( path[0] != '\0' && path[1] == ':' )
         strcpy(path, path+2);

       int L= strlen(path);
       for(int i= 0; i<L; i++)
       {
         if( path[i] == '\\' )
           path[i]= '/';
       }
       }}}}
       break;

     default:
       throwf("%4d ClientThread: VersionInfo %d", __LINE__, info->f[1]);
       break;
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       printAction
//
// Function-
//       Print action taken for an item.
//
//----------------------------------------------------------------------------
static void
   printAction(                     // Action was taken
     const char*       action,      // This action was taken
     DirEntry*         ptrE,        // This file was acted upon
     const char*       reason)      // This is the reason why
{
   if( sw_quiet )                   // If silent running
     msglog("  %-10s %c %-32s %s\n",
            action, getFileType(ptrE->fileInfo), ptrE->fileName, reason);
   else
     msgout("  %-10s %c %-32s %s\n",
            action, getFileType(ptrE->fileInfo), ptrE->fileName, reason);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       printPath
//
// Function-
//       If firstTime parameter is TRUE, print path name.
//
//----------------------------------------------------------------------------
static int                          // constant FALSE
   printPath(
     int               firstTime,   // Is this the first time?
     const char*       path)        // The path name to print
{
   if( firstTime == TRUE            // If this is the first time
       &&sw_quiet != TRUE )         // and we are not running silent
     msgout("\n%s\n", path);        // Display the current directory

   return(FALSE);                   // Always return FALSE
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
{
   IFHCDM( debugf("%4d ClientThread(%p)::~ClientThread()\n", __LINE__, this); )
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
     const char*       path)        // Initial directory
:  CommonThread(socket)
,  path(path)
{
   IFHCDM(
     debugf("%4d ClientThread(%p)::ClientThread(%p,%s)\n", __LINE__, this,
            socket, path);
   )
}

//----------------------------------------------------------------------------
//
// Method-
//       ClientThread::exchangeVersionID
//
// Function-
//       Exchange version identifiers.
//
//----------------------------------------------------------------------------
int                                 // TRUE if version identifiers match
   ClientThread::exchangeVersionID( void ) // Exchange version identifiers
{
   PeerRequest         query;       // RdServer request
   PeerResponse        qresp;       // RdServer response
   int                 L;           // Response length

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
     lVersionInfo.f[7] |= VersionInfo::VIF7_KSUM; // Indicate checksum switch

   query.oc= REQ_VERSION;
   nSend(&query, sizeof(query));
   nSendString(&lVersionInfo, sizeof(lVersionInfo));
   memset(&inpVersion, 0, sizeof(inpVersion));
   L= nRecvString(&inpVersion, sizeof(inpVersion));
   nRecv(&qresp, sizeof(qresp));
   if( strcmp(inpVersion.info.version, RD_VERSION) != 0 )
   {
     msgout("%4d ClientThread: Version mismatch: Here(%s) Peer(%s)\n",
            __LINE__, RD_VERSION, inpVersion.info.version);
     return FALSE;
   }

   if( L != sizeof(VersionInfo) )
   {
     msgout("%4d ClientThread: Version length: Got(%d) Expected(%ld)\n",
            __LINE__, L, (long)sizeof(VersionInfo));
     return FALSE;
   }

   if( qresp.rc != RSP_YO )
   {
     invalidResponse(__LINE__, "VERSION", qresp.rc);
     return FALSE;
   }

   memcpy(&rVersionInfo, &inpVersion.info, sizeof(rVersionInfo));
   globalVersionInformation();

   //-------------------------------------------------------------------------
   // Verify the Current Working Directory and OS
   //-------------------------------------------------------------------------
   if( sw_unsafe == FALSE )         // Verify CWD?
   {
     char clientCWD[4096];
     if( getcwd(clientCWD, sizeof(clientCWD)) == NULL )
     {
       msgout("%4d ClientThread: system(getcwd) error\n", __LINE__);
       return FALSE;
     }

     query.oc= REQ_CWD;
     nSend(&query, sizeof(query));
     nRecv(&qresp, sizeof(qresp));
     if( qresp.rc != RSP_YO )
     {
       invalidResponse(__LINE__, "GETCWD", qresp.rc);
       return FALSE;
     }

     char serverCWD[4096];
     L= nRecvString(serverCWD, sizeof(serverCWD)-1);
     serverCWD[L]= '\0';

     normalizeDirectory(&lVersionInfo, clientCWD); // Normalize the names
     normalizeDirectory(&rVersionInfo, serverCWD);
     if( strcmp(getNamePart(clientCWD), getNamePart(serverCWD)) != 0 )
     {
       msgout("Error: CWD name mismatch: server(%s) client(%s)\n",
              getNamePart(serverCWD), getNamePart(clientCWD));
       msgout("Use -U for unsafe operation\n");
       return FALSE;
     }

     if( gVersionInfo.f[1] == VersionInfo::VIF1_OMIX
         && (lVersionInfo.f[1] == VersionInfo::VIF1_OWIN
         ||  rVersionInfo.f[1] == VersionInfo::VIF1_OWIN) )
       msgout("WARNING: OS mismatch: server(%s) client(%s)\n",
              rVersionInfo.f[1] == VersionInfo::VIF1_OWIN ? "WIN" : "BSD",
              lVersionInfo.f[1] == VersionInfo::VIF1_OWIN ? "WIN" : "BSD");
   }

   return TRUE;
}

//----------------------------------------------------------------------------
//
// Method-
//       ClientThread::installItem
//
// Function-
//       Install one file, link or directory.
//
//----------------------------------------------------------------------------
int
   ClientThread::installItem(       // Install something
     const char*       path,        // Current Path
     DirEntry*         serverE,     // -> Server DirEntry
     DirEntry*         clientE)     // -> Target item descriptor
{
   PeerRequest         query;       // Order to server
   PeerResponse        qresp;       // Reply to client
   int                 result;      // Resultant

   char                fullName[MAX_DIRFILE+1]; // Current file name
   int                 outf;        // Output (new) file handle
   off64_t             left;        // Bytes of file left to send
   int                 rlen;        // Number of bytes read
   int                 wlen;        // Number of bytes written
   int                 rc;          // Called routine return code

   //-------------------------------------------------------------------------
   // Get the fully qualified file name
   //-------------------------------------------------------------------------
   makeFileName(fullName, path, serverE->fileName);

   //-------------------------------------------------------------------------
   // Diagnostics
   //-------------------------------------------------------------------------
   msglog("\n");
   msglog("installItem: %s\n-----------\n", serverE->fileName);
   serverE->display("SERVER:");
   clientE->display("CLIENT:");

   #if( BRINGUP )
     printAction("ignored", clientE, "[BRINGUP (won't install)]");
     return(RC_ERROR);
   #endif

   //-------------------------------------------------------------------------
   // Install the item
   //-------------------------------------------------------------------------
   result= RC_NORM;                 // Default, successful
   switch(getFileType(serverE->fileInfo)) // Process by item type
   {
     case FT_PATH:                  // If it's a directory
       //---------------------------------------------------------------------
       // Install a directory
       //---------------------------------------------------------------------
       if( mkdir(fullName, S_IRWXU) != 0 ) // Create writeable
       {
         msgerr("%4d ClientThread: mkdir(%s) failure", __LINE__, fullName);
         result= RC_ERROR;
       }
       return result;               // Attributes updated later
       break;

     case FT_LINK:                  // If it's a link
       //---------------------------------------------------------------------
       // Install a soft link
       //---------------------------------------------------------------------
       rc= symlink(serverE->linkName, fullName); // Create new link
       if( rc != 0 )                // If symlink failure
       {
         printAction("skipped", serverE, "[Cannot create link]");
         result= RC_ERROR;
         break;
       }
       strcpy(clientE->linkName, serverE->linkName);
       break;

     case FT_FILE:                  // If it's a file
       {{{{                         // (Backout object created)
       //---------------------------------------------------------------------
       // Request the file
       //---------------------------------------------------------------------
       query.oc= REQ_FILE;          // Request the file
       nSend(&query, 1);            // Get the next entry
       nSendString(serverE->fileName,
                   strlen(serverE->fileName)); // Tell SERVER its name
//                 strlen(serverE->fileName) + 1); // Tell SERVER its name

       nRecv(&qresp, 1);            // Get the reply
       if( qresp.rc != RSP_YO )     // If operation rejected
       {
         if( qresp.rc != RSP_NO )   // If operation garbled
           invalidResponse(__LINE__, "FILE", qresp.rc);

         printAction("skipped", clientE, "[Disallowed by SERVER]");
         return RC_ERROR;
         break;
       }

       //---------------------------------------------------------------------
       // Open the file
       //---------------------------------------------------------------------
       outf= open64(fullName,
                    O_WRONLY|O_BINARY|O_TRUNC|O_CREAT,
                    S_IRUSR|S_IWUSR);
       if( outf < 0 )               // Open failed
       {
         msgerr("%4d ClientThread: open64(%s) failure", __LINE__, fullName);
         printAction("aborted", clientE, "[Open failure]");
         result= RC_ERROR;
       }

       //---------------------------------------------------------------------
       // Install recovery handler
       //---------------------------------------------------------------------
       Backout backout(path, serverE, outf);

       //---------------------------------------------------------------------
       // Receive the file (using server attributes!)
       //---------------------------------------------------------------------
       left= serverE->fileSize;     // Entire file left to be sent
       while(left > 0 )             // More bytes need to be sent
       {
         rlen= (unsigned)min(left, MAX_TRANSFER);
         nRecvStruct(buffer, rlen); // Read from SERVER
         if( outf < 0 )
           wlen= rlen;
         else
           wlen= write(outf,buffer,rlen); // Write some of the file
         if( wlen != rlen )         // Wrong amount written
           throwf("%4d ClientThread: %d=write(%s,%d) error",
                  __LINE__, wlen, fullName, rlen);

         left -= rlen;              // Those read aren't left to read
       }                            // Done reading more bytes

       //---------------------------------------------------------------------
       // Close the file
       //---------------------------------------------------------------------
       backout.reset();             // Transfer complete, cancel backout
       rc= 0;                       // Default, closed
       if( outf >= 0 )
         rc= close(outf);           // Close the file
       if( rc != 0 )                // Close data file failed
       {
         removeItem(path, serverE);
         msgerr("%4d ClientThread: close(%s) failure", __LINE__, fullName);
         printAction("aborted", serverE, "[I/O error]");
         result= RC_ERROR;
       }
       }}}}
       break;

     case FT_FIFO:                  // If it's a pipe
       //---------------------------------------------------------------------
       // Install a pipe
       //---------------------------------------------------------------------
       #if defined(_OS_WIN) || defined(_OS_CYGWIN)
         clientE->fileInfo= serverE->fileInfo; // Set updated stats
         msgout("%4d ClientThread: mkfifo(%s) not supported\n",
                __LINE__, fullName);
         result= RC_ERROR;

       #elif defined(_OS_BSD)
         clientE->fileInfo= serverE->fileInfo; // Set updated stats
         rc=mkfifo(fullName, clientE->fileInfo);
         if( rc != 0 )              // Failed to make the pipe
         {
           msgerr("%4d ClientThread: mkfifo(%s) failure", __LINE__, fullName);
           result= RC_ERROR;
         }
       #endif
       break;

     default:                       // If it's of unknown type
       //---------------------------------------------------------------------
       // Install an item of unknown type
       //---------------------------------------------------------------------
       printAction("ignored", clientE, "[What kind of thing is it?]");
       result= RC_ERROR;
   }

   //-------------------------------------------------------------------------
   // Update the item's attributes
   //-------------------------------------------------------------------------
   if( result == RC_NORM )
     updateAttr(path, serverE, clientE); // Update attributes

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       ClientThread::removeDirectory
//
// Function-
//       Remove all files and directories from a subtree.
//
//----------------------------------------------------------------------------
int                                 // Return code
   ClientThread::removeDirectory(   // Remove a directory
     const char*       path,        // The current path
     DirEntry*         clientE)     // -> Client item descriptor
{
   int                 returncd= TRUE; // This routine's return code

   DirList*            clientL;     // -> Client DirList
   DirEntry*           ptrEntry;    // -> DirEntry (working)
   char                pathName[MAX_DIRFILE+1]; // The nested pathName

   int                 rc;

   //-------------------------------------------------------------------------
   // Get the fully qualified file name
   //-------------------------------------------------------------------------
   makeFileName(pathName, path, clientE->fileName);

   //-------------------------------------------------------------------------
   // Diagnostics
   //-------------------------------------------------------------------------
   msglog("\n");
   msglog("removeDir: %s\n-----------\n", pathName);
   clientE->display("CLIENT:");

   #if( BRINGUP )
     printAction("kept", clientE, "[BRINGUP (won't rmdir)]");
     return(RC_ERROR);
   #endif

   //-------------------------------------------------------------------------
   // Switch into new subdirectory
   //-------------------------------------------------------------------------
   if( (clientE->fileInfo&INFO_RUSR) == 0  // Don't have permission to read
       ||(clientE->fileInfo&INFO_WUSR) == 0 //  or can't write in it
       ||(clientE->fileInfo&INFO_XUSR) == 0 ) //  or can't change to it
   {
     rc= chmod(pathName,
               clientE->chmod()|(S_IRUSR|S_IWUSR|S_IXUSR));
     if( rc != 0 )                  // Couldn't give self permissions
       throwf("%4d ClientThread: chmod(%s) failure", __LINE__, pathName);
   }

   //-------------------------------------------------------------------------
   // Delete all items in the subdirectory, recursively
   //-------------------------------------------------------------------------
   clientL= new DirList(this, path, clientE); // Read/sort the delete directory
   ptrEntry= clientL->head;         // Address the first element
   while( ptrEntry != NULL )        // For each item in the directory
   {
     if( getFileType(ptrEntry->fileInfo) == FT_PATH ) // If a subdirectory
       removeDirectory(pathName, ptrEntry); // Remove it first

     removeItem(pathName, ptrEntry);
     ptrEntry= ptrEntry->next;
   }
   delete clientL;

   //-------------------------------------------------------------------------
   // Restore permissions
   //-------------------------------------------------------------------------
   if( (clientE->fileInfo&INFO_RUSR) == 0 // Didn't have permission to read
       ||(clientE->fileInfo&INFO_WUSR) == 0 //  or to write in it
       ||(clientE->fileInfo&INFO_XUSR) == 0 ) //  or to change to it
   {
     rc= chmod(pathName,            // Restore permissions
               clientE->chmod());
     if( rc != 0 )  
       throwf("%4d ClientThread: chmod(%s) restore failure", __LINE__, pathName);
   }

   return(returncd);
}

//----------------------------------------------------------------------------
//
// Method-
//       ClientThread::removeItem
//
// Function-
//       Delete a file, link or directory.
//
//----------------------------------------------------------------------------
int                                 // Return code
   ClientThread::removeItem(        // Remove something
     const char*       path,        // Current Path
     DirEntry*         clientE)     // -> Client file descriptor
{
   char                fileName[MAX_DIRFILE+1]; // Current file name

   //-------------------------------------------------------------------------
   // Verify that we're not trying to remove a "!const" file
   //-------------------------------------------------------------------------
   if( strcmp(constFile, clientE->fileName) == 0 )
     constModify(path);

   //-------------------------------------------------------------------------
   // Get the fully qualified file name
   //-------------------------------------------------------------------------
   makeFileName(fileName, path, clientE->fileName);

   //-------------------------------------------------------------------------
   // Diagnostics
   //-------------------------------------------------------------------------
   msglog("\n");
   msglog("removeItem: %s\n-----------\n", clientE->fileName);
   clientE->display("CLIENT:");

   #if( BRINGUP )
     printAction("kept", clientE, "[BRINGUP (won't remove)]");
     return(RC_ERROR);
   #endif

   //-------------------------------------------------------------------------
   // Remove the item
   //-------------------------------------------------------------------------
   switch(getFileType(clientE->fileInfo)) // Process by item type
   {
     case FT_PATH:                  // If it's a directory
       //---------------------------------------------------------------------
       // Remove a directory (Its content has already been removed)
       //---------------------------------------------------------------------
       if( rmdir(fileName) != 0 )   // Remove directory failed
       {
         msgerr("%4d ClientThread: rmdir(%s) failure", __LINE__, fileName);
         return(RC_ERROR);
       }
       break;

     case FT_LINK:                  // If it's a link
       //---------------------------------------------------------------------
       // Remove a soft link
       //---------------------------------------------------------------------
       if( unlink(fileName) != 0 )  // Remove link failed
       {
         msgerr("%4d ClientThread: unlink(%s) failure", __LINE__, fileName);
         return(RC_ERROR);
       }
       break;

     case FT_FILE:                  // If it's a file
     case FT_FIFO:                  // If it's a pipe
       //---------------------------------------------------------------------
       // Remove a file or pipe
       //---------------------------------------------------------------------
       #if defined(_OS_WIN) || defined(_OS_CYGWIN)
         chmod(fileName, clientE->chmod()|S_IWUSR);
       #endif

       if( remove(fileName) != 0 ) // Remove file failed
       {
         msgerr("%4d ClientThread: remove(%s) failure", __LINE__, fileName);
         return(RC_ERROR);
       }
       break;

     default:                       // If it's of unknown type
       //---------------------------------------------------------------------
       // Remove an item of unknown type
       //---------------------------------------------------------------------
       printAction("ignored", clientE, "[What kind of thing is it?]");
       return(RC_ERROR);
   }

   return(RC_NORM);
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
long                                // Return code (always 0)
   ClientThread::run( void )        // Operate this ClientThread
{
   IFHCDM( debugf("%4d ClientThread(%p)::run()..\n", __LINE__, this); )

   // Thread initialization
   msgout("Client: Started...\n");
   fsm= FSM_READY;                  // Indicate operational

   // Thread operation
   try {
     char clientCWD[1024];
     char* base= getcwd(clientCWD, sizeof(clientCWD));
     if( base == NULL )
       msgout("%4d Client: system(getcwd) error\n", __LINE__);
     else if( exchangeVersionID() ) // If valid version
     {
       DirEntry dirEntry(this);
       strcpy(dirEntry.fileName, path); // Set the initial path
       dirEntry.list= new DirList(this, base, &dirEntry);
       updateDirectory(base, NULL, &dirEntry); // Install initial directory
     }

     PeerRequest  query;            // RdServer request
     PeerResponse qresp;            // RdServer response

     query.oc= REQ_QUIT;
     nSend(&query, sizeof(query));
     nRecv(&qresp, sizeof(qresp));

     // Normal termination
     msgout("Client: ...Complete\n");
     fsm= FSM_CLOSE;
   } catch( const char* X ) {
     fprintf(stderr, "Client: exception(%s)\n", X);
              msglog("Client: exception(%s)\n", X);
   } catch( std::exception& X ) {
     fprintf(stderr, "Client: exception(%s)\n", X.what());
              msglog("Client: exception(%s)\n", X.what());
   } catch(...) {
     fprintf(stderr, "Client: exception(%s)\n", "...");
              msglog("Client: exception(%s)\n", "...");
     ::exit(2);
   }

   // Thread termination
   term();                          // Indicate terminated

   IFHCDM( debugf("%4d ..ClientThread(%p)::run()\n", __LINE__, this); )
   return 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       ClientThread::term
//
// Purpose-
//       Terminate this ClientThread.
//
//----------------------------------------------------------------------------
void
   ClientThread::term( void )       // Terminate this ClientThread
{
   IFHCDM( debugf("%4d ClientThread(%p)::term()\n", __LINE__, this); )

   if( fsm == FSM_READY )           // If forced termination
   {
     const char* peerName= socket->getPeerName();
     if( peerName == NULL )
       peerName= Socket::addrToChar(socket->getPeerAddr());
     msgout("Client: ...Terminated\n");
   }

   CommonThread::term();
}

//----------------------------------------------------------------------------
//
// Method-
//       ClientThread::updateAttr
//
// Function-
//       Update item attributes.
//
//----------------------------------------------------------------------------
void
   ClientThread::updateAttr(        // Update attributes
     const char*       path,        // Current Path
     DirEntry*         serverE,     // -> Source file descriptor
     DirEntry*         clientE)     // -> Target file descriptor
{
   //-------------------------------------------------------------------------
   // Diagnostics
   //-------------------------------------------------------------------------
   msglog("\n");
   msglog("updateAttr: %s\n-----------\n", clientE->fileName);

   serverE->display("SERVER:");
   clientE->display("CLIENT:");

   if( getFileType(serverE->fileInfo) != getFileType(clientE->fileInfo) )
     msglog("-----: Why do file types differ?\n");

   #if( BRINGUP )
     printAction("ignored", clientE, "[BRINGUP]");
     return;
   #endif

   //-------------------------------------------------------------------------
   // We don't update attributes for links!
   //-------------------------------------------------------------------------
   if( getFileType(serverE->fileInfo) == FT_LINK )
     return;

   //-------------------------------------------------------------------------
   // Update the attributes
   //-------------------------------------------------------------------------
   clientE->fileSize= serverE->fileSize;
   clientE->fileTime= serverE->fileTime;
   clientE->fileInfo= serverE->fileInfo;
   clientE->fileKsum= serverE->fileKsum;
   clientE->intoFile(path);
}

//----------------------------------------------------------------------------
//
// Method-
//       ClientThread::updateDirectory
//
// Function-
//       Update new and changed files, links and directories within a
//       a directory subtree.
//
//----------------------------------------------------------------------------
void
   ClientThread::updateDirectory(   // Update directory subtree
     const char*       base,        // Base path to directory
     DirList*          dirList,     // The DirList containing dirEntry
     DirEntry*         dirEntry)    // Path to directory
{
   PeerRequest         query;       // Order to server
   PeerResponse        qresp;       // Reply from server
   char                pathName[MAX_DIRFILE+1]; // Nested path name
   const char*         path= dirEntry->fileName; // File name part

   DirList*            clientL;     // -> Client DirList
   DirEntry*           clientE;     // -> Client DirEntry (active)
   DirEntry*           clientP;     // -> Client DirEntry (previous)
   DirEntry*           ptrEntry;    // -> DirEntry (working)

   DirList*            serverL;     // -> Server DirList
   DirEntry*           serverE;     // -> Server DirEntry

   int                 firstTime= TRUE; // First time printPath called
   int                 ac;          // Action code
   int                 rc;          // Called routine return code

   //-------------------------------------------------------------------------
   // Initialization
   //-------------------------------------------------------------------------
   msglog("ClientThread: updateDirectory(%s,%s)\n", base, path);
   makeFileName(pathName, base, path);

   query.oc= REQ_GOTO;          // Request the subdirectory
   nSend(&query, 1);
   nSendString(path, strlen(path));
   nRecv(&qresp, 1);
   if( qresp.rc != RSP_YO )
   {
     if( qresp.rc != RSP_NO )
       invalidResponse(__LINE__, "GOTO", qresp.rc);

     DirEntry dirEntry(this);       // (Temporary)
     dirEntry.fileInfo= INFO_ISPATH;
     strcpy(dirEntry.fileName, pathName);

     firstTime= printPath(firstTime, pathName);
     printAction("skipped", &dirEntry, "[Disallowed by SERVER]");
     return;
   }

   //-------------------------------------------------------------------------
   // Load the remote directory contents
   //-------------------------------------------------------------------------
   serverL= nRecvDirectory(pathName);
   serverE= serverL->head;

   //-------------------------------------------------------------------------
   // Install/remove/update items in this directory
   //-------------------------------------------------------------------------
   #ifdef USE_ASYNCHRONOUS_LOADER
     if( dirList != NULL )          // If not the base DirEntry
       dirList->wait();             // Wait for dirEntry->list to be set
   #else
     (void)dirList;                 // (Parameter unused in this path)
   #endif
   clientL= dirEntry->list;         // Get the sorted subdirectory list
   clientP= NULL;                   // Indicate first entry
   clientE= clientL->head;          // Address the first element
   for(;;)                          // Process this directory
   {
     // Diagnostics
     msglog("\n");
     if( serverE == NULL )
       msglog("SERVER: NULL\n");
     else
       serverE->display("SERVER:");

     if( clientE == NULL )
       msglog("CLIENT: NULL\n");
     else
       clientE->display("CLIENT:");

     //-----------------------------------------------------------------------
     // See if directory processing is complete.
     //-----------------------------------------------------------------------
     if( clientE == NULL && serverE == NULL ) // If complete
       break;                       

     //-----------------------------------------------------------------------
     // Determine relative positions of items
     //-----------------------------------------------------------------------
     ac= AC_NOP;                    // Default, no postprocessing
     if( clientE == NULL )          // If target end of file
       rc= (+1);                    // Target name > Source name

     else if( serverE == NULL )     // If source end of file
       rc= (-1);                    // Target name < Source name

     else                           // If within both directories
       rc= strCompare(this, clientE->fileName, serverE->fileName); // Check position

     //-----------------------------------------------------------------------
     // An item exists remotely but not locally.  Install it.
     //-----------------------------------------------------------------------
     if( rc > 0 )                   // If we are missing an item
     {
       msglog("ACTION: install\n");
       ptrEntry= new DirEntry(this); // Allocate a new element

       strcpy(ptrEntry->fileName, serverE->fileName); 
       strcpy(ptrEntry->linkName, serverE->linkName); 
       ptrEntry->fileInfo= serverE->fileInfo; // Set updated stats
       ptrEntry->fileSize= serverE->fileSize;
       ptrEntry->fileKsum= serverE->fileKsum;
       ptrEntry->fileTime= 0;

       firstTime= printPath(firstTime, pathName);
       rc= installItem(pathName, serverE, ptrEntry);
       if( rc != RC_NORM )
       {
         ac= AC_GETSERVER;
         delete ptrEntry;           // Discard allocated element
       }
       else
       {
         ac= AC_BOTH;
         printAction("installed", ptrEntry, "");

         // Insert the installed item onto the client list
         clientE= clientL->insert(ptrEntry, clientP);
       }

       goto deferred_action;
     }

     //-----------------------------------------------------------------------
     // Disallow possible constant file update attempt
     //-----------------------------------------------------------------------
     if( strCompare(this, clientE->fileName, constFile) == 0 ) // If constant file
     {
       if( serverE == NULL )
         constModify(pathName);     // Server does not contain file

       if( getFileType(serverE->fileInfo) != getFileType(clientE->fileInfo)
           || strcmp(serverE->fileName, clientE->fileName) != 0 )
         constModify(pathName);     // Names or types differ

       if( serverE->fileSize != clientE->fileSize
           || serverE->fileKsum != clientE->fileKsum
           || serverE->fileTime != clientE->fileTime )
         constModify(pathName);     // Contents or times differ

       if( clientE->compareInfo(serverE) != 0 )
         constModify(pathName);     // Attributes differ
     }

     //-----------------------------------------------------------------------
     // An item exists locally but not remotely.  Remove it.
     //-----------------------------------------------------------------------
     if( rc < 0 )
     {
       msglog("ACTION: remove\n");

       if( sw_erase )
       {
         firstTime= printPath(firstTime, pathName);
         if( getFileType(clientE->fileInfo) == FT_PATH ) // If a path
           removeDirectory(pathName, clientE); // Remove the entire subtree

         rc= removeItem(pathName, clientE); // Remove the item itself
         if( rc == RC_NORM )
           printAction("removed", clientE, "");
         else
           printAction("kept", clientE, "[unable to remove]");
       }
       else                         // If removal not allowed
       {
         firstTime= printPath(firstTime, pathName); // Informational display
         printAction("kept", clientE, "[-E parameter not specified]");
       }

       // (Unconditionally) remove the entry from our list, then delete it
       clientE= clientL->remove(clientE, clientP);
       continue;                    
     }

     //-----------------------------------------------------------------------
     // Check for ambiguous update.
     //-----------------------------------------------------------------------
     if( (gVersionInfo.f[0]&VersionInfo::VIF0_CASE) == 0 // If case insensitive
         && (lVersionInfo.f[0]&VersionInfo::VIF0_CASE) !=
            (rVersionInfo.f[0]&VersionInfo::VIF0_CASE) ) // And different
     {
       if( strcmp(clientE->fileName, serverE->fileName) != 0 ) // If inexact
       {
         // If local machine is case sensitive (but remote is not)
         if( (lVersionInfo.f[0]&VersionInfo::VIF0_CASE) != 0 )
         {
           if( clientE->next != NULL
               && stricmp(clientE->fileName, clientE->next->fileName) == 0 )
           {
             firstTime= printPath(firstTime, pathName); // Informational display
             printAction("skipped", clientE, "[ambiguous]");

             // Remove the entry from the list, then delete it
             clientE= clientL->remove(clientE, clientP);
             goto deferred_action;
           }
         }

         // If remote machine is case sensitive (but local is not)
         if( (rVersionInfo.f[0]&VersionInfo::VIF0_CASE) != 0 )
         {
           if( serverE->next != NULL
               && stricmp(serverE->fileName, serverE->next->fileName) == 0 )
           {
             ac= AC_GETSERVER;
             firstTime= printPath(firstTime, pathName);
             printAction("skipped", serverE, "[ambiguous]");

             goto deferred_action;
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
     if( getFileType(serverE->fileInfo) != getFileType(clientE->fileInfo)
         || strcmp(serverE->fileName, clientE->fileName) != 0 )
     {
       msglog("ACTION: name or type mismatch\n");

       firstTime= printPath(firstTime, pathName); // Informational display
       if( !sw_erase )              // If erasure not allowed
       {
         printAction("kept", clientE, "[-E parameter not specified]");
         if( getFileType(serverE->fileInfo) != getFileType(clientE->fileInfo) )
           printAction("remote", serverE, "[type differs]");
         else
           printAction("remote", serverE, "[name differs]");

         clientE->fileInfo &= ~(INFO_ISTYPE); // Prevent subdirectory scan
       }

       else                         // If erasure allowed
       {
         if( getFileType(clientE->fileInfo) == FT_PATH ) // If a path
           removeDirectory(pathName, clientE); // Remove the entire subtree

         rc= removeItem(pathName, clientE); // Remove the item itself
         if( rc == RC_NORM )
           printAction("removed", clientE, "");

         // Set updated attributes
         clientE->fileInfo= serverE->fileInfo;
         strcpy(clientE->fileName, serverE->fileName);
         clientE->fileSize= serverE->fileSize;
         clientE->fileTime= 0;

         rc= installItem(pathName, serverE, clientE);
         if( rc != RC_NORM )       
         {
           // Remove the entry from the list, then release it
           clientE= clientL->remove(clientE, clientP);
           continue;                
         }

         printAction("installed", serverE, "");
       }

       ac= AC_BOTH;
       goto deferred_action;
     }

     //-----------------------------------------------------------------------
     // An identically named and typed item exists.
     //-----------------------------------------------------------------------
     msglog("ACTION: name and type identical\n");

     ac= AC_BOTH;
     switch(getFileType(clientE->fileInfo))
     {
       case FT_PATH:                // If directory
         if( clientE->compareInfo(serverE) != 0 )
         {
           firstTime= printPath(firstTime, pathName);
           updateAttr(pathName, serverE, clientE);
           printAction("attributes", clientE, "");
           break;
         }
         break;

       case FT_LINK:                // If soft link
         if( strcmp(clientE->linkName, serverE->linkName) != 0 )
         {
           firstTime= printPath(firstTime, pathName);
           rc= updateItem(pathName, serverE, clientE);
           if( rc == RC_NORM )
             printAction("updated", clientE, "");
         }
         break;

       case FT_FILE:                // If file
         //-------------------------------------------------------------------
         // Identical file names exist at both sites
         //-------------------------------------------------------------------
         if( serverE->fileSize == clientE->fileSize
             && serverE->fileKsum == clientE->fileKsum
             && serverE->compareTime(clientE) == 0 )
         {
           if( clientE->compareInfo(serverE) != 0 )
           {
             firstTime= printPath(firstTime, pathName);
             updateAttr(pathName, serverE, clientE);
             printAction("attributes", clientE, "");
           }
           break;
         }

         //-------------------------------------------------------------------
         // Check whether the file is newer here (and we care)
         //-------------------------------------------------------------------
         if( serverE->compareTime(clientE) < 0 && sw_older == FALSE )
         {
           firstTime= printPath(firstTime, pathName);
           printAction("kept", clientE, "[-O parameter not specified]");
           break;
         }

         //-------------------------------------------------------------------
         // The file needs to be replaced.
         //-------------------------------------------------------------------
         firstTime= printPath(firstTime, pathName);
         rc= updateItem(pathName, serverE, clientE);
         if( rc == RC_NORM )
           printAction("updated", serverE, "");
         break;

       case FT_FIFO:                // If pipe
         if( (serverE->fileInfo&INFO_PERMITS) != (clientE->fileInfo&INFO_PERMITS)
             ||serverE->fileTime != clientE->fileTime )
         {
           firstTime= printPath(firstTime, pathName);
           updateAttr(pathName, serverE, clientE);
           printAction("attributes", clientE, "");
         }
         break;

       default:                     // If unknown type
         break;
     }

     //-----------------------------------------------------------------------
     // Process deferred action code.
     //-----------------------------------------------------------------------
deferred_action:
     switch(ac)                     // Process action code
     {
       case AC_NOP:                 // No action
         break;

       case AC_GETSERVER:           // Get next SERVER item
         serverE= serverE->next;
         break;

       case AC_GETCLIENT:           // Get next CLIENT item
         clientP= clientE;          
         clientE= clientE->next;    
         break;

       case AC_BOTH:                // Get next item
         // If the local machine is case sensitive and the remote is not,
         // we must skip duplicate local items
         if( (lVersionInfo.f[0]&VersionInfo::VIF0_CASE) != 0
             && (rVersionInfo.f[0]&VersionInfo::VIF0_CASE) == 0 )
         {
           while( clientE->next != NULL
               && stricmp(clientE->fileName, clientE->next->fileName) == 0 )
           {
             clientP= clientE;
             clientE= clientE->next;
             firstTime= printPath(firstTime, pathName);
             printAction("skipped", clientE, "[ambiguous]");
           }
         }

         // If the remote machine is case sensitive and the local is not,
         // we must skip duplicate remote items
         if( (lVersionInfo.f[0]&VersionInfo::VIF0_CASE) == 0
             && (rVersionInfo.f[0]&VersionInfo::VIF0_CASE) != 0 )
         {
           while( serverE->next != NULL
               && stricmp(serverE->fileName, serverE->next->fileName) == 0 )
           {
             serverE= serverE->next;
             firstTime= printPath(firstTime, pathName);
             printAction("skipped", serverE, "[ambiguous]");
           }
         }

         clientP= clientE;          
         clientE= clientE->next;    

         serverE= serverE->next;    
         break;

       default:                     // If unknown code
         throwf("%4d ClientThread: Action code(%d)", __LINE__, ac);
         break;
     }
   }

   //-------------------------------------------------------------------------
   // Process subdirectories
   //-------------------------------------------------------------------------
   #ifdef USE_ASYNCHRONOUS_LOADER
     clientL->start();              // Start the asynchronous loader
   #else
     clientL->runLoader();          // Run the synchronous loader
   #endif
   clientE= clientL->head;          // Address the first element
   while(clientE != NULL)           // Dive into each subdirectory
   {
     if( getFileType(clientE->fileInfo) == FT_PATH )
     {
       //---------------------------------------------------------------------
       // Install a subdirectory
       //---------------------------------------------------------------------
       updateDirectory(pathName, clientL, clientE); // Process the new subtree

//     #if !defined(_OS_WIN) && !defined(_OS_CYGWIN)
         updateAttr(pathName, clientE, clientE); // Update directory attributes
//     #endif
     }

     clientE= clientE->next;        // Process next element
   }

   //-------------------------------------------------------------------------
   // Complete current directory processing
   //-------------------------------------------------------------------------
   delete serverL;                  // We are done with the server list
   #ifdef USE_EARLY_CLEANUP         // If early cleanup
     delete clientL;                // (Otherwise deleted by ~DirEntry)
     dirEntry->list= NULL;
   #endif

   query.oc= REQ_QUIT;
   nSend(&query, 1);
   nRecv(&qresp, 1);
   if( qresp.rc != RSP_YO )
     invalidResponse(__LINE__, "QUIT", qresp.rc);

   msglog("%4d ClientThread: updateDirectory(%s) complete\n", __LINE__, pathName);
}

//----------------------------------------------------------------------------
//
// Method-
//       ClientThread::updateItem
//
// Function-
//       Update a file, link or directory.
//
//----------------------------------------------------------------------------
int                                 // Return code
   ClientThread::updateItem(        // Update something
     const char*       path,        // Current Path
     DirEntry*         serverE,     // -> Source file descriptor
     DirEntry*         clientE)     // -> Target file descriptor
{
   int                 returncd;    // This routine's return code

   //-------------------------------------------------------------------------
   // Diagnostics
   //-------------------------------------------------------------------------
   msglog("\n");
   msglog("updateItem: %s\n-----------\n", serverE->fileName);
   serverE->display("SERVER:");
   clientE->display("CLIENT:");

   #if( BRINGUP )
     printAction("kept", clientE, "[BRINGUP (won't update)]");
     return(RC_ERROR);
   #endif

   //-------------------------------------------------------------------------
   // Update the item
   //-------------------------------------------------------------------------
   returncd= RC_NORM;               // Default, normal return code
   switch(getFileType(clientE->fileInfo)) // Process by item type
   {
     case FT_FIFO:                  // If pipe
     case FT_PATH:                  // If directory
       ;                            // No function required
       break;

     case FT_LINK:                  // If soft link
       returncd= removeItem(path, clientE); 
       if( returncd == RC_NORM )    
         returncd= installItem(path, serverE, clientE);
       break;

     case FT_FILE:                  // If file
       returncd= removeItem(path, clientE);
       if( returncd == RC_NORM )
         returncd= installItem(path, serverE, clientE);
       break;

     default:                       // If unknown type
       returncd= RC_ERROR;          // Cannot update it
       break;
   }

   return(returncd);
}

