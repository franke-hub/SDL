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
//       BM_Node.java
//
// Purpose-
//       Beta Memory Node.
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
//       BM_Node
//
// Purpose-
//       Beta Memory Node.
//
// Reference-
//       beta-memory
//
//----------------------------------------------------------------------------
public class BM_Node extends Node { // Beta Memory Node
//----------------------------------------------------------------------------
// BM_Node.Attributes
//----------------------------------------------------------------------------
static int             globalSN= 0; // The GLOBAL serial number
int                    objectSN;    // The OBJECT serial number

Vector<Token>          items;       // List of Tokens
Vector<Node>           all_children; // List of ALL child nodes

//----------------------------------------------------------------------------
//
// Method-
//       BM_Node.BM_Node
//
// Purpose-
//       Constructor
//
// Implementation notes-
//       ONLY called from Rete.insertBeta (build-or-share-beta-memory-node)
//
//----------------------------------------------------------------------------
public
   BM_Node(                         // Constructor
     JoinNode          parent)      // The parent JoinNode
{
   super(parent);

   items= new Vector<Token>();
   all_children= new Vector<Node>();

   objectSN= globalSN++;            // First BM_Node is DUMMY
   if( SCDM && INFO ) debugln("new " + getReference());
}

//----------------------------------------------------------------------------
//
// Method-
//       BM_Node.MapDebug
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

   debugln(".. items: " + items.size());
   for(Iterator<Token> i= items.iterator(); i.hasNext();)
   {
     Token token= i.next();
     map.debug(".... ", token, token.wme == null ? "{<NULL>}" : token.wme.toString());
   }

   debugln(".. all_children: " + all_children.size());
   for(Iterator<Node> i= all_children.iterator(); i.hasNext();)
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
//       BM_Node.Accessors
//
// Purpose-
//       Accessor methods
//
//----------------------------------------------------------------------------
public boolean                      // TRUE iff items.size() != 0
   isItemsExist( )                  // Do items exist?
{  return items.size() != 0; }

//----------------------------------------------------------------------------
//
// Method-
//       BM_Node.activateLHS
//
// Purpose-
//       LEFT activation
//
// Reference-
//       beta-memory-left-activation
//
//----------------------------------------------------------------------------
public void
   activateLHS(                     // LEFT activate this BM_Node
     Token             parent,      // Activation parent Token
     WME               wme)         // Associated WME
{
   if( SCDM && INFO ) debugln(getReference()+".activateLHS("+parent.getReferWME()+","+(wme==null?"<NULL>":wme.getReferWME())+")");

   //=========================================================================
   // ADDED: Trim Token tree if wme == null or parent.wme == null
   if( wme == null || parent.wme == null )
     parent= null;
   //=========================================================================

// Token token= new Token(this, parent, wme); // (Parameters reordered)
   Token token= new Token(parent, this, wme); // Create a new child Token
   items.add(0, token);             // Add it to HEAD of token list

   for(Iterator<Node> i= getChildren().iterator(); i.hasNext();)
   {
     JoinNode join= (JoinNode)i.next();
     join.activateLHS(token);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       BM_Node.initialize
//
// Purpose-
//       Update parent Node with matches from above
//       Note: THIS Node is the parent node
//
// Reference-
//       update-new-node-with-matches-from-above
//       (Overrides Node.initialize)
//
//----------------------------------------------------------------------------
public void
   initialize(                      // Update THIS Node with matches from above
     Node              child)       // The new child JoinNode
{
   if( SCDM && INFO ) debugln(getReference()+".initialize("+child.getReference()+")");

   verify( false );                 // Is this method ever invoked?

   JoinNode join= (JoinNode)child;
   for(Iterator<Token> i= items.iterator(); i.hasNext();)
   {
     Token token= i.next();
     join.activateLHS(token);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       BM_Node.insertJoin
//
// Purpose-
//       Create or share a JoinNode
//
// Reference-
//       build-or-share-join-node
//
// Implementation notes-
//       ONLY called from Rete.insertJoin (build-or-share-join-node)
//
//----------------------------------------------------------------------------
public JoinNode                     // Resultant JoinNode
   insertJoin(                      // Insert JoinNode
     AM_Node           amem,        // The associated AM_Node
     Vector<JoinTest>  testList)    // The list of JoinTests
{
   if( SCDM && INFO ) debugln(getReference()+".insertJoin("+amem.getReference()+","+testList.size()+")");

   // Look for an existing JoinNode to share
   for(Iterator<Node> i= all_children.iterator(); i.hasNext();)
   {
     Node node= i.next();
     if( node instanceof JoinNode )
     {
       JoinNode join= (JoinNode)node;
       if( join.amem == amem && join.tests.equals(testList) )
         return join;
     }
   }

   // No existing JoinNode found. Create and initialize a new one.
   JoinNode join= new JoinNode(this, amem, testList);

   all_children.add(0, join);       // Add at HEAD of all_children list
   amem.reference_count++;          // Update reference count

   // Unlink right away if either memory is empty
   if( items.size() == 0 )
     join.unlinkRHS();
   else if( amem.items.size() == 0 )
     join.unlinkLHS();

   return join;
}
} // class BM_Node

