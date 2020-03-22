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
//       NetMiddle.h
//
// Purpose-
//       Define NN::MiddleLayer and associated storage objects
//
// Last change date-
//       2018/01/01
//
// Storage per FanoutNeuron token (before page rounding)-
//        2 getter
//        2 setter
//        2 weight
//       48 FanoutBundle[8] (32 bit Bundle_index_t Token space)
//       -- Total
//       54 bytes
//
// Storage per FanoutBundle token:
//       FanoutBundle::DIM * (sizeof(Bundle_index_t) + sizeof(Bundle_weight_t))
//       32 FanoutBundle[8] (16 bit Bundle_index_t Token space)
//       48 FanoutBundle[8] (32 bit Bundle_index_t Token space)
//       64 FanoutBundle[8] (48 bit Bundle_index_t Token space)
//          (64-bit combined weight/token; mask and shift operations required)
//       70 FanoutBundle[8] (64 bit Bundle_index_t Token space)
//
// Implementation notes-
//       See NetMiddle.i for current implementation notes.
//
//----------------------------------------------------------------------------
#ifndef NETMIDDLE_H_INCLUDED
#define NETMIDDLE_H_INCLUDED

#include "Network.h"

namespace NETWORK_H_NAMESPACE {
//----------------------------------------------------------------------------
//
// Class-
//       MiddleLayer
//
// Purpose-
//       Network storage container Layer.
//
// Implementation notes-
//       The MiddleLayer contains storage for contained Network tokens.
//       FanoutNeuron: 1 cycle delay, fanout to next contained Layer
//       FaninpNeuron: 0 cycle delay, fanin from previous Layer
//
//----------------------------------------------------------------------------
class MiddleLayer : public Layer { // Network storage container Layer
public:
std::string            name = "MiddleLayer"; // The MiddleLayer's name

// MiddleLayer data, built in MiddleLayer::build()
void*                  middle_data= nullptr; // Middle data, one chunk
Value_t*               getter= nullptr;  // The getter array
Value_t*               setter= nullptr;  // The setter array
Weight_t*              weight= nullptr;  // The weight trigger array
FanoutBundle*          bundle= nullptr;  // The fanout bundle array

size_t                 bundle_size= 0;   // Total bundle storage size
size_t                 getset_size= 0;   // Total getter or setter storage size
size_t                 weight_size= 0;   // Total weight storage size
size_t                 middle_size= 0;   // MiddleLayer storage size

// Indexing controls, updated during construction and build
Count                  fanout_length= 0; // The FanoutNeuron length
Count                  bundle_length= 0; // Extra bundle length
Count                  getset_length= 0; // Extra getset length
Count                  weight_length= 0; // Extra weight length

//----------------------------------------------------------------------------
// MiddleLayer::Destructor, Constructors
//----------------------------------------------------------------------------
   ~MiddleLayer( void )
{  IFDEBUG( debugf("MiddleLayer(%p).~MiddleLayer\n", this); )

   free(middle_data);
}

   MiddleLayer(
     Layer&            owner)       // The containing Layer
:  Layer(owner)
{  IFDEBUG( debugf("MiddleLayer(%p).MiddleLayer\n", this); )

   #if false // These are currently unused
     // This verifies that NN::atomic_flag and NN::v_char_flag are the same
     // length, so that their pointers can be interchanged.
     if( sizeof(atomic_flag) != sizeof(v_char_flag) ) {
       debugf("Implementation failure: sizeof(v_char_flag)\n");
       std::exit(1);
     }
   #endif

   #if false // This is guaranteed by the specification. No need to check.
     // This verifies that NN::atomic_flag is lock free.
     if( !std::atomic<atomic_flag>{}.is_lock_free() ) {
       debugf("Implementation failure: !atomic_flag.lock_free\n");
       std::exit(1);
     }
   #endif
}

//----------------------------------------------------------------------------
// MiddleLayer::Build methods
//----------------------------------------------------------------------------
virtual Count                       // The resultant length
   build(                           // Complete the build
     Token             origin);     // Assigned global origin
// Implementation in "NetMiddle.i"

virtual void
   build_debug( void ) const;       // Build debugging display
// Implementation in "NetMiddle.i"

inline void
   layer_debug(                     // Build layer debugging
     Buffer&           buffer) const // Using this buffer
{  Layer::layer_debug(buffer);

   debugf(">> name(%s)\n", name.c_str());
   if( fanout_length == 0 )
     debugf(">> fanout_layer: NOT PRESENT\n");
   else {
     debugf(">> middle_data(%p).%lx)\n", middle_data, (long)middle_size);
     debugf(">> getter(%p), setter(%p)\n", getter, setter);
     debugf(">> weight(%p), bundle(%p)\n", weight, bundle);
     debugf(">> [%.10lx::%.10lx].%.6lx fanout\n",
            0L, fanout_length, fanout_length);
     debugf(">> [%.10lx::%.10lx].%.6lx bundle\n",
            fanout_length, fanout_length+bundle_length, bundle_length);
     debugf(">> [%.10lx::%.10lx].%.6lx getset\n",
            fanout_length, fanout_length+getset_length, getset_length);
     debugf(">> [%.10lx::%.10lx].%.6lx weight\n",
            fanout_length, fanout_length+weight_length, weight_length);

     size_t per_fanout= sizeof(*getter) + sizeof(*setter)
                      + sizeof(*weight) + sizeof(*bundle);
     size_t per_faninp= sizeof(*bundle) + sizeof(*weight);
     debugf(">> per_fanout(%ld) per_faninp(%ld)\n",
            (long)per_fanout, (long)per_faninp);
   }
}

//----------------------------------------------------------------------------
// MiddleLayer::Methods
//----------------------------------------------------------------------------
virtual void                        // Called with threading inactive
   update( void )                   // Clock update
{  // Update the getter/setter data
   Value_t* new_getter= this->setter; // Reverse the getter and setter arrays
   Value_t* new_setter= this->getter;
   this->getter= new_getter;
   this->setter= new_setter;
   memset((void*)new_setter, 0, getset_size);

   Layer::update();                 // Update the contained layers last
}
}; // struct MiddleLayer

//----------------------------------------------------------------------------
//
// Class-
//       OutBuffer
//
// Purpose-
//       OutBuffer Network instance
//
// Usage notes-
//       The OutBuffer supports fanin() but does nothing on fanout().
//
//----------------------------------------------------------------------------
class OutBuffer : public OutNetwork { // OutBuffer Network descriptor
//----------------------------------------------------------------------------
// OutBuffer::Attributes
//----------------------------------------------------------------------------
public:
MiddleLayer&           _layer;      // Our MiddleLayer
Count                  _index;      // Our OutBuffer index

Value_t*               _getter= nullptr; // Getter array
std::atomic<Value_t>*  _setter= nullptr; // Setter array

//----------------------------------------------------------------------------
// OutBuffer::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~OutBuffer()                     // Virtual destructor
{  IFDEBUG( debugf("OutBuffer(%p).~OutBuffer\n", this); ) }

   OutBuffer(                       // Data constructor
     Count             _length,     // The number of tokens
     MiddleLayer&      _layer)      // Our Container
:  OutNetwork(_length)
,  _layer(_layer), _index(_layer.bundle_length)
{  IFDEBUG( debugf("OutBuffer(%p).OutBuffer\n", this); )

   _layer.getset_length += _length;
}

   OutBuffer( void ) = delete;      // NO Default Constructor
   OutBuffer(const OutBuffer&) = delete; // Disallowed copy constructor
   OutBuffer& operator=(const OutBuffer&) = delete; // Disallowed assignment operator

//----------------------------------------------------------------------------
// OutBuffer::Build methods
//----------------------------------------------------------------------------
virtual Count                       // The build length
   build(                           // Complete the build
     Token             origin)      // Assigned origin
{  Network::build(origin);

   _index += _layer.fanout_length;
   return _length;
}

virtual void
   build_debug( void ) const        // Build debugging display
{  debugf("   getset_index = [%.10lx], _getter(%10lx), _setter(%10lx)\n",
              _index, (long)_getter, (long)_setter);
}

virtual bool                        // TRUE if another pass needed
   build_update(                    // Update the build
     int               pass)        // Build pass number
{  update();                        // Initialize _getter and _setter
   return false;
}

//----------------------------------------------------------------------------
// OutBuffer::Accessor methods
//----------------------------------------------------------------------------
inline Count
   get_index(
     Token             token) const
{  IFCHECK( assert( contains(token) ); )
   return token - _origin;
}

//----------------------------------------------------------------------------
// OutBuffer::Methods
//----------------------------------------------------------------------------
virtual void
   debug( void ) const;             // Debugging display
// Implementation in "NetMiddle.i"

virtual RC                          // Fanin count
   fanin(                           // Process fanin
     Token             token,       // Storage locator
     Pulse             pulse)       // Weighted fanout value
{  Token index= get_index(token);
   _setter[index] += pulse;
   return 1;
}

virtual const Value_t*              // The current output Value_t array
   to_value( void ) const           // Get current output array
{  return _getter; }

virtual void
   update( void )                   // Update getter/setter
{  _getter= _layer.getter + _index;
   _setter= (std::atomic<Value_t>*)(_layer.setter + _index);
}
}; // class OutBuffer

//----------------------------------------------------------------------------
//
// Class-
//       FaninpNeuron
//
// Purpose-
//       FaninpNeuron Network instance
//
//----------------------------------------------------------------------------
class FaninpNeuron : public Network { // FaninpNeuron Network descriptor
//----------------------------------------------------------------------------
// FaninpNeuron::Attributes
//----------------------------------------------------------------------------
public:
MiddleLayer&           _layer;      // Our MiddleLayer
InpNetwork*            _prevN;      // Our input Network
Network*               _nextN;      // Our output Network
Count                  _bundle_index; // Our bundle index
Count                  _weight_index; // Our weight index

//----------------------------------------------------------------------------
// FaninpNeuron::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~FaninpNeuron()                  // Virtual destructor
{  IFDEBUG( debugf("FaninpNeuron(%p).~FaninpNeuron\n", this); ) }

   FaninpNeuron(                    // Data constructor
     Count             _length,     // The number of tokens
     MiddleLayer&      _layer)      // Our Container
:  Network(_length),  _layer(_layer)
,  _bundle_index(_layer.bundle_length)
,  _weight_index(_layer.weight_length)
{  IFDEBUG( debugf("FaninpNeuron(%p).FaninpNeuron\n", this); )
   assert( _length >= FanoutBundle::DIM ); // (TODO: TEMPORARY RESTRICTION?)

   _layer.bundle_length += _length;
   _layer.weight_length += _length;
}

   FaninpNeuron( void ) = delete;   // NO Default Constructor
   FaninpNeuron(const FaninpNeuron&) = delete; // Disallowed copy constructor
   FaninpNeuron& operator=(const FaninpNeuron&) = delete; // Disallowed assignment operator

//----------------------------------------------------------------------------
// FaninpNeuron::Build methods
//----------------------------------------------------------------------------
virtual Count                       // The build length
   build(                           // Complete the build
     Token             origin);     // Assigned origin
// Implementation in "NetMiddle.i"

virtual void
   build_debug( void ) const;       // Build debugging display
// Implementation in "NetMiddle.i"

virtual bool                        // TRUE if another pass needed
   build_update(                    // Update the build
     int               pass);       // Build pass number
// Implementation in "NetMiddle.i"

//----------------------------------------------------------------------------
// FaninpNeuron::Accessor methods
//----------------------------------------------------------------------------
inline Count
   get_bundle_index(
     Token             token) const
{  IFCHECK( assert( contains(token) ); )
   return token - _origin + _bundle_index;
}

inline Count
   get_weight_index(
     Token             token) const
{  IFCHECK( assert( contains(token) ); )
   return token - _origin + _weight_index;
}

//----------------------------------------------------------------------------
// FaninpNeuron::Methods
//----------------------------------------------------------------------------
virtual void
   debug( void ) const;             // Debugging display
// Implementation in "NetMiddle.i"

virtual RC                          // Fanin count
   fanin(                           // Process fanin
     Token             token,       // Storage locator
     Pulse             pulse)       // Weighted fanout value
{  return 0; }                      // Not counted!

virtual RC                          // Fanin count
   fanout(                          // Process fanout
     Token             token,       // First Token to process
     Count             count);      // Number of Tokens to process
// Implementation in "NetMiddle.i"

inline Pulse                        // Fanin value
   faninp_bundle(                   // Drive faninp bundle
     FanoutBundle&     bundle) const // The bundle
{  Token const origin= _prevN->origin();

   Pulse result= 0;
   for(int i= 0; i<FanoutBundle::DIM; i++) {
     Token index= origin + bundle.index[i];
     result += to_pulse(_prevN->to_value(index), bundle.weight[i]);
   }

   return result;
}
}; // class FaninpNeuron

//----------------------------------------------------------------------------
//
// Class-
//       FanoutNeuron
//
// Purpose-
//       FanoutNeuron Network instance
//
//----------------------------------------------------------------------------
class FanoutNeuron : public OutNetwork { // FanoutNeuron Network descriptor
//----------------------------------------------------------------------------
// FanoutNeuron::Attributes
//----------------------------------------------------------------------------
public:
MiddleLayer&           _layer;      // Our MiddleLayer
const Count            _index;      // Our FanoutNeuron index
Network*               _nextN= nullptr; // The next Layer Network

//----------------------------------------------------------------------------
// FanoutNeuron::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~FanoutNeuron()                  // Virtual destructor
{  IFDEBUG( debugf("FanoutNeuron(%p).~FanoutNeuron\n", this); ) }

   FanoutNeuron(                    // Data constructor
     Count             _length,     // The number of tokens
     MiddleLayer&      _layer)      // Our Container Layer
:  OutNetwork(_length)
,  _layer(_layer), _index(_layer.fanout_length)
{  IFDEBUG( debugf("FanoutNeuron(%p).FanoutNeuron\n", this); )
   assert( _length >= FanoutBundle::DIM ); // (TODO: TEMPORARY RESTRICTION?)

   _layer.fanout_length += _length;
}

   FanoutNeuron( void ) = delete;   // NO Default Constructor
   FanoutNeuron(const FanoutNeuron&) = delete; // Disallowed copy constructor
   FanoutNeuron& operator=(const FanoutNeuron&) = delete; // Disallowed assignment operator

//----------------------------------------------------------------------------
// FanoutNeuron::Build methods
//----------------------------------------------------------------------------
virtual Count                       // The resultant length
   build(                           // Complete the build
     Token             origin);     // Assigned origin
// Implementation in "NetMiddle.i"

virtual void
   build_debug( void ) const;       // Build debugging display
// Implementation in "NetMiddle.i"

virtual bool                        // TRUE if another pass needed
   build_update(                    // Update the build
     int               pass);       // Build pass number
// Implementation in "NetMiddle.i"

//----------------------------------------------------------------------------
// FanoutNeuron::Accessor methods
//----------------------------------------------------------------------------
inline Count
   get_index(
     Token             token) const
{  IFCHECK( assert( contains(token) ); )
   return token - _origin + _index;
}

//----------------------------------------------------------------------------
// FanoutNeuron::Methods
//----------------------------------------------------------------------------
virtual void
   debug( void ) const;             // Debugging display
// Implementation in "NetMiddle.i"

virtual RC                          // Fanin count
   fanin(                           // Process fanin
     Token             token,       // Storage locator
     Pulse             pulse);      // Weighted fanout value
// Implementation in "NetMiddle.i"

virtual RC                          // Fanin count
   fanout(                          // Process fanout
     Token             token,       // First Token to process
     Count             count);      // Number of Tokens to process
// Implementation in "NetMiddle.i"

inline RC                           // Fanin count
   fanout_bundle(                   // Drive fanout bundle
     FanoutBundle&     bundle,      // The bundle
     Value_t           trigger) const // The trigger value
{  RC rc= 0;

   for(int i= 0; i<FanoutBundle::DIM; i++) {
     Token index= bundle.index[i];
     Pulse pulse= to_pulse(trigger, bundle.weight[i]);
     if( pulse != 0 ) {
       Token token= _ending + index;
       rc += _nextN->fanin(token, pulse);
     }
   }

   return rc;
}

virtual const Value_t*              // The current output Value_t array
   to_value( void ) const           // Get current output array
{  return _layer.getter + _index; }
}; // class FanoutNeuron
}  // namespace NETWORK_H_NAMESPACE

#include "NetMiddle.i"

#endif // NETMIDDLE_H_INCLUDED
