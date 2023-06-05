//----------------------------------------------------------------------------
//
//       Copyright (C) 2022-2023 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       StreamSet.cpp
//
// Purpose-
//       Implement http/StreamSet.h
//
// Last change date-
//       2023/06/04
//
//----------------------------------------------------------------------------
#include <new>                      // For std::bad_alloc
#include <atomic>                   // For std::atomic
#include <cstring>                  // For memset
#include <stdexcept>                // For std::runtime_error, ...
#include <string>                   // For std::string

#include <assert.h>                 // For assert
#include <stdio.h>                  // For fprintf
#include <stdint.h>                 // For integer types

#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/Dispatch.h>           // For namespace pub::dispatch
#include <pub/Exception.h>          // For pub::Exception
#include <pub/Statistic.h>          // For pub::Active_record
#include <pub/Trace.h>              // For pub::Trace
#include <pub/utility.h>            // For pub::to_string, ...

#include "pub/http/Client.h"        // For pub::http::Client
#include "pub/http/Ioda.h"          // For pub::http::Ioda
#include "pub/http/Options.h"       // For pub::http::Options
#include "pub/http/Request.h"       // For pub::http::Request
#include "pub/http/Response.h"      // For pub::http::Response
#include "pub/http/Server.h"        // For pub::http::Server
#include "pub/http/Stream.h"        // For pub::http::Stream, implemented

using namespace _LIBPUB_NAMESPACE;
using namespace _LIBPUB_NAMESPACE::debugging;
using _LIBPUB_NAMESPACE::utility::to_string;
using _LIBPUB_NAMESPACE::utility::visify;
using std::string;

namespace _LIBPUB_NAMESPACE::http { // Implementation namespace
//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 1                       // Verbosity, higher is more verbose

,  BUFFER_SIZE= 8'096               // Input buffer size (Header collector)
,  POST_LIMIT= 1'048'576            // POST/PUT size limit
,  USE_ITRACE= true                 // Use internal trace?
,  USE_REPORT= false                // Use event Reporter?
}; // enum

//============================================================================
//
// Method-
//       StreamSet::Node::~Node
//
// Purpose-
//       Destructor
//
// Implementation notes-
//       It is an error to delete a Node that has a parent or child.
//
//----------------------------------------------------------------------------
   StreamSet::Node::~Node( void )
{
   assert( parent == nullptr && child == nullptr );
}

//----------------------------------------------------------------------------
//
// Method-
//       StreamSet::Node::insert
//
// Purpose-
//       Insert a child Node
//
// Implementation notes-
//       REQUIRES: The StreamSet must be locked
//       The inserted Node's child list is NOT inspected or modified.
//
//----------------------------------------------------------------------------
void
   StreamSet::Node::insert(         // Insert (at beginning of child list)
     Node*             node)        // This Node
{
   assert( node->parent == nullptr ); // Must not already be on a list
   node->parent= this;
   node->peer= child;
   child= node;
}

//----------------------------------------------------------------------------
//
// Method-
//       StreamSet::Node::remove
//
// Purpose-
//       Remove a child Node
//
// Implementation notes-
//       REQUIRES: The StreamSet must be locked
//       The removed Node's child list is NOT inspected or modified.
//
//----------------------------------------------------------------------------
void
   StreamSet::Node::remove(         // Remove (from the child list)
     Node*             node)        // This Node
{
   assert( node->parent == this );  // Must be our child Node

   node->parent= nullptr;           // Consider it already removed
   if( child == node ) {            // If removing the first child
     child= node->peer;             // Remove it from the list
     node->peer= nullptr;           // (Not strictly necessary)
     return;
   }

   Node* prev= child;
   while( prev ) {                  // Search the child list
     if( prev->peer == node ) {     // If prev->(Node to be removed)
       prev->peer= node->peer;      // Remove the Node from the list
       node->peer= nullptr;         // (Not strictly necessary)
       return;
     }

     prev= prev->peer;              // Follow the list
   }

   // SHOULD NOT OCCUR: The Node to be removed wasn't on the list
   debugf("StreamSet::Node(%p)::remove(%p), but it's not on the child list\n"
         , this, node);
   node->peer= nullptr;             // (Even now, not strictly necessary)
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void
   StreamSet::Node::remove( void )  // Remove THIS node from its parent
{
   assert( parent != nullptr );     // (It must actually *have* a parent)
   parent->remove(this);
}

//============================================================================
//
// Method-
//       StreamSet::StreamSet
//       StreamSet::~StreamSet
//
// Purpose-
//       Constructor
//       Destructor
//
//----------------------------------------------------------------------------
   StreamSet::StreamSet(            // Constructor
     Node*             node)        // The (user-owned) root Node
{  if( HCDM || VERBOSE > 1 ) debugf("StreamSet(%p)!\n", this);

   root= node;

// INS_DEBUG_OBJ("StreamSet");
}

   StreamSet::~StreamSet( void )    // Destructor
{  if( HCDM || VERBOSE > 1 ) debugf("StreamSet(%p)~\n", this);

   assert( root->child == nullptr ); // The StreamSet must be empty

// REM_DEBUG_OBJ("StreamSet");
}

//----------------------------------------------------------------------------
//
// Method-
//       StreamSet::get_stream
//
// Purpose-
//       Locate Stream by identifier
//
//----------------------------------------------------------------------------
StreamSet::stream_ptr               // The associated Stream
   StreamSet::get_stream(           // Locate the Stream given Stream::ident
     stream_id         id) const    // For this Stream identifier
{  std::lock_guard<decltype(mutex)> lock(mutex);

   const_iterator it= map.find(id);
   if( it != map.end() )
     return it->second;

   return nullptr;
}

//----------------------------------------------------------------------------
//
// Method-
//       StreamSet::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   StreamSet::debug(const char* info) const  // Debugging display
{  debugh("StreamSet(%p)::debug(%s)\n", this, info);

   debugf("root->parent(%p) ", root->parent);
   debugf("root->child(%p) ",  root->child);
   debugf("root->peer(%p) ",   root->peer);
}

//----------------------------------------------------------------------------
//
// Method-
//       StreamSet::assign_stream_id
//
// Purpose-
//       Assign a Stream identifier
//
//----------------------------------------------------------------------------
StreamSet::stream_id                // The next available Stream identifier
   StreamSet::assign_stream_id(     // Assign a Stream identifier
     int               addend)      // After incrementing it by this value
{
   std::atomic<stream_id>* ident_ptr= (std::atomic<stream_id>*)&ident;
   stream_id old_value= ident_ptr->load();
   for(;;) {
     stream_id new_value= old_value + addend;
     if( new_value < 0 )            // If 31-bit arithmetic overflow
       return -1;                   // We're out of identifiers
     if( ident_ptr->compare_exchange_strong(old_value, new_value) )
       return new_value;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       StreamSet::change
//
// Purpose-
//       Locate Stream by identifier
//
//----------------------------------------------------------------------------
void
   StreamSet::change(               // Change a Stream's parent
     Stream*           parent,      // The new parent Stream
     Stream*           stream)      // The Stream to move
{  std::lock_guard<StreamSet> lock(*this);

   (void)parent; (void)stream;
}

//----------------------------------------------------------------------------
//
// Method-
//       StreamSet::insert
//
// Purpose-
//       Locate Stream by identifier
//
//----------------------------------------------------------------------------
void
   StreamSet::insert(               // Insert Stream
     Stream*           parent,      // The parent Stream
     Stream*           stream)      // The Stream to insert
{  std::lock_guard<StreamSet> lock(*this);

   parent->insert(stream);
}

//----------------------------------------------------------------------------
//
// Method-
//       StreamSet::remove
//
// Purpose-
//       Locate Stream by identifier
//
//----------------------------------------------------------------------------
void
   StreamSet::remove(               // Remove Stream
     Stream*           stream)      // The Stream to remove
{  std::lock_guard<StreamSet> lock(*this);

   stream->remove();
}
}  // namespace _LIBPUB_NAMESPACE::http
