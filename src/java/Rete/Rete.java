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
//       Rete.java
//
// Purpose-
//       Rete object descriptor.
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
//       Rete
//
// Purpose-
//       Rete object descriptor.
//
// Reference-
//       rete
//
//----------------------------------------------------------------------------
class Rete extends MapDebugAdaptor { // Rete object descriptor
//----------------------------------------------------------------------------
// Rete.Attributes
//----------------------------------------------------------------------------
AM_Hash                amHash= new AM_Hash(); // Alpha Memory hash table
WM_Hash                wmHash= new WM_Hash(); // Working Memory hash table

AM_Node                amHead= null; // Dummy top node
BM_Node                bmHead= null; // Dummy top node
JoinNode               joinHD= null; // Dummy top node

//----------------------------------------------------------------------------
//
// Method-
//       Rete.Rete
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
public
   Rete( )                          // Default constructor
{
   // Initialize dummy Nodes
   WME_Key key= new WME_Key("*", "*", "*"); // (Not used in WM_Hash)
   amHead= new AM_Node(key);        // Create dummy AM_Node
   WME wme= new WME(key);           // Dummy WME
   wme= null;                       // NULL WME
   AME ame= new AME(amHead, wme);   // Dummy AME
// wme.alpha_mem_items.add(ame);    // WME contains dummy AME
   amHead.items.add(ame);           // AM_Node contains dummy AME

   bmHead= new BM_Node(null);       // Create dummy BM_Node
   Token tok= new Token(null, bmHead, wme); // Dummy top Token
// wme.insertToken(tok);            // WME contains dummy top Token
   bmHead.items.add(tok);           // Activate dummy Token
   joinHD= new JoinNode(bmHead, amHead, new Vector<JoinTest>()); // Create dummy JoinNode
   joinHD.cond= new Condition(key); // TODO: Remove (For debugging only)
}

//----------------------------------------------------------------------------
//
// Method-
//       Rete.MapDebug
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

   map.debug(".. amHash: ", amHash);
   map.debug(".. wmHash: ", wmHash);
   map.debug(".. amHead: ", amHead);
   map.debug(".. bmHead: ", bmHead);
   map.debug(".. joinHD: ", joinHD);
}

//----------------------------------------------------------------------------
//
// Method-
//       Rete.accessAM
//
// Purpose-
//       Locate AM_Node in amHash, creating one if non-existent
//
//----------------------------------------------------------------------------
public AM_Node                      // Resultant
   accessAM(                        // Locate/Create AM_Node for
     WME_Key           key)         // This key
{
   AM_Node am= amHash.locate(key);
   if( am == null )
   {
     am= new AM_Node(key);
     amHash.insert(key, am);
   }

   am.reference_count++;
   return am;
}

public AM_Node                      // Resultant
   accessAM(                        // Locate/Create AM_Node for
     String            id,          // This identifier,
     String            attr,        // This attribute, and
     String            value)       // This value
{
   return accessAM(new WME_Key(id, attr, value));
}

//----------------------------------------------------------------------------
//
// Method-
//       Rete.activateAME
//
// Purpose-
//       Create a new AME, add it to associated AM
//
// Reference-
//      alpha-memory-activation
//
//----------------------------------------------------------------------------
public void
   activateAME(                     // Create new AME
     AM_Node           amem,        // For this AM_Node  (If null, does nothing)
     WME               wme)         // And this WME
{
   if( amem != null )               // (Moved NULL test from insertWME)
   {
     AME ame= new AME(amem, wme);   // Create new AME

     wme.insertAME(ame);            // Add to WME.AME list (at HEAD)
     amem.insertAME(ame);           // Add to AM_Node.AME list (at HEAD)
                                    // and right-activate successors
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Rete.insertBeta
//
// Purpose-
//       Insert BM_Node
//
// Reference-
//       build-or-share-beta-memory-node
//
//----------------------------------------------------------------------------
public BM_Node                      // Resultant BM_Node
   insertBeta(                      // Share/create a BM_Node
     JoinNode          parent)      // With this parent
{
   if( SCDM && INFO ) debugln("insertBeta(" + parent.getReference() + ")");

   for(Iterator<Node> i= parent.getChildren().iterator(); i.hasNext();)
   {
     Node node= i.next();
     if( node instanceof BM_Node )
       return (BM_Node)node;        // Return existing BM_Node (shared)
   }

   BM_Node child= new BM_Node(parent); // Create/initialize the BM_Node
   parent.initialize(child);        // update-new-node-with-matches-from-above
   return child;                    // Return newly created BM_Node
}

//----------------------------------------------------------------------------
//
// Method-
//       Rete.insertCond
//
// Purpose-
//       Insert Condition
//
// Reference-
//       build-or-share-alpha-memory
//
//----------------------------------------------------------------------------
public AM_Node                      // Resultant AM_Node
   insertCond(                      // Insert Condition
     Condition         cond)        // The Condition
{
   if( SCDM && INFO ) debugln("insertCond("+cond.getReferWME()+")");

   WME_Key key= cond.getKey().dontCare(); // Get AM_Node key
   AM_Node am= locateAM(key);       // Locate existing AM
   if( am == null )                 // If non-existent
   {
     am= new AM_Node(key);          // Allocate a new AM_Node
     amHash.insert(key, am);        // Insert it into hash table

     for(Enumeration<WME> e= wmHash.getWME(); e.hasMoreElements();)
     {
       WME wme= e.nextElement();
       if( key.equalsDontCare(wme.getKey()) )
         activateAME(am, wme);
     }
   }

   return am;
}

//----------------------------------------------------------------------------
//
// Method-
//       Rete.insertData
//
// Purpose-
//       Insert constant
//
//----------------------------------------------------------------------------
public WME                          // The associated WME
   insertData(                      // Insert constant
     WME_Key           key)         // The data
{
   WME wme= wmHash.locate(key);
   if( wme == null )
   {
     wme= new WME(key);
     wmHash.insert(key, wme);
     insertWME(wme);
   }

   return wme;
}

public WME                          // The associated WME
   insertData(                      // insert constant
     String            id,          // Identifier
     String            attr,        // Attribute
     String            value)       // Value
{
   return insertData(new WME_Key(id, attr, value));
}

//----------------------------------------------------------------------------
//
// Method-
//       Rete.insertJoin
//
// Purpose-
//       Insert JoinNode
//
// Reference-
//       build-or-share-join-node
//
//----------------------------------------------------------------------------
public JoinNode                     // Resultant JoinNode
   insertJoin(                      // Share/create a JoinNode
     BM_Node           parent,      // With this parent
     AM_Node           amem,        // The associated AM_Node
     Vector<JoinTest>  testList)    // The list of JoinTests
{
   if( SCDM && INFO )
   {
     debugln("insertJoin("+parent.getReference()+","+amem.getReferKEY()+")");
     for(Iterator<JoinTest> i= testList.iterator(); i.hasNext();)
     {
       JoinTest test= i.next();
       debugln("<< " + test);
     }
   }

   JoinNode child= parent.insertJoin(amem, testList);
   if( Bringup.correct_insertJoin ) // HCDM: BRINGUP
     parent.initialize(child);      // update-new-node-with-matches-from-above
                                    // MISSING FROM THESIS here.

   return child;                    // Return newly created JoinNode
}

//----------------------------------------------------------------------------
//
// Method-
//       Rete.insertMesh
//
// Purpose-
//       Insert network for conditions
//
// Reference-
//       build-or-share-network-for-conditions
//
// Implementation note-
//       TODO: Only used by insertProd, negative conditions not supported yet
//
//----------------------------------------------------------------------------
public JoinNode                     // Resultant JoinNode
   insertMesh(                      // Share/create a condition network
     JoinNode          parent,      // With this parent
     Vector<Condition> condList,    // List of Conditions
     Vector<Condition> earlier)     // List of earlier Conditions (may be null)
{
   if( false || (SCDM && INFO) )
   {
     debugln("HCDM: insertMesh");
     debugln("parent: " + parent.getReference());
     parent.debug(".. ");
     debugln("condList: " + condList.size());
     for(int i= 0; i<condList.size(); i++)
       debugln("[" + i + "] " + condList.elementAt(i).getReferWME());

     if( earlier == null )
       debugln("earlier: <NULL>");
     else
     {
       debugln("earlier: " + earlier.size());
       for(int i= 0; i<earlier.size(); i++)
         debugln("[" + i + "] " + earlier.elementAt(i).getReferWME());
     }

     debugln("");
   }

   // Clone the earlier Vector<Condition>
   Vector<Condition> higher= new Vector<Condition>();
   if( earlier != null )
   {
     for(Iterator<Condition> i= earlier.iterator(); i.hasNext();)
       higher.add(i.next());
   }

   // Create the JoinTest network
   JoinNode join= parent;           // Working JoinNode
   final int M= condList.size();
   for(int i= 0; i<M; i++)
   {
     Condition cond= condList.elementAt(i);
     if( cond instanceof PosCondition )
     {
       BM_Node beta= insertBeta(join);
       Vector<JoinTest> tests= cond.getJoinTests(higher);
       AM_Node amem= insertCond(cond);
       join= insertJoin(beta, amem, tests);
       join.cond= cond;
     }
     else // NegCondition, NccCondition TODO: NOT CODED YET
       ;

     //-----------------------------------------------------------------------
     // The thesis, page 164, says: append Ci to conds-higher-up
     // The condition must be added to the HEAD of conds-higher-up
     higher.add(0, cond);
   }

   return join;
}

//----------------------------------------------------------------------------
//
// Method-
//       Rete.insertProd
//
// Purpose-
//       Insert Production for conditions
//
// Reference-
//       add-production
//
//----------------------------------------------------------------------------
public Production                   // Resultant Production
   insertProd(                      // Insert Production for conditions
     Vector<Condition> lhs,         // List of Conditions
     Vector<Action>    rhs)         // List of Actions
{
// try {
     JoinNode join= insertMesh(joinHD, lhs, null);
     Production prod= new Production(join, lhs, rhs);
     join.initialize(prod);         // update-new-node-with-matches-from-above
     return prod;
// } catch( Exception x ) {
//   debugln("Rete.insertProd: Exception: " + x);
//   x.printStackTrace();
//   return new Production(null, lhs, rhs);
// }
}

public Production                   // Resultant Production
   insertProd(                      // Insert Production for conditions
     Vector<Condition> lhs,         // List of Conditions
     Action            action)      // Single Action
{
   Vector<Action> rhs= new Vector<Action>();
   rhs.add(action);
   return insertProd(lhs, rhs);
}

//----------------------------------------------------------------------------
//
// Method-
//       Rete.insertWME
//
// Purpose-
//       Insert WME into AM_Hash
//
// Reference-
//       add-wme (Exhausinve hash table version)
//
//----------------------------------------------------------------------------
public void
   insertWME(                       // Insert
     WME               wme)         // This WME into AM_Hash
{
   WME_Key key= wme.getKey();       // Extract ID, ATTR, and VALUE
   String id=    key.id;
   String attr=  key.attr;
   String value= key.value;

   activateAME(amHash.locate( id, attr, value), wme);
   activateAME(amHash.locate( id, attr,   "*"), wme);
   activateAME(amHash.locate( id,  "*", value), wme);
   activateAME(amHash.locate( id,  "*",   "*"), wme);
   activateAME(amHash.locate("*", attr, value), wme);
   activateAME(amHash.locate("*", attr,   "*"), wme);
   activateAME(amHash.locate("*",  "*", value), wme);
   activateAME(amHash.locate("*",  "*",   "*"), wme);
}

//----------------------------------------------------------------------------
//
// Method-
//       Rete.locateAM
//
// Purpose-
//       Locate an existing AM_Node
//
// Reference-
//       lookup-in-hash-table
//
//----------------------------------------------------------------------------
public AM_Node                      // Resultant
   locateAM(                        // Locate exsiting AM_Node
     WME_Key           key)         // For this WME_Key
{
   return amHash.locate(key);
}

//----------------------------------------------------------------------------
//
// Method-
//       Rete.removeWME
//
// Purpose-
//       Remove WME
//
// Reference-
//       remove-wme
//
//----------------------------------------------------------------------------
public void
   removeWME(                       // Remove
     WME               wme)         // This WME
{
   wmHash.remove(wme.getKey());

   wme.delete();
}
} // class Rete

