pushd %~dp0

:: Need to make sure to set Windows environments for x64 first before we call
:: our cmake commands in cmake_refresh.sh
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
cmd.exe /B /C cmake_refresh.sh
exit
