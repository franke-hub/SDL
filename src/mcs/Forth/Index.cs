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
//       Index.cs
//
// Purpose-
//       Index class, array based.
//
// Last change date-
//       2015/01/28
//
// Implementation notes-
//       ** CODE FORMAT NOT UPDATED **
//
// Implementation notes-
//       See: ~/src/mcs/lib/rej/Index.cs for an alternate implementation.
//
//----------------------------------------------------------------------------
#if false                           // Hard-Core Debug Mode?
    #define HCDM
#endif

using System;
using System.Collections;           // For IEnumerator, IEnumerable
using System.Diagnostics;           // For [Conditional("...")] attribute

namespace Common {
    //------------------------------------------------------------------------
    //
    // Class-
    //   Index
    //
    // Purpose-
    //   Array-based Index class.
    //
    //------------------------------------------------------------------------
    public class Index<T>: IEnumerable // Array, implements foreach
    {
        // Debugging information
        string         name;        // Name
        static object  SerialNumber = 0; // Boxed for locking
        int            serialNumber; // Serial number

        protected int  changeID;    // Change ID (For foreach verification)

        // Index attributes
        protected long size;        // Total number of elements
        protected long length;      // Total number of used elements
        public    long Length { get { return length; } }
        protected T[]  array;       // The working array

        //--------------------------------------------------------------------
        // Exception handling
        //--------------------------------------------------------------------
        protected void assertChangeID(int changeID) // Verify changeID unchanged
        {
            if( this.changeID == changeID )
                return;

            throw new InvalidOperationException("EnumFailed(" + this + ")");
        }

        protected Exception indexOutOfRangeException(string s) // Build IndexOutOfRangeException
        {
            s = "IndexOutOfRangeException(" + s + ")";
            Exception e = new IndexOutOfRangeException(s);
            return e;
        }

        protected Exception indexOutOfRangeException(long index) // Build IndexOutOfRangeException
        {
            return indexOutOfRangeException(String.Format("{0}[{1}]", this, index));
        }

        protected Exception argumentException() // Build ArgumentException
        {
            return new ArgumentException();
        }

        //--------------------------------------------------------------------
        // Implement foreach
        //--------------------------------------------------------------------
        public class Enumerator: IEnumerator { // Implement foreach
            Index<T>   index;       // The Index<T> object
            long       currentX;    // Current index
            int        changeID;    // changeID verifier

            T          current;     // Current element
            public T   Current             { get { return current; } }
            object     IEnumerator.Current { get { return current; } }

            public Enumerator(Index<T> index) { // Constructor
                this.index = index;
                this.changeID = index.changeID;
                Reset();
            }

            public void Dispose() {}
            public bool MoveNext() {
                index.assertChangeID( changeID );

                if( ++currentX >= index.Length )
                {
                    current = default(T);
                    return false;
                }

                current = index[currentX];
                return true;
            }

            public void Reset() {
                currentX = (-1);
                current = default(T);
            }
        }

        public IEnumerator GetEnumerator()      { return new Enumerator(this); }
        IEnumerator IEnumerable.GetEnumerator() { return new Enumerator(this); }

        //--------------------------------------------------------------------
        // Index[] Implementation
        //--------------------------------------------------------------------
        public T this[long index]   // Indexer
        {
            get {                   // AKA Index.get_Item
                index = checkIndex(index);
                return array[index];
            }

            set {                   // AKA Index.set_Item
                index = checkIndex(index);
                array[index] = value;

                changeID++;
            }
        }

        //--------------------------------------------------------------------
        // Index: Implementation
        //--------------------------------------------------------------------
        public Index(string name = null) // Default constructor
        {
            if( name == null )
            {
                lock(SerialNumber) {
                    serialNumber = (int)SerialNumber + 1;
                    SerialNumber = serialNumber;
                }
                name = typeof(Index<T>) + "[" + serialNumber + "]";
            }
            this.name = name;

            Reset();
        }

#if ALTERNATE_IMPLEMENTATION
        internal long adjustedSize(long size) { // Recommended size
            const int  HUNK_SIZE = 1048576; // Maximum allocation hunk
            long adjusted = size;
            if( size < HUNK_SIZE )
            {
                adjusted = 64;      // Minimum recommended
                while( adjusted < size )
                    adjusted <<= 1;
            }
            else
            {
                adjusted += (adjusted/2);
                adjusted += HUNK_SIZE - 1;
                adjusted &= ~(HUNK_SIZE - 1);
           }

            return adjusted;
        }
#endif

        internal long adjustedSize(long size) { // Adjusted size
            long adjusted = 64;     // Minium recommended size
            while( adjusted < size )
                adjusted <<= 1;

            return adjusted;
        }

        protected long checkIndex(long index) { // Verify an index
            long x = index;
            if( x < 0 )
            {
                x += length;        // (Access from end of Index)
                if( x < 0 )
                    throw indexOutOfRangeException(index);
            }
            else if( x >= Length )  // If index out of range
                throw indexOutOfRangeException(index);

            return x;
        }

        public void debug(bool full = false)
        {
            Console.WriteLine("{0} size({1}) length({2}) changeID({3})",
                              ToString(), size, length, changeID);

            if( full )
            {
                for(long i= 0; i<size; i++)
                {
                    object o = array[i] as object;
                    string value = "<NULL>";
                    if( o != null )
                        value = o.ToString();
                    Console.WriteLine("[{0}] {1}", i, value);
                }
            }

        }

        protected void grow(long size) { // Grow to this size
            if( size > this.size )
            {
                long length = this.length;
                size = adjustedSize(size);
                resize(size);
                this.length = length;
            }
        }

        protected void resize(long size) { // Resize the Index
            if( size != this.size )  // If changing size
            {
                if( length > size )
                    length = size;

                T[] array = new T[size];
                for(int i= 0; i<length; i++)
                    array[i] = this.array[i];

                this.array = array;
                this.size = size;
            }

            length = size;
            changeID++;
        }

        public virtual void Reset() // Reset (empty) the Index
        {
            length = 0;
            size   = 64;
            array  = new T[64];

            changeID++;
        }

        protected void trim(long size) { // Trim any unused elements
            if( size < this.size )
            {
                long length = this.length;
                size = adjustedSize(size);
                resize(size);
                if( size > length )
                    this.length = length;
            }
        }

        public override string ToString() { return name; }
    } // class Index<T>
} // namespace Common

