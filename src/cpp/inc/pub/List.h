//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2022 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       List.h
//
// Purpose-
//       Describe the List objects.
//
// Last change date-
//       2022/04/05
//
// Implementation notes-
//       Unlike std::List<T>, pub::List<T> elements *are* links.
//       Pros:
//         This optimizes list handling, especially when copying is expensive.
//       Cons:
//         pub::List classes do not *own* List<T>::Link objects. It is the
//         user's responsibility to create and delete them.
//            Note that while this is similar to that of a std::list<T*>,
//            accessing a pub::List<T>::Link requires one less load operation.
//       Differences:
//         ++pub::List<T>::end() == pub::List<T>::end()
//         ++std::list<T>::end() == std::list<T>::begin()
//         --pub::List<T>::end() == pub::List<T>::end()
//         --std::list<T>::end() == std::list<T>::rbegin()
//
//         --pub::List<T>::begin() == pub::List<T>::end()
//         --std::list<T>::begin() == std::list<T>::rend()
//         *List<T>::end() and List<T>::end()-> throw an exception rather than
//         act in an undefined manner.
//
//         std::list has methods not available in pub::List. It handle arrays
//         extremely well. pub::AI_list provides a lock-free atomic insertion
//         capability, not available in std::list.
//
//       Note: pub::List<T>::Link construction and destruction is *always*
//       the user's responsibility. e.g. pub::~List<T> does nothing.
//
//       For all List classes, the is_coherent and is_on_list methods run in
//       linear time. Method is_coherent examines the entire List. Method
//       is_on_list traverses the List until either the Link is found or the
//       entire List examined.
//
//       In lieu of searching Lists for duplicated Links, is_coherent reports
//       FALSE should a List contains more than an implementation defined
//       (Currently 1G) Link count. Other methods assume that the List is
//       coherent and either ignore or do not check for usage errors.
//
//       By convention, Lists are ordered from head to tail. The head Link
//       is the Link that will be removed by REMQ and before which LIFO Links
//       are inserted. The tail Link is the insert point after which FIFO
//       Links are inserted.
//
// List types-
//       AI_list<T>:    Atomic Insert Singly Linked List, thread-safe.
//       DHDL_list<T>:  Doubly Headed Doubly Linked List.
//       DHSL_list<T>:  Doubly Headed Singly Linked List.
//       SHSL_list<T>:  Single Headed Singly Linked List.
//       List<T>:       Equivalent to DHDL_list<T>.
//
//       In each case, the associated Link class is defined within the List
//       class. All Link classes must be derived from that List::Link class.
//       An example follows:
//
// Example declaration and usage-
//       class My_link : public List<My_link>::Link {
//       public:
//         My_link(...) : List<My_link>::Link(), ... { ... } // Constructor
//         // Remainder of implementation of My_link
//       }; // class My_link, the elements to be put on a class List<My_link>
//
//       List<My_link> my_list1;    // A List containing My_link elements
//       List<My_link> my_list2;    // Another List contining My_link Links
//
//       My_link* link1= new My_link(); // Create a new My_link
//       my_list1.fifo(link1);         // Insert it (FIFO) onto my_list1
//       My_link* link2= my_list1.remq(); // Then remove it, emptying my_list1
//       assert( link1 == link2 );     // The link added is the link removed
//       my_list2.lifo(link2);         // Insert it (LIFO) onto my_list2
//       // my_list1 is empty, my_list2 only contains link2 (which == link1)
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_LIST_H_INCLUDED
#define _LIBPUB_LIST_H_INCLUDED

#include <atomic>                   // For std::atomic
#include <functional>               // For std::less<>

#include "bits/List.h"              // For List template definitions, ...

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
//----------------------------------------------------------------------------
//
// Class-
//       AI_list<T>
//
// Purpose-
//       Typed AI_list object, where T is of class AI_list<T>::Link.
//
//----------------------------------------------------------------------------
/*****************************************************************************
   @brief An atomic container with fixed time element insertion and iteration.

   @tparam T The type of the element, which *must* be a subclass of
     AI_list<T>::Link.

   Two classes of users can simultaneously access an AI_List<T>.
   <em>Producers</em> atomically add links to the list using the lock-free
   fifo() method. There may be any number of <em>producer</em> threads.
   <em>Consumers</em> serially use all other AI_list methods. Each AI_list
   only supports a single concurrent <em>consumer</em>.

   Note the fifo() method returns the previous _tail, the newest item on the
   List. If nullptr is returned, the List went from idle into active state.

   The begin() method creates an input_iterator in linear time, first removing
   all Links and then reversing that reversely inserted list. Those links,
   now in FIFO order, are presented to the consumer.  Addionally, this input
   iterator automatically handles links added to the List while iterating
   without changing its state from active to idle (empty.)
*****************************************************************************/
template<class T>
   class AI_list
   {
     public:
       typedef T                              value_type;
       typedef value_type*                    pointer;
       typedef value_type&                    reference;

       typedef AI_list<T>                     _Self;
       typedef _AI_iter<T>                    iterator;

       struct Link : protected __detail::_PREV_link
       {
         friend struct _AI_iter<T>;
         friend class AI_list;
         public:
           pointer get_prev( void ) const
           { return static_cast<pointer>(_prev); }
       }; // struct Link

     protected:
       std::atomic<pointer> _tail= nullptr; // The newest List element

     public:
       //---------------------------------------------------------------------
       // AI_list<T>::Constructor/Destructor
       //---------------------------------------------------------------------
       ~AI_list( void ) = default;
       AI_list( void ) = default;

       //---------------------------------------------------------------------
       //
       // Method-
       //       AI_list<T>::debug
       //
       // Purpose-
       //       Debugging display
       //
       // Implementation notes-
       //       Only the consumer thread can safely use this debugging method.
       //
       //---------------------------------------------------------------------
       void debug(const char* info= "")
       {
         pointer tail= _tail.load();
         debugf("AI_List(%p)::debug(%s) _tail(%p) __end(%p,%p)\n", this
               , info, tail, __detail::__end, &__detail::__end);
         _AI_iter<T>::debug(tail);
       }

       //---------------------------------------------------------------------
       //
       // Method-
       //       AI_list<T>::begin
       //       AI_list<T>::end
       //
       // Purpose-
       //       Create begin iterator
       //       Create end   iterator
       //
       // Implementation notes-
       //       Only the consumer can safely use this method.
       //
       //       The begin() iterator removes all current links, creating an
       //       input iterator from them. These links are *only* associated
       //       with that iterator. This process is automatically repeated
       //       when all removed links have been processed so that links
       //       inserted while the iteration is in progress are logically
       //       part of that iteration. If no new links were inserted, the
       //       AI_list becomes empty and the iterator == end().
       //
       //---------------------------------------------------------------------
       iterator begin() noexcept { return iterator(this); }
       iterator end()   noexcept { return iterator(); }

       //---------------------------------------------------------------------
       //
       // Method-
       //       AI_list<T>::fifo
       //
       // Purpose-
       //       Atomically insert a link onto the AI_list.
       //
       // Implementation note-
       //       THREAD SAFE.  Any number of producer threads may simultaneously
       //       use this method.
       //
       //---------------------------------------------------------------------
       /**
         @brief Thread-safe FIFO ordering Link insertion
         @param link The Link to insert.

         Inserts a Link into the list such that the begin() iterator has FIFO
         ordering. The List itself has LIFO ordering.
       **/
       pointer                      // -> Prior tail
         fifo(                      // Insert (fifo order)
           pointer     link)        // -> Link to insert
       {
          pointer prev= _tail.load();
          link->_prev= prev;
          while( !_tail.compare_exchange_weak(prev, link) )
            link->_prev= prev;

          return prev;
       }

       //---------------------------------------------------------------------
       //
       // Method-
       //       AI_list<T>::get_tail
       //
       // Purpose-
       //       Get the tail link.
       //
       // Implementation notes-
       //       Only the consumer thread can safely use this method.
       //
       //---------------------------------------------------------------------
       pointer                      // -> Tail Link
         get_tail( void ) const     // Get tail link
       { return _tail.load(); }

       //---------------------------------------------------------------------
       //
       // Method-
       //       AI_list<T>::is_coherent
       //
       // Purpose-
       //       Coherency check.
       //
       // Implementation notes-
       //       Only the consumer thread can safely use this debugging method.
       //
       //---------------------------------------------------------------------
       bool                         // TRUE if the object is coherent
         is_coherent( void ) const  // Coherency check
       {
          pointer link= _tail.load(); // The newest Link
          for(int count= 0; count < __detail::MAX_COHERENT; count++)
          {
            if( link == nullptr )
              return true;

            link= link->get_prev();
          }

          return false;
       }

       //---------------------------------------------------------------------
       //
       // Method-
       //       AI_list<T>::is_empty
       //
       // Purpose-
       //       (Instantaneous) test for empty list.
       //
       // Implementation notes-
       //       Only the consumer thread can safely use this method. Testing
       //       for an empty List isn't useful when active producers exist.
       //
       //---------------------------------------------------------------------
       bool                         // TRUE if the List is empty
         is_empty( void ) const     // Is the List empty?
       { return _tail.load() == nullptr; }

       //---------------------------------------------------------------------
       //
       // Method-
       //       AI_list<T>::is_on_list
       //
       // Purpose-
       //       Test whether link is present in this List.
       //
       // Implementation notes-
       //       Only the consumer thread can safely use this method.
       //
       //---------------------------------------------------------------------
       bool                         // TRUE if link is contained
         is_on_list(                // Is link contained?
           pointer     link) const  // -> Link
       {
          if( link )
          {
            pointer prev= _tail.load();
            while( prev != nullptr && (void*)prev != __detail::__end )
            {
              if( prev == link )
                return true;

              prev= prev->get_prev();
            }
          }

          return false;
       }

       //---------------------------------------------------------------------
       //
       // Method-
       //       AI_list<T>::reset
       //
       // Purpose-
       //       Remove ALL Links from the List.
       //
       // Implementation notes-
       //       Only the consumer thread can safely use this method.
       //       The returned Links are (reverse) ordered from tail to head.
       //
       //---------------------------------------------------------------------
       pointer                      // -> The set of removed Links
         reset( void )              // Reset (empty) the List
       {
          pointer link= _tail.load();
          while( !_tail.compare_exchange_weak(link, nullptr) )
            ;

          return link;
       }

       //---------------------------------------------------------------------
       //
       // Method-
       //       AI_list<T>::reset
       //
       // Purpose-
       //       Remove ALL Links from the List, replacing the List
       //       The returned Links are (reverse) ordered from tail to head.
       //
       //       This method is used by the iterator to prevent triggering
       //       of the empty to non-empty state transition. For a sample
       //       implementation, see ~/src/cpp/lib/pub/Dispatch.cpp::work
       //
       //---------------------------------------------------------------------
       /**
         @brief Atomically replace the List
         @param tail The tail pseudo-link that replaces the _List.
         @return The set of removed links (tail->pointer->...->nullptr)

         Note: The tail parameter is a pseudo-link. The first inserted Link
         points to it, but tail is not a Link. It doesn't point anywhere.

         @code
           { [[atomic]]
             if( _list::_tail == nullptr )
               return nullptr;
           }

           { [[atomic]]
             if( _list::_tail == tail )
             {
               _list::_tail= nullptr;
               return nullptr;
             }
           }

           { [[atomic]]
             pointer prev= _list::_tail;
             pointer _tail= (pointer)tail; // (tail is a pseudo-link)
             return prev;
           }
         @endcode
       **/
       pointer
         reset(                     // Reset (empty) the List
           void* tail) noexcept     // Replacing it with this pseudo-link
       {
          pointer link= _tail.load(); // Get the current tail
          if( link == nullptr )     // If the List is currently empty
            return nullptr;         // Do not replace it

          // Attempt replacement with empty pseudo-Link
          while( (void*)link == tail )
          {
            if( _tail.compare_exchange_weak(link, nullptr) )
              return nullptr;
          }

          // Replace the List
          while( !_tail.compare_exchange_weak(link, (pointer)tail) )
            ;

          return link;              // Return the newest existing Link
       }
   }; // class AI_list<T>

//----------------------------------------------------------------------------
//
// Class-
//       DHDL_list<T>
//
// Purpose-
//       Typed DHDL_list object, where T is of class DHDL_list<T>::Link.
//
//----------------------------------------------------------------------------
template<class T>
   class DHDL_list : public DHDL_list<void>
   {
     public:
       typedef T                              value_type;
       typedef T*                             pointer;
       typedef T&                             reference;

       typedef _DHDL_list_iterator<value_type> iterator;
       typedef _DHDL_list_const_iterator<value_type> const_iterator;

       typedef DHDL_list<void>                _Base;
#if ! USE_BASE_SORT
       typedef std::function<bool(pointer, pointer)>
                                              _Comparator;
#endif

       class Link : protected _Link
       {
         friend class DHDL_list;
         public:
           pointer get_next( void ) const
           { return static_cast<pointer>(_next); }

           pointer get_prev( void ) const
           { return static_cast<pointer>(_prev); }
       }; // class DHDL_list<T>::Link

       //---------------------------------------------------------------------
       // DHDL_list<T>::Constructor/Destructor
       //---------------------------------------------------------------------
       DHDL_list( void ) {}
       ~DHDL_list( void ) {}

       //---------------------------------------------------------------------
       // DHDL_list<T>::Methods
       //---------------------------------------------------------------------
             iterator begin()       noexcept { return iterator(_head); }
       const_iterator begin() const noexcept { return const_iterator(_head); }
             iterator end()         noexcept { return iterator(); }
       const_iterator end()   const noexcept { return const_iterator(); }

       void
         fifo(                      // Insert (FIFO order)
           pointer           link)  // -> Link to insert
       { _Base::fifo(link); }

       pointer                      // -> Head pointer on List
         get_head( void ) const     // Get head link
       {  return static_cast<pointer>(_head); }

       pointer                      // -> Tail pointer on List
         get_tail( void ) const     // Get tail link
       {  return static_cast<pointer>(_tail); }

       /**********************************************************************
         @brief Insert link at position.

         @param link The Link *before* the insert position, nullptr for head.
         @param head The first Link to insert.
         @param tail The last Link to insert.

         Preconditions: The head to tail chain must be well-formed.
         Postcondition: link->_next == head, head->_prev == link,
                        tail->_next == link->next, tail->_next->_prev == tail
       **********************************************************************/
       void
         insert(                    // Insert at position,
           pointer                link, // -> Link to insert after
           pointer                head, // -> First Link to insert
           pointer                tail) // -> Final Link to insert
       { _Base::insert(link, head, tail); }

       bool                         // TRUE if the object is coherent
         is_coherent( void ) const  // Coherency check
       { return _Base::is_coherent(); }

       bool                         // TRUE if link is contained
         is_on_list(                // Is Link contained?
           pointer                link) const  // -> Link
       { return _Base::is_on_list(link); }

       void
         lifo(                      // Insert (LIFO order)
           pointer                link) // -> Link to insert
       { _Base::lifo(link); }

       void
         remove(                    // Remove from list
           pointer                head, // -> First Link to remove
           pointer                tail) // -> Final Link to remove
       { _Base::remove(head, tail); }

       pointer                      // Removed pointer
         remq( void )               // Remove head link
       { return static_cast<pointer>(_Base::remq()); }

       pointer                      // -> The set of removed Links
         reset( void )              // Reset (empty) the List
       { return static_cast<pointer>(_Base::reset()); }

       /**********************************************************************
         @brief Sort using comparitor.

         Sample code:
         @code
           TODO: Copy from List.cpp or TestList.cpp when working
         @endcode
       **********************************************************************/
#if USE_BASE_SORT
       void sort(_Comparator cmp)   // Sort the List using Comparator
       { _Base::sort(cmp); }
#else
       void sort(_Comparator cmp)   // Sort the List using Comparator
       {
         pointer head= reset();

         while( head )
         {
           pointer low= head;
           pointer next= low->get_next();
           while( next != nullptr )
           {
             if( cmp(next, low) )
               low= next;

             next= next->get_next();
           }

           if( low == head )
             head= head->get_next();
           else
           {
             if( low->get_next() != nullptr )
               low->get_next()->_prev= low->_prev;
             low->_prev->_next= low->get_next();
           }

           fifo(low);
         }
       }
#endif
       }; // class DHDL_list<T>

//----------------------------------------------------------------------------
//
// Class-
//       DHSL_list<T>
//
// Purpose-
//       Typed DHSL_list object, where T is of class DHSL_list<T>::Link.
//
//----------------------------------------------------------------------------
template<class T>
   class DHSL_list : public DHSL_list<void>
   {
     public:
       typedef DHSL_list<void>                _Base;
       typedef DHSL_list<void>::_Link         _Link;

       class Link : protected _Link
       {
         friend class DHSL_list;
         public:
           T* get_next( void ) const
           {  return static_cast<T*>(_next); }
       }; // class DHSL_list<T>::Link

       //---------------------------------------------------------------------
       // DHSL_list<T>::Constructor/Destructor
       //---------------------------------------------------------------------
       DHSL_list( void ) {}
       ~DHSL_list( void ) {}

       //---------------------------------------------------------------------
       // DHSL_list<T>::Methods
       //---------------------------------------------------------------------
       void
         fifo(                      // Insert (FIFO order)
           T*                link)  // -> Link to insert
       { _Base::fifo(link); }

       T*                           // -> Head T* on List
         get_head( void ) const     // Get head Link
       { return static_cast<T*>(_head); }

       T*                           // -> Tail T* on List
         get_tail( void ) const     // Get tail Link
       { return static_cast<T*>(_tail); }

       void
         insert(                    // Insert at position,
           T*                link,  // -> Link to insert after
           T*                head,  // -> First Link to insert
           T*                tail)  // -> Final Link to insert
       { _Base::insert(link, head, tail); }

       bool                         // TRUE if the object is coherent
          is_coherent( void ) const // Coherency check
       { return _Base::is_coherent(); }

       bool                         // TRUE if Link is contained
         is_on_list(                // Is Link contained?
           T*                link) const  // -> Link
       { return _Base::is_on_list(link); }

       void
         lifo(                      // Insert (LIFO order)
           T*                link)  // -> Link to insert
       { _Base::lifo(link); }

       void
         remove(                    // Remove from DHSL_list
           T*                head,  // -> First Link to remove
           T*                tail)  // -> Final Link to remove
       { _Base::remove(head, tail); }

       T*                           // Removed T*
         remq( void )               // Remove head Link
       {  return static_cast<T*>(_Base::remq()); }

       T*                           // -> The set of removed Links
         reset( void )              // Reset (empty) the List
       { return static_cast<T*>(_Base::reset()); }
   }; // class DHSL_list<T>

//----------------------------------------------------------------------------
//
// Class-
//       List<T>
//
// Purpose-
//       Typed List object, where T is of class List<T>::Link.
//
//----------------------------------------------------------------------------
template<class T> class List : public DHDL_list<T> {};

//----------------------------------------------------------------------------
//
// Class-
//       SHSL_list<T>
//
// Purpose-
//       Typed SHSL_list object, where T is of class SHSL_list<T>::Link.
//
//----------------------------------------------------------------------------
template<class T>
   class SHSL_list : public SHSL_list<void>
   {
     public:
       typedef SHSL_list<void>                _Base; // The base List type
       typedef SHSL_list<void>::_Link         _Link; // The base Link type

       class Link : public _Link
       {
         friend class SHSL_list;
         public:
           T*                       // -> Next Link
             get_next( void ) const // Get next Link
           { return static_cast<T*>(_next); }
       }; // class SHSL_list<T>::Link ----------------------------------------

       //---------------------------------------------------------------------
       // SHSL_list<T>::Constructor/Destructor
       //---------------------------------------------------------------------
       SHSL_list( void ) {}
       ~SHSL_list( void ) {}

       //---------------------------------------------------------------------
       // SHSL_list<T>::Methods
       //---------------------------------------------------------------------
       void
         fifo(                      // Insert (FIFO order)
           T*                link)  // -> Link to insert
       { _Base::fifo(link); }

       T*                           // -> Head T* on List
         get_head( void ) const     // Get head link
       { return static_cast<T*>(_head); }

       void
         insert(                    // Insert at position,
           T*                link,  // -> Link to insert after
           T*                head,  // -> First Link to insert
           T*                tail)  // -> Final Link to insert
       { _Base::insert(link, head, tail); }

       bool                         // TRUE if the object is coherent
         is_coherent( void ) const  // Coherency check
       { return _Base::is_coherent(); }

       bool                         // TRUE if Link is contained
         is_on_list(                // Is Link contained?
           T*                link) const  // -> Link
       { return _Base::is_on_list(link); }

       void
         lifo(                      // Insert (LIFO order)
           T*                link)  // -> Link to insert
       { _Base::lifo(link); }

       void
         remove(                    // Remove from List
           T*                head,  // -> First Link to remove
           T*                tail)  // -> Final Link to remove
       { _Base::remove(head, tail); }

       T*                           // Removed T*
         remq( void )               // Remove head link
       { return static_cast<T*>(_Base::remq()); }

       T*                           // -> The set of removed Links
         reset( void )              // Reset (empty) the List
       {  return static_cast<T*>(_Base::reset()); }
   }; // class SHSL_list<T>
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_LIST_H_INCLUDED
