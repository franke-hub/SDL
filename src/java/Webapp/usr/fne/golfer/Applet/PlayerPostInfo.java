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
//       PlayerPostInfo.java
//
// Purpose-
//       Player post gross score information container class.
//
// Last change date-
//       2020/01/15
//
// Usage-
//       This is a container for POST information.
//
//       If cardInfo is present, its isValid() and update() methods
//         must be called before this objects methods.
//
//----------------------------------------------------------------------------
import java.awt.*;
import java.awt.event.*;

import java.lang.*;
import java.util.*;
import javax.swing.*;

//----------------------------------------------------------------------------
//
// Class-
//       PlayerPostInfo
//
// Purpose-
//       Player card gross score information container class.
//
//----------------------------------------------------------------------------
public class PlayerPostInfo  extends DatabaseInfo {
PlayerCardInfo         cardInfo;    // The PlayerCardInfo (optional)
CourseTboxInfo         tboxInfo;    // The CourseTboxInfo
String                 courseID;    // The COURSE_ID
String                 teeboxID;    // The TEEBOX_ID
String                 date;        // The post date (optional)
String                 time;        // The post time (optional)
String                 score;       // The ESC score
String                 rating;      // The course rating
String                 slope;       // The course slope
String                 type;        // The type (optional)
boolean                used;        // Used in handicap calculation?

//----------------------------------------------------------------------------
//
// Method-
//       PlayerPostInfo.debug
//
// Purpose-
//       Extend DebuggingAdaptor.
//
//----------------------------------------------------------------------------
public void
   debug( )                         // Debugging display
{
   super.debug();
   print(".courseID: " + courseID);
   print(".teeboxID: " + teeboxID);
   print(".date: " + date);
   print(".time: " + time);
   print(".score: " + score);
   print(".rating: " + rating);
   print(".slope: " + slope);
   print(".type: " + type);
   print(".used: " + used);
}

//----------------------------------------------------------------------------
//
// Method-
//       PlayerPostInfo.PlayerPostInfo
//
// Purpose-
//       Constructors.
//
//----------------------------------------------------------------------------
   PlayerPostInfo(                  // Constructor
     CourseTboxInfo    tboxInfo,    // The CourseTboxInfo
     PlayerCardInfo    cardInfo,    // (OPTIONAL) The source PlayerCardInfo
     String            postDate,    // (OPTIONAL) The post date
     String            postTime,    // (OPTIONAL) The post time
     String[]          post_info)   // (OPTIONAL) The post_info
{
   super();
   this.cardInfo= cardInfo;
   this.tboxInfo= tboxInfo;
   this.type= "A";
   this.date= postDate;
   this.time= postTime;

   score= "";
   slope= tboxInfo.stringMS;
   rating= tboxInfo.stringMR;
   courseID= tboxInfo.courseID;
   teeboxID= tboxInfo.title;
   if( post_info != null )
   {
     isPresent= true;
     score= post_info[0];
     rating= post_info[1];
     slope= post_info[2];
     if( post_info.length > 3 )
       courseID= post_info[3];
     if( post_info.length > 4 )
       teeboxID= post_info[4];
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       PlayerPostInfo.courseDiff
//
// Purpose-
//       Extract the men's course score differential.
//
//----------------------------------------------------------------------------
public double                       // The Course differential
   courseDiff( )                    // Get Course differential
{
   return tboxInfo.courseDiff(score);
}

//----------------------------------------------------------------------------
//
// Method-
//       PlayerPostInfo.genPanel
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
   throw new RuntimeException("ShouldNotOccur");
}

//----------------------------------------------------------------------------
//
// Method-
//       PlayerPostInfo.isValid
//
// Purpose-
//       Validate the panel or the score.
//
//----------------------------------------------------------------------------
public synchronized boolean         // TRUE iff valid
   isValid(                         // Is panel valid
     Validator         v,           // Validator
     String            score)       // The score
{
   try {
     if( !score.equals("") )
     {
       int value= Integer.parseInt(score);
       if( value < 18 || value > 144 )
         throw new Exception("NG");
     }
   } catch( Exception e) {
     return v.invalid("Invalid score: " + score);
   }

   return true;
}

public synchronized boolean         // TRUE iff valid
   isValid(                         // Is panel valid
     Validator         v)           // Validator
{
   if( isRemoved )
     return true;

   if( cardInfo != null )
     return cardInfo.isValid(v);

   return isValid(v, score);
}

//----------------------------------------------------------------------------
//
// Method-
//       PlayerPostInfo.toString
//
// Purpose-
//       Extract the ARRAY into space separated String objects.
//
//----------------------------------------------------------------------------
public String                       // Resultant String
   toString( )                      // Convert to String
{
   return score + " " + rating + " " + slope + " " + courseID + " " + teeboxID;
}

//----------------------------------------------------------------------------
//
// Method-
//       PlayerPostInfo.update
//
// Purpose-
//       Update the internal fields, setting isChanged as appropriate.
//
//----------------------------------------------------------------------------
public synchronized void
   update(                          // Update from DATA
     String            score)       // New score
{
   if( !this.score.equals(score) )
   {
     isChanged= true;
     this.score= score;
   }
}

public synchronized void
   update( )                        // Update from PANEL (in cardInfo)
{
   if( cardInfo == null )
     throw new RuntimeException("ShouldNotOccur");

   update(cardInfo.getEscText());
}
} // class PlayerPostInfo
