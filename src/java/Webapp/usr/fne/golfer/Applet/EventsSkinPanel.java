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
//       EventsSkinPanel.java
//
// Purpose-
//       Events skins panel.
//
// Last change date-
//       2020/01/15
//
// Side effect-
//       Colors the input panels!
//
//----------------------------------------------------------------------------
import java.awt.*;
import javax.swing.*;

import java.lang.*;
import java.util.*;

//----------------------------------------------------------------------------
//
// Class-
//       EventsSkinPanel
//
// Purpose-
//       Events skins Panel.
//
//----------------------------------------------------------------------------
public class EventsSkinPanel extends HolePanel {
//----------------------------------------------------------------------------
//
// Method-
//       EventsSkinPanel.EventsSkinPanel
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   EventsSkinPanel(                 // Constructor
     int               format,      // The number of fields
     String            title,       // Panel title
     HolePanel[]       holePanel)   // The associated HolePanel array
{
   super(format);

   Color               COLOR_SKIN= new Color(0x00E0E0FF);
   Color               COLOR_PUSH= new Color(0x00E0FFE0);

   int                 pCount= holePanel.length; // Number of panels
   int[]               sCount= new int[pCount]; // Number of skins in panel

   if( format <= HOLE_SKIN )
     throw new RuntimeException("EventsSkinPanel(format(" + format + "))");

   if( title == null )
     title= "Skins";
   field[HOLE_ID].setText(title);

   for(int j= 0; j<pCount; j++)
     sCount[j]= 0;

   int pushNo= 0; // PUSH total
   for(int hole= 1; hole <= 18; hole++)
   {
     int minIndex= (-1);
     int minValue= Integer.MAX_VALUE;
     boolean isPush= true;
     for(int j= 0; j<pCount; j++)
     {
       if( holePanel[j] == null )
         continue;

       boolean error= false;
       int     value= Integer.MAX_VALUE;
       try {
         value= holePanel[j].getFieldValue(hole);
       } catch(Exception e) {
         error= true;
       }

       if( !error )
       {
         if( minIndex < 0 )
         {
           minIndex= j;
           minValue= value;
           isPush= false;
         }
         else if( value < minValue )
         {
           minIndex= j;
           minValue= value;
           isPush= false;
         }
         else if( value == minValue )
           isPush= true;
       }
     }

     if( isPush )
     {
       field[hole].setText("" + 0);
       pushNo++;
     }
     else
     {
       ++pushNo;
       field[hole].setText("" + pushNo);
       sCount[minIndex] += pushNo;

       // Update background colors
       holePanel[minIndex].getField(hole).setBackground(COLOR_SKIN);
       int j= hole;
       while( pushNo > 1 )
       {
         j--;
         holePanel[minIndex].getField(j).setBackground(COLOR_PUSH);
         pushNo--;
       }

       pushNo= 0;
     }
   }

   field[HOLE_SKIN].setText("" + (18 - pushNo));

   // Update the net score skin results
   for(int j= 0; j<pCount; j++)
   {
     if( holePanel[j] != null )
       holePanel[j].getField(HOLE_SKIN).setText("" + sCount[j]);
   }
}

public
   EventsSkinPanel(                 // Constructor
     int               format,      // The number of fields
     String            title,       // Panel title
     EventsTeamInfo    teamInfo)    // The associated EventsTeamInfo
{
   this(format, title, infoToPanel(teamInfo));
}

public
   EventsSkinPanel(                 // Constructor
     int               format,      // The number of fields
     String            title,       // Panel title
     Vector<HolePanel> vector)      // Source net score panels
{
   this(format, title, vectorToArray(vector));
}

protected static HolePanel[]
   infoToPanel(
     HoleInfo[]        holeInfo)    // Source net score info
{
   HolePanel[]         array= new HolePanel[holeInfo.length];

   for(int i= 0; i<holeInfo.length; i++)
   {
     if( holeInfo[i] != null )
       array[i]= (HolePanel)holeInfo[i].getPanel();
   }

   return array;
}

protected static HolePanel[]
   infoToPanel(
     EventsTeamInfo    teamInfo)
{
   Vector<HolePanel>   vector= new Vector<HolePanel>();

   for(int i= 0; i<teamInfo.player_nets.length; i++)
   {
     if( teamInfo.player_nets[i] != null && !teamInfo.player_data[i].isGuest )
       vector.add((HolePanel)teamInfo.player_nets[i].getPanel());
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

//----------------------------------------------------------------------------
//
// Method-
//       EventsSkinPanel.countSkins
//
// Purpose-
//       Count the number of skins for a particular panel in a set
//
//----------------------------------------------------------------------------
public static int[]                 // The per hole skin count
   countSkins(                      // Count the number of skins
     int               panelIX,     // For this panel
     HolePanel[]       source)      // In this set of net score panels
{
   int                 pCount= source.length; // Number of panels
   int[]               result= new int[18]; // Hole-by-hole resultant

   for(int j= 0; j<result.length; j++)
     result[j]= 0;

   int pushNo= 0; // PUSH total
   for(int hole= 1; hole <= 18; hole++)
   {
     int minIndex= (-1);
     int minValue= Integer.MAX_VALUE;
     boolean isPush= true;
     for(int j= 0; j<pCount; j++)
     {
       if( source[j] == null )
         continue;

       boolean error= false;
       int     value= Integer.MAX_VALUE;
       try {
         value= source[j].getFieldValue(hole);
       } catch(Exception e) {
         error= true;
       }

       if( !error )
       {
         if( minIndex < 0 )
         {
           minIndex= j;
           minValue= value;
           isPush= false;
         }
         else if( value < minValue )
         {
           minIndex= j;
           minValue= value;
           isPush= false;
         }
         else if( value == minValue )
           isPush= true;
       }
     }

     if( isPush )
       pushNo++;
     else
     {
       if( minIndex == panelIX )
         result[hole-1]= pushNo + 1;
       pushNo= 0;
     }
   }

   return result;
}
} // class EventsSkinPanel
