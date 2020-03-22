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
//       NetTypes.h
//
// Purpose-
//       Define the macros, types and requisite includes for Network.h
//
// Last change date-
//       2018/01/01
//
// Usage note-
//       #define NETWORK_H_NAMESPACE to override the default namespace name
//
//----------------------------------------------------------------------------
#ifndef NETTYPES_H_INCLUDED
#define NETTYPES_H_INCLUDED

#include <assert.h>                 // For assert macro
#include <atomic>                   // For std::atomic_flag, ...
#include <boost/core/demangle.hpp>  // For boost::core::demangle
#include <cstdlib>                  // For rand, ...
#include <inttypes.h>               // For PRIx64, ...
#include <iostream>                 // For std::cout, ...
#include <limits.h>                 // For UINT_MAX, ...
#include <random>                   // For std::random and friends
#include <stdexcept>                // For std::runtime_error, ...
#include <stdint.h>                 // For int_16_t, ...
#include <stdio.h>                  // For snprintf, ...
#include <stdlib.h>                 // For aligned_alloc, ...
#include <string>                   // For std::string, ...
#include <string.h>                 // For memset, ...
#include <typeinfo>                 // For typeid

#include "com/Debug.h"

#include "Neuron.h"                 // Base class

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef NETWORK_H_NAMESPACE         // Define the namespace
#define NETWORK_H_NAMESPACE NN
#endif

#ifndef USE_CHECK
#define USE_CHECK                   // If defined, error checking code included
#endif

#ifndef USE_DEBUG
#define USE_DEBUG                   // If defined, debug tracing code included
#endif

//----------------------------------------------------------------------------
// Dependent macros
//----------------------------------------------------------------------------
#ifdef  USE_CHECK
  #define IFCHECK(x) {x}
  #define ELCHECK(x) {}
#else
  #define IFCHECK(x) {}
  #define ELCHECK(x) {x}
#endif

#ifdef  USE_DEBUG
  #define IFDEBUG(x) {x}
  #define ELDEBUG(x) {}
#else
  #define IFDEBUG(x) {}
  #define ELDEBUG(x) {x}
#endif

namespace NETWORK_H_NAMESPACE {
//----------------------------------------------------------------------------
// NN::Forward references
//----------------------------------------------------------------------------
inline std::string demangle(const char*); // Instantiated in this file

//----------------------------------------------------------------------------
// NN::Enumerations and typedefs
//----------------------------------------------------------------------------
typedef std::size_t    size_t;      // Import std::size_t type

typedef Neuron::Token  Token;       // Import Neuron types
typedef Neuron::Pulse  Pulse;
typedef Neuron::Count  Count;
typedef Neuron::RC     RC;

typedef volatile uint32_t
                       Clock_t;     // Clock interval
typedef int16_t        Value_t;     // Neuron value, must be signed.
typedef int16_t        Weight_t;    // Trigger weight, must be signed.

// Note: Overrides must be within NETWORK_H_NAMESPACE
#ifndef _NETTYPE_BUNDLE_DEFINED     // Bundle typedefs
#define _NETTYPE_BUNDLE_DEFINED
#define BUNDLE_DIM     8            // Number of elements in a bundle
typedef int32_t        Bundle_index_t;  // The Fanout index type
typedef int16_t        Bundle_weight_t; // The Fanout weight type

enum
{  Bundle_MAX= INT16_MAX            // Maximum Bundle_weight_t
,  Bundle_MIN= -INT16_MAX           // Minimum Bundle_weight_t
,  Bundle_ONE= INT16_MAX / 2        // Unit Bundle_weight_t
};
#endif // _NETTYPE_BUNDLE_DEFINED

// Probably obsolete
typedef std::atomic_flag
                       atomic_flag; // Import std::atomic_flag
typedef volatile char  v_char_flag; // A (volatile character)atomic_flag

//----------------------------------------------------------------------------
//
// Struct-
//       FanoutBundle
//
// Purpose-
//       Fanout bundle descriptor.
//
//----------------------------------------------------------------------------
struct FanoutBundle {               // FanoutBundle descriptor
enum {DIM= BUNDLE_DIM};
Bundle_index_t         index[DIM];  // The index array
Bundle_weight_t        weight[DIM]; // The weight array
}; // struct FanoutBundle

//----------------------------------------------------------------------------
// Exceptions
//----------------------------------------------------------------------------
class NetworkException : public std::runtime_error {
    using std::runtime_error::runtime_error;

public:
inline std::string                  // The class name
   get_class_name( void ) const     // Get the class name
{
   const char* mangled= typeid(*this).name();
   return demangle(mangled);
}
}; // class NetworkException

class BuildError : public NetworkException {
    using NetworkException::NetworkException;
};

class LocateException : public NetworkException {
    using NetworkException::NetworkException;
};

class NoStorageException : public NetworkException {
    using NetworkException::NetworkException;
};

class NotCodedYet : public NetworkException {
    using NetworkException::NetworkException;
};

class NotImplemented : public NetworkException {
    using NetworkException::NetworkException;
};

class SynchException : public NetworkException {
    using NetworkException::NetworkException;
};

class ShouldNotOccur : public NetworkException {
    using NetworkException::NetworkException;
};

//----------------------------------------------------------------------------
//
// Subroutines-
//       NN::aligned_alloc
//       NN::free
//       NN::malloc
//
// Purpose-
//       Invoke associated system function, but throws NoStorageException
//       rather than return nullptr.
//
//----------------------------------------------------------------------------
static inline void*                 // NEVER RETURNS nullptr
   aligned_alloc(size_t align, size_t size)
{  void* result= ::aligned_alloc(align, size);
   if( result == nullptr ) throw NoStorageException("NN::aligned_alloc");

   memset(result, 0, size);
   return result;
}

static inline void
   free(                            // Release storage, nullptr allowed
     void*             storage)     // Allocated storage address
{  if( storage != nullptr )
     ::free(storage);
}

static inline void*                 // NEVER RETURNS nullptr
   malloc(size_t size)
{  void* result= ::malloc(size);
   if( result == nullptr ) throw NoStorageException("NN::malloc");

   memset(result, 0, size);
   return result;
}

//----------------------------------------------------------------------------
//
// Subroutines-
//       NN::demangle
//       NN::_nop
//       NN::xxyyz
//
// Purpose-
//       Utility functions
//
//----------------------------------------------------------------------------
inline std::string                  // Resultant
   demangle(                        // Demangle a typeid
     const char*       mangled)     // Resultant of typeid(*this).name()
{//const char* mangled= typeid(*this).name(); // Caller does this
   return boost::core::demangle(mangled);
}

extern void
   _nop(                            // Does nothing (in Network.cpp)
     void*             thing);      // Except reference this unused variable

inline Pulse                        // Adjusted Value_t
   to_pulse(                        // Adjust value for weight
     Value_t           value,       // Input value
     Bundle_weight_t   weight)      // Input weight
{  Pulse pulse= value * weight;
   if( pulse >= 0 )                 // (Always true if Bundle_weight_t unsigned)
     pulse += (Bundle_ONE - 1);
   else
     pulse -= (Bundle_ONE - 1);
   pulse /= Bundle_ONE;
   return pulse;
}

//----------------------------------------------------------------------------
// Static flag for running explicit tests. Defined in Network.cpp
//----------------------------------------------------------------------------
extern const int       _testID;     // Explicit test identifier, for debugging
}  // namespace NETWORK_H_NAMESPACE

#endif // NETTYPES_H_INCLUDED
