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
//       PostPanel.java
//
// Purpose-
//       Define golf data panel.
//
// Last change date-
//       2020/01/15
//
//----------------------------------------------------------------------------
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;

//----------------------------------------------------------------------------
//
// Class-
//       PostPanel
//
// Purpose-
//       Golf data Panel.
//
//----------------------------------------------------------------------------
public class PostPanel extends DataPanel {
boolean                operational; // The OPERATIONAL control
PostButton             postButton;  // The POST Button
PostListener           postListener;// The POST Listener
JTextField             postMessage; // The POST Message
Validator              validator;   // The POST Button Validator

//----------------------------------------------------------------------------
//
// Method-
//       PostPanel.PostPanel
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   PostPanel(                       // Constructor
     Validator         validator)   // The POST Button Validator
{
   super(0);

   this.operational= false;
   this.postButton= new PostButton();
   this.postListener= new PostListener();
   this.postMessage= new DataField(100);
   this.postMessage.setForeground(Color.RED);
   this.validator= validator;

   add(new DataField(0));           // Prevents tab into PostButton
   add(postButton);
   add(postMessage);
}

//----------------------------------------------------------------------------
//
// Method-
//       PostPanel.UTILITIES
//
// Purpose-
//       Utility functions.
//
//----------------------------------------------------------------------------
public void
   debug( )                         // Debugging display
{
   System.out.println("PostPanel.debug()");
   System.out.println("isOperational: " + operational);
   System.out.println("message: '" + postMessage.getText() + "'");
}

//----------------------------------------------------------------------------
//
// Method-
//       PostPanel.getListener()
//       PostPanel.setMessage(String)
//       PostPanel.setOperational(boolean)
//
// Purpose-
//       Accessors.
//
//----------------------------------------------------------------------------
public FocusListener                // The PostListener
   getListener( )                   // Get PostListener
{
   return postListener;
}

public void
   setMessage(                      // Set PostMessage text
     String            string)      // To this String
{
   postMessage.setText(string);
}

public void
   setMessage( )                    // Set PostMessage text
{
   postMessage.setText("");
}

public void
   setOperational(                  // Set OPERATIONAL control
     boolean           operational) // To this
{
   this.operational= operational;
   postMessage.setText("");

   if( operational )
     postButton.validityCheck();
}

//----------------------------------------------------------------------------
//
// Class-
//       PostPanel.PostButton
//
// Purpose-
//       The POST Button.
//
//----------------------------------------------------------------------------
protected class PostButton extends JButton implements FocusListener {
   PostButton( )                    // The POST Button
{
   super("POST");

   setEnabled(false);
   addFocusListener(this);
}

public void
   focusGained(                     // Implement FocusListener
     FocusEvent        e)           // The FocusEvent
{
   if( validator.isValid() )        // If directly from change
   {
     setText("DONE");
     setEnabled(false);
     validator.actionPerformed();
   }
}

public void
   focusLost(                       // Implement FocusListener
     FocusEvent        e)           // The FocusEvent
{
}

public void
   validityCheck( )                 // Check Validator status
{
   setEnabled(false);
   if( operational && validator.isValid() )
   {
     setText("POST");
     setEnabled(true);
   }
}
} // class PostButton

//----------------------------------------------------------------------------
//
// Class-
//       PostPanel.PostListener
//
// Purpose-
//       PostButton FocusAdapter.
//
//----------------------------------------------------------------------------
protected class PostListener extends FocusAdapter {
public void
   focusLost(                       // Implement FocusListener
     FocusEvent        e)           // The FocusEvent
{
   postButton.validityCheck();
}
} // class PostListener
} // class PostPanel
