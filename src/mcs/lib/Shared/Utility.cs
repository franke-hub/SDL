//----------------------------------------------------------------------------
//
//       Copyright (c) 2019 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//-----------------------------------------------------------------------------
//
// Title-
//       Utility.cs
//
// Purpose-
//       Utility routines.
//
// Last change date-
//       2019/02/15
//
//-----------------------------------------------------------------------------
using System;                       // (Almost always required)
using System.Runtime.InteropServices; // For IsOSPlatform

namespace Shared {                  // The Shared library namespace
//=============================================================================
// Utility: Utility routines
//=============================================================================
static public class Utility {
//-----------------------------------------------------------------------------
// Utility.Attributes
//-----------------------------------------------------------------------------
static public bool     ALWAYS_TRUE { get {return true;} } // Used to avoid compiler warnings
static DateTime        tod_origin= new DateTime(1970,1,1,0,0,0);

// Runtime information, only set once
#if false //== This is the way is should work, but doesn't ====================
////////////== Compiles and operates properly on Windows, but not Linux========
static bool            have_linux= RuntimeInformation.IsOSPlatform(
                           System.Runtime.InteropServices.OSPlatform.Linux);
static public bool     is_linux { get { return have_linux; } }

static bool            have_windows= RuntimeInformation.IsOSPlatform(
                           System.Runtime.InteropServices.OSPlatform.Windows);
static public bool     is_windows { get { return have_windows; } }

#else     //== TEMPORARY WORK-AROUND ==========================================
//        //== Need to compile this class on Linux too! =======================
static public bool     is_linux   { get { return false; } }
static public bool     is_windows { get { return true;  } }
#endif

//-----------------------------------------------------------------------------
//
// Method-
//       Utility.humanify
//
// Purpose-
//       Convert a numeric size to a 4 character human readable form.
//
// Implementation notes-
//       Uses decimal suffixes.
//
//-----------------------------------------------------------------------------
static char[]          size_suffix= // Suffix array
{  'K' // Kilo  (1000)^1
,  'M' // Mega  (1000)^2
,  'G' // Giga  (1000)^3
,  'T' // Tera  (1000)^4
,  'P' // Pita  (1000)^5
,  'E' // Exa   (1000)^6  (Maximum ulong is 9.2E)
// 'Z' // Zetta (1000)^7  (Should not occur)
// 'Y' // Yotta (1000)^8  (Should not occur)
// '?' // Lotta (1000)^9  (Not a real name)
};

static public string humanify(ulong n)
{
   string              s= "????";   // Resultant string

   if( n < 9950 )
       s= String.Format("{0,4}", n); // 9950 rounds to " 10K"
   else {
       int X= 0;                    // Size suffix index
       while( X < size_suffix.Length )
       {
           if( n < 9950 ) {         // < "9.9Suffix"
               n += 50;             // Round up
               ulong m= n % 1000;   // Decimal point (*100)
               n /= 1000;
               m /= 100;
               s= String.Format("{0}.{1}{2}", n, m, size_suffix[X]);
               break;
           }

           if( n < 99500 ) {        // < " 99Suffix"
               n += 500;            // Round up
               n /= 1000;
               s= String.Format("{0,3}{1}", n, size_suffix[X]);
               break;
           }

           if( n < 999500 ) {       // < "999Suffix"
               n += 500;            // Round up
               n /= 1000;
               s= String.Format("{0,3}{1}", n, size_suffix[X]);
               break;
           }

           // Next grouping
           n /= 1000;
           X++;
       }
   }

   return s;
}

//-----------------------------------------------------------------------------
//
// Method-
//       Utility.nullify
//
// Purpose-
//       Instead of printing "", print "<null>"
//
//-----------------------------------------------------------------------------
static public string nullify(object o) // Return: o != null ? o : "<null>"
{
   return o != null ? o.ToString() : "<null>";
}

//-----------------------------------------------------------------------------
//
// Method-
//       Utility.tod()
//
// Purpose-
//       Obtain the current unix time (in seconds)
//
// Implementation notes-
//       Accurate to one "tick," about 1/64 second.
//
//-----------------------------------------------------------------------------
static public double tod()          // Current unix time (in seconds)
{
   TimeSpan ts= DateTime.UtcNow - tod_origin;
   return ts.TotalSeconds;
}

//-----------------------------------------------------------------------------
//
// Method-
//        Utility.unixify(string)
//
// Purpose-
//        On windows, return all '\' characters in string changed to '/'
//        (For others, just return string)
//
//-----------------------------------------------------------------------------
static public string unixify(string file)
{
   if( is_windows ) {
       Char[] inp= file.ToCharArray();
       for(int i= 0; i<inp.Length; i++) {
           char C= inp[i];
           if( C == '\\' )
               inp[i]= '/';
       }

       return new String(inp);
   }

   return file;
}

//-----------------------------------------------------------------------------
//
// Method-
//        Utility.wildmatch(wildcard-string, match-string)
//
// Purpose-
//        Wildcard string matcher
//
// Usage notes-
//        The '*' character matches zero or more characters
//        The '?' character matches exactly one character
//        The '!' character is reserved for expansion.
//           Any use other than within "(!!)" matches nothing.
//
//        The "?*" sequence matches one or more characters
//        The "**" sequence is invalid. It matches nothing.
//        The "*?" sequence is invalid. It matches nothing.
//
//        The "(**)" escape sequence matches one '*' character
//        The "(??)" escape sequence matches one '?' character
//        The "(!!)" escape sequence matches one '!' character
//
//        There is no escape sequence for '(' or ')', they are escape
//        wrappers ONLY when used for the exact sequences above.
//            "(*)"  Matches: '(', zero or more characters, and ')'
//            "(?)"  Matches: '(', any single character, and ')'
//            "(!)"  Matches: NOTHING. Reserved character '!' detected.
//            "(*?)" Matches: '(', one or more characters, and ')'
//            "((**)(**))" Exactly matches: '(', '*', '*', ')'
//
// Possible expansion-
//        Initial state: Case matching ON
//        Some sequence like "!<" turns chacter case matching OFF
//        Some sequence like ">!" turns chacter case matching ON
//
//-----------------------------------------------------------------------------
static public bool wildmatch(string wild, string text)
{
   int W= 0;                       // Wildcard index
   int T= 0;                       // Text index

// Debug.WriteLine("wildmatch({0},{1})", wild, text);

   while( W < wild.Length ) {
       char C= wild[W++];
       if( W >= wild.Length ) {    // If this is the last character
           if( C == '*' )          // Ending '*' matches anything
               return true;
       }

       if( T >= text.Length )      // If no text remains
           return false;

       // Handle special wildcard characters and sequences
       if( C == '*' ) {             // Matches any number of characters
           C= wild[W++];            // The next character to match

           if( C == '*' || C == '?' || C == '!' ) // If invalid sequence
               return false;

           if( C == '(' ) {         // If possible escape sequence
               if( (wild.Length - W) >= 3 && wild[W+2] == ')' ) {
                   if( wild[W] == '*' && wild[W+1] == '*' ) {
                       C= '*';
                       W += 3;
                   } else
                   if( wild[W] == '?' && wild[W+1] == '?' ) {
                       C= '?';
                       W += 3;
                   } else
                   if( wild[W] == '!' && wild[W+1] == '!' ) {
                       C= '!';
                       W += 3;
                   }
               }
           }

           string subWild= wild.Substring(W); // (Does not contain C)
           while( T < text.Length ) {
               if( C == text[T++] ) { // If a possible match found
                   string subText= text.Substring(T);
                   if( wildmatch(subWild, subText) )
                       return true;
               }
           }

           return false;
       } else if( C == '?' ) {      // Matches one character
           T++;                     // Matched one
           continue;
       } else if( C == '!' ) {      // If expansion character
           return false;            // (Outside of escape sequence)
       } else if( C == '(' ) {      // If possible escape sequence
           if( (wild.Length - W) >= 3 && wild[W+2] == ')' ) { // If '(',?,?,')'
               if( wild[W] == '*' && wild[W+1] == '*' ) {
                   C= '*';
                   W += 3;
               } else
               if( wild[W] == '?' && wild[W+1] == '?' ) {
                   C= '?';
                   W += 3;
               } else
               if( wild[W] == '!' && wild[W+1] == '!' ) {
                   C= '!';
                   W += 3;
               }
           }
       }

       // Handle must match characters
       if( C != text[T++] )
           return false;
   }

   if( T != text.Length )
       return false;

   return true;
}  // method wildmatch
}} // class Utility, namespace Shared
