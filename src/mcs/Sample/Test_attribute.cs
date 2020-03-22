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
//       Test_attribute.cs
//
// Purpose-
//       Test attributes.
//
// Last change date-
//       2019/02/15
//
// Implementation notes-
//       Currently only demonstrates custom attributes, not predefined ones.
//           See: https://www.tutorialspoint.com/csharp/csharp_attributes.htm
//
// Implementation TODOs-
//       TODO: Deferred. More why and/or how it works questions.
//       when #undef  USE_DEMO19_ONLY
//           TODO: Why does Demo19 work as the attribute name?
//           e.g.: Why are [Demo19(...)] and [Demo19Attribute(...)] identical?
//       when #define USE_RUNTIME_ERRORS
//           TODO: Why doesn't a constructor show up as a method?
//
//-----------------------------------------------------------------------------
#undef  USE_DEMO19_ONLY             // Define class Demo19, not Demo19Attribute
#undef  USE_COMPILE_ERRORS          // Demonstrate compile errors
#define USE_RUNTIME_ERRORS          // Demonstrate runtime errors

using System;                       // (Almost always required)
using System.Reflection;            // For Test_attribute
using System.Runtime.Serialization; // For ISerializable, SerializationInfo

using Shared;                       // For Debug

namespace Sample {                  // The Sample namespace
//=============================================================================
// Test_attribute: Demonstrate attributes
//=============================================================================
class Test_attribute: Test {        // Attributes *DEMO*, not a test
static string nullify(string s) { return Utility.nullify(s); }

#if USE_DEMO19_ONLY
    public class Demo19: Attribute {
#else
    public class Demo19Attribute: Attribute {
#endif
string                 url;
public string          Url { get { return url; } }

string                 topic;
public string          Topic        // Getter/Setter alternate format
{
   get {return topic;}
   set {topic= value;}
}

#if USE_DEMO19_ONLY
    public Demo19(string url) {     // Constructor
#else
    public Demo19Attribute(string url) { // Constructor
#endif
   this.url= url;
}
} // class Demo19Attribute (or Demo19)

// Note: Cannot use both definitions simultaneously. (Duplicates not allowed)
#if USE_DEMO19_ONLY
    [Demo19("http://example.com/Sample.htm")]
    #if USE_COMPILE_ERRORS
        [Demo19Attribute("http://example.com/Sample.htm")] // Does not exist
    #endif
#else
    #if USE_COMPILE_ERRORS
        [Demo19("http://example.com/Sample.htm")] // Duplicates Demo19Attribute
    #endif
    [Demo19Attribute("http://example.com/Sample.htm")]
#endif
public class Sample {
}  // class Sample

[Demo19("http://example.com/Fidget.htm", Topic= "Constructor")]
public class Fidget {
[Demo19("http://example.com/Fidget.htm", Topic= "Does-not-compute")]
public Fidget(string text) {}

[Demo19("http://example.com/Fidget.htm", Topic= "HelpFidget")]
public void Help( ) {}
}

public class Widget {
public Widget(string text) {}
[Demo19("http://example.com/Widget.htm", Topic= "HelpWidget")]
public void Help( ) {}
}

//-----------------------------------------------------------------------------
// Test_attribute.showAttribute: Display custom attributes
//-----------------------------------------------------------------------------
static void showAttribute(MemberInfo member) {
   string prefix="*DEMO*:";

   if( member == null ) {
       Debug.debug.putLine("{0} FAILED: member == null", prefix);
       return;
   }

#if USE_DEMO19_ONLY
   Demo19 a= Attribute.GetCustomAttribute(member, typeof(Demo19)) as Demo19;
#else
   Demo19Attribute a= Attribute.GetCustomAttribute(member,
                          typeof(Demo19Attribute)) as Demo19Attribute;
#endif
   if (a == null) {
       Debug.debug.putLine("{0} CustomAttribute for {1}: !NONE!",
                           prefix, member);
   } else {
       Debug.debug.putLine("{0} CustomAttribute for {1}:", prefix, member);
       Debug.debug.putLine("{0} >>Url={1}, Topic={2}", prefix,
                           nullify(a.Url), nullify(a.Topic));
   }
}

//=============================================================================
// Test_attribute.Test: Attribute demonstration
//=============================================================================
public void Test(object obj)
{
#if USE_DEMO19_ONLY
   string how= "with #define USE_DEMO19_ONLY";
#else
   string how= "with #undef USE_DEMO19_ONLY";
#endif

   Debug.debug.putLine("START>: Test_attribute... {0}", how);

   bool DEBUGGING= Debug.DEBUGGING;
   if( Options.level < 1 )
       Debug.DEBUGGING= false;

   showAttribute(typeof(Sample));

   Debug.debug.putLine();
   showAttribute(typeof(Fidget));
#if USE_RUNTIME_ERRORS // Or at least unexpected results
   Debug.debug.putLine("*DEMO*: Attempting: "
       + "showAttribute(typeof(Fidget).GetMethod(\"Fidget\"))");
   showAttribute(typeof(Fidget).GetMethod("Fidget"));
#endif

   Debug.debug.putLine();
   showAttribute(typeof(Widget));
   showAttribute(typeof(Widget).GetMethod("Help"));
   Debug.debug.putLine("<DEMO<: ...Test_attribute");

   Debug.DEBUGGING= DEBUGGING;
}
}  // class Test_attribute
}  // namespace Sample
