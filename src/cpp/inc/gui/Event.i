//----------------------------------------------------------------------------
//
//       Copyright (c) 2010 Frank Eskesen.
//
//       This file is free content, distributed under the MIT license.
//       (See accompanying file LICENSE.MIT or the original contained
//       within https://opensource.org/licenses/MIT)
//
//----------------------------------------------------------------------------
//
// Title-
//       Event.i
//
// Purpose-
//       Graphical User Interface: Event: Inline methods
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
#ifndef GUI_EVENT_I_INCLUDED
#define GUI_EVENT_I_INCLUDED

//----------------------------------------------------------------------------
//
// Method-
//       Event::~Event
//
// Purpose-
//       Destructor
//
//----------------------------------------------------------------------------
   Event::~Event( void )            // Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       Event::Event
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
   Event::Event( void )             // Constructor
{
}

   Event::Event(                    // Constructor
     EC                code,        // The event code
     int               data,        // The event data
     const XYOffset&   offset,      // Set Offset
     const XYLength&   length)      // Set Length
:  code(code), data(data), offset(offset), length(length)
{
}

//----------------------------------------------------------------------------
//
// Method-
//       Event::getCode
//       Event::getData
//       Event::getLength
//       Event::getOffset
//
//       Event::setCode
//       Event::setData
//       Event::setEvent
//       Event::setLength
//       Event::setOffset
//
// Purpose-
//       Accessor methods.
//
//----------------------------------------------------------------------------
Event::EC                           // The event code
   Event::getCode( void ) const     // Get event code
{
   return code;
}

int                                 // The event data
   Event::getData( void ) const     // Get event data
{
   return data;
}

const XYLength&                     // XYLength
   Event::getLength( void ) const   // Get Length of Event
{
   return length;
}

const XYOffset&                     // XYOffset
   Event::getOffset( void ) const   // Get Offset of Event
{
   return offset;
}

void
   Event::setCode(                  // Set the Event code
     EC                code)        // The event code
{
   this->code= code;
}

void
   Event::setData(                  // Set the Event data
     int               data)        // The event data
{
   this->data= data;
}

void
   Event::setEvent(                 // Set the Event
     EC                code,        // The event code
     int               data,        // The event data
     const XYOffset&   offset,      // Set Offset
     const XYLength&   length)      // Set Length
{
   this->code= code;
   this->data= data;
   this->offset= offset;
   this->length= length;
}

void
   Event::setLength(                // Set Length of Event
     const XYLength&   length)      // Set Length
{
   this->length= length;
}

void
   Event::setOffset(                // Set Offset of Event
     const XYOffset&   offset)      // Set Offset
{
   this->offset= offset;
}

#endif // GUI_EVENTI_INCLUDED
