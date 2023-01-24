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
//       DataPanel.java
//
// Purpose-
//       Define golf data panel.
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
//       DataPanel
//
// Purpose-
//       Golf data Panel.
//
//----------------------------------------------------------------------------
public class DataPanel extends JPanel {
//----------------------------------------------------------------------------
// DataPanel.Attributes
//----------------------------------------------------------------------------
DataField[]            field;       // The Field Array

//----------------------------------------------------------------------------
//
// Method-
//       DataPanel.DataPanel
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   DataPanel(                       // Constructor
     int               fields)      // The number of fields
{
   super();
   setLayout(new BoxLayout(this, BoxLayout.X_AXIS));
   setOpaque(true);

   field= new DataField[fields];
   for(int i= 0; i<fields; i++)
     field[i]= new DataField();
   setField(field);
}

public
   DataPanel( )                     // Constructor
{
   this(0);
}

//----------------------------------------------------------------------------
//
// Method-
//       DataPanel.UTILITIES
//
// Purpose-
//       Utility functions.
//
//----------------------------------------------------------------------------
public void
   debug( )                         // Debugging display
{
   System.out.println("DataPanel.debug()");
   for(int i= 0; i<field.length; i++)
     System.out.println("[" + i + "] cols(" + field[i].getColumns() + ") '" + field[i].getText() + "'");
}

//----------------------------------------------------------------------------
//
// Method-
//       DataPanel.getField()
//       DataPanel.getField(int)
//       DataPanel.getFieldText
//       DataPanel.getFieldValue
//
// Purpose-
//       Extract value.
//
//----------------------------------------------------------------------------
public DataField[]                  // The DataField[]
   getField( )                      // Get DataField[]
{
   return field;
}

public DataField                    // The DataField
   getField(                        // Get DataField
     int               index)       // For this index
{
   DataField           result;      // Resultant

   if( index >= field.length )
     result= null;
   else
     result= field[index];

   return result;
}

public String                       // The DataField text
   getFieldText(                    // Get DataField text
     int               index)       // For this index
{
   return getField(index).getText().trim();
}

public int                          // The DataField value
   getFieldValue(                   // Get DataField value
     int               index)       // For this index
{
   return getField(index).intValue();
}

//----------------------------------------------------------------------------
//
// Method-
//       DataPanel.setColor
//
// Purpose-
//       Update all field colors.
//
//----------------------------------------------------------------------------
public void
   setColor(                        // Update Field colors
     Color             bg,          // Background color
     Color             fg)          // Foreground color
{
   if( field != null )
   {
     for(int i= 0; i<field.length; i++)
     {
       field[i].setBackground(bg);
       field[i].setForeground(fg);
     }
   }
}

public void
   setColor(                        // Update Field colors
     Color[]           bgfg)        // [Background, foregound] colors
{
   setColor(bgfg[0], bgfg[1]);
}

//----------------------------------------------------------------------------
//
// Method-
//       DataPanel.setField
//
// Purpose-
//       Set the field array, adding each field to the panel.
//
//----------------------------------------------------------------------------
public void
   setField( )                      // Set the Field array
{
   removeAll();
   for(int i= 0; i<field.length; i++)
     add(field[i]);
}

public void
   setField(                        // Set the Field array
     DataField[]       array)       // To this
{
   field= array;

   setField();
}
} // class DataPanel
