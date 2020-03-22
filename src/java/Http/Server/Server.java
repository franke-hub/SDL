//----------------------------------------------------------------------------
//
//       Copyright (C) 2020 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Server.java
//
// Purpose-
//       Sample server.
//
// Last change date-
//       2020/01/12
//
//----------------------------------------------------------------------------
import java.net.*;

import user.util.*;

public class Server extends Debug {
//----------------------------------------------------------------------------
// Server.Attributes
//----------------------------------------------------------------------------
public ServerSocket    listen;      // Listener Socket
public boolean         operational; // TRUE while operational
public int             port= 8080;  // Listener port

//----------------------------------------------------------------------------
//
// Method-
//       Server.Server
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   Server( )                        // Constructor
{
   operational= true;
}
//----------------------------------------------------------------------------
//
// Method-
//       Server.run
//
// Purpose-
//       Listen for and accept new connections.
//
//----------------------------------------------------------------------------
public void
   run( )
   throws Exception
{
   listen= new ServerSocket(port);  // Create the Listener
   debugln("Listening on port: " + port);

   while( operational ) {
     Socket talk= listen.accept();  // Accept a connection
     if( operational ) {
       ServerThread thread= new ServerThread(this, talk);
       thread.setDaemon(true);
       thread.start();
     }
   }
}
} // Class Server
