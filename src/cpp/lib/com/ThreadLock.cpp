//----------------------------------------------------------------------------
//
//       Copyright (c) 2020 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       ThreadLock.cpp
//
// Purpose-
//       Implement ThreadLock object methods
//
// Last change date-
//       2020/10/03
//
//----------------------------------------------------------------------------
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <com/Atomic.h>
#include <com/Barrier.h>
#include <com/Debug.h>
#include <com/define.h>             // For NULL
#include <com/Semaphore.h>
#include <com/Software.h>
#include <com/Unconditional.h>

#include "com/ThreadLock.h"

#if defined(__GNUC__)
  // __x86_64__ is a builtin
#elif defined(_OS_WIN)
//  #error "WIN64 long is a 32 bit value"
//  #define __x86_64__
#else
  #error "Don't know how to determine word size via macro"
#endif

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
// Route debugging output to trace file
#define debugf traceh

#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef SCDM
#undef  SCDM                        // If defined, Soft Core Debug Mode
#endif

#ifndef VERIFY
#define VERIFY                      // If defined, internal verification
#endif

#define MINIMUM_LOCK_COUNT 64       // Minimum Object.lockCount
#define MINIMUM_USER_COUNT 64       // Minimum Object.userCount
#define MINIMUM_SIZE_COUNT 64       // Minimum UserEntry.size

#ifndef USE_LAZY_DEADLOCK_DETECTION
#undef  USE_LAZY_DEADLOCK_DETECTION // See ThreadLock::obtain
#endif

//----------------------------------------------------------------------------
// Dependent macros
//----------------------------------------------------------------------------
#include <com/ifmacro.h>

//----------------------------------------------------------------------------
// Typedefs and enumerations
//----------------------------------------------------------------------------
typedef Software::Tid_t THREAD_ID;  // Thread identifier

enum Mode                           // The ThreadLock mode
{  SHR                              // Shared mode
,  XCL                              // Exclusive mode
}; // enum Mode

//----------------------------------------------------------------------------
//
// Struct-
//       LockQueue
//
// Purpose-
//       Exclusive access queue entry.
//
//----------------------------------------------------------------------------
struct LockQueue {                  // Exclusive access queue entry
   LockQueue*          next;        // Next (older) entry in list

   Semaphore*          semaphore;   // The associated wait Semaphore
   THREAD_ID           thread;      // The associated thread identifier
}; // struct LockQueue

//----------------------------------------------------------------------------
//
// Struct-
//       LockEntry
//
// Purpose-
//       Everything needed to describe a lock.
//
//----------------------------------------------------------------------------
struct LockEntry {                  // Lock table entry
   LockEntry*          next;        // Next entry in list
   char*               name;        // The name of the lock

   int                 share;       // The number of shares
   LockQueue*          shrQueue;    // The NEWEST SHR LockQueue entry
   LockQueue*          xclQueue;    // The NEWEST XCL LockQueue entry
}; // struct LockEntry

//----------------------------------------------------------------------------
//
// Struct-
//       UserBlock
//
// Purpose-
//       A descriptor for a held lock.
//
//----------------------------------------------------------------------------
struct UserBlock {                  // User lock block
   Mode                mode;        // The lock mode
   LockEntry*          lockEntry;   // The associated LockEntry
}; // struct UserBlock

//----------------------------------------------------------------------------
//
// Struct-
//       UserEntry
//
// Purpose-
//       For each thread, the ordered list of locks held.
//
//----------------------------------------------------------------------------
struct UserEntry {                  // User table entry
   UserEntry*          next;        // Next entry in list
   THREAD_ID           thread;      // The associated thread identifier
   LockEntry*          waitEntry;   // Thread waiting for this LockEntry

   int                 held;        // The number of locks held
   int                 size;        // The number of available UserBlocks
   UserBlock*          lock;        // The UserBlock lock array
}; // struct UserEntry

//----------------------------------------------------------------------------
//
// Struct-
//       Object
//
// Purpose-
//       Hidden implementation
//
//----------------------------------------------------------------------------
struct Object {                     // Implementation Object
   Barrier             barrier;     // Barrier for THIS ThreadLock Object
   ThreadLock*         threadLock;  // Back pointer to ThreadLock

   unsigned int        counter;     // Hash table usage counter
   unsigned int        trigger;     // Hash table counter trigger

   unsigned int        lockCount;   // Number of lockTable entries
   LockEntry**         lockTable;   // The LockEntry hash table

   unsigned int        userCount;   // Number of userTable entries
   UserEntry**         userTable;   // The UserEntry hash table
}; // struct Object

//----------------------------------------------------------------------------
//
// Subroutine-
//       hashf
//
// Purpose-
//       Compute hash function for name.
//
//----------------------------------------------------------------------------
static unsigned int                 // The hash index
   hashf(                           // The hash function
     const char*       name)        // The lock name
{
   unsigned hash= 0;                // Resultant
   while( *name != '\0' )
   {
     hash *= 32;                    // hash * 31 + *(name++)
     hash += (*name);
     hash -= 32;
     name++;
   }

   return hash;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       userf
//
// Purpose-
//       Compute hash function for thread id.
//
//----------------------------------------------------------------------------
static unsigned int                 // The hash index
   userf(                           // The hash function
     THREAD_ID         index)       // The thread identifier/resultant
{
   #ifdef __x86_64__
     index += (index >> 32);        // Gets warning if not 64 bit machine
   #endif
   index += (index >> 16);
   index += (index >> 24);

   return index;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       findLockEntry
//
// Purpose-
//       Locate LockEntry.
//
//----------------------------------------------------------------------------
static LockEntry*                   // The LockEntry, NULL if non-existent
   findLockEntry(                   // Get LockEntry
     Object*           O,           // For this Object
     const char*       name)        // And this lock name
{
   unsigned index= hashf(name) % O->lockCount;

   LockEntry* lockEntry= O->lockTable[index];
   while( lockEntry != NULL )
   {
     if( strcmp(name, lockEntry->name) == 0 )
       break;

     lockEntry= lockEntry->next;
   }

   return lockEntry;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       findUserEntry
//
// Purpose-
//       Locate UserEntry.
//
//----------------------------------------------------------------------------
static UserEntry*                   // The UserEntry, NULL if non-existent
   findUserEntry(                   // Get UserEntry
     Object*           O,           // For this Object
     THREAD_ID         thread)      // And this thread identifier
{
   int index= userf(thread) % O->userCount;
   UserEntry* userEntry= O->userTable[index];
   while( userEntry != NULL )
   {
     if( thread == userEntry->thread )
       break;

     userEntry= userEntry->next;
   }

   return userEntry;
}

static UserEntry*                   // The UserEntry, NULL if non-existent
   findUserEntry(                   // Get UserEntry
     Object*           O)           // For this Object
{  return findUserEntry(O, Software::getTid());
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       modeName
//
// Purpose-
//       Get the name for the mode ("SHR" or "XCL")
//
//----------------------------------------------------------------------------
static inline const char*           // The Mode name
   modeName(                        // Get Mode name
     Mode              mode)        // For Mode value
{
   return (mode == SHR) ? "SHR" : "XCL";
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       post
//
// Purpose-
//       Drive a waiting thread.
//
//----------------------------------------------------------------------------
static void
   post(                            // Drive a waiting thread
     Object*           O,           // For this Object
     LockEntry*        lockEntry,   // And this LockEntry
     LockQueue*        lockQueue)   // And this LockQueue
{
   IFSCDM( debugf("%4d Threadlock(%p)::post(%s) %lx\n", __LINE__,
                  O->threadLock, lockEntry->name, lockQueue->thread); )

   UserEntry* userEntry= findUserEntry(O, lockQueue->thread);
   assert( userEntry != NULL && userEntry->waitEntry == lockEntry);

   userEntry->waitEntry= NULL;      // Indicate posted
   lockQueue->semaphore->post();    // Then post
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       updateCounter
//
// Purpose-
//       Every so often, resize the lock table.
//
//----------------------------------------------------------------------------
static void
   updateCounter(                   // Update the table change counter
     Object*           O)           // For this Object
{
   O->counter++;
   if( O->counter < O->trigger )
     return;
   O->counter= 0;                   // Reset for next time

   // Determine the number of existing lock names
   unsigned lockCount= 0;           // Number of existing lock names
   for(unsigned i= 0; i<O->lockCount; i++)
   {
     LockEntry* lockEntry= O->lockTable[i];
     while( lockEntry != NULL )
     {
       lockCount++;
       lockEntry= lockEntry->next;
     }
   }

   // Set the next trigger
   unsigned trigger= lockCount / 2;
   if( trigger < (MINIMUM_LOCK_COUNT * 4) )
     trigger= MINIMUM_LOCK_COUNT * 4;
   O->trigger= trigger;

   // Ideal range: O->lockCount <= max(MINIMUM,lockCount) < (O->lockCount/4)
   if( lockCount < O->lockCount )   // If contraction required
   {
     if( lockCount < MINIMUM_LOCK_COUNT ) // If we need less than the minimum
     {
       if( O->lockCount <= MINIMUM_LOCK_COUNT ) // If no contraction needed
         return;

       lockCount= MINIMUM_LOCK_COUNT;
     }
   }
   else if( lockCount < (O->lockCount/4) ) // If expansion not required
     return;

   //-------------------------------------------------------------------------
   // Table expansion/replacement
   IFSCDM( debugf("%4d %s expand/contract\n", __LINE__, __FILE__); )
// IFHCDM( debugObject(O); );

   lockCount /= 3;                  // Start with 1 for each 3 locks
   lockCount += (MINIMUM_LOCK_COUNT-1); // Round up
   lockCount &= (~(MINIMUM_LOCK_COUNT-1)); // Truncate down
   LockEntry** lockTable= (LockEntry**)malloc(lockCount * sizeof(LockEntry*));
   if( lockTable == NULL )          // If no room
     return;                        // Don't modify the table

   memset(lockTable, 0, lockCount * sizeof(LockEntry*));
   lockCount--;                     // (The last entry is unused)

   //-------------------------------------------------------------------------
   // Table replacement (Inverts lock order on hash list)
   for(unsigned i= 0; i<O->lockCount; i++)
   {
     LockEntry* lockEntry= O->lockTable[i];
     while( lockEntry != NULL )
     {
       LockEntry* lastEntry= lockEntry;
       lockEntry= lockEntry->next;

       unsigned int index= hashf(lastEntry->name) % lockCount;
       lastEntry->next= lockTable[index];
       lockTable[index]= lastEntry;
     }
   }

   //-------------------------------------------------------------------------
   // Table replacement complete
   free(O->lockTable);              // Remove the old table
   O->lockTable= lockTable;         // Install the new table
   O->lockCount= lockCount;

   IFSCDM( debugf("%4d %s lockCount(%d)\n", __LINE__, __FILE__, lockCount); )
// IFHCDM( debugObject(O); );
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       allocLockEntry
//
// Purpose-
//       Allocate a LockEntry, adding it to the LockTable
//
//----------------------------------------------------------------------------
static LockEntry*                   // The newly allocated LockEntry
   allocLockEntry(                  // Allocate a LockEntry object
     Object*           O,           // For this Object
     const char*       name)        // For this lock name
{
   LockEntry* lockEntry= (LockEntry*)malloc(sizeof(LockEntry));
   memset(lockEntry, 0, sizeof(LockEntry));
   lockEntry->name= must_strdup(name);

   int index= hashf(name) % O->lockCount;
   lockEntry->next= O->lockTable[index];
   O->lockTable[index]= lockEntry;

   updateCounter(O);                // Record name addition

   return lockEntry;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       allocLockQueue
//
// Purpose-
//       Allocate a LockQueue
//
//----------------------------------------------------------------------------
static LockQueue*                   // The newly allocated LockQueue
   allocLockQueue( void )           // Allocate a LockQueue object
{
   LockQueue* lockQueue= (LockQueue*)malloc(sizeof(LockQueue));
   memset(lockQueue, 0, sizeof(LockQueue));
   lockQueue->thread= Software::getTid();

   return lockQueue;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       allocUserEntry
//
// Purpose-
//       Allocate a UserEntry, adding it to the userTable
//
//----------------------------------------------------------------------------
static UserEntry*                   // The newly allocated UserEntry
   allocUserEntry(                  // Allocate a UserEntry
     Object*           O,           // For this Object
     THREAD_ID         thread)      // And this thread identifier
{
   UserEntry* userEntry= (UserEntry*)malloc(sizeof(UserEntry));
   memset(userEntry, 0, sizeof(UserEntry));
   userEntry->thread= thread;

   userEntry->size= MINIMUM_SIZE_COUNT;// Lock nesting level
   userEntry->lock= (UserBlock*)malloc(sizeof(UserBlock) * userEntry->size);
   memset(userEntry->lock, 0, sizeof(UserBlock) * userEntry->size);

   int index= userf(thread) % O->userCount;
   userEntry->next= O->userTable[index];
   O->userTable[index]= userEntry;

   return userEntry;
}

static UserEntry*                   // The UserEntry
   allocUserEntry(                  // Allocate a UserEntry
     Object*           O)           // For this Object
{  return allocUserEntry(O, Software::getTid());
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       debugObject
//
// Purpose-
//       Object debugging display
//
//----------------------------------------------------------------------------
static void
   debugObject(                     // Debugging display
     Object*           O)           // For this Object
{
   debugf(".. %4u counter\n", O->counter);
   debugf(".. %4u trigger\n", O->trigger);
   debugf(".. %4u lockCount\n", O->lockCount);
   for(unsigned i= 0; i<O->lockCount; i++)
   {
     LockEntry* lockEntry= O->lockTable[i];
     if( lockEntry != NULL )
     {
       while( lockEntry != NULL )
       {
         LockQueue* lockQueue;
         debugf(".... [%4d] %p '%s' share(%4d) shrQ(%p) xclQ(%p) next(%p)\n",
                i, lockEntry, lockEntry->name, lockEntry->share,
                lockEntry->shrQueue, lockEntry->xclQueue, lockEntry->next);

         lockQueue= lockEntry->shrQueue;
         while( lockQueue != NULL )
         {
           debugf("...... SHR %p semaphore(%p) thread(%lx) next(%p)\n",
                  lockQueue, lockQueue->semaphore, (long)lockQueue->thread,
                  lockQueue->next);
           lockQueue= lockQueue->next;
         }

         lockQueue= lockEntry->xclQueue;
         while( lockQueue != NULL )
         {
           debugf("...... XCL %p semaphore(%p) thread(%lx) next(%p)\n",
                  lockQueue, lockQueue->semaphore, (long)lockQueue->thread,
                  lockQueue->next);
           lockQueue= lockQueue->next;
         }

         lockEntry= lockEntry->next;
       }
     }
   }

   debugf(".. %4u userCount\n", O->userCount);
   for(unsigned i= 0; i<O->userCount; i++)
   {
     UserEntry* userEntry= O->userTable[i];
     while( userEntry != NULL )
     {
       debugf(".... [%4d] %p thread(%lx) held(%d) size(%d) next(%p)\n", i,
              userEntry, (long)userEntry->thread, userEntry->held,
              userEntry->size, userEntry->next);
       for(int j= 0; j<userEntry->held; j++)
       {
         UserBlock* userBlock= &userEntry->lock[j];
         LockEntry* lockEntry= userBlock->lockEntry;
         const char* mode= modeName(userBlock->mode);
         const char* name= lockEntry->name;
         debugf(".... [%4d][%4d] %p %s(%s)\n", i, j, lockEntry, mode, name);
       }

       userEntry= userEntry->next;
     }
   }
}

//----------------------------------------------------------------------------
//
// Struct-
//       Dependency
//
// Purpose-
//       Deadlock dependency block.
//
//----------------------------------------------------------------------------
struct Dependency {                 // Deadlock dependency block
   Dependency*         next;        // Next (prior) entry in list

   UserEntry*          userEntry;   // The associated UserEntry
   UserBlock*          userBlock;   // The associated UserBlock
}; // struct Dependency

//----------------------------------------------------------------------------
//
// Subroutine-
//       deadlockDependency
//
// Purpose-
//       Check whether a deadlock situation exists.
//
//----------------------------------------------------------------------------
static int                          // TRUE if dependency exists
   deadlockDependency(              // Detect deadlock dependency
     Object*           O,           // For this Object
     LockEntry*        root,        // The root LockEntry
     Dependency*       tail)        // The previous Dependency
{
   UserBlock*          userBlock= tail->userBlock;
   LockEntry*          lockEntry= tail->userBlock->lockEntry;
   Mode                mode= userBlock->mode;
   THREAD_ID           thread= tail->userEntry->thread;

   IFHCDM( debugf("[%lx] root(%s) deadlockDependency(%lx,%s(%s))\n",
                  Software::getTid(), root->name, thread,
                  modeName(tail->userBlock->mode), lockEntry->name); )

   // Check whether this dependency causes a conflict
   if( root == lockEntry )
   {
     debugf("Thread(%lx) holds %s(%s), **DEADLOCK**\n", (long)thread,
            modeName(mode), lockEntry->name);
     return TRUE;
   }

   // Check whether this dependency's dependencies cause a conflict
   LockQueue* lockQueue= lockEntry->shrQueue;
   while( lockQueue != NULL )
   {
     thread= lockQueue->thread;
     UserEntry* userEntry= findUserEntry(O, thread);
     if( userEntry != NULL && userEntry->waitEntry == lockEntry )
     {
       for(int i= 0; i<userEntry->held; i++)
       {
         userBlock= &userEntry->lock[i];
         Dependency dependency= {tail, userEntry, userBlock};
         if( deadlockDependency(O, root, &dependency) )
         {
           debugf("Thread(%lx) wants SHR(%s), holds %s(%s)\n",
                  (long)thread, lockEntry->name,
                  modeName(userBlock->mode), userBlock->lockEntry->name);
           return TRUE;
         }
       }
     }

     lockQueue= lockQueue->next;
   }

   lockQueue= lockEntry->xclQueue;
   while( lockQueue != NULL )
   {
     thread= lockQueue->thread;
     UserEntry* userEntry= findUserEntry(O, thread);
     if( userEntry != NULL && userEntry->waitEntry == lockEntry )
     {
       for(int i= 0; i<userEntry->held; i++)
       {
         userBlock= &userEntry->lock[i];
         Dependency dependency= {tail, userEntry, userBlock};
         if( deadlockDependency(O, root, &dependency) )
         {
           debugf("Thread(%lx) wants XCL(%s), holds %s(%s)\n",
                  (long)thread, lockEntry->name,
                  modeName(userBlock->mode), userBlock->lockEntry->name);
           return TRUE;
         }
       }
     }

     lockQueue= lockQueue->next;
   }

   return FALSE;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       deadlockDetector
//
// Purpose-
//       Abort if a deadlock situation exists.
//
//----------------------------------------------------------------------------
static void
   deadlockDetector(                // Look for deadlock conditions
     Object*           O,           // For this Object
     Mode              mode,        // Requesting this mode
     LockEntry*        lockEntry)   // For this LockEntry
{
   IFHCDM( debugf("[%lx] deadlockDetector(%s(%s))\n",
                  Software::getTid(), modeName(mode), lockEntry->name); )

   UserEntry* userEntry= findUserEntry(O);
   if( userEntry == NULL )          // If we hold no lock
     return;                        // No deadlock condition exists

   for(int i= 0; i<userEntry->held; i++)
   {
     UserBlock* userBlock= &userEntry->lock[i];
     Dependency dependency= {NULL, userEntry, userBlock};
     if( deadlockDependency(O, lockEntry, &dependency) )
     {
       debugf("Thread(%lx) wants %s(%s), holds %s(%s)\n",
              (long)userEntry->thread, modeName(mode), lockEntry->name,
              modeName(userBlock->mode), userBlock->lockEntry->name);
       throwf("ThreadLock(%p)::obtain(%s(%s)) Deadlock", O->threadLock,
              modeName(mode), lockEntry->name);
     }
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       freeLockEntry
//
// Purpose-
//       Release a LockEntry, first removing it from the LockTable
//
//----------------------------------------------------------------------------
static void
   freeLockEntry(                   // Release a LockEntry object
     Object*           O,           // For this Object
     LockEntry*        lockEntry)   // And this LockEntry
{
   unsigned int index= hashf(lockEntry->name) % O->lockCount;
   LockEntry* lastEntry= NULL;
   LockEntry* workEntry= O->lockTable[index];
   while( workEntry != lockEntry )
   {
     lastEntry= workEntry;

     assert( workEntry != NULL );
     workEntry= workEntry->next;
   }

   // Remove lockEntry from list
   if( lastEntry == NULL )
     O->lockTable[index]= lockEntry->next;
   else
     lastEntry->next= lockEntry->next;

   // Release allocated storage
   free(lockEntry->name);
   free(lockEntry);

   updateCounter(O);                // Record name subtraction
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       freeLockQueue
//
// Purpose-
//       Release a LockQueue
//
//----------------------------------------------------------------------------
static void
   freeLockQueue(                   // Release a LockQueue object
     LockQueue*        lockQueue)   // The LockQueue object
{
   delete lockQueue->semaphore;     // Delete semaphore, if any
   free(lockQueue);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       freeUserEntry
//
// Purpose-
//       Release a UserEntry, first removing it from the UserTable
//
//----------------------------------------------------------------------------
static void
   freeUserEntry(                   // Release a UserEntry object
     Object*           O,           // For this Object
     UserEntry*        userEntry)   // The UserEntry object
{
   unsigned int index= userf(userEntry->thread) % O->userCount;
   UserEntry* lastEntry= NULL;
   UserEntry* workEntry= O->userTable[index];
   while( workEntry != userEntry )
   {
     lastEntry= workEntry;

     assert( workEntry != NULL );
     workEntry= workEntry->next;
   }

   // Remove userEntry from list
   if( lastEntry == NULL )
     O->userTable[index]= userEntry->next;
   else
     lastEntry->next= userEntry->next;

   free(userEntry->lock);
   free(userEntry);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       grant
//
// Purpose-
//       Grant lock access (Update list of thread locks)
//
//----------------------------------------------------------------------------
static ThreadLock::Token            // The associated Token
   grant(                           // Grant lock access
     Object*           O,           // For this Object
     Mode              mode,        // And this Mode
     LockEntry*        lockEntry)   // And this LockEntry
{
   // Add the entry to the thread lock recovery array
   UserEntry* userEntry= findUserEntry(O);
   if( userEntry == NULL )
     userEntry= allocUserEntry(O);

   unsigned int held= userEntry->held++;
   if( userEntry->held > userEntry->size )
   {
     //-----------------------------------------------------------------------
     // User lock table expansion (it never contracts)
     IFHCDM( debugf("%4d %s User lock table expansion\n", __LINE__, __FILE__); )

     unsigned size= userEntry->size + ((userEntry->size >= 1024) ? 256 : userEntry->size);
     UserBlock* lock= (UserBlock*)malloc(size * sizeof(UserBlock));
     memset(lock, 0, size * sizeof(UserBlock));
     memcpy(lock, userEntry->lock, held * sizeof(UserBlock));

     free(userEntry->lock);         // Delete the old lock block
     userEntry->lock= lock;         // Activate the new lock block
     userEntry->size= size;         // Set updated element count

     IFHCDM( debugf("%4d %s UserEntry.size(%u)\n", __LINE__, __FILE__, size); )
   }

   UserBlock* userBlock= &userEntry->lock[held];
   userBlock->mode= mode;
   userBlock->lockEntry= lockEntry;

   return (ThreadLock::Token)lockEntry;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       unlock
//
// Purpose-
//       Release a lock
//
//----------------------------------------------------------------------------
static void
   unlock(                          // Release a lock
     Object*           O,           // From this Object
     LockEntry*        lockEntry)   // The lock to release
{
   int                 N;           // UserEntry.lock index (+1)

   // Remove the LockEntry from the UserEntry->lock
   UserEntry* userEntry= findUserEntry(O);

if( userEntry == NULL ){ debugObject(O); debugf("lockEntry(%p)(%s)\n", lockEntry, lockEntry->name); }
   assert( userEntry != NULL );     // Assert thread holds locks

   for(N= userEntry->held; N>0; N--)
   {
     if( userEntry->lock[N-1].lockEntry == lockEntry )
       break;
   }
   assert( N > 0 );                 // Assert thread holds specified lock

   for(int i= N; i<userEntry->held; i++) // Shift down the lock array
     userEntry->lock[i-1]= userEntry->lock[i];
   userEntry->held--;

   if( userEntry->held == 0 )       // If no more locks held
     freeUserEntry(O, userEntry);

   //-------------------------------------------------------------------------
   // Unlock
   if( lockEntry->share > 0 )       // If SHR lock held
   {
     lockEntry->share--;            // Release the SHR lock
     if( lockEntry->share != 0 )    // If more shares exist
       return;

     // Handle queued XCL lock
     LockQueue* lockQueue= lockEntry->xclQueue;
     if( lockQueue != NULL )        // If locks enqueued
     {
       // Find oldest enqueued lock and let it go
       while( lockQueue->next != NULL )
         lockQueue= lockQueue->next;

       post(O, lockEntry, lockQueue);
       return;
     }

     // The LockEntry is no longer in use. Remove it.
     freeLockEntry(O, lockEntry);
   }
   else                             // XCL lock held
   {
     LockQueue* lastQueue= NULL;
     LockQueue* lockQueue= lockEntry->xclQueue;
     while( lockQueue->next != NULL ) // Find actual lock holder
     {
       lastQueue= lockQueue;
       lockQueue= lockQueue->next;
     }

     if( lastQueue != NULL )        // If another requestor present
     {
       lastQueue->next= NULL;
       post(O, lockEntry, lastQueue);
       freeLockQueue(lockQueue);    // Lock remains in XCL mode
     }
     else                           // We are the last XCL holder
     {
       lockEntry->xclQueue= NULL;
       freeLockQueue(lockQueue);

       if( lockEntry->shrQueue != NULL ) // If SHR requestors present
       {
         lockQueue= lockEntry->shrQueue;
         while( lockQueue != NULL )
         {
           post(O, lockEntry, lockQueue);
           lockQueue= lockQueue->next;
         }
       }
       else                         // We are the last lock holder
         freeLockEntry(O, lockEntry); // Remove the LockEntry
     }
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       verify
//
// Purpose-
//       Verify parameters.
//
//----------------------------------------------------------------------------
static inline void
   verify(                          // Verify parameters
     const char*       name)        // The Lock name
{
#if defined(VERIFY)
   if( name == NULL )
     throwf("%4d Invalid ThreadLock::name(<NULL>)", __LINE__);
#endif
}

static inline void
   verify(                          // Verify parameters
     ThreadLock::Token token)       // The Lock Token
{
#if defined(VERIFY)
   if( token == NULL )
     throwf("%4d Invalid ThreadLock::Token(<NULL>)", __LINE__);
#endif
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       waitForLock
//
// Purpose-
//       Wait for lock to become available
//
//----------------------------------------------------------------------------
static void
   waitForLock(                     // Wait for lock to become available
     Object*           O,           // From this Object
     Mode              mode,        // The lock mode
     LockEntry*        lockEntry,   // Our LockEntry
     LockQueue*        lockQueue,   // Our LockQueue
     UserEntry*        userEntry)   // Our UserEntry (when waiting)
{
   //-------------------------------------------------------------------------
   // We must now wait for the lock. Posting the semaphore should guarantee
   // that we have obtained it, but the waitEntry check verifies it.
   while( userEntry->waitEntry != NULL )
   {
     IFHCDM( debugf("Semaphore wait T(%lx) %s(%s)\n", lockQueue->thread,
                    modeName(mode), lockEntry->name); )

     #ifndef USE_LAZY_DEADLOCK_DETECTION
       (void)O;                     // Unused on this path
       (void)mode;                  // Unused on this path (unless HCDM)
       (void)lockEntry;             // Unused on this path (unless HCDM)

       lockQueue->semaphore->wait();  // Wait for posting
       IFHCDM( debugf("...Wait complete T(%lx) %s(%s)\n", lockQueue->thread,
                      modeName(mode), lockEntry->name); )

     #else // defined(USE_LAZY_DEADLOCK_DETECTION)
       int rc= lockQueue->semaphore->wait(15.0);
       IFHCDM( debugf("...Wait complete(%d) T(%lx) %s(%s)\n", rc,
                      lockQueue->thread, modeName(mode), lockEntry->name); )

       //---------------------------------------------------------------------
       // If timeout, test for deadlock now
       if( rc != 0 )
       {
         {{{{
           AutoBarrier lock(O->barrier); // Exclusive table access
           deadlockDetector(O, mode, lockEntry);
         }}}}

         lockQueue->semaphore->wait();  // Wait for posting
         IFHCDM( debugf("...Wait complete T(%lx) %s(%s)\n", lockQueue->thread,
                        modeName(mode), lockEntry->name); )
       }
     #endif

     if( userEntry->waitEntry != NULL ) // The guarantee failed
       debugf("%4d %s Unexpected but handled\n", __LINE__, __FILE__);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       ThreadLock::~ThreadLock
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   ThreadLock::~ThreadLock( void )  // Destructor
{
// IFSCDM( debugf("%4d ThreadLock(%p)::~ThreadLock()\n", __LINE__, this); )

   //-------------------------------------------------------------------------
   // There must not be any locks held at this point.
   Object* O= (Object*)object;
   if( O != NULL )                  // (Should always be TRUE)
   {
     for(unsigned i= 0; i<O->lockCount; i++)
     {
       LockEntry* lockEntry= O->lockTable[i];
       if( lockEntry != NULL )      // If some lock is held
       {
         //-------------------------------------------------------------------
         // ERROR: Lock is held
         debug();

         if( lockEntry->share != 0 ) // If held shared
           throwf("%4d ~ThreadLock() but SHR(%s) %d", __LINE__,
                  lockEntry->name, lockEntry->share);

         // It must be held in exclusive mode, so find the holder
         LockQueue* lastEntry= lockEntry->xclQueue;
         while( lastEntry->next != NULL )
           lastEntry= lastEntry->next;

         throwf("%4d ~ThreadLock() but XCL(%s) %lx)", __LINE__,
                lockEntry->name, (long)lastEntry->thread);
       }
     }

     //-----------------------------------------------------------------------
     // No LockEntry objects exist, we can safely delete the Object
     free(O->lockTable);            // Delete the lockTable
     free(O->userTable);            // Delete the userTable
     free(O);                       // Delete the Object

     object= NULL;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       ThreadLock::ThreadLock
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   ThreadLock::ThreadLock( void )   // Constructor
:  NamedLock()
,  object(NULL)
{
// IFSCDM( debugf("%4d ThreadLock(%p)::ThreadLock()\n", __LINE__, this); )

   Object* O= (Object*)malloc(sizeof(Object));
   memset(O, 0, sizeof(Object));

   // Initialize the Object
   O->threadLock= this;
   O->trigger= MINIMUM_LOCK_COUNT * 4;

   // Create the thread lock table array
   O->lockCount= MINIMUM_LOCK_COUNT;// Initial (default) hash count
   O->lockTable= (LockEntry**)malloc(sizeof(LockEntry*) * O->lockCount);
   memset(O->lockTable, 0, sizeof(LockEntry*) * O->lockCount);
   O->lockCount--;                  // (The last entry is unused)

   // Create the thread lock recovery array
   O->userCount= MINIMUM_USER_COUNT;// Initial (default) hash count
   O->userTable= (UserEntry**)malloc(sizeof(UserEntry*) * O->userCount);
   memset(O->userTable, 0, sizeof(UserEntry*) * O->userCount);
   O->userCount--;                  // (The last entry is unused)

   this->object= O;
}

//----------------------------------------------------------------------------
//
// Method-
//       ThreadLock::debug
//
// Purpose-
//       Debugging display.
//
//----------------------------------------------------------------------------
void
   ThreadLock::debug( void ) const  // Debugging display
{
   Object* O= (Object*)object;

   debugf("%4d ThreadLock(%p)::debug\n", __LINE__, this);

   AutoBarrier lock(O->barrier);    // Exclusive table access
   debugObject(O);                  // Object debugging display
}

//----------------------------------------------------------------------------
//
// Method-
//       ThreadLock::attempt
//
// Purpose-
//       Attempt to obtain a Lock, returning immediately.
//
//----------------------------------------------------------------------------
ThreadLock::Token                   // The lock token, or NULL
   ThreadLock::attemptSHR(          // Attempt to obtain a SHR lock
     const char*       name)        // For this Lock name
{
   Object* O= (Object*)object;

   verify(name);                    // Verify parameters

   AutoBarrier lock(O->barrier);    // Exclusive table access

   LockEntry* lockEntry= findLockEntry(O, name);
   if( lockEntry != NULL )        // If the lock is held
   {
     if( lockEntry->xclQueue != NULL ) // If the lock is held XCL
       return NULL;               // RE-jected

     lockEntry->share++;        // Increment share count
   }
   else
   {
     // Attempt to obtain a lock that is not held
     lockEntry= allocLockEntry(O, name); // Allocate lock entry
     lockEntry->share= 1;
   }

   return grant(O, SHR, lockEntry);
}

ThreadLock::Token                   // The lock token, or NULL
   ThreadLock::attemptXCL(          // Attempt to obtain an XCL lock
     const char*       name)        // For this Lock name
{
   Object* O= (Object*)object;

   verify(name);                    // Verify parameters

   AutoBarrier lock(O->barrier);    // Exclusive table access

   LockEntry* lockEntry= findLockEntry(O, name);
   if( lockEntry != NULL )        // If the lock is held
     return NULL;                 // RE-jected

   // Attempt to obtain a lock that is not held
   lockEntry= allocLockEntry(O, name); // Allocate lock entry
   lockEntry->xclQueue= allocLockQueue(); // Indicate XCL lock

   return grant(O, XCL, lockEntry);
}

//----------------------------------------------------------------------------
//
// Method-
//       ThreadLock::modify
//
// Purpose-
//       Attempt to modify the lock Mode, returning immmediately.
//
//----------------------------------------------------------------------------
ThreadLock::Token                   // *ALWAYS* returns token
   ThreadLock::modifySHR(           // Modify (downgrade)
     Token             token)       // For this Lock
{
   Object* O= (Object*)object;

   verify(token);                   // Verify parameters

   AutoBarrier lock(O->barrier);    // Exclusive table access
   LockEntry* lockEntry= (LockEntry*)token;

   // Downgrade request: Always succeeds
   if( lockEntry->share == 0 )      // If held in XCL mode
   {
     // Locate and remove the lockQueue entry
     LockQueue* lastQueue= NULL;
     LockQueue* lockQueue= lockEntry->xclQueue;
     assert( lockQueue != NULL );
     while( lockQueue->next != NULL )
     {
       lastQueue= lockQueue;

       assert( lockQueue != NULL );
       lockQueue= lockQueue->next;
     }

     if( lastQueue == NULL )
       lockEntry->xclQueue= NULL;
     else
       lastQueue->next= NULL;

     // Change mode in user table
     UserEntry* userEntry= findUserEntry(O, lockQueue->thread);
     assert( userEntry != NULL );
     for(int i= userEntry->held; i>0; i--)
     {
       if( lockEntry == userEntry->lock[i-1].lockEntry )
       {
         userEntry->lock[i-1].mode= SHR;
         break;
       }
     }

     lockEntry->share= 1;           // Lock now held shared
     freeLockQueue(lockQueue);      // Remove dangling reference
   }

   return token;                    // Downgrade successful
}

ThreadLock::Token                   // NULL if lock cannot be upgraded
   ThreadLock::modifyXCL(           // Modify (upgrade)
     Token             token)       // This ThreadLock
{
   Object* O= (Object*)object;

   verify(token);                   // Verify parameters

   AutoBarrier lock(O->barrier);    // Exclusive table access
   LockEntry* lockEntry= (LockEntry*)token;

   // Upgrade request: Succeeds if we are only lock holder
   if( lockEntry->share <= 1 )
   {
     if( lockEntry->share == 0 )    // Convert from XCL to XCL?
       return token;                // OK, why not?

     // Locate the last lockQueue entry
     LockQueue* lastQueue= lockEntry->xclQueue;
     if( lastQueue != NULL )
     {
       while( lastQueue->next != NULL )
       {
         assert( lastQueue != NULL );
         lastQueue= lastQueue->next;
       }
     }

     LockQueue* lockQueue= allocLockQueue();
     if( lastQueue == NULL )
       lockEntry->xclQueue= lockQueue;
     else
       lastQueue->next= lockQueue;

     // Change mode in user table
     UserEntry* userEntry= findUserEntry(O, lockQueue->thread);
     assert( userEntry != NULL );
     for(int i= userEntry->held; i>0; i--)
     {
       if( lockEntry == userEntry->lock[i-1].lockEntry )
       {
         userEntry->lock[i-1].mode= XCL;
         break;
       }
     }

     lockEntry->share= 0;           // Lock now held exclusively
     return token;                  // Upgrade successful
   }

   return NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       ThreadLock::obtain
//
// Purpose-
//       Obtain a Lock, waiting if necessary until the lock becomes available.
//
//----------------------------------------------------------------------------
ThreadLock::Token                   // The lock token
   ThreadLock::obtainSHR(           // Obtain a SHR lock
     const char*       name)        // For this Lock name
{
   Object*             O= (Object*)object;
   LockEntry*          lockEntry;   // Our LockEntry
   LockQueue*          lockQueue;   // Our LockQueue
   UserEntry*          userEntry;   // Our UserEntry (when waiting)

   verify(name);                    // Verify parameters

   {{{{
     AutoBarrier lock(O->barrier);    // Exclusive table access

     lockEntry= findLockEntry(O, name); // Find existing lock entry
     if( lockEntry == NULL )        // If the lock is not held
     {
       lockEntry= allocLockEntry(O, name); // Add to head of list
       lockEntry->share= 1;
       return grant(O, SHR, lockEntry);
     }

     // The lock is already held
     if( lockEntry->xclQueue == NULL ) // If no exclusive requests pending
     {
       lockEntry->share++;          // Increment share count
       return grant(O, SHR, lockEntry);
     }

     // The lock is not immediately available. Prepare to wait for it.
     userEntry= findUserEntry(O);
     if( userEntry == NULL )          // If no user entry
       userEntry= allocUserEntry(O);  // Allocate one (with held == 0)

     userEntry->waitEntry= lockEntry; // Indicate waiting for lock

     lockQueue= allocLockQueue();
     lockQueue->next= lockEntry->shrQueue;
     lockEntry->shrQueue= lockQueue;
     lockQueue->semaphore= new Semaphore(0);

     #ifndef USE_LAZY_DEADLOCK_DETECTION
       deadlockDetector(O, SHR, lockEntry);
     #endif
   }}}}

   //-------------------------------------------------------------------------
   // We must now wait for the lock. (No Barrier)
   waitForLock(O, SHR, lockEntry, lockQueue, userEntry);

   // Done waiting. (None of our working storage can move)
   AutoBarrier lock(O->barrier);    // Regain exclusive table access

   lockEntry->share++;              // Indicate SHR lock held

   // We must now remove and delete our LockQueue entry
   LockQueue* lastQueue= NULL;
   LockQueue* workQueue= lockEntry->shrQueue;
   while( workQueue != lockQueue )
   {
     lastQueue= workQueue;

     assert( workQueue != NULL );
     workQueue= workQueue->next;
   }

   if( lastQueue == NULL )
     lockEntry->shrQueue= lockQueue->next;
   else
     lastQueue->next= lockQueue->next;

   freeLockQueue(lockQueue);

   return grant(O, SHR, lockEntry);
}

ThreadLock::Token                   // The lock token
   ThreadLock::obtainXCL(           // Obtain an XCL lock
     const char*       name)        // For this Lock name
{
   Object*             O= (Object*)object;
   LockEntry*          lockEntry;   // Our LockEntry
   LockQueue*          lockQueue;   // Our LockQueue
   UserEntry*          userEntry;   // Our UserEntry (when waiting)

   verify(name);                    // Verify parameters

   {{{{
     AutoBarrier lock(O->barrier);    // Exclusive table access

     lockEntry= findLockEntry(O, name); // Find existing lock entry
     if( lockEntry == NULL )        // If the lock is not held
     {
       lockEntry= allocLockEntry(O, name); // Add to head of list
       lockEntry->xclQueue= allocLockQueue();

       return grant(O, XCL, lockEntry);
     }

     // The lock is not immediately available. Prepare to wait for it.
     userEntry= findUserEntry(O);
     if( userEntry == NULL )          // If no user entry
       userEntry= allocUserEntry(O);  // Allocate one (with held == 0)

     userEntry->waitEntry= lockEntry; // Indicate waiting for lock

     lockQueue= allocLockQueue();
     lockQueue->next= lockEntry->xclQueue;
     lockEntry->xclQueue= lockQueue;
     lockQueue->semaphore= new Semaphore(0);

     #ifndef USE_LAZY_DEADLOCK_DETECTION
       deadlockDetector(O, XCL, lockEntry);
     #endif
   }}}}

   //-------------------------------------------------------------------------
   // We must now wait for the lock. (No Barrier)
   waitForLock(O, XCL, lockEntry, lockQueue, userEntry);

   // Done waiting. (None of our working storage can move)
   AutoBarrier lock(O->barrier);    // Regain exclusive table access

   // We already have the queue entry and it's properly positioned.
   // Since we only wait once for a lock, we can remove the semaphore.
   delete lockQueue->semaphore;
   lockQueue->semaphore= NULL;

   return grant(O, XCL, lockEntry);
}

//----------------------------------------------------------------------------
//
// Method-
//       ThreadLock::release
//
// Purpose-
//       Release a Lock.
//
//----------------------------------------------------------------------------
void
   ThreadLock::release(             // Release
     Token             token)       // This Lock
{
   Object* O=          (Object*)object;
   LockEntry*          lockEntry= (LockEntry*)token;

   AutoBarrier lock(O->barrier);    // Exclusive table access
   unlock(O, lockEntry);
}

//----------------------------------------------------------------------------
//
// Method-
//       ThreadLock::threadAbort
//
// Purpose-
//       Release all locks held by the specified Thread.
//
//----------------------------------------------------------------------------
void
   ThreadLock::threadAbort(         // Release all held locks
     unsigned long     thread)      // Held with this thread id
{
   Object* O= (Object*)object;
   unsigned int index= userf(thread) % O->userCount;

   UserEntry* userEntry= O->userTable[index];
   while( userEntry != NULL )
   {
     if( userEntry->thread == intptr_t(thread) )
       break;

     userEntry= userEntry->next;
   }

   // If locks are held
   if( userEntry != NULL )
   {
     int held= userEntry->held;
     while( held > 0 )
     {
       held--;
       unlock(O, userEntry->lock[held].lockEntry);
     }
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       ThreadLock::threadExit
//
// Purpose-
//       Release all locks held by the current Thread.
//
//----------------------------------------------------------------------------
void
   ThreadLock::threadExit( void )   // Release all held locks
{
   threadAbort(Software::getTid());
}

