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
//       Filename.cs
//
// Purpose-
//       Recursively find names of files.
//
// Last change date-
//       2019/02/15
//
// TODO: Open problems-
//       Does not work with parameter "C:\"
//
// Implementation notes-
//       This is the base for all file scan operations.
//       TODO: Maybe it can go into a library.
//
//-----------------------------------------------------------------------------
#undef  USE_DEBUG                   // If defined, adds debug.log output

using System;
using System.Collections.Generic;   // For List
using System.IO;                    // For GetFiles
using System.Security;              // For PermissionSet
using System.Security.Permissions;  // For PermissionState

using Shared;                       // For Debug
using Library;                      // For Options (in local Library)

namespace Program {                 // The Program namespace
//-----------------------------------------------------------------------------
//
// Class-
//        Fileinfo
//
// Purpose-
//        Contains all interesting information about a file or directory
//
// Implementation notes-
//        The constructor does not completely initialize Fileinfo data.
//        Use the update() method to set finish initialization.
//
//-----------------------------------------------------------------------------
class Fileinfo {
//-----------------------------------------------------------------------------
// Fileinfo.Utility methods
//-----------------------------------------------------------------------------
public string humanify(ulong size) {
   return Utility.humanify(size);
}

public string unixify(string name) {
   return Utility.unixify(name);
}

//-----------------------------------------------------------------------------
// Fileinfo.Constants
//-----------------------------------------------------------------------------
public const  ulong    Exists= 0x0000000100000000; // ExtendedAttribute
public const  ulong    Usable= 0x0000000200000000; // ExtendedAttribute
public static string   Separator= Path.DirectorySeparatorChar.ToString();

//-----------------------------------------------------------------------------
// Fileinfo.Attributes and Properties
//-----------------------------------------------------------------------------
public Fileinfo        parent;      // The parent Fileinfo

public ulong           attributes;  // The FileAttributes + ExtendedAttributes
public string          fileName;    // The file name (within subdirectory)
public ulong           fileSize;    // The file size  and ExtendedAttributes
public DateTime        fileTime;    // The UTC last write time

// Algorithmic Properties
public bool            is_directory {
   get { return (attributes&(ulong)FileAttributes.Directory) != 0; }
}

public bool            is_extant {
   get { return (attributes&Exists) != 0; }
}

public bool            is_system {
   get { return (attributes&(ulong)FileAttributes.System) != 0; }
}

public bool            is_usable {
   get { return (attributes&Usable) != 0; }
}

public string          PathName {   // The relative subdirectory name
   get {
       if( parent == null )
           return ".";

       return parent.FullName;
   }
}

public string          FullName {   // The file name (PathName + fileName)
   get {
       if( parent == null )
           return fileName;

       return PathName + Separator + fileName;
   }
}

//-----------------------------------------------------------------------------
// Fileinfo.Constructors
//-----------------------------------------------------------------------------
public Fileinfo( ) : this(null, null) {} // Default constructor
public Fileinfo(string fileName) : this(null, fileName) {} // fileName constructor

public Fileinfo(                    // Full constructor
       Fileinfo        parent,      // Parent (May be null)
       string          fileName)    // The file or directory name
{
   this.parent= parent;
   this.fileName= fileName;
}

public Fileinfo(Fileinfo source)    // Copy constructor
{
   this.parent= source.parent;
   this.attributes= source.attributes;
   this.fileName= source.fileName;
   this.fileSize= source.fileSize;
   this.fileTime= source.fileTime;
}

public void debug()                 // Debugging display
{
   Debug.WriteLine("Fileinfo.debug");
   Debug.WriteLine("Parent({0})", Utility.nullify(parent));
   Debug.WriteLine("Attributes(0x{0:X10})", attributes);
   Debug.WriteLine("FileName({0})", fileName);
   Debug.WriteLine("FileSize({0})", humanify(fileSize));
   Debug.WriteLine("FileTime({0})", fileTime);

   Debug.WriteLine("PathName({0})", PathName);
   Debug.WriteLine("FullName({0})", FullName);
   Debug.WriteLine("Separator({0})", Separator);
}

public override string ToString( )  // Formatter
{
   string state= "U";               // Unusable
   if( is_usable ) {
       state= "F";
       if( is_system )
           state= "S";
       else if( is_directory )
           state= "D";
   }

   return String.Format("{0} {2} {3}", state, attributes,
                        humanify(fileSize), unixify(FullName));
// return String.Format("{0} 0x{1:X10} {2} {3}", state, attributes,
//                      humanify(fileSize), unixify(FullName));
}

//-----------------------------------------------------------------------------
// Fileinfo.Methods
//-----------------------------------------------------------------------------
public void update( ) {             // Update the attributes
   try {
       attributes= 0;               // Initialize
       fileSize= 0;

       string path= Path.GetFullPath(FullName); // Evaluate FullName once
       if( File.Exists(path) ) {    // If the file exists
           attributes= (ulong)File.GetAttributes(path);
           attributes |= Exists;

           FileInfo info= new FileInfo(path);
           fileSize= (ulong)info.Length;
           fileTime= File.GetLastWriteTimeUtc(path);

           attributes |= Usable;    // This Fileinfo is usable
       } else
       if( Directory.Exists(path) ) { // If the directory exists
           DirectoryInfo info= new DirectoryInfo(path);
           attributes= (ulong)info.Attributes;
           attributes |= Exists;

#if false  // Since this doesn't work...
           fileTime= Directory.GetLastWriteTimeUtc(path);
           var wantPermit= new PermissionSet(PermissionState.None);
           var needRead=
               new FileIOPermission(FileIOPermissionAccess.Read, path);
           wantPermit.AddPermission(needRead);
           if( wantPermit.IsSubsetOf(AppDomain.CurrentDomain.PermissionSet) )
               attributes |= Usable; // The Directory is usable
           else
               Debug.WriteLine("Unusable({0})", path);

#else      // ...We'll just handle the permissions Exception and move on
           attributes |= Usable;    // The Directory is usable
#endif
       }
   } catch( Exception e ) {
       string s= String.Format("Exception: " + e.Message);
       Debug.WriteLine(s);
       Debug.WriteLine(e.StackTrace);
       Debug.Flush();
   }
}
}  // class Fileinfo

//-----------------------------------------------------------------------------
//
// Class-
//        Program
//
// Purpose-
//        Attribute and method holder, including Main
//
//-----------------------------------------------------------------------------
class Program {
//-----------------------------------------------------------------------------
// Program.Attributes
//-----------------------------------------------------------------------------
public static bool     USE_EMIT_DIR= false; // To emit directory names
public static bool     USE_GOODLIST= false; // To keep list for later
const string           PROGRAM= "Filename"; // This program's name
List<Fileinfo> goodList= new List<Fileinfo>(); // The full list of good files

//-----------------------------------------------------------------------------
// Program.Utility methods
//-----------------------------------------------------------------------------
public bool is_level(int level) {
   return Options.level >= level;
}

public bool wildmatch(string wild, string text) {
   return Utility.wildmatch(wild, text);
}

//-----------------------------------------------------------------------------
//
// Method-
//        Program.do_something
//
// Purpose-
//        Handle a file
//
//-----------------------------------------------------------------------------
public void do_something(Fileinfo info)
{
   Debug.WriteLine("{0}", info);
}

//-----------------------------------------------------------------------------
//
// Method-
//        Program.pull
//
// Purpose-
//        Process the list of files
//
//-----------------------------------------------------------------------------
public void pull()
{
// foreach(Fileinfo good in goodList ) {
//     do_something(good);
// }
}

//-----------------------------------------------------------------------------
//
// Method-
//        Program.push
//
// Purpose-
//        Scan a subdirectory (recursively)
//
//-----------------------------------------------------------------------------
public void push(Fileinfo path)
{
   if( is_level(3) ) Debug.WriteLine("push({0})", path.FullName);
   if( ! path.is_usable )
       return;

   // Ignoring finds and skips for now
   string pathName= path.FullName;
   int L= pathName.Length + 1;

   if( is_level(3) )
       Debug.WriteLine("Entering path({0}) from ({1})", pathName, path.PathName);

   try {
       string[] items= Directory.GetFiles(pathName);
       foreach(string item in items) {
           string name= item.Substring(L);

           bool keeper= false;
           foreach(string find in Options.findList) {
               if( wildmatch(find, name) ) {
//                 Debug.WriteLine("Found({0})", name);
                   keeper= true;
                   break;
               }
           }

           if( keeper ) {
               foreach(string skip in Options.skipList) {
                   if( wildmatch(skip, name) ) {
                       Debug.WriteLine("Skipped({0})", name);
                       keeper= false;
                       break;
                   }
               }

               if( keeper ) {
                   Fileinfo info= new Fileinfo(path, name);
                   info.update();
                   if( USE_GOODLIST ) goodList.Add(info);
                   do_something(info);
               }
           }
       }

       items= Directory.GetDirectories(pathName);
       foreach(string item in items) {
           string name= item.Substring(L);

           Fileinfo info= new Fileinfo(path, name);
           info.update();

           if( info.is_usable ) {
               if( USE_EMIT_DIR ) { // Emit directory names?
                   if( USE_GOODLIST ) goodList.Add(info);
                   do_something(info);
               }
               push(info);
           }
       }
   } catch( Exception e ) {
       if( is_level(2) ) Debug.WriteLine("Exception: {0}", e.Message);
       path.attributes &= ~(Fileinfo.Usable);
   }

   if( is_level(3) )
       Debug.WriteLine("Exiting path({0}) into ({1})", pathName, path.PathName);
}

//-----------------------------------------------------------------------------
//
// Method-
//        Main
//
// Purpose-
//        Mainline code
//
//-----------------------------------------------------------------------------
static void Main(string[] args)
{
#if USE_DEBUG
   Debug debug= new Debug("debug.log");
#endif

   try {
       Options.SOURCE= PROGRAM;
       Options.DOWHAT= "Recursively list file names";

       if( Options.parm(args) ) {
           Debug.WriteLine("{0}: started...", PROGRAM);

           if( Options.findList.Count == 0 ) // If the findList is empty
               Options.findList.Add("*");   // Select all files

           Program program= new Program();
           string path= ".";
           if( Options.extras.Length != 0 )
               path= Options.extras[0];

           Fileinfo info= new Fileinfo(path);
           info.update();
           if( USE_GOODLIST ) program.goodList.Add(info);
           program.push(info);
           program.pull();

           Debug.WriteLine("Memory used: {0}",
                           Utility.humanify((ulong)GC.GetTotalMemory(false)));
           Debug.WriteLine("{0}: ...complete", PROGRAM);
       }
   } catch( Exception e) {
       Debug.WriteLine(e.Message);
       Debug.WriteLine(e.StackTrace);
       Debug.Flush();
#if USE_DEBUG
   } finally {
       debug.close();
#endif
   }
}
}} // } class Program } namespace Program
