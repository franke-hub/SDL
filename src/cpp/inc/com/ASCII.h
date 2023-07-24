//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under creative commons CC0,
//       explicitly released into the Public Domain.
//       (See accompanying html file LICENSE.ZERO or the original contained
//       within https://creativecommons.org/publicdomain/zero/1.0/legalcode)
//
//----------------------------------------------------------------------------
//
// Title-
//       ASCII.h
//
// Purpose-
//       Enumerate the ascii character set. (Only used as reference.)
//
// Last change date-
//       2007/01/01
//
// See also-
//       KeyCode.h, ScanCode.h
//
//----------------------------------------------------------------------------
#ifndef ASCII_H_INCLUDED
#define ASCII_H_INCLUDED

//----------------------------------------------------------------------------
//
// Class-
//       ASCII
//
// Purpose-
//       Enumerate the ASCII character set.
//
//----------------------------------------------------------------------------
class ASCII {                       // ASCII character set
public:
enum                                // ASCII enumeration
{  NUL=                0x00         // '\0' NULL character
,  SOH=                0x01         // Start Of Heading
,  STX=                0x02         // Start of TeXt
,  ETX=                0x03         // End of TeXt
,  EOT=                0x04         // End Of Transmission
,  ENQ=                0x05         // ENQuery
,  ACK=                0x06         // ACKnowledge
,  BEL=                0x07         // '\a' Alarm (BELl)
,  BS=                 0x08         // '\b' BackSpace
,  HT=                 0x09         // '\t' Horizontal Tab
,  LF=                 0x0A         // '\n' Line Feed
,  VT=                 0x0B         // '\v' Vertical Tab
,  FF=                 0x0C         // '\f' Form Feed
,  CR=                 0x0D         // '\r' Carriage Return
,  SO=                 0x0E         // Shift Out
,  SI=                 0x0F         // Shift In
,  DLE=                0x10         // Data Link Escape
,  DC1=                0x11         // Device Control 1
,  DC2=                0x12         // Device Control 2
,  DC3=                0x13         // Device Control 3
,  DC4=                0x14         // Device Control 4
,  NAK=                0x15         // Negative AcKnowledge
,  SYN=                0x16         // SYNchronize
,  ETB=                0x17         // End of Transmission Block
,  CAN=                0x18         // CANcel
,  EM=                 0x19         // End of Media
,  SUB=                0x1A         // SUBstitute
,  ESC=                0x1B         // ESCape
,  FS=                 0x1C         // Field Separator
,  GS=                 0x1D         // Group Separator
,  RS=                 0x1E         // Record Separator
,  US=                 0x1F         // Unit Separator
,  SP=                 0x20         // SPace

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
,  _B_=                'B'          // 0x42
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
///////////////        'Cs'         // 0x80 CIDILLA
///////////////        'u:'         // 0x81 u umlaut
///////////////        'e''         // 0x82 e acute accent
///////////////        'a^'         // 0x83 a circumflex accent
///////////////        'a:'         // 0x84 a umlaut
///////////////        'a`'         // 0x85 a grave accent
///////////////        'ao'         // 0x86 a circle
///////////////        'cs'         // 0x87 cidilla
///////////////        'e^'         // 0x88 e circumflex accent
///////////////        'e:'         // 0x89 e umlaut
///////////////        'e`'         // 0x8A e grave accent
///////////////        'i:'         // 0x8B i umlaut
///////////////        'i^'         // 0x8C i circumflex accent
///////////////        'i`'         // 0x8D i grave accent
///////////////        'A:'         // 0x8E A umlaut
///////////////        'Ao'         // 0x8F A circle
///////////////        'E''         // 0x90 E acute accent
///////////////        'ae'         // 0x91 ae
///////////////        'ff'         // 0x92 ff
///////////////        'o^'         // 0x93 o circumflex accent
///////////////        'o:'         // 0x94 o umlaut
///////////////        'o`'         // 0x95 o grave accent
///////////////        'u^'         // 0x96 u circumflex accent
///////////////        'u`'         // 0x97 u grave accent
///////////////        'y:'         // 0x98 y umlaut
///////////////        'O:'         // 0x99 O umlaut
///////////////        'U:'         // 0x9A U umlaut
///////////////        'o/'         // 0x9B o /
///////////////        'LL'         // 0x9C British pound sign
///////////////        'O/'         // 0x9D O /
///////////////        'x*'         // 0x9E x (Times sign)
///////////////        '  '         // 0x9F f (function)
///////////////        'a''         // 0xA0 a acute accent
///////////////        'i''         // 0xA1 i acute accent
///////////////        'o''         // 0xA2 o acute accent
///////////////        'u''         // 0xA3 u acute accent
///////////////        'n~'         // 0xA4 n tilde
///////////////        'N~'         // 0xA5 N tilde
///////////////        'a-'         // 0xA6 a hyphen
///////////////        'o-'         // 0xA7 o hyphen
///////////////        '??'         // 0xA8 ? inverted
///////////////        '  '         // 0xA9 (R) registered
///////////////        '  '         // 0xAA NOT sign
///////////////        '  '         // 0xAB 1/2
///////////////        '  '         // 0xAC 1/4
///////////////        '!!'         // 0xAD ! inverted
///////////////        '<<'         // 0xAE <<
///////////////        '>>'         // 0xAF >>
///////////////        '  '         // 0xB0 Light gray
///////////////        '  '         // 0xB1 Gray
///////////////        '  '         // 0xB2 Dark gray
///////////////        '  '         // 0xB3 Edge
///////////////        '  '         // 0xB4 Edge, right connector
///////////////        'A''         // 0xB5 A acute accent
///////////////        'A^'         // 0xB6 A circumflex accent
///////////////        'A`'         // 0xB7 A grave accent
///////////////        '  '         // 0xB8 (C) copyright
///////////////        '  '         // 0xB9 Pipe, right connector
///////////////        '  '         // 0xBA Pipe
///////////////        '  '         // 0xBB Pipe, top right
///////////////        '  '         // 0xBC Pipe, bottom right
///////////////        'cc'         // 0xBD Cent sign
///////////////        'Y='         // 0xBE Yen sign
///////////////        '  '         // 0xBF Edge, top right
///////////////        '  '         // 0xC0 Edge, bottom left
///////////////        '  '         // 0xC1 Edge, bottom connector
///////////////        '  '         // 0xC2 Edge, top connector
///////////////        '  '         // 0xC3 Edge, left connector
///////////////        '  '         // 0xC4 Edge, horizontal
///////////////        '  '         // 0xC5 Edge, center connector
///////////////        'a~'         // 0xC6 a tilde
///////////////        'A~'         // 0xC7 A tilde
///////////////        '  '         // 0xC8 Pipe, bottom left
///////////////        '  '         // 0xC9 Pipe, top left
///////////////        '  '         // 0xCA Pipe, bottom connector
///////////////        '  '         // 0xCB Pipe, top connector
///////////////        '  '         // 0xCC Pipe, left connector
///////////////        '  '         // 0xCD Pipe, horizontal
///////////////        '  '         // 0xCE Pipe, center connector
///////////////        '  '         // 0xCF :O:
///////////////        '  '         // 0xD0 delta
///////////////        '  '         // 0xD1 Deuchmark
///////////////        'E^'         // 0xD2 E circumflex accent
///////////////        'E:'         // 0xD3 E umlaut
///////////////        'E`'         // 0xD4 E grave accent
///////////////        '  '         // 0xD5 Edge, upper
///////////////        'I''         // 0xD6 I acute accent
///////////////        'I^'         // 0xD7 I circumflex accent
///////////////        'I:'         // 0xD8 I umlaut
///////////////        '  '         // 0xD9 Edge, bottom right (upper)
///////////////        '  '         // 0xDA Edge, top left (upper)
///////////////        '  '         // 0xDB Black
///////////////        '  '         // 0xDC Lower black
///////////////        '  '         // 0xDD
///////////////        'I`'         // 0xDE I grave accent
///////////////        '  '         // 0xDF Upper black
///////////////        'O''         // 0xE0 O acute accent
///////////////        'ss'         // 0xE1 German ff
///////////////        'O^'         // 0xE2 O circumflex accent
///////////////        'O`'         // 0xE3 O grave accent
///////////////        'o~'         // 0xE4 o tilde
///////////////        'O~'         // 0xE5 O tilde
///////////////        '  '         // 0xE6
///////////////        '  '         // 0xE7
///////////////        '  '         // 0xE8
///////////////        'U''         // 0xE9 U acute accent
///////////////        'U^'         // 0xEA U circumflex accent
///////////////        'U`'         // 0xEB U grave accent
///////////////        'y''         // 0xEC y acute accent
///////////////        'Y''         // 0xED Y acute accent
///////////////        '  '         // 0xEE
///////////////        ''''         // 0xEF acute accent
///////////////        '  '         // 0xF0
///////////////        '+-'         // 0xF1 Plus or minus
///////////////        '  '         // 0xF2
///////////////        '  '         // 0xF3 3/4
///////////////        'PP'         // 0xF4 Paragraph
///////////////        '  '         // 0xF5
///////////////        ':-'         // 0xF6 division sign :-
///////////////        '  '         // 0xF7
///////////////        'oo'         // 0xF8 circle
///////////////        '::'         // 0xF9 umlaut
///////////////        '..'         // 0xFA dot (above)
///////////////        '1S'         // 0xFB 1 superscript
///////////////        '3S'         // 0xFC 3 superscript
///////////////        '2S'         // 0xFD 2 superscript
///////////////        '  '         // 0xFE
///////////////        '  '         // 0xFF
}; // enum
}; // class ASCII

#endif // ASCII_H_INCLUDED
