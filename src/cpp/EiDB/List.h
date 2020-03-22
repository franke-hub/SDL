//----------------------------------------------------------------------------
//
//       Copyright (C) 2002 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       List.h
//
// Purpose-
//       Describe a generic List.
//
// Last change date-
//       2002/09/18
//
// Classes-
         class List;                // List of (void*)
//
//----------------------------------------------------------------------------
#ifndef LIST_H_INCLUDED
#define LIST_H_INCLUDED

//----------------------------------------------------------------------------
//
// Class-
//       List
//
// Purpose-
//       Describe the Exon/Intron DataBase.
//
//----------------------------------------------------------------------------
class List                          // List of (void*)
{
//----------------------------------------------------------------------------
// List::Constructors
//----------------------------------------------------------------------------
public:
   ~List( void );                   // Default destructor
   List( void );                    // Default constructor

private:                            // Bitwise copy prohibited
   List(const List&);
   List& operator=(const List&);

//----------------------------------------------------------------------------
// List::methods
//----------------------------------------------------------------------------
public:
unsigned                            // Item number
   append(                          // Add item to List
     void*             item);       // Item to append

void*                               // -> Item
   getItem(                         // Get the Item
     unsigned          index);      // Associated with this index

void
   empty( void );                   // Empty the List

void
   debug( void ) const;             // Debugging List display

//----------------------------------------------------------------------------
// List::Attributes
//----------------------------------------------------------------------------
protected:
   unsigned            count;       // Number of elements on List
   void*               head;        // -> First element on List

   unsigned            index;       // Working index
   void*               tail;        // -> Associated Item
}; // class List

#endif // LIST_H_INCLUDED
