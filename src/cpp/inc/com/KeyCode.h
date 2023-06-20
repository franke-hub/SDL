//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2023 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       KeyCode.h
//
// Purpose-
//       Enumerate the extended keyboard character codes for the IBM PC.
//
// Last change date-
//       2023/06/19 (Editor version 2, release 2)
//
// See also-
//       ASCII.h, ScanCode.h
//
//----------------------------------------------------------------------------
#ifndef KEYCODE_H_INCLUDED
#define KEYCODE_H_INCLUDED

//----------------------------------------------------------------------------
// Typedefs
//----------------------------------------------------------------------------
typedef int            KeyPress;    // A KeyCode value

//----------------------------------------------------------------------------
//
// Class-
//       KeyCode
//
// Purpose-
//       Enumerate the extended WINDOWS keyboard character set.
//
//----------------------------------------------------------------------------
class KeyCode {                     // The extended WINDOWS character set
public:
enum                                // The extended WINDOWS character set
{  NUL=                0x00         // '\0' NULL character (no key)
,  BEL=                0x07         // '\a' BELl
,  BS=                 0x08         // '\b' BackSpace
,  TAB=                0x09         // '\t' Horizontal tab
,  ENTER=              0x0A         // '\n' Enter
,  VT=                 0x0B         // '\v' Vertical Tab
,  FF=                 0x0C         // '\f' Form Feed
,  CR=                 0x0D         // '\r' Carriage Return
,  CTL_A=              0x01         // Control-A
,  CTL_B=              0x02         // Control-B
,  CTL_C=              0x03         // Control-C
,  CTL_D=              0x04         // Control-D
,  CTL_E=              0x05         // Control-E
,  CTL_F=              0x06         // Control-F
,  CTL_G=              0x07         // Control-G '\a' Alarm (Bell)
,  CTL_H=              0x08         // Control-H '\b' BackSpace
,  CTL_I=              0x09         // Control-I '\t' Horizontal Tab
,  CTL_J=              0x0A         // Control-J '\n' Line Feed
,  CTL_K=              0x0B         // Control-K '\v' Vertical Tab
,  CTL_L=              0x0C         // Control-L '\f' Form Feed
,  CTL_M=              0x0D         // Control-M '\r' Carriage Return
,  CTL_N=              0x0E         // Control-N
,  CTL_O=              0x0F         // Control-O
,  CTL_P=              0x10         // Control-P
,  CTL_Q=              0x11         // Control-Q
,  CTL_R=              0x12         // Control-R
,  CTL_S=              0x13         // Control-S
,  CTL_T=              0x14         // Control-T
,  CTL_U=              0x15         // Control-U
,  CTL_V=              0x16         // Control-V
,  CTL_W=              0x17         // Control-W
,  CTL_X=              0x18         // Control-X
,  CTL_Y=              0x19         // Control-Y
,  CTL_Z=              0x1A         // Control-Z
,  ESC=                0x1B         // ESCape
,  CTL_ENTER=          0x1C         // Control-ENTER
,  CTL_RightBracket=   0x1D         // Control-Right bracket
,  CTL_6=              0x1E         // Control-6
,  CTL_Hyphen=         0x1F         // Control-'-'

,  SP=                 ' '          // 0x20
,  Space=              ' '          // 0x20
,  ExclamationPoint=   '!'          // 0x21
,  QuotationMark=      '"'          // 0x22
,  PoundSign=          '#'          // 0x23
,  DollarSign=         '$'          // 0x24
,  PercentSign=        '%'          // 0x25
,  Ampersand=          '&'          // 0x26
,  Quote=              '\''         // 0x27
,  LeftParenthesis=    '('          // 0x28
,  RightParenthesis=   ')'          // 0x29
,  Asterisk=           '*'          // 0x2A
,  PlusSign=           '+'          // 0x2B
,  Comma=              ','          // 0x2C
,  Hyphen=             '-'          // 0x2D
,  Period=             '.'          // 0x2E
,  RightSlash=         '/'          // 0x2F

,  _0_=                '0'          // 0x30
,  _1_=                '1'          // 0x31
,  _2_=                '2'          // 0x32
,  _3_=                '3'          // 0x33
,  _4_=                '4'          // 0x34
,  _5_=                '5'          // 0x35
,  _6_=                '6'          // 0x36
,  _7_=                '7'          // 0x37
,  _8_=                '8'          // 0x38
,  _9_=                '9'          // 0x39

,  Colon=              ':'          // 0x3A
,  Semicolon=          ';'          // 0x3B
,  LeftAngle=          '<'          // 0x3C
,  EqualSign=          '='          // 0x3D
,  RightAngle=         '>'          // 0x3E
,  QuestionMark=       '?'          // 0x3F
,  AtSign=             '@'          // 0x40

,  _A_=                'A'          // 0x41
,  _B_=                'A'          // 0x41
,  _C_=                'C'          // 0x43
,  _D_=                'D'          // 0x44
,  _E_=                'E'          // 0x45
,  _F_=                'F'          // 0x46
,  _G_=                'G'          // 0x47
,  _H_=                'H'          // 0x48
,  _I_=                'I'          // 0x49
,  _J_=                'J'          // 0x4A
,  _K_=                'K'          // 0x4B
,  _L_=                'L'          // 0x4C
,  _M_=                'M'          // 0x4D
,  _N_=                'N'          // 0x4E
,  _O_=                'O'          // 0x4F
,  _P_=                'P'          // 0x50
,  _Q_=                'Q'          // 0x51
,  _R_=                'R'          // 0x52
,  _S_=                'S'          // 0x53
,  _T_=                'T'          // 0x54
,  _U_=                'U'          // 0x55
,  _V_=                'V'          // 0x56
,  _W_=                'W'          // 0x57
,  _X_=                'X'          // 0x58
,  _Y_=                'Y'          // 0x59
,  _Z_=                'Z'          // 0x5A

,  LeftBracket=        '['          // 0x5B
,  LeftSlash=          '\\'         // 0x5C
,  RightBracket=       ']'          // 0x5D
,  Carat=              '^'          // 0x5E
,  Underscore=         '_'          // 0x5F
,  GraveAccent=        '`'          // 0x60

,  _a_=                'a'          // 0x61
,  _b_=                'b'          // 0x62
,  _c_=                'c'          // 0x63
,  _d_=                'd'          // 0x64
,  _e_=                'e'          // 0x65
,  _f_=                'f'          // 0x66
,  _g_=                'g'          // 0x67
,  _h_=                'h'          // 0x68
,  _i_=                'i'          // 0x69
,  _j_=                'j'          // 0x6A
,  _k_=                'k'          // 0x6B
,  _l_=                'l'          // 0x6C
,  _m_=                'm'          // 0x6D
,  _n_=                'n'          // 0x6E
,  _o_=                'o'          // 0x6F
,  _p_=                'p'          // 0x70
,  _q_=                'q'          // 0x71
,  _r_=                'r'          // 0x72
,  _s_=                's'          // 0x73
,  _t_=                't'          // 0x74
,  _u_=                'u'          // 0x75
,  _v_=                'v'          // 0x76
,  _w_=                'w'          // 0x77
,  _x_=                'x'          // 0x78
,  _y_=                'y'          // 0x79
,  _z_=                'z'          // 0x7A

,  LeftBrace=          '{'          // 0x7B
,  Bar=                '|'          // 0x7C
,  RightBrace=         '}'          // 0x7D
,  Tilde=              '~'          // 0x7E
,  CTL_BS=             0x7F         // Control-Backspace
,  ALT_ESC=            0x0101       // Alt-Escape
,  ALT_BS=             0x010E       // Alt-Backspace
,  BACKTAB=            0x010F       // Reverse horizontal tab
,  ALT_Q=              0x0110       // Alt-q
,  ALT_W=              0x0111       // Alt-w
,  ALT_E=              0x0112       // Alt-e
,  ALT_R=              0x0113       // Alt-r
,  ALT_T=              0x0114       // Alt-t
,  ALT_Y=              0x0115       // Alt-y
,  ALT_U=              0x0116       // Alt-u
,  ALT_I=              0x0117       // Alt-i
,  ALT_O=              0x0118       // Alt-o
,  ALT_P=              0x0119       // Alt-p
,  ALT_A=              0x011E       // Alt-a
,  ALT_S=              0x011F       // Alt-s
,  ALT_D=              0x0120       // Alt-d
,  ALT_F=              0x0121       // Alt-f
,  ALT_G=              0x0122       // Alt-g
,  ALT_H=              0x0123       // Alt-h
,  ALT_J=              0x0124       // Alt-j
,  ALT_K=              0x0125       // Alt-k
,  ALT_L=              0x0126       // Alt-l
,  ALT_Semicolon=      0x0127       // Alt-; : key
,  ALT_Quote=          0x0128       // Alt-' " key
,  ALT_GraveAccent=    0x0129       // Alt-` ~ key

,  ALT_Z=              0x012C       // Alt-z
,  ALT_X=              0x012D       // Alt-x
,  ALT_C=              0x012E       // Alt-c
,  ALT_V=              0x012F       // Alt-v
,  ALT_B=              0x0130       // Alt-b
,  ALT_N=              0x0131       // Alt-n
,  ALT_M=              0x0132       // Alt-m
,  ALT_Comma=          0x0133       // Alt-, < key
,  ALT_Period=         0x0134       // Alt-. > key
,  ALT_RightSlash=     0x0135       // Alt-/ ? key
,  F01=                0x013B       // F01
,  F02=                0x013C       // F02
,  F03=                0x013D       // F03
,  F04=                0x013E       // F04
,  F05=                0x013F       // F05
,  F06=                0x0140       // F06
,  F07=                0x0141       // F07
,  F08=                0x0142       // F08
,  F09=                0x0143       // F09
,  F10=                0x0144       // F10

,  Home=               0x0147       // Home
,  CursorUp=           0x0148       // Cursor up
,  PageUp=             0x0149       // Page up
,  CursorLeft=         0x014B       // Cursor left
,  Center=             0x014C       // Keypad 5
,  CursorRight=        0x014D       // Cursor right
,  End=                0x014F       // End
,  CursorDown=         0x0150       // Cursor down
,  PageDown=           0x0151       // Page down
,  Insert=             0x0152       // Insert
,  Delete=             0x0153       // Delete

,  CTL_F01=            0x015E       // Control-F01
,  CTL_F02=            0x015F       // Control-F02
,  CTL_F03=            0x0160       // Control-F03
,  CTL_F04=            0x0161       // Control-F04
,  CTL_F05=            0x0162       // Control-F05
,  CTL_F06=            0x0163       // Control-F06
,  CTL_F07=            0x0164       // Control-F07
,  CTL_F08=            0x0165       // Control-F08
,  CTL_F09=            0x0166       // Control-F09
,  CTL_F10=            0x0167       // Control-F10

,  ALT_F01=            0x0168       // Alt-F01
,  ALT_F02=            0x0169       // Alt-F02
,  ALT_F03=            0x016A       // Alt-F03
,  ALT_F04=            0x016B       // Alt-F04
,  ALT_F05=            0x016C       // Alt-F05
,  ALT_F06=            0x016D       // Alt-F06
,  ALT_F07=            0x016E       // Alt-F07
,  ALT_F08=            0x016F       // Alt-F08
,  ALT_F09=            0x0170       // Alt-F09
,  ALT_F10=            0x0171       // Alt-F10

,  CTL_CursorLeft=     0x0173       // Control-Cursor left
,  CTL_CursorRight=    0x0174       // Control-Cursor right
,  CTL_End=            0x0175       // Control-End
,  CTL_PageDown=       0x0176       // Control-Page down
,  CTL_Home=           0x0177       // Control-Home

,  ALT_1=              0x0178       // Alt-1
,  ALT_2=              0x0179       // Alt-2
,  ALT_3=              0x017A       // Alt-3
,  ALT_4=              0x017B       // Alt-4
,  ALT_5=              0x017C       // Alt-5
,  ALT_6=              0x017D       // Alt-6
,  ALT_7=              0x017E       // Alt-7
,  ALT_8=              0x017F       // Alt-8
,  ALT_9=              0x0180       // Alt-9
,  ALT_0=              0x0181       // Alt-0
,  ALT_Hyphen=         0x0182       // Alt-'-'
,  ALT_PlusSign=       0x0183       // Alt-'+'

,  CTL_PageUp=         0x0184       // Control-Page up
,  F11=                0x0185       // F11
,  F12=                0x0186       // F12
,  CTL_F11=            0x0189       // Control-F11
,  CTL_F12=            0x018A       // Control-F12

,  ALT_F11=            0x018B       // Alt-F11
,  ALT_F12=            0x018C       // Alt-F12

,  CTL_CursorUp=       0x018D       // Control-Cursor up
,  CTL_Center=         0x018F       // Control-Keypad 5
,  CTL_CursorDown=     0x0191       // Control-Cursor down
,  CTL_Insert=         0x0192       // Control-Insert
,  CTL_Delete=         0x0193       // Control-Delete

,  ALT_Home=           0x0197       // Alt-Home
,  ALT_CursorUp=       0x0198       // Alt-Cursor up
,  ALT_PageUp=         0x0199       // Alt-Page up
,  ALT_CursorLeft=     0x019B       // Alt-Cursor left
,  ALT_CursorRight=    0x019D       // Alt-Cursor right
,  ALT_End=            0x019F       // Alt-End
,  ALT_CursorDown=     0x01A0       // Alt-Cursor down
,  ALT_PageDown=       0x01A1       // Alt-Page down
,  ALT_Insert=         0x01A2       // Alt-Insert
,  ALT_Delete=         0x01A3       // Alt-Delete
,  ALT_TAB=            0x01A5       // Alt-TAB
,  ALT_PAD_ENTER=      0x01A6       // Alt-Keypad enter

,  SYS_RESIZE=         0x01F0       // Resize event

,  MOUSE_1=            0x0201       // Mouse button 1
,  MOUSE_2=            0x0202       // Mouse button 2
,  MOUSE_3=            0x0203       // Mouse button 3
,  MOUSE_WHEEL_DOWN=   0x0204       // Mouse wheel down (toward)
,  MOUSE_WHEEL_UP=     0x0205       // Mouse wheel up   (away)
,  MOUSE_WHEEL_LEFT=   0x0206       // Mouse wheel tilted left
,  MOUSE_WHEEL_RIGHT=  0x0207       // Mouse wheel tilted right
}; // enum
}; // class KeyCode

#endif // KEYCODE_H_INCLUDED
