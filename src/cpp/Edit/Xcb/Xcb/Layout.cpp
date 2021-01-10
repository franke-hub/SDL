//----------------------------------------------------------------------------
//
//       Copyright (C) 2020 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Xcb/Layout.cpp
//
// Purpose-
//       Implement Xcb/Layout.h
//
// Last change date-
//       2021/01/10
//
//----------------------------------------------------------------------------
#include <mutex>                    // For std::lock_guard

#include <pub/Debug.h>              // For Debug object

#include "Xcb/Device.h"             // For Device
#include "Xcb/Layout.h"             // Implementation class
#include "Xcb/Window.h"             // For Window

using pub::Debug;                   // For Debug object
using namespace pub::debugging;     // For debugging

namespace xcb {
//----------------------------------------------------------------------------
// External data areas
//----------------------------------------------------------------------------
// Layout                 Layout::zero_Layout= { {0, 0, 0, 0}, {0, 0}
//                            , {0, 0}, {0, 0}, {0, 0}, {0, 0} };

//----------------------------------------------------------------------------
// DEBUGGING: TODO: REMOVE
//----------------------------------------------------------------------------
void
   Layout::config_t::debug(const char* name, const char* info) const
{
   debugf("Config(%s)::debug(%s)", name ? name : "", info ? info : "" );
   debugf(", rect(%d,%d,%u,%u)", rect.x, rect.y, rect.width, rect.height);
   debugf(", cur_disp(%u,%u)\n", cur_disp.x,     cur_disp.y);
   debugf(": max_size(%u,%u)",   max_size.width, max_size.height);
   debugf(", min_size(%u,%u)",   min_size.width, min_size.height);
   debugf(", use_size(%u,%u)",   use_size.width, use_size.height);
   debugf(", use_unit(%u,%u)\n", use_unit.width, use_unit.height);
}

void
   Layout::config_inp(config_t config, const char* type)
{
   debugf(">>>>>>>>>>>>>>>> %s >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n", type);
   config.debug(get_name().c_str(), type);
   debug(type);
}

void
   Layout::config_out(config_t config, const char* type)
{
   debugf("\n");
   debugf("<<<<<<<<<<<<<<<< %s <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n", type);
   config.debug(get_name().c_str(), type);
   debug(type);
}


//----------------------------------------------------------------------------
// Utilities
//----------------------------------------------------------------------------
static inline WH_size_t
   add(
     WH_size_t         lhs,
     const WH_size_t&  rhs)
{
// lhs.width  += rhs.width;         // (Ok in GCC version 10.2.0
// lhs.height += rhs.height;
   lhs.width=  WH_t(lhs.width + rhs.width); // (Ok in GCC version 9.3.0 also)
   lhs.height= WH_t(lhs.height + rhs.height);
   return lhs;
}

static inline WH_size_t
   add(
     WH_size_t         lhs,
     const XY_size_t&  rhs)
{
// lhs.width  += rhs.x;             // (Ok in GCC version 10.2.0
// lhs.height += rhs.y;
   lhs.width=  WH_t(lhs.width + rhs.x); // (Ok in GCC version 9.3.0 also)
   lhs.height= WH_t(lhs.height + rhs.y);
   return lhs;
}

static inline WH_size_t
   max(
     WH_size_t         lhs,
     const WH_size_t&  rhs)
{
   if( rhs.width > lhs.width )
     lhs.width= rhs.width;
   if( rhs.height > lhs.height )
     lhs.height= rhs.height;
   return lhs;
}

//----------------------------------------------------------------------------
//
// Method-
//       Layout::Layout
//       Layout::~Layout
//
// Purpose-
//       Constructor/Destructor.
//
//----------------------------------------------------------------------------
   Layout::Layout(                  // Constructor
     Widget*           parent,      // Our parent Widget
     const char*       name)        // Widget name
:  Widget(parent, name)
{
   if( opt_hcdm && opt_verbose > 1)
     debugh("Layout(%p)::Layout(%p)\n", this, parent);
}

   Layout::~Layout( void )          // Destructor
{
   if( opt_hcdm && opt_verbose > 1 )
     debugh("Layout(%p)::~Layout\n", this);
}

//----------------------------------------------------------------------------
//
// Method-
//       Layout::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   Layout::debug(const char* info) const
{
   debugf("Layout(%p)::debug(%s) Named(%s)", this, info ? info : ""
         , get_name().c_str());
   debugf(", rect(%d,%d,%u,%u)", rect.x, rect.y, rect.width, rect.height);
   debugf(", cur_disp(%u,%u)\n", cur_disp.x,     cur_disp.y);
   debugf(": max_size(%u,%u)",   max_size.width, max_size.height);
   debugf(", min_size(%u,%u)",   min_size.width, min_size.height);
   debugf(", use_size(%u,%u)",   use_size.width, use_size.height);
   debugf(", use_unit(%u,%u)\n", use_unit.width, use_unit.height);
}

//----------------------------------------------------------------------------
//
// Method-
//       Layout::configure
//
// Purpose-
//       Layout configurator, using default BoxLayout
//
//----------------------------------------------------------------------------
void
   Layout::configure(               // Configure Layout
     config_t&         config)      // Layout configuration controls
{
   if( opt_hcdm ) {
     debugh("Layout(%p)::configure Named(%s)\n", this, get_name().c_str());
     if( opt_verbose >= 0 )
       config_inp(config, "Box");
   }

   // Box configurator
   cur_disp= config.cur_disp;       // Accept current position
   use_size= max(use_size, min_size); // Desired >= Minimum size
   max_size= max(max_size, use_size); // Maximum >= Desired size
   rect.x= cur_disp.x;
   rect.y= cur_disp.y;

   config_t modfig= { config.rect, config.cur_disp
                    , max_size, min_size, use_size, use_unit};

   for(Widget* widget= get_first(); widget; widget= widget->get_next()) {
     Layout* layout= dynamic_cast<Layout*>(widget);
     if( layout ) {
       if( opt_hcdm && opt_verbose >= 0 ) {
         debugh("\n");
         debugh("BoxLayout(%s)--------------------------------------------\n"
               , name.c_str());
       }
       layout->configure(modfig);
     }
   }

   // Update the configuration
// config.rect=     ** UNCHANGED **
// config.cur_disp= ** UNCHANGED **
   config.max_size= max(config.max_size, add(modfig.max_size, cur_disp));
   config.min_size= max(config.min_size, add(modfig.min_size, cur_disp));
   config.use_size= max(config.use_size, add(modfig.use_size, cur_disp));
   config.use_unit= max(config.use_unit, modfig.use_unit);
   rect.width= use_size.width;
   rect.height= use_size.height;

   if( opt_hcdm && opt_verbose >= 0 )
     config_out(config, "Box");
}

//----------------------------------------------------------------------------
//
// Method-
//       ColLayout::ColLayout
//       ColLayout::~ColLayout
//
// Purpose-
//       Constructor/Destructor.
//
//----------------------------------------------------------------------------
   ColLayout::ColLayout(            // Constructor
     Widget*           parent,      // Parent Widget
     const char*       name)        // Widget name
:  Layout(parent, name)
{
   if( opt_hcdm && opt_verbose > 1)
     debugh("ColLayout(%p)::ColLayout(%p)\n", this, parent);
}

   ColLayout::~ColLayout( void )    // Destructor
{
   if( opt_hcdm && opt_verbose > 1 )
     debugh("ColLayout(%p)::~ColLayout\n", this);
}

//----------------------------------------------------------------------------
//
// Method-
//       ColLayout::configure
//
// Purpose-
//       Layout configurator
//
//----------------------------------------------------------------------------
void
   ColLayout::configure(            // Configure Layout
     config_t&         config)      // Layout configuration controls
{
   if( opt_hcdm ) {
     debugh("ColLayout(%p)::configure Named(%s)\n", this, get_name().c_str());
     if( opt_verbose >= 0 )
       config_inp(config, "Col");
   }

   // Column configurator
   cur_disp= config.cur_disp;       // Accept current position
   use_size= max(use_size, min_size); // Desired >= Minimum size
   max_size= max(max_size, use_size); // Maximum >= Desired size
   rect.x= cur_disp.x;
   rect.y= cur_disp.y;

   config_t modfig= { config.rect, config.cur_disp
                    , max_size, min_size, use_size, use_unit};

   for(Widget* widget= get_first(); widget; widget= widget->get_next()) {
     Layout* layout= dynamic_cast<Layout*>(widget);
     if( layout ) {
       if( opt_hcdm && opt_verbose >= 0 ) {
         debugh("\n");
         debugh("ColLayout(%s)--------------------------------------------\n"
               , name.c_str());
       }
       modfig.use_size= {0,0};
       layout->configure(modfig);
       modfig.cur_disp.x = modfig.use_size.width;
       modfig.cur_disp.y = cur_disp.y;
     }
   }

   // Update configuration
// config.rect=     ** UNCHANGED **
   config.cur_disp.x= modfig.cur_disp.x;
// config.cur_disp.y= ** UNCHANGED **
   config.max_size= max(config.max_size, modfig.max_size);
   config.min_size= max(config.min_size, modfig.min_size);
   config.use_size= max(config.use_size, modfig.use_size);
   config.use_unit= max(config.use_unit, modfig.use_unit);
   rect.width= use_size.width;
   rect.height= use_size.height;

   if( opt_hcdm && opt_verbose >= 0 )
     config_out(config, "Col");
}

//----------------------------------------------------------------------------
//
// Method-
//       RowLayout::RowLayout
//       RowLayout::~RowLayout
//
// Purpose-
//       Constructor/Destructor.
//
//----------------------------------------------------------------------------
   RowLayout::RowLayout(            // Constructor
     Widget*           parent,      // Parent Widget
     const char*       name)        // Layout name
:  Layout(parent, name)
{
   if( opt_hcdm && opt_verbose > 1)
     debugh("RowLayout(%p)::RowLayout(%p)\n", this, parent);
}

   RowLayout::~RowLayout( void )    // Destructor
{
   if( opt_hcdm && opt_verbose > 1 )
     debugh("RowLayout(%p)::~RowLayout\n", this);
}

//----------------------------------------------------------------------------
//
// Method-
//       RowLayout::configure
//
// Purpose-
//       Layout configurator
//
//----------------------------------------------------------------------------
void
   RowLayout::configure(            // Configure Layout
     config_t&         config)      // Layout configuration controls
{
   if( opt_hcdm ) {
     debugh("RowLayout(%p)::configure Named(%s)\n", this, get_name().c_str());
     if( opt_verbose >= 0 )
       config_inp(config, "Row");
   }

   // Row configurator
   cur_disp= config.cur_disp;       // Accept current position
   use_size= max(use_size, min_size); // Desired >= Minimum size
   max_size= max(max_size, use_size); // Maximum >= Desired size
   rect.x= cur_disp.x;
   rect.y= cur_disp.y;

   config_t modfig= { config.rect, config.cur_disp
                    , max_size, min_size, use_size, use_unit};

   for(Widget* widget= get_first(); widget; widget= widget->get_next()) {
     Layout* layout= dynamic_cast<Layout*>(widget);
     if( layout ) {
       if( opt_hcdm && opt_verbose >= 0 ) {
         debugh("\n");
         debugh("RowLayout(%s)--------------------------------------------\n"
               , name.c_str());
       }
       modfig.use_size= {0,0};
       layout->configure(modfig);
       modfig.cur_disp.x = cur_disp.x;
       modfig.cur_disp.y = modfig.use_size.height;
     }
   }

   // Update configuration (NEEDS WORK)
// config.rect=     ** UNCHANGED **
// config.cur_disp.x= ** UNCHANGED **
   config.cur_disp.y= modfig.cur_disp.y;
   config.max_size= max(config.max_size, modfig.max_size);
   config.min_size= max(config.min_size, modfig.min_size);
   config.use_size= max(config.use_size, modfig.use_size);
   config.use_unit= max(config.use_unit, modfig.use_unit);
   rect.width= use_size.width;
   rect.height= use_size.height;

   if( opt_hcdm && opt_verbose >= 0 )
     config_out(config, "Row");
}
}  // namespace xcb
