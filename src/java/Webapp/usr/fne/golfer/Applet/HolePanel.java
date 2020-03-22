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
//       HolePanel.java
//
// Purpose-
//       The minimal golf hole panel.
//
// Last change date-
//       2020/01/15
//
//----------------------------------------------------------------------------
import java.awt.*;
import java.lang.*;
import java.util.*;
import javax.swing.*;

//----------------------------------------------------------------------------
//
// Class-
//       HolePanel
//
// Purpose-
//       Basic golf hole Panel.
//
//----------------------------------------------------------------------------
public class HolePanel extends DataPanel {
//----------------------------------------------------------------------------
// HolePanel.Attributes
//----------------------------------------------------------------------------
// Hole panel field index
static final int       HOLE_ID=   0;// Hole title
static final int       HOLE_OUT= 19;// OUT index
static final int       HOLE_IN=  20;// IN  index
static final int       HOLE_TOT= 21;// TOT index

// Optional fields
static final int       HOLE_ESC= 22;// ESC index
static final int       HOLE_HCP= 23;// HCP index
static final int       HOLE_NET= 24;// NET index

static final int       HOLE_LDO= 25;// LDO index (Longest Drive OUT)
static final int       HOLE_CPO= 26;// CPO index (Closest to Pin OUT)
static final int       HOLE_LDI= 27;// LDI index (Longest Drive IN)
static final int       HOLE_CPI= 28;// CPI index (Closest to Pin IN)
static final int       HOLE_FWH= 29;// CPI index (Closest to Pin IN)
static final int       HOLE_GIR= 30;// CPI index (Closest to Pin IN)

static final int       HOLE_SKIN=31;// SKINS index

// Field constants
static final String    FIELD_DP=  DataField.FIELD_DP; // Double par data entry
static final String    FIELD_ERR= DataField.FIELD_ERR; // Field error

// Format constants
static final int       FORMAT_USER= HOLE_TOT  + 1;
static final int       FORMAT_BASE= HOLE_NET  + 1;
static final int       FORMAT_EVNT= HOLE_GIR  + 1;
static final int       FORMAT_SKIN= HOLE_SKIN + 1;

//----------------------------------------------------------------------------
//
// Method-
//       HolePanel.HolePanel
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   HolePanel(                       // Constructor
     int               format,      // The number of fields
     String            title,       // Panel title
     String[]          array)       // Panel hole value array
{
   super(format);

   if( format <= HOLE_TOT )
     throw new RuntimeException("HolePanel(format(" + format + "))");

   if( title != null )
     field[HOLE_ID].setText(title);

   if( array != null )
   {
     for(int hole= 1; hole<=18; hole++)
       field[hole].setText(array[hole-1]);
   }

   for(int i= 0; i<field.length; i++)
     field[i].setColumns(getColumnSize(i));
}

public
   HolePanel(                       // Constructor
     int               format)      // The number of fields
{
   this(format, null, null);
}

public
   HolePanel( )                     // Constructor
{
   this(HOLE_TOT+1);
}

//----------------------------------------------------------------------------
//
// Method-
//       HolePanel.fieldColor
//
// Purpose-
//       Return color based on data value.
//
//----------------------------------------------------------------------------
public static Color                 // Resultant Color
   fieldColor(                      // Get Color for
     int               value)       // This value
{
   Color result= Color.BLACK;
   if( value < 0 )
     result= Color.RED;
   else if( value == 0 )
     result= Color.GREEN.darker();

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       HolePanel.getColumnSize
//
// Purpose-
//       Return the column size for a field.
//
//----------------------------------------------------------------------------
public static int                   // The column size
   getColumnSize(                   // Get column size
     int               field)       // For this field
{
   int result= 4;
   switch( field )
   {
     case HOLE_ID:
       result= 8;
       break;

     case 1:
     case 2:
     case 3:
     case 4:
     case 5:
     case 6:
     case 7:
     case 8:
     case 9:
     case 10:
     case 11:
     case 12:
     case 13:
     case 14:
     case 15:
     case 16:
     case 17:
     case 18:
       result= 3;
       break;

     default:
       break;
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       HolePanel.getPanelSize
//
// Purpose-
//       Return the number of columns in a panel.
//
//----------------------------------------------------------------------------
public static int                   // The column count
   getPanelSize(                    // Get column count
     int               fieldCount)  // The number of fields
{
   int result= 0;

   if( fieldCount > 0 )
   {
     result += 8;
     fieldCount--;
   }

   if( fieldCount <= 18 )
     result += (3 * fieldCount);
   else
   {
     result += (3 * 18);
     fieldCount -= 18;
     result += (4 * fieldCount);
   }

   return result;
}

public int                          // The column count
   getPanelSize( )                  // Get column count
{
   return getPanelSize(field.length);
}

//----------------------------------------------------------------------------
//
// Method-
//       HolePanel.setField
//
// Purpose-
//       Set the field array, adding each field to the panel.
//
//----------------------------------------------------------------------------
public void
   setField( )                      // Set the Field array
{
   removeAll();

   add(field[HOLE_ID]);

   for(int hole= 1; hole<=9; hole++)
     add(field[hole]);

   if( field.length > HOLE_OUT )
     add(field[HOLE_OUT]);

   for(int hole= 10; hole<=18; hole++)
     add(field[hole]);

   for(int i= HOLE_IN; i<field.length; i++)
     add(field[i]);
}

//----------------------------------------------------------------------------
//
// Method-
//       HolePanel.toString
//
// Purpose-
//       Convert to (card) String.
//
//----------------------------------------------------------------------------
public String                       // The String
   toString( )                      // Convert to (card) String
{
   StringBuffer        buff;        // Resultant

   buff= new StringBuffer();
   for(int hole= 1; hole<=18; hole++)
   {
     if( hole != 1 )
       buff.append(" ");

     buff.append(field[hole].getText());
   }

   return buff.toString();
}
} // class HolePanel
