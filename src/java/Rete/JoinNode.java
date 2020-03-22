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
//       JoinNode.java
//
// Purpose-
//       Join Node.
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
//       JoinNode
//
// Purpose-
//       Join Node.
//
// Reference-
//       join-node
//
// Modifications-
//       linkedRHS <== <ADDED> per footnote page 103
//       linkedLHS <== <ADDED> for symmetry. Identical to isLinked.
//
//----------------------------------------------------------------------------
public class JoinNode extends Node { // Join Node
//----------------------------------------------------------------------------
// JoinNode.Attributes
//----------------------------------------------------------------------------
Condition              cond;        // TODO: REMOVE: DEBUGGING ONLY

static int             globalSN= 0; // The GLOBAL serial number
int                    objectSN;    // The OBJECT serial number

boolean                linkedRHS;   // TRUE iff right linked (to Alpha Memory)
boolean                linkedLHS;   // TRUE iff left  linked (to Beta Memory)
AM_Node                amem;        // Associated AM_Node
Vector<JoinTest>       tests;       // List of tests

// Nearest ancestor with same AM_Node
JoinNode               nearest_ancestor_with_same_amem;

//----------------------------------------------------------------------------
//
// Method-
//       JoinNode.JoinNode
//
// Purpose-
//       Constructor
//
// Implementation notes-
//       ONLY called from BM_Node.insertJoin (build-or-share-join-node)
//
//----------------------------------------------------------------------------
public
   JoinNode(                        // Constructor
     BM_Node           parent,      // Parent BM_Node
     AM_Node           amem,        // Associated AM_Node
     Vector<JoinTest>  tests)       // Ths list of tests
{
   super(parent);
   linkedLHS= true;

   this.amem= amem;
   this.tests= tests;
   if( parent != null )             // If not dummy node
     this.nearest_ancestor_with_same_amem= parent.nearestParent(amem);

   if( amem != null )               // If not dummy node
     amem.addSuccessor(this, null); // Add at HEAD of amem.successors list
   linkedRHS= true;

   objectSN= globalSN++;            // FIRST JoinNode is DUMMY
   if( SCDM && INFO ) { debugln("new " + getReference()); debug(); }
}

//----------------------------------------------------------------------------
//
// Method-
//       JoinNode.delete
//
// Purpose-
//       Destructor
//
//----------------------------------------------------------------------------
public void
   delete( )                        // Destructor
{
   if( parent != null )
     ((BM_Node)parent).all_children.remove(this);

   super.delete();
}

//----------------------------------------------------------------------------
//
// Method-
//       JoinNode.Accessor methods
//
// Purpose-
//       Accessor methods
//
//----------------------------------------------------------------------------
// None defined

//----------------------------------------------------------------------------
//
// Method-
//       JoinNode.MapDebug
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

   debugln(".. linkedRHS: " + linkedRHS);
   debugln(".. linkedLHS: " + linkedLHS);
   if( cond != null ) debugln(".. cond: " + cond.getReferWME());
   map.debug(".. amem: ", amem, amem.key.toString());
   if( nearest_ancestor_with_same_amem == null )
     map.debug(".. near: ", nearest_ancestor_with_same_amem);
   else
     map.debug(".. near: ", nearest_ancestor_with_same_amem.getReference());

   if( tests == null )              // If dummmy node
     map.debug(".. tests: ", tests);
   else
   {
     debugln(".. tests: " + tests.size());
     for(Iterator<JoinTest> i= tests.iterator(); i.hasNext();)
     {
       JoinTest test= i.next();
       map.debug(".... ", test, test.toString());
     }
   }
}

public int                          // The object serial number
   getObjectSN( )                   // Get object serial number
{  return objectSN; }

//----------------------------------------------------------------------------
//
// Method-
//       JoinNode.activateLHS
//
// Purpose-
//       LEFT activation
//
// Reference-
//       join-node-left-activation (page  25)
//       join-node-left-activation (page  32)
//       join-node-left-activation (page  88) just-became-nonempty
//       join-node-left-activation (page 103)
//       join-node-left-activation (page 152)
//
//----------------------------------------------------------------------------
public void
   activateLHS(                     // LEFT activate this JoinNode
     Token             token)       // Activation Token
{
   if( SCDM && INFO ) debugln(getReference()+".activateLHS("+token.getReferWME()+")");

   if( linkedRHS == false )
   {
     relinkRHS();                   // Relink to alpha memory

     if( amem.isItemsExist() == false ) // If no alpha memory items
       unlinkLHS();                 // Remove node from child list
   }

   for(Iterator<AME> iI= amem.items.iterator(); iI.hasNext();)
   {
     AME ame= iI.next();
     WME wme= ame.getWME();
     if( joinTest(token, wme) )
     {
       for(Iterator<Node> iJ= getChildren().iterator(); iJ.hasNext();)
       {
         BM_Node beta= (BM_Node)iJ.next();
         beta.activateLHS(token, wme);
       }
     }
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       JoinNode.activateRHS
//
// Purpose-
//       RIGHT activation
//
// Reference-
//       join-node-right-activation (page  24)
//       join-node-right-activation (page 103)
//       join-node-right-activation (page 152)
//
//----------------------------------------------------------------------------
public void
   activateRHS(                     // RIGHT activate this JoinNode
     WME               wme)         // Associated WME
{
   if( SCDM && INFO ) debugln(getReference()+".activateRHS("+(wme == null?"<NULL>":wme.getReferWME())+")");

   BM_Node parent= (BM_Node)this.parent; // Get parent BM_Node
   if( linkedLHS == false )
   {
     relinkLHS();                   // Relink to beta memory

     if( parent.isItemsExist() == false )
       unlinkRHS();
   }

   for(Iterator<Token> iI= parent.items.iterator(); iI.hasNext();)
   {
     Token token= iI.next();
     if( joinTest(token, wme) )
     {
       for(Iterator<Node> iJ= getChildren().iterator(); iJ.hasNext();)
       {
         BM_Node beta= (BM_Node)iJ.next();
         beta.activateLHS(token, wme);
       }
     }
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       JoinNode.debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
public void
   debug(                           // Debugging display
     String            PREFIX)      // Prefix string
{
   debugln(PREFIX+"parent: " + parent.getReference());
   Vector<Node> children= getChildren();
   debugln(PREFIX+"children: " + children.size());
   for(Iterator<Node> i= children.iterator(); i.hasNext();)
   {
     Node child= i.next();
     debugln(PREFIX+">> " + child.getReferWME());
   }

   if( cond == null )               // TODO: DEBUG ONLY
     debugln(PREFIX+"cond: " + cond);
   else
     debugln(PREFIX+"cond: " + cond.getReferWME());

   if( amem == null )
     debugln(PREFIX+"amem: " + amem);
   else
   {
     debugln(PREFIX+amem.getReferKEY()+".items "+amem.items.size());
     for(Iterator<AME> i= amem.items.iterator(); i.hasNext();)
     {
       AME ame= i.next();
       debugln(PREFIX+"<< " + ame.wme);
     }
   }

   if( tests == null )
     debugln(PREFIX+"tests: " + tests);
   else
     for(int x= 0; x<tests.size(); x++)
       debugln(PREFIX+">> " + tests.elementAt(x));
}

public void
   debug( )
{  debug("");
}

//----------------------------------------------------------------------------
//
// Method-
//       JoinNode.initialize
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
     Node              child)       // The new child BM_Node
{
   if( SCDM && INFO ) debugln(getReference()+".initialize("+child.getReference()+")");

   Vector<Node> children= this.children; // Save children list

   this.children= new Vector<Node>(); // children= | child |
   this.children.add(child);

   for(Iterator<AME> i= amem.items.iterator(); i.hasNext();)
   {
     AME ame= i.next();
     activateRHS(ame.wme);
   }

   this.children= children;         // Restore children list
}

//----------------------------------------------------------------------------
//
// Method-
//       JoinNode.joinTest
//
// Purpose-
//       Perform the associated join tests.
//
// Reference-
//       perform-join-tests (for all tests)
//       Note: the list of tests is CONTAINED within this object.
//
//----------------------------------------------------------------------------
public boolean                      // Resultant
   joinTest(                        // Perform join tests
     Token             token,       // Associated token
     WME               wme1)        // Associated WME
{
   if( SCDM && INFO )
   {
     debugln(getReference()+".joinTest("+token.getReferWME()+","+(wme1==null?"<NULL>":wme1.getReferWME())+")");
     debugln(".. tests: " + tests.size());
     for(int i= 0; i<tests.size(); i++)
       debugln(".... " + tests.elementAt(i).toString());

     debugln(".. tokens: " + token.size());
     debugln(".... [0] " + (wme1==null?"<NULL>":wme1.getReferWME()));
     Token node= token;
     for(int x= 1; node != null; node= (Token)node.parent)
       debugln(".... [" + (x++) + "] " + node.getReferWME());
   }

   for(Iterator<JoinTest> i= tests.iterator(); i.hasNext();)
   {
     JoinTest test= i.next();
     String arg1= wme1.index(test.f1);
     WME wme2= token.index(test.c2).getWME(); // wme2<-token[test.c2]
     if( test.c2 == 0 )
       wme2= wme1;

     String arg2= wme2.index(test.f2);
     verify( arg2.equals("*") == false );
     boolean b= arg1.equals(arg2);
     if( false )                    // Super Hard-Core-Debug-Mode?
     {
       debugln(b+"= "+test.getReference()+"("+arg1+"::"+arg2+")");
       if( false )                  // HCDM: Debugging
       {
         debugln(getReference() + " !!HCDM!!");
         debugln("'"+wme2.getReferWME()+"'= "+token.getReference()+"["+test.c2+"]");
         debugln("'"+arg1+"'= "+wme1.getReferWME()+"["+test.f1+"]");
         debugln("'"+arg2+"'= "+wme2.getReferWME()+"["+test.f2+"]");
         debugln("<< [0] " + wme1.getReferWME());
         token.debug();
         debug();
       }
     }
     if( b == false )
       return false;
   }

// if( SCDM && INFO  )              // (For debugging)
//   debugln(".. SUCCESS");

   return true;
}

public boolean                      // Resultant
   joinTest(                        // Perform join tests
     Vector<Token>     list,        // Associated token list
     WME               wme1)        // Associated WME
{
   if( SCDM && INFO ) debugln(getReference()+".joinTest(Vector<Token>,"+wme1.getReferWME()+")");

   for(Iterator<Token> i= list.iterator(); i.hasNext();)
   {
     Token token= i.next();
     if( joinTest(token, wme1) == false )
       return false;
   }

   return true;
}

//----------------------------------------------------------------------------
//
// Method-
//       JoinNode.nearestParent
//
// Purpose-
//       Locate nearest parent with the same AM_Node
//
// Reference-
//       find-nearest-ancestor-with-same-amem
//       (Overrides Node.nearestParent)
//
//----------------------------------------------------------------------------
public JoinNode                     // The nearest parent with the same AM_Node
   nearestParent(                   // Get nearest parent with the same AM_Node
     AM_Node           amem)        // The associated AM_Node
{
   if( this.amem == amem )
     return this;

   return super.nearestParent(amem);
}

//----------------------------------------------------------------------------
//
// Method-
//       JoinNode.relinkRHS
//
// Purpose-
//       Relink to Alpha Memory
//
// Reference-
//       relink-to-alpha-memory
//
// Implementation notes-
//       The nearest_ancestor_with_same_amem *MUST BE* a JoinNode because
//       the JoinNode is the only type of ancestor with an AM_Node object.
//
//----------------------------------------------------------------------------
public void
   relinkRHS( )                     // Relink to Alpha Memory
{
   if( HCDM && SCDM && INFO ) debugln(getReference()+".relinkRHS()");

   JoinNode ancestor= nearest_ancestor_with_same_amem;
   while( ancestor != null && ancestor.linkedRHS == false )
     ancestor= ancestor.nearest_ancestor_with_same_amem;

   if( ancestor == null )
     amem.addSuccessor(this, ancestor); // Insert BEFORE ancestor
   else
     amem.addSuccessor(this);       // Add to TAIL
   linkedRHS= true;
}

//----------------------------------------------------------------------------
//
// Method-
//       JoinNode.relinkLHS
//
// Purpose-
//       Relink to Beta Memory
//
// Reference-
//       relink-to-beta-memory
//
//----------------------------------------------------------------------------
public void
   relinkLHS( )                     // Relink to Beta Memory
{
   if( HCDM && SCDM && INFO ) debugln(getReference()+".relinkLHS()");

   relink(null);                    // Insert at HEAD of parent.children
   linkedLHS= true;
}

//----------------------------------------------------------------------------
//
// Method-
//       JoinNode.unlinkRHS
//
// Purpose-
//       Unlink from Alpha Memory
//
//----------------------------------------------------------------------------
public void
   unlinkRHS( )                     // Unlink from Alpha Memory
{
   if( HCDM && SCDM && INFO ) debugln(getReference()+".unlinkRHS()");

   amem.delSuccessor(this);         // Remove this Node from successor list
   linkedRHS= false;
}

//----------------------------------------------------------------------------
//
// Method-
//       JoinNode.unlinkLHS
//
// Purpose-
//       Unlink from Beta Memory
//
//----------------------------------------------------------------------------
public void
   unlinkLHS( )                     // Unlink from Beta Memory
{
   if( HCDM && SCDM && INFO ) debugln(getReference()+".unlinkLHS()");

   unlink();                        // Remove this Node from parent.children
   linkedLHS= false;
}
} // class JoinNode

