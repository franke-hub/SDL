//----------------------------------------------------------------------------
//
//       Copyright (C) 2008-2020 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       DatabaseInfo.java
//
// Purpose-
//       Database item base class.
//
// Last change date-
//       2020/01/15
//
// Usage--
//       Fields in this class are public. Access them directly.
//
//----------------------------------------------------------------------------
import java.awt.event.FocusListener;
import java.io.IOException;
import java.lang.System;

//----------------------------------------------------------------------------
//
// Class-
//       DatabaseInfo
//
// Purpose-
//       Database item base class.
//
//----------------------------------------------------------------------------
public class DatabaseInfo extends DebuggingAdaptor {
public boolean         isChanged;   // Is changed in database
public boolean         isPresent;   // Is present in database
public boolean         isRemoved;   // Is removed from database

// Data panel
DataPanel              panel;       // The associated Panel

// Constants
static final String[]  empty= null; // Differentiates String and String[] null

//----------------------------------------------------------------------------
//
// Method-
//       DatabaseInfo.DebuggingAdaptor
//
// Purpose-
//       Extend DebuggingAdaptor
//
//----------------------------------------------------------------------------
public void
   debug( )                         // Object debugging message
{
   print(".isChanged: " + isChanged);
   print(".isPresent: " + isPresent);
   print(".isRemoved: " + isRemoved);
   print(".panel: " + (panel != null));
}

public boolean                      // TRUE iff debug should write
   isDebug( )                       // Is debugging active?
{
   return false;                    // DEBUGGING control
}

//----------------------------------------------------------------------------
//
// Method-
//       DatabaseInfo.METHODS
//
// Purpose-
//       Methods typically overridden in subclasses.
//
//----------------------------------------------------------------------------
public synchronized boolean         // TRUE iff valid
   isValid(                         // Is panel valid
     Validator         v)           // Validator
{
   return false;
}

public String                       // Resultant String
   toString( )                      // Convert to String
{
   return null;
}

public synchronized void
   update( )                        // Update from panel
{
}

//----------------------------------------------------------------------------
//
// Method-
//       DatabaseInfo.UTILITIES
//
// Purpose-
//       Utility methods
//
//----------------------------------------------------------------------------
protected static String             // Resultant
   addQuotes(                       // Add quotes
     String            string)      // Onto this String
{
   return DbStatic.addQuotes(string);
}

protected static String             // Resultant
   stripQuotes(                     // Strip quotes
     String            string)      // From this String
{
   return DbStatic.stripQuotes(string);
}

protected static String[]           // Resultant
   tokenize(                        // Tokenize
     String            string)      // This String
{
   String[] result= null;

   if( string != null )
     result= DbStatic.tokenize(string);

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       DatabaseInfo.arrayToQuotedString
//
// Purpose-
//       Extract an array into space separated quoted String objects.
//
//----------------------------------------------------------------------------
public static String                // Resultant String
   arrayToQuotedString(             // Convert to String
     String[]          array)       // This array
{
   StringBuffer result= new StringBuffer();

   if( array == null )
     return null;

   for(int i= 0; i<array.length; i++)
     result.append((i == 0 ? "\"" : " \"") + array[i] + "\"");

   return result.toString();
}

//----------------------------------------------------------------------------
//
// Method-
//       DatabaseInfo.arrayToString
//
// Purpose-
//       Extract an array into space separated String objects.
//
//----------------------------------------------------------------------------
public static String                // Resultant String
   arrayToString(                   // Convert to String
     String[]          array)       // This array
{
   StringBuffer result= new StringBuffer();

   if( array == null )
     return null;

   for(int i= 0; i<array.length; i++)
     result.append((i == 0 ? "" : " ") + array[i]);

   return result.toString();
}

//----------------------------------------------------------------------------
//
// Method-
//       DatabaseInfo.genPanel
//
// Purpose-
//       Convenience method. Generates a read/only panel.
//
//----------------------------------------------------------------------------
public synchronized DataField       // Resultant tail DataField
   genPanel(                        // Generate new Panel
     FocusListener     listener,    // (OPTIONAL) FocusListener
     DataField         tail)        // (OPTIONAL) The current tail DataField
{
   return tail;
}

public DataPanel                    // Resultant DataPanel
   genPanel( )                      // Generate new Panel
{
   genPanel(null, null);
   return getPanel();
}

//----------------------------------------------------------------------------
//
// Method-
//       DatabaseInfo.getPanel
//
// Purpose-
//       Return the associated DataPanel.
//
//----------------------------------------------------------------------------
public DataPanel                    // The DataPanel
   getPanel( )                      // Get DataPanel
{
   return panel;
}

//----------------------------------------------------------------------------
//
// Method-
//       DatabaseInfo.insert
//
// Purpose-
//       Insert the database data, update the state.
//
//----------------------------------------------------------------------------
public synchronized void
   insert(                          // Insert database data
     DbClient          client,      // The database client
     String            type,        // The database item type
     String            item)        // The database item key
{
   isRemoved= false;
   output(client, type, item);
}

//----------------------------------------------------------------------------
//
// Method-
//       DatabaseInfo.output
//
// Purpose-
//       Change the database data, update the state.
//
//----------------------------------------------------------------------------
public synchronized void
   output(                          // Output to database
     DbClient          client,      // The database client
     String            type,        // The database item type
     String            item)        // The database item key
{
   if( isRemoved )                  // If the item is removed
   {
     if( isPresent )                // If it is present
     {
       try {
         client.remove(type, item);
       } catch(IOException e) {
         System.out.println("client.remove: " + e);
       } catch(Exception e) {
         e.printStackTrace();
         System.out.println("client.remove: " + e);
       }
     }

     isChanged= false;
     isPresent= false;
   }
   else                             // If the item is extant
   {
     if( isChanged || !isPresent )
     {
       try {
         client.put(type, item, toString());
       } catch(IOException e) {
         System.out.println("client.put: " + e);
       } catch(Exception e) {
         e.printStackTrace();
         System.out.println("client.put: " + e);
       }
     }

     isChanged= false;
     isPresent= true;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       DatabaseInfo.remove
//
// Purpose-
//       Remove the database data, update the state.
//
//----------------------------------------------------------------------------
public synchronized void
   remove(                          // Remove from database
     DbClient          client,      // The database client
     String            type,        // The database item type
     String            item)        // The database item key
{
   isRemoved= true;
   output(client, type, item);
}
} // class DatabaseInfo
