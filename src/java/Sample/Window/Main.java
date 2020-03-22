//----------------------------------------------------------------------------
//
//       Copyright (C) 2014 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Main.java
//
// Purpose-
//       Sample window.
//
// Last change date-
//       2014/01/01
//
//----------------------------------------------------------------------------
import java.awt.*;
import java.awt.event.*;
import java.io.*;
import java.lang.*;
import java.util.*;
import user.util.*;

public class Main extends Debug implements ActionListener
{
//----------------------------------------------------------------------------
// Main.Constants for parameterization
//----------------------------------------------------------------------------
static final boolean   makeFrameVisible= true;  // Make Frame visible?
static final boolean   makeValid=        false; // Make Window valid?
static final boolean   useTextArea=      true;
static final boolean   useTextField=     true;

//----------------------------------------------------------------------------
// Main.Attributes
//----------------------------------------------------------------------------
boolean                ready;       // Status

GraphicsEnvironment    ge;          // GraphicsEnvironment
Font                   df;          // Default Font

Frame                  frame;       // Frame
Window                 window;      // Window
Button                 button;      // Button
PopupMenu              menu;        // Menu
MenuItem               menuItem;    // MenuItem
TextArea               textArea;    // TextArea
TextField              textField;   // TextField

//----------------------------------------------------------------------------
//
// Method-
//       Main.Main
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   Main(
     String            args[])      // Argument list
   throws Exception
{
   Font[]              fontArray;
   int                 i;

   ready= true;

   if( false )
   {
     ge= GraphicsEnvironment.getLocalGraphicsEnvironment();
     fontArray= ge.getAllFonts();
     df= null;
     for(i= 0; i<fontArray.length; i++)
     {
////// System.out.println(fontArray[i].getName());
       if( fontArray[i].getName().equals("Helvetica") )
         df= fontArray[i];
     }
   }
   else
     df= new Font("Helvetica", Font.PLAIN, 12);

   if( df == null )
   {
     System.out.println("No default font!");
     System.exit(1);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Main.close
//
// Purpose-
//       Close the window.
//
//----------------------------------------------------------------------------
public void
   close( )
   throws Exception
{
   System.out.println("Main.close started");

   frame.dispose();
   frame= null;

   System.out.println("Main.close completed");
}

//----------------------------------------------------------------------------
//
// Method-
//       Main.open
//
// Purpose-
//       Open the window.
//
// Usage notes-
//       Open the window.
//
//----------------------------------------------------------------------------
public void
   open( )
   throws Exception
{
   Insets              insets;
   Rectangle           rectangle;
   int                 winX, winY;

   System.out.println("Main.open started");

   // Create Frame
   System.out.println("...Frame");
   frame= new Frame("MainFrame");
   frame.setSize(900,600);
   frame.setBackground(Color.green);
   frame.setForeground(Color.yellow);
   frame.setFont(df);
   if( makeFrameVisible )           // Should be TRUE
   {
     if( true )                     // Should be TRUE
     {
       frame.setVisible(true);
       System.out.println("...Frame visible");
       rectangle= frame.getBounds();
       insets=    frame.getInsets();
       if( false )                  // Should be FALSE, even though
         insets.top -= 4;           // Frame top visible
     }
     else                           // Inset values will be zero
     {
       rectangle= frame.getBounds();
       insets=    frame.getInsets();
       frame.setVisible(true);
       System.out.println("...Frame visible");
     }
   }
   else                             // Fixed size window, frame unused
   {
     rectangle= new Rectangle(0, 0, 900, 600);
     insets=    new Insets(0, 0, 0, 0);
   }

   // Create Window
   System.out.println("...Window");
   winX= rectangle.width  - insets.left - insets.right;
   winY= rectangle.height - insets.top  - insets.bottom;
   if( true )                       // For debugging
   {
     System.out.println("Rectangle: " +
                        "Width("  + rectangle.width  + "), " +
                        "Height(" + rectangle.height + "), " +
                        "X("      + rectangle.x      + "), " +
                        "Y("      + rectangle.y      + ")");
     System.out.println("Insets: " +
                        "Left("   + insets.left   + "), " +
                        "Right("  + insets.right  + "), " +
                        "Top("    + insets.top    + "), " +
                        "Bottom(" + insets.bottom + ")");
     System.out.println("WinX(" + winX + "), winY(" + winY + ")");
   }
   window= new Window(frame);
   window.setSize(winX, winY);
   if( true )                       // Should be TRUE
     window.setLocation(insets.left, insets.top);
   else
     window.setLocation(0, 0);
   window.setBackground(Color.blue);
   window.setForeground(Color.white);
   window.setFont(df);
// window.toFront();

   // Create Button
   System.out.println("...Button");
   button= new Button("Button");
   button.setSize(90,60);
   button.setBackground(Color.red);
   button.setForeground(Color.black);
   button.addActionListener(this);
   button.setActionCommand("Command");

   // Create Menu
   if( true )                       // Has no effect
   {
     System.out.println("...Menu");
     menu= new PopupMenu("MyMenu");

     System.out.println("...MenuItem");
     menuItem= new MenuItem("MyMenuItem");
     menu.add(menuItem);
   }

   // Create TextField
   System.out.println("...TextField");
   textField= new TextField("TextField");
// window.add(textField);
// window.validate();
   textField.setBackground(Color.black);
   textField.setForeground(Color.red);
   textField.setLocation(0, 60);
   textField.setSize(120,30);
   textField.setFont(df);

   // Create TextArea
   System.out.println("...TextArea");
   textArea= new TextArea("TextArea", 1, 32);
// window.add(textArea);
// window.validate();
   textArea.setBackground(Color.black);
   textArea.setForeground(Color.red);
   textArea.setLocation(0, 90);
   textArea.setSize(120,90);
   textArea.setFont(df);

   // Control lower level visibility
   if( true )                       // MUST BE TRUE
   {
     window.setVisible(true);
     System.out.println("...Window visible");
     window.add(button);
     System.out.println("...Added Button");
     window.add(menu);
     System.out.println("...Added Menu");
     if( useTextArea )
     {
       window.add(textArea);
       System.out.println("...Added TextArea");
     }
     if( useTextField )
     {
       window.add(textField);
       System.out.println("...Added TextField");
     }
     if( false )                    // Not needed
     {
       button.setSize(90,60);
       textArea.setSize(90,15);
       textField.setSize(90,15);
     }
   }
   else                             // Button is sizeof(window)!
   {
     window.add(button);
     System.out.println("...Added Button");
     if( useTextArea )
     {
       window.add(textArea);
       System.out.println("...Added TextArea");
     }
     if( useTextField )
     {
       window.add(textField);
       System.out.println("...Added TextField");
     }
     window.setVisible(true);
     System.out.println("...Window visible");
     if( false )                    // Needed to retrim button size
     {
       button.setSize(90,60);
       textArea.setSize(90,15);
       textField.setSize(90,15);
     }
   }
   window.toFront();
   if( makeValid )
   {
     window.validate();
     System.out.println("...Window validated");
   }

   if( false )                      // HAS NO EFFECT (button visible anyway)
   {
     button.setVisible(true);
     System.out.println("...Button visible");
   }

   if( false )
   {
     textArea.setVisible(true);
     System.out.println("...TextArea visible");
   }

   if( false )
   {
     textField.setVisible(true);
     System.out.println("...TextField visible");
   }

   if( true )
   {
     System.out.println("Window:");
     System.out.println(window.toString());
     System.out.println("TextField:");
     System.out.println(textField.toString());
   }

   System.out.println("Main.open completed");
}

//----------------------------------------------------------------------------
//
// Method-
//       Main.actionPerformed
//
// Purpose-
//       Instantiate ActionListener.
//
//----------------------------------------------------------------------------
public void
   actionPerformed(                 // Handle an ActionEvent
     ActionEvent       event)       // The ActionEvent
{
   System.out.println("Main.actionPerformed");

   if( true  )
   {
     if( ready )
       textField.setText("Busy ");
     else
       textField.setText("Ready");

     ready= !ready;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Main.run
//
// Purpose-
//       Code driver
//
//----------------------------------------------------------------------------
public void
   run( )
   throws Exception
{
   System.out.println("Sleep...");

   Thread.sleep(60000);

   System.out.println("...Sleep");
}

//----------------------------------------------------------------------------
//
// Method-
//       Main.main
//
// Purpose-
//       Mainline code.
//
//----------------------------------------------------------------------------
public static void
   main(                            // Mainline code
     String[]          args)        // Argument array
   throws Exception
{
   Main                main= new Main(args);

   System.out.println("Main.main started");
   main.open();
   main.run();
   main.close();

   System.out.println("Main.main complete");
   System.exit(0);
}
} // Class Main

