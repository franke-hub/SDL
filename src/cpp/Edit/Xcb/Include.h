//----------------------------------------------------------------------------
//
//       Copyright (C) 2020-2021 Frank Eskesen.
//
//       This file is free content, distributed under the MIT license.
//       (See accompanying file LICENSE.MIT or the original contained
//       within https://opensource.org/licenses/MIT)
//
//----------------------------------------------------------------------------
//
// Title-
//       Include.h
//
// Purpose-
//       Test include prerequisites. Include one and only one file.
//
// Last change date-
//       2021/01/22
//
//----------------------------------------------------------------------------
#if false                           // Always false
static_assert(false, "Always False");
#elif false
   #include <Xcb/Config.h>
#elif false
   #include <Xcb/Device.h>
#elif false
   #include <Xcb/EdFile.h>
#elif false
   #include <Xcb/EdHist.h>
#elif false
   #include <Xcb/EdMark.h>
#elif false
   #include <Xcb/EdMisc.h>
#elif false
   #include <Xcb/EdPool.h>
#elif false
   #include <Xcb/EdText.h>
#elif false
   #include <Xcb/EdView.h>
#elif false
   #include <Xcb/Editor.h>
#elif false
   #include <Xcb/Font.h>
#elif false
   #include <Xcb/Global.h>
#elif false
   #include <Xcb/Keysym.h>
#elif false
   #include <Xcb/Layout.h>
#elif false
   #include <Xcb/Pixmap.h>
#elif false
   #include <Xcb/Types.h>
#elif false
   #include <Xcb/Widget.h>
#elif false
   #include <Xcb/Window.h>
#else
static_assert(0 == 1, "NOTHING INCLUDED");
#endif
