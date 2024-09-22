%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%
%%       Copyright (C) 2020 Frank Eskesen.
%%
%%       This file is free content, distributed under cc by-sa version 3.0,
%%       with attribution required.
%%       (See accompanying file LICENSE.BY_SA-3.0 or the original contained
%%       within https://creativecommons.org/licenses/by-sa/3.0/us/legalcode)
%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%       This copyright applies only to this Lily representation, not to the
%%       score by Ludwig van Beethoven.
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%
%% Title-
%%       Moonlight.ly
%%
%% Purpose-
%%       Moonlight Sonata, Ludwig van Beethoven.
%%
%% Last change date-
%%       2020/02/05
%%
%% Implementation notes-
%%       B# is used in place of C#b ... C#
%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
\version "2.18.8"
\header {
   title = "Sonata quasi una Fantasia"
   composer = "Ludwig van BEETHOVEN"
   copyright = "Public Domain"
}

%% Triplet add-ons %%%%%%%%%%%%%%%%%%%%%%%% (A# A# A#)
%%%%%%% = \relative c  {a8    a    a   } %% (A  A  A )
rbsfcd  = \relative c  {fis8  cis' dis } %% (F# C# D#)

rtacsds = \relative c' {a8    cis  dis } %% (A  C# D#)
rtacsfs = \relative c' {a8    cis  fis } %% (A  C# F#)
rtadsfs = \relative c' {a8    dis  fis } %% (A  D# F#)
rtaegs  = \relative c' {a8    e'   gis } %% (A  E  G#)
rtsbfg  = \relative c' {bis8  fis  gis } %% (B# F# G#)
rtcsegs = \relative c' {cis8  e    gis } %% (C# E  G#)
rtsceg  = \relative c' {cis8  eis  gis } %% (C# E# G#)
rtdfsa  = \relative c' {d8    fis  a   } %% (D  F# A )

signature = {
   \key e \major
   \override Staff.TimeSignature #'style = #'()
   \override Staff.VerticalAxisGroup #'minimum-Y-extent = #'(-3 . 3)
   \tempo 4 = 52
   \time 4/4
}

%% softBreak = {\break}
softBreak = {}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% The Voices
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% rInstrument = \set Staff.midiInstrument = "acoustic grand"
%% lInstrument = \set Staff.midiInstrument = "acoustic grand"

%% For voice separation
rInstrument = \set Staff.midiInstrument = "acoustic grand"
lInstrument = \set Staff.midiInstrument = "acoustic grand"

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Right hand
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% Sorted by notes, not name
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% \relative c (Bass)
rbregs  = \relative c  {r8    e    gis } %% (-- E  G#)

%%%%%%% = \relative c  {a8    a    a   } %% (A  A  A )
rbsceg  = \relative c  {cis8  eis  gis } %% (C# E# G#)
rbcsfsa = \relative c  {cis8  fis  a   } %% (C# F# A )
rbscfg  = \relative c  {cis8  fis  gis } %% (C# F# G#)

jbdsafs = \relative c  {dis8  a'   fis } %% (D# A  F#) %% 3rd lower

rbebcs  = \relative c  {e8    b'   cis } %% (E  B  C#)
jbecsgs = \relative c  {e8    cis' gis } %% (E  C# G#) %% 3rd lower
rbeegs  = \relative c  {e8    e'   gis } %% (E  E  G#)
rbegscs = \relative c  {e8    gis  cis } %% (E  G# C#)

rbfsacs = \relative c  {fis8  a    cis } %% (F# A  C#)
rbfsads = \relative c  {fis8  a    dis } %% (F# A  D#)
rbsfac  = \relative c  {fis8  ais  cis } %% (F# A# C#)
rbfsbd  = \relative c  {fis8  b    d   } %% (F# B  D )
rbsfbd  = \relative c  {fis8  bis  dis } %% (F# B# D#)
rbfscse = \relative c  {fis!8 cis' e   } %% (F# C# E )

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% relative c' (Tenor)
rbsss   = \relative c  {s8    s    s   } %% (        )
rtsss   = \relative c  {s8    s    s   } %% (        )
rtrrr   = \relative c  {r8    r    r   } %% (-- -- --)
rtrbsds = \relative c' {r8    bis  dis } %% (-- B# D#)

%%%%%%% = \relative c' {a8    a    a   } %% (A  A  A )
rtabds  = \relative c' {a8    b    dis } %% (A  B  E )
rtabsds = \relative c' {a8    bis  dis } %% (A  B# D#)
rtacse  = \relative c' {a8    cis  e   } %% (A  C# E )
rtacsfs = \relative c' {a8    cis  fis } %% (A  C# F#)
rtadfs  = \relative c' {a8    d    fis } %% (A  D  F#)

rtbcsgs = \relative c' {b8    cis  gis'} %% (B  C# G#)
rtbdes  = \relative c' {b8    d    eis } %% (B  D  E#)
rtbdfs  = \relative c' {b8    d    fis } %% (B  D  F#)
rtbdsfs = \relative c' {b8    dis  fis } %% (B  D# F#)
rtbeg   = \relative c' {b8    e    g   } %% (B  E  G )
rtbegs  = \relative c' {b8    e    gis } %% (B  E  G#)
itbsads = \relative c' {bis8  a    dis } %% (B# A  D#) %% 2nd lower
rtbfsa  = \relative c' {b8    fis' a   } %% (B  F# A )
rtbsfsa = \relative c' {bis8  fis' a   } %% (B# F# A#)
rtsbfg  = \relative c' {bis8  fis' gis } %% (B# F# G#)

rtcsegs = \relative c' {cis8  e    gis } %% (C# E  G#)
rtcsfsa = \relative c' {cis8  fis  a   } %% (C# F# A )
rtcseas = \relative c' {cis8  e    ais } %% (C# E  A#)
itcsegs = \relative c' {cis8  e,   gis } %% (C# E  G#) %% 2nd lower
rtcsgsb = \relative c' {cis8  gis' b   } %% (C# G# B )

rtdsfsa = \relative c' {dis8  fis  a   } %% (D# F# A )
rtsdfg  = \relative c' {dis8  fis  gis } %% (D# F# E#)

iteegs  = \relative c' {e8    e,   gis } %% (E  E  G#) %% 2nd lower
rtegscs = \relative c' {e8    gis  cis } %% (E  G# C#)

itsfbd  = \relative c' {fis8  bis, dis } %% (F# B# D#) %% 2nd lower

rtgbcs  = \relative c' {g8    b    cis } %% (G  B  C#)
rtgbd   = \relative c' {g8    b    d   } %% (G  B  D )
rtgbe   = \relative c' {g8    b    e   } %% (G  B  E )
rtgbf   = \relative c' {g8    b    f'  } %% (G  B  F )
rtgce   = \relative c' {g8    c    e   } %% (G  C  E )
rtgcse  = \relative c' {g8    cis  e   } %% (G  C# E )
rtsgbd  = \relative c' {gis8  bis  dis } %% (G# B# D#)
rtgsbe  = \relative c' {gis8  b    e   } %% (G# B  E )
rtsgbf  = \relative c' {gis8  bis  fis'} %% (G# B# F#)
rtsgcd  = \relative c' {gis8  cis  dis } %% (G# C# D#)
rtgscse = \relative c' {gis8  cis  e   } %% (G# C# E )
rtgsdse = \relative c' {gis8  cis  e   } %% (G# D# E )
rtsgdf  = \relative c' {gis8  dis' fis } %% (G# D# F#)

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% relative c'' (Soprano)
%%%%%%% = \relative c' {a'8   a    a   } %% (A  A  A )
isabsds = \relative c' {a'!8  bis, dis } %% (A# B# D#) %% 2nd lower (!)
issbbd  = \relative c' {bis'8 bis, dis } %% (B# B# D#) %% 2nd lower
iscsegs = \relative c' {cis'8 e,   gis } %% (C# E  G#) %% 2nd lower
iseegs  = \relative c' {e'8   e,   gis } %% (E  E  G#) %% 2nd lower
issgbd  = \relative c' {gis'8 bis, dis } %% (G# B# D#) %% 2nd lower

%%============================================================================
rVoiceU = \relative c'' {           %% 4/4 Treble clef
%{ COMMENT OUT %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% COMMENT IN %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% %}
   s1 s s s |                       %% 01
   s2. {gis8.[ gis16]} |            %% 05
   {gis2. gis8.[ gis16]} |          %% 06
   {gis2 a} |                       %% 07
   {gis2 fis4 b} |                  %% 08
   {e,4 r4 r2} |                    %% 09
   {r2 r4 g8.[ g16]} |              %% 10
   {g2. g8.[ g16]} |                %% 11
   {g2.( fis!4)} |                  %% 12
   {fis2( g4 e)} |                  %% 13
   {fis2 fis2(} |                   %% 14
   {b,4) r r b'} |                  %% 15
   {c2. ais4} |                     %% 16
   {b2. b4(} |                      %% 17
   {c2. ais4} |                     %% 18
   {b2) b} |                        %% 19
   {b2( a} |                        %% 20
   {g2 fis} |                       %% 21
   {cis2) cis} |                    %% 22
   {s2 r4 cis'8. cis16} |           %% 23
   {cis2. cis8. cis16} |            %% 24
   {cis2 bis4 cis} |                %% 25
   {dis2. dis4} |                   %% 26
   {e2 dis4 cis} |                  %% 27
   {bis8 r gis4 a fis} |            %% 28
   { s1 s s } |                     %% 29, 30, 31
   { s1 s s } |                     %% 32, 33, 34
   { s1 s s } |                     %% 35, 36, 37
   { s1 s s } |                     %% 38, 39, 40
   s1                               %% 41
   {r2. gis8. gis16} |              %% 42
   {gis2. gis8. gis16} |            %% 43
   {gis2 a} |                       %% 44
   {gis2 fis4 b} |                  %% 45
   {e,4 r r b'8. b16} |             %% 46
   {b2. b8. b16} |                  %% 47
   {b2 bis4 cis} |                  %% 48
   {dis2 e} |                       %% 49
   {d2 bis} |                       %% 50
   {cis2. cis4} |                   %% 51
   {d2. bis4} |                     %% 52
   {cis2. cis4} |                   %% 53
   {d2. bis4} |                     %% 54
   {cis2 cis} |                     %% 55
   {b2. b4} |                       %% 56
   {a4 a gis gis} |                 %% 57
   {fis2 gis4 a} |                  %% 58
   {gis2 gis} |                     %% 59
   {cis,4 r r2} |                   %% 60
   {s4 s s s} |                     %% 61
   s1 |                             %% 62
   {s2. bis4(} |                    %% 63
   {cis8) s s2.} |                  %% 64
   s1 |                             %% 65
   s1 |                             %% 66
   s1 |                             %% 67
   {r2 <cis e a>2} |                %% 68
   {<cis e a>1\fermata} |           %% 69
   \bar "|."
} %% rVoiceU

%%============================================================================
rVoiceL = \scaleDurations 2/3 \relative c' { %% 2/3 Treble clef
%{ COMMENT OUT %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% COMMENT IN %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% %}
   \rtgscse \rtgscse \rtgscse \rtgscse | %% 01
   \rtgscse \rtgscse \rtgscse \rtgscse | %% 02
   \rtacse  \rtacse  \rtadfs  \rtadfs  | %% 03
   \rtsgbf  \rtgscse \rtsgcd  \rbsfbd  | %% 04
   \rbegscs \rtgscse \rtgscse \rtgscse | %% 05
   \rtsgdf  \rtsgdf  \rtsgdf  \rtsgdf  | %% 06
   \rtgsdse \rtgsdse \rtacsfs \rtacsfs | %% 07
   \rtgsbe  \rtgsbe  \rtabds  \rtabds  | %% 08
   \rtgsbe  \rtgsbe  \rtgsbe  \rtgsbe  | %% 09
   \rtgbe   \rtgbe   \rtgbe   \rtgbe   | %% 10
   \rtgbf   \rtgbf   \rtgbf   \rtgbf   | %% 11
   \rtgce   \rtgbe   \rtgcse  \rbfscse | %% 12
   \rbfsbd  \rbfsbd  \rtgbcs  \rbebcs  | %% 13
   \rbfsbd  \rbfsbd  \rbsfac  \rbsfac  | %% 14
   \rtbdfs  \rtbdfs  \rtbdsfs \rtbdsfs | %% 15
   \rtbeg   \rtbeg   \rtbeg   \rtbeg   | %% 16
   \rtbdsfs \rtbdsfs \rtbdsfs \rtbdsfs | %% 17
   \rtbeg   \rtbeg   \rtbeg   \rtbeg   | %% 18
   \rtbdsfs \rtbdsfs \rtbdes  \rtbdes  | %% 19
   \rtbcsgs \rtbcsgs \rtacsfs \rtacsfs | %% 20
   \rtgbd   \rtgbd   \rbfsads \rbfsads | %% 21
   \rtsss   \rtsss   \rtsss   \rtsss   | %% 22
   \rtsss   \rtacsfs \rtcsfsa \rtcsfsa | %% 23
   \rtcsgsb \rtcsgsb \rtcsgsb \rtcsgsb | %% 24
   \rtcsfsa \rtcsfsa \rtbsfsa \rtcsfsa | %% 25
   \rtsdfg  \rtsdfg  \rtsdfg  \rtsdfg  | %% 26
   \rtegscs \rtegscs \rtdsfsa \rtcseas | %% 27
   \issbbd  \issgbd  \isabsds \itsfbd  | %% 28
   \rtrbsds \rtsgbd  \rtabsds \rbsfbd  | %% 29
   \rbeegs  \iscsegs \iseegs  \iscsegs | %% 30
   \rtsss   \rtsss   \rtsss   \rtsss   | %% 31
   \relative c' {
     \rtsss   \rtsss
     {bis8 fis' dis} {a' fis bis}    | %% 32
   }
   \rtsss
   \relative c' {
     {e8 cis gis'} {e cis' gis}
     {e' cis gis}                      | %% 33
   }
   \relative c' {
     {cis8 fisis e} {ais fisis cis'}
     {ais8 e' cis} {fisis e ais}       | %% 34
   }
   \relative c' {
     {fis!8 bis a!} {dis bis fis'}
     {dis8 a' fis} {bis a dis}         | %% 35
   }
   \relative c''' {
     {bis8 fis a} {dis, fis bis,}
     {dis8 a bis} {fis a dis,}         | %% 36
   }
   \relative c' {
     {fis8 bis, dis} {s s s}
     {s s s} {s s s}                   | %% 37
   }
   \rtsss   \rtsss   \rtsss   \rtsss   | %% 38
   \rtsss   \rtsss   \rtsss   \rtsss   | %% 39
   \rtsss   \rtsss   \rtsss   \rtsss   | %% 40
   \rtsss   \rtsss   \rtsss   \rtsss   | %% 41
   \relative c {
     {e8 gis cis} {gis cis e}
     {gis, cis e} {gis, cis e}         | %% 42
   }
   \relative c' {
     {gis8 dis' fis} {gis,8 dis' fis}
     {gis,8 dis' fis} {gis,8 dis' fis} | %% 43
   }
   \rtgscse \rtgscse \rtacsfs \rtacsfs | %% 44
   \rtgsbe  \rtgsbe  \rtabds  \rtabds  | %% 45
   \rtgsbe  \rtbegs  \rtbegs  \rtbegs  | %% 46
   \rtbfsa  \rtbfsa  \rtbfsa  \rtbfsa  | %% 47
   \rtbegs  \rtbegs  \rtsbfg  \rtcsegs | %% 48
   \rtsdfg  \rtsdfg  \rtegscs \rtegscs | %% 49
   \rtdfsa  \rtdfsa  \rtsbfg  \rtsbfg  | %% 50
   \rtcsegs \rtcsegs \rtsceg  \rtsceg  | %% 51
   \rtcsfsa \rtcsfsa \rtcsfsa \rtcsfsa | %% 52
   \rtsceg  \rtsceg  \rtsceg  \rtsceg  | %% 53
   \rtcsfsa \rtcsfsa \rtcsfsa \rtcsfsa | %% 54
   \rtsceg  \rtsceg  \rtcsfsa \rtcsfsa | %% 55
   \rtbfsa  \rtbfsa  \rtbfsa  \rtbegs  | %% 56
   \rtaegs  \rtadsfs \rtsgdf  \rtgscse | %% 57
   \rbsfcd  \rbsfcd  \rtsgcd  \rtacsds | %% 58
   \rtgscse \rtgscse \rbsfbd  \rbsfbd  | %% 59
   \rbegscs \rtgscse \rtgscse \rtgscse | %% 60
   \rtsgdf  \rtsgdf  \rtsgdf  \rtsgdf  | %% 61
   \relative c' {
     {gis8 e' cis} {gis' e cis'}
     {gis e' cis} {gis' e cis}         | %% 62
   }
   \relative c'' {
     {bis8 dis a} {bis fis a}
   }                 \rbsss   \rbsss   | %% 63
   \relative c {
     {e8 e' cis} {gis' e cis'}
     {gis e' cis} {gis' e cis}         | %% 64
   }
   \relative c'' {
     {bis8 dis a} {bis fis a}
     {dis, fis a,} {a gis fis}         | %% 65
   }
   \relative c {
     {e8 gis cis} {e cis gis}
   }                 \rtsss   \rtsss   | %% 66
   \rtsss   \rtsss   \rtsss   \rtsss   | %% 67
   \rtsss   \rtsss   \rtsss   \rtsss   | %% 68
   \rtsss   \rtsss   \rtsss   \rtsss   | %% 69
   \bar "|."
}

%%============================================================================
%% Left hand
lVoiceU = \scaleDurations 2/3 \relative c' { %% 2/3 Bass clef
%{ COMMENT OUT %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% COMMENT IN %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% %}
   \scaleDurations 3/2 {
     {s1 s s s s s s s s s} |       %% 01 .. 10
     {s1 s s s s s s s s s} |       %% 11 .. 20
     {s1}                           %% 21
   }
   \rbcsfsa \rbcsfsa \rbscfg  \rbsceg  | %% 22
   \rbfsacs \rtsss   \rtsss   \rtsss   | %% 23
   \scaleDurations 3/2 {
     {s1 s s s s} |                 %% 24 .. 30
   }
   \rbsss   \rbsss   \rbsss   \rbsss   | %% 29
   \rbsss   \rbsss   \rbsss   \rbsss   | %% 30
   \rbregs  \itcsegs \iteegs  \itcsegs | %% 31
   \jbdsafs \itbsads \rbsss   \rbsss   | %% 32
   \jbecsgs \rbsss   \rbsss   \rbsss   | %% 33
   \rbsss   \rbsss   \rbsss   \rbsss   | %% 34
   \rbsss   \rbsss   \rbsss   \rbsss   | %% 35
   \rbsss   \rbsss   \rbsss   \rbsss   | %% 36
   \relative c' {
     {s8 s s} {a bis fis}
     {a8 dis, fis} {cis fis a}         | %% 37
   }
   \relative c {
     {bis fis' gis} {a gis fis}
     {dis8 fis a} {cis, fis a}         | %% 38
   }
   \relative c {
     {bis8 fis' gis} {a gis fis}
     {d8 fis a} {cis, fis a}           | %% 39
   }
   \relative c {
     {bis8( fis' gis} {a gis fis}
     {cis8) e cis'} {cis, e cis'}      | %% 40
   }
   \relative c {
     {dis8 a' cis} {dis, a' cis}
     {dis, gis bis} {dis, fis bis}     | %% 41
   }
   \rbsss   \rbsss   \rbsss   \rbsss   | %% 42
   \rbsss   \rbsss   \rbsss   \rbsss   | %% 43
   \rbsss   \rbsss   \rbsss   \rbsss   | %% 44
   \rbsss   \rbsss   \rbsss   \rbsss   | %% 45
   \rbsss   \rbsss   \rbsss   \rbsss   | %% 46
   \rbsss   \rbsss   \rbsss   \rbsss   | %% 47
   \rbsss   \rbsss   \rbsss   \rbsss   | %% 48
   \rbsss   \rbsss   \rbsss   \rbsss   | %% 49
   \rbsss   \rbsss   \rbsss   \rbsss   | %% 50
   \rbsss   \rbsss   \rbsss   \rbsss   | %% 51
   \rbsss   \rbsss   \rbsss   \rbsss   | %% 52
   \rbsss   \rbsss   \rbsss   \rbsss   | %% 53
   \rbsss   \rbsss   \rbsss   \rbsss   | %% 54
   \rbsss   \rbsss   \rbsss   \rbsss   | %% 55
   \rbsss   \rbsss   \rbsss   \rbsss   | %% 56
   \rbsss   \rbsss   \rbsss   \rbsss   | %% 57
   \rbsss   \rbsss   \rbsss   \rbsss   | %% 58
   \rbsss   \rbsss   \rbsss   \rbsss   | %% 59
   \rbsss   \rbsss   \rbsss   \rbsss   | %% 60
   \rbsss   \rbsss   \rbsss   \rbsss   | %% 61
   \rbsss   \rbsss   \rbsss   \rbsss   | %% 62
   \rbsss   \rbsss
            \relative c' {dis8 fis a,}
            \relative c' {a8 gis fis}  | %% 63
   \rbsss   \rbsss   \rbsss   \rbsss   | %% 64
   \rbsss   \rbsss   \rbsss   \rbsss   | %% 65
   \rbsss   \rbsss
   \relative c {
     {r e gis} {cis gis e}             | %% 66
   }
   \relative c {
     {r8 cis e} {gis e cis}
     {gis cis gis} {e gis e}           | %% 67
   }
   \rbsss   \rbsss   \rbsss   \rbsss   | %% 68
   \rbsss   \rbsss   \rbsss   \rbsss   | %% 69
   \bar "|."
}

lVoiceL = \relative c {             %% 4/4 Bass clef
%{ COMMENT OUT %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% COMMENT IN %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% %}
   {<cis cis,(>1} |                 %% 01
   {<b b,>1} |                      %% 02
   {<a a,)>2 <fis fis,>} |          %% 03
   {<b b,>2 <b b,>} |               %% 04
   {<cis gis cis,>1} |              %% 05
   {<bis gis bis,>1} |              %% 06
   {<cis cis,(>2 <fis, fis,)>} |    %% 07
   {<b! b,!>2 <b b,>2} |            %% 08
   {<e e,>1} |                      %% 09
   {<e e,>1} |                      %% 10
   {<d d,>1} |                      %% 11
   {<c c,>4 <b b,>4 <ais ais,>2} |  %% 12
   {<b b,>2 e,4 g} |                %% 13
   {fis2 <fis fis,>2} |             %% 14
   {<b b,>1~} |                     %% 15
   {<b b,>4 <e e,> <g g,> <e e,>} | %% 16
   {<b b,>1~} |                     %% 17
   {<b b,>4 <e e,> <g g,> <e e,>} | %% 18
   {<b b,>2 <gis! gis,!>} |         %% 19
   {<eis eis,>2 <fis fis,>} |       %% 20
   {<b b,(>2 <bis bis,>} |          %% 21
   {cis,2) cis} |                   %% 22
   {<fis cis fis,>1} |              %% 23
   {<eis cis' eis>1} |              %% 24
   {<fis fis'>2 <dis dis'>4 <cis cis'>4} | %% 25
   {<bis gis' bis>2. <bis gis' bis>4} | %% 26
   {<cis gis' cis>2  <fis, fis'>4 <fisis fisis'>} | %% 27
   {<gis gis'>1} |                  %% 28
   {<gis gis'>1} |                  %% 29
   {<gis gis'>1} |                  %% 30
   {<gis gis'>1} |                  %% 31
   {<gis gis'>1} |                  %% 32
   {<gis gis'>1} |                  %% 33
   {<gis gis'>1} |                  %% 34
   {<gis gis'>1~} |                 %% 35
   {<gis gis'>1~} |                 %% 36
   {<gis gis'>1} |                  %% 37
   {<gis gis'>1} |                  %% 38
   {<gis gis'>1} |                  %% 39
   {<gis gis'>2 <a a'>} |           %% 40
   {<fis fis'>2 <gis gis'>} |       %% 41
   {<cis gis' cis>1} |              %% 42
   {<bis gis' bis>1} |              %% 43
   {<cis cis'>2 <fis, fis'>} |      %% 44
   {<b b'>2 <b b'>} |               %% 45
   {<e e'>1} |                      %% 46
   {<dis dis'>1} |                  %% 47
   {<e e'>2 <dis dis'>4 <cis cis'>} | %% 48
   {<bis gis' bis>2 <cis gis' cis>} | %% 49
   {<fis, fis'>2 <gis gis'>} |      %% 50
   {<cis cis'>1~} |                 %% 51
   {<cis cis'>4 <fis fis'> <a a'> <fis fis'>} | %% 52
   {<cis cis'>1~} |                 %% 53
   {<cis cis'>4 <fis fis'> <a a'> <fis fis'>} | %% 54
   {<cis cis'>2 <fis, fis'>} |      %% 55
   {<dis' dis'>2. <e e'>4} |        %% 56
   {<cis cis'>4 <dis dis'> <bis bis'> <cis cis'>} | %% 57
   {<a a'>2 <gis gis'>4 <fis fis'>} | %% 58
   {<gis gis'>2 <gis gis'>} |       %% 59
   {<< cis1 \\ {gis'2. gis8. gis16} >>} | %% 60
   {<< bis,1 \\ {gis'2. gis8. gis16} >>} | %% 61
   {<< cis,1 \\ {gis'2. gis8. gis16} >>} | %% 62
   {<< gis,1 \\ {gis'2. gis8. gis16} >>} | %% 63
   {<< cis,1 \\ {gis'2. gis8. gis16} >>} | %% 64
   {<< gis,1 \\ {gis'2. gis8. gis16} >>} | %% 65
   {<< cis,1 \\ {gis'2 cis} >>} |   %% 66
   {<< cis,1 \\ {gis'2 s} >>} |     %% 67
   \relative c, {cis2 <cis gis' cis>2} | %% 68
   \relative c, {     <cis gis' cis>1_\fermata} | %% 69
   \bar "|."
}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% PS/PDF output
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
\score
{
   \new PianoStaff
   {
     <<
       \new Staff
       {
         \signature
         \clef treble
         << \rVoiceU \\ \rVoiceL >>
       }
       \new Staff
       {
         \signature
         \clef bass
         << \lVoiceU \\ \lVoiceL >>
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
\score
{
   \new PianoStaff
   {
     <<
       \new Staff
       {
         \signature
         \rInstrument
         << \rVoiceU \\ \rVoiceL >>
       }
       \new Staff
       {
         \signature
         \lInstrument
         << \lVoiceU \\ \lVoiceL >>
       }
     >>
   }
   \midi
   {
     \tempo 4 = 52
   }
}
