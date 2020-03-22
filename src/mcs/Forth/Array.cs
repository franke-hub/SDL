//----------------------------------------------------------------------------
//
//       Copyright (c) 2015 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Array.cs
//
// Purpose-
//       Array accessor class.
//
// Last change date-
//       2015/01/29
//
// Implementation notes-
//       ** CODE FORMAT NOT UPDATED **
//
//----------------------------------------------------------------------------
#if false                           // Hard-Core Debug Mode?
    #define HCDM
#endif

using System;
using System.Collections;           // For IEnumerator, IEnumerable

namespace Common.System {
    //------------------------------------------------------------------------
    //
    // Class-
    //   Array
    //
    // Purpose-
    //   Extendable Array.
    //
    /// <summary>
    /// This class differs in construction, usage, and performance from the
    /// System.Array class. (It is much more limited.)
    ///
    /// <remarks>
    /// The name Common.System.Array duplicates System.Array.
    /// </remarks>
    /// <example>
    /// This example shows how to select either this class or the default
    /// System.Array class.
    /// <code>
    /// using Common.System;       // An override selector will be required
    /// internal class Array&lt;T&gt; : Common.System.Array&lt;T&gt; {
    ///     // (Used) constructors must be provided
    ///     Array(string name = null) : base(name) {}
    ///     Array(long size) : base(size) {}
    /// }
    /// -or-
    /// internal class Array: System.Array {
    ///     // (Used) constructors must be provided
    ///     public Array() : base() {}
    /// }
    /// </code>
    /// Then you can use the selected class. For example
    /// <code>
    /// Array&lt;Whatever&gt; array;       // (When using Common.System.Array)
    /// -or-
    /// Array           array;       // (When using System.Array)
    /// </code></example>
    /// </summary>
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
} // namespace Common.System

