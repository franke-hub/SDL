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
//       Rule.java
//
// Purpose-
//       Rule descriptor.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
import java.io.*;
import java.lang.*;
import java.util.*;

import user.util.*;

//----------------------------------------------------------------------------
//
// Class-
//       Rule
//
// Purpose-
//       Describe a Rule.
//
//----------------------------------------------------------------------------
class Rule implements Serializable { // Rule descriptor
//----------------------------------------------------------------------------
// Rule.Attributes
//----------------------------------------------------------------------------
String                 name;        // Rule name
int                    index;       // Rule index

//----------------------------------------------------------------------------
// Rule.Static attributes
//----------------------------------------------------------------------------
static final long      serialVersionUID= 0x000000fe20080101L;
static Vector<Rule>    vector= new Vector<Rule>(); // Rule Vector

//----------------------------------------------------------------------------
//
// Method-
//       Rule.Rule
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   Rule( )                          // Constructor (for Serializable)
{
   name= null;
   index= (-1);
}

public
   Rule(                            // Constructor
     String            name)        // Rule name
{
   this.name= name;
   index= (-1);
}

//----------------------------------------------------------------------------
//
// Method-
//       Rule.getIndex
//
// Purpose-
//       Get the index.
//
//----------------------------------------------------------------------------
public int                          // The vector index
   getIndex( )                      // Get vector index
{
   return index;
}

//----------------------------------------------------------------------------
//
// Method-
//       Rule.getName
//
// Purpose-
//       Get the name.
//
//----------------------------------------------------------------------------
public String                       // The name String
   getName( )                       // Get name String
{
   return name;
}

//----------------------------------------------------------------------------
//
// Method-
//       Rule.insert
//
// Purpose-
//       Insert a Rule.
//
//----------------------------------------------------------------------------
public static void
   insert(                          // Insert a Rule
     Rule              rule)        // The Rule
{
   vector.add(rule);
   rule.index= vector.size() - 1;
}

//----------------------------------------------------------------------------
//
// Method-
//       Rule.check
//
// Purpose-
//       Verify the Rule list.
//
//----------------------------------------------------------------------------
public static void
   check( )                         // Verify the Rule list
   throws Exception
{
   for(int i= 0; i<vector.size(); i++)
   {
     Rule rule= vector.elementAt(i);
     String name= rule.getName();

     for(int j= i+1; j<vector.size(); j++)
     {
       if( name.equals(vector.elementAt(j).getName()) )
         throw new Exception("Rule.check duplicate(" + name + ")");
     }

     if( i != rule.getIndex() )
       throw new Exception("Rule.check index(" + name + ")");
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Rule.display
//
// Purpose-
//       Display the Rule list.
//
//----------------------------------------------------------------------------
public static void
   display( )                       // Display the Rule list
{
   for(int i= 0; i<vector.size(); i++)
   {
     Rule rule= vector.elementAt(i);
     String name= rule.getName();

     System.out.println(name);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Rule.load
//
// Purpose-
//       Load the Rule list.
//
//----------------------------------------------------------------------------
@SuppressWarnings("unchecked")
public static void
   load(                            // Load the Rule list
     java.io.ObjectInputStream
                       inp)         // From this Input Stream
   throws IOException, ClassNotFoundException
{
}

public static void
   load(                            // Load the Rule list
     Reader            inp)         // From this Reader
   throws IOException
{
}

//----------------------------------------------------------------------------
//
// Method-
//       Rule.store
//
// Purpose-
//       Store the Rule list.
//
//----------------------------------------------------------------------------
public static void
   store(                           // Store the Rule list
     java.io.ObjectOutputStream
                       out)         // Into this Output Stream
   throws IOException
{
}

public static void
   store(                           // Store the Rule list
     Writer            out)         // Into this Writer
   throws IOException
{
}

//----------------------------------------------------------------------------
//
// Method-
//       Rule.readObject
//
// Purpose-
//       Implement Serializable.
//
//----------------------------------------------------------------------------
private void
   readObject(                      // Read the Object
     java.io.ObjectInputStream
                       inp)         // From this Input Stream
   throws IOException, ClassNotFoundException
{
   name= (String)inp.readObject();
}

//----------------------------------------------------------------------------
//
// Method-
//       Rule.writeObject
//
// Purpose-
//       Implement Serializable.
//
//----------------------------------------------------------------------------
private void
   writeObject(                     // Write the Object
     java.io.ObjectOutputStream
                       out)         // Into this Input Stream
   throws IOException
{
   out.writeObject(name);
}
} // class Rule

