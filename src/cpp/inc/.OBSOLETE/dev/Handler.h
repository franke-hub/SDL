//----------------------------------------------------------------------------
//
//       Copyright (C) 2022 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Handler.h
//
// Purpose-
//       Generic work item handler.
//
// Last change date-
//       2022/02/23
//
// Implementation notes-
//       TODO: MOVE to ~/src/cpp/inc/pub/.
//
//----------------------------------------------------------------------------
#ifndef _PUB_HANDLER_H_INCLUDED
#define _PUB_HANDLER_H_INCLUDED

#include <functional>               // For std::function

namespace pub {
//----------------------------------------------------------------------------
//
// Class-
//       Handler<T>
//
// Purpose-
//       Generic work item handler.
//
//----------------------------------------------------------------------------
template<class T>
class Handler {                     // Generic work item handler
public:
typedef std::function<void(T*)> handler_t; // Work handler function type

protected:
handler_t              handler;     // The work item handler

public:
virtual
   ~Handler( void ) = default;
   Handler( void ) = default;

void
   on_work(const handler_t& f)
{  handler= f; }

void
   work(T* item)                    // Handle work item
{  handler(item); }
}; // class Handler
}  // namespace pub
#endif // _PUB_HANDLER_H_INCLUDED
