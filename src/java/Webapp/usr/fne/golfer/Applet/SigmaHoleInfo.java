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
//       SigmaHoleInfo.java
//
// Purpose-
//       Sigma hole information container class.
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
//       SigmaHoleInfo
//
// Purpose-
//       Sigma hole information container class.
//
//----------------------------------------------------------------------------
public class SigmaHoleInfo  extends HoleInfo {
//----------------------------------------------------------------------------
//
// Method-
//       SigmaHoleInfo.SigmaHoleInfo
//
// Purpose-
//       Constructors.
//
//----------------------------------------------------------------------------
   SigmaHoleInfo(                   // Constructor
     int               format,      // The HolePanel format
     String            title,       // The ID string, iff editable
     String[]          array)       // The hole data array
{
   super(format, title, array);
}

   SigmaHoleInfo(                   // Constructor
     int               format,      // The HolePanel format
     String            title,       // The ID string, iff editable
     String            string)      // The hole String
{
   super(format, title, string);
}

   SigmaHoleInfo(                   // Copy constructor
     SigmaHoleInfo     source)      // Source SigmaHoleInfo
{
   super(source);
}

//----------------------------------------------------------------------------
//
// Method-
//       SigmaHoleInfo.genPanel
//
// Purpose-
//       Generate the internal PANEL, optional FocusListener and NextFocusAdapter
//
//----------------------------------------------------------------------------
public synchronized DataField       // Resultant tail DataField
   genPanel(                        // Generate new Panel
     FocusListener     listener,    // (OPTIONAL) FocusListener
     DataField         tail)        // (OPTIONAL) The current tail DataField
{
   // The panel is always generated
   panel= new SigmaHolePanel(format, title, array);
   DataField[] df= panel.getField();
   if( listener != null )
   {
     if( title != null )
     {
       df[HOLE_ID].addFocusListener(listener);
       df[HOLE_ID].setEditable(true);
     }

     for(int hole= 1; hole<=18; hole++)
     {
       df[hole].addFocusListener(listener);
       df[hole].setEditable(true);
     }
   }

   if( tail != null )
   {
     tail.addFocusListener(new NextFocusAdapter(title != null ? df[HOLE_ID] : df[1]));
     df[9].addFocusListener(new NextFocusAdapter(df[10]));
     tail= df[18];
   }

   return tail;
}
} // class SigmaHoleInfo
