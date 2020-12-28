//----------------------------------------------------------------------------
//
//       Copyright (C) 2007-2020 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
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
//       2020/12/28
//
//----------------------------------------------------------------------------
#include <new>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <pub/Debug.h>
using namespace pub::debugging;

#include "pub/List.h"
using pub::AU_List;
using pub::DHDL_List;
using pub::DHSL_List;
using pub::NODE_List;
using pub::SHSL_List;
using pub::SORT_List;
using pub::List;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum {DIM= 12};                     // Array size. Use: 9 < DIM < 100
#define USE_ERROR_CHECK false       // Run type checking? (Compile errors)

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

// MARKER
//----------------------------------------------------------------------------
// AU block
//----------------------------------------------------------------------------
struct AU_block
:  public Prefix, public AU_List<AU_block>::Link, public Suffix {
int                    index;
}; // class AU_block

//----------------------------------------------------------------------------
// DHDL block
//----------------------------------------------------------------------------
struct DHDL_block
:  public Prefix, public DHDL_List<DHDL_block>::Link, public Suffix {
int                    index;
}; // class DHDL_block

//----------------------------------------------------------------------------
// DHSL block
//----------------------------------------------------------------------------
struct DHSL_block
:  public Prefix, public DHSL_List<DHSL_block>::Link, public Suffix {
int                    index;
}; // class DHSL_block

//----------------------------------------------------------------------------
// NODE block
//----------------------------------------------------------------------------
struct NODE_block
:  public Prefix, public NODE_List<NODE_block>::Link, public Suffix {
int                    index;
}; // class NODE_block

//----------------------------------------------------------------------------
// SHSL block
//----------------------------------------------------------------------------
struct SHSL_block
:  public Prefix, public SHSL_List<SHSL_block>::Link, public Suffix {
int                    index;
}; // class SHSL_block

//----------------------------------------------------------------------------
// SORT block
//----------------------------------------------------------------------------
struct SORT_block
:  public Prefix, public SORT_List<SORT_block>::Link, public Suffix {
int                    index;

virtual int compare(const SORT_List<void>::Link* that) const override
{  return (index - static_cast<const SORT_block*>(that)->index); }
}; // class SORT_block

//----------------------------------------------------------------------------
//
// Subroutine-
//       show_AU
//
// Purpose-
//       Display atomic update list.
//
//----------------------------------------------------------------------------
static void
   show_AU(                         // Display a list
     AU_List<AU_block>*
                       anchor)      // The list anchor
{
   printf("List:");
   AU_block* link= anchor->get_tail(); // Get head element
   while( link != NULL ) {
     printf(" %2d", link->index);
     link= link->get_prev();
   }

   printf("\n");
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       show_AU
//
// Purpose-
//       Display atomic update List with removed element
//
//----------------------------------------------------------------------------
static void
   show_AU(                         // Display a list
     AU_List<AU_block>*
                       anchor,      // The list anchor
     AU_block*         removed)     // The removed link
{
   printf("List:");
   AU_block* link= anchor->get_tail(); // Get head element
   while( link != NULL ) {
     printf(" %2d", link->index);
     link= link->get_prev();
   }

   printf(" --(%2d)", removed->index);
   printf("\n");
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       show_DHDL
//
// Purpose-
//       Display a double headed doubly linked list.
//
//----------------------------------------------------------------------------
static void
   show_DHDL(                       // Display a list
     DHDL_List<DHDL_block>*
                       anchor)      // The list anchor
{
   printf("List:");
   DHDL_block* link= anchor->get_head(); // Get head element
   while( link != NULL ) {
     printf(" %2d", link->index);
     link= link->get_next();
   }

   printf("\n");
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       show_DHDL
//
// Purpose-
//       Display a double headed doubly linked list with removal.
//
//----------------------------------------------------------------------------
static void
   show_DHDL(                       // Display a list
     DHDL_List<DHDL_block>*
                       anchor,      // The list anchor
     DHDL_block*       removed)     // The removed link
{
   printf("List:");
   DHDL_block* link= anchor->get_head(); // Get head element
   while( link != NULL ) {
     printf(" %2d", link->index);
     link= link->get_next();
   }

   printf(" --(%2d)", removed->index);
   printf("\n");
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       show_DHSL
//
// Purpose-
//       Display a double headed singly linked list.
//
//----------------------------------------------------------------------------
static void
   show_DHSL(                       // Display a list
     DHSL_List<DHSL_block>*
                       anchor)      // The list anchor
{
   printf("List:");
   DHSL_block* link= anchor->get_head(); // Get head element
   while( link != NULL ) {
     printf(" %2d", link->index);
     link= link->get_next();
   }

   printf("\n");
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       show_DHSL
//
// Purpose-
//       Display a double headed singly linked list with removal.
//
//----------------------------------------------------------------------------
static void
   show_DHSL(                       // Display a list
     DHSL_List<DHSL_block>*
                       anchor,      // The list anchor
     DHSL_block*       removed)     // The removed link
{
   printf("List:");
   DHSL_block* link= anchor->get_head(); // Get head element
   while( link != NULL ) {
     printf(" %2d", link->index);
     link= link->get_next();
   }

   printf(" --(%2d)", removed->index);
   printf("\n");
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       show_NODE
//
// Purpose-
//       Display a generic block doubly linked NODE list
//
//----------------------------------------------------------------------------
static void
   show_NODE(                       // Display a list
     NODE_List<NODE_block>*
                       anchor)      // The list anchor
{
   printf("List:");
   NODE_block* link= anchor->get_head(); // Get head element
   while( link != NULL ) {
     assert( link->get_parent() == anchor );

     printf(" %2d", link->index);
     link= link->get_next();
   }

   printf("\n");
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       show_NODE
//
// Purpose-
//       Display a generic block doubly linked NODE list.
//
//----------------------------------------------------------------------------
static void
   show_NODE(                       // Display a list
     NODE_List<NODE_block>*
                       anchor,      // The list anchor
     NODE_block*       removed)     // The removed link
{
   printf("List:");
   NODE_block* link= anchor->get_head(); // Get head element
   while( link != NULL ) {
     assert( link->get_parent() == anchor );

     printf(" %2d", link->index);
     link= link->get_next();
   }

   printf(" --(%2d)", removed->index);
   assert( removed->get_parent() == nullptr );
   printf("\n");
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       show_SHSL
//
// Purpose-
//       Display a singly headed singly linked list.
//
//----------------------------------------------------------------------------
static void
   show_SHSL(                       // Display a list
     SHSL_List<SHSL_block>*
                       anchor)      // The list anchor
{
   printf("List:");
   SHSL_block* link= anchor->get_head(); // Get head element
   while( link != NULL ) {
     printf(" %2d", link->index);
     link= link->get_next();
   }

   printf("\n");
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       show_SHSL
//
// Purpose-
//       Display a singly headed singly linked list with removal.
//
//----------------------------------------------------------------------------
static void
   show_SHSL(                       // Display a list
     SHSL_List<SHSL_block>*
                       anchor,      // The list anchor
     SHSL_block*       removed)     // The removed link
{
   printf("List:");
   SHSL_block* link= anchor->get_head(); // Get head element
   while( link != NULL ) {
     printf(" %2d", link->index);
     link= link->get_next();
   }

   printf(" --(%2d)", removed->index);
   printf("\n");
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       show_SORT
//
// Purpose-
//       Display a SORT_List.
//
//----------------------------------------------------------------------------
static void
   show_SORT(                       // Display a list
     SORT_List<SORT_block>*
                       anchor)      // The list anchor
{
   printf("List:");
   SORT_block* link= anchor->get_head(); // Get head element
   while( link != NULL ) {
     printf(" %2d", link->index);
     link= link->get_next();
   }

   printf("\n");
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       main
//
// Purpose-
//       Mainline code.
//
//----------------------------------------------------------------------------
extern int
   main(void)                       // Mainline code
{
   //-------------------------------------------------------------------------
   // AU tests
   //-------------------------------------------------------------------------
   AU_block                    au_data[DIM];
   AU_List<AU_block>           au_list;
   AU_block*                   au_link;

   printf("\n");
   printf("AU Storage:\n");
   printf("%8zd Sizeof(AU_List)\n", sizeof(AU_List<AU_block>));
   printf("%8zd Sizeof(AU_Link)\n", sizeof(AU_List<AU_block>::Link));

   printf("\n");
   printf("Null AU:\n");
   show_AU(&au_list);

   //-------------------------------------------------------------------------
   // AU FIFO/REMQ test
   //-------------------------------------------------------------------------
   printf("\n");
   printf("AU_FIFO test:\n");       // This test does NOT use an AU_FIFO object
   for(int i=0; i<DIM; i++) {
     au_data[i].index= i + 1;
     au_list.fifo(&au_data[i]);
     show_AU(&au_list);
   }
   for(int i=0; i<DIM; i++)
     assert(au_list.is_on_list(&au_data[i]));
   assert(au_list.is_coherent());

   for(;;) {
     au_link= au_list.remq();
     if( au_link == NULL )
       break;
     show_AU(&au_list, au_link);
   }
   for(int i=0; i<DIM; i++)
     assert(!au_list.is_on_list(&au_data[i]));
   assert(au_list.is_coherent());
   au_list.reset();

   //-------------------------------------------------------------------------
   // DHDL tests
   //-------------------------------------------------------------------------
   DHDL_block                  dhdl_data[DIM];
   DHDL_List<DHDL_block>       dhdl_list;
   DHDL_block*                 dhdl_link;

   printf("\n");
   printf("DHDL Storage:\n");
   printf("%8zd Sizeof(List)\n", sizeof(List<DHDL_block>));
   printf("%8zd Sizeof(Link)\n", sizeof(List<DHDL_block>::Link));

   printf("\n");
   printf("Null DHDL:\n");
   show_DHDL(&dhdl_list);

   //-------------------------------------------------------------------------
   // DHDL LIFO test
   //-------------------------------------------------------------------------
   printf("\n");
   printf("DHDL_LIFO test (1..%d):\n", DIM);
   for(int i=0; i<DIM; i++) {
     dhdl_data[i].index= i + 1;
     dhdl_list.lifo(&dhdl_data[i]);
     show_DHDL(&dhdl_list);
   }
   for(int i=0; i<DIM; i++)
     assert(dhdl_list.is_on_list(&dhdl_data[i]));
   assert(dhdl_list.is_coherent());

   for(;;) {
     dhdl_link= dhdl_list.remq();
     if( dhdl_link == NULL )
       break;
     show_DHDL(&dhdl_list, dhdl_link);
   }
   for(int i=0; i<DIM; i++)
     assert(!dhdl_list.is_on_list(&dhdl_data[i]));
   assert(dhdl_list.is_coherent());

   //-------------------------------------------------------------------------
   // DHDL FIFO test
   //-------------------------------------------------------------------------
   printf("\n");
   printf("DHDL_FIFO test:\n");
   for(int i=0; i<DIM; i++) {
     dhdl_list.fifo(&dhdl_data[i]);
     show_DHDL(&dhdl_list);
   }
   for(int i=0; i<DIM; i++)
     assert(dhdl_list.is_on_list(&dhdl_data[i]));
   assert(dhdl_list.is_coherent());

   for(;;) {
     dhdl_link= dhdl_list.remq();
     if( dhdl_link == NULL )
       break;
     show_DHDL(&dhdl_list, dhdl_link);
   }
   for(int i=0; i<DIM; i++)
     assert(!dhdl_list.is_on_list(&dhdl_data[i]));
   assert(dhdl_list.is_coherent());

   //-------------------------------------------------------------------------
   // DHDL remove/insert specific
   //-------------------------------------------------------------------------
   printf("\n");
   printf("DHDL_REMOVE(position) test:\n");
   for(int i=0; i<DIM; i++)
     dhdl_list.fifo(&dhdl_data[i]);
   show_DHDL(&dhdl_list);

   printf("\n");
   printf("DHDL_REMOVE(1) test:\n");
   dhdl_link= &dhdl_data[1-1];
   dhdl_list.remove(dhdl_link, dhdl_link);
   show_DHDL(&dhdl_list, dhdl_link);
   assert(!dhdl_list.is_on_list(&dhdl_data[1-1]));
   assert(dhdl_list.is_coherent());

   printf("\n");
   printf("DHDL_REMOVE(5) test:\n");
   dhdl_link= &dhdl_data[5-1];
   dhdl_list.remove(dhdl_link, dhdl_link);
   show_DHDL(&dhdl_list, dhdl_link);
   assert(!dhdl_list.is_on_list(&dhdl_data[5-1]));
   assert(dhdl_list.is_coherent());

   printf("\n");
   printf("DHDL_REMOVE(%d) test:\n", DIM);
   dhdl_link= &dhdl_data[DIM-1];
   dhdl_list.remove(dhdl_link, dhdl_link);
   show_DHDL(&dhdl_list, dhdl_link);
   assert(!dhdl_list.is_on_list(&dhdl_data[DIM-1]));
   assert(dhdl_list.is_coherent());

   printf("\n");
   printf("DHDL_INSERT(1) at head:\n");
   dhdl_list.insert(NULL, &dhdl_data[1-1], &dhdl_data[1-1]);
   show_DHDL(&dhdl_list);
   assert(dhdl_list.is_on_list(&dhdl_data[1-1]));
   assert(dhdl_list.is_coherent());

   printf("\n");
   printf("DHDL_INSERT(%d) at tail:\n", DIM);
   dhdl_list.insert(dhdl_list.get_tail(), &dhdl_data[DIM-1], &dhdl_data[DIM-1]);
   show_DHDL(&dhdl_list);
   assert(dhdl_list.is_on_list(&dhdl_data[DIM-1]));
   assert(dhdl_list.is_coherent());

   printf("\n");
   printf("DHDL_INSERT(5) after(4):\n");
   dhdl_list.insert(&dhdl_data[4-1], &dhdl_data[5-1], &dhdl_data[5-1]);
   show_DHDL(&dhdl_list);
   assert(dhdl_list.is_on_list(&dhdl_data[5-1]));
   assert(dhdl_list.is_coherent());

   printf("\n");
   printf("DHDL_REMOVE(5..8):\n");
   dhdl_list.remove(&dhdl_data[5-1], &dhdl_data[8-1]);
   show_DHDL(&dhdl_list);
   assert(dhdl_list.is_on_list(&dhdl_data[4-1]));
   assert(!dhdl_list.is_on_list(&dhdl_data[5-1]));
   assert(!dhdl_list.is_on_list(&dhdl_data[6-1]));
   assert(!dhdl_list.is_on_list(&dhdl_data[7-1]));
   assert(!dhdl_list.is_on_list(&dhdl_data[8-1]));
   assert(dhdl_list.is_on_list(&dhdl_data[9-1]));
   assert(dhdl_list.is_coherent());

   printf("\n");
   printf("DHDL_INSERT(5..8):\n");
   dhdl_list.insert(&dhdl_data[4-1], &dhdl_data[5-1], &dhdl_data[8-1]);
   show_DHDL(&dhdl_list);
   assert(dhdl_list.is_on_list(&dhdl_data[4-1]));
   assert(dhdl_list.is_on_list(&dhdl_data[5-1]));
   assert(dhdl_list.is_on_list(&dhdl_data[6-1]));
   assert(dhdl_list.is_on_list(&dhdl_data[7-1]));
   assert(dhdl_list.is_on_list(&dhdl_data[8-1]));
   assert(dhdl_list.is_on_list(&dhdl_data[9-1]));
   assert(dhdl_list.is_coherent());

   printf("\n");
   printf("DHDL_REMOVE(1..%d):\n", DIM);
   dhdl_list.remove(&dhdl_data[1-1], &dhdl_data[DIM-1]);
   show_DHDL(&dhdl_list);
   for(int i=0; i<DIM; i++)
     assert(!dhdl_list.is_on_list(&dhdl_data[i]));
   assert(dhdl_list.is_coherent());

   printf("\n");
   printf("DHDL_INSERT(1..%d):\n", DIM);
   dhdl_list.insert(NULL, &dhdl_data[1-1], &dhdl_data[DIM-1]);
   show_DHDL(&dhdl_list);
   for(int i=0; i<DIM; i++)
     assert(dhdl_list.is_on_list(&dhdl_data[i]));
   assert(dhdl_list.is_coherent());
   dhdl_list.reset();

   //-------------------------------------------------------------------------
   // DHSL tests
   //-------------------------------------------------------------------------
   DHSL_block                  dhsl_data[DIM];
   DHSL_List<DHSL_block>       dhsl_list;
   DHSL_block*                 dhsl_link;

   printf("\n");
   printf("DHSL Storage:\n");
   printf("%8zd Sizeof(DHSL_List)\n", sizeof(DHSL_List<DHSL_block>));
   printf("%8zd Sizeof(DHSL_Link)\n", sizeof(DHSL_List<DHSL_block>::Link));

   printf("\n");
   printf("Null DHSL:\n");
   show_DHSL(&dhsl_list);

   //-------------------------------------------------------------------------
   // DHSL LIFO test
   //-------------------------------------------------------------------------
   printf("\n");
   printf("DHSL_LIFO test (1..%d):\n", DIM);
   for(int i=0; i<DIM; i++) {
     dhsl_data[i].index= i + 1;
     dhsl_list.lifo(&dhsl_data[i]);
     show_DHSL(&dhsl_list);
   }
   for(int i=0; i<DIM; i++)
     assert(dhsl_list.is_on_list(&dhsl_data[i]));
   assert(dhsl_list.is_coherent());

   for(;;) {
     dhsl_link= dhsl_list.remq();
     if( dhsl_link == NULL )
       break;
     show_DHSL(&dhsl_list, dhsl_link);
   }
   for(int i=0; i<DIM; i++)
     assert(!dhsl_list.is_on_list(&dhsl_data[i]));
   assert(dhsl_list.is_coherent());

   //-------------------------------------------------------------------------
   // DHSL FIFO test
   //-------------------------------------------------------------------------
   printf("\n");
   printf("DHSL_FIFO test:\n");
   for(int i=0; i<DIM; i++) {
     dhsl_list.fifo(&dhsl_data[i]);
     show_DHSL(&dhsl_list);
   }
   for(int i=0; i<DIM; i++)
     assert(dhsl_list.is_on_list(&dhsl_data[i]));
   assert(dhsl_list.is_coherent());

   for(;;) {
     dhsl_link= dhsl_list.remq();
     if( dhsl_link == NULL )
       break;
     show_DHSL(&dhsl_list, dhsl_link);
   }
   for(int i=0; i<DIM; i++)
     assert(!dhsl_list.is_on_list(&dhsl_data[i]));
   assert(dhsl_list.is_coherent());
   dhsl_list.reset();

   //-------------------------------------------------------------------------
   // NODE tests
   //-------------------------------------------------------------------------
   NODE_block                  node_data[DIM];
   NODE_List<NODE_block>       node_list;
   NODE_block*                 node_link;

   printf("\n");
   printf("NODE Storage:\n");
   printf("%8zd Sizeof(List)\n", sizeof(NODE_List<NODE_block>));
   printf("%8zd Sizeof(Link)\n", sizeof(NODE_List<NODE_block>::Link));

   printf("\n");
   printf("Null NODE:\n");
   show_NODE(&node_list);

   //-------------------------------------------------------------------------
   // NODE LIFO test
   //-------------------------------------------------------------------------
   printf("\n");
   printf("NODE_LIFO test (1..%d):\n", DIM);
   for(int i=0; i<DIM; i++) {
     node_data[i].index= i + 1;
     node_list.lifo(&node_data[i]);
     show_NODE(&node_list);
   }
   for(int i=0; i<DIM; i++)
     assert(node_list.is_on_list(&node_data[i]));
   assert(node_list.is_coherent());

   for(;;) {
     node_link= node_list.remq();
     if( node_link == NULL )
       break;
     show_NODE(&node_list, node_link);
   }
   for(int i=0; i<DIM; i++)
     assert(!node_list.is_on_list(&node_data[i]));
   assert(node_list.is_coherent());

   //-------------------------------------------------------------------------
   // NODE FIFO test
   //-------------------------------------------------------------------------
   printf("\n");
   printf("NODE_FIFO test:\n");
   for(int i=0; i<DIM; i++) {
     node_list.fifo(&node_data[i]);
     show_NODE(&node_list);
   }
   for(int i=0; i<DIM; i++)
     assert(node_list.is_on_list(&node_data[i]));
   assert(node_list.is_coherent());

   for(;;) {
     node_link= node_list.remq();
     if( node_link == NULL )
       break;
     show_NODE(&node_list, node_link);
   }
   for(int i=0; i<DIM; i++)
     assert(!node_list.is_on_list(&node_data[i]));
   assert(node_list.is_coherent());

   //-------------------------------------------------------------------------
   // NODE remove/insert specific
   //-------------------------------------------------------------------------
   printf("\n");
   printf("NODE_REMOVE(position) test:\n");
   for(int i=0; i<DIM; i++)
     node_list.fifo(&node_data[i]);
   show_NODE(&node_list);

   printf("\n");
   printf("NODE_REMOVE(1) test:\n");
   node_link= &node_data[1-1];
   node_list.remove(node_link, node_link);
   show_NODE(&node_list, node_link);
   assert(!node_list.is_on_list(&node_data[1-1]));
   assert(node_list.is_coherent());

   printf("\n");
   printf("NODE_REMOVE(5) test:\n");
   node_link= &node_data[5-1];
   node_list.remove(node_link, node_link);
   show_NODE(&node_list, node_link);
   assert(!node_list.is_on_list(&node_data[5-1]));
   assert(node_list.is_coherent());

   printf("\n");
   printf("NODE_REMOVE(%d) test:\n", DIM);
   node_link= &node_data[DIM-1];
   node_list.remove(node_link, node_link);
   show_NODE(&node_list, node_link);
   assert(!node_list.is_on_list(&node_data[DIM-1]));
   assert(node_list.is_coherent());

   printf("\n");
   printf("NODE_INSERT(1) at head:\n");
   node_list.insert(NULL, &node_data[1-1], &node_data[1-1]);
   show_NODE(&node_list);
   assert(node_list.is_on_list(&node_data[1-1]));
   assert(node_list.is_coherent());

   printf("\n");
   printf("NODE_INSERT(%d) at tail:\n", DIM);
   node_list.insert(node_list.get_tail(), &node_data[DIM-1], &node_data[DIM-1]);
   show_NODE(&node_list);
   assert(node_list.is_on_list(&node_data[DIM-1]));
   assert(node_list.is_coherent());

   printf("\n");
   printf("NODE_INSERT(5) after(4):\n");
   node_list.insert(&node_data[4-1], &node_data[5-1], &node_data[5-1]);
   show_NODE(&node_list);
   assert(node_list.is_on_list(&node_data[5-1]));
   assert(node_list.is_coherent());

   printf("\n");
   printf("NODE_REMOVE(5..8):\n");
   node_list.remove(&node_data[5-1], &node_data[8-1]);
   show_NODE(&node_list);
   assert(node_list.is_on_list(&node_data[4-1]));
   assert(!node_list.is_on_list(&node_data[5-1]));
   assert(!node_list.is_on_list(&node_data[6-1]));
   assert(!node_list.is_on_list(&node_data[7-1]));
   assert(!node_list.is_on_list(&node_data[8-1]));
   assert(node_list.is_on_list(&node_data[9-1]));
   assert(node_list.is_coherent());

   printf("\n");
   printf("NODE_INSERT(5..8):\n");
   node_list.insert(&node_data[4-1], &node_data[5-1], &node_data[8-1]);
   show_NODE(&node_list);
   assert(node_list.is_on_list(&node_data[4-1]));
   assert(node_list.is_on_list(&node_data[5-1]));
   assert(node_list.is_on_list(&node_data[6-1]));
   assert(node_list.is_on_list(&node_data[7-1]));
   assert(node_list.is_on_list(&node_data[8-1]));
   assert(node_list.is_on_list(&node_data[9-1]));
static_assert(DIM >= 9, "DIM too small");
   assert(node_list.is_coherent());

   printf("\n");
   printf("NODE_REMOVE(1..%d):\n", DIM);
   node_list.remove(&node_data[1-1], &node_data[DIM-1]);
   show_NODE(&node_list);
   for(int i=0; i<DIM; i++)
     assert(!node_list.is_on_list(&node_data[i]));
   assert(node_list.is_coherent());

   printf("\n");
   printf("NODE_INSERT(1..%d):\n", DIM);
   node_list.insert(NULL, &node_data[1-1], &node_data[DIM-1]);
   show_NODE(&node_list);
   for(int i=0; i<DIM; i++)
     assert(node_list.is_on_list(&node_data[i]));
   assert(node_list.is_coherent());
   node_list.reset();

   //-------------------------------------------------------------------------
   // SHSL tests
   //-------------------------------------------------------------------------
   SHSL_block                  shsl_data[DIM];
   SHSL_List<SHSL_block>       shsl_list;
   SHSL_block*                 shsl_link;

   printf("\n");
   printf("SHSL Storage:\n");
   printf("%8zd Sizeof(SHSL_List)\n", sizeof(SHSL_List<SHSL_block>));
   printf("%8zd Sizeof(SHSL_Link)\n", sizeof(SHSL_List<SHSL_block>::Link));

   printf("\n");
   printf("Null SHSL:\n");
   show_SHSL(&shsl_list);

   //-------------------------------------------------------------------------
   // SHSL LIFO test
   //-------------------------------------------------------------------------
   printf("\n");
   printf("SHSL_LIFO test (1..%d):\n", DIM);
   for(int i=0; i<DIM; i++) {
     shsl_data[i].index= i + 1;
     shsl_list.lifo(&shsl_data[i]);
     show_SHSL(&shsl_list);
   }
   for(int i=0; i<DIM; i++)
     assert(shsl_list.is_on_list(&shsl_data[i]));
   assert(shsl_list.is_coherent());

   for(;;) {
     shsl_link= shsl_list.remq();
     if( shsl_link == NULL )
       break;
     show_SHSL(&shsl_list, shsl_link);
   }
   for(int i=0; i<DIM; i++)
     assert(!shsl_list.is_on_list(&shsl_data[i]));
   assert(shsl_list.is_coherent());

   //-------------------------------------------------------------------------
   // SHSL FIFO test
   //-------------------------------------------------------------------------
   printf("\n");
   printf("SHSL_FIFO test:\n");
   for(int i=0; i<DIM; i++) {
     shsl_data[i].index= i + 1;
     shsl_list.fifo(&shsl_data[i]);
     show_SHSL(&shsl_list);
   }
   for(int i=0; i<DIM; i++)
     assert(shsl_list.is_on_list(&shsl_data[i]));
   assert(shsl_list.is_coherent());

   for(;;) {
     shsl_link= shsl_list.remq();
     if( shsl_link == NULL )
       break;
     show_SHSL(&shsl_list, shsl_link);
   }
   for(int i=0; i<DIM; i++)
     assert(!shsl_list.is_on_list(&shsl_data[i]));
   assert(shsl_list.is_coherent());
   shsl_list.reset();

   //-------------------------------------------------------------------------
   // SORT tests
   //-------------------------------------------------------------------------
   SORT_block                  sort_data[DIM];
   SORT_List<SORT_block>       sort_list;

   printf("\n");
   printf("SORT Storage:\n");
   printf("%8zd Sizeof(List)\n", sizeof(SORT_List<SORT_block>));
   printf("%8zd Sizeof(Link)\n", sizeof(SORT_List<SORT_block>::Link));

   //-------------------------------------------------------------------------
   // SORT LIFO test
   //-------------------------------------------------------------------------
   printf("\n");
   printf("SORT_LIFO test (1..%d):\n", DIM);
   for(int i=0; i<DIM; i++) {
     sort_data[i].index= i + 1;
     sort_list.lifo(&sort_data[i]);
   }
   show_SORT(&sort_list);
   sort_list.sort();
   show_SORT(&sort_list);

   for(int i=0; i<DIM; i++)
     assert(sort_list.is_on_list(&sort_data[i]));
   assert(sort_list.is_coherent());
   sort_list.reset();

   //-------------------------------------------------------------------------
   // SORT FIFO test
   //-------------------------------------------------------------------------
   printf("\n");
   printf("SORT_FIFO test:\n");
   for(int i=0; i<DIM; i++)
     sort_list.fifo(&sort_data[i]);
   show_SORT(&sort_list);
   sort_list.sort();
   show_SORT(&sort_list);

   for(int i=0; i<DIM; i++)
     assert(sort_list.is_on_list(&sort_data[i]));
   assert(sort_list.is_coherent());
   sort_list.reset();

   //-------------------------------------------------------------------------
   // ERROR CHECKING: **ALL** check lines should fail to copile
   //-------------------------------------------------------------------------
   #if( USE_ERROR_CHECK )           // Test strong list types
     SORT_block*  sort_link;        // This is OK
     struct Diff : public List<Diff>::Link { int N= 732; }; // This is OK

     Diff diff= dhdl_list.remq();   // Remove from wrong list
     dhdl_list.fifo(diff);          // Insert onto wrong list

     diff= dhdl_list.remq();
     node_link= dhdl_list.remq();
     node_list.insert(node_link, dhdl_link, dhdl_link);
     sort_list.insert(sort_link, dhdl_link, dhdl_link);
     sort_list.insert(sort_link, node_link, node_link);
     sort_list.insert(sort_link, diff, diff);
   #endif

   return 0;
}
