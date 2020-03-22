//----------------------------------------------------------------------------
//
//       Copyright (C) 2010 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       LayoutDemo.java
//
// Purpose-
//       Examine the various LayoutManager implementations.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.event.*;

public class LayoutDemo {
//----------------------------------------------------------------------------
//
// Method-
//       LayoutDemo.createAndShowGUI
//
// Purpose-
//       Create the GUI and show it.
//
// Implementation notes-
//       This must be invoked from the event-dispatching thread.
//       This must be an object method.
//
//----------------------------------------------------------------------------
private void
   createAndShowGUI( )
{
   //-------------------------------------------------------------------------
   // Create and set up the BorderLayoutPanel window.
   JFrame frame= new JFrame("BorderLayoutPanel");
   frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);

   // Create and set up the content pane.
   JComponent content= new BorderLayoutPanel();
   content.setOpaque(true);         // Content panes must be opaque
   frame.setContentPane(content);

   // Display the window.
   frame.pack();
   frame.setVisible(true);

   //-------------------------------------------------------------------------
   // Create and set up the GridLayoutPanel window.
   frame= new JFrame("GridLayoutPanel");
   frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);

   // Create and set up the content pane.
   content= new GridLayoutPanel();
   content.setOpaque(true);         // Content panes must be opaque
   frame.setContentPane(content);

   // Display the window.
   frame.pack();
   frame.setVisible(true);

   //-------------------------------------------------------------------------
   // Create and set up the GridBagLayoutPanel window.
   frame= new JFrame("GridBagLayoutPanel");
   frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);

   // Create and set up the content pane.
   content= new GridBagLayoutPanel();
   content.setOpaque(true);         // Content panes must be opaque
   frame.setContentPane(content);

   // Display the window.
   frame.pack();
   frame.setSize(frame.getPreferredSize());
   frame.setVisible(true);

   //-------------------------------------------------------------------------
   // Create and set up the TestBagLayoutPanel window.
   frame= new JFrame("TestBagLayoutPanel");
   frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);

   // Create and set up the content pane.
   content= new TestBagLayoutPanel();
   content.setOpaque(true);         // Content panes must be opaque
   frame.setContentPane(content);

   // Display the window.
   frame.pack();
   frame.setSize(frame.getPreferredSize());
   frame.setVisible(true);
}

//----------------------------------------------------------------------------
//
// Method-
//       LayoutDemo.createButton
//
// Purpose-
//       Create a simple Button object
//
//----------------------------------------------------------------------------
public static Container             // Resultant
   createButton(String ident)       // Create a simple Button
{
   return new JButton(ident);
}

//----------------------------------------------------------------------------
//
// Method-
//       LayoutDemo.createHoleString
//
// Purpose-
//       Create a hole string array.
//
//----------------------------------------------------------------------------
public static String[]              // Resultant
   createHoleString( )              // Create a hole string array
{
   String[] hole= new String[19];
   hole[ 0]= "Item name";
   hole[ 1]= "01";
   hole[ 2]= "02";
   hole[ 3]= "03";
   hole[ 4]= "04";
   hole[ 5]= "05";
   hole[ 6]= "06";
   hole[ 7]= "07";
   hole[ 8]= "08";
   hole[ 9]= "09";
   hole[10]= "10";
   hole[11]= "11";
   hole[12]= "12";
   hole[13]= "13";
   hole[14]= "14";
   hole[15]= "15";
   hole[16]= "16";
   hole[17]= "17";
   hole[18]= "18";

   return hole;
}

//----------------------------------------------------------------------------
//
// Method-
//       LayoutDemo.createHorizontal
//
// Purpose-
//       Create a horizontal panel object
//
//----------------------------------------------------------------------------
public static Container             // Resultant
   createHorizontal( )              // Create a Horizontal group
{
   // Create a panel that uses BoxLayout.
   JPanel panel= new JPanel();
   panel.setLayout(new BoxLayout(panel, BoxLayout.LINE_AXIS));
   panel.add(new JButton("Left"));
   panel.add(Box.createHorizontalStrut(2));
   panel.add(new JSeparator(SwingConstants.VERTICAL));
   panel.add(Box.createHorizontalStrut(2));
   panel.add(new JTextField("Middle", 10));
   panel.add(Box.createHorizontalStrut(2));
   panel.add(new JSeparator(SwingConstants.VERTICAL));
   panel.add(Box.createHorizontalStrut(2));
   panel.add(new JButton("Right"));
   panel.setBorder(BorderFactory.createEmptyBorder(5,5,5,5));

   return panel;
}

//----------------------------------------------------------------------------
//
// Method-
//       LayoutDemo.createListModel
//
// Purpose-
//       Create a ListModel object
//
//----------------------------------------------------------------------------
public static Container             // Resultant
   createListModel( )               // Create a ListModel object
{
   JList               list;
   DefaultListModel    listModel;

   listModel= new DefaultListModel();
   listModel.addElement("Joe Blow");
   listModel.addElement("Jane Doe");
   listModel.addElement("Donkey");
   listModel.addElement("Lord Farquar");
   listModel.addElement("Kathy Green");
   listModel.addElement("Shrek");
   listModel.addElement("John Smith");

   // Create the list and put it in a scroll pane.
   list= new JList(listModel);
   list.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
   list.setSelectedIndex(0);
   list.setVisibleRowCount(5);
   JScrollPane scrollPane= new JScrollPane(list);

   return scrollPane;
}

//----------------------------------------------------------------------------
//
// Method-
//       LayoutDemo.createText
//
// Purpose-
//       Create a simple JTextField object
//
//----------------------------------------------------------------------------
public static Container             // Resultant
   createText(String ident)         // Create a simple JTextField
{
   return new JTextField(ident);
}

//----------------------------------------------------------------------------
//
// Method-
//       LayoutDemo.createVertical
//
// Purpose-
//       Create a vertical panel object
//
//----------------------------------------------------------------------------
public static Container             // Resultant
   createVertical( )                // Create a Vertical group
{
   // Create a panel that uses GridLayout.
   JPanel panel= new JPanel();
   panel.setLayout(new GridLayout(0,1));
   panel.add(new JButton("Top"));
   panel.add(new JTextField("Middle", 6));
   panel.add(new JButton("Bottom"));
   panel.setBorder(BorderFactory.createEmptyBorder(5,5,5,5));

   return panel;
}

//----------------------------------------------------------------------------
//
// Method-
//       LayoutDemo.main
//
// Purpose-
//       Mainline code.
//
//----------------------------------------------------------------------------
public static void
   main(                            // Mainline code
     String[]          args)        // Argument array
{
   // Schedule a job for the event-dispatching thread,
   // creating and showing this application's GUI.
   javax.swing.SwingUtilities.invokeLater(
       new Runnable()
       {
           public void run()
           {
               LayoutDemo object= new LayoutDemo();
               object.createAndShowGUI();
           }
       }
   );
}

//----------------------------------------------------------------------------
//
// Class-
//       LayoutDemo.BorderLayoutPanel
//
// Purpose-
//       Demonstrate BorderLayout
//
//----------------------------------------------------------------------------
class BorderLayoutPanel extends JPanel {
public BorderLayoutPanel() {
   super(new BorderLayout());

   add(createListModel(), BorderLayout.CENTER);
   add(createText("North"), BorderLayout.NORTH);
   add(createHorizontal(), BorderLayout.SOUTH);
   add(createVertical(), BorderLayout.WEST);
   add(createButton("East"), BorderLayout.EAST);

   // Note how the borders are additive!
   setBorder(BorderFactory.createEmptyBorder(5,5,5,5));
}
} // class LayoutDemo.BorderLayoutPanel

//----------------------------------------------------------------------------
//
// Class-
//       LayoutDemo.GridBagLayoutPanel
//
// Purpose-
//       Demonstrate GridBagLayout
//
//----------------------------------------------------------------------------
class GridBagLayoutPanel extends JPanel {
public GridBagLayoutPanel() {
   GridBagLayout gridbag= new GridBagLayout();
   GridBagConstraints c= new GridBagConstraints();

   setFont(new Font("SansSerif", Font.PLAIN, 14));
   setLayout(gridbag);

   c.fill= GridBagConstraints.BOTH;
   c.weightx= 1.0;
   makebutton("Button1", gridbag, c);
   makebutton("Button2", gridbag, c);
   makebutton("Button3", gridbag, c);

   c.gridwidth= GridBagConstraints.REMAINDER; // end row
   makebutton("Button4", gridbag, c);

   c.weightx= 0.0;                  // reset to the default
   makebutton("Button5", gridbag, c); // another row

   c.gridwidth= GridBagConstraints.RELATIVE; // next-to-last in row
   makebutton("Button6", gridbag, c);

   c.gridwidth= GridBagConstraints.REMAINDER; // end row
   makebutton("Button7", gridbag, c);

   c.gridwidth= 1;                  // reset to the default
   c.gridheight= 2;
   c.weighty= 1.0;
   makebutton("Button8", gridbag, c);

   c.weighty= 0.0;                  // reset to the default
   c.gridwidth= GridBagConstraints.REMAINDER; // end row
   c.gridheight= 1;                 // reset to the default
   makebutton("Button9", gridbag, c);
   makebutton("Button10", gridbag, c);

   setSize(300, 100);
}

protected
   void makebutton(                 // Create constrained button
      String           name,
      GridBagLayout    gridbag,
      GridBagConstraints c)
{
   Button button= new Button(name);
   gridbag.setConstraints(button, c);
   add(button);
}
} // class LayoutDemo.GridBagLayoutPanel

//----------------------------------------------------------------------------
//
// Class-
//       LayoutDemo.GridLayoutPanel
//
// Purpose-
//       Demonstrate GridLayout
//
//----------------------------------------------------------------------------
class GridLayoutPanel extends JPanel {
public GridLayoutPanel() {
   super(new GridLayout(3,2));

   add(createButton("1"));
   add(createButton("02"));
   add(createButton("003"));
   add(createListModel());
   add(createButton("00005"));
   add(createButton("000006"));
}
} // class LayoutDemo.GridLayoutPanel

//----------------------------------------------------------------------------
//
// Class-
//       LayoutDemo.TestBagLayoutPanel
//
// Purpose-
//       Demonstrate TestBagLayout
//
// Implementation notes-
//       Requires JTextField.setColumns() to operate reasonably.
//
//----------------------------------------------------------------------------
class TestBagLayoutPanel extends JPanel {
public TestBagLayoutPanel() {
   GridBagLayout gridbag= new GridBagLayout();
   GridBagConstraints c= new GridBagConstraints();

   setFont(new Font("SansSerif", Font.PLAIN, 14));
   setLayout(gridbag);

   String[] hole= createHoleString();

   c.gridwidth= GridBagConstraints.REMAINDER; // end row
   Component comp= new JTextField("Golf club name", 97);
   gridbag.setConstraints(comp, c);
   add(comp);

   c.gridwidth= GridBagConstraints.REMAINDER; // end row
   comp= new JTextField("Player name", 97);
   gridbag.setConstraints(comp, c);
   add(comp);

   comp= createLinePanel(hole,gridbag);
   c.gridwidth= GridBagConstraints.REMAINDER; // end row
   gridbag.setConstraints(comp, c);
   add(comp);

   comp= createLinePanel(hole,gridbag);
   c.gridwidth= GridBagConstraints.REMAINDER; // end row
   gridbag.setConstraints(comp, c);
   add(comp);

   comp= createLinePanel(hole,gridbag);
   c.gridwidth= GridBagConstraints.REMAINDER; // end row
   gridbag.setConstraints(comp, c);
   add(comp);

   c.gridwidth= GridBagConstraints.REMAINDER; // end row
   comp= createButton("Button");
   gridbag.setConstraints(comp, c);
   add(comp);
}

public void
   createLineItem(                  // Insert a LinePanel object
      String           ident,       // The element name
      Container        content,     // The associated container
      GridBagLayout    gridbag,     // The layout
      GridBagConstraints c)         // The constraints
{
   JTextField field= new JTextField(ident, c.gridwidth);
   gridbag.setConstraints(field,c);
   content.add(field);
}

public Container                    // Resultant
   createLinePanel(                 // Create a LinePanel object
      String[]         list,        // The list of elements
      GridBagLayout    gridbag)     // The layout
{
   gridbag= new GridBagLayout();
   JPanel panel= new JPanel(gridbag);
   GridBagConstraints c= new GridBagConstraints();
   c.fill= GridBagConstraints.BOTH;

   panel.setFont(new Font("SansSerif", Font.PLAIN, 14));
   c.gridwidth= 10;
   createLineItem(list[0], panel, gridbag, c);

   c.gridwidth= 3;
   for(int i= 1; i<=9; i++)
   {
     createLineItem(list[i], panel, gridbag, c);
   }

   c.gridwidth= 4;
   createLineItem("Out", panel, gridbag, c);

   c.gridwidth= 3;
   for(int i= 10; i<=18; i++)
   {
     createLineItem(list[i], panel, gridbag, c);
   }

   c.gridwidth= 4;
   createLineItem("In", panel, gridbag, c);
   createLineItem("Tot", panel, gridbag, c);
   createLineItem("Esc", panel, gridbag, c);
   createLineItem("Hdcp", panel, gridbag, c);
   createLineItem("Net", panel, gridbag, c);

// c.gridwidth= GridBagConstraints.REMAINDER; // end row
// createLineItem("", panel, gridbag, c);

   return panel;
}
} // class LayoutDemo.TestBagLayoutPanel
} // class LayoutDemo

