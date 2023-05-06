@echo off

@Set "RootDir=%~dp0..\bin"

@del /S /Q "%RootDir%\Debug-windows-x86_64\TestApp\networkFiles"
@del /S /Q "%RootDir%\Debug-windows-x86\TestApp\networkFiles"
@del /S /Q "%RootDir%\Release-windows-x86_64\TestApp\networkFiles"
@del /S /Q "%RootDir%\Release-windows-x86\TestApp\networkFiles"

@_cpyStandardNetworks.bat

@pause