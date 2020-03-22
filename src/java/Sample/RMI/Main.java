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
//       Main.java
//
// Purpose-
//       Sample client or server.
//
//----------------------------------------------------------------------------
import java.io.*;
import java.lang.*;
import java.rmi.*;
import java.rmi.registry.*;
import java.rmi.server.*;
import java.security.*;

public class Main
{
//----------------------------------------------------------------------------
// Main.Attributes
//
// Working configuration-
//       server DIR=     . or ./server
//       client DIR=     . or ./client
//       bindObject=     *
//       createRegistry= true
//       createRegistry= false && rmiregistry has server files in CLASSPATH
//       echoArgs=       *
//       systemExit=     *
//       anySecurityMgr= true
//       rmiSecurityMgr= false
//       rmiSecurityMgr= true  && -Djava.security.policy=./policy
//       -Djava.rmi.server.codebase=file://$(PWD)/
//----------------------------------------------------------------------------
static boolean         bindObject=     false;  // Binding ObjectOB?
static boolean         createRegistry= true;   // Local Registry?
static boolean         echoArgs=       false;  // Echo arguments?
static boolean         systemExit=     false;  // System.exit() at end?

static boolean         anySecurityMgr= true;   // Set security manager?
static boolean         rmiSecurityMgr= true;   // Set RMI security manager?

static String          objectName= "ObjectOB"; // Object name
static String          serverName= "ServerOB"; // Server name

//----------------------------------------------------------------------------
//
// Method-
//       Main.info
//
// Purpose-
//       Informational exit.
//
//----------------------------------------------------------------------------
public static void
   info( )
   throws Exception
{
   System.out.println("Need one argument: 'client' or 'server'");
   throw new Exception("Parameter error");
}

//----------------------------------------------------------------------------
//
// Method-
//       Main.config
//
// Purpose-
//       Display configuration.
//
//----------------------------------------------------------------------------
public static void
   config( )
   throws Exception
{
   System.out.println("bindObject:     " + bindObject);
   System.out.println("createRegistry: " + createRegistry);
   System.out.println("echoArgs:       " + echoArgs);
   System.out.println("systemExit:     " + systemExit);

   if( anySecurityMgr == false )
     System.out.println("securityMgr:    NONE");
   else if( rmiSecurityMgr )
     System.out.println("securityMgr:    RMISecurityManager");
   else
     System.out.println("securityMgr:    NullSecurityManager");
}

//----------------------------------------------------------------------------
//
// Method-
//       Main.client
//
// Purpose-
//       Client.
//
//----------------------------------------------------------------------------
public static void
   client( )
   throws Exception
{
   ObjectIF            object= new ObjectOB();
   ServerIF            server= (ServerIF)java.rmi.Naming.lookup("ServerOB");
   Object              result;

   System.out.println("Main.client");
   if( bindObject )
   {
     try {
       Naming.bind(objectName, object);
     } catch( Exception e ) {
       System.out.println("Main.client Exception: " + e.toString());
     }
   }

   result= server.serve(object);
   System.out.println("Main.client: result: '" +
                      result.toString() + "'");

   result= server.serve(object);
   System.out.println("Main.client: result: '" +
                      result.toString() + "'");

   System.out.println("Main.client: object: '" +
                      ((ObjectOB)object).method("Main.client").toString() +
                      "'");

   server.stop();

   if( bindObject )
   {
     try {
       Naming.unbind(objectName);
     } catch( Exception e ) {
       System.out.println("Main.client Exception: " + e.toString());
     }
   }
   System.out.println("Main.client complete");
}

//----------------------------------------------------------------------------
//
// Method-
//       Main.createregistry
//
// Purpose-
//       Create a registry
//       A Registry object is needed only if you aren't running rmiregistry
//
//----------------------------------------------------------------------------
public static Registry
   createRegistry( )
   throws Exception
{
   Registry            registry= null;

   try {
     if( createRegistry )
       registry= LocateRegistry.createRegistry(1099);
   } catch( Exception e ) {
     System.out.println("Main.createRegistry Exception: " + e.toString());
   }

   return registry;
}

//----------------------------------------------------------------------------
//
// Method-
//       Main.server
//
// Purpose-
//       Server.
//
//----------------------------------------------------------------------------
public static void
   server( )
   throws Exception
{
   boolean             bound;
   Registry            registry;
   ServerOB            server= new ServerOB();

   System.out.println("Main.server");
   registry= createRegistry();
   bound= false;
   try {
     Naming.bind(serverName, server);
     bound= true;
   } catch( Exception e ) {
     System.out.println("Main.server Exception: " + e.toString());
     try {
       Naming.rebind(serverName, server);
       bound= true;
     } catch( Exception e1 ) {
       System.out.println("Main.server Exception: " + e1.toString());
     }
   }

   if( bound )
     server.run();

   try {
     Naming.unbind(serverName);
   } catch( Exception e ) {
     System.out.println("Main.server Exception: " + e.toString());
   }
   System.out.println("Main.server complete");
}

//----------------------------------------------------------------------------
//
// Method-
//       Main.echo
//
// Purpose-
//       Echo the argument array.
//
//----------------------------------------------------------------------------
public static void
   echo(                            // Mainline code
     String[]          args)        // Argument array
{
   int                 i;

   System.out.println("Main.echo: " + args.length + " arguments");
   for(i= 0; i<args.length; i++)
     System.out.println("'" + args[i] + "'");
}

//----------------------------------------------------------------------------
//
// Method-
//       Main.main
//
// Purpose-
//       Mainline code.
//
//----------------------------------------------------------------------------
public static void
   main(                            // Mainline code
     String[]          args)        // Argument array
   throws Exception
{
   if( echoArgs )
     echo(args);

   if( args.length == 0 )
     info();

   if( anySecurityMgr )
   {
     if( System.getSecurityManager() == null )
     {
       if( rmiSecurityMgr )
         System.setSecurityManager(new RMISecurityManager());
       else
         System.setSecurityManager(new NullSecurityManager());
     }
   }

   if( args[0].equals("client") )
     client();

   else if( args[0].equals("server") )
     server();

   else if( args[0].equals("config") )
     config();

   else
     info();

   System.out.println("Main complete");
   System.gc();
   System.runFinalization();
   System.gc();
   if( systemExit )
     System.exit(0);
}
} // Class Main

class NullSecurityManager extends SecurityManager
{
public
   NullSecurityManager() { }

public void
   checkPermission(Permission p) throws SecurityException { }
} // Class NullSecurityManager

