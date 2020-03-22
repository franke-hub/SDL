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
//       EventsTeamInfo.java
//
// Purpose-
//       Common Events team data.
//
// Last change date-
//       2020/01/15
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
//       EventsTeamInfo
//
// Purpose-
//       Common Events team data.
//
//----------------------------------------------------------------------------
public class EventsTeamInfo extends DatabaseInfo {
//----------------------------------------------------------------------------
// EventsTeamInfo.Attributes
//----------------------------------------------------------------------------
String                 date;        // The date
String                 time;        // The tee time
PlayerData[]           player_data; // The team
PlayerCardInfo[]       player_card; // The gross score information (hole-by-hole)
PlayerNetsInfo[]       player_nets; // The net score information
PlayerPostInfo[]       player_post; // The gross score information (summary)

HolePanel              bestPanel;   // Team BEST panel
HolePanel              ldcpPanel;   // Team LD/CP panel
HolePanel              skinPanel;   // Player SKIN panel
DataPanel              teamPanel;   // Team (time) panel

//----------------------------------------------------------------------------
//
// Method-
//       EventsTeamInfo.DebuggingInterface
//
// Purpose-
//       Extend DebuggingInterface
//
//----------------------------------------------------------------------------
public void
   debug( )                         // Debugging display
{
   print(".date: " + date);
   print(".time: " + time);
   for(int i= 0; i<player_data.length; i++)
   {
     StringBuffer buff= new StringBuffer();
     buff.append(".player[" + i + "]");
     buff.append(" " + player_data[i].playerNN);
     if( player_card[i] == null )
       buff.append(" <No score>");
     else
       buff.append(" " + player_card[i].toString());
     print(buff.toString());
   }
}

public boolean                      // If debugging active
   isDebug( )                       // Is debugging active?
{
   return false || super.isDebug();
}

//----------------------------------------------------------------------------
//
// Method-
//       EventsTeamInfo.highlight
//
// Purpose-
//       Highlight the LongDrive and ClosePin holes
//
// Prerequisites-
//       EventsTeam panels already generated, ldcpInfo panel generated.
//
//----------------------------------------------------------------------------
public void
   highlight(                       // Highlight LcCp holes
     CourseLdCpInfo    ldcpInfo)    // For this LD/CP info
{
   DataPanel panel= ldcpInfo.getPanel();
   DataField[] df= panel.getField();
   for(int hole= 1; hole<=18; hole++)
   {
     if( !df[hole].getText().equals(CourseLdCpInfo.DEFAULT_VALUE) )
     {
       df[hole].setBackground(Color.YELLOW.brighter());
       for(int j= 0; j<player_data.length; j++)
       {
         if( player_card[j] != null )
           player_card[j].getPanel().getField(hole).setBackground(Color.YELLOW.brighter());
       }
     }
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       EventsTeamInfo.toString
//
// Purpose-
//       Used for EVENTS_TIME data.
//
//----------------------------------------------------------------------------
public String                       // Associated EVENTS_TIME data
   toString( )                      // Convert to String
{
   StringBuffer buff= new StringBuffer();
   for(int i= 0; i<player_data.length; i++)
     buff.append((i == 0 ? "" : " ") + player_data[i].playerNN);

   return buff.toString();
}
} // class EventsTeamInfo
