//----------------------------------------------------------------------------
//
//       Copyright (C) 2010-2013 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       DbCommon.java
//
// Purpose-
//       Golfer Database Common static attributes.
//
// Last change date-
//       2013/01/01
//
//----------------------------------------------------------------------------
// package usr.fne.golfer;

import java.util.Vector;
import usr.fne.common.QuotedTokenizer;

public class DbCommon {
//----------------------------------------------------------------------------
// DbCommon.Attributes
//----------------------------------------------------------------------------
static final String    VERSION= "DbServer 1.0 2008.05.01";
static final boolean   DEBUG= true; // DEBUG control
static final int       PORT= 0x0000fe01; // Listener port number

static final String    CMD_GET=         "GET";
static final String    CMD_NEXT=        "NEXT";
static final String    CMD_PUT=         "PUT";
static final String    CMD_REMOVE=      "REM";
static final String    CMD_NULL=        "<NULL>";

// Thread states
static final int       FSM_RESET= 0;// Reset, not initialized
static final int       FSM_READY= 1;// Running
static final int       FSM_FINAL= 2;// Terminated
static final int       FSM_ERROR= 3;// Terminated in error

//----------------------------------------------------------------------------
// DbCommon.Utilities
//----------------------------------------------------------------------------
public static String[]              // Resultant String[]
   tokenize(                        // Extract tokens from
     String            string)      // This String
{
   QuotedTokenizer     tokenizer= new QuotedTokenizer(string);
   Vector<String>      vector= new Vector<String>();

   while( tokenizer.hasMoreTokens() )
   {
       String token= tokenizer.nextToken();
       if( token == null )
           break;

       // Remove quotes from quoted strings
       int length= token.length();
       if( length > 1
           && token.charAt(0) == '\"' && token.charAt(length-1) == '\"' )
           token= token.substring(1, length-1);

       vector.add(token);
   }

   // Vector.toArray() does not do enough
   String[] result= new String[vector.size()];
   for(int i= 0; i<result.length; i++)
     result[i]= vector.elementAt(i);

   return result;
}
} // class DbCommon
