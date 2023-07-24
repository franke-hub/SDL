//----------------------------------------------------------------------------
//
//       Copyright (C) 2022-2023 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       http/StreamSet.h
//
// Purpose-
//       HTTP StreamSet object.
//
// Last change date-
//       2023/06/04
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_HTTP_STREAMSET_H_INCLUDED
#define _LIBPUB_HTTP_STREAMSET_H_INCLUDED

#include <memory>                   // For std::shared_ptr
#include <mutex>                    // For std::mutex

#include "dev/bits/devconfig.h"     // For HTTP config controls

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
namespace http {
//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class Stream;

//----------------------------------------------------------------------------
//
// Class-
//       StreamSet
//
// Purpose-
//       Control a set of Stream objects.
//
// Implementation notes-
//       Note: StreamSet::Node is Stream's base class.
//       Although Stream objects are usually referenced using std::shared_ptr,
//       we use Node* to maintain the StreamSet Node tree.
//       We can do this because Stream guarantees that we will *always* have a
//       corresponding std::shared_ptr in the the StreamSet map for an item in
//       the Node tree.
//
//----------------------------------------------------------------------------
class StreamSet {                   // A set of Stream objects
//----------------------------------------------------------------------------
// StreamSet::Node
//----------------------------------------------------------------------------
public:
struct Node {                       // StreamSet Node
// Node::Attributes
Node*                  parent= nullptr; // The parent Node
Node*                  peer= nullptr;   // The next peer Node
Node*                  child= nullptr;  // The head child Node

// Node::Constructors/destructor
   Node( void ) = default;          // Default constructor
   Node(                            // Construct and insert this Node
     Node*             parent)      // As a child of this parent Node
{  parent->insert(this); }

   ~Node( void );                   // Destructor

// Node::Methods
void
   insert(                          // Insert (at head of the child list)
     Node*             child);      // This child (Stream) Node

void
   remove(                          // Remove
     Node*             child);      // This  child (Stream) Node

void
   remove( void );                  // Remove THIS (Stream) Node from its parent
}; // struct StreamSet::Node

//----------------------------------------------------------------------------
// StreamSet::Typedefs and enumerations
//----------------------------------------------------------------------------
protected:
typedef int32_t                     stream_id;
typedef std::shared_ptr<Stream>     stream_ptr;
typedef std::unordered_map<stream_id, stream_ptr>       map_t;
typedef map_t::const_iterator       const_iterator;

//----------------------------------------------------------------------------
// StreamSet::Attributes
//----------------------------------------------------------------------------
mutable std::mutex     mutex;       // The SteamSet mutex
map_t                  map;         // The (Stream) Node map
Node*                  root= nullptr; // The root Node

stream_id              ident= 0;    // The current Stream identifier

//----------------------------------------------------------------------------
// StreamSet::Destructor, constructors, operators
//----------------------------------------------------------------------------
public:
   StreamSet(                       // Constructor
     Node*             node);       // The (user-owned) root Node

   StreamSet(const StreamSet&) = delete; // Disallowed copy constructor

StreamSet&
   operator=(const StreamSet&) = delete; // Disallowed assignment operator

   ~StreamSet( void );              // Destructor

//----------------------------------------------------------------------------
// StreamSet::debug
//----------------------------------------------------------------------------
void debug(const char* info="") const; // Debugging display

//----------------------------------------------------------------------------
// StreamSet::Accessor methods
//----------------------------------------------------------------------------
stream_id                           // The next available Stream identifier
   assign_stream_id(                // Assign a Stream identifier
     int               addend= 2);  // After incrementing it by this value

Node*                               // The root Node
   get_root( void ) const           // Get root Node
{  return const_cast<Node*>(root); }

stream_ptr                          // The associated Stream
   get_stream(stream_id) const;     // Locate the Stream given Stream::ident

//----------------------------------------------------------------------------
// StreamSet::Lockable
//----------------------------------------------------------------------------
void
   lock( void ) const               // Obtain the StreamSet lock
{  mutex.lock(); }                  // (mutex is mutable)

void
   unlock( void ) const             // Release the StreamSet lock
{  mutex.unlock(); }                // (mutex is mutable)

//----------------------------------------------------------------------------
// StreamSet::methods
//----------------------------------------------------------------------------
void
   change(                          // Change a Stream's parent
     Stream*           parent,      // The new parent Stream
     Stream*           child);      // The child Stream to move

void
   insert(                          // Insert Stream
     Stream*           parent,      // The parent Stream
     Stream*           child);      // The child Stream to insert

void
   remove(                          // Remove Stream
     Stream*           stream);     // The Stream to remove
}; // class StreamSet
}  // namespace http
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_HTTP_STREAMSET_H_INCLUDED
