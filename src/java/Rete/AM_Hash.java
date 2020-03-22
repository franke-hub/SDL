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
//       AM_Hash.java
//
// Purpose-
//       Alpha Memory hash table.
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
//       AM_Hash
//
// Purpose-
//       Alpha Memory hash table.
//
// Reference-
//       (unnamed hash table)
//
//----------------------------------------------------------------------------
class AM_Hash extends MapDebugAdaptor { // Alpha Memory hash table
//----------------------------------------------------------------------------
// AM_Hash.Attributes
//----------------------------------------------------------------------------
Hashtable<WME_Key, AM_Node> hashList; // AM_Node hash table

//----------------------------------------------------------------------------
//
// Method-
//       AM_Hash.AM_Hash
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
public
   AM_Hash( )                       // Default constructor
{
   hashList= new Hashtable<WME_Key, AM_Node>(65536); // Create the Hashtable
}

//----------------------------------------------------------------------------
//
// Method-
//       AM_Hash.MapDebug
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

   debugln(".. hashList: " + hashList.size());
   Set<Map.Entry<WME_Key, AM_Node>> set= hashList.entrySet();
   for(Iterator<Map.Entry<WME_Key, AM_Node>> i= set.iterator(); i.hasNext();)
   {
     Map.Entry<WME_Key, AM_Node> entry= i.next();
     WME_Key key=  entry.getKey();
     AM_Node amem= entry.getValue();

     map.debug(".... ", amem, key.toString());
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       AM_Hash.insert
//
// Purpose-
//       Insert an AM_Node into the table
//
// Usage notes-
//       The WME_Key parameter must be a constant, e.g. a COPY
//
//----------------------------------------------------------------------------
public void
   insert(                          // Insert AM_Node into table
     WME_Key           key,         // The WME_Key
     AM_Node           node)        // The AM_Node to insert
{
   verify( hashList.get(key) == null ); // TODO: REMOVE

   node= hashList.put(key, node);
   verify( node == null );
}

//----------------------------------------------------------------------------
//
// Method-
//       AM_Hash.locate
//
// Purpose-
//       Locate an existing AM_Node
//
// Reference-
//       lookup-in-hash-table
//
//----------------------------------------------------------------------------
public AM_Node                      // Resultant
   locate(                          // Locate exsiting AM_Node
     WME_Key           key)         // For this WME_Key
{
   return hashList.get(key);
}

public AM_Node                      // Resultant
   locate(                          // Locate exsiting AM_Node for
     String            id,          // This identifier,
     String            attr,        // This attribute, and
     String            value)       // This value
{
   return hashList.get(new WME_Key(id, attr, value));
}

//----------------------------------------------------------------------------
//
// Method-
//       AM_Hash.remove
//
// Purpose-
//       Remove an AM_Node from the table
//
//----------------------------------------------------------------------------
public AM_Node                      // The removed AM_Node
   remove(                          // Remove AM_Node from table
     WME_Key           key)         // The WME_Key
{
   return hashList.remove(key);
}
} // class AM_Hash

