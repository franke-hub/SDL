//----------------------------------------------------------------------------
//
//       Copyright (c) 2011-2020 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Visitors.cpp
//
// Purpose-
//       Read the log, looking for visitors to a SIM.
//
// Last change date-
//       2020/10/04 (More compiler warnings)
//
// Parameters-
//       Optional: -npc (Include NPC avatars.)
//       Optional: -active (Show only active users.)
//       Optional: -all (Same as -begin:0001-01-01.)
//       Optional: -begin:yyyy-mm-dd (Beginning date.) Default is 7 days ago.
//       Optional: -days:n (Beginning n days ago.)
//       Optional: -recent (Beginning at the last restart.)
//       Optional: -today  (Beginning today.)
//       Optional: Name of input file. Default is OpenSim.log
//
// Input-
//       OpenSim.log
//
//       0123456789012345678901234567890123456789 [Columns]
//       yyyy-mm-dd hh:mm:ss,ttt DEBUG - xxxxxxxx [Format]
//
// Output-
//       stdout: The vistor log list.
//
//----------------------------------------------------------------------------
#include <ctype.h>                  // For isdigit()
#include <stdarg.h>                 // For error()
#include <stdio.h>                  // For printf
#include <stdlib.h>                 // For various
#include <string.h>                 // For strcmp
#include <time.h>                   // For localtime

#include <com/Calendar.h>
#include <com/Debug.h>
#include <com/Julian.h>
#include <com/List.h>
#include <com/Parser.h>
#include <com/Reader.h>
#include <com/Tokenizer.h>

//----------------------------------------------------------------------------
// Constant for parameterization
//----------------------------------------------------------------------------
#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE  1
#endif

#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

//----------------------------------------------------------------------------
// Constant for parameterization
//----------------------------------------------------------------------------
// 012345678901234567890123456789
// yyyy-mm-dd hh:mm:ss,sss XXXXX [YYYYY]: Text
#define DT_SIZE 24                  // Size of Date/Time prefix

//----------------------------------------------------------------------------
//
// Subroutine-
//       getToday
//
// Purpose-
//       Return Julian for today's date
//
//----------------------------------------------------------------------------
static Julian                       // Resultant
   getToday( void )                 // Get Julian for today's date
{
   time_t              now_time_t;  // Current time
   struct tm           now_tm;      // Current time
   Julian              result;      // Resultant

   now_time_t= time(NULL);          // Get current time
   now_tm= *localtime(&now_time_t); // Get local time

   Calendar c;
   c.setYMDHMSN(now_tm.tm_year+1900, now_tm.tm_mon+1, now_tm.tm_mday, 0,0,0);
   return c.toJulian();
}

//----------------------------------------------------------------------------
//
// Class-
//       Date
//
// Purpose-
//       Date (year, month, day)
//
//----------------------------------------------------------------------------
class Date {
public:
   long                year;        // The associated year
   int                 month;       // The associated month
   int                 day;         // The associated day

   char                time[16];    // Time, format hh:mm:ss,sss

inline
   ~Date( void )                    // Destructor
{
}

inline
   Date( void )                     // Default constructor
{
   Julian j= getToday();
   j -= (7.0 * Julian::SECONDS_PER_DAY);
   Calendar c(j);
   year=  c.getYear();
   month= c.getMonth();
   day=   c.getDay();

   strcpy(time, "00:00:00,000");
}

inline
   Date(                            // Constructor
     Julian&           j)           // From Julian
{
   Calendar c(j);
   year=  c.getYear();
   month= c.getMonth();
   day=   c.getDay();

   strcpy(time, "00:00:00,000");
}

inline
   Date(                            // Constructor
     const char*       parm)        // String, format mmmm_mm_dd
{
   set(parm);
}

inline int
   operator <(                      // Compare to
     const Date&       that) const  // Other date
{
   if( this->year < that.year )
     return TRUE;
   if( this->year > that.year )
     return FALSE;

   if( this->month < that.month )
     return TRUE;
   if( this->month > that.month )
     return FALSE;

   if( this->day < that.day )
     return TRUE;
   if( this->day > that.day )
     return FALSE;

   if( strcmp(this->time,that.time) < 0 )
     return TRUE;
   return FALSE;
}

inline void
   set(                             // Set year, month, day
     const char*       parm)        // String, format yyyy-mm-dd
{
   Parser parser(parm);

   year= parser.toDec();
   if( parser.current() != '-' )
     throw "Malformed Date string";

   parser.next();
   month= parser.toDec();
   if( month < 1 || month > 12 )
     throw "Invalid Date string";
   if( parser.current() != '-' )
     throw "Malformed Date string";

   parser.next();
   day= parser.toDec();
   if( day < 1 || day > 31 )
     throw "Invalid Date string";

   parm= parser.getString();
   if( *parm != '\0' && *parm != ' ' )
     throw "Malformed Date string";

   strcpy(time, "00:00:00,000");
   if( *parm == ' '
       && isdigit(parm[1])
       && isdigit(parm[2])
       && parm[3] == ':'
       && isdigit(parm[4])
       && isdigit(parm[5])
       && parm[6] == ':'
       && isdigit(parm[7])
       && isdigit(parm[8])
       && parm[9] == ','
       && isdigit(parm[10])
       && isdigit(parm[11])
       && isdigit(parm[12])
       && parm[13] == ' ')
   {
     memcpy(time, parm+1, 12);
     time[12]= '\0';
   }
}
}; // class Date

//----------------------------------------------------------------------------
//
// Class-
//       User
//
// Purpose-
//       Describe a visitor.
//
//----------------------------------------------------------------------------
class User : public List<User>::Link {
public:
   char                region[32];  // The associated Region name
   char                uuid[40];    // The associated UUID

   const char*         type;        // The name type
   char*               fName;       // First name
   char*               lName;       // Last name

inline
   ~User( void )                    // Destructor
{
   free(fName);
   free(lName);
}

inline
   User(                            // Constructor
     const char*       type,        // The name type
     const char*       fName,       // The first name
     const char*       lName,       // The last name
     const char*       region)      // The region name
:  type(type)
,  fName(NULL)
,  lName(NULL)
{
   setUUID(NULL);
   this->fName= strdup(fName);
   this->lName= strdup(lName);

   setRegion(this->region, region); // (Region is always char[32])
}

inline
   User(                            // Copy constructor
     const User&       user)        // The source object
:  type(user.type)
,  fName(NULL)
,  lName(NULL)
{
   memcpy(region, user.region, sizeof(region));
   setUUID(user.uuid);

   fName= strdup(user.fName);
   lName= strdup(user.lName);
}

inline void
   debug( void )                    // Debugging printf
{
   debugf("%4d HCDM %p:debug()\n", __LINE__, this);

   debugf("..uuid(%s)\n", uuid);
   debugf("..type(%s)\n", type);
   debugf("..fName(%s)\n", fName);
   debugf("..lName(%s)\n", lName);
   debugf("..region(%s)\n", region);
}

inline int                         // TRUE if equal
   compare(                        // Compare this User
     const char*       fName,      // To this first name
     const char*       lName)      // and this last name
{
   if( strcmp(this->fName, fName) == 0 && strcmp(this->lName, lName) == 0 )
     return TRUE;

   return FALSE;
}

inline int                         // TRUE if equal
   compare(                        // Compare this User
     const char*       uuid)       // To this UUID
{
   return strcmp(this->uuid, uuid);
}

static inline void
   setRegion(                       // Set region field
     char*             target,      // Target region (char[32])
     const char*       source)      // Source region field
{
   // Note: All region names are this length. Expansion is not necessary
   //              1234567890123456789012345678901234567890
   strcpy(target, "                        "); // Blank region field

   if( source == NULL )
     source= "";
   int length= strlen(source);
   if( length > 24 )
     memcpy(target, source, 24);
   else
     memcpy(target, source, length);
}

inline void
   setUUID(                        // Set the UUID
     const char*       uuid)       // From this string
{
   strcpy(this->uuid, "00000000-0000-0000-0000-000000000000");
   if( uuid != NULL && strlen(uuid) < sizeof(this->uuid) )
     strcpy(this->uuid, uuid);
}
}; // class User

//----------------------------------------------------------------------------
//
// Typedef-
//       OptionsEmptyQ
//
// Purpose-
//       Options for emptyQ subroutine.
//
//----------------------------------------------------------------------------
enum OptionsEmptyQ                  // Options for emptyQ subroutine
{  OPT_NPC =             0x00000001 // Display NPC characters
}; // enum OptionsEmptyQ

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static char            inpBuff[65536]; // The input line buffer
static List<User>      active;      // Active visitors
static Date            begin_date;  // Beginning date
static List<User>      people;      // Visitors who aren't NPCs
static const char*     SOURCE_FILE; // The source file name
static int             SWITCH_ACTIVE; // -active switch?
static int             SWITCH_NPC;  // Display NPC characters?
static int             SWITCH_RECENT; // -recent switch?

//----------------------------------------------------------------------------
//
// Subroutine-
//       error
//
// Purpose-
//       Write a message onto stderr
//
//----------------------------------------------------------------------------
static void
   error(                           // Write error message
     const char*       fmt,         // The PRINTF format string
                       ...)         // The remaining arguments
{
   va_list             argptr;      // Argument list pointer

   va_start(argptr, fmt);           // Initialize va_ functions
   vfprintf(stderr, fmt, argptr);
   va_end(argptr);                  // Close va_ functions
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       locate
//
// Purpose-
//       Locate user entry.
//
//----------------------------------------------------------------------------
static User*                        // Associated user entry
   locate(                          // Locate entry in list
     List<User>&       list,        // The list
     const char*       fName,       // First name
     const char*       lName)       // Last name
{
   User* result= (User*)list.getHead();
   while( result != NULL )
   {
     if( result->compare(fName,lName) )
       break;

     result= (User*)result->getNext();
   }

   return result;
}

static User*                        // Associated user entry
   locate(                          // Locate entry in list
     List<User>&       list,        // The list
     const char*       uuid)        // For this UUID
{
   User* result= (User*)list.getHead();
   while( result != NULL )
   {
     if( result->compare(uuid) )
       break;

     result= (User*)result->getNext();
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       remove
//
// Purpose-
//       Remove entry from active list, then delete it.
//
//----------------------------------------------------------------------------
static void
   remove(                          // Remove entry from active list
     User*             user)        // The associated entry
{
   active.remove(user, user);       // Remove entry from list
   delete user;                     // Delete the entry
}

static void
   remove(                          // Remove entry from active list
     const char*       fName,       // With this first name,
     const char*       lName,       // This last name, and
     const char*       rName)       // This region name
{
   User* user= locate(active, fName, lName);
   if( user != NULL )
   {
     char region[32];
     User::setRegion(region, rName);
     if( strcmp(region, user->region) == 0 )
       remove(user);
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       setRecent
//
// Purpose-
//       Set the begin_date to the most recent restart.
//
//----------------------------------------------------------------------------
static void
   setRecent( void )                // Set recent begin_date
{
   FileReader          reader(SOURCE_FILE); // The input file

   if( reader.getState() != reader.STATE_INPUT )
   {
     fprintf(stderr, "File(%s): NOT READABLE\n", SOURCE_FILE);
     exit(EXIT_FAILURE);
   }

   begin_date.year= 0;
   begin_date.month= 0;
   begin_date.day= 0;

   for(;;)
   {
     int rc= reader.readLine(inpBuff, sizeof(inpBuff));
     if( rc == Reader::RC_EOF )
       break;

     char* head= strstr(inpBuff, "==== STARTING OPENSIM ====");
     if( head != NULL )
     {
       try {
         begin_date.set(inpBuff);
       } catch(...) {
         error("%4d %s Exception(...)\n", __LINE__, __FILE__);
       }
     }
   }

   reader.close();
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       emptyQ
//
// Purpose-
//       Remove all active entries.
//
//----------------------------------------------------------------------------
static void
   emptyQ(                          // Empty active visitor list
     const char*       message,     // Associated message if not empty
     const int         options= 0)  // Associated options (OptionsEmptyQ)
{
   User*
   user= (User*)active.getHead();
   if( user != NULL )
   {
     if( (options & OPT_NPC) == 0 )
     {
       while( user != NULL && *user->type == 'N' )
       {
         active.remq();
         delete user;

         user= (User*)active.getHead();
       }
     }
   }

   if( user != NULL )
   {
     printf("\n");
     printf(">>> %s\n", message);
     while( user != NULL )
     {
       if( *user->type != 'N' || (options & OPT_NPC) != 0 )
         printf("%s %s: %s %s\n", user->region, user->type,
                user->fName, user->lName);

       active.remq();
       delete user;

       user= (User*)active.getHead();
     }
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       addAgent
//
// Purpose-
//       Read the log, locating visitor entries.
//
//----------------------------------------------------------------------------
static User*                        // The associated User entry
   addAgent(                        // Extract agent information
     char*             inpBuff)     // The input buffer string
{
   User*               result= NULL;// Resultant
   char*               head;        // Working char*
   char*               tail;        // Working char*
   const char*         fName;       // -> First name
   const char*         lName;       // -> Last name
   const char*         uuid;        // -> UUID string

   //                             012345678901234567890123456789
   head= strstr(inpBuff+DT_SIZE, " [SCENE]: Region ");
   if( head != NULL )
   {
     //                  01234567890123456789012345678901234567890
     tail= strstr(head, " told of incoming root agent ");
     if( tail != NULL )
     {
       Tokenizer avatar(tail+29);
       fName= avatar.nextToken();
       lName= avatar.nextToken();
       uuid= avatar.nextToken();
       if( locate(people, fName, lName) == NULL ) // Real person located
       {
         User* user= new User("R", fName, lName, NULL);
         user->setUUID(uuid);
         people.lifo(user);
       }
     }

     //                  01234567890123456789012345678901234567890
     tail= strstr(head, " told of incoming child agent ");
     if( tail != NULL )
     {
       Tokenizer avatar(tail+30);
       fName= avatar.nextToken();
       lName= avatar.nextToken();
       uuid= avatar.nextToken();
       if( *uuid == '@' )           // If grid user
         uuid= avatar.nextToken();
       if( locate(people, fName, lName) == NULL ) // Real person located
       {
         User* user= new User("R", fName, lName, NULL);
         user->setUUID(uuid);
         people.lifo(user);
       }
     }
   }

   //                             012345678901234567890123456789012345678901234567890
   head= strstr(inpBuff+DT_SIZE, " [SCENE PRESENCE]: Completing movement of ");
   if( head != NULL )
   {
     Tokenizer avatar(head+42);
     fName= avatar.nextToken();
     lName= avatar.nextToken();

     head= (char*)avatar.remainder(); // Last name may be followed by gridname
     //                  012345678901234567890
     head= strstr(head, "into region ");
     if( head )
     {
       head += 12;                      // Beginning of region name
       tail= strstr(head, " in position "); // End of region name
       if( tail != NULL )
       {
         *tail= '\0';
         result= new User("N", fName, lName, head);

         User* user= locate(people, fName, lName);
         if( user != NULL )
           result->type= user->type;
       }
     }
   }

#if 0 // (OBSOLETE)
   //                             01234567890123456789012345678901234567890
   head= strstr(inpBuff+DT_SIZE, " [NPC MODULE]: Creating NPC ");
   if( head != NULL )
   {
     Tokenizer avatar(head+28);
     fName= avatar.nextToken();
     lName= avatar.nextToken();
     uuid = avatar.nextToken();

     const char* region= "*ERROR*";
     tail= (char*)avatar.remainder();
     tail= strstr(tail, " in ");
     if( tail != NULL )
       region= tail+4;

     remove(fName, lName, region);  // Remove any prior duplicate
     result= new User("N", fName, lName, region);
     result->setUUID(uuid);
   }
#endif

   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       remAgent
//
// Purpose-
//       Read the log, remove exiting visitor entries.
//
//----------------------------------------------------------------------------
static void
   remAgent(                        // Remove agent information on exit
     char*             inpBuff)     // The input buffer string
{
   char*               head;        // Working char*
   const char*         tail;        // Working (const) char*
   const char*         fName;       // -> First name
   const char*         lName;       // -> Last name
   const char*         uuid;        // -> UUID string

   head= strstr(inpBuff, "==== STARTING OPENSIM ====");
   if( head != NULL )
   {
     Tokenizer dateTime(inpBuff);
     const char* date= dateTime.nextToken();
     const char* time= dateTime.nextToken();
     printf(">> %s %s SIM (RE)STARTED\n", date, time);

     emptyQ("Users active during restart:");
     printf("\n");
     return;
   }

   //                             01234567890123456789012345678901234567890
   head= strstr(inpBuff+DT_SIZE, " [CLIENT]: Close has been called for ");
   if( head != NULL )
   {
     Tokenizer avatar(head+37);
     fName= avatar.nextToken();
     lName= avatar.nextToken();

     User* user= locate(active, fName, lName);
     if( user != NULL )
     {
       tail= avatar.remainder();
       //                  0123456789012345678901234567890
       tail= strstr(tail, "attached to scene ");
       if( tail )
       {
         tail += 18;
         remove(fName, lName, tail);
       }
     }

     return;
   }

   //                             01234567890123456789012345678901234567890
   head= strstr(inpBuff+DT_SIZE, " [SCENE]: Removing root agent ");
   if( head != NULL )
   {
     Tokenizer avatar(head+30);
     fName= avatar.nextToken();
     lName= avatar.nextToken();
     User* user= locate(active, fName, lName);
     if( user != NULL )             // Found <first> <last>
     {
       tail= (char*)avatar.remainder();
       //                  012345678901234567890
       tail= strstr(tail, " from ");
       if( tail )
       {
         tail += 6;
         remove(fName, lName, tail);
       }
     }
     else                           // Missed <first> <last>, try <uuid>
     {
       uuid= avatar.nextToken();
       if( *uuid == '@' )           // If grid id, not uuid
         uuid= avatar.nextToken();
       user= locate(active, uuid);
       if( user != NULL )
       {
         char region[32];             // The region name

         avatar.nextToken();          // "from"
//       avatar.nextToken();          // "region" (OBSOLETE)
         tail= avatar.remainder();    // Region name
         User::setRegion(region, tail); // Sets local region variable

         if( strcmp(region, user->region) == 0 )
           remove(user);
       }
     }
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       examine
//
// Purpose-
//       Read the log, display visitor entries.
//
//----------------------------------------------------------------------------
static void
   examine( void )                  // Read the log
{
   FileReader          reader(SOURCE_FILE); // The input file

   if( reader.getState() != reader.STATE_INPUT )
   {
     fprintf(stderr, "File(%s): NOT READABLE\n", SOURCE_FILE);
     exit(EXIT_FAILURE);
   }

   for(;;)
   {
     int rc= reader.readLine(inpBuff, sizeof(inpBuff));
     if( rc == Reader::RC_EOF )
       break;

     try {
       Date log_date(inpBuff);
       if( log_date < begin_date )
         throw "skip";
     } catch(...) {
       continue;
     }

     User* user= addAgent(inpBuff);
     if( user == NULL )
       remAgent(inpBuff);
     else
     {
       const char* moved= "";

       // Remove any duplicate
       User* item= locate(active, user->fName, user->lName);
       if( item != NULL )
       {
         // If already in the same region, delete the duplicate.
         // (Also handles NPC)
         if( strcmp(user->region, item->region) == 0 )
         {
           delete user;
           continue;
         }

         // Already exists in different region, delete the original.
         moved= " (Moved)";
         remove(item);
       }

       Tokenizer dateTime(inpBuff);
       const char* date= dateTime.nextToken();
       const char* time= dateTime.nextToken();

       if( SWITCH_ACTIVE == FALSE && (*user->type != 'N' || SWITCH_NPC) )
         printf("%s: %s %s %s %s %s%s\n", user->type, date, time,
                user->region, user->fName, user->lName, moved);

       active.fifo(user);
     }
   }

   // Display active visitor list
   emptyQ("Active visitors:", SWITCH_NPC ? OPT_NPC : 0);

   reader.close();
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       info
//
// Purpose-
//       Parameter informational display.
//
//----------------------------------------------------------------------------
static void
   info( void )                     // Parameter analysis
{
   fprintf(stderr,
           "Visitors: Display vistor information\n"
           "\n"
           "Options:\n"
           "  -npc              (Include NPC avatars.)\n"
           "  -active           (Show only active users.)\n"
           "  -all              (Same as -begin:0001-01-01.)\n"
           "  -begin:yyyy-mm-dd (Beginning date.)\n"
           "  -days:n           (Beginning n days ago.)\n"
           "  -recent           (Beginning at the last restart.\n"
           "  -today            (Beginning today.)\n"
           "\n"
           "Parameters: (optional) Name of log file. Default is OpenSim.log\n"
           "Input: File in OpenSim.log format\n"
           "Output: stdout (The visitor log entries)\n"
          );

   exit(EXIT_FAILURE);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       parm
//
// Purpose-
//       Parameter analysis
//
//----------------------------------------------------------------------------
static void
   parm(                            // Parameter analysis
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   char*               argp= NULL;  // Argument parameter (NULL for glitch)
   int                 argi;        // Argument index

   int                 ERROR;       // Error encountered indicator
   int                 HELPI;       // Help encountered indicator
   int                 verify;      // Verification control

   //-------------------------------------------------------------------------
   // Defaults
   //-------------------------------------------------------------------------
   ERROR= FALSE;                    // Set defaults
   HELPI= FALSE;
   verify= FALSE;
   SOURCE_FILE= NULL;
   SWITCH_ACTIVE= FALSE;
   SWITCH_NPC= FALSE;
   SWITCH_RECENT= FALSE;

   //-------------------------------------------------------------------------
   // Argument analysis
   //-------------------------------------------------------------------------
   for( argi=1; argi<argc; argi++ ) // Analyze variable controls
   {
     argp= argv[argi];              // Address the parameter

     if( *argp == '-' )             // If this parameter is in switch format
     {
       if( strcmp("-help", argp) == 0 // If help request
           || strcmp("--help", argp) == 0 )
         HELPI= TRUE;

       else if( strcmp("-active", argp) == 0 ) {
         SWITCH_ACTIVE= TRUE;
         SWITCH_RECENT= TRUE;
       }

       else if( strcmp("-all", argp) == 0 )
         begin_date.set("0001-01-01");

       else if( memcmp("-begin:", argp, 7) == 0 )
       {
         argp += 7;
         try {
           begin_date.set(argp);
         } catch(...) {
           ERROR= TRUE;
           error("Malformed or invalid date '%s'\n", argp);
         }
       }

       else if( strcmp("-npc", argp) == 0 )
         SWITCH_NPC= TRUE;

       else if( memcmp("-days:", argp, 6) == 0 )
       {
         int ago= atol(argp+6);
         Julian j= getToday();
         j -= double(ago) * Julian::SECONDS_PER_DAY;
         Date date(j);
         begin_date= date;
       }

       else if( strcmp("-recent", argp) == 0 )
         SWITCH_RECENT= TRUE;

       else if( strcmp("-today", argp) == 0 )
       {
         Julian j= getToday();
         Date today(j);
         begin_date= today;
       }

       else if( strcmp("-verify", argp) == 0 )
         verify= TRUE;

       else                         // If invalid switch
       {
         ERROR= TRUE;
         error("Invalid parameter '%s'\n", argv[argi]);
       }
     }
     else                           // If filename parameter
     {
       if( SOURCE_FILE != NULL)
       {
         ERROR= TRUE;
         error("Unexpected file name '%s'\n", argv[argi]);
       }
       else
         SOURCE_FILE= argp;
     }
   }

   //-------------------------------------------------------------------------
   // Completion analysis
   //-------------------------------------------------------------------------
   if( SOURCE_FILE == NULL )
     SOURCE_FILE= "OpenSim.log";

   if( HELPI || ERROR )
   {
     if( ERROR )
       error("\n");

     info();
   }

   if( SWITCH_RECENT )
     setRecent();

   if( verify )
   {
     fprintf(stderr, "Source: '%s'\n", SOURCE_FILE);
     fprintf(stderr, "-active: %s\n", SWITCH_ACTIVE ? "TRUE" : "FALSE");
     fprintf(stderr, "-begin: %.4ld-%.2d-%.2d\n",
                     begin_date.year, begin_date.month, begin_date.day);
     fprintf(stderr, "  -npc: %s\n", SWITCH_NPC ? "TRUE" : "FALSE");
   }
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
     char*             argv[])      // Argument array
{
   parm(argc, argv);                // Parameter analysis

   examine();                       // Read the log

   return EXIT_SUCCESS;
}

