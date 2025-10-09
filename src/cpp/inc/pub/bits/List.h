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
//       2025/10/09
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_BITS_LIST_H_INCLUDED
#define _LIBPUB_BITS_LIST_H_INCLUDED

#include <stdexcept>                // For std::domain_error
#include <string>                   // For std::string TODO: REMOVE

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
template<class T> class AI_list;    // Atomic Insert list
template<class T> class DHDL_list;  // Doubly Headed Doubly Linked list
template<class T> class DHDL_sort;  // Doubly Headed Doubly Linked sortable list
template<class T> class DHSL_list;  // Doubly Headed Singly Linked list
template<class T> class SHSL_list;  // Singly Headed Singly Linked list
template<class T> class List;       // List (Is a DHDL_list)
template<class T> class Sort;       // List (Is a DHDL_sort)

namespace __detail
{
   /// Compile-time constants
   enum
   { HCDM= false                    // Hard Core Debug Mode?
   , MAX_COHERENT= 1'000'000'000    // Maximum coherent List element count
   }; // generic enum

   /// Exceptions
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
   }; // _BIDL_link

   /// Common parts of a bidirectional singly linked link
   ///   AI_list Links use LIFO ordering; _AI_iter Links use FIFO ordering
   struct _BISL_link
   {
     typedef _BISL_link                       _Self;
     _Self* _link= nullptr;

     _Self* get_next() { return _link; }
     _Self* get_prev() { return _link; }
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

//============================================================================
//
// Struct-
//       _AI_iter<void>
//
// Purpose-
//       Define the AI_list<void> iterator
//
//----------------------------------------------------------------------------
template<class T> class _AI_iter;

template<>
   struct _AI_iter<void>
   {
     typedef ptrdiff_t                        difference_type;
     typedef std::input_iterator_tag          iterator_category;

     typedef __detail::_BISL_link             _Link;
     typedef _Link*                           pointer;
     typedef _Link&                           reference;
     typedef AI_list<void>                    _List;
     typedef _AI_iter<void>                   _Self;

     pointer      _head= nullptr;   // The remaining _Links (in FIFO order)
     pointer      _link= nullptr;   // The current _Link*
     _List* const _list= nullptr;   // The associated _List*

     _AI_iter() noexcept = default; // Default, end() constructor

     _AI_iter(const _AI_iter& that) noexcept // The copy constructor
     : _head(that._head), _link(that._link), _list(that._list) {}

     explicit
     _AI_iter(_List* list) noexcept; // The begin constructor

     pointer
     get() const noexcept
     { return _link; }

     operator bool()
     { return bool(_link); }

     reference
     operator*() const
     { if( _link )
         return (reference)(*_link);
       throw __detail::end_dereferenced();
     }

     pointer
     operator->() const
     { if( _link )
         return _link;
       throw __detail::end_dereferenced();
     }

     _Self&
     operator++() noexcept;

     _Self
     operator++(int) noexcept;
   }; // struct _AI_iter<void>

//----------------------------------------------------------------------------
//
// Struct-
//       _AI_iter<T>
//
// Purpose-
//       Define the AI_list<T> iterator
//
// Implementation notes-
//       There is no AI_const_iter<T>
//
//----------------------------------------------------------------------------
template<typename T>
   struct _AI_iter : public _AI_iter<void>
   {
     typedef T                                value_type;
     typedef T*                               pointer;
     typedef T&                               reference;

     typedef _AI_iter<void>                   _Base;
     typedef T                                _Link;
     typedef AI_list<T>                       _List;
     typedef _AI_iter<T>                      _Self;

     _AI_iter() noexcept = default; // Default, end() constructor

     _AI_iter(const _AI_iter& that) noexcept // The copy constructor
     : _Base(that) {}

     explicit
     _AI_iter(_List* list) noexcept // The begin() constructor
     : _Base(list) {}

     pointer
     get() const noexcept
     { return static_cast<pointer>(_Base::get()); }

     operator bool()
     { return bool(_link); }

     reference
     operator*() const
     { return static_cast<reference>(_Base::operator*()); }

     pointer
     operator->() const
     { return static_cast<pointer>(_Base::operator->()); }

     _Self&
     operator++() noexcept
     { return static_cast<_Self&>(_Base::operator++()); }

     _Self
     operator++(int) noexcept
     {
       _Self __tmp= *this;
       operator++();
       return __tmp;
     }

     friend bool
     operator==(const _Self& lhs, const _Self& rhs) noexcept
     { return lhs._link == rhs._link; }

     friend bool
     operator!=(const _Self& lhs, const _Self& rhs) noexcept
     { return lhs._link != rhs._link; }
   }; // struct _AI_iter<T>

//----------------------------------------------------------------------------
//
// Class-
//       AI_list<void>
//
// Purpose-
//       Define the AI_list<T> base class
//
//----------------------------------------------------------------------------
template<>
   class AI_list<void>
   {
     public:
       typedef __detail::_BISL_link           _Link;
       typedef _Link*                         pointer;
       typedef _Link&                         reference;

       //---------------------------------------------------------------------
       // AI_list<void>::Attributes
       //---------------------------------------------------------------------
     protected:
       std::atomic<pointer> _tail= nullptr; // The newest List element

       //---------------------------------------------------------------------
       // AI_list<void>::Constructors/destructor
       //---------------------------------------------------------------------
     public:
       AI_list( void ) = default;
       ~AI_list( void );

       //---------------------------------------------------------------------
       // AI_list<void>::Methods
       //---------------------------------------------------------------------
       void
         debug(const char* info= "") const; // Write a debugging message

       pointer                      // -> Prior tail
         fifo(                      // Insert (fifo order)
           pointer     link);       // -> Link to insert

       bool                         // TRUE if the Link set is coherent
         is_coherent( void ) const; // Coherency check

       bool                         // TRUE if link is contained
         is_on_list(                // Is link contained?
           pointer     link) const; // -> Link

       pointer                      // The set of removed Links
         reset(                     // Reset (replace) the List set with
           const void* tail) noexcept; // This replacement pseudo-Link
   }; // struct AI_list<void>

//============================================================================
//
// Struct-
//       _DHDL_iter<T>
//
// Purpose-
//       Define the DHDL_list<T> iterator
//
//----------------------------------------------------------------------------
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

     // _Self
     // _const_cast() const noexcept
     // { return *this; }

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

//----------------------------------------------------------------------------
//
// Struct-
//       _DHDL_const_iter<T>
//
// Purpose-
//       Define the DHDL_list<T> const iterator
//
//----------------------------------------------------------------------------
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
//       Define the DHDL_list<T> base class
//
// Implementation notes-
//       The DHDL_list is not thread safe. Method usage must be serialized.
//
//----------------------------------------------------------------------------
template<>
   class DHDL_list<void>
   {
     public:
       typedef __detail::_BIDL_link           _Link;
       typedef _Link*                         pointer;
       typedef _Link&                         reference;

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
       //       DHDL_list<void>::debug
       //
       // Purpose-
       //       Debugging display
       //
       //---------------------------------------------------------------------
       void debug(const char* info="") const;

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
   }; // class DHDL_list<void>

//============================================================================
//
// Struct-
//       _SORT_iter<T>
//
// Purpose-
//       Define the DHDL_sort<T> iterator
//
//----------------------------------------------------------------------------
template<typename T>
   struct _SORT_iter
   {
     typedef ptrdiff_t                        difference_type;
     typedef std::bidirectional_iterator_tag  iterator_category;
     typedef T                                value_type;
     typedef T*                               pointer;
     typedef T&                               reference;

     typedef T                                _Link;
     typedef DHDL_sort<T>                     _List;
     typedef _SORT_iter<T>                    _Self;

     pointer _link;

     _SORT_iter() noexcept
     : _link(nullptr) {}

     explicit
     _SORT_iter(_List* list) noexcept
     : _link(list->get_head()) {}

     // _Self
     // _const_cast() const noexcept
     // { return *this; }

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
   }; // _SORT_iter

//----------------------------------------------------------------------------
//
// Struct-
//       _SORT_const_iter<T>
//
// Purpose-
//       Define the DHDL_sort<T> const iterator
//
//----------------------------------------------------------------------------
template<typename T>
   struct _SORT_const_iter
   {
     typedef ptrdiff_t                        difference_type;
     typedef std::bidirectional_iterator_tag  iterator_category;
     typedef T                                value_type;
     typedef const T*                         pointer;
     typedef const T&                         reference;

     typedef T                                _Link;
     typedef DHDL_sort<T>                     _List;
     typedef _SORT_const_iter<T>              _Self;

     pointer _link;

     _SORT_const_iter() noexcept
     : _link(nullptr) {}

     explicit
     _SORT_const_iter(const _List* list) noexcept
     : _link(list->get_head()) {}

     _SORT_const_iter(const _SORT_iter<T>& _it) noexcept
     : _link(_it._link) {}

     _SORT_const_iter(const _SORT_const_iter& _it) noexcept
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
   }; // _SORT_const_iter

//----------------------------------------------------------------------------
//
// Class-
//       DHDL_sort<void>
//
// Purpose-
//       Implement the DHDL_sort<T> base class.
//
//----------------------------------------------------------------------------
template<>
   class DHDL_sort<void>
   {
     public:
       struct _Link                 // (Default: sort by address)
       {
         _Link* _next= nullptr;
         _Link* _prev= nullptr;

         virtual ~_Link( void ) = default;

         // OVERRIDE this method
         virtual bool operator<(const _Link& that) const
         { printf("DHDL_sort<void> operator<\n");
           return (char*)this < (char*)&that; }
       }; // _Link

       typedef _Link*                         pointer;
       typedef _Link&                         reference;

     protected:
       //---------------------------------------------------------------------
       // DHDL_sort<void>::Attributes
       //---------------------------------------------------------------------
       _Link* _head= nullptr;
       _Link* _tail= nullptr;

       //---------------------------------------------------------------------
       // DHDL_sort<void>::Constructors/Destructor
       //---------------------------------------------------------------------
       DHDL_sort( void ) = default;
       DHDL_sort(const DHDL_sort&) = delete; // *NO* copy constructor
       DHDL_sort(DHDL_sort&&) = delete; // *NO* move constructor

       ~DHDL_sort( void ) = default;

       //---------------------------------------------------------------------
       // DHDL_sort<void>::Operators
       //---------------------------------------------------------------------
       DHDL_sort& operator=(const DHDL_sort&) = delete; // *NO* copy assignment
       DHDL_sort& operator=(DHDL_sort&&) = delete; // *NO* move assignment

       //---------------------------------------------------------------------
       //
       // Method-
       //       DHDL_sort<void>::begin
       //       DHDL_sort<void>::end
       //
       // Purpose-
       //       Create an DHDL_sort FIFO iterator.
       //       Create an DHDL_sort FIFO end() iterator.
       //
       //---------------------------------------------------------------------
       // Implemented in DHDL_sort<T>

       //---------------------------------------------------------------------
       //
       // Method-
       //       DHDL_sort<void>::debug
       //
       // Purpose-
       //       Debugging display
       //
       //---------------------------------------------------------------------
       void debug(const char* info="") const;

       //---------------------------------------------------------------------
       //
       // Method-
       //       DHDL_sort<void>::fifo
       //
       // Purpose-
       //       Insert a _Link onto the list with FIFO ordering.
       //
       //---------------------------------------------------------------------
       void fifo(_Link* link);

       //---------------------------------------------------------------------
       //
       // Method-
       //       DHDL_sort<void>::get_head
       //       DHDL_sort<void>::get_tail
       //
       // Purpose-
       //       Get the head _Link. (Implemented in DHDL_sort<T>, not here,)
       //       Get the tail _Link. (Implemented in DHDL_sort<T>, not here.)
       //
       //---------------------------------------------------------------------

       //---------------------------------------------------------------------
       //
       // Method-
       //       DHDL_sort<void>::insert
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
       //       DHDL_sort<void>::is_coherent
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
       //       DHDL_sort<void>::is_on_list
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
       //       DHDL_sort<void>::lifo
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
       //       DHDL_sort<void>::remove
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
       //       DHDL_sort<void>::remq
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
       //       DHDL_sort<void>::reset
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
       //       DHDL_sort<void>::size
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
       //       DHDL_sort<void>::sort
       //
       // Purpose-
       //       Sort the _Links
       //
       //---------------------------------------------------------------------
       void sort( void );           // Sort the Links

       //---------------------------------------------------------------------
       //
       // Method-
       //       DHDL_sort<void>::sort_split
       //       DHDL_sort<void>::sort_merge
       //
       // Purpose-
       //       Split the list into parts.
       //       Merge the parts into one sorted _Link set.
       //
       //---------------------------------------------------------------------
       _Link*                       // The set of removed _Links (left)
         sort_split(                // Split the _Link set
           _Link*&           top,   // INP: The head element
                                    // OUT: The remaining _Links (remainder)
           size_t            N);    // The number of _Links to remove

       std::pair<_Link*,_Link*>     // The combined set
         sort_merge(                // Merge
           _Link*            L,     // The Left (lower) _Link set
           _Link*            R);    // The Right (remaining) _Link set
   }; // class DHDL_sort<void>

//============================================================================
//
// Struct-
//       _DHSL_iter<T>
//
// Purpose-
//       Define the DHSL_list<T> iterator
//
//----------------------------------------------------------------------------
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

     // _Self
     // _const_cast() const noexcept
     // { return *this; }

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

//----------------------------------------------------------------------------
//
// Struct-
//       _DHSL_const_iter<T>
//
// Purpose-
//       Define the DHSL_list<T> const iterator
//
//----------------------------------------------------------------------------
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
//       Define the DHSL_list<T> base class
//
//----------------------------------------------------------------------------
template<>
   class DHSL_list<void>
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
       //       DHSL_list<void>::debug
       //
       // Purpose-
       //       Debugging display
       //
       //---------------------------------------------------------------------
       void debug(const char* info="") const;

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

//============================================================================
//
// Struct-
//       _SHSL_iter<T>
//
// Purpose-
//       Define the SHDL_list<T> iterator
//
//----------------------------------------------------------------------------
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

     // _Self
     // _const_cast() const noexcept
     // { return *this; }

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
// Struct-
//       _SHSL_const_iter<T>
//
// Purpose-
//       Define the SHDL_list<T> const iterator
//
//----------------------------------------------------------------------------
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
//       _SHSL_list<void>
//
// Purpose-
//       Define the SHSL_list<T> base class
//
//----------------------------------------------------------------------------
template<>
   class SHSL_list<void>
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
       //       SHSL_list<void>::debug
       //
       // Purpose-
       //       Debugging display
       //
       //---------------------------------------------------------------------
       void debug(const char* info="") const;

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
