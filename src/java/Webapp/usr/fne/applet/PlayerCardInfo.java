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
//       PlayerCardInfo.java
//
// Purpose-
//       Player card gross score information container class.
//
// Last change date-
//       2020/01/15
//
//----------------------------------------------------------------------------
import java.awt.*;
import java.awt.event.*;

import java.lang.*;
import java.util.*;
import javax.swing.*;

//----------------------------------------------------------------------------
//
// Class-
//       PlayerCardInfo
//
// Purpose-
//       Player card gross score information container class.
//
//----------------------------------------------------------------------------
public class PlayerCardInfo  extends SigmaHoleInfo {
String                 playerShow;  // The Player's name
String                 playerHdcp;  // The Player's handicap
CourseParsInfo         parsInfo;    // The CourseParsInfo
CourseTboxInfo         tboxInfo;    // The CourseTboxInfo

// Hole field indexes
static final int       HOLE_ESC= SigmaHolePanel.HOLE_ESC;
static final int       HOLE_HCP= SigmaHolePanel.HOLE_HCP;
static final int       HOLE_NET= SigmaHolePanel.HOLE_NET;

static final int       HOLE_LDO= SigmaHolePanel.HOLE_LDO;
static final int       HOLE_CPO= SigmaHolePanel.HOLE_CPO;
static final int       HOLE_LDI= SigmaHolePanel.HOLE_LDI;
static final int       HOLE_CPI= SigmaHolePanel.HOLE_CPI;
static final int       HOLE_FWH= SigmaHolePanel.HOLE_FWH;
static final int       HOLE_GIR= SigmaHolePanel.HOLE_GIR;

// Format constants
static final int       FORMAT_USER= SigmaHolePanel.FORMAT_USER;
static final int       FORMAT_BASE= SigmaHolePanel.FORMAT_BASE;
static final int       FORMAT_EVNT= SigmaHolePanel.FORMAT_EVNT;
static final int       FORMAT_SKIN= SigmaHolePanel.FORMAT_SKIN;

//----------------------------------------------------------------------------
//
// Method-
//       PlayerCardInfo.PlayerCardInfo
//
// Purpose-
//       Constructors.
//
//----------------------------------------------------------------------------
   PlayerCardInfo(                  // Constructor
     int               format,      // The HolePanel format
     String            playerShow,  // The Player's name
     String[]          score,       // The score data array
     String            playerHdcp,  // The Player's handicap
     CourseParsInfo    parsInfo,    // The CourseParsInfo
     CourseTboxInfo    tboxInfo)    // The CourseTboxInfo
{
   super(format, null, score);

   this.playerShow= playerShow;
   this.playerHdcp= playerHdcp;
   this.parsInfo= parsInfo;
   this.tboxInfo= tboxInfo;

   if( score == null )
   {
     array= new String[ARRAY_TEEBOX+1];
     for(int hole= 1; hole<=array.length; hole++)
       array[hole-1]= "";

     array[ARRAY_COURSE]= tboxInfo.courseID;
     array[ARRAY_TEEBOX]= tboxInfo.title;
   }
}

   PlayerCardInfo(                  // Constructor
     int               format,      // The HolePanel format
     String            playerShow,  // The Player's name
     String            score,       // The score data String
     String            playerHdcp,  // The Player's handicap
     CourseParsInfo    parsInfo,    // The CourseParsInfo
     CourseTboxInfo    tboxInfo)    // The CourseTboxInfo
{
   this(format, playerShow, tokenize(score), playerHdcp,
        parsInfo, tboxInfo);
}

   PlayerCardInfo(                  // Copy constructor
     PlayerCardInfo    source)      // Source PlayerCardInfo
{
   super(source);

   this.playerShow= source.playerShow;
   this.playerHdcp= source.playerHdcp;
   this.parsInfo= source.parsInfo;
   this.tboxInfo= source.tboxInfo;
}

//----------------------------------------------------------------------------
//
// Method-
//       PlayerCardInfo.DebuggingAdaptor
//
// Purpose-
//       Extend DebuggingAdaptor.
//
//----------------------------------------------------------------------------
public void
   debug( )                         // Debugging display
{
   super.debug();
   print(".playerShow: " + playerShow);
   print(".playerHdcp: " + playerHdcp);
   print(".isDefault: " + isDefault());
}

//----------------------------------------------------------------------------
//
// Method-
//       PlayerCardInfo.change
//
// Purpose-
//       Update HOLE_ESC and HOLE_NET in panel
//
//----------------------------------------------------------------------------
public void
   change( )                        // Handle HOLE_TOT field change
{
   DataField[] df= panel.getField();
   boolean isDefault= true;
   for(int hole= 1; hole<=18; hole++)
   {
     if( !df[hole].getText().equals("") )
     {
       isDefault= false;
       break;
     }
   }

   if( isDefault )
   {
     df[HOLE_OUT].setText("");
     df[HOLE_IN].setText("");
     df[HOLE_TOT].setText("");
     df[HOLE_ESC].setText("");
     df[HOLE_NET].setText("");
   }
   else
   {
     try {
       int courseHdcp= tboxInfo.courseHdcp(playerHdcp);
       int esc= 0;
       for(int hole= 1; hole<=18; hole++)
         esc += escScore(courseHdcp, df[hole].getText(), parsInfo.array[hole-1]);

       df[HOLE_ESC].setText("" + esc);
       int net= esc - tboxInfo.courseHdcp(playerHdcp);
       df[HOLE_NET].setText("" + net);
     } catch(Exception e) {
       df[HOLE_ESC].setText(FIELD_ERR);
       df[HOLE_NET].setText(FIELD_ERR);
     }
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       PlayerCardInfo.getEscText
//       PlayerCardInfo.setEscText
//
// Purpose-
//       Retrieve/set the ESC field.
//       Setting the ESC field removes SIGMA controls.
//
//----------------------------------------------------------------------------
public String                       // The ESC field
   getEscText( )                    // Get ESC field
{
   return panel.getFieldText(HOLE_ESC);
}

public void
   setEscText(                      // Set ESC field
     String            esc)         // Set ESC field
{
   DataField[] df= panel.getField();
   df[HOLE_OUT]= new DataField(df[HOLE_OUT]);
   df[HOLE_IN]=  new DataField(df[HOLE_IN]);
   df[HOLE_TOT]= new DataField(df[HOLE_TOT]);
   df[HOLE_ESC]= new DataField(df[HOLE_ESC]);

   df[HOLE_OUT].setText("");
   df[HOLE_IN].setText("");
   df[HOLE_TOT].setText("");
   df[HOLE_ESC].setText(esc);
   panel.setField();
}

//----------------------------------------------------------------------------
//
// Method-
//       PlayerCardInfo.genPanel
//
// Purpose-
//       Generate the internal panel, optional FocusListener and NextFocusAdapter
//
//----------------------------------------------------------------------------
public synchronized DataField       // Resultant tail DataField
   genPanel(                        // Generate new Panel
     FocusListener     listener,    // (OPTIONAL) FocusListener
     DataField         tail)        // (OPTIONAL) The current tail DataField
{
   panel= null;                     // The panel is conditionally generated
   if( !isRemoved )
   {
     tail= super.genPanel(listener, tail);
     DataField[] df= panel.getField();
     df[HOLE_ID].setText(playerShow);

     int[] par= parsInfo.toInt();
     for(int hole= 1; hole<=18; hole++)
       df[hole].setNamedValuePAR2(par[hole-1]);

     df[HOLE_IN].change();          // Corrects "DP" fields in super.genPanel()
     df[HOLE_OUT].change();
     df[HOLE_TOT].change();

     if( playerHdcp != null )
     {
       df[HOLE_HCP].setText("" + tboxInfo.courseHdcp(playerHdcp));
       df[HOLE_TOT].addActionListener(new TotalFieldListener(this));
       change();
     }

     if( format > HOLE_CPI )
     {
       df[HOLE_LDO].setNamedValueLONG().setText(getLDO());
       df[HOLE_CPO].setNamedValueNEAR().setText(getCPO());
       df[HOLE_LDI].setNamedValueLONG().setText(getLDI());
       df[HOLE_CPI].setNamedValueNEAR().setText(getCPI());
       df[HOLE_FWH].setNamedValueLONG().setText(getFWH());
       df[HOLE_GIR].setNamedValueLONG().setText(getGIR());
       if( tail != null )
       {
         tail.addFocusListener(new NextFocusAdapter(df[HOLE_LDO]));
         tail= df[HOLE_GIR];
       }

       if( listener != null )
       {
         df[HOLE_LDO].addFocusListener(listener);
         df[HOLE_CPO].addFocusListener(listener);
         df[HOLE_LDI].addFocusListener(listener);
         df[HOLE_CPI].addFocusListener(listener);
         df[HOLE_FWH].addFocusListener(listener);
         df[HOLE_GIR].addFocusListener(listener);

         df[HOLE_LDO].setEditable(true);
         df[HOLE_CPO].setEditable(true);
         df[HOLE_LDI].setEditable(true);
         df[HOLE_CPI].setEditable(true);
         df[HOLE_FWH].setEditable(true);
         df[HOLE_GIR].setEditable(true);
       }
     }
   }

   return tail;
}

//----------------------------------------------------------------------------
//
// Method-
//       PlayerCardInfo.isDefault
//
// Purpose-
//       Does the ARRAY contain defaulted values?
//
//----------------------------------------------------------------------------
public synchronized boolean         // TRUE iff default
   isDefault( )                     // Is ARRAY default?
{
   boolean result= true;

   for(int i= 0; i<18; i++)
   {
     if( !array[i].equals("") )
     {
       result= false;
       break;
     }
   }

   if( (!getLDO().equals("") && !getLDO().equals("-"))
       || (!getLDI().equals("") && !getLDI().equals("-"))
       || (!getCPO().equals("") && !getCPO().equals("-"))
       || (!getCPI().equals("") && !getCPI().equals("-"))
       || (!getFWH().equals("") && !getGIR().equals("-"))
     )
     result= false;

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       PlayerCardInfo.isValid
//
// Purpose-
//       Validate the panel.
//
// Validators-
//       super()
//       range(3 .. 5)
//
//----------------------------------------------------------------------------
public synchronized boolean         // TRUE iff valid
   isValid(                         // Is panel valid
     Validator         v)           // Validator
{
   if( !isRemoved && panel != null)
   {
     DataField[] df= panel.getField();
     try {
       boolean isDefault= true;
       for(int hole= 1; hole<=18; hole++)
       {
         if( !df[hole].getText().equals("") )
         {
           isDefault= false;
           break;
         }
       }

       if( df.length > HOLE_CPI )
       {
         if( df[HOLE_LDO].getText().equals("") )
           df[HOLE_LDO].setText("-");
         if( df[HOLE_LDI].getText().equals("") )
           df[HOLE_LDI].setText("-");
         if( df[HOLE_CPO].getText().equals("") )
           df[HOLE_CPO].setText("-");
         if( df[HOLE_CPI].getText().equals("") )
           df[HOLE_CPI].setText("-");
         if( df[HOLE_FWH].getText().equals("") )
           df[HOLE_FWH].setText("-");
         if( df[HOLE_GIR].getText().equals("") )
           df[HOLE_GIR].setText("-");

         if( !df[HOLE_LDO].getText().equals("-")
             || !df[HOLE_LDI].getText().equals("-")
             || !df[HOLE_CPO].getText().equals("-")
             || !df[HOLE_CPI].getText().equals("-")
             || !df[HOLE_FWH].getText().equals("-")
             || !df[HOLE_GIR].getText().equals("-") )
           isDefault= false;
       }

       if( isDefault )
         return true;

       if( !super.isValid(v) )
         return false;

       for(int hole= 1; hole<=18; hole++)
       {
         int score= df[hole].intValue();
         if( score < 1 )
           return v.invalid("Hole[" + hole + "] SCORE(" + score+ ") range(>0)");
       }

       if( df.length > HOLE_CPI )
       {
         if( !v.isValidCP("CPI("+playerShow+")", df[HOLE_CPO])
             || !v.isValidCP("CPI("+playerShow+")", df[HOLE_CPI])
             || !v.isValidLD("LDO("+playerShow+")", df[HOLE_LDO])
             || !v.isValidLD("LDI("+playerShow+")", df[HOLE_LDI]) )
           return false;
         if( df[HOLE_FWH].intValue() < 0 || df[HOLE_FWH].intValue() > 18 )
           return v.invalid("FWH range(0..18) " + df[HOLE_FWH].intValue());
         if( df[HOLE_GIR].intValue() < 0 || df[HOLE_GIR].intValue() > 18 )
            return v.invalid("GIR range(0..18) " + df[HOLE_GIR].intValue());
       }
     } catch( NumberFormatException e ) {
       return v.invalid("NumberFormat: " + e);
     } catch( Exception e ) {
       e.printStackTrace();
       System.out.println("Exception: " + e);
       return v.invalid("Exception: " + e);
     }
   }

   return true;
}

//----------------------------------------------------------------------------
//
// Method-
//       PlayerCardInfo.toString
//
// Purpose-
//       Extract the ARRAY into space separated String objects.
//
//----------------------------------------------------------------------------
public String                       // Resultant String
   toString( )                      // Convert to String
{
   return arrayToString();
}

//----------------------------------------------------------------------------
//
// Method-
//       PlayerCardInfo.update
//
// Purpose-
//       Update the internal fields, setting isChanged as appropriate.
//
//----------------------------------------------------------------------------
public synchronized void
   update( )                        // Update from PANEL
{
   if( panel != null )
   {
     super.update();

     if( format > HOLE_CPI )
     {
       DataField df[]= panel.getField();
       if( !df[HOLE_LDO].getText().equals(getLDO())
           || !df[HOLE_LDI].getText().equals(getLDI())
           || !df[HOLE_CPO].getText().equals(getCPO())
           || !df[HOLE_CPI].getText().equals(getCPI())
           || !df[HOLE_FWH].getText().equals(getFWH())
           || !df[HOLE_GIR].getText().equals(getGIR())
         )
       {
         isChanged= true;
         setLDO(df[HOLE_LDO].getText());
         setLDI(df[HOLE_LDI].getText());
         setCPO(df[HOLE_CPO].getText());
         setCPI(df[HOLE_CPI].getText());
         setFWH(df[HOLE_FWH].getText());
         setGIR(df[HOLE_GIR].getText());
       }
     }
   }
}

//----------------------------------------------------------------------------
//
// Class-
//       TotalFieldListener
//
// Purpose-
//       Listen for and drive field change events.
//
//----------------------------------------------------------------------------
public class TotalFieldListener implements ActionListener {
PlayerCardInfo         owner;       // The associated PlayerCardInfo

   TotalFieldListener(              // Constructor
     PlayerCardInfo    owner)       // The owner
{
   this.owner= owner;
}

public void
   actionPerformed(                 // Implement ActionListener
     ActionEvent       e)           // The ActionEvent
{
   owner.change();
}
} // class TotalFieldListener
} // class PlayerCardInfo
