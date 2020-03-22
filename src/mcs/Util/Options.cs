//----------------------------------------------------------------------------
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
//       Default/sample options file.
//
// Last change date-
//       2019/02/15
//
// Implementation notes-
//       This is the base for all file scan operations.
//       (Maybe it can go into a library.)
//
//-----------------------------------------------------------------------------
using System;
using System.Collections.Generic;   // For List
using System.IO;                    // For GetFiles

using Shared;                       // For Debug
using NDesk.Options;                // For Options

namespace Library {                 // The Library namespace
//-----------------------------------------------------------------------------
//
// Class-
//        Options
//
// Purpose-
//        Parameter analysis
//
//-----------------------------------------------------------------------------
static public class Options {
//-----------------------------------------------------------------------------
// Options.Attributes
//-----------------------------------------------------------------------------
static public string   SOURCE= "Filename"; // The name of the source file
static public string   DOWHAT= "Recursively list file names"; // Function desc

static OptionSet opts= new OptionSet() // Initialize the OptionSet
{  { "h|help",       "Show this help message", v => help= (v != null)}
,  { "d|debug=",     "Debugging level(=1)", (int v) => level= v}
,  { "f|find=",      "Find file=", v => findList.Add(v)}
,  { "s|skip=",      "Skip file=", v => skipList.Add(v)}
,  { "show",         "Show options", v => opt_show= (v != null)}
}; // opts= new OptionSet()

static public bool     help= false; // The --help option
static public int      level= 1;    // The --level option
static public bool     opt_show= false; // The --show option
static public string[] extras= null; // Extra options
static public List<string> findList= new List<string>();
static public List<string> skipList= new List<string>();

//-----------------------------------------------------------------------------
// Options.Methods
//-----------------------------------------------------------------------------
static public void info( )          // Parameter description message
{
   Debug.WriteLine("{0} {{options}} {{base-directory}}", SOURCE);
   Debug.WriteLine("{0}", DOWHAT);
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
       help= true;
   }

   if( help ) {
       if( opt_show )               // -show with -help (or Exception)
           show();

       info();
       return false;
   }

   extras= new string[flats.Count];
   int X= 0;
   foreach(string s in flats) {
       extras[X++]= s;
       if( X > 1 ) {
           help= true;
           Debug.WriteLine("Unexpected parameter: {0}", s);
       }
   }

   if( opt_show )                   // -show (with extras)
       show();

   if( help ) {
       Debug.WriteLine("\nUsage information:\n");
       info();
       return false;
   }

   return true;
}

static public void show( )          // Display options (testing)
{
   Debug.WriteLine("Options:");
   Debug.WriteLine("  level({0})", level);
   Debug.WriteLine("  [{0,2}] extras", extras.Length);
   int X= 0;
   foreach(string s in extras)
       Debug.WriteLine("  [{0,2}] '{1}'", X++, s);
   if( X > 0 ) Debug.WriteLine();

   Debug.WriteLine("  [{0,2}] finds", findList.Count);
   X= 0;
   foreach(string s in findList)
       Debug.WriteLine("  [{0,2}] '{1}'", X++, s);
   if( X > 0 ) Debug.WriteLine();

   Debug.WriteLine("  [{0,2}] skips", skipList.Count);
   X= 0;
   foreach(string s in skipList)
       Debug.WriteLine("  [{0,2}] '{1}'", X++, s);
   if( X > 0 ) Debug.WriteLine();
}
}} // } class Options } namespace Library
