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
//       CourseNearInfo.java
//
// Purpose-
//       Closest to pin information container class.
//
// Last change date-
//       2020/01/15
//
//----------------------------------------------------------------------------
import java.awt.Color;
import java.awt.event.FocusListener;

import java.lang.*;
import java.util.*;
import javax.swing.*;

//----------------------------------------------------------------------------
//
// Class-
//       CourseNearInfo
//
// Purpose-
//       Closest to pin information container class.
//
//----------------------------------------------------------------------------
public class CourseNearInfo  extends SelectHoleInfo {
//----------------------------------------------------------------------------
//
// Method-
//       CourseNearInfo.CourseNearInfo
//
// Purpose-
//       Constructors.
//
//----------------------------------------------------------------------------
   CourseNearInfo(                  // Constructor
     int               format,      // The HolePanel format
     String[]          array)       // The hole selection data array
{
   super(format, array);
}

   CourseNearInfo(                  // Constructor
     int               format,      // The HolePanel format
     String            string)      // The hole selection  String
{
   this(format, tokenize(string));
}

   CourseNearInfo(                  // Constructor
     int               format)      // The HolePanel format
{
   this(format, empty);
}
} // class CourseNearInfo
