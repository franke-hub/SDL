//----------------------------------------------------------------------------
//
//       Copyright (c) 2018-2020 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       DispatchItem.h
//
// Purpose-
//       Standard Dispatch work Item object.
//
// Last change date-
//       2020/06/13
//
//----------------------------------------------------------------------------
#ifndef _PUB_DISPATCHITEM_H_INCLUDED
#define _PUB_DISPATCHITEM_H_INCLUDED

#include "List.h"                   // Base class
#include "DispatchDone.h"           // Used in post

namespace _PUB_NAMESPACE::Dispatch {
//----------------------------------------------------------------------------
//
// Class-
//       Item
//
// Purpose-
//       The Dispatcher work Item Object.
//
// Implementation notes-
//       The negative function codes are handled internally by the Dispatch
//       object. They are not passed to DispatchTask::work().
//
//       When post() is invoked:
//         if done == nullptr, the Item is deleted.
//         if done != nullptr, done->done(this) is invoked.
//
//----------------------------------------------------------------------------
class Item : public AU_List<Item>::Link { // A dispatcher work item
//----------------------------------------------------------------------------
// Item::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
enum CC                             // Completion codes
{  CC_NORMAL= 0                     // Normal (OK)
,  CC_ERROR                         // Generic error
,  CC_PURGE                         // Function purged
,  CC_INVALID_FC                    // Function code rejected
}; // enum CC

enum FC                             // Function codes
{  FC_CHASE= (-1)                   // Chase (NOP)
,  FC_TRACE= (-2)                   // Trace (NOP)
,  FC_RESET= (-3)                   // Reset the Worker
,  FC_VALID= 0                      // All user function codes are positive
}; // enum FC

//----------------------------------------------------------------------------
// Item::Attributes
//----------------------------------------------------------------------------
public:
int                    fc;          // Function code
int                    cc;          // Completion code
Done*                  done;        // Completion callback
void*                  work;        // (Available for application usage)

//----------------------------------------------------------------------------
// Item::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Item( void ) {}                 // Destructor
   Item( void )                     // Default constructor
:  fc(FC_VALID), cc(CC_NORMAL), done(nullptr) {}

   Item(                            // Constructor
     int               fc,          // Function code
     Done*             done= nullptr) // -> Done callback object
:  fc(fc), cc(CC_NORMAL), done(done) {}

private:                            // Bitwise copy is prohibited
   Item(const Item&); // Disallowed copy constructor
   Item& operator=(const Item&); // Disallowed assignment operator

//----------------------------------------------------------------------------
// Item::Methods
//----------------------------------------------------------------------------
public:
virtual void
   debug( void ) const;             // Debugging display

void
   post(                            // Complete the Work Item
     int               cc= 0)       // With this completion code
{
   if( done == nullptr )
     delete this;
   else {
     this->cc= cc;
     done->done(this);
   }
}
}; // class Item
}  // namespace _PUB_NAMESPACE::Dispatch
#endif // _PUB_DISPATCHITEM_H_INCLUDED
