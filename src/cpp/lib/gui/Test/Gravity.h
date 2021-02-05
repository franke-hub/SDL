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
//       2021/02/04
//
//----------------------------------------------------------------------------
#ifndef GRAVITY_H_INCLUDED
#define GRAVITY_H_INCLUDED

#include <memory>                   // For std::shared_ptr, std::unique_ptr
#include <string>                   // For std::string
#include <math.h>                   // For sqrt(), ...
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

Xyz&
   operator+=(                      // Replacement addition
     const Xyz&        rhs)         // Addend
{
   x += rhs.x;
   y += rhs.y;
   z += rhs.z;
   return *this;
}

Xyz&
   operator-=(                      // Replacement subtraction
     const Xyz&        rhs)         // Subtrahend
{
   x -= rhs.x;
   y -= rhs.y;
   z -= rhs.z;
   return *this;
}

Xyz&
   operator+=(                      // Replacement addition
     const double      rhs)         // Addend
{
   x += rhs;
   y += rhs;
   z += rhs;
   return *this;
}

Xyz&
   operator-=(                      // Replacement subtraction
     const double      rhs)         // Addend
{
   x -= rhs;
   y -= rhs;
   z -= rhs;
   return *this;
}

Xyz&
   operator*=(                      // Replacement multiplication
     const double      rhs)         // Multiplior
{
   x *= rhs;
   y *= rhs;
   z *= rhs;
   return *this;
}

Xyz&
   operator/=(                      // Replacement division
     const double      rhs)         // Divisor
{
   x /= rhs;
   y /= rhs;
   z /= rhs;
   return *this;
}

double                              // The magnitude
   mag( void ) const                // Get magnitude
{  return sqrt(x*x + y*y + z*z); }

double                              // The magnitude
   mag(                             // Get magnitude
     const Xyz&        that) const  // of difference
{  Xyz tmp= {that.x-x, that.y-y, that.z-z}; return tmp.mag(); }

void
   max(                             // Maximum between this
     const Xyz&        that)        // and that
{
   if( that.x > x ) x= that.x;
   if( that.y > y ) y= that.y;
   if( that.z > z ) z= that.z;
}

void
   min(                             // Minimum between this
     const Xyz&        that)        // and that
{
   if( that.x < x ) x= that.x;
   if( that.y < y ) y= that.y;
   if( that.z < z ) z= that.z;
}

int                                 // Return code, 0 OK
   fr_string(                       // Get value from
     const char*       text);       // This input string
}; // struct Xyz

struct Pos : public Xyz {           // Position vector, kilometers
}; // struct Pos

struct Vel : public Xyz {           // Velocity vector, kilometers/second
}; // struct Vel

// Global operators ==========================================================
static inline Xyz                   // The difference rhs - lhs
   operator-(                       // Get difference
     const Xyz&        lhs,         // Left hand side
     const Xyz&        rhs)         // Right hand sit
{  return {rhs.x-lhs.x, rhs.y-lhs.y, rhs.z-lhs.z}; }

static inline Pos                   // The difference rhs - lhs
   operator-(                       // Get difference
     const Pos&        lhs,         // Left hand side
     const Pos&        rhs)         // Right hand sit
{  return {rhs.x-lhs.x, rhs.y-lhs.y, rhs.z-lhs.z}; }

static inline Vel                   // The difference rhs - lhs
   operator-(                       // Get difference
     const Vel&        lhs,         // Left hand side
     const Vel&        rhs)         // Right hand sit
{  return {rhs.x-lhs.x, rhs.y-lhs.y, rhs.z-lhs.z}; }

//----------------------------------------------------------------------------
//
// Struct-
//       sim::Orb
//
// Purpose-
//       Orbital descriptor
//
//----------------------------------------------------------------------------
struct Com;                         // Forward reference, Com derived from Orb

struct Orb : public pub::List<Orb>::Link { // Orbital descriptor
std::string            name;        // The object name
Com*                   com= nullptr; // Associated center of mass
uint32_t               color= 0;    // Display color

double                 circ= 0;     // Orbital circumference
Mass                   mass= 0;     // Mass (in kilograms)
Pos                    pos= {};     // Position (relative to barycenter)
Vel                    vel= {};     // Velocity (relative to barycenter)

   Orb(std::string name, Com* root= nullptr)
:  name(name), com(root) {}
}; // struct Orb

//----------------------------------------------------------------------------
//
// Struct-
//       sim::Com
//
// Purpose-
//       Center of mass
//
//----------------------------------------------------------------------------
struct Com : public Orb {           // Center of mass
pub::List<Orb>         orb_list;    // Associated Orbitals

   Com(std::string name, Com* root= nullptr)
:  Orb(name, root) {};

void
   init( void )                     // Initialize (mass)
{
   mass= 0.0;                       // Mass of all objects
   for(Orb* orb= orb_list.get_head(); orb; orb= orb->get_next()) {
     mass += orb->mass;
   }

   if( mass == 0.0 ) {
     fprintf(stderr, "%s massless\n", name.c_str());
     throw "Massless Center of Mass";
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       com
//
// Purpose-
//       Compute curent of mass (relative to current position)
//
//----------------------------------------------------------------------------
Pos                                 // Current (relative) Center of Mass
   com( void )                      // Compute (relative) Center of Mass
{
   Pos mxp= {};                     // Sigma mass * position
   if( mass == 0.0 )                // If Massless
     return mxp;                    // (It could be anywhere)

   for(Orb* orb= orb_list.get_head(); orb; orb= orb->get_next()) {
     Pos V= orb->pos - pos;         // Relative position
     V *= orb->mass;                // Relative mass * position
     mxp += V;
   }
   mxp /= mass;                     // Sigma mass * position / mass

   return mxp;
}
}; // struct Com

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
char                   key_debug[128]= {}; // Switchable debug flags

// XCB fields
int                    center_x;    // X Center of screen
int                    center_y;    // Y Center of screen
uint32_t               pdata= 0;    // The XCB pixel manipulator data
xcb_gcontext_t         drawGC= 0;   // The default graphic context
xcb_image_t            image= {};   // XCB image manipulator
xcb_image_t            pixel= {};   // XCB pixel manipulator

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
//
// Method-
//       sim::Window::put_xy
//
// Purpose-
//       Draw one pixel on screen
//
//----------------------------------------------------------------------------
void
   put_xy(                          // Draw pixel at location
     int               x,           // X (Width) index  (from left)
     int               y,           // Y (Height) index (from top)
     uint32_t          p);          // The Pixel to draw

void
   put_xy(                          // Draw pixel at location from screen
     int               x,           // X (Width) index  (from left)
     int               y);          // Y (Height) index (from top)

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
