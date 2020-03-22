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
//       ServerIF.java
//
// Purpose-
//       Sample RMI server interface.
//
//----------------------------------------------------------------------------
import java.io.*;
import java.lang.*;
import java.rmi.*;

public interface ServerIF extends java.rmi.Remote
{
//----------------------------------------------------------------------------
//
// Method-
//       ServerIF.serve
//
// Purpose-
//       Sample RMI method using a remote object.
//
//----------------------------------------------------------------------------
public Object                       // Resultant
   serve(                           // Serve function
     ObjectIF          object)      // Remote Object
   throws Exception;

//----------------------------------------------------------------------------
//
// Method-
//       ServerIF.stop
//
// Purpose-
//       Stop the Server.
//
//----------------------------------------------------------------------------
public void
   stop( )                          // Stop the Server
   throws Exception;
} // Class ServerIF

