//----------------------------------------------------------------------------
//
//       Copyright (c) 2014 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       CommonThread.cpp
//
// Purpose-
//       Implement CommonThread object methods
//
// Last change date-
//       2014/01/01
//
//----------------------------------------------------------------------------
#include <stdlib.h>
#include <string.h>

#include <com/Atomic.h>
#include <com/Barrier.h>
#include <com/Debug.h>
#include <com/define.h>             // For NULL
#include <com/Unconditional.h>

#include "CommonThread.h"

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
// CommonThread::Global attributes
//----------------------------------------------------------------------------
Semaphore              CommonThread::semaphore;      // Completion semaphore
int                    CommonThread::threadCount= 0; // The number of threads
CommonThread**         CommonThread::threadArray= NULL; // The thread array

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static Barrier         barrier= BARRIER_INIT;

//----------------------------------------------------------------------------
//
// Method-
//       CommonThread::~CommonThread
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   CommonThread::~CommonThread( void ) // Destructor
{
   IFHCDM( debugf("%4d CommonThread(%p)::~CommonThread()\n", __LINE__, this); )

   if( socket != NULL )
   {
     socket->close();
     socket= NULL;
   }

   if( buffer != NULL )
   {
     mx_buffer->release(buffer);
     buffer= NULL;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       CommonThread::CommonThread
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   CommonThread::CommonThread(      // Constructor
     Socket*           socket)      // Associated Socket
:  Thread()
,  fsm(FSM_RESET)
,  socket(socket)
,  buffer(NULL)
{
   IFHCDM( debugf("%4d CommonThread(%p)::CommonThread(%p)\n", __LINE__, this,
                  socket); )

   buffer= (char*)mx_buffer->allocate(); // Allocate a transfer buffer
   init();                          // Add this to CommonThread array
}

//----------------------------------------------------------------------------
//
// Method-
//       CommonThread::init
//
// Purpose-
//       Initialize this CommonThread.
//
//----------------------------------------------------------------------------
void
   CommonThread::init( void )       // Initialize this CommonThread
{
   IFHCDM( debugf("%4d CommonThread(%p)::init()\n", __LINE__, this); )

   AutoBarrier lock(barrier);
   {{{{
     for(int i= 0; i<threadCount; i++)
     {
       if( threadArray[i] == NULL )
       {
         threadArray[i]= this;
         return;
       }
     }

     // Need more worker thread slots
     int updatedThreadCount= threadCount + 8;
     int updatedSize= updatedThreadCount * sizeof(CommonThread*);
     CommonThread** updatedThreadArray= (CommonThread**)must_malloc(updatedSize);
     memset(updatedThreadArray, 0, updatedSize);
     for(int i= 0; i<threadCount; i++)
       updatedThreadArray[i]= threadArray[i];

     updatedThreadArray[threadCount]= this;

     if( threadArray != NULL )
       free(threadArray);
     threadCount= updatedThreadCount;
     threadArray= updatedThreadArray;
   }}}}
}

//----------------------------------------------------------------------------
//
// Method-
//       CommonThread::term
//
// Purpose-
//       Terminate this CommonThread.
//
//----------------------------------------------------------------------------
void
   CommonThread::term( void )       // Terminate this CommonThread
{
   IFHCDM( debugf("%4d CommonThread(%p)::term()\n", __LINE__, this); )

   fsm= FSM_FINAL;                  // Terminated
   semaphore.post();
}

//----------------------------------------------------------------------------
//
// Method-
//       CommonThread::globalVersionInformation
//
// Purpose-
//       Combine the local and remote capabilities vectors.
//
//----------------------------------------------------------------------------
void
   CommonThread::globalVersionInformation( void ) // Initialize global info
{
   gVersionInfo= lVersionInfo;
   for(int i=0; i<sizeof(gVersionInfo.f); i++)
     gVersionInfo.f[i]= lVersionInfo.f[i] & rVersionInfo.f[i];
}

//----------------------------------------------------------------------------
//
// Method-
//       CommonThread::localVersionInformation
//
// Purpose-
//       Set the local capability vector.
//
//----------------------------------------------------------------------------
void
   CommonThread::localVersionInformation( void )  // Initialize local info
{
   memset(&lVersionInfo, 0, sizeof(lVersionInfo));
   strcpy(lVersionInfo.version, RD_VERSION);
   #if defined(_OS_WIN)             // For Windows
     lVersionInfo.f[0] |= VersionInfo::VIF0_AWIN; // Windows attributes
     lVersionInfo.f[1] |= VersionInfo::VIF1_OWIN; // WIN operating system

   #elif defined(_OS_CYGWIN)        // For Cygwin (BSD on Windows)
     lVersionInfo.f[0] |= VersionInfo::VIF0_ABSD; // BSD attributes
     lVersionInfo.f[1] |= VersionInfo::VIF1_OCYG; // CYG operating system

   #elif defined(_OS_BSD)           // For standard BSD
     lVersionInfo.f[0] |= VersionInfo::VIF0_ABSD; // BSD attributes
     lVersionInfo.f[0] |= VersionInfo::VIF0_CASE; // Case sensitivity applies
     lVersionInfo.f[1] |= VersionInfo::VIF1_OBSD; // BSD operating system
   #endif

   // Operational controls
   if( sw_verify )
     lVersionInfo.f[7] |= VersionInfo::VIF7_KSUM; // Verify checksum
}

//----------------------------------------------------------------------------
//
// Method-
//       CommonThread::notify
//
// Purpose-
//       Thread event notification.
//
//----------------------------------------------------------------------------
int                                 // Return code (always 0)
   CommonThread::notify(            // Notify this CommonThread
     int               code)        // Using this enum NFC
{
   IFHCDM( debugf("%4d CommonThread(%p)::notify(%d)\n", __LINE__, this, code); )

   // If already terminated, ignore
   if( fsm == FSM_FINAL )
     return 0;

   // Thread termination
   fsm= FSM_CLOSE;                  // Indicate closing

   // THIS IS DANGEROUS: The thread is immediately cancelled
   cancel();                        // Terminate the Thread
   term();                          // Indicate terminated

   return 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       CommonThread::notifyAll
//
// Purpose-
//       Thread event notification.
//
//----------------------------------------------------------------------------
void
   CommonThread::notifyAll(         // Notity this CommonThread
     int               code)        // Using this enum NFC
{
   IFHCDM( debugf("%4d CommonThread(*)::notifyAll(%d)\n", __LINE__, code); )

   AutoBarrier lock(barrier);
   {{{{
     for(int i= 0; i<threadCount; i++) // Signal all the CommonThreads
     {
       if( threadArray[i] != NULL )
         threadArray[i]->notify(code);
     }

     for(int i= 0; i<threadCount; i++) // Insure thread termination
     {
       if( threadArray[i] != NULL )
       {
         if( threadArray[i]->getFSM() != FSM_FINAL )
           fprintf(stderr, "Waiting for CommonThread(%p)\n", threadArray[i]);

         threadArray[i]->wait();
         delete threadArray[i];
         threadArray[i]= NULL;
       }
     }
   }}}}
}

//----------------------------------------------------------------------------
//
// Method-
//       CommonThread::nRecv
//
// Purpose-
//       Read from network.
//
//----------------------------------------------------------------------------
unsigned                            // Number of bytes read
   CommonThread::nRecv(             // Read from network
     void*             addr,        // Data address
     unsigned          size)        // Data length
{
   int                 L;           // Read length

   L= socket->recv((char*)addr, size);
   if( iodm )
   {
     msglog("\n");
     msglog("%4d= nRecv(%p,%u)\n", L, addr, size);
     if( L > 0 )
       msgdump(addr, (unsigned)min(L,iodm));
   }

   if( L < 1 )
     throwf("%4d ERROR: %d=nRecv errno(%d) %s", __LINE__, L,
            socket->getSocketEC(), socket->getSocketEI());

   return L;
}

//----------------------------------------------------------------------------
//
// Method-
//       CommonThread::nRecvDirectory
//
// Purpose-
//       Read a sorted directory from the network.
//
//----------------------------------------------------------------------------
DirList*                            // -> DirList
   CommonThread::nRecvDirectory(    // Receive a sorted directory
     const char*       path)        // With this relative path
{
   const char*         ptrC;        // -> Generic character
   DirEntry*           prvE;        // -> DirEntry
   DirEntry*           ptrE;        // -> DirEntry
   DirList*            ptrL;        // -> DirList (resultant)

   HOST32              count;       // Number of directory elements
   InputBuffer         iBuffer(this); // Input buffer
   PeerDesc            peerDesc;    // File descriptor assembly area
   PeerPath            peerPath;    // Path descriptor
   PEER16              peerSize;    // String length (network format)
   unsigned            L;           // Generic length

   int                 i;

   msglog("nRecvDirectory(%s)\n", path);

   while( iBuffer.getDataSize() < sizeof(peerPath) )
     iBuffer.fill();
   ptrC= iBuffer.getDataAddr();
   memcpy(&peerPath, ptrC, sizeof(peerPath));
   iBuffer.use(sizeof(peerPath));
   if( hcdm > 8 )
   {
     msglog("nRecvDirectory count\n");
     msgdump(&peerPath, sizeof(peerPath));
   }
   count= peerToHost(peerPath.count);
   ptrL= new DirList(this, path);   // Allocate a DirList
   prvE= NULL;
   for(i=0; i<count; i++)           // Read the directory
   {
     ptrE= new DirEntry(this);      // Allocate a DirEntry
     if( prvE == NULL )
       ptrL->head= ptrE;
     else
       prvE->next= ptrE;
     prvE= ptrE;

     while( iBuffer.getDataSize() < sizeof(peerDesc) )
       iBuffer.fill();
     ptrC= iBuffer.getDataAddr();
     memcpy(&peerDesc, ptrC, sizeof(peerDesc));
     ptrE->fileInfo= peerToHost(peerDesc.fileInfo);
     ptrE->fileTime= peerToHost(peerDesc.fileTime);
     ptrE->fileSize= peerToHost(peerDesc.fileSize);
     ptrE->fileKsum= peerToHost(peerDesc.fileKsum);
     iBuffer.use(sizeof(peerDesc));

     while( iBuffer.getDataSize() < sizeof(peerSize) )
       iBuffer.fill();
     ptrC= iBuffer.getDataAddr();
     memcpy(&peerSize, ptrC, sizeof(peerSize));
     iBuffer.use(sizeof(peerSize));
     L= peerToHost(peerSize);
     if( L >= MAX_DIRNAME )
       throwf("%4d nRecvDirectory error: String overflow Length(%d)",
              __LINE__, L);

     while( iBuffer.getDataSize() < L )
       iBuffer.fill();
     ptrC= iBuffer.getDataAddr();
     memcpy(ptrE->fileName, ptrC, L);
     iBuffer.use(L);
     ptrE->fileName[L]= '\0';       // Set string delimiter

     if( getFileType(ptrE->fileInfo) == FT_LINK ) // If this is a link
     {
       while( iBuffer.getDataSize() < sizeof(peerSize) )
         iBuffer.fill();
       ptrC= iBuffer.getDataAddr();
       memcpy(&peerSize, ptrC, sizeof(peerSize));
       iBuffer.use(sizeof(peerSize));
       L= peerToHost(peerSize);
       if( L >= MAX_DIRNAME )
         throwf("%4d nRecvDirectory error: String overflow Length(%d)",
                __LINE__, L);

       while( iBuffer.getDataSize() < L )
         iBuffer.fill();
       ptrC= iBuffer.getDataAddr();
       memcpy(ptrE->linkName, ptrC, L);
       iBuffer.use(L);
       ptrE->linkName[L]= '\0';     // Set string delimiter
     }
   }

   ptrL->count= count;              // Set the count
   if( hcdm > 8 )
     ptrL->display("nRecvDirectory");

   return ptrL;
}

//----------------------------------------------------------------------------
//
// Method-
//       CommonThread::nRecvString
//
// Purpose-
//       Read string from network.
//
//----------------------------------------------------------------------------
int                                 // Number of bytes read
   CommonThread::nRecvString(       // Read string from network
     void*             addr,        // Data address
     unsigned          size)        // Data length
{
   PEER16              peerSize;    // String length (network format)
   HOST16              hostSize;    // String length (host format)

   peerSize= 0;                     // In case of abort
   nRecvStruct(&peerSize, sizeof(peerSize));
   hostSize= peerToHost(peerSize);
   if( hostSize > (size-1) )
     throwf("%4d nRecvString error: String overflow:\n"
            ">>Length(%d), Size(%d)",
            __LINE__, hostSize, size);

   nRecvStruct(addr, hostSize);
   ((char*)addr)[hostSize]= '\0';

   return hostSize;
}

//----------------------------------------------------------------------------
//
// Method-
//       CommonThread::nRecvStruct
//
// Purpose-
//       Read structure from network.
//
//----------------------------------------------------------------------------
void
   CommonThread::nRecvStruct(       // Read from network
     void*             addr,        // Data address
     unsigned          size)        // Data length
{
   char*               ccp;         // Current character position
   int                 L;           // Read length

   ccp= (char*)addr;
   L= 0;
   while( L < size )
   {
     ccp  += L;
     size -= L;

     L= nRecv(ccp, size);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       CommonThread::nSend
//
// Purpose-
//       Send to network.
//
//----------------------------------------------------------------------------
unsigned                            // Number of bytes sent
   CommonThread::nSend(             // Send to network
     const void*       addr,        // Data address
     unsigned          size)        // Data length
{
   int                 L;           // Send length

   if( MAX_SENDSIZE > 0 && size > MAX_SENDSIZE )
     size= MAX_SENDSIZE;

   L= socket->send((const char*)addr, size); // Send message
   if( iodm )
   {
     msglog("\n");
     msglog("%4d= nSend(%p,%u)\n", L, addr, size);
     msgdump(addr, (unsigned)min(size,iodm));
   }

   if( L < 1 )
     throwf("%4d ERROR: %d=nSend errno(%d) %s", __LINE__, L,
            socket->getSocketEC(), socket->getSocketEI());

   return L;
}

//----------------------------------------------------------------------------
//
// Method-
//       CommonThread::nSendDirectory
//
// Purpose-
//       Send a sorted directory on network.
//
//----------------------------------------------------------------------------
void
   CommonThread::nSendDirectory(    // Send a sorted directory
     DirList*          ptrL)        // -> DirList
{
   char*               ptrC;        // -> Generic character
   DirEntry*           ptrE;        // -> DirEntry

   OutputBuffer        oBuffer(this); // Output buffer
   PeerDesc            peerDesc;    // File descriptor assembly area
   PeerPath            peerPath;    // Path descriptor
   PEER16              peerSize;    // String length (network format)
   unsigned            L;           // Generic length

   int                 i;

   msglog("nSendDirectory\n");

   peerPath.count= hostToPeer(ptrL->count);
   ptrC= (char*)&peerPath;
   for(i=0; i<sizeof(peerPath); i++)
     oBuffer.putChar(ptrC[i]);
   if( hcdm > 8 )
   {
     msglog("nSendDirectory count:\n");
     msgdump(&peerPath, sizeof(peerPath));
   }

   ptrE= ptrL->head;                // Begin at the beginning
   while( ptrE != NULL )            // Write the directory
   {
     peerDesc.fileInfo= hostToPeer(ptrE->fileInfo);
     peerDesc.fileSize= hostToPeer(ptrE->fileSize);
     peerDesc.fileTime= hostToPeer(ptrE->fileTime);
     peerDesc.fileKsum= hostToPeer(ptrE->fileKsum);

     while( oBuffer.getDataSize() < sizeof(peerDesc) )
       oBuffer.empty();
     ptrC= oBuffer.getDataAddr();

     memcpy(ptrC, &peerDesc, sizeof(peerDesc));
     oBuffer.use(sizeof(peerDesc));

     L= strlen(ptrE->fileName);
     peerSize= hostToPeer((HOST16)L);
     while( oBuffer.getDataSize() < sizeof(peerSize) )
       oBuffer.empty();
     ptrC= oBuffer.getDataAddr();
     memcpy(ptrC, &peerSize, sizeof(peerSize));
     oBuffer.use(sizeof(peerSize));

     while( oBuffer.getDataSize() < L )
       oBuffer.empty();
     ptrC= oBuffer.getDataAddr();
     memcpy(ptrC, ptrE->fileName, L);
     oBuffer.use(L);

     if( getFileType(ptrE->fileInfo) == FT_LINK ) // If this is a link
     {
       L= strlen(ptrE->linkName);
       peerSize= hostToPeer((HOST16)L);
       while( oBuffer.getDataSize() < sizeof(peerSize) )
         oBuffer.empty();
       ptrC= oBuffer.getDataAddr();
       memcpy(ptrC, &peerSize, sizeof(peerSize));
       oBuffer.use(sizeof(peerSize));

       while( oBuffer.getDataSize() < L )
         oBuffer.empty();
       ptrC= oBuffer.getDataAddr();
       memcpy(ptrC, ptrE->linkName, L);
       oBuffer.use(L);
     }

     ptrE= ptrE->next;
   }

   oBuffer.empty();                 // Empty the buffer
}

//----------------------------------------------------------------------------
//
// Method-
//       CommonThread::nSendString
//
// Purpose-
//       Send name on network.
//
//----------------------------------------------------------------------------
void
   CommonThread::nSendString(       // Send name to network
     const void*       addr,        // Data address
     unsigned          size)        // Data length
{
   PEER16              peerSize;    // String length (network format)

   peerSize= hostToPeer((PEER16)size);
   nSendStruct(&peerSize, sizeof(peerSize));
   nSendStruct(addr, size);
}

//----------------------------------------------------------------------------
//
// Method-
//       CommonThread::nSendStruct
//
// Purpose-
//       Send structure to network.
//
//----------------------------------------------------------------------------
void
   CommonThread::nSendStruct(       // Send to network
     const void*       addr,        // Data address
     unsigned          size)        // Data length
{
   const char*         ccp;         // Current character position
   int                 L;           // Send length

   ccp= (const char*)addr;
   L= 0;
   while( L < size )
   {
     ccp  += L;
     size -= L;

     L= nSend(ccp, size);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       CommonThread::status
//
// Purpose-
//       Display the status of all CommonThread objects.
//
//----------------------------------------------------------------------------
void
   CommonThread::status( void )     // Display status
{
   AutoBarrier lock(barrier);

   for(int i= 0; i<threadCount; i++)
   {
     if( threadArray[i] != NULL )
     {
       //---------------------------------------------------------------------
       // Display the status
       //---------------------------------------------------------------------
       CommonThread* thread= threadArray[i];

       const char* state= "FSM_ERROR";
       switch( thread->getFSM() )
       {
         case FSM_READY:
           state= "FSM_READY";
           break;

         case FSM_CLOSE:
           state= "FSM_CLOSE";
           break;

         case FSM_RESET:
           state= "FSM_RESET";
           break;

         case FSM_FINAL:
           state= "FSM_FINAL";
           break;
       }

       //---------------------------------------------------------------------
       // Implementation note-
       //   Dynamic cast cannot be used here (in Linux) because ListenThread.o
       //   is not included in RdClient. This causes the error message:
       //   undefined reference to `typeinfo for ListenThread' when linking.
       //---------------------------------------------------------------------
       Socket* socket= thread->socket; // Get associated
       if( socket == NULL )
         fprintf(stderr, "Status: %s Host(UNKNOWN)%s\n", state,
                         thread->isListenThread() ? " [LISTEN]" : "");
       else if( thread->isListenThread() )
       {
         const char* name= socket->getHostName();
         int port= socket->getHostPort();
         fprintf(stderr, "Status: %s Host(%s:%d) [LISTEN]\n",
                         state, name, port);
       }
       else
       {
         const char* name= socket->getPeerName();
         int port= socket->getPeerPort();
         fprintf(stderr, "Status: %s Host(%s:%d)\n", state, name, port);
       }
     }
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       CommonThread::wait
//
// Purpose-
//       Thread::wait with debugging information
//
//----------------------------------------------------------------------------
long
   CommonThread::wait( void )       // Wait with debugging message
{
   IFHCDM( debugf("%4d CommonThread(%p)::wait()\n", __LINE__, this); )
   return Thread::wait();
}

//----------------------------------------------------------------------------
//
// Method-
//       CommonThread::waiter
//
// Purpose-
//       Wait for interrupt
//
//----------------------------------------------------------------------------
void
   CommonThread::waiter( void )     // Wait for termination interrupt
{
   for(;;)
   {
     semaphore.wait();

     AutoBarrier lock(barrier);
     {{{{
       for(int i= 0; i<threadCount; i++)
       {
         if( threadArray[i] != NULL )
         {
           int fsm= threadArray[i]->getFSM();
           switch( fsm )
           {
             case FSM_READY:
               break;

             case FSM_CLOSE:
             case FSM_RESET:
               IFHCDM(
                 debugf("%4d CommonThread [%d] fsm(%d)\n", __LINE__, i, fsm);
               )
               break;

             case FSM_FINAL:
               threadArray[i]->wait();
               delete threadArray[i];
               threadArray[i]= NULL;
               break;

             default:
               throwf("%4d CommonThread [%d] fsm(%d) INVALID", __LINE__,
                      i, fsm);
               break;
           }
         }
       }

       int operational= FALSE;
       for(int i= 0; i<threadCount; i++)
       {
         if( this == threadArray[i] )
         {
           operational= TRUE;
           break;
         }
       }

       if( operational == FALSE )
         return;
     }}}}
   }
}

