//-----------------------------------------------------------------------------
//
//       Copyright (c) 2019 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//-----------------------------------------------------------------------------
//
// Title-
//       Options.cs
//
// Purpose-
//       Run-time options.
//
// Last change date-
//       2019/02/15
//
//-----------------------------------------------------------------------------
using System;                       // (Almost always required)
using System.Collections.Generic;   // For List

using NDesk.Options;                // For Options
using Shared;                       // For Debug

namespace Sample {                  // The Sample namespace
//=============================================================================
// Options: (static) Option control
//=============================================================================
static public class Options {
//-----------------------------------------------------------------------------
// Options.Attributes
//-----------------------------------------------------------------------------
static OptionSet opts= new OptionSet() // Initialize the OptionSet
{  { "h|help",       "Show this help message", v => opt_help= (v != null)}
,  { "d|debug=",     "Debugging level", (int v) => level= v}
,  { "test=",        "Activate test=", v => test_opt(v)}

// Testcase controls
,  { "USE_LOCKING",  "Test_threading control", v => USE_LOCKING= (v != null)}
}; // opts= new OptionSet()

static public bool     opt_help= false; // The --help option
static public int      level= 1; // The --level option
static public string[] extra= null; // Extra options

static public bool     test_backtrace= false; // Run backtrace test?
static public bool     test_options= false; // Run options test?
static public bool     USE_LOCKING= false; // Test_threading control

//-----------------------------------------------------------------------------
// Options.Methods
//-----------------------------------------------------------------------------
static public void info( )          // Parameter description message
{
   Debug.WriteLine("Sample {{options}} {{ignored}}");
   Debug.WriteLine("Run a set of tests, some of which are optional");
   Debug.WriteLine("\nOptions:");
   opts.WriteOptionDescriptions(Console.Out);
   if( Debug.debug != null )
       opts.WriteOptionDescriptions(Debug.debug.Out);
}

static public bool parm(string[] args) // Parameter analysis, returns success
{
   List<string> flats= null;
   try {
       flats= opts.Parse(args);     // Parse the arguments
   } catch(OptionException e) {     // If failure
       Debug.WriteLine(e.Message);
       Debug.WriteLine("\nUsage information:\n");
       opt_help= true;
   }

   if( opt_help ) {
       info();
       return false;
   }

   int count= flats.Count;
   extra= new string[count];
   int X= 0;
   foreach(string s in flats) {
       extra[X++]= s;
       if( s[0] == '-' ) {
           opt_help= true;
           Debug.WriteLine("Unknown option: {0}", s);
       }
   }

   if( opt_help ) {
       Debug.WriteLine("\nUsage information:\n");
       info();
       return false;
   }

   return true;
}

static public void test_opt(string s) // Set a test option
{
   Debug.debug.putLine("test_opt({0})", s);

   if( s.ToLower() == "backtrace" )
       test_backtrace= true;
   else if( s.ToLower() == "options" )
       test_options= true;
   else
       throw new OptionException("Unknown test: " + s, "test=");
}
}  // class Options
}  // namespace Sample
