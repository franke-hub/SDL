//----------------------------------------------------------------------------
//
//       Copyright (c) 2010-2018 Frank Eskesen.
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
//       Standard Dispatch Item objects. (Included from Dispatch.h)
//
// Last change date-
//       2018/01/01
//
//----------------------------------------------------------------------------
#ifndef DISPATCHITEM_H_INCLUDED
#define DISPATCHITEM_H_INCLUDED

#ifndef DISPATCH_H_INCLUDED
#include "Dispatch.h"
#endif

//----------------------------------------------------------------------------
//
// Class-
//       DispatchItem
//
// Purpose-
//       The Dispatcher work Item Object.
//
// Implementation notes-
//       The negative function codes are handled internally by the Dispatch
//       object. They are not passed to DispatchTask::work().
//
//       When post() is invoked:
//         if done == NULL, the DispatchItem is deleted.
//         if done != NULL, done->done(this) is invoked.
//
//----------------------------------------------------------------------------
class DispatchItem : public AU_List<DispatchItem>::Link { // A dispatcher work item
friend class DispatchTask;          // Accesses AU_Link in method debug
//----------------------------------------------------------------------------
// DispatchItem::Enumerations and typedefs
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
,  FC_RESET= (-3)                   // Reset the Task
,  FC_VALID= 0                      // All user function codes are positive
}; // enum FC

//----------------------------------------------------------------------------
// DispatchItem::Attributes
//----------------------------------------------------------------------------
protected:
int                    fc;          // Function code/modifier
int                    cc;          // Completion code
DispatchDone*          done;        // -> Done callback object

//----------------------------------------------------------------------------
// DispatchItem::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~DispatchItem( void );           // Destructor
   DispatchItem( void );            // Default constructor

   DispatchItem(                    // Constructor
     int               fc,          // Function code
     DispatchDone*     done = NULL); // -> Done callback object

private:                            // Bitwise copy is prohibited
   DispatchItem(const DispatchItem&); // Disallowed copy constructor
   DispatchItem& operator=(const DispatchItem&); // Disallowed assignment operator

//----------------------------------------------------------------------------
// DispatchItem::Accessor methods
//----------------------------------------------------------------------------
public:
void
   debug( void ) const;             // Debugging display

inline int                          // The completion code
   getCC( void ) const              // Get completion code
{
   return cc;
}

inline int                          // The function code
   getFC( void ) const              // Get function code
{
   return fc;
}

inline DispatchDone*                // -> Done object
   getDone( void ) const            // Get Done object
{
   return done;
}

inline void
   setFC(                           // Set function code
     int               fc)          // To this value
{
   this->fc= fc;
}

inline void
   setDone(                         // Replace Done object
     DispatchDone*     done)        // With this one
{
   this->done= done;
}

//----------------------------------------------------------------------------
// DispatchItem::Methods
//----------------------------------------------------------------------------
public:
void
   post(                            // Complete the work Item
     int               cc);         // With this completion code
}; // class DispatchItem

#endif // DISPATCHITEM_H_INCLUDED
