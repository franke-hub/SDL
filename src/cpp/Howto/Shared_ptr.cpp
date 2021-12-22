//----------------------------------------------------------------------------
//
//       Copyright (C) 2020-2021 Frank Eskesen.
//
//       This file is free content, distributed under the MIT license.
//       (See accompanying file LICENSE.MIT or the original contained
//       within https://opensource.org/licenses/MIT)
//
//----------------------------------------------------------------------------
//
// Title-
//       Shared_ptr.cpp
//
// Purpose-
//       Sample program: How to use std::shared_ptr
//
// Last change date-
//       2021/11/14
//
//----------------------------------------------------------------------------
#include <memory>                   // For std::shared_ptr

#include <stdio.h>                  // For printf
#include <string.h>                 // For strcmp

//----------------------------------------------------------------------------
// Struct: Thing
//----------------------------------------------------------------------------
struct Thing {                      // A testable Thing
const char*            one;         // Thing one
const char*            two;         // Thing two
const char*            three;       // Thing three

   ~Thing( void )
{  printf("Thing(%p)::~Thing(%s)\n", this, three ? three : ""); }

   Thing( void )
:  one("Thing one"), two("Thing two"), three(nullptr)
{  printf("Thing(%p)::Thing()\n", this); };

   Thing(
     const char*       three)       // Constructor with option
:  one("Thing one"), two("Thing two"), three(three)
{  printf("Thing(%p)::Thing(%s)\n", this, three); }

void
   test( void )                     // Test this Thing
{
   printf("Test(%p)::test\n", this);

   if( strcmp(one, "Thing one") != 0 || strcmp(two, "Thing two") != 0 )
   {
     printf("Bad Thing(%p,%p)\n", one, two);
     printf("Bad Thing(%s,%s)\n", one, two);
   }

   if( three )                      // If optional parameter set
     printf("Thing(%p).three(%s)\n", this, three);
}
}; // struct Thing

using Thing_p= std::shared_ptr<Thing>;

//----------------------------------------------------------------------------
//
// Subroutine-
//       give  (FOR DOCUMENTATION ONLY - NEVER CALLED)
//
// Purpose-
//       Disable a share_ptr, taking back ownership of the object.
//
// Implementation note-
//       Once a share_ptr owns an object, there is no normal way of giving it
//       back. Although you might be able to implement some hack that works
//       for a particular implementation, you shouldn't.
//
//----------------------------------------------------------------------------
static inline void                  // Give up ownership of an object
   give(                            // Give up ownership Thing*
     Thing_p&          thing)       // From this Thing_p
{
   (void)thing;                     // Parameter ignored
   throw "This is unreasonable, perhaps impossible";
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       take
//
// Purpose-
//       Take ownership of a Thing, creating std::shared_ptr<Thing>.
//
//----------------------------------------------------------------------------
static inline Thing_p               // Take ownership of an object
   take(                            // Take ownership of
     Thing*            thing)       // This Thing
{
   std::shared_ptr<Thing> share(thing); // Take ownership in share
   printf("take(%p) share(%p).%ld->%p\n", thing, &share,
          share.use_count(), share.get());

   // This return uses copy by value to return the Thing_p.
   // The returned Thing_p is a copy of share, NOT share itself.
   // (Many compilers can optimize out intermediate copies of share on return.)
   return share;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       call_by_ref
//       call_by_val
//
// Purpose-
//       Demonstrate call by reference vs call by value
//
//----------------------------------------------------------------------------
static inline void
   call_by_ref(                     // Test call by reference
     Thing_p&          thing)       // This Thing
{
   {{{{
       printf("call_by_ref(%p).%ld...\n", &thing, thing.use_count());

       Thing_p share(thing);        // Use copy constructor
       share= thing;                // Copied shared_ptr
       share->test();

       // The use count is two. There is no intermediate Thing_p.
       printf("thing(%p).%ld->%p\n", &thing, thing.use_count(), thing.get());
       printf("share(%p).%ld->%p\n", &share, share.use_count(), share.get());
   }}}}

   printf("...call_by_ref(%p).%ld\n", &thing, thing.use_count());
}

static inline void
   call_by_val(                     // Test call by value
     Thing_p           thing)       // This Thing
{
   {{{{
       printf("call_by_val(%p).%ld...\n", &thing, thing.use_count());

       Thing_p share(thing);        // Use copy constructor
       share= thing;                // Copied shared_ptr
       share->test();

       // The use count is three. An intermediate Thing_p copy by value
       // argument exists and remains in-scope.
       printf("thing(%p).%ld->%p\n", &thing, thing.use_count(), thing.get());
       printf("share(%p).%ld->%p\n", &share, share.use_count(), share.get());

       // We can use the value argument for any purpose.
       thing.reset();               // Reset the intermediate copy
       printf("thing.reset\n");
       printf("thing(%p).%ld->%p\n", &thing, thing.use_count(), thing.get());
       printf("share(%p).%ld->%p\n", &share, share.use_count(), share.get());
   }}}}

   printf("...call_by_val(%p).%ld\n", &thing, thing.use_count());
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       demo
//
// Purpose-
//       Demonstrate proper shared_ptr usage.
//
//----------------------------------------------------------------------------
inline void
   demo( void )                     // Demonstrate proper shared_ptr usage
{
   printf("demo...\n");

{{
  {{
   printf("{{ scope..\n");
   // A less efficient way to create a std::shared_ptr<T>
   // In this instance the Thing's reference control structure is allocated
   // separately from the Thing.
   std::shared_ptr<Thing> thing(new Thing("First Thing"));
   printf("..scope }}\n");
  }}
   printf("\n");

   // A more efficient way to create a std::shared_ptr<T>
   // In this instance the Thing's reference control structure is allocated
   // along with Thing.
   // Downside: The object's new and delete methods are not called.
   // Untested: Use std::allocate_shared with your custom Allocator.
   std::shared_ptr<Thing> thing= std::make_shared<Thing>("demo Thing");
   printf("INIT thing(%p).%ld->%p\n", &thing, thing.use_count(),
          thing.get());

  {{
   printf("{{ scope..\n");
   // Demonstrate shared pointer usage
   Thing_p share;                   // Empty shared_ptr<Thing>
   printf("NULL share(%p).%ld->%p\n", &share, share.use_count(), share.get());
   share= thing;
   printf("FULL share(%p).%ld->%p\n", &share, share.use_count(), share.get());
   printf("COPY thing(%p).%ld->%p\n", &thing, thing.use_count(), thing.get());

   // We use a shared_ptr just like a regular pointer
   printf("one(%s) two(%s) three(%s)\n", share->one, share->two, share->three);
   share->test();
   printf("..scope }}\n");
  }}
   printf("HAVE thing(%p).%ld->%p\n", &thing, thing.use_count(), thing.get());
}}

   printf("...demo\n");
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
extern int                          // Return code
   main(                            // Mainline code
     int               argc,        // Argument count
     char*             argv[])      // Argument array (Unused)
{
   (void)argv;                      // Unused parameter
   printf("main...\n");

   printf("\n");                    // Demonstrate proper usage
   demo();

   printf("\n");                    // Demonstrate ownership of object
   Thing* thing= new Thing();       // Create a Thing
   thing->test();                   // Test it out
   std::shared_ptr<Thing> share= take(thing); // Take ownership of the Thing

   printf("\n");                    // Demo call by reference vs call by value
   printf("main.share(%p).%ld->%p\n", &share, share.use_count(), share.get());
   call_by_ref(share);
   printf("\n");
   call_by_val(share);
   printf("main.share(%p).%ld->%p\n", &share, share.use_count(), share.get());

   //-------------------------------------------------------------------------
   // USAGE ERROR: Duplicate ownership
   if( argc > 1 ) {                 // Any argument drives this
     {{{{
       printf("\nRunning ERROR demo, duplicated pointer ownership\n");
       Thing_p taken= take(thing);    // Take ownership of thing
       printf("taken(%p).%ld->%p\n", &taken, taken.use_count(), taken.get());

       printf("share(%p).%ld->%p\n", &share, share.use_count(), share.get());
       share.reset();                 // We reset the share, deleting thing
       printf("share(%p).%ld->%p\n", &share, share.use_count(), share.get());
       printf("taken(%p).%ld->%p\n", &taken, taken.use_count(), taken.get());
     }}}} // taken goes out of scope, thus deleting thing once again

     // More often than not, the duplicate delete aborts.
     printf("UNEXPECTED:\n");       // We didn't abort
   }

   printf("\n");                    // Exit, share going out of scope
   printf("...main\n");
   return 0;
}
