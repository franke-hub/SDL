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
//       NetRoot.h
//
// Purpose-
//       Define the root Network Layer: Drives threads.
//
// Last change date-
//       2018/01/01
//
// Implementation notes-
//       TODO: Divide work by work_charge rather than by Token count.
//
//----------------------------------------------------------------------------
#ifndef NETROOT_H_INCLUDED
#define NETROOT_H_INCLUDED

#include <pthread.h>
#include <unistd.h>

#include "Network.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef NETROOT_H_THREAD_STACK_SIZE
#define NETROOT_H_THREAD_STACK_SIZE 0 // (0) For system default
#endif

#ifndef USE_THREADING_MODEL
#define USE_THREADING_MODEL false
#endif

namespace NETWORK_H_NAMESPACE {
//----------------------------------------------------------------------------
//
// Class-
//       Thread
//
// Purpose-
//       The Network thread driver class.
//
//----------------------------------------------------------------------------
class Thread : public Network {     // A Network Thread
//----------------------------------------------------------------------------
// Thread::Attributes
//----------------------------------------------------------------------------
public:
Layer&                 root;        // The Root Layer, our constructor
pthread_t              thread_id;   // Our thread identifier

//----------------------------------------------------------------------------
// Thread::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Thread()                        // Virtual destructor
{  IFDEBUG( debugf("Thread(%p).~Thread\n", this); ) }

   Thread(                          // Data constructor
     Layer&            root)        // The Root Layer
:  Network(0)
,  root(root), thread_id(0)
{  IFDEBUG( debugf("Thread(%p).Thread\n", this); ) }

   Thread( void ) = delete;         // NO Default Constructor
   Thread(const Thread&) = delete;  // Disallowed copy constructor
   Thread& operator=(const Thread&) = delete; // Disallowed assignment operator

//----------------------------------------------------------------------------
// Thread::Methods
//----------------------------------------------------------------------------
protected:
static inline void*                 // Resultant
   driver(                          // Operate
     void*             thread)      // This thread
{  IFDEBUG( debugf("Thread(%p).driver\n", thread); )

   try {
     ((Thread*)thread)->run();
   } catch(NetworkException& X) {
     debugf("Thread.run %s(%s)\n", X.get_class_name().c_str(), X.what());
   } catch(std::exception& X) {
     debugf("Thread.run std::exception.what(%s)\n", X.what());
   } catch(...) {
     debugf("Thread.run catch(...)\n");
   }

   IFDEBUG( debugf("Thread(%p).*DONE*\n", thread); )
   return nullptr;
}

public:
inline void                          // Wait for this thread to complete
   join( void )                      // Wait for completion
{  IFDEBUG( debugf("Thread(%p).join\n", this); )

#if false // timedjoin not available on Cygwin
   Buffer              buffer;       // to_thread buffer
   timespec            ts= {1, 25000000}; // 1.25 second wait
   int                 rc;
   do {
     rc= pthread_timedjoin_np(thread_id, NULL, ts);
     if( rc )
       debugf("%s still running\n", to_buffer(buffer));
   } while(rc);
#else
   int rc= pthread_join(thread_id, NULL);
   if( rc != 0 ) assert( false );
#endif

   thread_id= 0;
}

inline void
   run( void )                      // Operate
{  IFDEBUG( debugf("Thread(%p).run\n", this); )

   root.fanout(_origin, _length);
}

inline void
   start(                           // Start the Thread
     Token             origin,      // Using this origin
     Count             count)       // And this count
{  IFDEBUG( debugf("Thread(%p).start(%lx,%lx)\n", this, origin, count); )
   assert( thread_id == 0 );        // Must not be running

   _origin= origin;                 // Set the runtime attributes
   _length= count;
   _ending= origin + count;

   // Create a pthread to actually drive the Thread
   pthread_attr_t      attr;
   int rc= pthread_attr_init(&attr);
   assert( rc == 0 );
   int detach_state= PTHREAD_CREATE_JOINABLE;
   pthread_attr_setdetachstate(&attr, detach_state);
   pthread_attr_setstacksize(&attr, NETROOT_H_THREAD_STACK_SIZE);
   rc= pthread_create(&thread_id, &attr, driver, (void*)this);
   assert( rc == 0 );
   pthread_attr_destroy(&attr);
}
}; // class Thread

//----------------------------------------------------------------------------
//
// Class-
//       Root
//
// Purpose-
//       The Root Layer.
//
//----------------------------------------------------------------------------
class Root : public Layer  {       // Root Neuron
//----------------------------------------------------------------------------
// Root::Attributes
//----------------------------------------------------------------------------
public:
unsigned               thread_count; // Our Thread count
Thread**               thread_array; // Our Thread array

//----------------------------------------------------------------------------
// Root::Destructor/Constructor
//----------------------------------------------------------------------------
public:
   ~Root( void )
{  IFDEBUG( debugf("Root(%p).~Root\n", this); )

   for(unsigned i= 0; i<thread_count; i++)
     delete thread_array[i];

   delete thread_array;
}

   Root(
     unsigned          thread_count) // The number of Threads
:  Layer(*this)
,  thread_count(thread_count), thread_array(nullptr)
{  IFDEBUG( debugf("Root(%p).Root\n", this); )

   thread_array= new Thread*[thread_count];
   for(unsigned i= 0; i<thread_count; i++)
     thread_array[i]= new Thread(*this);
}

//----------------------------------------------------------------------------
// Root::Build methods
//----------------------------------------------------------------------------
virtual Network*                    // The associated Network
   build_locate(                    // Locate the Network
     Token             token) const // For this token
{  IFDEBUG( if( verbose() > 0 ) {
     Buffer buffer;
     debugf("%s.build_locate(%lx)\n", to_buffer(buffer), token);
   } ) // IFDEBUG( if ...

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

   Buffer buffer;
   debugf("%s *ERROR*\n>> build_locate(%lx)\n", to_buffer(buffer), token);
   throw LocateException("Root");
   return (Network*)this;           // Avoid compiler warnings
}

//----------------------------------------------------------------------------
// Root::Methods
//----------------------------------------------------------------------------
virtual Network*                    // The associated Network
   locate(                          // Locate the Network
     Token             token) const // For this token
{   for(size_t i= 0; i<layers_used; i++)
     if( layer_array[i]->contains(token) )
       return layer_array[i]->locate(token);

   Buffer buffer;
   debugf("%s *ERROR*\n>> locate(%lx)\n", to_buffer(buffer), token);
   throw LocateException("Root");
   return (Network*)this;           // Avoid compiler warnings
}

virtual void
   update( void )                   // Process new thread cycle
{  Layer::update();                 // Prepare for new clock cycle

#if USE_THREADING_MODEL
   // Start the fanout threads
   Count count= length();           // Number of Networks
   Count per_thread= count / thread_count;
   if( per_thread == 0 )
     throw ShouldNotOccur("More Threads than Networks");

   Token token= origin();
   for(int i= 0; i<thread_count; i++) {
     if( i == (thread_count-1) )    // The last thread can get extra
       per_thread= count;

     debugf("STARTING: [%lx:%.lx].%lx\n", token, token+per_thread, per_thread);
     debugf("STARTING THREAD>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
     thread_array[i]->start(token, per_thread);
     debugf("STARTED  THREAD<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
     token += per_thread;
     count -= per_thread;
   }
   assert( count == 0 );            // Internal consistency check
   assert( token == length() );     // Internal consistency check

   // Wait for all threads to complete, then exit
   for(int i= 0; i<thread_count; i++)
     thread_array[i]->join();
#else
   // Single-threaded model, mostly for debugging
   fanout(_origin, _length);
#endif
}
}; // class Root
}  // namespace NETWORK_H_NAMESPACE

#endif // NETROOT_H_INCLUDED
