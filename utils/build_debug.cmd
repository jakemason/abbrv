:: Allows us to quickly recompile the game via
:: a useful keyboard shortcut so we don't have to mess
:: with any editor-based help with this at all
pushd %~dp0

:: Need to make sure to set Windows environments for x64 first before we call
:: our cmake commands in cmake_refresh.sh
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
cmd.exe /B /C cmake_build.sh
exit
