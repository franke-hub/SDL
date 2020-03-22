//----------------------------------------------------------------------------
//
//       Copyright (C) 2007-2008 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Word.java
//
// Purpose-
//       Generic Word descriptor.
//
// Last change date-
//       2008/01/01
//
//----------------------------------------------------------------------------
import java.io.*;
import java.lang.*;
import java.util.*;

import user.util.*;

//----------------------------------------------------------------------------
//
// Class-
//       Word
//
// Purpose-
//       Describe a Word.
//
//----------------------------------------------------------------------------
class Word implements Serializable { // Word descriptor
//----------------------------------------------------------------------------
// Word.Attributes
//----------------------------------------------------------------------------
// Word types
public static final int TYPE_KNOWN= 0xffffffff; // Any known type

public static final int TYPE_ART=   0x00000300; // A, an, or the
public static final int TYPE_A=     0x00000100; // A or an
public static final int TYPE_THE=   0x00000200; // The

public static final int TYPE_VERB=  0x00001000; // Verb
public static final int TYPE_NOUN=  0x00002000; // Noun
public static final int TYPE_PREP=  0x00004000; // Preposition
public static final int TYPE_CONJUNCTION=  0x00008000; // Conjunction
public static final int TYPE_INTERJECTION= 0x00010000; // Interjection
public static final int TYPE_ADVERB=    0x00020000; // Adverb
public static final int TYPE_ADJECTIVE= 0x00040000; // Adjective
public static final int TYPE_PRONOUN=   0x00080000; // Pronoun

public static final int TYPE_NAME=  0x00100000; // Proper name
public static final int TYPE_JUNK=  0x80000000; // Known nonsense word

String                 text;        // Word text
int                    count;       // Insert counter
int                    index;       // Word index

//----------------------------------------------------------------------------
// Word.Static attributes
//----------------------------------------------------------------------------
static final long      serialVersionUID= 0x000000fe20080101L;
static TreeMap<String,Word>
                       treeMap= new TreeMap<String,Word>(); // Word TreeMap
static Vector<Word>    vector= new Vector<Word>(); // Word Vector

//----------------------------------------------------------------------------
//
// Method-
//       Word.Word
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   Word( )                          // Constructor (for Serializable)
{
   text= null;
   count= 1;
   index= (-1);
}

public
   Word(                            // Constructor
     String            text)        // Word text
{
   this.text= text;
   count= 1;
   index= (-1);
}

//----------------------------------------------------------------------------
//
// Method-
//       Word.get
//
// Purpose-
//       Extract Word.
//
//----------------------------------------------------------------------------
public static Word                  // The associated Word
   get(                             // Get associated Word
     String            text)        // The Word text
{
   return treeMap.get(text);
}

public static Word                  // The associated Word
   get(                             // Get associated Word
     int               index)       // The Word index
{
   return vector.get(index);
}

//----------------------------------------------------------------------------
//
// Method-
//       Word.getCount
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
//       Word.getIndex
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
//       Word.getType
//
// Purpose-
//       Get the type.
//
//----------------------------------------------------------------------------
public int                          // The type
   getType( )                       // Get type
{
   return 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       Word.getString
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
//       Word.setCount
//
// Purpose-
//       Set the insert counter.
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
//       Word.increment
//
// Purpose-
//       Increment the count.
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
//       Word.insert
//
// Purpose-
//       Insert a word.
//
//----------------------------------------------------------------------------
public static void
   insert(                          // Insert a Word
     Word              word)        // The word
{
   Word mapped= treeMap.get(word.text);
   if( mapped != null )
   {
     word= mapped;
     word.count++;
   }
   else
   {
     vector.add(word);
     word.index= vector.size() - 1;
     treeMap.put(word.text, word);
   }
}

public static void
   insert(                          // Insert a Word
     String            string)      // The Word String
{
   Word word= new Word(string);
   insert(word);
}

//----------------------------------------------------------------------------
//
// Method-
//       Word.check
//
// Purpose-
//       Verify the word list.
//
//----------------------------------------------------------------------------
public static void
   check( )                         // Verify the word list
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
//       Word.display
//
// Purpose-
//       Display the word list.
//
//----------------------------------------------------------------------------
public static void
   display( )                       // Display the word list
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
//       Word.setVector
//
// Purpose-
//       Set the Vector from the TreeMap.
//
//----------------------------------------------------------------------------
protected static void
   setVector( )                     // Set the Vector from the TreeMap
   throws IOException
{
   vector= new Vector<Word>();
   for(Iterator i= treeMap.entrySet().iterator(); i.hasNext();)
   {
     StringFormat format= new StringFormat();
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
//       Word.load
//
// Purpose-
//       Load the word list.
//
//----------------------------------------------------------------------------
@SuppressWarnings("unchecked")
public static void
   load(                            // Load the word list
     java.io.ObjectInputStream
                       inp)         // From this Input Stream
   throws IOException, ClassNotFoundException
{
   treeMap= (TreeMap<String,Word>)inp.readObject();
   setVector();
}

public static void
   load(                            // Load the word list
     Reader            inp)         // From this Reader
   throws IOException
{
   BufferedReader br= new BufferedReader(inp);

   treeMap= new TreeMap<String,Word>();

   for(;;)
   {
     String line= br.readLine();
     if( line == null )
       break;

     Tokenizer token= new Tokenizer(line);
     int count= Integer.decode(token.get()).intValue();
     String text= token.get();

     Word word= new Word(text);
     word.count= count;
     Word mapped= treeMap.get(text);
     if( mapped == null )
     {
       vector.add(word);
       word.index= vector.size() - 1;
       treeMap.put(word.text, word);
     }
   }

   setVector();
}

//----------------------------------------------------------------------------
//
// Method-
//       Word.store
//
// Purpose-
//       Store the word list.
//
//----------------------------------------------------------------------------
public static void
   store(                           // Store the word list
     java.io.ObjectOutputStream
                       out)         // Into this Output Stream
   throws IOException
{
   out.writeObject(treeMap);
}

public static void
   store(                           // Store the word list
     Writer            out)         // Into this Writer
   throws IOException
{
   for(Iterator i= treeMap.entrySet().iterator(); i.hasNext();)
   {
     StringFormat format= new StringFormat();
     Map.Entry me= (Map.Entry)i.next();
     String text= (String)me.getKey();
     Word   word= (Word)me.getValue();

     format.setRadix(10).append(word.getCount(), 12);
     out.write(format.toString() + " " + text + "\n");
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Word.readObject
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
//       Word.writeObject
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
} // class Word

