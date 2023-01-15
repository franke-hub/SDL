//----------------------------------------------------------------------------
//
//       Copyright (C) 2008-2023 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       DataField.java
//
// Purpose-
//       Define golf data Field.
//
// Last change date-
//       2023/01/15
//
//----------------------------------------------------------------------------
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;

import java.lang.*;
import java.util.*;

//----------------------------------------------------------------------------
//
// Class-
//       DataField
//
// Purpose-
//       Golf data Field.
//
//----------------------------------------------------------------------------
public class DataField extends JTextField implements DebuggingInterface {
//----------------------------------------------------------------------------
// DataField.Attributes
//----------------------------------------------------------------------------
static boolean         HCDM= false; // Hard Core Debug Mode

NamedValue[]           namedValue;  // Name/value array

//----------------------------------------------------------------------------
// Static fields, used to initialized namedValue array
protected static NamedValue[]
                       longArray;   // {"-", 0}
protected static NamedValue[]
                       nearArray;   // {"-", 300}
protected static NamedValue[][]
                       par2Array;   // [par] {FIELD_DP, 2*par}

//----------------------------------------------------------------------------
// Constants
static final String    FIELD_DP=  "DP"; // Double par data entry
static final String    FIELD_ERR= "!ERR"; // Field error
static final int       CLOSE_MAX= 100;  // Maximum closest to pin value

//----------------------------------------------------------------------------
//
// Method-
//       DataField.DataField
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   DataField(                       // Constructor
     int               cols,        // The number of columns
     Color             bg,          // Background color
     Color             fg)          // Foreground color
{
   super();

   setEditable(false);
   setBackground(bg);
   setForeground(fg);
   setHorizontalAlignment(JTextField.CENTER);
   if( cols > 0 )
     setColumns(cols);
}

public
   DataField(                       // Constructor
     int               cols)        // The number of columns
{
   this(cols, Color.WHITE, Color.BLACK);
}

public
   DataField( )                     // Default constructor
{
   this(0, Color.WHITE, Color.BLACK);
}

public
   DataField(                       // Copy constructor
     DataField         copy)        // Source DataField
{
   load(copy);
}

//----------------------------------------------------------------------------
//
// Method-
//       DataField.DebuggingInterface
//
// Purpose-
//       Implement DebuggingInterface methods.
//
//----------------------------------------------------------------------------
public void
   debug( )                         // Object debugging message
{
   print("DataField.debug()");
   print(".namedValue: " + debugNamedValue(namedValue));
   print(".columns: " + getColumns());
   print(".text: " + getText());
   print(".editable: " + isEditable());
   print(".horizontalAlignment: " + getHorizontalAlignment());

   if( HCDM )
   {
     print(".longArray: " + debugNamedValue(longArray));
     print(".nearArray: " + debugNamedValue(nearArray));
     for(int i= 0; i<par2Array.length; i++)
       print(".par2Array[" + i + "]: " + debugNamedValue(par2Array[i]));
   }
}

public boolean                      // TRUE iff debug should write
   isDebug( )                       // Is debugging active?
{
   return false;
}

public void
   print(                           // Debugging message
     String            string)      // The message String
{
   System.out.println(string);
}

public void
   debug(                           // Debugging message
     String            string)      // The message String
{
   if( isDebug() )
     print(string);
}

public void
   error(                           // Debugging error message
     String            string)      // The message String
{
   System.err.println(string);
}

public String
   debugNamedValue(
     NamedValue[]      namedValue)
{
   StringBuffer buff= new StringBuffer();

   if( namedValue == null )
     buff.append("NULL");
   else
   {
     for(int i= 0; i<namedValue.length; i++)
     {
       buff.append("\n");
       buff.append("[" + i + "] "
                 + "name(" + namedValue[i].name + ") "
                 + "value(" + namedValue[i].value + ")");
     }
   }

   return buff.toString();
}

//----------------------------------------------------------------------------
//
// Method-
//       DataField.hcdmDebug
//
// Purpose-
//       DataField Hard Core Debug Mode
//
//----------------------------------------------------------------------------
public void
   hcdmDebug(                       // Debug a DataField
     String            info)        // Field descriptor
{
   debug("debugField(" + info + ")");
   debug("       text: " + getText());
   debug(" backGround: " + getBackground());
   debug(" foreGround: " + getForeground());
   debug("    columns: " + getColumns());
   debug("       size: " + getSize());
   debug(" isDispable: " + isDisplayable());
   debug("isEditable:: " + isEditable());
   debug("  isEnabled: " + isEnabled());
   debug("   isOpaque: " + isOpaque());
   debug("  isShowing: " + isShowing());
   debug("    isValid: " + isValid());
   debug("  isVisible: " + isVisible());
}

//----------------------------------------------------------------------------
//
// Method-
//       DataField.change
//
// Purpose-
//       Handle change event.
//
//----------------------------------------------------------------------------
public void
   change( )                        // Handle change event
{
   fireActionPerformed();
}

//----------------------------------------------------------------------------
//
// Method-
//       DataField.getChangeInputVerifier
//
// Purpose-
//       Get the DataField InputVerifier.
//
//----------------------------------------------------------------------------
public InputVerifier                // The change() InputVerifier
   getChangeInputVerifier( )        // Get change() InputVerifier
{
   InputVerifier result= new InputVerifier()
   {
     public boolean
        verify(
          JComponent        comp)
     {
        boolean result= true;

        if( comp instanceof DataField )
        {
          DataField field= (DataField)comp;
          try {
            field.intValue();
          } catch(Exception e) {
            result= false;
          }
        }

        return result;
     }

     public boolean
        shouldYieldFocus(
          JComponent        out,
          JComponent        inp)
     {
        boolean valid= super.shouldYieldFocus(out, inp);
        if( !valid )
          getToolkit().beep();
        else
          change();

        return valid;
     }
   }; // new InputVerifier()

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       DataField.initStatic
//
// Purpose-
//       Initialize the constant NamedValue arrays
//
//----------------------------------------------------------------------------
private void
   initStatic( )                    // Initialize constant NamedValue arrays
{
   synchronized(new DataField().getClass())
   {{{{
     if( longArray == null )
     {
       longArray= new NamedValue[1];
       longArray[0]= new NamedValue("-", 0);
     }

     if( nearArray == null )
     {
       nearArray= new NamedValue[1];
       nearArray[0]= new NamedValue("-", CLOSE_MAX);
     }

     if( par2Array == null )
     {
       par2Array= new NamedValue[10][];
       for(int par= 0; par<par2Array.length; par++)
       {
         par2Array[par]= new NamedValue[1];
         par2Array[par][0]= new NamedValue(FIELD_DP, 2*par);
       }
     }
   }}}}
}

//----------------------------------------------------------------------------
//
// Method-
//       DataField.intValue
//
// Purpose-
//       Extract integer value.
//
//----------------------------------------------------------------------------
public int                          // The integer value
   intValue( )                      // Get integer value
{
   String text= getText().trim();
   setText(text);

   if( namedValue != null )
   {
     for(int i= 0; i<namedValue.length; i++)
     {
       String string= namedValue[i].name;
       if( string != null
           && text.equalsIgnoreCase(string) )
       {
         setText(string);
         return namedValue[i].value;
       }
     }

     if( text.length() == 0 && namedValue.length > 0 )
       return namedValue[0].value;
   }

   if( text.length() == 0 )
     throw new EmptyFieldException();
   if( text.charAt(0) == '+' )
     text= text.substring(1);
   return Integer.parseInt(text);
}

//----------------------------------------------------------------------------
//
// Method-
//       DataField.load
//
// Purpose-
//       Load DataField (Copy pseudo-constructor)
//
//----------------------------------------------------------------------------
public DataField                    // Constant THIS
   load(                            // Load DataField
     DataField         field)       // The field to copy
{
   setText(field.getText());
   setBackground(field.getBackground());
   setForeground(field.getForeground());
   if( field.getColumns() > 0 || getColumns() > 0 )
     setColumns(field.getColumns());
   setEditable(field.isEditable());
   setHorizontalAlignment(field.getHorizontalAlignment());

   return this;
}

//----------------------------------------------------------------------------
//
// Method-
//       DataField.setEditable
//
// Purpose-
//       setEditable(true) and addFocusListener() iff listener specified
//
//----------------------------------------------------------------------------
public void
   setEditable(                     // Make the DataField editable
     FocusListener     listener)    // And add this FocusListener
{
   if( listener != null )
   {
     setEditable(true);
     addFocusListener(listener);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       DataField.setNamedValue
//
// Purpose-
//       Set the NamedValue array.
//
//----------------------------------------------------------------------------
public DataField                    // Constant THIS
   setNamedValue(                   // Set NamedValue array
     NamedValue[]      array)       // The NamedValue array
{
   namedValue= array;
   return this;
}

public DataField                    // Constant THIS
   setNamedValueLONG( )             // Set NamedValue LONG
{
   initStatic();
   return setNamedValue(longArray);
}

public DataField                    // Constant THIS
   setNamedValueNEAR( )             // Set NamedValue NEAR
{
   initStatic();
   return setNamedValue(nearArray);
}

public DataField                    // Constant THIS
   setNamedValuePAR2(               // Set NamedValue 2*PAR
     int               par)         // Par value
{
   initStatic();
   return setNamedValue(par2Array[par]);
}

//----------------------------------------------------------------------------
//
// Class-
//       DataField.NamedValue
//
// Purpose-
//       Name/Value correlation.
//
//----------------------------------------------------------------------------
class NamedValue {                  // Name/Value correlation
   String              name;        // The name
   int                 value;       // The value

   NamedValue(                      // Constructor
     String            name,        // The name
     int               value)       // The value
{
   this.name= name;
   this.value= value;
}
} // class NamedValue

//----------------------------------------------------------------------------
//
// Class-
//       DataFieldListener
//
// Purpose-
//       Listen for and drive field change events.
//
//----------------------------------------------------------------------------
public class DataFieldListener implements ActionListener {
DataField              owner;       // The associated DataField

   DataFieldListener(               // Constructor
     DataField         owner)       // The owner
{
   this.owner= owner;
}

public void
   actionPerformed(                 // Implement ActionListener
     ActionEvent       e)           // The ActionEvent
{
   owner.change();
}
} // class DataFieldListener

//----------------------------------------------------------------------------
//
// Class-
//       DataField.UNUSED
//
// Purpose-
//       Container for unused debugging classes
//
//----------------------------------------------------------------------------
private class UNUSED extends DataField {
public void
   addFocusListener(                // Add FocusListener
     FocusListener     listener)    // The FocusListener
{
   if( HCDM )
   {
     if( listener instanceof NextFocusAdapter )
       debugTail((DataField)((NextFocusAdapter)listener).next);
     else
     {
////// debug("addFocusListener(" + listener.getClass().getName() + ")");
     }
   }
   super.addFocusListener(listener);
}

public void
   debugTail(                       // Debugging display for tail fields
     DataField         next)        // New tail
{
   StringBuffer buff= new StringBuffer();

   buff.append("TAIL(" + getText() + ")");
   if( next != null )
     buff.append("=>NEXT(" + next.getText() + ")");

   debug(buff.toString());
}

public void
   debugTail( )
{
   debugTail(null);
}
} // class UNUSED
} // class DataField
