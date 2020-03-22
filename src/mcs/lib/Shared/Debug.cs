//----------------------------------------------------------------------------
//
//       Copyright (c) 2019 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Debug.cs
//
// Purpose-
//       Sample debugging control
//
// Last change date-
//       2019/02/15
//
// Usage notes-
//       When any Debug object is created, if( Debug.debug == null), we set
//       Debug.debug to the created object. When set, Debug.debug remains set
//       until closed. The close method is either explicitly called or called
//       indirectly by Dispose(), which is called when a using statement
//       completes.
//
//       Users may choose to invoke the static Debug.WriteLine or Debug.Write
//       methods. These methods always invoke Console.Write with or without
//       a trailing "\n" added to the format string and, if Debug.debug is not
//       null, similarly invoke Debug.debug.writer.Write.
//
//       The Debug.Write and Debug.WriteLine methods ignore the static
//       DEBUGGING attribute.
//
//----------------------------------------------------------------------------
using System;
using System.IO;
using System.Runtime.InteropServices;

namespace Shared {                  // The Shared library namespace
//----------------------------------------------------------------------------
//
// Class-
//        Debug
//
// Purpose-
//        Debugging helper class.
//
/// <summary>
/// The Debug class is helpful in program debugging. It only requires built-in
/// System classes for proper operation.
/// </summary>
//
/// <remarks>
/// When the System.Diagnostics class is also used, the Debug class choice
/// requires a using statement, to wit:
/// <code>
/// using Debug = Shared.Debug;     // Defines the Debug choice
/// using SharedDebug = Shared.Debug; // Defines an alias for Shared.Debug
/// using DiagDebug = System.Diagnostics.Debug; // Defines another alias
/// </code>
/// </remarks>
//
//----------------------------------------------------------------------------
public class Debug : IDisposable {  // Debugging helper class
//----------------------------------------------------------------------------
// Debug.Attributes
//----------------------------------------------------------------------------
protected string       fileName;    // The output filename
protected StreamWriter writer;      // The output file
public TextWriter      Out { get { return (TextWriter)writer; } }

//----------------------------------------------------------------------------
// Debug.Statics
//----------------------------------------------------------------------------
public static bool     DEBUGGING = true; // Debugging control
public static Debug    debug= null; // The shared Debug object

public static void assert(bool cond) // assert( true ) else Exception
{
   if( cond != true ) {
       Console.Out.Flush();
       // Here we don't want to throw the closed writer exception.
       if( debug != null && debug.writer != null )
           debug.writer.Flush();

       throw new Exception("Assertion failed");
   }
}

public static void Flush() // Flush Console.Out and, if present, debug.writer
{
   Console.Out.Flush();
   if( debug != null ) {
       debug.checkWriter();
       debug.writer.Flush();
   }
}

// Write to Console and, if present, debug.writer
public static void Write(string fmt=null, params object[] args)
{
   if( fmt == null ) fmt= "";
   Console.Write(fmt, args);
   if( debug != null ) {
       debug.checkWriter();
       debug.writer.Write(fmt, args);
   }
}

// WriteLine to Console and, if present, debug.writer
public static void WriteLine(string fmt=null, params object[] args)
{
   if( fmt == null ) fmt= "";
   Write(fmt + "\n", args);
}

//----------------------------------------------------------------------------
// Debug.Constructors/Destructor
//----------------------------------------------------------------------------
public Debug() : this("debug.log", true) {} // Default constuctor, append=true

public Debug(string name, bool append= false) // Full constructor
{
// Console.WriteLine("Debug.Debug({0},{1})", name, append);
   fileName= name;
   if( append )
       writer= File.AppendText(fileName);
   else
       writer= File.CreateText(fileName);

   if( debug == null )
     debug= this;
}

~Debug( ) // Destructor, used when Dispose() not invoked
{
// Console.WriteLine("Debug.~Debug");
// Dispose(false);                  // (Never does anything)
}

//----------------------------------------------------------------------------
// Debug.IDisposable implementation
//----------------------------------------------------------------------------
public void Dispose()
{
   Dispose(true);
   GC.SuppressFinalize(this);
}

protected virtual void Dispose(bool disposing)
{
   if( writer == null )
       return;

   if( disposing )
       close();
}

//----------------------------------------------------------------------------
// Debug.Methods
//----------------------------------------------------------------------------
public void checkWriter() // Throw an Exception if writer == null
{
   if( writer == null )
       throw new Exception("Debug[" + fileName + "]: closed");
}

public void close() // Close the log (Unset Debug.debug if appropriate)
{
   checkWriter();
   writer.Close();
   writer= null;

   if( debug == this )
       debug= null;
}

public void flush() // Flush Console.Out and writer
{
   Console.Out.Flush();
   checkWriter();
   writer.Flush();
}

public void log(string fmt=null, params object[] args) // Write only to writer
{
   if( fmt == null ) fmt= "";
   if( DEBUGGING ) {
       checkWriter();
       writer.Write(fmt, args);
   }
}

public void logLine(string fmt=null, params object[] args) // WriteLine only to writer
{
   if( fmt == null ) fmt= "";
   if( DEBUGGING ) {
       log(fmt + "\n", args);
   }
}

// Write to System.Console and log
public void put(string fmt=null, params object[] args)
{
   if( fmt == null ) fmt= "";
   if( DEBUGGING ) {
       Console.Write(fmt, args);
       log(fmt, args);
   }
}

// WriteLine to System.Console and log
public void putLine(string fmt=null, params object[] args)
{
   if( fmt == null ) fmt= "";
   if( DEBUGGING ) {
       put(fmt + "\n", args);
   }
}
} // class Debug
} // namespace Shared
