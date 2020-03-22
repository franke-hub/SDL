//----------------------------------------------------------------------------
//
//       Copyright (C) 2008-2020 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       MonteCarloTeamEvaluator.java
//
// Purpose-
//       Common Events team data.
//
// Last change date-
//       2020/01/15
//
// Usage notes-
//       html/EventsTeam.html now writes to the active database.
//
//       To absolutely insure that an absolute minimum is found, use
//       "make EventsTeam" and inspect the debugging information.
//       This applet html provides debugging information, the production
//       version does not. Both versions update the database.
//
// Implementation note-
//       A single evaluation can get stuck on a local minimum.
//       To avoid this, multiple smaller random probes are used.
//
//       Each probe repeats using random player selection until it finds
//       no better result in CLEAN_COUNT/PROBE_COUNT attempts.
//       Optimizations are then repeated until no progress is made.
//
//       The probing sequence repeats until no better probe result is found
//       within PROBE_COUNT attempts.
//
//----------------------------------------------------------------------------
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;

import java.lang.*;
import java.util.*;

//----------------------------------------------------------------------------
//
// Class-
//       MonteCarloTeamEvaluator
//
// Purpose-
//       Select teams using Monte Carlo evaluation.
//
//----------------------------------------------------------------------------
public class MonteCarloTeamEvaluator extends DebuggingAdaptor {
//----------------------------------------------------------------------------
// MonteCarloTeamEvaluator.Attributes
//----------------------------------------------------------------------------
boolean                DEBUG= true; // <DEBUG> DEBUGGING active?
boolean                DEBUG_EVAL= false; // <DEBUG_EVAL> (Now unused)
boolean                DEBUG_HCDM= false; // <DEBUG_HCDM> Hard Core Debug Mode
boolean                DEBUG_TEST= false; // <DEBUG_TEST> Bringup diagnostics

// Constants for parameterization
int                    CLEAN_DEFAULT= 5000000; // Default CLEAN_COUNT
int                    PROBE_DEFAULT= 50; // Number of clean probes needed

// Construction data
int                    CLEAN_COUNT; // Required number of clean evaluations
int                    PROBE_COUNT; // Number of clean probes needed
Vector<PlayerData>     playerVector;// The PlayerData Vector
EventsDateInfo[]       date_info;   // The Event information for date
Random                 random= new Random(); // Random number generator
double[]               days;        // Number of days a player plays

// Iteration data areas
int                    CLEAN_PROBE; // CLEAN_COUNT/PROBE_COUNT
int                    clean_count; // Current clean count
int                    iteration;   // Iteration number
int                    probe_count; // Current probe count

int[][]                best;        // [day][player] Best evaluation
int[][]                work;        // [day][player] Working evaluation

// Evaluation data areas, exposed for debugging
double                 resultant;   // Final resultant evaluation
double                 captEval;    // Current captain evaluation
double                 cseqEval;    // Current captain sequential evaluation
double                 diffEval;    // Current driver/passenger balance evaluation
double                 dseqEval;    // Sequential driver/passenger balance evaluation
double                 partEval;    // Current cart partner evaluation
double                 pseqEval;    // Current partner sequential evaluation
double                 selfEval;    // Current alone in cart evaluation
double                 sseqEval;    // Current alone in cart sequential evaluation
double                 teamEval;    // Current team evaluation
double                 tseqEval;    // Current team sequential evaluation
double                 xtraEval;    // Current XTRA evaluation

// [playerIX] evaluation arrays
double[]               captCount;   // Captain count
double[]               cseqCount;   // Captain sequential count
double[]               diffCount;   // Sum ((driver count) - (passenger count))
double[]               dseqCount;   // Sequential driver or passenger count
double[]               selfCount;   // Alone in cart count
double[]               sseqCount;   // Sequential alone in cart count

// [playerIX][playerIX] paring arrays
double[][]             partCount;   // Cart partner count
double[][]             pseqCount;   // Sequential cart partner pairing count
double[][]             teamCount;   // Team pairint count
double[][]             tseqCount;   // Sequential team pairing count

// Weights: Higher weights have more effect
// Note: diffEval minimizes sum(driverCount-passengerCount)
//       dseqEval minimizes sequential driver/passenger count
static final double    captFactor= 1.5; // Captain factor
static final double    cseqFactor= 2.0; // Captain sequential factor
static final double    diffFactor= 0.8; // Driver/passenger weight
static final double    dseqFactor= 0.6; // Sequential driver|passenger weight
static final double    partFactor= 8.0; // Cart partner pairings weight
static final double    pseqFactor= 4.0; // Sequential partner parings weight
static final double    selfFactor= 4.5; // Alone in cart weight
static final double    sseqFactor= 9.0; // Sequential alone in cart weight
static final double    teamFactor= 4.0; // Team pairings weight
static final double    tseqFactor= 2.0; // Sequential team parings weight
static final double    xtraFactor= 0.0; // Extra evaluations weight

//----------------------------------------------------------------------------
//
// Method-
//       MonteCarloTeamEvaluator.DebuggingInterface
//
// Purpose-
//       Implement DebuggingInterface
//
//----------------------------------------------------------------------------
public void
   debug( )                         // Debugging display
{
   debug("MonteCarloTeamEvaluator");
   debug("CLEAN_COUNT: " + CLEAN_COUNT);
   debug("PROBE_COUNT: " + PROBE_COUNT);

   debug("Players:");
   for(int i= 0; i<playerVector.size(); i++)
   {
     PlayerData player= playerVector.elementAt(i);
     debug("[" + i + "] " + days[i] + " ID(" + player.playerID + ")"
          + " NN(" + player.playerNN + ")" );
   }

   debug("\n");
   debug("Rounds:");
   for(int i= 0; i<date_info.length; i ++ )
   {
     EventsDateInfo info= date_info[i];
     StringBuffer buff= new StringBuffer();
     buff.append("[" + i + "] ");
     buff.append(info.date);
     for(int j= 0; j<info.time.length; j++)
       buff.append(" " + info.time[j]);
     for(int j= 0; j<info.player_data.length; j++)
       buff.append(" " + info.player_data[j].playerNN);
     debug(buff.toString());
   }
}

public boolean                      // If debugging active
   isDebug( )                       // Is debugging active?
{
   return DEBUG;
}

//----------------------------------------------------------------------------
//
// Method-
//       MonteCarloTeamEvaluator.MonteCarloTeamEvaluator
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   MonteCarloTeamEvaluator(         // Constructor
     EventsDateInfo[]  date_info,   // The EventsDateInfo array
     int               CLEAN_COUNT, // The number of clean evaluations required
     int               PROBE_COUNT) // The number of clean evaluations required
{
   if( CLEAN_COUNT == 0 )
     CLEAN_COUNT= CLEAN_DEFAULT;
   if( PROBE_COUNT == 0 )
     PROBE_COUNT= PROBE_DEFAULT;

   this.CLEAN_COUNT= CLEAN_COUNT;
   this.PROBE_COUNT= PROBE_COUNT;
   this.date_info= date_info;

   // Build the PlayerData Vector
   playerVector= new Vector<PlayerData>();
   for(int i= 0; i<date_info.length; i++)
   {
     EventsDateInfo info= date_info[i];
     for(int j= 0; j<info.player_data.length; j++)
     {
       PlayerData player= info.player_data[j];
       if( !playerVector.contains(player) )
         playerVector.add(player);
     }
   }

   // Build evaluation arrays
   int players= playerVector.size();
   captCount= new double[players];
   cseqCount= new double[players];
   diffCount= new double[players];
   dseqCount= new double[players];
   selfCount= new double[players];
   sseqCount= new double[players];

   partCount= new double[players][];
   pseqCount= new double[players][];
   teamCount= new double[players][];
   tseqCount= new double[players][];
   for(int i= 0; i<players; i++)
   {
     partCount[i]= new double[players];
     pseqCount[i]= new double[players];
     teamCount[i]= new double[players];
     tseqCount[i]= new double[players];
   }

   best= monteCarloInit();
   work= monteCarloInit();

   // Count the number of days each player plays
   days= new double[players];
   for(int i= 0; i<days.length; i++)
     days[i]= 0.0;

   for(int i= 0; i<best.length; i++)// Count number of days each player plays
   {
     for(int j= 0; j<best[i].length; j++)
     {
       int pX= best[i][j];
       days[pX] += 1.0;
     }
   }

   for(int i= 0; i<days.length; i++) // Should not occur
     verify(days[i] > 0, "Player[" + i + "] never played");

   debug();
}

//----------------------------------------------------------------------------
//
// Method-
//       MonteCarloTeamEvaluator.monteCarloDebug
//
// Purpose-
//       Display a working matrix or array
//
//----------------------------------------------------------------------------
protected void
   monteCarloDebug(                 // Display an integer matrix
     String            name,        // The Matrix name
     int[][]           inp)         // INP Matrix
{
   debug(name);
   int length= inp.length;
   for(int i= 0; i<length; i++)
   {
     StringBuffer buff= new StringBuffer();
     buff.append("[" + i + "]");

     for(int j= 0; j<inp[i].length; j++)
       buff.append(" " + inp[i][j]);

     debug(buff.toString());
   }
}

protected void
   monteCarloDebug(                 // Display a double array
     String            name,        // The Array name
     double[]          inp)         // INP Array
{
   debug(name);
   int length= inp.length;

   StringBuffer buff= new StringBuffer();
   buff.append("[*]");
   for(int i= 0; i<length; i++)
     buff.append(" " + inp[i]);

   debug(buff.toString());
}

protected void
   monteCarloDebug(                 // Display a double matrix
     String            name,        // The Matrix name
     double[][]        inp)         // INP Matrix
{
   debug(name);
   int length= inp.length;
   for(int i= 0; i<length; i++)
   {
     StringBuffer buff= new StringBuffer();
     buff.append("[" + i + "]");

     for(int j= 0; j<inp[i].length; j++)
       buff.append(" " + inp[i][j]);

     debug(buff.toString());
   }

   boolean diagonal= true;
   for(int i= 0; i<length; i++)
   {
     if( inp[i].length != length )
     {
       diagonal= false;
       break;
     }
   }

   if( diagonal )
   {
     StringBuffer buff= new StringBuffer();
     buff.append("[*]");
     for(int i= 0; i<length; i++)
       buff.append(" " + inp[i][i]);

     debug(buff.toString());
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       MonteCarloTeamEvaluator.monteCarloDiagnostics
//
// Purpose-
//       [date][player] diagnostic display, not normally used
//
//----------------------------------------------------------------------------
protected void
   monteCarloDiagnostics(           // Display [date][player] diagnostics
     String            name,        // Associated name
     int[][]           inp)         // [date][player] matrix
{
   debug("======== DIAGNOSTICS(" + name + ") ========");
   monteCarloDebug(name, inp);      // Display the array
   evaluationDisplay(name, inp);

   debug("TEAMS");
   for(int day= 0; day<inp.length; day++)
   {
     int teamPlay[][]= monteCarloTEAM(inp, day);

     StringBuffer buff= new StringBuffer();
     buff.append("[" + day + "]");
     for(int team= 0; team<teamPlay.length; team++)
     {
       buff.append(" {");
       for(int play= 0; play<teamPlay[team].length; play++)
       {
         if( play != 0 )
           buff.append(",");

         buff.append(teamPlay[team][play]);
       }

       buff.append("}");
     }

     debug(buff.toString());
   }

   debug("CARTS");
   for(int day= 0; day<inp.length; day++)
   {
     StringBuffer buff= new StringBuffer();
     buff.append("[" + day + "]");
     for(int x= 0; x<inp[day].length; x++)
     {
       int play= inp[day][x];
       int cart= monteCarloCART(inp, day, play);
       buff.append(" {" + cart + "=cart(" + play + ")}");
     }

     int cart= monteCarloCART(inp, day, -1);
     buff.append(" {" + cart + "=cart(-1)}");

     debug(buff.toString());
   }

   debug("PARTS");
   for(int day= 0; day<inp.length; day++)
   {
     StringBuffer buff= new StringBuffer();
     buff.append("[" + day + "]");
     for(int x= 0; x<inp[day].length; x++)
     {
       int play= inp[day][x];
       int part= monteCarloPART(inp, day, play);
       buff.append(" {" + part + "=part(" + play + ")}");
     }

     int part= monteCarloPART(inp, day, -1);
     buff.append(" {" + part + "=part(-1)}");

     debug(buff.toString());
   }

   if( true  )                      // Omit function diagnostics?
     return;

   // Create known value array
   int M= playerVector.size();
   double[][] samp= new double[M][];
   for(int i= 0; i<M; i++)
   {
     samp[i]= new double[M];
     for(int j= 0; j<M; j++)
       samp[i][j]= 1.0 + (double)(j%2);

     samp[i][i]= (double)(i-(M/2));
   }

   monteCarloDebug("SAMP", samp);
   debug("DEVIATION ===============");
   debug("deviation0: " + deviation(samp));
   debug("deviation1: " + deviation(samp, true));
   debug("deviationR: " + deviation(samp[0]));
   debug("DIAGONAL ================");
   debug("diagonal:   " + diagonal(samp));
   debug("EQUALIZER================");
   debug("equalizer0: " + equalizer(samp));
   debug("equalizer1: " + equalizer(samp, true));
   debug("equalizerR: " + equalizer(samp[0]));
   debug("MINIMIZER ===============");
   debug("minimizer0: " + minimizer(samp));
   debug("minimizer1: " + minimizer(samp, true));
   debug("minimizerR: " + minimizer(samp[0]));
}

//----------------------------------------------------------------------------
//
// Method-
//       MonteCarloTeamEvaluator.monteCarloCopy
//
// Purpose-
//       Copy an integer array into another of the same size
//
//----------------------------------------------------------------------------
protected int[][]                   // out
   monteCarloCopy(                  // Generate the master list
     int[][]           out,         // OUT array
     int[][]           inp)         // INP array
{
   for(int i= 0; i<inp.length; i++)
   {
     for(int j= 0; j<inp[i].length; j++)
       out[i][j]= inp[i][j];
   }

   return out;
}

//----------------------------------------------------------------------------
//
// Method-
//       MonteCarloTeamEvaluator.monteCarloInit
//
// Purpose-
//       Generate the "master" player list.
//
//----------------------------------------------------------------------------
protected int[][]                   // [day][player] array
   monteCarloInit()                 // Generate the master list
{
   int[][] master= new int[date_info.length][];
   for(int i= 0; i<date_info.length; i++)
   {
     EventsDateInfo info= date_info[i];

     master[i]= new int[info.player_data.length];
     for(int j= 0; j<info.player_data.length; j++)
     {
       master[i][j]= playerVector.indexOf(info.player_data[j]);
       if( master[i][j] < 0 )
         throw new RuntimeException("Checkstop: " + info.player_data[j]);
     }
   }

   return master;
}

//----------------------------------------------------------------------------
//
// Method-
//       MonteCarloTeamEvaluator.monteCarloSamp
//
// Purpose-
//       Monte Carlo randomization of [date][player] array.
//
//----------------------------------------------------------------------------
protected int[][]                   // The modified array
   monteCarloSamp(                  // Randomize the [date][player] array
     int[][]           inp)         // INP/OUT pairing array
{
   for(int i= 0; i<inp.length; i++)
   {
     int length= inp[i].length;
     for(int j= 0; j<length; j++)
     {
       int k= random.nextInt(length); // Can self-swap
       int t= inp[i][j];
       inp[i][j]= inp[i][k];
       inp[i][k]= t;
     }
   }

   return inp;
}

//----------------------------------------------------------------------------
//
// Method-
//       MonteCarloTeamEvaluator.monteCarloCAPT
//
// Purpose-
//       Return 1 if player is captain on this date, 0 otherwise.
//       (Used to count sequential captain days.)
//
//----------------------------------------------------------------------------
protected int                       // One if player is captain, zero otherwise
   monteCarloCAPT(                  // Determine player captain
     int[][]           inp,         // The [day][player] pairing array
     int               day,         // For this day
     int               player)      // And this player
{
   int teamPlay[][]= monteCarloTEAM(inp, day);

   int teamNo= teamPlay.length;
   for(int team= 0; team<teamNo; team++)
   {
     if( teamPlay[team][0] == player )
       return 1;
   }

   return 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       MonteCarloTeamEvaluator.monteCarloCART
//
// Purpose-
//       Return driver/passenger cart status for player on date.
//
// Returns-
//       +1 if player was driver or alone
//       =0 if player did not play
//       -1 if player was passenger
//
//----------------------------------------------------------------------------
protected int                       // See above
   monteCarloCART(                  // Determine player driver/passenger status
     int[][]           inp,         // The [day][player] pairing array
     int               day,         // For this day
     int               player)      // And this player
{
   int teamPlay[][]= monteCarloTEAM(inp, day);

   int teamNo= teamPlay.length;
   for(int team= 0; team<teamNo; team++)
   {
     int playNo= teamPlay[team].length;
     for(int play= 0; play<playNo; play+=2) // Consider team, cart by cart
     {
       int drvr= teamPlay[team][play]; // Driver
       int pass= drvr;
       if( play < (playNo-1) )      // If passenger present
         pass= teamPlay[team][play+1];

       if( drvr == player )
         return +1;
       if( pass == player )
         return -1;
     }
   }

   return 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       MonteCarloTeamEvaluator.monteCarloPART
//
// Purpose-
//       Determine cart partner player index, (-1) if player did not play.
//
//----------------------------------------------------------------------------
protected int                       // The player's cart partner
   monteCarloPART(                  // Determine cart partner
     int[][]           inp,         // The [day][player] pairing array
     int               day,         // For this day
     int               player)      // And this player
{
   int teamPlay[][]= monteCarloTEAM(inp, day);

   int teamNo= teamPlay.length;
   for(int team= 0; team<teamNo; team++)
   {
     int playNo= teamPlay[team].length;
     for(int play= 0; play<playNo; play+=2) // Consider team, cart by cart
     {
       int drvr= teamPlay[team][play]; // Driver
       int pass= drvr;
       if( play < (playNo-1) )      // If passenger present
         pass= teamPlay[team][play+1];

       if( drvr == player )
         return pass;
       if( pass == player )
         return drvr;
     }
   }

   return (-1);                     // Player does not play on date
}

//----------------------------------------------------------------------------
//
// Method-
//       MonteCarloTeamEvaluator.monteCarloSORT
//
// Purpose-
//       Return a sorted list of players for a given day.
//
//----------------------------------------------------------------------------
protected int[]                     // The sorted player list
   monteCarloSORT(                  // Get sorted player list
     int[][]           inp,         // The [day][player] pairing array
     int               day)         // For this day
{
   int M= inp[day].length;          // Number of players
   int[] sort= new int[M];          // The sorted player list

   for(int i= 0; i<M; i++)
     sort[i]= inp[day][i];

   for(int i= 0; i<M; i++)          // Simple bubble sort
   {
     for(int j= i+1; j<M; j++)
     {
       if( sort[i] > sort[j] )
       {
         int t= sort[i];
         sort[i]= sort[j];
         sort[j]= t;
       }
     }
   }

   return sort;
}

//----------------------------------------------------------------------------
//
// Method-
//       MonteCarloTeamEvaluator.monteCarloTEAM
//
// Purpose-
//       Determine the teams for a day
//
//----------------------------------------------------------------------------
protected int[][]                   // [team][player] array
   monteCarloTEAM(                  // Determine player team number
     int[][]           inp,         // The [day][player] pairing array
     int               day)         // Day index
{
   EventsDateInfo info= date_info[day]; // Associated EventsDateInfo info
   int players= inp[day].length;    // Number of players for day
   int teamNum= info.time.length;   // Number of teams for day
   int playMin= players/teamNum;    // Minimum number of players/team
   int xPlayer= players - (teamNum * playMin); // Number of extra players

   int[][] result= new int[teamNum][];

   int LX= 0;                       // Lower index for team
   for(int team= 0; team<teamNum; team++)
   {
     int playNum = playMin;
     if( xPlayer > 0 )
     {
       playNum++;
       xPlayer--;
     }

     result[team]= new int[playNum];

     for(int play= 0; play<playNum; play++)
       result[team][play]= inp[day][LX+play];

     LX += playNum;
   }

   if( xPlayer != 0 )
     throw new RuntimeException("Checkstop"); // Didn't handle extra players

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       MonteCarloTeamEvaluator.monteCarloTEAM
//
// Purpose-
//       Determine the team number for a [day][player]
//
//----------------------------------------------------------------------------
protected int                       // Team number, -1 if did not play
   monteCarloTEAM(                  // Determine player team number
     int[][]           inp,         // The [day][player] pairing array
     int               day,         // Day index
     int               player)      // Player index
{
   int teamPlay[][]= monteCarloTEAM(inp, day);

   int teamNo= teamPlay.length;
   for(int team= 0; team<teamNo; team++)
   {
     int playNo= teamPlay[team].length;
     for(int play= 0; play<playNo; play++)
     {
       if( teamPlay[team][play] == player )
         return team;
     }
   }

   return (-1);                     // Not playing that day
}

//----------------------------------------------------------------------------
//
// Method-
//       MonteCarloTeamEvaluator.evaluationDisplay
//
// Purpose-
//       Display detailed evaluation information.
//
//----------------------------------------------------------------------------
protected void
   evaluationDisplay(               // Monte-carlo evaluation display
     String            what,        // What are we displaying?
     int[][]           inp)         // The [day][player] pairing array
{
   double whatEval= monteCarloEval(inp);
   monteCarloDebug("CAPT COUNT", captCount);
   monteCarloDebug("CSEQ COUNT", cseqCount);
   monteCarloDebug("DIFF COUNT", diffCount);
   monteCarloDebug("DSEQ COUNT", dseqCount);
   monteCarloDebug("PART COUNT", partCount);
   monteCarloDebug("PSEQ COUNT", pseqCount);
   monteCarloDebug("SELF COUNT", selfCount);
   monteCarloDebug("SSEQ COUNT", sseqCount);
   monteCarloDebug("TEAM COUNT", teamCount);
   monteCarloDebug("TSEQ COUNT", tseqCount);
   monteCarloDebug(what + " " + whatEval, inp);
   debug("captEval: " + captFactor + " * " + captEval);
   debug("cseqEval: " + cseqFactor + " * " + cseqEval);
   debug("diffEval: " + diffFactor + " * " + diffEval);
   debug("dseqEval: " + dseqFactor + " * " + dseqEval);
   debug("partEval: " + partFactor + " * " + partEval);
   debug("pseqEval: " + pseqFactor + " * " + pseqEval);
   debug("selfEval: " + selfFactor + " * " + selfEval);
   debug("sseqEval: " + sseqFactor + " * " + sseqEval);
   debug("teamEval: " + teamFactor + " * " + teamEval);
   debug("tseqEval: " + tseqFactor + " * " + tseqEval);
   debug("xtraEval: " + xtraFactor + " * " + xtraEval);
}

//----------------------------------------------------------------------------
//
// Method-
//       MonteCarloTeamEvaluator.evaluationResult
//
// Purpose-
//       Gather the team evalauation resultant.
//
//----------------------------------------------------------------------------
protected EventsTeamInfo[]          // The resultant EventsTeamInfo array
   evaluationResult( )              // Monte-carlo evaluation resultant
{
   // Generate TWEAK
   for(int i= 0; i<best.length; i++)
   {
     for(int j= 0; j<best[i].length; j++)
       debug("best[" + i + "][" + j + "]= " + best[i][j] + ";");
   }

   // Generate resultant
   int count= 0;
   for(int i= 0; i<date_info.length; i++)
     count += date_info[i].time.length;
   EventsTeamInfo[] team_info= new EventsTeamInfo[count];

   int teamIndex= 0;
   for(int day= 0; day<date_info.length; day++)
   {
     EventsDateInfo dateInfo= date_info[day];
     int teamPlay[][]= monteCarloTEAM(best, day);

     int playerIndex= 0;
     int teamNo= teamPlay.length;
     for(int team= 0; team<teamNo; team++)
     {
       EventsTeamInfo teamInfo= new EventsTeamInfo();
       teamInfo.date= dateInfo.date;
       teamInfo.time= dateInfo.time[team];

       int playNo= teamPlay[team].length;
       teamInfo.player_data= new PlayerData[playNo];
       for(int play= 0; play<playNo; play++)
       {
         int x= best[day][playerIndex++];
         teamInfo.player_data[play]= playerVector.elementAt(x);
       }

       team_info[teamIndex++]= teamInfo;
     }

   }
   verify( teamIndex == count );

   return team_info;
}

//----------------------------------------------------------------------------
//
// Method-
//       MonteCarloTeamEvaluator.evaluationOptimizer
//
// Purpose-
//       Monte-carlo team evalauation optimizations.
//
//----------------------------------------------------------------------------
protected double                    // The new optimization value
   evaluationOptimizer(             // Special case optimization
     double            bestEval)    // The evaluation to beat
{
   int D= best.length;              // The number of days
   int[][] swap= monteCarloInit();  // Working evaluation array

   //-------------------------------------------------------------------------
   // Try swapping each day with a later equivalent day
   // (This can bypass sequential day problems.)
   monteCarloCopy(swap, best);      // Start with the best result
   for(int i= 0; i<D; i++)          // For each day
   {
     int M= best[i].length;         // The number of players this day
     int[] pI= monteCarloSORT(best, i); // Players for this day
     for(int j= i+1; j<D; j++)      // For each subsequent day
     {
       if( best[j].length != M )    // If different player counts
         continue;                  // They cannot be identical

       int[] pJ= monteCarloSORT(best, j); // Players for that day
       boolean same= true;          // Check for equality
       for(int k= 0; k<M; k++)
       {
         if( pI[k] != pJ[k] )       // If not equal
         {
           same= false;
           break;
         }
       }

       if( same )                   // If they're identical
       {
         for(int k= 0; k<M; k++)
         {
           swap[i][k]= best[j][k];  // Swap the days
           swap[j][k]= best[i][k];
         }

         double swapEval= monteCarloEval(swap); // Evaluate
         if( swapEval < bestEval )  // Woo hoo!
         {
         //debug("swapped days: " + i + " and " + j);
         //monteCarloDebug("SWAP " + swapEval, swap);
           monteCarloCopy(best, swap);
           return swapEval;
         }

         for(int k= 0; k<M; k++)
         {
           swap[i][k]= best[i][k];  // Restore changes
           swap[j][k]= best[j][k];
         }
       }
     }
   }

   //-------------------------------------------------------------------------
   // For each day, try all possible player swaps
   // (This just works.)
   monteCarloCopy(swap, best);      // Start with the best result
   for(int d= 0; d<D; d++)          // For each day
   {
     int M= best[d].length;         // The number of players this day

     for(int i= 0; i<M; i++)
     {
       for(int j= i+1; j<M; j++)
       {
         swap[d][i]= best[d][j];    // Swap player positions
         swap[d][j]= best[d][i];

         double swapEval= monteCarloEval(swap); // Evaluate
         if( swapEval < bestEval )  // Woo hoo!
         {
         //debug("swapped players: ["+d+"] "+best[d][i]+" and "+best[d][j]);
         //monteCarloDebug("SWAP " + swapEval, swap);
           monteCarloCopy(best, swap);
           return swapEval;
         }

         swap[d][i]= best[d][i];    // Restore changes
         swap[d][j]= best[d][j];
       }
     }
   }

   //-------------------------------------------------------------------------
   // Try swapping player pairs
   // (This changes the team captain, and nothing else.)
   monteCarloCopy(swap, best);      // Start with the best result
   for(int d= 0; d<D; d++)          // For each day
   {
     int teamPlay[][]= monteCarloTEAM(best, d);
     int M= teamPlay.length;        // For each team

     for(int team= 0; team<M; team++)
     {
       if( teamPlay[team].length == 4 ) // If the team is swapable
       {
         int k= 0;                  // Compute k*4 the hard way
         for(int i= 0; i<team; i++)
           k += teamPlay[i].length;

         swap[d][k+0]= best[d][k+2]; // Swap team positions
         swap[d][k+1]= best[d][k+3];
         swap[d][k+2]= best[d][k+0];
         swap[d][k+3]= best[d][k+1];

         double swapEval= monteCarloEval(swap); // Evaluate
         if( swapEval < bestEval )  // Woo hoo!
         {
         //debug("swapped captain: [" + d + "] team: " + team);
         //monteCarloDebug("SWAP " + swapEval, swap);
           monteCarloCopy(best, swap);
           return swapEval;
         }

         swap[d][k+0]= best[d][k+0]; // Restore team positions
         swap[d][k+1]= best[d][k+1];
         swap[d][k+2]= best[d][k+2];
         swap[d][k+3]= best[d][k+3];
       }
     }
   }

// debug("optimization complete");
   return bestEval;
}

//----------------------------------------------------------------------------
//
// Method-
//       MonteCarloTeamEvaluator.deviation
//
// Purpose-
//       Compute the standard deviation of a value collection.
//
//----------------------------------------------------------------------------
protected double                    // The standard deviation
   deviation(                       // Compute standard deviation set
     double[][]        inp)         // The M x M pairing array
{
   return deviation(inp, false);
}

protected double                    // The standard deviation
   deviation(                       // Compute standard deviation set
     double[][]        inp,         // The M x M pairing array
     boolean           diag)        // Include diagonal?
{
   int                 M= inp.length; // inp is an M x M array
   double              dM= diag ? (double)(M*M) : (double)(M*(M-1));

   if( dM == 0.0 )
     return 0.0;

   double average= 0.0;
   for(int i= 0; i<M; i++)
   {
     for(int j= 0; j<M; j++)
     {
       if( i != j || diag )
         average += inp[i][j];
     }
   }
   average /= dM;

   double result= 0.0;
   for(int i= 0; i<M; i++)
   {
     for(int j= 0; j<M; j++)
     {
       if( i != j || diag )
       {
         double diff= inp[i][j] - average;
         result += (diff*diff);
       }
     }
   }

   result= Math.sqrt(result / dM);
   return result;
}

protected double                    // The standard deviation
   deviation(                       // Compute standard deviation
     double[]          inp)         // For this array
{
   int                 M= inp.length;
   double              dM= (double)(M);

   double average= 0.0;
   for(int i= 0; i<M; i++)
     average += inp[i];
   average /= dM;

   double result= 0.0;
   for(int i= 0; i<M; i++)
   {
     double diff= inp[i] - average;
     result += (diff*diff);
   }

   result= Math.sqrt(result / dM);
   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       MonteCarloTeamEvaluator.diagonal
//
// Purpose-
//       Compute the standard deviation of the diagonal
//
//----------------------------------------------------------------------------
protected double                    // The standard deviation
   diagonal(                        // Compute diagonal standard deviation
     double[][]        inp)         // The M x M pairing array
{
   int M= inp.length;               // inp is an M x M array
   double[] diag= new double[M];

   for(int i= 0; i<M; i++)          // Extract the diagonal
     diag[i]= inp[i][i];

   return deviation(diag);
}

//----------------------------------------------------------------------------
//
// Method-
//       MonteCarloTeamEvaluator.equalizer
//
// Purpose-
//       Treat each value as something unwanted. Attempt to "equalize the
//       pain" by pro-rating it among all days played.
//
// Evaluation-
//       Compute averagePain, average amount of pain in a day.
//       Compute pain, total amount of pain gotten.
//       Compute painPerDay, pain/days
//       painPerDay - averagePain as input to deviation.
//
//----------------------------------------------------------------------------
protected double                    // The standard deviation
   equalizer(                       // Compute standard deviation set
     double[][]        inp)         // The M x M pairing array
{
   return equalizer(inp, false);
}

protected double                    // The equalization factor
   equalizer(                       // Compute equalization factor
     double[][]        inp,         // The M x M pairing array
     boolean           diag)        // Include diagonal?
{
   int                 M= inp.length; // inp is an M x M array
   double              dM= diag ? (double)(M*M) : (double)(M*(M-1));

   if( dM == 0.0 )
     return 0.0;

   double totalDays= 0.0;           // Total number of days played
   double totalPain= 0.0;           // Total amount of pain
   for(int i= 0; i<M; i++)
   {
     totalDays += days[i];

     for(int j= 0; j<M; j++)
       if( i != j || diag )
         totalPain += Math.abs(inp[i][j]);
   }
   totalPain /= totalDays;          // Average pain per day

   double[] row= new double[M];
   for(int i= 0; i<M; i++)          // Sum row values by player
   {
     double pain= 0.0;
     for(int j= 0; j<M; j++)
     {
       if( i != j || diag )
         pain += Math.abs(inp[i][j]);
     }

     pain /= (double)days[i];       // Player pain/day
     row[i]= pain - totalPain;      // Difference from average
   }

   return deviation(row);           // Share the pain
}

protected double                    // The equalization factor
   equalizer(                       // Compute equalization factor
     double[]          inp)         // For this array
{
   int                 M= inp.length;
   double              dM= (double)(M);

   if( dM == 0.0 )
     return 0.0;

   double totalDays= 0.0;           // Total number of days played
   double totalPain= 0.0;           // Total amount of pain
   for(int i= 0; i<M; i++)
   {
     totalDays += days[i];          // Number of days for player
     totalPain += Math.abs(inp[i]); // Amount of pain for player
   }
   totalPain /= totalDays;          // Average pain per day

   double[] row= new double[M];
   for(int i= 0; i<M; i++)          // Equalize by player
   {
     double pain= Math.abs(inp[i]); // Amount of pain for player

     pain /= (double)days[i];       // Player pain/day
     row[i]= pain - totalPain;      // Difference from average
   }

   return deviation(row);
}

//----------------------------------------------------------------------------
//
// Method-
//       MonteCarloTeamEvaluator.mimimizer
//
// Purpose-
//       Compute the minimization factor for a value collection,
//       the deviation from zero.
//
//----------------------------------------------------------------------------
protected double                    // The standard deviation
   minimizer(                       // Compute standard deviation set
     double[][]        inp)         // The M x M pairing array
{
   return minimizer(inp, false);
}

protected double                    // The minimization factor
   minimizer(                       // Compute minimization factor
     double[][]        inp,         // The M x M pairing array
     boolean           diag)        // Include diagonal?
{
   int                 M= inp.length; // inp is an M x M array
   double              dM= diag ? (double)(M*M) : (double)(M*(M-1));

   if( dM == 0.0 )
     return 0.0;

   double result= 0.0;
   for(int i= 0; i<M; i++)
   {
     for(int j= 0; j<M; j++)
     {
       if( i != j || diag )
       {
         double diff= inp[i][j];
         result += (diff*diff);
       }
     }
   }

   result= Math.sqrt(result / dM);
   return result;
}

protected double                    // The minimization factor
   minimizer(                       // Compute minimization factor
     double[]          inp)         // For this array
{
   int                 M= inp.length;
   double              dM= (double)(M);

   double result= 0.0;
   for(int i= 0; i<M; i++)
   {
     double diff = inp[i];
     result += (diff*diff);
   }

   result= Math.sqrt(result / dM);
   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       MonteCarloTeamEvaluator.monteCarloXTRA
//
// Purpose-
//       Special case evaluations, essentially patches.
//
//----------------------------------------------------------------------------
protected double                    // Rating
   monteCarloXTRA(                  // Evaluate a sample pairing
     int[][]           inp)         // The [day][player] pairing array
{
   double result= 0.0;

   //-------------------------------------------------------------------------
   // We dislike having the team value lower than the cart partner value
   double partValue = partFactor * partEval;
   double teamValue = teamFactor * teamEval;
   if( teamValue < partValue )
     result += (partValue - teamValue);

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       MonteCarloTeamEvaluator.monteCarloEval
//
// Purpose-
//       Evaluate a pairing array. LOWER is better.
//
//----------------------------------------------------------------------------
protected double                    // Rating
   monteCarloEval(                  // Evaluate a sample pairing
     int[][]           inp)         // The [day][player] pairing array
{
   int length= playerVector.size();
   for(int i= 0; i<length; i++)
   {
     captCount[i]= 0.0;
     cseqCount[i]= 0.0;
     diffCount[i]= 0.0;
     dseqCount[i]= 0.0;
     selfCount[i]= 0.0;
     sseqCount[i]= 0.0;

     for(int j= 0; j<length; j++)
     {
       partCount[i][j]= 0.0;
       pseqCount[i][j]= 0.0;
       teamCount[i][j]= 0.0;
       tseqCount[i][j]= 0.0;
     }
   }

   for(int day= 0; day<inp.length; day++)
   {
     int teamPlay[][]= monteCarloTEAM(inp, day);

     int teamNo= teamPlay.length;
     for(int team= 0; team<teamNo; team++)
     {
       int playNo= teamPlay[team].length;

       // Update captain counts
       int capt= teamPlay[team][0]; // First player is captain
       captCount[capt]++;
       if( day > 1 )
       {
         // Special adjustment: Last day captains tend to have more experience
         if( day == (inp.length-1) ) // On last day
           captCount[capt] -= 0.25; // Captain is less important

         cseqCount[capt] += monteCarloCAPT(inp, day-1, capt);
       }

       for(int play= 0; play<playNo; play++) // For each player
       {
         int playIX= teamPlay[team][play];
         int prevIX= (-1);          // Previous team index
         if( day > 0 )
           prevIX= monteCarloTEAM(inp, day-1, playIX);

         // Update team counts, diagonal is days played
         for(int part= 0; part<playNo; part++) // For each partner
         {
           int partIX= teamPlay[team][part];
           teamCount[playIX][partIX]++;

           // Update sequential team counts
           if( prevIX >= 0 && playIX != partIX )
           {
             if( prevIX == monteCarloTEAM(inp, day-1, partIX) )
               tseqCount[playIX][partIX]++;
           }
         }

         // Update cart partner counts
         if( (play%2) == 0 )      // Only when considering a driver
         {
           int drvr= teamPlay[team][play]; // Driver
           int pass= drvr;
           if( play < (playNo-1) ) // If passenger present
             pass= teamPlay[team][play+1];

           partCount[drvr][pass]++;
           diffCount[drvr]++;
           diffCount[pass]--;
           if( drvr == pass )       // If alone in cart
           {
             selfCount[drvr]++;

             if( day > 0 )          // Is it sequential?
             {
               if( monteCarloCART(inp, day-1, drvr) > 0 )
                 dseqCount[drvr]++;

               if( pass == monteCarloPART(inp, day-1, drvr) )
                 sseqCount[drvr]++;
             }
           } else {                 // Have cart passenger
             partCount[pass][drvr]++; // Count passenger

             if( day > 0 )
             {
               if( monteCarloCART(inp, day-1, drvr) > 0 )
                 dseqCount[drvr]++;

               if( monteCarloCART(inp, day-1, pass) < 0 )
                 dseqCount[pass]++;

               if( pass == monteCarloPART(inp, day-1, drvr) )
               {
                 pseqCount[drvr][pass]++;
                 pseqCount[pass][drvr]++;
               }
             }
           }
         }
       }
     }
   }

   // ========================================================================
   // EVALUATION <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
   captEval= equalizer(captCount); // Captain count
   cseqEval= minimizer(cseqCount);

   diffEval= minimizer(diffCount); // Driver/passenger balance
   dseqEval= minimizer(dseqCount); // Driver/passenger sequence

   partEval= deviation(partCount); // Cart partner
   pseqEval= minimizer(pseqCount);

   selfEval= equalizer(selfCount); // Alone in cart
   sseqEval= minimizer(sseqCount);

   teamEval= deviation(teamCount); // Team partners
   tseqEval= minimizer(tseqCount);

   xtraEval= monteCarloXTRA(inp);  // Patch
   double result= captEval*captFactor
                + cseqEval*cseqFactor
                + diffEval*diffFactor
                + dseqEval*dseqFactor
                + partEval*partFactor
                + pseqEval*pseqFactor
                + selfEval*selfFactor
                + sseqEval*sseqFactor
                + teamEval*teamFactor
                + tseqEval*tseqFactor
                + xtraEval*xtraFactor;
   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       MonteCarloTeamEvaluator.evaluateProbe
//
// Purpose-
//       Monte-carlo team evalauation attempt.
//
//----------------------------------------------------------------------------
protected double                    // The probe resultant
   evaluateProbe( )                 // Monte-carlo event evaluation probe
{
   clean_count= 0;
   monteCarloSamp(best);            // MUST start with new best
   double bestEval= monteCarloEval(best);

   while( clean_count < CLEAN_PROBE )
   {
     iteration++;
     if( (iteration%(CLEAN_COUNT/100)) == 0 )
     {
       debug("Iterations: " + iteration + ", bestEval: " + bestEval
            + " progress(" + progress() + ")");
     }

     monteCarloSamp(work);          // Create new sample
     double workEval= monteCarloEval(work);
     if( workEval < bestEval )      // LOWER is better
     {
       clean_count= 0;
       monteCarloCopy(best, work);
       bestEval= workEval;
     }
     else
       clean_count++;
   }

   // Run optimizer until no additional optimization found
// debug("Running optimization, iterations: " + iteration);
// monteCarloDebug("BEST " + bestEval, best);
   for(;;)
   {
     double workEval= evaluationOptimizer(bestEval);
     if( workEval >= bestEval )
       break;

     bestEval= workEval;
   }

   debug("Iterations: " + iteration + ", bestEval: " + bestEval
        + " progress(" + progress() + ")");

   return bestEval;
}

//----------------------------------------------------------------------------
//
// Method-
//       MonteCarloTeamEvaluator.evaluate
//
// Purpose-
//       Monte-carlo team evalauation.
//
//----------------------------------------------------------------------------
public synchronized EventsTeamInfo[]// The resultant EventsTeamInfo array
   evaluate( )                      // Monte-carlo event evaluator
{
   debug("MonteCarloTeamEvaluator.evaluate");

   if( DEBUG_TEST ) {               // Test mode
     debug("Running in DEBUG_TEST mode");
     CLEAN_COUNT= 100000;

     if( false )                    // Bringup diagnostics
     {
       monteCarloDebug("INIT", work); // Debug initial value
       monteCarloSamp(work);        // Create random sample
       monteCarloDiagnostics("TEST", work); // Run diagnostics
       throw new RuntimeException("Checkstop: HCDM");
     }
   }

   // The initial probe is a single unoptimized evaluation
   int[][] bestProbe= monteCarloInit(); // Probe resultant
   double bestEval= monteCarloEval(bestProbe); // Initial short probe

   CLEAN_PROBE= CLEAN_COUNT/PROBE_COUNT; // Clean iterations/probe
   iteration= 0;                    // For progress display
   probe_count= 0;
   while( probe_count < PROBE_COUNT ) // Probe sequence
   {
     double probeEval= evaluateProbe(); // Create new probe
     print("=============================");
     print("Prior best: " + bestEval);
     print("This probe: " + probeEval);

     if( probeEval < bestEval )     // If better result
     {
       probe_count= 0;              // Start over
       bestEval= probeEval;         // Save better evaluation
       monteCarloCopy(bestProbe, best);
       evaluationDisplay("BEST", best); // Detailed result examination
     }
     else                           // If same or worse result
       probe_count++;               // Discard this probe
   }

   resultant= bestEval;             // For insertion as database comment
   monteCarloCopy(best, bestProbe); // Set best resultant

   // Display and generate RESULTANT
   print("\n\n\n=========================");
   print("Evaluation complete, best: " + resultant);
   print("Iterations: " + iteration);
   evaluationDisplay("BEST", best);

   return evaluationResult();
}

//----------------------------------------------------------------------------
//
// Method-
//       MonteCarloTeamEvaluator.progress
//
// Purpose-
//       Return the progress, range 0.0 .. 1.0
//
//----------------------------------------------------------------------------
public double                       // The progress
   progress( )                      // Get progress
{
   double factor= 1.0 / PROBE_COUNT; // Retry factor

   double result= factor * (double)probe_count;
   result += factor * ((double)(clean_count) / (double)(CLEAN_PROBE));

   return result;
}
} // class MonteCarloTeamEvaluator
