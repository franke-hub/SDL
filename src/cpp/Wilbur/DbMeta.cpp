//----------------------------------------------------------------------------
//
//       Copyright (c) 2010-2018 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       DbMeta.cpp
//
// Purpose-
//       Implement DbMeta object methods
//
// Last change date-
//       2018/01/01 DB4/5 compatibility
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <com/Barrier.h>
#include <com/Debug.h>

#include "Common.h"
#include "DbBase.h"
#include "DbAttr.h"
#include "DbNada.h"
#include "DbWord.h"
#include "DbFile.h"
#include "DbHttp.h"
#include "DbText.h"
#include "DbTime.h"

#include "DbMeta.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#include <com/ifmacro.h>

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static Barrier         barrier= BARRIER_INIT; // For singleton generation
static volatile void*  dbMeta= NULL;// The DbMeta singleton

//----------------------------------------------------------------------------
//
// Method-
//       DbMeta::~DbMeta
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   DbMeta::~DbMeta( void )          // Destructor
{
   IFHCDM( logf("DbMeta(%p)::~DbMeta()\n", this); )

   AutoBarrier lock(barrier);

   //-------------------------------------------------------------------------
   // Delete the databases
   DbBase** dbList= (DbBase**)&dbWord;
   for(int i= 0; i<DATABASE_COUNT; i++)
   {
     if( dbList[i] != NULL )
     {
       delete dbList[i];
       dbList[i]= NULL;
     }
   }

   dbMeta= NULL;                    // Delete the singleton
}

//----------------------------------------------------------------------------
//
// Method-
//       DbMeta::DbMeta
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   DbMeta::DbMeta( void )           // Constructor
:  DbBase()
{
   IFHCDM( logf("DbMeta(%p)::DbMeta()\n", this); )

   //-------------------------------------------------------------------------
   // Initialize the databases
   DbBase** dbList= (DbBase**)&dbWord;
   for(int i= 0; i<DATABASE_COUNT; i++)
     dbList[i]= NULL;

   dbWord= new DbWord("_en");
// dbName= new DbName();
   dbFile= new DbFile();
   dbHttp= new DbHttp();
   dbAttr= new DbAttr();
   dbText= new DbText();
   dbTime= new DbTime();

   dbMeta= this;
}

//----------------------------------------------------------------------------
//
// Method-
//       DbMeta::get
//
// Purpose-
//       Access the DbMeta singleton.
//
//----------------------------------------------------------------------------
DbMeta*                             // The DbMeta singleton
   DbMeta::get( void )              // Get DbMeta singleton
{
   DbMeta*             result= (DbMeta*)dbMeta; // Resultant

   if( result == NULL )
   {
     AutoBarrier lock(barrier);

     if( dbMeta == NULL )
       dbMeta= new DbMeta();

     result= (DbMeta*)dbMeta;
   }

   return result;
}

