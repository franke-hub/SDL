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
//       ServerOB.java
//
// Purpose-
//       Sample RMI server object.
//
//----------------------------------------------------------------------------
import java.io.*;
import java.lang.*;
import java.rmi.*;
import java.rmi.server.*;

public class ServerOB extends    java.rmi.server.UnicastRemoteObject
                      implements ServerIF
{
//----------------------------------------------------------------------------
// ServerOB.Attributes
//----------------------------------------------------------------------------
boolean                operational; // TRUE while operational
int                    callcount;

//----------------------------------------------------------------------------
// ServerOB.Constructors
//----------------------------------------------------------------------------
public
   ServerOB( )
   throws Exception
{
   super();
   callcount= 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       ServerOB.serve
//
// Purpose-
//       Sample RMI method using a remote object.
//
//----------------------------------------------------------------------------
public Object                       // Resultant
   serve(                           // Serve function
     ObjectIF          object)      // Remote Object
   throws Exception
{
   Object              result;

   System.out.println("ServerOB.serve");
   result= object.method("Server parameter");
   System.out.println("ServerOB.serve: Object.method result: '" +
                      result.toString() + "'");

   result= object.method(object);
   System.out.println("ServerOB.serve: Object.method result: '" +
                      result.toString() + "'");

   callcount++;
   return new String("Server resultant " + callcount);
}

//----------------------------------------------------------------------------
//
// Method-
//       ServerOB.stop
//
// Purpose-
//       Stop the Server.
//
//----------------------------------------------------------------------------
public void
   stop( )                          // Stop the Server
   throws Exception
{
   System.out.println("ServerOB.stop");
   operational= false;
}

//----------------------------------------------------------------------------
//
// Method-
//       ServerOB.run
//
// Purpose-
//       Operational loop.
//
//----------------------------------------------------------------------------
public void
   run( )                           // Operate the Server
   throws Exception
{
   System.out.println("ServerOB.run");
   callcount= 0;
   operational= true;
   while( operational )
     Thread.currentThread().sleep(1000);

   Thread.currentThread().sleep(1000);
}
} // Class ServerOB

