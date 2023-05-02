//-----------------------------------------------------------------------------
//
//       Copyright (c) 2019 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//-----------------------------------------------------------------------------
//
// Title-
//       DispatchTask.cs
//
// Purpose-
//       Dispatcher Task Object
//
// Last change date-
//       2019/02/15
//
//-----------------------------------------------------------------------------
#define USE_CHECKING                // Use internal consistency checks?
#define USE_WORKER_POOL             // Use WorkerPool (not ThreadPool)?

using System;
using System.Collections.Generic;   // For List
using System.Threading;             // For ThreadPoool

using Shared;                       // For Debug

namespace Shared.Dispatch {         // The Dispatcher namespace
//-----------------------------------------------------------------------------
//
// Class-
//       (Shared.Dispatch.)Task
//
// Purpose-
//       Task Object.
//
//-----------------------------------------------------------------------------
public class Task : Worker {        // Task Object
//-----------------------------------------------------------------------------
// Task.Attributes
//-----------------------------------------------------------------------------
Item                   fake;        // A fake Item, prevents extra scheduling
Queue<Item>            itemQ;       // The Item Queue

#if USE_CHECKING
int                    threads= 0;  // Number of threads (consistency check)
#endif

//-----------------------------------------------------------------------------
// Task.Constructors
//-----------------------------------------------------------------------------
public Task( )
{
   this.fake= new Item();
   this.itemQ= new Queue<Item>();
}

//-----------------------------------------------------------------------------
// Task.Methods
//-----------------------------------------------------------------------------
Item[]                              // The list of work elements, null if none
   get_work( )                      // Get list of work elements
{
   Item[] itemA;                    // The itemQ converted to an array

   lock(itemQ) {
       itemA= itemQ.ToArray();
       if( itemA.Length != 0 ) {
           itemQ.Clear();
           itemQ.Enqueue(fake);
#if USE_CHECKING
           threads++; Debug.assert( threads == 1 );
#endif

       }
   }

   return itemA;
}

public void
   run( )                           // Drain work from this Task
{  drain(null); }

public void
   drain(Object ignored)            // Drain work from this Task
{
   Dispatch.trace("Task.drain()");

   Item[] itemA= get_work();
   while( itemA.Length != 0 )
   {
       foreach(Item item in itemA)
           work(item);

       lock(itemQ) {
           Item item= itemQ.Dequeue();
           Debug.assert( item == fake );

#if USE_CHECKING
           threads--; Debug.assert( threads == 0 );
#endif
           itemA= get_work();
       }
   }
}

public void
   enqueue(                         // Add
     Item              item)        // This Item to list of work items
{
   Dispatch.trace("Task.enqueue()");
   int count= 0;

   lock(itemQ) {
       count= itemQ.Count;
       itemQ.Enqueue(item);
   }

   if( count == 0 ) {               // If the queue was empty
#if USE_WORKER_POOL
       WorkerPool.work(this);
#else
       ThreadPool.QueueUserWorkItem(drain);
#endif
   }
}

public void
   reset( )                         // Reset the Task
{
   lock(itemQ) {
       foreach(Item item in itemQ)
           item.post((int)Item.CC.PURGE);
       itemQ.Clear();
   }
}

public virtual void                 // OVERRIDE this method
   work(                            // Process one Item
     Item              item)        // The Item to process
{  item.post(); }                   // Just indicate work complete
}  // class Task
}  // namespace Shared.Dispatch
