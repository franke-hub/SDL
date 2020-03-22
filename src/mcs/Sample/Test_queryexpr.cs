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
//       Test_queryexpr.cs
//
// Purpose-
//       Test query expressions.
//
// Last change date-
//       2019/02/15
//
//-----------------------------------------------------------------------------
#define USE_LINQ                    // Use LINQ or "How it works" demo code

using System;                       // (Almost always required)
using System.Collections.Generic;   // For IEnumeration
using System.Linq;                  // Enable query expressions

using Shared;                       // For Debug

namespace Sample {                  // The Sample namespace
//=============================================================================
//
// Class-
//       Test_queryexpr
//
// Purpose-
//       Test query expressions.
//
// Implementation notes-
//       An alternate DEMO version provides a reference implementation.
//
//=============================================================================

class Test_queryexpr: Test {        // Test query expressions
public void Test(object obj)
{
   int[] samples= { 101, 171, 82, 293, 25, 171 }; // Data samples

#if USE_LINQ
   Debug.WriteLine("START>: Test_queryexpr LINQ version");

   // Evaluate a simple LINQ query expression.
   IEnumerable<int> results=        // The query resultant
       from sample in samples       // (For each sample in samples)
       where sample > 100           // Optional selection qualifier
       orderby sample ascending     // Optional sorter, ascending/descending
       select sample;               // Must end with either select or group

#else
   Debug.WriteLine("START>: Test_queryexpr DEMO version");

   // First, select the samples from the source
   IList<int> results= new List<int>();
   foreach(int sample in samples) { // from sample in samples
       if( sample > 100 )           // where sample > 100
           results.Add(sample);     // select sample
   }

   // Second, (inefficiently) bubble sort the list, ascending
   // (Bubble sort is used for clarity. More elegant sort mechanisms exist.)
   for(int i= 0; i<results.Count; i++) { // orderby sample ascending
       for(int j= i+1; j<results.Count; j++) {
          if( results[j] < results[i] )
          {
              int T= results[j];
              results[j]= results[i];
              results[i]= T;
          }
       }
   }
#endif

   // Verify the Query resultant
   int N= 0;
   foreach(int result in results)
   {
       switch(N++)
       {
           case 0:
               Debug.assert(result == 101);
               break;

           case 1:
               Debug.assert(result == 171);
               break;

           case 2:
               Debug.assert(result == 171);
               break;

           case 3:
               Debug.assert(result == 293);
               break;

           default:
               throw new Exception("ShouldNotOccur");
       }
   }
   Debug.assert(N == 4);

   Debug.WriteLine("PASSED: Test_queryexpr");
}
}  // class Test_queryexpr
}  // namespace Sample
