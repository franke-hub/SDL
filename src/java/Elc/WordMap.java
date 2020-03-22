//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       WordMap.java
//
// Purpose-
//       A Word TreeMap and Vector.
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
//       WordMap
//
// Purpose-
//       A Word TreeMap and Vector.
//
//----------------------------------------------------------------------------
class WordMap implements Serializable { // WordMap descriptor
//----------------------------------------------------------------------------
// WordMap.Attributes
//----------------------------------------------------------------------------
TreeMap<String,Word>   treeMap= new TreeMap<String,Word>(); // Word TreeMap
Vector<Word>           vector= new Vector<Word>(); // Word Vector

//----------------------------------------------------------------------------
// WordMap.Static attributes
//----------------------------------------------------------------------------
static final long      serialVersionUID= 0x000000fe20080101L;

//----------------------------------------------------------------------------
//
// Method-
//       WordMap.WordMap
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   WordMap( )                       // Constructor (for Serializable)
{
}

//----------------------------------------------------------------------------
//
// Method-
//       WordMap.get
//
// Purpose-
//       Extract Word.
//
//----------------------------------------------------------------------------
public Word                         // The associated Word
   get(                             // Get associated Word
     String            text)        // The Word text
{
   return treeMap.get(text);
}

public Word                         // The associated Word
   get(                             // Get associated Word
     int               index)       // The Word index
{
   return vector.get(index);
}

//----------------------------------------------------------------------------
//
// Method-
//       WordMap.insert
//
// Purpose-
//       Insert a Word.
//
//----------------------------------------------------------------------------
public void
   insert(                          // Insert a Word
     Word              word)        // The word
{
   Word mapped= treeMap.get(word.text);
   if( mapped == null )
   {
     vector.add(word);
     word.index= vector.size() - 1;
     treeMap.put(word.text, word);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       WordMap.check
//
// Purpose-
//       Verify the WordMap list.
//
//----------------------------------------------------------------------------
public void
   check( )                         // Verify the WordMap list
   throws Exception
{
   for(Iterator i= treeMap.entrySet().iterator(); i.hasNext();)
   {
     Map.Entry me= (Map.Entry)i.next();
     String text= (String)me.getKey();
     Word   word= (Word)me.getValue();

     if( text != word.text )
       throw new Exception("Word.check text");
     if( word.index < 0 || word.index > vector.size() )
       throw new Exception("Word.check range(" + word.index + ")");
     if( vector.elementAt(word.index) != word )
       throw new Exception("Word.check index");
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       WordMap.display
//
// Purpose-
//       Display the WordMap list.
//
//----------------------------------------------------------------------------
public void
   display( )                       // Display the WordMap list
{
   for(Iterator i= treeMap.entrySet().iterator(); i.hasNext();)
   {
     StringFormat format= new StringFormat();
     Map.Entry me= (Map.Entry)i.next();
     String text= (String)me.getKey();
     Word   word= (Word)me.getValue();

     format.append(word.getCount(), 12);
     System.out.println(format.toString() + " " + text);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       WordMap.readObject
//
// Purpose-
//       Implement Serializable.
//
//----------------------------------------------------------------------------
@SuppressWarnings("unchecked")
private void
   readObject(                      // Read the Object
     java.io.ObjectInputStream
                       inp)         // From this Input Stream
   throws IOException, ClassNotFoundException
{
   treeMap= (TreeMap<String,Word>)inp.readObject();

   vector= new Vector<Word>();
   for(Iterator i= treeMap.entrySet().iterator(); i.hasNext();)
   {
     Map.Entry me= (Map.Entry)i.next();
     String text= (String)me.getKey();
     Word   word= (Word)me.getValue();

     vector.add(word);
     word.index= vector.size() - 1;

     // Remove duplicate copy of key text
     if( !text.equals(word.text) )
       throw new IOException("Validity check failure");
     word.text= text;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       WordMap.writeObject
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
   out.writeObject(treeMap);
}
} // class WordMap

