@echo off

@del /S /Q "Aux0\lib\Debug-windows-x86\Aux0.lib"
@del /S /Q "Aux0\lib\Debug-windows-x86_64\Aux0.lib"
@del /S /Q "Aux0\lib\Release-windows-x86\Aux0.lib"
@del /S /Q "Aux0\lib\Release-windows-x86_64\Aux0.lib"

@del /S /Q "AgentSystem\lib\Debug-windows-x86\AgentSystem.lib"
@del /S /Q "AgentSystem\lib\Debug-windows-x86_64\AgentSystem.lib"
@del /S /Q "AgentSystem\lib\Release-windows-x86\AgentSystem.lib"
@del /S /Q "AgentSystem\lib\Release-windows-x86_64\AgentSystem.lib"

@del /S /Q "CommLayer\lib\Debug-windows-x86\CommLayer.lib"
@del /S /Q "CommLayer\lib\Debug-windows-x86_64\CommLayer.lib"
@del /S /Q "CommLayer\lib\Release-windows-x86\CommLayer.lib"
@del /S /Q "CommLayer\lib\Release-windows-x86_64\CommLayer.lib"