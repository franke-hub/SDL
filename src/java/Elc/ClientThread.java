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
//       ClientThread.java
//
// Purpose-
//       Client Thread descriptor.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
import java.lang.*;

//----------------------------------------------------------------------------
//
// Class-
//       ClientThread
//
// Purpose-
//       Client Thread descriptor.
//
//----------------------------------------------------------------------------
class ClientThread extends Thread   // Client Thread descriptor
{
//----------------------------------------------------------------------------
// ClientThread.Attributes
//----------------------------------------------------------------------------
// None defined

//----------------------------------------------------------------------------
// ClientThread.Constructors
//----------------------------------------------------------------------------
public
   ClientThread(                    // Constructor
     String             name)       // The ClientThread name
{
   super(name);
}

//----------------------------------------------------------------------------
// ClientThread::Methods
//----------------------------------------------------------------------------
public void
   run( )                           // Run the Thread
{
   System.out.println("ClientThread.run start");
   try {
     sleep(1000);
   } catch(Exception X) {
     System.err.println("ClientThread.run Exception: " + X);
     X.printStackTrace();
   }
   System.out.println("ClientThread.run exit");
}
}; // class ClientThread

