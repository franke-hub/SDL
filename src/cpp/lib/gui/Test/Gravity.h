//----------------------------------------------------------------------------
//
//       Copyright (C) 2021 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Gravity.h
//
// Purpose-
//       Gravitational simulator objects.
//
// Last change date-
//       2021/01/29
//
//----------------------------------------------------------------------------
#ifndef GRAVITY_H_INCLUDED
#define GRAVITY_H_INCLUDED

#include <memory>                   // For std::shared_ptr, std::unique_ptr
#include <string>                   // For std::string
#include <xcb/xcb_image.h>          // For xcb_image_t, associated functions

#include <pub/List.h>               // For pub::List
#include <gui/Types.h>              // For GUI types
#include <gui/Window.h>             // For gui::Window

namespace sim {                     // Simulation objects
//----------------------------------------------------------------------------
//
// Struct-
//       sim::typedefs
//
// Purpose-
//       Define simulator types
//
//----------------------------------------------------------------------------
typedef double         Mass;        // Mass in kilograms

//----------------------------------------------------------------------------
//
// Struct-
//       sim::Xyz
//       sim::Pos
//       sim::Vel
//
// Purpose-
//       X, Y, Z containers
//
//----------------------------------------------------------------------------
struct Xyz {                        // XYZ container
double                 x= 0.0;      // X value
double                 y= 0.0;      // Y value
double                 z= 0.0;      // Z value
}; // struct Xyz

struct Pos {                        // Position vector, kilometers
double                 x= 0.0;      // X value
double                 y= 0.0;      // Y value
double                 z= 0.0;      // Z value
}; // struct Pos

struct Vel {                        // Velocity vector, kilometers/second
double                 x= 0.0;      // X value
double                 y= 0.0;      // Y value
double                 z= 0.0;      // Z value
}; // struct Vel

//----------------------------------------------------------------------------
//
// Struct-
//       sim::Orbital
//
// Purpose-
//       Orbital descriptor
//
//----------------------------------------------------------------------------
struct Orbital : public pub::List<Orbital>::Link { // Orbital descriptor
std::string            name;        // The object name
Orbital*               barycenter;  // Associated center of mass
pub::List<Orbital>     orb_list;    // Associated Orbitals

Mass                   mass= 0;     // Mass (in kilograms)
Pos                    pos= {};     // Position (relative to barycenter)
Vel                    vel= {};     // Velocity (relative to barycenter)

   Orbital(std::string name, Orbital* root= nullptr)
:  name(name), barycenter(root) {}
}; // struct Orbital

//----------------------------------------------------------------------------
//
// Class-
//       sim::Window
//
// Purpose-
//       Display Window
//
//----------------------------------------------------------------------------
class Window : public gui::Window { // Display Window
//----------------------------------------------------------------------------
// sim::Window::Attributes
//----------------------------------------------------------------------------
public:
typedef gui::Window    super;       // Our super class

// XCB fields
xcb_gcontext_t         drawGC= 0;   // The default graphic context
xcb_image_t            image= {};   // XCB image manipulator

//----------------------------------------------------------------------------
// sim::Window::Constructor/Destructor/Operators
//----------------------------------------------------------------------------
public:
   Window(                          // Constructor
     Widget*           parent= nullptr, // The parent Widget
     const char*       name= nullptr, // The Widget name
     unsigned          width= 900,   // (X) size width (pixels)
     unsigned          height= 900); // (Y) size height (pixels)

virtual
   ~Window( void );                 // Destructor

   Window(const Window&) = delete;  // NO copy constructor
Window& operator=(const Window&) = delete; // NO assignment operator

//----------------------------------------------------------------------------
//
// Method-
//       sim::Window::configure
//
// Purpose-
//       Configure the Window
//
//----------------------------------------------------------------------------
virtual void
   configure( void );               // Configure the Window

//----------------------------------------------------------------------------
//
// Method-
//       sim::Window::draw
//
// Purpose-
//       Draw the Window
//
//----------------------------------------------------------------------------
virtual void
   draw( void );                    // Draw the Window

//----------------------------------------------------------------------------
//
// Method-
//       sim::Window::image_draw
//       sim::Window::image_init
//       sim::Window::image_term
//
// Purpose-
//       Draw the image
//       Create the image
//       Delete the image
//
//----------------------------------------------------------------------------
void
   image_draw( void );              // Update the image

void
   image_init( void );              // Create and initialze the image

void
   image_term( void );              // Clean up the image

//----------------------------------------------------------------------------
// sim::Window::Event handlers
//----------------------------------------------------------------------------
void
   configure_notify(                // Handle this
     xcb_configure_notify_event_t* E); // Configure notify event

void
   expose(                          // Handle this
     xcb_expose_event_t* event);    // Expose event

void
   key_input(                       // Handle this
     xcb_keysym_t      key,         // Key input event
     int               state);      // Alt/Ctl/Shift state mask
}; // class sim::Window
}  // namespace sim
#endif // GRAVITY_H_INCLUDED
