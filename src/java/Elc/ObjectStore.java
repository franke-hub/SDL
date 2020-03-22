//----------------------------------------------------------------------------
//
//       Copyright (C) 2007-2013 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       ObjectStore.java
//
// Purpose-
//       Define the ObjectStore database I/O layer.
//
// Last change date-
//       2013/01/01
//
// Description-
//       The ObjectStore is the mechanism used to access Serializable database
//       items. Objects are accessed by an object key, always a java long.
//
//       See: README.DATA
//
// Notes-
//       This version does not use locking.
//       All external operations are synchronized.
//
//----------------------------------------------------------------------------
import java.io.*;
import java.lang.*;
import java.util.*;

import user.util.*;
import user.util.logging.*;

//----------------------------------------------------------------------------
//
// Class-
//       ObjectStore
//
// Purpose-
//       The ObjectStore database.
//
//----------------------------------------------------------------------------
class ObjectStore                   // ObjectStore
{
//----------------------------------------------------------------------------
// ObjectStore.Controls
//----------------------------------------------------------------------------
final static boolean   HCDM= false; // Hard Core Debug Mode?
final static boolean   IODM= HCDM;  // I/O Debug Mode?
final static boolean   SCDM= HCDM;  // Soft Core Debug Mode?
final static int       init_cacheSize= 1024; // The initial cache size

final static String    hextab= "0123456789ABCDEF"; // Hex conversion table
final static String    X_OBJ= ".obj"; // Object filename extension
final static String    X_OLD= ".old"; // Old Object filename extension

//----------------------------------------------------------------------------
// ObjectStore.Cache
//----------------------------------------------------------------------------
private class Cache                 // Cache item
{
  int                  index;       // The cache index, or next free index
  int                  count;       // The use count
  long                 ident;       // The Object identifier
  long                 lru;         // The Object LRU sequence
  Serializable         obj;         // The Serializable Object
}; // class Cache

//----------------------------------------------------------------------------
// ObjectStore.Attributes
//----------------------------------------------------------------------------
protected String       root;        // The root directory name
protected StreamLogger logger;      // The Logger

protected Cache[]      cache;       // The Object cache, cache[0] unused
protected int          freeX;       // The first free cache index
protected long         nextLRU;     // The next assigned LRU identifier
protected Hashtable<Long,Cache>
                       mapIDENT;    // The IDENT map
protected TreeMap<Long,Cache>
                       mapLRU;      // The LRU map

//----------------------------------------------------------------------------
//
// Method-
//       ObjectStore.findCache
//
// Purpose-
//       Return the next free Cache element
//
//----------------------------------------------------------------------------
protected Cache                     // The next free Cache item
   findCache( )                     // Allocate a Cache element
{
   int                 X;           // Resultant index

   if( freeX != 0 )
   {
     X= freeX;
     freeX= cache[X].index;
   }
   else
   {
     Map.Entry<Long,Cache> entry= mapLRU.firstEntry();
     if( entry == null )
     {
       int oldSize= cache.length;
       int newSize= oldSize + oldSize / 4;
       logger.log("ObjectStore.findCache() expand from(" +
                  oldSize + ") to(" + newSize + ")");
       Cache[] nc= new Cache[newSize];

       // Copy the existing cache
       for(int index= 0; index<oldSize; index++)
         nc[index]= cache[index];

       // Expand the cache, leaving the last entry allocated
       X= newSize-1;
       for(int index=oldSize; index<X; index++)
       {
         nc[index]= new Cache();
         nc[index].index= index;
         freeCache(nc[index]);
       }
       nc[X]= new Cache();

       cache= nc;
       // Cannot check here: nc[X] has no object.
     }
     else
     {
       Cache cache= entry.getValue();
       mapIDENT.remove(cache.ident);
       mapLRU.remove(cache.lru);
       X= cache.index;
     }
   }

   cache[X].index= X;
   cache[X].count= 1;
   cache[X].ident= 0;
   cache[X].lru= 0;
   cache[X].obj= null;

   return cache[X];
}

//----------------------------------------------------------------------------
//
// Method-
//       ObjectStore.finalize
//
// Purpose-
//       Finalize the ObjectStore
//
//----------------------------------------------------------------------------
protected void
   finalize( )                      // Finalize the ObjectStore
   throws Throwable
{
   logger.log("ObjectStore.finalize()");
   close();
}

//----------------------------------------------------------------------------
//
// Method-
//       ObjectStore.freeCache
//
// Purpose-
//       Add a Cache element to the free list
//
//----------------------------------------------------------------------------
protected void
   freeCache(                       // Release a Cache element
     Cache             cache)       // The element to release
{
   int index= cache.index;
   cache.index= freeX;
   freeX= index;

   cache.count= 0;
   cache.ident= 0;
   cache.lru= 0;
   cache.obj= null;
}

//----------------------------------------------------------------------------
//
// Method-
//       ObjectStore.identFile
//
// Purpose-
//       Convert an identifier to the associated filename.
//
//----------------------------------------------------------------------------
protected static String             // The file name String
   identFile(                       // Convert identifier to file name String
     long              ident)       // The identifier
{
   char[]              result= new char[16];

   int                 i;

   for(i= 0; i<16; i++)
   {
     int x= (int)(ident >> (4*(15-i)));
     x &= 0x0000000f;
     result[i]= hextab.charAt(x);
   }

   return new String(result, 0,4) + "-" +
          new String(result, 4,4) + "-" +
          new String(result, 8,4) + "-" +
          new String(result,12,4);
}

//----------------------------------------------------------------------------
//
// Method-
//       ObjectStore.identPath
//
// Purpose-
//       Convert an identifier to the associated pathname.
//
//----------------------------------------------------------------------------
protected String                    // The path name String
   identPath(                       // Convert identifier to path name String
     long              ident)       // The identifier
{
   char[]              result= new char[16];

   int                 i;

   for(i= 0; i<16; i++)
   {
     int x= (int)(ident >> (4*(15-i)));
     x &= 0x0000000f;
     result[i]= hextab.charAt(x);
   }

   return root                    + "/" +
          new String(result, 0,5) + "/" +
          new String(result, 5,3) + "/" +
          new String(result, 8,4) + "/";
}

//----------------------------------------------------------------------------
//
// Method-
//       ObjectStore.objectRD
//
// Purpose-
//       Read an Object from the store
//
//----------------------------------------------------------------------------
protected Cache                     // The resultant Cache container
   objectRD(                        // Read an Object
     long              ident)       // Object identifier
{
   Cache               cache= null; // Resultant

   File                objFile= null; // path + file + X_OBJ
   File                oldFile= null; // path + file + X_OLD
   String              file= identFile(ident);
   String              path= identPath(ident);

   // Clean up any half-baked writes
   objFile= new File(path + file + X_OBJ);
   oldFile= new File(path + file + X_OLD);
   if( oldFile.exists() )
   {
     if( objFile.exists() )
     {
       if( !objFile.delete() )
       {
         logger.log("ERROR: ObjectStore.objectRD(" + file + ") " +
                    "Unable to delete " + X_OBJ);
         objFile= new File(path + file + X_OLD);
       }
       else if( !oldFile.renameTo(objFile) )
       {
         logger.log("ERROR: ObjectStore.objectRD(" + file + ") " +
                    "Unable to rename " + X_OLD + " => " + X_OBJ);
         objFile= new File(path + file + X_OLD);
       }
     }
     else
     {
       if( !oldFile.renameTo(objFile) )
       {
         logger.log("ERROR: ObjectStore.objectRD(" + file + ") " +
                    "Unable to rename " + X_OLD + " => " + X_OBJ);
         objFile= new File(path + file + X_OLD);
       }
     }
   }

   // Read the Object
   if( objFile.exists() )
   {
     try {
       ObjectInputStream ois= new ObjectInputStream(new FileInputStream(objFile));
       Object obj= ois.readObject();
       ois.close();

       cache= findCache();
       cache.ident= ident;
       cache.obj= (Serializable)obj;
       mapIDENT.put(ident, cache);
     } catch (Exception x) {
       logger.log("ObjectStore.objectRD(" + identFile(ident) + ")", x);
     }
   }

   // Return the result
   if( HCDM )
     logger.log((cache == null ? "<NULL>" : "OBJECT") +
                "= ObjectStore.objectRD(" + identFile(ident) + ")");
   return cache;
}

//----------------------------------------------------------------------------
//
// Method-
//       ObjectStore.objectWR
//
// Purpose-
//       Write an Serializable Object into the store.
//
// Sequence-
//       xxxx-xxxx-xxxx-xxxx.obj => xxxx-xxxx-xxxx-xxxx.old
//       write: xxxx-xxxx-xxxx-xxxx.obj
//       erase: xxxx-xxxx-xxxx-xxxx.old
//
//----------------------------------------------------------------------------
protected void
   objectWR(                        // Write an Object
     Cache             cache)       // Object Cache container
{
   File                dirFile= null; // path + "."
   File                objFile= null; // path + file + X_OBJ
   File                oldFile= null; // path + file + X_OLD
   String              file= identFile(cache.ident);
   String              path= identPath(cache.ident);

   if( HCDM )
     logger.log("ObjectStore.objectWR(" + file + ")");

   // Create any necessary path components
   dirFile= new File(path + ".");
   if( !dirFile.exists() )
   {
     if( !dirFile.mkdirs() )
     {
       logger.log("ERROR: ObjectStore.objectWR(" + file + ") " +
                  "Unable to create subdirectories");
       return;
     }
   }
   dirFile= null;

   // Prepare to write, moving ~.obj to ~.old
   objFile= new File(path + file + X_OBJ);
   oldFile= new File(path + file + X_OLD);
   if( oldFile.exists() )
   {
     if( objFile.exists() )
     {
       if( !objFile.delete() )
         logger.log("ERROR: ObjectStore.objectWR(" + file + ") " +
                    "Unable to delete " + X_OBJ);
     }
   }
   else if( objFile.exists() )
   {
     if( !objFile.renameTo(oldFile) )
     {
       logger.log("ERROR: ObjectStore.objectRD(" + file + ") " +
                  "Unable to rename " + X_OBJ + " => " + X_OLD);
       objFile= new File(path + file + X_OBJ);
       oldFile= null;
     }
   }
   else
     oldFile= null;

   // Write the Object, ~.obj
   boolean error= true;
   try {
     ObjectOutputStream oos= new ObjectOutputStream(new FileOutputStream(objFile));
     oos.writeObject(cache.obj);
     oos.close();
     error= false;
   } catch (Exception x) {
     logger.log("ObjectStore.objectWR(" + file + ")", x);
   }

   // Complete the transaction, removing ~.old
   if( error )
   {
     objFile.delete();
     if( oldFile != null )
     {
       if( !oldFile.renameTo(objFile) )
       {
         logger.log("ERROR: ObjectStore.objectWR(" + file + ") " +
                    "Unable to rename " + X_OLD + " => " + X_OBJ);
       }
     }
   }
   else
   {
     if( oldFile != null )
     {
       if( !oldFile.delete() )
       {
         logger.log("ERROR: ObjectStore.objectWR(" + file + ") " +
                    "Unable to remove " + X_OLD);
       }
     }
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       ObjectStore.setLRU
//
// Purpose-
//       Add the Cache element to the LRU list
//
//----------------------------------------------------------------------------
protected void
   setLRU(                          // Add to LRU list
     Cache             cache)       // This Cache element
{
   if( cache.lru != 0 )
     logger.log("Error: SNO ObjectStore.setLRU(" + identFile(cache.ident) +
                ") cache.lru(" + cache.lru + ")");
   else
   {
     cache.lru= ++nextLRU;
     if( cache.lru <= 0 )
     {
       // LRU overflow, should not occur.
       // If it does, the LRU settings will temporarily be arbitrary.
       logger.log("ObjectStore.setLRU(" + nextLRU + ") lru overflow");
       nextLRU= 0;
       mapLRU= new TreeMap<Long,Cache>();
       for(int i= 0; i<this.cache.length; i++)
       {
         if( this.cache[i].lru != 0 )
         {
           this.cache[i].lru= ++nextLRU;
           mapLRU.put(this.cache[i].lru, this.cache[i]);
         }
       }

       check();
     }
     else
       mapLRU.put(cache.lru, cache);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       ObjectStore.ObjectStore
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
   ObjectStore(                     // Constructor
     String            propName)    // The properties file name
{
   Common              common= Common.get();
   Properties          properties= new Properties(); // The Common Properties

   if( HCDM )
     logger.log("ObjectStore.ObjectStore(" + propName + ")");

   //-------------------------------------------------------------------------
   // Set the logger
   logger= common.logger;           // Use the Common logger

   //-------------------------------------------------------------------------
   // Get Properties
   try
   {
     FileInputStream inp= new FileInputStream(propName);
     properties.load(inp);
   } catch(FileNotFoundException x) {
     logger.log("OStore.Properties(" + propName + ") not found");
   } catch(Exception x) {
     logger.log("OStore.Properties(" + propName + ")", x);
   }

   //-------------------------------------------------------------------------
   // Interpret Properties
   root= properties.getProperty("ObjectStore.Root", common.prefix + "_root/");
   if( root.substring(root.length()-1) != "/" )
     this.root= root + "/";

   //-------------------------------------------------------------------------
   // Initialize the cache
   freeX= 0;
   nextLRU= 0;
   mapLRU= new TreeMap<Long,Cache>();
   mapIDENT= new Hashtable<Long,Cache>();
   cache= new Cache[init_cacheSize];
   for(int index= 0; index<cache.length; index++)
   {
     cache[index]= new Cache();
     cache[index].index= index;
     freeCache(cache[index]);
   }

   // Check
   if( HCDM )
     check();
}

//----------------------------------------------------------------------------
//
// Method-
//       ObjectStore.check
//
// Purpose-
//       Check the ObjectStore
//
//----------------------------------------------------------------------------
public synchronized void
   check( )                         // Self-check
{
   int                 index= 0;    // Cache index
   Cache               cache= null; // Failing cache entry

   logger.log("ObjectStore.check()");

   // Catch exceptions
   try {
     // Check the IDENT map.
     for(Iterator i= mapIDENT.entrySet().iterator(); i.hasNext();)
     {
       Map.Entry me= (Map.Entry)i.next();

       long ident= (Long)me.getKey();
       cache= (Cache)me.getValue();

       Debug.verify( cache.index != 0 );
       Debug.verify( cache.equals(this.cache[cache.index]) );
       if( cache.count == 0 )
         Debug.verify( cache.lru != 0 );
       else
         Debug.verify( cache.lru == 0 );
       Debug.verify( cache.ident == ident );
       if( cache.lru != 0 )
       {
         Debug.verify( cache.count == 0 );
         Debug.verify( cache.equals(mapLRU.get(cache.lru)) );
       }
       Debug.verify( cache.obj != null );
     }

     // Check the LRU map.
     for(Iterator i= mapLRU.entrySet().iterator(); i.hasNext();)
     {
       Map.Entry me= (Map.Entry)i.next();

       long lru= (Long)me.getKey();
       cache= (Cache)me.getValue();

       Debug.verify( cache.index != 0 );
       Debug.verify( cache.equals(this.cache[cache.index]) );
       Debug.verify( cache.count == 0 );
       Debug.verify( cache.equals(mapIDENT.get(cache.ident)) );
       Debug.verify( cache.lru == lru );
       Debug.verify( cache.obj != null );
     }

     // Check the CACHE
     for(index= 1; index<this.cache.length; index++)
     {
       cache= this.cache[index];

       if( cache.count == 0 && cache.lru == 0 )
       {
         Debug.verify( cache.count == 0 ) ;
         Debug.verify( cache.ident == 0 ) ;
         Debug.verify( cache.obj == null ) ;

         boolean found= false;
         for(int j= freeX; j != 0; j= this.cache[j].index)
         {
           if( index == j )
           {
             found= true;
             break;
           }
         }

         if( !found )
           throw new Exception("Cache[" + index + "] not on free list");
       }
       else
       {
         Debug.verify( cache.index == index );
         Debug.verify( cache.equals(mapIDENT.get(cache.ident)) );
         if( cache.lru != 0 )
         {
           Debug.verify( cache.count == 0 );
           Debug.verify( cache.equals(mapLRU.get(cache.lru)) );
         }
       }
     }

   // Catch, mostly ignore exceptions
   } catch( Exception x) {
     logger.log("Exception:", x);
     if( cache != null )
       logger.log((index != 0 ? "[" + index + "] " : "") + debugCache(cache));
     debug();
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       ObjectStore.debugCache
//
// Purpose-
//       Debug a Cache element.
//
//----------------------------------------------------------------------------
protected String                    // Cache debug String
   debugCache(                      // Write debugging messages
     Cache             cache)       // For this Cache element
{
   return "index(" + cache.index + ") " +
          "count(" + cache.count + ") " +
          "ident(" + identFile(cache.ident) + ") " +
          "lru(" + identFile(cache.lru) + ") " +
          (cache.obj == null ? "<NULL>" : "OBJECT");
}

//----------------------------------------------------------------------------
//
// Method-
//       ObjectStore.debug
//
// Purpose-
//       Debug the ObjectStore
//
//----------------------------------------------------------------------------
public synchronized void
   debug( )                         // Write debugging messages
{
   logger.log("ObjectStore.debug()");
   logger.log("freeX(" + freeX + ")");
   logger.log("nextLRU(" + nextLRU + ")");
   for(int index= 0; index<cache.length; index++)
     logger.log("[" + index + "] " + debugCache(cache[index]));
}

//----------------------------------------------------------------------------
//
// Method-
//       ObjectStore.getIdentString
//
// Purpose-
//       Public version of identFile, may differ in future.
//
//----------------------------------------------------------------------------
public static String                // The associated String
   getIdentString(                  // Convert identifier to file name String
     long              ident)       // The identifier
{
   return identFile(ident);
}

//----------------------------------------------------------------------------
//
// Method-
//       ObjectStore.change
//
// Purpose-
//       Modify an Object. The use count is unchanged.
//
//----------------------------------------------------------------------------
public synchronized void
   change(                          // Change an Object
     long              ident,       // Object identifier
     Serializable      object)      // The Serializable Object to change
{
   Cache               cache;       // The cache entry

   if( HCDM )
     logger.log("ObjectStore.change(" + identFile(ident) + ")");

   cache= mapIDENT.get(ident);
   if( cache == null )
   {
     cache= findCache();
     cache.ident= ident;
     cache.obj= object;

     mapIDENT.put(ident, cache);
     cache.count= 0;
     setLRU(cache);
   }
   else
   {
     if( cache.count == 0 )
       logger.log("Error: SNO ObjectStore.change(" + identFile(ident) +
                  ") cache.count(0)");

     else if( cache.lru != 0 )
     {
       logger.log("Error: SNO ObjectStore.change(" + identFile(ident) +
                  ") cache.lru(" + cache.lru + ")");
       mapLRU.remove(cache.lru);
       cache.lru= 0;
     }
   }

   cache.obj= object;
   objectWR(cache);
}

//----------------------------------------------------------------------------
//
// Method-
//       ObjectStore.close
//
// Purpose-
//       Close the ObjectStore
//
//----------------------------------------------------------------------------
public synchronized void
   close( )                         // Close the ObjectStore
   throws Throwable
{
   logger.log("ObjectStore.close()");

   // Insure that there are no outstanding fetch operations
   for(int index= 0; index<cache.length; index++)
   {
     if( cache[index].count != 0 )
     {
       logger.log("Error: cache[" + index + "] fetch without store");
       logger.log(debugCache(cache[index]));
     }
   }

   check();
   if( HCDM )
     debug();
}

//----------------------------------------------------------------------------
//
// Method-
//       ObjectStore.fetchRD
//       ObjectStore.fetchWR
//
// Purpose-
//       Fetch an Object, incrementing the use count
//
//----------------------------------------------------------------------------
public synchronized Serializable    // The resultant Object, null if none
   fetchRD(                         // Fetch an Object for read
     long              ident)       // Object identifier
{
   Cache               cache= null; // Cache entry
   Serializable        result= null;// Resultant

   cache= mapIDENT.get(ident);
   if( cache == null )
     cache= objectRD(ident);
   else
   {
     if( cache.count == 0 )
     {
       if( mapLRU.remove(cache.lru) == null )
         logger.log("Error: SNO ObjectStore.fetch(" + identFile(ident) +
                    ") cache.lru(" + cache.lru + ")");
       cache.lru= 0;
     }

     cache.count++;
   }

   if( cache != null )
     result= cache.obj;

   if( HCDM )
     logger.log((result == null ? "<NULL>" : "OBJECT") +
                "= ObjectStore.fetchRD(" + identFile(ident) + ")");

   return result;
}

public Serializable                 // The resultant Object, null if none
   fetchWR(                         // Fetch an Object for update
     long              ident)       // Object identifier
{
   // TODO: Reserved for expansion
   return fetchRD(ident);
}

//----------------------------------------------------------------------------
//
// Method-
//       ObjectStore.flush
//
// Purpose-
//       Flush an object, removing it from the Cache.
//
//----------------------------------------------------------------------------
public synchronized void
   flush(                           // Store an Object
     long              ident)       // Object identifier
{
   Cache               cache;       // The cache entry

   if( HCDM )
     logger.log("ObjectStore.flush(" + identFile(ident) + ")");

   cache= mapIDENT.get(ident);
   if( cache != null )
   {
     if( cache.count == 0 )
     {
       if( cache.lru != 0 )
         mapLRU.remove(cache.lru);

       mapIDENT.remove(cache.ident);
       freeCache(cache);
     }
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       ObjectStore.remove
//
// Purpose-
//       Remove an Object from the ObjectStore.
//
//----------------------------------------------------------------------------
public synchronized Serializable    // The removed Serializable Object
   remove(                          // Remove an Object
     long              ident)       // Object identifier
{
   Cache               cache= null; // Cache entry
   Serializable        result= null;// Resultant

   if( HCDM )
     logger.log("ObjectStore.remove(" + identFile(ident) + ")");

   cache= mapIDENT.get(ident);
   if( cache == null )
   {
     cache= objectRD(ident);
     if( cache != null )
       cache.count= 0;
   }

   if( cache != null )
   {
     if( cache.count != 0 )
       logger.log("Error: SNO ObjectStore.remove(" + identFile(ident) +
                  ") cache.count(" + cache.count + ")");
     else
     {
       if( cache.lru != 0 )
         mapLRU.remove(cache.lru);
       mapIDENT.remove(cache.ident);

       // Remove the external object
       File file= new File(identPath(cache.ident) + identFile(cache.ident) + ".obj");
       if( file.delete() )
         result= cache.obj;

       // Release the cache entry
       freeCache(cache);
     }
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       ObjectStore.store
//
// Purpose-
//       Decrement the use count for an Object.
//
//----------------------------------------------------------------------------
public synchronized void
   store(                           // Store an Object
     long              ident)       // Object identifier
{
   Cache               cache;       // The cache entry

   if( HCDM )
     logger.log("ObjectStore.store(" + identFile(ident) + ")");

   cache= mapIDENT.get(ident);
   if( cache == null )
     logger.log("Error: SNO ObjectStore.store(" + identFile(ident) + ") cache(null)");
   else
   {
     if( cache.count == 0 )
       logger.log("Error: SNO ObjectStore.store(" + identFile(ident) + ") cache.count(0)");
     else
     {
       cache.count--;
       if( cache.count == 0 )
       {
         if( cache.lru != 0 )
           logger.log("Error: SNO ObjectStore.store(" + identFile(ident) +
                      ") cache.lru(" + cache.lru + ")");
         else
           setLRU(cache);
       }
     }
   }
}
}; // class ObjectStore

