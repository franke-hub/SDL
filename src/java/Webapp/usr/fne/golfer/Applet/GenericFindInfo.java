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
//       GenericFindInfo.java
//
// Purpose-
//       Generic xxxxxx_ID information container class.
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
//       GenericFindInfo
//
// Purpose-
//       Generic xxxxxx_ID information container class.
//
//----------------------------------------------------------------------------
public class GenericFindInfo  extends DatabaseInfo {
String                 find;        // The xxxxxx_ID String

//----------------------------------------------------------------------------
//
// Method-
//       GenericFindInfo.debug
//
// Purpose-
//       Extend DebuggingAdaptor
//
//----------------------------------------------------------------------------
public void
   debug( )                         // Debugging display
{
   super.debug();
   print(".find: " + find);
}

//----------------------------------------------------------------------------
//
// Method-
//       GenericFindInfo.GenericFindInfo
//
// Purpose-
//       Constructors.
//
//----------------------------------------------------------------------------
   GenericFindInfo(                 // Constructor
     String            string)      // The xxxxxx_ID String
{
   find= string;
   isPresent= true;
}

//----------------------------------------------------------------------------
//
// Method-
//       GenericFindInfo.genPanel
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
   df[0].setText(find);

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
//       GenericFindInfo.isValid
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
   // A xxxxxx_ID panel is validated whether or not is is removed
   if( panel != null )
   {
     DataField[] df= panel.getField();
     if( !v.isValidText("IDENT", df[0]) )
       return false;
   }

   return true;
}

//----------------------------------------------------------------------------
//
// Method-
//       GenericFindInfo.toString
//
// Purpose-
//       Extract find into a String.
//
//----------------------------------------------------------------------------
public String                       // Resultant String
   toString( )                      // Convert to String
{
   return find;
}

//----------------------------------------------------------------------------
//
// Method-
//       GenericFindInfo.update
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
     if( !find.equals(df[0].getText()) )
     {
       isChanged= true;
       find= df[0].getText();
     }
   }
}
} // class GenericFindInfo
