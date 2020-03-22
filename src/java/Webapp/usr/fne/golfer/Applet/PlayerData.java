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
//       PlayerData.java
//
// Purpose-
//       Common Player data.
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
//       PlayerData
//
// Purpose-
//       Common Player data.
//
// Implementation notes-
//       EVENTS.PLAY <event>.<date> <name>.G
//       Turns on isGuest. This indicates that the player DOES NOT participate
//       in scoring for team events, beer fund, and is not considered for
//       high/low statistics. Scores DO count for player handicaps.
//
//----------------------------------------------------------------------------
public class PlayerData extends DebuggingAdaptor {
//----------------------------------------------------------------------------
// PlayerData.Attributes
//----------------------------------------------------------------------------
String                 playerID;    // The PLAYER_ID
String                 playerNN;    // The PLAYER_NN
String                 playerShow;  // The PLAYER_SHOW
String                 playerHdcp;  // The PLAYER_HDCP <OPTIONAL>
boolean                isGuest;     // TRUE iff playing as guest

//----------------------------------------------------------------------------
//
// Method-
//       PlayerData.DebuggingInterface
//
// Purpose-
//       Implement DebuggingInterface
//
//----------------------------------------------------------------------------
public void
   debug( )                         // Debugging display
{
   print("PlayerData.debug()");
   print(".playerID: "   + playerID);
   print(".playerNN: "   + playerNN);
   print(".playerShow: " + playerShow);
   print(".playerHdcp: " + playerHdcp);
   print(".isGuest: "    + isGuest);
}

public boolean                      // If debugging active
   isDebug( )                       // Is debugging active?
{
   return false || super.isDebug();
}

//----------------------------------------------------------------------------
//
// Method-
//       PlayerData.PlayerData
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   PlayerData( )                    // Default constructor
{
}

   PlayerData(                      // Constructor
     String            playerID,    // The PLAYER_ID
     String            playerNN,    // The PLAYER_NN
     String            playerShow,  // The PLAYER_SHOW
     String            playerHdcp,  // The PLAYER_HDCP <OPTIONAL>
     boolean           isGuest)     // TRUE if playing as guest
{
   this.playerID=   playerID;
   this.playerNN=   playerNN;
   this.playerShow= playerShow;
   this.playerHdcp= playerHdcp;
   this.isGuest=    isGuest;
}

   PlayerData(                      // Constructor
     String            playerID,    // The PLAYER_ID
     String            playerNN,    // The PLAYER_NN
     String            playerShow,  // The PLAYER_SHOW
     String            playerHdcp)  // The PLAYER_HDCP <OPTIONAL>
{
   this(playerID, playerNN, playerShow, playerHdcp, false);
}

public
   PlayerData(                      // Copy constructor
     PlayerData        copy)        // Source PlayerData
{
   synchronized(copy)
   {{{{
     this.playerID=   copy.playerID;
     this.playerNN=   copy.playerNN;
     this.playerShow= copy.playerShow;
     this.playerHdcp= copy.playerHdcp;
     this.isGuest=    copy.isGuest;
   }}}}
}

//----------------------------------------------------------------------------
//
// Method-
//       PlayerData.equals
//
// Purpose-
//       Equality evaluator
//
//----------------------------------------------------------------------------
public boolean                      // TRUE iff equal
   equals(                          // Test for equality
     Object            object)      // With this Object
{
   boolean result= false;
   if( object instanceof PlayerData )
     result= playerID.equals(((PlayerData)object).playerID);

   return result;
}

//----------------------------------------------------------------------------
//
// Static method-
//       PlayerData.getNickName
//
// Purpose-
//       Remove .G suffix from nickname, if present
//
//----------------------------------------------------------------------------
static public String                // Nickname without suffix
   getNickName(                     // Remove .G suffix, if present
      String             string)    // From this String
{
   String result = string;
   int L = string.length();
   if( L > 2 && string.charAt(L-2) == '.' && string.charAt(L-1) == 'G' )
     result = string.substring(0, L-2);

   return result;
}

//----------------------------------------------------------------------------
//
// Static method-
//       PlayerData.isGuestName
//
// Purpose-
//       Is there a .G suffix on the nickname?
//
//----------------------------------------------------------------------------
static public boolean               // TRUE iff .G suffix present
   isGuestName(                     // Is there a .G nickname suffix present?
      String             string)    // In this nickname string
{
   boolean result = false;

   int L = string.length();
   if( L > 2 && string.charAt(L-2) == '.' && string.charAt(L-1) == 'G' )
     result= true;
   if( string.equals("guest") )
     result= true;

   return result;
}
} // class PlayerData
