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
//       Token.java
//
// Purpose-
//       Join token Node
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
//       Token
//
// Purpose-
//       Join token Node
//
// Reference-
//       token
//
// Page references-
//       22, 29, 41, 48, 149
//
//----------------------------------------------------------------------------
public class Token extends Node {   // Join token Node
//----------------------------------------------------------------------------
// Token.Attributes
//----------------------------------------------------------------------------
static int             globalSN= 0; // The GLOBAL serial number
int                    objectSN;    // The OBJECT serial number

// BM_Node             parent;      // (This is a Node)                @REMOVE
WME                    wme;         // Associated Working Memory Element
Node                   node;        // Associated Node
// Vector<BM_Node>     children;    // (This is a Node)                @REMOVE
// Vector<Token>       join_results;// List of negative join results
// Vector<Token>       ncc_results; // Like join_results, but for NCC nodes
// Token               owner;       // Owner Token

//----------------------------------------------------------------------------
//
// Method-
//       Token.Token
//
// Purpose-
//       Constructor
//
// Reference-
//       make-token(Node, Token, WME) !! Note parameter order mismatch !!
//
//----------------------------------------------------------------------------
public
   Token(                           // Constructor
     Token             parent,      // Parent Token
     Node              node,        // Source Node
     WME               wme)         // Associated WME
{
   super(parent);

   this.wme= wme;
   this.node= node;

   if( wme != null )                // (Required for negative conditions)
     wme.insertToken(this);         // (For tree based removal)
   else                             // TODO: REMOVE (BRINGUP)
   {
     if( parent == null )           // Is this a dummy Token?
       return;                      // wme == null allowed

     debugln("parent: " + parent.getReferWME());
     debugln("node: " + node.getReference());
     throw new RuntimeException("Token with wme== null");
   }


// join_results= new Vector();
// ncc_results= new Vector<Token>();
// owner= null;

   objectSN= ++globalSN;            // DUMMY Tokens have no wme or parent
   if( SCDM && INFO ) debugln("new " + getReference());
}

//----------------------------------------------------------------------------
//
// Method-
//       Token.delete
//
// Purpose-
//       Delete this object and all its descendents
//
// Reference-
//       delete-token-and-descendents
//
//----------------------------------------------------------------------------
public void
   delete( )                        // Delete this Token and all descendents
{
   // Avoid problems with Vector changing during deletions
   Object[] nodeList= getChildren().toArray();
   for(int i= 0; i<nodeList.length; i++)
     ((Node)nodeList[i]).delete();

   if( wme != null )                // Remove from associated WME list
     wme.removeToken(this);

   if( node instanceof BM_Node )    // If node is a memory node
   {
     BM_Node beta= (BM_Node)node;   // (tok.node)
     if( beta.isItemsExist() == false )
     {
       for(Iterator<Node> i= beta.getChildren().iterator(); i.hasNext();)
       {
         JoinNode join= (JoinNode)i.next(); // (tok.node child instance)
         join.amem.delSuccessor(join);
       }
     }
   }

   // If node is a negative node (NOT IMPLEMENTED)
   // If node is an NCC node (NOT IMPLEMENTED)
   // If node is an NCC partner node (NOT IMPLEMENTED)

   // Java cleanup (for garbage collector)
   wme= null;
   node= null;

   super.delete();
}

//----------------------------------------------------------------------------
//
// Method-
//       Token.Accessor methods
//
// Purpose-
//       Accessor methods
//
//----------------------------------------------------------------------------
public Node                         // The associated Node
   getNode( )                       // Get associated Node
{  return node; }

public WME                          // The associated WME
   getWME( )                        // Get associated WME
{  return wme; }

public String
   getReferWME( )
{  return getReference()+( wme==null ? "{<NULL>}" : wme); }

//----------------------------------------------------------------------------
//
// Method-
//       Token.MapDebug
//
// Purpose-
//       Override MapDebugAdaptor methods
//
//----------------------------------------------------------------------------
public void
   debug(                           // Debugging display
     DebugMap          map)         // DebugMap extention
{
   //-------------------------------------------------------------------------
   // Replaces super.debug(map)
   debugln(getReference());
   Token parent= (Token)this.parent;

   debugln(".. isLinked: " + isLinked);
   map.debug(".. parent: ", parent, parent == null || parent.wme == null ? "" : parent.wme.toString());

   Vector<Node> children= getChildren();
   debugln(".. children: " + children.size());
   for(Iterator<Node> i= children.iterator(); i.hasNext();)
   {
     Token token= (Token)i.next();
     map.debug(".... ", token, token.wme==null ? "" : token.wme.toString());
   }
   //-------------------------------------------------------------------------

   map.debug(".. wme: ", wme, wme == null ? "" : wme.fields.toString());
   map.debug(".. node: ", node);
// map.debug(".. owner: ", owner);

// map.debug(".. join_results: ", join_results); // TODO: Enumerate
// map.debug(".. ncc_results: ",  ncc_results); // TODO: Enumerate
}

public int                          // The object serial number
   getObjectSN( )                   // Get object serial number
{  return objectSN; }

//----------------------------------------------------------------------------
//
// Method-
//       Token.debug
//
// Purpose-
//       Token (tree) debugging display
//
//----------------------------------------------------------------------------
public void
   debug(                           // Debugging display
     String            PREFIX)      // Prefix string
{
   Token token= this;
   int index= 1;
   while( token != null )
   {
     debugln(PREFIX + "<< [" + (index++) + "] " + token.getReferWME());
     token= (Token)token.parent;
   }
}

public void
   debug( )
{  debug("");
}

//----------------------------------------------------------------------------
//
// Method-
//       Token.deleteChildren
//
// Purpose-
//       Delete all children
//
// Reference-
//       delete-descendents-of-token pg. 158
//
//----------------------------------------------------------------------------
public void
   deleteChildren( )                // Delete all descendents
{
   // Avoid problems with Vector changing during deletions
   Object[] nodeList= getChildren().toArray();
   for(int i= 0; i<nodeList.length; i++)
     ((Node)nodeList[i]).delete();

   verify( children == null );
}

//----------------------------------------------------------------------------
//
// Method-
//       Token.index
//
// Purpose-
//       Get the higher token, for items 0..size()-1
//
//----------------------------------------------------------------------------
public Token                        // The associated Token
   index(                           // Get higher Token
     int               x)           // For this index
{
   Node t= this;                    // Index[1] is THIS node
   while(x > 1 && t != null )
   {
     t= t.parent;
     x--;
   }

   return (Token)t;
}

//----------------------------------------------------------------------------
//
// Method-
//       Token.size
//
// Purpose-
//       Get the number of higher tokens (+1).
//
//----------------------------------------------------------------------------
public int                          // The number of higher Tokens
   size( )                          // Get number of higher Tokens
{
   int x= 0;

   Node t= this;
   while( t != null )
   {
     t= t.parent;
     x++;
   }

   return x;
}
} // class Token

