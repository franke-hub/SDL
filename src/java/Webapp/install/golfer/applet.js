//----------------------------------------------------------------------------
//
//       Copyright (C) 2010 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       applet.js
//
// Purpose-
//       Common applet javascript.
//
// Last change date-
//       2010/10/19
//
//----------------------------------------------------------------------------
var out;     // Output document

//----------------------------------------------------------------------------
// Exemplary COMPOUND contains a compound object.
//
// var sample= new Array(9);
// var sample[0]= new Array(9);
//
// sample[0][0]= new COMPOUND;
// sample[0][0].name= "name";
// sample[0][0].value= "value";
//
//----------------------------------------------------------------------------
function COMPOUND()
{
   this.name= "0";
   this.value= "0";
}

//----------------------------------------------------------------------------
// abort
//
// Write an error message and terminate the script.
//----------------------------------------------------------------------------
function abort(string)
{
   var undefined= null;
   alert('ABORT: ' + string);
   undefined.stop= true;
   return false;
}

//----------------------------------------------------------------------------
// appHead
//
// Generate html header for application.
//----------------------------------------------------------------------------
function appHead(title,cname,height,width)
{
/*****************************************************************************
// var todoWindow= window.open('','Temp.jnlp','');
// out= todoWindow.document;
// out.write('<html>');
// out.write('<head><title>' + title + '</title></head>');
// out.write('<body>\n');
// out.write('<applet code="' + cname + '.class"');
// out.write('    codebase="./"')
// out.write('    archive="applet.jar,jars/common.jar"');
// out.write('    width="' + width + '" height="' + height + '">\n');

// out.write('    <param name="DEBUGGING"   value="TRUE">\n');
// out.write('    <param name="DEBUG_IODM"  value="TRUE">\n');
*****************************************************************************/

   out= cname + ',' + title;
// out.write('<?xml version="1.0" encoding="utf-8"?>\n');
// out.write('<jnlp spec="1.0+" codebase="http://localhost:8080/golfer" href="Temp.jnlp">\n');
// out.write('    <information>\n');
// out.write('        <title>' + title + '</title>\n');
// out.write('        <vendor>Frank Eskesen</vendor>\n');
// out.write('        <homepage href="http://localhost:8080/golfer"/>\n');
// out.write('        <description>' + title + '</description>\n');
// out.write('    </information>\n');
// out.write('    <security>\n');
// out.write('        <all-permissions/>\n');
// out.write('    </security>\n');
// out.write('    <resources>\n');
// out.write('        <j2se version="1.7+"/>\n');
// out.write('        <jar href="applet.jar"/>\n');
// out.write('        <jar href="jars/common.jar"/>\n');
// out.write('    </resources>\n');
// out.write('    <applet-desc main-class="' + cname + '" name=' + title + ' height="95%" width="100%">\n');

// out.write('    <param name="DEBUGGING"   value="TRUE">\n');
// out.write('    <param name="DEBUG_IODM"  value="TRUE">\n');
}

//----------------------------------------------------------------------------
// appParm
//
// Generate html parameter information.
//----------------------------------------------------------------------------
function appParm(name, value)
{
// out.write('<param name="' + name + '" value="' + value + '">\n');
// out.write('        <param-name="' + name + '" value="' + value + '"/>\n');
   out= out + ',' + name + '=' + value;
}

//----------------------------------------------------------------------------
// appTail
//
// Generate html trailer information.
//----------------------------------------------------------------------------
function appTail()
{
/*****************************************************************************
// out.write('Your browser is completely ignoring the &lt;APPLET&gt; tag!\n');
// out.write('</applet>');
// out.write('<form>');
// out.write('<input type="button" value="Done" onclick="window.close()">');
// out.write('</form>');
// out.write('</body>');
// out.write('</html>');
*****************************************************************************/

// out.write('    </applet-desc>\n');
// out.write('</jnlp>\n');
// out.close();
// out= null;
   var specs= 'menubar=yes,toolbar=yes';
   window.open('Applet.jnlp?' + out, '_self', specs);
}

//----------------------------------------------------------------------------
// trace
//
// Generate trace information in html.
//----------------------------------------------------------------------------
function trace(msg)
{
   out.write(msg + '<br>');
}

//----------------------------------------------------------------------------
// cardEvents
//
// Display scorecard for selected date.
//----------------------------------------------------------------------------
function cardEvents(eventsID, obj)
{
   if( obj.selectedIndex == 0 )
   {
     alert("No date selected");
     return;
   }

   appHead('Score card', 'EventsCard', '100%', '100%'); // 660, 1170
   appParm('events-nick', eventsID);
   appParm('events-date', obj[obj.selectedIndex].value);
   appTail();

   reset();
}

//----------------------------------------------------------------------------
// cardPlayer
//
// Edit player scorecard data.
//----------------------------------------------------------------------------
function cardPlayer(playerNN, courseID, teeboxObj, playerDate, playerTime)
{
   if( teeboxObj.selectedIndex == 0 )
   {
     alert("No teebox selected");
     return;
   }

   appHead('Player Card', 'PlayerCard', '100%', '100%');
   appParm('player-nick', playerNN);
   appParm('course-nick', courseID);
   appParm('teebox-name', teeboxObj[teeboxObj.selectedIndex].value);
   appParm('player-date', playerDate.value);
   if( playerTime.value != null && playerTime.value != "" && playerTime.value != " " )
     appParm('player-time', playerTime.value);
   appTail();

   reset();
}

//----------------------------------------------------------------------------
// changeApplet
//
// Change applets
//----------------------------------------------------------------------------
function changeApplet(obj)
{
   var url= 'Applet?event=DEFAULT';
   if( obj.selectedIndex != 0 )
     url= 'Applet?event=' + obj[obj.selectedIndex].value;

   window.open(url);
   reset();
}

//----------------------------------------------------------------------------
// changeAppletMaint
//
// Change applet, maintenance mode
//----------------------------------------------------------------------------
function changeAppletMaint(obj, username, password)
{
   var url= 'Applet?event=DEFAULT';
   if( obj.selectedIndex != 0 )
     url= 'Applet?maint=PANEL'
        + ',event=' + obj[obj.selectedIndex].value
        + ',username=' + username
        + ((password != null) ? ',password=' + password : '');

   window.open(url);
   reset();
}

//----------------------------------------------------------------------------
// changeEvent
//
// Change event
//----------------------------------------------------------------------------
function changeEvent(obj)
{
   var url= 'Events?event=DEFAULT';
   if( obj.selectedIndex != 0 )
     url= 'Events?event=' + obj[obj.selectedIndex].value;

   window.open(url);
   reset();
}

//----------------------------------------------------------------------------
// changeEventsMaint
//
// Change event, maintenance mode
//----------------------------------------------------------------------------
function changeEventsMaint(obj, username, password)
{
   var url= 'Events?event=DEFAULT';
   if( obj.selectedIndex != 0 )
     url= 'Events?maint=PANEL'
        + ',event=' + obj[obj.selectedIndex].value
        + ',username=' + username
        + ((password != null) ? ',password=' + password : '');

   window.open(url);
   reset();
}

//----------------------------------------------------------------------------
// changePlayer
//
// Change player
//----------------------------------------------------------------------------
function changePlayer(obj)
{
   var url= 'Player?player=DEFAULT';
   if( obj.selectedIndex != 0 )
     url= 'Player?player=' + obj[obj.selectedIndex].value;

   window.open(url);
   reset();
}

//----------------------------------------------------------------------------
// changePlayerMaint
//
// Change player, maintenance mode
//----------------------------------------------------------------------------
function changePlayerMaint(obj, username, password)
{
   var url= 'Player?player=DEFAULT';
   if( obj.selectedIndex != 0 )
     url= 'Player?maint=PANEL'
        + ',player=' + obj[obj.selectedIndex].value
        + ',username=' + username
        + ((password != null) ? ',password=' + password : '');

   window.open(url);
   reset();
}

//----------------------------------------------------------------------------
// editCourse
//
// Edit course information.
//----------------------------------------------------------------------------
function editCourse(obj)
{
   appHead('Course Update', 'CourseEdit', '100%', '100%'); // 270, 1196
   if( obj.selectedIndex != 0 )
     appParm('course-nick', obj[obj.selectedIndex].value);
   appTail();

   reset();
}

//----------------------------------------------------------------------------
// editEvents
//
// Edit events information.
//----------------------------------------------------------------------------
function editEvents(obj)
{
   appHead('Events Update', 'EventsEdit', '100%', '100%'); // 270, 1196
   if( obj.selectedIndex != 0 )
     appParm('events-nick', obj[obj.selectedIndex].value);
   appTail();

   reset();
}

//----------------------------------------------------------------------------
// editPlayer
//
// Edit player information.
//----------------------------------------------------------------------------
function editPlayer(obj)
{
   appHead('Player Update', 'PlayerEdit', '100%', '100%'); // 270, 1196
   if( obj.selectedIndex != 0 )
     appParm('player-nick', obj[obj.selectedIndex].value);
   appTail();

   reset();
}

//----------------------------------------------------------------------------
// insertCourse
//
// Add course to database.
//----------------------------------------------------------------------------
function insertCourse()
{
   appHead('New Course', 'CourseAdd', '100%', '100%'); // 270, 1196
   appTail();

   reset();
}

//----------------------------------------------------------------------------
// insertEvents
//
// Add event to database.
//----------------------------------------------------------------------------
function insertEvents()
{
   appHead('New Event ', 'EventsAdd', '100%', '100%'); // 270, 1196
   appTail();

   reset();
}

//----------------------------------------------------------------------------
// insertPlayer
//
// Add player to database.
//----------------------------------------------------------------------------
function insertPlayer()
{
   appHead('New Player', 'PlayerAdd', '100%', '100%'); // 270, 1196
   appTail();

   reset();
}

//----------------------------------------------------------------------------
// maintApplet
//
// Applet maintenace panel.
//----------------------------------------------------------------------------
function maintApplet(eventsID, username, userpass)
{
   var url= 'Applet?maint=PANEL,event=' + eventsID
          + ',username=' + username.value
          + ',password=' + userpass.value;
   window.open(url);
   reset();
}

//----------------------------------------------------------------------------
// maintEvents
//
// Events maintenace panel.
//----------------------------------------------------------------------------
function maintEvents(eventsID, username, userpass)
{
   var url= 'Events?maint=PANEL,event=' + eventsID
          + ',username=' + username.value
          + ',password=' + userpass.value;
   window.open(url);
   reset();
}

//----------------------------------------------------------------------------
// maintPlayer
//
// Player maintenace panel.
//----------------------------------------------------------------------------
function maintPlayer(playerID, username, userpass)
{
   var url= 'Player?maint=PANEL,player=' + playerID
          + ',username=' + username.value
          + ',password=' + userpass.value;
   window.open(url);
   reset();
}

//----------------------------------------------------------------------------
// playStats
//
// View player results.
//----------------------------------------------------------------------------
function playStats(eventsID)
{
   appHead('Player results', 'PlayerStat', '100%', '100%'); // 600, 1200
   appParm('events-nick', eventsID);
   appTail();

   reset();
}

//----------------------------------------------------------------------------
// postCourse
//
// Generate TEEBOX selection
//----------------------------------------------------------------------------
function postCourse(playerID, courseObj)
{
   if( courseObj.selectedIndex == 0 )
   {
     alert("No course selected");
     return;
   }
   var url= 'Player?course=' + courseObj[courseObj.selectedIndex].value
          + ',player=' + playerID;
   window.open(url);
   reset();
}

//----------------------------------------------------------------------------
// postPlayer
//
// Edit player result for selected date.
//----------------------------------------------------------------------------
function postPlayer(playerNN, courseID, teeboxObj, playerDate, playerTime)
{
   if( teeboxObj.selectedIndex == 0 )
   {
     alert("No teebox selected");
     return;
   }

   appHead('Player Post', 'PlayerPost', '100%', '100%');
   appParm('player-nick', playerNN);
   appParm('course-nick', courseID);
   appParm('teebox-name', teeboxObj[teeboxObj.selectedIndex].value);
   appParm('player-date', playerDate.value);
   if( playerTime.value != null && playerTime.value != "" && playerTime.value != " " )
     appParm('player-time', playerTime.value);
   appTail();

   reset();
}

//----------------------------------------------------------------------------
// setDefaults
//
// Set the defaults.
//----------------------------------------------------------------------------
function setDefaults()
{
   appHead('Set Defaults', 'DefaultsEdit', '100%', '100%');
   appTail();

   reset();
}

//----------------------------------------------------------------------------
// viewCourse
//
// View course information.
//----------------------------------------------------------------------------
function viewCourse(obj)
{
   if( obj.selectedIndex == 0 )
   {
     alert("No course selected");
     return;
   }

   appHead('Course Viewer', 'CourseView', '100%', '100%'); // 270, 1196
   appParm('course-nick', obj[obj.selectedIndex].value);
   appTail();

   reset();
}

//----------------------------------------------------------------------------
// viewEvents
//
// View event results for selected date.
//----------------------------------------------------------------------------
function viewEvents(eventsID, obj)
{
   if( obj.selecctedIndex == 0 )
   {
     alert("No date selected");
     return;
   }

   appHead('Result Viewer', 'EventsView', '100%', '100%'); // 624, 1216
   appParm('events-nick', eventsID);
   appParm('events-date', obj[obj.selectedIndex].value);
   appTail();

   reset();
}

//----------------------------------------------------------------------------
// viewHandicap
//
// View handicap for selected player.
//----------------------------------------------------------------------------
function viewHandicap(eventsID, obj)
{
   if( obj.selectedIndex == 0 )
   {
     alert("No player selected");
     return;
   }

   appHead('Handicap Viewer', 'PlayerHdcp', '100%', '100%'); // 875, 492
   appParm('events-nick', eventsID);
   appParm('player-nick', obj[obj.selectedIndex].value);
   appParm('MAX_DISPLAY', '20');
   appTail();

   reset();
}

//----------------------------------------------------------------------------
// viewResult
//
// View player result for selected date.
//----------------------------------------------------------------------------
function viewResult(playerNN, obj)
{
   if( obj.selectedIndex == 0 )
   {
     alert("No date selected");
     return;
   }

   var dateTime= obj[obj.selectedIndex].value;
   var date= dateTime;
   var time= null;
   if( dateTime.length > 10 )
   {
     date= dateTime.substring(0,10);
     time= dateTime.substring(11);
   }

   appHead('Result Viewer', 'PlayerView', '100%', '100%'); // 624, 1216
   appParm('player-nick', playerNN);
   appParm('player-date', date);
   if( time != null )
     appParm('player-time', time);
   appTail();

   reset();
}

//----------------------------------------------------------------------------
// viewStats
//
// View statistics information.
//----------------------------------------------------------------------------
function viewStats(eventsID)
{
   appHead('Statistics Viewer', 'EventsStat', '100%', '100%'); // 600, 1200
   appParm('events-nick', eventsID);
   appTail();

   reset();
}

//----------------------------------------------------------------------------
// viewSwing (Bringup)
//
// View Swing app information.
//----------------------------------------------------------------------------
function viewSwing(eventsID)
{
   var url= 'Invoke?EventsStat'
          + ',events-nick=' + eventsID
   window.open(url);

   reset();
}
