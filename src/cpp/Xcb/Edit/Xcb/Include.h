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
//       2020/09/06
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
   #include <Xcb/EdFind.h>
static_assert(false, "EdFind");
#elif true
   #include <Xcb/Editor.h>
static_assert(false, "Editor");
#elif true
   #include <Xcb/EdMain.h>
static_assert(false, "EdMain");
#elif true
   #include <Xcb/EdMenu.h>
static_assert(false, "EdMeun");
#elif true
   #include <Xcb/EdMisc.h>
static_assert(false, "EdMisc");
#elif true
   #include <Xcb/EdPool.h>
static_assert(false, "EdPool");
#elif true
   #include <Xcb/EdTabs.h>
static_assert(false, "EdTabs");
#elif true
   #include <Xcb/EdText.h>
static_assert(false, "EdText");
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
   #include <Xcb/TestWindow.h>
static_assert(false, "TestWindow");
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
