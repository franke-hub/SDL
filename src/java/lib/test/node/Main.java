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
//       Main.java
//
// Purpose-
//       Test user.node Objects.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
package test.node;

import  user.node.*;

//----------------------------------------------------------------------------
//
// Class-
//       Main
//
// Purpose-
//       Test user.node Objects.
//
//----------------------------------------------------------------------------
public class Main
{
//----------------------------------------------------------------------------
// Main.attributes
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Method-
//       Main.Main
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
public
   Main( )                         // Default constructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       Main.main
//
// Purpose-
//       Test driver.
//
//----------------------------------------------------------------------------
public static void
   main(                            // Mainline code
     String[]          args)        // Argument array
   throws Exception
{
   Node[]              node= new Node[32];
   Fanin[]             fanin= new Fanin[32];

   for(int i= 0; i<node.length; i++)
   {
     fanin[i]= new Fanin();
     node[i]= new MyNode();
     node[i].set(fanin[i]);
     fanin[0].add(node[i]);
   }

   node[0].clock(1);
}
} // class Main

class MyNode extends Node
{
public Fanout                       // Extract the Fanout object
   resolve( )                       // Extract the Fanout object
{
   int                 M= fanin.size(); // Number of FANIN elements

   display("resolve()");
   for(int i= 0; i<M; i++)
   {
     Node node= fanin.get(i);
     Fanout fanout= node.get();
   }

   return new Fanout();
}
} // class MyNode

