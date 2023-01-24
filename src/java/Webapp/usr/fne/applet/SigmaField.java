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
//       SigmaField.java
//
// Purpose-
//       Define a field that contains the sum of other fields.
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
//       SigmaField
//
// Purpose-
//       Define a field that contains the sum of other fields.
//
//----------------------------------------------------------------------------
public class SigmaField extends DataField {
//----------------------------------------------------------------------------
// SigmaField.Attributes
//----------------------------------------------------------------------------
static int             IDENT= 0;    // Last serial number

int                    ident;       // Serial number
DataField[]            sigma;       // Source fields

//----------------------------------------------------------------------------
//
// Method-
//       SigmaField.SigmaField
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   SigmaField(                      // Constructor
     DataField[]       sigma)       // The source fields
{
   super();

   this.sigma= sigma;
   this.ident= ++IDENT;

   setEditable(false);
   setHorizontalAlignment(JTextField.CENTER);

   DataFieldListener dataFieldListener= new DataFieldListener(this);
   for(int i= 0; i<sigma.length; i++)
   {
     sigma[i].addActionListener(dataFieldListener);
     if( !(sigma[i] instanceof SigmaField) )
       sigma[i].setInputVerifier(getChangeInputVerifier());
   }

   change();
}

//----------------------------------------------------------------------------
//
// Method-
//       SigmaField.debug
//
// Purpose-
//       Debugging utilities.
//
//----------------------------------------------------------------------------
public void
   print(                           // Print debugging message
     String            message)     // The debugging message
{
   super.print("Sigma[" + ident + "] " + message);
}

public void
   debug( )                         // Print debugging message
{
   super.debug();
   StringBuffer buff= new StringBuffer();
   for(int i= 0; i<sigma.length; i++)
   {
     if( i != 0 )
       buff.append(" ");
     if( sigma[i] instanceof SigmaField )
     {
       SigmaField sf= (SigmaField)sigma[i];
       buff.append("[" + sf.ident + "]");
     }
     buff.append("'" + sigma[i].getText() + "'");
   }
   print(".sigma: " + buff.toString());
}

//----------------------------------------------------------------------------
//
// Method-
//       SigmaField.change
//
// Purpose-
//       Handle change event.
//
//----------------------------------------------------------------------------
public void
   change( )                        // Handle change event
{
// print("change()");

   boolean empty= false;
   boolean error= false;
   int total= 0;
   for(int i= 0; i<sigma.length; i++)
   {
     int value= 0;
     try {
       value= sigma[i].intValue();
     } catch( EmptyFieldException e) {
       empty= true;
     } catch( Exception e) {
       debug("ErrorField[" + i + "] " + e);
       error= true;
       break;
     }

     total += value;
   }

   if( error )
     setText(FIELD_ERR);
   else if( empty )
     setText("");
   else
     setText("" + total);

   fireActionPerformed();
}

//----------------------------------------------------------------------------
//
// Method-
//       SigmaField.toSigmaField
//
// Purpose-
//       Convert DataField to SigmaField
//
//----------------------------------------------------------------------------
public static SigmaField            // The SigmaField
   toSigmaField(                    // Convert DataField to SigmaField
     DataField[]       sigma,       // The source fields
     DataField         field)       // The field to convert
{
   SigmaField result= new SigmaField(sigma);
   result.load(field);
   return result;
}
} // class SigmaField
