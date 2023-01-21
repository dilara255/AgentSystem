@echo off

Set "RootDir=%~dp0..\bin"

set dt=%DATE:~6,4%_%DATE:~3,2%_%DATE:~0,2%__%TIME:~0,2%_%TIME:~3,2%_%TIME:~6,2%
set dt=%dt: =0%

TestApp_DW64.lnk > "%RootDir%\Debug-windows-x86_64\TestApp\logFiles\logDW64_%dt%.txt"
TestApp_DW86.lnk > "%RootDir%\Debug-windows-x86\TestApp\logFiles\logDW86_%dt%.txt"
TestApp_RW64.lnk > "%RootDir%\Release-windows-x86_64\TestApp\logFiles\logRW64_%dt%.txt"
TestApp_RW86.lnk > "%RootDir%\Release-windows-x86\TestApp\logFiles\logRW86_%dt%.txt"