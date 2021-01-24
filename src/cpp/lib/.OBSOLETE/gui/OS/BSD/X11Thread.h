//----------------------------------------------------------------------------
//
//       Copyright (c) 2010 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       OS/BSD/X11Thread.h
//
// Purpose-
//       Graphical User Interface: Thread implementation (X11).
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
// Only included from OS/BSD/Device.cpp

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class X11Device;

//----------------------------------------------------------------------------
//
// Class-
//       X11Thread
//
// Purpose-
//       Define system dependent implementation methods.
//
//----------------------------------------------------------------------------
class X11Thread : public Thread  {  // X11 Thread
friend class X11Device;
//----------------------------------------------------------------------------
// X11Thread::Attributes
//----------------------------------------------------------------------------
protected:
int                    operational; // TRUE iff Thread operational
X11Device*             device;      // Associated device

//----------------------------------------------------------------------------
// X11Thread::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~X11Thread( void );              // Destructor
   X11Thread(                       // Constructor
     X11Device*        device);     // -> Source Device

//----------------------------------------------------------------------------
// X11Thread::Methods
//----------------------------------------------------------------------------
public:
virtual int                         // Return code (UNUSED)
   notify(                          // Notify (TERMINATE) the Thread
     int               id);         // Notification identifier

virtual long                        // The Thread's return code
   run( void );                     // Operate the Thread
}; // class X11Thread

