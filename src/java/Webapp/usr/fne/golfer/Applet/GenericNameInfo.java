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
//       GenericNameInfo.java
//
// Purpose-
//       Generic name information container class.
//
// Last change date-
//       2020/01/15
//
//----------------------------------------------------------------------------
import java.awt.event.FocusListener;
import java.lang.*;
import java.util.*;
import javax.swing.*;

//----------------------------------------------------------------------------
//
// Class-
//       GenericNameInfo
//
// Purpose-
//       Generic name information container class.
//
//----------------------------------------------------------------------------
public class GenericNameInfo  extends DatabaseInfo {
String[]               name;        // The name String array

//----------------------------------------------------------------------------
//
// Method-
//       GenericNameInfo.debug
//
// Purpose-
//       Extend DebuggingAdaptor
//
//----------------------------------------------------------------------------
public void
   debug( )                         // Debugging display
{
   super.debug();
   print(".name: " + name[0] + "," + name[1]);
}

//----------------------------------------------------------------------------
//
// Method-
//       GenericNameInfo.GenericNameInfo
//
// Purpose-
//       Constructors.
//
//----------------------------------------------------------------------------
   GenericNameInfo(                 // Constructor
     String[]          string)      // The name String
{
   name= string;
   if( name == null )
   {
     isChanged= true;
     name= new String[0];
   }
   else
     isPresent= true;

   if( name.length != 2 )
   {
     String[] copy= new String[2];
     copy[0]= "Unknown name";
     copy[1]= "Unknown location";

     if( name.length >= 2 )
       copy[1]= name[1];
     if( name.length >= 1 )
       copy[0]= name[0];

     name= copy;
   }

   name[0]= stripQuotes(name[0]);
   name[1]= stripQuotes(name[1]);
}

   GenericNameInfo(                 // Constructor
     String            string)      // The name String
{
   this(tokenize(string));
}

   GenericNameInfo( )               // Constructor
{
   this(empty);
}

//----------------------------------------------------------------------------
//
// Method-
//       GenericNameInfo.genPanel
//
// Purpose-
//       Generate the internal panel, optional FocusListener and NextFocusAdapter
//
//----------------------------------------------------------------------------
public synchronized DataField       // Resultant tail DataField
   genPanel(                        // Generate new Panel
     FocusListener     listener,    // (OPTIONAL) FocusListener
     DataField         tail)        // (OPTIONAL) The current tail DataField
{
   // The panel is always generated
   panel= new DataPanel(2);
   DataField[] df= panel.getField();
   df[0].setText(name[0]);
   df[0].setColumns(50);
   df[1].setText(name[1]);
   df[1].setColumns(50);

   if( listener != null )
   {
     df[0].addFocusListener(listener);
     df[0].setEditable(true);
     df[1].addFocusListener(listener);
     df[1].setEditable(true);
   }

   if( tail != null )
   {
     tail.addFocusListener(new NextFocusAdapter(df[0]));
     tail= df[1];
   }

   return tail;
}

//----------------------------------------------------------------------------
//
// Method-
//       GenericNameInfo.isValid
//
// Purpose-
//       Validate the panel.
//
// Validators-
//       title, if present, must be a valid name.
//       array must be numeric.
//
//----------------------------------------------------------------------------
public synchronized boolean         // TRUE iff valid
   isValid(                         // Is panel valid
     Validator         v)           // Validator
{
   // A name panel is validated whether or not is is removed
   if( panel != null )
   {
     DataField[] df= panel.getField();
     if( !v.isValidText("Name", df[0]) || !v.isValidText("Location", df[1]) )
       return false;
   }

   return true;
}

//----------------------------------------------------------------------------
//
// Method-
//       GenericNameInfo.toString
//
// Purpose-
//       Extract the name into a String.
//
//----------------------------------------------------------------------------
public String                       // Resultant String
   toString( )                      // Convert to String
{
   String result= null;

   synchronized(this)
   {{{{
     if( panel != null )
     {
       DataField[] df= panel.getField();
       result= addQuotes(df[0].getText()) + " "
             + addQuotes(df[1].getText());
     }
   }}}} // synchronized(this)

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       GenericNameInfo.update
//
// Purpose-
//       Update the internal fields, setting isChanged as appropriate.
//
//----------------------------------------------------------------------------
public synchronized void
   update( )                        // Update from panel
{
   // A name panel is updated whether or not is is removed
   if( panel != null )
   {
     DataField df[]= panel.getField();
     for(int i= 0; i<df.length; i++)
     {
       if( !name[i].equals(df[i].getText()) )
       {
         isChanged= true;
         name[i]= df[i].getText();
       }
     }
   }
}
} // class GenericNameInfo
