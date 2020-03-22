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
//       user.util.HashVector.java
//
// Purpose-
//       Define the HashVector class.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
package user.util;

import java.util.Enumeration;
import java.util.Hashtable;
import java.util.Vector;

//----------------------------------------------------------------------------
//
// Class-
//       HashVector
//
// Purpose-
//       A hash table whose objects are Vectors.
//
//----------------------------------------------------------------------------
/**
 * The HashVector object contains a Hashtable whose value objects are Vectors,
 * thus allowing a single key to be associated with multiple values.
 *
 * @see java.lang.Hashtable
 */
public class HashVector
{
//----------------------------------------------------------------------------
// HashVector.attributes
//----------------------------------------------------------------------------
Hashtable              table;       // The hash table

//----------------------------------------------------------------------------
//
// Method-
//       HashVector.HashVector
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
/**
 * Construct a HashVector with default attributes, similarly to Hashtable.
 *
 * @see java.lang.Hashtable
 */
public
   HashVector( )                  // Constructor
{
   table= new Hashtable();
}

/**
 * Construct a HashVector with a specified capacity, similarly to Hashtable.
 *
 * @see java.lang.Hashtable
 */
public
   HashVector(                      // Constructor
     int               capacity)    // Initial capacity
{
   table= new Hashtable(capacity);
}

/**
 * Construct a HashVector with a specified capacity and load factor,
 * similarly to Hashtable.
 *
 * @see java.lang.Hashtable
 */
public
   HashVector(                      // Constructor
     int               capacity,    // Initial capacity
     float             loadFactor)  // Initial load factor
{
   table= new Hashtable(capacity, loadFactor);
}

//----------------------------------------------------------------------------
//
// Method-
//       HashVector.clear
//
// Purpose-
//       Clear the HashVector so that it contains no elements
//
//----------------------------------------------------------------------------
/**
 * Clear the HashVector, similarly to Hashtable.
 *
 * @see java.lang.Hashtable
 */
public void
   clear( )                         // Clear the HashVector
{
   table.clear();
}

//----------------------------------------------------------------------------
//
// Method-
//       HashVector.getVector
//
// Purpose-
//       Return the Vector of elements which match the specified key
//
//----------------------------------------------------------------------------
/**
 * Return the Vector associated with the specified key.
 *
 * Instead of returning a null value when there is no Vector associated with
 * a key, an empty Vector is returned.  An empty Vector is not necessarily
 * associated with the key.
 *
 * The resultant Vector should be treated as constant.  Unpredictable results
 * occur if the Vector is modified.  Future implementations may get more
 * picky and, for example, return a clone of the resultant Vector.
 *
 * @see java.lang.Hashtable
 */
public synchronized Vector          // Resultant (may be empty)
   getVector(                       // Return all instances
     Object            key)         // Matching this key
{
   Vector              v;           // Resultant

   v= (Vector)table.get(key);       // The resultant Vector
   if( v == null )                  // If empty Vector
     v= new Vector();               // Return an empty Vector

   return v;
}

//----------------------------------------------------------------------------
//
// Method-
//       HashVector.exists
//
// Purpose-
//       Determine whether a key/value pair is in the table.
//
//----------------------------------------------------------------------------
/**
 * Determine whether a specific key/value pair exists in the HashVector.
 *
 * This method uses the equals method both for the key Object and for
 * each object in the associated Vector.
 */
public synchronized boolean         // TRUE if key/value pair was found
   exists(                          // Remove a key/value pair
     Object            key,         // Use this key
     Object            value)       // Use thie value
{
   Enumeration         e;           // An Vector enumeration
   Object              o;           // The current Vector object
   Vector              v;           // The resultant Vector

   v= (Vector)getVector(key);       // Get the resultant Vector
   for(e= v.elements(); e.hasMoreElements(); )
   {
     o= e.nextElement();
     if( o.equals(value) )
       return true;
   }

   return false;
}

//----------------------------------------------------------------------------
//
// Method-
//       HashVector.insert
//
// Purpose-
//       Insert a key/value pair into the table.
//
//----------------------------------------------------------------------------
/**
 * Insert a key/value pair into the HashVector.  Duplicates are allowed.
 *
 * This method associates a value Object with a key Object.  Since duplicate
 * associations are explicitly allowed, the value Object is added to a
 * Vector object associated with the key.
 */
public synchronized void
   insert(                          // Insert a key/value pair
     Object            key,         // Use this key
     Object            value)       // Use thie value
{
   Vector              v;           // The resultant Vector

   v= (Vector)table.get(key);       // The resultant Vector
   if( v == null )                  // If empty Vector
   {
     v= new Vector();               // Create a new Vector
     table.put(key, v);             // Add it to the table
   }

   v.addElement(value);             // Add the resultant Vector
}

//----------------------------------------------------------------------------
//
// Method-
//       HashVector.remove
//
// Purpose-
//       Remove a key/value pair from the table.
//
//----------------------------------------------------------------------------
/**
 * Remove the first instance of a key/value pair from the HashVector.
 *
 * @returns true: if a matching key/value pair existed in the HashVector
 * (and was removed.)
 * @returns false: otherwise.
 */
public synchronized boolean         // TRUE if element was found
   remove(                          // Remove a key/value pair
     Object            key,         // Use this key
     Object            value)       // Use this value
{
   boolean             result;      // Resultant
   Vector              v;           // The resultant Vector

   v= (Vector)table.get(key);       // The resultant Vector
   if( v == null )                  // If empty Vector
     return false;                  // It's not there

   result= v.removeElement(value);  // Remove the Object from the Vector
   if( v.isEmpty() )                // If the Vector is now empty
     table.remove(key);             // Remove it from the table

   return result;
}
} // Class HashVector

