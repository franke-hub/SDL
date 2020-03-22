//----------------------------------------------------------------------------
//
//       Copyright (c) 2018 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       NetVideo.h
//
// Purpose-
//       Define Network video input and output classes.
//
// Last change date-
//       2018/01/01
//
//----------------------------------------------------------------------------
#ifndef NETVIDEO_H_INCLUDED
#define NETVIDEO_H_INCLUDED

#include "Network.h"

namespace NETWORK_H_NAMESPACE {
//----------------------------------------------------------------------------
//
// Struct-
//       Pixel
//
// Purpose-
//       Pixel content descriptor.
//
//----------------------------------------------------------------------------
struct Pixel {                      // Pixel content
unsigned char          W;           // White (grey scale)
unsigned char          R;           // Red
unsigned char          G;           // Green
unsigned char          B;           // Blue
}; // struct Pixel

typedef uint16_t       Noise;       // Fanin (noise) values

//----------------------------------------------------------------------------
//
// Class-
//       VideoSource
//
// Purpose-
//       This is a pseudo-device we use to fetch our input data.
//
// Implementation notes-
//       The default source provides pseudo-random data, and we don't
//       particularly care that it's not mathmatically random.
//
//----------------------------------------------------------------------------
class VideoSource {                 // Our Video Source
public:
virtual
   ~VideoSource( void ) {}          // Destructor
   VideoSource( void ) {}           // Constructor

virtual int                         // Expected output classification
   fetch(                           // Load the video
     Pixel*            addr,        // Target address
     size_t            x,           // X dimension
     size_t            y)           // Y dimension
{
   for(size_t i= 0; i<x; i++) {
     for(size_t j= 0; j<y; j++) {
       addr->W= rand();
       addr->R= rand();
       addr->G= rand();
       addr->B= rand();
       addr++;
     }
   }

   return -1;                       // Unspecified output
}
}; // class VideoSource

//----------------------------------------------------------------------------
//
// Class-
//       VideoInp
//
// Purpose-
//       The video input class.
//
// Implementation notes-
//       Each Pixel contains 4 unsigned char values: W, R, G, B
//         (W= White grey scale, R= Red, G= Green, B= Blue)
//       Group[0]: The current  Pixel set: X * Y * 4 Tokens
//   (Perhaps later)
//       Group[1]: The prior    Pixel set: X * Y * 4 Tokens
//       Group[2]: The spacial  Pixel set: X * Y * 4 Tokens (No storage)
//       Group[3]: The temporal Pixel set: X * Y * 4 Tokens (No storage)
//       Group[4]: The faninout Noise set: X * Y * 1 Tokens (uint16_t)
//
//----------------------------------------------------------------------------
class VideoInp : public InpNetwork { // A video input Network
//----------------------------------------------------------------------------
// VideoInp::Attributes
//----------------------------------------------------------------------------
public:
Layer&                 _layer;      // Our lookup source
VideoSource&           source;      // Our VideoSource
Count                  x_size;      // X length
Count                  y_size;      // Y length
Count                  a_size;      // x * y

Pixel*                 current;     // The current Pixel set
Pixel*                 prior;       // The prior Pixel set
//                     spacial;     // The space-adjacent Pixel set (computed)
//                     temporal;    // The temporal-adjacent Pixel set (computed)
size_t                 s_size;      // a_size * sizeof(*setter)
Noise*                 getter;      // The current Noise fanout
Noise*                 setter;      // The current Noise fanin

//----------------------------------------------------------------------------
// VideoInp::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~VideoInp()                      // Virtual destructor
{  IFDEBUG( debugf("VideoInp(%p).~VideoInp\n", this); )

   free(current);
   free(prior);
   free(getter);
   free(setter);
}

   VideoInp(                        // Data constructor
     Count             x,           // X length, in Pixels
     Count             y,           // X length, in Pixels
     Layer&            layer,       // Our containing Layer
     VideoSource&      source)      // Our video source
:  InpNetwork(x*y), _layer(layer), source(source)
,  x_size(x), y_size(y), a_size(x*y)
{  IFDEBUG( debugf("VideoInp(%p).VideoInp(%ld,%ld)\n", this, x, y); )

   s_size= a_size * sizeof(*setter);
   current= (Pixel*)malloc(a_size * sizeof(Pixel));
   prior=   (Pixel*)malloc(a_size * sizeof(Pixel));
   getter=  (Noise*)malloc(s_size);
   setter=  (Noise*)malloc(s_size);
}

   VideoInp( void ) = delete;       // NO Default Constructor
   VideoInp(const VideoInp&) = delete; // Disallowed copy constructor
   VideoInp& operator=(const VideoInp&) = delete; // Disallowed assignment operator

//----------------------------------------------------------------------------
// VideoInp::Methods
//----------------------------------------------------------------------------
public:
virtual RC                          // Fanin count
   fanin(                           // Process fanin
     Token             token,       // Token address
     Pulse             pulse)       // Weighted fanout value
{  return 1; }                      // Base class implementation

virtual RC                          // Fanin count
   fanout(                          // Process fanout
     Token             token,       // First token
     Count             count)       // Token count
{  IFDEBUG( if( verbose() > 0 ) fanout_begin(token, count); )

   // BRINGUP: Just fanout pixel.token randomly to next layer
   Network* neuron= _layer.locate(_ending);
   Count out_count= neuron->length();

   RC rc= 0;
   for(Count i= 0; i<count; i++) {
     if( (token-_origin) + i >= a_size )
       break;

     Token target= rand() & 0x7fffffff;
     target= target % out_count;
     target += _ending;
     Pulse pulse= current[i].W;
     rc += neuron->fanin(target, pulse);
   }

   IFDEBUG( if( verbose() > 0 ) fanout_end(rc, count); )
   return rc;
}

virtual Value_t                     // The current Value_t
   to_value(                        // Get current Value_t
     Token             token) const // For this Token
{  IFDEBUG( if( verbose() > 3 ) {
     Buffer buffer;
     debugf("%s.to_value(%ld)\n", to_buffer(buffer), token);
   } ) // IFDEBUG( if ...
   assert( contains(token) );

   Count index= token - _origin;
   return current[index].W;
}

virtual void
   update( void )                   // Clock update
{  IFDEBUG(
     Buffer buffer;
     debugf("%s.update\n", to_buffer(buffer));
   )

   // Swap the current and prior arrays
   Pixel* new_current= this->prior;
   Pixel* new_prior= this->current;
   this->current= new_current;
   this->prior=   new_prior;
   source.fetch(new_current, x_size, y_size);

   // Update the getter/setter arrays
   Noise* new_getter= this->setter;
   Noise* new_setter= this->getter;
   this->getter= new_getter;
   this->setter= new_setter;
   memset((void*)new_setter, 0, a_size);
}
}; // class VideoInp

//----------------------------------------------------------------------------
//
// Class-
//       VideoOut
//
// Purpose-
//       The Network video output class.
//
// Implementation notes-
//       BRINGUP: to_value available only during update and before update
//       called for this Network.
//
//----------------------------------------------------------------------------
class VideoOut : public OutNetwork { // A video output Network
//----------------------------------------------------------------------------
// VideoOut::Attributes
//----------------------------------------------------------------------------
public:
Count                  x_size;      // X length
Count                  y_size;      // Y length

Count                  s_size;      // x_size * y_size * sizeof(*setter)
Value_t*               setter;      // The output array

//----------------------------------------------------------------------------
// VideoOut::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~VideoOut()                      // Virtual destructor
{  IFDEBUG( debugf("VideoOut(%p).~VideoOut\n", this); )

   free(setter);
}

   VideoOut(                        // Data constructor
     Count             x,           // X length, in Pixels
     Count             y)           // X length, in Pixels
:  OutNetwork(x*y), x_size(x), y_size(y)
{  IFDEBUG( debugf("VideoOut(%p).VideoOut(%ld,%ld)\n", this, x, y); )

   s_size= x * y * sizeof(*setter);
   setter= (Value_t*)malloc(s_size);
}

   VideoOut( void ) = delete;       // NO Default Constructor
   VideoOut(const VideoOut&) = delete; // Disallowed copy constructor
   VideoOut& operator=(const VideoOut&) = delete; // Disallowed assignment operator

virtual void
   debug( void ) const              // Debug the VideoOut instance
{  Network::debug();

   IFDEBUG( if( verbose() > 0 ) {
     int count= 0;
     for(Count i= 0; i<_length; i++) {
       if( count == 0 )
         debugf(">>>> [%.6lx]", i);

       debugf(" %6d", setter[i]);
       count++;
       if( count == 8 ) {
         debugf("\n");
         count= 0;
       }
     }

     if( count != 0 )
         debugf("\n");

   } ) // IFDEBUG( if( ...
}

//----------------------------------------------------------------------------
// VideoOut::Methods
//----------------------------------------------------------------------------
public:
virtual RC                          // Fanin count
   fanin(                           // Process fanin
     Token             token,       // Token address
     Pulse             pulse)       // Weighted fanout value
{  // TODO: Check for overflow (Maybe use compare_and_exchange directly)
   Token index= token - _origin;
   std::atomic<Value_t>* _setter= (std::atomic<Value_t>*)setter;
   _setter[index] += pulse;

   return 1;
}

virtual const Value_t*              // The current Value_t array
   to_value( void ) const           // Get current Value_t array
{  IFDEBUG( if( verbose() > 0 ) {
     Buffer buffer;
     debugf("%s.to_value\n", to_buffer(buffer));
   } ) // IFDEBUG( if ...

   return setter;
}

virtual void
   update( void )                   // Clock update
{  IFDEBUG(
     Buffer buffer;
     debugf("%s.update\n", to_buffer(buffer));
   )

   memset((void*)setter, 0, s_size);
}
}; // class VideoOut
}  // namespace NETWORK_H_NAMESPACE

#include "NetVideo.i"

#endif // NETVIDEO_H_INCLUDED
