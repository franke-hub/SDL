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
//       Stack.cs
//
// Purpose-
//       Extendable Stack.
//
// Last change date-
//       2015/01/28
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
    //   Stack
    //
    // Purpose-
    //   Extendable Stack.
    //
    /// <summary>
    /// This class differs in construction, usage, and performance from the
    /// System.Collections.Stack class. These are different classes.
    ///
    /// <remarks>
    /// The name Common.System.Stack duplicates System.Collections.Stack.
    /// </remarks>
    /// <example>
    /// This example shows how to select either this class or the default
    /// System.Collections.Stack class.
    /// <code>
    /// using Common.System;       // An override selector will be required
    /// internal class Stack&lt;T&gt; : Common.System.Stack&lt;T&gt; {
    ///     // The constructor must be provided
    ///     Stack(string name = null) : base(name) {}
    /// }
    /// -or-
    /// internal class Stack: System.Collections.Stack {
    ///     // (Used) constructors must be provided
    ///     public Stack() : base() {}
    ///     public Stack(ICollection c) : base(c) {}
    ///     public Stack(Int32 i) : base(i) {}
    /// }
    /// </code>
    /// Then you can use the selected class. For example
    /// <code>
    /// Stack&lt;Whatever&gt; stack;       // (When using Common.System.Stack)
    /// -or-
    /// Stack           stack;       // (When using System.Stack)
    /// </code></example>
    /// </summary>
    //
    //------------------------------------------------------------------------
    public class Stack<T>: Index<T> // Extendable Stack, implements foreach
    {
        //--------------------------------------------------------------------
        // Stack: Implementation
        //--------------------------------------------------------------------
        public Stack(string name = null) : base(name) {} // Default constructor

        //--------------------------------------------------------------------
        // Stack[] Implementation
        //--------------------------------------------------------------------
        public new T this[long index]   // Indexer
        {
            get { return base[index]; }  // AKA Stack.get_Item
            protected set { base[index] = value; } // AKA Stack.set_Item
        }

        //--------------------------------------------------------------------
        // Stack: Methods
        //--------------------------------------------------------------------
        public T Peek()             // Examine top element
        {
            if( length == 0 )
                throw indexOutOfRangeException(-1);

            return array[length - 1];
        }

        public void Poke(T t)       // Replace top element
        {
            if( length == 0 )
                throw indexOutOfRangeException(-1);

            array[length - 1] = t;
            changeID++;
        }

        public virtual T Pop()      // Remove from Stack
        {
            if( length <= 0 )
                throw indexOutOfRangeException(0);

            T result = array[--length];
            array[length] = default(T); // Remove object reference

            changeID++;
            return result;
        }

        public virtual void Push(T t) // Append to Stack
        {
            if( length >= size )
                grow(length + 1);

            array[length++] = t;
            changeID++;
        }
    } // class Stack<T>
} // namespace Common.System

