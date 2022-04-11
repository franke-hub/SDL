//----------------------------------------------------------------------------
//
//       Copyright (c) 2022 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       bits/List.h
//
// Purpose-
//       ../List.h template definitions and internal base classes.
//
// Last change date-
//       2022/04/05
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_BITS_LIST_H_INCLUDED
#define _LIBPUB_BITS_LIST_H_INCLUDED
/** @file pub/bits/List.h
**  This is an internal header file, included by other library headers.
**  Do not attempt to use it directly.
**/
#include <stdexcept>                // For std::domain_error

#define USE_BASE_SORT false         // Use List<void>::sort : List<T>::sort

#include <pub/bits/pubconfig.h>     // For _LIBPUB_ macros

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
template<class T> class AI_list;    // Atomic update List user interface
template<class T> class DHDL_list;  // DHDL List
template<class T> class DHSL_list;  // DHSL List
template<class T> class List;       // List (Is a DHDL_list)
template<class T> class SHSL_list;  // SHSL List

namespace __detail
{
   /// Compile-time constants
   enum
   { HCDM= false                    // Hard Core Debug Mode?
   , MAX_COHERENT= 1000000000       // Maximum coherent List element count
   }; // generic enum

   /// Exceptions
   /**
     @brief Exception pub::__detail::end_dereferenced

     In lieu of undefined behavior, this std::domain_error exception is thrown
     when an end() iterator is dereferenced.
   **/
   class end_dereferenced : public std::domain_error
   {
     public:
       using std::domain_error::domain_error;
       end_dereferenced() : domain_error("end() dereferenced") {}
   }; // class end_dereferenced

   /// Common parts of a bidirectional link
   struct _BIDI_link
   {
     typedef _BIDI_link                       _Self;

     _Self* _next;
     _Self* _prev;

     static void
     swap(_Self& lhs, _Self& rhs) noexcept;
   }; // _BIDI_link

   /// Common parts of a forward link
   struct _NEXT_link
   {
     typedef _NEXT_link                       _Self;
     _Self* _next;

     static void
     swap(_Self& lhs, _Self& rhs) noexcept;
   };

   /// Common parts of a reverse link
   struct _PREV_link
   {
     typedef _PREV_link                       _Self;
     _Self* _prev;

     static void
     swap(_Self& lhs, _Self& rhs) noexcept;
   };

   /// A dummy end of list value, used internally.
   extern const void*  __end;
} // namespace __detail

//----------------------------------------------------------------------------
//
// Class-
//       _AI_iter<T>
//
// Purpose-
//       Atomic Insertion list iterator.
//
//----------------------------------------------------------------------------
/**
   @brief An AI_list<T> iterator

   There isn't an AI_const_iter.

   This iterator reverses the AI_list reverse links without changing the Link
   type, so get_prev() and _prev in the code actually refer to the logically
   NEXT link.
**/
template<typename T>
   struct _AI_iter
   {
     typedef ptrdiff_t                        difference_type;
     typedef std::input_iterator_tag          iterator_category;
     typedef T                                value_type;
     typedef T*                               pointer;
     typedef T&                               reference;

     typedef T                                _Link;
     typedef AI_list<T>                       _List;
     typedef _AI_iter<T>                      _Self;

     pointer      _link= nullptr;   // The current T*
     _List* const _list= nullptr;   // The associated List<T>*
     pointer      _todo= nullptr;   // The remaining _Links

     _AI_iter() noexcept = default;

     _AI_iter(const _AI_iter& that) noexcept
     :  _link(that._link), _list(that._list), _todo(that._todo) {}

     explicit
     _AI_iter(_List* list) noexcept
     :  _list(list)
     {
       T* tail= _list->reset((void*)__detail::__end);
       while( tail )
       {
         T* prev= tail->get_prev();
         tail->_prev= _todo;
         _todo= tail;
         tail= prev;
       }
       _link= _todo;
       if( _todo )
         _todo= _todo->get_prev();
     }

     pointer
     get() const noexcept
     { return _link; }

     operator bool()
     { return bool(_link); }

     reference
     operator*() const // noexcept
     { if( _link )
         return (reference)(*_link);
       throw __detail::end_dereferenced();
     }

     pointer
     operator->() const // noexcept
     { if( _link )
         return _link;
       throw __detail::end_dereferenced();
     }

     _Self&
     operator++() noexcept
     {
       if( _todo ) {
         _link= _todo;
         _todo= _todo->get_prev();
       } else {
         _next();
       }
       return *this;
     }

     _Self
     operator++(int) noexcept
     {
       _Self __tmp = *this;
       operator++();
       return __tmp;
     }

     friend bool
     operator==(const _Self& lhs, const _Self& rhs) noexcept
     { return lhs._link == rhs._link; }

     friend bool
     operator!=(const _Self& lhs, const _Self& rhs) noexcept
     { return lhs._link != rhs._link; }

     protected:
       /**
         @pre _todo == nullptr
       **/
       void _next() noexcept
       {
         _link= nullptr;
         pointer tail= _list->reset((void*)__detail::__end);
         if( tail )
         {
           do
           {
             pointer prev= tail->get_prev();
             tail->_prev= _todo;
             _todo= tail;
             tail= prev;
           } while( (void*)tail != __detail::__end );
           _link= _todo;
           if( _todo )
             _todo= _todo->get_prev();
         }
       }
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
 *  @brief A DHDL_list::iterator.
*/
template<typename T>
   struct _DHDL_list_iterator
   {
     typedef ptrdiff_t                        difference_type;
     typedef std::bidirectional_iterator_tag  iterator_category;
     typedef T                                value_type;
     typedef T*                               pointer;
     typedef T&                               reference;

     typedef _DHDL_list_iterator<T>           _Self;
     typedef __detail::_BIDI_link             _Link_base;

     _Link_base* _link;

     _DHDL_list_iterator() noexcept
     : _link(nullptr) {}

     explicit
     _DHDL_list_iterator(__detail::_BIDI_link* base) noexcept
     : _link(base) {}

     _Self
     _const_cast() const noexcept
     { return *this; }

     pointer
     get() const noexcept
     { return (pointer)_link; }

     operator bool() const noexcept
     { return bool(_link); }

     reference
     operator*() const
     { if( _link )
         return *((pointer)_link);
//         return *static_cast<pointer>(_link); // (This compiles)
       throw __detail::end_dereferenced();
     }

     pointer
     operator->() const
     { if( _link )
         return (pointer)_link;
//         return static_cast<pointer>(_link); // (Inaccessable base class)
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
   }; // _DHDL_list_iterator

/**
 *  @brief A DHDL_list::const_iterator.
*/
template<typename T>
   struct _DHDL_list_const_iterator
   {
     typedef _DHDL_list_const_iterator<T>     _Self;
     typedef _DHDL_list_iterator<T>           iterator;

     typedef ptrdiff_t                        difference_type;
     typedef std::bidirectional_iterator_tag  iterator_category;
     typedef T                                value_type;
     typedef const T*                         pointer;
     typedef const T&                         reference;

     const __detail::_BIDI_link* _link;

     _DHDL_list_const_iterator() noexcept
     : _link(nullptr) {}

     explicit
     _DHDL_list_const_iterator(const __detail::_BIDI_link* base) noexcept
     : _link(base) {}

     _DHDL_list_const_iterator(const iterator& _it) noexcept
     : _link(_it._link) {}

     iterator
     _const_cast() const noexcept
     { return iterator(const_cast<__detail::_BIDI_link*>(_link)); }

     pointer
     get() const noexcept
     { return (pointer)_link; }

     operator bool() const noexcept
     { return bool(_link); }

     reference
     operator*() const noexcept
     { if( _link )
         return *static_cast<pointer>(_link);
       throw __detail::end_dereferenced();
     }

     pointer
     operator->() const noexcept
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
   }; // _DHDL_list_const_iterator

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
       typedef __detail::_BIDI_link           value_type;
       typedef value_type*                    pointer;
       typedef value_type&                    reference;

       typedef _DHDL_list_iterator<pointer>   iterator;
       typedef _DHDL_list_const_iterator<pointer> const_iterator;

       typedef __detail::_BIDI_link           _Link;

#if USE_BASE_SORT
       typedef std::function<bool(const _Link*, const _Link*)>
                                              _Comparator;
#endif

     protected:
       //----------------------------------------------------------------------------
       // DHDL_list<void>::Attributes
       //----------------------------------------------------------------------------
       _Link* _head= nullptr;
       _Link* _tail= nullptr;

       //----------------------------------------------------------------------------
       // DHDL_list<void>::Constructors/Destructor
       //----------------------------------------------------------------------------
       DHDL_list( void ) = default;
       ~DHDL_list( void ) = default;

       DHDL_list(const DHDL_list&) = delete;
       DHDL_list& operator=(const DHDL_list&) = delete;

       //----------------------------------------------------------------------------
       //
       // Method-
       //       DHDL_list<void>::begin
       //
       // Purpose-
       //       Create an DHDL_list FIFO iterator.
       //
       //----------------------------------------------------------------------------
//             iterator begin()       noexcept { return iterator(_head); }
//       const_iterator begin() const noexcept { return const_iterator(_head); }

       //----------------------------------------------------------------------------
       //
       // Method-
       //       DHDL_list<void>::end
       //
       // Purpose-
       //       Create an DHDL_list FIFO end() iterator.
       //
       //----------------------------------------------------------------------------
//             iterator end()       noexcept { return iterator(); }
//       const_iterator end() const noexcept { return const_iterator(); }

       //----------------------------------------------------------------------------
       //
       // Method-
       //       DHDL_list<void>::fifo
       //
       // Purpose-
       //       Insert a _Link onto the list with FIFO ordering.
       //
       //----------------------------------------------------------------------------
       void fifo(_Link* link);

       //----------------------------------------------------------------------------
       //
       // Method-
       //       DHDL_list<void>::get_head
       //       DHDL_list<void>::get_tail
       //
       // Purpose-
       //       Get the head _Link. (Implemented in DHDL_list<T>, not here,)
       //       Get the tail _Link. (Implemented in DHDL_list<T>, not here.)
       //
       //----------------------------------------------------------------------------

       //----------------------------------------------------------------------------
       //
       // Method-
       //       DHDL_list<void>::insert
       //
       // Purpose-
       //       Insert a chain of elements onto the list at the specified position.
       //
       //----------------------------------------------------------------------------
       void
         insert(                    // Insert at position,
           _Link*             link, // -> _Link to insert after
           _Link*             head, // -> First _Link to insert
           _Link*             tail); // -> Final _Link to insert

       //----------------------------------------------------------------------------
       //
       // Method-
       //       DHDL_list<void>::is_coherent
       //
       // Purpose-
       //       List coherency check.
       //
       //----------------------------------------------------------------------------
       bool                         // TRUE if the object is coherent
         is_coherent( void ) const; // Coherency check

       //----------------------------------------------------------------------------
       //
       // Method-
       //       DHDL_list<void>::is_on_list
       //
       // Purpose-
       //       Test whether _Link is present in this List.
       //
       //----------------------------------------------------------------------------
       bool                         // TRUE if _Link is contained
         is_on_list(                // Is _Link contained?
           _Link*             link) const; // -> _Link

       //----------------------------------------------------------------------------
       //
       // Method-
       //       DHDL_list<void>::lifo
       //
       // Purpose-
       //       Insert a _Link onto the list with LIFO ordering.
       //
       //----------------------------------------------------------------------------
       void
         lifo(                      // Insert (LIFO order)
           _Link*             link); // -> _Link to insert

       //----------------------------------------------------------------------------
       //
       // Method-
       //       DHDL_list<void>::remove
       //
       // Purpose-
       //       Remove a chain of elements from the list.
       //
       //----------------------------------------------------------------------------
       void
         remove(                    // Remove from list
           _Link*             head, // -> First _Link to remove
           _Link*             tail); // -> Final _Link to remove

       //----------------------------------------------------------------------------
       //
       // Method-
       //       DHDL_list<void>::remq
       //
       // Purpose-
       //       Remove the head _Link from the List.
       //
       //----------------------------------------------------------------------------
       _Link*                       // -> Removed _Link
         remq( void );              // Remove head _Link

       //----------------------------------------------------------------------------
       //
       // Method-
       //       DHDL_list<void>::reset
       //
       // Purpose-
       //       Remove ALL _Links from the List.
       //
       //----------------------------------------------------------------------------
       _Link*                       // The set of removed _Links
         reset( void );             // Reset (empty) the list

       //----------------------------------------------------------------------------
       //
       // Method-
       //       DHDL_list<void>::sort
       //
       // Purpose-
       //       Sort the List.
       //
       //----------------------------------------------------------------------------
#if USE_BASE_SORT
       void sort(_Comparator less); // Sort the List using Comparitor
#endif
   }; // class DHDL_list<void>

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
       typedef __detail::_NEXT_link      _Link;

     protected:
       //----------------------------------------------------------------------------
       // DHSL_list<void>::Attributes
       //----------------------------------------------------------------------------
       _Link* _head= nullptr; // -> Head _Link
       _Link* _tail= nullptr; // -> Tail _Link

       //----------------------------------------------------------------------------
       // DHSL_list<void>::Constructors/Destructor
       //----------------------------------------------------------------------------
       ~DHSL_list( void ) {}
       DHSL_list( void ) {}

       DHSL_list(const DHSL_list&) = delete;
       DHSL_list& operator=(const DHSL_list&) = delete;

       //----------------------------------------------------------------------------
       //
       // Method-
       //       DHSL_list<void>::fifo
       //
       // Purpose-
       //       Insert a _Link onto the List with FIFO ordering.
       //
       //----------------------------------------------------------------------------
       void
         fifo(                      // Insert (FIFO order)
           _Link*             link); // -> _Link to insert

       //----------------------------------------------------------------------------
       //
       // Method-
       //       DHSL_list<void>::get_head
       //
       // Purpose-
       //       Get the head _Link. (Implemented in DHSL_list<T>, not here,)
       //
       //----------------------------------------------------------------------------

       //----------------------------------------------------------------------------
       //
       // Method-
       //       DHSL_list<void>::insert
       //
       // Purpose-
       //       Insert a chain of elements onto the list at the specified position.
       //
       //----------------------------------------------------------------------------
       void
         insert(                    // Insert at position,
           _Link*             link, // -> _Link to insert after
           _Link*             head, // -> First _Link to insert
           _Link*             tail); // -> Final _Link to insert

       //----------------------------------------------------------------------------
       //
       // Method-
       //       DHSL_list<void>::is_coherent
       //
       // Purpose-
       //       List coherency check.
       //
       //----------------------------------------------------------------------------
       bool                         // TRUE if the object is coherent
         is_coherent( void ) const; // Coherency check

       //----------------------------------------------------------------------------
       //
       // Method-
       //       DHSL_list<void>::is_on_list
       //
       // Purpose-
       //       Test whether _Link is present in this List.
       //
       //----------------------------------------------------------------------------
       bool                         // TRUE if _Link is contained
         is_on_list(                // Is _Link contained?
           _Link*             link) const; // -> _Link

       //----------------------------------------------------------------------------
       //
       // Method-
       //       DHSL_list<void>::lifo
       //
       // Purpose-
       //       Insert a _Link onto the List with LIFO ordering.
       //
       //----------------------------------------------------------------------------
       void
         lifo(                      // Insert (LIFO order)
           _Link*             link); // -> _Link to insert

       //----------------------------------------------------------------------------
       //
       // Method-
       //       DHSL_list<void>::remove
       //
       // Purpose-
       //       Remove a chain of elements from the List.
       //       This is an expensive operation for a DHSL_list.
       //
       //----------------------------------------------------------------------------
       void
         remove(                    // Remove from DHSL_list
           _Link*             head, // -> First _Link to remove
           _Link*             tail); // -> Final _Link to remove

       //----------------------------------------------------------------------------
       //
       // Method-
       //       DHSL_list<void>::remq
       //
       // Purpose-
       //       Remove the head _Link from the list.
       //
       //----------------------------------------------------------------------------
       _Link*                       // -> Removed _Link
         remq( void );              // Remove head _Link

       //----------------------------------------------------------------------------
       //
       // Method-
       //       DHSL_list<void>::reset
       //
       // Purpose-
       //       Remove ALL _Links from the List.
       //
       //----------------------------------------------------------------------------
       _Link*                       // The set of removed _Links
         reset( void );             // Reset (empty) the List
   }; // class DHSL_list<void>

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
//       The FIFO and REMOVE methods run in linear time.
//
//----------------------------------------------------------------------------
template<> class SHSL_list<void>
   {
     public:
       typedef __detail::_NEXT_link      _Link;

     protected:
       //----------------------------------------------------------------------------
       // SHSL_list<void>::Attributes
       //----------------------------------------------------------------------------
       _Link* _head= nullptr; // -> Head _Link

       //----------------------------------------------------------------------------
       // SHSL_list<void>::Constructors/Destructor
       //----------------------------------------------------------------------------
       SHSL_list( void ) = default;
       ~SHSL_list( void ) = default;

       SHSL_list(const SHSL_list&) = delete;
       SHSL_list& operator=(const SHSL_list&) = delete;

       //----------------------------------------------------------------------------
       //
       // Method-
       //       SHSL_list<void>::fifo
       //
       // Purpose-
       //       Insert a _Link onto the List with FIFO ordering.
       //
       // Implementation notes-
       //       This examines all existing _Link elements and takes linear time.
       //
       //----------------------------------------------------------------------------
       void
         fifo(                      // Insert (FIFO order)
           _Link*             link); // -> _Link to insert

       //----------------------------------------------------------------------------
       //
       // Method-
       //       SHSL_list<void>::get_head
       //
       // Purpose-
       //       Get the head _Link. (Implemented in SHSL_list<T>, not here,)
       //
       //----------------------------------------------------------------------------

       //----------------------------------------------------------------------------
       //
       // Method-
       //       SHSL_list<void>::insert
       //
       // Purpose-
       //       Insert a chain of elements onto the list at the specified position.
       //
       //----------------------------------------------------------------------------
       void
         insert(                    // Insert at position,
           _Link*             link, // -> _Link to insert after
           _Link*             head, // -> First _Link to insert
           _Link*             tail); // -> Final _Link to insert

       //----------------------------------------------------------------------------
       //
       // Method-
       //       SHSL_list<void>::is_coherent
       //
       // Purpose-
       //       List coherency check.
       //
       //----------------------------------------------------------------------------
       bool                         // TRUE if the object is coherent
         is_coherent( void ) const; // Coherency check

       //----------------------------------------------------------------------------
       //
       // Method-
       //       SHSL_list<void>::is_on_list
       //
       // Purpose-
       //       Test whether _Link is present in this List.
       //
       //----------------------------------------------------------------------------
       bool                         // TRUE if _Link is contained
         is_on_list(                // Is _Link contained?
           _Link*             link) const; // -> _Link

       //----------------------------------------------------------------------------
       //
       // Method-
       //       SHSL_list<void>::lifo
       //
       // Purpose-
       //       Insert a _Link onto the List with LIFO ordering.
       //
       //----------------------------------------------------------------------------
       void
         lifo(                      // Insert (LIFO order)
           _Link*             link); // -> _Link to insert

       //----------------------------------------------------------------------------
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
       //----------------------------------------------------------------------------
       void
         remove(                    // Remove from List
           _Link*             head, // -> First _Link to remove
           _Link*             tail); // -> Final _Link to remove

       //----------------------------------------------------------------------------
       //
       // Method-
       //       SHSL_list<void>::remq
       //
       // Purpose-
       //       Remove the head _Link from the list.
       //
       // Implementation notes-
       //       REMQ is logically consistent with the LIFO and FIFO methods.
       //       The list is ordered from head to tail, so FIFO scans the list,
       //       inserting at the end of it, the tail.
       //
       //----------------------------------------------------------------------------
       _Link*                       // -> Removed _Link
         remq( void );              // Remove head _Link

       //----------------------------------------------------------------------------
       //
       // Method-
       //       SHSL_list<void>::reset
       //
       // Purpose-
       //       Remove ALL _Links from the List.
       //
       //----------------------------------------------------------------------------
       _Link*                       // The set of removed _Links
         reset( void );             // Reset (empty) the List
   }; // class SHSL_list<void>
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_BITS_LIST_H_INCLUDED
