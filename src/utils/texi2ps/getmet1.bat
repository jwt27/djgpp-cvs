@echo off

echo %1 %2 %3
del %2
echo float %3[] = { > %2
echo 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 >> %2
gs.exe -q -sDEVICE=pcx16 -sthe_font=%1 metrics.psc >> %2
echo }; >> %2
utod %2
