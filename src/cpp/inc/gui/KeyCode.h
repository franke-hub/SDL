//----------------------------------------------------------------------------
//
//       Copyright (c) 2010 Frank Eskesen.
//
//       This file is free content, distributed under the MIT license.
//       (See accompanying file LICENSE.MIT or the original contained
//       within https://opensource.org/licenses/MIT)
//
//----------------------------------------------------------------------------
//
// Title-
//       KeyCode.h
//
// Purpose-
//       Enumerate the keyboard character codes.
//
// Last change date-
//       2010/01/01
//
// See also-
//       ../com/ASCII.h, ../com/KeyCode.h, ../com/ScanCode.h
//
//----------------------------------------------------------------------------
#ifndef GUI_KEYCODE_H_INCLUDED
#define GUI_KEYCODE_H_INCLUDED

#include "namespace.gui"
//----------------------------------------------------------------------------
//
// Class-
//       KeyCode
//
// Purpose-
//       Enumerate the standard and extended keyboard character set.
//
// Usage notes-
//       The shift modifier only applies to extended keys, i.e. keys with
//       codes >= 0x0000f800 and <= 0x0000f8ff, and the control keys, i.e.
//       keys with a code less than 0x00000020.
//
// Usage notes-
//       This map does not differentiate keypad and base keys.
//
// Implementation notes-
//       Extended codes use a private area of the Unicode character space.
//       Extended codes 0x0000f800 .. 0x0000f83f reserved for function keys.
//       Extended codes 0x0000f840 .. 0x0000f84f reserved for movement keys.
//       Extended codes 0x0000f850 .. 0x0000f87f reserved for expansion.
//       Extended codes 0x0000f880 .. 0x0000f8ff special purpose keys.
//
//----------------------------------------------------------------------------
class KeyCode {                     // The extended character set
public:
enum                                // The extended character set
{  NUL=                0x00         // '\0' NUL character (no key)
,  BS=                 0x08         // '\b' BackSpace
,  TAB=                0x09         // '\t' Horizontal tab
,  ENTER=              0x0D         // '\n' Enter
,  ESC=                0x1B         // ESCape

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
,  DEL=                0x7F         // DELete

//----------------------------------------------------------------------------
// Modifiers
,  _SHIFT=             0x10000000   // Shift modifier (For extended keys)
,  _ALT=               0x20000000   // Alt/meta modifier
,  _CTRL=              0x40000000   // Control modifier
,  _CODE=              0x80000000   // Keycode modifier (No mapping for code)

//----------------------------------------------------------------------------
// Extended Function keys
,  EXTENDED_ORIGIN=    0x0000f800   // First extended key
,  F01=                0x0000f801   // F01
,  F02=                0x0000f802   // F02
,  F03=                0x0000f803   // F03
,  F04=                0x0000f804   // F04
,  F05=                0x0000f805   // F05
,  F06=                0x0000f806   // F06
,  F07=                0x0000f807   // F07
,  F08=                0x0000f808   // F08
,  F09=                0x0000f809   // F09
,  F10=                0x0000f80a   // F10
,  F11=                0x0000f80b   // F11
,  F12=                0x0000f80c   // F12
,  F13=                0x0000f80d   // F13
,  F14=                0x0000f80e   // F14
,  F15=                0x0000f80f   // F15
,  F16=                0x0000f810   // F16
,  F17=                0x0000f811   // F17
,  F18=                0x0000f812   // F18
,  F19=                0x0000f813   // F19
,  F20=                0x0000f814   // F20
,  F21=                0x0000f815   // F21
,  F22=                0x0000f816   // F22
,  F23=                0x0000f817   // F23
,  F24=                0x0000f818   // F24
,  F25=                0x0000f819   // F25
,  F26=                0x0000f81a   // F26
,  F27=                0x0000f81b   // F27
,  F28=                0x0000f81c   // F28
,  F29=                0x0000f81d   // F29
,  F30=                0x0000f81e   // F30
,  F31=                0x0000f81f   // F31
,  F32=                0x0000f820   // F32
,  F33=                0x0000f821   // F33
,  F34=                0x0000f822   // F34
,  F35=                0x0000f823   // F35
,  F36=                0x0000f824   // F36
,  F37=                0x0000f825   // F37
,  F38=                0x0000f826   // F38
,  F39=                0x0000f827   // F39
,  F40=                0x0000f828   // F40
,  F41=                0x0000f829   // F41
,  F42=                0x0000f82a   // F42
,  F43=                0x0000f82b   // F43
,  F44=                0x0000f82c   // F44
,  F45=                0x0000f82d   // F45
,  F46=                0x0000f82e   // F46
,  F47=                0x0000f82f   // F47

//----------------------------------------------------------------------------
// Extended keyboard characters
,  Up=                 0x0000f841   // Move up
,  Down=               0x0000f842   // Move down
,  Left=               0x0000f844   // Move left
,  Right=              0x0000f848   // Move right
,  Center=             0x0000f84f   // Move center
,  UpLeft=             0x0000f845   // Move northwest
,  UpRight=            0x0000f849   // Move northeast
,  DownLeft=           0x0000f846   // Move southwest
,  DownRight=          0x0000f84a   // Move southeast

,  Home=               0x0000f8e0   // Home
,  End=                0x0000f8e1   // End
,  PageUp=             0x0000f8e4   // Page up
,  PageDown=           0x0000f8e5   // Page down
,  Print=              0x0000f8ee   // Print
,  Pause=              0x0000f8ef   // Pause

,  Insert=             0x0000f8f1   // Insert lock
,  ScrollLock=         0x0000f8f2   // Scroll lock
,  Delete=             0x0000f8ff   // Delete
,  EXTENDED_END=       0x0000f8ff   // Last extended key
}; // enum
}; // class KeyCode
#include "namespace.end"

#endif // GUI_KEYCODE_H_INCLUDED
