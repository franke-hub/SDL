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
//       EventsDateInfo.java
//
// Purpose-
//       Common Events date data.
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
//       EventsDateInfo
//
// Purpose-
//       Common Events team data.
//
//----------------------------------------------------------------------------
public class EventsDateInfo extends DatabaseInfo {
//----------------------------------------------------------------------------
// EventsDateInfo.Attributes
//----------------------------------------------------------------------------
String                 date;        // Event date
String[]               time;        // Event times
String                 courseID;    // The COURSE_ID
String                 teeboxID;    // The TEEBOX_ID
PlayerData[]           player_data; // The PlayerData array

//----------------------------------------------------------------------------
//
// Method-
//       EventsDateInfo.DebuggingInterface
//
// Purpose-
//       Extend DebuggingInterface
//
//----------------------------------------------------------------------------
public void
   debug( )                         // Debugging display
{
   print(".date: " + date);
   StringBuffer buff= new StringBuffer();
   for(int i= 0; i<time.length; i++)
     buff.append((i == 0 ? "" : " ") + time[i]);
   print(".time: " + buff.toString());
   print(".courseID: " + courseID);
   print(".teeboxID: " + teeboxID);
   for(int i= 0; i<player_data.length; i++)
   {
     buff= new StringBuffer();
     buff.append(".player[" + i + "]");
     buff.append(" " + player_data[i].playerID);
     buff.append(" " + player_data[i].playerNN);
     print(buff.toString());
   }
}

public boolean                      // If debugging active
   isDebug( )                       // Is debugging active?
{
   return false || super.isDebug();
}
} // class EventsDateInfo
