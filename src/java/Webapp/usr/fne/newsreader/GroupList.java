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
//       GroupList.java
//
// Purpose-
//       Java News Reader: List of NewsGroup Objects.
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

public class GroupList {
//----------------------------------------------------------------------------
// GroupList.Attributes
//----------------------------------------------------------------------------
protected TreeMap<String,NewsGroup>
                       map;         // (String,NewsGroup) correlator
protected boolean      changed;     // Changed indicator

//----------------------------------------------------------------------------
//
// Method-
//       GroupList.GroupList
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   GroupList( )                     // Constructor
{
   map= new TreeMap<String,NewsGroup>();
   changed= false;
}

//----------------------------------------------------------------------------
//
// Method-
//       GroupList.Accessors
//
// Purpose-
//       Attribute accessors.
//
//----------------------------------------------------------------------------
public boolean                      // The CHANGED attribute
   getChanged( )                    // Get CHANGED attribute
{
   return changed;
}

public void
   setChanged(                      // Set CHANGED attribute
     boolean           changed)     // The CHANGED attribute
{
   this.changed= changed;
}

//----------------------------------------------------------------------------
//
// Method-
//       GroupList.contains
//
// Purpose-
//       Determine whether a NewsGroup is in the List.
//
//----------------------------------------------------------------------------
public synchronized boolean         // TRUE iff contained
   contains(                        // Is NewsGroup present
     String            name)        // The NewsGroup name
{
   return map.containsKey(name);
}

//----------------------------------------------------------------------------
//
// Method-
//       GroupList.iterator
//
// Purpose-
//       Iterate the NewsGroups
//
//----------------------------------------------------------------------------
public synchronized Iterator        // Resultant
   iterator( )                      // Iterate the GroupList
{
   return map.values().iterator();
}

//----------------------------------------------------------------------------
//
// Method-
//       GroupList.get
//
// Purpose-
//       Get a NewsGroup from the List
//
//----------------------------------------------------------------------------
public synchronized NewsGroup       // Resultant
   get(                             // Locate a NewsGroup
     String            string)      // With this name
{
   NewsGroup result= map.get(string);
// MainLogger.logger.log("GroupList: " + result + "= get("+string+")");
   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       GroupList.put
//
// Purpose-
//       Put a NewsGroup onto the GroupList
//
//----------------------------------------------------------------------------
public synchronized boolean         // TRUE iff unique
   put(                             // Add a NewsGroup to the list
     NewsGroup         group)       // The group to add
{
   boolean result= false;
   if( !map.containsKey(group.name) )
   {
     changed= true;
     map.put(group.name, group);
     result= true;
   }

// MainLogger.logger.log("GroupList: " + result + "= put("+group+")");
   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       GroupList.remove
//
// Purpose-
//       Remove a NewsGroup from the List
//
//----------------------------------------------------------------------------
public synchronized NewsGroup       // Resultant
   remove(                          // Remove the NewsGroup
     String            string)      // With this name
{
   changed= true;
   return map.remove(string);
}

//----------------------------------------------------------------------------
//
// Method-
//       GroupList.read
//
// Purpose-
//       Read the list of NewsGroups from a file.
//
//----------------------------------------------------------------------------
public static GroupList             // The resultant GroupList
   read(                            // Read NewsGroup list
     String            fileName)    // Associated file name
   throws Exception
{
   GroupList           result= new GroupList();

   BufferedReader reader= new BufferedReader(
                          new FileReader(fileName));
   for(;;)
   {
     String string= reader.readLine();
     if( string == null )
       break;

     result.put(new NewsGroup(string));
   }

   result.changed= false;
   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       GroupList.write
//
// Purpose-
//       Write the list of NewsGroups into a file.
//
//----------------------------------------------------------------------------
public void
   write(                           // Write NewsGroup list
     String            fileName)    // Associated file name
   throws Exception
{
   NewsGroup           group;       // Working NewsGroup
   BufferedWriter      writer;      // Output Writer

   writer= new BufferedWriter(
               new FileWriter(fileName)
               );

   for(Iterator itor= iterator(); itor.hasNext(); )
   {
     group= (NewsGroup)itor.next();
     writer.write(group.toString() + "\n");
   }

   writer.close();
}
} // Class GroupList

