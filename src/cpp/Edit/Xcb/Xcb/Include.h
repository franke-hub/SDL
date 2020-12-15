//----------------------------------------------------------------------------
//
//       Copyright (C) 2020 Frank Eskesen.
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
//       2020/12/12
//
//----------------------------------------------------------------------------
#if false                           // Always false
static_assert(false, "Always False");
#elif true
   #include <Xcb/Active.h>
static_assert(false, "Active");
#elif true
   #include <Xcb/Device.h>
static_assert(false, "Device");
#elif true
   #include <Xcb/EdFile.h>
static_assert(false, "EdFile");
#elif true
   #include <Xcb/EdHist.h>
static_assert(false, "EdHist");
#elif true
   #include <Xcb/EdMark.h>
static_assert(false, "EdMark");
#elif true
   #include <Xcb/EdMisc.h>
static_assert(false, "EdMisc");
#elif true
   #include <Xcb/EdPool.h>
static_assert(false, "EdPool");
#elif true
   #include <Xcb/EdText.h>
static_assert(false, "EdText");
#elif true
   #include <Xcb/EdView.h>
static_assert(false, "EdView");
#elif true
   #include <Xcb/Editor.h>
static_assert(false, "Editor");
#elif true
   #include <Xcb/Font.h>
static_assert(false, "Font");
#elif true
   #include <Xcb/Global.h>
static_assert(false, "Global");
#elif true
   #include <Xcb/Keysym.h>
static_assert(false, "Keysym");
#elif true
   #include <Xcb/Layout.h>
static_assert(false, "Layout");
#elif true
   #include <Xcb/TextWindow.h>
static_assert(false, "TextWindow");
#elif true
   #include <Xcb/Types.h>
static_assert(false, "Types");
#elif true
   #include <Xcb/Widget.h>
static_assert(false, "Widget");
#elif true
   #include <Xcb/Window.h>
static_assert(false, "Window");
#else
static_assert(false, "NOTHING INCLUDED");
#endif
