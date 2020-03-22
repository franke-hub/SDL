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
//       WM_Hash.java
//
// Purpose-
//       Working Memory hash table.
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
//       WM_Hash
//
// Purpose-
//       Working Memory hash table.
//
// Reference-
//       (Unnamed list of WME objects)
//
//----------------------------------------------------------------------------
class WM_Hash extends MapDebugAdaptor { // Working Memory hash table
//----------------------------------------------------------------------------
// WM_Hash.Attributes
//----------------------------------------------------------------------------
Hashtable<WME_Key, WME> hashList;   // Working Memory hash table

//----------------------------------------------------------------------------
//
// Method-
//       WM_Hash.WM_Hash
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
public
   WM_Hash( )                       // Default constructor
{
   hashList= new Hashtable<WME_Key, WME>(65536); // Create the Hashtable
}

//----------------------------------------------------------------------------
//
// Method-
//       WM_Hash.MapDebug
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
   Set<Map.Entry<WME_Key, WME>> set= hashList.entrySet();
   for(Iterator<Map.Entry<WME_Key, WME>> i= set.iterator(); i.hasNext();)
   {
     Map.Entry<WME_Key, WME> entry= i.next();
     WME_Key key= entry.getKey();
     WME wme= entry.getValue();

     map.debug(".... " + key + ": ", wme);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       WM_Hash.getWME
//
// Purpose-
//       Get the WME Enumeration
//
//----------------------------------------------------------------------------
public Enumeration<WME>             // The WME Enumeration
   getWME( )                        // Get WME Enumeration
{
   return hashList.elements();
}

//----------------------------------------------------------------------------
//
// Method-
//       WM_Hash.insert
//
// Purpose-
//       Insert a WME into the table
//
//----------------------------------------------------------------------------
public void
   insert(                          // Insert WME into table
     WME_Key           key,         // The WME_Key
     WME               wme)         // The WME to insert
{
   verify( hashList.get(key) == null ); // TODO: REMOVE

   wme= hashList.put(key, wme);
   verify( wme == null );
}

//----------------------------------------------------------------------------
//
// Method-
//       WM_Hash.locate
//
// Purpose-
//       Locate an existing WME
//
//----------------------------------------------------------------------------
public WME                          // Resultant
   locate(                          // Locate exsiting WME
     WME_Key           key)         // For this WME_Key
{
   return hashList.get(key);
}

public WME                          // Resultant
   locate(                          // Locate exsiting WME for
     String            id,          // This identifier,
     String            attr,        // This attribute, and
     String            value)       // This value
{
   return hashList.get(new WME_Key(id, attr, value));
}

//----------------------------------------------------------------------------
//
// Method-
//       WM_Hash.remove
//
// Purpose-
//       Remove a WME from the table
//
//----------------------------------------------------------------------------
public WME                          // The removed WME
   remove(                          // Remove WME from table
     WME_Key           key)         // The WME_Key
{
   return hashList.remove(key);
}
} // class WM_Hash

