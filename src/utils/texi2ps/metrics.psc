%!PS-Adobe

%% I called this file "metrics.ps" once, and "make clean" deleted it.  Sigh.

the_font findfont 1 scalefont setfont
/c 1 string def
/tmp 20 string def

32 1 255 {
  /ci exch def
  c 0 ci put
  c stringwidth
  pop
  (,) print
  tmp cvs print
  (
) print
} for

quit
