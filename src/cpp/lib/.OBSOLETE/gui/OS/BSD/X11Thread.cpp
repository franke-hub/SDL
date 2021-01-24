//----------------------------------------------------------------------------
//
//       Copyright (c) 2010-2020 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       OS/BSD/X11Thread.cpp
//
// Purpose-
//       Graphical User Interface: X11Thread implementation.
//
// Last change date-
//       2020/10/03
//
//----------------------------------------------------------------------------
// Included only from OS/BSD/Thread.cpp

//----------------------------------------------------------------------------
//
// Method-
//       X11Thread::~X11Thread
//
// Purpose-
//       Destructor
//
//----------------------------------------------------------------------------
   X11Thread::~X11Thread( void )    // Destructor
{
   IFHCDM( Logger::log("%4d X11Thread(%p)::~X11Thread()\n", __LINE__, this); )
}

//----------------------------------------------------------------------------
//
// Method-
//       X11Thread::X11Thread
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
   X11Thread::X11Thread(            // Constructor
     X11Device*        device)      // Source X11Device
:  Thread()
,  operational(TRUE)
,  device(device)
{
   IFHCDM( Logger::log("%4d X11Thread(%p)::X11Thread(%p)\n",
                  __LINE__, this, device); )
}

//----------------------------------------------------------------------------
//
// Method-
//       X11Thread::notify
//
// Purpose-
//       Notify (TERMINATE) the Thread
//
//----------------------------------------------------------------------------
int                                 // Return code (UNUSED)
   X11Thread::notify(               // Notify (TERMINATE) the Thread
     int               id)          // Notification ID
{
   IFHCDM( Logger::log("%4d X11Thread(%p)::notify(%d)\n",
                       __LINE__, this, id); )
   ELHCDM((void)id;)                // Unused parameter

   operational= FALSE;              // Timer takes care of the rest

   return 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       X11Thread::run
//
// Purpose-
//       Operate the Thread
//
// Implementation note-
//       device->nextEvent() can be called either ONLY by the device
//       (when USE_X11THREAD == FALSE,) or ONLY by the thread. We don't
//       want both the thread and the device sharing X calls.
//
//       Also see- X11Device::nextEvent
//
//----------------------------------------------------------------------------
long
   X11Thread::run( void )           // Operate the Thread
{
   IFHCDM( Logger::log("%4d X11Thread(%p)::run()...\n", __LINE__, this); )

   int                 rc;

   try {
     Interval interval;
     while( operational && device->operational )
     {
////   debugf("%4d THREAD HCDM\n", __LINE__);

       {{{{
         AutoMutex lock(device->unitMutex);

         rc= XPending(device->disp);
         X11DEBUG(rc, "XPending-Thread");
       }}}}
       if( rc == 0 )
       {
         double t=interval.stop();
         if( t < 0.01 )
           t= 0.001;
         else if( t > 1.0 )
           t= 1.0;

         t *= 2.0;
         Thread::sleep(t);
       }

       if( rc != 0 )
       {
         while( rc != 0 )
         {
           XEvent e;
           device->nextEvent(e);
           if( !operational || !device->operational )
             break;

           {{{{
             AutoMutex lock(device->unitMutex);

             rc= XPending(device->disp);
             X11DEBUG(rc, "XPending-Thread");
           }}}}
         }

         interval.start();
       }
     }
   } catch(const char* X) {
     #ifdef HCDM
       debugf("X11Thread.catch(const char*(%s))\n", X);
     #else
       fprintf(stderr, "X11Thread.catch(const char*(%s))\n", X);
     #endif
   } catch(...) {
     #ifdef HCDM
       debugf("X11Thread.catch(...)\n");
     #else
       fprintf(stderr, "X11Thread.catch(...)\n");
     #endif
   }

   operational= FALSE;
   device->operational= FALSE;
   IFHCDM( Logger::log("%4d ...X11Thread(%p)::run()\n", __LINE__, this); )
   return 0;
}

