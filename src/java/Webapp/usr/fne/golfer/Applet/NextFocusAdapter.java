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
//       NextFocusAdapter.java
//
// Purpose-
//       Set the next Component to get focus.
//
// Last change date-
//       2020/01/15
//
//----------------------------------------------------------------------------
import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.lang.*;
import java.util.*;

//----------------------------------------------------------------------------
//
// Class-
//       NextFocusAdapter
//
// Purpose-
//       Set the next Component to get focus.
//
//----------------------------------------------------------------------------
public class NextFocusAdapter extends FocusAdapter {
//----------------------------------------------------------------------------
// NextFocusAdapter.Attributes
//----------------------------------------------------------------------------
String                 name;        // The Adapter name
JComponent             next;        // The next focus Component

//----------------------------------------------------------------------------
//
// Method-
//       NextFocusAdapter.UTILITIES
//
// Purpose-
//       Utility functions.
//
//----------------------------------------------------------------------------
protected void debug(String string) {
   if( name != null )
     System.out.println(toString() + ": " + string);
}

//----------------------------------------------------------------------------
//
// Method-
//       NextFocusAdapter.NextFocusAdapter
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   NextFocusAdapter(                // Constructor
     JComponent        next)        // The next Component to get Focus
{
   super();

   this.next= next;
   this.name= null;
}

//----------------------------------------------------------------------------
//
// Method-
//       NextFocusAdapter.focusGained
//
// Purpose-
//       Handle GAINED FocusEvent (NOP).
//
//----------------------------------------------------------------------------
/***
public void
   focusGained(                     // Handle FocusEvent
     FocusEvent        e)           // GAINED FOCUS
{
   debug("focusGained(" + e.isTemporary() + ")");
}
***/

//----------------------------------------------------------------------------
//
// Method-
//       NextFocusAdapter.focusLost
//
// Purpose-
//       Handle LOST FocusEvent.
//
//----------------------------------------------------------------------------
public void
   focusLost(                       // Handle FocusEvent
     FocusEvent        e)           // LOST FOCUS
{
   Component component= e.getOppositeComponent();

// debug("focusLost(" + e.isTemporary() + "," + (component == null) + ")");

   // Ignore temporary or alternate window events
   if( e.isTemporary() || component == null )
     return;

   // Ignore when the opposite Component has a NextFocusAdapter
   // (This can occur when focus is shifed manually.)
   FocusListener[] listeners= component.getFocusListeners();
   for(int i= 0; i<listeners.length; i++)
   {
     if( listeners[i] instanceof NextFocusAdapter )
       return;
   }

   // Pass focus to the next Component
   next.requestFocusInWindow();
}

//----------------------------------------------------------------------------
//
// Method-
//       NextFocusAdapter.toString
//
// Purpose-
//       Convert to String.
//
//----------------------------------------------------------------------------
public String                       // String representation
   toString( )                      // Convert to String
{
   return "NextFocusAdapter(" + ((name != null) ? name : "null")  + ")";
}
} // class NextFocusAdapter
