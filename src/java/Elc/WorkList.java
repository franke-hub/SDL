//----------------------------------------------------------------------------
//
//       Copyright (C) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       WorkList.java
//
// Purpose-
//       Define a "to do" list, comprised of WorkItem objects.
//
// Last change date-
//       2007/01/01
//
// Notes-
//       Requires: Scheduler.
//
//       A WorkList processes one (and only one) WorkItem at a time, thus
//       making the WorkList inherently synchronized.
//
//       WorkItems are added to the WorkList using WorkList.todo(). This
//       occurs at any time. When the first WorkItem is added to the WorkList,
//       it is activated and begins processing the WorkItems. WorkItems are
//       handled serially in the order that they were received.
//
//----------------------------------------------------------------------------
import java.io.*;
import java.lang.*;
import java.util.*;

import user.util.*;
import user.util.logging.*;

//----------------------------------------------------------------------------
//
// Class-
//       WorkList
//
// Purpose-
//       Describe a unit of work that is run by a WorkList
//
//----------------------------------------------------------------------------
class WorkList {                    // WorkList descriptor
//----------------------------------------------------------------------------
// WorkList.Controls
//----------------------------------------------------------------------------
final static boolean   HCDM= false; // Hard Core Debug Mode?
final static boolean   IODM= HCDM;  // I/O Debug Mode?
final static boolean   SCDM= HCDM;  // Soft Core Debug Mode?

static StreamLogger    logger= Common.get().logger;

//----------------------------------------------------------------------------
// WorkList.Attributes
//----------------------------------------------------------------------------
LinkedList<WorkItem>   list= new LinkedList<WorkItem>(); // The list of WorkItems

//----------------------------------------------------------------------------
//
// Method-
//       WorkList.WorkList
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   WorkList( )                      // Default constructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       WorkList.next
//
// Purpose-
//       Process a WorkItem.
//       Override this method in derived WorkList Objects.
//
//----------------------------------------------------------------------------
protected void
   next(                            // Process a WorkItem
     WorkItem          item)        // The WorkItem
{
   if( HCDM )
     logger.log("WorkList(" + toString() + ") " +
                "next(" + item.toString() + ")");
}

//----------------------------------------------------------------------------
//
// Method-
//       WorkList.empty
//
// Purpose-
//       Process WorkItems
//
//----------------------------------------------------------------------------
public final void
   empty( )                         // Process WorkItems
{
   WorkItem            item;        // The current WorkItem

   if( HCDM )
     logger.log("WorkList(" + toString() + ") empty()");

   synchronized (this)
   {{{{
     item= list.peek();
   }}}}

   while( item != null )
   {
     next(item);

     synchronized (this)
     {{{{
       list.poll();
       item= list.peek();
     }}}}
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       WorkList.todo
//
// Purpose-
//       Add a WorkItem to the WorkList.
//
//----------------------------------------------------------------------------
public final void
   todo(                            // Add a WorkItem to the WorkList
     WorkItem          item)        // The WorkItem
{
   if( HCDM )
     logger.log("WorkList(" + toString() + ") " +
                "todo(" + item.toString() + ")");

   Scheduler scheduler= null;
   synchronized (this)
   {{{{
     if( list.peek() == null )
       scheduler= Scheduler.get();

     list.add(item);
   }}}}

   if( scheduler != null )
     scheduler.schedule(this);
}
} // class WorkList

