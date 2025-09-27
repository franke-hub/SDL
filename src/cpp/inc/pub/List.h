//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2025 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
// SPDX-License-Identifier: LGPL-3.0-only
//----------------------------------------------------------------------------
//
// Title-
//       List.h
//
// Purpose-
//       Describe the List objects.
//
// Last change date-
//       2025/09/23
//
// Implementation notes-
//       "Link set" refers to the set of Links owned by a List.
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_LIST_H_INCLUDED
#define _LIBPUB_LIST_H_INCLUDED

#include <atomic>                   // For std::atomic
#include <functional>               // For std::less<>

#include "pub/bits/pubconfig.h"     // For _LIBPUB_ macros
#include "bits/List.h"              // For List template implementations, ...

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
/** **************************************************************************
   @brief The AI_list is an atomic container with fixed time element insertion
   and iterator extraction.

   @tparam Type (template) of the element, which *must* be a subclass of
     AI_list<T>::Link.

   @details
   ~/doc/_ALGORITHMS/AI_list.md contains additional information.

************************************************************************** **/
template<class T>
   class AI_list
   {
     public:
       typedef T                              value_type;
       typedef value_type*                    pointer;
       typedef value_type&                    reference;

       typedef AI_list<T>                     _Self;
       typedef AI_iter<T>                     iterator;

       struct Link : protected __detail::_BISL_link
       {
         friend struct AI_iter<T>;
         friend class AI_list;
         public:
           pointer get_next( void ) const
           { return static_cast<pointer>(_link); }
           pointer get_prev( void ) const
           { return static_cast<pointer>(_link); }
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

       ~AI_list( void ) { AI_list<void>::verify_nullptr(_tail.load()); }

       //---------------------------------------------------------------------
       // AI_list<T>::Operators
       //---------------------------------------------------------------------
       AI_list& operator=(const AI_list<T>&) = delete; // *NO* copy assignment
       AI_list& operator=(AI_list<T>&&) = delete; // *NO* move assignment

       /** *******************************************************************
         @brief [linear time] Create a begin iterator

         @details
         The iterator's Link set uses FIFO ordering.

         Not thread safe: The begin .. end iterator sequence may only be used
         by one thread at a time.
       ******************************************************************* **/
       iterator begin() noexcept { return iterator(this); }

       /** *******************************************************************
         @brief [constant time] Create an end (comparison) iterator
       ******************************************************************* **/
       iterator end()   noexcept { return iterator(); }

       /**
         @brief [constant time] Thread-safe FIFO ordered atomic Link insertion
         @param link The Link to insert.

         @details
         While the AI_list itself has LIFO ordering, the begin() iterator
         maintains Links in FIFO order.

         Thread safety: Any number of threads may simultaneously invoke this
         method.
       **/
       pointer                      // -> Prior tail
         fifo(                      // Insert (fifo order)
           pointer     link)        // -> Link to insert
       {
          pointer prev= _tail.load();
          link->_link= prev;
          while( !_tail.compare_exchange_weak(prev, link) )
            link->_link= prev;

          return prev;
       }

       /** *******************************************************************
         @brief [constant time] Access the tail link

         @details
         This returns an instantaneous pointer to the current tail link.
         Note that this may be obsolete by the time the method returns.
       ******************************************************************* **/
       pointer                      // -> Tail Link
         get_tail( void ) const     // Get tail link
       {
         return _tail.load();
       }

       /** *******************************************************************
         @brief [linear time] AI_link coherency check

         @details
         This only verifies that the AI_list Link set is finite.
       ******************************************************************* **/
       bool                         // TRUE if the Link set is coherent
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

       /** *******************************************************************
         @brief [constant time] Test whether the AI_list is empty.

         @details
         This returns an instantaneous check whether the AI_list is empty.
         Note that the AI_list state can change before this method returns.
       ******************************************************************* **/
       bool                         // TRUE if no elements are present in list
         is_empty( void ) const     // Is the list empty?
       {
         return _tail.load() == nullptr;
       }

       /** *******************************************************************
         @brief [linear time] Test whether a Link is included in the current
         Link set.
       ******************************************************************* **/
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

       /**
         @brief [constant time] Atomically replace the List

         @param (const void*) tail List replacement tail.

         @return The set of removed links (tail->pointer->...->nullptr)

         @details
         When used internally, the tail parameter is a (constant) pseudo-link.
         When it replaces the list, it becomes the head (oldest) Link and,
         since it will always remain the oldest Link, can be simultaneously
         used by multiple AI_lists.
       **/
       pointer                      // The set of removed Links
         reset(                     // Reset (replace) the List set with
           const void* tail) noexcept // This replacement pseudo-Link
       {
          pointer link= _tail.load(); // Get the current tail
          if( link == nullptr )     // If the List is currently empty
            return nullptr;         // Do not replace it

          // If the Link set hasn't changed since it was replaced, we're done
          while( (void*)link == tail )
          {
            if( _tail.compare_exchange_weak(link, nullptr) )
              return nullptr;
          }

          // The Link set changed. Replace it with the pseudo-link
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

       class Link : protected _Link
       {
         friend class DHDL_list;
         public:
           pointer get_next( void ) const
           { return static_cast<pointer>(_next); }

           pointer get_prev( void ) const
           { return static_cast<pointer>(_prev); }

           // Note: DHDL_list<void>::operator<() compares Link addresses, i.e.
           // bool operator<(const Link& that) const
           // { return this < &that; }
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
         debug(const char* info= "") const // Debugging display
       { _Base::debug(info); }

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

       /** *******************************************************************
         @brief Insert link at position.

         @param link The Link *before* the insert position, nullptr for head.
         @param head The first Link to insert.
         @param tail The last Link to insert.

         Preconditions: The head to tail chain must be well-formed.
         Postcondition: link->_next == head, head->_prev == link,
                        tail->_next == link->next, tail->_next->_prev == tail
       ******************************************************************* **/
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

       size_t                       // The _Link count
         size( void ) const         // Get the _Link count
       { return _Base::size(); }
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
//       TODO: Consider implementing begin..end
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
//       SORT_list<T>
//
// Purpose-
//       Typed SORT_list object, where T is of class SORT_list<T>::Link.
//
// Implementation notes-
//       TODO: Consider implementing move constructor and assignment.
//
//----------------------------------------------------------------------------
template<class T>
   class SORT_list : public SORT_list<void>
   {
     public:
       typedef T                              value_type;
       typedef T*                             pointer;
       typedef T&                             reference;
       typedef _SORT_const_iter<value_type>   const_iterator;
       typedef _SORT_iter<value_type>         iterator;
       typedef SORT_list<void>                _Base;

       class Link : protected _Link
       {
         friend class SORT_list;
         public:
           typedef SORT_list<void>::_Link      Base;

           pointer get_next( void ) const
           { return static_cast<pointer>(_next); }

           pointer get_prev( void ) const
           { return static_cast<pointer>(_prev); }

           // OVERRIDE this method
           virtual bool operator<(const Base& that) const override = 0;
       }; // class SORT_list<T>::Link

       //---------------------------------------------------------------------
       // SORT_list<T>::Constructors/Destructor
       //---------------------------------------------------------------------
       SORT_list( void ) = default;
       SORT_list(const SORT_list<T>&) = delete; // *NO* copy constructor
       SORT_list(SORT_list<T>&&) = delete; // *NO* move constructor

       ~SORT_list( void ) = default;

       //---------------------------------------------------------------------
       // SORT_list<T>::Operators
       //---------------------------------------------------------------------
       SORT_list& operator=(const SORT_list<T>&) = delete; // *NO* copy
       SORT_list& operator=(SORT_list<T>&&) = delete; // *NO* move assignment

       //---------------------------------------------------------------------
       // SORT_list<T>::Methods
       //---------------------------------------------------------------------
             iterator begin()       noexcept { return iterator(this); }
       const_iterator begin() const noexcept { return const_iterator(this); }
             iterator end()         noexcept { return iterator(); }
       const_iterator end()   const noexcept { return const_iterator(); }

       void
         debug(const char* info= "") const // Debugging display
       { _Base::debug(info); }

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

       /** *******************************************************************
         @brief Insert link at position.

         @param link The Link *before* the insert position, nullptr for head.
         @param head The first Link to insert.
         @param tail The last Link to insert.

         Preconditions: The head to tail chain must be well-formed.
         Postcondition: link->_next == head, head->_prev == link,
                        tail->_next == link->next, tail->_next->_prev == tail
       ******************************************************************* **/
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

       size_t                       // The _Link count
         size( void ) const         // Get the _Link count
       { return _Base::size(); }

       /** *******************************************************************
         @brief Sort the SORT_list

         Sorts the list using the Merge sort algorithm and using
         "Link::operator<(const Link& that) const" to compare Links.
       ******************************************************************* **/
       void sort( void )            // Sort the Links
       { _Base::sort(); }
       }; // class SORT_list<T>

//----------------------------------------------------------------------------
//
// Class-
//       List<T> (Alias of DHDL_list)
//
// Purpose-
//       Typed List object, where T is of class List<T>::Link.
//
//----------------------------------------------------------------------------
template<class T> class List : public DHDL_list<T> {};

//----------------------------------------------------------------------------
//
// Class-
//       Sort_list<T> (Alias of SORT_list)
//
// Purpose-
//       Typed Sort_list object, where T is of class Sort_list<T>::Link.
//
//----------------------------------------------------------------------------
template<class T> class Sort_list : public SORT_list<T> {};
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_LIST_H_INCLUDED
