//----------------------------------------------------------------------------
//
//       Copyright (C) 2020-2021 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       gui/Layout.h
//
// Purpose-
//       XCB Layout Widget descriptor
//
// Last change date-
//       2021/01/22
//
//----------------------------------------------------------------------------
#ifndef GUI_LAYOUT_H_INCLUDED
#define GUI_LAYOUT_H_INCLUDED

#include <string>                   // For std::string

#include "Global.h"                 // For opt_* controls
#include "Types.h"                  // For gui and system types
#include "Widget.h"                 // For Widget

namespace gui {
//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class Device;
class Window;

//----------------------------------------------------------------------------
//
// Class-
//       Layout
//
// Purpose-
//       Extend a class to include Layout functions.
//
//----------------------------------------------------------------------------
class Layout : public Widget {      // Layout widget
//----------------------------------------------------------------------------
// Layout::config_t
//----------------------------------------------------------------------------
public:
struct config_t {                   // Parameters to Layout::configure
xcb_rectangle_t        rect= {0,0,0,0}; // Current rectangle
XY_size_t              cur_disp= {0,0}; // Current (origin 0) X/Y displacement
WH_size_t              max_size= {0,0}; // Maximum usable size (zero if none)
WH_size_t              min_size= {0,0}; // Minimum usable size (zero if none)
WH_size_t              use_size= {0,0}; // Desired size
WH_size_t              use_unit= {0,0}; // The size of each unit (zero == one)

void
   debug(const char* name= nullptr, const char* info= nullptr) const;
}; // struct config_t

//----------------------------------------------------------------------------
// Layout::Attributes
//----------------------------------------------------------------------------
public:                             // Exactly matches Layout::config_t
xcb_rectangle_t        rect= {0,0,0,0}; // Updated geometry
XY_size_t              cur_disp= {0,0}; // Current (origin 0) X/Y displacement
WH_size_t              max_size= {0,0}; // Maximum usable size (zero if none)
WH_size_t              min_size= {0,0}; // Minimum usable size (zero if none)
WH_size_t              use_size= {0,0}; // Desired size
WH_size_t              use_unit= {0,0}; // The size of each unit (zero == one)

//----------------------------------------------------------------------------
// BRINGUP: Debugging displays. TODO: REMOVE???
//----------------------------------------------------------------------------
void
   config_inp(config_t config, const char* type);

void
   config_out(config_t config, const char* type);

//----------------------------------------------------------------------------
// Layout::Constructors/Destructors
//----------------------------------------------------------------------------
public:
   Layout(                          // Constructor
     Widget*           parent= nullptr, // Our parent Widget
     const char*       name= nullptr); // Widget name

virtual
   ~Layout( void );                 // Destructor

//----------------------------------------------------------------------------
//
// Method-
//       Layout::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
public:
void
   debug(const char* info= nullptr) const; // Debugging display

//----------------------------------------------------------------------------
//
// Method-
//       Layout::configure
//
// Purpose-
//       Layout configurator (The base class is a Box Layout)
//
//----------------------------------------------------------------------------
virtual void
   configure(                       // Configure Layout
     config_t&         config);     // Layout configuration controls
}; // class Layout

//----------------------------------------------------------------------------
//
// Class-
//       ColLayout
//
// Purpose-
//       Column Layout. Subcomponents are vertical columns.
//
//----------------------------------------------------------------------------
class ColLayout : public Layout {   // Column Layout
//----------------------------------------------------------------------------
// ColLayout::Constructors/Destructors
//----------------------------------------------------------------------------
public:
virtual
   ~ColLayout( void );              // Destructor
   ColLayout(                       // Constructor
     Widget*           parent= nullptr, // Parent Widget
     const char*       name= nullptr); // Widget name

//----------------------------------------------------------------------------
// ColLayout::Methods
//----------------------------------------------------------------------------
public:
virtual void
   configure(                       // Configure Layout
     config_t&         config);     // Layout configuration controls
}; // class ColLayout

//----------------------------------------------------------------------------
//
// Class-
//       RowLayout
//
// Purpose-
//       Row Layout. Subcomponents are horizontal rows.
//
//----------------------------------------------------------------------------
class RowLayout : public Layout {   // Row Layout
//----------------------------------------------------------------------------
// RowLayout::Constructors/Destructors
//----------------------------------------------------------------------------
public:
virtual
   ~RowLayout( void );              // Destructor
   RowLayout(                       // Constructor
     Widget*           parent= nullptr, // Parent Widget
     const char*       name= nullptr); // Widget name

//----------------------------------------------------------------------------
// RowLayout::Methods
//----------------------------------------------------------------------------
public:
virtual void
   configure(                       // Configure Layout
     config_t&         config);     // Layout configuration controls
}; // class RowLayout
}  // namespace gui
#endif // GUI_LAYOUT_H_INCLUDED
