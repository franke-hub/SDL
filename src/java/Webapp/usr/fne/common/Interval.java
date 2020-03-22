//----------------------------------------------------------------------------
//
//       Copyright (C) 2008 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Interval.java
//
// Purpose-
//       Time interval calculator.
//
// Last change date-
//       2008/01/01
//
//----------------------------------------------------------------------------
package usr.fne.common;

import java.io.*;
import java.lang.*;
import java.util.*;

public class Interval
{
//----------------------------------------------------------------------------
// Interval.Attributes
//----------------------------------------------------------------------------
Date                   time1;       // The start time

//----------------------------------------------------------------------------
//
// Method-
//       Interval.Interval
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   Interval( )                    // Constructor
{
   time1= new Date();
}

//----------------------------------------------------------------------------
//
// Method-
//       Interval.get
//
// Purpose-
//       Compute the Interval, in milliseconds, since the Interval was
//       constructed or the time function was called.
//
//----------------------------------------------------------------------------
public long                         // The interval, in milliseconds
   get( )                           // Compute interval
{
   long                result;      // Resultant

   Date time2= new Date();
   result= time2.getTime() - time1.getTime();
   time1= time2;

   return result;
}
} // Class Interval

