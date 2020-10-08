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
//       Network.h
//
// Purpose-
//       Define classes that can be used to build a Neural Network.
//
// Last change date-
//       2018/01/01
//
// Implementation notes-
//       While this implementation is a working example of a neural network,
//       it is experimental, somewhat complex, and does not use validated
//       training mechanisms. The code is the only documentation.
//
// Implementation overview-
//       All Network objects and methods are contained within a namespace.
//         The default NN namespace can be overridden; See "Network_types.h"
//       Class Network is the base class for most Network classes.
//         It is a direct subclass of Neuron.
//       Class Layer defines a Network object container.
//       Class MiddenLayer, a subclass of Layer, additionally adds the data
//         used by its contained Networks.
//       Class MiddleNeuron is a flyweight Network object. It can control
//         can control any number of Tokens. The data used by each Token
//         is dependent upon the size of the FanoutBundle. See NetMiddle.h
//
// Implementation files-
//       Nettypes.h: Contains macros, types, built-in functions, and all
//         external include files needed. Network.h always includes this
//         prerequisite header.
//
//       Network.h contains the basic Network class definitions and most of
//         their method implementations. Network.i, automatically included,
//         contains the remaining Network implementations.
//
//       NetMiddle.h: Contains MiddleLayer and MiddleNeuron objects, mostly
//         implemented in NetMiddle.h with NetMiddle.i containing the remainder.
//
//       NetVideo.h: Contains NetVideo input and output classes and most of
//         their method implementations. NetVideo.i, automatically included,
//         contains the remaining NetVideo implementations.
//
//----------------------------------------------------------------------------
#ifndef NETWORK_H_INCLUDED
#define NETWORK_H_INCLUDED

#include "NetTypes.h"

namespace NETWORK_H_NAMESPACE {
//----------------------------------------------------------------------------
//
// Class-
//       Network
//
// Purpose-
//       The base Neuron for all Network Neurons.
//
//----------------------------------------------------------------------------
class Network : public Neuron {     // A Network Neuron
//----------------------------------------------------------------------------
// Network::Attributes
//----------------------------------------------------------------------------
protected:
Token                  _origin;     // Our origin Token (after build)
Count                  _length;     // Our Token length
Token                  _ending;     // _offset + _length (after build)
Count                  _charge;     // Our work length

static int             _verbose;    // Debugging verbosity (in Network.cpp)

//----------------------------------------------------------------------------
// Network::Typedefs
//----------------------------------------------------------------------------
public:
enum { BUFF_SIZE= 256 };            // to_buffer size
typedef char           Buffer[BUFF_SIZE]; // A to_buffer buffer

//----------------------------------------------------------------------------
// Network::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Network()                       // Virtual destructor
{  IFDEBUG( debugf("Network(%p).~Network\n", this); ) }

   Network(                         // Data constructor
     Count             length)      // Our length, in Tokens
:  Neuron()                         // (Not really needed)
,  _origin(0), _length(length), _ending(0), _charge(_length)
{  IFDEBUG( debugf("Network(%p).Network(0x%lx)\n", this, length); )

   if( _charge == 0 ) _charge= 1;   // Minimum work length
}

   Network( void ) = delete;        // NO Default Constructor
   Network(const Network&) = delete; // Disallowed copy constructor
   Network& operator=(const Network&) = delete; // Disallowed assignment operator

//----------------------------------------------------------------------------
// Network::Build methods (Called after all constructors complete)
//----------------------------------------------------------------------------
virtual Count                       // The resultant length
   build(                           // Complete the build
     Token             origin)      // Assigned global origin
{  _origin= origin;
   _ending= origin + _length;

   IFDEBUG(
     Buffer            buffer;
     debugf("%s.build(%lx)\n", to_buffer(buffer), origin);
   )

   return _length;
}

virtual void                        // (Display static data)
   build_debug( void ) const        // Display the build data
{  }

virtual Count                       // The content count, in Tokens
   build_length( void ) const       // Get content count
{  return _length; }

virtual bool                        // TRUE if build_update should be recalled
   build_update(                    // Update build
     int               pass)        // Build update counter
{  (void)pass; return false; }      // (Parameter unused)

//----------------------------------------------------------------------------
// Network::Layer methods (These do not apply to leaf Network objects)
//----------------------------------------------------------------------------
public:
virtual Network*                    // The associated Network*
   build_locate(                    // Locate the Network
     Token             token) const // For this token
{  (void)token; not_implemented("build_locate"); // Should never be called
// return this;                     // Probably won't get here
}

virtual Network*                    // The associated Network*
   locate(                          // Locate the Network
     Token             token) const // For this token
{  (void)token; not_implemented("locate"); // Should never be called
// return this;                     // Probably won't get here
}

[[noreturn]] inline void
   not_implemented(                 // Function not implemented
     const char*       what) const  // Function name
{  Buffer              buffer;      // Working buffer
   debugf("%s\n", to_buffer(buffer));
   debugf("%s.%s not implemented", get_class_name().c_str(), what);

   throw NotImplemented(what);
}

//----------------------------------------------------------------------------
// Network::Accessor methods
//----------------------------------------------------------------------------
public:
inline bool                         // True iff Token is within range
   contains(                        // Is
     Token             token) const // This Token within our range?
{  return token >= _origin && token < _ending; }

inline bool                         // True iff Tokens are all within range
   contains(                        // Is
     Token             token,       // This Token within our range
     Count             count) const // For this length?
{  if( count == 0 )
     return token >= _origin && token <= _ending;
   else
     return token >= _origin && token < _ending;
}

inline std::string                  // The class name
   get_class_name( void ) const     // Get the class name
{  const char* mangled= typeid(*this).name();
   return demangle(mangled);
}

inline Count                        // The number of associated Tokens
   length( void ) const             // Return the associated Token length
{  return _length; }                // The number of Tokens in this Network

inline Token                        // The starting Token number
   origin( void ) const             // Return the origin Token
{  return _origin; }                // The first Token for this Network

inline char*                        // Buffer
   to_buffer(                       // Get object descriptor
     Buffer             buffer) const // Working buffer
{  snprintf(buffer, BUFF_SIZE, "NN(%.10lx) [%.10lx::%.10lx].%.6lx %s",
            (long)this, _origin, _ending, _length, get_class_name().c_str());
   return buffer;
}

virtual int
   verbose( void ) const            // Get debug verbosity
{  return _verbose; }

virtual Count                       // The work charge
   work_charge( void ) const        // Get work charge
{  return _charge; }

//----------------------------------------------------------------------------
// Network::Methods
//----------------------------------------------------------------------------
public:
virtual void
   debug( void ) const              // Debug the Network instance
{  Buffer              buffer;
   debugf(">> %s.debug\n", to_buffer(buffer));
}

virtual RC                          // Fanin count
   fanin(                           // Process fanin
     Token             token,       // Token address
     Pulse             pulse)       // Weighted fanout value
{  (void)token; (void)pulse; return 1; } // Base class implementation

inline void
   fanout_begin(                    // Fanout begin message
     Token             token,       // Token address
     Count             count) const // Token count
{  Buffer buffer;
   debugf(">> %s.fanout(%lx,%lx)\n", to_buffer(buffer), token, count);
   assert( contains(token, count) );
}

inline void
   fanout_end(                      // Fanout complete message
     RC                rc,          // Fanin count
     Count             count) const // Token count
{  Buffer buffer;
   debugf("<< %s.fanout\n   %ld fanins for %ld fanout tokens\n",
     to_buffer(buffer), rc, count);
}

virtual RC                          // Fanin count                  ts
   fanout(                          // Process fanout
     Token             token,       // Token address
     Count             count)       // Token count
{  (void)token; (void)count; return 0; } // Base class implementation

virtual void
   update( void )                   // Clock update
{  }

virtual void
   work(                            // Begin work
     Token             token,       // Work Token address
     Count             count)       // Work Token count
{  (void)token; (void)count; }      // Base class implementation NOP
}; // class Network

//----------------------------------------------------------------------------
//
// Class-
//       InpNetwork
//
// Purpose-
//       An input Network instance.
//
//----------------------------------------------------------------------------
class InpNetwork : public Network { // An Input Network Neuron
   using Network::Network;
//----------------------------------------------------------------------------
// InpNetwork::Methods
//----------------------------------------------------------------------------
public:
virtual Value_t                     // The current Value_t
   to_value(                        // Get Value_t of
     Token             token) const // This Token
{  (void)token; return 0; }         // Override this method
}; // class InpNetwork

//----------------------------------------------------------------------------
//
// Class-
//       OutNetwork
//
// Purpose-
//       An output Network instance.
//
//----------------------------------------------------------------------------
class OutNetwork : public InpNetwork { // An Output Network Neuron
   using InpNetwork::InpNetwork;
//----------------------------------------------------------------------------
// OutNetwork::Methods
//----------------------------------------------------------------------------
public:
virtual const Value_t*              // The current output Value_t* array
   to_value( void ) const           // Get current output array
{  return nullptr; }                // Override this method

virtual Value_t                     // The current Value_t
   to_value(                        // Get Value_t of
     Token             token) const // This Token
{  assert( contains(token) );

   Count index= token - _origin;
   return to_value()[index];
}
}; // class OutNetwork

//----------------------------------------------------------------------------
//
// Class-
//       Layer
//
// Purpose-
//       Network container layer.
//
// Implementation note-
//       All Network layers must be allocated. ~Layer deletes them.
//
// Implementation note-
//       Some Layers are buffered. It can take multiple cycles for a signal
//       to propagate from one Layer to the next. The application is
//       responsible for synchronization between Layer groups.
//
// Implementation notes-
//       During the build, the _length and _ending values for Layers have not
//       been set. The build_length() and build_locate() methods should be
//       used to locate the token.
//
//       build_length() deterines the length of a Network during the build.
//       build_locate() can be used to locate a Network during the build.
//
//----------------------------------------------------------------------------
class Layer : public Network {      // Network container layer
public:
std::atomic<uint32_t>  clock;       // The current clock index
double                 clock_time;  // The time that the clock was set

const Layer&           _owner;      // The owning container
size_t                 layer_count; // The number of layer_arrays allocated
size_t                 layers_used; // The number of layer_arrays used
Network**              layer_array; // The contained Networks or Layers

// Input and output layer accessors
Network*               input_layer=  nullptr; // The input layer
Network*               output_layer= nullptr; // The output Layer

//----------------------------------------------------------------------------
// Layer::Destructor, Constructors
//----------------------------------------------------------------------------
   ~Layer( void )
{  IFDEBUG( debugf("Layer(%p).~Layer\n", this); )

   // Free the associated storage arrays
   for(size_t i= 0; i<layers_used; i++)
     delete layer_array[i];

   free((void*)layer_array);
}

   Layer(                           // This is NOT a copy constructor
     Layer&            owner)       // The container Layer for this Layer
:  Network(0)
,  clock(0), clock_time(0.0)
,  _owner(owner), layer_count(0), layers_used(0), layer_array(nullptr)
{  IFDEBUG( debugf("Layer(%p).Layer\n", this); )
}

//----------------------------------------------------------------------------
// Layer::Build methods
//----------------------------------------------------------------------------
virtual Count                       // The resultant length
   build(                           // Complete the build
     Token             origin)      // Origin index
{  IFDEBUG(
     _origin= origin;               // (Set here only for debugf message.)
     _length= build_length();
     _ending= _origin + _length;

     Buffer buffer;
     debugf("%s.build\n", to_buffer(buffer));
   )

   // Contained layers depend upon accuracy of these variables during build
   _origin= origin;
   _length= 0;
   _ending= origin + build_length();
   for(size_t i= 0; i<layers_used; i++) {
     Token length= layer_array[i]->build(origin); // Complete the layer
      origin += length;             // Update for length
     _length += length;
   }

   return _length;
}

virtual void
   build_debug( void ) const        // Debug the build
{  Buffer buffer;
   layer_debug(buffer);

   for(size_t i= 0; i<layers_used; i++) {
     debugf(">>>> %s\n", layer_array[i]->to_buffer(buffer));
     layer_array[i]->build_debug();
   }
}

virtual Count                       // The resultant length
   build_length( void ) const       // Get total contained length
{  Count count= 0;
   for(size_t i= 0; i<layers_used; i++)
     count += layer_array[i]->build_length();

   return count;
}

virtual Network*                    // The associated Network
   build_locate(                    // Locate the Network
     Token             token) const // For this token
{  IFDEBUG( if( verbose() > 0 ) {
     Buffer buffer;
     debugf("%s.build_locate(%lx)\n", to_buffer(buffer), token);
   } ) // IFDEBUG( if ...

   if( token < _origin || token >= (_origin + build_length()) )
     return _owner.build_locate(token);

   Count count= token - _origin;
   for(size_t i= 0; i<layers_used; i++) {
     Count length= layer_array[i]->build_length();
     if( count < length ) {
       IFDEBUG( if( verbose() > 0 ) {
         Buffer buffer;
         debugf("%s <<<<located\n", layer_array[i]->to_buffer(buffer));
       } ) // IFDEBUG( if ...

       return layer_array[i];
     }

     count -= length;
   }

   throw ShouldNotOccur("Layer.build_locate");
   return (Network*)this;           // Avoid compiler warnings
}

virtual bool                        // TRUE if build_update should be recalled
   build_update(                    // Update build
     int               pass)        // Build update counter
{  bool result= false;
   for(size_t i= 0; i<layers_used; i++) {
     Buffer buffer;
     bool rc= layer_array[i]->build_update(pass);
     IFDEBUG(
       debugf("%s.build_update(%d) %s\n", layer_array[i]->to_buffer(buffer),
              pass, rc ? "true" : "false");
     )
     result |= rc;
   }

   _charge= 0;
   for(size_t i= 0; i<layers_used; i++)
     _charge += layer_array[i]->work_charge();
   if( _charge == 0 ) _charge= 1;

   return result;
}

void
   insert_layer(                    // Insert
     Network*          layer)       // This Network Layer
{  IFDEBUG(
     Buffer buffer;
     debugf("%s.insert_layer\n", to_buffer(buffer));
     debugf("%s <<<<inserted\n", layer->to_buffer(buffer));
   )

   if( input_layer == nullptr )
     input_layer= layer;
   output_layer= layer;

   if( layers_used >= layer_count ) {
     size_t      new_count= layer_count + 8;
     Network** new_array= (Network**)malloc(new_count*sizeof(Network*));
     for(size_t i= 0; i<layers_used; i++)
       new_array[i]= layer_array[i];

     free((void*)layer_array);

     layer_count= new_count;
     layer_array= new_array;
   }

   layer_array[layers_used]= layer;
   layers_used++;
}

inline void
   layer_debug(                     // Debug the build layer
     Buffer&           buffer) const // Using this working buffer
{  debugf("%s.layer_debug\n", to_buffer(buffer));
   debugf(">> inp_layer(%p)\n", input_layer);
   debugf(">> out_layer(%p)\n", output_layer);
   debugf(">> Layers(%p): (%ld of %ld)\n",
          layer_array, (long)layers_used, (long)layer_count);
}

//----------------------------------------------------------------------------
// Layer::Methods
//----------------------------------------------------------------------------
virtual void
   debug( void ) const              // Dynamic debuging
{  Network::debug();

   for(size_t i= 0; i<layers_used; i++)
     layer_array[i]->debug();
}

virtual RC                          // Fanin count
   fanin(                           // Process fanin
     Token             token,       // Token address
     Pulse             pulse)       // Weighted fanout value
{  IFDEBUG(
     Buffer buffer;
     debugf("%s.fanin(%lx,%d)\n", to_buffer(buffer), token, pulse);
   )
   assert( contains(token) );

   return locate(token)->fanin(token, pulse);
}

virtual RC                          // Fanin count
   fanout(                          // Process fanout
     Token             token,       // Token address
     Count             count)       // Token count
{  IFDEBUG( if( verbose() > 0 ) fanout_begin(token, count); )
// assert( contains(token,count) ); // (In fanout_begin)

   RC rc= 0;
   Count dones= 0;
   for(size_t i= 0; i<layers_used; i++) {
     Network* layer= layer_array[i];
     Count layer_count= layer->length();
     if( layer->contains(token, 0) ) {
       layer_count -= (token - layer->origin());
       layer_count= std::min(count-dones, layer_count);
#if false // Zero-length layers supported (NOT TESTED WITH ZERO-LENGTH LAYERS)
       if( layer_count > 0 ) {
         rc += layer->fanout(token, layer_count);
         dones += layer_count;
         if( dones >= count )
           break;
       } else {
         if( layer->length() > 0 )
           break;
         else
           rc += layer->fanout(token, 0);
       }
     }
#else     // Zero-length layers not supported
       rc += layer->fanout(token, layer_count);
       dones += layer_count;
       if( dones >= count )
         break;
     }
#endif

     token += layer_count;
   }

   IFDEBUG( if( verbose() > 0 ) fanout_end(rc, count); )
   return rc;
}

virtual Network*                    // The associated Network*
   locate(                          // Locate the Network
     Token             token) const // For this token
{  if( !contains(token) )
     return _owner.locate(token);

   for(size_t i= 0; i<layers_used; i++)
     if( layer_array[i]->contains(token) )
       return layer_array[i];

   throw ShouldNotOccur("Layer.locate");
}

virtual void
   update( void )                   // Clock update
{  IFDEBUG(
     Buffer buffer;
     debugf("%s.update\n", to_buffer(buffer));
   )

   clock++;                         // Increment the Clock
   for(size_t i= 0; i<layers_used; i++) // Update contained Layers
     layer_array[i]->update();
}
}; // struct Layer
}  // namespace NETWORK_H_NAMESPACE

#include "Network.i"

#endif // NETWORK_H_INCLUDED
