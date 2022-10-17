//----------------------------------------------------------------------------
//
//       Copyright (c) 2018-2022 Frank Eskesen.
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
//       2022/10/05
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_DISPATCHITEM_H_INCLUDED
#define _LIBPUB_DISPATCHITEM_H_INCLUDED

#include "pub/List.h"               // Base class
#include "pub/DispatchDone.h"       // Used in post

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
namespace dispatch {
//----------------------------------------------------------------------------
//
// Class-
//       Item
//
// Purpose-
//       The Dispatcher work Item Object.
//
// Implementation notes-
//       All negative function codes are handled internally by the Dispatcher.
//       They are not passed to DispatchTask::work().
//
//       When post() is invoked:
//         if done != nullptr, done->done(this) is invoked.
//         if done == nullptr, the Item is deleted.
//
//----------------------------------------------------------------------------
class Item : public AI_list<Item>::Link { // A dispatcher work item
//----------------------------------------------------------------------------
// Item::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
enum CC                             // Completion codes
{  CC_NORMAL= 0                     // Normal (OK)
,  CC_PURGE= -1                     // Function purged
,  CC_ERROR= -2                     // Generic error
,  CC_ERROR_FC= -3                  // Invalid function code
}; // enum CC

enum FC                             // Function codes
{  FC_VALID= 0                      // All user function codes are positive
,  FC_CHASE= -1                     // Chase (Handled by Dispatcher)
,  FC_TRACE= -2                     // Trace (Handled by Dispatcher)
}; // enum FC

//----------------------------------------------------------------------------
// Item::Attributes
//----------------------------------------------------------------------------
public:
int                    fc= FC_VALID;  // Function code
int                    cc= CC_NORMAL; // Completion code
Done*                  done= nullptr; // Completion callback

//----------------------------------------------------------------------------
// Item::Constructors/destructor
//----------------------------------------------------------------------------
public:
   Item( void ) = default;          // Default constructor

   Item(                            // Constructor
     Done*             done)        // -> Done callback object
:  done(done) { }

   Item(                            // Constructor
     int               fc,          // Function code
     Done*             done= nullptr) // -> Done callback object
:  fc(fc), done(done) { }

virtual
   ~Item( void ) = default;         // Destructor

private:
   Item(const Item&) = delete;      // Disallowed copy constructor
   Item& operator=(const Item&) = delete; // Disallowed assignment operator

//----------------------------------------------------------------------------
// Item::Methods
//----------------------------------------------------------------------------
public:
virtual void
   debug(const char* info= "") const; // Debugging display

void
   post(                            // Complete the Work Item
     int               user_cc= 0)  // With this completion code
{
   if( done ) {
     cc= user_cc;
     done->done(this);
   } else {
     delete this;
   }
}
}; // class Item
}  // namespace dispatch
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_DISPATCHITEM_H_INCLUDED
