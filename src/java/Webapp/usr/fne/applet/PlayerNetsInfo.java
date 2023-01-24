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
//       PlayerNetsInfo.java
//
// Purpose-
//       Player card net score information container class.
//
// Last change date-
//       2020/01/15
//
// Implementation notes-
//       The associated panel cannot be edited.
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
//       PlayerNetsInfo
//
// Purpose-
//       Player card net score information container class.
//
//----------------------------------------------------------------------------
public class PlayerNetsInfo extends HoleInfo {
String                 playerShow;  // The Player's name
String                 playerHdcp;  // The Player's handicap
CourseParsInfo         parsInfo;    // The CourseParsInfo
CourseTboxInfo         tboxInfo;    // The CourseTboxInfo
CourseHdcpInfo         hdcpInfo;    // The CourseHdcpInfo

// Hole field indexes
static final int       HOLE_ESC= SigmaHolePanel.HOLE_ESC;
static final int       HOLE_HCP= SigmaHolePanel.HOLE_HCP;
static final int       HOLE_NET= SigmaHolePanel.HOLE_NET;

static final int       HOLE_LDO= SigmaHolePanel.HOLE_LDO;
static final int       HOLE_CPO= SigmaHolePanel.HOLE_CPO;
static final int       HOLE_LDI= SigmaHolePanel.HOLE_LDI;
static final int       HOLE_CPI= SigmaHolePanel.HOLE_CPI;

//----------------------------------------------------------------------------
//
// Method-
//       PlayerNetsInfo.debug
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
}

//----------------------------------------------------------------------------
//
// Method-
//       PlayerNetsInfo.PlayerNetsInfo
//
// Purpose-
//       Constructors.
//
//----------------------------------------------------------------------------
   PlayerNetsInfo(                  // Constructor
     int               format,      // The HolePanel format
     String            playerShow,  // The Player's name
     String[]          score,       // The score data array
     String            playerHdcp,  // The Player's handicap
     CourseParsInfo    parsInfo,    // The CourseParsInfo
     CourseTboxInfo    tboxInfo,    // The CourseTboxInfo
     CourseHdcpInfo    hdcpInfo)    // The CourseHdcpInfo
{
   super(format, null, score);

   this.playerShow= playerShow;
   this.playerHdcp= playerHdcp;
   this.parsInfo= parsInfo;
   this.tboxInfo= tboxInfo;
   this.hdcpInfo= hdcpInfo;

   if( score == null )
   {
     for(int hole= 1; hole<=18; hole++)
       this.array[hole-1]= parsInfo.array[hole-1];
   }
}

   PlayerNetsInfo(                  // Constructor
     int               format,      // The HolePanel format
     String            playerShow,  // The Player's name
     String            score,       // The score data String
     String            playerHdcp,  // The Player's handicap
     CourseParsInfo    parsInfo,    // The CourseParsInfo
     CourseTboxInfo    tboxInfo,    // The CourseTboxInfo
     CourseHdcpInfo    hdcpInfo)    // The CourseHdcpInfo
{
   this(format, playerShow, tokenize(score), playerHdcp, parsInfo, tboxInfo, hdcpInfo);
}

//----------------------------------------------------------------------------
//
// Method-
//       PlayerNetsInfo.genPanel
//
// Purpose-
//       Generate the internal panel
//
//----------------------------------------------------------------------------
public synchronized DataField       // Resultant tail DataField (UNCHANGED)
   genPanel(                        // Generate new Panel
     FocusListener     listener,    // (UNUSED) FocusListener
     DataField         tail)        // (UNUSED) The current tail DataField
{
   // The panel is always generated
   super.genPanel(null, null);
   DataField[] df= panel.getField();
   df[HOLE_ID].setText(playerShow);

   boolean error= false;            // ERROR indicator
   int[] net= new int[18];          // The net score array
   int[] par= parsInfo.toInt();     // The par array
   int oTotal= 0;                   // OUT total
   int iTotal= 0;                   // IN  total
   int pTotal= 0;                   // PAR total
   int courseHdcp= tboxInfo.courseHdcp(playerHdcp);
   for(int hole= 1; hole<=18; hole++)
   {
     try {
       pTotal += par[hole-1];
       net[hole-1]= netScore(courseHdcp, array[hole-1], parsInfo.array[hole-1], hdcpInfo.array[hole-1]);
       if( parsInfo.array[hole-1].equalsIgnoreCase(FIELD_DP) )
         df[hole].setText(FIELD_DP);
       else
         df[hole].setText("" + net[hole-1]);

       if( hole < 10 )
         oTotal += net[hole-1];
       else
         iTotal += net[hole-1];
     } catch(Exception e) {
       if( !error )
       {
         e.printStackTrace();
         debug("Exception: " + e);
       }
       error= true;
     }
   }

   if( error )
     df[HOLE_TOT].setText(FIELD_ERR);
   else
   {
     int nTotal= (oTotal+iTotal) - pTotal; // NET total (relative to par)

     df[HOLE_OUT].setText("" + oTotal);
     df[HOLE_IN ].setText("" + iTotal);
     df[HOLE_TOT].setText("" + (oTotal+iTotal));
     df[HOLE_NET].setText((nTotal > 0 ? "+" : "") + nTotal);

     oTotal= 0; // Reuse, but now only for fieldColor calculation
     iTotal= 0;
     for(int hole= 1; hole<=18; hole++)
     {
       df[hole].setForeground(fieldColor(net[hole-1]-par[hole-1]));
       if( hole < 10 )
         oTotal += (net[hole-1]-par[hole-1]);
       else
         iTotal += (net[hole-1]-par[hole-1]);
     }
     df[HOLE_OUT].setForeground(fieldColor(oTotal));
     df[HOLE_IN ].setForeground(fieldColor(iTotal));
     df[HOLE_TOT].setForeground(fieldColor(nTotal));
     df[HOLE_NET].setForeground(fieldColor(nTotal));
   }

   if( format > HOLE_CPI )
   {
     df[HOLE_LDO].setText(getLDO());
     df[HOLE_CPO].setText(getCPO());
     df[HOLE_LDI].setText(getLDI());
     df[HOLE_CPI].setText(getCPI());
   }

   return tail;
}

//----------------------------------------------------------------------------
//
// Method-
//       PlayerNetsInfo.netScore
//
// Purpose-
//       Calculate NET score for hole.
//
//----------------------------------------------------------------------------
public static int                   // The NET hole score
   netScore(                        // Get NET hole score
     int               courseHdcp,  // The player's course handicap
     String            scoredCard,  // Carded score for hole
     String            par4Hole,    // Par for hole
     String            hcp4Hole)    // Handicap for hole
{
   int result= escScore(courseHdcp, scoredCard, par4Hole);
   result -= (courseHdcp/18);
   if( Integer.parseInt(hcp4Hole) <= (courseHdcp%18) )
     result--;

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       PlayerNetsInfo.isValid
//
// Purpose-
//       Validate the panel. (SHOULD NOT OCCUR)
//
//----------------------------------------------------------------------------
public synchronized boolean         // TRUE iff valid
   isValid(                         // Is panel valid
     Validator         v)           // Validator
{
   return false;                    // SHOULD NOT OCCUR
}

//----------------------------------------------------------------------------
//
// Method-
//       PlayerNetsInfo.toString
//
// Purpose-
//       (SHOULD NOT OCCUR)
//
//----------------------------------------------------------------------------
public String                       // Resultant String
   toString( )                      // Convert to String
{
   throw new RuntimeException("ShouldNotOccur");
}
} // class PlayerNetsInfo
