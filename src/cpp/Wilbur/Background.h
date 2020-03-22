//----------------------------------------------------------------------------
//
//       Copyright (c) 2011-2018 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Background.h
//
// Purpose-
//       Drive background services.
//
// Last change date-
//       2018/01/01
//
//----------------------------------------------------------------------------
#ifndef BACKGROUND_H_INCLUDED
#define BACKGROUND_H_INCLUDED

#include <com/Dispatch.h>

#include "BG_CleanCache.h"

//----------------------------------------------------------------------------
//
// Class-
//       BackgroundTask
//
// Purpose-
//       Drive background services.
//
//----------------------------------------------------------------------------
class BackgroundTask : public DispatchTask {
//----------------------------------------------------------------------------
// BackgroundTask::Attributes
//----------------------------------------------------------------------------
protected:
BG_CleanCache          cleanCache;  // The cache cleaner

//----------------------------------------------------------------------------
// BackgroundTask::Constructors
//----------------------------------------------------------------------------
public:
   ~BackgroundTask( void );         // Destructor
   BackgroundTask( void );          // Constructor

//----------------------------------------------------------------------------
// BackgroundTask::Methods
//----------------------------------------------------------------------------
public:
void
   close( void );                   // Reset background Tasks

virtual void
   work(                            // Process
     DispatchItem*     item);       // This work Item
}; // class BackgroundTask

//----------------------------------------------------------------------------
//
// Class-
//       Background
//
// Purpose-
//       Drive background services and insure they terminate properly.
//
// Services-
//       BG_HttpCache        HTTP Cache cleanup.
//
//----------------------------------------------------------------------------
class Background : public DispatchWait {
//----------------------------------------------------------------------------
// Background::Attributes
//----------------------------------------------------------------------------
protected:
BackgroundTask         task;        // Our Task Block
DispatchItem           item;        // Our (only) work Item
DispatchWait           wait;        // Posted when background terminates

//----------------------------------------------------------------------------
// Background::Constructors
//----------------------------------------------------------------------------
public:
   ~Background( void );             // Destructor
   Background( void );              // Constructor

//----------------------------------------------------------------------------
// Background::Methods
//----------------------------------------------------------------------------
public:
virtual void
   done(                            // Complete
     DispatchItem*     item);       // This work Item
}; // class Background

#endif // BACKGROUND_H_INCLUDED
