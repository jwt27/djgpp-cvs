@echo off

:again
if "%1" == "" goto done

grep %1 mkdist.bat > mkone2.bat
call mkone2.bat
del mkone2.bat

shift
goto again

:done
