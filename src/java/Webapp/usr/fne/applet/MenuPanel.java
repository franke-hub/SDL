//----------------------------------------------------------------------------
//
//       Copyright (C) 2023 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       MenuPanel.java
//
// Purpose-
//       Define golf Menu panel.
//
// Last change date-
//       2023/01/24
//
//----------------------------------------------------------------------------
import java.awt.*;
import javax.swing.*;

//----------------------------------------------------------------------------
//
// Class-
//       MenuPanel
//
// Purpose-
//       Golf Menu Panel.
//
//----------------------------------------------------------------------------
public class MenuPanel extends JPanel {
//----------------------------------------------------------------------------
// MenuPanel.Attributes
//----------------------------------------------------------------------------
JMenu                  array[];     // The JMenu Array
JMenuBar               panel;       // The JMenuBar panel

//----------------------------------------------------------------------------
//
// Method-
//       MenuPanel.MenuPanel
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   MenuPanel(                       // Constructor
     int               count)       // The number of menus
{
   super();
   setLayout(new FlowLayout());
   setOpaque(true);

   array= new JMenu[count];
   for(int i= 0; i<count; i++)
     array[i]= new JMenu();
   setArray(array);
}

public
   MenuPanel( )                     // Constructor
{
   this(1);
}

//----------------------------------------------------------------------------
//
// Method-
//       MenuPanel.UTILITIES
//
// Purpose-
//       Utility functions.
//
//----------------------------------------------------------------------------
public void
   debug( )                         // Debugging display
{
   System.out.println("MenuPanel.debug()");
   for(int i= 0; i<array.length; i++)
     System.out.println("[" + i + "] '" + array[i].getText() + "'");
}

//----------------------------------------------------------------------------
//
// Method-
//       MenuPanel.getArray()
//       MenuPanel.getArray(int)
//       MenuPanel.getArrayText
//       MenuPanel.getArrayValue
//
// Purpose-
//       Extract value.
//
//----------------------------------------------------------------------------
public JMenu[]                      // The JMenu[]
   getArray( )                      // Get JMenu[]
{
   return array;
}

public JMenu                        // The JMenu
   getArray(                        // Get JMenu
     int               index)       // For this index
{
   JMenu               result= null; // Resultant

   if( index < array.length )
     result= array[index];

   return result;
}

public String                       // The JMenu text
   getArrayText(                    // Get JMenu text
     int               index)       // For this index
{
   return getArray(index).getText().trim();
}

public JMenuBar                     // The JMenuBar
   getMenuBar( )                    // Get JMenuBar
{  return panel; }

//----------------------------------------------------------------------------
//
// Method-
//       MenuPanel.setColor
//
// Purpose-
//       Update all array colors.
//
//----------------------------------------------------------------------------
public void
   setColor(                        // Update array colors
     Color             bg,          // Background color
     Color             fg)          // Foreground color
{
   if( array != null )
   {
     for(int i= 0; i<array.length; i++)
     {
       array[i].setBackground(bg);
       array[i].setForeground(fg);
     }
   }
}

public void
   setColor(                        // Update array colors
     Color[]           bgfg)        // [Background, foregound] colors
{
   setColor(bgfg[0], bgfg[1]);
}

//----------------------------------------------------------------------------
//
// Method-
//       MenuPanel.setArray
//
// Purpose-
//       Set the JMenu array, adding each JMenu to the panel.
//
//----------------------------------------------------------------------------
public void
   setArray( )                      // Set the JMenu array
{
   panel= new JMenuBar();

   removeAll();
   for(int i= 0; i<array.length; i++)
   {
     array[i].setBorder(BorderFactory.createLineBorder(Color.BLACK, 2));
     panel.add(array[i]);
   }
   add(panel);
}

public void
   setArray(                        // Set the JMenu array
     JMenu             jmenu[])     // To this
{
   array= jmenu;
   setArray();
}
} // class MenuPanel
