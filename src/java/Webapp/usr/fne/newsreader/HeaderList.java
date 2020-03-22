//----------------------------------------------------------------------------
//
//       Copyright (C) 2014 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       HeaderList.java
//
// Purpose-
//       Java News Reader: List of headers.
//
// Last change date-
//       2014/01/01
//
//----------------------------------------------------------------------------
package usr.fne.newsreader;

import java.io.*;
import java.lang.*;
import java.util.*;

import usr.fne.common.*;

public class HeaderList {
//----------------------------------------------------------------------------
// HeaderList.Attributes
//----------------------------------------------------------------------------
protected TreeMap<String,NewsHeader>
                       map;         // (String,NewsHeader) correlator

//----------------------------------------------------------------------------
//
// Method-
//       HeaderList.HeaderList
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   HeaderList( )                    // Constructor
{
   map= new TreeMap<String,NewsHeader>();
}

//----------------------------------------------------------------------------
//
// Method-
//       HeaderList.contains
//
// Purpose-
//       Determine whether a NewsHeader is in the List.
//
//----------------------------------------------------------------------------
public synchronized boolean         // TRUE iff contained
   contains(                        // Is NewsHeader present
     String            name)        // The NewsHeader name
{
   return map.containsKey(name);
}

//----------------------------------------------------------------------------
//
// Method-
//       HeaderList.debug
//
// Purpose-
//       Debugging display.
//
//----------------------------------------------------------------------------
public void
   debug( )                         // Debugging display
{
   System.out.println("");
   System.out.println("HeaderList:");
   for(Iterator<Map.Entry<String,NewsHeader>> iter= map.entrySet().iterator(); iter.hasNext();)
   {
     Map.Entry<String,NewsHeader> me= iter.next();
     String string= me.getKey();
     NewsHeader header= me.getValue();

     System.out.print("'" + string + "'= "); header.debug();
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       HeaderList.get
//
// Purpose-
//       Get a NewsHeader from the List
//
//----------------------------------------------------------------------------
public synchronized NewsHeader      // Resultant
   get(                             // Locate a NewsHeader
     String            string)      // With this name
{
   NewsHeader result= (NewsHeader)map.get(string);
// MainLogger.logger.log("HeaderList: " + result + "= get("+string+")");
   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       HeaderList.put
//
// Purpose-
//       Put a NewsHeader onto the List
//
//----------------------------------------------------------------------------
public synchronized boolean         // TRUE iff unique
   put(                             // Add a NewsHeader to the list
     NewsHeader        header)      // The header to add
{
   boolean result= false;
   if( !map.containsKey(header.getIdentifier()) )
   {
     map.put(header.getIdentifier(), header);
     result= true;
   }

// MainLogger.logger.log("HeaderList: " + result + "= put("+header+")");
   return result;
}
} // Class HeaderList

