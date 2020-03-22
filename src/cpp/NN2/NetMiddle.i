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
//       NetMiddle.i
//
// Purpose-
//       NetMiddle.h method implementations.
//
// Last change date-
//       2018/01/01
//
// Implementation notes-
//       For 64-bit GCC compile only (Does not use inttypes.h macros.)
//
//       The current implementation only forwards triggered FanoutNeurons to
//       the next Layer. Each FanoutNeuron Layer therefore delays one cycle.
//
//----------------------------------------------------------------------------
#ifndef NETMIDDLE_I_INCLUDED
#define NETMIDDLE_I_INCLUDED

namespace NETWORK_H_NAMESPACE {
//----------------------------------------------------------------------------
//
// Class-
//       MiddleLayer
//
// Purpose-
//       MiddleLayer method implementations
//
//----------------------------------------------------------------------------
Count                               // The resultant length
   MiddleLayer::build(              // Complete the build
     Token             origin)      // The assigned address
{  Layer::build(origin);            // Container build

   // (NOTE: Not much use to have an MiddleLayer without data.)
   // Allocate the middle data in one contiguous area.
   size_t ALIGNMENT= 4096;          // Allocate groups on different pages
   size_t ALIGN_MASK= ~(ALIGNMENT - 1); // ALIGNMENT truncation mask
   size_t gs_length= fanout_length; // Getter/setter bundle count
   gs_length += getset_length;
   gs_length *= sizeof(Value_t);    // Total getter or setter length
   getset_size= gs_length;
   gs_length += (ALIGNMENT - 1);    // Round up to or past next ALIGNMENT
   gs_length &= ALIGN_MASK;         // Truncate down to ALIGNMENT boundary
   size_t wt_length= fanout_length; // Trigger weight count
   wt_length += weight_length;
   wt_length *= sizeof(Weight_t);   // Trigger weight length
   weight_size= wt_length;
   wt_length += (ALIGNMENT - 1);    // Round up to or past next ALIGNMENT
   wt_length &= ALIGN_MASK;         // Truncate down to ALIGNMENT boundary
   size_t fb_length= fanout_length; // FanoutBundle count
   fb_length += bundle_length;
   fb_length *= sizeof(FanoutBundle);
   bundle_size= fb_length;
   // Rounded up because we don't want to share the last page of the
   // FanoutBundle storage with non-static data.
   fb_length += (ALIGNMENT - 1);    // Round up to or past next ALIGNMENT
   fb_length &= ALIGN_MASK;         // Truncate down to ALIGNMENT boundary

   size_t hd_length= 0;             // Required length (total)
   hd_length += gs_length;          // Getter pages (Dynamic)
   hd_length += gs_length;          // Setter pages (Dynamic)
   hd_length += wt_length;          // Trigger weight pages (Static)
   hd_length += fb_length;          // FanoutBundle pages (Static)
   middle_size= hd_length;          // Allocation length

   // Allocate and assign the storage
   char* storage= (char*)aligned_alloc(ALIGNMENT, hd_length);

   middle_data= (void*)storage;     // (So we know how to free it.)
   this->getter= (Value_t*)storage;  storage += gs_length;
   this->setter= (Value_t*)storage;  storage += gs_length;
   this->weight= (Weight_t*)storage; storage += gs_length;
   this->bundle= (FanoutBundle*)storage;

   return _length;
}

void
   MiddleLayer::build_debug( void ) const // Build data debugging
{  Buffer buffer;
   layer_debug(buffer);

   IFDEBUG( if( verbose() > 0 ) {
     for(size_t i= 0; i<layers_used; i++) {
       debugf(">> %s\n", layer_array[i]->to_buffer(buffer));
       layer_array[i]->build_debug();
     }
   } ) // IFDEBUG( if ...
}

//----------------------------------------------------------------------------
//
// Class-
//       OutBuffer
//
// Purpose-
//       OutBuffer method implementations
//
//----------------------------------------------------------------------------
void
   OutBuffer::debug( void ) const   // Debugging display
{  Network::debug();

   IFDEBUG(
     int      count;                // (For line formatting)
     Value_t* getter= _layer.getter + _index;
     Value_t* setter= _layer.setter + _index;

     debugf(">>>> Getters\n");
     count= 0;
     for(int j= 0; j<_length; j++) {
       if( count == 0 )
         debugf(">>>> [%.6x]", j);
       debugf(" %6d", *getter);
       getter++;
       count++;
       if( count == 8 ) {
         count= 0;
         debugf("\n");
       }
     }
     if( count != 0 )
       debugf("\n");

     debugf(">>>> Setters\n");
     count= 0;
     for(int j= 0; j<_length; j++) {
       if( count == 0 )
         debugf(">>>> [%.6x]", j);
       debugf(" %6d", *setter);
       setter++;
       count++;
       if( count == 8 ) {
         count= 0;
         debugf("\n");
       }
     }
     if( count != 0 )
       debugf("\n");
   ) // IFDEBUG(
}

//----------------------------------------------------------------------------
//
// Class-
//       FaninpNeuron
//
// Purpose-
//       FaninpNeuron method implementations
//
//----------------------------------------------------------------------------
Count                               // The build length
   FaninpNeuron::build(             // Complete the build
     Token             origin)      // Assigned origin
{  Network::build(origin);
   if( this == _layer.input_layer || this == _layer.output_layer )
     throw BuildError("FaninpNeuron cannot be the input or output Layer");

   _bundle_index += _layer.fanout_length;
   _weight_index += _layer.fanout_length;
   _prevN= dynamic_cast<InpNetwork*>(_layer.build_locate(origin-1));
   if( _prevN == nullptr )
     throw BuildError("FaninpNeuron must follow InpNetwork");
   _nextN= _layer.build_locate(_ending);
   if( _nextN->length() != _length )
     throw BuildError("TEMPORARY: FaninpNeuron _nextN->length()");

   return _length;
}

void
   FaninpNeuron::build_debug( void ) const // Build debugging display
{  debugf("   bundle_index = [%.10lx]\n", _bundle_index);
   debugf("   weight_index = [%.10lx]\n", _weight_index);

   IFDEBUG( if( verbose() > 0 ) {
     int count;

     debugf(">>>> Faninps\n");
     FanoutBundle* bundle= _layer.bundle + _bundle_index;
     count= 0;
     for(Count i= 0; i<_length; i++) {
       for(int j= 0; j<FanoutBundle::DIM; j++) {
         if( count == 0 )
           debugf(">>>> [%.6lx][%d]", i, j);
         debugf(" [%.6x:%6.3f]", bundle->index[j],
                (double)bundle->weight[j]/(double)Bundle_ONE);
         count++;
         if( count == 4 ) {
           count= 0;
           debugf("\n");
         }
       }
       bundle++;
       if( count != 0 )
         debugf("\n");
       count= 0;
     }

     debugf(">>>> Weights\n");
     Weight_t* weight= _layer.weight + _weight_index;
     for(Count i= 0; i<_length; i++) {
       if( count == 0 )
         debugf(">>>> [%.6lx]", i);
       debugf(" %6d", *weight);
       weight++;
       count++;
       if( count == 8 ) {
         count= 0;
         debugf("\n");
       }
     }
     if( count != 0 )
       debugf("\n");
   } ) // IFDEBUG( if ...
}

bool                                // TRUE if build not completed
   FaninpNeuron::build_update(      // Complete the build
     int               pass)        // Build pass number
{  if( pass != 0 ) return false;

   // Random distribution controls
   Count               _prevLength= _prevN->length();
   std::random_device  rd;          // Used to seed mte
   std::mt19937        mte(rd());   // Standard mersenne_twister_engine
   std::uniform_int_distribution<>  // Trigger weight
                       trigger_weight(-127, 255);

   assert( _prevLength >= FanoutBundle::DIM ); // (TEMPORARY RESTRICTION)
   long max_bundle_weight= (_prevLength*Bundle_ONE*1)/(_length*2);
   max_bundle_weight= std::min((long)Bundle_MAX, max_bundle_weight);
   long min_bundle_weight= std::max(1L, max_bundle_weight/8);

   std::uniform_int_distribution<>  // Bundle weight
                  bundle_weight(min_bundle_weight, max_bundle_weight);
   std::uniform_int_distribution<>
                  bundle_index(0, _prevLength - 1);
   std::uniform_int_distribution<>
                  positive(0, 2);

   // Initialize weights and FaninpBundles
   FanoutBundle* _thisBundle= _layer.bundle + _bundle_index;
   Weight_t*     _thisWeight= _layer.weight + _weight_index;
   for(size_t j= 0; j<_length; j++) {
     (*_thisWeight)= trigger_weight(mte);
     _thisWeight++;

     int min_k= 0;
     for(int k=min_k; k<FanoutBundle::DIM; k++) {
       _thisBundle->weight[k]= bundle_weight(mte);
       if( positive(mte) == 0 )
         _thisBundle->weight[k]= -_thisBundle->weight[k];

       bool duplicate= true;
       while( duplicate ) {
         duplicate= false;
         _thisBundle->index[k]= bundle_index(mte);
         for(int x= 0; x<k; x++) {
           if( _thisBundle->index[x] == _thisBundle->index[k] ) {
             duplicate= true;
             break;
           }
         }
       }
     }
     _thisBundle++;
   }

   return false;
}

void
   FaninpNeuron::debug( void ) const // Debugging display
{  Network::debug();

   IFDEBUG( if( verbose() > 0 ) {
     FanoutBundle* bundle= _layer.bundle + _bundle_index;
     debugf(">>>> Getters\n");
     int count= 0;
     for(int j= 0; j<_length; j++) {
       if( count == 0 )
         debugf(">>>> [%.6x]", j);
       debugf(" %6d", faninp_bundle(*bundle));
       bundle++;
       count++;
       if( count == 8 ) {
         count= 0;
         debugf("\n");
       }
     }
     if( count != 0 )
       debugf("\n");
   } ) // IFDEBUG( if ...
}

RC                                  // Fanin count
   FaninpNeuron::fanout(            // Process fanout
     Token             token,       // Fanout address
     Count             count)       // Fanout count
{  IFDEBUG( if( verbose() > 0 ) fanout_begin(token, count); )

   Count bundle_index= get_bundle_index(token);
   FanoutBundle* _bundle= _layer.bundle + bundle_index;
   Count weight_index= get_weight_index(token);
   Weight_t* _weight= _layer.weight + weight_index;

   // TODO: Figure out how to distribute the pulse
   token += _length; // TEMPORARY: Just drive next layer
   // BUILD: Just drive the associated fanin
   RC rc= 0;
   for(Count i= 0; i<count; i++) {
     Pulse pulse= faninp_bundle(*_bundle);
     Weight_t weight= *_weight;
     if( weight > 0 ) {
       if( pulse >= weight )
         rc += _nextN->fanin(token, pulse); // TEMPORARY
     } else if( weight < 0 ) {
       if( pulse <= weight )
         rc += _nextN->fanin(token, pulse); // TEMPORARY
     }

     _bundle++;
     _weight++;
     token++; // TEMPORARY
   }

   IFDEBUG( if( verbose() > 0 ) fanout_end(rc, count); )
   return rc;
}

//----------------------------------------------------------------------------
//
// Class-
//       FanoutNeuron
//
// Purpose-
//       FanoutNeuron method implementations
//
//----------------------------------------------------------------------------
Count                               // The resultant length
   FanoutNeuron::build(             // Complete the build
     Token             origin)      // Assigned origin
{  Network::build(origin);
   if( this == _layer.output_layer )
     throw BuildError("FanoutNeuron cannot be the output Layer");

   _nextN= _layer.build_locate(_ending);  // Locate the next Network group
   return _length;
}

void
   FanoutNeuron::build_debug( void ) const // Build debugging display
{  debugf("   fanout_index = [%.10lx]\n", _index);

   IFDEBUG( if( verbose() > 0 ) {
     int count;
     FanoutBundle* bundle= _layer.bundle + _index;
     Value_t*      weight= _layer.weight + _index;

     debugf(">>>> Fanouts\n");
     count= 0;
     for(Count i= 0; i<_length; i++) {
       for(int j= 0; j<FanoutBundle::DIM; j++) {
         if( count == 0 )
           debugf(">>>> [%.6lx][%d]", i, j);
         debugf(" [%.6x:%6.3f]", bundle->index[j],
                (double)bundle->weight[j]/(double)Bundle_ONE);
         count++;
         if( count == 4 ) {
           count= 0;
           debugf("\n");
         }
       }
       bundle++;
       if( count != 0 )
         debugf("\n");
       count= 0;
     }
     debugf("\n");

     debugf(">>>> Weights\n");
     for(Count i= 0; i<_length; i++) {
       if( count == 0 )
         debugf(">>>> [%.6lx]", i);
       debugf(" %6d", *weight);
       weight++;
       count++;
       if( count == 8 ) {
         count= 0;
         debugf("\n");
       }
     }
     if( count != 0 )
       debugf("\n");
   } ) // IFDEBUG( if ...
}

bool                                // TRUE if build not completed
   FanoutNeuron::build_update(      // Complete the build
     int               pass)        // Build pass number
{  if( pass != 0 ) return false;

   // Random distribution controls
   Count               _nextLength= _nextN->length();
   std::random_device  rd;          // Used to seed mte
   std::mt19937        mte(rd());   // Standard mersenne_twister_engine
   std::uniform_int_distribution<>  // Trigger weight
                       trigger_weight(-127, 255);

   assert( _nextLength >= FanoutBundle::DIM ); // (TEMPORARY RESTRICTION)
   long max_bundle_weight= (_nextLength*Bundle_ONE*1)/(_length*2);
   max_bundle_weight= std::min((long)Bundle_MAX, max_bundle_weight);
   long min_bundle_weight= std::max(1L, max_bundle_weight/8);

   std::uniform_int_distribution<>  // Bundle weight
                  bundle_weight(min_bundle_weight, max_bundle_weight);
   std::uniform_int_distribution<>
                  bundle_index(0, _nextLength - 1);
   std::uniform_int_distribution<>
                  positive(0, 2);

   // Initialize weights and FanoutBundles
   FanoutBundle* _thisBundle= _layer.bundle + _index;
   Value_t*      _thisWeight= _layer.weight + _index;
   for(size_t j= 0; j<_length; j++) {
     (*_thisWeight)= trigger_weight(mte);
     _thisWeight++;

     int min_k= 0;
     if( _nextLength == _length ) {
       Bundle_index_t L= j-1;
       Bundle_index_t R= j+1;
       if( j == 0 ) L= _length - 1;
       if( R >= _length ) R= 0;
       _thisBundle->index[0]= L;
       _thisBundle->index[1]= j;
       _thisBundle->index[2]= R;

       _thisBundle->weight[0]= Bundle_ONE/8;
       _thisBundle->weight[1]= Bundle_ONE/4;
       _thisBundle->weight[2]= Bundle_ONE/8;
       min_k= 3;
     }
     for(int k=min_k; k<FanoutBundle::DIM; k++) {
       _thisBundle->weight[k]= bundle_weight(mte);
       if( positive(mte) == 0 )
         _thisBundle->weight[k]= -_thisBundle->weight[k];

       bool duplicate= true;
       while( duplicate ) {
         duplicate= false;
         _thisBundle->index[k]=  bundle_index(mte);
         for(int x= 0; x<k; x++) {
           if( _thisBundle->index[x] == _thisBundle->index[k] ) {
             duplicate= true;
             break;
           }
         }
       }
     }
     _thisBundle++;
   }

   return false;
}

void
   FanoutNeuron::debug( void ) const // Debugging display
{  Network::debug();

   IFDEBUG(
     int      count;                // (For line formatting)
     Value_t* getter= _layer.getter + _index;
     Value_t* setter= _layer.setter + _index;

     debugf(">>>> Getters\n");
     count= 0;
     for(int j= 0; j<_length; j++) {
       if( count == 0 )
         debugf(">>>> [%.6x]", j);
       debugf(" %6d", *getter);
       getter++;
       count++;
       if( count == 8 ) {
         count= 0;
         debugf("\n");
       }
     }
     if( count != 0 )
       debugf("\n");

     debugf(">>>> Setters\n");
     count= 0;
     for(int j= 0; j<_length; j++) {
       if( count == 0 )
         debugf(">>>> [%.6x]", j);
       debugf(" %6d", *setter);
       setter++;
       count++;
       if( count == 8 ) {
         count= 0;
         debugf("\n");
       }
     }
     if( count != 0 )
       debugf("\n");
   ) // IFDEBUG(
}

RC                                  // Fanin count
   FanoutNeuron::fanin(             // Process fanin
     Token             token,       // Token address
     Pulse             pulse)       // Weighted fanout value
{  // TODO: Check for overflow (Maybe use compare_and_exchange directly)
   Token index= get_index(token);
   std::atomic<Value_t>* _setter= (std::atomic<Value_t>*)_layer.setter;
   _setter[index] += pulse;

   IFDEBUG( if( _testID == 201 ) {
     debugf("FanoutNeuron(%p).fanin(%.16lx,%4d) %5d\n", this,
            token, pulse, _setter[index].load());
   } ) // IFDEBUG( if ...

   return 1;
}

RC                                  // Fanin count
   FanoutNeuron::fanout(            // Process fanout
     Token             token,       // Fanout address
     Count             count)       // Fanout count
{  IFDEBUG( if( verbose() > 0 ) fanout_begin(token, count); )

   Token index= get_index(token);
   FanoutBundle* _bundle= _layer.bundle + index;
   Value_t* _getter= _layer.getter + index;
   Value_t* _weight= _layer.weight + index;

#if true
  #define _OUTPUT _getter
#else
  #define _OUTPUT _weight
#endif
   RC rc= 0;
   for(Count i= 0; i<count; i++) {
     if( (*_weight) > 0 ) {
       if( (*_getter) >= (*_weight) )
         rc += fanout_bundle(*_bundle, *_OUTPUT);
     } else if( (*_weight) < 0 ) {
       if( (*_getter) <= (*_weight) )
         rc += fanout_bundle(*_bundle, *_OUTPUT);
     }

     _bundle++;
     _getter++;
     _weight++;
   }
#undef _OUTPUT

   IFDEBUG( if( verbose() > 0 ) fanout_end(rc, count); )
   return rc;
}
}  // namespace NETWORK_H_NAMESPACE

#endif // NETMIDDLE_I_INCLUDED
