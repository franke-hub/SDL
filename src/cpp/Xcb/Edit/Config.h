//----------------------------------------------------------------------------
//
//       Copyright (C) 2020 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Config.h
//
// Purpose-
//       Editor: Configuration controls (namespace config)
//
// Last change date-
//       2020/10/17
//
//----------------------------------------------------------------------------
#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED

//----------------------------------------------------------------------------
//
// Namespace-
//       config
//
// Purpose-
//       Configuration controls.
//
//----------------------------------------------------------------------------
namespace config {                  // Configuration controls
enum
{  _UNUSED_                         // (All other entries are comma delimited)
// Color definitions
,  CLR_Black=           0x00000000
,  CLR_DarkRed=         0x00900000  // A.K.A red4
,  CLR_FireBrick=       0x00B22222
,  CLR_LightBlue=       0x00C0F0FF
,  CLR_LightSkyBlue=    0x00B0E0FF
,  CLR_PaleMagenta=     0x00FFC0FF  // A.K.A plum1
,  CLR_PaleYellow=      0x00FFFFF0  // A.K.A ivory
,  CLR_PowderBlue=      0x00B0E0E0
,  CLR_White=           0x00FFFFFF
,  CLR_Yellow=          0x00FFFF00

// Color selectors
,  CHG_FG= CLR_DarkRed              // FG: Status line, file changed
,  CHG_BG= CLR_LightBlue            // BG: Status line, file changed

,  CMD_FG= CLR_Black                // FG: Command line
,  CMD_BG= CLR_PaleMagenta          // BG: Command line

,  MSG_FG= CLR_DarkRed              // FG: Message line
,  MSG_BG= CLR_Yellow               // BG: Message line

,  STS_FG= CLR_Black                // FG: Status line, file unchanged
,  STS_BG= CLR_LightBlue            // BG: Status line, file unchanged

,  TXT_FG= CLR_Black                // FG: Text
,  TXT_BG= CLR_PaleYellow           // BG: Text
}; // generic enum
}  // namespace config
#endif // CONFIG_H_INCLUDED
