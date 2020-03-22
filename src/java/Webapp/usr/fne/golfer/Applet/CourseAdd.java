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
//       CourseAdd.java
//
// Purpose-
//       Add course information to database.
//
// Last change date-
//       2020/01/15
//
//----------------------------------------------------------------------------
import java.awt.*;
import java.awt.event.*;
import java.lang.*;
import java.util.*;
import javax.swing.*;

// import usr.fne.common.SwingWorker;
import usr.fne.common.QuotedTokenizer;

//----------------------------------------------------------------------------
//
// Class-
//       CourseAdd
//
// Purpose-
//       Add course information to database.
//
//----------------------------------------------------------------------------
public class CourseAdd extends CourseEdit {
//----------------------------------------------------------------------------
//
// Method-
//       CourseAdd.CourseAdd
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   CourseAdd( )                     // Constructor
{
   super();                         // CourseEdit()
   appletName= "CourseAdd";
}

//----------------------------------------------------------------------------
//
// Class-
//       CourseAdd.Worker
//
// Purpose-
//       Generate new course information.
//
//----------------------------------------------------------------------------
class Worker extends SwingWorker<Object, Void>
{
//----------------------------------------------------------------------------
//
// Method-
//       CourseAdd.Worker.doInBackground
//
// Purpose-
//       Initialize the data.
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

   // Generate course information
   try {
     if( client.fsm != DbStatic.FSM_READY )
       throw new Exception("DbServer offline");

     // Find free courseID
     String CMD=  CMD_COURSE_FIND;
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
         int last= 0;
         if( !NEXT.equals("") )
           last= Integer.parseInt(NEXT.substring(1));
         int next= Integer.parseInt(item.substring(1));

         if( next != (last+1) )
           break;
       } catch(Exception e) {
         debug("item(" + item + "): " + e);
       }

       NEXT= item;
     }

     int last= 0;
     if( !NEXT.equals("") )
       last= Integer.parseInt(NEXT.substring(1));
     courseNN= "" + (last+1);
     while( courseNN.length() < 4 )
       courseNN= "0" + courseNN;
     courseID= courseNN= "C" + courseNN;

     // Generate database items that are never removed
     courseFind= new GenericFindInfo(courseID); courseFind.isPresent= false;
     courseShow= new GenericItemInfo();
     courseName= new GenericNameInfo();
     courseHttp= new GenericItemInfo("http://newcourse.net");
     courseHttp.isPresent= false;
     courseHole= new CourseHoleInfo(HOLE_COUNT);
     courseHcpM= new CourseHdcpInfo(HOLE_COUNT);
     courseHcpW= new CourseHdcpInfo(HOLE_COUNT);
     courseLong= new CourseLongInfo(HOLE_COUNT);
     courseNear= new CourseNearInfo(HOLE_COUNT);
     courseLdCp= new CourseLdCpInfo(HOLE_COUNT, courseLong, courseNear);
     courseParM= new CourseParsInfo(HOLE_COUNT);
     courseParW= new CourseParsInfo(HOLE_COUNT);

     // Generate initial teebox list
     teeboxVector= new Vector<TeeboxInfo>();
     teeboxVector.add(new TeeboxInfo());
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
//       CourseAdd.Worker.done
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
     new Format().execute();
}
} // class CourseAdd.Worker

//----------------------------------------------------------------------------
//
// Method-
//       CourseAdd.getAppletInfo
//
// Purpose-
//       Returns information about this Applet.
//
//----------------------------------------------------------------------------
public String getAppletInfo()
{
   return "Title: CourseAdd v1.0, 01 Jan 2008\n"
          + "Author: Frank Eskesen\n"
          + "Display golf course info.";
}

//----------------------------------------------------------------------------
//
// Method-
//       CourseAdd.getParameterInfo
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
//       CourseAdd.init
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
} // class CourseAdd
