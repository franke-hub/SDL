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
//       Xcb/Types.h
//
// Purpose-
//       XCB Type descriptors
//
// Last change date-
//       2020/09/06
//
// Implementation notes-
//       Use: xcb_point_t     <x,y> for screen point
//       Use: xcb_rectangle_t <x,y,width,height> for window placement/size
//         int16_t x,y; uint16_t width,height;
//
//----------------------------------------------------------------------------
#ifndef XCB_TYPES_H_INCLUDED
#define XCB_TYPES_H_INCLUDED

#include <sys/types.h>              // For system types
#include <xcb/xproto.h>             // For xcb types
#include <pub/List.h>               // For List

namespace xcb {
//----------------------------------------------------------------------------
// Typedefs
//----------------------------------------------------------------------------
typedef uint16_t       CR_t;        // XCB Column/Row size dimension
typedef int16_t        PT_t;        // XCB xcb_point_t X/Y dimension
typedef uint16_t       WH_t;        // XCB Width/Height size dimension
typedef uint16_t       XY_t;        // XCB X/Y size dimension
typedef uint32_t       Pixel_t;     // XCB Pixel type

typedef struct {
uint16_t               col;         // Column (X)
uint16_t               row;         // Row    (Y)
} CR_size_t;                        // Column/Row number

typedef struct {
uint16_t               width;       // Width  (X)
uint16_t               height;      // Height (Y)
} WH_size_t;                        // Width/Height size (Pixels)

typedef struct {
uint16_t               x;           // X (absolute) offset
uint16_t               y;           // Y (absolute) offset
} XY_size_t;                        // X/Y size (Pixels)

typedef struct {
XY_size_t              xy;          // X/Y (absolute) offsets
WH_size_t              wh;          // W/H (absolute) lengths
} XYWH_rect_t;                      // XY/WH rectangle (Pixels)

//----------------------------------------------------------------------------
// Enumerations
//----------------------------------------------------------------------------
enum // Constants for parameterization
{  DEV_EVENT_MASK= XCB_EVENT_MASK_NO_EVENT // Minimum Window event mask
                 | XCB_EVENT_MASK_EXPOSURE
                 | XCB_EVENT_MASK_PROPERTY_CHANGE
}; // Generic enum

// Use these values when examining xcb_button_press/release_event_t
// Users can inadvertently cause  WT_PUSH/PULL when attempting WT_LEFT/RIGHT.
enum BUTTON_TYPE                    // VALUE for xcb_button_press_event.detail
{  BT_LEFT=  1                      // LEFT button
,  BT_CNTR=  2                      // CENTER button (or WHEEL PRESS)
,  WT_PRESS= 2                      // WHEEL PRESS (or CENTER button)
,  BT_RIGHT= 3                      // RIGHT button
,  WT_PUSH=  4                      // WHEEL PUSH  (spin top away from user)
,  WT_PULL=  5                      // WHEEL PULL  (spin top toward user)
,  WT_LEFT=  8                      // WHEEL LEFT  (push wheel to the left)
,  WT_RIGHT= 9                      // WHEEL RIGHT (push wheel to the right)
}; // enum BUTTON_TYPE

enum KEY_STATE                      // MASK for xcb_key_press_event.state
{  KS_SHIFT= XCB_KEY_BUT_MASK_SHIFT    // 0x0001 (Values subject to change)
,  KS_LOCK=  XCB_KEY_BUT_MASK_LOCK     // 0x0002
,  KS_CTRL=  XCB_KEY_BUT_MASK_CONTROL  // 0x0004
,  KS_MOD1=  XCB_KEY_BUT_MASK_MOD_1    // 0x0008 (ALT)
,  KS_MOD2=  XCB_KEY_BUT_MASK_MOD_2    // 0x0010 (NUM LOCK)
,  KS_MOD3=  XCB_KEY_BUT_MASK_MOD_3    // 0x0020
,  KS_MOD4=  XCB_KEY_BUT_MASK_MOD_4    // 0x0040
,  KS_MOD5=  XCB_KEY_BUT_MASK_MOD_5    // 0x0080

,  BS_BUTT1= XCB_KEY_BUT_MASK_BUTTON_1 // 0x0100 (LEFT)
,  BS_BUTT2= XCB_KEY_BUT_MASK_BUTTON_2 // 0x0200 (CENTER)
,  BS_BUTT3= XCB_KEY_BUT_MASK_BUTTON_3 // 0x0400 (RIGHT)
,  BS_BUTT4= XCB_KEY_BUT_MASK_BUTTON_4 // 0x0800 (PUSH)
,  BS_BUTT5= XCB_KEY_BUT_MASK_BUTTON_5 // 0x1000 (PULL)

,  KS_ALT=   KS_MOD1                   // 0x0008 (ALT key alias)
,  KS_NUML=  KS_MOD2                   // 0x0010 (NUM LOCK key alias)
}; // enum KEY_STATE

//----------------------------------------------------------------------------
//
// Class-
//       xcb::Line
//
// Purpose-
//       Define the Line interface
//
// Implementation note-
//       The text field is neither allocated nor deleted by this class.
//
//----------------------------------------------------------------------------
class Line : public pub::List<Line>::Link { // XCB Line interface
//----------------------------------------------------------------------------
// Line::Attributes
public:
const char*            text;        // Text, never nullptr

//----------------------------------------------------------------------------
// Line::Constructor/Destructor
//----------------------------------------------------------------------------
public:
   Line(                            // Constructor
     const char*       text= nullptr) // Line text
:  ::pub::List<Line>::Link(), text(text ? text : "") {}

//----------------------------------------------------------------------------
virtual
   ~Line( void ) {}                 // Destructor
}; // class Line
}  // namespace xcb
#endif // XCB_TYPES_H_INCLUDED
