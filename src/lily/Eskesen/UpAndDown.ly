%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%
%%       Copyright (C) 2007 Frank Eskesen.
%%
%%       This file is free content, distributed under cc by-sa version 3.0,
%%       with attribution required.
%%       (See accompanying file LICENSE.BY_SA-3.0 or the original contained
%%       within https://creativecommons.org/licenses/by-sa/3.0/us/legalcode)
%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%
%% Title-
%%       UpAndDown.ly
%%
%% Purpose-
%%       In which the melody goes up and down.
%%
%% Last change date-
%%       2007/01/01
%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
\version "2.8.8"
\header {
   title = "Up and Down"
   composer = "Frank Eskesen"
%  poet = "Frank Eskesen"

%% In which the melody goes up and down, over and over again.
%% Is this original? Carol thinks not.
}

%% showLastLength = R1*4
softBreak = { \break }

signature = {
   \key g \major
   \override Staff.TimeSignature #'style = #'()
   \override Staff.VerticalAxisGroup #'minimum-Y-extent = #'(-3 . 3)
   \time 4/4
}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% The Singer's Lyrics
sLyric = \new Lyrics \lyricmode {
   \set associatedVoice = #"melody"
   \barNumberCheck #1
   One4 two three "(four)" |
   One4 two three "(four)" |

   One4 two three "(four)" |
   One4 two three "(four)" |

   One8 and two and three4 four |
   One8 and two and three4 four |

   One8 and two and three4 four |
   One8 and two and three4 four |

   \barNumberCheck #9
   One4 two three "(four)" |
   One4 two three "(four)" |

   One4 two Three8 and "(four)" and |
   One4 two three "(four)" |

   "(One)"8 and two and three4 "(four)" |
   "(One)"8 and two and three4 "(four)" |

   One4 two three "(four)" |
   One4 two three "(four)" |

   \barNumberCheck #17
   One4 two three "(four)" |
   One4 two three "(four)" |

   One4 two Three8 and "(four)" and |
   One4 two three "(four)" |

   One4 two three "(four)" |
   One4 two three "(four)" |

   One4 two three "(four)" |
   \softBreak _1

   \bar "|."
}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% The Singer's voice
sVoice = \new Voice = "melody" {
   \barNumberCheck #1
   \relative c' { e4 fis g2 } |
   \relative c' { g'4 fis e2 } |

   \relative c' { e4 g b2 } |
   \relative c' { b'4 a g2 } |

   \relative c' { c8 e g c, b'4 g } |
   \relative c' { b'8 a g b a4 g } |

   \relative c' { g'8 e c g' d4 b } |
   \relative c' { b8 c d b e4 c } |

   \barNumberCheck #9
   \relative c' { e4 fis b2 } |
   \relative c' { b'4 a g2 } |

   \relative c' { e4 fis d'8 d4 d8 } |
   \relative c' { d'4 a g2 } |

   \relative c' { r8 c e c d2 } |
   \relative c' { r8 b d b c2 } |

   \relative c' { e4 fis b2 } |
   \relative c' { b'4 a g2 } |

   \barNumberCheck #17
   \relative c' { e4 fis b2 } |
   \relative c' { b'4 a g2 } |

   \relative c' { e4 fis d'8 d4 d8 } |
   \relative c' { d'4 a g2 } |

   \relative c' { e4 fis b2 } |
   \relative c' { b'4 a g2 } |

   \relative c' { g'4 a g2^\fermata } |
   \softBreak s1

   \bar "|."
}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Piano Right Hand
pRH = {
   \barNumberCheck #1
   \relative c' { <c e g>4 <c fis a> <d g b>2 } |
   \relative c' { <d g b>4 <d fis a> <c e g>2 } |

   \relative c' { c8 e g c, <d g b>4 d8 g } |
   \relative c' { <d g b>4 <d fis a> c8 g' c, e } |

   \relative c' { c8 g' c, e d b' d, g } |
   \relative c' { d8 fis d g c, g' c, e } |

   \relative c' { c8 g' c, e d b' d, g } |
   \relative c' { d8 b' d, g c, g' c, e } |

   \barNumberCheck #9
   \relative c' { c8 g' a b d, b' d, g } |
   \relative c' { d8 b' a g c, g' c, e } |

   \relative c' { c8 g' a  fis d b' d, g } |
   \relative c' { d8 b' a fis c g' c, e } |

   \relative c' { c8 g' a b d, b' d, g } |
   \relative c' { d8 b' a g c, g' c, e } |

   \relative c' { c8 g' a fis d b' d, g } |
   \relative c' { d8 g a fis c g' c, e } |

   \barNumberCheck #17
   \relative c' { c8 e fis a <d, g b>4 d8 b' } |
   \relative c' { d'8 b a fis <c e g>4 g'8 e  } |

   \relative c' { c8 g' a  fis d b' d, g } |
   \relative c' { d8 b' a fis c g' c, e } |

   \relative c' { <c e g>4 <c fis a> <d g b>2 } |
   \relative c' { <d g b>4 <d fis a> <c e g>2 } |

   \relative c' { <c e g>4 <d fis a> <b d g>2^\fermata } |
   \softBreak s1

   \bar "|."
}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Piano Left Hand
pLH = \relative c  {
   \barNumberCheck #1
   \relative c  { <c g'>2 <d b'>~ } |
   \relative c  { <d b'>2 <c g'>~ } |

   \relative c  { <c g'>2 <d b'>~ } |
   \relative c  { <d b'>2 <c g'>~ } |

   \relative c  { <c g'>2 <d b'>~ } |
   \relative c  { <d b'>2 <c g'>~ } |

   \relative c  { <c g'>2 <d b'>~ } |
   \relative c  { <d b'>2 <c g'>~ } |

   \barNumberCheck #9
   \relative c  { <c g'>2 <d b'>~ } |
   \relative c  { <d b'>2 <c g'>~ } |

   \relative c  { <c g'>2 <d b'>~ } |
   \relative c  { <d b'>2 <c g'>~ } |

   \relative c  { <c g'>2 <d b'>~ } |
   \relative c  { <d b'>2 <c g'>~ } |

   \relative c  { <c g'>2 <d b'>~ } |
   \relative c  { <d b'>2 <c g'>~ } |

   \barNumberCheck #17
   \relative c  { <c g'>2 <d b'>~ } |
   \relative c  { <d b'>2 <c g'>~ } |

   \relative c  { <c g'>2 <d b'>~ } |
   \relative c  { <d b'>2 <c g'>~ } |

   \relative c  { <c g'>2 <d b'>~ } |
   \relative c  { <d b'>2 <c g'>~ } |

   \relative c  { <c g'>2 <b g'>2^\fermata } |
   \softBreak s1

   \bar "|."
}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% PS/PDF output
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
\score
{
   \new GrandStaff
   {
   <<
     \new Staff
     {
     <<
       \signature
       \clef treble
       <<
         \sVoice
         \sLyric
       >>
     >>
     }

     \new PianoStaff
     {
     <<
       \new Staff
       {
         \signature
         \clef treble
         \pRH
       }
       \new Staff
       {
         \signature
         \clef bass
         \pLH
       }
     >>
     }
   >>
   }
}
\paper
{
   between-system-padding = #1
   ragged-bottom = ##t
   ragged-last-bottom = ##t
}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% MIDI output
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
sInstrument = \set Staff.midiInstrument = "voice oohs"
sInstrument = \set Staff.midiInstrument = "synth voice"
sInstrument = \set Staff.midiInstrument = "acoustic grand"
sInstrument = \set Staff.midiInstrument = "choir aahs"
\score
{
   \unfoldRepeats
   {
     \new GrandStaff
     {
     <<
       \new Staff
       {
         \signature
         \sInstrument
         \sVoice
       }
       \new PianoStaff
       {
       <<
         \new Staff
         {
           \signature
           \pRH
         }
         \new Staff
         {
           \signature
           \pLH
         }
       >>
       }
     >>
     }
   }
   \midi
   {
%%   \tempo 4 = 72   %% For Debugging
     \tempo 4 = 112  %% Normal tempo
   }
}
