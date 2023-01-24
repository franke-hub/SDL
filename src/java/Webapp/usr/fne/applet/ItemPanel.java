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
//       ItemPanel.java
//
// Purpose-
//       Define golf item panel, an initialized DataPanel.
//
// Last change date-
//       2020/01/15
//
//----------------------------------------------------------------------------
import java.awt.Color;
import java.awt.Component;
import javax.swing.BoxLayout;
import javax.swing.JPanel;

//----------------------------------------------------------------------------
//
// Class-
//       ItemPanel
//
// Purpose-
//       Golf item Panel.
//
//----------------------------------------------------------------------------
public class ItemPanel extends DataPanel {
//----------------------------------------------------------------------------
//
// Method-
//       ItemPanel.ItemPanel
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   ItemPanel(                       // Constructor
     String[]          array)       // The data field array
{
   super(array.length);
   for(int i= 0; i<array.length; i++)
   {
     field[i].setText(array[i]);
     field[i].setColumns(100/array.length);
   }
}

public
   ItemPanel(                       // Constructor
     String            text)        // The data field
{
   super(1);
   field[0].setText(text);
   field[0].setColumns(100);
}

public
   ItemPanel( )                     // Constructor
{
   super(0);
}
} // class ItemPanel
