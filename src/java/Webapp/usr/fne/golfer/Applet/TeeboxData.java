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
//       TeeboxData.java
//
// Purpose-
//       Common Teebox data.
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
//       TeeboxData
//
// Purpose-
//       Common Teebox data.
//
//----------------------------------------------------------------------------
public class TeeboxData extends DebuggingAdaptor {
//----------------------------------------------------------------------------
// TeeboxData.Attributes
//----------------------------------------------------------------------------
String                 courseID;    // The COURSE_ID
String                 teeboxID;    // The TEEBOX_ID
String                 teeboxColor; // The TEEBOX color

//----------------------------------------------------------------------------
//
// Method-
//       TeeboxData.DebuggingInterface
//
// Purpose-
//       Implement DebuggingInterface
//
//----------------------------------------------------------------------------
public void
   debug( )                         // Debugging display
{
   print("TeeboxData.debug()");
   print(".courseID: "    + courseID);
   print(".teeboxID: "    + teeboxID);
   print(".teeboxColor: " + teeboxColor);
}

public boolean                      // If debugging active
   isDebug( )                       // Is debugging active?
{
   return false || super.isDebug();
}

//----------------------------------------------------------------------------
//
// Method-
//       TeeboxData.TeeboxData
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   TeeboxData(                      // Constructor
     String            courseID,    // The COURSE_ID
     String            teeboxID,    // The TEEBOX_ID
     String            teeboxColor) // The TEEBOX color
{
   this.courseID=    courseID;
   this.teeboxID=    teeboxID;
   this.teeboxColor= teeboxColor;
}

public
   TeeboxData( )                    // Default constructor
{
}

public
   TeeboxData(                      // Copy constructor
     TeeboxData        copy)        // Source TeeboxData
{
   synchronized(copy)
   {{{{
     this.courseID=    copy.courseID;
     this.teeboxID=    copy.teeboxID;
     this.teeboxColor= copy.teeboxColor;
   }}}}
}
} // class TeeboxData
