@echo off

@Set "RootDir=%~dp0..\bin"
@Set "NetworksDir=%~dp0\standardNetworks"

@xcopy "%NetworksDir%" "%RootDir%\Debug-windows-x86_64\TestApp\networkFiles" /v /f /d /s /-y
@xcopy "%NetworksDir%" "%RootDir%\Debug-windows-x86\TestApp\networkFiles" /v /f /d /s /-y
@xcopy "%NetworksDir%" "%RootDir%\Release-windows-x86_64\TestApp\networkFiles" /v /f /d /s /-y
@xcopy "%NetworksDir%" "%RootDir%\Release-windows-x86\TestApp\networkFiles" /v /f /d /s /-y

@pause