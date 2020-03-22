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
//       GenericListInfo.java
//
// Purpose-
//       Generic named list information container class.
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
//       GenericListInfo
//
// Purpose-
//       Generic named list information container class.
//
//----------------------------------------------------------------------------
public class GenericListInfo  extends DatabaseInfo {
boolean                isQuoted;    // Are list items quoted?
String                 name;        // The list name
String[]               list;        // The list String[]

//----------------------------------------------------------------------------
//
// Method-
//       GenericListInfo.debug
//
// Purpose-
//       Extend DebuggingAdaptor
//
//----------------------------------------------------------------------------
public void
   debug( )                         // Debugging display
{
   super.debug();
   print(".isQuoted: " + isQuoted);
   print(".name: " + name);
   print(".list: " + arrayToString(list));
}

//----------------------------------------------------------------------------
//
// Method-
//       GenericListInfo.GenericListInfo
//
// Purpose-
//       Constructors.
//
//----------------------------------------------------------------------------
   GenericListInfo( )               // Default constructor
{
   name= "";
   list= new String[0];
}

   GenericListInfo(                 // Constructor
     String            name,        // The list name
     String[]          list)        // The list elements
{
   if( name == null )
     name= "";
   this.name= name;

   if( list == null )
     list= new String[0];
   else
   {
     isPresent= true;

     String[] copy= new String[list.length];
     for(int i= 0; i<list.length; i++)
       copy[i]= stripQuotes(list[i]);

     list= copy;
   }
   this.list= list;
}

   GenericListInfo(                 // Constructor
     String            name,        // The list name
     String            item)        // The list element
{
   if( name == null )
     name= "";
   this.name= name;

   this.list= new String[1];
   this.list[0]= "";
   if( item != null )
   {
     isPresent= true;
     this.list[0]= stripQuotes(item);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       GenericListInfo.genPanel
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
   panel= new DataPanel(list.length + 1);
   DataField[] df= panel.getField();

   df[0].setText(name);
   for(int i= 1; i<df.length; i++)
   {
     df[i].setText(list[i-1]);
     df[i].setEditable(listener);
   }

   if( tail != null && list.length > 0 )
   {
     tail.addFocusListener(new NextFocusAdapter(df[1]));
     tail= df[list.length];
   }

   return tail;
}

//----------------------------------------------------------------------------
//
// Method-
//       GenericListInfo.isValid
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
     for(int i= 1; i<df.length; i++)
     {
       if( isQuoted )
       {
         if( !v.isValidText(name, df[i]) )
           return false;
       }
       else
       {
         if( !v.isValidName(name, df[i]) )
           return false;
       }
     }
   }

   return true;
}

//----------------------------------------------------------------------------
//
// Method-
//       GenericListInfo.toString
//
// Purpose-
//       Extract the item into a String.
//
//----------------------------------------------------------------------------
public String                       // Resultant String
   toString( )                      // Convert to String
{
   String result= null;

   synchronized(this)
   {{{{
     if( isQuoted )
       result= arrayToQuotedString(list);
     else
       result= arrayToString(list);
   }}}} // synchronized(this)

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       GenericListInfo.update
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
     for(int i= 1; i<df.length; i++)
     {
       if( !df[i].getText().equals(list[i-1]) )
       {
         isChanged= true;
         list[i-1]= df[i].getText();
       }
     }
   }
}
} // class GenericListInfo
