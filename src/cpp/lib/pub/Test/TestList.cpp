//----------------------------------------------------------------------------
//
//       Copyright (C) 2007-2023 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       TestList.cpp
//
// Purpose-
//       List tests.
//
// Last change date-
//       2023/09/21
//
//----------------------------------------------------------------------------
#include <new>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <pub/Debug.h>              // For namespace pub::debugging
#include "pub/List.h"               // For pub::List, tested

#include "pub/TEST.H"               // For VERIFY, ...
#include "pub/Wrapper.h"            // For class Wrapper

#define PUB _LIBPUB_NAMESPACE
using namespace PUB::debugging;     // For debugf, ...
using PUB::AI_list;
using PUB::DHDL_list;
using PUB::DHSL_list;
using PUB::SHSL_list;
using PUB::List;
using PUB::Wrapper;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, greater is more verbose

,  DIM= 12, MID= DIM/2              // Array size. Use: 9 < DIM < 100
};
#define USE_ERROR_CHECK false       // Run type checking? (Cause compile errors)
#define USE_BEGIN_END   true        // Use begin()/end() logic?

//----------------------------------------------------------------------------
//
// Class-
//       Prefix
//
// Purpose-
//       Self-validating class.
//
//----------------------------------------------------------------------------
class Prefix {                      // Self-validating class
private:
enum                                // Generic enumerations
{  VALIDATOR=            0x02469773 // Validation constant
}; // enum

public:
   ~Prefix( void )                  // Destructor
{  assert(isValid()); }

   Prefix( void )                   // Constructor
:  word(VALIDATOR)
{  }

int                                 // TRUE if valid
   isValid( void )
{  return (word == VALIDATOR); }

private:
   long                word;        // Validiation word
}; // class Prefix

//----------------------------------------------------------------------------
//
// Class-
//       Suffix
//
// Purpose-
//       Self-validating class.
//
//----------------------------------------------------------------------------
class Suffix {                      // Self-validating class
private:
enum                                // Generic enumerations
{  VALIDATOR=            0x37796420 // Validation constant
}; // enum

public:
   ~Suffix( void )                  // Destructor
{  assert(isValid()); }

   Suffix( void )                   // Constructor
:  word(VALIDATOR)
{  }

int                                 // TRUE if valid
   isValid( void )
{  return (word == VALIDATOR); }

private:
   long                word;        // Validiation word
}; // class Suffix

//----------------------------------------------------------------------------
//
// Class-
//       Vclass
//
// Purpose-
//       A class with  virtual methods.
//
//----------------------------------------------------------------------------
struct Vclass {                     // Class with virtual functions
int                    some_data= -1; // (and some unused data)

virtual
~Vclass() = default;
Vclass() = default;

virtual void debug(const char* info)
{  debugf("Vclass(%p)::info(%s) %d\n", this, info, some_data); }
}; // struct Vclass

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_AI
//
// Purpose-
//       Test List.h, AI_link
//
//----------------------------------------------------------------------------
#if true // Working version 2, complex, Link isn't first base class
// This replicates a problem environment found in testing
struct AI_block : public Vclass
,  public Prefix, public AI_list<AI_block>::Link, public Suffix {
int                    index;

AI_block() : Vclass(), Prefix(), AI_list<AI_block>::Link(), Suffix()
{  /* debugf("AI_block(%p)::AI_block\n", this); */ }

#elif true // Working version 1, simple, Link is first base class
struct AI_block : public AI_list<AI_block>::Link, public Vclass {
Prefix                 prefix;
int                    index;
Suffix                 suffix;

AI_block() : AI_list<AI_block>::Link()
{  /* debugf("AI_block(%p)::AI_block\n", this); */ }

#endif

virtual ~AI_block()
{  /* debugf("AI_block(%p)::~AI_block\n", this); */ }

virtual void debug(const char* info)
{  debugf("AI_block(%p)::info(%s) %d\n", this, info, index); }
}; // class AI_block

static void
   show_AI(                         // Display an AI_list
     AI_list<AI_block>*
                       anchor)      // The list anchor
{
   if( opt_verbose ) {
     debugf("List:");
     AI_block* link= anchor->get_tail();
     while( link != nullptr ) {
       debugf(" %2d", link->index);
       link= link->get_prev();
     }

     debugf("\n");
   }
}

//----------------------------------------------------------------------------
// AI tests
//----------------------------------------------------------------------------
static int
   test_AI(void)                    // Test AI_link
{
   int error_count= 0;

   AI_block            ai_data[DIM];
   AI_list<AI_block>   ai_list;
   AI_block*           ai_link;

   if( opt_verbose > 1 ) {
     debugf("\n");
     debugf("AI Storage:\n");
     debugf("%8zd Sizeof(AI_list)\n", sizeof(AI_list<AI_block>));
     debugf("%8zd Sizeof(AI_link)\n", sizeof(AI_list<AI_block>::Link));

     debugf("\n");
     debugf("Empty AI_list:\n");
     show_AI(&ai_list);
   }

   //-------------------------------------------------------------------------
   // AI FIFO/REMQ test
   //-------------------------------------------------------------------------
   if( opt_verbose ) {
     debugf("\n");
     debugf("AI_list fifo() test:\n");
   }
   for(int i=0; i<DIM; i++) {
     ai_data[i].index= i + 1;
     ai_list.fifo(&ai_data[i]);
     show_AI(&ai_list);
   }
   for(int i=0; i<DIM; i++)
     error_count += VERIFY( ai_list.is_on_list(&ai_data[i]) );
   error_count += VERIFY( ai_list.is_coherent() );

   ai_list.reset(nullptr);

   //-------------------------------------------------------------------------
   // AI iterator test
   //-------------------------------------------------------------------------
   if( opt_verbose ) {
     debugf("\n");
     debugf("AI_iter test:\n");
   }
   for(int i=0; i<MID; i++) {
     ai_list.fifo(&ai_data[i]);
     show_AI(&ai_list);
   }

   if( opt_verbose )
     debugf("Iter:");
   int index= 0;
   for(auto it= ai_list.begin(); it; ++it) {
     // Add to the list while iterating, testing _AI_list_iterator::_next
     if( MID + index < DIM ) {
       ai_list.fifo(&ai_data[MID + index]);
       error_count += VERIFY( ai_list.is_on_list(&ai_data[MID + index]) );
     }
     error_count += VERIFY( !ai_list.is_on_list(&ai_data[index]) );

     // Verify and display the index
     ai_link= it.get();
     error_count += VERIFY( ai_link->index == it->index);
     error_count += VERIFY( it->index == (index+1) );
     if( opt_verbose ) {
       if( ai_link->index == (index+1) )
         debugf(" %2d", it->index);
       else
         debugf(" %2d!=%2d\n", it->index, index);
     }
     ++index;
   }
   if( opt_verbose )
     debugf("\n");
   assert(index == DIM);

   for(int i=0; i<DIM; i++)
     error_count += VERIFY( !ai_list.is_on_list(&ai_data[i]) );
   error_count += VERIFY( ai_list.is_coherent());
   error_count += VERIFY( ai_list.is_empty() );
   return error_count;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_DHDL
//
// Purpose-
//       Test List.h, DHDL_list (a.k.a. List)
//
//----------------------------------------------------------------------------
#if true // Working version 1, complex, and Link is not first base class
struct DHDL_block :  public Vclass
,  public Prefix, public DHDL_list<DHDL_block>::Link, public Suffix {
int                    index;
}; // class DHDL_block

#elif true // Working version 2, simple, but Link is not first base class
struct DHDL_block
:  public Prefix, public DHDL_list<DHDL_block>::Link, public Suffix {
int                    index;
}; // class DHDL_block
#endif

static void
   show_DHDL(                       // Display a DHDL_list
     DHDL_list<DHDL_block>*
                       anchor)      // The list anchor
{
   if( opt_verbose ) {
     debugf("List:");
#if USE_BEGIN_END
     for(auto it= anchor->begin(); it != anchor->end(); ++it) {
       debugf(" %2d", it->index);
     }
#else
     DHDL_block* link= anchor->get_head(); // Get head element
     while( link != nullptr ) {
       debugf(" %2d", link->index);
       link= link->get_next();
     }
#endif

     debugf("\n");
   }
}

static void
   show_DHDL(                       // Display a DHDL_list, removed element
     DHDL_list<DHDL_block>*
                       anchor,      // The list anchor
     DHDL_block*       removed)     // The removed link
{
   if( opt_verbose ) {
     debugf("List:");
#if USE_BEGIN_END
     for(auto it= anchor->begin(); it != anchor->end(); ++it) {
       debugf(" %2d", it->index);
     }
#else
     DHDL_block* link= anchor->get_head(); // Get head element
     while( link != nullptr ) {
       debugf(" %2d", link->index);
       link= link->get_next();
     }
#endif

     debugf(" --(%2d)", removed->index);
     debugf("\n");
   }
}

//----------------------------------------------------------------------------
// DHDL tests
//----------------------------------------------------------------------------
static int
   test_DHDL(void)                  // Test DHDL_list
{
   int error_count= 0;

   DHDL_block                  dhdl_data[DIM];
   DHDL_list<DHDL_block>       dhdl_list;
   DHDL_block*                 dhdl_link;

   if( opt_verbose > 1 ) {
     debugf("\n");
     debugf("DHDL Storage:\n");
     debugf("%8zd Sizeof(List)\n", sizeof(List<DHDL_block>));
     debugf("%8zd Sizeof(Link)\n", sizeof(List<DHDL_block>::Link));

     debugf("\n");
     debugf("Empty DHDL_list:\n");
     show_DHDL(&dhdl_list);
   }

   //-------------------------------------------------------------------------
   // DHDL LIFO test
   //-------------------------------------------------------------------------
   if( opt_verbose ) {
     debugf("\n");
     debugf("DHDL_LIFO test (1..%d):\n", DIM);
   }
   for(int i=0; i<DIM; i++) {
     dhdl_data[i].index= i + 1;
     dhdl_list.lifo(&dhdl_data[i]);
     show_DHDL(&dhdl_list);
   }
   for(int i=0; i<DIM; i++)
     error_count += VERIFY( dhdl_list.is_on_list(&dhdl_data[i]) );
   error_count += VERIFY( dhdl_list.is_coherent() );

   for(;;) {
     dhdl_link= dhdl_list.remq();
     if( dhdl_link == nullptr )
       break;
     show_DHDL(&dhdl_list, dhdl_link);
   }
   for(int i=0; i<DIM; i++)
     error_count += VERIFY( !dhdl_list.is_on_list(&dhdl_data[i]) );
   error_count += VERIFY( dhdl_list.is_coherent() );

   //-------------------------------------------------------------------------
   // DHDL FIFO test
   //-------------------------------------------------------------------------
   if( opt_verbose ) {
     debugf("\n");
     debugf("DHDL_FIFO test:\n");
   }
   for(int i=0; i<DIM; i++) {
     dhdl_list.fifo(&dhdl_data[i]);
     show_DHDL(&dhdl_list);
   }
   for(int i=0; i<DIM; i++)
     error_count += VERIFY( dhdl_list.is_on_list(&dhdl_data[i]) );
   error_count += VERIFY( dhdl_list.is_coherent() );

   for(;;) {
     dhdl_link= dhdl_list.remq();
     if( dhdl_link == nullptr )
       break;
     show_DHDL(&dhdl_list, dhdl_link);
   }
   for(int i=0; i<DIM; i++)
     error_count += VERIFY( !dhdl_list.is_on_list(&dhdl_data[i]) );
   error_count += VERIFY( dhdl_list.is_coherent());

   //-------------------------------------------------------------------------
   // DHDL_iter test
   //-------------------------------------------------------------------------
   if( opt_verbose ) {
     debugf("\n");
     debugf("DHDL_iter test:\nIter:");
   }
   for(int i=0; i<DIM; i++) {
     dhdl_list.fifo(&dhdl_data[i]);
   }
   int ix= 1;
   for(auto it= dhdl_list.begin(); it != dhdl_list.end(); ++it) {
     if( opt_verbose )
       debugf(" %2d", it->index);

     dhdl_link= it.get();
     error_count += VERIFY( it->index == ix );
     error_count += VERIFY( dhdl_link->index == ix );
     ++ix;
   }
   if( opt_verbose )
     debugf("\n");

   for(int i=0; i<DIM; i++)
     error_count += VERIFY( dhdl_list.is_on_list(&dhdl_data[i]) );
   error_count += VERIFY( dhdl_list.is_coherent());
   dhdl_list.reset();

   //-------------------------------------------------------------------------
   // DHDL remove/insert specific
   //-------------------------------------------------------------------------
   if( opt_verbose ) {
     debugf("\n");
     debugf("DHDL_REMOVE(position) test:\n");
   }
   for(int i=0; i<DIM; i++)
     dhdl_list.fifo(&dhdl_data[i]);
   show_DHDL(&dhdl_list);

   if( opt_verbose ) {
     debugf("\n");
     debugf("DHDL_REMOVE(1) test:\n");
   }
   dhdl_link= &dhdl_data[1-1];
   dhdl_list.remove(dhdl_link, dhdl_link);
   show_DHDL(&dhdl_list, dhdl_link);
   error_count += VERIFY( !dhdl_list.is_on_list(&dhdl_data[1-1]) );
   error_count += VERIFY( dhdl_list.is_coherent() );

   if( opt_verbose ) {
     debugf("\n");
     debugf("DHDL_REMOVE(5) test:\n");
   }
   dhdl_link= &dhdl_data[5-1];
   dhdl_list.remove(dhdl_link, dhdl_link);
   show_DHDL(&dhdl_list, dhdl_link);
   error_count += VERIFY( !dhdl_list.is_on_list(&dhdl_data[5-1]) );
   error_count += VERIFY( dhdl_list.is_coherent() );

   if( opt_verbose ) {
     debugf("\n");
     debugf("DHDL_REMOVE(%d) test:\n", DIM);
   }
   dhdl_link= &dhdl_data[DIM-1];
   dhdl_list.remove(dhdl_link, dhdl_link);
   show_DHDL(&dhdl_list, dhdl_link);
   error_count += VERIFY( !dhdl_list.is_on_list(&dhdl_data[DIM-1]) );
   error_count += VERIFY( dhdl_list.is_coherent() );

   if( opt_verbose ) {
     debugf("\n");
     debugf("DHDL_INSERT(1) at head:\n");
   }
   dhdl_list.insert(nullptr, &dhdl_data[1-1], &dhdl_data[1-1]);
   show_DHDL(&dhdl_list);
   error_count += VERIFY( dhdl_list.is_on_list(&dhdl_data[1-1]) );
   error_count += VERIFY( dhdl_list.is_coherent() );

   if( opt_verbose ) {
     debugf("\n");
     debugf("DHDL_INSERT(%d) at tail:\n", DIM);
   }
   dhdl_list.insert(dhdl_list.get_tail(), &dhdl_data[DIM-1], &dhdl_data[DIM-1]);
   show_DHDL(&dhdl_list);
   error_count += VERIFY( dhdl_list.is_on_list(&dhdl_data[DIM-1]) );
   error_count += VERIFY( dhdl_list.is_coherent() );

   if( opt_verbose ) {
     debugf("\n");
     debugf("DHDL_INSERT(5) after(4):\n");
   }
   dhdl_list.insert(&dhdl_data[4-1], &dhdl_data[5-1], &dhdl_data[5-1]);
   show_DHDL(&dhdl_list);
   error_count += VERIFY( dhdl_list.is_on_list(&dhdl_data[5-1]) );
   error_count += VERIFY( dhdl_list.is_coherent() );

   if( opt_verbose ) {
     debugf("\n");
     debugf("DHDL_REMOVE(5..8):\n");
   }
   dhdl_list.remove(&dhdl_data[5-1], &dhdl_data[8-1]);
   show_DHDL(&dhdl_list);
   error_count += VERIFY(  dhdl_list.is_on_list(&dhdl_data[4-1]) );
   error_count += VERIFY( !dhdl_list.is_on_list(&dhdl_data[5-1]) );
   error_count += VERIFY( !dhdl_list.is_on_list(&dhdl_data[6-1]) );
   error_count += VERIFY( !dhdl_list.is_on_list(&dhdl_data[7-1]) );
   error_count += VERIFY( !dhdl_list.is_on_list(&dhdl_data[8-1]) );
   error_count += VERIFY(  dhdl_list.is_on_list(&dhdl_data[9-1]) );
   error_count += VERIFY(  dhdl_list.is_coherent() );

   if( opt_verbose ) {
     debugf("\n");
     debugf("DHDL_INSERT(5..8):\n");
   }
   dhdl_list.insert(&dhdl_data[4-1], &dhdl_data[5-1], &dhdl_data[8-1]);
   show_DHDL(&dhdl_list);
   error_count += VERIFY( dhdl_list.is_on_list(&dhdl_data[4-1]) );
   error_count += VERIFY( dhdl_list.is_on_list(&dhdl_data[5-1]) );
   error_count += VERIFY( dhdl_list.is_on_list(&dhdl_data[6-1]) );
   error_count += VERIFY( dhdl_list.is_on_list(&dhdl_data[7-1]) );
   error_count += VERIFY( dhdl_list.is_on_list(&dhdl_data[8-1]) );
   error_count += VERIFY( dhdl_list.is_on_list(&dhdl_data[9-1]) );
   error_count += VERIFY( dhdl_list.is_coherent() );

   if( opt_verbose ) {
     debugf("\n");
     debugf("DHDL_REMOVE(1..%d):\n", DIM);
   }
   dhdl_list.remove(&dhdl_data[1-1], &dhdl_data[DIM-1]);
   show_DHDL(&dhdl_list);
   for(int i=0; i<DIM; i++)
     error_count += VERIFY( !dhdl_list.is_on_list(&dhdl_data[i]) );
   error_count += VERIFY( dhdl_list.is_coherent() );

   if( opt_verbose ) {
     debugf("\n");
     debugf("DHDL_INSERT(1..%d):\n", DIM);
   }
   dhdl_list.insert(nullptr, &dhdl_data[1-1], &dhdl_data[DIM-1]);
   show_DHDL(&dhdl_list);
   for(int i=0; i<DIM; i++)
     error_count += VERIFY( dhdl_list.is_on_list(&dhdl_data[i]) );
   error_count += VERIFY( dhdl_list.is_coherent() );
   dhdl_list.reset();

   return error_count;
} // static int test_DHDL

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_DHSL
//
// Purpose-
//       Test List.h, DHSL_list
//
//----------------------------------------------------------------------------
struct DHSL_block
:  public Prefix, public DHSL_list<DHSL_block>::Link, public Suffix {
int                    index;
}; // class DHSL_block

static void
   show_DHSL(                       // Display a list
     DHSL_list<DHSL_block>*
                       anchor)      // The list anchor
{
   if( opt_verbose ) {
     debugf("List:");
     DHSL_block* link= anchor->get_head(); // Get head element
     while( link != nullptr ) {
       debugf(" %2d", link->index);
       link= link->get_next();
     }

     debugf("\n");
   }
}

static void
   show_DHSL(                       // Display a list
     DHSL_list<DHSL_block>*
                       anchor,      // The list anchor
     DHSL_block*       removed)     // The removed link
{
   if( opt_verbose ) {
     debugf("List:");
     DHSL_block* link= anchor->get_head(); // Get head element
     while( link != nullptr ) {
       debugf(" %2d", link->index);
       link= link->get_next();
     }

     debugf(" --(%2d)", removed->index);
     debugf("\n");
   }
}

//----------------------------------------------------------------------------
// DHSL tests
//----------------------------------------------------------------------------
static int
   test_DHSL(void)                  // Test DHSL_list
{
   int error_count= 0;

   DHSL_block                  dhsl_data[DIM];
   DHSL_list<DHSL_block>       dhsl_list;
   DHSL_block*                 dhsl_link;

   if( opt_verbose > 1 ) {
     debugf("\n");
     debugf("DHSL Storage:\n");
     debugf("%8zd Sizeof(DHSL_list)\n", sizeof(DHSL_list<DHSL_block>));
     debugf("%8zd Sizeof(DHSL_link)\n", sizeof(DHSL_list<DHSL_block>::Link));

     debugf("\n");
     debugf("Empty DHSL_list:\n");
     show_DHSL(&dhsl_list);
   }

   //-------------------------------------------------------------------------
   // DHSL LIFO test
   //-------------------------------------------------------------------------
   if( opt_verbose ) {
     debugf("\n");
     debugf("DHSL_LIFO test (1..%d):\n", DIM);
   }
   for(int i=0; i<DIM; i++) {
     dhsl_data[i].index= i + 1;
     dhsl_list.lifo(&dhsl_data[i]);
     show_DHSL(&dhsl_list);
   }
   for(int i=0; i<DIM; i++)
     error_count += VERIFY( dhsl_list.is_on_list(&dhsl_data[i]) );
   error_count += VERIFY( dhsl_list.is_coherent() );

   for(;;) {
     dhsl_link= dhsl_list.remq();
     if( dhsl_link == nullptr )
       break;
     show_DHSL(&dhsl_list, dhsl_link);
   }
   for(int i=0; i<DIM; i++)
     error_count += VERIFY( !dhsl_list.is_on_list(&dhsl_data[i]) );
   error_count += VERIFY( dhsl_list.is_coherent() );

   //-------------------------------------------------------------------------
   // DHSL FIFO test
   //-------------------------------------------------------------------------
   if( opt_verbose ) {
     debugf("\n");
     debugf("DHSL_FIFO test:\n");
   }
   for(int i=0; i<DIM; i++) {
     dhsl_list.fifo(&dhsl_data[i]);
     show_DHSL(&dhsl_list);
   }
   for(int i=0; i<DIM; i++)
     error_count += VERIFY( dhsl_list.is_on_list(&dhsl_data[i]) );
   error_count += VERIFY( dhsl_list.is_coherent() );

   for(;;) {
     dhsl_link= dhsl_list.remq();
     if( dhsl_link == nullptr )
       break;
     show_DHSL(&dhsl_list, dhsl_link);
   }
   for(int i=0; i<DIM; i++)
     error_count += VERIFY( !dhsl_list.is_on_list(&dhsl_data[i]) );
   error_count += VERIFY( dhsl_list.is_coherent() );

   //-------------------------------------------------------------------------
   // DHSL iterator test
   //-------------------------------------------------------------------------
   if( opt_verbose ) {
     debugf("\n");
     debugf("DHSL_iter test:\nIter:");
   }
   for(int i=0; i<DIM; i++) {
     dhsl_list.fifo(&dhsl_data[i]);
   }
   for(int i=0; i<DIM; i++)
     error_count += VERIFY( dhsl_list.is_on_list(&dhsl_data[i]) );
   error_count += VERIFY( dhsl_list.is_coherent() );

   int ix= 0;
   for(auto it= dhsl_list.begin(); it != dhsl_list.end(); ++it) {
     if( opt_verbose )
       debugf(" %2d", it->index);

     dhsl_link= it.get();
     error_count += VERIFY( dhsl_link->index == (ix + 1) );
     error_count += VERIFY( it->index == (ix + 1) );
     ++ix;
   }
   if( opt_verbose )
     debugf("\n");

   for(int i=0; i<DIM; i++)
     error_count += VERIFY( dhsl_list.is_on_list(&dhsl_data[i]) );
   error_count += VERIFY( dhsl_list.is_coherent() );

   dhsl_list.reset();

   return error_count;
} // static int test_DHSL

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_SHSL
//
// Purpose-
//       Test List.h, SHSL_list
//
//----------------------------------------------------------------------------
struct SHSL_block : public Vclass
,  public Prefix, public SHSL_list<SHSL_block>::Link, public Suffix {
int                    index;
}; // class SHSL_block

static void
   show_SHSL(                       // Display a list
     SHSL_list<SHSL_block>*
                       anchor)      // The list anchor
{
   if( opt_verbose ) {
     debugf("List:");
#if USE_BEGIN_END
     for(auto it= anchor->begin(); it != anchor->end(); ++it) {
       debugf(" %2d", it->index);
     }
#else
     SHSL_block* link= anchor->get_tail(); // Get tail element
     while( link != nullptr ) {
       debugf(" %2d", link->index);
       link= link->get_prev();
     }
#endif

     debugf("\n");
   }
}

static void
   show_SHSL(                       // Display a list
     SHSL_list<SHSL_block>*
                       anchor,      // The list anchor
     SHSL_block*       removed)     // The removed link
{
   if( opt_verbose ) {
     debugf("List:");
#if USE_BEGIN_END
     for(auto it= anchor->begin(); it != anchor->end(); ++it) {
       debugf(" %2d", it->index);
     }
#else
     SHSL_block* link= anchor->get_tail(); // Get tail element
     while( link != nullptr ) {
       debugf(" %2d", link->index);
       link= link->get_prev();
     }
#endif

     debugf(" --(%2d)", removed->index);
     debugf("\n");
   }
}

//----------------------------------------------------------------------------
// SHSL tests
//----------------------------------------------------------------------------
static int
   test_SHSL(void)                  // Test SHSL_list
{
   int error_count= 0;

   SHSL_block                  shsl_data[DIM];
   SHSL_list<SHSL_block>       shsl_list;
   SHSL_block*                 shsl_link;

   if( opt_verbose > 1 ) {
     debugf("\n");
     debugf("SHSL Storage:\n");
     debugf("%8zd Sizeof(SHSL_list)\n", sizeof(SHSL_list<SHSL_block>));
     debugf("%8zd Sizeof(SHSL_link)\n", sizeof(SHSL_list<SHSL_block>::Link));

     debugf("\n");
     debugf("Empty SHSL_list:\n");
     show_SHSL(&shsl_list);
   }

   //-------------------------------------------------------------------------
   // SHSL LIFO test
   //-------------------------------------------------------------------------
   if( opt_verbose ) {
     debugf("\n");
     debugf("SHSL_LIFO test (%d..1):\n", DIM);
   }
   for(int i=0; i<DIM; i++) {
     shsl_data[i].index= i + 1;
     shsl_list.lifo(&shsl_data[i]);
     show_SHSL(&shsl_list);
   }
   for(int i=0; i<DIM; i++)
     error_count += VERIFY( shsl_list.is_on_list(&shsl_data[i]) );
   error_count += VERIFY( shsl_list.is_coherent() );

   for(;;) {
     shsl_link= shsl_list.remq();
     if( shsl_link == nullptr )
       break;
     show_SHSL(&shsl_list, shsl_link);
   }
   for(int i=0; i<DIM; i++)
     error_count += VERIFY( !shsl_list.is_on_list(&shsl_data[i]) );
   error_count += VERIFY( shsl_list.is_coherent() );

   //-------------------------------------------------------------------------
   // SHSL ITER test
   //-------------------------------------------------------------------------
   if( opt_verbose ) {
     debugf("\n");
     debugf("SHSL_iter test:\nIter:");
   }
   for(int i=0; i<DIM; i++) {
     shsl_data[i].index= i + 1;
     shsl_list.lifo(&shsl_data[i]);
   }
   for(int i=0; i<DIM; i++)
     error_count += VERIFY( shsl_list.is_on_list(&shsl_data[i]) );
   error_count += VERIFY( shsl_list.is_coherent() );

   int ix= 0;
   for(auto it= shsl_list.begin(); it != shsl_list.end(); ++it) {
     if( opt_verbose )
       debugf(" %2d", it->index);

     shsl_link= it.get();
     error_count += VERIFY( shsl_link->index == (DIM - ix) );
     error_count += VERIFY( it->index == (DIM - ix) );
     ++ix;
   }
   if( opt_verbose )
     debugf("\n");
   for(int i=0; i<DIM; i++)
     error_count += VERIFY( shsl_list.is_on_list(&shsl_data[i]) );
   error_count += VERIFY( shsl_list.is_coherent() );
   shsl_list.reset();

   return error_count;
} // static int test_SHSL

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_SORT
//
// Purpose-
//       Test List.h, List::sort
//
//----------------------------------------------------------------------------
struct SORT_block
:  public Prefix, public List<SORT_block>::Link, public Suffix {
typedef SORT_block     _Self;

int                    index;
}; // class SORT_block

static void
   show_SORT(                       // Display a list
     List<SORT_block>* anchor)      // The list anchor
{
   if( opt_verbose ) {
     debugf("List:");
#if USE_BEGIN_END
     for(auto it= anchor->begin(); it != anchor->end(); ++it) {
       debugf(" %2d", it->index);
     }
#else
     SORT_block* link= anchor->get_head(); // Get head element
     while( link != nullptr ) {
       debugf(" %2d", link->index);
       link= link->get_next();
     }
#endif

     debugf("\n");
   }
}

//----------------------------------------------------------------------------
// SORT tests
//----------------------------------------------------------------------------
static int
   test_SORT(void)                  // Test List::sort
{
   int error_count= 0;

   SORT_block          sort_data[DIM];
   List<SORT_block>    sort_list;

   if( opt_verbose ) {
     debugf("\n");
     debugf("SORT Storage:\n");
     debugf("%8zd Sizeof(SORT_list)\n", sizeof(List<SORT_block>));
     debugf("%8zd Sizeof(SORT_link)\n", sizeof(List<SORT_block>::Link));
   }

   typedef const SORT_block         _Link;
#if USE_BASE_SORT
   typedef const List<void>::_Link  _Void;
   static const struct {
     bool operator()(_Void* lhs, _Void* rhs) const
     { return ((_Link*)lhs)->index < ((_Link*)rhs)->index; }
   } cmp;
#else
   static const struct {
     bool operator()(_Link* lhs, _Link* rhs) const
     { return lhs->index < rhs->index; }
   } cmp;
#endif

   //-------------------------------------------------------------------------
   // SORT Lambda test
   //-------------------------------------------------------------------------
   if( opt_verbose ) {
     debugf("\n");
     debugf("SORT lambda test:\n");
   }
   for(int i=0; i<DIM; i++) {
     sort_data[i].index= i + 1;
     sort_list.fifo(&sort_data[DIM - i - 1]);
   }
   show_SORT(&sort_list);

#if USE_BASE_SORT
   sort_list.sort([](_Void* lhs, _Void* rhs)
     { return ((_Link*)lhs)->index < ((_Link*)rhs)->index; }
   );
#else
   sort_list.sort([](_Link* lhs, _Link* rhs)
     { return lhs->index < rhs->index; }
   );
#endif

   show_SORT(&sort_list);

   int
   index= 1;
   for(auto it= sort_list.begin(); it != sort_list.end(); ++it) {
     error_count += VERIFY( index == it->index );
     index++;
   }

   for(int i=0; i<DIM; i++)
     error_count += VERIFY( sort_list.is_on_list(&sort_data[i]) );
   error_count += VERIFY( sort_list.is_coherent() );
   sort_list.reset();

   //-------------------------------------------------------------------------
   // SORT Struct test
   //-------------------------------------------------------------------------
   if( opt_verbose ) {
     debugf("\n");
     debugf("SORT struct test:\n");
   }
   for(int i=0; i<DIM; i++)
     sort_list.fifo(&sort_data[DIM - i - 1]);
   show_SORT(&sort_list);

   sort_list.sort(cmp);

   show_SORT(&sort_list);

   index= 1;
   for(auto it= sort_list.begin(); it != sort_list.end(); ++it) {
     error_count += VERIFY( index == it->index );
     index++;
   }

   for(int i=0; i<DIM; i++)
     error_count += VERIFY( sort_list.is_on_list(&sort_data[i]) );
   error_count += VERIFY( sort_list.is_coherent() );
   sort_list.reset();

   return error_count;
} // static int test_SORT

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_ERRS
//
// Purpose-
//       Test strong List typing
//
//----------------------------------------------------------------------------
static int
   test_ERRS(void)                  // Test strong List typing
{
   //-------------------------------------------------------------------------
   // ERROR CHECKING: **ALL** check lines should fail to compile
   //-------------------------------------------------------------------------
   #if( USE_ERROR_CHECK )           // Test strong list typing
     // These statements are OK
     DHDL_list<DHDL_block> dhdl_list;
     List<SORT_block>      sort_list;
     DHDL_block*           dhdl_link= dhdl_list.remq();
     SORT_block*           sort_link= sort_list.remq();
     struct Diff : public List<Diff>::Link { int N= 732; }; // A different type
     List<Diff>            diff_list;
     Diff*                 diff_link= diff_list.remq();

     // These statements are error tests, each testing type mismatches
     diff_link= dhdl_list.remq();   // ERROR: Remove dhdl_link from dhdl_list
     dhdl_list.fifo(diff_link);     // ERROR: Insert dhdl_link onto dhdl_list

     // ERROR: Insert after diff_link onto sort_list
     sort_list.insert(diff_link, sort_link, sort_link);

     // ERROR: Insert diff_link onto sort_list
     sort_list.insert(sort_link, diff_link, diff_link);
   #endif

   return 0;
} // static int test_ERRS

//----------------------------------------------------------------------------
//
// Subroutine-
//       main
//
// Purpose-
//       Mainline code.
//
//----------------------------------------------------------------------------
int
   main(                            // Mainline code
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   //-------------------------------------------------------------------------
   // Initialize
   Wrapper  tc;                     // The test case wrapper
   Wrapper* tr= &tc;                // A test case wrapper pointer

   tc.on_main([tr](int, char*[])
   {
     if( opt_verbose ) {
       debugf("%s: %s %s\n", __FILE__, __DATE__, __TIME__);
       debugf("USE_BEGIN_END(%s)\n", USE_BEGIN_END ? "true" : "false");
     }

     int error_count= 0;

     error_count += test_AI();        // AI_list
     error_count += test_DHDL();      // DHDL_list, aka List
     error_count += test_DHSL();      // DHSL_list
     error_count += test_SHSL();      // SHSL_list
     error_count += test_SORT();      // SORT_list
     error_count += test_ERRS();      // Test strong List typing

     if( opt_verbose ) {
       debugf("\n");
       tr->report_errors(error_count);
     }
     return error_count != 0;
   });

   //-------------------------------------------------------------------------
   // Run the test
   return tc.run(argc, argv);
}
