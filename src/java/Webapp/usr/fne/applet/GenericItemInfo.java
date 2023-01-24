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
//       GenericItemInfo.java
//
// Purpose-
//       Generic item information container class.
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
//       GenericItemInfo
//
// Purpose-
//       Generic item information container class.
//
//----------------------------------------------------------------------------
public class GenericItemInfo  extends DatabaseInfo {
String                 item;        // The item String

//----------------------------------------------------------------------------
//
// Method-
//       GenericItemInfo.debug
//
// Purpose-
//       Extend DebuggingAdaptor
//
//----------------------------------------------------------------------------
public void
   debug( )                         // Debugging display
{
   super.debug();
   print(".item: " + item);
}

//----------------------------------------------------------------------------
//
// Method-
//       GenericItemInfo.GenericItemInfo
//
// Purpose-
//       Constructors.
//
//----------------------------------------------------------------------------
   GenericItemInfo(                 // Constructor
     String            string)      // The item String
{
   item= stripQuotes(string);
   if( item == null )
   {
     isChanged= true;
     item= "Unknown";
   }
   else
     isPresent= true;
}

   GenericItemInfo( )               // Constructor
{
   item= "Unknown";
   isChanged= true;
}

//----------------------------------------------------------------------------
//
// Method-
//       GenericItemInfo.genPanel
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
   panel= new DataPanel(1);
   DataField[] df= panel.getField();
   df[0].setText(item);
   df[0].setColumns(100);

   if( listener != null )
   {
     df[0].addFocusListener(listener);
     df[0].setEditable(true);
   }

   if( tail != null )
   {
     tail.addFocusListener(new NextFocusAdapter(df[0]));
     tail= df[0];
   }

   return tail;
}

//----------------------------------------------------------------------------
//
// Method-
//       GenericItemInfo.isValid
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
   // An item panel is validated whether or not is is removed
   if( panel != null )
   {
     DataField[] df= panel.getField();
     if( !v.isValidText("Item", df[0]) )
       return false;
   }

   return true;
}

//----------------------------------------------------------------------------
//
// Method-
//       GenericItemInfo.toString
//
// Purpose-
//       Extract the item into a String.
//
//----------------------------------------------------------------------------
public String                       // Resultant String
   toString( )                      // Convert to String
{
   return addQuotes(item);
}

//----------------------------------------------------------------------------
//
// Method-
//       GenericItemInfo.update
//
// Purpose-
//       Update the internal fields, setting isChanged as appropriate.
//
//----------------------------------------------------------------------------
public synchronized void
   update( )                        // Update from panel
{
   if( panel != null )
   {
     DataField df[]= panel.getField();
     if( !item.equals(df[0].getText()) )
     {
       isChanged= true;
       item= df[0].getText();
     }
   }
}
} // class GenericItemInfo
