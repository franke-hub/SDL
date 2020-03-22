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
//       DebugMap.java
//
// Purpose-
//       The map used by MapDebug interface objects.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
package user.util;

import java.io.*;
import java.util.*;

//----------------------------------------------------------------------------
//
// Class-
//       DebugMap
//
// Purpose-
//       Debug helper methods: keeps track of referenced elements
//
//----------------------------------------------------------------------------
public class DebugMap extends Debug { // Debug helper methods
//-------------------------------------------------------------------------
// DebugMap.Attributes
//-------------------------------------------------------------------------
String                 next;        // Next GET string
TreeMap<String, Object>done;        // Reference map (Already referenced)
TreeMap<String, MapDebug>
                       refs;        // Reference map (All MapDebugIF objects)

//-------------------------------------------------------------------------
//
// Method-
//       DebugMap.DebugMap
//
// Purpose-
//       Constructor.
//
//-------------------------------------------------------------------------
public
   DebugMap( )                      // Default constructor
{
   reset();
}

//-------------------------------------------------------------------------
//
// Method-
//       DebugMap.debug
//
// Purpose-
//       Add/display reference to an Object
//
//-------------------------------------------------------------------------
public void
   debug(                           // Add Debug reference
     String            prefix,      // Output prefix
     Object            object,      // The Debug Object (NULL allowed)
     String            suffix)      // Output suffix
{
   if( object == null )             // If null element
   {
     debugf(prefix + "<NULL>" + suffix + "\n");
     return;
   }

   if( object instanceof MapDebug ) // If MapDebug object
   {
     MapDebug mdo= (MapDebug)object;
     String key= mdo.getReference();
     debugf(prefix + key + suffix + "\n");

     if( refs.containsKey(key) == false )
     {
       refs.put(key, mdo);

       if( next != null && key.compareTo(next) < 0 )
         next= key;
     }
   }
   else
   {
     debugf(prefix + Debug.getClassName(object) + "(" + object + ")"
          + suffix + "\n");
     return;
   }
}
public void
   debug(                           // Add Debug reference
     String            prefix,      // Output prefix
     Object            object)      // The Debug Object (NULL allowed)
{  debug(prefix, object, "");
}

public void
   debug(                           // Add Debug reference
     Object            object)      // The Debug Object
{  debug(".... ", object, "");      // (Generic array element)
}

//-------------------------------------------------------------------------
//
// Method-
//       DebugMap.reset
//
// Purpose-
//       Reset memory references
//
//-------------------------------------------------------------------------
public void
   reset( )                         // Reset memory references
{
   done= new TreeMap<String, Object>();
   refs= new TreeMap<String, MapDebug>();
   next= null;
}

//-------------------------------------------------------------------------
//
// Method-
//       DebugMap.unwind
//
// Purpose-
//       Unwind the the reference map
//
//-------------------------------------------------------------------------
public void
   unwind(                          // Unwind stacked objects
     boolean           space)       // TRUE to space between objects
{
   try {
     next= refs.firstKey();         // Begin at the beginning
   } catch( Exception e ) {
     next= null;
   }

   while( next != null )            // Unwind the reference tree
   {
     String prev= next;             // Save prior key

     if( done.containsKey(next) == false )
     {
       if( space )                  // Add spacer?
         debugf("\n");

       MapDebug mdo= refs.get(next);
       mdo.debug(this);

       done.put(prev, null);
     }

     if( next == prev )             // If next key (pointer) did not change
       next= refs.higherKey(next);  // Get next higher key
   }
}

public void
   unwind( )                        // Unwind stacked objects
{  unwind(true);                    // DEFAULT, add spacer
}
} // class DebugMap

