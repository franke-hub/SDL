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
//       2022/09/02
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
//       The negative function codes are handled internally by the Dispatch
//       object. They are not passed to DispatchTask::work().
//
//       When post() is invoked:
//         if done == nullptr, the Item is deleted.
//         if done != nullptr, done->done(this) is invoked.
//
//----------------------------------------------------------------------------
class Item : public AI_list<Item>::Link { // A dispatcher work item
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
   ~Item( void ) { _term(); }       // Destructor
   Item( void )                     // Default constructor
:  fc(FC_VALID), cc(CC_NORMAL), done(nullptr) { _init(); }

   Item(                            // Constructor
     int               fc,          // Function code
     Done*             done= nullptr) // -> Done callback object
:  fc(fc), cc(CC_NORMAL), done(done) { _init(); }

private:
   void _init() const noexcept;
   void _term() const noexcept;

   Item(const Item&) = delete;      // Disallowed copy constructor
   Item& operator=(const Item&) = delete; // Disallowed assignment operator

//----------------------------------------------------------------------------
// Item::Methods
//----------------------------------------------------------------------------
public:
virtual void
   debug(const char* info) const;   // Debugging display

void debug( void ) { debug(""); }

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
}  // namespace dispatch
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_DISPATCHITEM_H_INCLUDED
