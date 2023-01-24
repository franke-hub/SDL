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
//       CourseData.java
//
// Purpose-
//       Common Course data.
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
//       CourseData
//
// Purpose-
//       Common Course data.
//
//----------------------------------------------------------------------------
public class CourseData extends DebuggingAdaptor {
//----------------------------------------------------------------------------
// CourseData.Attributes
//----------------------------------------------------------------------------
String                 courseID;    // The COURSE_ID
String                 courseShow;  // The COURSE_SHOW

//----------------------------------------------------------------------------
//
// Method-
//       CourseData.DebuggingInterface
//
// Purpose-
//       Implement DebuggingInterface
//
//----------------------------------------------------------------------------
public void
   debug( )                         // Debugging display
{
   print("CourseData.debug()");
   print(".courseID: "   + courseID);
   print(".courseShow: " + courseShow);
}

public boolean                      // If debugging active
   isDebug( )                       // Is debugging active?
{
   return false || super.isDebug();
}

//----------------------------------------------------------------------------
//
// Method-
//       CourseData.CourseData
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   CourseData(                      // Constructor
     String            courseID,    // The COURSE_ID
     String            courseShow)  // The COURSE_SHOW
{
   this.courseID=   courseID;
   this.courseShow= courseShow;
}

public
   CourseData( )                    // Default constructor
{
}

public
   CourseData(                      // Copy constructor
     CourseData        copy)        // Source CourseData
{
   synchronized(copy)
   {{{{
     this.courseID=   copy.courseID;
     this.courseShow= copy.courseShow;
   }}}}
}
} // class CourseData
