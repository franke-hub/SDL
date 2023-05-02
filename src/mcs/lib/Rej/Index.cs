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
//       Extendable array.
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
using System.Collections;           // For IEnumerator, IEnumerable
using System.Diagnostics;           // For [Conditional("...")] attribute

namespace Rejected {
    //------------------------------------------------------------------------
    //
    // Class-
    //   Index
    //
    // Purpose-
    //   Extendable array.
    //
    /// <summary>
    /// <remarks>
    /// The Index implementation contains discrete blocks of elements rather
    /// than a contiguous array.
    ///
    /// Indexing an Index is an O(Log HUNK_LOG2 n) operation.
    /// Resizing an Index is an O(1) operation.
    /// Trimming an Index is an O(1) operation.
    ///
    /// However, this class does not perform as well as expected.
    /// Compared to a simple array-based Index:
    ///    derived Array index functions performs at about 22% efficiency.
    ///    derived Stack store function performs at about 15% efficiency.
    ///
    ///    derived Stack index functions performs at about 22% efficiency.
    ///    derived Stack Push/Pop functions performs at about 66% efficiency.
    ///
    /// This is the base class for Rejected.Array and Rejected.Stack.
    /// </remarks>
    /// </summary>
    //
    //------------------------------------------------------------------------
    public class Index<T>: IEnumerable // Extendable array, implements foreach
    {
        protected const int HUNK_SIZE = 1024; // Production value
        protected const int HUNK_LOG2 = 10;   // Production value
        // protected const int HUNK_SIZE = 4;  // Bringup value
        // protected const int HUNK_LOG2 = 2;  // Bringup value
        // protected const int HUNK_SIZE = 64; // Performance test value
        // protected const int HUNK_LOG2 = 6;  // Performance test value

        // Debugging information
        string         name;        // Name
        static object  SerialNumber = 0; // Boxed for locking
        int            serialNumber; // Serial number

        // Index attributes
        protected long length;      // Total number of used elements
        public    long Length { get { return length; } }

        protected int  order;       // The number of levels in the Tree
        protected Hunk head;        // Head Hunk
        protected int  changeID;    // Change ID (For foreach verification)

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
        // Internal classes: Tree, Hunk, Leaf
        //--------------------------------------------------------------------
        protected class Tree {      // Hunk base class
            public virtual void debug(bool full) {
                throw new Exception("ShouldNotOccur");
            }
        }

        protected class Hunk: Tree { // Leaf locator node
            public Tree[]     tree = new Tree[HUNK_SIZE];

            public override void debug(bool full) {
                Console.WriteLine(">>>>>> " + ToString());
                if( full ) {
                    for(int i= 0; i<HUNK_SIZE; i++) {
                        Console.WriteLine("[{0,4:D}] {1}", i, tree[i] == null ? "<NULL>" : tree[i].ToString());
                    }

                    for(int i= 0; i<HUNK_SIZE; i++) {
                        if( tree[i] != null )
                            tree[i].debug(full);
                    }
                }
            }

            public static int index(long index) { // Hunk index for index
                return (int)(index >> HUNK_LOG2) & (HUNK_SIZE - 1);
            }

            public override string ToString() { // For Debugging
                return "Hunk";
            }
        }

        protected class Leaf: Tree { // Data element node
            public T[]        data = new T[HUNK_SIZE];

            public override void debug(bool full) {
                Console.WriteLine(">>>>>> " + ToString());
                if( full ) {
                    for(int i= 0; i<HUNK_SIZE; i++) {
                        Console.WriteLine("[{0,4:D}] {1}", i, data[i] == null ? "<NULL>" : data[i].ToString());
                    }
                }
            }

            public static int index(long index) { // Hunk index for index
                return (int)index & (HUNK_SIZE - 1);
            }

            public override string ToString() { // For Debugging
                return "Leaf";
            }
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
                Leaf leaf = getLeaf(index);
                return leaf.data[Leaf.index(index)];
            }

            set {                   // AKA Index.set_Item
                index = checkIndex(index);
                Leaf leaf = getLeaf(index);
                leaf.data[Leaf.index(index)] = value;

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
            Console.WriteLine("{0} order({1}) length({2}) head({3})",
                              ToString(), order, length, head);

            if( full && head != null )
                head.debug(full);
        }

        protected Hunk getHunk(long index) { // Get Hunk for (checked) index
            Tree tree;              // Working Tree (i.e. Hunk or Leaf)
            int  hunkX;             // Working Hunk index
            int  o = order;         // Working order

            Hunk hunk = head;
            while( o > 1 )
            {
                hunkX = (int)(index >> (HUNK_LOG2*o)) & (HUNK_SIZE - 1);
                tree = hunk.tree[hunkX];
                if( tree == null )
                    hunk.tree[hunkX] = tree = new Hunk();

                hunk = (Hunk)tree;
                o--;
            }

            return hunk;
        }

        protected Leaf getLeaf(long index) { // Get Leaf for index
            Hunk hunk = getHunk(index);
            int hunkX = Hunk.index(index);
            Tree tree = hunk.tree[hunkX];
            if( tree == null )
                hunk.tree[hunkX] = tree = new Leaf();

            return (Leaf)tree;
        }

        protected void grow(long size) { // Grow to this size
            int o = orderOf(size);
            while( o > order )      // Grow the order
            {
                order++;
                Hunk hunk = new Hunk();
                hunk.tree[0] = head;
                head = hunk;
            }

            changeID++;
        }

        protected int orderOf(long index) { // Get order of index
            int o = 1;
            while( index > HUNK_SIZE ) {
                o++;
                index >>= HUNK_LOG2;
            }

            return o;
        }

        protected void resize(long size) { // Resize the Index
            if( size > length )     // If resizing larger
                grow( size );
            else if( size < length )
                trim( size );

            length = size;
            changeID++;
        }

        public virtual void Reset() // Reset (empty) the Index
        {
            head = new Hunk();
            length = 0;
            order = 1;

            changeID++;
        }

        protected void trim(long size) { // Trim any unused elements
            Hunk hunk;              // Working Hunk
            int  hunkX;             // Working Hunk index
            Leaf leaf;              // Working Leaf
            int  leafX;             // Working Leaf index

            if( size <= 0 )         // Minimum size
                size = 1;

            if( length < size )
                length = size;

            int o = orderOf(size);
            while( order > o )      // Trim the order
            {
                head = (Hunk)head.tree[0];
                order--;
            }

            hunk = getHunk(size - 1);
            hunkX = Hunk.index(size - 1);
            for(hunkX= hunkX+1; hunkX<HUNK_SIZE; hunkX++)
                hunk.tree[hunkX] = null;

            leaf = getLeaf(size - 1);
            leafX = Leaf.index(size - 1);
            for(leafX= leafX + 1; leafX<HUNK_SIZE; leafX++)
                leaf.data[leafX]= default(T);

            changeID++;
        }

        public override string ToString() { return name; }

        [Conditional("CURRENTLY_UNUSED")]
        protected void insert(long index, T source) { // Insert one element
            Leaf fromLeaf;          // CopyFrom Leaf
            int  fromLeafX;         // CopyFrom Leaf index

            Leaf intoLeaf;          // CopyInto Leaf
            int  intoLeafX;         // CopyInto Leaf index

            index = checkIndex(index); // Normalize into index
            long fromX = length - 1;// Copy from index
            //// intoX = fromX + 1; // Copy into index (always fromX + 1)

            resize(length + 1);     // Updates changeID, length

            // Make room for the insert
            fromLeaf  = getLeaf(fromX);
            fromLeafX = Leaf.index(fromX);
            intoLeaf  = getLeaf(fromX + 1);
            intoLeafX = Leaf.index(fromX + 1);
            while( fromX >= index )
            {
                if( fromLeafX < 0 )
                {
                    fromLeafX = HUNK_SIZE - 1;
                    fromLeaf = getLeaf(fromX);
                }

                if( intoLeafX < 0 )
                {
                    intoLeafX = HUNK_SIZE - 1;
                    intoLeaf = getLeaf(fromX + 1);
                }

                intoLeaf.data[intoLeafX--] = fromLeaf.data[fromLeafX--];
                fromX--;
            }

            if( intoLeafX < 0 )
            {
                intoLeafX = HUNK_SIZE - 1;
                intoLeaf = getLeaf(fromX + 1);
            }

            intoLeaf.data[intoLeafX] = source;
        }

        [Conditional("CURRENTLY_UNUSED")]
        protected void insert(long index, Index<T> source) { // Insert an Index
            Leaf fromLeaf;          // CopyFrom Leaf
            int fromLeafX;          // CopyFrom Leaf index

            Leaf intoLeaf;          // CopyInto Leaf
            int intoLeafX;          // CopyInto Leaf index

            // This check asserts that we have a current, valid source.length
            // We check this once more after the operation completes.
            int  changeID     = source.changeID;
            long sourceLength = source.length;
            source.assertChangeID(changeID); // Integrity check

            if( sourceLength == 0 ) // Special case
                return;

            index = checkIndex(index); // Normalize into index

            long fromX = length - 1; // Set from index
            resize(length + sourceLength); // Updates changeID, length
            long intoX = length - 1; // Set into index

            // Make room for the insert
            fromLeaf  = getLeaf(fromX);
            fromLeafX = Leaf.index(fromX);
            intoLeaf  = getLeaf(intoX);
            intoLeafX = Leaf.index(intoX);
            while( fromX >= index )
            {
                if( fromLeafX < 0 )
                {
                    fromLeafX = HUNK_SIZE - 1;
                    fromLeaf = getLeaf(fromX);
                }

                if( intoLeafX < 0 )
                {
                    intoLeafX = HUNK_SIZE - 1;
                    intoLeaf = getLeaf(intoX);
                }

                intoLeaf.data[intoLeafX--] = fromLeaf.data[fromLeafX--];
                fromX--;
                intoX--;
            }

            fromLeaf  = source.getLeaf(0);
            fromLeafX = 0;
            intoLeaf  = getLeaf(index);
            intoLeafX = Leaf.index(index);
            for(int i= 0; i<sourceLength; i++)
            {
                if( fromLeafX >= HUNK_SIZE )
                {
                    fromLeafX = 0;
                    fromLeaf = source.getLeaf(i);
                }

                if( intoLeafX >= HUNK_SIZE )
                {
                    intoLeafX = 0;
                    intoLeaf = getLeaf(index+i);
                }

                intoLeaf.data[intoLeafX++] = fromLeaf.data[fromLeafX++];
            }

            // We check for a source Index change during our use of it (that
            // was not otherwise detected by a runtime exception.)
            // This can only occur due to a usage error.
            // If the check fails, this Index may be corrupt.
            source.assertChangeID(changeID); // Integrity check
        }

        [Conditional("CURRENTLY_UNUSED")]
        protected void remove(long loInp, long hiInp) { // Remove index elements
            Leaf fromLeaf;          // CopyFrom Leaf
            int  fromLeafX;         // CopyFrom Leaf index
            long hiX;               // Working from index

            Leaf intoLeaf;          // CopyInto Leaf
            int  intoLeafX;         // CopyInto Leaf index
            long loX;               // Working into index

            long intoX = checkIndex(loInp); // Normalize into index
            long fromX = checkIndex(hiInp); // Normalize from index
            if( intoX > fromX )
                throw argumentException();

            changeID++;             // Pre-change
            if( fromX < (length - 1) )
            {
                hiX = fromX + 1;
                loX = intoX;

                intoLeaf = getLeaf(loX);
                intoLeafX = Leaf.index(loX);

                fromLeaf = getLeaf(hiX);
                fromLeafX = Leaf.index(hiX);
                while( hiX < length )
                {
                    if( fromLeafX >= HUNK_SIZE )
                    {
                        fromLeafX = 0;
                        fromLeaf = getLeaf(hiX);
                    }

                    if( intoLeafX >= HUNK_SIZE )
                    {
                        intoLeafX = 0;
                        intoLeaf = getLeaf(loX);
                    }

                    intoLeaf.data[intoLeafX++] = fromLeaf.data[fromLeafX++];
                    loX++;
                    hiX++;
                }
            }

            // Update the length, saving the current length
            hiX = length;
            length -= (fromX - intoX) + 1;
            loX = length;

            // Remove deleted references
            intoLeaf = getLeaf(loX);
            intoLeafX = Leaf.index(loX);
            while( loX < hiX )
            {
                if( intoLeafX >= HUNK_SIZE )
                {
                    intoLeafX = 0;
                    intoLeaf = getLeaf(loX);
                }

                intoLeaf.data[intoLeafX++] = default(T);
                loX++;
            }

            changeID++;             // Post-change
        }
    } // class Index<T>
} // namespace Rejected

