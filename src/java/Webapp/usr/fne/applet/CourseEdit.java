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
//       CourseEdit.java
//
// Purpose-
//       Edit course information.
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

import usr.fne.common.QuotedTokenizer;

//----------------------------------------------------------------------------
//
// Class-
//       CourseEdit
//
// Purpose-
//       Edit course information.
//
//----------------------------------------------------------------------------
public class CourseEdit extends CourseView {
//----------------------------------------------------------------------------
//
// Method-
//       CourseEdit.CourseEdit
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   CourseEdit( )                    // Constructor
{
   super();                         // CourseView constructor
   appletName= "CourseEdit";
}

//----------------------------------------------------------------------------
//
// Method-
//       CourseEdit.actionPerformed
//
// Purpose-
//       Handle POST Button depress event.
//
// Notes-
//       Called from Validator.actionPerformed().
//       This method implements the ActionListener interface.
//
//----------------------------------------------------------------------------
public void
   actionPerformed(ActionEvent e)
{
   debug("actionPerformed()");
   new Output().execute();
   waitUntilDone();
}

//----------------------------------------------------------------------------
//
// Method-
//       CourseEdit.createGUI
//
// Purpose-
//       Create the GUI.
//
// Notes-
//       Invoke only from EDT.
//
//----------------------------------------------------------------------------
protected void
   createGUI()
{
   super.createGUI();

   // Add the PostListener
   postListener= postPanel.getListener();
}

//----------------------------------------------------------------------------
//
// Method-
//       CourseEdit.getAppletInfo
//
// Purpose-
//       Returns information about this Applet.
//
//----------------------------------------------------------------------------
public String getAppletInfo()
{
   return "Title: CourseEdit v1.0, 01 Jan 2008\n"
          + "Author: Frank Eskesen\n"
          + "Display golf course info.";
}

//----------------------------------------------------------------------------
//
// Method-
//       CourseEdit.isUpdateValid
//
// Purpose-
//       Test whether all data are valid.
//
//----------------------------------------------------------------------------
public synchronized boolean         // TRUE iff all data valid
   isUpdateValid( )
{
   debug("isUpdateValid()");
   postPanel.setMessage();

   // Validate the standard fields
   if( !courseShow.isValid(validator) )
     return false;
   if( !courseName.isValid(validator) )
     return false;
   if( !courseHttp.isValid(validator) )
     return false;
   if( !courseHole.isValid(validator) )
     return false;

   if( !courseParM.isValid(validator) )
     return false;
   if( !courseParW.isValid(validator) )
     return false;

   if( !courseHcpM.isValid(validator) )
     return false;
   if( !courseHcpW.isValid(validator) )
     return false;

   // Verify the teebox panels
   try {
     for(int i= 0; i<teebox_info.length; i++)
     {
       if( !teebox_info[i].isValid(validator) )
         return false;
       if( teebox_info[i].isRemoved )
         continue;

       String string= teebox_info[i].panel.getFieldText(HOLE_ID);
       for(int j= i+1; j<teebox_info.length; j++)
       {
         if( teebox_info[j].isRemoved )
           continue;
         if( string.equalsIgnoreCase(teebox_info[j].panel.getFieldText(HOLE_ID)) )
           return validator.invalid("Duplicate teebox name: " + string);
       }
     }
   } catch(NumberFormatException e) {
     return validator.invalid("Invalid yardage");
   } catch(Exception e) {
     e.printStackTrace();
     return validator.invalid("Teebox.isValid: " + e);
   }

   if( !courseLdCp.isValid(validator) )
     return false;

   return true;
}

//----------------------------------------------------------------------------
//
// Method-
//       CourseEdit.outputDel
//
// Purpose-
//       Delete database entries that are never removed.
//
//----------------------------------------------------------------------------
public void
   outputDel( )                     // Delete database entries
{
   dbReady();

   courseFind.remove(client, CMD_COURSE_FIND, courseID);
   courseShow.remove(client, CMD_COURSE_SHOW, courseID);
   courseName.remove(client, CMD_COURSE_NAME, courseID);
   courseHttp.remove(client, CMD_COURSE_HTTP, courseID);
   courseHole.remove(client, CMD_COURSE_HOLE, courseID);
   courseHcpM.remove(client, CMD_COURSE_HDCP, courseID);
   courseParM.remove(client, CMD_COURSE_PARS, courseID);
   courseHcpW.remove(client, CMD_COURSE_HDCP, concat(courseID,"WOMEN"));
   courseParW.remove(client, CMD_COURSE_PARS, concat(courseID,"WOMEN"));

   courseHole.isChanged= !courseHole.isDefault();
   courseHcpM.isChanged= !courseHcpM.isDefault();
   courseParW.isChanged= !courseParW.isDefault();

   for(int i= 0; i<teebox_info.length; i++)
   {
     TeeboxInfo info= teebox_info[i];
     info.remove(client, CMD_COURSE_TBOX, concat(courseID, info.title));
   }

   dbReset();
}

//----------------------------------------------------------------------------
//
// Method-
//       CourseEdit.output
//
// Purpose-
//       Output the data.
//
//----------------------------------------------------------------------------
public void
   output( )                        // Output the data
{
   debug("output()");

   // See if any teeboxes remain
   boolean isEmpty= true;
   for(int i= 0; i<teebox_info.length; i++)
   {
     if( !teebox_info[i].isRemoved )
     {
       isEmpty= false;
       break;
     }
   }
   if( isEmpty )                    // No tee boxes, delete course
   {
     outputDel();
     return;
   }

   dbReady();

   // Standard fields
   courseFind.insert(client, CMD_COURSE_FIND, courseID);
   courseShow.insert(client, CMD_COURSE_SHOW, courseID);
   courseName.insert(client, CMD_COURSE_NAME, courseID);
   courseHttp.insert(client, CMD_COURSE_HTTP, courseID);

   // Course hole number
   courseHole.isRemoved= courseHole.isDefault();
   courseHole.output(client, CMD_COURSE_HOLE, courseID);

   // Course handicap
   courseHcpM.isRemoved= courseHcpM.isDefault();
   courseHcpM.output(client, CMD_COURSE_HDCP, courseID);

   courseHcpW.isRemoved= courseHcpW.equals(courseHcpM);
   courseHcpW.output(client, CMD_COURSE_HDCP, concat(courseID, "WOMEN"));

   // Course par
   courseParM.isRemoved= courseParM.isDefault();
   courseParM.output(client, CMD_COURSE_PARS, courseID);

   courseParW.isRemoved= courseParW.equals(courseParM);
   courseParW.output(client, CMD_COURSE_PARS, concat(courseID, "WOMEN"));

   // Handle teebox changes: Do not remove if duplicate in teeboxVector
   for(int i= 0; i<teeboxVector.size(); i++)
   {
     TeeboxInfo info= teeboxVector.elementAt(i);
     if( info.isRemoved )
     {
       if( info.isPresent )
       {
         boolean isPresent= false;
         for(int j= 0; j<teeboxVector.size(); j++)
         {
           TeeboxInfo jnfo= teeboxVector.elementAt(j);
           if( !jnfo.isRemoved
               && info.title.equalsIgnoreCase(jnfo.title) )
           {
             isPresent= true;
             break;
           }
         }
         if( !isPresent )
           dbRemove(CMD_COURSE_TBOX, concat(courseID,info.title));
       }

       info.isChanged= false;
       info.isPresent= false;
     }
   }

   // Output the remaining entries
   sortTeeboxInfo();
   for(int i= 0; i<teebox_info.length; i++)
   {
     TeeboxInfo info= teebox_info[i];
     info.output(client, CMD_COURSE_TBOX, concat(courseID,info.title));
   }

   // Course longest drive, closest to pin
   courseLdCp.output(client, courseID);

   dbReset();
}

//----------------------------------------------------------------------------
//
// Method-
//       CourseEdit.update
//
// Purpose-
//       Update the data.
//
//----------------------------------------------------------------------------
public void
   update( )                        // Update the data
{
   debug("update()");

   // Standard fields
   courseShow.update();
   courseName.update();
   courseHttp.update();
   courseHole.update();

   courseHcpM.update();
   courseHcpW.update();

   courseParM.update();
   courseParW.update();

   // Teebox data
   for(int i= 0; i<teebox_info.length; i++)
     teebox_info[i].update();

   // Drive/Close data
   courseLdCp.update();
}

//----------------------------------------------------------------------------
//
// Class-
//       CourseEdit.Output
//
// Purpose-
//       Create a background task to write the data.
//
//----------------------------------------------------------------------------
class Output extends SwingWorker<Object, Void>
{
//----------------------------------------------------------------------------
//
// Method-
//       CourseEdit.Output.doInBackground
//
// Purpose-
//       Write the data.
//
// Notes-
//       Called by SwingWorker via execute().
//
//----------------------------------------------------------------------------
@Override
public Object
   doInBackground()
{
   debug("Output.doInBackground()");

   synchronized(owner)
   {{{{
     update();
     output();
   }}}} // Synchronized(owner)

   return new Object();
}

//----------------------------------------------------------------------------
//
// Method-
//       CourseEdit.Output.done
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
   debug("Output.done()");
   new Format().execute();
}
} // class CourseEdit.Output
} // class CourseEdit
