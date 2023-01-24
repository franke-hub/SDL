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
//       EventsData.java
//
// Purpose-
//       Common Events data.
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
//       EventsData
//
// Purpose-
//       Common Events data.
//
//----------------------------------------------------------------------------
public class EventsData extends DebuggingAdaptor {
//----------------------------------------------------------------------------
// EventsData.Attributes
//----------------------------------------------------------------------------
String                 eventsID;    // The EVENTS_ID
String                 eventsShow;  // The EVENTS_SHOW

//----------------------------------------------------------------------------
//
// Method-
//       EventsData.DebuggingInterface
//
// Purpose-
//       Implement DebuggingInterface
//
//----------------------------------------------------------------------------
public void
   debug( )                         // Debugging display
{
   print("EventsData.debug()");
   print(".eventsID: "   + eventsID);
   print(".eventsShow: " + eventsShow);
}

public boolean                      // If debugging active
   isDebug( )                       // Is debugging active?
{
   return false || super.isDebug();
}

//----------------------------------------------------------------------------
//
// Method-
//       EventsData.EventsData
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   EventsData(                      // Constructor
     String            eventsID,    // The EVENTS_ID
     String            eventsShow)  // The EVENTS_SHOW
{
   this.eventsID=   eventsID;
   this.eventsShow= eventsShow;
}

public
   EventsData( )                    // Default constructor
{
}

public
   EventsData(                      // Copy constructor
     EventsData        copy)        // Source EventsData
{
   synchronized(copy)
   {{{{
     this.eventsID=   copy.eventsID;
     this.eventsShow= copy.eventsShow;
   }}}}
}
} // class EventsData
