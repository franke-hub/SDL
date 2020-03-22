//----------------------------------------------------------------------------
//
//       Copyright (c) 2010 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       HttpClientThread.cpp
//
// Purpose-
//       HttpClientThread implementation methods.
//
// Last change date-
//       2010/01/01
//
// Implementation notes-
//       The HttpClientThread uses static objects.
//       Only one HttpClientThread instance can exist.
//
//----------------------------------------------------------------------------
#if defined(_OS_CYGWIN) || defined(_OS_WIN)
  #include <process.h>
#endif
#include <unistd.h>
#include <stdio.h>

#include <com/Barrier.h>
#include <com/Status.h>
#include <com/Thread.h>

#include <gui/Action.h>
#include <gui/Event.h>
#include <gui/Font.h>
#include <gui/KeyCode.h>
#include <gui/Text.h>
#include <gui/Window.h>

#include "Common.h"
#include "HttpClientThread.h"
using namespace GUI;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef SCDM
#undef  SCDM                        // If defined, Soft Core Debug Mode
#endif

#ifndef IODM
#undef  IODM                        // If defined, Input/Output Debug Mode
#endif

#include <com/ifmacro.h>

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define USE_WINDOW FALSE            // TRUE to create control window

//----------------------------------------------------------------------------
//
// Subroutine-
//       openBrowser
//
// Purpose-
//       Open browser window
//
//----------------------------------------------------------------------------
static inline void
   openBrowser(                     // Open browser window
     const char*       url)         // For this URL
{
   logf("HttpClientThread::openBrowser(%s)\n", url);

   const char* program= "firefox";
   const char* const param[]= {program, url,  NULL};

   #if defined(_OS_WIN) || defined(_OS_CYGWIN)
     #if defined(_OS_WIN)
       program= "C:\\Program Files\\Mozilla Firefox\\firefox.exe";
     #elif defined(_OS_CYGWIN)
       program= "/cygdrive/C/Program Files/Mozilla Firefox/firefox.exe";
     #endif

     int rc= spawnvp(_P_NOWAITO, program, param);
     logf("%d= spawnvp(\"%s\",\"%s\")\n", rc, program, url);
   #else                            // _OS_BSD
     pid_t pid= fork();
     if( pid == 0 )
     {
       int rc= execvp(program, (char* const*)param);
       logf("%d= execvp(\"%s\",\"%s\")\n", rc, program, url);
     }
   #endif
}

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static const XYLength  winLength= {512, 128};
static const XYOffset  texOffset= {  0,  64};
static const XYLength  texLength= {512,  64};

static Barrier         barrier= BARRIER_INIT; // Serialization control
static Status          status;      // Window completion control

#if( USE_WINDOW )
static Window          window(winLength); // Our GUI Window
static Text            text1(&window, texLength); // Our Text
static Text            text2(&window, texOffset, texLength); // Our Text
static Font            font(NULL);  // Our Font

//----------------------------------------------------------------------------
//
// Class-
//       HttpClientThread_Action
//
// Purpose-
//       Our action control.
//
//----------------------------------------------------------------------------
static class HttpClientThread_Action : public Action {
public:
virtual
   HttpClientThread_Action::~HttpClientThread_Action( void ) {}
   HttpClientThread_Action::HttpClientThread_Action(
     Object*           parent= NULL)
:  Action(parent) {}

virtual void
   callback(                        // Handle callback
     const Event&      e)           // For this Event
{
   Common* common= Common::get();

   unsigned code= e.getCode();
   unsigned data= e.getData();
   logf("HttpClientThread_Action::callback() code(%d), data(%x)\n", code, data);
   char buffer[128];
   unsigned addr= common->serverThread.getAddr();
   unsigned port= common->serverThread.getPort();
   sprintf(buffer, "http://%d.%d.%d.%d:%d",
                   ((addr >> 24) & 0x000000ff),
                   ((addr >> 16) & 0x000000ff),
                   ((addr >>  8) & 0x000000ff),
                   ((addr >>  0) & 0x000000ff),
                   port);

   switch(code)
   {
     case Event::EC_KEYDOWN:
       if( data == ((KeyCode::_CTRL) | 'c')
           || data == ((KeyCode::_CTRL) | 'C') )
         common->httpClient.notify(0);
       else if( data == ((KeyCode::_CTRL) | 's')
           || data == ((KeyCode::_CTRL) | 'S') )
         openBrowser(buffer);
       break;

     default:
       break;
   }
}
}                      action(&window); // class HttpClientThread_Action
#endif // USE_WINDOW

//----------------------------------------------------------------------------
//
// Method-
//       HttpClientThread::~HttpClientThread
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   HttpClientThread::~HttpClientThread( void )
{
   logf("HttpClientThread(%p)::~HttpClientThread()\n", this);

#if( USE_WINDOW )
   window.setAttribute(Window::VISIBLE, FALSE);
#endif
}

//----------------------------------------------------------------------------
//
// Method-
//       HttpClientThread::HttpClientThread
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   HttpClientThread::HttpClientThread( void )
:  NamedThread("HTTP::Client")
,  fsm(FSM_RESET)
{
   logf("HttpClientThread(%p)::HttpClientThread()\n", this);

   AutoBarrier lock(barrier);

#if( USE_WINDOW )
   text1.setFont(&font);
   text1.setJustification(Justification::LR_CENTER | Justification::TB_CENTER);
   text1.setText("Use CTRL-C to terminate");

   text2.setFont(&font);
   text2.setJustification(Justification::LR_CENTER | Justification::TB_CENTER);
   text2.setText("Use CTRL-S to start browser");
   window.redraw();
   window.setAttribute(Window::VISIBLE, TRUE);
#endif

   fsm= FSM_READY;
}

//----------------------------------------------------------------------------
//
// Method-
//       HttpClientThread::notify
//
// Purpose-
//       Termination notification.
//
//----------------------------------------------------------------------------
int                                 // Return code
   HttpClientThread::notify(        // Notify HttpClientThread
     int               code)        // Notification code
{
   logf("HttpClientThread(%p)::notify(%d)\n", this, code);

   AutoBarrier lock(barrier);
   if( fsm == FSM_READY )
   {
     fsm= FSM_CLOSE;
     status.post(code);
   }

   return 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       HttpClientThread::run
//
// Purpose-
//       HttpClient driver.
//
//----------------------------------------------------------------------------
long                                // Return code
   HttpClientThread::run( void )    // HttpClient driver
{
   logf("HttpClientThread(%p)::run\n", this);

   status.wait();

   logf("HttpClientThread(%p)::terminated\n", this);

   return 0;
}

