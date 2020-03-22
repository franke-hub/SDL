//----------------------------------------------------------------------------
//
//       Copyright (c) 2011-2018 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       DbInfo.cpp
//
// Purpose-
//       Implement DbInfo object methods
//
// Last change date-
//       2018/01/01 DB4/5 compatibility
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <com/Debug.h>

#include "DbBase.h"
#include "DbInfo.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define DIM_COUNT 6                 // The number of type/link pairs

#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

//----------------------------------------------------------------------------
//
// Method-
//       DbInfo::~DbInfo
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   DbInfo::~DbInfo( void )          // Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       DbInfo::DbInfo
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   DbInfo::DbInfo(                  // Default constructor
     int               fc,          // Function code
     int               fm)          // Function code modifier
{
   this->fc= fc;
   this->fm= fm;

   for(int i= 0; i<DIM_COUNT; i++)
   {
     type[i]= 0;
     link[0]= 0;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       DbInfo::getLink
//
// Purpose-
//       Retrieve link information
//
//----------------------------------------------------------------------------
uint64_t                            // Resultant link value
   DbInfo::getLink(                 // Get link information
     int               index,       // For this link index
     int*              type,        // The link type  (NULL allowed)
     uint64_t*         link)        // The link value (NULL allowed)
{
   uint64_t            result= 0;   // Resultant

   if( index >= 0 && index < DIM_COUNT )
   {
     if( type != NULL )
       *type= this->type[index];

     result= DbBase::fetch64(&this->link[index]);
     if( link != NULL )
       *link= result;
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       DbInfo::getType
//
// Purpose-
//       Retrieve type information
//
//----------------------------------------------------------------------------
int                                 // Resultant type value
   DbInfo::getType(                 // Get type information
     int               index)       // For this link index
{
   int                 result= 0;   // Resultant

   if( index >= 0 && index < DIM_COUNT )
   {
     result= this->type[index];
     if( type != NULL )
       *type= result;
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       DbInfo::setLink
//
// Purpose-
//       Update link information
//
//----------------------------------------------------------------------------
void
   DbInfo::setLink(                 // Set link information
     int               index,       // For this link index
     int               type,        // The link type
     uint64_t          link)        // The link value
{
   if( index >= 0 && index < DIM_COUNT )
   {
     this->type[index]= type;
     DbBase::store64(&this->link[index], link);
   }
}

