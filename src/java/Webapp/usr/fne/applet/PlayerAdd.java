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
//       PlayerAdd.java
//
// Purpose-
//       Add player information to database.
//
// Last change date-
//       2020/01/15
//
//----------------------------------------------------------------------------
import java.awt.*;
import java.awt.event.*;
import java.util.*;
import javax.swing.*;

import usr.fne.common.QuotedTokenizer;

//----------------------------------------------------------------------------
//
// Class-
//       PlayerAdd
//
// Purpose-
//       Add player information to database.
//
//----------------------------------------------------------------------------
public class PlayerAdd extends PlayerEdit {
//----------------------------------------------------------------------------
//
// Method-
//       PlayerAdd.PlayerAdd
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   PlayerAdd( )                     // Constructor
{
   super();
   appletName= "PlayerAdd";
}

//----------------------------------------------------------------------------
//
// Method-
//       PlayerAdd.getAppletInfo
//
// Purpose-
//       Returns information about this Applet.
//
//----------------------------------------------------------------------------
public String getAppletInfo()
{
   return "Title: PlayerAdd v1.0, 01 Jan 2008\n"
          + "Author: Frank Eskesen\n"
          + "Add player to database.";
}

//----------------------------------------------------------------------------
//
// Method-
//       PlayerAdd.getParameterInfo
//
// Purpose-
//       Returns information about this Applet's parameters.
//
//----------------------------------------------------------------------------
public String[][]
   getParameterInfo()
{
   return null;
}

//----------------------------------------------------------------------------
//
// Method-
//       PlayerAdd.init
//
// Purpose-
//       Initialize.
//
// Notes-
//       Called when this Applet is loaded into the browser.
//
//----------------------------------------------------------------------------
public void
   init()
{
   initCommon();
   new Worker().execute();
}

//----------------------------------------------------------------------------
//
// Class-
//       PlayerAdd.Worker
//
// Purpose-
//       Create a background task to add the data.
//
//----------------------------------------------------------------------------
class Worker extends SwingWorker<Object, Void>
{
//----------------------------------------------------------------------------
//
// Method-
//       PlayerAdd.Worker.doInBackground
//
// Purpose-
//       Load the data.
//
// Notes-
//       Called by SwingWorker via execute().
//
//----------------------------------------------------------------------------
@Override
public Object
   doInBackground()
{
   debug("Worker.doInBackground()");

   dbReady();

   // Load the data
   try {
     if( client.fsm != DbStatic.FSM_READY )
       throw new Exception("DbServer offline");

     // Find free playerID
     int last= (-1);
     String CMD=  CMD_PLAYER_FIND;
     String NEXT= "";
     for(;;)
     {
       String text= dbNext(CMD, NEXT);
       if( text == null )
         break;

       QuotedTokenizer t= new QuotedTokenizer(text);
       String type= t.nextToken();
       String item= t.nextToken();

       if( !type.equals(CMD) )
         break;

       try {
         if( !NEXT.equals("") )
         {
           last= Integer.parseInt(NEXT.substring(1));
           break;
         }
       } catch(Exception e) {
         debug("item(" + item + "): " + e);
       }

       NEXT= item;
     }

     if( last == (-1) )
       last= 0;
     else
     {
       for(;;)
       {
         String text= dbNext(CMD, NEXT);
         if( text == null )
           break;

         QuotedTokenizer t= new QuotedTokenizer(text);
         String type= t.nextToken();
         String item= t.nextToken();

         if( !type.equals(CMD) )
           break;

         try {
           int next= Integer.parseInt(item.substring(1));
           if( next != (last+1) )
             break;
           last= next;
         } catch(Exception e) {
           debug("item(" + item + "): " + e);
           break;
         }

         NEXT= item;
       }
     }
     playerID= "" + (last+1);
     while( playerID.length() < 4 )
       playerID= "0" + playerID;
     playerID= "P" + playerID;
     playerNN= "nickn";

     playerFind= new GenericListInfo("PlayerID", playerID);
     playerNick= new GenericListInfo("Database nick", playerNN);
     playerFind.isPresent= playerNick.isPresent= false;
     playerShow= new GenericListInfo("Display name", "Nick N");
     playerShow.isQuoted= true;
     playerName= new GenericNameInfo();
     playerMail= new GenericListInfo("E-mail", "nobody@noemail");
     removeNick= new GenericListInfo("UNUSED", playerNN);
     removeNick.isPresent= false;
     playerShow.isPresent= playerName.isPresent= playerMail.isPresent= false;
   } catch( Exception e ) {
     setERROR("Worker: Client exception: " + e);
     e.printStackTrace();
   }

   dbReset();

   return new Object();
}

//----------------------------------------------------------------------------
//
// Method-
//       PlayerAdd.Worker.done
//
// Purpose-
//       Complete the image load.
//
// Notes-
//       Called by SwingWorker in EWT after doInBackground completes.
//
//----------------------------------------------------------------------------
@Override
public void done()
{
   debug("Worker.done()");

   // If error, put message in the loaderPanel
   if( errorString != null )
   {
     loaderPanel.getField(0).setText(errorString);
     content.revalidate();
   }
   else
   {
     Format format= new Format();
     format.execute();
   }
}
} // class PlayerAdd.Worker
} // class PlayerAdd
