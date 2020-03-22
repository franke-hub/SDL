//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       OldPlex.cpp
//
// Purpose-
//       Restore the Units from disk - old format
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
void
   Plex::oldFormat(                 // Restore the Units from disk
     int             handle)        // Handle
{
   Checksum64        checksum;      // Working Checksum
   int               diff;          // Number of differing rules
   DasdOld           header;        // Header
   Network::Byte     inpsum[sizeof(uint64_t)]; // Checksum input area
   int               keep;          // The number of units on disk
   const char*       oldRule;       // -> Rule[0]
   Unit*             ptrUnit;       // -> Unit
   unsigned          saveIndex0;    // Saved histIndex0
   uint64_t          validator;     // Checksum validator

   int               i, j;

   //-------------------------------------------------------------------------
   // Read and verify the header
   //-------------------------------------------------------------------------
   checksum.reset();

   diskRead(handle, &header, sizeof(header));
   checksum.accumulate((char*)&header, sizeof(header));

   if( strcmp(header.cbid, PLEX_CBID) != 0
       || header.releaseId != DasdOld::ReleaseId
       || header.versionId != DasdOld::VersionId
       || header.unitCount != DIM_UNIT
       || header.l3ArraySize != DasdOld::L3ArraySize
       || header.l2ArraySize != DasdOld::L2ArraySize
       || header.l1ArraySize != DasdOld::L1ArraySize
       || header.l0ArraySize != DasdOld::L0ArraySize)
   {
     fprintf(stderr, "Invalid Header Data\n");
     exit(EXIT_FAILURE);
   }

   //-------------------------------------------------------------------------
   // Load the Rules
   //-------------------------------------------------------------------------
   keep= DIM_UNIT - header.cullCount;
   for(i= 0; i<keep; i++)
   {
     ptrUnit= (Unit*)(getUnit(i)->castConcrete());
     diskRead(handle, ptrUnit->rule, RULE_SIZE);
     checksum.accumulate((char*)ptrUnit->rule, RULE_SIZE);

     ptrUnit->evaluation= header.evaluation[i];
     ptrUnit->cash=       header.evaluation[i];
     ptrUnit->stock=      0;
     ptrUnit->lastTransfer= histJulian[histIndexN-1];
     ptrUnit->fee=        0;
     for(j=0; j<DIM_OUT; j++)
     {
       ptrUnit->outs[j]= header.output[i][j];
     }
   }
   for(i= keep; i<DIM_UNIT; i++)
   {
     ptrUnit= (Unit*)(getUnit(i)->castConcrete());
     ptrUnit->random();
     ptrUnit->evaluation= 0;
   }
   if( global.seedControl == 0 )    // If not random seed
     RNG.setSeed(header.randSeed);  // Use saved seed
debugDump();
   generate();

   //-------------------------------------------------------------------------
   // Read the trailing checksum
   //-------------------------------------------------------------------------
   diskRead(handle, inpsum, sizeof(inpsum));
   validator= Network::load64(inpsum);
   if( checksum.getValue() != validator )
   {
     fprintf(stderr, "Data checksum\n");
     exit(EXIT_FAILURE);
   }

   //-------------------------------------------------------------------------
   // See if rules differ
   //-------------------------------------------------------------------------
   ptrUnit= (Unit*)(getUnit(0)->castConcrete());
   oldRule= (char*)ptrUnit->rule;
   diff= 0;
   for(i= 0; i<keep; i++)
   {
     ptrUnit= (Unit*)(getUnit(i)->castConcrete());
     oldRule= (char*)ptrUnit->rule;
     for(j= i+1; j<keep; j++)
     {
       ptrUnit= (Unit*)(getUnit(j)->castConcrete());
       if( memcmp(oldRule, ptrUnit->rule, RULE_SIZE) != 0 )
       {
         diff++;
         break;
       }
     }
   }
   debugf("\n");
   if( diff == 0 )
     debugf("!! WARNING !! All Rules are the same\n");

   else
     debugf("%d of %d Rules differ\n", diff+1, keep);

   //-------------------------------------------------------------------------
   // Check for incremental evaluation
   //-------------------------------------------------------------------------
   if( global.revalControl != 0 )
     debugf("Re-evaluation forced\n");
   else if( header.index0 == histIndex0
       && header.indexN == histIndexN
       && histJulian[histIndex0] == header.julian0
       && histJulian[histIndexN-1] == header.julianN )
   {
     debugf("Continuing evaluation, %d units already valid\n", keep);
     for(i= 0; i<keep; i++)
     {
       ptrUnit= (Unit*)(getUnit(i)->castConcrete());
       ptrUnit->isValid= 1;
     }
   }

   else if( header.index0 == histIndex0
       && histIndexN > header.indexN
       && histJulian[histIndex0] == header.julian0
       && histJulian[header.indexN-1] == header.julianN )
   {
     debugf("Incremental evaluation\n");
     saveIndex0= histIndex0;
     histIndex0= header.indexN;
     for(i= 0; i<keep; i++)
     {
       ptrUnit= (Unit*)(getUnit(i)->castConcrete());
       ptrUnit->isValid= 1;         // Indicate incremental evaluation
       ptrUnit->evaluation= ptrUnit->evaluate();
     }
     histIndex0= saveIndex0;
   }
   else
     debugf("Re-evaluation required\n");

   debugf("Training mode...\n");
   debugf("\n");
}
