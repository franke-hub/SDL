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
//       Node.java
//
// Purpose-
//       Relate multiple Fanin Objects to one Fanout Object.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
package user.node;

import  user.node.*;

//----------------------------------------------------------------------------
//
// Class-
//       Node
//
// Purpose-
//       Relate multiple Fanin Objects to one Fanout Object.
//
//----------------------------------------------------------------------------
public class Node
{
//----------------------------------------------------------------------------
// Node.attributes
//----------------------------------------------------------------------------
protected Fanin        fanin;       // Fanin Object Vector
protected Fanout       fanout;      // Fanout Object
protected int          cycleTime;   // Number of delay cycles
private long           lastClock;   // Last clock value
private long           nextClock;   // Next clock value
private static Fanin   privateFanin=  new Fanin();  // Default Fanin
private static Fanout  privateFanout= new Fanout(); // Default Fanout

public void display(String string)
{  System.out.println("node::" + toString() + ":" + string);
}

//----------------------------------------------------------------------------
//
// Method-
//       Node.Node
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
public
   Node( )                         // Default constructor
{
   display("Node()");
   fanin= privateFanin;
   fanout= privateFanout;
   cycleTime= 1;
}

//----------------------------------------------------------------------------
//
// Method-
//       Node.clock
//
// Purpose-
//       Clock the Node.
//
//----------------------------------------------------------------------------
public final int                    // The minimum clock delay
   clock(                           // Clock the Node
     long              master)      // Master clock value
{
   display("clock(" + master + ")");
   if( master <= lastClock )        // If already clocked
     return cycleTime;

   int                 M= fanin.size();      // Number of FANIN elements
   int                 minClock;    // Next clock time
   int                 curClock;    // Current node clock time

   lastClock= master;               // Only drive once per cycle
   minClock= cycleTime;             // Number of delay cycles
   if( nextClock > lastClock )
     minClock= (int)(nextClock - lastClock);

   for(int i= 0; i<M; i++)
   {
     Node n= fanin.get(i);
     curClock= n.clock(master);
     if( curClock < minClock )
       minClock= curClock;
   }

   if( master >= nextClock )
   {
     fanout= resolve();
     nextClock= master + cycleTime;
   }

   return minClock;
}

//----------------------------------------------------------------------------
//
// Method-
//       Node.get
//
// Purpose-
//       Extract the Fanout Object.
//
//----------------------------------------------------------------------------
public final Fanout                 // Extract the Fanout object
   get( )                           // Extract the Fanout object
{
   return fanout;
}

//----------------------------------------------------------------------------
//
// Method-
//       Node.set
//
// Purpose-
//       Replace the Fanin Object.
//
//----------------------------------------------------------------------------
public final void
   set(                             // Replace the Fanin object
     Fanin             fanin)       // Replacement Fanin object
{
   this.fanin= fanin;
}

//----------------------------------------------------------------------------
//
// Method-
//       Node.resolve
//
// Purpose-
//       Extract the Fanout Object.
//
// Override-
//       This returns the Empty Fanout object.
//
//----------------------------------------------------------------------------
public Fanout                       // Extract the Fanout object
   resolve( )                       // Extract the Fanout object
{
   display("resolve() - base class");
   return null;
}
} // class Node

