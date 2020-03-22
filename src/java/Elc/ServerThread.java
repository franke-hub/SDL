//----------------------------------------------------------------------------
//
//       Copyright (C) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       ServerThread.java
//
// Purpose-
//       Server Thread descriptor.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
import java.lang.*;

//----------------------------------------------------------------------------
//
// Class-
//       ServerThread
//
// Purpose-
//       Server Thread descriptor.
//
//----------------------------------------------------------------------------
class ServerThread extends Thread   // Server Thread descriptor
{
//----------------------------------------------------------------------------
// ServerThread.Attributes
//----------------------------------------------------------------------------
// None defined

//----------------------------------------------------------------------------
// ServerThread.Constructors
//----------------------------------------------------------------------------
public
   ServerThread(                    // Constructor
     String             name)       // The ServerThread name
{
   super(name);
}

//----------------------------------------------------------------------------
// ServerThread::Methods
//----------------------------------------------------------------------------
public void
   run( )                           // Run the Thread
{
   System.out.println("ServerThread.run start");
   try {
     sleep(1000);
   } catch(Exception X) {
     System.err.println("ServerThread.run Exception: " + X);
     X.printStackTrace();
   }
   System.out.println("ServerThread.run exit");
}
}; // class ServerThread

