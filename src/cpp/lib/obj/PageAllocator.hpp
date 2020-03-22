//----------------------------------------------------------------------------
//
//       Copyright (c) 2018 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       PageAllocator.hpp
//
// Purpose-
//       Aligned storage allocator definition and implementation.
//
// Last change date-
//       2018/01/01
//
//----------------------------------------------------------------------------

#ifdef _OS_WIN
  #define aligned_alloc _aligned_malloc
#endif

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static std::bad_alloc  bad_alloc;   // Prebuilt bad_alloc

static unsigned        index2size[16]=
                           {      4096 // 0x00001000 [ 0] -
                           ,      8192 // 0x00002000 [ 1]
                           ,     16384 // 0x00004000 [ 2]
                           ,     32768 // 0x00008000 [ 3]
                           ,     65536 // 0x00010000 [ 4] -
                           ,    131072 // 0x00020000 [ 5]
                           ,    262144 // 0x00040000 [ 6]
                           ,    524288 // 0x00080000 [ 7]
                           ,   1048576 // 0x00100000 [ 8] -
                           ,   2097152 // 0x00200000 [ 9]
                           ,   4194304 // 0x00400000 [10]
                           ,   8388608 // 0x00800000 [11]
                           ,  16777216 // 0x01000000 [12] -
                           ,  33554432 // 0x02000000 [13] -
                           ,  67108864 // 0x04000000 [14] (Unused)
                           , 134217728 // 0x08000000 [15] (Unused)
                           }; // size   index

//----------------------------------------------------------------------------
//
// Subroutine-
//       checkstop
//
// Purpose-
//       Handle check fault
//
//----------------------------------------------------------------------------
static inline void
   checkstop(                       // Handle fault
     const char*       mess)        // Error text
{
   debugf("checkstop(%s)\n", mess);
   Exception::abort("%s", mess);
}

//----------------------------------------------------------------------------
//
// Class-
//       PageAllocator
//
// Purpose-
//       Page Allocator descriptor.
//
//----------------------------------------------------------------------------
class PageAllocator {               // Page Allocator descriptor
//----------------------------------------------------------------------------
// Allocator::Typedefs and enumerations
//----------------------------------------------------------------------------
protected:
enum {LOG_SIZE_ZERO= 12};           // log2(SIZE_ZERO)
enum {SIZE_COUNT= 14};              // Number of supported allocation sizes
enum {SIZE_ZERO=  4096};            // Minimum PageAllocator size
enum {BULK_SIZE=  (SIZE_ZERO << SIZE_COUNT) - 16}; // Bulk allocation length

enum {USE_CHECK= false};            // Use checking logic?
enum {USE_HCDM= false};             // Use Hard Core Debug Mode?
enum {USE_SYSTEM_ALLOC= false};     // Always use aligned_alloc?

//----------------------------------------------------------------------------
// Allocator::struct Bulk
// Allocator::struct Page
//----------------------------------------------------------------------------
protected:
struct Page : public List<Page>::Link { // Aligned storage available Page
}; // struct Page

struct Bulk : public List<Bulk>::Link { // Aligned storage Bulk allocation
   void*               storage;     // Storage allocation physical origin
   char*               origin;      // Storage allocation origin
   char*               ending;      // Storage allocation ending address (+1)

   Latch               pageLatch;   // Protects all Page lists
   List<Page>          pageList[SIZE_COUNT]; // Free Pages by size

void
   allocate_prefix(                 // Allocate from front
     char*&            origin,      // At this address
     unsigned          thisIndex,   // From this index
     unsigned          nextIndex)   // Upto this index
{
   unsigned thisLength= index2size[thisIndex];
   unsigned nextLength= index2size[nextIndex];
   while( intptr_t(origin) & intptr_t(nextLength-1) )
   {
     Page* page= (Page*)origin;
     if( USE_HCDM ) debugf("[%2d] %p 0x%.8x\n", thisIndex, page, thisLength);
     pageList[thisIndex].fifo(page);
     origin += thisLength;
   }
}

void
   allocate_suffix(                 // Allocate from back
     char*&            origin,      // At this address
     const char*       ending,      // To this address
     unsigned          thisIndex)   // For this index
{
   unsigned thisLength= index2size[thisIndex];
   while( (origin + thisLength) <= ending )
   {
     Page* page= (Page*)origin;
     if( USE_HCDM ) debugf("[%2d] %p 0x%.8x\n", thisIndex, page, thisLength);
     pageList[thisIndex].fifo(page);
     origin += thisLength;
   }
}

inline void                         // Must hold pageLatch
   check_range(                     // Range check
     unsigned          index,       // For this index
     void*             page)        // And this page
{  if( USE_CHECK )
   {
     if( page >= origin && page < ending )
       return;

     debug_locked();
     checkstop(
       built_in::to_string("Bulk(%p) [%d] page(%p) not in range(%p..%p)",
                           this, index, page, origin, ending).c_str()
                           );
   }
}

inline void                         // Must hold pageLatch
   check( void )                    // Verification check
{  if( USE_CHECK )
   {
     for(int i= 0; i<SIZE_COUNT; i++) // Check list consistency
     {
       Page* prev= nullptr;
       Page* page= pageList[i].get_head();
       while( page )
       {
         check_range(i, page);

         if( page->get_prev() != prev )
         {
           debug_locked();
           checkstop(
             built_in::to_string("Bulk(%p) [%d] page(%p)->prev(%p)!=prev(%p)",
                                 this, i, page, page->get_prev(), prev).c_str()
                                 );
         }

         prev= page;
         page= page->get_next();
       }
     }
   }
}

inline void                         // Must hold pageLatch
   debug_locked( void )             // Debug bulk block
{
   debugf("Bulk(%p) debug: range(%p:%p)\n",
          this, origin, ending);

   unsigned bulkTotal= 0;
   for(int i= 0; i<SIZE_COUNT; i++)
   {
     unsigned size= index2size[i];
     unsigned count= 0;
     unsigned pageTotal= 0;
     Page* prev= nullptr;
     Page* page= pageList[i].get_head();
     while( page )
     {
       if( (char*)page < origin || (char*)page >= ending )
       {
         debugf("..[%2d] %p Not in range\n", i, page);
         return;
       }

       if( USE_HCDM )
         debugf("..[%2d] %3d (%p)<-(%p)->(%p)\n",
                i, count, page->get_prev(), page, page->get_next());

       if( page->get_prev() != prev )
       {
         debugf("[%2d] %p->prev(%p) != prev(%p)\n", i, page,
                page->get_prev(), prev);
         return;
       }

       count++;
       pageTotal += size;
       prev= page;
       page= page->get_next();
     }
     bulkTotal += pageTotal;

     if( pageTotal > 0 )
       debugf("..[%2d] %p 0x%.8x (%7d x 0x%.8x==%8d)\n", i,
              pageList[i].get_head(), pageTotal, count, size, size);
   }
   debugf("..[==] %p 0x%.8x available, 0x%.8zx used of 0x%.8zx\n",
          this, bulkTotal, size_t((ending - origin) - bulkTotal),
          size_t((ending - origin)));
}

inline void                         // CANNOT hold pageLatch
   debug( void )                    // Debug bulk block
{
   std::lock_guard<decltype(pageLatch)> lock(pageLatch);
   check();
   debug_locked();
}
}; // struct Bulk

//----------------------------------------------------------------------------
// PageAllocator::Attributes
//----------------------------------------------------------------------------
protected:
STATISTIC              stat_bulk;   // Number of allocate_bulk calls
STATISTIC              stat_find;   // Number of find_page calls
STATISTIC              stat_free;   // Number of free_page calls

SharedLatch            masterShare; // Master latch: Protects attributes

Bulk*                  bulkSize[SIZE_COUNT]; // Available Bulk entry by size
List<Bulk>             bulkList;    // List of Bulk allocation blocks

//----------------------------------------------------------------------------
// PageAllocator::Destructor/Constructor/Assignment
//----------------------------------------------------------------------------
public:
   PageAllocator( void )            // Default Constructor
:  stat_bulk(0), stat_find(0), stat_free(0)
,  masterShare(), bulkList()
{  if( false ) debugf("PageAllocator(%p)::PA()\n", this);

   for(int i= 0; i<SIZE_COUNT; i++)
     bulkSize[i]= nullptr;
}

   ~PageAllocator( void )           // Destructor
{  if( false ) debugf("PageAllocator(%p)::~PA()\n", this);
}

// Disallowed: Copy constructor, assignment operator
   PageAllocator(const PageAllocator&) = delete;
PageAllocator& operator=(const PageAllocator&) = delete;

//----------------------------------------------------------------------------
// PageAllocator::debug Debugging display
//----------------------------------------------------------------------------
public:
void
   debug( void )                    // (Internal) debugging display
{  debugf("PageAllocator(%p)::debug()\n", this);
   debugf("USE_CHECK(%s) USE_SYSTEM_ALLOC(%s)\n",
          USE_CHECK ? "true" : "false", USE_SYSTEM_ALLOC ? "true" : "false");
   debugf("stat_bulk(%zd) stat_find(%zd) stat_free(%zd)\n",
          stat_bulk.load(), stat_find.load(), stat_free.load());

   std::lock_guard<decltype(masterShare)> share(masterShare);

   debugf("bulkSize:\n");
   for(int i= 0; i<SIZE_COUNT; i++)
   {
     debugf("[%2d] %p\n", i, bulkSize[i]);
   }

   Bulk* bulk= bulkList.get_head();
   if( bulk )
     debugf("bulkList(%p):\n", &bulkList);
   else
     debugf("bulkList(%p): *EMPTY*\n", &bulkList);
   while( bulk )
   {
     bulk->debug();
     bulk= bulk->get_next();
   }
}

//----------------------------------------------------------------------------
// PageAllocator::Methods
//----------------------------------------------------------------------------
public:
protected:
Bulk*                               // The resultant Bulk*
   allocate_bulk( void )            // Bulk allocation and initialization
{
   statistic(stat_bulk);            // Statistical counter

   char* storage= (char*)malloc(BULK_SIZE); // Allocate a new Bulk block
   if( storage == nullptr ) throw bad_alloc; // Exception if failure
   char* origin= storage;           // First available address
   char* ending= origin + BULK_SIZE; // Last available address (+1)

   intptr_t prefix_space= intptr_t(SIZE_ZERO)
                        - (intptr_t(origin) & intptr_t(SIZE_ZERO-1));
   intptr_t suffix_space= intptr_t(ending-1) & intptr_t(SIZE_ZERO-1);

   //-------------------------------------------------------------------------
   // Allocate the Bulk descriptor from the Bulk area. It goes either at the
   // beginning or the end of the allocated space.
   Bulk* bulk= (Bulk*)origin;
   if( prefix_space >= sizeof(Bulk) || suffix_space < sizeof(Bulk) )
   {
     // bulk= (Bulk*)origin;
     origin += sizeof(Bulk);
   } else {
     ending -= sizeof(Bulk);
     bulk= (Bulk*)ending;
   }

   memset(bulk, 0, sizeof(Bulk));
   origin= (char*)((intptr_t(origin) + SIZE_ZERO - 1) & ~intptr_t(SIZE_ZERO-1));
   ending= (char*)(intptr_t(ending) & ~intptr_t(SIZE_ZERO - 1));
   bulk->storage= storage;
   bulk->origin= origin;
   bulk->ending= ending;

   //-------------------------------------------------------------------------
   // Allocate storage
   if( USE_HCDM ) debugf("Bulk(%p) Storage allocation:\n", bulk);
   bulk->allocate_prefix(origin, 0, 4);
   bulk->allocate_prefix(origin, 4, 8);
   bulk->allocate_prefix(origin, 8, 12);
   bulk->allocate_prefix(origin, 12, 13);

   bulk->allocate_suffix(origin, ending, 13);
   bulk->allocate_suffix(origin, ending, 12);
   bulk->allocate_suffix(origin, ending, 8);
   bulk->allocate_suffix(origin, ending, 4);
   bulk->allocate_suffix(origin, ending, 0);

   if( USE_HCDM ) bulk->debug();
   return bulk;
}

void                                // masterShare exclusive Latch required
   reset_bulkSize( void )           // Reset the bulkSize table
{
   for(unsigned sizeIndex= 0; sizeIndex < SIZE_COUNT; sizeIndex++)
     bulkSize[sizeIndex]= nullptr;

   Bulk* bulk= bulkList.get_head();
   for(unsigned sizeIndex= 0; sizeIndex < SIZE_COUNT; sizeIndex++)
   {
     while( bulk )
     { std::lock_guard<decltype(bulk->pageLatch)> lock(bulk->pageLatch);

       for(unsigned listIndex= sizeIndex; listIndex < SIZE_COUNT; listIndex++)
       {
         if( bulk->pageList[listIndex].get_head() )
         {
           bulkSize[sizeIndex]= bulk;
           break;
         }
       }

       if( bulkSize[sizeIndex] )
         break;

       bulk= bulk->get_next();
     }
   }
}

public:
void*                               // The allocated extention Page
   find_page(                       // Allocate extention Page
     size_t            size)        // Of this length
{
   statistic(stat_find);            // Statistical counter

   if( USE_SYSTEM_ALLOC || size < SIZE_ZERO || size > (BULK_SIZE >> 1) )
   {
     void* page= ::aligned_alloc(size, size); // Allocate an aligned page
     if( !page ) throw bad_alloc;   // Handle allocation failure
     return page;
   }

   unsigned index= log2(size >> LOG_SIZE_ZERO);
   while( true )
   {
     {{{{ std::lock_guard<decltype(masterShare)> share(masterShare);
       Bulk* bulk= bulkSize[index];
       if( bulk )
       {
         std::lock_guard<decltype(bulk->pageLatch)> lock(bulk->pageLatch);
         Page* page= bulk->pageList[index].remq();
         if( page )
         {
           bulk->check_range(index, page);

           if( USE_HCDM )
             debugf("[%2d] %p< PA(%p)::find_page(%zx==%zd)\n",
                    index, page, this, size, size);
           return page;
         }
       }
     }}}}

     //-----------------------------------------------------------------------
     // Unable to allocate using shared master Latch. Obtain exclusive Latch.
     ExclusiveLatch masterLatch(masterShare);
     {{{{ std::lock_guard<decltype(masterLatch)> master(masterLatch);
       Bulk* bulk= bulkSize[index];
       if( bulk )
       {
         std::lock_guard<decltype(bulk->pageLatch)> lock(bulk->pageLatch);
         Page* page= bulk->pageList[index].remq();
         if( page )
         {
           bulk->check_range(index, page);

           if( USE_HCDM )
             debugf("[%2d] %p< PA(%p)::find_page(%zx==%zd)\n",
                    index, page, this, size, size);
           return page;
         }

         //-------------------------------------------------------------------
         // We have a Bulk allocation but no Pages of the proper size.
         // See if a larger Page is available to be split.
         unsigned bigger= index+1;
         while( bigger < SIZE_COUNT )
         {
           page= bulk->pageList[bigger].remq();
           if( page )
           {
             size_t size= index2size[bigger-1];
             bulk->pageList[bigger-1].fifo(page);
             page= (Page*)((char*)page + size);

             while( --bigger > index )
             {
               bulk->pageList[bigger-1].fifo(page);

               size= index2size[bigger-1];
               page= (Page*)((char*)page + size);
             }
             return page;
           }

           bigger++;
         }
       }

       //---------------------------------------------------------------------
       // If required, allocate a new Bulk block
       reset_bulkSize();           // Reset the bulkSize array
       if( bulkSize[index] == nullptr )
       {
         bulk= allocate_bulk();
         bulkList.lifo(bulk);
         reset_bulkSize();
       }
     }}}}

   if( USE_HCDM ) debug();
   }
}

void
   free_page(                       // Delete Page
     void*             page,        // The page to delete
     size_t            size)        // The page length
{
   statistic(stat_free);            // Statistical counter

   if( USE_SYSTEM_ALLOC || size < SIZE_ZERO || size > (BULK_SIZE >> 1) )
   {
     ::free(page);
     return;
   }

   unsigned index= log2(size >> LOG_SIZE_ZERO);
   if( USE_HCDM )
     debugf("[%2d] %p> PA(%p)::free_page(%zx==%zd)\n",
            index, page, this, size, size);
   {{{{ std::lock_guard<decltype(masterShare)> share(masterShare);
     char* addr= static_cast<char*>(page);
     Bulk* bulk= bulkList.get_head();
     while( bulk )
     {
       if( addr >= bulk->origin && addr < bulk->ending )
       { std::lock_guard<decltype(bulk->pageLatch)> lock(bulk->pageLatch);

         bulk->pageList[index].fifo(static_cast<Page*>(page));
         return;
       }

       bulk= bulk->get_next();
     }
   }}}}

   // The page to delete is not in any Bulk block
   debug();
   Exception::abort("PageAllocator::free(%p) not allocated", page);
}
}; // class PageAllocator

