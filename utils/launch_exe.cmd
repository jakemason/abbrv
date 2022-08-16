:: This bat file is used to launch the game exe via
:: a programmable keyboard shortcut. This allows me to quickly
:: launch the game from anywhere, without needing to touch the
:: windows explorer at all.

:: "%~dp0" is the .bat code for current the directory containing
:: this file.
pushd %~dp0..\bin
abbrv.exe
exit
