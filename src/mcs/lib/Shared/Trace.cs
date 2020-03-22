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
//       Trace.cs
//
// Purpose-
//       In-memory trace control
//
// Last change date-
//       2019/02/15
//
//----------------------------------------------------------------------------
using System;
using System.IO;

namespace Shared {                  // The Shared library namespace
//----------------------------------------------------------------------------
//
// Class-
//        TextWrapper
//
// Purpose-
//        Defer text formatting.
//
//----------------------------------------------------------------------------
internal class TextWrapper {        // Deferred text formatting
static  object[]       empty= new object[0];

string                 form;
Object[]               data;

public TextWrapper(string form= null, object[] data= null) {
   this.form= form;
   this.data= data;
}

public override string ToString() {
   string   form= this.form;
   if( form == null ) form= "";

   object[] data= this.data;
   if( data == null ) data= empty;

   return String.Format(form, data);
}
}  // class TextWrapper

//----------------------------------------------------------------------------
//
// Static class-
//        Trace
//
// Purpose-
//        Memory trace class.
//
//----------------------------------------------------------------------------
public static class Trace {         // Memory trace
//----------------------------------------------------------------------------
// Trace.Attributes
//----------------------------------------------------------------------------
static Object          mutex= new Object(); // Table mutex
static uint            next;        // The next trace table entry
static bool            ready= false; // Operational state
static public bool     Ready { get { return ready; } }
static ulong           wrap_count;  // Wrap counter

static object[]        table;       // The trace table

//----------------------------------------------------------------------------
// Trace.Methods
//----------------------------------------------------------------------------
public static void dump( )          // Dump the trace table
{  lock(mutex) {

   if( table == null )
       return;

   Debug old_debug= Debug.debug;
   Debug debug= old_debug;
   if( old_debug == null )
       debug= new Debug();
   bool debugging= Debug.DEBUGGING;
   Debug.DEBUGGING= true;

   debug.logLine("==================================================");
   debug.logLine(">>>>>>> {0:0.000} Trace.dump", Utility.tod());
   debug.logLine("[{0,5}] Wrap count\n", wrap_count);

   if( table[next] != null ) {
       for(uint i= next; i<table.Length; i++)
           debug.logLine("[{0,5}] {1}", i, table[i]);
   }

   for(uint i= 0; i<next; i++)
       debug.logLine("[{0,5}] {1}", i, table[i]);

   Debug.DEBUGGING= debugging;

   if( old_debug == null )
       debug.close();
}  }

public static void reset(uint count) // (Unconditionally) reset the trace table
{  lock(mutex) {

   next= 0;
   table= new object[count];
   ready= true;
   if( count == 0 )
       ready= false;
}  }

public static void start( )         // Start tracing
{  lock(mutex) {

   if( table == null )
       reset(65536);

   ready= true;
}  }

public static void stop( )          // Stop tracing
{  lock(mutex) {

   ready= false;
}  }

internal class TimeWrapper {        // Add time to trace record
double                 time;
Object                 text;

public TimeWrapper(Object text) {
   this.time= Utility.tod();
   this.text= text;
}

public override string ToString() {
   return String.Format("{0:0.000} {1}", time, text);
}
}  // class TimeWrapper

public static void trace(object O)  // Insert trace record
{
   // Time is always added to the record
   O= new TimeWrapper(O);

   lock(mutex) {

   if( ready ) {
       if( next >= table.Length ) {
           wrap_count++;
           next= 0;
       }

       table[next++]= O;
   }
}  }

public static Object wrap(string form, params object[] data)
{
   return new TextWrapper(form, data);
}
} // class Trace
} // namespace Shared
