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
//       EventsBestPanel.java
//
// Purpose-
//       Events best two combined scores panel.
//
// Last change date-
//       2020/01/15
//
//----------------------------------------------------------------------------
import java.awt.*;
import javax.swing.*;

import java.lang.*;
import java.util.*;

//----------------------------------------------------------------------------
//
// Class-
//       EventsBestPanel
//
// Purpose-
//       Events best two combined scores panel.
//
//----------------------------------------------------------------------------
public class EventsBestPanel extends HolePanel {
//----------------------------------------------------------------------------
//
// Method-
//       EventsBestPanel.EventsBestPanel
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   EventsBestPanel(                 // Constructor
     int               format,      // The number of fields
     String            title,       // Panel title
     CourseParsInfo    parsInfo,    // The associated CourseParsInfo
     HolePanel[]       escScore,    // Source ESC score panels
     HolePanel[]       netScore)    // Source net score panels
{
   super(format);

   int                 pCount= netScore.length; // Number of panels
   int[]               sCount= new int[pCount]; // Number of skins

   if( format <= HOLE_NET )
     throw new RuntimeException("EventsBestPanel(format(" + format + "))");

   if( title == null )
     title= "Skins";
   field[HOLE_ID].setText(title);

   boolean error= false;
   int iScore= 0;
   int oScore= 0;
   int oPar= 0;
   int iPar= 0;
   for(int hole= 1; hole <= 18; hole++)
   {
     int par= 4;
     try {
       par= parsInfo.getPanel().getFieldValue(hole);
     } catch(Exception e) {
       error= true;
     }

     int minIndex1= (-1);
     int minIndex2= (-1);
     int minValue1= Integer.MAX_VALUE;
     int minValue2= Integer.MAX_VALUE;
     for(int j= 0; j<pCount; j++)
     {
       if( escScore[j] == null || netScore[j] == null )
         continue;

       if( escScore[j].getFieldText(hole).equalsIgnoreCase(FIELD_DP) )
         continue;

       int value= Integer.MAX_VALUE;
       try {
         value= netScore[j].getFieldValue(hole);

         if( minIndex1 < 0 )
         {
           minIndex1= j;
           minValue1= value;
         }
         else if( minIndex2 < 0 )
         {
           minIndex2= j;
           minValue2= value;
         }
         else if( value < minValue1 || value < minValue2 )
         {
           if( minValue1 < minValue2 )
           {
             minIndex2= j;
             minValue2= value;
           }
           else
           {
             minIndex1= j;
             minValue1= value;
           }
         }
       } catch(Exception e) {
       }
     }

     int hTotal= par + par;
     if( minIndex2 < 0 )
     {
       hTotal= par + par + par + par;
       field[hole].setText(FIELD_DP);
     }
     else
     {
       hTotal= minValue1 + minValue2;
       field[hole].setText("" + hTotal);
     }
     field[hole].setForeground(fieldColor(hTotal-par-par));
     if( hole <= 9 )
     {
       oPar   += par;
       oScore += hTotal;
     }
     else
     {
       iPar   += par;
       iScore += hTotal;
     }
   }

   field[HOLE_OUT].setText("" + oScore);
   field[HOLE_OUT].setForeground(fieldColor(oScore-oPar-oPar));
   field[HOLE_IN ].setText("" + iScore);
   field[HOLE_IN ].setForeground(fieldColor(iScore-iPar-iPar));

   if( error )
   {
     field[HOLE_TOT].setText(FIELD_ERR);
     field[HOLE_NET].setText(FIELD_ERR);
   }
   else
   {
     int net= oScore+iScore;
     field[HOLE_TOT].setText("" + net);
     net -= (oPar+oPar+iPar+iPar);
     field[HOLE_TOT].setForeground(fieldColor(net));
     String string= net > 0 ? "+" : "";
     field[HOLE_NET].setText(string + net);
     field[HOLE_NET].setForeground(fieldColor(net));
   }
}

public
   EventsBestPanel(                 // Constructor
     int               format,      // The number of fields
     String            title,       // Panel title
     CourseParsInfo    parsInfo,    // The associated CourseParsInfo
     EventsTeamInfo    teamInfo)    // The associated EventsTeamInfo
{
   this(format, title, parsInfo, infoToPanel(teamInfo,teamInfo.player_card), infoToPanel(teamInfo,teamInfo.player_nets));
}

protected static HolePanel[]
   infoToPanel(
     EventsTeamInfo    teamInfo,
     HoleInfo[]        holeInfo)
{
   Vector<HolePanel>   vector= new Vector<HolePanel>();

   for(int i= 0; i<holeInfo.length; i++)
   {
     if( holeInfo[i] != null && !teamInfo.player_data[i].isGuest )
       vector.add((HolePanel)holeInfo[i].getPanel());
   }

   return vectorToArray(vector);
}

protected static HolePanel[]
   vectorToArray(
     Vector<HolePanel> vector)
{
   HolePanel[] array= new HolePanel[vector.size()];
   for(int i= 0; i<vector.size(); i++)
     array[i]= vector.elementAt(i);

   return array;
}
} // class EventsBestPanel
