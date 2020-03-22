//----------------------------------------------------------------------------
//
//       Copyright (C) 2013 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       AM_Node.java
//
// Purpose-
//       Alpha Memory object
//
// Last change date-
//       2013/01/01
//
//----------------------------------------------------------------------------
import java.lang.*;
import java.util.*;
import user.util.*;

//----------------------------------------------------------------------------
//
// Class-
//       AM_Node
//
// Purpose-
//       Alpha Memory object
//
// Reference-
//       alpha-memory
//
//----------------------------------------------------------------------------
public class AM_Node extends MapDebugAdaptor { // Alpha Memory object
//----------------------------------------------------------------------------
// AM_Node.Attributes
//----------------------------------------------------------------------------
static int             globalSN= 0; // The GLOBAL serial number
int                    objectSN;    // The OBJECT serial number

WME_Key                key;         // TODO: Remove, debugging only
Vector<AME>            items;       // List of AME's
Vector<Node>           successors;  // List of successor JoinNodes or NegNodes
int                    reference_count;  // Reference counter

//----------------------------------------------------------------------------
//
// Method-
//       AM_Node.AM_Node
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
public
   AM_Node(                         // Default constructor
     WME_Key           key)         // TODO: Remove (For debugging only)
{
   super();

   this.key= key;                   // TODO: Remove (for debugging only)
   items= new Vector<AME>();
   successors= new Vector<Node>();
   reference_count= 0;

   objectSN= globalSN++;            // FIRST AM_Node is DUMMY
   if( SCDM && INFO ) debugln("new " + getReferKEY());
}

//----------------------------------------------------------------------------
//
// Method-
//       AM_Node.Accessor methods
//
// Purpose-
//       Accessor methods
//
//----------------------------------------------------------------------------
public boolean                      // TRUE iff items.size() != 0
   isItemsExist( )                  // Do items exist?
{  return items.size() != 0; }

public String
   getReferKEY( )
{  return getReference()+( key==null ? "{<NULL>}" : key); }

//----------------------------------------------------------------------------
//
// Method-
//       AM_Node.MapDebug
//
// Purpose-
//       Override MapDebugAdaptor methods
//
//----------------------------------------------------------------------------
public void
   debug(                           // Debugging display
     DebugMap          map)         // DebugMap extention
{
   super.debug(map);                // Display this entry

   debugln(".. key: " + key);
   debugln(".. refs: " + reference_count);

   debugln(".. items: " + items.size());
   for(Iterator<AME> i= items.iterator(); i.hasNext();)
   {
     AME ame= i.next();
     if( ame.wme != null )
       map.debug(".... ", ame, ":" + ame.wme.getReferWME());
     else
       map.debug(".... ", ame, ":<NULL>");
   }

   debugln(".. joins: " + successors.size());
   for(Iterator<Node> i= successors.iterator(); i.hasNext();)
   {
     Object o= i.next();
     map.debug(o);
   }
}

public int                          // The object serial number
   getObjectSN( )                   // Get object serial number
{  return objectSN; }

//----------------------------------------------------------------------------
//
// Method-
//       AM_Node.addSuccessor
//
// Purpose-
//       Insert a Node onto the successor Node list
//
//----------------------------------------------------------------------------
public void
   addSuccessor(                    // Add successor JoinNode/NegNode
     Node              node,        // The Node to insert
     Node              before)      // Insert BEFORE this node, null for HEAD
{
   if( HCDM ) verify( successors.indexOf(node) < 0 );

   int index= 0;                    // DEFAULT, add to HEAD
   if( before != null )             // If add at position
     index= successors.indexOf(before);

   successors.add(index, node);
}

public void
   addSuccessor(                    // Add successor JoinNode/NegNode
     Node              node)        // The Node to insert
{
   if( HCDM ) verify( successors.indexOf(node) < 0 );

   successors.add(node);            // Add to TAIL
}

//----------------------------------------------------------------------------
//
// Method-
//       AM_Node.delSuccessor
//
// Purpose-
//       Remove a Node from the successor Node list
//
//----------------------------------------------------------------------------
public void
   delSuccessor(                    // Remove successor JoinNode/NegNode
     Node              node)        // The Node to remove
{
   successors.remove(node);         // Remove the Node
}

//----------------------------------------------------------------------------
//
// Method-
//       AM_Node.insertAME
//
// Purpose-
//       Insert an AME to the HEAD of the AME list
//
// Reference-
//       alpha-memory-activation (From Rete.activateAME)
//
//----------------------------------------------------------------------------
public void
   insertAME(                       // Insert AME on HEAD of AME list
     AME               ame)         // The AME to insert
{
   if( HCDM && SCDM && INFO ) debugln(getReferKEY()+".insertAME("+ame.getReferWME()+")");

   items.add(0, ame);

   WME wme= ame.getWME();           // Get associated WME
   for(Iterator<Node> i= successors.iterator(); i.hasNext();)
   {
     JoinNode join= (JoinNode)i.next(); // TODO: Handle NegNode elements
     join.activateRHS(wme);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       AM_Node.removeAME
//
// Purpose-
//       Remove an AME from the AME list
//
// Reference-
//       remove-wme (**ONLY** From AME.delete)
//
//----------------------------------------------------------------------------
public void
   removeAME(                       // Remove AME from the AME list
     AME               ame)         // The AME to remove
{
   items.remove(ame);

   if( items.size() == 0 )
   {
     for(Iterator<Node> i= successors.iterator(); i.hasNext();)
     {
       Node node= i.next();
       if( node instanceof JoinNode ) // (Don't unlink negative nodes
         node.unlink();
     }
   }
}
} // class AM_Node

