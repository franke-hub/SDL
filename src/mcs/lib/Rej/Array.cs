//----------------------------------------------------------------------------
//
//       Copyright (c) 2015 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Array.cs
//
// Purpose-
//       Extendable Array.
//
// Last change date-
//       2015/01/13
//
// Implementation notes-
//       Improperly formatted
//
//----------------------------------------------------------------------------
#if false                           // Hard-Core Debug Mode?
    #define HCDM
#endif

using System;

namespace Rejected.System {
    //------------------------------------------------------------------------
    //
    // Class-
    //   Array
    //
    // Purpose-
    //   Extendable Array.
    //
    //------------------------------------------------------------------------
    public class Array<T>: Index<T> // Extendable Array
    {
        //--------------------------------------------------------------------
        // Array: Implementation
        //--------------------------------------------------------------------
        public Array(string name = "Array") : base(name) {} // Debugging constructor
        public Array(long size) : base("Array") { // Initial size specified
            resize(size);
        }

        // Expose Index functions
        public new void resize(long size) // Resize the Array
        {
            base.resize(size);
        }
    } // class Array<T>
} // namespace Rejected.System

