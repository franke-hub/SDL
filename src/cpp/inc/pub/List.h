//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2023 Frank Eskesen.
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
//       2023/09/21
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
// Implementation notes-
//       See begin()..end() usage warning.
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

   Applications MUST ALWAYS complete an AI_list begin()..end() loop.
   Not doing so can result in more kinds of problems than you might imagine.
   Loop exception handling must either continue the loop, prevent any further
   access of the associated AI_list, or terminate the application.

   Usage warning-
       A multi-thread timing anomaly can and does occur: When using
       `for(auto it= list.begin(); it != list.end(); ++it)`, while between
       processing the `++it` and  the `it != list.end()`, the iterator is in a
       temporary "limbo" state. The iterator is associated with and will
       eventually reference the AI_list, but cannot guarantee that the AI_list
       still exists without application assistance.

       Applications MAY need to add code in destructors of objects that
       contain an AI_list to insure that all iterators have been processed.
       For sample code, see ~/src/cpp/lib/pub/Dispatch.cpp methods
       dispatch::Task::~Task() and dispatch::Task::work(). [Additional code
       was required in ~/src/cpp/dev/Client.cpp and ~/src/cpp/dev/Server.cpp
       where item->post() calls were changed to dispatch::Disp::post() to
       avoid a deadlock condition.]

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
       // AI_list<T>::Constructors/Destructor
       //---------------------------------------------------------------------
       AI_list( void ) = default;   // (Default constructor)
       AI_list(const AI_list<T>&) = delete; // *NO* copy constructor
       AI_list(AI_list<T>&&) = delete; // *NO* move constructor

       ~AI_list( void ) = default;

       //---------------------------------------------------------------------
       // AI_list<T>::Operators
       //---------------------------------------------------------------------
       AI_list& operator=(const AI_list<T>&) = delete; // *NO* copy assignment
       AI_list& operator=(AI_list<T>&&) = delete; // *NO* move assignment

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
       {
         return _tail.load();
       }

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
            if( link == nullptr || (void*)link == &__detail::__end )
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
       //       Test whether any link is present in this List.
       //
       // Implementation notes-
       //       Only the consumer thread can safely use this method.
       //
       //---------------------------------------------------------------------
       bool                         // TRUE if no elements are present in list
         is_empty( void ) const     // Is the list empty?
       {
         return _tail.load() == nullptr;
       }

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
            while( prev != nullptr && (void*)prev != &__detail::__end )
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
       /* DEPRECATED, REMOVED ************************************************
       pointer                      // The set of removed Links
         reset( void )              // Reset (empty) the List
       {
          pointer link= _tail.load();
          while( !_tail.compare_exchange_weak(link, nullptr) )
            ;

          return link;
       }
       ** DEPRECATED, REMOVED ***********************************************/

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
       //       of the empty to non-empty state transition.
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
       pointer                      // The set of removed Links
         reset(                     // Reset (empty) the List
           const void* tail) noexcept // Replacing it with this pseudo-link
       {
          pointer link= _tail.load(); // Get the current tail
          if( link == nullptr )     // If the List is currently empty
            return nullptr;         // Do not replace it

          // If re-replacing the tail with the dummy pseudo-link
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
// Implementation notes-
//       TODO: Consider implementing move constructor and assignment.
//
//----------------------------------------------------------------------------
template<class T>
   class DHDL_list : public DHDL_list<void>
   {
     public:
       typedef T                              value_type;
       typedef T*                             pointer;
       typedef T&                             reference;
       typedef _DHDL_const_iter<value_type>   const_iterator;
       typedef _DHDL_iter<value_type>         iterator;

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
       // DHDL_list<T>::Constructors/Destructor
       //---------------------------------------------------------------------
       DHDL_list( void ) = default;
       DHDL_list(const DHDL_list<T>&) = delete; // *NO* copy constructor
       DHDL_list(DHDL_list<T>&&) = delete; // *NO* move constructor

       ~DHDL_list( void ) = default;

       //---------------------------------------------------------------------
       // DHDL_list<T>::Operators
       //---------------------------------------------------------------------
       DHDL_list& operator=(const DHDL_list<T>&) = delete; // *NO* copy
       DHDL_list& operator=(DHDL_list<T>&&) = delete; // *NO* move assignment

       //---------------------------------------------------------------------
       // DHDL_list<T>::Methods
       //---------------------------------------------------------------------
             iterator begin()       noexcept { return iterator(this); }
       const_iterator begin() const noexcept { return const_iterator(this); }
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
           pointer                after, // -> Link to insert after
           pointer                head, // -> First Link to insert
           pointer                tail) // -> Final Link to insert
       { _Base::insert(after, head, tail); }

       void
         insert(                    // Insert at position,
           pointer                after, // -> Link to insert after
           pointer                link) // -> The Link to insert
       { _Base::insert(after, link, link); }

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

       void
         remove(                    // Remove from list
           pointer                link) // -> The Link to remove
       { _Base::remove(link, link); }

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
// Implementation notes-
//       TODO: Consider implementing move constructor and assignment.
//
//----------------------------------------------------------------------------
template<class T>
   class DHSL_list : public DHSL_list<void>
   {
     public:
       typedef T                              value_type;
       typedef T*                             pointer;
       typedef T&                             reference;
       typedef _DHSL_const_iter<value_type>   const_iterator;
       typedef _DHSL_iter<value_type>         iterator;

       typedef DHSL_list<void>                _Base;
       typedef DHSL_list<void>::_Link         _Link;

       class Link : protected _Link
       {
         friend class DHSL_list;
         public:
           pointer get_next( void ) const
           {  return static_cast<pointer>(_next); }
       }; // class DHSL_list<T>::Link

       //---------------------------------------------------------------------
       // DHSL_list<T>::Constructors/Destructor
       //---------------------------------------------------------------------
       DHSL_list( void ) = default;
       DHSL_list(const DHSL_list<T>&) = delete; // *NO* copy constructor
       DHSL_list(DHSL_list<T>&&) = delete; // *NO* move constructor

       ~DHSL_list( void ) = default;

       //---------------------------------------------------------------------
       // DHSL_list<T>::Operators
       //---------------------------------------------------------------------
       DHSL_list& operator=(const DHSL_list<T>&) = delete; // *NO* copy
       DHSL_list& operator=(DHSL_list<T>&&) = delete; // *NO* move assignment

       //---------------------------------------------------------------------
       // DHSL_list<T>::Methods
       //---------------------------------------------------------------------
             iterator begin()       noexcept { return iterator(this); }
       const_iterator begin() const noexcept { return const_iterator(this); }
             iterator end()         noexcept { return iterator(); }
       const_iterator end()   const noexcept { return const_iterator(); }

       void
         fifo(                      // Insert (FIFO order)
           pointer           link)  // -> Link to insert
       { _Base::fifo(link); }

       pointer                      // -> Head T* on List
         get_head( void ) const     // Get head Link
       { return static_cast<pointer>(_head); }

       pointer                      // -> Tail T* on List
         get_tail( void ) const     // Get tail Link
       { return static_cast<pointer>(_tail); }

       void
         insert(                    // Insert at position,
           pointer           link,  // -> Link to insert after
           pointer           head,  // -> First Link to insert
           pointer           tail)  // -> Final Link to insert
       { _Base::insert(link, head, tail); }

       bool                         // TRUE if the object is coherent
          is_coherent( void ) const // Coherency check
       { return _Base::is_coherent(); }

       bool                         // TRUE if Link is contained
         is_on_list(                // Is Link contained?
           pointer           link) const  // -> Link
       { return _Base::is_on_list(link); }

       void
         lifo(                      // Insert (LIFO order)
           pointer           link)  // -> Link to insert
       { _Base::lifo(link); }

       void
         remove(                    // Remove from DHSL_list
           pointer           head,  // -> First Link to remove
           pointer           tail)  // -> Final Link to remove
       { _Base::remove(head, tail); }

       void
         remove(                    // Remove from list
           pointer                link) // -> The Link to remove
       { _Base::remove(link, link); }

       pointer                      // Removed T*
         remq( void )               // Remove head Link
       {  return static_cast<pointer>(_Base::remq()); }

       pointer                      // -> The set of removed Links
         reset( void )              // Reset (empty) the List
       { return static_cast<pointer>(_Base::reset()); }
   }; // class DHSL_list<T>

//----------------------------------------------------------------------------
//
// Class-
//       SHSL_list<T>
//
// Purpose-
//       Typed SHSL_list object, where T is of class SHSL_list<T>::Link.
//
// Implementation notes-
//       TODO: Consider implementing move constructor and assignment.
//
//----------------------------------------------------------------------------
template<class T>
   class SHSL_list : public SHSL_list<void>
   {
     public:
       typedef T                              value_type;
       typedef T*                             pointer;
       typedef T&                             reference;
       typedef _SHSL_iter<value_type>         iterator;
       typedef _SHSL_const_iter<value_type>   const_iterator;

       typedef SHSL_list<void>                _Base; // The base List type
       typedef SHSL_list<void>::_Link         _Link; // The base Link type

       class Link : public _Link
       {
         friend class SHSL_list;
         public:
           pointer                  // -> Prev Link
             get_prev( void ) const // Get prev Link
           { return static_cast<pointer>(_prev); }
       }; // class SHSL_list<T>::Link ----------------------------------------

       //---------------------------------------------------------------------
       // SHSL_list<T>::Constructor/Destructor
       //---------------------------------------------------------------------
       SHSL_list( void ) = default;
       SHSL_list(const SHSL_list<T>&) = delete; // *NO* copy constructor
       SHSL_list(SHSL_list<T>&&) = delete; // *NO* move constructor

       ~SHSL_list( void ) = default;

       //---------------------------------------------------------------------
       // SHSL_list<T>::Operators
       //---------------------------------------------------------------------
       SHSL_list& operator=(const SHSL_list<T>&) = delete; // *NO* copy
       SHSL_list& operator=(SHSL_list<T>&&) = delete; // *NO* move assignment

       //---------------------------------------------------------------------
       // SHSL_list<T>::Methods
       //---------------------------------------------------------------------
               iterator begin()       noexcept { return iterator(this); }
         const_iterator begin() const noexcept { return const_iterator(this); }
               iterator end()         noexcept { return iterator(); }
         const_iterator end()   const noexcept { return const_iterator(); }

/***** [[deprecated("Use lifo to insert and begin/end to iterate")]] *********
       void
         fifo(                      // Insert (FIFO order)
           pointer           link)  // -> Link to insert
       { _Base::fifo(link); }
*****************************************************************************/

       pointer                      // -> Tail T* on List
         get_tail( void ) const     // Get tail link
       { return static_cast<pointer>(_tail); }

       void
         insert(                    // Insert at position,
           pointer           link,  // -> Link to insert after
           pointer           head,  // -> First Link to insert
           pointer           tail)  // -> Final Link to insert
       { _Base::insert(link, head, tail); }

       bool                         // TRUE if the object is coherent
         is_coherent( void ) const  // Coherency check
       { return _Base::is_coherent(); }

       bool                         // TRUE if Link is contained
         is_on_list(                // Is Link contained?
           pointer           link) const  // -> Link
       { return _Base::is_on_list(link); }

       void
         lifo(                      // Insert (LIFO order)
           pointer           link)  // -> Link to insert
       { _Base::lifo(link); }

/***** [[deprecated("No use case and it takes linear time")]] ****************
       void
         remove(                    // Remove from List
           pointer           head,  // -> First Link to remove
           pointer           tail)  // -> Final Link to remove
       { _Base::remove(head, tail); }
*****************************************************************************/

       pointer                      // Removed T*
         remq( void )               // Remove TAIL link
       { return static_cast<pointer>(_Base::remq()); }

       pointer                      // -> The set of removed Links
         reset( void )              // Reset (empty) the List
       {  return static_cast<pointer>(_Base::reset()); }
   }; // class SHSL_list<T>

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
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_LIST_H_INCLUDED
