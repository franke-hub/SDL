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
//       Root.java
//
// Purpose-
//       Root descriptor.
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
//       Root
//
// Purpose-
//       Describe a word root.
//
//----------------------------------------------------------------------------
class Root implements Serializable { // Root descriptor
//----------------------------------------------------------------------------
// Root.Attributes
//----------------------------------------------------------------------------
Word[]                 word;        // Word array
String                 text;        // Word text
int                    count;       // Insert counter
int                    index;       // Root index

//----------------------------------------------------------------------------
// Root.Static attributes
//----------------------------------------------------------------------------
static final long      serialVersionUID= 0x000000fe20080101L;
static Vector<Root>    vector= new Vector<Root>(); // Root Vector
static TreeMap<String,Root>
                       treeMap= new TreeMap<String,Root>(); // Root TreeMap

//----------------------------------------------------------------------------
//
// Method-
//       Root.Root
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   Root( )                          // Constructor (for Serializable)
{
   word= null;
   text= null;
   count= 1;
   index= (-1);
}

public
   Root(                            // Constructor
     String            text)        // Root text
{
   word= null;
   this.text= text;
   count= 1;
   index= (-1);
}

//----------------------------------------------------------------------------
//
// Method-
//       Root.get
//
// Purpose-
//       Extract Root.
//
//----------------------------------------------------------------------------
public static Root                  // The associated Root
   get(                             // Get associated Root
     String            text)        // The Root text
{
   return treeMap.get(text);
}

public static Root                  // The associated Root
   get(                             // Get associated Root
     int               index)       // The Root index
{
   return vector.get(index);
}

//----------------------------------------------------------------------------
//
// Method-
//       Root.getCount
//
// Purpose-
//       Get the insert counter.
//
//----------------------------------------------------------------------------
public int                          // The usage count
   getCount( )                      // Get usage count
{
   return count;
}

//----------------------------------------------------------------------------
//
// Method-
//       Root.getIndex
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
//       Root.getString
//
// Purpose-
//       Get the String
//
//----------------------------------------------------------------------------
public String                       // The String
   getString( )                     // Get String
{
   return text;
}

//----------------------------------------------------------------------------
//
// Method-
//       Root.setCount
//
// Purpose-
//       Set the use counter.
//
//----------------------------------------------------------------------------
public void
   setCount(                        // Set usage count
     int               count)       // New usage count
{
   this.count= count;
}

//----------------------------------------------------------------------------
//
// Method-
//       Root.increment
//
// Purpose-
//       Increment the use counter.
//
//----------------------------------------------------------------------------
public void
   increment( )                     // Increment usage count
{
   count++;
}

//----------------------------------------------------------------------------
//
// Method-
//       Root.insert
//
// Purpose-
//       Insert a Root.
//
//----------------------------------------------------------------------------
public static void
   insert(                          // Insert a Root
     Root              root)        // The Root
{
   Root mapped= treeMap.get(root.text);
   if( mapped != null )
   {
     root= mapped;
     root.count++;
   }
   else
   {
     vector.add(root);
     root.index= vector.size() - 1;
     treeMap.put(root.text, root);
   }
}

public static void
   insert(                          // Insert a Root
     String            string)      // The Root String
{
   Root root= new Root(string);
   insert(root);
}

//----------------------------------------------------------------------------
//
// Method-
//       Root.check
//
// Purpose-
//       Verify the Root list.
//
//----------------------------------------------------------------------------
public static void
   check( )                         // Verify the Root list
   throws Exception
{
   for(Iterator i= treeMap.entrySet().iterator(); i.hasNext();)
   {
     StringFormat format= new StringFormat();
     Map.Entry me= (Map.Entry)i.next();
     String text= (String)me.getKey();
     Root root= (Root)me.getValue();

     if( text != root.text )
       throw new Exception("Root.check text");
     if( root.index < 0 || root.index > vector.size() )
       throw new Exception("root.check range(" + root.index + ")");
     if( vector.elementAt(root.index) != root )
       throw new Exception("Root.check index");
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Root.display
//
// Purpose-
//       Display the Root list.
//
//----------------------------------------------------------------------------
public static void
   display( )                       // Display the Root list
{
   for(Iterator i= treeMap.entrySet().iterator(); i.hasNext();)
   {
     StringFormat format= new StringFormat();
     Map.Entry me= (Map.Entry)i.next();
     String text= (String)me.getKey();
     Root root= (Root)me.getValue();

     format.append(root.getCount(), 12);
     System.out.println(format.toString() + " " + text);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Root.setVector
//
// Purpose-
//       Set the Vector from the TreeMap.
//
//----------------------------------------------------------------------------
protected static void
   setVector( )                     // Set the Vector from the TreeMap
   throws IOException
{
   vector= new Vector<Root>();
   for(Iterator i= treeMap.entrySet().iterator(); i.hasNext();)
   {
     StringFormat format= new StringFormat();
     Map.Entry me= (Map.Entry)i.next();
     String text= (String)me.getKey();
     Root   root= (Root)me.getValue();

     vector.add(root);
     root.index= vector.size() - 1;

     // Remove duplicate copy of key text
     if( !text.equals(root.text) )
       throw new IOException("Validity check failure");
     root.text= text;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Root.load
//
// Purpose-
//       Load the Root list.
//
//----------------------------------------------------------------------------
@SuppressWarnings("unchecked")
public static void
   load(                            // Load the Root list
     java.io.ObjectInputStream
                       inp)         // From this Input Stream
   throws IOException, ClassNotFoundException
{
   treeMap= (TreeMap<String,Root>)inp.readObject();
   setVector();
}

public static void
   load(                            // Load the Root list
     Reader            inp)         // From this Reader
   throws IOException
{
   BufferedReader br= new BufferedReader(inp);

   treeMap= new TreeMap<String,Root>();

   for(;;)
   {
     String line= br.readLine();
     if( line == null )
       break;

     Tokenizer token= new Tokenizer(line);
     int count= Integer.decode(token.get()).intValue();
     String text= token.get();

     Root root= new Root(text);
     root.count= count;
   }

   setVector();
}

//----------------------------------------------------------------------------
//
// Method-
//       Root.store
//
// Purpose-
//       Store the Root list.
//
//----------------------------------------------------------------------------
public static void
   store(                           // Store the Root list
     java.io.ObjectOutputStream
                       out)         // Into this Output Stream
   throws IOException
{
   out.writeObject(treeMap);
}

public static void
   store(                           // Store the Root list
     Writer            out)         // Into this Writer
   throws IOException
{
   for(Iterator i= treeMap.entrySet().iterator(); i.hasNext();)
   {
     StringFormat format= new StringFormat();
     Map.Entry me= (Map.Entry)i.next();
     String text= (String)me.getKey();
     Root   root= (Root)me.getValue();

     format.setRadix(10).append(root.getCount(), 12);
     out.write(format.toString() + " " + text + "\n");
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Root.readObject
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
   count= inp.readInt();
   text= (String)inp.readObject();
}

//----------------------------------------------------------------------------
//
// Method-
//       Root.writeObject
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
   out.writeInt(count);
   out.writeObject(text);
}
} // class Root

