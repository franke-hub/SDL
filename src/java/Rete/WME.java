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
//       WME.java
//
// Purpose-
//       Working Memory Element.
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
//       WME
//
// Purpose-
//       Working Memory Element.
//
// Reference-
//       WME
//
//----------------------------------------------------------------------------
public class WME extends MapDebugAdaptor { // Working Memory Element
//----------------------------------------------------------------------------
// WME.Attributes
//----------------------------------------------------------------------------
static int             globalSN= 0; // The GLOBAL serial number
int                    objectSN;    // The OBJECT serial number

WME_Key                fields;      // Identifier, attribute, value
Vector<AME>            alpha_mem_items; // List of AME's containing this
Vector<Token>          tokens;      // List of tokens containing this
//Vector               negative_join_results; // List of negative join results

//----------------------------------------------------------------------------
//
// Method-
//       WME.WME
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
private void
   constructor(                     // Constructor
     WME_Key           key)         // Associated WME_Key
{
   this.fields= key;

   alpha_mem_items= new Vector<AME>();
   tokens= new Vector<Token>();
// negative_join_results= new Vector();

   objectSN= globalSN++;            // FIRST WME is DUMMY
}

public
   WME(                             // Constructor
     WME_Key           key)         // Associated WME_Key
{
   constructor(key);
}

public
   WME(                             // Constructor
     String            id,          // Identifier
     String            attr,        // Attribute
     String            value)       // Value
{
   constructor(new WME_Key(id, attr, value));
}

//----------------------------------------------------------------------------
//
// Method-
//       WME.delete
//
// Purpose-
//       Delete this object and all its descendents
//
// Reference-
//       remove-wme (From Rete.removeWME)
//
//----------------------------------------------------------------------------
public void
   delete( )                        // Delete this WME
{
   // Delete associated AMEs
   for(Iterator<AME> i= alpha_mem_items.iterator(); i.hasNext();)
   {
     AME ame= i.next();
     ame.delete();                  // (Also removes it from AM_Node)
   }
   alpha_mem_items= null;

   // Delete tokens and their descendents
   for(Iterator<Token> i= tokens.iterator(); i.hasNext();)
   {
     Token token= i.next();
     i.remove();

     token.delete();
   }
   tokens= null;

   // Delete negative join results (NOT IMPLEMENTED)
}

//----------------------------------------------------------------------------
//
// Method-
//       WME.Accessor Methods
//
// Purpose-
//       Accessor methods
//
//----------------------------------------------------------------------------
public WME_Key                      // The associated WME_Key
   getKey( )                        // Get associated WME_Key
{  return fields; }

public String
   getReferWME( )
{  return getReference()+toString(); }

//----------------------------------------------------------------------------
//
// Method-
//       WME.MapDebug
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

   debugln(".. fields: " + fields);

   debugln(".. alpha_mem_items: " + alpha_mem_items.size());
   for(Iterator<AME> i= alpha_mem_items.iterator(); i.hasNext();)
   {
     Object o= i.next();
     map.debug(o);
   }

   debugln(".. tokens: " + tokens.size());
   for(Iterator<Token> i= tokens.iterator(); i.hasNext();)
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
//       WME.index
//
// Purpose-
//       Return fields.index()
//
//----------------------------------------------------------------------------
public String                       // The id, attribute, or value
   index(                           // Get id, attribute, or value
     int               x)           // INDEX_* control
{
   return fields.index(x);
}

//----------------------------------------------------------------------------
//
// Method-
//       WME.insertAME
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
   alpha_mem_items.add(0, ame);
}

//----------------------------------------------------------------------------
//
// Method-
//       WME.insertToken
//
// Purpose-
//       Insert a Token on the HEAD of the Token list
//
//----------------------------------------------------------------------------
public void
   insertToken(                     // Insert Token on HEAD of Token list
     Token             token)       // The Token to insert
{
   tokens.add(0, token);
}

//----------------------------------------------------------------------------
//
// Method-
//       WME.removeToken
//
// Purpose-
//       Remove a Token from the Token list
//
//----------------------------------------------------------------------------
public void
   removeToken(                     // Remove Token from Token list
     Token             token)       // The Token to remove
{
   tokens.remove(token);
}

//----------------------------------------------------------------------------
//
// Method-
//       WME.toString
//
// Purpose-
//       Convert to String
//
//----------------------------------------------------------------------------
public String                       // The display String
   toString( )                      // Get display String
{
   return fields.toString();
}
} // class WME

