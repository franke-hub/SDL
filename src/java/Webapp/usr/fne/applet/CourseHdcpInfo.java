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
//       CourseHdcpInfo.java
//
// Purpose-
//       Course handicap by hole information container class.
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
//       CourseHdcpInfo
//
// Purpose-
//       Course handicap by hole information container class.
//
//----------------------------------------------------------------------------
public class CourseHdcpInfo  extends HoleInfo {
//----------------------------------------------------------------------------
//
// Method-
//       CourseHdcpInfo.CourseHdcpInfo
//
// Purpose-
//       Constructors.
//
//----------------------------------------------------------------------------
   CourseHdcpInfo(                  // Constructor
     int               format,      // The HolePanel format
     String[]          array)       // The hole data array
{
   super(format, null, array);

   if( array == null )
   {
     isChanged= false;

     for(int hole= 1; hole<=18; hole++)
       this.array[hole-1]= new String("" + hole);
   }
}

   CourseHdcpInfo(                  // Constructor
     int               format,      // The HolePanel format
     String            string)      // The hole String
{
   this(format, tokenize(string));
}

   CourseHdcpInfo(                  // Constructor
     int               format)      // The HolePanel format
{
   this(format, empty);
}

   CourseHdcpInfo(                  // Copy constructor
     CourseHdcpInfo    source)      // Source CourseHdcpInfo
{
   super(source);
}

//----------------------------------------------------------------------------
//
// Method-
//       CourseHdcpInfo.genPanel
//
// Purpose-
//       Generate the internal panel, optional FocusListener and NextFocusAdapter
//
//----------------------------------------------------------------------------
public synchronized DataField       // Resultant tail DataField
   genPanel(                        // Generate new Panel
     FocusListener     listener,    // (OPTIONAL) FocusListener
     DataField         tail)        // (OPTIONAL) The current tail DataField
{
   // The panel is always generated
   tail= super.genPanel(listener, tail);
   panel.getField(HOLE_ID).setText("Hdcp");
   panel.setColor(Color.ORANGE, Color.BLACK);

   return tail;
}

//----------------------------------------------------------------------------
//
// Method-
//       CourseHdcpInfo.isDefault
//
// Purpose-
//       Does the ARRAY contain defaulted values?
//
//----------------------------------------------------------------------------
public synchronized boolean         // TRUE iff default
   isDefault( )                     // Is ARRAY default?
{
   boolean result= true;

   for(int hole= 1; hole<=18; hole++)
   {
     String string= "" + hole;
     if( !array[hole-1].equals(string) )
     {
       result= false;
       break;
     }
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       CourseHdcpInfo.isValid
//
// Purpose-
//       Validate the panel.
//
// Validators-
//       super()
//       range(3 .. 5)
//
//----------------------------------------------------------------------------
public synchronized boolean         // TRUE iff valid
   isValid(                         // Is panel valid
     Validator         v)           // Validator
{
   if( !super.isValid(v) )
     return false;

   if( !isRemoved && panel != null)
   {
     SelectHoleInfo select= new SelectHoleInfo(HOLE_TOT);
     int[] hcp= toInt();
     for(int hole= 1; hole<=18; hole++)
     {
       if( select.select(hcp[hole-1]) )
         return v.invalid("Handicap: multiple value: " + hcp[hole-1]);
     }
   }

   return true;
}
} // class CourseHdcpInfo
