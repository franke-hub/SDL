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
//       CourseLongInfo.java
//
// Purpose-
//       Long drive information container class.
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
//       CourseLongInfo
//
// Purpose-
//       Long drive information container class.
//
//----------------------------------------------------------------------------
public class CourseLongInfo  extends SelectHoleInfo {
//----------------------------------------------------------------------------
//
// Method-
//       CourseLongInfo.CourseLongInfo
//
// Purpose-
//       Constructors.
//
//----------------------------------------------------------------------------
   CourseLongInfo(                  // Constructor
     int               format,      // The HolePanel format
     String[]          array)       // The hole selection data array
{
   super(format, array);
}

   CourseLongInfo(                  // Constructor
     int               format,      // The HolePanel format
     String            string)      // The hole selection  String
{
   this(format, tokenize(string));
}

   CourseLongInfo(                  // Constructor
     int               format)      // The HolePanel format
{
   this(format, empty);
}
} // class CourseLongInfo
