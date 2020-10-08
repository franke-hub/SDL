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
//       DbWord.cpp
//
// Purpose-
//       Implement DbWord object methods
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

#include "DbWord.h"

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static Barrier         barrier= BARRIER_INIT; // Single thread latch
static unsigned        langIX[]=    // Current language index
{  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 // 00-0F
,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 // 10-1F
,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 // 20-2F
,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 // 30-3F
,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 // 40-4F
,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 // 50-5F
,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 // 60-6F
,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 // 70-7F
,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 // 80-8F
,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 // 90-9F
,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 // A0-AF
,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 // B0-BF
,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 // C0-CF
,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 // D0-DF
,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 // E0-EF
,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 // F0-FF
}; // langIX

//----------------------------------------------------------------------------
// Constants
//----------------------------------------------------------------------------
static const char*     langNM[]=    // Language names (see www.sil.org)
{  "_"                              // (TEMPORARY, Wilbur-0)
,  "_en"                            // 0x01 English
,  "_es"                            // 0x02 Spanish (Espanol)
,  "_fr"                            // 0x03 French
,  "_de"                            // 0x04 German (Deutch)
,  "_it"                            // 0x05 Italian
,  "_zh"                            // 0x06 Chinese
,  "_ja"                            // 0x07 Japanese
,  "_ru"                            // 0x08 Russian
,  "_la"                            // 0x09 Latin
,  "_0A"                            // 0x0A Wilbur-A
,  "_0B"                            // 0x0B Wilbur-B
,  "_0C"                            // 0x0C Wilbur-C
,  "_0D"                            // 0x0D Wilbur-D
,  "_0E"                            // 0x0E Wilbur-E
,  "_0F"                            // 0x0F Wilbur-F
,  "_ab"                            // Abkahzian
,  "_af"                            // Afrikaans
,  "_ak"                            // Akan
,  "_am"                            // Amharic
,  "_ar"                            // Arabic
,  "_az"                            // Azerbaijani
,  "_be"                            // Belarusian
,  "_bg"                            // Bulgarian
,  "_bn"                            // Bengali
,  "_bo"                            // Tibetian
,  "_br"                            // Breton
,  "_bs"                            // Bosnian
,  "_ce"                            // Chechen
,  "_ch"                            // Chamorro
,  "_co"                            // Corsican
,  "_cr"                            // Cree
,  "_cs"                            // Czech
,  "_cu"                            // Church Slavic
,  "_cy"                            // Welsh
,  "_da"                            // Danish
,  "_dz"                            // Dzongkha
,  "_ee"                            // Ewe
,  "_el"                            // Modern Greek
,  "_eo"                            // Esperanto
,  "_et"                            // Estonian
,  "_eu"                            // Basque
,  "_fa"                            // Persian (Farsi)
,  "_ff"                            // Fulah
,  "_fi"                            // Finnish
,  "_fj"                            // Fijian
,  "_ga"                            // Irish Gaelic
,  "_gd"                            // Scottish Gaelic
,  "_he"                            // Hebrew
,  "_hi"                            // Hindi
,  "_ho"                            // Hiri Motu
,  "_hr"                            // Croatian
,  "_ht"                            // Hatian
,  "_hu"                            // Hungarian
,  "_hy"                            // Armenian
,  "_ia"                            // Interlingue (IALA)
,  "_id"                            // Indonesian
,  "_ie"                            // Interlingue
,  "_ii"                            // Sichuan Yi
,  "_ik"                            // Inupiaq
,  "_is"                            // Icelandic
,  "_iu"                            // Inuktitut
,  "_jv"                            // Javanese
,  "_ka"                            // Georgian
,  "_kg"                            // Kongo
,  "_kk"                            // Kazakh
,  "_km"                            // Central Khmer
,  "_ko"                            // Korean
,  "_kr"                            // Kanuri
,  "_ks"                            // Kasmiri
,  "_ku"                            // Kurdish
,  "_kv"                            // Komi
,  "_kw"                            // Cornish
,  "_ky"                            // Kirghiz
,  "_lb"                            // Luxembourgish
,  "_lg"                            // Ganda
,  "_lo"                            // Lao
,  "_lv"                            // Latvian
,  "_lt"                            // Lithuanian
,  "_mg"                            // Malagasy
,  "_mh"                            // Marshalese
,  "_mi"                            // Maori
,  "_mk"                            // Macedonian
,  "_ml"                            // Malayalam
,  "_mn"                            // Mongolian
,  "_ms"                            // Malay
,  "_mt"                            // Maltese
,  "_my"                            // Burmese
,  "_nb"                            // Norwegian Bokmal
,  "_nl"                            // Dutch
,  "_ne"                            // Nepali
,  "_no"                            // Norwegian
,  "_nn"                            // Norwegian Nynorsk
,  "_nv"                            // Navajo
,  "_oj"                            // Ojibwa
,  "_or"                            // Oromo
,  "_pa"                            // Panjabi
,  "_pi"                            // Pali
,  "_pl"                            // Polish
,  "_ps"                            // Pushto
,  "_pt"                            // Portuguese
,  "_rm"                            // Romansh
,  "_ro"                            // Romanian
,  "_rw"                            // Kinyarwanda
,  "_sa"                            // Sanskrit
,  "_sw"                            // Swahili
,  "_sv"                            // Swedish
,  "_ta"                            // Tamil
,  "_th"                            // Thai
,  "_tk"                            // Turkmen
,  "_tl"                            // Tagalog
,  "_to"                            // Tonga (Tonga Islands)
,  "_tn"                            // Tswana
,  "_tr"                            // Turkish
,  "_ts"                            // Tsonga
,  "_ty"                            // Tahitian
,  "_ug"                            // Uighir
,  "_uk"                            // Ukranian
,  "_ur"                            // Urdu
,  "_uz"                            // Uzbek
,  "_ve"                            // Venda
,  "_vi"                            // Vietnamese
,  "_yi"                            // Yiddish
,  "_za"                            // Zhuang
,  "_zu"                            // Zulu
,  NULL                             // (End of list)
}; // langName[]

//----------------------------------------------------------------------------
//
// Subroutine-
//       setSecondary
//
// Purpose-
//       Set the secondary (value) index.
//
//----------------------------------------------------------------------------
static int                          // Return code, 0 OK
   setSecondary(                    // Set the secondary (value) index
     Db*                db,         // -> DB (unused)
     const Dbt*         xDbt,       // Index Dbt descriptor
     const Dbt*         vDbt,       // Value Dbt descriptor
     Dbt*               sDbt)       // Resultant (secondary index)
{
   (void)db; (void)xDbt;            // (Unused)
   sDbt->set_data(vDbt->get_data());// The secondary key *IS* the value
   sDbt->set_size(vDbt->get_size());
   return 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       DbWord::~DbWord
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   DbWord::~DbWord( void )          // Destructor
{
   reset();
}

//----------------------------------------------------------------------------
//
// Method-
//       DbWord::DbWord
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   DbWord::DbWord(                  // Constructor
     const char*       lang)        // The (default) language
:  DbBase()
,  dbIndex(NULL), ixValue(NULL), language(0), langMask(0)
{
   int                 rc;

   //-------------------------------------------------------------------------
   // Translate the language
   for(language= 0; langNM[language] != NULL; language++)
   {
     if( strcmp(langNM[language], lang) == 0 )
       break;
   }

   dbCheck(__FILE__, __LINE__, langNM[language] != NULL, "Language(%s)", lang);
   langMask= (language << 24);

   // Open the database and indexes
   {{{{
   DbTxn* dbTxn= NULL;
   dbEnv->txn_begin(NULL, &dbTxn, 0);

   dbIndex= new Db(dbEnv, 0);
   ixValue= new Db(dbEnv, 0);
   ixValue->set_flags(DB_DUP);       // Allow duplicates

   // Open the databases, making the handles thread-safe
   uint32_t flags= DB_CREATE | DB_THREAD;
   dbIndex->open(dbTxn, "DbWord.db", NULL, DB_BTREE, flags, 0);
   ixValue->open(dbTxn, "DbWord_ixWord.db", NULL, DB_BTREE, flags, 0);
   dbIndex->associate(dbTxn, ixValue, setSecondary, 0);

   dbTxn->commit(0);
   }}}}

   //-------------------------------------------------------------------------
   // Get the current langIX
   AutoBarrier lock(barrier);       // Index initialization is protected

   if( langIX[language] == 0 )
   {
     langIX[language]= getIndex("*");
     if( langIX[language] == 0 )
     {
       //---------------------------------------------------------------------
       // The spot index does not exist. Create the special entries
       // {0xLL000000/"_ll"}, {0xLL000001/"*"}
       lang= langNM[language];      // Make lang ultra-constant
       uint32_t xBuff;              // Index buffer
       Dbt spot((void*)"*", 1);
       Dbt xDbt(&xBuff, sizeof(uint32_t)); // Index Dbt
       Dbt vDbt((void*)lang, strlen(lang));

       DbTxn* dbTxn= NULL;          // Create transaction
       dbEnv->txn_begin(NULL, &dbTxn, 0);

       store32(&xBuff, langMask);   // Add special language word
       rc= dbIndex->put(dbTxn, &xDbt, &vDbt, DB_NOOVERWRITE);
       DBDEBUG(rc, "db->put");
       if( rc == 0 )
       {
         store32(&xBuff, langMask+1); // Add language index word
         rc= dbIndex->put(dbTxn, &xDbt, &spot, DB_NOOVERWRITE);
         DBDEBUG(rc, "db->put");
       }

       if( rc != 0 )
       {
         dbTxn->abort();
         reset();
         checkstop(__FILE__, __LINE__, "rc(%d)", rc);
       }

       //---------------------------------------------------------------------
       // Attempt to create the special entry (0x00000000/""). This may fail
       // if this is not the first language entry, so we ignore the return.
       store32(&xBuff, 0);
       spot.set_size(0);
       rc= dbIndex->put(dbTxn, &xDbt, &spot, DB_NOOVERWRITE);
       DBDEBUG(rc, "db->put");

       dbTxn->commit(0);
       langIX[language]= langMask + 1; // Initial index
     }
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       DbWord::getIndex
//
// Purpose-
//       Get index for word.
//
//----------------------------------------------------------------------------
uint32_t                            // The word index (0 if error/missing)
   DbWord::getIndex(                // Get word index
     const char*       value)       // For this word value
{
   uint32_t            result= 0;   // Resultant
   const unsigned      length= strlen(value); // Word length

   Dbt                 xRet;        // Index Dbt (returned)
   Dbt                 vDbt((void*)value, length); // Value Dbt
   Dbt                 vRet;        // Value Dbt (returned)

   int                 rc;

   //-------------------------------------------------------------------------
   // Return the existing mapping, if any.
   DbTxn* dbTxn= NULL;              // Create transaction
   dbEnv->txn_begin(NULL, &dbTxn, 0);

   Dbc* dbc= NULL;                  // Create cursor
   ixValue->cursor(dbTxn, &dbc, 0);

   rc= dbc->pget(&vDbt, &xRet, &vRet, DB_SET);
   DBDEBUG(rc, "dbc->pget");
   if( rc == 0 )
   {
     //-----------------------------------------------------------------------
     // A result is only valid if it is in the same language
     for(;;)
     {
       if( xRet.get_size() != sizeof(uint32_t) )
         checkstop(__FILE__, __LINE__, "size(%d)", xRet.get_size());

       result= fetch32((uint32_t*)xRet.get_data());
       if( (result & 0xff000000) == langMask )
         break;

       result= 0;
       rc= dbc->pget(&vDbt, &xRet, &vRet, DB_NEXT);
       DBDEBUG(rc, "dbc->pget");
       if( rc != 0
           || vRet.get_size() != length
           || memcmp(value, vRet.get_data(), length) != 0 )
         break;
     }
   }

   dbc->close();
   dbTxn->commit(0);

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       DbWord::getValue
//
// Purpose-
//       Get value for index.
//
// Implementation notes-
//       dbIndex->get cannot be used, gets
//       DB_THREAD mandates memory allocation flag on data DBT.
//
//----------------------------------------------------------------------------
char*                               // Result (NULL if error/missing)
   DbWord::getValue(                // Get word value
     uint32_t          index,       // For this word index
     char*             value)       // (OUTPUT) Result string
{
   char*               result= NULL;// Resultant
   uint32_t            xBuff;       // Index buffer

   Dbt                 xDbt(&xBuff, sizeof(uint32_t)); // Index Dbt
   Dbt                 vRet;        // Value Dbt (returned)

   int                 rc;

   //-------------------------------------------------------------------------
   // If the index is mapped, return the associated word.
   store32(&xBuff, index);          // Set the index

   DbTxn* dbTxn= NULL;              // Create transaction
   dbEnv->txn_begin(NULL, &dbTxn, 0);

   Dbc* dbc= NULL;                  // Create cursor
   dbIndex->cursor(dbTxn, &dbc, 0);

   // Get the current map
   rc= dbc->get(&xDbt, &vRet, DB_SET);
   DBDEBUG(rc, "dbc->get");
   if( rc == 0 )
   {
     result= value;
     unsigned length= vRet.get_size();
     memcpy(result, vRet.get_data(), length);
     result[length]= '\0';
   }

   dbc->close();
   dbTxn->commit(0);

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       DbWord::insert
//
// Purpose-
//       Insert word into database.
//
//----------------------------------------------------------------------------
uint32_t                            // The word index (0 if error)
   DbWord::insert(                  // Insert into database
     const char*       value)       // This word value
{
   uint32_t            result;      // Resultant
   uint32_t            xBuff;       // Index buffer
   const unsigned      length= strlen(value); // Word length

   Dbt                 xDbt(&xBuff, sizeof(uint32_t)); // Index Dbt
   Dbt                 vDbt((void*)value, length); // Value Dbt

   unsigned            i;
   int                 rc;

   // If the word is already indexed, return that word
   result= getIndex(value);
   if( result != 0 )
     return result;

   // Verify word validity
   if( length == 0 || length >= MAX_VALUE_LENGTH )
     return 0;

   // If this is a direct index word, generate the index
   if( length < 4 )
   {
     result= 0x20202020;            // SPACES
     for(i= 0; i<length; i++)
       result= ((result << 8) & 0xffffff00) | (value[i] & 0x000000ff);

     result &= 0x00ffffff;
     result |= langMask;
     store32(&xBuff, result);

     rc= dbIndex->put(NULL, &xDbt, &vDbt, DB_AUTO_COMMIT | DB_OVERWRITE_DUP);
     DBDEBUG(rc, "db->put");
     if( rc != 0 )
       return 0;

     return result;
   }

   //-------------------------------------------------------------------------
   // Long word (requires a generated index)
   AutoBarrier lock(barrier);       // Thread latch required (for index)

   Dbt spot((void*)"*", 1);
   Dbt xRet; // Index Dbt (returned)
   Dbt ignore; // Value Dbt (returned, ignored)

   DbTxn* dbTxn= NULL;              // Create transaction
   dbEnv->txn_begin(NULL, &dbTxn, 0);

   Dbc* dbc= NULL;                  // Create cursor
   ixValue->cursor(dbTxn, &dbc, 0);

   // Delete the current spot
   result= langIX[language];
   store32(&xBuff, result);
   rc= dbc->pget(&spot, &xDbt, &ignore, DB_GET_BOTH | DB_RMW);
   DBDEBUG(rc, "dbc->pget");
   if( rc == 0 )
   {
     rc= dbc->del(0);
     DBDEBUG(rc, "dbc->del");
   }

   // Write the database record
   if( rc == 0 )
   {
     rc= dbIndex->put(dbTxn, &xDbt, &vDbt, DB_NOOVERWRITE);
     DBDEBUG(rc, "db->put");
   }

   // Insert the new spot
   if( rc == 0 )
   {
     store32(&xBuff, result+1);
     rc= dbIndex->put(dbTxn, &xDbt, &spot, DB_NOOVERWRITE);
     DBDEBUG(rc, "db->put");
   }

   if( rc != 0 )
   {
     dbTxn->abort();
     result= 0;
   }
   else
   {
     langIX[language]++;

     dbc->close();
     dbTxn->commit(0);
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       DbWord::nextIndex
//
// Purpose-
//       Get next database word index
//
//----------------------------------------------------------------------------
uint32_t                            // Result (0 if error/missing)
   DbWord::nextIndex(               // Get next word index
     uint32_t          index,       // After this index
     char*             value)       // If not NULL, returned word value
{
   uint32_t            result= 0;   // Resultant

   uint32_t            xBuff;       // Index buffer
   Dbt                 xDbt(&xBuff, sizeof(uint32_t)); // Index Dbt
   Dbt                 xRet;        // Index Dbt (returned)
   Dbt                 vRet;        // Value Dbt (returned)

   int                 rc;

   //-------------------------------------------------------------------------
   // If the word is mapped, return the next mapping.
   DbTxn* dbTxn= NULL;            // Create transaction
   dbEnv->txn_begin(NULL, &dbTxn, 0);

   Dbc* dbc= NULL;                // Create cursor
   dbIndex->cursor(dbTxn, &dbc, 0);

   store32(&xBuff, index);        // Current index
   rc= dbc->get(&xDbt, &vRet, DB_SET);
   DBDEBUG(rc, "dbc->get");
   if( rc == 0 )
   {
     rc= dbc->get(&xRet, &vRet, DB_NEXT);
     DBDEBUG(rc, "dbc->get");
     if( rc == 0 )
     {
       if( xRet.get_size() != sizeof(uint32_t) )
         checkstop(__FILE__, __LINE__, "size(%d)", xRet.get_size());

       result= fetch32((uint32_t*)xRet.get_data());
       if( value != NULL )
       {
         unsigned length= vRet.get_size();
         memcpy(value, vRet.get_data(), length);
         value[length]= '\0';
       }
     }
   }

   dbc->close();
   dbTxn->commit(0);

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       DbWord::nextValue
//
// Purpose-
//       Get next database word value
//
//----------------------------------------------------------------------------
char*                               // Result (NULL if error/missing)
   DbWord::nextValue(               // Get next word value
     char*             value,       // (IN/OUT) Word value
     uint32_t*         index)       // If not NULL, associated index
{
   char*               result= NULL;// Resultant
   unsigned            length= strlen(value); // Word length

   Dbt                 xRet;        // Index Dbt (returned)
   Dbt                 vDbt((void*)value, length); // Value Dbt
   Dbt                 vRet;        // Value Dbt (returned)
   Dbt                 ignore;      // Value Dbt (returned, ignored)

   int                 rc;

   //-------------------------------------------------------------------------
   // If the word is mapped, return the next mapping.
   DbTxn* dbTxn= NULL;            // Create transaction
   dbEnv->txn_begin(NULL, &dbTxn, 0);

   Dbc* dbc= NULL;                // Create cursor
   ixValue->cursor(dbTxn, &dbc, 0);

   rc= dbc->get(&vDbt, &xRet, DB_SET);
   DBDEBUG(rc, "dbc->get");
   if( rc == 0 )
   {
     uint32_t xOut;
     rc= dbc->pget(&vRet, &xRet, &ignore, DB_NEXT);
     DBDEBUG(rc, "dbc->pget");
     while( rc == 0 )
     {
       //---------------------------------------------------------------------
       // Do not return a value in a different language
       if( xRet.get_size() != sizeof(uint32_t) )
         checkstop(__FILE__, __LINE__, "size(%d)", xRet.get_size());

       xOut= fetch32((uint32_t*)xRet.get_data());
       if( (xOut & 0xff000000) == langMask )
         break;

       rc= dbc->pget(&vRet, &xRet, &ignore, DB_NEXT);
       DBDEBUG(rc, "dbc->pget");
     }

     if( rc == 0 )
     {
       result= value;
       length= vRet.get_size();
       memcpy(result, vRet.get_data(), length);
       result[length]= '\0';

       if( index != NULL )
         *index= xOut;
     }
   }

   dbc->close();
   dbTxn->commit(0);

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       DbWord::remove
//
// Purpose-
//       Delete word from database.
//
//----------------------------------------------------------------------------
int                                 // Return code (0 if OK)
   DbWord::remove(                  // Remove
     uint32_t          index)       // This word index
{
   uint32_t            xBuff;       // Index buffer

   Dbt                 xDbt(&xBuff, sizeof(uint32_t)); // Index Dbt
   Dbt                 vRet;        // Value Dbt (returned)

   int                 rc;

   if( index == 0 )
     return (-1);

   DbTxn* dbTxn= NULL;              // Create transaction
   dbEnv->txn_begin(NULL, &dbTxn, 0);

   Dbc* dbc= NULL;                  // Create cursor
   dbIndex->cursor(dbTxn, &dbc, 0);

   // Delete the current map
   store32(&xBuff, index);          // Set the index
   rc= dbc->get(&xDbt, &vRet, DB_SET);
   DBDEBUG(rc, "dbc->get");
   if( rc == 0 )
   {
     if( vRet.get_size() == 1
         && *((char*)vRet.get_data()) == '*' )
       rc= (-1);
     else
     {
       rc= dbc->del(0);
       DBDEBUG(rc, "dbc->del");
     }
   }

   dbc->close();

   if( rc != 0 )
     dbTxn->abort();
   else
     dbTxn->commit(0);

   return rc;
}

//----------------------------------------------------------------------------
//
// Method-
//       DbWord::reset
//
// Purpose-
//       Reset the database.
//
//----------------------------------------------------------------------------
void
   DbWord::reset( void )            // Reset the database
{
   // Create a checkpoint
   dbEnv->txn_checkpoint(0, 0, 0);

   // Delete all the open databases
   if( ixValue != NULL )
   {
     delete ixValue;
     ixValue= NULL;
   }

   if( dbIndex != NULL )
   {
     delete dbIndex;
     dbIndex= NULL;
   }
}

