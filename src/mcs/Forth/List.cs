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
//       List.cs
//
// Purpose-
//       (Linked) List class.
//
// Last change date-
//       2015/02/15
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
    //   List
    //
    // Purpose-
    //   (Linked) List.
    //
    /// <summary>
    /// This class differs in construction, usage, and performance from the
    /// System.Collections.Generic.List class. It is more closely related
    /// to the System.Collections.Generic.LinkedList class, but does not
    /// provide all the functionality and the property and function names
    /// differ. List is a strongly typed class.
    /// <remarks>
    /// The name Common.System.List duplicates System.Collections.Generic.List.
    /// See the remarks in Common.Core for implementation notes.
    /// </remarks>
    /// </summary>
    //
    //------------------------------------------------------------------------
    public class List<T>            // Extendable List
    {
        public class Link           // The Linked part of the List
        {
            internal protected Link next; // Next Link in List
            internal protected Link prev; // Prior Link in List
        }

        protected Link head;        // First Link in List
        protected Link tail;        // Last  Link in List
        protected long count;       // Number of elements in List

        public long Length { get { return count; } }

        public Link Head {          // First T in List
            get { return head == null ? null : head; }
        }

        public Link Tail {          // Last  T in List
            get { return tail == null ? null : tail; }
        }

        //--------------------------------------------------------------------
        // List: Methods
        //--------------------------------------------------------------------
        public void Fifo(Link link) { // Insert in FIFO (First In First Out) order
            link.next = null;
            link.prev = tail;

            if( head == null )
                head = link;
            else
                tail.next = link;

            tail = link;
            count++;
        }

        public void debug() {
            Console.WriteLine("{0} head({1}) tail({2}) count({3})",
                              ToString(), head, tail, count);

            Link link = head;
            while( link != null )
            {
                Console.WriteLine("[{0}] next({1}) prev({2})",
                                  link, link.next, link.prev);

                link = link.next;
            }
        }

        public void Lifo(Link link) { // Insert in LIFO (Last  In First Out) order
            link.next = head;
            link.prev = null;

            if( head == null )
                tail = link;
            else
                head.prev = link;

            head = link;
            count++;
        }

        public void Insert(Link after, Link head, Link tail) { // Remove a set of Links from the List
            if( after == null )     // Add to head of List
            {
                if( this.head == null )
                    this.tail = tail;
                else
                    this.head.prev = tail;

                head.prev = null;
                tail.next = this.head;
                this.head = head;
            }
            else if( after.next == null ) // Add to tail of List
            {
                after.next = head;
                head.prev = after;

                tail.next = null;
                this.tail = tail;
            }
            else
            {
                after.next = head;
                head.prev = after;

                tail.next = after.next;
                tail.next.prev = tail;
            }
        }

        public Link Remove( ) {     // Remove oldest Link from the List
            Link link = head;
            if( link != null )
            {
                head = link.next;
                if( head == null )
                    tail = null;
                else
                    head.prev = null;

                link.next = link.prev = null; // Remove dangling references
                count--;
            }

            return link;
        }

        public void Remove(Link head, Link tail) { // Remove a set of Links from the List
            if( head.prev == null )
                this.head = tail.next;
            else
                head.prev.next = tail.next;

            if( tail.next == null )
                this.tail = head.prev;
            else
                tail.next.prev = head.prev;

            head.prev = null;
            tail.next = null;
        }

        public Link Reset( ) {   // Remove ALL Links from the List
            Link link = head;

            head = tail = null;
            return link;
        }
    } // class List<T>
} // namespace Common.System

