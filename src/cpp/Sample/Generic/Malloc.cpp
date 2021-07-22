//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2021 Frank Eskesen.
//
//       This file is free content, distributed under the MIT license.
//       (See accompanying file LICENSE.MIT or the original contained
//       within https://opensource.org/licenses/MIT)
//
//-----------------------------------------------------------------------------
//
// Title-
//       Malloc.cpp
//
// Purpose-
//       Storage allocation/release sample.
//
// Last change date-
//       2021/07/17
//
//----------------------------------------------------------------------------
#include "logger.h"
#include <stdlib.h>

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#define DIM_DATA 1000000

//----------------------------------------------------------------------------
//
// Struct-
//       Data
//
// Purpose-
//       Allocated data
//
//----------------------------------------------------------------------------
struct Data
{
   Data*               next;        // -> Next element
   unsigned            random;      // Random value
   unsigned char       data[504];   // Data bytes
}; // struct Data

//----------------------------------------------------------------------------
//
// Subroutine-
//       test00
//
// Purpose-
//       Simple Malloc/Free
//
//----------------------------------------------------------------------------
static int                          // Error count
   test00( void )                   // Function test
{
   void*               ptrV;

   syslog(LOG_INFO, "MALLOC: test00 (basic)\n");
   ptrV= malloc(8192);
   free(ptrV);

   syslog(LOG_INFO, "MALLOC: Test free(NULL)\n");
   ptrV= NULL;
   free(ptrV);

   return 0;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test01
//
// Purpose-
//       Large allocation test
//
//----------------------------------------------------------------------------
static int                          // Error count
   test01( void )                   // Function test
{
   int                 errorCount= 0; // Error cunter

   Data*               head;        // First Data pointer
   Data*               tail;        // Last Data pointer
   Data*               data;        // Current Data pointer
   unsigned            last;        // Last random value

   size_t              i, j;

   syslog(LOG_INFO, "MALLOC: test01: %d blocks of size %zd\n",
          DIM_DATA, sizeof(Data));

   head= tail= NULL;
   srand(123456);
   syslog(LOG_INFO, "MALLOC: allocating storage\n");
   for(i= 0; i<DIM_DATA; i++)
   {
     data= (Data*)malloc(sizeof(Data));
     if( data == NULL )
     {
       errorCount++;
       syslog(LOG_ERR, "%4d: Allocation failure(%zd)\n",
              __LINE__, sizeof(Data));
       break;
     }

     data->next= NULL;
     if( head == NULL )
     {
       head= tail= data;
     }
     else
     {
       tail->next= data;
       tail= data;
     }

     last= rand();
     data->random= last;
     for(j= 0; j<sizeof(data->data); j++)
     {
       last= rand();
       data->data[j]= last;
     }
   }

   srand(123456);
   i= 0;
   syslog(LOG_INFO, "MALLOC: verifying/releasing storage\n");
   while( head != NULL )
   {
     data= head;
     head= data->next;

     last= rand();
     if( data->random != last )
     {
       errorCount++;
       syslog(LOG_ERR, "%4d: Data corrupt(%x/%x)\n",
              __LINE__, last, data->random);
       break;
     }

     for(j= 0; j<sizeof(data->data); j++)
     {
       last= rand() & 0x00ff;
       if( data->data[j] != last )
       {
         syslog(LOG_ERR, "%4d: Data corrupt(%x/%x)\n",
                __LINE__, last, data->random);
         break;
       }
     }

     free(data);
     i++;
   }

   if( i != DIM_DATA )
   {
     errorCount++;
     syslog(LOG_ERR, "%4d: ERROR: released %zd of %d elements\n",
            __LINE__, i, DIM_DATA);
   }

   return errorCount;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       malloc
//
// Purpose-
//       MALLOC test
//
//----------------------------------------------------------------------------
extern int malloc(int, char**);     // (Not very far) Forward reference
extern int                          // Error count
   malloc(int, char**)              // MALLOC test
//   int               argc,        // Argument count (Unused)
//   char*             argv[])      // Argument array (Unused)
{
   int                 errorCount= 0; // Error count

   //-------------------------------------------------------------------------
   // Drive the tests
   //-------------------------------------------------------------------------
   syslog(LOG_INFO, "MALLOC: started..\n");
   if( 1 ) errorCount += test00();
   if( 1 ) errorCount += test01();

   syslog(LOG_INFO, "MALLOC: ..complete!\n");
   return errorCount;
}

