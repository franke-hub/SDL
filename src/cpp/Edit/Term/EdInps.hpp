//----------------------------------------------------------------------------
//
//       Copyright (C) 2024 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       EdInps.hpp
//
// Purpose-
//       Curses key definitions (experimentally determined)
//
// Last change date-
//       2024/05/15
//
//----------------------------------------------------------------------------
#ifndef EDINPS_HPP_INCLUDED
#define EDINPS_HPP_INCLUDED

//----------------------------------------------------------------------------
//
// Struct-
//       key_defs
//
// Purpose-
//       Key definitions
//
// Implementation notes-
//       *PRAGMATIC* Windows test results (US Keyboard only)
//       Acronyms: ALT_: Alt key; CTL_: Ctrl key; SFT_: Shift key
//
//----------------------------------------------------------------------------
struct key_defs {
enum
{  DEAD=               0            // Dead keys have no application effect
                                    // but may cause system interaction
,  PRINT_SCREEN=       DEAD         // (Windows action?)
,  SCROLL_LOCK=        DEAD         // (Windows action?)
,  PAUSE_BREAK=        DEAD         // (Windows action?)

,  F1=                 00411        // 0x0109 KEY_F( 1)
,  F2=                 00412        // 0x010A KEY_F( 2)
,  F3=                 00413        // 0x010B KEY_F( 3)
,  F4=                 00414        // 0x010C KEY_F( 4)
,  F5=                 00415        // 0x010D KEY_F( 5)
,  F6=                 00416        // 0x010E KEY_F( 6)
,  F7=                 00417        // 0x010F KEY_F( 7)
,  F8=                 00420        // 0x0110 KEY_F( 8)
,  F9=                 00421        // 0x0111 KEY_F( 9)
,  F10=                00422        // 0x0112 KEY_F(10)
,  F11=                00423        // 0x0113 KEY_F(11)
,  F12=                00424        // 0x0114 KEY_F(12)

,  SFT_F1=             00425        // 0x0115 + 0:0014 0x000C (12)
,  SFT_F2=             00426        // 0x0116
,  SFT_F3=             00427        // 0x0117
,  SFT_F4=             00430        // 0x0118
,  SFT_F5=             00431        // 0x0119
,  SFT_F6=             00432        // 0x011A
,  SFT_F7=             00433        // 0x011B
,  SFT_F8=             00434        // 0x011C
,  SFT_F9=             00435        // 0x011D
,  SFT_F10=            00436        // 0x011E
,  SFT_F11=            00437        // 0x011F
,  SFT_F12=            00440        // 0x0120

,  CTL_F1=             00441        // 0x0121 + 0:0030 0x0018 (24)
,  CTL_F2=             00442        // 0x0122
,  CTL_F3=             00443        // 0x0123
,  CTL_F4=             00444        // 0x0124
,  CTL_F5=             00445        // 0x0125
,  CTL_F6=             00446        // 0x0126
,  CTL_F7=             00447        // 0x0127
,  CTL_F8=             00450        // 0x0128
,  CTL_F9=             00451        // 0x0129
,  CTL_F10=            00452        // 0x012A
,  CTL_F11=            00453        // 0x012B
,  CTL_F12=            00454        // 0x012C

,  CTL_SFT_F1=         00455        // 0x012D + 0:0044 0x0024 (36)
,  CTL_SFT_F2=         00456        // 0x012E
,  CTL_SFT_F3=         00457        // 0x012F
,  CTL_SFT_F4=         00460        // 0x0130
,  CTL_SFT_F5=         00461        // 0x0131
,  CTL_SFT_F6=         00462        // 0x0132
,  CTL_SFT_F7=         00463        // 0x0133
,  CTL_SFT_F8=         00464        // 0x0134
,  CTL_SFT_F9=         00465        // 0x0135
,  CTL_SFT_F10=        00465        // 0x0136
,  CTL_SFT_F11=        00467        // 0x0137
,  CTL_SFT_F12=        00470        // 0x0138

,  ALT_F1=             00471        // 0x0139 + 0:0060 0x0030 (48)
,  ALT_F2=             00472        // 0x013A
,  ALT_F3=             00473        // 0x013B
,  ALT_F4=             00474        // 0x013C (Window manager: Close Window)
,  ALT_F5=             00475        // 0x013D
,  ALT_F6=             00476        // 0x013E
,  ALT_F7=             00477        // 0x013F
,  ALT_F8=             00500        // 0x0140
,  ALT_F9=             00501        // 0x0141
,  ALT_F10=            00502        // 0x0142
,  ALT_F11=            00503        // 0x0143
,  ALT_F12=            00504        // 0x0144
,  ALT_CTL_Fnn=        DEAD         // ALT-CTL-KEY_F(*) are all dead keys

// ALT-SFT-F4 is treated as ALT-F4, a "Close Window" demand sequence.
// ALT-SFT-F5..F12, if the sequence were to be continued, already have other
//   KEY_ definitions. (CURSES returns an escape sequence instead.)
,  ALT_SFT_F1=         00505        // 0x0145 + 0:0074 0x003C (60)
,  ALT_SFT_F2=         00506        // 0x0146
,  ALT_SFT_F3=         00507        // 0x0147
// ALT_SFT_F4=         00510        // 0x0148 // KEY_DL (Delete line)
// ALT_SFT_F5=         00511        // 0x0149 // KEY_IL
// ALT_SFT_F6=         00512        // 0x014A // KEY_DC
// ALT_SFT_F7=         00513        // 0x014B // KEY_IC
// ALT_SFT_F8=         00514        // 0x014C // KEY_EIC
// ALT_SFT_F9=         00515        // 0x014D // KEY_CLEAR
// ALT_SFT_F10=        00516        // 0x014E // KEY_EOS
// ALT_SFT_F11=        00517        // 0x014F // KEY_EOL
// ALT_SFT_F12=        00520        // 0x0150 // KEY_SF

,  DELete=             00512        // 0x014A KEY_DC
,  ALT_delete=         01016        // 0x020E + 0:0304 0x00C4
,  CTL_delete=         01020        // 0x0210 + 0:0306 0x00C6
,  ALT_CTL_delete=     00000        // 0x0000 (Windows task manager sequence)

,  insert=             00513        // 0x014B KEY_IC
,  ALT_insert=         01043        // 0x0223 + 0:0330 0x00D8
,  CTL_insert=         01045        // 0x0225 + 0:0332 0x00DA
,  ALT_CTL_insert=     01047        // 0x0227 + 0:0334 0x00DC

,  home=               00406        // 0x0106 KEY_HOME
,  ALT_home=           01036        // 0x021E + 0:0118 0x0430
,  CTL_home=           01040        // 0x0220 + 0:011A 0x0432
,  ALT_CTL_home=       01042        // 0x0222 + 0:011C 0x0434

,  end=                00550        // 0x0168 KEY_END
,  ALT_end=            01031        // 0x0219 + 0:0261 0x00B1
,  CTL_end=            01033        // 0x021B + 0:0263 0x00B3
,  ALT_CTL_end=        01035        // 0x021D + 0:0265 0x00B5

,  page_down=          00522        // 0x0152 KEY_NPAGE
,  ALT_page_down=      01055        // 0x022D + 0:0333 0x00DB
,  CTL_page_down=      01057        // 0x022F + 0:0335 0x00DD
,  ALT_CTL_page_down=  01061        // 0x0231 + 0:0337 0x00DF

,  page_up=            00523        // 0x0153 KEY_PPAGE
,  ALT_page_up=        01062        // 0x0232 + 0:0337 0x00DF
,  CTL_page_up=        01064        // 0x0234 + 0:0341 0x00E1
,  ALT_CTL_page_up=    01066        // 0x0236 + 0:0343 0x00E3

// Arrow keys- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
,  arrow_down=         00402        // 0x0102 KEY_DOWN
,  ALT_arrow_down=     01024        // 0x0214 + 0:0422 0x0112
,  CTL_arrow_down=     01026        // 0x0216 + 0:0424 0x0114
,  ALT_CTL_arrow_down= 01030        // 0x0218 + 0:0426 0x0116

,  arrow_left=         00404        // 0x0104 KEY_LEFT
,  ALT_arrow_left=     01050        // 0x0228 + 0:0444 0x0124
,  CTL_arrow_left=     01052        // 0x022A + 0:0446 0x0126
,  ALT_CTL_arrow_left= 01054        // 0x022C + 0:0450 0x0128

,  arrow_right=        00405        // 0x0105 KEY_RIGHT
,  ALT_arrow_right=    01067        // 0x0237 + 0:0462 0x0132
,  CTL_arrow_right=    01071        // 0x0239 + 0:0464 0x0134
,  ALT_CTL_arrow_right=01073        // 0x023B + 0:0466 0x0136

,  arrow_up=           00403        // 0x0103 KEY_UP
,  ALT_arrow_up=       01075        // 0x023D + 0:0472 0x013A
,  CTL_arrow_up=       01077        // 0x023F + 0:0474 0x013C
,  ALT_CTL_arrow_up=   01101        // 0x0241 + 0:0476 0x013E
}; // (Generic enum)
}; // key_defs

//----------------------------------------------------------------------------
//
// Subroutine-
//       translate_irregular_keys
//
// Purpose-
//       *MINIMAL* Key control modifications
//
// Implementation notes-
//       ONLY modified key values that are used are updated.
//       (Currently Ctrl-F2)
//
//----------------------------------------------------------------------------
static inline void
   translate_irregular_keys(        // Translate irregular key values
     uint32_t&         key,         // IN/OUT The input key
     uint32_t&         state)       // IN/OUT The Alt/Ctl/Shift state mask
{
   switch( key ) {                  // Handle irregular key values
     case key_defs::CTL_F2:         // CTL-F2
       state |= KS_CTL;
       key= key_defs::F2;
       break;
     default:                       // No translation provided
       break;
   }
}
#endif // EDINPS_HPP_INCLUDED
