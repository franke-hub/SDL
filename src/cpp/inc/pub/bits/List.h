//----------------------------------------------------------------------------
//
//       Copyright (c) 2023-2025 Frank Eskesen.
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
//       bits/List.h
//
// Purpose-
//       ../List.h template definitions and internal base classes.
//
// Last change date-
//       2025/09/20
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_BITS_LIST_H_INCLUDED
#define _LIBPUB_BITS_LIST_H_INCLUDED
/** @file pub/bits/List.h
**  This is an internal header file, included by other library headers.
**  Do not attempt to use it directly.
**/
#include <stdexcept>                // For std::domain_error

#include <pub/utility.h>            // For pub::utility::checkstop

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
template<class T> class AI_list;    // Atomic Insert list
template<class T> class DHDL_list;  // Doubly Headed Doubly Linked list
template<class T> class DHSL_list;  // Doubly Headed Singly Linked list
template<class T> class SHSL_list;  // Singly Headed Singly Linked list
template<class T> class List;       // List (Is a DHDL_list)

namespace __detail
{
   /// Compile-time constants
   enum
   { HCDM= false                    // Hard Core Debug Mode?
   , MAX_COHERENT= 1'000'000'000    // Maximum coherent List element count
   }; // generic enum

   /// Exceptions
   /**
     @brief Exception __detail::end_dereferenced

     In lieu of undefined behavior, this std::domain_error exception is thrown
     when an end() iterator is dereferenced.
   **/
   class end_dereferenced : public std::domain_error
   {
     public:
       using std::domain_error::domain_error;
       end_dereferenced() : domain_error("end() dereferenced") {}
   }; // class end_dereferenced

   /// Common parts of a bidirectional doubly linked link
   struct _BIDL_link
   {
     typedef _BIDL_link                       _Self;

     _Self* _next= nullptr;
     _Self* _prev= nullptr;

     bool operator<(const _Self& that) const // (Default: sort by address)
     { return this < &that; }
   }; // _BIDL_link

   /// Common parts of a bidirectional singly linked link
   ///   AI_list Links use LIFO ordering; AI_iter Links use FIFO ordering
   struct _BISL_link
   {
     typedef _BISL_link                       _Self;
     _Self* _link= nullptr;
   }; // _BISL_link

   /// Common parts of a forward link
   struct _NEXT_link
   {
     typedef _NEXT_link                       _Self;
     _Self* _next= nullptr;
   };

   /// Common parts of a reverse link
   struct _PREV_link
   {
     typedef _PREV_link                       _Self;
     _Self* _prev= nullptr;
   };

   /// __detail::end: An end of list pseudo-link, used internally.
   extern const void* __end;
} // namespace __detail

//----------------------------------------------------------------------------
//
// Class-
//       AI_list<void>
//
// Purpose-
//       AI_list static methods
//
//----------------------------------------------------------------------------
template<> class AI_list<void>
   {
     public:
       /** *******************************************************************
         @brief Iterator error: begin but pseudo-link present

         This is a fatal error.
       ******************************************************************* **/
       [[noreturn]]
       static void
       iterator_begin_error( void ); // Begin but pseudo-link present

       /** *******************************************************************
         @brief Iterator error: operator++ but pseudo-link missing

         This is a fatal error.
       ******************************************************************* **/
       [[noreturn]]
       static void
       iterator_increment_error( void ); // AI_iter++ but pseudo-link missing

       /** *******************************************************************
         @brief [constant time] Verify that the AI_list is empty

         @param tail The current tail pointer
       ******************************************************************* **/
       static void
       verify_nullptr(void* tail);  // Verify: Link must be nullptr
   }; // AI_list<void>

/** **************************************************************************
   @brief An AI_list<T> iterator

   @tparam T The (template) type of the element, which *must* be a subclass of
     AI_list<T>::Link.

   @details
   There isn't any AI_const_iter.

   This iterator presents Links to the application in FIFO order, i.e. in the
   order they were enqueued. The implementation uses get_next() to refer to
   Links on its FIFO list.

   While this iterates in a forward direction, it cannot be a forward_iterator
   because it is not a multi-pass iterator. Incrementing the iterator removes
   the Link from the iterator.
************************************************************************** **/
template<typename T>
   struct AI_iter
   {
     typedef ptrdiff_t                        difference_type;
     typedef std::input_iterator_tag          iterator_category;
     typedef T                                value_type;
     typedef T*                               pointer;
     typedef T&                               reference;

     typedef T                                _Link;
     typedef AI_list<T>                       _List;
     typedef AI_iter<T>                       _Self;

     pointer      _head= nullptr;   // The remaining _Links (in FIFO order)
     pointer      _link= nullptr;   // The current T*
     _List* const _list= nullptr;   // The associated List<T>*

     /** *********************************************************************
       @brief [constant time] The default [end()] constructor
     ********************************************************************* **/
     AI_iter() noexcept = default;  // Default, end() constructor

     /** *********************************************************************
       @brief [constant time] The copy constructor
     ********************************************************************* **/
     AI_iter(const AI_iter& that) noexcept
     : _head(that._head), _link(that._link), _list(that._list) {}

     /** *********************************************************************
       @brief [linear time] The begin() constructor

       @details
       This constructor *REMOVES* all elements from the list replacing the
       list with a single pseudo-link, the constant &__detail::__end.

       The set of removed links are owned by this iterator. When they were
       on the AI_list, the links had LIFO ordering. This constructor inverts
       the list, converting it into FIFO ordering.
     ********************************************************************* **/
     explicit
     AI_iter(_List* list) noexcept
     :  _list(list)
     {
       T* tail= _list->reset(&__detail::__end);
       while( tail )
       {
         if( (void*)tail == &__detail::__end ) // (Should not occur)
           AI_list<void>::iterator_begin_error();
         T* prev= tail->get_prev();
         tail->_link= _head;
         _head= tail;
         tail= prev;
       }
       _link= _head;
       if( _head )
         _head= _head->get_prev();
     }

     /** *********************************************************************
       @brief [constant time] Get the current iteration Link

       @details
       A nullptr is returned when used on an end() iterator.
     ********************************************************************* **/
     pointer
     get() const noexcept
     { return _link; }

     /** *********************************************************************
       @brief [constant time] Returns true iff the current iteration Link != end()
     ********************************************************************* **/
     operator bool()
     { return bool(_link); }

     /** *********************************************************************
       @brief [constant time] Dereference the current iteration Link

       @details
       An "end_dereferenced" error is thrown when used on an end() iterator.
     ********************************************************************* **/
     reference
     operator*() const
     { if( _link )
         return (reference)(*_link);
       throw __detail::end_dereferenced();
     }

     /** *********************************************************************
       @brief [constant time] Address the current iteration Link

       @details
       An "end_dereferenced" error is thrown when used on an end() iterator.
     ********************************************************************* **/
     pointer
     operator->() const
     { if( _link )
         return _link;
       throw __detail::end_dereferenced();
     }

     /** *********************************************************************
       @brief [variable time] Increment the iterator (prefix notation)

       @details
       An "end_dereferenced" error is thrown when used on an end() iterator.

       This is usually a constant time operation.
       However, if: 1) We've emptied the AI_iter's Link set and 2) New Links
       were added to the AI_list's Link set while emptying the AI_iter's set.
       (We refill the AI_iter's Link set using the newly added Links, taking
       linear time.)
     ********************************************************************* **/
     _Self&
     operator++() noexcept
     {
       if( _head ) {
         _link= _head;
         _head= _head->get_next();
       } else {
         _link= nullptr;
         pointer tail= _list->reset(&__detail::__end);
         if( tail )
         {
           do
           {
             pointer prev= tail->get_prev();
             if( prev == nullptr )  // (Should not occur)
               AI_list<void>::iterator_increment_error();
             tail->_link= _head;
             _head= tail;
             tail= prev;
           } while( (void*)tail != &__detail::__end );
           _link= _head;
           _head= _head->get_next();
         }
       }
       return *this;
     }

     /** *********************************************************************
       @brief [variable time] Increment the iterator (postfix notation)

       @details
       An "end_dereferenced" error is thrown when used on an end() iterator.
     ********************************************************************* **/
     _Self
     operator++(int) noexcept
     {
       _Self __tmp = *this;
       operator++();
       return __tmp;
     }

     /** *********************************************************************
       @brief [constant time] Iterator equality comparison
     ********************************************************************* **/
     friend bool
     operator==(const _Self& lhs, const _Self& rhs) noexcept
     { return lhs._link == rhs._link; }

     /** *********************************************************************
       @brief [constant time] Iterator inequality comparison
     ********************************************************************* **/
     friend bool
     operator!=(const _Self& lhs, const _Self& rhs) noexcept
     { return lhs._link != rhs._link; }
   }; // struct AI_iter<T>

//----------------------------------------------------------------------------
//
// Class-
//       DHDL_list<>, aka List<>
//
// Purpose-
//       A Doubly Headed Doubly linked list.
//
//----------------------------------------------------------------------------
/**
   @brief A DHDL_list::iterator.
**/
template<typename T>
   struct _DHDL_iter
   {
     typedef ptrdiff_t                        difference_type;
     typedef std::bidirectional_iterator_tag  iterator_category;
     typedef T                                value_type;
     typedef T*                               pointer;
     typedef T&                               reference;

     typedef T                                _Link;
     typedef DHDL_list<T>                     _List;
     typedef _DHDL_iter<T>                    _Self;

     pointer _link;

     _DHDL_iter() noexcept
     : _link(nullptr) {}

     explicit
     _DHDL_iter(_List* list) noexcept
     : _link(list->get_head()) {}

     _Self
     _const_cast() const noexcept
     { return *this; }

     pointer
     get() const noexcept
     { return _link; }

     operator bool() const noexcept
     { return bool(_link); }

     reference
     operator*() const
     { if( _link )
         return *((pointer)_link);
       throw __detail::end_dereferenced();
     }

     pointer
     operator->() const
     { if( _link )
         return (pointer)_link;
       throw __detail::end_dereferenced();
     }

     _Self&
     operator++() noexcept
     {
       if( _link )
         _link = _link->get_next();
       return *this;
     }

     _Self
     operator++(int) noexcept
     {
       _Self __tmp = *this;
       if( _link )
         _link = _link->get_next();
       return __tmp;
     }

     _Self&
     operator--() noexcept
     {
       if( _link )
         _link = _link->_prev;
       return *this;
     }

     _Self
     operator--(int) noexcept
     {
       _Self __tmp = *this;
       if( _link )
         _link = _link->_prev;
       return __tmp;
     }

     friend bool
     operator==(const _Self& lhs, const _Self& rhs) noexcept
     { return lhs._link == rhs._link; }

     friend bool
     operator!=(const _Self& lhs, const _Self& rhs) noexcept
     { return lhs._link != rhs._link; }
   }; // _DHDL_iter

/**
   @brief A DHDL_list::const_iterator.
*/
template<typename T>
   struct _DHDL_const_iter
   {
     typedef ptrdiff_t                        difference_type;
     typedef std::bidirectional_iterator_tag  iterator_category;
     typedef T                                value_type;
     typedef const T*                         pointer;
     typedef const T&                         reference;

     typedef T                                _Link;
     typedef DHDL_list<T>                     _List;
     typedef _DHDL_const_iter<T>              _Self;

     pointer _link;

     _DHDL_const_iter() noexcept
     : _link(nullptr) {}

     explicit
     _DHDL_const_iter(const _List* list) noexcept
     : _link(list->get_head()) {}

     _DHDL_const_iter(const _DHDL_iter<T>& _it) noexcept
     : _link(_it._link) {}

     _DHDL_const_iter(const _DHDL_const_iter& _it) noexcept
     : _link(_it._link) {}

     pointer
     get() const noexcept
     { return _link; }

     operator bool() const noexcept
     { return bool(_link); }

     reference
     operator*() const
     { if( _link )
         return *static_cast<pointer>(_link);
       throw __detail::end_dereferenced();
     }

     pointer
     operator->() const
     { if( _link )
         return static_cast<pointer>(_link);
       throw __detail::end_dereferenced();
     }

     _Self&
     operator++() noexcept
     {
       if( _link )
         _link = _link->_next;
       return *this;
     }

     _Self
     operator++(int) noexcept
     {
       _Self __tmp = *this;
       if( _link )
         _link = _link->_next;
       return __tmp;
     }

     _Self&
     operator--() noexcept
     {
       if( _link )
         _link = _link->_prev;
       return *this;
     }

     _Self
     operator--(int) noexcept
     {
       _Self __tmp = *this;
       if( _link )
         _link = _link->_prev;
       return __tmp;
     }

     friend bool
     operator==(const _Self& lhs, const _Self& rhs) noexcept
     { return lhs._link == rhs._link; }

     friend bool
     operator!=(const _Self& lhs, const _Self& rhs) noexcept
     { return lhs._link != rhs._link; }
   }; // _DHDL_const_iter

//----------------------------------------------------------------------------
//
// Class-
//       DHDL_list<void>
//
// Purpose-
//       The Doubly Headed, Doubly Linked List is a general purpose List.
//
// Implementation notes-
//       The DHDL_list is not thread safe. Method usage must be serialized.
//       The FIFO, LIFO, INSERT, and REMOVE methods run in constant time.
//
//----------------------------------------------------------------------------
template<> class DHDL_list<void>
   {
     public:
       typedef __detail::_BIDL_link           value_type;
       typedef value_type*                    pointer;
       typedef value_type&                    reference;

       typedef __detail::_BIDL_link           _Link;

     protected:
       //---------------------------------------------------------------------
       // DHDL_list<void>::Attributes
       //---------------------------------------------------------------------
       _Link* _head= nullptr;
       _Link* _tail= nullptr;

       //---------------------------------------------------------------------
       // DHDL_list<void>::Constructors/Destructor
       //---------------------------------------------------------------------
       DHDL_list( void ) = default;
       DHDL_list(const DHDL_list&) = delete; // *NO* copy constructor
       DHDL_list(DHDL_list&&) = delete; // *NO* move constructor

       ~DHDL_list( void ) = default;

       //---------------------------------------------------------------------
       // DHDL_list<void>::Operators
       //---------------------------------------------------------------------
       DHDL_list& operator=(const DHDL_list&) = delete; // *NO* copy assignment
       DHDL_list& operator=(DHDL_list&&) = delete; // *NO* move assignment

       //---------------------------------------------------------------------
       //
       // Method-
       //       DHDL_list<void>::begin
       //       DHDL_list<void>::end
       //
       // Purpose-
       //       Create an DHDL_list FIFO iterator.
       //       Create an DHDL_list FIFO end() iterator.
       //
       //---------------------------------------------------------------------
       // Implemented in DHDL_list<T>

       //---------------------------------------------------------------------
       //
       // Method-
       //       DHDL_list<void>::fifo
       //
       // Purpose-
       //       Insert a _Link onto the list with FIFO ordering.
       //
       //---------------------------------------------------------------------
       void fifo(_Link* link);

       //---------------------------------------------------------------------
       //
       // Method-
       //       DHDL_list<void>::get_head
       //       DHDL_list<void>::get_tail
       //
       // Purpose-
       //       Get the head _Link. (Implemented in DHDL_list<T>, not here,)
       //       Get the tail _Link. (Implemented in DHDL_list<T>, not here.)
       //
       //---------------------------------------------------------------------

       //---------------------------------------------------------------------
       //
       // Method-
       //       DHDL_list<void>::insert
       //
       // Purpose-
       //       Insert a chain of elements onto the list at the specified
       //       position.
       //
       //---------------------------------------------------------------------
       void
         insert(                    // Insert at position,
           _Link*             link, // -> _Link to insert after
           _Link*             head, // -> First _Link to insert
           _Link*             tail); // -> Final _Link to insert

       //---------------------------------------------------------------------
       //
       // Method-
       //       DHDL_list<void>::is_coherent
       //
       // Purpose-
       //       List coherency check.
       //
       //---------------------------------------------------------------------
       bool                         // TRUE if the object is coherent
         is_coherent( void ) const; // Coherency check

       //---------------------------------------------------------------------
       //
       // Method-
       //       DHDL_list<void>::is_on_list
       //
       // Purpose-
       //       Test whether _Link is present in this List.
       //
       //---------------------------------------------------------------------
       bool                         // TRUE if _Link is contained
         is_on_list(                // Is _Link contained?
           _Link*             link) const; // -> _Link

       //---------------------------------------------------------------------
       //
       // Method-
       //       DHDL_list<void>::lifo
       //
       // Purpose-
       //       Insert a _Link onto the list with LIFO ordering.
       //
       //---------------------------------------------------------------------
       void
         lifo(                      // Insert (LIFO order)
           _Link*             link); // -> _Link to insert

       //---------------------------------------------------------------------
       //
       // Method-
       //       DHDL_list<void>::remove
       //
       // Purpose-
       //       Remove a chain of elements from the list.
       //
       //---------------------------------------------------------------------
       void
         remove(                    // Remove from list
           _Link*             head, // -> First _Link to remove
           _Link*             tail); // -> Final _Link to remove

       //---------------------------------------------------------------------
       //
       // Method-
       //       DHDL_list<void>::remq
       //
       // Purpose-
       //       Remove the head _Link from the List.
       //
       //---------------------------------------------------------------------
       _Link*                       // -> Removed _Link
         remq( void );              // Remove head _Link

       //---------------------------------------------------------------------
       //
       // Method-
       //       DHDL_list<void>::reset
       //
       // Purpose-
       //       Remove ALL _Links from the List.
       //
       //---------------------------------------------------------------------
       _Link*                       // The set of removed _Links
         reset( void );             // Reset (empty) the list

       //---------------------------------------------------------------------
       //
       // Method-
       //       DHDL_list<void>::size
       //
       // Purpose-
       //       Count the _Links
       //
       //---------------------------------------------------------------------
       size_t                       // The _Link count
         size( void ) const;        // Get the _Link count

       //---------------------------------------------------------------------
       //
       // Method-
       //       DHDL_list<void>::sort
       //
       // Purpose-
       //       Sort the List.
       //
       //---------------------------------------------------------------------
       void sort( void );           // Sort the Links

       //---------------------------------------------------------------------
       //
       // Method-
       //       DHDL_list<void>::sort_split
       //       DHDL_list<void>::sort_merge
       //
       // Purpose-
       //       Split the _Link segment.
       //       Merge the _Link segments.
       //
       //---------------------------------------------------------------------
       _Link*                       // The set of removed _Links (head)
         sort_split(                // Split the _Link set
           _Link*&            top,  // INP: The head element
                                    // OUT: The remaining _Links (tail)
           size_t             N);   // The number of _Links to remove

       std::pair<_Link*,_Link*>     // The combined {L,R} _Link set
         sort_merge(                // Merge
           _Link*             L,    // The left _Link set
           _Link*             R);   // The Right _Link set
   }; // class DHDL_list<void>

//----------------------------------------------------------------------------
//
// Class-
//       DHSL_list<>
//
// Purpose-
//       A Doubly Headed Singly linked list.
//
//----------------------------------------------------------------------------
/**
   @brief A DHSL_list::iterator.
**/
template<typename T>
   struct _DHSL_iter
   {
     typedef ptrdiff_t                        difference_type;
     typedef std::forward_iterator_tag        iterator_category;
     typedef T                                value_type;
     typedef T*                               pointer;
     typedef T&                               reference;

     typedef T                                _Link;
     typedef DHSL_list<T>                     _List;
     typedef _DHSL_iter<T>                    _Self;

     pointer _link;

     _DHSL_iter() noexcept
     : _link(nullptr) {}

     explicit
     _DHSL_iter(_List* list) noexcept
     : _link(list->get_head()) {}

     _Self
     _const_cast() const noexcept
     { return *this; }

     pointer
     get() const noexcept
     { return _link; }

     operator bool() const noexcept
     { return bool(_link); }

     reference
     operator*() const
     { if( _link )
         return *_link;
       throw __detail::end_dereferenced();
     }

     pointer
     operator->() const
     { if( _link )
         return _link;
       throw __detail::end_dereferenced();
     }

     _Self&
     operator++() noexcept
     {
       if( _link )
         _link = _link->get_next();
       return *this;
     }

     _Self
     operator++(int) noexcept
     {
       _Self __tmp = *this;
       if( _link )
         _link = _link->get_next();
       return __tmp;
     }

     friend bool
     operator==(const _Self& lhs, const _Self& rhs) noexcept
     { return lhs._link == rhs._link; }

     friend bool
     operator!=(const _Self& lhs, const _Self& rhs) noexcept
     { return lhs._link != rhs._link; }
   }; // _DHSL_iter

/**
   @brief A DHSL_list::const_iterator.
*/
template<typename T>
   struct _DHSL_const_iter
   {
     typedef ptrdiff_t                        difference_type;
     typedef std::forward_iterator_tag        iterator_category;
     typedef T                                value_type;
     typedef const T*                         pointer;
     typedef const T&                         reference;

     typedef T                                _Link;
     typedef DHSL_list<T>                     _List;
     typedef _DHSL_const_iter<T>              _Self;

     pointer _link;

     _DHSL_const_iter() noexcept
     : _link(nullptr) {}

     explicit
     _DHSL_const_iter(const _List* list) noexcept
     : _link(list->get_head()) {}

     _DHSL_const_iter(const _DHSL_iter<T>& _it) noexcept
     : _link(_it._link) {}

     _DHSL_const_iter(const _DHSL_const_iter<T>& _it) noexcept
     : _link(_it._link) {}

     pointer
     get() const noexcept
     { return _link; }

     operator bool() const noexcept
     { return bool(_link); }

     reference
     operator*() const noexcept
     { if( _link )
         return *_link;
       throw __detail::end_dereferenced();
     }

     pointer
     operator->() const noexcept
     { if( _link )
         return _link;
       throw __detail::end_dereferenced();
     }

     _Self&
     operator++() noexcept
     {
       if( _link )
         _link = _link->_next;
       return *this;
     }

     _Self
     operator++(int) noexcept
     {
       _Self __tmp = *this;
       if( _link )
         _link = _link->_next;
       return __tmp;
     }

     friend bool
     operator==(const _Self& lhs, const _Self& rhs) noexcept
     { return lhs._link == rhs._link; }

     friend bool
     operator!=(const _Self& lhs, const _Self& rhs) noexcept
     { return lhs._link != rhs._link; }
   }; // _DHSL_const_iter

//----------------------------------------------------------------------------
//
// Class-
//       DHSL_list<void>
//
// Purpose-
//       The Doubly Headed, Singly Linked List.
//
// Implementation notes-
//       The DHSL_list is not thread safe. Method usage must be serialized.
//
//       The FIFO, LIFO, REMQ, and RESET methods run in constant time.
//       The INSERT and REMOVE methods run in linear time.
//
//----------------------------------------------------------------------------
template<> class DHSL_list<void>
   {
     public:
       typedef __detail::_NEXT_link           value_type;
       typedef value_type*                    pointer;
       typedef value_type&                    reference;

       typedef __detail::_NEXT_link           _Link;

     protected:
       //---------------------------------------------------------------------
       // DHSL_list<void>::Attributes
       //---------------------------------------------------------------------
       _Link* _head= nullptr; // -> Head _Link
       _Link* _tail= nullptr; // -> Tail _Link

       //---------------------------------------------------------------------
       // DHSL_list<void>::Constructors/Destructor
       //---------------------------------------------------------------------
       DHSL_list( void ) = default;
       DHSL_list(const DHSL_list&) = delete; // *NO* copy constructor
       DHSL_list(DHSL_list&&) = delete; // *NO* move constructor

       ~DHSL_list( void ) = default;

       //---------------------------------------------------------------------
       // DHSL_list<void>::Operators
       //---------------------------------------------------------------------
       DHSL_list& operator=(const DHSL_list&) = delete; // *NO* copy assignment
       DHSL_list& operator=(DHSL_list&&) = delete; // *NO* move assignment

       //---------------------------------------------------------------------
       //
       // Method-
       //       DHSL_list<void>::begin
       //       DHSL_list<void>::end
       //
       // Purpose-
       //       Create an DHSL_list FIFO iterator.
       //       Create an DHSL_list FIFO end() iterator.
       //
       //---------------------------------------------------------------------
       // Implemented in DHSL_list<T>

       //---------------------------------------------------------------------
       //
       // Method-
       //       DHSL_list<void>::fifo
       //
       // Purpose-
       //       Insert a _Link onto the List with FIFO ordering.
       //
       //---------------------------------------------------------------------
       void
         fifo(                      // Insert (FIFO order)
           _Link*             link); // -> _Link to insert

       //---------------------------------------------------------------------
       //
       // Method-
       //       DHSL_list<void>::get_head
       //
       // Purpose-
       //       Get the head _Link. (Implemented in DHSL_list<T>, not here,)
       //
       //---------------------------------------------------------------------

       //---------------------------------------------------------------------
       //
       // Method-
       //       DHSL_list<void>::insert
       //
       // Purpose-
       //       Insert a chain of elements onto the list at the specified
       //       position.
       //
       //---------------------------------------------------------------------
       void
         insert(                    // Insert at position,
           _Link*             link, // -> _Link to insert after
           _Link*             head, // -> First _Link to insert
           _Link*             tail); // -> Final _Link to insert

       //---------------------------------------------------------------------
       //
       // Method-
       //       DHSL_list<void>::is_coherent
       //
       // Purpose-
       //       List coherency check.
       //
       //---------------------------------------------------------------------
       bool                         // TRUE if the object is coherent
         is_coherent( void ) const; // Coherency check

       //---------------------------------------------------------------------
       //
       // Method-
       //       DHSL_list<void>::is_on_list
       //
       // Purpose-
       //       Test whether _Link is present in this List.
       //
       //---------------------------------------------------------------------
       bool                         // TRUE if _Link is contained
         is_on_list(                // Is _Link contained?
           _Link*             link) const; // -> _Link

       //---------------------------------------------------------------------
       //
       // Method-
       //       DHSL_list<void>::lifo
       //
       // Purpose-
       //       Insert a _Link onto the List with LIFO ordering.
       //
       //---------------------------------------------------------------------
       void
         lifo(                      // Insert (LIFO order)
           _Link*             link); // -> _Link to insert

       //---------------------------------------------------------------------
       //
       // Method-
       //       DHSL_list<void>::remove
       //
       // Purpose-
       //       Remove a chain of elements from the List.
       //       This is an expensive operation for a DHSL_list.
       //
       //---------------------------------------------------------------------
       void
         remove(                    // Remove from DHSL_list
           _Link*             head, // -> First _Link to remove
           _Link*             tail); // -> Final _Link to remove

       //---------------------------------------------------------------------
       //
       // Method-
       //       DHSL_list<void>::remq
       //
       // Purpose-
       //       Remove the head _Link from the list.
       //
       //---------------------------------------------------------------------
       _Link*                       // -> Removed _Link
         remq( void );              // Remove head _Link

       //---------------------------------------------------------------------
       //
       // Method-
       //       DHSL_list<void>::reset
       //
       // Purpose-
       //       Remove ALL _Links from the List.
       //
       //---------------------------------------------------------------------
       _Link*                       // The set of removed _Links
         reset( void );             // Reset (empty) the List
   }; // class DHSL_list<void>

//----------------------------------------------------------------------------
//
// Class-
//       SHSL_list<>
//
// Purpose-
//       A Singly Headed Singly linked list.
//
//----------------------------------------------------------------------------
/**
   @brief An SHSL_list::iterator.

   This is a reverse iterator, iterating from the oldest element on the list
   to the newest.
**/
template<typename T>
   struct _SHSL_iter
   {
     typedef ptrdiff_t                        difference_type;
     typedef std::input_iterator_tag          iterator_category;
     typedef T                                value_type;
     typedef T*                               pointer;
     typedef T&                               reference;

     typedef SHSL_list<T>                     _List;
     typedef _SHSL_iter<T>                    _Self;

     pointer _link;

     _SHSL_iter() noexcept
     : _link(nullptr) {}

     explicit
     _SHSL_iter(_List* list) noexcept
     { _link= list->get_tail(); }

     _SHSL_iter(const _SHSL_iter<T>& that) noexcept
     :  _link(that._link) {}

     _Self
     _const_cast() const noexcept
     { return *this; }

     pointer
     get() const noexcept
     { return _link; }

     operator bool() const noexcept
     { return bool(_link); }

     reference
     operator*() const
     { if( _link )
         return *_link;
       throw __detail::end_dereferenced();
     }

     pointer
     operator->() const
     { if( _link )
         return _link;
       throw __detail::end_dereferenced();
     }

     _Self&
     operator++() noexcept
     {
       if( _link )
         _link = _link->get_prev();
       return *this;
     }

     _Self
     operator++(int) noexcept
     {
       _Self __tmp = *this;
       if( _link )
         _link = _link->get_prev();
       return __tmp;
     }

     friend bool
     operator==(const _Self& lhs, const _Self& rhs) noexcept
     { return lhs._link == rhs._link; }

     friend bool
     operator!=(const _Self& lhs, const _Self& rhs) noexcept
     { return lhs._link != rhs._link; }
   }; // _SHSL_iter

/**
   @brief An SHSL_list::const_iterator.

   This is a reverse iterator, iterating from the oldest element on the list
   to the newest.
**/
template<typename T>
   struct _SHSL_const_iter
   {
     typedef ptrdiff_t                        difference_type;
     typedef std::input_iterator_tag          iterator_category;
     typedef T                                value_type;
     typedef const T*                         pointer;
     typedef const T&                         reference;

     typedef SHSL_list<T>                     _List;
     typedef _SHSL_const_iter<T>              _Self;

     pointer _link;

     _SHSL_const_iter() noexcept
     : _link(nullptr) {}

     explicit
     _SHSL_const_iter(const _List* list) noexcept
     { _link= list->get_tail(); }

     _SHSL_const_iter(const _SHSL_iter<T>& that) noexcept
     :  _link(that._link) {}

     _SHSL_const_iter(const _SHSL_const_iter<T>& that) noexcept
     :  _link(that._link) {}

     pointer
     get() const noexcept
     { return _link; }

     operator bool() const noexcept
     { return bool(_link); }

     reference
     operator*() const
     { if( _link )
         return *_link;
       throw __detail::end_dereferenced();
     }

     pointer
     operator->() const
     { if( _link )
         return _link;
       throw __detail::end_dereferenced();
     }

     _Self&
     operator++() noexcept
     {
       if( _link )
         _link = _link->get_prev();
       return *this;
     }

     _Self
     operator++(int) noexcept
     {
       _Self __tmp = *this;
       if( _link )
         _link = _link->get_prev();
       return __tmp;
     }

     friend bool
     operator==(const _Self& lhs, const _Self& rhs) noexcept
     { return lhs._link == rhs._link; }

     friend bool
     operator!=(const _Self& lhs, const _Self& rhs) noexcept
     { return lhs._link != rhs._link; }
   }; // _SHSL_iter

//----------------------------------------------------------------------------
//
// Class-
//       SHSL_list<void>
//
// Purpose-
//       The Singly Headed, Singly Linked List.
//
// Implemenation notes-
//       The SHDL_list is not thread safe. Method usage must be serialized.
//       The SHSL_list is optimized for LIFO operation. If you think of
//       this List as a Stack, LIFO == PUSH and REMQ == PULL.
//
//       The INSERT, LIFO and REMQ methods run in constant time.
//       The REMOVE method run in linear time. The FIFO method is deprecated.
//
// List structure-
//       SHSL_list::_tail -> newest -> older -> ... -> oldest
//       REMQ() removes the newest element.
//
//       begin() removes all elements from the list, reordering the list from
//       oldest to older to newest, and creating a FIFO input_iterator.
//       This reordering takes linear time, requiring a single pass through the
//       list.
//
//----------------------------------------------------------------------------
template<> class SHSL_list<void>
   {
     public:
       typedef __detail::_PREV_link           value_type;
       typedef value_type*                    pointer;
       typedef value_type&                    reference;

       typedef __detail::_PREV_link           _Link;

     protected:
       //---------------------------------------------------------------------
       // SHSL_list<void>::Attributes
       //---------------------------------------------------------------------
       _Link* _tail= nullptr;

       //---------------------------------------------------------------------
       // SHSL_list<void>::Constructors/Destructor
       //---------------------------------------------------------------------
       SHSL_list( void ) = default;
       SHSL_list(const SHSL_list&) = delete; // *NO* copy constructor
       SHSL_list(SHSL_list&&) = delete; // *NO* move constructor

       ~SHSL_list( void ) = default;

       //---------------------------------------------------------------------
       // SHSL_list<void>::Operators
       //---------------------------------------------------------------------
       SHSL_list& operator=(const SHSL_list&) = delete; // *NO* copy assignment
       SHSL_list& operator=(SHSL_list&&) = delete; // *NO* move assignment

       //---------------------------------------------------------------------
       //
       // Method-
       //       SHSL_list<void>::begin
       //       SHSL_list<void>::end
       //
       // Purpose-
       //       Create an SHSL_list LIFO iterator.
       //       Create an SHSL_list LIFO end() iterator.
       //
       //---------------------------------------------------------------------
       // Implemented in SHSL_list<T>

       //---------------------------------------------------------------------
       //
       // Method-
       //       SHSL_list<void>::fifo
       //
       // Purpose-
       //       Insert a _Link onto the List with FIFO ordering.
       //
       // Implementation notes-
       //       This examines all existing _Link elements taking linear time.
       //
       //---------------------------------------------------------------------
       _LIBPUB_DEPRECATED_USE("Use lifo to insert and begin/end to iterate")
       void
         fifo(                      // Insert (FIFO order)
           _Link*             link); // -> _Link to insert

       //---------------------------------------------------------------------
       //
       // Method-
       //       SHSL_list<void>::get_tail
       //
       // Purpose-
       //       Get the tail _Link. (Implemented in SHSL_list<T>, not here,)
       //
       //---------------------------------------------------------------------

       //---------------------------------------------------------------------
       //
       // Method-
       //       SHSL_list<void>::insert
       //
       // Purpose-
       //       Insert a chain of elements onto the list at the specified
       //       position.
       //
       // Implementation notes-
       //       The SHSL list is reverse ordered, LIFO list. New elements get
       //       added to to the tail, and the inserted link chain goes from the
       //       tail toward the head.
       //
       //---------------------------------------------------------------------
       void
         insert(                    // Insert at position
           _Link*             link, // -> _Link to insert after
           _Link*             tail, // -> First _Link to insert
           _Link*             head); // -> Final _Link to insert

       //---------------------------------------------------------------------
       //
       // Method-
       //       SHSL_list<void>::is_coherent
       //
       // Purpose-
       //       List coherency check.
       //
       //---------------------------------------------------------------------
       bool                         // TRUE if the object is coherent
         is_coherent( void ) const; // Coherency check

       //---------------------------------------------------------------------
       //
       // Method-
       //       SHSL_list<void>::is_on_list
       //
       // Purpose-
       //       Test whether _Link is present in this List.
       //
       //---------------------------------------------------------------------
       bool                         // TRUE if _Link is contained
         is_on_list(                // Is _Link contained?
           _Link*             link) const; // -> _Link

       //---------------------------------------------------------------------
       //
       // Method-
       //       SHSL_list<void>::lifo
       //
       // Purpose-
       //       Insert a _Link onto the List with LIFO ordering.
       //
       //---------------------------------------------------------------------
       void
         lifo(                      // Insert (LIFO order)
           _Link*             link); // -> _Link to insert

       //---------------------------------------------------------------------
       //
       // Method-
       //       SHSL_list<void>::remove
       //
       // Purpose-
       //       Remove a chain of elements from the List.
       //
       // Implementation notes-
       //       This examines existing _Link elements, taking linear time.
       //
       //---------------------------------------------------------------------
       _LIBPUB_DEPRECATED           // No use case and it takes linear time.
       void
         remove(                    // Remove from List
           _Link*             tail, // -> First _Link to remove
           _Link*             head); // -> Final _Link to remove

       //---------------------------------------------------------------------
       //
       // Method-
       //       SHSL_list<void>::remq
       //
       // Purpose-
       //       Remove the tail _Link from the list.
       //
       // Implementation notes-
       //       REMQ is logically consistent with the LIFO method, removing
       //       the newest (tail) link.
       //
       //---------------------------------------------------------------------
       _Link*                       // -> Removed _Link
         remq( void );              // Remove head _Link

       //---------------------------------------------------------------------
       //
       // Method-
       //       SHSL_list<void>::reset
       //
       // Purpose-
       //       Remove ALL _Links from the List.
       //
       //---------------------------------------------------------------------
       _Link*                       // The set of removed _Links
         reset( void );             // Reset (empty) the List
   }; // class SHSL_list<void>
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_BITS_LIST_H_INCLUDED
