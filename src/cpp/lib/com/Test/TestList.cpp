//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2020 Frank Eskesen.
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
//       Linked list tests.
//
// Last change date-
//       2020/06/13
//
//----------------------------------------------------------------------------
#include <new>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <com/sysmac.h>
#include <com/Debug.h>
#include "com/List.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define DIM                      12 // Array size, 5< size <100

//----------------------------------------------------------------------------
// Internal macros
//----------------------------------------------------------------------------
#define _OFFSET_(_struc, _field) size_t(&((_struc*)0)->_field)

//----------------------------------------------------------------------------
// AU_Block (only used for sizeof)
//----------------------------------------------------------------------------
struct AU_Block : public AU_List<AU_Block>::Link
{
// AU_Link             ignored;
   int                 au_value;
};

//----------------------------------------------------------------------------
// DHDL_Block (only used for sizeof)
//----------------------------------------------------------------------------
struct DHDL_Block : public DHDL_List<DHDL_Block>::Link
{
// Link                ignored;
   int                 dhdl_value;
};

//----------------------------------------------------------------------------
// DHSL_Block (only used for sizeof)
//----------------------------------------------------------------------------
struct DHSL_Block : public DHSL_List<DHSL_Block>::Link
{
// SL_Link             ignored;
   int                 dhsl_value;
};

//----------------------------------------------------------------------------
// SHSL_Block (only used for sizeof)
//----------------------------------------------------------------------------
struct SHSL_Block : public SHSL_List<SHSL_Block>::Link
{
// SL_Link             ignored;
   int                 shsl_value;
};

//----------------------------------------------------------------------------
//
// Class-
//       Prefix
//
// Purpose-
//       Self-validating class.
//
//----------------------------------------------------------------------------
class Prefix                        // Self-validating class
{
private:
enum                                // Generic enumerations
{
   VALIDATOR=            0x02469773 // Validation constant
}; // enum

public:
   ~Prefix( void )                  // Destructor
{
   assert(isValid());
}

   Prefix( void )                   // Constructor
:
   word(VALIDATOR)
{
}

int                                 // TRUE if valid
   isValid( void )
{
   return (word == VALIDATOR);
}

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
class Suffix                        // Self-validating class
{
private:
enum                                // Generic enumerations
{
   VALIDATOR=            0x37796420 // Validation constant
}; // enum

public:
   ~Suffix( void )                  // Destructor
{
   assert(isValid());
}

   Suffix( void )                   // Constructor
:
   word(VALIDATOR)
{
}

int                                 // TRUE if valid
   isValid( void )
{
   return (word == VALIDATOR);
}

private:
   long                word;        // Validiation word
}; // class Suffix

//----------------------------------------------------------------------------
// Auxiliary link
//----------------------------------------------------------------------------
struct AUX_Link : public SHSL_List<AUX_Link>::Link
{
}; // class AUX_Link

//----------------------------------------------------------------------------
// Generic block
//----------------------------------------------------------------------------
struct GEN_block : public Prefix,
                   public AU_List<GEN_block>::Link,
#ifdef USE_STANDALONE_SORT_LINK
                   public DHDL_List<GEN_block>::Link,
#else
  #define DHDL_List SORT_List
#endif
                   public DHSL_List<GEN_block>::Link,
                   public SHSL_List<GEN_block>::Link,
                   public SORT_List<GEN_block>::Link,
                   public Suffix {
// Attributes
public:
int                    gen_value;
AUX_Link               aux_link;

// Methods
static inline GEN_block*            // -> GEN_block
   make(                            // Convert from AU_Link
     AU_List<GEN_block>::Link*
                       link)        // -> AU_Link
{
   return (GEN_block*)link;
}

static inline GEN_block*            // -> GEN_block
   make(                            // Convert from AUX_Link
     AUX_Link*         link)        // -> AUX_Link
{
   char*               source;
   size_t              offset;

   source= (char*)link;
   offset= _OFFSET_(GEN_block, aux_link);
// offset= offsetof(GEN_block, aux_link);
// offset= (&((GEN_block*)link)->aux_link - link);
   source -= offset;
   return (GEN_block*)source;
}

static inline GEN_block*            // -> GEN_block
   make(                            // Convert from DHDL_Link
     DHDL_List<GEN_block>::Link*
                       link)        // -> SL_Link
{
   return (GEN_block*)link;
}

static inline GEN_block*            // -> GEN_block
   make(                            // Convert from DHSL_Link
     DHSL_List<GEN_block>::Link*
                       link)        // -> SL_Link
{
   return (GEN_block*)link;
}

static inline GEN_block*            // -> GEN_block
   make(                            // Convert from SHSL_Link
     SHSL_List<GEN_block>::Link*
                       link)        // -> SL_Link
{
   return (GEN_block*)link;
}

#ifdef USE_STANDALONE_SORT_LINK
static inline GEN_block*            // -> GEN_block
   make(                            // Convert from Sort_Link
     SORT_List<GEN_block>::Link*
                       link)        // -> Link
{
   return (GEN_block*)link;
}
#endif

virtual int
   compare(
     const SORT_List<void>::Link* that) const
{  return (gen_value - static_cast<const GEN_block*>(that)->gen_value); }
}; // class GEN_block

// Implement BASE CLASS method compare
template <> int
   SORT_List<GEN_block>::Link::compare(
     const SORT_List<void>::Link* that) const
{  printf("This class has been overridden, hasn't it?\n");
   return (static_cast<const GEN_block*>(this)->gen_value
         - static_cast<const GEN_block*>(that)->gen_value); }

//----------------------------------------------------------------------------
//
// Subroutine-
//       debug_DHDL
//
// Purpose-
//       Debug a DHDL list.
//
//----------------------------------------------------------------------------
static inline void
   debug_DHDL(                      // Debugging display of DHDL list
     DHDL_List<void>*  anchor)      // The list anchor
{
   DHDL_List<void>::Link* link;

   printf("List(%p,%p):\n", anchor->getHead(), anchor->getTail());
   link= anchor->getHead();         // Get head element
   while( link != NULL )
   {
     printf(": %p %p %p\n", link, link->getPrev(), link->getNext());
     link= link->getNext();
   }

   printf("\n");
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       debug_DHDL_array
//
// Purpose-
//       Debug a GenBlock link array (on SortLink)
//
//----------------------------------------------------------------------------
static inline void
   debug_DHDL_array(                // Debug GEN_block array
     GEN_block*        block,       // The list anchor
     int               count)       // Number of elements
{
   printf("debug_DHDL_array(%p,%d)\n", block, count);
   for(int i= 0; i<count; i++)
   {
     DHDL_List<void>::Link* link= (DHDL_List<void>::Link*)&block[i];
     printf(": [%.2d] %p %p %p\n", i, link, link->getPrev(), link->getNext());
   }

   printf("\n");
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       show_AU
//
// Purpose-
//       Display AU_List elements.
//
//----------------------------------------------------------------------------
static void
   show_AU(                         // Display a list
     AU_List<GEN_block>*
                       anchor)      // The list anchor
{
   AU_List<GEN_block>::Link* ptr_link;
   GEN_block*          elem;

   printf("List:");
   ptr_link= anchor->getTail();     // Get head element
   while( ptr_link != NULL )
   {
     elem= GEN_block::make(ptr_link);

     printf(" %2d", elem->gen_value);
     ptr_link= ptr_link->getPrev();
   }

   printf("\n");
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       show_AU
//
// Purpose-
//       Display AU_List elements.
//
//----------------------------------------------------------------------------
static void
   show_AU(                         // Display a list
     AU_List<GEN_block>*
                       anchor,      // The list anchor
     AU_List<GEN_block>::Link*
                       inp_link)    // The removed link
{
   AU_List<GEN_block>::Link* ptr_link;

   printf("List:");
   ptr_link= anchor->getTail(); // Get head element
   while( ptr_link != NULL )
   {
     printf(" %2d", GEN_block::make(ptr_link)->gen_value);
     ptr_link= ptr_link->getPrev();
   }

   printf(" --(%2d)", GEN_block::make(inp_link)->gen_value);

   printf("\n");
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       show_AUX
//
// Purpose-
//       Display a generic block singly linked list.
//
//----------------------------------------------------------------------------
static void
   show_AUX(                        // Display a list
     SHSL_List<AUX_Link>*
                       anchor)      // The list anchor
{
   AUX_Link*           ptr_link;
   GEN_block*          elem;

   printf("List:");
   ptr_link= anchor->getHead(); // Get head element
   while( ptr_link != NULL )
   {
     elem= GEN_block::make(ptr_link);

     printf(" %2d", elem->gen_value);
     ptr_link= ptr_link->getNext();
   }

   printf("\n");
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       show_AUX
//
// Purpose-
//       Display a special singly linked list.
//
//----------------------------------------------------------------------------
static void
   show_AUX(                        // Display a list
     SHSL_List<AUX_Link>*
                       anchor,      // The list anchor
     AUX_Link*         inp_link)    // The removed link
{
   AUX_Link*           ptr_link;

   printf("List:");
   ptr_link= anchor->getHead();     // Get head element
   while( ptr_link != NULL )
   {
     printf(" %2d", GEN_block::make(ptr_link)->gen_value);
     ptr_link= ptr_link->getNext();
   }

   printf(" --(%2d)", GEN_block::make(inp_link)->gen_value);

   printf("\n");
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       show_DHSL
//
// Purpose-
//       Display a generic block doubly linked list.
//
//----------------------------------------------------------------------------
static void
   show_DHSL(                       // Display a list
     DHSL_List<GEN_block>*
                       anchor)      // The list anchor
{
   DHSL_List<GEN_block>::Link* ptr_link;
   GEN_block*          elem;

   printf("List:");
   ptr_link= anchor->getHead();     // Get head element
   while( ptr_link != NULL )
   {
     elem= GEN_block::make(ptr_link);

     printf(" %2d", elem->gen_value);
     ptr_link= ptr_link->getNext();
   }

   printf("\n");
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       show_DHSL
//
// Purpose-
//       Display a generic block doubly headed singly linked list.
//
//----------------------------------------------------------------------------
static void
   show_DHSL(                       // Display a list
     DHSL_List<GEN_block>*
                       anchor,      // The list anchor
     DHSL_List<GEN_block>::Link*
                       inp_link)    // The removed link
{
   DHSL_List<GEN_block>::Link* ptr_link;
   GEN_block*          elem;

   printf("List:");
   ptr_link= anchor->getHead();     // Get head element
   while( ptr_link != NULL )
   {
     elem= GEN_block::make(ptr_link);

     printf(" %2d", elem->gen_value);
     ptr_link= ptr_link->getNext();
   }

   elem= GEN_block::make(inp_link);
   printf(" --(%2d)", elem->gen_value);

   printf("\n");
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       show_DHDL
//
// Purpose-
//       Display a generic block doubly linked list.
//
//----------------------------------------------------------------------------
static void
   show_DHDL(                       // Display a list
     DHDL_List<GEN_block>*
                       anchor)      // The list anchor
{
   DHDL_List<GEN_block>::Link*
                       ptr_link;
   GEN_block*          elem;

   printf("List:");
   ptr_link= anchor->getHead(); // Get head element
   while( ptr_link != NULL )
   {
     elem= GEN_block::make(ptr_link);

     printf(" %2d", elem->gen_value);
     ptr_link= ptr_link->getNext();
   }

   printf("\n");
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       show_DHDL
//
// Purpose-
//       Display a generic block doubly linked list.
//
//----------------------------------------------------------------------------
static void
   show_DHDL(                       // Display a list
     DHDL_List<GEN_block>*
                       anchor,      // The list anchor
     DHDL_List<GEN_block>::Link*
                       inp_link)    // The removed link
{
   DHDL_List<GEN_block>::Link*
                       ptr_link;
   GEN_block*          elem;

   printf("List:");
   ptr_link= anchor->getHead();     // Get head element
   while( ptr_link != NULL )
   {
     elem= GEN_block::make(ptr_link);

     printf(" %2d", elem->gen_value);
     ptr_link= ptr_link->getNext();
   }

   elem= GEN_block::make(inp_link);
   printf(" --(%2d)", elem->gen_value);

   printf("\n");
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       show_SHSL
//
// Purpose-
//       Display a generic block singly linked list.
//
//----------------------------------------------------------------------------
static void
   show_SHSL(                       // Display a list
     SHSL_List<GEN_block>*
                       anchor)      // The list anchor
{
   SHSL_List<GEN_block>::Link* ptr_link;
   GEN_block*          elem;

   printf("List:");
   ptr_link= anchor->getHead();     // Get head element
   while( ptr_link != NULL )
   {
     elem= GEN_block::make(ptr_link);

     printf(" %2d", elem->gen_value);
     ptr_link= ptr_link->getNext();
   }

   printf("\n");
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       show_SHSL
//
// Purpose-
//       Display a special singly linked list.
//
//----------------------------------------------------------------------------
static void
   show_SHSL(                       // Display a list
     SHSL_List<GEN_block>*
                       anchor,      // The list anchor
     SHSL_List<GEN_block>::Link*
                       inp_link)    // The removed link
{
   SHSL_List<GEN_block>::Link* ptr_link;

   printf("List:");
   ptr_link= anchor->getHead();     // Get head element
   while( ptr_link != NULL )
   {
     printf(" %2d", GEN_block::make(ptr_link)->gen_value);
     ptr_link= ptr_link->getNext();
   }

   printf(" --(%2d)", GEN_block::make(inp_link)->gen_value);

   printf("\n");
}

#ifdef USE_STANDALONE_SORT_LINK
//----------------------------------------------------------------------------
//
// Subroutine-
//       show_SORT
//
// Purpose-
//       Display a SORT_List block doubly linked list.
//
//----------------------------------------------------------------------------
static void
   show_SORT(                       // Display a list
     SORT_List<GEN_block>*
                       anchor)      // The list anchor
{
   SORT_List<GEN_block>::Link* ptr_link;
   GEN_block*          elem;

   printf("List:");
   ptr_link= anchor->getHead();     // Get head element
   while( ptr_link != NULL )
   {
     elem= GEN_block::make(ptr_link);

     printf(" %2d", elem->gen_value);
     ptr_link= ptr_link->getNext();
   }

   printf("\n");
}
#endif

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
   AU_List<GEN_block>            au_anchor;
   DHSL_List<GEN_block>        dhsl_anchor;
   SHSL_List<GEN_block>        shsl_anchor;
   SHSL_List<AUX_Link>          aux_anchor;
   SORT_List<GEN_block>        sort_anchor;

   GEN_block                    gen_array[DIM];

   AU_List<GEN_block>::Link*     au_link;
   DHSL_List<GEN_block>::Link* dhsl_link;
   SHSL_List<GEN_block>::Link* shsl_link;
   AUX_Link*                    aux_ptr;
   SORT_List<GEN_block>::Link* sort_link;

#ifdef USE_STANDALONE_SORT_LINK
   DHDL_List<GEN_block>        dhdl_anchor;
   DHDL_List<GEN_block>::Link* dhdl_link;
#else
   #define dhdl_anchor sort_anchor
   #define dhdl_link   sort_link
   #define show_SORT   show_DHDL
#endif

   int                 i;

   for(i=0; i<DIM; i++)             // Identify the elements
   {
     new(&gen_array[i]) GEN_block();

     gen_array[i].gen_value= i + 1;

     assert( ((Prefix)gen_array[i]).isValid() );
     assert( ((Suffix)gen_array[i]).isValid() );
   }

   //-------------------------------------------------------------------------
   // AU tests
   //-------------------------------------------------------------------------
   printf("\n");
   printf("AU Storage:\n");
   printf("%8ld Sizeof(AU_List)\n", (long)sizeof(AU_List<GEN_block>));
   printf("%8ld Sizeof(AU_Link)\n", (long)sizeof(AU_List<GEN_block>::Link));
   printf("%8zd Sizeof(AU_Link)\n", (size_t)_OFFSET_(AU_Block, au_value));
// printf("%8ld Sizeof(AU_Link)\n", (long)offsetof(AU_Block, au_value));

   printf("\n");
   printf("Null AU:\n");
   show_AU(&au_anchor);

   //-------------------------------------------------------------------------
   // AU FIFO test
   //-------------------------------------------------------------------------
   printf("\n");
   printf("AU_FIFO test:\n");
   for(i=0; i<DIM; i++)
   {
     au_anchor.fifo(&gen_array[i]);
     show_AU(&au_anchor);
   }
   for(i=0; i<DIM; i++)
   {
     assert(au_anchor.isOnList(&gen_array[i]));
   }
   assert(au_anchor.isCoherent());

   for(;;)
   {
     au_link= au_anchor.remq();
     if( au_link == NULL )
       break;
     show_AU(&au_anchor, au_link);
   }
   for(i=0; i<DIM; i++)
   {
     assert(!au_anchor.isOnList(&gen_array[i]));
   }
   assert(au_anchor.isCoherent());

   //-------------------------------------------------------------------------
   // DHDL tests
   //-------------------------------------------------------------------------
   printf("\n");
   printf("DHDL Storage:\n");
   printf("%8ld Sizeof(List)\n", (long)sizeof(List<GEN_block>));
   printf("%8ld Sizeof(Link)\n", (long)sizeof(List<GEN_block>::Link));
   printf("%8zd Sizeof(Link)\n", (size_t)_OFFSET_(DHDL_Block, dhdl_value));
// printf("%8ld Sizeof(Link)\n", (long)offsetof(DHDL_Block, dhdl_value));

   printf("\n");
   printf("Null DHDL:\n");
   show_DHDL(&dhdl_anchor);

   //-------------------------------------------------------------------------
   // DHDL LIFO test
   //-------------------------------------------------------------------------
   printf("\n");
   printf("DHDL_LIFO test (1..%d):\n", DIM);
   for(i=0; i<DIM; i++)
   {
     dhdl_anchor.lifo(&gen_array[i]);
     show_DHDL(&dhdl_anchor);
   }
   for(i=0; i<DIM; i++)
   {
     assert(dhdl_anchor.isOnList(&gen_array[i]));
   }
   assert(dhdl_anchor.isCoherent());

   for(;;)
   {
     dhdl_link= dhdl_anchor.remq();
     if( dhdl_link == NULL )
       break;
     show_DHDL(&dhdl_anchor, dhdl_link);
   }
   for(i=0; i<DIM; i++)
   {
     assert(!dhdl_anchor.isOnList(&gen_array[i]));
   }
   assert(dhdl_anchor.isCoherent());

   //-------------------------------------------------------------------------
   // DHDL FIFO test
   //-------------------------------------------------------------------------
   printf("\n");
   printf("DHDL_FIFO test:\n");
   for(i=0; i<DIM; i++)
   {
     dhdl_anchor.fifo(&gen_array[i]);
     show_DHDL(&dhdl_anchor);
   }
   for(i=0; i<DIM; i++)
   {
     assert(dhdl_anchor.isOnList(&gen_array[i]));
   }
   assert(dhdl_anchor.isCoherent());

   for(;;)
   {
     dhdl_link= dhdl_anchor.remq();
     if( dhdl_link == NULL )
       break;
     show_DHDL(&dhdl_anchor, dhdl_link);
   }
   for(i=0; i<DIM; i++)
   {
     assert(!dhdl_anchor.isOnList(&gen_array[i]));
   }
   assert(dhdl_anchor.isCoherent());

   //-------------------------------------------------------------------------
   // DHDL remove/insert specific
   //-------------------------------------------------------------------------
   printf("\n");
   printf("DHDL_REMOVE(position) test:\n");
   for(i=0; i<DIM; i++)
   {
     dhdl_anchor.fifo(&gen_array[i]);
   }
   show_DHDL(&dhdl_anchor);

   printf("\n");
   printf("DHDL_REMOVE(1) test:\n");
   dhdl_link= &gen_array[1-1];
   dhdl_anchor.remove(dhdl_link, dhdl_link);
   show_DHDL(&dhdl_anchor, dhdl_link);
   assert(!dhdl_anchor.isOnList(&gen_array[1-1]));
   assert(dhdl_anchor.isCoherent());

   printf("\n");
   printf("DHDL_REMOVE(5) test:\n");
   dhdl_link= &gen_array[5-1];
   dhdl_anchor.remove(dhdl_link, dhdl_link);
   show_DHDL(&dhdl_anchor, dhdl_link);
   assert(!dhdl_anchor.isOnList(&gen_array[5-1]));
   assert(dhdl_anchor.isCoherent());

   printf("\n");
   printf("DHDL_REMOVE(%d) test:\n", DIM);
   dhdl_link= &gen_array[DIM-1];
   dhdl_anchor.remove(dhdl_link, dhdl_link);
   show_DHDL(&dhdl_anchor, dhdl_link);
   assert(!dhdl_anchor.isOnList(&gen_array[DIM-1]));
   assert(dhdl_anchor.isCoherent());

   printf("\n");
   printf("DHDL_INSERT(1) at head:\n");
   dhdl_anchor.insert(NULL, &gen_array[1-1], &gen_array[1-1]);
   show_DHDL(&dhdl_anchor);
   assert(dhdl_anchor.isOnList(&gen_array[1-1]));
   assert(dhdl_anchor.isCoherent());

   printf("\n");
   printf("DHDL_INSERT(%d) at tail:\n", DIM);
   dhdl_anchor.insert(dhdl_anchor.getTail(), &gen_array[DIM-1], &gen_array[DIM-1]);
   show_DHDL(&dhdl_anchor);
   assert(dhdl_anchor.isOnList(&gen_array[DIM-1]));
   assert(dhdl_anchor.isCoherent());

   printf("\n");
   printf("DHDL_INSERT(5) after(4):\n");
   dhdl_anchor.insert(&gen_array[4-1], &gen_array[5-1], &gen_array[5-1]);
   show_DHDL(&dhdl_anchor);
   assert(dhdl_anchor.isOnList(&gen_array[5-1]));
   assert(dhdl_anchor.isCoherent());

   printf("\n");
   printf("DHDL_REMOVE(5..8):\n");
   dhdl_anchor.remove(&gen_array[5-1], &gen_array[8-1]);
   show_DHDL(&dhdl_anchor);
   assert(dhdl_anchor.isOnList(&gen_array[4-1]));
   assert(!dhdl_anchor.isOnList(&gen_array[5-1]));
   assert(!dhdl_anchor.isOnList(&gen_array[6-1]));
   assert(!dhdl_anchor.isOnList(&gen_array[7-1]));
   assert(!dhdl_anchor.isOnList(&gen_array[8-1]));
   assert(dhdl_anchor.isOnList(&gen_array[9-1]));
   assert(dhdl_anchor.isCoherent());
// debug_DHDL(&dhdl_anchor);
// debug_DHDL_array(gen_array, DIM);

   printf("\n");
   printf("DHDL_INSERT(5..8):\n");
   dhdl_anchor.insert(&gen_array[4-1], &gen_array[5-1], &gen_array[8-1]);
   show_DHDL(&dhdl_anchor);
   assert(dhdl_anchor.isOnList(&gen_array[4-1]));
   assert(dhdl_anchor.isOnList(&gen_array[5-1]));
   assert(dhdl_anchor.isOnList(&gen_array[6-1]));
   assert(dhdl_anchor.isOnList(&gen_array[7-1]));
   assert(dhdl_anchor.isOnList(&gen_array[8-1]));
   assert(dhdl_anchor.isOnList(&gen_array[9-1]));
   assert(dhdl_anchor.isCoherent());

   printf("\n");
   printf("DHDL_REMOVE(1..12):\n");
   dhdl_anchor.remove(&gen_array[1-1], &gen_array[12-1]);
   show_DHDL(&dhdl_anchor);
   for(i=0; i<12; i++)
     assert(!dhdl_anchor.isOnList(&gen_array[i]));
   assert(dhdl_anchor.isCoherent());

   printf("\n");
   printf("DHDL_INSERT(1..12):\n");
   dhdl_anchor.insert(NULL, &gen_array[1-1], &gen_array[12-1]);
   show_DHDL(&dhdl_anchor);
   for(i=0; i<12; i++)
     assert(dhdl_anchor.isOnList(&gen_array[i]));
   assert(dhdl_anchor.isCoherent());
   dhdl_anchor.reset();

   //-------------------------------------------------------------------------
   // SHSL tests
   //-------------------------------------------------------------------------
   printf("\n");
   printf("SHSL Storage:\n");
   printf("%8ld Sizeof(SHSL_List)\n", (long)sizeof(SHSL_List<GEN_block>));
   printf("%8ld Sizeof(SL_Link)\n", (long)sizeof(SHSL_List<GEN_block>::Link));
   printf("%8zd Sizeof(SL_Link)\n", (size_t)_OFFSET_(SHSL_Block, shsl_value));
// printf("%8ld Sizeof(SL_Link)\n", (long)offsetof(SHSL_Block, shsl_value));

   printf("\n");
   printf("Null SHSL:\n");
   show_SHSL(&shsl_anchor);

   //-------------------------------------------------------------------------
   // SHSL LIFO test
   //-------------------------------------------------------------------------
   printf("\n");
   printf("SHSL_LIFO test (1..%d):\n", DIM);
   for(i=0; i<DIM; i++)
   {
     shsl_anchor.lifo(&gen_array[i]);
     show_SHSL(&shsl_anchor);
   }
   for(i=0; i<DIM; i++)
   {
     assert(shsl_anchor.isOnList(&gen_array[i]));
   }
   assert(shsl_anchor.isCoherent());

   for(;;)
   {
     shsl_link= shsl_anchor.remq();
     if( shsl_link == NULL )
       break;
     show_SHSL(&shsl_anchor, shsl_link);
   }
   for(i=0; i<DIM; i++)
   {
     assert(!shsl_anchor.isOnList(&gen_array[i]));
   }
   assert(shsl_anchor.isCoherent());

   //-------------------------------------------------------------------------
   // SHSL FIFO test
   //-------------------------------------------------------------------------
   printf("\n");
   printf("SHSL_FIFO test:\n");
   for(i=0; i<DIM; i++)
   {
     shsl_anchor.fifo(&gen_array[i]);
     show_SHSL(&shsl_anchor);
   }
   for(i=0; i<DIM; i++)
   {
     assert(shsl_anchor.isOnList(&gen_array[i]));
   }
   assert(shsl_anchor.isCoherent());

   for(;;)
   {
     shsl_link= shsl_anchor.remq();
     if( shsl_link == NULL )
       break;
     show_SHSL(&shsl_anchor, shsl_link);
   }
   for(i=0; i<DIM; i++)
   {
     assert(!shsl_anchor.isOnList(&gen_array[i]));
   }
   assert(shsl_anchor.isCoherent());

   // Leave the list populated
   for(i=0; i<DIM; i++)
     shsl_anchor.fifo(&gen_array[i]);

   //-------------------------------------------------------------------------
   // DHSL tests
   //-------------------------------------------------------------------------
   printf("\n");
   printf("DHSL Storage:\n");
   printf("%8ld Sizeof(DHSL_List)\n", (long)sizeof(DHSL_List<GEN_block>));
   printf("%8ld Sizeof(DHSL_Link)\n", (long)sizeof(DHSL_List<GEN_block>::Link));
   printf("%8zd Sizeof(DHSL_Link)\n", (size_t)_OFFSET_(DHSL_Block, dhsl_value));
// printf("%8ld Sizeof(DHSL_Link)\n", (long)offsetof(DHSL_Block, dhsl_value));

   printf("\n");
   printf("Null DHSL:\n");
   show_DHSL(&dhsl_anchor);

   //-------------------------------------------------------------------------
   // DHSL LIFO test
   //-------------------------------------------------------------------------
   printf("\n");
   printf("DHSL_LIFO test (1..%d):\n", DIM);
   for(i=0; i<DIM; i++)
   {
     dhsl_anchor.lifo(&gen_array[i]);
     show_DHSL(&dhsl_anchor);
   }
   for(i=0; i<DIM; i++)
   {
     assert(dhsl_anchor.isOnList(&gen_array[i]));
   }
   assert(dhsl_anchor.isCoherent());

   for(;;)
   {
     dhsl_link= dhsl_anchor.remq();
     if( dhsl_link == NULL )
       break;
     show_DHSL(&dhsl_anchor, dhsl_link);
   }
   for(i=0; i<DIM; i++)
   {
     assert(!dhsl_anchor.isOnList(&gen_array[i]));
   }
   assert(dhsl_anchor.isCoherent());

   //-------------------------------------------------------------------------
   // DHSL FIFO test
   //-------------------------------------------------------------------------
   printf("\n");
   printf("DHSL_FIFO test:\n");
   for(i=0; i<DIM; i++)
   {
     dhsl_anchor.fifo(&gen_array[i]);
     show_DHSL(&dhsl_anchor);
   }
   for(i=0; i<DIM; i++)
   {
     assert(dhsl_anchor.isOnList(&gen_array[i]));
   }
   assert(dhsl_anchor.isCoherent());

   for(;;)
   {
     dhsl_link= dhsl_anchor.remq();
     if( dhsl_link == NULL )
       break;
     show_DHSL(&dhsl_anchor, dhsl_link);
   }
   for(i=0; i<DIM; i++)
   {
     assert(!dhsl_anchor.isOnList(&gen_array[i]));
   }
   assert(dhsl_anchor.isCoherent());

   //-------------------------------------------------------------------------
   // AUX tests
   //-------------------------------------------------------------------------
   printf("\n");
   printf("AUX Storage:\n");
   printf("%8ld Sizeof(Link)\n", (long)sizeof(AUX_Link));
   printf("%8zd Offset(Link)\n", (size_t)_OFFSET_(GEN_block, aux_link));
// printf("%8ld Offset(Link)\n", (long)offsetof(GEN_block, aux_link));

   printf("\n");
   printf("Null AUX:\n");
   show_AUX(&aux_anchor);

   //-------------------------------------------------------------------------
   // AUX LIFO test
   //-------------------------------------------------------------------------
   printf("\n");
   printf("AUX_LIFO test (1..%d):\n", DIM);
   for(i=0; i<DIM; i++)
   {
     aux_anchor.lifo(&gen_array[i].aux_link);
     show_AUX(&aux_anchor);
   }
   for(i=0; i<DIM; i++)
   {
     assert(aux_anchor.isOnList(&gen_array[i].aux_link));
   }
   assert(aux_anchor.isCoherent());

   for(;;)
   {
     aux_ptr= (AUX_Link*)aux_anchor.remq();
     if( aux_ptr == NULL )
       break;
     show_AUX(&aux_anchor, aux_ptr);
   }
   for(i=0; i<DIM; i++)
   {
     assert(!aux_anchor.isOnList(&gen_array[i].aux_link));
   }
   assert(aux_anchor.isCoherent());

   //-------------------------------------------------------------------------
   // AUX FIFO test
   //-------------------------------------------------------------------------
   printf("\n");
   printf("AUX_FIFO test:\n");
   for(i=0; i<DIM; i++)
   {
     aux_anchor.fifo(&gen_array[i].aux_link);
     show_AUX(&aux_anchor);
   }
   for(i=0; i<DIM; i++)
   {
     assert(aux_anchor.isOnList(&gen_array[i].aux_link));
   }
   assert(aux_anchor.isCoherent());

   for(;;)
   {
     aux_ptr= (AUX_Link*)aux_anchor.remq();
     if( aux_ptr == NULL )
       break;
     show_AUX(&aux_anchor, aux_ptr);
   }
   for(i=0; i<DIM; i++)
   {
     assert(!aux_anchor.isOnList(&gen_array[i].aux_link));
   }
   assert(aux_anchor.isCoherent());

   //-------------------------------------------------------------------------
   // SORT configuration
   //-------------------------------------------------------------------------
   printf("\n");
   const char* info= "NOT";
#ifdef USE_STANDALONE_SORT_LINK
   info= "IS";
#endif
   printf("SORT: USE_STANDALONE_SORT_LINK(%s DEFINED)\n", info);

#ifndef USE_STANDALONE_SORT_LINK
#if 0 // This no longer compiles with class SORT_List<void> : PRIVATE ...
   #undef DHDL_List
   try {                            // Verify SortCastException
     DHDL_List<void>::Link* link= &gen_array[0];
     sort_anchor.fifo(link);        // This can't work
     printf("%4d NOT EXPECTED\n", __LINE__);
   } catch(const char* X) {         // This is expected
     if( strcmp(X, "SortCastException") == 0 )
       printf("%4d As expected, exception(SortCastException)\n", __LINE__);
     else
       printf("Exception(%s), but expected(SortCastException)\n", X);
   } catch(...) {
     printf("Exception(...) NOT EXPECTED\n");
   }

   sort_anchor.reset();
#endif
#endif

   //-------------------------------------------------------------------------
   // SORT LIFO test
   //-------------------------------------------------------------------------
   printf("\n");
   printf("SORT_LIFO test (1..%d):\n", DIM);
   for(i=0; i<DIM; i++)
     sort_anchor.lifo(&gen_array[i]);
   show_SORT(&sort_anchor);
   sort_anchor.sort();
   show_SORT(&sort_anchor);

   for(i=0; i<DIM; i++)
     assert(sort_anchor.isOnList(&gen_array[i]));
   assert(sort_anchor.isCoherent());
   sort_anchor.reset();

   //-------------------------------------------------------------------------
   // SORT FIFO test
   //-------------------------------------------------------------------------
   printf("\n");
   printf("SORT_FIFO test:\n");
   for(i=0; i<DIM; i++)
     sort_anchor.fifo(&gen_array[i]);
   show_SORT(&sort_anchor);
   sort_anchor.sort();
   show_SORT(&sort_anchor);

   for(i=0; i<DIM; i++)
     assert(sort_anchor.isOnList(&gen_array[i]));
   assert(sort_anchor.isCoherent());
   sort_anchor.reset();

   return 0;
}

